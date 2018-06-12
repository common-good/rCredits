<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

require_once __DIR__ . '/changepassword.inc';
include_once __DIR__ . '/signin.inc';


/**
 * Reset password by choosing a new one from an official email link.
 */
function formResetPassword($form, &$sta, $args = '') {
  extract(just('id code', $args)); // linked from an email

  if (flooding($uid = r\loginString(@$id), $sta, $err)) return w\softErr($err); // getting hammered
  $mya = r\acct($uid);

  if (!$mya->passwordOkay(@$code, 'pass', $err)) return w\softErr($err);
  noFlood($sta);
  
  $title = t('Choose a New Password');
  $subtext = t('Your password must have at least 8 characters, including one uppercase, one lowercase, one digit, and one punctuation mark.');
  $uid = hidFld($uid);
  $submit = t('Save');
  return cgform(compact('title', 'subtext') + w\pickPassword() + compact(ray('submit uid')));
}

function formResetPassword_validate($form, &$sta) {return formChangePassword_validate($form, $sta);}

function formResetPassword_submit($form, &$sta) {
  extract(just('pass1 uid', $sta['input']));
  $mya = r\acct(@$uid); // defaults to current account (called from formVerify_submit)
  $mya->update('pass oneTimePass', $pass1, '');
  $msg = 'pass saved';
  if (!$mya->stepsDone['verify']) {
    $msg .= '|step completed';
    $mya->stepDone('verify');
  }
  if (@$uid) { // no uid and no signin if coming from formVerify_submit
    $sta += compact('uid'); // _submit requires call by reference
    formSignin_submit('', $sta); // lead user to next step
  }
  say($msg); // must postcede formSignin_submit
}
