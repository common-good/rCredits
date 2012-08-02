<?php
namespace rCredits\Web;
use \rCredits as r;
/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

$GLOBALS['Web messages'] = array(
  'missing what' => 'You must say what goods or services are being purchased. If this is a',
);

function account_form($form, &$form_state) {
  $accounts = array(0 => 'Primary Account ... $347.12', 1 => 'whatever'); // select from accounts table
  $prompt = (count($accounts) > 1) ? t('(Select an account)') : '';
  $form = r\prefix_keys('account_', array(
    'description' => form_field('item', t('Account Selector')),
    'account' => form_field('select', t('Current Account:'), $prompt, array('default_value' => 0), $accounts),
  ));
  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  return $form;
}
   
function pay_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = r\prefix_keys('payment_', array(
    'description' => form_field('item', t('Payment')),
    'amount' => form_field('textfield', '$', t('Amount to pay')),
    'who' => form_field('textfield', 'to:', t('Pay whom?')),
    'type' => form_field('radios', 'for:', '', array(), array(t('cash/credit'), t('gift'), t('goods & services'))),
    'what' => form_field('textfield', '...', t('Description of goods and services')),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Pay'),
  ));
  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  return $form;
}

function charge_form($form, &$form_state) {
  $form = r\prefix_keys('charge_', array(
    'description' => form_field('item', t('Charge / Invoice')),
    'amount' => form_field('textfield', '$', t('Amount to charge')),
    'who' => form_field('textfield', 'from:', t('Charge whom?')),
    'what' => form_field('textfield', 'for:', t('Description of goods and services')),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Charge'),
  ));
  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  return $form;
}

function exchange_form($form, &$form_state) {
  $form = r\prefix_keys('exchange', array(
    'description' => form_field('item', t('Cash Exchange')),
    'amount' => form_field('textfield', '$', t('Amount to exchange')),
    'way' => form_field('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
    'who' => form_field('textfield', 'to/from:', t('Exchange with whom?')),
    'exchange' => form_field('submit', NULL, t('Submit button'), 'Exchange'),
  ));
  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  return $form;
}

function manage_form($form, &$form_state) {
  $form = r\prefix_keys('manage_', array(
    'description' => form_field('item', t('Manage')),
    'transactions' => form_field('submit', NULL, t('Submit button'), 'Manage Transactions'),
    'account' => form_field('submit', NULL, t('Submit button'), 'Manage Account'),
    'sharing' => form_field('submit', NULL, t('Submit button'), 'Share Account'),
  ));
  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  return $form;
}

function form_field($type, $title, $description = '', $other = array(), $options = NULL, $ajax = array()) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  $field = r\prefix_keys('#', array_merge(compact(r\ray('type title description options')), $other));
  return $field;
}

function pay_form_validate($form, &$form_state) {
  extract($form_state['values']); //, EXTR_PREFIX_ALL, 'my');
  if(isset($form_state['confirm'])) {
    if ($op == 'Cancel') {
      $form_state = $form_state['confirm_form'];
      unset($form_state['confirm']);
      $form_state['rebuild'] = TRUE;
      \drupal_set_message('', 'Operation Canceled.'); // not an error message, else confirmation form persists
    }
    return; // don't validate the confirmation button push
  }
  $form_state['confirm'] = transfer('payment', $pay_who, $pay_amount, $pay_what, FALSE);
}

function pay_form_submit($form, &$form_state) {
  if (isset($form_state['confirm'])) {
    $form_state['rebuild'] = TRUE; // along with this
    return;
  }

  extract($form_state['values']); //, EXTR_PREFIX_ALL, 'my');
  transfer('payment', $pay_who, $pay_amount, $pay_what, TRUE);
}

/**
 * Transfer funds
 * @return confirmation message (NULL if confirmation not appropriate)
 */
function transfer($type, $who, $amount, $what, $confirmed) {
  global $user;
  if (!is_numeric($amount)) {\form_set_error($type . '_amount', t('Amount must be a number (no commas).')); return NULL;}
  if (!($other_uid = r\uid_from_crid($who))) {\form_set_error($type . '_who', tt('unknown user', compact('who'))); return NULL;}

  list ($message, $args, $confirm) = r\transfer($type, $user->uid, $other_uid, $amount, $what, $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (substr($message, 0, 6) == 'report') {\drupal_set_message(tt($message, $args)); return NULL;}
  {\form_set_error($type . '_amount', tt($message, $args)); return NULL;}
}

/**
 * Replacement for \confirm_form
 */
function sure_form(&$form_state, $title = 'Please Confirm') {
  if (!isset($form_state['confirm'])) return FALSE;
  
  $form_state['confirm_form'] = $form_state;
  $form = array(
    'description' => form_field('item', $title),
    'question' => form_field('item', NULL, $form_state['confirm']),
    'confirm' => form_field('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => form_field('submit', NULL, t("Don't do it"), 'Cancel'),
  );
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => 'confirmation');

  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

function charge_form_validate($form, &$form_state) {return pay_form_validate($form, $form_state);}
function exchange_form_validate($form, &$form_state) {return pay_form_validate($form, $form_state);}

function charge_form_submit($form, &$form_state) {return pay_form_submit($form, $form_state);}
function exchange_form_submit($form, &$form_state) {return pay_form_submit($form, $form_state);}

// one-line functions that need no explanation
function block_def($info, $cache = DRUPAL_CACHE_PER_ROLE) {return compact(r\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(r\ray('subject content'));}
function tt($message, $args) {return r\SMS\tt($message, $args);}