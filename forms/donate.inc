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
function formDonate($form, &$sta) {
  global $mya;
  $donated = @$mya->stepsDone['donate'] ? t('<p>You have already made a donation. Thank you! Please feel free to donate again.</p>') : '';
  //  Whatever "share" percentage you choose this time will replace your previous choice') . " ($mya->share%).
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
  $oftens = [0 => t('when?')] + ray(OFTENS);
  if (!$mya->ok or !db\exists('r_gifts', "uid=:id AND often<>'1'", ['id' => $mya->id])) unset($oftens['1']);
  $honors = array('-' => '(optional)', 'honor' => 'in honor of', 'memory' => 'in memory of', 'other' => 'other');
//  $giftAttrib = w\onchange(str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_GIFT_SHOWHIDE));
   
//  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p><p>Also choose a percentage of your ongoing incentive rewards to share with Common Good Finance.');
  $title = t('Donate to %PROJECT');
  $amountTip = $mya->co ? '' : t('The most common amounts chosen are $50 and $25.');
  $subtext = $donated . t('<p>(optional) Make a tax-deductible gift of any size: monthly, quarterly, or yearly.</p></p><p>Half of your gift supports whatever projects you decide on together in your community; half supports the operation of the %PROJECT system (to donate zero, choose "Water"). %amountTip</p><p><b class="loud">NOTE: This is a donation, not a deposit.</b> Thank you for your support!</p>', compact('amountTip'));
  $gift = selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT), $giftLevels);
  $amount = textFld(t('Other amount $'), '');
  $often = selectFld(t('When:'), '', required(), $oftens);
  $honor = selectFld(t('Honoring:'), '', '', $honors);
  $honored = areaFld('', [t('Honoring whom or what')]);
//    $share = $mya->ok ? '' : w\shareFld($mya),
//    $roundup = $mya->ok ? '' : w\roundupFld($mya),
//  if (!$mya->ok) $wait = submit(t('Ask me in a few weeks'), 'info', 'xs');

  jsx('gift');
  return cgform(compact(ray('title subtext gift amount often honor honored')) + w\setupFoot(t('Donate')) + compact('wait'));
}

function formDonate_validate($form, &$sta) {
  global $mya;

  if (op($sta) == 'wait') return;
  
  extract(just('gift amount share often', $sta['input']));
//  $amount = $gift ?: $amount;
  if ($gift != -1) $amount = $gift;
//  if (!$amount) return say('missing field', array('field' => 'amount'), 'amount');
  if ($amount and !$often) return say('missing field', array('field' => 'when'), 'often');
  if ($err = u\badAmount($amount, '>=0')) return say($err, 'amount');
//  if (!$mya->ok and $err = w\badShare($share)) return say($err, 'share');
//  $share = min($share, 999.999); // any bigger kills PDO
  $sta['input'] = compact(ray('amount share')) + $sta['input'];
}

function formDonate_submit($form, &$sta) {
  global $mya;
  $info = just('amount often honor honored share roundup', $sta['input']);
  if ($info['amount'] > 0 and op($sta) != 'wait') {
    if ($info['honor'] == '-') $info['honor'] = 'honor';
    if (!$info['honored']) $info['honor'] = '';
    u\setDft($info['often'], '1');
    $uid = $mya->id;
    $giftDate = r\rTime();
    $info += compact('uid', 'giftDate');
    $donid = db\insert('r_gifts', $info);
  //  if (!$mya->ok) $mya->update(just('share', $info));
  //  if ($mya->ok) $mya->setbit(B_ROUNDUP, $roundup);

    $msg = 'gift successful';
    if (!r\acceptGift($info + compact('donid'))) $msg .= '|gift transfer later';
    $amount = u\fmtAmt($info['amount']);
    say($msg, compact('amount'));
  }
  if ($mya->ok) r\tellAdmin('gift', $info);

  return w\goNextStep('donate', '');
}
