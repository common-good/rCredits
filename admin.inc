<?php
/**
 * @file
 * rCredits admin
 */

use rCredits as r;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;

/**
 * List the USD account status of each account.
 */
function usdAccts($where = "postalCode LIKE '013%'") {
  if (!r\acct()->can(B_ADMIN)) return;
  $result = r\dbQ("SELECT uid FROM users WHERE $where ORDER BY fullName");
  $head = u\ray('Name Dwolla? Type Verified OK Chg Co Bal');
  while ($row = $result->fetch()) {
    $usA = new r\usd($a = r\acct($row->uid));
    $bal = $usA->bal($err);
    $hasDwolla = $err ? '-' : 'Yes';
    $source = $usA->source($err);
    $type = @$source['Type'];
    $verified = @$source['Verified'] ? 'Yes' : '-';
    $count = @$source['count'];
    
    $ok = $a->can(B_OK) ? 'ok' : '';
    $charge = $a->can(B_CHARGE) ? 'Chg' : '';
    $company = $a->can(B_COMPANY) ? 'Co' : '';
    $line = array($a->fullName, $hasDwolla, $type, $verified, $ok, $charge, $company, $bal);
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
      if ($value and in_array($key, u\ray(R_DATE_FIELDS))) $value = u\formatDate($value);
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
  if (!$result = r\dbQ($sql, $subs)) return \drupal_set_message('bad query');
  return showTable($result->fetchAll());
}

/**
 * Utility to do something to each existing transaction
 include __DIR__ . '/../rcredits/admin.inc';
 eachTx();
 */
function eachTx($callBack = 'changeIt') {
  $result = r\dbQ('SELECT * FROM r_txs')->fetchRow();
  if (!function_exists($callBack)) $callBack = 'rCredits\\' . $callBack;
  foreach ($result as $one) $callBack($one);
}

/**
 * Utility to do something to each member account (see example below)
 include __DIR__ . '/../rcredits/admin.inc';
 eachAcct();
 */
function eachAcct($callBack = 'changeIt') {
  $result = r\dbQ('SELECT uid FROM users WHERE uid>1')->fetchCol();
  if (!function_exists($callBack)) $callBack = 'rCredits\\' . $callBack;
  foreach ($result as $one) $callBack(r\acct($one));
}

/**
 * General purpose utility, usually called from eachAcct, as above.
 * For example:
 *    $us = new r\usd($a);
 *    $us->each('rCredits\\Cron\\bankFollowup', 0, u\ray('deposit withdrawal'));
 */
function changeIt($a) {
  $us = new r\usd($a);
  $us->each('rCredits\\Cron\\bankFollowup', 0, u\ray('deposit withdrawal'));
}

function suUsd() {
  $testUsdMin = DW_FEE_THRESHOLD;
  $a = r\acct('NEW.AAA');
  $us = new r\usd($a);
  if (isDEV) {
    t\clear(TX_WEB); // make sure the usdAccount is not used by anyone else
    $a->update(u\ray('usdAccount', DW_TESTER_ACCT));
    $us->getAuth();
/*/*/  die('should not return to suUsd');
  }
  if (isPRODUCTION and $us->bal() < $testUsdMin) {
    $cgf = r\acct('NEW.AAB');
    $for = 'testing';
    be\transfer('payment', $a, $cgf, $testUsdMin, $for);
    be\transfer('payment', $cgf, $a, $testUsdMin, $for); // CGF sends back the whole amount in usd
/*/*/  debug("$$testUsdMin traded with CGF");
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

