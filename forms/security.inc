<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get the member's security info.
 * @param string $arg: "gotPhoto" if just sucessfully uploaded a photo
 */
function formSecurity($form, &$sta, $arg = '') {
  global $base_url;
  global $mya;

  if ($arg == 'gotPhoto') say('got photo');

//  button(t('password'), "$base_url/settings/security/change-password", '', 'warning'), t('Change:'), ['', $mya->ok ? t('triggers new card', 'field', 'photo') : ' ']);
  $btns = btn("$base_url/settings/security/change-password", t('password'), 'warning')
  . ' ' . btn("$base_url/settings/photo", t('photo'), 'warning');
  
  $title = t('Security Information');
  $change = item($btns, t('Change:'), ['', $mya->ok ? t('triggers new card', 'field', 'photo') : ' ']);
  
  $disAdmin = ($mya->cAdmin and !$mya->proSe) ? disabled() : [];
  $questionDesc = t('Type a question no one but you could guess the answer to.');
  $question = $mya->co ? NULL : textFld(t('Question:'), [t('Choose a security question'), $questionDesc], required($mya->question) + $disAdmin);
  $answer = $mya->co ? NULL : textFld(t('Answer:'), [t('The answer to that question')], required(@$mya->answer) + $disAdmin);

  $submit = submit();
  $form = compact(ray('title change question answer submit'));

  return cgform($form);
}

function formSecurity_validate($form, &$sta) {
  global $mya;
//  extract(just('', $sta['input']));
}

function formSecurity_submit($form, &$sta) {
  global $mya;
  $info = just('question answer', $sta['input']);

  foreach ($info as $k => $v) if ($v == R_ON_FILE or $v == @$form[$k]['#default_value']) unset($info[$k]); // don't change if on file or unchanged

  if (!empty($info)) {
    if (!$mya->update($info)) return say('info not saved', 'ERR');
  }

  say('info saved');
}