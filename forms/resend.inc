<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/pass.inc';

/**
 * Give the new member a chance to change email address during signup process.
 */
function formResend($form, &$sta) {
  global $mya;
  $title = t('Resend Verification Email');
  $email = textFld(t('Email:'), [t('Email'), t('Type carefully.')], required($mya->email));
  $submit = t('Resend');
  return cgform(compact(ray('title email submit')));
}

function formResend_validate($form, &$sta) {
  global $mya;
  extract(just('email', $sta['input']));
  if (!emailOkay($email, $mya->co, $mya)) return;
}

function formResend_submit($form, &$sta) {
  global $mya;
  extract(just('email', $sta['input']));
  $mya->update('email', $email);
  return formPass_submit($form, $sta, '/settings/verify');
}