<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace rCredits\Testing;
use rCredits\Testing as t;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Backend as be;

global $base_path;
define('BASE_PATH', $base_path);
define('DW_TEST_PASS', 'Password1'); // usdPass for sandbox accounts
define('DW_TEST_PIN', '9999'); // usdPin
define('DW_TEST_ACCTS', DW_TESTER_ACCT . ' ' . '8127419900 8127431130 8127423717 8127418871 8127401872'); // usdAccount
define('DW_TEST_AUTHS',
  'lBtIKmAL/b9PzZhrPX0m5SwIGMtAIHcxXiEq2gTMYwtZ+LYK7x '
. 'Y6dveGxn4tFpaLCpYPy5p0Dr+fKS5yt1sQcdnMuZwd2fEFeha3 '
. 'pxj/Lr+UEFRJIXdqykS6GK8eOwFb5RJacv6PRenp1c+GCJvTW6 '
. 'ekRW2CfBdsuezuuZapK2mnWK1pqy157zoZnXmUXYoL2rBEEYWw ' // x0QfZr7LuNwt4XFPMh%2FWhSRsOyYXsNW0Ye8z0BpMZ%2FusVEVdEk ?
. 'W6J/pu0Gu3aLD7htZ+2AKBjvi7dnHYXeU0VKHu0SRFoOZB3T7G '
. 'v8bSxiLSb58y42i1lBeH7x1zxUrnygEnpe3AKFSy7HCjhTsC7V '
);

function makeRecords($list, $type, $testSeparate = FALSE) {
  global $testOnly;
  $table = 'r_' . strtolower($type) . 's';
  $function = 'rCredits\\Testing\\';
  if (!function_exists($function . "make$type")) {
/**/    if ($testOnly) t\output("Expect $type: " . print_r($list, 1), 'expect');
    $type = 'Thing';
  }
  $function .= ($testSeparate and $testOnly) ? "verify$type" : "make$type";
  foreach ($list as $one) if (!$function($one, $testOnly, $table)) return FALSE;
  return TRUE;
}

/*
 * Generic record-maker/record-tester.
 */
function makeThing($info, $testOnly, $table) {
  extract($info);
  $info['uid'] = t\uid($id); unset($info['id']);
  return $testOnly ? verifyRecord($table, $info) : db\insert($table, $info);
}

function gifts($list) {return makeRecords($list, 'Gift');}
function notices($list) {return makeRecords($list, 'Notice');}
function members($list) {return makeRecords($list, 'Account');}
function devices($list) {return makeRecords($list, 'Boxe');}
function relations($list) {return makeRecords($list, 'Relation');}
function transactions($list) {return makeRecords($list, 'Tx', TRUE);}
function balances($list) {return makeRecords($list, 'Balance');}
function membersHave($list) {return makeRecords($list, 'MemberFields');}
function selling($list) {return makeRecords($list, 'Selling');}
function companyFlags($list) {return makeRecords($list, 'CompanyFlags');}

/**
 * Create device (box) records.
 */
function makeBoxe($info = array(), $testOnly = FALSE) {
  global $channel; // be careful not to overwrite this with a local variable
  //$boxName = $info['boxName'];
  $info['channel'] = @strtr(@$info['channel'], u\ray('sms app web', TX_SMS, TX_SMART, TX_WEB)) ?: $channel;
  //if ($testOnly) $boxName = htmlspecialchars(@$boxName);
  return makeThing($info, $testOnly, 'r_boxes');
}

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function makeAccount($info = array(), $testOnly = FALSE) {
/**/  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info); // any user table fields, plus dw (dwolla test account number)
  u\setDft($id, r\acct::nextId()); // default to first available number
  $uid = uid($id); // change .AAB to 1, etc.

  if (@$email) $mail = $info['mail'] = $email;

  if (!$testOnly) { // when making, set many defaults and mods
    u\setDft($rebate, 5); // should be R_REBATE, but many of the test would have to be written on changing it
    u\setDft($fullName, 'makeAccount ' . randomString(20, 'word'));
    $name = u\shortName($fullName);
    u\setDft($legalName, $fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
    u\setDft($pass, '123');
    u\setDft($mail, chr(ord('a') + $uid - r\unQuid('.ZZA')) . '@example.com'); // a@example.com, etc.
    u\setDft($community, r\serverUid());
    u\setDft($postalCode, '01301');
    u\setDft($country, 'US');
    $moneyFields = 'share committed floor r usd rewards'; // not minimum because NULL means not set, in Membership page
    foreach (u\ray($moneyFields) as $one) u\setDft($$one, 0);
    foreach (u\ray('cc cc2') as $one) if (isset($$one)) {
      $secure[str_replace('cc', 'cardCode', $one)] = $$one;
      unset($info[$one]);
    }
    $fixFields = 'rebate fullName legalName name pass mail secure community postalCode country ' . $moneyFields;
  } else $fixFields = '';

  $flags0 = @$flags;
    /*if (strpos($flags0, '%') !== FALSE) {
    $flags = B_DFT | u\bit(isset($flags0) ? @$flags0 : (u\bit(B_MEMBER) | u\bit(B_OK)));
  } else { */
  $flags = 0;
  if (@$flags0) foreach (u\ray($flags0) as $one) {
    $bit = u\consta('b', $one);
    $flags |= u\eq($bit, B_DFT, u\bit($bit));
  }
  if ($flags & u\bit(B_OK)) $flags |= u\bit(B_MEMBER);

  if (isset($acctType)) $flags |= (($acctType === R_PERSONAL or $acctType == 'personal') ? u\bit(B_PERSON) : u\bit(B_COMPANY)); // === required
  
  foreach (u\ray('id email acctType dw') as $one) unset($info[$one]);
  $info = compact(u\ray('uid flags ' . $fixFields)) + $info;
  fixValues($info, TRUE);

  if ($testOnly) {
    if (!$acct = r\acct($uid)) return output("No such uid: $uid");
    foreach ($info as $key => $value) {
      if (!eq($acct->$key, $value)) {
        if ($key == 'pass' and (!isset($pass) or r\Web\passwordOkay($acct, @$pass))) continue;
        return output("Wrong value for $key. Expected:$value Got:" . $acct->$key);
      }
    }
    return TRUE;
  }
  if (!$a = new r\acct($info)) return FALSE;

  if (is_numeric(@$dw)) {
    $accts = u\ray(DW_TEST_ACCTS);
    $auths = u\ray(DW_TEST_AUTHS);
    list ($usdAccount, $auth) = array($accts[$dw], $auths[$dw]);
    list ($usdEmail, $usdPass, $usdPin) = array("gordon+william+$dw@dwolla.com", DW_TEST_PASS, DW_TEST_PIN);
    $usdType = 'Dwolla';
    $pin = $usdPin;
    $secure = compact(u\ray('usdType usdPin pin auth')) + ($a->secure ?: array());
    $a->update(compact(u\ray('usdAccount secure')));
    $us = new r\usd($a);
///debug("dw=$dw usnum=$us->number test=" . DW_TESTER_ACCT); 
    if ($us->number != DW_TESTER_ACCT) $us->setbal(0, $err);
  }
  
  if (@$picture) if (!makePicture($picture, $a)) return FALSE;

  return $uid;
}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if called with $testOnly TRUE, return assoc with the information actually written (called from verifyTxs())
 *   otherwise return serial (treated as TRUE in test steps)
 */
function makeTx($info, $testOnly = FALSE) {
  global $channel;
  extract($info);

  if (@$xid and !is_numeric($xid)) $xid = u\a2n(substr(strpbrk($xid, R_MEMBER_MARK . R_AGENT_MARK), 1));
  u\setDft($type, TX_TRANSFER);
  u\setDft($state, TX_DONE);
  u\setDft($created, REQUEST_TIME);
  foreach (u\ray('type state') as $one) if (!is_numeric($$one)) $$one = u\consta('tx', $$one);
  $payer = uid($from);
  $payerAgent = uid(@$fromAgent ?: ($type == TX_TRANSFER ? $from : r\communityUid()));
  $payee = uid($to); 
  $payeeAgent = uid(@$toAgent ?: $to );
//  u\setDft($r, $amount);
  u\setDft($taking, 0);
//  if ($amount != $r) u\setDft($usdXid, 10000 + $xid);
  $for = @$purpose ?: 'cash';
  if (strpos($for, ',')) $for = explode(',', $for); // specifying both payerFor and payeeFor

  u\setDft($goods, ($type != TX_TRANSFER or u\forCash($for)) ? 0 : 1);
  if ($goods) list ($rebate, $bonus) = array(r\rebate($payer, $payee, $amount), r\bonus($payee, $payer, $amount));
  foreach (u\ray('from to purpose for') as $one) unset($info[$one]);
  
  $info = compact(u\ray('xid created r payer payerAgent payee payeeAgent for type state channel goods taking data')) + $info;
  if ($testOnly or $type == TX_REBATE or $type == TX_BONUS) {
    list ($info['payerFor'], $info['payeeFor']) = is_array($for) ? $for : array($for, $for);
    unset($info['for']);
    return $info;
  }
  
  $txs = r\transact($info);
  return $txs[0]['xid'];
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function makeRelation($info, $testOnly = FALSE) {
  extract($info);
  
  $reid = u\num($id, u\a2n(substr($id, -3)));
  if (!$mainAcct = r\acct(t\uid($main))) return output("main:$main is not an id");
  if (!$agtAcct = r\acct(t\uid($agent))) return output("agent:$agent is not an id");
  u\EXPECT(!r\foreignServer(r\uidRegion($mainAcct->id)) or !r\foreignServer(r\uidRegion($agtAcct->id)));
  u\EXPECT($mainAcct->proSe() and $agtAcct->proSe());
  list ($main, $other) = array($mainAcct->id, $agtAcct->id);
  $info['permission'] = @$permission ? u\consta('b', $permission) - B_RELATED : 0;
  if (@$draw) $agtAcct->setBit(B_DRAWS);
  
  foreach (u\ray('id main agent') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other'));
  return $testOnly ? verifyRecord('r_relations', $info) : db\insert('r_relations', $info);
}

/**
 * List what a company sells
 */
function makeSelling($info, $testOnly = FALSE) {
  extract(u\just('id selling', $info));
  $selling = str_replace(',', "\n", $selling);
  return r\acct(t\uid($id))->update(compact('selling'));
}

/**
 * List what flags the company has set
 */
function makeCompanyFlags($info, $testOnly = FALSE) {
  extract(u\just('id flags', $info));
  $coFlags = 0;
  if ($flags) foreach (u\ray($flags) as $one) $coFlags |= u\bit(u\consta('co', $one));
  return r\acct(t\uid($id))->update(compact('coFlags'));
}

/**
 * Return TRUE if the notice has been queued to member $id
 */
function notice($notice, $id, $subs) {
  global $base_url;
  global $testOnly; if (!$testOnly) return FALSE;

  $subs = @$subs[0];
  if (strpos($nm = @$subs['otherName'], 'href="do/')) {
    $subs['otherName'] = str_replace('href="do/', "href=\"$base_url/do/", $nm);
  }

  $uid = uid($id);
  if (is_array($subs)) foreach ($subs as $key => $value) {
    if (preg_match('/^\$(.*)(r|us)?$/U', $value, $matches)) {
      list ($num, $currency) = array($matches[1], @$matches[2]);
      if ($currency == 'us') $currency = 'u';
      $subs[$key] = u\fmtAmt($num, 's$' . $currency);
    }
  }
  $message = u\tt($notice, @$subs);
  $sent = @$subs['sent'] ?: 0;
  $info = compact(u\ray('uid message sent'));
/**/ return db\exists('r_notices', $info) ?: t\output('Expected notice ' . print_r($info, 1));
}
  
/**
 * Make the member fields be thus and such OR test to see if they ARE thus and such.
 * "?" in a field means ignore its value when testing
 */
function makeMemberFields($one, $testOnly) {
  $balFields = u\just('id dw/usd r usd balance dwolla rewards minimum share committed', $one);
  if (!makeBalance($balFields, $testOnly)) return;

  $a = r\acct($id = t\uid($one['id']));
  foreach ($balFields as $k => $v) unset($one[$k]); // don't rehandle balance fields, and already got id
  fixValues($one, TRUE);

  if ($testOnly) {
    foreach ($one as $key => $value) if ($value != '?') {
      if (t\badEq("db $key", $a->$key, $value, $id)) return FALSE;
    }
    return TRUE;
  } else return ($a->update($one));
}

/**
 * Make the cached balances and other numeric fields be thus and such.
 * "dwolla" is a special field that sets or tests the actual dwolla balance for a test account
 * "dw/usd" means set or test both dwolla and usd
 * "?" in a field means ignore its value when testing
 */
function makeBalance($one, $testOnly) {
  if (isset($one['dw/usd'])) {
    $one['usd'] = $one['dwolla'] = $one['dw/usd'];
    unset($one['dw/usd']);
  }
  extract(u\just('id r usd balance dwolla rewards minimum share committed', $one));
  $a = r\acct($id = t\uid($id0 = $id));
  if ($hasdw = in_array($a->usdAccount, array(DW_TEST_ACCT, DW_TESTER_ACCT))) $us = new r\usd($a);
  $dodw = (isset($dwolla) and $hasdw);

  if (!$testOnly) { // setting the values
    if ($dodw and !$us->setbal($dwolla, $err)) return t\output("Can't set dwolla bal for $id to $$dwolla -- $err");
    if ($hasdw and @$usd == '?') {
      $one['usd'] = $us->bal($err);
      if (@$err) return t\output("Can't get dwolla bal for $id -- $err");
    }
    unset($one['id']);
    unset($one['dwolla']);
    return ($a->update($one));
  }

  // verify
  $calc = t\uidCredit($id);
  foreach (u\ray('r usd balance rewards') as $one) {
    if (isset($$one) and $$one != '?') {
      $cachedVal = $one == 'balance' ? $a->r + $a->usd : $a->$one;
      if (($id > 0 or $one == 'usd') and t\badEq("cached $one", $cachedVal, $$one, $id)) return FALSE;
      if ($one != 'usd' and ($id > 0 or $one == 'r')) {
        if (t\badEq("calc $one", $calc->$one, $$one, $id)) return FALSE;
      }
    }
  }
  if ($dodw and $dwolla != '?' and t\badEq('dwolla bal', $us->bal($err), $dwolla, $id)) return t\output("Dwolla err: $err");
  foreach (u\ray('minimum share committed') as $one) {
    if (isset($$one) and $$one != '?' and $id > 0) if (t\badEq("db $one", $a->$one, $$one, $id)) return FALSE;
  }
  return TRUE;
}

function usdTransferCount($where = '1') {
  global $testOnly;
  
  if($testOnly) {
    return db\lookup('COUNT(*)', 'r_usd', $where);
  } else {
    db\q("DELETE FROM r_usd WHERE $where");
    return 0; // 0 is the only thing we can make it!
  }
}

function usdTransfers($txs) {
  global $testOnly;
  
  foreach ($txs as $tx) {
    if ($testOnly) {
      extract(u\just('payer payee amount', $tx));
      $payer = t\uid($payer);
      $payee = @$payee ? (is_numeric($payee) ? $payee : t\uid($payee)) : 0;
      if (!db\exists('r_usd', 'payer=:payer AND amount=:amount and payee=:payee', compact(u\ray('payer payee amount')))) return FALSE;
    } else {
      foreach (u\just('payer payee', $tx) as $key => $value) {
        if ($value) $tx[$key] = t\uid($value);
      }
      u\setDft($tx['txid'], t\seq());
      if (!db\insert('r_usd', $tx)) return FALSE;
      extract($tx);
      $a1 = r\acct($payer);
      
      if (@$payee and $payee != -1) {
        $a2 = r\acct($payee);
        $a1->update('usd r', $a1->usd - $amount, $a1->r + $amount);
        $a2->update('usd r', $a2->usd + $amount, $a2->r - $amount);
        if ($amount > DW_FEE_THRESHOLD) r\queue('coverFee', compact(u\ray('txid payee')));
      } elseif (@$completed) $a1->update('usd', $a1->usd - $amount);
    }
  }
  return TRUE;
}

/**
 * Return FALSE if the cached and actual USD balances are not as expected.
 *//*
function usd($list) {
  global $testOnly; if (!$testOnly) return t\output('usd cache/actual cannot be made');
  foreach ($list as $one) {
    extract(u\just('id cached actual', $one));
    $us = new r\usd($a = r\acct(t\uid($id)));
    if (!in_array($a->usdAccount, array(DW_TEST_ACCT))) return t\output('bad account for usd');
    if (t\badEq('cached usd', $a->usd, $cached, $id)) return FALSE;
    if (t\badEq('actual usd', $us->bal($err), $actual, $id)) return t\output("Dwolla err: $err");
  }
  return TRUE;
}
*/

/**
 * Run the op in the background, return TRUE.
 */
function cronRuns($op) {
  global $testOnly; if($testOnly) return FALSE;
  global $cronOp; $cronOp = $op;
  $func = "rCredits\\Cron\\$op";

  $now = REQUEST_TIME;
  \variable_set('r_last_cron', u\ray('day week month', $now, $now, $now)); // normally don't run periodics

  if ($op == 'ALL' or strpos(R_DAILY_TASKS, " $op ")) {
/**/  if (strftime('%H') < R_DAILY_HOUR) die('Developer: For testing between midnight and :R_DAILY_HOUR:00, you need to temporarily change R_DAILY_HOUR in -settings.');
    db\q("DELETE FROM queue WHERE name='q'");
    \variable_set('r_last_cron', u\ray('day week month', $now - DAY_SECS, 0, 0));
    \drupal_cron_run();
  } elseif ($op == '') {
    \drupal_cron_run();
  } elseif ($op == 'everyWeek') {
    $func(); // queue up weekly tasks
    \drupal_cron_run();
  } else $func();

  return TRUE;
}

/**
 * Return the credit info for the given uid.
 * To get just the r balance, use uidCredit(uid)->r
 * @param int $uid: return credit info for this account id
 */
function uidCredit($uid) {
  $info = be\creditInfo(compact('uid'));
  $a = r\acct($uid);
  if ($uid >0) foreach (u\ray('r rewards') as $one) { // (community amounts don't get cached) (don't include usd)
    if (!t\eq($v1 = $info->$one, $v2 = $a->$one + 0)) {
      output("Calculated $one different from cache: $v1 <> $v2");
      $info->$one = 'error';
    }
  }
  return $info;
}

function uid($id) {
  return is_numeric($id) ? $id
    : (($id == 'community' or $id == 'ctty') ? r\serverUid() 
    : ($id == 'cgf' ? \variable_get('cgf_uid')
    : r\unQuid($id)));
}

/**
 * Accept abbreviations of alphabetic ids for accounts, transactions, and relations
 * @param string $q: the possibly-abbreviated id
 * @return: the full id (if $q is numeric or an already-full alphabetic id, return it unchanged)
 */
function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == R_MEMBER_MARK or $c1 == R_AGENT_MARK) ? (R_SERVER_ID . $q) : $q;
}

function getErrors() {
  $errors = \drupal_get_messages('error', FALSE);
  $errors = @$errors['error'] ?: array();
  foreach ($errors as $key => $value) {
    if (u\abbreviates('Unable to send e-mail', $value) or strpos(strip_tags($value), 'expected to be a reference, value given in drupal_retrieve_form()')) unset($errors[$key]); // ignore these when testing
  }
  return $errors;
}

/**
 * Find the message in the log table.
 * @param assoc $subs: subs for message iff message index is passed.
 */
function findMessage($type, $fields, $message = '', $subject = '', $subs = array()) {
  global $channel;

  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $myid = @r\acct()->id;
  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $fields['message'] = $message;
    if ($subject) { // must be email
      $fields['message'] = str_replace("\r\n", '', $fields['message']);
      $fields['subject'] = $subject;
    }
  } else unset($fields['message']); // empty message in features means NO message
  $agent = @r\acct()->agentId;
  output(array('EXPECT' => '') + $fields, @$subject ? 'email' : 'output'); // keep this
  $info = u\jsonEncode($fields);
  $crit = "channel=:channel AND type=:type AND myid<=>:myid AND agent<=>:agent ORDER BY logid DESC";
/// debug(compact(u\ray('info type myid agent channel crit'))); // for debugging mysterious email mismatches
/// debug(compact(u\ray('info type myid agent channel')), '', 1);
/// debug("info=:info AND $crit"); debug(compact(u\ray('info type myid agent')));
  return db\exists('r_log', "info=:info AND $crit", compact(u\ray('info type myid agent channel')));
}

function weTellStaff($topic, $subs = array()) {
  global $testOnly; if (!$testOnly) return FALSE;
  if (!@$subs[0]) $subs[0] = array();
  foreach ($subs as $one) {
    $index = 'tell-staff';
    $subject = str_replace('{topic}', $topic, $GLOBALS['emailSubjects'][$index]);
    $find = '%"index":"@index","email":"@R_ORG_EMAIL","subject":"@subject"%';
    if ($id = @$one['id']) {
      $id = t\uid($id);
      $find .= "(#$id)%";
      unset($one['id']);
    }
    $find = u\tt($find, compact('index', 'subject'));
    foreach ($one as $k => $value) $find .= "$value%";
    if (!$last = db\lookup('info', 'r_log', "info LIKE '$find' ORDER BY logid DESC")) {
      if (count($subs) > 1) output("tell-staff can't find $find");
      return FALSE;
    }
  }
  return TRUE;
}

function findEmail($index, $email, $subs, $noBody = FALSE) {
  global $testOnly; if (!$testOnly) return FALSE;
  $subs = @$subs[0] ?: @$subs;
  $esubs = array();
  foreach ($subs as $k => $value) {
    if (strpos($value, ',')) $value = '<li>' . join("</li>\n<li>", explode(',', $value)) . "</li>\n";
    $esubs['{' . $k . '}'] = $value;
  }
  $message = strtr($noBody ? r\emailTemplate($index) : r\emailBody($index), $esubs);
  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
  t\fixEmail($email);
///  debug(compact('index','email','subs','esubs','message','subject'));
  return findMessage('email', compact(u\ray('index email subject')), $message, $subject, $subs);
}

/**
 * Convert country and state names or abbreviations to the corresponding indices, and other tweaks.
 * @param assoc $info: field names and values
 * @param bool $direct: directly to or from db, rather than user input (default FALSE)
 */
function fixValues(&$info, $direct = FALSE) {
  global $rUrl;
  $fields = 'postalCode country state email mail phone dob federalId idProof';
  extract(u\just($fields, $info));
  if (@$postalCode or @$country or @$state) {
    u\setDft($country, 'US');
    if ($country == 'US' or $country == US_COUNTRY_ID) u\setDft($state, 'MA');
  }
  if (@$country) $country = db\lookup('id', 'r_countries', ':country IN (name, iso_code)', compact('country'));
  if (@$state) $state = db\lookup('id', 'r_states', ':state IN (name, abbreviation) AND country_id=:country', compact('state', 'country'));
  if (@$mail) t\fixEmail($mail);
  if (@$email) t\fixEmail($email);
  if (strlen(@$phone) == 1) u\prefix('+1413772000', $phone);
  if (@$idProof) u\prefix("$rUrl/images/", $idProof);
  if ($direct) {
    if (@$federalId) u\digits($federalId);
    if (@$dob) $dob = strtotime(str_replace('/', '-', $dob));
  }
  $info = compact(u\ray($fields)) + $info;
}

function fixEmail(&$email) {
  $email = str_replace("'", '&#039;', htmlspecialchars($email)); // Drupal does this in user_register()
  if (substr(@$email, -1, 1) == '@') $email .= 'example.com'; 
  return $email;
}

function makePicture($picture, $a) {
  $filename = DRUPAL_ROOT . $a->auxFilename('picture', TRUE);
  return (bool) file_put_contents($filename, $picture);
}

// UNUSED
function getPicture($picture) {
  return file_get_contents(DRUPAL_ROOT . R_PICTURE_DIR . "/$picture.jpg");
}

function lastMinuteSubs($string) {
  if (u\abbreviates('%ymd', $string)) return subAgo($string, '%Y-%m-%d');
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if ($string == '%chk') return '&#10004;';
  if ($string == '%ctty') return R_REGION_NAME;
  return $string;
}
  
/**
 * Return the date, formatted as desired.
 * @param string $string: a named date format, dash, how long ago (eg %dmy-3d means 3 days ago "dmy" format)
 * @param string $fmt: what strftime format to use
 */ 
function subAgo($string, $fmt) {
  if (!strpos($string, '-')) $string .= '-0d';
  $lastC = substr($string, strlen($string) - 1);
  list ($a, $b) = explode('-', $string); // $a is ignored
  $bPreC = substr($b, 0, strlen($b) - 1);
  $periods = array('d' => 'days', 'w' => 'weeks', 'm' => 'months', 'y' => 'years');
  $period = $periods[$lastC];
///  debug(compact(u\ray('string fmt lastC a b bPreC period')));
  return (strftime($fmt, strtotime("$bPreC $period ago")));
}

/**
 * Send an RPC API request to the server, for rPOS.
 */
function hitServer($op, $extra = array(), $agent, $device = '', $version = '999') {
  global $rposResult;
  $agent = t\fullQid($agent);
  if (@$extra['member']) $extra['member'] = t\fullQid($extra['member']);
  $input = compact(u\ray('op agent device version')) + $extra;
/**/ t\output('request: ' . print_r($input, 1), 'input'); // keep this
  $rposResult = r\Smart\pos($input);
  if ($op != 'photo') $rposResult = (array) json_decode($rposResult);
/**/ t\output('answer: ' . print_r($rposResult, 1), 'output');
  return TRUE;
}

function parseQr($qr) {
  list ($member, $m, $code) = preg_split('/([\.-])/', $qr, 0, PREG_SPLIT_DELIM_CAPTURE);
  $member = R_SERVER_ID . ($m == '-' ? ':' : $m) . $member;
  return array($member, $code);
}

function snapShot(&$form) { // (additional sta values have no effect at this stage)
  addAttribs($form);
  return; // the rest is redundant
  //global $formOut['text']; $zot = $form; $formOut['text'] = strip(\render($zot)); // for testing
  //output('ACTUAL or EXPECTED: ' . $formOut['text'], 'screen');
}

function addAttribs(&$thing) {
  foreach ($thing as $key => $value) { // else Drupal complains when rendering
    if (substr($key, 0, 1) == '#') continue;
    if (!is_array($value)) continue;
    if (!isset($value['#attributes']['id'])) $thing[$key]['#attributes']['id'] = 'edit-' . strtolower($key);
    if (!isset($value['#title_display'])) $thing[$key]['#title_display'] = 'before';
    addAttribs($thing[$key]);
  }
}

/**
 * Log the given id in.
 * @param string $id: full or partial quid or '?' (meaning anonymous user, so don't log in)
 * @return: successful (always successful for anonymous)
 */
function logIn($id) {
  global $boxUser;
  if ($id != '?') {
    r\acct::setDefault($a = r\acct(t\fullQid($id))); // fullQid not uid
    $boxUser = r\Web\boxUser(r\Web\box(), $a->id);
  }
  return TRUE;
}

function verifyTx($info) {
  $info2 = makeTx($info, TRUE);
  return verifyRecord('r_txs', $info2);
}

/**
 * Compare the data to a corresponding record in the table.
 * Return TRUE if the record matches the data.
 * Example call:
 *   $info = makeRelation($info, TRUE);
 *   return verifyRecord('relations', $info);
 */
function verifyRecord($table, $info) {
/**/  if (!is_array($info)) die('in verifyRecord: ' . print_r(debug_backtrace(), 1)); // keep
  
  unset($info['data']);
  foreach ($info as $key => $value) {
    $crit[] = $value === '' ? "($key IS NULL or $key='')"
    : ((in_array($key, u\ray(R_DATE_FIELDS)) or stripos($key, 'date') !== FALSE) ? 
      "$key-($key%:DAY_SECS)<=>:$key-(:$key%:DAY_SECS)" 
      : "$key<=>:$key"); // null-safe ==
  }
  $crit = join(' AND ', $crit);
  $result = db\exists($table, $crit, $info);
/**/ if (!$result) output("verifyRecord failed. table=$table crit=$crit EXPECTED info=" . print_r($info, 1)); // keep
  return $result;
}

/**
 * Simulate completion (and possible confirmation) of a form.
 * @param string $id: quid for the user (or company agent) completing the form
 * @param string $page: path to form from drupal root
 * @param array $values: array of assoc of field names/values (or empty)
 * @param bool $confirmed: whether the user has confirmed the values (default FALSE)
 * @return TRUE on success
 *
 * @todo: Rewrite this whole form testing system to use just menu_execute_active_handler,
 *   drupal_validate_form, etc. (assuring official behavior)
 */
function completeForm($id, $page, $values = '', $confirmed = FALSE) {
  global $testOnly; if ($testOnly) return FALSE;
  global $fieldErr;
  global $testOp; $testOp = @$values[0]['op'];
  global $testConfirmation; // confirmation message output, if any
  global $formSta;
/// debug(compact(u\ray('id page values confirmed testOnly'))); // keep

  $values = @$values[0] ?: array();
  t\fixValues($values);
  if (isset($values['amount']) and !is_numeric($values['amount'])) {
    $a = r\acct(t\fullQid($id));
    list ($values['amount']) = t\parseAmt($values['amount'], $a);
  }
  if (@$values['email']) $values = u\changeKey('email', 'mail', $values);
  
  t\logIn($id);

  $validate = TRUE;
  if (u\abbreviates('transactions/', $page) and $confirmed) {
    $validate = FALSE;
    $formSta['confirm'] = TRUE;
  }

  $_POST = $values;
  if (!$form = t\pageForm(t\fixPageArgs($page), FALSE)) return FALSE; // probably access error
  $sta = $formSta; $formSta = NULL; // don't let $formSta influence other forms that get invoked
  
  if ($page == 'pay' or $page == 'charge') {
    $page = 'Tx';
    if ($confirmed) $validate = FALSE;
  }
  $errorsBefore = t\getErrors();
  
  $dfts = array();
  foreach ($form as $key => $field) if (substr($key, 0, 1) != '#') {
//    u\setDft($dft, @$field['#default_value']);
    if (@$field['#type'] == 'hidden') $dfts[$key] = $field['#value'];
  }
  $sta['values'] = $sta['input'] = $values + $dfts; // add to whatever status the form left off in
  $fieldErr = FALSE; // no field errors yet
  
//  $form_id = "rCredits\\Web\\
//  if ($validate) drupal_validate_form($form_id, $form, $formSta);
  if ($validate) foreach (t\postProcess($form, $page, 'validate') as $one) $one($form, $sta);
  
//  if (count($errors = t\getErrors()) > count($errorsBefore)) return TRUE;
    // NO! if (!empty($errors)) return FALSE;
/// debug(compact(u\ray('id page values confirmed one errors')));
//    return FALSE; // ? (added 7/17/2013)
//  } else {
//    $testConfirmation = @$sta['confirm'];
  $lastIdentifierTag = '</div identifiers>';
  if ($i = strpos($testConfirmation, $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
  if ($fieldErr) return TRUE; // no submit if field error
  foreach (t\postProcess($form, $page, 'submit') as $one) $one($form, $sta);

  if ($page == '/user/login') { // fudge this test until we can take over the login process from Drupal
    if (!$id = @$sta['uid']) return TRUE; // login failed, but test succeeded
    t\logIn($id);
    r\Web\loginFollowup($sta, r\acct($id)->account());
  }

  return TRUE;
}

/**
 * Simulate a visit to the page, return its form.
 * @param string $page: the Drupal URL to visit
 * @param string $msg: message to output, if any
 * @param string $field: error field, if any
 * @return: the form before rendering
 */
function pageForm($page, $msg = '', $field = '') {
  global $formArray, $formSta;
  
  if ($msg) r\Web\say($msg, $field);
  $login = (strpos($page, 'user/login') !== FALSE);
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);

  if (u\abbreviates('/user/', $page)) {
    $sta = @$formSta ?: array();
//    list ($form, $function) = strpos($page, '/signup') ? array('user_register_form', 'formRegister')
    list ($form, $function) = FALSE ? array('user_register_form', 'formRegister')
      : ($login ? array('user_login', 'formLogin') : array());
    $function = 'rCredits\\Web\\' . $function;
    // (FAILS for user_login) $form = \drupal_get_form($form, $args);
    $form = $login ? array() : \drupal_get_form($form, $args);
    $function($form, $sta, $args);
    if ($login and @$form['newPass']) $form['newPass'] = \form_process_password_confirm($form['newPass']);
    $form2 = $form; $rent = \drupal_render($form2);
    getFormOut($rent);
    return $form;
  }     

  $result = \menu_execute_active_handler($page, FALSE);
  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found' 
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\output("page \"$page\": $result in t\\pageForm()");
  } else getFormOut($result);
  return $formArray;
}

function getFormOut($out) {
  global $formOut;

  $formOut = array();
  $formOut['options'] = t\dropdownOptions($out) . PHP_EOL . t\radioOptions($out);
  $formOut['steps'] = t\getSteps($out);
  $formOut['done'] = join('', t\getSteps($out, TRUE));
  $formOut['text'] = t\strip($out);
  $formOut['notags'] = strip_tags($out);
  t\output('ACTUAL: ' . $formOut['text'], 'screen');
}

/**
 * Return an array of steps in the output
 * @param string $out: the rendered form
 * @param bool $set: TRUE = return just the done steps, FALSE = just the not-done steps, DEFAULT = both
 * @return: array of step numbers
 */
function getSteps($out, $set = NULL) {
  if (!@$set) $set = isset($set) ? '>' : '';
  if (!preg_match_all("/done-(\d) state$set/", $out, $matches)) return array();
  return $matches[1];
}

function postProcess($form, $formName, $type) {
  $functions = @$form["#$type"];
  if (!@$functions or @($functions == '<')) {
    $function = "rCredits\\Web\\form{$formName}_$type";
    $functions = function_exists($function) ? array($function) : array();
  }
  return $functions;
}

function postLogout($function) {
  return TRUE; // never actually log out
  global $skipToStep;
/*  if ($messages = @cache_get('t_messages')->data) {
    extract(u\just('msgs skipToStep', $messages));
    if ($skipToStep == $function) {
      if (strlen(serialize($msgs)) > strlen(serialize(@$_SESSION['messages']))) {
        foreach (u\ray('status error notice') as $type) {
          $_SESSION['messages'][$type] = array_merge((@$msgs[$type] ?: array()), (@$_SESSION['messages'][$type] ?: array()));
          if (empty($_SESSION['messages'][$type])) unset($_SESSION['messages'][$type]);
        }
      }
//      cache_set('t_messages', FALSE);
u\deb('postLogout returning TRUE, skipToStep was ='.@$skipToStep.' (now NULL)');
      $skipToStep = NULL; // strangely, this has to be = NULL not unset()
      return TRUE;
    }
  }
  */
u\deb('postLogout returning FALSE, about to save skiptostep '.$function);
  $vars = array(
    'msgs' => @$_SESSION['messages'] ?: array(),
    'skipToStep' => $function, // tell gherkin function to skip steps in interrupted scene until this step
  );
//  cache_set('t_messages', $vars); // save the test results so far
  return FALSE;
}

function dropdownOptions($form) {
  if (!preg_match_all('~<option [^>]*>.*?</option>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function radioOptions($form) {
  if (!preg_match_all('~<label class="option" [^>]*>.*? </label>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}  

function strip($form) {

  $form = str_replace('&nbsp;', ' ', $form);
  $form = preg_replace('~<fieldset id="help".*?</fieldset>~sm', '', $form); // remove help div
  $form = preg_replace('~<div[^>]*class="invisible.*?</div>~sm', '', $form); // remove invisible divs (not nested)
  $form = preg_replace('~<option value="[^"]*">.*?</option>~sm', '', $form); // remove unselected options
  $form = preg_replace('~<input type="hidden".*?/>~sm', '', $form); // remove hidden inputs
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/sm', '$1', $form); // show button labels
  $form = preg_replace('/> *</sm', '> <', $form); // make sure there's exactly one space between things
  return preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form));
}

function eq($a, $b) {
  $result = (is_numeric($a) and is_numeric($b)) ? 
      (abs($a - $b) < .0001)
    : ($a == $b or (empty($a) and empty($b)));
/**/ if (!$result) echo 'not equal: ' . print_r(compact('a','b'), 1); // keep for now
//  if (!$result) output("unequal: a=$a b=$b", 'error'); // keep for now
  return $result;
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  if (!t\est()) return $_SERVER[$key];
  global $T_SERVER;
  if (isset($value)) $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 * Not to be confused with the color() function in test.php
 */
function color($msg, $color = 'normal') {
  $colors = array(
    '' => 'ivory',
    'normal' => 'ivory',
    'expect' => 'cyan',
    'email' => 'lightblue',
    'input' => 'orange',
    'output' => 'thistle',
    'screen' => 'burlywood',
    'error' => 'salmon',
  );
  $color = @$colors[$color] ?: $color;
/**/ $msg = print_r($msg, 1); // keep this
  return "<pre style='background-color:$color;'>$msg</pre>";
}

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function output($msg, $color = 'error') {
  global $testModule;
  if (t\est() and !@$testModule) \drupal_set_message(color($msg, $color));
  return $color == 'error' ? FALSE : TRUE;
}

/**
 * Return 't' and a number unique to the current test run (also meaning TRUE) if we are testing
 * otherwise FALSE. The return is used with USD_NOTE_PREFIX in usd class
 * call by t\est()
 */
function est() {return (@TESTING == 1 ? 't' . REQUEST_TIME : FALSE);}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch) {
  global $formOut, $formArray, $formSta, $testOp, $testConfirmation;
  global $sms_devel; $sms_devel = TRUE;
    
  $formOut = $formArray = $formSta = $testOp = $testConfirmation = NULL;
  global $sms_devel; $sms_devel = TRUE;
  global $testSeq; $testSeq = NULL;
  global $channel; $channel = $ch;

  $cgfUid = \variable_get('cgf_uid');
  $tables = 'r_boxes r_companies r_do r_gifts r_invites r_log r_notices r_proxies r_relations r_txs r_stats r_usd r_user_industries queue';

  db\q('DELETE FROM users WHERE uid>:cgfUid OR (uid>1 AND uid<:cgfUid-1)', compact('cgfUid'));
  foreach (u\ray($tables) as $one) db\q("TRUNCATE $one");
  db\q('UPDATE users SET floor=0,minimum=NULL,r=0,usd=0,rewards=0,committed=0');

//  $files = glob(DRUPAL_ROOT . R_PICTURE_DIR . "/*"); // get all picture file names
  $dir = DRUPAL_ROOT . R_PICTURE_DIR;
  $zza = '' . r\unQuid('.ZZA');
  $zza = substr($zza, 0, strlen($zza) - 1); // ignore last digit
  shell_exec(str_replace('/', '\\', "DEL \"$dir/$zza*\""));

//  t\setupDwAccts();

  tlog('clearing');
  r\acct::_clear(); // empty the acct cache
  \drupal_static_reset(); // clear out drupal central storage
}

function fixPageArgs($page) {
  $special = array(
    '(by|id)=([A-Z]{3}?' . R_MARKS . '[A-Z]{3})',
    '(flow)=(.*?)',
  );
  foreach ($special as $one) if (preg_match($pat = "`(/|&)$one(&|$)`", $page, $matches)) {
    list ($zot, $start, $word, $what, $end) = $matches;
    if ($word == 'by' or $word == 'id') $what = t\uid($what);
    if ($word == 'flow') $what = u\consta('flow', $what);
    $page = preg_replace($pat, "$start$word=$what$end", $page);
  }
  return $page;
}

/**
 * Parse a partially-USD amount expression, such as "AMT+34.21"
 * @param string $var: the expression to parse -- a simple addition or subtraction to/from a named variable
 *   representing the account's initial USD balance -- or just the named variable itself.
 *   if the variable name is preceded by '+', the variable is set to the USD balance
 * @param acct $a: the account whose USD balance might be a parameter in the expression $s
 * @return: [$value, $bal], where
 *   $bal is the current actual USD balance for account $a
 *   $value is the value of the expression
 */
function parseAmt($var, $a) {
  u\EXPECT(!is_numeric($var), 'expected non-numeric var');
  
  $bal = $a->actualUsd();
///  debug(compact('a','bal','var'));

  if (($i = strpos($var, '+')) or ($i = strpos($var, '-'))) {
    $op = substr($var, $i, 1);
    list ($var, $value) = explode($op, $var);
    if ($op == '-') $value = -$value;
  } else $value = 0;
  if (substr($var, 0, 1) == '+') list ($set, $var) = array(TRUE, substr($var, 1));
  global $$var;
  if (@$set) $$var = $bal; // not defined yet, set the var to actual USD balance
///  debug(compact(u\ray('var a bal i op value')));
  return array($$var + $value, $bal);
}

function badEq($type, $v1, $v2, $id) {
  if (t\eq($v1, $v2)) return FALSE;
  t\output("bad $type v1=$v1, expected v2=$v2 uid=$id");
  return TRUE;
}

/**
 * Say whether the most recent page included the given text or options.
 * @param array $content: array of assocs -- named values to check for
 *   The names (labels) will be sought once also, unless preceded by underscore (_).
 *   The array can go either way, horizontally or vertically
 * @param string $type: description of what to check for (default 'text')
 * @param bool $showDebugs: whether to report debugging info while looking
 * @return TRUE if all the content is included
 */
function shownWith($content, $type = 'text', $showDebugs = TRUE) {
  global $formOut;
  $model = $formOut[$type];
  if (!$content) return FALSE; // probably an accidental single-row array with no labels

  array_unshift($content, array_keys($content[0])); // look for labels, too (first line)

  $new = array();
  if ($vertical = (count($content) > 2 and count($content[1]) == 2)) { // keys on the left, so switch it
    foreach ($content as $one) {
      $one = array_values($one);
      if (substr($one[0], 0, 1) == '_') $one[0] = ''; // ignore _label
      foreach ($one as $oneV) if ($oneV !== '') $new[] = $oneV; // label and value(s)
    }
  } else {
    foreach ($content[0] as $i => $k) {if (substr($k, 0, 1) == '_') $content[0][$i] = '';} // ignore _label
    $new = array();
    foreach ($content as $one) foreach ($one as $v) $new[] = $v;
  }
  
  $content = $new;
  $xi = $i = 0;
  foreach ($content as $one) {
    if (strpos($one, '%') !== FALSE) $one = t\lastMinuteSubs($one);
    if ($showDebugs) t\output("EXPECT page with " . u\purify($one, ''));
    if (($one = u\redash($one)) !== '') $i = strpos($model, $one, $xi = $i);
    if ($showDebugs) t\output(compact('xi','i','one'));
    if ($i === FALSE) {
      if ($showDebugs) t\output(substr($model, 0, $xi) . ' <b style="font-size:200%;">[OK until here]</b> ' . substr($model, $xi));
      return FALSE;
    } else $i += strlen($one);
  }
  return TRUE;
}

function constants($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(u\just('name value', $one));
    
  }
}

function seq($table = 'r_usd') {
  global $testSeq;
  $id = $testSeq[$table] = @$testSeq[$table] + 1;
  $k = $table == 'r_usd' ? 'txid' : '';
  tlog("seq table=$table id=$id");
  u\EXPECT(!db\exists('r_usd', "$k=:id", compact('id')), "duplicate seq: $id in table $table");
  return $id;
}

function tlog($msg) {u\log($msg, 'debug');}