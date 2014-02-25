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
  $mya = r\acct();
  $admin = $mya->admin(); // as distinct from merely ctty_admin
  
  extract(u\just('account make', $sta['input']));
  $op = r\Web\op($sta);

  if ($op == 'start' or $op == 'stop') {
    \variable_set('up', $op == 'start');
    return r\up() ? say('System is UP.') : say('System is DOWN.', 'ERR');
  }

  if (@$account) {
    $a = a($account); // should use whois first
    if (!$admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  }
  
  if ($op == 'printId') {
    if (@$make) {
      seeSecure($a);
      r\makeCardCode($a);
      seeSecure($a);
    }
    return r\go('print-rcard/' . $a->qid());
  } elseif ($op == 'seeSecure') {
    if (@$a) {
/**/  debug(u\ray('legalName phone federalId physicalAddr', @$a->legalName, substr(@$a->phone, 2), @$a->federalId, @$a->address . ' ' . $a->postalCode));
      seeSecure($a);
    }
  } elseif ($op == 'connectBank') {
    $secure = $a->secure;
    if (!$bankAccount = @$secure['bankAccount']) return say(t('Member has not yet entered bank info.'), 'zot');
    if ($a->country == US_COUNTRY_ID) {
      $routingNumber = substr(@$bankAccount, 4, 9); // chop of USkk
      $bankAccount = substr(@$bankAccount, 4 + 9); // everything after the routing number
    }
    if (!@$secure['auth']) return say(t('You must first set up the Dwolla account for this member.'), 'zot');
    $us = new r\usd($a);
    if (!$us->addBank($bankAccount, @$routingNumber, $secure['bankAccountType'], $err)) return say($err, 'zot');
    say('info saved');
  } elseif ($op == 'do1099b') {
    include __DIR__ . '/../rcredits/rcredits-yearly.inc';
    r\Special\do1099bs();
  } elseif ($op == 'makeCtty') {
    return r\go('make-community');
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, MID(phone,3) AS phone, flags, r, usd, rewards, minimum AS min, committed AS comit, share, floor, address, city, postalCode AS zip';
    return say(showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,agent, REPLACE(special, ',', ', ') AS special, REPLACE(info, ',', ', ') AS info";

  if ($op == 'log') return say(showQuery('r_log', 1, 'logid DESC', $logFields));
  if ($op == 'tells') return say(showQuery('r_log', "info LIKE '%tell-staff%' ORDER BY logid DESC LIMIT 50", 'logid DESC', $logFields));

  if ($op == 'txs') return say(showQuery('r_txs', isDEV ? 1 : 'type NOT IN (:TX_REBATE,:TX_BONUS)', 'xid DESC', 'xid, t.created, type AS ty, t.state AS st, goods AS gd, payer, payee, amount, payerFor AS for1, payeeFor AS for2, payerTid AS tid1, payeeTid AS tid2', array('payer', 'payee')));

  if ($op == 'gifts') return say(showQuery('r_gifts', "honor<>'share'", 'donid DESC', 'donid,giftDate,t.uid,amount,often,honor,honored,t.share,completed', 'uid'));
  if ($op == 'invites') return say(showQuery('r_invites', 1, 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(showQuery('r_notices', 1, 'msgid DESC'));
  if ($op == 'proxies') return say(showQuery('r_proxies', 1, 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(showQuery('r_relations', 1, 'main, reid', 'reid,main,other,draw,permission,employerOk,employeeOk,isOwner', 'main'));
  foreach (u\ray('boxes do stats usd') as $one) if ($op == $one) return say(showQuery("r_$one"));
}

    /*
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
    return; */


function seeSecure($a) {
  $secure = $a->secure;
  u\setDft($secure['usdType'], 'Dwolla');
  foreach (u\ray(R_SECURE_FIELDS) as $one) if (isset($secure[$one])) $new[$one] = $secure[$one]; // reorder
  $a->update(u\ray('secure', @$new ?: array()));
/**/ debug($a->secure);    
}

/**
 * List the USD account status of each account.
 *//*
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
  return showRecords($lines);
}*/

function showRecords($ray) {
  $cgfUid = \variable_get('cgf_uid');
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $key => $value) {
      if ($value and in_array($key, u\ray('id uid t.uid myid payer payee main other inviter invitee proxy person owner defaultAgent'))) $value = r\acct($value)->name; // don't include agent/payerAgent/etc. here
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
  if (!$admin = $mya->admin()) {
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
OR changeIt(a('aaa'));
}

 */
function changeIt($a) {
  if (!$a->ok() or !$a->can(B_BONA) or $a->co()) return;
  if (db\exists('r_txs', "payee=:uid AND payer=-8915 AND amount=100", u\ray('uid', $a->id))) return;
/**/debug("Pioneer bonus to $a->fullName");
  be\fund($a->id, TX_SIGNUP, 100, 'pioneer bonus');

  /*
  $q = db\q('SELECT xid, serial, data, payer, payee, payerTid, payeeTid, payerFor, payeeFor, (payee=:uid) AS tome FROM r_txs WHERE :uid IN (payer, payee) ORDER BY xid', u\ray('uid', $a->id));
  $tid = 0;
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($serial == @$serialx) { // rebate / bonus
      $fornm = $tome ? 'payeeFor' : 'payerFor';
      $$fornm = str_replace("#$oldtid", "#$tid", $$fornm);
      db\update('r_txs', compact('xid', $fornm), 'xid');
    } else { // main tx
      $tidnm = $tome ? 'payeeTid' : 'payerTid';
      $oldtid = $$tidnm;
      $$tidnm = $tid = $tid + 1;
      db\update('r_txs', compact('xid', $tidnm), 'xid');
      $serialx = $serial;
    }
  }
  */    
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

function say($index, $args = array(), $errField = '') {return r\Web\say($index, $args, $errField);}