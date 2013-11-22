<?php
/**
 * @file
 * rCredits admin
 */

use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;

/**
 * Handle results submitted on formAdmin.
 */
function adminForm($sta) {
  extract(u\just('account', $sta['input']));
  $op = r\Web\op($sta);
  if ($op == 'USD_Accounts') return r\Web\say(usdAccts());
  if ($op == 'suUsd') return suUsd();
  if (@$account) $a = aa($account); // should use whois first
  
  if ($op == 'printId') {
    $qid = $a->qid();
    $agent = $a->agent;
    $field = strpos($qid, R_AGENT_MARK) ? 'cardCode2' : 'cardCode';
    $secure = $agent->secure;
    if ($field == 'cardCode2') if (isDEV or !($code = @$secure[$field]) or strlen($code) > R_CARDCODE_LEN) {
      if (!isDEV) u\EXPECT($field == 'cardCode2', 'missing cardCode'); // only cardCode2 gets created here
      $secure = u\ray($field, r\cardCode($qid)) + $secure; // create an rCard security code
      $agent->update(compact('secure')); // resave the secure fields
    }
    return r\go('print-rcard/' . $qid);
  } elseif ($op == 'seeUsd') {
    if (@$a->secure['auth'] and !@$a->secure['usdEmail']) {
      $old = $a->secure;
      $usdPass = user_password(20) . '*B2'; // satisfy Dwolla (and our own security)
      $usdEmail = strtolower(\user_password(4) . '.' . $a->mainQid) . '@rc4.me';
      $usdPin = u\randomString(4, 'digits');
      $usdPhone = $a->phone;
      $secure = compact(u\ray('usdPass usdEmail usdPin usdPhone')) + $old;
      if (isset($secure['old'])) $secure['old'] = '';
      if (isset($secure['card_code'])) $secure['card_code'] = '';
      $a->update(compact('secure'));
    }
    return;
  }
  
  // handle all other buttons with showTableQ()
  $where = "uid>1";
  $userSql = "SELECT uid, MID(phone,3) AS phone, flags, r, usd, rewards, minimum AS min, committed AS comit, share, floor, address, city, postalCode AS zip FROM users WHERE $where ORDER BY name";
  if ($op == 'users') return r\Web\say(showTableQ($userSql));
  $userSql = str_replace($where, "postalCode LIKE '013%'", $userSql);

  $logQuery = "SELECT channel,type,myid,agent, REPLACE(special, ',', ', ') AS special, REPLACE(info, ',', ', ') AS info FROM r_log";
  if ($op == 'log') return r\Web\say(showTableQ($logQuery));
  if ($op == 'tells') return r\Web\say(showTableQ("$logQuery WHERE info LIKE '%tell-staff%' ORDER BY logid DESC LIMIT 50"));
  if ($op == 'Gfld') return r\Web\say(showTableQ($userSql));
  if ($op == 'txs') return r\Web\say(showTableQ('SELECT xid, type, state, goods, payer, payee, amount, r, payerFor, payeeFor, payerTid, payeeTid FROM r_txs'));

  foreach (u\ray(R_SHOWABLE_TABLES) as $one) if ($op == $one) return r\Web\say(showTableQ("r_$one"));
//  list ($community) = explode('-', $community, 2); // get the absolute value of community
//  if ($community) svar('myid', -$community); // make it the new current account
}

/**
 * List the USD account status of each account.
 */
function usdAccts($where = "postalCode LIKE '013%'") {
  if (!r\acct()->admin()) return;
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
    
    $ok = $a->ok() ? 'ok' : '';
//    $charge = $a->can(B_CHARGE) ? 'Chg' : '';
    $company = $a->co() ? 'Co' : '';
    $line = array($a->fullName, $hasDwolla, $type, $verified, $ok, $company, $bal);
    $lines[] = array_combine($head, $line);
  }
  return showTable($lines);
}

function showTable($ray) {
  $cgfUid = \variable_get('cgf_uid');
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $key => $value) {
      if ($value and in_array($key, u\ray('id uid payer payee main other inviter invitee proxy person owner defaultAgent'))) $value = $value < 0 ? 'ctty' : ($value == $cgfUid ? 'CGF' : r\acct($value)->fullName);
      if ($key == 'reid') $value = u\n2a($value, -3);
      if ($value and in_array($key, u\ray(R_DATE_FIELDS))) $value = u\fmtDate($value);
      if ($key == 'flags') $value = "<div style='text-transform:capitalize; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
      if ($key == 'payeeFor' and $value == $row->payerFor) $value = 'same';
      $line .= "<td>$value</td>";
    }
    $lines[] = "<tr>$line</tr>";
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>", $lines) . "</tr>\n</table>";
}

function showTableQ($sql, $subs = array()) {
  if (!strpos($sql, ' ')) {$sql = "SELECT * FROM $sql " . ($subs ?: ''); $subs = array();}
  if (!$result = db\q($sql, $subs)) return \drupal_set_message('bad query');
  return showTable($result->fetchAll());
}

/**
 * Utility to do something to each existing transaction
 include __DIR__ . '/../rcredits/admin.inc';
 eachTx();
 */
function eachTx($func = 'changeIt') {
  $result = db\q('SELECT * FROM r_txs');
  if (!function_exists($func)) $func = 'rCredits\\' . $func;
  while ($row = $result->fetchAssoc()) $func($row);
}

/**
 * Use this to cobble together a new account, pending the new Dwolla reg API working
 */
function cobble($id, $federalId, $dob, $pin, $question = '', $answer = '') {
  if (!is_numeric($id) and strlen($id) == 3) $id = 'NEW.' . $id;
  if (!$mya = r\acct($id)) return print_r('bad id');
  $secure = compact(u\ray('usdType pin')) + (@$mya->secure ?: array()); // secure includes more than these

  $federalId = str_replace('-', '', $federalId);
  if (!is_numeric($dob)) $dob = strtotime($dob);
  $usdType = 'Dwolla';
  $info = compact(u\ray('federalId dob question answer'));
  $mya->update($info + compact('secure'));
  print_r(compact('id', 'pin') + $info);
  $us = new r\usd($mya);
  $us->getAuth();
}

/**
 * Utility to do something to each member account (see example below)
 include __DIR__ . '/../rcredits/admin.inc';
 eachAcct();
 */
function eachAcct($func = 'changeIt') {
  $result = db\q('SELECT uid FROM users WHERE uid>1')->fetchCol();
  if (!function_exists($func)) $func = 'rCredits\\' . $func;
  foreach ($result as $one) $func(r\acct($one));
}

/**
 * General purpose utility, usually called from eachAcct, as above.
 * For example:
 *    changeIt($a):
 *    $us = new r\usd($a);
 *    $us->each('rCredits\\Cron\\bankFollowup', 0, u\ray('deposit withdrawal'));
 
  include __DIR__ . '/../rcredits/admin.inc';
foreach (array(3201932, 3244519, 3350610, 3510402) as $one) {
  changeIt($one);
}

 */
function changeIt($a) {
  $postalAddr = $a->address . ', ' . $a->city . ', ' . r\realState($a->state) . ' ' . $a->postalCode;
  list ($address) = explode(',', $a->postalAddr);
  if (!$address) $address = $a->address;
  $a->update(compact('address', 'postalAddr'));

//$payee = db\lookup('payee', 'r_usd', 'txid=:txid', compact('txid'));
//r\Cron\coverFee(compact('txid', 'payee'));
}

function suUsd() {
  return; // unused
  $testUsdMin = DW_FEE_THRESHOLD;
  $a = r\acct('NEW.AAA');
  $us = new r\usd($a);
  if (isDEV) {
    t\clear(TX_WEB); // make sure the usdAccount is not used by anyone else
    $a->update(u\ray('usdAccount', DW_TESTER_ACCT));
    $us->getAuth();
/**/  die('should not return to suUsd');
  }
  if (isPRODUCTION and $us->bal() < $testUsdMin) {
    $cgf = r\acct('NEW.AAB');
    $for = 'testing';
    be\transfer('payment', $a, $cgf, $testUsdMin, $for);
    be\transfer('payment', $cgf, $a, $testUsdMin, $for); // CGF sends back the whole amount in usd
/**/  debug("$$testUsdMin traded with CGF");
  }
}

/*
https://rcredits.org:2083/cpsess4564554376/frontend/x3/mail/addfwd.html

<form method="post" name="fwdf" id="fwdf" action="doaddfwd.html">
<input id="email" type="text" name="email" size="25" value="new" />
<input name="domain" value="new.rcredits.org">
<input name="fwdopt" id="fwd_radio" type="radio" value="fwd" checked="checked" />
<input name="fwdemail" id="fwdemail"  type="text" size="40" value="new@rcredits.org" />
<input class="input-button" type="submit" id="submit" value="Add Forwarder" />
</form>

list ($txid, $amount, $payer, $payee, $created) = array(3198706, 1.81, 26742000000001, 26742000000018, strtotime('6/12/2013'));
$list = array(compact(rCredits\Util\ray('txid amount payer payee created')));
$a = rCredits\acct('NEW.AAA');
$us = new rCredits\usd($a);
$us->rollback($list);

*/

