<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/invest.inc';

/**
 * Let Investment Club administrators accommodate member requests to reclaim all or part of their investment.
 */
function formInvestCashout($form, &$sta, $args = '') {
	global $mya;
	$clubid = $mya->iclubid;
	
  $clubVal = clubVal($investments, $liquid, $reserve);
	$avail = $liquid - $reserve;
	$sql = 'SELECT SUM(request) AS total, COUNT(request) AS count FROM r_stakes WHERE clubid=:clubid AND request<0';
	extract(db\q($sql, compact('clubid'))->fetchAssoc());

  $title = t('Handle Requests to Cash Out Investments');
  $available = item(u\fmtAmt($avail), t('Available Funds:'));
	$requests = item(tr('%total (%count)', 'total count', u\fmtAmt(-$total), number_format($count)), t('Total Requests:'));
	if ($avail < $total) $method = w\radiosFld(t('Method:'), '', ray('inline', TRUE), [t('Oldest Request First'), t('Partial Cashout for All')]);
	$submit = t('Cash Them Out');
  $back = backButton();
	$hid = hidFld(serialize(compact(ray('clubid avail total'))));
  
  return cgform(compact(ray('back title available requests method submit hid')));
}

function formInvestCashout_submit($form, &$sta) {
	global $mya;
	
	extract(just('method hid', $sta['input']));
	extract(just('clubid avail total', unserialize($hid)));
	$icA = r\acct($clubid);
	$pct = ($partial = @$method) ? min(1, $avail / $total) : 1;
	$count = $sum = 0;
	
	$q = db\q('SELECT stakeid,uid,request FROM r_stakes WHERE clubid=:clubid AND request<0 ORDER BY requestedOut', compact('clubid'));
  while ($row = $q->fetchAssoc()) {
		extract($row);
		$amount = round($pct * -$request, 2);
		if ($amount > 0 and ($partial or $sum + $amount <= $avail)) {
			$DBTX = db_transaction();
			$icA->pay(r\acct($uid), $amount, t('reclaim investment'), ray('goods stake', FOR_USD, 1));
			db\update('r_stakes', ray('stakeid request', $stakeid, round($request + $amount, 2)), 'stakeid');
			$count++;
			$sum += $amount;
		  unset($DBTX);
		}
	}
	say(t('The club paid %count members a total of %sum.', 'count sum', number_format($count), u\fmtAmt($sum)));
}