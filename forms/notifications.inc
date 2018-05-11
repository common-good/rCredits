<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Choose how often to send each type of notification.
 */
function formNotifications($form, &$sta) {
  global $notifyWhens, $notifyNowBys;
  
  global $mya;
  $title = t('Notifications');
  $subtext = t('How often do you want to hear from us about these various things?');
  
  $events = [
    t('explicit payment/transfer from your account'),
    t('automatic payment from your account'),
    t('automatic transfer into your account initiated'),
    t('transfer into your account completed'),
    t('payment received'),
    t('inflation adjustment received'),
    t('other incentive reward received'),
    t('fine, grant, loan, or investment received'),
    t('transaction/invoice edited, canceled, or disputed'),
//    t('unpaid invoice notice'),
    t('your invitee needs a nudge'),
    t('suggestion from the system'),
  ];

  $whens = ray($notifyWhens);
  $alwaysNow = [
    t('invoice received'),
    t('password reset request'),
    t('request to join with another account'),
    t('invitation/trust confirmation request'),
    t('remember to finish setting up your account'),
    t('account approval'),
    t('unusual activity'),
    t('you connected a bank account'),
  ];
  
  $textColW = 12 - count($whens); // width of event text
  $defaults = $mya->notices ?: N_DEFAULTS;
  
  list ($whenI, $cols) = [0, ''];
  foreach ($whens as $k => $v) {
    $cols .= "<div class=\"col-xs-1\"><div class=\"vertical\">$v</div></div>";
    $whenI++;
  }  
  $table = <<<EOF
<div class="row header"><div class="col-xs-$textColW"></div>$cols</div>
EOF;
  
  for ($eventI = 0; $eventI < count($events); $eventI++) {
    $event = $events[$eventI];
    list ($whenI, $cols) = [0, ''];
    foreach ($whens as $k => $v) {
      $checked = $defaults[$eventI] == $k ? 'checked="checked"' : '';
      $input = <<<EOF
<input type="radio" id="edit-e$eventI-$k" name="e$eventI" value="$whenI" class="form-radio" $checked />
EOF;
      $cols .= "<div class=\"col-xs-1\">$input</div>";
      $whenI++;
    }
    $parity = $eventI % 2 ? 'even' : 'odd';
    $table .= <<<EOF
<div class="row $parity"><div class="col-xs-$textColW">$event</div>$cols</div>
EOF;
  }
  $table = item($table);
  
/*
    t('message from another member'),
    t('%PROJECT community newsletter'),
*/
  $methods = ray($notifyNowBys);
//  $method = radiosFld(t('Immediate by:'), t('Receive immediate messages by text, email, or both?'), 1, $methods);
  return cgform(compact(ray('title subtext table')));
}
