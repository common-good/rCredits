<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 * NOTE: currently handles only pay and charge
 */
function formTx($form, &$sta, $args = '') {
  extract(just('who amount goods purpose', $args));
  global $mya;

  if (!$mya->can(B_SELL)) return w\go('summary');
  $type = strtolower($_SERVER['REQUEST_URI']);
  $paying = ($type == 'pay');
  
  list ($opDesc, $role) = $paying
  ? [t('Pay'), 'payer']
  : [t('Charge'), 'payee'];
  
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($paying ? '' : @$selling[0]);
  
  focusOn(@$who ? 'amount' : 'who');

  $title = $opDesc;
  $subtext = $paying ? w\btn('/pay/payments', t('Upload Payments'), 'default') : '';
  if ($mya->co) $subtext .= ' ' . ($paying 
    ? w\btn('/pay/payroll', t('Upload Payroll'), 'default')
    : w\btn('/charge/invoices', t('Upload Invoices'), 'default')
  );
  $who = textFld($opDesc . t(' whom:'), [t('name, id, email, or phone')], required(@$who));

  $amount = textFld(t('Amount:'), [t('amount to ') . strtolower($opDesc)], required(@$amount));
  $goodsOpts = ray(R_WHYS);
  unset($goodsOpts[FOR_SHARE]); // this gets set only automatically
  $purpose = textFld(t('For:'), [t('purpose or description of goods and services')], required(@$purpose));
  $scanClass = 'not-scanned';
  $$type = submit($opDesc);

  w\whoFldSubmit('who', $opDesc . t(' %amount to %name?'), ray('allowNonmember', !$paying));
  
  $form = cgform(compact(ray("title subtext who amount goods purpose calc $type")));
  $form['#attributes']['class'][] = $scanClass;

  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  extract(just('who', $sta['input']));
  if (strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
}

function formTx_submit($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract($info = just('who amount goods purpose', $sta['input']));
  if (strpos($who, '@')) return w\go('charge/nonmember/' . u\urlify(serialize($info)));

  $paying = (op($sta) == 'pay');
  if (!$a = w\whois($who, 'who', $info, 'no self-trading', $paying)) return FALSE;
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');

  $func = $paying ? 'pay' : 'invoice';
  list ($msg, $args) = $mya->$func($a, $amount, $purpose, compact('goods'));
  return say($msg, $args, @$args['success'] ? '' : 'amount');
}
