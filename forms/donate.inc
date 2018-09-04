<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/*
  Give the person a chance to donate to CG from the current account.
  If they haven't chosen a recurring donation yet, a recurring donation is the only option
*/
function formDonate($form, &$sta, $args = '') {
  $mya = w\eLinkAcct($sta, $args);

	//  $oftens = [0 => t('when?')] + ray(OFTENS);
  $oftens = ray(OFTENS);
  $giftLevels = array(
  // 10000 => t('Marble .. $10,000'),
  // 5000 => t('Granite ... 5,000'),
    2500 => t('Slate .... $2,500'),
    1000 => t('Glass .... $1,000'),
     500 => t('Iron ....... $500'),
     250 => t('Oak ........ $250'),
     100 => t('Two Bricks . $100'),
      50 => t('One Brick ... $50'),
      25 => t('Half Brick .. $25'),
//       0 => t('None for now . $0'),
      -1 => t('Water ... (other)'),
//      10 => t('Tile ........ $10'),
//       5 => t('Cloth ........ $5'),
  );

	if ($recurs = db\get('id,period,amount', 'r_recurs', 'payer=:payer AND payee=:CGID AND ended=0', ray('payer', $mya->id))) {
		extract($recurs);
		$recurId = hidFld($id);
    $replaces = t('If you choose a new recurring donation, it will replace your existing %period donation of %amt.', 'period amt', $oftens[$period], u\fmtAmt($amount));
	}
	
  if (!$mya->activated or !$recurs) {
		unset($oftens['1']); // no one-time donation unless the account also has a recurring donation
		$recurOnly = t(': monthly, quarterly, or yearly');
	}
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of');
   
//  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>');
  $title = t('Donate to %PROJECT');
  $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
  $subtext = t('<p>(optional) Make a tax-deductible gift of any size%recurOnly. %replaces</p>', 'recurOnly replaces', @$recurOnly, @$replaces) . t('</p><p>Half of your gift supports whatever projects you decide on together in your community; half supports the operation of the %PROJECT system (to donate zero, choose "Water"). %amountTip</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', compact('amountTip'));
  $gift = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
  $amount = textFld(t('Other amount $'), '');
  $period = selectFld(t('When:'), '', required(), $oftens);
  $honor = selectFld(t('Honoring:'), '', '', $honors);
  $honored = areaFld('', [t('Honoring whom or what')]);
//    $roundup = $mya->ok ? '' : w\roundupFld($mya),
//  if (!$mya->ok) $wait = submit(t('Ask me in a few weeks'), 'info', 'xs');

  jsx('gift');
  return cgform(compact(ray('title subtext gift amount period honor honored recurId')) + w\setupFoot(t('Donate')) + compact('wait'));
}

function formDonate_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta, $args); 

  if (op($sta) == 'wait') return;
  
  extract(just('gift amount period', $sta['input']));
//  $amount = $gift ?: $amount;
  if ($gift != -1) $amount = $gift;
//  if (!$amount) return say('missing field', array('field' => 'amount'), 'amount');
  if ($amount and !$period) return say('missing field', array('field' => 'when'), 'period');
  if ($err = u\badAmount($amount, '>=0')) return say($err, 'amount');
  $sta['input'] = compact(ray('amount')) + $sta['input'];
}

function formDonate_submit($form, &$sta) {
  $mya = w\eLinkAcct($sta, $args); 

  extract(just('amount period honor honored roundup recurId', $sta['input']));
  if ($amount > 0 and op($sta) != 'wait') {
    if ($honored) db\insert('r_honors', ray('uid honor honored', $mya->id, $honor, $honored));
    if ($period != '1') {
			if (@$recurId) db\update('r_recurs', ray('id ended', $recurId, NOW), 'id');
			db\insert('r_recurs', ray('payer payee amount period', $mya->id, CGID, $amount, $period));
		}

    $msg = 'gift successful';
		$info = ray('payer,payee,amount,period,created,lastTx,invoiced', $mya->id, CGID, $amount, $period, NOW, 0, 0);
    if (!r\acceptGift($info)) $msg .= '|gift transfer later';
    say($msg);
  }
  if ($mya->ok) r\tellAdmin('gift', $info);

  return r\acct() ? w\goNextStep('donate', '') : w\doSay('');
}
