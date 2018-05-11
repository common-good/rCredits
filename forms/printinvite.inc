<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Print a sheet of invitation cards for the member or members.
 * @param array $accts: the accounts for which to print invitation cards (defaults to current account)
 */
function printInvite($accts = '') {
  require_once __DIR__ . '/../pdf.class';
  $pdf = new r\Pdf();
//  $pdf->addTTFfont(__DIR__ . '/../../tcpdf/fonts/verdana.ttf'); // do just once on DEV, then upload font
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  if (is_array($accts)) {
    foreach ($accts as $a) printInvite1($a, $pdf);
  } else printInvite1($a = $accts ?: r\acct(), $pdf); // default to current account
  if ($accts) say($pdf->pageNo() . t(' invitation cards have been downloaded to the %PROJECT temp directory.'));
  $pdf->output(DRUPAL_ROOT . R_PATH . '/temp/' . str_replace(' ', '', PROJECT) . '-Invitation-Cards.pdf', $accts ? 'F' : 'I'); //Close and output PDF document
}

/**
 * Print a sheet of invitation cards for one member company.
 * @param acct $a: the account for which to print invitation cards
 * @param object $pdf: the pdf object in process
 */
function printInvite1($a, &$pdf) {
  $forCG = ($a->id == r\cgfId() and $a->superAdmin);
  require_once __DIR__ . '/../pdf.class';
  $slogans = ray(SLOGANS);
  $cardi = 0;

  $co = ($a->co and !$forCG); // pocket-size for individuals and CGF, otherwise bigger for companies
  $name = $a->bestName;
  $invite = $forCG ? t('You are invited to:') : t('%name invites you to:', compact('name'));
//  $iCode = $a->iCardCode(IBY_EMAIL);
  
  list ($mT, $mR, $mB, $mL) = $co ? [0, 0, 0, 0] : [.5, .75, .5, .75]; // margins
  list ($y0, $dy, $wInvite, $dyCode, $wCodeOLD, $ptCode) = $co ? [.2, .23, 3.75, 1.35, 1.9, 13] : [.125, .19, 3, .95, 1.5, 12];
  list ($hLogo, $dySite, $ptSite, $sSite) = $co ? [.9, 2.3, 14, 'semi-expanded'] : [.66, 1.66, 11, 'normal'];
  list ($dxHeart, $dyHeart, $hHeart) = $co ? [2.5, 1.6, .4] : [2.05, 1.15, .3];
  list ($x0, $wLogo, $wHeart) = [$y0, $hLogo, 1.9*$hHeart];
  list ($sSite, $sSlogan, $sProject) = $co ? ['semi-expanded', 'normal', 'normal'] : ['expanded', 'semi-expanded', 'semi-expanded'];
  list ($sloganMax, $sSloganTight) = $co ? [18, 'semi-condensed'] : [20, 'normal'];
  list ($dxInvite, $dyInvite, $ptInvite, $dyLine, $dxLine, $dyGoodFor) = $co ? [$x0+$wLogo+.07, .07, 11.5, .84, .25, .95] : [$x0+$wLogo+.07, 0, 9, .69, .125, .74]; // Line UNUSED
  list ($dxProject, $dyProject, $ptProject, $dySlogan) = $co ? [$dxInvite-.02, .27, 24, .7] : [$dxInvite-.02, .15, 20, .52];
  list ($wCard, $hCard, $tx0) = $co ? [4.25, 2.75, $x0+.04] : [3.5, 2, $x0+.02]; // card dimensions, text indent
  list ($goodFor) = $co ? [t('This card good for')] : [t('Get')];
  list ($wPage, $hPage) = [8.5, 11]; // page dimensions
  $gLen = .1; // cutting guide length from card corner (for pluses in each corner of each card)
  $gStyle = ['color' => [192,192,192]];
  $wCode = $wCard; // code is centered on card

  $sInvite = 'normal';
  if (strlen($name) > 20) $sInvite = strlen($name) > 29 ? 'condensed' : 'semi-condensed';
//  $invite = str_replace(t(' to:'), ':', $invite);
  $pdf->AddPage();
  
  $y = $mT; $x = $mL;
  // cutting guides (for redesign)
  if (FALSE) foreach ([0, $hCard] as $dyL) foreach ([0, $wCard] as $dxL) { // for each corner
    $pdf->Line($x+$dxL-$gLen, $y+$dyL, $x+$dxL+$gLen, $y+$dyL, $gStyle); // horizontal
    $pdf->Line($x+$dxL, $y+$dyL-$gLen, $x+$dxL, $y+$dyL+$gLen, $gStyle); // vertical
  }

  for ($y = $mT; $y < $hPage - $mB; $y += $hCard) for ($x = $mL; $x < $wPage - $mR; $x += $wCard) { // each card
//  { // uncomment this line and comment out the preceding one, to test the format of just one card (much faster)
    $pdf->Image(__DIR__ . '/../images/' . PROJECT_LOGO, $x+$x0, $y+$y0, $hLogo, $hLogo, '', '', '', true); // file, x, y, w, h, type, link, align, resize
    $pdf->say($invite, $x+$dxInvite, $y+$y0+$dyInvite, $wInvite, '', "$ptInvite;font-stretch:$sInvite", 'L');
    $pdf->say(PROJECT . '<sup style="font-size:50%;">&trade;</sup>', $x+$dxProject, $y+$y0+$dyProject, $wInvite, '', "$ptProject;B;color:darkblue;font-stretch:$sProject", 'L');
    $scardi = strlen($slogans[$cardi]) > $sloganMax ? $sSloganTight : $sSlogan;
    $pdf->say(t('the payment card ' . $slogans[$cardi]), $x+$dxInvite, $y+$y0+$dySlogan, $wInvite, '', "$ptInvite;font-stretch:$scardi;color:darkgray", 'L');
//    $pdf->Line($x+$dxLine, $y+$dyLine, $x+$wCard-$dxLine, $y+$dyLine, $gStyle); // x1, y1, x2, y2, style

    $pdf->say('Invitation #', $x+$tx0, $y+$y0+$dyCode, $wCode, '', $ptCode, 'C'); // text, x, y, w, h, format, align, borders
    $iCode = $a->iCardCode(IBY_ICARD + $cardi);
    $iCode = substr($iCode, 0, 4) . ' &nbsp;' . substr($iCode, 4, 4) . ' &nbsp;' . substr($iCode, 8);
    $pdf->say($iCode, $x+$tx0, $y+$y0+$dyCode+$dy, $wCode, '', $ptCode . ';color:darkred;B;Verdana;font-stretch:normal', 'C'); // text, x, y, w, h, fmt, align, borders
//    $pdf->Image(__DIR__ . '/../images/heart+$70.png', $x+$dxHeart, $y+$dyHeart, $wHeart, $hHeart, '', '', '', true); // file, x, y, w, h, type, link, align, resize

    $sSite = 'normal'; // looks better
    $pdf->say('Join us at <span style="font-weight:bold; color:darkgreen; font-size:120%">' . CG_DOMAIN . '</span>', $x, $y+$dySite, $wCard, '', "$ptSite;font-stretch:$sSite", 'C');
    $cardi++;
  }
}
