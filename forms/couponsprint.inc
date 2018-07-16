<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Print a sheet of coupons for the current account.
 type amount minimum ulimit count automatic start end on
 */
function couponsPrint($args = '') {
  global $mya;
  require_once __DIR__ . '/../pdf.class';
  
  if (strpos($args, 'type=') === FALSE) $args = u\deurlify($args);
  extract(just('type amount minimum ulimit count automatic start end on', $args));
	
  if (!$gift = (@$type == 'gift') and @$type != 'discount') return w\hack('bad coupon args');
  $coGift = ($amount == @$minimum);
  $amount =  $amount < 0 ? u\fmtAmt(-$amount / 100, '%s') : u\fmtAmt($amount, 's$');
  u\setDft($count, 8);

  if (@$start == strtotime('today')) $start = '';
  if (@$end) {
    $end = u\fmtDate(@$end, TRUE);
    $dates = @$start ? u\fmtDate($start, TRUE) . t(' to ') . $end : (t('expires ') . $end);
  } else {
    $dates = @$start ? t('beginning ') . u\fmtDate($start, TRUE) : '';
    $dates .= t(' (offer does not expire)');
  }
  if (@$ulimit) $dates .= t(' &mdash; Limit: %ulimit uses per member', compact('ulimit'));
  
  $pdf = new r\Pdf();
//  $pdf->addTTFfont(__DIR__ . '/../../tcpdf/fonts/verdana.ttf'); // do just once on DEV, then upload font
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  $name = $mya->bestName;
  $coup = t('%PROJECT %type', 'type', ($gift or $coGift) ? t('Gift Certificate') : t('Discount Coupon'));
	$off = ($gift or $coGift) ? '' : t(' OFF');
	$goodFor = $off ? '' : t('Good for ');
//  $iCode = $mya->iCardCode(IBY_EMAIL);
  $auto = (@$automatic and $off) ? t('automatically') : '';
  
  list ($mT, $mR, $mB, $mL) = [0, 0, 0, 0]; // margins
  list ($hLogo, $dySite, $ptSite, $sSite) = [.9, 2.3, 14, 'semi-expanded'];
  list ($dxHeart, $dyHeart, $hHeart) = [2.5, 1.6, .4];
  list ($y0, $dy, $ptDetails, $ptSlogan, $ptCode) = [.2, .23, 12.5, 11, 13];
  $dyDetails = $y0 + 1.15;
  list ($x0, $wLogo, $wHeart) = [$y0, $hLogo, 1.9*$hHeart]; // left card margin, logo widths
  list ($wCard, $hCard, $tx0) = [4.25, 2.75, $x0+.04]; // card dimensions, text indent
  $wCoup = $wCard-$wLogo-$x0;
  list ($sSlogan, $sOff) = ['normal', 'normal'];
  list ($dxCoup, $dyCoup, $ptCoup, $dyGoodFor) = [$x0+$wLogo+.08, $y0+.07, 12.5, .95];
  list ($dxOff, $dyOff, $ptOff, $dyAt) = [$dxCoup-.02, $y0+.29, 24, $y0+.7];
  list ($dxLine, $dyLine) = [$x0, 1.25];
//  list ($goodFor) = [t('This card good for')];
  list ($wPage, $hPage) = [8.5, 11]; // page dimensions
  $gLen = .1; // cutting guide length from card corner (for pluses in each corner of each card)
  $gStyle = ['color' => [192,192,192]];
  $wCode = $wCard; // code is centered on card
  $slogan = t('the payment card for community power');
  
  $sAt = 'normal';
  if (strlen($name) > 20) $sAt = strlen($name) > 29 ? 'condensed' : 'semi-condensed';

  $pdf->AddPage();
  
  $y = $mT; $x = $mL;
  // cutting guides (for redesign)
  if (FALSE) foreach ([0, $hCard] as $dyL) foreach ([0, $wCard] as $dxL) { // for each corner
    $pdf->Line($x+$dxL-$gLen, $y+$dyL, $x+$dxL+$gLen, $y+$dyL, $gStyle); // horizontal
    $pdf->Line($x+$dxL, $y+$dyL-$gLen, $x+$dxL, $y+$dyL+$gLen, $gStyle); // vertical
  }

  $cardi = 0;
  while ($cardi < $count) {
    for ($y = $mT; $y < $hPage - $mB; $y += $hCard) for ($x = $mL; $x < $wPage - $mR; $x += $wCard) { // each card
  //  { // uncomment this line and comment out the preceding one, to test the format of just one card (much faster)
      $pdf->Image(__DIR__ . '/../images/' . PROJECT_LOGO, $x+$x0, $y+$y0, $hLogo, $hLogo, '', '', '', true); // file, x, y, w, h, type, link, align, resize
      $pdf->say($coup, $x+$dxCoup, $y+$dyCoup, $wCoup, '', "$ptCoup;color: #404040", 'L');
      $pdf->say("$goodFor$amount$off", $x+$dxCoup, $y+$dyOff, $wCoup, '', "$ptOff;B;color:darkred;font-stretch:$sOff", 'L');
      $sSite = 'normal'; // looks better
      $pdf->say(t('Join us! at ') . '<span style="font-weight:bold; color:darkgreen; font-size:120%">' . CG_DOMAIN . '</span>', $x, $y+$dySite, $wCard, '', "$ptSite;font-stretch:$sSite", 'C');
        $pdf->Line($x+$dxLine, $y+$dyLine, $x+$wCard-$dxLine, $y+$dyLine, $gStyle); // x1, y1, x2, y2, style
      
      if ($gift) { // gift coupon
        $pdf->say(t('at any member business', compact('name')), $x+$dxCoup, $y+$dyAt, $wCoup, '', "$ptCoup;font-stretch:semi-expanded;color:darkred", 'L');
        $pdf->say(t('when you pay with <b>%PROJECT</b>') . '<sup style="font-size:50%;">&trade;</sup>', $x, $y+$dyDetails, $wCard, '', $ptDetails, 'C'); // text, x, y, w, h, format, align, borders
        $pdf->say($slogan, $x, $y+$dyDetails+$dy, $wCard, '', "$ptSlogan;I;Verdana;font-stretch:normal;color:darkgray", 'C'); // text, x, y, w, h, fmt, align, borders
  //      $pdf->Image(__DIR__ . '/../images/heart+$70.png', $x+$dxHeart, $y+$dyHeart, $wHeart, $hHeart, '', '', '', true); // file, x, y, w, h, type, link, align, resize

        $iCode = $mya->iCardCode($mya->giftCoupons + $cardi);
        $iCode = substr($iCode, 0, 4) . ' &nbsp;' . substr($iCode, 4, 4) . ' &nbsp;' . substr($iCode, 8);
        $pdf->say(t('Redeem at Community > Coupons: <b>%iCode</b>', '_iCode', $iCode), $x, $y+$dyDetails+$dy*2.5, $wCard, '', "$ptDetails;Verdana;font-stretch:semi-condensed;color:#404040", 'C'); // text, x, y, w, h, fmt, align, borders
      } else { // discount coupon
        $pdf->say($auto, $x+$wLogo, $y+$dyOff+.14, $wCoup-.3, '', '9;color:darkgray', 'R'); // .24 for baseline
        $pdf->say(t('at <b>%name</b>', compact('name')), $x+$dxCoup, $y+$dyAt, $wCoup, '', "$ptCoup;font-stretch:semi-condensed;color:black", 'L');
        if (!$goodFor) $pdf->say(t('on ') . $on, $x, $y+$dyDetails, $wCard, '', $ptDetails, 'C'); // text, x, y, w, h, format, align, borders
        $pdf->say(t('when you pay with <b>%PROJECT</b>'), $x, $y+$dyDetails+$dy, $wCard, '', $ptDetails, 'C'); // text, x, y, w, h, format, align, borders
        $pdf->say($slogan, $x, $y+$dyDetails+$dy*2, $wCard, '', "$ptSlogan;I;Verdana;font-stretch:normal;color:darkgray", 'C'); // text, x, y, w, h, fmt, align, borders
        $pdf->say($dates, $x, $y+$dyDetails+$dy*3, $wCard, '', "10;color:darkgray", 'C'); // text, x, y, w, h, format, align, borders
      }
      $cardi++; // how many cards have been completed
      if ($cardi >= @$count) {$y = 999; break;} elseif ($cardi % 8 == 0) $pdf->AddPage();
    }
  }

  if ($gift) $mya->update('giftCoupons', $mya->giftCoupons + $count);

  if (!u\test())
  $pdf->output('cgCoupons.pdf', 'I'); //Close and output PDF document
}
