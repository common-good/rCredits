<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Print coupons for the account.
 */
function formCouponsList($form, &$sta, $args = '') {
  global $mya;

	if ($args == 'ALL') return showDiscounts();
	
  $title = t('Your Coupons and Gift Certificates');

	$fields = ray('type amount on start end minimum ulimit action');
  $hdrs = u\tableRow(ray('Type,Amount,On,Starting,Ending,Min&nbsp;Purchase,Max&nbsp;Uses,'));
	$automatic = TRUE;

  $sql = <<< X
    SELECT * FROM r_coupons 
		WHERE fromId=:uid
		ORDER BY coupid DESC
X;
  $q = db\q($sql, ray('uid', $mya->id));
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
		$gift = $flags;
		$type = $gift ? t('gift') : t('discount');
		
		if ($gift) {
			$args = "type=gift&amount=$amount";
			$count = $end - $start;
		} else {
      $args = 'print/' . u\urlify(http_build_query(compact($fields, 'automatic')));
      if (u\test()) $args = u\deurlify($args);
  		$start = u\fmtDate($start);
  		$end = $end ? u\fmtDate($end) : t('indefinite');
			$on = t('on ') . $on;
//  		$on = $on ? w\popHelp(t(' on'), $on) : '';
		}

		$action = w\btn("/community/coupons/$args", t('reprint'), 'primary', 'xs', $gift ? NULL: w\away());

		$amount = $amount < 0 ? u\fmtAmt(-$amount/100, '%') : u\fmtAmt($amount);
		$minimum = $minimum ? u\fmtAmt($minimum) : t('none');
		$ulimit = ($ulimit + 0) ? number_format($ulimit) : t('no limit');

    $rows[] = u\tableRow(compact($fields));
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="coupons">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('You do not have any coupons yet.');
  
  return cgform(compact(ray('title list')));
}

/**
 * Show automatic discounts available to the current account.
 */
function showDiscounts() {
	global $mya;

	$title = t('Automatic Discounts in Your Region');
	$subtext = t('as of %today', 'today', u\fmtDate(NOW));
	$fields = ray('company amount on end ulimit');
  $hdrs = u\tableRow(ray('Company,Discount,On,Ending,Max&nbsp;Uses'));
	
  $sql = <<< X
  	SELECT uid,amount,`on`,end,ulimit FROM r_coupons c JOIN users u ON u.uid=c.fromId
		WHERE (u.community=:ctty OR u.state=:state) AND NOT c.flags AND (end=0 OR end>:NOW) AND start<=:NOW
		ORDER BY end=0, end
X;
  $q = db\q($sql, $subs = ray('ctty state', $mya->community, $mya->state));

  while ($row = $q->fetchAssoc()) {
    extract($row);
		$a = r\acct($uid);
		$company = $a->bestName;
  	$amount = $amount < 0 ? u\fmtAmt(-$amount/100, '%') : u\fmtAmt($amount);
 		$end = $end ? u\fmtDate($end) : t('--');
  	$on = t('on ') . $on;
		$ulimit = ($ulimit + 0) ? number_format($ulimit) : t('no limit');
    $rows[] = u\tableRow(compact($fields));
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="coupons">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('There are no coupons in your community at this time.');
  
  return cgform(compact(ray('title subtext list')));	
}