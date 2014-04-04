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

function showCardCode($a) {
  $len = strlen(@$a->secure['cardCode']);
  if ($len == 14 or $a->co) return;
/**/ if ($a->ok) debug("$a->fullName\t" . $a->secure['cardCode'] . " (" . $len . ')');
}
