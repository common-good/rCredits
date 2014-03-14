<?php
/**
 * @file
 * rCredits admin
 */
namespace rCredits\Web;

use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;

function seeSecure($a) {
  $secure = $a->secure;
  u\setDft($secure['usdType'], 'Dwolla');
  foreach (u\ray(R_SECURE_FIELDS) as $one) if (isset($secure[$one])) $new[$one] = $secure[$one]; // reorder
  $a->update(u\ray('secure', @$new ?: array()));
/**/ debug($a->secure);    
}

/**
 * Change a transaction amount (only if the transaction happened today and only for a non-rewardable type).
 * @param int $xid: tx record ID
 * @param float $oldAmt: the old amount (for a consistency check)
 * @param float $newAmt: what the amount should be
 */
function changeTxAmt($xid, $oldAmt, $newAmt) {
  if (!$rec = db\lookup('payer,payee,amount,created,goods', 'r_txs', 'xid=:xid', compact('xid'))) return say(t('No such transaction.'), 'err');
  extract($rec);
  if ($amount != $oldAmt) return say(t('The existing amount of that transaction is different.'), 'err');
  if (u\fmtDate($created) != u\fmtDate(REQUEST_TIME)) return say(t('That transaction is not from today.'), 'err');
  if ($goods) return say(t('That transaction has rewards.'), 'err');
  $DBTX = \db_transaction();
  if (!$amount = round($newAmt, 2)) return say(t('Bad new amount.'));
  db\update('r_txs', compact('xid', 'amount'), 'xid');
  $change = $newAmt - $oldAmt;
  if ($payee > 0) db\q('UPDATE users SET r=r+:change WHERE uid=:payee', compact('change', 'payee'));
  if ($payer > 0) db\q('UPDATE users SET r=r-:change WHERE uid=:payer', compact('change', 'payer'));
  say(t('done'));
  unset($DBTX);
}

/**
 * Return a display of steps done and steps remaining (to open an account).
 */
function showSteps($a) {
  $steps = '';
  if (!$a->ok) foreach ($a->stepsDone as $k => $done) {
    $color = $done ? 'silver' : 'red';
    if ($a->co) {
      if ($k == 'proxies') $color = 'white';
    } elseif ($k == 'company' or $k == 'relations') $color = 'white';
    $steps .= "<b style=\"color:$color;\">$k </b>";
  }
  if (!$a->ok) $steps .= ' [Next step: <b style="color:red;">' . ($a->nextStep() ?: 'NONE!') . '</b>] ';
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
  $uid = $a->id;
  $ret = '';
  $sql = <<<EOF
    SELECT IF(r.main=:uid, other, main) AS them, main, IF(r.main=:uid, u2.fullName, u1.fullName) as who, 
      permission, employeeOk, employerOk, isOwner, draw,
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
        : (($permission >= B_SCAN or $employerOk) ? t('employs')
        : ($employeeOk ? t('employs')
        : ($permission >= B_READ ? t('readable by')
        : t('? by'))))));
    } else {
      $role = $isOwner ? t('owns')
        : ($draw ? t('draws from')
        : ($permission >= B_MANAGE ? t('manages')
        : (($permission >= B_SCAN or $employerOk) ? t('works for')
        : ($employeeOk ? t('works for')
        : ($permission >= B_READ ? t('can read')
        : t('can ?'))))));
    }
    $ret .= ($ret ? ', ' : '') . "$role <a href=\"$them\">$who</a>";
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
  $cgfUid = r\cgfId();
  $cgfLen = strlen(r\acct($cgfUid)->id);
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
  $a->setBit(B_u5, FALSE);
  $stepsDone = array_fill_keys(u\ray(R_STEPS), FALSE);
  $a->update(u\ray('data', compact('stepsDone') + $a->data));
  if ($a->signed) $a->stepDone('sign');
  if ($a->postalAddr) $a->stepDone('contact');
  if (db\exists('r_gifts', 'uid=:uid', u\ray('uid', $a->id))) $a->stepDone('donate');
  if (db\exists('r_proxies', 'person=:uid', u\ray('uid', $a->id))) $a->stepDone('proxies');
  if (file_exists(DRUPAL_ROOT . $a->auxFilename('picture', TRUE))) $a->stepDone('photo');
  if (!is_null($a->minimum)) $a->stepDone('prefs');
  if (@$a->secure['bankAccount']) $a->stepDone('connect');
  if ($a->can(B_BANK)) $a->stepDone('verify');
  if ($a->selling) $a->stepDone('company');
  if ($a->co and $a->member) $a->stepDone('relations');
  
}

function fixdw($a) {
  if ($a->ok and $a->can(B_DW)) {$a->stepDone('dw'); $a->stepDone('connect');}
}

function pioneerBonus($a) {
  if (!$a->ok or !$a->can(B_BONA) or $a->co) return;
  if (db\exists('r_txs', "payee=:uid AND payer=-8915 AND amount=100", u\ray('uid', $a->id))) return;
/**/debug("Pioneer bonus to $a->fullName");
  be\fund($a->id, TX_SIGNUP, 100, 'pioneer bonus');
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
