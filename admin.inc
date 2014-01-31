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
  extract(u\just('account make', $sta['input']));
  $op = r\Web\op($sta);

  if ($op == 'start' or $op == 'stop') {
    \variable_set('up', $op == 'start');
    return r\up() ? r\Web\say('System is UP.') : r\Web\say('System is DOWN.', 'ERR');
  }

  if ($op == 'USD_Accounts') return r\Web\say(usdAccts());
  if (@$account) $a = a($account); // should use whois first
  
  if ($op == 'printId') {
    if (@$make) {
      seeSecure($a);
      r\makeCardCode($a);
      seeSecure($a);
    }
    return r\go('print-rcard/' . $a->qid());
  } elseif ($op == 'seeSecure') {
    if (@$a) seeSecure($a);
  } elseif ($op == 'newUsdEmail') {
    seeSecure($a);
    if (!@$a->secure['auth'] or !$make) return;
    $usdPass = user_password(20) . '*B2'; // satisfy Dwolla (and our own security)
    $usdEmail = strtolower(\user_password(4) . '.' . $a->mainQid) . '@rc4.me';
    $usdPin = u\randomString(4, 'digits');
    $usdPhone = $a->phone;
    $secure = compact('usdEmail') + $a->secure + compact(u\ray('usdPass usdPin usdPhone')); // preempt only e
    $a->update(compact('secure'));
    seeSecure($a);
    return;
  } elseif ($op = 'do1099b') {
    include __DIR__ . '/../rcredits/rcredits-yearly.inc';
    r\Special\do1099bs();
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

function seeSecure($a) {
  $secure = $a->secure;
  u\setDft($secure['usdType'], 'Dwolla');
  foreach (u\ray(R_SECURE_FIELDS) as $one) if (isset($secure[$one])) $new[$one] = $secure[$one]; // reorder
  $a->update(u\ray('secure', @$new ?: array()));
/**/ debug($a->secure);    
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
//      if ($key == 'flags') $value = "<div style='text-transform:capitalize; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
      if ($key == 'flags') $value = "<div style='text-transform:none; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
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
/**/ if (!$mya = r\acct($id)) return print_r('bad id');
  $secure = compact(u\ray('usdType pin')) + (@$mya->secure ?: array()); // secure includes more than these

  $federalId = str_replace('-', '', $federalId);
  if (!is_numeric($dob)) $dob = strtotime($dob);
  $usdType = 'Dwolla';
  $info = compact(u\ray('federalId dob question answer'));
  $mya->update($info + compact('secure'));
/**/  print_r(compact('id', 'pin') + $info);
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
  $data = $a->data;
  $signupCo = u\just($k1 = 'company companyPhone EmployeeOk isOwner contractor', $data);
  $stats = u\just($k2 = 'giftsEver giftsPastMo beneEver benePastMo extraEver extraPastMo bankedEver bankedPastMo avgBalPastMo avgBalEver', $data);
  foreach (u\ray("$k1 $k2") as $one) unset($data[$one]);
  $data += compact('signupCo', 'stats');
  if ($a->co()) {
    $coflds = db\lookup('website,selling,coFlags,description', 'r_companies', 'uid=:uid', u\ray('uid', $a->id));
    $data += $coflds;
  }
  $data['legalName'] = $a->fullName;
  $a->update(compact('data'));
/**/debug($a->data);
}

/**
 * Re-setup the test cards on the STAGING server, after importing data from the PRODUCTION server.
include __DIR__ . '/../rcredits/admin.inc';
makeTestCards();
 */
function makeTestCards() {
  if (isPRODUCTION) return;

  $a = a('aab');
  $selling = "groceries\ngifts\nsundries\ndeli\nbaked goods";
  $a->update(u\ray('fullName selling', 'Corner Store', $selling));
  $a->setCoBit(CO_REQUIRE_CASHIER, FALSE);

  $a = a('aaa');
  $a->update(u\ray('fullName', 'Bob Bossman'));
  $a->upSecure('cardCode2', 'WeHlioM5JZv1O9G');
  
  $a = a('aad');
  $a->update(u\ray('fullName', 'Cathy Cashier'));
  $a->upSecure('cardCode2', 'ME04nW44DHzxVDg');
  
  $a = a('aaq');
  $a->update(u\ray('fullName', 'Helga\'s Hardware'));
  $a->setCoBit(CO_REQUIRE_CASHIER, TRUE);
  
  $a = a('aak');
  $a->update(u\ray('fullName', 'Curt Customer'));
  $a->upSecure('cardCode', 'NyCBBlUF1qWNZ2k');
  $a->upSecure('cardCode2', 'utbYceW3KLLCcaw');
  
  $a = a('abb');
  $a->update(u\ray('fullName', 'Susan Shopper'));
  $a->upSecure('cardCode', 'ZzhWMCq0zcBowqw');
}

/**
 * Reimburse the Dwolla fee (if any) of a recent transaction.
 * @param assoc $item:
 *   txid: fee id (payee's txid is two less, payer's is one less)
 *   payee: uid of recipient
 */ 
function cover($txid, $payee) {
  $usA = new r\usd($a = r\acct($payee));
  $txid -= 2; // get the payee's txid
  if (!$fee = $usA->fee($txid)) return;
  if ($fee == '?') {
/// debug('no Dwolla for coverFee');
  }
  $for = t('Dwolla fee');
  $keys = 'payer payee type amount for taking';
  
  r\usd::beginAtom();
  r\transact(u\ray($keys, $a->community, $payee, TX_REFUND, $fee, $for, 0));
  r\usd::addToAtom($txid + 2, $fee, $payee, -1, REQUEST_TIME); // -1 to distinguish from bank transfers
  $a->actualUsd(-$fee);
  r\usd::commit();
}
