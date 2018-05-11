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
 */
function formTx($form, &$sta, $args = '') {
//  if ($confirm = sureForm($sta)) return $confirm;
  extract(just('who amount goods purpose', $args));
  global $mya;

  if (!$mya->can(B_SELL)) return r\go('summary');

  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path())); // pay, charge, etc.
  $role = in_array($type, ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, ray('fine grant loan invest'));
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($type == 'charge' ? @$selling[0] : ($type == 'pay' ? '' : $type));
  
  focusOn(@$who ? 'amount' : 'who');

  // set up form fields
  $title = ucwords($type);
  $subtext = $type == 'pay' ? w\btn('/pay/payments', t('Upload Payments'), 'default') : '';
  if ($mya->co) $subtext .= ' ' . ($type == 'pay' 
    ? w\btn('/pay/payroll', t('Upload Payroll'), 'default')
    : w\btn('/charge/invoices', t('Upload Invoices'), 'default')
  );
//    list ($thing, $url) = $type == 'pay' ? [t('Payroll'), '/pay/payroll'] : [t('Invoices'), '/charge/invoices'];
//    $subtext = item('<small>[' . t('<%a>Upload %thing</a>', '_a thing', atag($url), $thing) . ']</small>');
//  }

  $whoLabel = ($type == 'loan' ? t('Lend to') : ucwords($type)) . t(' whom:');
  $question = ($type == 'loan' ? t('Lend') : ucwords($type)) . t(' %amount to %name?'); 
  $who = textFld($whoLabel, [t('name, id, email, or phone')], required(@$who));

  $amount = textFld(t('Amount:'), [t('amount to ') . $type], required(@$amount));
  $goodsOpts = ray(R_WHYS);
  unset($goodsOpts[FOR_SHARE]); // this gets set only automatically
  if ($mya->cttyRewardy) {
    $goods = $incentive ? hidFld(FOR_NONGOODS) : radiosFld(t('For:'), '', dft((int) @$goods), $goodsOpts); // required() fails here
  } else $goods = hidFld($incentive ? FOR_NONGOODS : FOR_GOODS);
  $purpose = textFld($mya->cttyRewardy ? ' ' : t('For:'), [t('purpose or description of goods and services')], required(@$purpose));
// (keep this for a while)  $calc = item(u\calculator('amount'));
  $scanClass = 'not-scanned';
//  $isGift = hidfld(FALSE);
  $$type = submit(ucwords($type));

  w\whoFldSubmit('who', $question, ray('allowNonmember', $type == 'charge'));
  
  $form = cgform(compact(ray("title subtext who amount goods purpose calc $type")));
  $form['#attributes']['class'][] = $scanClass;

  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  extract(just('who', $sta['input']));
  if (strpos($who, '@') and !u\validEmail($email = $who)) return say('bad email', compact('email'), 'who');
}

function formTx_submit($form, &$sta) {
  extract($info = just('who amount goods purpose', $sta['input']));
  if (strpos($who, '@')) return r\go('charge/nonmember/' . u\urlify(serialize($info)));
  w\transfer($op = op($sta), $info); // op who amount goods what
}

/**
 * Transfer funds
 * @param string $op: pay, charge, grant, loan, fine, or invest
 * @param array $info: associative array indexed by field names (who, amount, goods, and purpose)
 * @param boolean $confirmed: UNUSED whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($op, $info) {
  global $mya;
  $myid = $mya->id;
  extract($params = just('who amount purpose goods', $info)); // used to include isGift
  if (!isset($goods) or $goods === '') return say('required field', ['field'=>'"For"'], 'ERR'); // '' is for test
///NO!  if (!in_array($op, ['pay', 'charge'])) $goods = FOR_USD; // make sure this is set, for blank_field check
  $txType = $op == 'pay' ? 'payment' : strtolower($op);
//  if (blank_field($params)) return FALSE;
  if (!$a = w\whois($who, 'who', $info, 'no self-trading', $op != 'charge')) return FALSE;
//  if ($goods == FOR_GOODS and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
//  $op = $op == 'pay' ? t('paid') : ($op == 'charge' ? 'charged' : $op . 'ed'); // for possible error message
//  if (u\forCash($purpose)) $goods = FOR_USD;

  if (@$isGift) { // tag gifts to nonprofits and handle special case of "giving back" rewards to the community
    $data = compact('isGift');
//    if (r\isGiveback($myid, $a->id, $data)) list ($mya, $a, $amount, $goods) = [$a,$mya,-$amount,FOR_NONGOODS];
  } else $data = [];

  list ($msg, $args) = 
     $txType == 'payment' ? $mya->pay($a, $amount, $purpose, $data + compact('goods'))
     : ($txType == 'charge' ? $mya->invoice($a, $amount, $purpose, compact('goods'))
     : communityTransfer($txType, $a, $amount, $purpose));

//  if (!$ok = @$args['success']) return t('confirm ' . $msg, $args);
//  if (!@$args['success']) return say($msg, $args, 'amount');
  return say($msg, $args, @$args['success'] ? '' : 'amount');
}

/**
 * Record a transfer to or from the community.
 * @param int $type: what type of transfer
 * @param acct $acct: the account to credit or debit
 * @param int $amount: how much to transfer
 * @param string $purpose: what's the transfer for
 * @param bool $confirmed: UNUSED has this transaction been confirmed by the user?
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean UNUSED "this needs to be confirmed"
 */
function communityTransfer($type, $acct, $amount, $purpose) {
  $success = (bool) be\fund($acct->id, u\consta('tx', $type), $amount, $purpose);
  $msg = $success ? ($confirmed ? 'report funding' : 'funding') : 'funding error'; // 'funding' not used?!
  $confirmAction = ucwords($type);
  $did = $confirmAction . 'ed';
  $otherName = $acct->fullName;
  $amount = u\fmtAmt($amount);
  return [$msg, compact(ray('success did confirmAction amount otherName'))];
}  
