<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Change from existing password to a new one.
 * @todo: refactor this with formResetPassword
 */
function formChangePassword($form, &$sta) {
  global $mya;
  $title = t('Choose a New Password');
  $subtext = t('Mixed-case, digits, punctuation, and plain old length make your password strong.');
  if (!$mya->cAdmin) $pass = passFld(t('Current (old) Password:'), '', required());
  $submit = t('Save');
  return cgform(compact('title', 'subtext') + w\pickPassword() + compact('pass', 'submit'));
}

/**
 * Check new password strength, match with second typing, and validity of existing password, if any.
 * ("if any" because this function is called by formResetPassword_validate() and formVerify_validate)
 */
function formChangePassword_validate($form, &$sta) {
  global $mya;
  extract(just('pass1 pass2 strong pass', $sta['input']));

  if (!$mya or !$mya->admin) {
    if (!$strong and u\weakPass(@$pass1)) return say('weak pass', 'pass1');
    if (isset($pass) and !$mya->passwordOkay($pass, 'pass', $err)) return say($err, 'pass');
  }
  if ($pass1 != $pass2) return say('mismatch', ['thing' => t('password')], 'pass1');
}

function formChangePassword_submit($form, &$sta) {
  extract($info = just('pass1', $sta['input']));
  global $mya;
  $mya->update('pass', $pass1);
  say(t('Your new password has been saved.'));
  return w\go('/settings/security');
}