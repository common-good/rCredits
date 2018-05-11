<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/changepassword.inc';
include_once __DIR__ . '/resetpassword.inc';

/**
 * Send the new member a link and a code, to verify their email address.
 */
function formVerify($form, &$sta, $arg = '') {
  global $base_url, $mya;

  $title = t('Verify Your Email Address');
  $subtext = t('sent email', 'email', $mya->email);
  $resend = ' &nbsp; ' . t('<%a>Resend or change email</a>', '_a', atag('/settings/resend'));
  $verify = textFld(t('Code:'), [t('Verification Code')], required() + suffix($resend));
  svar('msg', 'yes');
  $form = compact(ray('title subtext verify'));
  if (!$mya->co) $form += w\pickPassword(FALSE);

  $mya->verifySsnInBackground();
  
  return cgform($form + w\setupFoot());
}

function formVerify_validate($form, &$sta) {
  global $mya;
  extract(just('verify', $sta['input']));
  if (!$mya->passwordOkay(strtoupper($verify))) return say(t('That is not the right code.'), 'verify');
  if (!$mya->co) formChangePassword_validate($form, $sta);
}

function formVerify_submit($form, &$sta) {
  global $mya;
  if (!$mya->co) formResetPassword_submit($form, $sta);
  return w\goNextStep('verify');
}