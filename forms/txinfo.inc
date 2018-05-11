<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show information about a specific transaction and its risks.
 */
function formTxInfo($form, &$sta, $xid = '') {
  include R_ROOT . '/rweb/risk-descs.inc';
  global $base_url;
  $x = r\x($xid);
  extract($x->ray);
  extract(just('xfee force undoes undoneBy', $x->data));
  
  $title = item(t('Transaction #') . $xid);
  $when = item(u\fmtDate($created) . t(' at ') . u\fmtTime($created) . (@$force ? t(' (created offline)') : ''), t('Date/Time:'));
  $amountDesc = item(u\fmtAmt($amount), t('Amount:'));
  $from = item(txInfoParty($payer, $payerAgent, $payerTid), t('From:'));
  $to = item(txInfoParty($payee, $payeeAgent, $payeeTid), t('To:'));
  $same = ($payerFor == $payeeFor);
  $goodsDesc = ' (' . ray(R_WHYS)[$goods] . ')';
  $for1 = item($payerFor . $goodsDesc, $same ? t('For:') : t('Payer Desc:'));
  if (!$same) $for2 = item($payeeFor, t('Payee Desc:'));
  $by = item($x->taking ? t('by payee') : t('by payer'), t('Initiated:'));
  $channels = ray(TX_CHANNELS);
  $where = item($channels[$channel] . t(' (Device #box)', compact('box')), t('Method:'));
  // related transactions (xfer/undoes/undoneBy
  if (@$undoes) $rel[] = t('undoes tx#') . txLink($undoes);
  if (@$undoneBy) $rel[] = t('undone by tx#') . txLink($undoneBy);
  if (@$xfee) $rel[] = u\fmtAmt($xfee) . ' xfee';

  if (@$rel) $related = item(join(', ', $rel), t('Related Txs:'));
  $riskDesc = $risk < K_RED ? t('low') : ($risk < 2 * K_RED ? t('moderate') : t('high'));
  $risks = r\riskList($risks0 = $risks, K_TX_RISKS); 
  $risk = item(round($risk) . button(t('Recalculate'), "$base_url/rerisk/tx/$xid/1") . " ($riskDesc): $risks", t('Risk:'));
  $adminOk = boolFld(t('Risk Ok?'), '', $risks0 & 1);
  $submit = submit();
  return cgform(compact(ray('title when amountDesc from to for1 for2 by where risk $adminOk related submit')));
}

function txInfoParty($uid, $agent, $tid) {
  $desc = r\acct($uid)->linkedDesc();
  $by = $agent == $uid ? t('pro se') : (t('by ') . r\acct($agent)->linkedDesc());
  return "$desc $by (#$tid)";
}
  
//function txLink($xid) {global $base_url; return "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";}