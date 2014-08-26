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

/**
 * Set up the basic user accounts and relations on the server.
 * called from installation and test setup
 */
function setupBasicAccounts() {
  $server = r\serverUid();
  if (db\exists('users', 'uid=:server', compact('server'))) return; // already done
  $keys = u\ray('uid community name fullName mail postalCode country minimum rebate flags');

  $values = array($server, $server, R_REGION_SHORTNAME, R_REGION_NAME, R_REGION_EMAIL, R_REGION_POSTAL_AREA, R_COUNTRY_ID, 0, 0, u\bit(array(B_OK, B_DW, B_CO)));
  $info = array_combine($keys, $values);
  $info += u\ray('legalName postalAddr', R_REGION_LEGALNAME, R_REGION_ADDR);
  new r\acct($info);

  $values = array(r\unQuid('R_ADMIN_QID'), $server, 'williamspademan', ADMIN_NAME, R_TESTER_EMAIL, '01330', US_COUNTRY_ID, 0, R_REBATE, u\bit(array(B_MEMBER, B_OK, B_DW)));
  $info = array_combine($keys, $values);
  list ($phone, $address, $city, $state) = array('+14136281021', 'PO Box 305', 'Ashfield', R_STATE_ID);
  $info += compact(u\ray('mail phone address city state'));
  $wws = new r\acct($info);
  
  $values = array(r\unQuid('NEW.AAB'), $server, 'cgf', 'Common Good Finance', CGF_EMAIL, '01330', US_COUNTRY_ID, 0, R_REBATE, u\bit(array(B_MEMBER, B_OK, B_DW, B_CO)));
  $info = array_combine($keys, $values);
  list ($phone, $address, $city, $state) = array(CGF_PHONE, CGF_ADDRESS, CGF_CITY, R_STATE_ID);
  $info += compact(u\ray('mail phone address city state'));
  $cgf = new r\acct($info);
  \variable_set('cgf_uid', $cgf->id);
  
  list ($usdAccount, $auth, $usdEmail, $usdPhone, $usdPass, $usdPin) = array(DW_TESTER_ACCT, DW_TESTER_AUTH, R_TESTER_EMAIL, $wws->phone, DW_TEST_PASS, DW_TEST_PIN);
  $secure = compact(u\ray('auth usdEmail usdPhone usdPass usdPin'));
  $wws->update(compact('usdAccount', 'secure'));
  
//  list ($main, $other, $permission) = array($cgf->id, $wws->id, r\perm(B_MANAGE));
//  db\insert('r_relations', compact(u\ray('main other permission'))); // wws can manage CGF
  $cgf->newRelation($wws->id, r\perm(B_MANAGE));
  
  $a1 = r\acct(1); // set up system admin record (uid=1)
  foreach (array(B_ADMIN, B_CO) as $bit) $a1->setBit($bit, TRUE, FALSE);
  $a1->update(compact(u\ray('fullName', t('System Administrator'))));
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
  $when = strtotime('-3 months');
  $count = $a->co ? db\recCount('r_txs', 'payee=:uid AND created>:when', compact('uid', 'when'))
    : db\recCount('r_txs', 'payer=:uid AND payee<>:cgf AND created>:when', compact(u\ray('uid cgf when')));
  $ret = t('@count @what', u\ray('@count @what', $count, $a->co ? t('sales') : t('purchases')));
  $subs = u\ray('@signed', u\fmtDate($a->signed));
  $ret .= (@$a->signed and $a->signed < $when) 
    ? t(' in the past 3 months (member since @signed)', $subs)
    : t(' since joining on @signed', $subs);
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
    if ($isOwner and !$iCo and !$uCo) {
      $role = t('related to');
    } elseif ($uid == $main) {
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
    $ret .= ($ret ? ', ' : '') . "$role <a href=\"$base_url/sadmin/member-info/$them\">$who</a>";
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

function showRecords($ray) {
  $cgfLen = strlen(r\cgfId());
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $key => $value) {
      if ($value and in_array($key, u\ray('id uid t.uid myid payer payee main other inviter invitee proxy person owner defaultAgent')) and strlen($value) >= $cgfLen) {
        if ($a = r\acct($value)) $value = $a->name; else $value .= t(' (missing)'); // eg, when uid is changed
      } // member ID. Don't include agent/payerAgent/etc. here
      if ($value and $key == 'channel') $value = u\rayv(u\ray(TX_CHANNELS), $value);
      if ($key == 'reid') $value = u\n2a($value, -3);
      if ($value and in_array($key, u\ray(R_DATE_FIELDS))) $value = '<div style="line-height:65%;">' . strftime('%d%b', $value) . '<span style="font-size:50%;">\'' . strftime('%y', $value) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $value) . '</span></div>';
//      if ($key == 'flags') $value = "<div style='text-transform:capitalize; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
      if ($key == 'flags') $value = "<div style='text-transform:none; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
      if ($key == 'for2' and $value == $row->for1) $value = 'same';
      $line .= "  <td>$value</td>\n";
    }
    $lines[] = $line;
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>\n", $lines) . "</tr>\n</table>";
}

/**
 * Display the given table or query.
 * @param string $table: table name
 * @param string $where: criteria
 * @param string $order: row order
 * @param string $fields: list of fields to display
 * @param array $joinFields: list of fields joining to users table (limit by community if called by Ctty Admin)
 */
function showQuery($table, $where = '1', $order = '', $fields = '*', $joinFields = array()) {
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
 */
function ssnCheck($a) {
  if (!$ssnData = @$a->ssnData) {
    $ssnData = u\ssnLookup($a->federalId);
    $xml = simplexml_load_string($ssnData);
    $json = json_encode($xml);
    $ssnData = json_decode($json, TRUE);
    if ($ssnData['ResultCount']) $a->update(compact('ssnData')); // ResultCount is 0 for tests and failures
  }
  $d = $ssnData['searchResults'];
  
  $name = $a->fullName . ($a->legalName == $a->fullName ? '' : "<br>$a->legalName");
  $rows[] = array(t('Data Given'), $name, u\fmtDate($a->dob), r\location($a, TRUE), '', '');
  $name = ssnName($one = (object) $d['ssnnames']['ssnname']);
  $rows[] = array(t('Found'), $name, u\fmtDate(strtotime($one->dob)), $d['deathsearchresults'] ? 'DEAD' : '', '', '');
  
  foreach($d['addresses']['address'] as $one) {
    $one = (object) $one;
    $rows[] = array(u\fmtDate(strtotime($one->datereported)), ssnName($one), u\fmtDate(strtotime($one->dob)), ssnAddr($one), ucwords(strtolower($one->county)));
  }
  
  $lines = '';
  foreach ($rows as $one) {
    list ($head, $name, $dob, $address, $county) = $one;
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
  return ucWords(strtolower(str_replace('-', '', @"$v->firstname $v->middlename $v->lastname")));
}
function ssnAddr($v) {return ucWords(strtolower(@"$v->address, $v->city, $v->state $v->zip"));}

/*
function fixdw($a) {
  if ($a->ok and $a->can(B_DW)) {$a->stepDone('dw'); $a->stepDone('connect');}
}*/

/**
 * Redo all the stats
include_once __DIR__ . '/../rcredits/admin/admin.inc';
f('a.fixStats');
 */
function fixStats($keepUpto = 0) {
  include_once __DIR__ . '/../rcron/rcron.inc';
  
  $sql = <<<EOF
    SELECT community AS ctty, MIN(t.created) AS start
    FROM users u LEFT JOIN r_txs t ON t.payee=u.uid 
    WHERE uid>1 AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
EOF;
  $cttys = db\q($sql)->fetchAllKeyed();
  u\preray(array(0 => min($cttys)), $cttys); // show all dates overall

  db\q($keepUpto ? "DELETE FROM r_stats WHERE id>$keepUpto" : 'TRUNCATE r_stats');
  $start = $keepUpto ? strtotime('tomorrow', db\lookup('MAX(created)', 'r_stats')) : (@$cttys[0] ?: REQUEST_TIME);
  $created = strtotime('today', $start); // use first moment of the day
  
  while ($created <= REQUEST_TIME) {
    $acctQ = db\q('SELECT uid FROM users WHERE uid>1');
    db\q('UPDATE users SET picture=0'); // clear temp field  
    while ($row = $acctQ->fetchAssoc()) r\Cron\acctStats1($row, $created); // set individual account stats
    foreach ($cttys as $ctty => $start) if ($created >= $start) {
      r\getStats($ctty, $created); // set community stats
      if ($ctty != 0) if (!checkStats($ctty, $created)) return; // verify
    }
    $created = strtotime('tomorrow', $created);
  }
}
  
/**
 * Check consistency with individual stats
 * @param int $ctty: record ID of community to check
 * @param int $created: stats date to check
 */
function checkStats($ctty, $asof) {
  $q = db\q('SELECT uid FROM users WHERE community=:ctty', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    extract(u\just('uid', $row));
    $info = u\just('r usd', (array) be\creditInfo(compact('uid','asof')));
//    unset($info['fancy']);
    if ($uid < 0) $info['r'] = -$info['usd']; // don't include r for communities (it would cancel out the others)
    if (@$totals) {
      foreach($info as $k => $v) $totals[$k] += $v;
    } else $totals = $info;
  }
  $stats = db\lookup('r,usd', 'r_stats', 'ctty=:ctty AND created=:asof', compact('ctty', 'asof'));
  foreach (u\ray('r usd') as $k) if ($stats[$k] != round($totals[$k], 2)) {
/**/ debug(u\ray('STATS_ERROR', compact(u\ray('k ctty asof stats totals')))); return FALSE;
//      r\tellStaff(t('bad stats'), compact(u\ray('k ctty asof stats totals')));
  }
  return TRUE;
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
    //$newbs = db\recCount('users', 'created<:created', compact('created')) - $pAccts - $bAccts;
///    debug(compact(u\ray('id created r usd newbs users'))); break;
//    db\update('r_stats', compact(u\ray('r usd newbs id')), 'id');

    $newbs = db\recCount('users', 'created<:created AND community=:new', compact('created', 'new')) - $pAccts - $bAccts;
    $id = db\lookup('id', 'r_stats', 'created=:created AND ctty<>0', compact('created'));
    db\update('r_stats', compact(u\ray('newbs id')), 'id');
  }
*/
function pioneerBonus($a) {
  if (!$a->ok or !$a->can(B_BONA) or $a->co) return;
  if (db\exists('r_txs', "payee=:uid AND payer=-8915 AND amount=100", u\ray('uid', $a->id))) return;
/**/debug("Pioneer bonus to $a->fullName");
  be\fund($a->id, TX_SIGNUP, 100, 'pioneer bonus');
}

/**
 * Re-setup the test cards on the STAGING server, after importing data from the PRODUCTION server.
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  f('a.makeTestCards');
 */
function makeTestCards() {
  if (isPRODUCTION) return;

  $a = a('aab');
  $selling = "groceries\ngifts\nsundries\ndeli\nbaked goods";
  $a->update(u\ray('fullName selling', 'Corner Store', $selling));
//  $a->setCoBit(CO_REQUIRE_CASHIER, FALSE);

  $a = a('aaa');
  $a->update(u\ray('fullName', 'Bob Bossman'));
  $a->upSecure('cardCode2', 'WeHlioM5JZv1O9G');
  
  $a = a('aad');
  $a->update(u\ray('fullName', 'Cathy Cashier'));
  $a->upSecure('cardCode2', 'ME04nW44DHzxVDg');
  
  $a = a('aaq');
  $a->update(u\ray('fullName', 'Helga\'s Hardware'));
//  $a->setCoBit(CO_REQUIRE_CASHIER, TRUE);
  
  $a = a('aak');
  $a->update(u\ray('fullName', 'Curt Customer'));
  $a->upSecure('cardCode', 'NyCBBlUF1qWNZ2k');
  $a->upSecure('cardCode2', 'utbYceW3KLLCcaw');
  
  $a = a('abb');
  $a->update(u\ray('fullName', 'Susan Shopper'));
  $a->upSecure('cardCode', 'ZzhWMCq0zcBowqw');
}

/* for changing archaic Dwolla info
$a = a('nya.aaa');
    $usdPass = f('u.code') . '*B2'; // satisfy Dwolla (and our own security)
    $usdEmail = strtolower(f('u.code',4) . '.' . $a->mainQid . '@rc4.me');
    //if (t\est()) $usdEmail = 'wspademan+' . strtolower(\user_password(4) . '.' . $this->mainQid) . '@gmail.com';
    $usdPin = f('u.randomString', 4, 'digits');
//    if (isDEV) list ($usdPass, $usdPin) = array('Password1', 9999);
    $usdPhone = $a->phone;
    $secure = compact(f('u.ray','usdPhone usdEmail usdPass usdPin')) + (@$a->secure ?: array());
///debug($secure);
    $a->update(compact('secure'));
    */
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
 * Create a smaller photo to send to POS devices.
 * @param acct $a: the account whose photo is to be shrunk
 * @param string $err: (RETURNED) the error message, if any
 * @return <success>
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  debug (rCredits\Admin\shrinkPhoto(a('aaa'), $err));
  debug ($err);
 */
function shrinkPhoto($a, &$err = '') {
  if ($a->co) return !$err = 'tried to shrink a company photo';
//  if ($a->hasAux('picture2')) return !$err = 'POS photo already exists';
  if (@$a->photo) $err = 'POS photo already exists'; // not really an error
  $photoPath = DRUPAL_ROOT . $a->auxFilename('picture', TRUE);
//  $smallPath = DRUPAL_ROOT . $a->auxFilename('picture2', TRUE);
//  if (!$success = copy($photoPath, $smallPath)) return !$err = 'copy failed';
  if (!$photo = u\fixPicture($photoPath, R_PERSONAL_ASPECT, R_PIXEL_FACTOR / 2, FALSE, $err)) return FALSE;
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
    $a->update(u\ray($to, $v));
  }
}

/**
 * Utility to do something to each existing transaction
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  eachTx();
 */
function eachTx($func = 'changeIt') {
  $result = db\q('SELECT * FROM r_txs');
  if (!function_exists($func)) $func = 'rCredits\\' . $func;
  while ($row = $result->fetchAssoc()) $func($row);
}

/**
 * Utility to do something to each member account (see example below)
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  f('a.eachAcct');
 */
function eachAcct($func = 'changeIt') {
  $result = db\q('SELECT uid FROM users WHERE uid>1')->fetchCol();
  if (!function_exists($func)) {
    if (function_exists($func2 = 'rCredits\\' . $func)) $func = $func2;
    if (function_exists($func2 = 'rCredits\\Web\\' . $func)) $func = $func2;
    if (function_exists($func2 = 'rCredits\\Admin\\' . $func)) $func = $func2;
  }
  foreach ($result as $one) $func(r\acct($one));
}

/**
 * General purpose utility, usually called from eachAcct, as above.
 * For example:
 *    changeIt($a):
 *    $us = new r\usd($a);
 *    $us->each('rCredits\\Cron\\bankFollowup', 0, u\ray('deposit withdrawal'));
 
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  foreach (array(3201932, 3244519, 3350610, 3510402) as $one) {
  f('a.changeIt', $one);
OR changeIt(a('aaa'));
}

 */
function changeIt($a) {
  if (!$a->ok) return;
  $err = '';
  shrinkPhoto($a, $err); if (!$err) $err = 'OK';
/**/  debug("$a->fullName: $err");
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