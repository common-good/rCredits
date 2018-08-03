<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Join an investment club or manage membership therein.
 */
function formInvest($form, &$sta, $args) {
  global $mya; $uid = $mya->id;

  $title = t('Investment Club');
  
  if (!$clubid = $mya->iclubid) {
    $subtext = t('There is no investment club in your community at this time. Consider starting one! Your %PROJECT community administrator can set it up for you.');
    return cgform(compact(ray('title subtext')));
  }
  
  if ($res = db\get('*', 'r_stakes', compact(ray('uid clubid')))) { // member!
    extract(just('request stake', $res));
//    extract(just('stake', $res));
//		$request = db\sum('amount', 'r_invoices', 'status=:TX_PENDING AND payer=:clubid AND payee=:uid', compact('clubid', 'uid'));
    $old = hidFld(serialize(just('stakeid clubid stake request requestedOut', $res)));
    $request += 0; // enable if($request)
    $stake += 0;
    $buttons = btn("/invest/list/clubid=$clubid", t('List Investments'), 'success');
    if ($mya->icAdmin) $buttons .= btn("/invest/propose/clubid=$clubid", t('Propose'), 'warning');
		if ($mya->icAdmin) $buttons .= btn("/invest/cashout/clubid=$clubid", t('Requests to Cash Out'), 'warning');
    $amount = textFld($stake ? t('Buy or sell:') : t('Invest:'), [t('Amount'), t('If you reduce your investment amount, it will take effect when funds become available.') . $request ? t('  Whatever amount you request here will be in addition to your current request.') : ''], required()); // must be before request item
    $myRequest = $request ? item(u\fmtAmt($request, ''), t('Change request:'), t('Your pending request to %change your investment', 'change', $request < 0 ? t('reclaim part of') : t('increase'))) : NULL;
//    $togAttrs = ray('on off offstyle', t('Sell'), t('Buy'), 'warning');
//    $buyorsell = togFld('buysell', t('Buy or Sell:'), '', 1, $togAttrs);
    $buy = submi(t('Buy'), '', 'md');
    if ($stake or $request) $sell = submi(t('Sell'), '', 'md');
    //$myStake = item(u\fmtAmt($stake, ''), t('Your investment:'));
    $clubVal = clubVal($investments, $liquid, $reserve);
		$myVal = myClubVal($clubVal);
		$pct = $clubVal ? $myVal / $clubVal : 0;
		$clubVal = item(t('%val (%liquid liquid)', 'val liquid', u\fmtAmt($clubVal), u\fmtAmt($liquid)), t('Club Value:'));
		$myShare = item(tr('%amt (%pct)', 'amt pct', u\fmtAmt($myVal), u\fmtAmt($pct, '%')), t('Your Share:'));
    $submit = fieldSet('submit2', compact('buy', 'sell'), ' ');
  } else { // not a member yet
    $old = hidFld(serialize(compact('clubid')));
    $title = t('Join Your Local Investment Club');
    $subtext = t(<<< X
  <p>Yes, I want to join the %cttyName Investment Club (the Club). I agree to the Club's <%a>Partnership Agreement</a>. In particular:</p>
  <ol><li>I recognize my investment is at risk. I may not be able to reclaim it right away and may <i>lose</i> some or all of it.</li>
  <li>I agree to participate actively in overseeing the Club's investments &mdash; at a minimum by voting on the proposed investments each calendar quarter (voting directly, not by proxy).</li>
  </ol>
X
    , 'cttyName _a', $mya->cttyA->fullName, atag('http://partnershipLink'));
    $signedBy = w\signedBy();
    $submit = t('Join');
  }

  return cgform(compact(ray('title subtext buttons signedBy old clubVal myShare myRequest amount submit')));
}

function formInvest_validate($form, &$sta) {
  global $mya;

  extract(just('signedBy amount old', $sta['input']));

  if (isset($signedBy)) return !w\badSignature($sta);
  
 // buy or sell
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  extract(just('request stake', unserialize($old)));

  if (op($sta) == 'buy') {
    if (@$request + $amount > 0 and $short = $mya->shortfall(FOR_USD, @$request + $amount)) {
      if (!$mya->hasBank) return say(t('You are %short short for that investment.', 'short', u\fmtAmt($short)), 'amount');
      $sta['input']['short'] = $short;
    }
  } elseif ($stake + @$request - $amount < 0) return say(t('That is more than you have invested!'), 'amount');
}

function formInvest_submit($form, &$sta) {
  global $mya;
  extract(just('signedBy old amount short', $sta['input']));
  
  if (@$signedBy) {
    extract(just('clubid', unserialize($old)));
    db\insert('r_stakes', ray('uid clubid joined', $mya->id, $clubid, time()));
    return say(t('You are now a member of the club!'));
  }

  // buy or sell  
  extract(just('stakeid clubid stake request requestedOut', unserialize($old)));
  $buy = (op($sta) == 'buy');
  $request = round($request + ($buy ? $amount : -$amount), 2);
	if (!$buy) $requestedOut = NOW;
  db\update('r_stakes', compact(ray('stakeid request requestedOut')), 'stakeid');
  
  if ($buy and $request > 0) {
    if (@$short) {
      say(t('You do not currently have enough funds in your %PROJECT account for that investment, so %short will be drawn automatically from your bank account. Once that transfer is complete, your requested investment will be made automatically.', 'short', u\fmtAmt($short)));
      $mya->bank($short, $msg);
    } else r\handleInvestmentRequest($stakeid, $msg);
  } elseif (!$buy and $request < 0) { // sell
		$msg = t('Your request to reclaim %request of your investment will be handled by a club administrator when funds become available.', 'request', u\fmtAmt(-$request));
	} else $msg = t('Your request has been adjusted accordingly.');

  say($msg);
}

//		list ($msg, $subs) = $mya->invoice(r\acct($mya->iclubid), $amount, t('reclaim investment'), ray('goods stakeid', FOR_STAKE, $stakeid));
//		say($msg, $subs);

/**
 * Return the investment club's total current value.
 * @param numeric $investments: (RETURNED) value of club's investments
 * @param numeric $liquid: (RETURNED) club's cash assets (including reserve)
 * @param numeric $reserve: (RETURNED) club's cash reserve
 */
function clubVal(&$investments = 0, &$liquid = 0, &$reserve = 0) {
	global $mya;
	$clubid = $mya->iclubid;
	$icA = r\acct($clubid);
	$investments = db\sum('shares*price', 'r_shares s JOIN r_investments i ON i.vestid=s.vestid', ray('clubid', $clubid));
	list ($liquid, $reserve) = [$icA->balance, $icA->minimum];
	return round($liquid - $reserve + $investments, 2); // cash less reserve plus investments
}

/**
 * Return the value of the current account's investment in the club.
 * A CG member who is not a club member can represent their spouse at a club meeting, but there is no joint investment account.
 * @param numeric $icVal: club's total current value
 */
function myClubVal($clubVal) {
	global $mya;
	$stake = 'IF(payer=:clubid,-amount,amount)';
	$share = "(:NOW-created)*$stake";
	$isMe = ':myid IN (payer, payee)';
	$sql = <<<X
	  SELECT SUM($stake) AS totalStake,
		       SUM($share) AS totalShare,
		       SUM(IF($isMe, $stake, 0)) AS myStake,
					 SUM(IF($isMe, $share, 0)) AS myShare
	  FROM r_txs WHERE :clubid IN (payer, payee) AND :IS_STAKE
X;
  extract(db\q($sql, ray('myid clubid', $mya->id, $mya->iclubid))->fetchAssoc());
	$share = ($totalShare + 0) ? $myShare / $totalShare : 0;
	if ($clubVal > $totalStake) $share *= (1 - ICLUB_TITHE); // reserve some profit for community and CG
	return round($myStake + $share * ($clubVal - $totalStake), 2);
}