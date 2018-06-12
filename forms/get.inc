<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Exchange USD for rCredits or vice-versa (the Bank tab)
 */
function formGet($form, &$sta, $args = '') {
  global $mya;
  global $base_url;
  
  if (!$mya->hasBank) return w\go('/settings/connect');
  
  \drupal_page_is_cacheable(false); // suppress caching, so buttons work right
  $myid = $mya->id;
  $jid = $mya->jid ?: 0;
  extract(just('cancel', $args));
  if ($txid = @$cancel) {
    db\del('r_usd', $txid);
    say('transfer canceled', compact('txid'));
    r\tellAdmin("canceled exchange #$txid");
  }
  $title = t('Exchange US Dollars for %RCREDITS');
  $subtext = t('<p>Exchange US Dollars from your connected bank account in exchange for %RCREDITS. Or vice versa. It may take several business days for your bank to send the funds. Funds going TO your bank account generally arrive within one business day.</p><p>To refill your %PROJECT Account automatically (or not), as needed, adjust your "refills" setting on the <%a>Banking</a> settings page.</p>', '_a', atag('/settings/connect')) . t('note|ach warning');

//  $minimum = textFld(t('Minimum:'), t('prompt min'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->minimum : R_MINR_DEFAULT, 's')));
//  $toSpend = u\fmtAmt($mya->avail(FOR_GOODS));
//  $toSpend = u\fmtAmt($mya->r - $mya->committed);
  $balance = u\fmtAmt($mya->balDpy);
  $credit = u\fmtAmt(-$mya->floor);
  $waiting0 = $mya->waitingFromBank();

  if (!db\exists('r_usd', 'payee IN (:myid,:jid) AND amount<0 AND created>:dayAgo', ray('myid jid dayAgo', $myid, $jid, strtotime('-1 day')))) { // limit to one withdrawal request per 24 hours
    $avail = $mya->avail(FOR_USD); // drawing from connected accounts not allowed
    foreach (ray(R_EXCHANGE_LIMITS) as $days => $limit) {
      $sofar = $days == 1 ? 0 : db\get('-SUM(amount)', 'r_usd', 'payee=:myid AND amount<0 AND created>:date', ray('myid date', $myid, strtotime("-$days days")));
      $avail = min($avail, $limit - $sofar);
    }
  } else $avail = 0;

  $cashoutable = u\fmtAmt(max(0, min($mya->balDpy, $avail)));
  if ($waiting = u\fmtAmt(abs($waiting0))) {
    $where = 'payee IN (:myid, :jid) AND completed=0 AND deposit=0 ORDER BY txid DESC';
    if ($rec = db\get('amount, txid', 'r_usd', $where, compact('myid', 'jid'))) {
      extract($rec);
      $text = t('Cancel') . (($amount == $waiting0) ? '' : (' ' . u\fmtAmt(abs($amount)) . t(' request')));
      $link = " <a class=\"buttino\" href=\"$base_url/get/cancel=$txid\">$text</a>";
    }
  }
  $way = $waiting0 < 0 ? t('to') : ('from');
  $waiting = item($waiting == '$0' ? t('You have no pending exchange requests.') : (t('You have total pending exchange requests of %waiting %way your bank account.', compact('waiting', 'way')) . @$link), t('Pending:')); // amount already requested (if any)
  $avail = hidFld($avail);
  $credit = u\fmtAmt(-$mya->floor);
  $balances = item(t('balances', compact(ray('balance credit cashoutable'))), t('Balance:'));
  $amount = textFld(t('Amount $'), t('Transfer how much?') . ($mya->cAdmin ? t(' (no limit for admin)') : ''));
  $get = submit(BARE . t('From Bank'));
//  $fromBank = fld('checkbox', t('Automate:'), t('(recommended) Refill my %PROJECT Account from my bank account automatically, as needed.'), dft(!$mya->prefsSet ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = textFld(t('Maximum:'), t('prompt max'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = textFld(t('Excess'), '', dft(u\fmtAmt($mya->overMax(), 's')));
//  $toBank = fld('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft(!$mya->prefsSet ? $mya->can(B_TO_BANK) : TRUE));
  $put = submit(BARE . t('To Bank'));
  $actions = fieldSet('actions', compact('get', 'put'));

  return cgform(compact(ray('title subtext waiting balances amount actions avail')));
}
  
function formGet_validate($form, &$sta) {
  global $mya;
  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // negative caught by < R_ACHMIN below
  extract(just('amount avail', $sta['input']));
  if (!$mya->cAdmin and $amount < R_ACHMIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > ($mya->cAdmin ? round($mya->balance, 2) : $avail)) {
    $max = u\fmtAmt($avail);
    $extra = $mya->cttyRewardy ? '|short cash help' : '';
    return say('short put' . $extra, compact('max'), 'amount');
  }
  if (round($mya->balance - $amount, 2) < $mya->minimum and $mya->refill) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  global $mya;
//  $us = new r\usd($mya = r\acct());
  extract(just('amount', $sta['input']));

  $mya->bank(op($sta) == 'get' ? $amount : -$amount, $msg);
  say($msg);
//  r\tellAdmin('banked' . $extra, $subs);
}