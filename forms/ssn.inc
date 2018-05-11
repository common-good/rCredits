<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Ask new member to confirm their SSN and/or birthdate by typing it again. (sometimes with no-signin link)
 */
function formSsn($form, &$sta, $args = '') {
  $mya = w\eLinkAcct($sta, $args, 'acct');
  $ssnData = $mya->ssnData;
  $badBday = (@$ssnData[SSN_ERR_INDEX] == t('bad birthdate'));
  
  $thing = $badBday ? t('Birth Date') : ($mya->co ? t('EIN') : t('Social Security Number'));
  $title = t('Confirm Your ') . $thing;
  $subtext = t('Our 3rd party service was unable to verify your %thing automatically. Please type it again (carefully) in case there was a typo.', 'thing', strtolower($thing));
  $submit = t('Submit');
  $field = hidFld($badBday ? 'dob' : 'federalId');
  $form = compact(ray('title subtext field')) + ssnFields(!$mya->co) + compact('submit');
  if ($badBday) unset($form['federalId']); else unset($form['dob']);
  return cgform($form);
}

function formSsn_validate($form, &$sta) {
  $mya = w\eLinkAcct($sta);
  extract(just('field', $sta['input']));
  return formBasic_validate($form, $sta, FALSE, $field);
}

function formSsn_submit($form, &$sta, $args = '') {
  $mya = w\eLinkAcct($sta);
  extract(just('field', $sta['input']));

  extract(just($field, $sta['input']));
  if ($same = ($mya->$field == $$field)) $mya->setRisk($field == 'dob' ? 'dobOff' : 'ssnOff');
  $mya->update(compact($field));
//  r\tellAdmin('new ' . $field, ray('uid same', $mya->id, $same ? t('Yes') : t('No')));
  return r\acct() ? w\goNextStep('ssn') : doSay(t('info saved|setup complete'), 'ok'); // r\acct not $mya!
}