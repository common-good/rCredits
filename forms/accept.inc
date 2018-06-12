<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Accept an invitation by supplying the invitee email address.
 * @parm string $args: '', 'zip', or 'self'
 */
function formAccept($form, &$sta, $args = '') {
  if (@$args == 'zip') {
    $title = t('Who do you know?');
    $friend = textFld(t('Friend\'s name:'), '', required());
    $zip = textFld(t('Their zipcode:'), '', required());
  } elseif (@$args == 'self') {
    $title = t('Where do you live?');
    $friend = hidFld('self');
    $zip = textFld(t('Your zipcode:'), '', required());
  } else {
    $title = t('Accept Your Invitation');
    $invitation = textFld(t('Invitation #:'), t('Carefully type your invitation number.'), required());
  }
  
  $submit = t('Continue');
  return cgform(compact(ray('title subtext invitation friend zip submit')));

  //$subtext = item(
//  $subtext = item(strtr(t('A short @BLURB was sent with the invitation (click the link to see it).'), ['@BLURB' => blurbLink()]));
//  $email = textFld(t('Your email:'), t('Type the email address to which the invitation was sent.'));
}

function formAccept_validate($form, &$sta) {
  extract(just('invitation friend zip', $sta['input']));
  \drupal_get_messages(); // drop previous errors
  
  if (isset($invitation)) {
    if (!r\iCardAcct($invitation)) return say('bad invite num', 'invitation');
  } elseif ($err = u\badZip($zip)) {
    return say($err, 'zip');
  } elseif (@$friend == 'self') {
    $community = r\communityUid(@$zip);
    if (r\acct($community)->invites) return say('invitation required', ['_a1' => atag(PROMO_URL . '/signup')], 'zip');
  }

//  if (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
}

function formAccept_submit($form, &$sta) {
  extract(just('invitation friend zip', $sta['input']));
//  $email = u\cry('P', $email);
//  if (!$code = db\get('code', 'r_invites', 'email=:email ORDER BY (invitee=0) DESC, invited DESC', compact('email'))) return say('bad invite', 'email');
  say(t('Hurray! You qualify to open a %PROJECT Account.'));
  $code = @$invitation ?: "$friend ($zip)";
  return w\go("signup/code=$code");
}