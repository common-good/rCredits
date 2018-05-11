<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Send another member a message.
 */
function formMessage($form, &$sta) {
  $title = t('Send a Message');
  $subtext = t('Complete this form and click Send. The other member will receive an email from your email address. When they reply, you will also have their email address, so you can communicate by email after that (you won\'t need to use this form again).');
  $to = textFld(t('To:'), [t('Recipient'),t('The account, name, or partial name of the member you want to send a message to.')], required());
  $subject = textFld(t('Subject:'), [t('Subject')], required(t('a message from %PROJECT member ') . r\acct()->fullName));
  $message = areaFld(t('Message:'), [t('Your message goes here.')], required());
  $submit = t('Send');
  w\whoFldSubmit('to', t('Send to %name?'));

  return cgform(compact(ray('title subtext to subject message submit')));
}

function formMessage_validate($form, &$sta) {
  extract($info = just('to subject message', $sta['input']));
  if (strpos($to, '@')) return say(t('If you know the member\'s email address, don\'t use this form -- just send them an email.'), 'to');
  if (!$a = w\whois($to, 'to', $info)) return FALSE;
  u\preray(ray('to fullName', $a->email, $a->fullName), $sta['input']);
}

function formMessage_submit($form, &$sta) {
  extract(just('to subject message fullName', $sta['input']));
  global $mya;
  r\rMail('', $to, ray('noFrame subject body', TRUE, $subject, $message), [$mya->email => $mya->fullName]);
  say('sent message', compact('fullName'));
}
