<?php

require_once('tcpdf/config/lang/eng.php');
require_once('tcpdf/tcpdf.php');
extract($_GET[

$pdf = new TCPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
$pdf->setPrintHeader(FALSE);
$pdf->setPrintFooter(FALSE);
$pdf->AddPage();

$style = array( // set style for barcode
	'border' => 0,
	'vpadding' => '0',
	'hpadding' => '0',
	'fgcolor' => array(0,0,0),
	'bgcolor' => false, //array(255,255,255)
	'module_width' => 1, // width of a single module in points
	'module_height' => 1 // height of a single module in points
);

// Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=array())
$pdf->Image('images/idcard/frame.jpg', 5, 5, 101, 64, '', '', '', true, 150, '', false, false, 0, false, false, false);
$pdf->Image('sites/default/files/pictures/picture-5-1348671380.jpg', 10, 10, 40, 40, '', 'account', '', true, 150, '', false, false, 0, false, false, false);
$pdf->write2DBarcode('NEW.AAEFGH', 'DATAMATRIX', 53, 25, 25, 25, $style, 'N');
$pdf->Image('images/rlogo150.png', 85, 25, 14.93, 20, '', '', '', true, 150, '', false, false, 0, false, false, false);

$html = '<span style="font-size:22px;">Member ID<br><span style="font-size:38px; color:darkred;">%CODE</span><br>%REGION</span>';
$pdf->writeHTMLCell(50, 15, 52, 10, $html); // w, h, x, y
$pdf->writeHTMLCell(20, 5, 85, 47, '<div style="font-size:22px;">rCredits.org</div>');
$pdf->writeHTMLCell(91, 20, 10, 55, "<div style=\"font-size:60px; text-align:center; color:midnightblue; font-weight:bold;\">$name</div>");

$pdf->Output('example_050.pdf', 'I'); //Close and output PDF document
