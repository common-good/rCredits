<?php
/**
 * @file
 * rCredits admin
   error_reporting(E_ALL);
  ini_set("display_errors", 1);

   include_once __DIR__ . '/../rcredits/admin/admin.inc';
 */
namespace CG\Admin;

use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;

/**
 * Set up the basic user accounts and relations on the server.
 * called from installation and test setup
 * To prepare for export for developers, f('t.clear');
 */
function setupBasicAccounts() {
  $keys = ray('uid community name fullName email zip country minimum rebate flags');
  $flags = u\bit(B_CO) | u\bit(B_OK);
  
  $q = db\q('SELECT :R_REGION_FIELDS FROM r_regions');
  while ($row = $q->fetchAssoc()) { // create or update all regions
    extract($row);
    if (!$hasServer) continue;
//    if (isDEV and !in_array($region, [R_SERVER_ID])) continue; // don't slow the tests down
    if ($uid = db\get('uid', 'users', "name=CONCAT(:region, '.')", compact('region'))) {
      r\acct($uid)->update(compact(ray('zip federalId flags')));
    } else {
      $fullName = $fullName ?: db\get('name', 'r_states', 'abbreviation=:st', compact('st'));
      $email = u\shortName($fullName) . '@' . CG_DOMAIN;
      $fullName = PROJECT . ' ' . $fullName;
      $uid = r\serverUid($region);
      $state = db\get('id', 'r_states', 'abbreviation=:st', compact('st'));
      $values = array($uid, $uid, $region . '.', $fullName, $email, $zip, R_COUNTRY_ID, 0, 0, $flags);
      $info = array_combine($keys, $values);
      $info += compact(ray('postalAddr state federalId'));
      new r\Acct($info);
    }
  }
  //$region = db\get("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  if (!isDEV) return; // don't mess with existing members on stage or production server
  
  $values = array(1, 0, 'admin', t('System Administrator'), r\regionField('email'), '', 0, 0, 0, u\bit([B_ADMIN, B_CO]));
  $info = array_combine($keys, $values);
  $a1 = new r\Acct($info); // set up system admin record (uid=1)
  if (isDEV) $a1->update('pw2 vKeyPw', r\passHash(DEV_PW2), u\b64decode(DEV_VKEYPW));

  foreach (ray(t('2:PlaceHolder One,3:PlaceHolder Two')) as $k => $v) {
    $name = str_replace(' ', '', strtolower($v));
    $values = array($k, 0, $name, $v, 'z@o.t', '', 0, 0, 0, 0);
    $info = array_combine($keys, $values);
    new r\Acct($info);
  }  

  $server = r\serverUid();
  if ($server == r\serverUid('NEW')) { // no need to set up CGF unless in Western Mass (or testing)
    r\acct($server)->update('created', R_LAUNCH); // a date in the past makes tests work better
    if (db\exists('users', 'uid=' . r\cgfId())) return; // already done
    if (NOT_PRODUCTION and db\exists('users', "name='cornerstore'")) return; // already done

    $values = array(r\qo('NEWAAB')->id, $server, u\shortName(PROJECT), PROJECT, CGF_EMAIL, '01330', US_COUNTRY_ID, 0, R_REBATE, u\bit([B_MEMBER, B_CONFIRMED, B_OK, B_CO, B_ICADMIN]));
    $info = array_combine($keys, $values);
    list ($phone, $address, $city, $state) = array(u\fmtPhone(CGF_PHONE, '+n'), CGF_ADDRESS, CGF_CITY, R_STATE_ID);
    $helper = 1; //$wws->id;
    $info += compact(ray('email phone address city state helper'));
    $cgf = new r\Acct($info);
    \variable_set('cgf_uid', $cgf->id);
    $cgf->update('emailCode website', '1495kJHm0h145PHh2345h', 'cg4.us');
//    $cgf->newRelation($wws->id, r\perm(B_MANAGE));
  }
}

/**
 * Re-setup the test cards on the STAGING server, after importing data from the PRODUCTION server.
 * Set bogus email addresses and phones so we DEFINITELY don't accidentally email someone (happened once)
 */
 define('TEST_ACCTS', 'aaa aab aad aaq aak abb');
 define('DEV_ACCTS', ''); // developer accounts
 define('R_COS', 'CAAAAA RMCAAA MIWAAA INAAAA NEWAAD NEWAIG NEWAKH NENAAB NEWALJ NEWAKD'); // community organizers, staff, and programmers to treat as cAdmins on test server (not the superAdmin's personal account, so the superAdmin can sign in as an ordinary individual user).
 
function makeTestAccounts() {
  if (isPRODUCTION) return; // protects us even if the constant is not set

  $a = a('aab');
  $selling = "groceries\ngifts\nsundries\ndeli\nbaked goods";
  $a->update('name fullName selling', 'cornerstore', 'Corner Store', $selling);
//  $a->setBitx(CO_REQUIRE_CASHIER, FALSE, 'coFlags');

  $a = a('aaa');
  $a->update('name fullName cardCode2', 'mariamanager', 'Maria Manager', 'WeHlioM5JZv1O9G');
  a\photoFromFile($a);
  
  $a = a('aad');
  $a->update('name fullName cardCode2', 'cathycashier', 'Cathy Cashier', 'ME04nW44DHzxVDg');
  a\photoFromFile($a);
  
  $a = a('aaq');
  $a->update('name fullName', 'helgas', 'Helga\'s Hardware');
//  $a->setBitx(CO_REQUIRE_CASHIER, TRUE, 'coFlags');
  a\photoFromFile($a);
  
  $a = a('aak');
  $a->update('name fullName cardCode cardCode2', 'curtcustomer', 'Curt Customer', 'NyCBBlUF1qWNZ2k', 'utbYceW3KLLCcaw');
  a\photoFromFile($a);
  
  $a = a('abb');
  $a->update('name fullName cardCode', 'susanshopper', 'Susan Shopper', 'ZzhWMCq0zcBowqw');
  a\photoFromFile($a);
  
  foreach (ray(TEST_ACCTS) as $k) a($k)->update('email phone pin', 'test@example.com', '+14130000000', '1234'); // prevent accidental texts and emails to real accts

  if (!db\exists('users', 'uid=-2')) new r\Acct(ray('uid community name fullName email flags', -2, -2, 'example', t('Common Good Example'), 'ctty@example.com', u\bit([B_CO, B_CGC, B_UP]))); // create a bogus community
  foreach (ray(TEST_ACCTS . ' ' . DEV_ACCTS) as $k) a($k)->update('community', -2); // isolate everyone into it
  
  a\makeTestAdmins();
}

/**
 * Let COs test, let techs manage testville
 */
function makeTestAdmins() {
  if (isSTAGE) foreach (ray(R_COS . ' ' . DEV_ACCTS) as $k) if ($k and $a = a($k)) $a->setBit(B_CADMIN);
}

/**
 * Return a display of steps done and steps remaining (to open an account).
 */
function showSteps($a) {
  $steps = '';
  if (!$a->ok and $a->id != 1) foreach (($a->stepsDone ?: []) as $k => $done) {
    $color = $done ? 'done' : 'not-done';
    if (!in($k, $a->co ? 'proxies' : 'company relations')) $steps .= "<b class=\"step-$color\">$k </b>";
  }
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
    : db\count('r_txs', 'payer=:uid AND payee<>:cgf AND created>:when', compact(ray('uid cgf when')));
  $ret = t('%count %what', 'count what', $count, $a->co ? t('sales') : t('purchases'));
  foreach (ray('created activated signed') as $k) $subs[$k] = $a->$k ? u\fmtDate($a->$k) : t('NO');
  $subs['use'] = u\fmtDate($a->ok ? $a->access : $a->login);
  $ret .= t(' in the past 3 months (created %created, signed %signed, activated %activated, last use/signin %use)', $subs);
  return $ret;
}

/**
 * Return a list of the account's relations.
 */
function showRelations($a) {
  global $base_url;
  $uid = $a->id;
  $jid = $a->jid ?: 0;
  $ret = '';
  $sql = <<<EOF
    SELECT IF(r.main=:uid, other, main) AS them, main, IF(r.main=:uid, u2.fullName, u1.fullName) as who, 
      permission, r.:IS_EMPLOYEE AS employee, r.:IS_OWNER AS owner, r.:IS_DRAW AS draw,
      r.:IS_CUSTOMER AS customer, r.:IS_AUTOPAY AS autopay, u1.:IS_CO AS iCo, u2.:IS_CO AS uCo
    FROM r_relations r LEFT JOIN users u1 ON u1.uid=r.main LEFT JOIN users u2 ON u2.uid=r.other
    WHERE :uid IN (r.main, r.other) AND :uid<>:jid AND (permission OR r.:IS_EMPLOYEE OR r.:IS_OWNER OR r.:IS_DRAW OR NOT r.:IS_CUSTOMER)
EOF;
  $q = db\q($sql, compact('uid', 'jid'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $permission += B_RELATED; // shift to use bit names
    $extra = '';
    if ($owner and !$iCo and !$uCo) { // family
      $role = t('related to');
      if ($draw) $role .= $uid == $main ? t(' and drawn on by') : t(' and draws on');
    } elseif ($uid == $main) {
      $qo = r\qo($main, $them);
      $extra = '<small> (' . r\qid($them) . ' / ' . a\cardLink("$main/$them") . ')</small>';
      $role = $owner ? t('owned by')
        : ($draw ? t('drawn on by')
        : ($permission >= B_MANAGE ? t('managed by')
        : (($permission >= B_SCAN or $employee) ? t('employs')
        : ($permission >= B_READ ? t('readable by')
        : t('probably employs')))));
    } else {
      $role = $owner ? t('owns')
        : ($draw ? t('draws from')
        : ($permission >= B_MANAGE ? t('manages')
        : (($permission >= B_SCAN or $employee) ? t('works for')
        : ($permission >= B_READ ? t('can read')
        : ($customer ? t('buys from')
        : t('probably works for'))))));
    }
    $ret .= ($ret ? ', ' : '') . "$role <a href=\"$base_url/change-account/acct=$them&page=summary\">$who</a>$extra";
  }
  return $ret;
}

define('ID_KEYS', 'id uid t.uid myid payer payee main other inviter invitee proxy person owner defaultAgent');

/**
 * Return the given field, formatted for viewing.
 */
function formatField($k, $v, $flagsField = B_LIST) {
  $v = $v . ''; // convert to string
//  if ($k == 'bankAccount' and u\starts($v, '~')) $v = 'c' . $v; // FUDGE to repair broken crypt flag (?)
//  while (u\starts($v, CRYPT_FLAG)) $v = u\decrypt($v);
  if ($v and in($k, ID_KEYS) and strlen($v) >= strlen(r\cgfId())) {
    if ($a = r\acct($v)) $v = $a->name; else $v .= t(' (missing)'); // eg, when uid is changed
  } // member ID. Don't include agent/payerAgent/etc. here
  if ($v and $k == 'channel') $v = u\rayv(ray(TX_CHANNELS), $v);
///  if ($k == 'reid') $v = str_replace(R_MEMBER_MARK, R_AGENT_MARK, r\qid($v));
  if ($v and in($k, R_DATE_FIELDS)) $v = '<div class="date-field">' . strftime('%d%b', $v) . '<span class="year-field">\'' . strftime('%y', $v) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $v) . '</span></div>';
  if ($k == 'flags') $v = "<div class=\"format-flags\">" . a\flagsDesc($v, $flagsField) . '</div>';
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
  global $mya;
  
  $flagsField = $table == 'r_txs' ? TX_FLAGS : B_LIST;
  if (!$mya->admin and $joinFields) {
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
  return w\showRecords(db\q("SELECT $fields FROM $table WHERE $where LIMIT $limit")->fetchAll(), 'CG\\Admin\\formatField', $flagsField);
}

/**
 * Show results of an SSN search, compared to personal data supplied by the member.
 * Updates the moves field, if it isn't set yet.
 */
function ssnShow($a) {
  if (isDEV) return t('no SSN data');
  
  $ssnData = @$a->ssnCheck();
  $d = @$ssnData['searchResults'] ?: []; // [] for admin without vKeyE
  if ($err = @$ssnData[SSN_ERR_INDEX]) $rows[] = ["<b class=\"err\">ERROR</b>", $err, '', '', ''];
  $name = $a->fullName . ($a->legalName == $a->fullName ? '' : "<br>$a->legalName");
  $rows[] = array(t('Data Given'), $name, u\fmtDate(is_numeric($a->dob) ? $a->dob: 0), r\location($a, TRUE), '', ''); // is_numeric check for admin without vKeyE
  $name = u\ssnName($one = (object) @$d['ssnnames']['ssnname']);
  $dob = $one->dob ? u\fmtDate(strtotime($one->dob)) : ''; // arrives as an empty array if none
  $rows[] = array(t('Found'), $name, $dob, @$d['deathsearchresults'] ? '<h1 color="red">DEAD</h1>' : '', '', '');
  
  $counties = [];
  $addresses = @$d['addresses']['address'] ?: [];
  if (@$addresses['lastname']) $addresses = [$addresses];
  foreach($addresses as $rec) {
    $rec = (object) $rec;
/**/ $rows[] = array($rec->datereported ? u\fmtDate(strtotime($rec->datereported)) : '', u\ssnName($rec), $rec->dob ? u\fmtDate(strtotime($rec->dob)) : '', u\ssnAddr($rec), ucwords(@strtolower(print_r($rec->county, 1)))); // county was an array once
/**/ if (r\rTime() - @strtotime($rec->datereported) < 10 * YEAR_SECS) $counties[print_r($rec->county, 1)] = 1;
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

/**
 * Redo all the stats
 * @param int $keepUpto: date of last valid stats records (redo all subsequent dates)
 */
function fixStats() {
/*
  db\q('TRUNCATE r_stats');
  f('cr.cttyStats');
*/
}

function pioneerBonus($a) {
  if (!$a->ok or $a->co) return;
  if (db\exists('r_txs', "payee=:uid AND payer=-8915 AND amount=100", ray('uid', $a->id))) return;
/**/  debug("Pioneer bonus to $a->fullName");
//  be\fund($a->id, TX_SIGNUP, 100, 'pioneer bonus');
}


// MEMBER LIST: SELECT fullName, city FROM users WHERE (zip LIKE '010%' OR zip LIKE '013%') AND NOT (flags&(1<<6)) AND NOT (flags&(1<<9)) ORDER BY city, fullName

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
  $lens = ray('9 1 9 1 6 9 36 36 20 2 9 10 1 1'); // also 5 chars of filler at end
  $fields = array_combine(ray($fieldNames), $lens);
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
    foreach (ray('name address city') as $field) $$field = ucwords(strtolower(trim($$field)));
    db\insert('r_banks', compact(ray($fieldNames)));
  }
}

/**
 * Create a smaller photo to send to POS devices
 * CALLED ONLY FROM a\makeTestAccounts()!
 * @param acct $a: the account whose photo is to be shrunk
 * @param string $filename: the photo filename (defaults to account's shortname)
 * @param string $err: (RETURNED) the error message, if any
 * @return <success>
  include_once __DIR__ . '/../rcredits/admin/admin.inc';
  debug (f('a.photoFromFile', a('aaa'), $err));
  debug ($err);
  currently called only from a.makeTestAccounts()
 */
function photoFromFile($a, $filename = '', &$err = '') {
  if ($a->co) return !$err = 'tried to shrink a company photo';
//  if ($a->hasPhoto('picture2')) return !$err = 'POS photo already exists';
  if (@$a->photo) $err = 'POS photo already exists'; // not really an error
  $photoPath = DRUPAL_ROOT . R_PICTURE_DIR . ($filename ?: "$a->name.jpg");
//  $a->photoFilename(TRUE));
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
  $uids = db\q('SELECT uid FROM users WHERE uid<0 OR uid>:CANONIC_ACCTS')->fetchCol();
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
  $q = db\q("SELECT uid,$field FROM users WHERE uid<0 OR uid>:CANONIC_ACCTS");
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

/**
 * Return a readable list of names of bits set in the given integer bit array.
 * @param int $n: the bit array
 * @param string $list: delimited list of bit names
 * @return list of bit names set in $n
 */
function flagsDesc($n, $list) {
  $bits = ray($list);
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
  global $base_url;
  $sql = <<<EOF
    SELECT SUM(IF(amount>0 AND txid>0, amount, 0)) AS amountIn, SUM(IF(amount>0 AND txid>0, 1, 0)) AS countIn,
      SUM(IF(amount<0 AND txid>0, amount, 0)) AS amountOut, SUM(IF(amount<0 AND txid>0, 1, 0)) AS countOut,
      SUM(IF(txid<0, amount, 0)) AS amountBad, SUM(IF(txid<0, 1, 0)) AS countBad,
      deposit
    FROM r_usd
    GROUP BY deposit ORDER BY deposit<>0, deposit DESC, amount<0
EOF;

  $today = strtotime('today');
  $ways = ray('In Out Bad', t('IN'), t('OUT'), t('BAD')); // translate, don't use strtoupper
  $q = db\q($sql); // include today's potential deposit at the top

  while ($row = $q->fetchAssoc()) {
    extract($row);
    $firstline = TRUE;
    foreach ($ways as $way => $wayDesc) {
      list ($total, $count) = [u\fmtAmt(${"amount$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      if ($firstline) {
        $dateDpy = $deposit == 1 ? t('<b>ANCIENT</b>') : ($deposit ? u\fmtDate($deposit) : t('<b>TODAY</b>'));
        $date = strftime('%Y%m%d', $deposit ?: $today);
        $details = w\btn("/sadmin/deposit-details/date=$deposit&total=$total&count=$count", t('details'));
        if (!$deposit or $deposit >= $today - 90 * DAY_SECS) $details .=
          ' ' . w\btn('', t('flnm'), 'default', 'xs', ray('class data-flnm', 'filename', "CGach-$date.csv"))
        . ' ' . w\lnk("/sadmin/achs/date=$deposit&mark=0", t('download'));
        if ($count) $details .= ' ' . w\btn("/sadmin/checks/way=In&date=$deposit&mark=0", t('checks')); 
      } else $dateDpy = $details = '';

      $deposits[] =<<<EOF
<tr>
  <td>$dateDpy</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details</td>
</tr>
EOF;
      $firstline = FALSE;
    }
  }
  $deposits = join("\n", @$deposits ?: []);
  $header = t('<tr><th>Transfers</th><th></th><th>Count</th><th>Total</th><th></th></tr>');
  return <<<EOF
<table id="deposits">
$header
$deposits
</table>
EOF;

}

/*
    foreach (['In'=>t('IN'), 'Out'=>t('OUT'), 'Bad'=>t('BAD')] as $way => $wayDesc) {
      list ($total, $count) = [u\fmtAmt(${"amount$way"}, ''), number_format(${"count$way"})];
      if (!$count) continue;
      $details = w\btn("/sadmin/deposit-details/way=$way&date=$deposit&total=$total&count=$count", t('details'), w\away());
      if ($way == 'Out') {
        list ($WAY, $date) = [t('OUT'), strftime('%Y%m%d', $deposit ?: strtotime('today'))];
        $details = w\btn('', t('flnm'), 'default', 'xs', ray('class data-flnm', 'filename', "CGach$WAY-$date.csv")) . ' ' . $details;
      }

      list ($func, $label) = ($way != 'Out' or ($deposit > 0 and $deposit < strtotime('1/10/2017')))
      ? ['checks', t('checks')] 
      : ['achs', t('ACHes')];
      $deposits[] =<<<EOF
<tr>
  <td>$dateDpy</td>
  <td>$wayDesc</td>
  <td>$count</td>
  <td>$total</td>
  <td>$details &nbsp; <a href="$base_url/sadmin/$func/way=$way&date=$deposit&mark=0" target="_blank">$label</a></td>
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
*/

/**
 * Print a check
 * @param Pdf $ck: the Check object to place the check in.
 * @param int $pos: what position on the current page (1, 2, or 3)
 * @param assoc $tx: relevant fields from the r_usd record
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printCheck(&$ck, $pos, $tx, $mark) {
  extract(just('txid created payee amount deposit bankAccount', $tx));
  $escrow = 'USkk' . ESCROW_ROUTING . ($amount > 0 ? ESCROW_IN : ESCROW_OUT);
	$bankAccount = u\decry('V', $bankAccount);
  $a = r\acct($payee);
//  $a->fixTxBankAccount($bankAccount, $txid);
  
  list ($fromName, $addr, $phone, $fromAcct, $toName, $toAcct) = $amount > 0 // which way
//    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, PROJECT, $escrow] // from bank to escrow
    ? [$a->legalNameDpy, $a->postalAddr, $a->phone, $bankAccount, BANK_DBA, $escrow] // from bank to escrow
    : [PROJECT . ' (' . CGF_LEGALNAME . ')', CGF_POSTALADDR, CGF_PHONE, $escrow, $a->legalNameDpy, $bankAccount]; //e2b
  $memo = $amount > 0 ? t('for %RCREDITS') : t('for DEPOSIT to account #') . substr($toAcct, 4 + 9);

  list ($signature, $belowLine) = $amount > 0 
  ? [t('Pre-authorized Check'), t('No Signature Required')] 
  : ['', SIGNATORY];
  
  list ($W, $H) = array($ck->pageW, $ck->pageH);
  $lineH = $ck->lineH; // standard line height in inches
  $m = .25; // left/right margins
  $mt = ($pos - 1) * CHECK_HEIGHT + $m; // top margin

  $phone = u\fmtPhone($phone);
  $routing = substr($fromAcct, 4, 9);
  $account = substr($fromAcct, 4 + 9);
  
  $bankInfo = db\get('*', 'r_banks', 'route=:routing', compact('routing'));
  if (!$bankInfo) {w\say("Skipped transaction #$txid for $a->fullName (no bank info)"); return;}
  //u\EXPECT((bool) $bankInfo, 'no bank info');
  extract($bankInfo, EXTR_PREFIX_ALL, 'b');
  if (strlen($b_zip) > 5) $b_zip = substr($b_zip, 0, 5) . '-' . substr($b_zip, 5);
  if (!$prefix = db\get('id', 'r_transit', 'location=:location', ray('location', "$b_city, $b_state"))) {
    $prefix = db\get('id', 'r_transit', 'location=:b_state', compact('b_state'));
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
  
  if (!$deposit and $mark) db\update('r_usd', ray('deposit txid', strtotime('today'), $txid), 'txid');
}

/**
 * Do automatic stuff that can be done only when superAdmin signs in
 */
function adminSignin() {
  global $box;
//  $pw2 = @$_COOKIE['pw2'];
//  u\EXPECT($pw2, 'no pw2!');
  
/*  $q = db\q("SELECT txid,payee,bankAccount FROM r_usd WHERE bankAccount NOT LIKE ':CRYPT_FLAG%'");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if (substr($bankAccount, 0, 2) == 'US') {
      $bankAccount = r\crypt($bankAccount . '', 0, $pw2);
      db\update('r_usd', compact('txid', 'bankAccount'), 'txid');
      w\say(t('bank account super-encrypted in exchange record #%txid.', compact('txid')));
    } else r\tellAdmin('bad bank account in exchange record', ray('txid b0 hex', $txid, $b0, bin2hex($b0)));

    $a = r\acct($payee);
    if ($b0 = $bankAccount = $a->bankAccount and !u\starts($a->secure['bankAccount'], CRYPT_FLAG)) {
      if (substr($bankAccount, 0, 2) == 'US') {
//        $bankAccount = u\crypt($bankAccount . '', 0, $pw2);
        $a->update(compact('bankAccount')); // might be different (newer) than in r_usd (super-encryption is automatic)
        w\say(t('bank account super-encrypted for user %user', 'user', $a->fullName));
      } else r\tellAdmin('bad bank account', ray('b0 hex', $b0, bin2hex($b0)));
    }
  }
	*/
}

/*
function moveFieldsToHidden() {
  $q = db\q("SELECT uid FROM users WHERE uid<>0");
  $dataFields = [];
  $secureFields = ray('photoId pin bankAccount');
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    if ($a->pin or $a->bankAccount) continue; // don't do this twice (and overwrite with nulls)!
    $data = $a->data;
    $secure = $a->secure;
    foreach ($dataFields as $k) $$k = @$data[$k];
    foreach ($secureFields as $k) $$k = @$secure[$k];
    $data = $secure = [];
    foreach (ray(R_DATA_FIELDS) as $k) if (!is_null($a->$k)) $data[$k] = $a->$k;
    foreach (ray(R_SECURE_FIELDS) as $k) if (!is_null($a->$k)) $secure[$k] = $a->$k;
///    debug($row + compact($dataFields, $secureFields, 'data', 'secure'));
    $a->update(compact($dataFields, $secureFields, 'data', 'secure'));
  }
}
*/

/**
 * Make sure the very secure fields are encrypted with the admin's extra password.
 *//*
function secureVery($a = '') {
  if (!r\acct()->superAdmin) return; // don't bother if not admin
  if ($a) {
    if ($vflds = just(R_VSECURE_FIELDS, $a->secure)) return $a->update($vflds); // extra encryption
  } else {
    $q = db\q('SELECT uid FROM users WHERE :IS_OK');
    while ($row = $q->fetchAssoc()) secureVery(r\acct($row['uid']));
  }
}*/

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
return;
  eachX(function($x) {
    $cgf = 26742000000002;
    foreach (ray('type payer payee goods flags taking amount payeeFor') as $k) $$k = $x->$k;
    if ($payeeFor == 'contribution (percentage of past month receipts)' and ($payee < 0 or $payee == $cgf)) {
      foreach ([B_GIFT, B_CRUMBS] as $k) u\setBit($flags, $k);
      $payee = $cgf;
    } elseif ($payeeFor == 'contribution of rounded-up payment change' and ($payee < 0 or $payee == $cgf)) {
      foreach ([B_GIFT, B_ROUNDUPS] as $k) u\setBit($flags, $k);
      $payee = $cgf;
    } elseif ($payee == $cgf and $payer > 0) {
      foreach (ray('ontribution onation everses') as $k) if (strpos($payeeFor, $k)) {
        if (strpos($payeeFor, 'everses')) u\setBit($flags, B_UNDOES);
        if (strpos($payeeFor, 'eversed')) u\setBit($flags, B_UNDONE);
        u\setBit($flags, B_GIFT);
        if (strpos($payeeFor, 'onation (')) u\setBit($flags, B_PATRONAGE);
      }
    }
    if ($flags & u\bit(B_GIFT)) $x->update(compact(ray('flags payee')));
  });
}

function special2() {
return;
  eachA(function($a) {
    list ($balance, $rewards) = [$a->balance, $a->rewards]; // remember old values
/**/    if (!$a->cacheOk(FALSE, FALSE) and (abs($a->balance - $balance) > .005 or abs($a->rewards - $rewards > 20))) debug("$a->id $a->fullName new: $a->balance+$a->rewards old: $balance+$rewards jid=$a->jid");
  });
}

// go through and revise rebates to negative something when appropriate and set "short" bit
function special3() {
  return; // too complicated; don't do it.
  global $asql;
  $asql = <<<EOF
SELECT xid,type,goods,amount,payer,payee,payerReward,payeeReward,flags,created FROM r_txs WHERE payer=:uid OR payee=:uid
UNION (SELECT txid AS xid,:TX_BANK AS type,0 AS goods,amount,payer,0 AS payee,0 AS payerReward,0 AS payeeReward,0 AS flags,created FROM r_usd WHERE payer=:uid)
ORDER BY created,xid
EOF;
  eachA(function($a) {
    $TEST = TRUE;
    global $asql;
    $balance = $rewards = 0;
    $q = db\q($asql, ['uid' => $a->id]);
    while ($row = $q->fetchAssoc()) {
      extract($row);
      if ($type == 0 AND $a->id == $payer AND $goods != FOR_SHARE AND !(flags & B_UNDONE)) {
        $payerReward = r\reward($payer, $payee, $amount);
        if ($payerReward != $row['payerReward']) {
/**/          debug("$a->fullName xid=$xid newreward=$payerReward old=" . $row['payerReward']); 
          if (!$TEST) x($xid)->update(compact('payerReward'));
        }
      }
      list ($dbal, $drew) = $a->id == $payer ? [-$amount, $payerReward] : [$amount, $payeeReward];
///      debug(compact(ray('row balance dbal rewards drew')));
      $balance += $dbal;
      $rewards += $drew;
    }
    list ($newbal, $newrew) = [round($balance, 2), round($rewards, 2)];
    list ($balance, $rewards) = [$a->balance, $a->rewards]; // remember old values
/**/    if (!$a->cacheOk(FALSE, FALSE)) flog("$a->id $a->fullName new: $a->balance+$a->rewards old: $balance+$rewards jid=$a->jid");
/**/    if ($a->balance != $newbal or $a->rewards != $newrew) debug("MISCALC $a->id $a->fullName cache: $a->balance+$a->rewards runningCalc: $newbal+$newrew ");
  });
}

function showRequests() {
$cutoff = strtotime('-1 year');
$q = f('db.q','SELECT * FROM r_request WHERE created>:cutoff ORDER BY listid DESC', compact('cutoff'));
  $list[] = "id\tfirst name\tlast name\tphone\temail\tcreated";
  while ($row = $q->fetchAssoc()) {
    extract($row); // listid created first last phone email
    foreach (['phone', 'email'] as $k) $$k = f('u.decryptN',$$k);
    $list[] = "req$listid\t$first\t$last\t$phone\t$email\t$created";
  }
  w\say(plain(join("\n", $list)));
}

function showInactive() {
  $ctty = a('aaa')->community;
  $q = f('db.q', 'select uid from users where community=:ctty order by access', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    $a = a($row['uid']);
    if ($a->slave) continue;
    $dt = f('u.fmtDate', $a->access);
    $ok = $a->ok ? '(member)' : '';
/**/ echo "$a->phone\t$a->fullName\t$dt\t$ok\n";
  }
}

function cardLink($link) {return spinLink("/print-rcard/$link", t('Card'), '', 'primary', 'xs', w\away());}
function recropLink() {return spinLink('/settings/photo/recrop=1', t('Recrop Photo'), '', 'primary', 'xs');}

function nextMember($id = 0) {
  $members = db\col('uid', 'users', ':IS_OK AND NOT :IS_CO AND activated<1486098000 ORDER BY community, uid');
  $i = $id ? array_search($id, $members) : -1;
  return @$members[$i + 1];
}

/**
 * Return a detailed script for calls to account-holders.
 * @param string $type: annual, welcome, setup, pre, 
 * @return a short description and details if suggested date is within a month +/- of standard welcome or annual call
 */
function callDetails($type = '', $when = '') {
  global $mya;
  
  $now = time();
  $keys = ray('nocall pre setup activate welcome annual');
  $heads = ray(t('No call,Just beginning,Setup,Activate,Welcome,Annual'));
  $heads = array_combine($keys, $heads);
  $head = $heads[$type ?: 'nocall'];
  $setup = in($type, 'pre setup');
  $late = round(($now - $when) / DAY_SECS);
  $subtext = $now < $when ? t('in %days days', 'days', -$late) : t('%late days overdue', compact('late'));
  $subtext = !$when ? '' : "<br><span>$subtext</span>";
  
  if (!$mya->member) {
    $head .= $setup ? '<p>Do you need help?</p>' : '';
//    return "<div class=\"well\"><h3>$head$body</h3></div>";
  }

  $gifts = $mya->giftsDesc() ?: db\sum('amount', 'r_gifts', 'uid=:id', ['id' => $mya->id]); // a promised gift counts here
  
/*        $help = $gifts ? 'thanks' : 'ask';
        $msg = t('<span class="loud">%when <b>Welcome</b></span> to %PROJECT, How\'s it going%lastTx? %help donation(s), %signupCo volunteer? [<%a>details</a>]', 'when lastTx help signupCo _a', u\fmtDate($when, TRUE), $a->lastTxDesc(), ucfirst($help), @$signupCo, w\atag("/help/welcome/gifts=$gifts"));
         if ($a->paper) $msg .= t(' <span class="loud">Paper statements</span> at $%R_STATEMENT_COST/month &mdash; really?');
    $gift = $gifts ? t('Thanks for donation(s). Annual gift?') : t('Donate?');
    $feature = $a->co ? t('cash-out') : t('joint account');

    return [t('Annual'), $when = strtotime("+$yearsIn years", $activated), t('<span class="loud">%when <b>Annual</b></span>: Thanks for participating. How\'s it going%lastTx? %gift Mailing address okay? Volunteer? Consider %feature? [<%a>details</a>]', 'when lastTx gift _a feature', u\fmtDate($when, TRUE), $a->lastTxDesc(), $gift, w\atag("/help/annual/gifts=$gifts&years=$yearsIn"), $feature)];
         */
      
  $welcome = t('<li><b>Welcome</b> to %PROJECT. Thanks for participating!</li>');
  if (!$mya->co and $myCo = $mya->firstCo()) {
    list ($signupCo, $myCo) = ['', t(' (personally rather than for company "%myCo")', compact('myCo'))];
  } else $signupCo = $mya->signupCo ? t('<li><b>Company</b>. How about signing up your company: %coName? (%co)</li>', 'co coName', w\signupCoDisplay($mya->signupCo, $coName), $coName) : '';
  $notyet = t("<li><b>Want</b>. What's a place you would like to use your %PROJECT card but can't yet?</li>");
  $how = t("<li><b>How's it going?</b> %myCo %lastTx Do you have any questions, suggestions, or concerns? Are you getting our newsletter? What do you find most valuable about the %PROJECT system? If you could change or improve one thing about the %PROJECT system, what would it be?</li>", '_lastTx myCo', $mya->lastTxDesc(), @$myCo) . $notyet;
  if ($mya->paper) $how .= t('<li><b>Paper statements</b>. Do you really want paper statements? ($%R_STATEMENT_COST/month)</li>');
  if ($mya->balance < 0) $how .= t('<li><b>Negative</b>. You have a negative balance of <%b>%bal</b>. Do you need help getting money into your account? That amount is, in effect, borrowed from your local community of %PROJECT members. So it is much better of course, for the community\'s financial health, if you bring your balance back up above zero when you can.</li>', '_b bal', 'b class="outgoing"', u\fmtAmt($mya->balance));
  
  $reason = $setup ? t('see if I might help you finish opening your %PROJECT account')
  : ($type == 'welcome' ? t('welcome you to %PROJECT, check in, make sure you got your card, and ask you some questions')
  : ($type == 'annual' ? t('thank you for being a member, make sure we have your address right, and ask some questions')
  : t('ERROR')));
  $intro = t('Hi, this is %name with %PROJECT. I\'m calling to %reason.', 'name reason', $mya->agentA->fullName, $reason);
  $live = $intro . t(' Is this a good time?');
  $callback = t('Please give me a call back when you can at %phone', ['phone' => u\fmtPhone($mya->agentA->phone, '-')]);
  $msg1 = "$intro $callback" . t(' &mdash; or I\'ll try again in a day or two.');
  $msg2 = "$intro $callback" . t(' &mdash; I\'ll also send you an email.');
  $email = t('<p>Dear %name,</p><p>Following up on my phone message, I\'m writing to %reason. %callback &mdash; or reply to this email.</p>', 'name email reason callback', $mya->fullName, $mya->email, $reason, $callback);
  if ($setup) {
    if ($info = db\get('reid,code', 'r_relations', 'other=:myid AND :IS_CUSTOMER', ['myid' => $mya->id])) {
      $_how = t('using <%a>this link</a>', '_a', w\atag(tr('%BASE_URL/signup/reid=%reid&customer=%code', $info)));
    } else $_how = t('by signing in at %CG_DOMAIN with account ID "%acct" (then click "Sign-in problems" if you don\'t remember your password)', 'acct', $mya->mainQid);
    $email .= t('<p>Or you can complete the account on your own, %how. But please feel free to ask for help if you run into any problems at all.</p>', compact('_how'));
  } else $email .= t('<p>Here are the questions and information we have for you:</p>');
  
  if ($mya->co) { // company
    $volunteer = '';
    $coAsk = ($mya->crumbs or TRUE) ? '' : t('<li><b>Crumbs</b>. We particularly encourage businesses to contribute what we call "crumbs" to support the system and the common good. Do you know about this option? It means contributing a small percentage of each payment received, instead of the system charging you credit card fees. What percentage would you like to contribute? [one percent is a common choice, a fraction of a percent is also wonderful]</li>'); // "or TRUE" is temporary 201801
    $coAsk .= t('<li><b>Coupons</b>. In addition to providing <i>free</i> advertising, %PROJECT can bring new customers to you with coupons. We give each new member a coupon to a %PROJECT business. Would you like to be in that rotation? (yes: we\'ll call you back about that)</li>');
    $coAsk .= t('<li><b>CC Fees</b>. Since credit card companies typically charge two or three percent plus a per-transaction fee, our board has proposed charging business members a small percentage on each payment received, to make the system sustainable sooner. Do you think this is a good idea and if so what percentage would be reasonable? What rate do you currently pay for credit card transactions? Would you want to <i>donate</i> a small percentage instead? If so, how much? [one percent is a common choice, a fraction of a percent is also wonderful]</li>'); // temporary 201801
    $options = db\exists('r_usd', 'payee=:myid AND amount<0 AND created>:now-:DAY_SECS*30', ray('myid now', $mya->id, $now)) ? t('Do you know about our automatic cash-out option? If you want your company account to transfer your surplus funds to your bank account automatically every week, we can set that up. ') : '';
    $options .= t('If you have questions about our "Pay With %PROJECT" button for your website, our invoicing system, or accounting statements, we can help you with any of those.');
  } else { // individual
    $_volunteer = join("</li>\n<li>", explode('~', t(
      'Hand out promo cards and schedule appointments with local businesses.'
    . '~Help new members open their %PROJECT account.'
    . '~Call new members to welcome them and see if they have questions.'
    . '~Assemble promotional materials (at home or with other volunteers).'
    . '~Deliver things.'
    . '~Print and mail things.'
    . '~Take photographs.'
    . '~Help set up for meetings.'
    . '~Help add new features to the software.'
    . '~Serve on the board of directors.'
    )));
    $_volunteerLink = t('<%a>volunteer</a>', '_a', 'a href="#volunteer" data-toggle="collapse"');
    $volunteer = t('<li><b>Volunteer</b>. (if supportive) Would you like to %volunteerLink with %PROJECT?<ul id="volunteer" class="collapse"><li>%volunteer</li></ul></li>', compact(ray('_volunteerLink _volunteer')));
    $coAsk = '';
    $options = t('Do you know about our joint account option? Sign in at %CG_DOMAIN and visit the Relations page on the Settings menu, to join an existing account with yours. Or to open a new account joined to yours, click "Make this a Joint Account" on your Summary page.');
    if (!$mya->roundup) $options .= t(' And do you know about the roundups option? Roundups let you always pay a whole dollar amount and donate the change to the community fund. Choose it on the Preferences page on the Settings menu. Or just ask and we\'ll set it for you.');
  }
  $giftDetails = t('(Wait for answer, then ask:) What amount would you like to give and do you prefer yearly, quarterly, or monthly? Suggested amounts are $50 for individuals and $250 for companies, but any amount is wonderful.');
  $ask = t('<li><b>Offer</b>. The idea of %PROJECT is to empower us to take responsibility for our local economy, so most of our members contribute some small amount periodically and/or choose roundups or crumbs, to support the system. Would you like to contribute at this time? %details</li>', 'details', $giftDetails) . $coAsk;
  $thx = t('<li><b>Donations</b>. Thank you very much %gifts.</li>', '_gifts', a\giftsToDate());
  $reask = @$often ? $thx : $thx . t('<li><b>Gift</b>. Would you like to %contribute at this time? %details</li>', 'contribute details', $type == 'annual' ? t('make an annual contribution') : t('contribute'), $giftDetails);
  if ($options) $options = t('<li><b>Features</b>. %options</li>', compact('options'));
  $options .= t('<li><b>Tell people!</b> Do you need more handout cards or brochures, to tell %who about %PROJECT?', 'who', $mya->co ? t('your customers and suppliers') : t('your friends or local businesses'));
  $addressCheck = t('<li><b>Address</b>. Are you still at %addr? (if not, ask about postal address too)</li>', 'addr', r\location($mya, TRUE));

  if ($mya->co) $ask = $reask = $coAsk; // temporary 201801
  
  $yearsIn = max(1, round(($now - $mya->activated) / DAY_SECS / 365.25)); // how many years after activation are we
  $annualAsk = '<ol><li>' . t("<b>Thanks</b>. Thank you for being a pioneer of economic democracy! (member for %yearsIn year/s) Thanks to your participation, along with other members, we were able to issue $10,000 in grants and loans this year. (Were you in on the discussions of what to fund?)", compact('yearsIn')) . "</li>\n$how\n$ask\n$addressCheck\n$signupCo\n$volunteer\n$options\n</ol>";
  
  $res = '';
  if ($type == 'annual') $res = @$gifts ? str_replace($ask, $reask, $annualAsk) : $annualAsk;
  if ($type == 'welcome') $res = @$gifts ? "<ol>$welcome\n$thx\n$how\n$signupCo\n$volunteer\n</ol>" : "<ol>$welcome\n$how\n$ask\n$volunteer\n</ol>";
  $email .= $res . t('<p>I look forward to hearing from you!</p><p>Sincerely,</p><p>%name<br>for the %PROJECT Team</p>', 'name', $mya->agentA->shortName);
  $live .= $res . t('<p>Thank you very much for your time and for being a %PROJECT member. Have a great day.</p>');
  if (@$_volunteerLink) $email = str_replace($_volunteerLink, t('volunteer'), $email);
  $email = str_replace('id="volunteer"', '', $email);  // make sure the div isn't duplicated (causing expansion to fail)
  $email = preg_replace('/\([^)]+\)/', '', $email);
  
  $links = $details = '';
  foreach (ray('msg1 msg2 email live') as $k) {
    $details .= tr('<div id="%k-details" class="collapse">%detail</div>', 'k _detail', $k, $$k);
    $links .= tr(' <a id="%k-link" href="#%k-details" data-toggle="collapse" class="btn btn-xs btn-warning">%k</a>', 'k', $k);
  }

  $email = str_replace('href="#email-details" data-toggle="collapse"', '', $email); // href=\"javascript:followupEmail();\"
  $email .= '<' . u\emailTag($mya->email, t('your %PROJECT account'), '', $mya->fullName, 'id="email-do" class="collapse"') . '></a>';
  
  w\jsx('followup-email');

  return "<div class=\"well scripts\"><h3 class=\"above-details\">$head$subtext</h3><div id=\"intros\"><b class=\"loud\">$mya->phoneDash</b>$links</div>$details</div>";
}

/**
 * Return statistics about member donations for members signed up after a certain date.
 */
function donationTrend($co = FALSE, $since = NULL) {
	u\setDft($since, NOW - YEAR_SECS);
	$co = $co ? 'u.:IS_CO' : 'NOT u.:IS_CO';
	$where = "$co AND u.:IS_OK AND u.created>:since";
	
	$sql =<<< X
	  SELECT AVG(g.amount*IF(often='M', 12, IF(often='Q', 4, IF(often='Y', 1, 0)))) AS regular
		FROM r_gifts g LEFT JOIN users u ON u.uid=g.uid 
		WHERE g.completed=0 AND $where
X;
  $subs = compact('since');
	$res = db\q($sql, $subs)->fetchAssoc();
	
	$sql = "SELECT AVG(u.:IS_ROUNDUP) AS roundupRatio, AVG(u.crumbs) AS crumbs FROM users u WHERE $where";
	$res += db\q($sql, $subs)->fetchAssoc();
	
	$accts = db\count('users u', $where, $subs);
	
	$sql = "SELECT COUNT(*) AS txCount,AVG(amount%1) AS cents FROM r_txs t JOIN users u ON u.uid=t.payer WHERE $where";
	$res += db\q($sql, $subs)->fetchAssoc();
	$txTotal = db\q("SELECT SUM(t.amount) FROM r_txs t JOIN users u ON u.uid=t.payee WHERE $where", $subs)->fetchField();
	
	return $res + compact(ray('accts txCount txTotal'));
}

/**
 * Return an eloquent statement of the member's gifts to date, to follow "Thank you ".
 */
function giftsToDate() {
  global $mya;

  if (!$mya) return '';
  $mya->giftsDesc($often, $all);
  if (!$all) return '';
  list ($total, $x1, $x2) = explode('+', "$all++");
  if ($x = u\unite(trim($x1), $x2)) $x = t('for choosing %x', '_x', $x);
  if ($total) $total = t('for your contributions, totaling ') . $total;
  return u\unite($total, $x, t(', and '));
}

/*
to find orphaned customers

eachA(function ($a) {
  $name = str_replace("'", "\\'", $a->fullName);
///  if ($reid = f('db.lookup', 'reid', 'r_relations', "other=0 AND data LIKE '%$name%' AND reid NOT IN(26742000000222, 26742000000216)")) debug("$reid $name $a->id");
});
*/