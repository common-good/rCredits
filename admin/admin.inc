<?php
/**
 * @file
 * rCredits admin
   error_reporting(E_ALL);
  ini_set("display_errors", 1);

   include_once __DIR__ . '/../rcredits/admin/admin.inc';
 */
namespace rCredits\Admin;

use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;
use rCredits\Admin as a;
use rCredits\Web as w;

/**
 * Set up the basic user accounts and relations on the server.
 * called from installation and test setup
 */
function setupBasicAccounts() {
  $keys = u\ray('uid community name fullName email postalCode country minimum rebate flags');

  $q = db\q('SELECT :R_REGION_FIELDS FROM r_regions');
  while ($row = $q->fetchAssoc()) { // create or update all regions
    extract($row);
    if (!$hasServer) continue;
//    if (isDEV and !in_array($region, [R_SERVER_ID])) continue; // don't slow the tests down
    if ($uid = db\lookup('uid', 'users', "name=CONCAT(:region, '.')", compact('region'))) {
      r\acct($uid)->update(compact(u\ray('postalCode federalId')));
    } else {
      $fullName = $fullName ?: db\lookup('name', 'r_states', 'abbreviation=:st', compact('st'));
      $email = u\shortName($fullName) . "@rCredits.org";
      $fullName = 'rCredits ' . $fullName;
      $uid = r\serverUid($region);
      $state = db\lookup('id', 'r_states', 'abbreviation=:st', compact('st'));
      $values = array($uid, $uid, $region . '.', $fullName, $email, $postalCode, R_COUNTRY_ID, 0, 0, u\bit(B_CO) | u\bit(B_OK));
      $info = array_combine($keys, $values);
      $info += compact(u\ray('postalAddr state federalId'));
      new r\acct($info);
    }
  }
  //$region = db\lookup("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));

  $a1 = r\acct(1); // set up system admin record (uid=1)
  if (isDEV) $a1->update('pw2', r\passHash(DEV_PW2));
  $flags = $a1->flags;
  foreach (array(B_ADMIN, B_CO) as $bit) u\setBit($flags, $bit);
  $a1->update('fullName flags', t('System Administrator'), $flags);

  $server = r\serverUid();
  if ($server == r\serverUid('NEW')) { // no need to set up CGF unless in Western Mass (or testing)
    if (db\exists('users', "name='cgf'")) return; // already done

    $values = array(r\unQuid('NEW.AAB'), $server, 'cgf', 'Common Good Finance', CGF_EMAIL, '01330', US_COUNTRY_ID, 0, R_REBATE, u\bit([B_MEMBER, B_CONFIRMED, B_OK, B_CO]));
    $info = array_combine($keys, $values);
    list ($phone, $address, $city, $state) = array(CGF_PHONE, CGF_ADDRESS, CGF_CITY, R_STATE_ID);
    $helper = 1; //$wws->id;
    $info += compact(u\ray('email phone address city state helper'));
    $cgf = new r\acct($info);
    \variable_set('cgf_uid', $cgf->id);
//    $cgf->newRelation($wws->id, r\perm(B_MANAGE));
  }
}

/**
 * Re-setup the test cards on the STAGING server, after importing data from the PRODUCTION server.
  f('a.makeTestCards');
 */
function makeTestCards() {
  if (isPRODUCTION) return;

  $a = a('aab');
  $selling = "groceries\ngifts\nsundries\ndeli\nbaked goods";
  $a->update('name fullName selling', 'cornerstore', 'Corner Store', $selling);
//  $a->setBitx(CO_REQUIRE_CASHIER, FALSE, 'coFlags');

  $a = a('aaa');
  $a->update('name fullName cardCode2 pin', 'bobbossman', 'Bob Bossman', 'WeHlioM5JZv1O9G', '1234');
  a\shrinkPhoto($a);
  
  $a = a('aad');
  $a->update('name fullName cardCode2 pin', 'cathycashier', 'Cathy Cashier', 'ME04nW44DHzxVDg', '1234');
  a\shrinkPhoto($a);
  
  $a = a('aaq');
  $a->update('name fullName', 'helgas', 'Helga\'s Hardware');
//  $a->setBitx(CO_REQUIRE_CASHIER, TRUE, 'coFlags');
  a\shrinkPhoto($a);
  
  $a = a('aak');
  $a->update('name fullName cardCode cardCode2 pin', 'curtcustomer', 'Curt Customer', 'NyCBBlUF1qWNZ2k', 'utbYceW3KLLCcaw', '1234');
  a\shrinkPhoto($a);
  
  $a = a('abb');
  $a->update('name fullName cardCode pin', 'susanshopper', 'Susan Shopper', 'ZzhWMCq0zcBowqw', '1234');
  a\shrinkPhoto($a);
}

/**
 * Return a display of steps done and steps remaining (to open an account).
 */
function showSteps($a) {
  $steps = '';
  if (!$a->ok) foreach ($a->stepsDone ?: r\stepsDone0() as $k => $done) { // :? for testing
    $color = $done ? 'silver' : 'red';
    if ($a->co) {
      if ($k == 'proxies') $color = 'white';
    } elseif ($k == 'company' or $k == 'relations') $color = 'white';
    $steps .= "<b style=\"color:$color;\">$k </b>";
  }
// (Takes too long)  if (!$a->ok) $steps .= ' [Next step: <b style="color:red;">' . ($a->nextStep() ?: 'NONE!') . '</b>] ';
  return $steps;
}

/**
 * Return a description of the account's recent purchase activity (sales activity for companies).
 */
function showActivity($a) {
  $uid = $a->id;
  $cgf = r\cgfId();
  $when = u\plusMonths(-3);
  $count = $a->co ? db\count('r_txs', 'payee=:uid AND created>:when', compact('uid', 'when'))
    : db\count('r_txs', 'payer=:uid AND payee<>:cgf AND created>:when', compact(u\ray('uid cgf when')));
  $ret = t('@count @what', u\ray('@count @what', $count, $a->co ? t('sales') : t('purchases')));
  $subs = u\ray('@activated @signed @use', $a->ok ? u\fmtDate($a->activated) : t('NO'), $a->signed ? u\fmtDate($a->signed) : t('NO'), u\fmtDate($a->ok ? $a->access : $a->login));
  $ret .= t(' in the past 3 months (signed @signed, activated @activated, last use/signin @use)', $subs);
  return $ret;
}

/**
 * Return a list of the account's relations.
 */
function showRelations($a) {
  global $base_url;
  $uid = $a->id;
  $ret = '';
  $sql = <<<EOF
    SELECT IF(r.main=:uid, other, main) AS them, main, IF(r.main=:uid, u2.fullName, u1.fullName) as who, 
      permission, employee, isOwner, draw,
      u1.:IS_CO AS iCo, u2.:IS_CO AS uCo
    FROM r_relations r LEFT JOIN users u1 ON u1.uid=r.main LEFT JOIN users u2 ON u2.uid=r.other
    WHERE :uid IN (r.main, r.other)
EOF;
  $q = db\q($sql, compact('uid'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $permission += B_RELATED; // shift to use bit names
    $extra = '';
    if ($isOwner and !$iCo and !$uCo) {
      $role = t('related to');
    } elseif ($uid == $main) {
      $rel = r\quid($main, $them);
      $extra = '<small> (' . r\quid($them) . ' / ' . spinLink("$base_url/sadmin/panel/qid=$rel", $rel, '', 'primary', 'xs') . ')</small>';
      $role = $isOwner ? t('owned by')
        : ($draw ? t('drawn on by')
        : ($permission >= B_MANAGE ? t('managed by')
        : (($permission >= B_SCAN or $employee) ? t('employs')
        : ($permission >= B_READ ? t('readable by')
        : t('? by')))));
    } else {
      $role = $isOwner ? t('owns')
        : ($draw ? t('draws from')
        : ($permission >= B_MANAGE ? t('manages')
        : (($permission >= B_SCAN or $employee) ? t('works for')
        : ($permission >= B_READ ? t('can read')
        : t('can ?')))));
    }
    $ret .= ($ret ? ', ' : '') . "$role <a href=\"$base_url/sadmin/acct-info/$them\">$who</a>$extra";
  }
  return $ret;
}

/**
 * List the USD account status of each account.
 *//*
function usdAccts($where = "postalCode LIKE '013%'") {
  if (!r\acct()->admin) return;
  $result = db\q("SELECT uid FROM users WHERE $where ORDER BY fullName");
  $head = u\ray('Name Dwolla? Type Verified OK Co Bal');
  while ($row = $result->fetch()) {
    $usA = new r\usd($a = r\acct($row->uid));
    $bal = $usA->bal($err);
    $hasDwolla = $err ? '-' : 'Yes';
    $source = $usA->source($err);
    $type = @$source['Type'];
    $verified = @$source['Verified'] ? 'Yes' : '-';
    $count = @$source['count'];
    
    $ok = $a->ok ? 'ok' : '';
//    $charge = $a->can(B_CHARGE) ? 'Chg' : '';
    $company = $a->co ? 'Co' : '';
    $line = array($a->fullName, $hasDwolla, $type, $verified, $ok, $company, $bal);
    $lines[] = array_combine($head, $line);
  }
  return showRecords($lines);
}*/
define('ID_KEYS', 'id uid t.uid myid payer payee main other inviter invitee proxy person owner defaultAgent');

function showRecords($ray) {
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $k => $v) {
      $v = a\formatField($k, $v);
      if ($k == 'for2' and $v == $row->for1) $v = 'same';
      $line .= "  <td>$v</td>\n";
    }
    $lines[] = $line;
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>\n", $lines) . "</tr>\n</table>";
}

/**
 * Return the given field, formatted for viewing.
 */
function formatField($k, $v) {
  if ($k == 'bankAccount' and u\starts($v, '~')) $v = 'c' . $v; // FUDGE to repair broken crypt flag (?)
  while (u\starts($v, CRYPT_FLAG)) $v = u\decrypt($v);
  if ($v and in_array($k, u\ray(ID_KEYS)) and strlen($v) >= strlen(r\cgfId())) {
    if ($a = r\acct($v)) $v = $a->name; else $v .= t(' (missing)'); // eg, when uid is changed
  } // member ID. Don't include agent/payerAgent/etc. here
  if ($v and $k == 'channel') $v = u\rayv(u\ray(TX_CHANNELS), $v);
  if ($k == 'reid') $v = str_replace(R_MEMBER_MARK, R_AGENT_MARK, r\quid($v));
  if ($v and in_array($k, u\ray(R_DATE_FIELDS))) $v = '<div style="line-height:65%;">' . strftime('%d%b', $v) . '<span style="font-size:50%;">\'' . strftime('%y', $v) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $v) . '</span></div>';
  if ($k == 'flags') $v = "<div style='text-transform:none; font-size:7px; line-height:11px;'>" . a\flagsDesc($v, B_LIST) . '</div>';
  return $v;
}

/**
 * Display the given table or query.
 * @param string $table: table name
 * @param string $where: criteria
 * @param string $order: row order
 * @param string $fields: list of fields to display
 * @param array $joinFields: list of fields joining to users table (limit by community if called by Ctty Admin)
 */
function showQuery($table, $where = '1', $order = '', $fields = '*', $joinFields = []) {
  $limit = 500; // max records to show
  
  $mya = r\acct();
  if (!$admin = $mya->admin) {
    if ($table != 'users') {
      if (!is_array($joinFields)) $joinFields = array($joinFields);
      $fieldCount = count($joinFields);
      $table .= ' t';
      for ($fieldI = 0; $fieldI < $fieldCount; $fieldI++) {
        $table .= " INNER JOIN users u$fieldI ON u$fieldI.uid=t." . $joinFields[$fieldI];
        $crit = (@$crit ? "$crit OR " : '') . "u$fieldI.community=$mya->community";
      }
      $where .= " AND ($crit)";
    } else $where .= " AND community=$mya->community";
  } else $fields = str_replace('t.', '', $fields);

  if ($order) $where .= " ORDER BY $order";
  return showRecords(db\q("SELECT $fields FROM $table WHERE $where LIMIT $limit")->fetchAll());
}

/**
 * Show results of an SSN search, compared to personal data supplied by the member.
 * Updates the moves field, if it isn't set yet.
 */
function ssnCheck($a) {
  if (isDEV) return t('no SSN data');
  if (!$ssnData = @$a->ssnData) {
    if (!$ssnData0 = u\ssnLookup($a->federalId)or strpos($ssnData0, 'RequestTimedOutException')) return w\say(t('No data returned from ssnLookup (probably timed out).'));
//    if (strpos($ssnData0), 'timeout')
    $xml = simplexml_load_string($ssnData0);
    $json = json_encode($xml);
    $ssnData = json_decode($json, TRUE);
    if (!is_array($d = $ssnData['searchResults']) or $ssnData['ResultCount'] + 0 <= 0) {
/**/  return w\say(t('No SSN data available: ') . print_r($ssnData0, 1));
    }
    unset($ssnData['searchResults']['ssn']); // has first 5 digits of SSN -- don't store it
    $a->update(compact('ssnData')); // ResultCount is 0 for tests, <=0 for failure
  } else $d = @$ssnData['searchResults'] ?: []; // [] for admin without pw2
  
  $name = $a->fullName . ($a->legalName == $a->fullName ? '' : "<br>$a->legalName");
  $rows[] = array(t('Data Given'), $name, u\fmtDate(is_numeric($a->dob) ? $a->dob: 0), r\location($a, TRUE), '', ''); // is_numeric check for admin without pw2
  $name = ssnName($one = (object) @$d['ssnnames']['ssnname']);
  $dob = $one->dob ? u\fmtDate(strtotime($one->dob)) : ''; // arrives as an empty array if none
  $rows[] = array(t('Found'), $name, $dob, @$d['deathsearchresults'] ? 'DEAD' : '', '', '');
  
  $counties = [];
  $addresses = @$d['addresses']['address'] ?: [];
  if (@$addresses['lastname']) $addresses = [$addresses];
  foreach($addresses as $rec) {
    $rec = (object) $rec;
    $rows[] = array($rec->datereported ? u\fmtDate(strtotime($rec->datereported)) : '', ssnName($rec), $rec->dob ? u\fmtDate(strtotime($rec->dob)) : '', ssnAddr($rec), ucwords(strtolower($rec->county)));
    if (r\rTime() - @strtotime($rec->datereported) < 10 * YEAR_SECS) $counties[$rec->county] = 1;
  }
  if (!@$a->moves) $a->update('moves', max(0, count($counties) - 1));
  
  $lines = '';
  foreach ($rows as $rec) {
    list ($head, $name, $dob, $address, $county) = $rec;
    $lines .= "<tr><th>$head</th><td>$name</td><td>$dob</td><td>$address</td><td>$county</td></tr>\n";
  }
  
  return <<< EOF
  <table>
  <tr><th>SSN CHECK</th><th>Name</th><th>BDate</th><th>Address</th><th>County</th></tr>
  $lines
  </table>
EOF;
} 

function ssnName($v) {
  foreach (u\ray('firstname middlename lastname') as $k) $$k = $v->$k ?: ''; // convert [] to ''
  return ucWords(strtolower(str_replace('-', '', @"$firstname $middlename $lastname")));
}
function ssnAddr($v) {return ucWords(strtolower(@"$v->address, $v->city, $v->state $v->zip"));}

/**
 * Redo all the stats
 * @param int $keepUpto: date of last valid stats records (redo all subsequent dates)
include_once __DIR__ . '/../rcredits/admin/admin.inc';
f('a.fixStats');
 */
function fixStats($keepUpto = 0) {
//  include_once __DIR__ . '/../rcron/rcron.inc';
  $blockAgo = strtotime(-STATS_GRAIN, strtotime('today'));
  
  $sql = <<<EOF
    SELECT community AS ctty, MIN(t.created) AS start
    FROM users u LEFT JOIN r_txs t ON t.payee=u.uid 
    WHERE uid>1 AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
EOF;
  $cttys = db\q($sql)->fetchAllKeyed();
  u\preray(array(0 => min($cttys)), $cttys); // show all dates overall

  $now = r\rTime();
  db\q($keepUpto ? "DELETE FROM r_stats WHERE id>$keepUpto" : 'TRUNCATE r_stats');
  $start0 = $keepUpto ? strtotime('+' . ($keepUpto < $blockAgo ? STATS_GRAIN : 1), db\lookup('MAX(created)', 'r_stats')) : (@$cttys[0] ?: $now);
  $created = strtotime('today', $start0); // use first moment of the day
  
  while ($created <= $now) {
  /*
    $acctQ = db\q('SELECT uid FROM users WHERE uid>1');
    db\q('UPDATE users SET picture=0'); // clear temp field  
    while ($row = $acctQ->fetchAssoc()) r\Cron\acctStats1($row, $created); // set individual account stats
    */
    foreach ($cttys as $ctty => $start) if ($created >= $start) {
      r\getStats($ctty, $created); // set community stats
/// this fails on cross-community txs!  if ($ctty != 0) if ($err = badStats($ctty, $created)) return debug($err); // verify
    }
    $delta = $created < $blockAgo ? STATS_GRAIN : 1;
    $created = strtotime("+$delta days", $created);
  }
}
  
/**
 * Check consistency with individual stats
 * @param int $ctty: record ID of community to check
 * @param int $created: stats date to check
 */
function badStats($ctty, $asof) {
  $q = db\q('SELECT uid FROM users WHERE community=:ctty AND uid > 1', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    extract(u\just('uid', $row));
    $r = be\creditInfo(compact('uid','asof'))->r;
    $total = @$total + $r;
  }
  $rStat = db\lookup('r', 'r_stats', 'ctty=:ctty AND created=:asof', compact('ctty', 'asof'));
  if ($rStat == round($total, 2)) return FALSE;
  return ['STATS_ERROR' => compact(u\ray('ctty asof rStat total'))];
}

/*
  $r = $usd = 0;
  $q = db\q("SELECT id, created, pAccts, bAccts FROM r_stats WHERE ctty<>0 ORDER BY created");
  $new = r\acct('R_ADMIN_QID')->community;
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $crit = 'BETWEEN :created AND :created+:DAY_SECS-1';
    //$r += db\lookup('SUM(amount)', 'r_txs', 'payer<0 AND created ' . $crit, compact('created'));
    //$usd += db\lookup('-SUM(amount)', 'r_usd', 'payee=0 AND completed ' . $crit, compact('created'));
    //$newbs = db\count('users', 'created<:created', compact('created')) - $pAccts - $bAccts;
///    debug(compact(u\ray('id created r usd newbs users'))); break;
//    db\update('r_stats', compact(u\ray('r usd newbs id')), 'id');

    $newbs = db\count('users', 'created<:created AND community=:new', compact('created', 'new')) - $pAccts - $bAccts;
    $id = db\lookup('id', 'r_stats', 'created=:created AND ctty<>0', compact('created'));
    db\update('r_stats', compact(u\ray('newbs id')), 'id');
  }
*/
function pioneerBonus($a) {
  if (!$a->ok or !$a->can(B_BONA) or $a->co) return;
  if (db\exists('r_txs', "payee=:uid AND payer=-8915 AND amount=100", u\ray('uid', $a->id))) return;
/**/  debug("Pioneer bonus to $a->fullName");
  be\fund($a->id, TX_SIGNUP, 100, 'pioneer bonus');
}


// MEMBER LIST: SELECT fullName, city FROM users WHERE (postalCode LIKE '010%' OR postalCode LIKE '013%') AND NOT (flags&(1<<6)) AND NOT (flags&(1<<9)) ORDER BY city, fullName

/**
 * Read and parse an updated list of financial institution routing numbers into the r_banks table.
 * Download from http://www.fededirectory.frb.org/agreement.cfm
 * branch is o (main office) or B (branch)
 * type is 0=fed bank 1=use routing 2=use newRouting
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  f('a.getBanks');
 */
function getBanks() {
  return; // seldom used (and move SQL name fixes here? see changes.log)
  $fieldNames = 'route branch fedRoute type modified newRoute name address city state zip phone status view';
  $lens = u\ray('9 1 9 1 6 9 36 36 20 2 9 10 1 1'); // also 5 chars of filler at end
  $fields = array_combine(u\ray($fieldNames), $lens);
  db\q('TRUNCATE r_banks');
  
  $s = explode("\n", file_get_contents(__DIR__ . '/../FedACHdir.txt'));
  foreach ($s as $line) {
    if (!$line) continue; // handle possible blank line at end
    $i = 0;
    foreach ($fields as $field => $len) {
      $$field = substr($line, $i, $len);
      $i += $len;
    }
    $branch = $branch == 'B' ? 1 : 0;
    foreach (u\ray('name address city') as $field) $$field = ucwords(strtolower(trim($$field)));
    db\insert('r_banks', compact(u\ray($fieldNames)));
  }
}

/**
 * Create a smaller photo to send to POS devices
 * CALLED ONLY FROM a\makeTestCards()!
 * @param acct $a: the account whose photo is to be shrunk
 * @param string $err: (RETURNED) the error message, if any
 * @return <success>
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  debug (f('a.shrinkPhoto', a('aaa'), $err));
  debug ($err);
 */
function shrinkPhoto($a, &$err = '') {
  if ($a->co) return !$err = 'tried to shrink a company photo';
//  if ($a->hasPhoto('picture2')) return !$err = 'POS photo already exists';
  if (@$a->photo) $err = 'POS photo already exists'; // not really an error
  $photoPath = DRUPAL_ROOT . $a->photoFilename(TRUE);
  $img = u\alterImg(imagecreatefromjpeg($photoPath), R_PHOTO_WIDTH, R_PHOTO_HEIGHT);
  ob_start();
  imagejpeg($img, NULL, 50);
  $photo = ob_get_clean();
  //if (!$photo = u\fixPicture($photoPath, R_PERSONAL_ASPECT, R_PIXEL_FACTOR / 2, FALSE, $err)) return FALSE;
  return $a->update(compact('photo'));
}

/**
 * Move a field out of data or secure and into its own field in users.
 * (call after reinstall)
   include_once __DIR__ . '/../rcredits/admin/admin.inc';
  f('a.moveFieldFrom', 'helper', 'data');
 */
function moveFieldFrom($field, $from = 'data') {
  $uids = db\q('SELECT uid FROM users WHERE uid>1')->fetchCol();
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    $ray = $a->$from;
    $$field = @$ray[$field]; // not $$from[] because PHP attaches the index first
    if ($field = 'helper') {
      $$field = $$field ? r\acct($$field)->id : 0;
    }
    unset($ray[$field]);
    $$from = $ray;
    $a->update(compact($from, $field));
  }
}

/**
 * Move a field into data or secure in users.
   include_once __DIR__ . '/../rcredits/admin/admin.inc';
  f('a.moveFieldTo', 'question', 'secure');
 */
function moveFieldTo($field, $to = 'data') {
  $q = db\q("SELECT uid,$field FROM users WHERE uid>1");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    $v = $a->$to;
    $v += compact($field);
    $a->update($to, $v);
  }
}

/**
 * General purpose utility, usually called from eachAcct, as above.
 * For example:
 *    changeIt($a);
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  foreach (array(3201932, 3244519, 3350610, 3510402) as $one) {
  f('a.changeIt', $one);
OR changeIt(a('aaa'));
}
 */
function changeIt($a) {
  if ($a->bankAccount) $a->setRisk('hasBank');
  $a->setBit(4, FALSE);
}

function fixRelations() {
  $q = db\q('SELECT reid, main, other, permission, isOwner, draw, employee FROM r_relations');

  while ($row = $q->fetchAssoc()) {
    extract($row);
    list ($mainName, $otherName) = array(r\acct($main)->fullName, r\acct($other)->fullName);
    if (!$permission and !$isOwner and !$draw and !$employee) {
      db\q('DELETE FROM r_relations WHERE reid=:reid', compact('reid'));
/**/  debug("Record DELETED: $mainName -- $otherName");
    } else if ($reid < 999999) {
      $region = r\acct($main)->region;
      if ($region == r\serverUid()) {
        $newReid = -$region + $reid;
        db\q('UPDATE r_relations SET reid=:newReid WHERE reid=:reid', compact('reid', 'newReid'));
      } else { // foreign region
        $newReid = r\acct($main)->newRelation($other, $permission, $employee, $isOwner, $draw);
        db\q('DELETE FROM r_relations WHERE reid=:reid', compact('reid'));
      }
/**/  debug("Record MOVED: $reid to $newReid ($mainName -- $otherName)");
    }
  }
}

/**
 * Return a readable list of names of bits set in the given integer bit array.
 * @param int $n: the bit array
 * @param string $list: delimited list of bit names
 * @return list of bit names set in $n
 */
function flagsDesc($n, $list) {
  $bits = u\ray($list);
  if (!is_numeric(key($bits))) $bits = array_keys($bits);
  for ($i = 0; $i < 32; $i++) if (!@$bits[$i]) $bits[$i] = $i; // show bit number if no name
  $res = '';
  for ($i=0; $i <= B_MAX; $i++) if ($n & u\bit($i)) {$res .= $bits[$i] . ' ';}
  return $res;
}

/**
 * Return a table of deposits.
 */
function deposits() {
  $sql = <<<EOF
    SELECT SUM(IF(amount<0, -amount, 0)) AS amountIn, SUM(IF(amount<0, 1, 0)) AS countIn,
      SUM(IF(amount>0, -amount, 0)) AS amountOut, SUM(IF(amount>0, 1, 0)) AS countOut,
      deposit
    FROM r_usd
    GROUP BY deposit ORDER BY deposit<>0, deposit DESC, amount>0
EOF;

  $q = db\q($sql); // include today's potential deposit at the top
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $date = $deposit == 1 ? t('<b>ANCIENT</b>') : ($deposit ? u\fmtDate($deposit) : t('<b>TODAY</b>'));
    foreach (['In', 'Out'] as $way) {
      list ($total, $count) = [u\fmtAmt(${"amount$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      $details = t('<@a>details</a>', ['@a'=>"a href=deposit-details/way=$way&date=$deposit&total=$total&count=$count"]);
      $checks = t('<@a>checks</a>', ['@a'=>"a href=checks/way=$way&date=$deposit&mark=0"]);
      $wayDesc = $way == 'In' ? t('IN') : t('OUT');
      $deposits[] =<<<EOF
<tr>
  <td>$date</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details &nbsp; $checks</td>
</tr>
EOF;
    }
  }
  $deposits = @$deposits ? join("\n", $deposits) : '<tr><td colspan=5>No deposits</td></tr>';
  $header = '<tr><th>Deposit</th><th></th><th>Count</th><th>Total</th><th></th></tr>';
  return <<<EOF
<table id="deposits">
$header
$deposits
</table>
EOF;
}

/**
 * Print a check
 * @param Pdf $ck: the Check object to place the check in.
 * @param int $pos: what position on the current page (1, 2, or 3)
 * @param assoc $tx: relevant fields from the r_usd record
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printCheck(&$ck, $pos, $tx, $mark) {
  extract(u\just('txid created payer amount deposit bankAccount', $tx));
  $escrow = 'USkk' . ESCROW_ROUTING . ESCROW_ACCOUNT;
  $a = r\acct($payer);
  $bankAccount = ($pw2 = @$_COOKIE['pw2']) ? u\decrypt($bankAccount, 0, $pw2) : '';
  if (substr($bankAccount, 0, 2) <> 'US' and ($bankAccount = $a->bankAccount) and $pw2) { // bank account got screwed up somehow -- probably double-encrypted (happened again on 2/10/2016)
    db\update('r_usd', u\ray('txid bankAccount', $txid, u\crypt($bankAccount, 0, $pw2)), 'txid');
    w\say('fixed bankAccount for ' . $a->fullName);
  }
  list ($fromName, $addr, $phone, $fromAcct, $toName, $toAcct) = $amount < 0 // which way
    ? [$a->legalName, $a->postalAddr, $a->phone, $bankAccount, 'rCredits', $escrow] // from bank to escrow
    : ['rCredits (' . CGF_LEGALNAME . ')', CGF_POSTALADDR, CGF_PHONE, $escrow, $a->legalName, $bankAccount];//e2b
  $memo = $amount < 0 ? t('for rCredits') : t('for DEPOSIT to account #') . substr($toAcct, 4 + 9);

  list ($signature, $belowLine) = $amount < 0 
  ? [t('Pre-authorized Check'), t('No Signature Required')] 
  : ['', SIGNATORY];
  
  list ($W, $H) = array($ck->pageW, $ck->pageH);
  $lineH = $ck->lineH; // standard line height in inches
  $m = .25; // left/right margins
  $mt = ($pos - 1) * CHECK_HEIGHT + $m; // top margin

  $phone = u\fmtPhone($phone);
  $routing = substr($fromAcct, 4, 9);
  $account = substr($fromAcct, 4 + 9);
  
  $bankInfo = db\lookup('*', 'r_banks', 'route=:routing', compact('routing'));
  if (!$bankInfo) {w\say("Skipped transaction #$txid for $a->fullName (no bank info)"); return;}
  //u\EXPECT((bool) $bankInfo, 'no bank info');
  extract($bankInfo, EXTR_PREFIX_ALL, 'b');
  if (strlen($b_zip) > 5) $b_zip = substr($b_zip, 0, 5) . '-' . substr($b_zip, 5);
  if (!$prefix = db\lookup('id', 'r_transit', 'location=:location', u\ray('location', "$b_city, $b_state"))) {
    $prefix = db\lookup('id', 'r_transit', 'location=:b_state', compact('b_state'));
    //u\EXPECT((bool) $prefix, 'no transit prefix');
  }
  $transit = $prefix . '-' . substr($routing, 4, 4) . '/' . substr($routing, 0, 4);
  
  $amount = abs($amount);
  $dollars = floor($amount);
  $cents = round(100 * ($amount - $dollars)) ?: t('NO');

  $ck->say("<b>$fromName</b><br><small>$addr<br>$phone</small>", $m, $y = $mt, $W - $m);
  $ck->say("$transit<br>$account", $W / 2, '', $W / 4, '', '8', 'C');
  $ck->say("<b>$txid</b>", -$W / 4, '', -$W / 4 - $m, '', '', 'R');

  $ck->say('DATE:', 6.7, $y += .4, '', .25, '8');
  $ck->say(u\fmtDate($deposit ?: r\rTime()), 7.1, '', '', .25);

  $ck->say('Pay to the<br>Order of', $m, $y += .4, '', 2 * $lineH, '8;CAPS');
  $ck->say("  $toName", $m + .7, $y + $lineH, 5.9, $lineH, '', '', 'BR');
  $ck->say('$ ' . number_format($amount, 2), 7, '');
  
  $ck->say(u\n2english($dollars) . " and $cents/100", $m, $y += .65, 6.9, $lineH, '', '', 'B');
  $ck->say('DOLLARS', $m + 6.9, '', '', $lineH, '8');
//  $ck->Image(__DIR__ . '/../images/icons/padlock.gif', $W - $m - .5, $y, $lineH, $lineH);
  
  $ck->say("<b>$b_name</b><br><small>$b_address<br>$b_city, $b_state $b_zip</small>", $m + 1, $y += .4, '', '', '9');
  
  $ck->say($signature, $W - $W / 3, $y + .35, $W / 3 - $m, $lineH, 'CAPS', 'C', 'B'); // $y+ not +=
  $ck->say($belowLine, '', $y + .35 + $lineH, $W / 3 - $m, $lineH, '8;CAPS', 'C');
//  if (!$signature) $ck->Image(__DIR__ . '/../images/checksig.png', $W - $W / 3, $y, $W / 3 - $m, 5 * $lineH);
  
  $ck->say('MEMO:', $m, $y += .5, '', $lineH, '8');
  $ck->say($memo, $m + .5, '');

  // clear band must be at least .625" high, at least 0.1875" from bottom (so leave more). Micr should be 12pt
//  if ($a->id != 26742000000042) {
//    $ck->say("A{$routing}A {$account}C $txid", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR 12');
//  } else 
  $account = str_replace('-', 'D', $account); // show dashes properly
  $ck->say("C000{$txid}C A{$routing}A {$account}C", $m, $mt - $m + CHECK_HEIGHT - .25 - $lineH * (12 / PDF_FONT_SIZE), '', '', 'GnuMICR;12');
//  $ck->say('', 0, $mt - $m + CHECK_HEIGHT, $W, 1, '', '', 'T'); // comment this out
  
  if (!$deposit and $mark) db\update('r_usd', u\ray('deposit txid', strtotime('today'), $txid), 'txid');
}

/**
 * Do automatic stuff that can be done only when admin signs in
 */
function adminSignin() {
  $pw2 = @$_COOKIE['pw2'];
  u\EXPECT($pw2, 'no pw2!');
  
  $q = db\q("SELECT txid,payer,bankAccount FROM r_usd WHERE bankAccount<>'' and bankAccount NOT LIKE ':CRYPT_FLAG%'");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $bankAccount = u\crypt($bankAccount . '', 0, $pw2);
    db\update('r_usd', compact('txid', 'bankAccount'), 'txid');
    w\say(t('bank account encrypted in exchange record #TXID.', ['TXID'=>$txid]));
    $a = r\acct($payer);
    if ($bankAccount = $a->bankAccount and !u\starts($bankAccount, CRYPT_FLAG)) {
      $bankAccount = u\crypt($bankAccount . '', 0, $pw2); // might be different (newer) than in r_usd
      $a->update(compact('bankAccount'));
      w\say(t('bank account encrypted for user USER', ['USER'=>$a->fullName]));
    }
  }
}

/*
function vote20141214($uid = FALSE) {
  $info = ['noFrame'=>TRUE, 'subject'=>'rCredits celebration! when can you come?'];
  $sql = $uid ? 'SELECT uid,fullName,mail AS email FROM users WHERE NOT :IS_NONUDGE AND community=:ctty AND uid=:uid AND NOT :IS_CO ORDER BY fullName'
    : 'SELECT uid,fullName,mail AS email FROM users WHERE NOT :IS_NONUDGE AND community=:ctty AND uid>1 AND NOT :IS_CO ORDER BY fullName';
  $q = db\q($sql, ['ctty'=>r\serverUid()] + ($uid ? compact('uid') : []));
  for ($i = 0; $row = $q->fetchAssoc(); $i++) {
    $action = r\acct($row['uid'])->makeDo0('vote', '20141214');
    $row += $info + compact('action');
    w\say("mailing to ".$row['fullName']);
    r\rMail('survey2014-12-14', $row['email'], $row); // strangely, $row + $info + compact('action') fails here
  }
  w\say($i . ' emails sent.');
}

function rsvp20141220($uid = FALSE) {
  $info = ['noFrame'=>TRUE, 'subject'=>'rCredits Celebration / rCredits Seminar'];
  $sql = $uid ? 'SELECT uid,fullName,mail AS email FROM users WHERE NOT :IS_NONUDGE AND community=:ctty AND uid=:uid AND NOT :IS_CO ORDER BY fullName'
    : 'SELECT uid,fullName,mail AS email FROM users WHERE NOT :IS_NONUDGE AND community=:ctty AND uid>1 AND NOT :IS_CO ORDER BY fullName';
  $q = db\q($sql, ['ctty'=>r\serverUid()] + ($uid ? compact('uid') : []));
  for ($i = 0; $row = $q->fetchAssoc(); $i++) {
    $action = r\acct($row['uid'])->makeDo0('yesno', '20141220');
    $yes = "$action/yes=1";
    $no = "$action/no=1";
    $row += $info + compact('yes', 'no');
    w\say("mailing to ".$row['fullName']);
    r\rMail('rsvp2014-12-20', $row['email'], $row); // strangely, $row + $info + compact('action') fails here
  }
  w\say($i . ' emails sent.');
}

function endofyear2014($uid = FALSE) {
  $info = ['noFrame'=>TRUE, 'subject'=>'rCredits Pilot Completed! / Seminar TONIGHT'];
  $where = $uid ? 'uid=:uid' : 'NOT :IS_NONUDGE AND community=:ctty AND uid>1 AND NOT :IS_CO ORDER BY fullName';
  $sql = 'SELECT uid,fullName,mail AS email FROM users WHERE ' . $where;
  $q = db\q($sql, $uid ? compact('uid') : ['ctty'=>r\serverUid()]);
  for ($i = 0; $row = $q->fetchAssoc(); $i++) {
    $row += $info;
    w\say("mailing to ".$row['fullName']);
    r\rMail('endofyear2014', $row['email'], $row); // strangely, $row + $info + compact('action') fails here
  }
  w\say($i . ' emails sent.');
}

function moveFieldsToHidden() {
  $q = db\q("SELECT uid FROM users WHERE uid<>0");
  $dataFields = [];
  $secureFields = u\ray('photoId pin bankAccount');
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    if ($a->pin or $a->bankAccount) continue; // don't do this twice (and overwrite with nulls)!
    $data = $a->data;
    $secure = $a->secure;
    foreach ($dataFields as $k) $$k = @$data[$k];
    foreach ($secureFields as $k) $$k = @$secure[$k];
    $data = $secure = [];
    foreach (u\ray(R_DATA_FIELDS) as $k) if (!is_null($a->$k)) $data[$k] = $a->$k;
    foreach (u\ray(R_SECURE_FIELDS) as $k) if (!is_null($a->$k)) $secure[$k] = $a->$k;
///    debug($row + compact($dataFields, $secureFields, 'data', 'secure'));
    $a->update(compact($dataFields, $secureFields, 'data', 'secure'));
  }
}
*/

/**
 * Make sure the very secure fields are encrypted with the admin's extra password.
 */
function secureVery($a = '') {
  if (!r\acct()->admin) return; // don't bother if not admin
  if ($a) {
    if ($vflds = u\just(R_VSECURE_FIELDS, $a->secure)) return $a->update($vflds); // extra encryption
  } else {
    $q = db\q('SELECT uid FROM users WHERE :IS_OK');
    while ($row = $q->fetchAssoc()) secureVery(r\acct($row['uid']));
  }
}

/**
 * Show sluggish accounts
 */
 /*
function showSlugs() {
  $q = f('db.q', 'SELECT u.uid, u.fullName, (SELECT created FROM r_txs WHERE payer=u.uid AND payee>0 AND payee<>26742000000002 ORDER BY created DESC LIMIT 1) AS lastUse, u.activated FROM users u WHERE u.uid>0  AND community=-26742000000001 ORDER BY lastUse, u.activated');
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = a($uid);
    if ($a->ok) {
      $ray = [$a->fullName, f('u.fmtPhone', $a->phone), $a->email, @$lastUse ? f('u.fmtDate', $lastUse) : t('never'), '(' . f('u.fmtDate', @$activated) . ')'];
/** /    echo join("\t", $ray) . "\n";
    }
  }
}
*/

// print iCards for all active members in the greater Greenfield ctty
// be sure to chmod ...rcredits/temp to 777 before running this.
function special() {
  $cgfId = r\cgfId();
  $ctty = a('!MIW')->id;
  $accts = [];
  $addrs[] = 'name~addr1~addr2';
  $q = db\q("SELECT uid, fullName, (SELECT created FROM r_txs WHERE payer=u.uid AND payee>0 AND payee<>$cgfId ORDER BY created DESC LIMIT 1) AS lastUse FROM users u WHERE uid>0 AND community=$ctty ORDER BY fullName");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = a($uid);
    $salvage[] = "$fullName: $a->lastiCode";
    if ($a->ok and ($ctty == r\serverUid() ? $lastUse : TRUE) and !@$a->lastiCode) {
      $accts[] = $a;
      $addrs[] = "$fullName~" . join('~', explode(', ', $a->postalAddr, 2));
    }
  }
/**/  debug(compact('salvage','addrs'));
  file_put_contents(DRUPAL_ROOT . R_PATH . '/temp/merge.txt', join($addrs, "\n"));
  w\printInvite($accts);
}

/*
function retro() {
  $q=f('db.q','Select uid from users WHERE NOT :IS_CO AND uid>1 AND :IS_BONA');
  while ($row=$q->fetchAssoc()) {
    extract($row);
    $a = a($uid);
    $h = a($a->helper);
    if (!$h->cAdmin) { // helper is not admin
      $fullName = $a->fullName;
      f('be.fund',$a->helper, TX_HELPER, R_HELPER_BONUS, f('u.tt','invite reward', compact('fullName')));
///      debug("helper bonus to $h->fullName for inviting $fullName");
    }
  }
}*/