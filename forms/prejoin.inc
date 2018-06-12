<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Create a joint account with a new or existing member.
 */
function formPrejoin($form, &$sta) {
  $title = t('Create a Joint Account');
  $subtext = t('If the person you want to join with does not yet have a %PROJECT account, you will need to go through most of the same steps as setting up a separate account for them. We will guide you through those steps.');
  $old = boolFld(t('Already member?'), t('Is the person you want to join your account with already a %PROJECT member?'), TRUE);
  $account = textFld(t('Account:'), [t('Other Account ID'), t('Type the account ID of the account you want to join with.')]);
  $submit = t('Go');

  jsx('prejoint');
  
  return cgform(compact(ray('title subtext old account submit')));
}

function formPrejoin_validate($form, &$sta) {
  global $mya;
  if ($mya->jid or $mya->co) return w\hack(t('prejoin to already joined or co'));
  
  extract(just('old account', $sta['input']));
  if ($old) {
    if (!$account) return say(t('You must give the other person\'s account ID.'), 'account');
    if (!$oldA = r\acct(trim(strtoupper($account))) or !$oldA->ok) return say('not an account', 'account');
    if ($oldA->co) return say(t('You cannot join a company account.'), 'account');
    $sta['input']['account'] = $oldA->id;
  }
}

function formPrejoin_submit($form, &$sta) {
  global $mya;
  extract(just('old account', $sta['input']));
  if (!$old) return w\go('signup/joint=1');

  if (!$reid = r\relation('reid', $mya->id, $account)) $reid = $mya->newRelation(ray('other permission', $account, r\perm(B_JOINT)));
  $msg = r\setJoin($reid, TRUE);
  return w\go('', $msg);
}