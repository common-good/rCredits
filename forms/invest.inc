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
  
  if (!$clubid = db\lookup('uid', 'users', ':IS_CO AND community=:ctty AND :IS_ICADMIN', ray('ctty', $mya->community))) {
    $subtext = t('There is no investment club in your community at this time. Consider starting one! Your %PROJECT community administrator can set it up for you.');
    return cgform(compact(ray('title subtext')));
  }
  
  if ($res = db\lookup('*', 'r_stakes', compact(ray('uid clubid')))) { // member!
    extract(just('request stake', $res));
    $old = hidFld(serialize(just('stakeid clubid stake request', $res)));
    $request += 0; // enable if($request)
    $stake += 0;
    $buttons = btn("/invest/list/clubid=$clubid", t('List Investments'), 'success');
    if ($mya->icAdmin) $buttons .= btn("/invest/propose/clubid=$clubid", t('Propose'), 'success');
    $amount = textFld($stake ? t('Buy or sell:') : t('Invest:'), [t('Amount'), t('If you reduce your investment amount, it will take effect (if possible), when the next investment round happens.') . $request ? t('  Whatever amount you request here will be in addition to your current request.') : ''], required()); // must be before request item
    $myRequest = $request ? item(u\fmtAmt($request, ''), t('Change request:'), t('Your pending request to %change your investment', 'change', $request < 0 ? t('reclaim part of') : t('increase'))) : NULL;
//    $togAttrs = ray('on off offstyle', t('Sell'), t('Buy'), 'warning');
//    $buyorsell = togFld('buysell', t('Buy or Sell:'), '', 1, $togAttrs);
    $buy = submi(t('Buy'), '', 'md');
    if ($stake) $sell = submi(t('Sell'), '', 'md');
    $myStake = item(u\fmtAmt($stake, ''), t('Your investment:'));
    $submit = fieldSet('submit2', compact('buy', 'sell'), ' ');
  } else { // not a member yet
    $old = hidFld(serialize(compact('clubid')));
    $title = t('Join Your Local Investment Club');
    $subtext = t(<<< X
  <p>Yes, I want to join the %cttyName Investment Club (the Club). I agree to the Club's <%a>Partnership Agreement</a>. In particular:</p>
  <ol><li>I recognize my investment is at risk. I may not be able to request it right away and may <i>lose</i> some or all of it.</li>
  <li>I agree to participate actively in overseeing the Club's investments &mdash; at a minimum by voting on the proposed investments each calendar quarter (voting directly, not by proxy).</li>
  </ol>
X
    , 'cttyName _a', $mya->cttyA->fullName, atag('http://partnershipLink'));
    $signedBy = w\signedBy();
    $submit = t('Join');
  }

  return cgform(compact(ray('title subtext buttons signedBy old myStake myRequest amount submit')));
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
  extract(just('stakeid clubid stake request', unserialize($old)));
  $buy = (op($sta) == 'buy');
  $request += $buy ? $amount : -$amount;
  db\update('r_stakes', compact(ray('stakeid request')), 'stakeid');
  
  if ($buy) {
    if (@$short) {
      say(t('You do not currently have enough funds in your %PROJECT account for that investment, so %short will be drawn automatically from your bank account. Once that transfer is complete, your requested investment will be made automatically.', 'short', u\fmtAmt($short)));
    } elseif ($request > 0) {
      r\handleInvestmentRequest($stakeid, $msg);
      say($msg);
    } else say(t('Your request has been adjusted accordingly.'));
  } else say(t('Your request to reclaim some of your investment will be attempted upon the next investment cycle.'));
}

