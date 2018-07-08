<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle coupons for the account.
 */
function formCoupons($form, &$sta, $args = '') {
  global $mya;
  extract(just('type amount', $args));
  
  if (@$type == 'redeem') {
    $title = t('Redeem a General Gift Certificate');
    $subtext = t('Find the code on your gift certificate. If the the certificate is just for one company, there is no code &mdash; just go buy something at that business.');
    $code = textFld(t('Certificate Code:'), [t('Code')], required());
    $submit = t('Redeem');
  } elseif (@$type == 'gift') {
    $title = t('Create a Gift Certificate');
    $subtext = t('Each gift certificate is assigned a <b class="loud">unique code</b>. Print this sheet only once and do not photocopy it.');
    $amount = textFld(t('Amount:'), [t('Gift amount')], required(@$amount ? u\fmtAmt($amount) : ''));
    $count = textFld(t('Count:'), [t('How many?'), t('Print how many coupons? (8 to a sheet, to be cut in half 3 times)')], required());
    $submit = t('Create');
  } elseif (@$type == 'discount') {
    $title = t('Create a Discount Coupon');
    $subtext = t(<<<X
      <p>How it works:</p>
      <ol><li>Fill out the form.</li>
      <li>You choose:
        <ul><li>%PROJECT members get an automatic rebate when they pay you OR</li>
        <li>You charge less when they present the coupon.</li>
        </ul>
      </li>
      <li>Print a sheet of coupons (or copy and paste into a print ad or newsletter). Cut the sheet exactly in half three times.</li>
      </ol>
      <p>%PROJECT will also distribute your coupon electronically to each %PROJECT member!</p>
X
    );
  
    $amount = textFld(t('Discount:'), [t('Amount or Percentage'), t('For example $20 or 10%')], required());
    $minimum = textFld(t('Minimum:'), [t('Minimum purchase'), t('Minimum purchase amount, to get the discount. If this amount equals the discount amount, your coupons will be labeled "Gift Certificate".')], required(0));
    $start = textFld(t('Valid from:'), [t('Start date')], required(u\fmtDate(time(), TRUE)));
    $end = textFld(t('Valid until:'), [t('End date'), t('An end date is strongly encouraged.')]);
    $ulimit = textFld(t('Limit:'), t('Leave this blank for unlimited uses.'), suffix(t(' uses per member')));
    $automatic = w\boolFld(t('Automatic?'), t('If the discount is not automatic, you will need to do the math each time.'), TRUE);
    $on = textFld(t('Discount on:'), [t('Discount on what')]);
    $submit = t('Create Coupon');
  } else {
    $title = t('Coupons and Gift Certificates');
    $subtext = t('<p>A <b>Discount Coupon</b> is redeemable only at your business. If the minimum purchase amount equals the discount amount, it is equivalent to a traditional gift certificate.</p><p>A <b>General Gift Certificate</b> is redeemable at any %PROJECT business.</p>');
    $buttons = 
      w\btn('/community/coupons/type=discount', t('Create a Discount Coupon'))
    . w\btn('/community/coupons/type=gift', t('Create a General Gift Certificate'))
    . w\btn('/community/coupons/type=redeem', t('Redeem a General Gift Certificate'))
    . w\btn('/community/coupons/list', t('List Your Coupons'))
    . w\btn('/community/coupons/list/ALL', t('Show All Discounts'));
  }
  
  $type = hidFld(@$type);
  jsx('coupons');
  
  return cgform(compact(ray('title buttons subtext code amount count minimum start end ulimit automatic on submit type')));
}

function formCoupons_validate($form, &$sta) {
  global $mya;
  $today = strtotime('today');
  extract(just($fields = 'type code amount count minimum start end ulimit', $sta['input']));

  if ($type == 'gift') {
    if ($err = u\badAmount($amount, '>0')) return w\sayFieldErr($err, 'amount');
    if ($err = u\badAmount($count, '>0')) return w\sayFieldErr($err, 'count');
    if ($mya->creditLine < $amount * $count) return say(t('You cannot issue more Gift Certificates than your credit line allows: (%try > %max).', 'try  max', u\fmtAmt($amount * $count), u\fmtAmt($mya->creditLine)), 'err');
    $ulimit = 1;
  } elseif ($type == 'discount') {
    $amt0 = trim($amount);
    $amount = str_replace('%', '', $amount);
    $nums = 'amount minimum' . ($ulimit === '' ? '' : ' ulimit');
    foreach (ray($nums) as $k) if ($err = u\badAmount($$k, $k == 'amount' ? '>0' : '>=0')) return w\sayFieldErr($err, $k);
//    if ($amt0[0] == '$' and $amount < 1) return w\sayFieldErr(t('The discount amount cannot be less than $1.'), 'amount');
//    if (substr($amt0, -1, 1) == '%' and $amount > 100) return w\sayFieldErr(t('The discount percentage must be no more than 100%.'), 'amount');
    if (strpos($amt0, '%')) $amount = -$amount; // we use negative to mean percentage
    
    foreach (ray('start end') as $k) if ($$k and $err = u\badDate($$k, $today)) return w\sayFieldErr($err, $k);
    if ($end and $end < $start) return w\sayFieldErr(t('Ending date cannot be before starting date.'), 'end');
    if ($start == u\fmtDate($today, TRUE)) $start = ''; // empty means now
    foreach (ray('ulimit end') as $k) $$k = (int) $$k;
  } elseif ($type == 'redeem') {
    if (!$a = r\iCardAcct($code, $iCode)) return say('invalid gift card', 'code');
    if ($a->id == $mya->id) return say(t('You cannot redeem your own gift card.'), 'err');
    $coupid = db\get('coupid', 'r_coupons', 'fromId=:id AND :iCode BETWEEN start AND end-1', ray('id iCode', $a->id, $iCode)) ?: 0;
    if (db\get('id', 'r_coupated', 'uses=:iCode AND coupid=:coupid', compact(ray('iCode coupid')))) return say('already redeemed', 'err');
    u\preray(compact(ray('coupid iCode')), $sta['input']); // pass the results to submit function
  } else return say(t('bad coupon'), 'err');
  
  u\preray(compact(ray($fields)), $sta['input']); // remember fixed numeric values
}

function formCoupons_submit($form, &$sta) {
  global $mya; $myid = $mya->id;
  extract(just('type amount coupid iCode count minimum start end ulimit automatic on', $sta['input']));

  if ($type == 'redeem') {
    $res = db\get('fromId,amount', 'r_coupons', 'coupid=:coupid', compact('coupid'));
    extract($res);
    $fromA = r\acct($fromId);
    $fromA->pay($mya, $amount, t('gift'), ray('force', 1));
    db\insert('r_coupated', ray('uid coupid uses when', $myid, $coupid, $iCode, time()));
    $mya->update('giftPot', $mya->giftPot + $amount);
    return say(t('coupon redeemed', 'amount giver', u\fmtAmt($amount), $fromA->fullName));
  }

  // gift or discount  
  $gift = ($type == 'gift');
  $flags = $gift ?: 0; // set gift flag

  if ($gift) {
    list ($start, $end) = [$mya->giftCoupons + 0, $coups = $mya->giftCoupons + $count];
  } else {
		if (!@$on) $on = $minimum ? t('your purchase of %min or more', 'min', u\fmtAmt($minimum)) : t('any purchase');
		if ($end) $end = strtotime('tomorrow', $end) - 1; // last second of the day
	}

  if ($gift or $automatic) db\insert('r_coupons', compact(ray('amount on minimum start end ulimit flags')) + ray('fromId', $myid));
  if ($gift) {unset($start); unset($end);} // don't pass these to printCoupons()

  $args = u\urlify(http_build_query($ray = compact(ray('type amount minimum ulimit count on start end automatic'))));
	r\tellCo(t('New Coupons!'), $ray);
  if (u\test()) $args = u\deurlify($args);
	$url = "/community/coupons/print/$args";
  return u\test() ? w\go($url) : w\go2('', $url);
}
