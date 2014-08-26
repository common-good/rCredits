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
use rCredits\Admin as a;

global $base_path;
define('BASE_PATH', $base_path);
define('DW_TEST_PASS', 'Password1'); // usdPass for sandbox accounts
define('DW_TEST_PIN', '9999'); // usdPin
define('DW_TEST_ACCTS', DW_TESTER_ACCT . ' ' . '8127419900 8127431130 8127423717 8127418871 8127401872'); // usdAccount
define('DW_TEST_AUTHS',
  DW_TESTER_AUTH . ' '
. 'Y6dveGxn4tFpaLCpYPy5p0Dr+fKS5yt1sQcdnMuZwd2fEFeha3 '
. 'pxj/Lr+UEFRJIXdqykS6GK8eOwFb5RJacv6PRenp1c+GCJvTW6 '
. 'ekRW2CfBdsuezuuZapK2mnWK1pqy157zoZnXmUXYoL2rBEEYWw ' // x0QfZr7LuNwt4XFPMh%2FWhSRsOyYXsNW0Ye8z0BpMZ%2FusVEVdEk ?
. 'W6J/pu0Gu3aLD7htZ+2AKBjvi7dnHYXeU0VKHu0SRFoOZB3T7G '
. 'v8bSxiLSb58y42i1lBeH7x1zxUrnygEnpe3AKFSy7HCjhTsC7V '
);

function doRecords($list, $type) {
  global $testOnly;
/**/    if ($testOnly) t\output("Expect $type: " . print_r($list, 1), 'expect');
/**/  u\EXPECT(!is_numeric(key($list[0])), 'doThing should be an assoc: ' . print_r($list, 1));

  $function = 'rCredits\\Testing\\do';
  $function .= function_exists($function . $type) ? $type : 'Thing';
  foreach ($list as $one) if (!$function($one, $type)) return FALSE;
  return TRUE;
}

/*
 * Generic record-maker/record-tester.
 */
function doThing($info, $type) {
  global $testOnly;
  extract($info);
  if (@$id) $info['uid'] = t\uid($id); unset($info['id']);
  foreach (u\ray('inviter invitee') as $one) if (@$$one) $info[$one] = t\uid($$one);
  $table = 'r_' . strtolower($type) . 's';
  return $testOnly ? verifyRecord($table, $info) : db\insert($table, $info);
}

function invites($list) {return doRecords($list, 'Invite');}
function gifts($list) {return doRecords($list, 'Gift');}
function notices($list) {return doRecords($list, 'Notice');}
function members($list) {return doRecords($list, 'Account');}
function invoices($list) {return doRecords($list, 'Invoice');}
function devices($list) {return doRecords($list, 'Boxe');}
function relations($list) {return doRecords($list, 'Relation');}
function transactions($list) {return doRecords($list, 'Tx');}
function balances($list) {return doRecords($list, 'Balance');}
function membersHave($list) {return doRecords($list, 'MemberFields');}
function selling($list) {return doRecords($list, 'Selling');}
function companyFlags($list) {return doRecords($list, 'CompanyFlags');}

/**
 * Create device (box) records.
 */
function doBoxe($info = array()) {
  global $testOnly, $channel; // be careful not to overwrite channel with a local variable
  //$boxName = $info['boxName'];
  $info['channel'] = @strtr(@$info['channel'], u\ray('sms app web', TX_SMS, TX_SMART, TX_WEB)) ?: $channel;
  //if ($testOnly) $boxName = htmlspecialchars(@$boxName);
  return doThing($info, 'boxe');
}

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function doAccount($info = array()) {
  global $testOnly;
/**/  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info); // any user table fields, plus dw (dwolla test account number)
  u\setDft($id, r\acct::nextId()); // default to first available number
  $uid = uid($id); // change .AAB to 1, etc.

  if (@$email) $mail = $info['mail'] = $email;

  if (!$testOnly) { // when making, set many defaults and mods
    u\setDft($fullName, 'doAccount ' . randomString(20, 'word'));
    $name = u\shortName($fullName);
    u\setDft($legalName, $fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
    u\setDft($pass, '123');

    $zza = r\unQuid('.ZZA');
    $mailDft = ($zza <= $uid and $uid < $zza + 26) ? chr(ord('a') + $uid - $zza) : \user_password(10);
    u\setDft($mail, $mailDft . '@example.com'); // a@example.com, etc.

    u\setDft($community, r\serverUid());
    u\setDft($postalCode, '01301');
    u\setDft($country, 'US');
    $moneyFields = 'share committed floor r usd rewards'; // not minimum because NULL means not set, in Membership page
    foreach (u\ray($moneyFields) as $one) u\setDft($$one, 0);
    if (@$cc) $cardCode = $cc;
    if (@$cc2) $cardCode2 = $cc2;

    $fixFields = 'rebate fullName legalName name pass mail secure community postalCode country cardCode cardCode2 ' . $moneyFields;
  } else $fixFields = '';

  $flags0 = @$flags;
  $flags = 0;
  if (@$flags0) foreach (u\ray($flags0) as $one) $flags |= u\bit(u\consta('b', $one));
  if (!$testOnly) {
    if ($flags & u\bit(B_OK)) $flags |= u\bit(B_MEMBER);
    if ($flags & u\bit(B_DW)) $flags |= u\bit(B_BANK);
  }
  
  if (isset($acctType)) $flags |= (($acctType === R_PERSONAL or $acctType == 'personal') ? 0 : u\bit(B_CO)); // === required
  
  foreach (u\ray('id email acctType dw cc cc2') as $one) unset($info[$one]);
  u\preray(compact(u\ray('uid flags ' . $fixFields)), $info);
  fixValues($info, TRUE);

  if ($testOnly) {
    if (!$acct = r\acct($uid)) return output("No such uid: $uid");
    foreach ($info as $key => $value) {
      if ($key == 'uid') $key = 'id';
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
 * Fabricate a bogus invoice.
 * Minimum parameters: amount, from, to
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the record ID of the created invoices (treated as TRUE in test steps)
 */
function doInvoice($info, $type) {return doTx($info, $type);}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the serial of the created transaction(s) (treated as TRUE in test steps)
 */
function doTx($info, $thing) {
  global $testOnly, $channel;
  
  $inv = ($thing == 'Invoice');
  extract($info);

  u\setDft($created, REQUEST_TIME);
  $payer = uid($from);
  $payee = uid($to); 
//  u\setDft($r, $amount);
//  if ($amount != $r) u\setDft($usdXid, 10000 + $xid);
  $for = @$purpose ?: 'cash';
  if (strpos($for, ',')) $for = explode(',', $for); // specifying both payerFor and payeeFor

  foreach (u\ray('from to for') as $one) unset($info[$one]);
  
  if ($inv) {
    u\setDft($goods, u\forCash($for) ? 0 : 1);
  } else {
    if (@$xid and !is_numeric($xid)) $xid = u\a2n(substr(strpbrk($xid, R_MEMBER_MARK . R_AGENT_MARK), 1));
    u\setDft($type, TX_TRANSFER);
    foreach (u\ray('type') as $one) if (!is_numeric($$one)) $$one = u\consta('tx', $$one);
    u\setDft($goods, ($type != TX_TRANSFER or u\forCash($for)) ? 0 : 1);
    if ($goods) list ($rebate, $bonus) = array(r\rebate($payer, $payee, $amount), r\bonus($payee, $payer, $amount));
    $payerAgent = uid(@$fromAgent ?: ($type == TX_TRANSFER ? $from : r\communityUid()));
    $payeeAgent = uid(@$toAgent ?: $to );
    u\setDft($taking, 0);
    unset($info['purpose']);
    $info = compact(u\ray('xid created payer payerAgent payee payeeAgent type channel goods taking')) + $info;
  }

  if ($testOnly or @$type == TX_REBATE or @$type == TX_BONUS) {
    if ($inv) return verifyRecord('r_invoices', $info);
    if ($testOnly) list ($info['payerFor'], $info['payeeFor']) = is_array($for) ? $for : array($for, $for);
    unset($info['for']);
    return verifyRecord('r_txs', $info); // fails if rebate/bonus description is wrong (so omit it on create)
  }
  
  if ($inv) {
    return db\insert('r_invoices', $info);
  } else {
    $txs = r\transact($info + compact('for'));
    return $txs[0]['xid'];
  }
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function doRelation($info) {
  include_once __DIR__ . '/admin/admin-forms.inc';
  global $testOnly;
  extract($info);
  
  if (@$id) $reid = t\uid(str_replace(R_AGENT_MARK, R_MEMBER_MARK, $id));
//  u\num($id, u\a2n(substr($id, -3)));
  if (!$mainAcct = r\acct(t\uid($main))) return output("main:$main is not an id");
  if (!$agtAcct = r\acct(t\uid($agent))) return output("agent:$agent is not an id");
  //u\EXPECT(!r\foreignServer(r\uidRegion($mainAcct->id)) or !r\foreignServer(r\uidRegion($agtAcct->id)));
  u\EXPECT($mainAcct->proSe and $agtAcct->proSe);
  list ($main, $other) = array($mainAcct->id, $agtAcct->id);
  $info['permission'] = @$permission ? u\consta('b', $permission) - B_RELATED : 0;
  if (@$draw) $agtAcct->setBit(B_DRAWS);
  
  foreach (u\ray('id main agent rCard') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other'));
  
  if ($testOnly) {
    return verifyRecord('r_relations', $info);
  } else {
//    db\insert('r_relations', $info);
    r\acct($main)->newRelation($other, $info['permission'], @$employee, @$isOwner, @$draw, $reid);
    if (@$rCard == 'yes') r\Web\memberId($id); // make a Company Agent rCard
    return TRUE;
  }
}

/**
 * List what a company sells
 */
function doSelling($info) {
  global $testOnly;
  extract(u\just('id selling', $info));
  $selling = str_replace(',', "\n", $selling);
  return r\acct(t\uid($id))->update(compact('selling'));
}

/**
 * List what flags the company has set
 */
function doCompanyFlags($info) {
  global $testOnly;
  extract(u\just('id flags', $info));
  $coFlags = 0;
  if ($flags) foreach (u\ray($flags) as $one) $coFlags |= u\bit(u\consta('co', $one));
  return r\acct(t\uid($id))->update(compact('coFlags'));
}

/**
 * Return TRUE if the notice has been queued to member $id
 */
function notice($notice, $id, $subs = array()) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $base_url, $thatThing;

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
  $message = strip_tags(u\tt($notice, @$subs));
  $sql = 'SELECT message FROM r_notices WHERE uid=:uid AND sent=:sent ORDER BY msgid DESC';
  $info = u\ray('uid sent', $uid, @$subs['sent'] ?: 0);
  $msgs = db\q($sql, $info)->fetchCol();
///  debug(compact(u\ray('message sql info msgs')));
  foreach ($msgs as $msg) if (strip_tags($msg) == $message) {
    $thatThing['notice'] = $msg; // remember for "And that <thing> blah blah blah"
    return TRUE;
  }
  
/**/ return t\output('Expected notice ' . print_r($info + compact('message'), 1));
}

/**
 * Pretend the account has complete the given steps.
 */ 
function doneStep($uid, $step) {
  global $testOnly; if ($testOnly) return FALSE;
  if (strpos($step, ' ')) {
    foreach (u\ray($step) as $one) if (!doneStep($uid, $one)) return FALSE;
    return TRUE;
  }
  
  $a = r\acct($uid = t\uid($uid));
  return @$a->stepDone($step); // might have to do the steps below also
  
  if ($step == 'photo') return t\makePicture('whatever', $a);
  if ($step == 'sign') return $a->update(u\ray('signed', REQUEST_TIME));
  if ($step == 'donate') return db\insert('r_gifts', u\ray('uid amount', $uid, 123));
  if ($step == 'proxies') return db\q('INSERT INTO r_proxies (person, proxy) VALUES (:uid, 2),(:uid, 3)', compact('uid'));
  
  if ($step == 'contact') return $a->update(u\ray('postalAddr', 'The World'));
  if ($step == 'connect') {
    $secure = @$a->secure;
    $secure['bankAccount'] = 'USkk123456789whatever';
    return $a->update(compact('secure'));
  }
  if ($step == 'prefs') return $a->update(u\ray('minimum', 0));
  //if ($step == 'invite') return db\insert('r_invites', u\ray('inviter email code', $uid, 'any@example.com', 'whatever'));
  if ($step == 'company') return $a->update(u\ray('selling', 'stuff'));
  // if ($step == 'relations') ?
  return FALSE;
}

/**
 * Make the member fields be thus and such OR test to see if they ARE thus and such.
 * "?" in a field means ignore its value when testing
 */
function doMemberFields($one) {
  global $testOnly;
  $balFields = u\just('id dw/usd r usd balance dwolla floor rewards minimum share committed', $one);
  if (!doBalance($balFields, $testOnly)) return;

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
function doBalance($one) {
  global $testOnly;
  if (isset($one['dw/usd'])) {
    $one['usd'] = $one['dwolla'] = $one['dw/usd'];
    unset($one['dw/usd']);
  }
  extract(u\just('id r usd balance dwolla floor rewards minimum share committed', $one));
  if (!$a = r\acct($id = t\uid($id0 = $id))) return t\output("account $id0 does not exist");
/// print_r(compact('a','id','id0'));
/// print_r(' unQuid=' . r\unQuid($id0));
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
  foreach (u\ray('floor minimum share committed') as $one) {
    if (isset($$one) and $$one != '?' and $id > 0) if (t\badEq("db $one", $a->$one, $$one, $id)) return FALSE;
  }
  return TRUE;
}

/**
 * Return a field for the account with the given cell number.
 */
function phoneField($field, $number) {
  return @r\acct(r\SMS\uid($number))->$field;
}

function usdTransferCount($where = '1') {
  global $testOnly;
  
  if($testOnly) {
    return db\recCount('r_usd', $where);
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
///      debug(compact(u\ray('payer payee amount')));
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

/**
 * Interpret the given identifier (int, full quid, or local quid) as an account ID.
 */
function uid($id) {
  return is_numeric($id) ? $id
    : (($id == 'community' or $id == 'ctty') ? r\serverUid() 
    : ($id == 'cgf' ? r\cgfId()
    : r\unQuid($id)));
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

  $mya = r\acct();
  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $agent = $mya ? $mya->agentId : 0;
  $myid = $mya ? $mya->id : 0;
/*  if (!$myid and $type == 'email' and $email = @$fields['email']) {
    $myid = $agent = db\lookup('uid', 'users', 'mail=:email', compact('email'));
  } */

  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $fields['message'] = $message;
/*    if ($subject) { // must be email
      $fields['message'] = str_replace("\r\n", '', $fields['message']);
      $fields['subject'] = $subject;
    } */
  } else unset($fields['message']); // empty message in features means NO message
//  output(array('EXPECT' => '') + $fields, @$subject ? 'email' : 'output'); // keep this
  output(array('EXPECT' => '') + $fields, 'output'); // keep this
  $info = u\jsonEncode($fields);
  $crit = "channel=:channel AND type=:type AND myid<=>:myid AND agent<=>:agent ORDER BY logid DESC";
/// debug("info=:info AND $crit"); debug(compact(u\ray('info type myid agent channel')));
  return db\exists('r_log', "info=:info AND $crit", compact(u\ray('info type myid agent channel')));
}

function weTellStaff($topic, $subs = array()) {
  global $testOnly; if (!$testOnly) return FALSE;
  if (!@$subs[0]) $subs[0] = array();
  foreach ($subs as $one) {
    $index = 'tell-staff';
    $subject = str_replace('{topic}', $topic, $GLOBALS['emailSubjects'][$index]);
    $find = '%"index":"@index","email":"@R_REGION_EMAIL","subject":"@subject"%';
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

/**
 * Say whether any such email has been sent to the specified address.
 */
function noSuchEmail($index, $email) {
  global $testOnly; if (!$testOnly) return FALSE;
//  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
  $pattern = strtr('%"index":"INDEX","email":"EMAIL"%', u\ray('INDEX EMAIL', $index, $email));
  return !db\exists('r_log', 'info LIKE :pattern', compact('pattern'));
}

/** 
 * See whether the specified email has been sent (by searching the log file)
 */
function findEmail($index, $email, $subs) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testEmails; if (!@$testEmails) return FALSE;

  $subs = @$subs[0] ?: @$subs;
  t\fixEmail($email);
  foreach ($subs as $k => $v) {
    if (strpos($v, ',')) $subs[$k] = $v = '<li>' . join("</li>\n<li>", explode(',', $v)) . "</li>";
    $esubs['{' . $k . '}'] = $v;
  }
  //foreach ($subs as $k => $v) $esubs[u\hug($k, '{}')] = $v;
  $message = @$subs['noFrame'] ? r\emailTemplate($index) : r\emailBody($index);
  $message = str_replace("\r\n", '', $message);
  $messageDpy = t\showSubs($message, $esubs);
  $message = strtr($message, $esubs);
  $subject = @$info['subject'] ?: $GLOBALS['emailSubjects'][$index];
  $subject = $GLOBALS['emailSubjects'][$index];
  $subjectDpy = t\showSubs($subject, $esubs);
  $subject = strtr($subject, $esubs);
  $pattern = '/\{[A-Z]+[A-Z0-9]*\}/i';
  if (preg_match($pattern, $message. $subject)) return FALSE; // no other subs
  ksort($subs);
  
  foreach ($testEmails as $one) {
    extract($one, EXTR_PREFIX_ALL, 't');
    if ($t_index == $index and $t_email == $email) {
      ksort($t_subs);
      $keys = join(' ', array_keys($subs));
      $t_keys = join(' ', array_keys($t_subs));
      if (u\just($keys, $t_subs) == u\just($t_keys, $subs)) return TRUE; // has these subs
    }
  }
  output(u\ray('EXPECT', '') + compact(u\ray('index email subjectDpy subs messageDpy')), 'email');
  return FALSE;
}

/**
 * Highlight the substitutions in a given text.
 * @param string $s: the text
 * @param assoc $subs: the substitutions
 * @return the text with substitutions highlighted
 */
function showSubs($s, $subs) {
  foreach ($subs as $k=>$v) $subs[$k] = "<span style=\"background-color:yellow;\">$v</span>";
  return strtr($s, $subs);
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

function makePicture($photo, $a) {
//  $filename = DRUPAL_ROOT . $a->auxFilename('picture2', TRUE);
//  return (bool) file_put_contents($filename, $picture);
  return $a->update(compact('photo'));
}

// UNUSED
function getPicture($picture) {
  return file_get_contents(DRUPAL_ROOT . R_PICTURE_DIR . "/small/$picture.jpg");
}

function lastMinuteSubs($string) {
  if (u\abbreviates('%ymd', $string)) return subAgo($string, '%Y-%m-%d');
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if (u\abbreviates('%md', $string)) return subAgo($string, '%b %d');
  if (u\abbreviates('%can_', $string)) {
    $perm = u\consta('b', substr($string, 5));
    return htmlspecialchars($GLOBALS['share permissions'][$perm - B_RELATED]);
  }
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
  $agent = r\fullQid($agent);
  if (@$extra['member']) $extra['member'] = r\fullQid($extra['member']);
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
  return $form; // the rest is redundant
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
    r\acct::setDefault($a = r\acct(r\fullQid($id))); // fullQid not uid
    $boxUser = r\Web\boxUser(r\Web\box(), $a->id);
  }
  return TRUE;
}

/**
 * Compare the data to a corresponding record in the table.
 * Return TRUE if the record matches the data.
 * Ignore fields with value "?".
 * Example call:
 *   $info = doRelation($info, TRUE);
 *   return verifyRecord('relations', $info);
 */
function verifyRecord($table, $info) {
///debug(compact('table','info'));
/**/  if (!is_array($info)) die('in verifyRecord: ' . print_r(debug_backtrace(), 1)); // keep
  
  unset($info['data']);
  foreach ($info as $key => $value) {
    if ($value == '?') {unset($info[$key]); continue;}
    $crit[] = $value === '' ? "($key IS NULL or $key='')"
    : ((in_array($key, u\ray(R_DATE_FIELDS)) or stripos($key, 'date') !== FALSE) ? 
      "$key-($key%:DAY_SECS)<=>:$key-(:$key%:DAY_SECS)" 
      : "$key<=>:$key"); // null-safe ==
  }

  $crit = join(' AND ', $crit);
  if (!$result = db\exists($table, $crit, $info)) {
    foreach ($info as $k => $value) $subs[$k] = "'$value'";
    $sql = "SELECT * FROM $table WHERE " . strtr(u\SUBS($crit), u\prefixKeys(':', $subs));
/**/ output("verifyRecord failed. sql=$sql<br>table=$table crit=$crit EXPECTED info=" . print_r($info, 1));
  }
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
  global $formSta, $lastGo;
  $xlastGo = @$lastGo; // notice whether a goto happens, so we can skip post-processing
/// debug(compact(u\ray('id page values confirmed testOnly')));

  $values = @$values[0] ?: array();
  t\fixValues($values);
  if (isset($values['amount']) and !is_numeric($values['amount'])) {
    $a = r\acct(r\fullQid($id));
    list ($values['amount']) = t\parseAmt($values['amount'], $a);
  }
  if (@$values['email']) $values = u\changeKey('email', 'mail', $values);
  
  t\logIn($id);

  $validate = TRUE;
  if (u\abbreviates('history/', $page) and $confirmed and !strpos($page, '&do=')) { // re-examine this
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
  
  $dfts = array();
  foreach ($form as $key => $field) if (substr($key, 0, 1) != '#') {
    if (@$field['#type'] == 'hidden') $dfts[$key] = $field['#value'];
  }
  $sta['values'] = $sta['input'] = $values + $dfts; // add to whatever status the form left off in
  $fieldErr = FALSE; // no field errors yet
  
//  $form_id = "rCredits\\Web\\
//  if ($validate) drupal_validate_form($form_id, $form, $formSta);
  if ($validate and @$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'validate') as $one) $one($form, $sta);
  
  $lastIdentifierTag = '</div identifiers>';
  if ($i = strpos($testConfirmation, $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
  if ($fieldErr) return TRUE; // no submit if field error
  if (@$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'submit') as $one) $one($form, $sta);

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
  global $formArray, $formSta, $lastGo;

  if (!$page) $page = 'summary';
  if ($msg) r\Web\say($msg, $field);
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);
  $xlastGo = @$lastGo; // notice whether a goto happens. ignore all but the deepest (last).

  if (u\abbreviates('/user/', $page)) {
    $login = (strpos($page, 'user/login') !== FALSE);
    $sta = @$formSta ?: array();
    list ($form, $function) = FALSE ? array('user_register_form', 'formRegister')
      : ($login ? array('user_login', 'formLogin') : array());
    $function = 'rCredits\\Web\\' . $function;
    // (FAILS for user_login) $form = \drupal_get_form($form, $args);
    $form = $login ? array() : \drupal_get_form($form, $args);
    $function($form, $sta, $args);
    if ($login and @$form['newPass']) $form['newPass'] = \form_process_password_confirm($form['newPass']);
    $form2 = $form; $rent = \drupal_render($form2);
    if (@$lastGo == $xlastGo) getFormOut($rent);
    return $form;
  }

  $result = \menu_execute_active_handler($page, FALSE);

  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found' 
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\output("page \"$page\": $result in t\\pageForm()");
  } elseif (@$lastGo == $xlastGo) getFormOut($result);
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
//u\deb('postLogout returning TRUE, skipToStep was ='.@$skipToStep.' (now NULL)');
      $skipToStep = NULL; // strangely, this has to be = NULL not unset()
      return TRUE;
    }
  }
  */
//u\deb('postLogout returning FALSE, about to save skiptostep '.$function);
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
  include_once __DIR__ . '/admin/admin.inc';
  global $lastGo, $formOut, $formArray, $formSta, $testOp, $testConfirmation;
  global $sms_devel; $sms_devel = TRUE;
    
  $lastGo = $formOut = $formArray = $formSta = $testOp = $testConfirmation = NULL;
  global $sms_devel; $sms_devel = TRUE;
  global $testSeq; $testSeq = NULL;
  global $channel; $channel = $ch;

  $cgfUid = r\cgfId();
  $tables = 'r_boxes r_do r_gifts r_invites r_log r_notices r_proxies r_relations r_txs r_invoices r_stats r_usd r_user_industries queue';

  //db\q('DELETE FROM users WHERE uid>:cgfUid OR (uid>1 AND uid<:cgfUid-1)', compact('cgfUid'));
  db\q('DELETE FROM users WHERE uid>1 or uid<0');
  a\setupBasicAccounts();

  foreach (u\ray($tables) as $one) db\q("TRUNCATE $one");
  db\q('UPDATE users SET floor=0,minimum=NULL,r=0,usd=0,rewards=0,committed=0');

//  $files = glob(DRUPAL_ROOT . R_PICTURE_DIR . "/*"); // get all picture file names
//  $dir = DRUPAL_ROOT . R_PICTURE_DIR;
//  $zza = '' . r\unQuid('.ZZA');
//  $zza = substr($zza, 0, strlen($zza) - 1); // ignore last digit
//  shell_exec(str_replace('/', '\\', "DEL \"$dir/small/$zza*\""));
  ctty('aaa')->setBit(B_MEMBER);
  ctty('aaa')->setBit(B_OK);

//  t\setupDwAccts();

  t\log('clearing');
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
/**/  u\EXPECT(is_numeric(key($content[0])), 'shown with should be array, not assoc: ' . print_r($content, 1));
  $A = call_user_func_array('array_merge', $content);
  $xi = $i = 0;
  foreach ($A as $one) {
    if ($one === '' or substr($one, 0, 1) == '_') continue; // ignore _labels
    if (strpos($one, '%') !== FALSE) $one = t\lastMinuteSubs(htmlspecialchars($one));
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

function weShow($what, $show = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($what === '') return TRUE;
  return t\shownWith(is_array($what) ? $what : array(array($what)), 'text', $show);
}

/**
 * Check the linked URL content and any Drupal messages (which don't show up in file_get_contents) against expectations.
 */
function hasLinkResults($thing, $info) {
  global $formOut;
  global $testOnly; if (!$testOnly) return FALSE;

  if (!preg_match('/ href="([^"]*)"/', t\that($thing), $matches)) return FALSE;
  
  $s = file_get_contents($matches[1]);
  foreach (u\ray('error status') as $one) $$one = @file_get_contents(t\dosayFilename($one));
  $formOut['text'] = str_replace($was = '#header -->', "$was$error$status", $s);
  return t\shownWith($info, 'text', TRUE);
}

function seq($table = 'r_usd') {
  global $testSeq;
  $id = $testSeq[$table] = @$testSeq[$table] + 1;
  $k = $table == 'r_usd' ? 'txid' : '';
  t\log("seq table=$table id=$id");
  u\EXPECT(!db\exists('r_usd', "$k=:id", compact('id')), "duplicate seq: $id in table $table");
  return $id;
}

function log($msg) {if (t\est()) u\log($msg, 'debug');}
function that($thing) {global $thatThing; return $thatThing[$thing];}
function dosayFilename($type) {return __DIR__ . "/../dosay-$type.txt";}
