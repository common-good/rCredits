<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;

include_once __DIR__ . '/../cg-txs.inc';

/**
 * Generate an account statement for the given month or year.
 * @param string $month: mmmyyyy or CYyyyy (calendar year) or FYyyyy (fiscal year)
 */
function statement($month = '') {
  include_once __DIR__ . '/../cg-txs.inc';
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$mya = r\acct()) return w\go('', t('You must first sign in.'), 'ERR');

  $type = substr($month, 0, 2); // CY, FY, Q<n>, or other

  if (is_numeric($year = substr($month, 2))) {  
    if (substr($type, 0, 1) == 'Q') {
      list ($endMo, $endDay) = [3 * substr($type, 1), in($type, 'Q1 Q4') ? 31 : 30];
      $lastDay = "$endMo/$endDay/$year";
    } else $lastDay = $type == 'CY' ? "12/31/$year" : "$mya->fyEnd/$year";
    $end = strtotime($lastDay) + DAY_SECS - 1; // fails if FY ends on clock change day
    list ($start, $period) = [u\plusMonths(-12, $end + 1), "$type $year"];
  } else {
    if (!$start = strtotime("1$month")) return w\softErr(t('That is not a recognizable month.'));
    $start = u\monthDay1($start);
    list ($end, $period) = [u\plusMonths(1, $start) - 1, strftime('%B %Y', $start)];
  }
  
  include_once __DIR__ . '/../statement.class';
  
  $pdf = new r\Statement($mya, $period);

  $size = $pdf->getFontSizePt();
  $m = 5; // left/right margin size
  $pdf->SetMargins($m, PDF_MARGIN_TOP, $m);
  $pdf->SetAutoPageBreak(TRUE, 15); // leave space at bottom
  $pdf->setLanguageArray([]); // (eg "page" in footer)
  $pdf->AddPage();
  $pageWidth = $pdf->getPageWidth();

/*  $pdf->SetLineStyle(array('width' => 0.5, 'cap' => 'butt', 'join' => 'miter', 'dash' => 0, 'color' => array(0, 0, 0)));
  $pdf->RoundedRect(91, 30, 60, 60, 3.50, '1111', 'DF', NULL, array(255, 255, 255)); // x, y, w, h, ?, fill?
  */
  //$pdf->Cell(0, 0, 'Page ', 0, false, 'L', 0, '', 0, false, 'T', 'M');
  //$pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">CommonGood.earth</div>');
  list ($address, $city, $state, $zip) = u\parseAddr($mya->postalAddr);
  $pdf->setX($m + 10);
  $pdf->Cell(0, 0, $mya->j_fullName, '', 2);
  $pdf->Cell(0, 0, $address, '', 2);
  $pdf->Cell(0, 0, "$city, $state $zip", '', 2);
  
  //$pdf->Line($m, $y = $pdf->getY(), $pageWidth - $m, $y);
  
  $pdf->setX($m);
  $pdf->setY($pdf->getY() + 10);

  list ($header, $classes, $rows, $tot) = x\getTxRows($start, $end, ray('descending', FALSE)); // get the transaction data
  extract(just('frombank fromyou toyou reward roundups', $tot));
//  list ($fromyou, $toyou) = $amount < 0 ? [-$amount, 0] : [0, $amount];
  $info = be\creditInfo(ray('asof', $end));

  $info0 = be\creditInfo(ray('asof', $start - 1));
  $bal0 = $info0->balance - $info0->partRounds; // balance BEFORE start of this period
  list ($savings0, $savings9) = [$info0->savings, $info->savings];
  $dSavings = round($savings9 - $reward - $savings0, 2);
  list ($toSavings, $fromSavings) = u\order($dSavings >= 0, abs($dSavings), 0);
  // $fromyou += $toSavings;
  // $toyou += $fromSavings;
  $balTitle = t('Balance:');
  $savingsTitle = t('Credit line:');

  list ($bal9, $rewardsEver) = array($info->balance - $info->partRounds, $info->rewards);
//  u\EXPECT($bal9 == round($bal0 + @$frombank - @$fromyou + @$toyou + @$reward, 2), 'balance mismatch');
///**/    if (round($bal9, 2) != round($bal0 + @$frombank - @$fromyou + @$toyou, 2)) return w\softErr('balance mismatch: ' . print_r(compact('tot', 'bal0', 'bal9'), 1));
  
  $pdf->barHead('SUMMARY');
  if ($mya->cttyRewardy) {
    list ($w, $rew1, $fromRew, $rew2, $rew34) = [8, ' /plus/8/R/; Rewards**//23/R;', ' /plus/8/R/; /reward/23/R;', 'fromSavings', ' //8; //23;'];
  } else {
//    $rew1 = $rew2 = $rew34 = '';
    list ($w, $rew1, $fromRew, $rew2, $rew34) = [15, '', '', '', ''];
    $toSavings = $savings9 - $savings0;
  }
  $fields[1] = explode('; ', "/balTitle/15/; Starting/bal0/28/R; /plus/$w/R; From Bank/frombank/23/R; /minus/$w/C; Paid*/fromyou/23/R; /plus/$w/R; Received/toyou/23/R;$rew1 /equals/$w/R; Ending/bal9/28/R");
  $pdf->colHeads($fields[1]);
//  $fields[2] = explode('; ', "/savingsTitle/15/; /savings0/28/R; //$w/R; //23/R; /minus/$w/C; /$fromRew/23/R; /plus/$w/R; /toSavings/23/R;$rew2 //$w/R; /savings9/28/R");
//  $fields[3] = explode('; ', "//15; /underline0/28/R; //$w; //23; //$w; //23; //$w; //23;$rew34 //$w; /underline9/28/R");
//  $fields[4] = explode('; ', "//15; /total0/28/R; //$w; //23; //$w; //23; //$w; //23;$rew34 //$w; /total9/28/R");
  
  list ($plus, $minus, $equals, $space) = array('+', '-', '=', ' ');
  list ($total0, $underline9, $total9) = [$bal0 + $savings0, str_repeat('-', 17), $bal9 + $savings9];
  $underline0 = $underline9;
  $committed = $mya->committed;
  list ($avg) = r\averageBalance($mya->id, $mya->created, $end, TRUE);
  if ($jid = $mya->jid and $j = r\acct($jid)) {
    list ($avg2) = r\averageBalance($j->id, $j->created, $end, TRUE);
    $avg += $avg2;
  }
  
  $yield = number_format($avg ? round(100 * $rewardsEver / $avg) : 0, 1) . '%';
  $numFields = ray('total0 bal0 amount frombank fromyou toyou reward bal9 rewardsEver committed savings0 savings9 fromSavings toSavings total9');
  foreach ($numFields as $one) $$one = number_format(@$$one ?: 0, 2); // ?: is needed because $$one might be null or ''
//  u\prefix('+ ', $savings0);
//  u\prefix('+ ', $savings9);
  
  $pdf->setFontSize(.9 * $size);
  for ($i = 1; $i <= 1; $i++) foreach ($fields[$i] as $one) {
    list ($head, $fldName, $width, $align) = explode('/', "$one/");
    foreach (ray('fromyou fromSavings reward', 1, 1, 3) as $k => $v) if ($fldName == $k) $$k .= str_repeat(' ', $v); // footnote alignment
    $pdf->Cell($width, 0, @$fldName ? @$$fldName : '', '', @$fldName[strlen(@$fldName) - 1] == '9' ? 1 : 0, $align);
  }
  
  $pdf->newLine();
  list ($labelWidth, $numWidth) = array(45, 25);
  if ($mya->cttyRewardy) $pdf->Cell($labelWidth, 0, "Incentive Rewards Ever: $rewardsEver", '', 1);
//  $pdf->Cell($numWidth, 0, $rewardsEver, '', 0, 'R');
//  $pdf->Cell(0, 0, " (Effective yield $yield APR)", '', 1, 'L');

  if ($mya->id == CGF_ID) {
//    $shared = db\get('SUM(amount)', 'r_txs', "payee=:CGF_ID AND payeeFor=':R_SHARING_MSG' AND created<=:end", compact('end'));
//    $pdf->Cell($labelWidth, 0, 'Sharing Donations Ever:');  
//    $pdf->Cell($numWidth, 0, number_format($shared, 2), '', 1, 'R');
  } elseif ($start == u\monthDay1()) { // committed amount is not available retroactively
    //$pdf->Cell($labelWidth, 0, 'Reserved to donate to CGF:');  
    //$pdf->Cell($numWidth, 0, $committed, '', 1, 'R');
  }
  $pdf->setFontSize(.8 * $size);
  $pdf->newLine();
  $change = '$' . number_format(@$roundups + 0, 2);
  $pdf->Cell($pageWidth, 0, "  * Including $change in rounded-up change donated to the community fund.", '', 1, 'L');
  if ($mya->cttyRewardy) $pdf->Cell($pageWidth, 0, t('** Rewards are yours to keep once your %PROJECT community says so. In the meantime you can borrow them interest-free.'), '', 1, 'L');
  
  $pdf->newLine();

  $pdf->setFontSize($size);
  $pdf->barHead('DETAILS');
  list ($rew, $lastHead, $purposeW) = $mya->cttyRewardy ? [' Reward//22/R', 'Reward', 70] : ['', 'Amount', 90];
  $fields = ray("Tx#/tid/15/C /space/1/ Date//17/ Name//55/ /space/1/ Purpose//$purposeW/ /space/1/ Amount//20/R$rew");
  $pdf->colHeads($fields);
  $pdf->setFontSize(.9 * $size);
  $none = strip_tags(R_NONE);
  foreach ($rows as $row) {
    foreach ($row as $k => $v) $row[$k] = str_replace(',', '', strip_tags($v));
    extract(just('tid date name amount purpose reward', array_combine($classes, $row)));
//    foreach (['amount', 'reward'] as $k) if ($$k == R_NONE) $$k = '0.00';
//    list ($fromyou, $toyou) = $amount < 0 ? [-$amount, 0] : [0, $amount]; // just for totals and CGF sharing income
//    $amount = number_format($amount, 2);
    $date = strftime('%b %d', strtotime($date)); // reformat for single month
    if (strlen($purpose) > $purposeLenMax + 3) $purpose = substr($purpose, 0, $purposeLenMax) . '...';
//    $extra = number_format($reward, 2);
    foreach ($fields as $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      if (!$fldName) $fldName = strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = str_replace(',', '', $$fldName)) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      if (in($fldName, $numFields) and $$fldName != $none) $$fldName = number_format($$fldName, 2);
      $pdf->Cell($width, 0, $$fldName, '', $head == $lastHead ? 1 : 0, $align, '', '', 1);
    }
    foreach (['amount', 'reward'] as $k) $$k = $$k == R_NONE ? 0 : (str_replace(',', '', $$k) + 0); // convert from string
    if ($mya->id == CGF_ID and $purpose == R_SHARING_MSG) list ($amount, $reward) = [$reward, $amount];
    if ($reward and $mya->cttyRewardy) {
      $catDesc = r\usdin($purpose) 
      ? t('exchange fees') // this is not a reward and should be moved out of this block
      : (u\starts($purpose, INFLATION_DESC)
        ? (INFLATION_DESC . t(' rewards')) // can't put reward in parens because then addCat strips it off
        : t('other rewards'));
      x\addCat($cats, $catDesc, $reward, $zot);
    }
    if ($amount) x\addCat($cats, $purpose, $amount, $dups);
  }
  
  if (!$rows) {
    $pdf->newLine();
    $pdf->Cell($pageWidth, 0, t('There are no transactions this month.'), '', 1 , 'L');
  }
//  $pdf->Cell($pageWidth, 0, t('* The Extras column includes incentive rewards and any fees paid or received.'), '', 1 , 'L');
  
  if (@$dups) { // don't show categorization unless it's worthwhile
    $pdf->setFontSize($size);
    $pdf->newLine();
    $pdf->barHead('CATEGORY TOTALS');
    $fields = ray('Category/cat/75/ Total/total/25/R Count/count/17/R');
    $pdf->colHeads($fields);
    $pdf->setFontSize(.9 * $size);

    x\finishCats($cats);
    
    foreach ($cats as $cat => $ray) {
      list ($count, $amount) = $ray;
      $total = number_format($amount, 2);
      foreach ($fields as $one) {
        list ($head, $fldName, $width, $align) = explode('/', $one);
        $pdf->Cell($width, 0, $$fldName, '', $head == 'Count' ? 1 : 0, $align, '', '', 1);
      }
    }
  }
  
  $pdf->finish(str_replace(' ', '', PROJECT) . '-' . strftime('%Y-%m', $start) . '.pdf'); //Close and output PDF document
}
