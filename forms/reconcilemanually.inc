<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Upload a bank statement (or two) to reconcile the community fund.
 * Handle two files, in case the Community Fund is split into INCOMING and OUTGOING
 */
function formReconcileManually($form, &$sta) {
  $title = t('Reconcile Community Fund Discrepancies');
	$subtext = t('<p>Here is a list of all the transactions downloaded from the bank that have no clear corresponding transfer of dollars to or from the Community Fund.</p><ol><li>Select one or more downloaded (OFX) transactions (as few as possible).</li><li>Then select one or more corresponding batches, some of whose individual transfers add up to that same amount.</li><li>If the bank made a mistake and you need to compensate, put an amount in the "Create offset tx" field as well. This amount will be added to our record of how much is in the Community Fund bank account(s).</li><li>Click "Match".</li></ol>');
  
  if (!$txs = getv('ofxTxs')) w\say(t('There are no unmatched downloaded (OFX) transactions.'), 'err');
  if (!$batches = getv('batches')) w\say(t('There are no unmatched bank transfer batches.'), 'err');
  
  foreach ($txs as $t => $tx) {
    extract(just(OFX_TX_FLDS, $tx)); // trnType dtPosted trnAmt fitId name memo
    $dtPosted = u\fmtDate(strtotime($dtPosted), TRUE);
    $amt = ' ' . u\fmtAmt($trnAmt, ''); // negative numbers disappear without the leading space (bug?)
    $id = 'tx-' . $t;
    $fitId = strpos($fitId, ESCROW_IN) !== FALSE ? t('IN') : t('OUT');
    $trnType = strtolower($trnType);
    $form[$id] = boxFld($id, $amt, tr('ofx %dtPosted: <b>%fitId</b> (%trnType) %name - %memo', compact(ray(OFX_TX_FLDS))), 0, clas('ofxTx'));
  }
  
  foreach ($batches as $batch => $batAmt) {
		$batDt = substr($batch, 1) + 0; // unix date of batch
    $txidSign = $batch[0] == 'B' ? '<0' : '>0';
    $amtSign = $batAmt < 0 ? '<0' : '>0';
    $list = db\lst('amount', 'r_usd', "deposit=:batDt AND bankTxId=0 AND txid$txidSign AND amount$amtSign", compact('batDt'));
    $list = str_replace('.00', '', $list); // visually confusing otherwise
    $id = 'bt-' . $batch;
    $batDt = u\fmtDate($batDt, TRUE);
    $type = $batch[0] == 'B' ? t('BAD') : ($batAmt < 0 ? t('OUT') : t('IN'));
    $form[$id] = boxFld($id, u\fmtAmt($batAmt), "<b>$type</b> $batDt: $list", 0);
  }
  $form['offset'] = textFld(t('Create offset tx:'), [t('Amount')]);
  $form['memo'] = textFld(t('Memo:'), [t('Description'), t('A positive amount here reflects a positive transaction reported by the bank, that has no bearing on any %PROJECT account.')]);
  $form['submit'] = t('Match');

  return cgform(compact(ray('title subtext')) + $form);
}

function formReconcileManually_validate($form, &$sta) {
  extract(just('txs bts offset memo', getParams($sta)));
  if ($offset) {
    if ($err = u\badAmount($offset)) return say($err, 'offset');
    if (!$memo) return say(t('You must say what the offset transaction is about.'), 'memo');
  }
  if (!$txs) return say(t('You must select at least one OFX transaction.'), 'err');
  if (!$bts and !$offset) return say(t('You must select at least one batch transaction and/or create an offset.'), 'err');
  
  $txTot = $btTot = 0;
  foreach ($txs as $tx) $txTot += $tx['trnAmt'];
  foreach ($bts as $batch => $batAmt) $btTot += $batAmt;
  if ($txTot != $btTot + $offset) return say(t('OFX total must equal batch total (plus offset).'), 'err');
  u\preray(compact('offset'), $sta['input']);
}

function formReconcileManually_submit($form, &$sta) {
  extract(just('txs bts offset memo txs0 bts0', getParams($sta)));
  foreach ($txs as $t => $tx) {
    unset($txs0[$t]);
    $bankTxId = bankTxId($tx['fitId']); // get most recent transaction ID (somewhat arbitrarily)
    $dt = strtotime(substr($tx['fitId'], -15, 8)); // and most recent transaction date
  }

  $DBTX = \db_transaction();
  foreach ($bts as $batch => $batAmt) {
    unset($bts0[$batch]);
		$batDt = substr($batch, 1) + 0; // unix date of batch
    $txidSign = $batch[0] == 'B' ? '<0' : '>0';
    $amtSign = $batAmt < 0 ? '<0' : '>0';
    $sql = "UPDATE r_usd SET bankTxId=:bankTxId WHERE deposit=:batDt AND bankTxId=0 AND txid$txidSign AND amount$amtSign";
    db\q($sql, compact('bankTxId', 'batDt'));
/**/    debug(tr(str_replace(':', '%', $sql), compact('bankTxId', 'batDt')));
  }
  if ($offset) db\insert('r_usd2', ray('amount completed bankTxId type memo', $offset, $dt, $bankTxId, 'S', $memo));

  setv('ofxTxs', $txs0);
  setv('batches', $bts0);
  unset($DBTX);
}

function getParams($sta) {
  extract(just('offset memo', $sta['input']));
  $txs0 = getv('ofxTxs');
  $bts0 = getv('batches');
  
  $txs = $bts = [];
  foreach (array_keys($sta['input']) as $k0) {
    $k = substr($k0, 3);
    if (u\starts($k0, 'tx-')) $txs[$k] = $txs0[$k];
    if (u\starts($k0, 'bt-')) $bts[$k] = $bts0[$k];
  }
  return compact(ray('txs bts offset memo txs0 bts0'));
}
