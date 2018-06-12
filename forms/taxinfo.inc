<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;

include __DIR__ . '/../cg-txs.inc';

/**
 * Show forms 1099-B for the previous calendar (or fiscal) year.
 * @param int $ending: date/time year ends (defaults to end of previous calendar year)
 */
function formTaxInfo($form, &$sta, $ending = '') {
  global $base_url;
  global $mya;
  
  if (!$ending) $ending = strtotime('last day of December last year'); // can't use setDft because cmdline param is never null
  $title = t('Your "Barter" Income <small>(Forms 1099-B)</small>');
  $endDesc = u\fmtDate($ending, TRUE);

  $list = "<table id='f1099b'>\n";
  $list .= "<tr><th>Date</th><th class=\"num\">Income</th><th>From</th><th>Description</th></tr>\n";
  $total = 0;
  $negatives = 0; // total negative transactions (refunds that do not specifically reverse another tx)
  $cats = [];
  $map = [t('rebate on') => t('rewards'), t('bonus on') => t('rewards')];
  
  while ($row = be\get1099b($mya->id, $ending)) {
    extract(just('created amount payer for', $row));
    if ($amount <= 0) { // do not issue 1099-B for negative amounts (per IRC 1.6041-...(c)?)
      $negatives += $amount;
      continue;
    }
    x\addCat($cats, strtr($for, $map), $amount, $dups);
    $date = strftime('%d-%b', $created);
    $total += $amount;
    $amount = str_replace('-', '- ', number_format($amount, 2));
    $a = r\acct($payer);
    $addr = str_replace(', Massachusetts', ', MA', $a->postalAddr);
    $phone = str_replace('+1 ', '', u\fmtPhone($a->phone));
    $text = plain("$a->fullName, $addr ($phone)");
    $who = $a->fullName;
    $list .= <<<EOF
<tr>
<td>$date</td>
<td class="num">$amount</td>
<td>$who</td>
<td>$for</td>
</tr>
EOF;
  }

  x\finishCats($cats);
  $total = str_replace('-', '- ', number_format($total, 2));
  $list .= "<tr><td><b>TOTAL</b></td><td class=\"num\"><b>$total</b></td><td></td><td></td></tr>\n</table>\n";
  $list = item($list, t('Detail:'));

  $summary = '';
  foreach ($cats as $cat => $ray) { // for each category
    list ($count, $amount) = $ray;
    $amount = number_format($amount, 2);
    $summary .= <<<EOF
    <tr><td class="category">$cat</td><td class="count">$count</td><td class="amount">$amount</td></tr>\n
EOF;
  }
  $summary = <<<EOF
<table id="summary">
<tr><th>Category</th><th>Count</th><th class="num">Total</th></tr>
$summary
<tr><td><b>TOTAL</b></td><td class="count"></td><td class="num"><b>$total</b></td></tr>
</table>
EOF;
  
  $summary = item($summary, t('Summary:'));
  $regName = r\regionfield('legalName');
  $regPhone = u\fmtPhone(r\regionfield('phone'), '');
  $negatives = $negatives ? t('<p>You also had negative (refund) transactions totaling %neg. These are not reported to the IRS, so you might include this negative transaction total on line 2 of 1040 Form C.</p>', 'neg', u\fmtAmt($negatives)) : '';
 
  $subtext = t(<<<EOF
  <p>If you pay excise, sales or income taxes, include any income you receive through the %PROJECT system.</p>
  <p>The %PROJECT system is required by law to report (to the IRS and to state government) all %PROJECT payments made to you in the past calendar year (on Forms 1099-B). This page shows a list of those transactions. Each individual income transaction is reported.</p>
  <p class="loud">YOU are not required to file Forms 1099-B. So the information on this page is for just for your information.</p>
  <p>The purpose of 1099-B is to report transactions, not total income. So don't worry if some of the payments listed here aren't income (such as reimbursements). Also, you may receive a Form W2 or 1099-MISC from your employer or customers for some of these payments. Take care not to over-report your income by counting things twice.</p>
  <p>For more information, see <%aTips>%PROJECT Payments and Accounting Tips</a>.</p>
EOF
  , '_aTaxHelp _aTips', atag('/help/taxes'), atag('http://cg4.us/doc/payments.pdf?t=' . time())) . $negatives;
  
  $submit = submit(); // display suppressed in CSS
  $go = w\submi(t('Change')); // requires suppressed submit button (find a better way)
  
  $ending = textFld(t('For year ending:'), [t('Fiscal Year End')], required($endDesc) + w\suffix('&nbsp;' . \render($go)));
  $posttext = item(t('<p>This report is provided by %regName (EIN #%R_REGION_EIN). Address: %regAddr (%regPhone, %regEmail).</p><p>If you would rather receive reports by US Mail, you can change that setting in your <%aPrefs>Preferences</a>.', 'regName regAddr regPhone regEmail thisYear _aPrefs', $regName, r\regionfield('postalAddr'), $regPhone, r\regionfield('email'), date('Y'), atag('/settings/preferences')));
  return cgform(compact(ray('title ending subtext table summary list posttext submit')));
}

function formTaxInfo_validate($form, &$sta) {
  if ($err = u\badDate($sta['input']['ending'])) return say($err, 'ending');
}

function formTaxInfo_submit($form, &$sta) {
  w\go('history/tax-info/' . $sta['input']['ending']);
}
