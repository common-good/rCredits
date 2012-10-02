<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use \rCredits\SMS as rSMS;
use \rCredits\API as api;
use \rCredits\Utilities as u;
use \rCredits as r; // get rid of this

/**
 * @file
 * Forms for managing transactions, and related functions
 */

/**
 * List Transactions
 */
function txs_form($form, &$form_state, $period = '', $do = '') {
  $cuid = r\cuid();
  global $base_path;

  list ($real, $period) = $period ? explode('-', $period) : array(r\is_rTrader(), 30);
  if ($real and !r\is_rTrader()) $real = FALSE;
  if ($do) list ($do, $zxid) = explode('=', $do);

  //  if ($confirm = sure_form($form_state)) return $confirm;
  if ($do == 'edit') return tx_edit($zxid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm_form = tx_confirm($do, $zxid, $form_state)) return $confirm_form;
    if (!$form_state['rebuild']) tx_okorno($do, $zxid); // don't do anything if canceling
    drupal_goto("transactions/$real-$period"); // back to the normal form
  }

  $tx_days = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  $ending = time();
  $starting = strtotime($period == '-1' ? '1jan' : ((1 - @$tx_days[$period]) . ' days'), $ending);
  $can_sell = api\access('sell');
  $can_buy = api\access('buy and sell');

  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'denied', TX_DISPUTED => 'disputed');
  $colors = array('&#10004;' => 'green', 'ok?' => 'orange', 'pending' => 'darkgreen', 'denied' => 'red', 'disputed' => 'red');

  $records = api\my_transactions(compact(u\ray('starting ending real'))); // we use $rows for another purpose, below
  $rows = array();
  foreach ($records as $record) {
    extract($record, EXTR_PREFIX_ALL, 'one');
    $tid = api\zxid2tid($one_zxid);
    $data = unserialize($one_data);
    $is_reward = ($one_type != TX_TRANSFER);
    $taking = ($one_payee == $cuid);
    $reward = @$data[$taking ? 'bonus' : 'rebate'];
    $reward = $reward ? number_format($reward, 2) : '';
    $amount = number_format($one_amount, 2);
    if ($is_reward) list ($amount, $reward) = array('', $amount); // show interest and signup bonus as rewards
    list ($to_you, $from_you) = $taking ? array($amount, '') : array('', $amount);
    $state = ($one_state == TX_PENDING and $one_payee == $cuid) ? 'pending' : $states[$one_state];
    $color = $colors[$state];
    $state = "<span style='color:$color;'>$state</span>";
    $button1 = (!$taking and $one_state == TX_PENDING) ? button('OK', "{$base_path}transactions/$real-$period/ok=$one_zxid", tx_action('ok')) : '';
    $button2 = (!$is_reward and !@$data['undone']) ? button(' X ', "{$base_path}transactions/$real-$period/no=$one_zxid", tx_action('no', $one_state, $taking)) : '';
    $button3 = (($taking ? $can_sell : $can_buy) and !$is_reward) ? "<a href='{$base_path}transactions/$real-$period/edit=$one_zxid' title='Edit Purpose'><img src='{$base_path}images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    $rows[] = array($tid, date('d-M', $one_created), $one_name, $from_you, $to_you, $state, $buttons, $one_purpose, $reward);
  }

  $title = "<div id='txs-title'>TRANSACTIONS</div>";
  $classes = u\prefix_values('tx-', u\ray('tid date name fromyou toyou state buttons purpose reward'));
  $attributes = array('id' => 'txlist');
  $header = u\ray('#,Date,Name,From you,To you,Status,,Purpose,Rewards');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));

  $form = u\prefix_keys('tx_', array(
    'period' => form_field('select', t('Show transactions for the past'), '', dft($period), $tx_days),
    'real' => !r\is_rTrader() ? form_field('hidden', FALSE) 
      : form_field('radios', t('Show:'), '', dft($real) + array(u\get_constants(t('Show :R_ASIF_TEXT Transactions')), t('Show Real Transactions'))),
    'show' => form_field('submit', NULL, t('Submit button'), 'Show'),
    'do' => form_field('hidden', $do),
    'summary' => form_field('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => form_field('markup', '', '', array('markup' => $title . $list . '<br>&nbsp;')),
  ));
  return $form;
}

function tx_confirm($do, $zxid, &$form_state) {
  $cuid = r\cuid();
  if (confirming_v($form_state)) return FALSE;
  if (!($details = tx_details($zxid))) return FALSE; // bail on hack attempt
  extract($details, EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_payee == $cuid);
  confirm("Are you sure you want to $action?<br>($my_summary)", $form_state); // sets 'confirm'
  return sure_form($form_state);
}

/**
 * Return the action option for a transaction
 * @param string $do: which major type of action ('ok' or 'no')
 * @param int $state: the current transaction state
 * @param bool $is_payee: user is the payee
 * @return string: how to describe the ok or no
 * NOTE: $state and $is_payee are unused for 'ok'
 */
function tx_action($do, $state = '', $is_payee = '') {
  if ($do == 'ok') return t('Approve this charge');
  
  if ($is_payee) {
    if ($state == TX_DISPUTED) return t('Reverse this transaction'); // let the customer win
    if ($state == TX_PENDING) return t('Cancel this invoice');
    return t('Reverse this transaction'); // let the customer win
  } else return t('Dispute this charge');
}

function txs_form_validate($form, &$form_state) {
/*
  if ($op == 'Go') {
    extract($form_state['values']);
    if (no_selection($tx_transactions)) return say('nothing selected');
    if (confirming_v($form_state)) return;
    if ($tx_task != 'edit') confirm("Are you sure you want to $tx_task the selected transactions?", $form_state);
  }
*/
}

function txs_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  extract($form_state['values']);
  if ($op == 'Show') $form_state['redirect'] = "transactions/$tx_real-$tx_period";
}

function tx_okorno($tx_task, $zxid) {
  $cuid = r\cuid();
  if (!($result = api\last_tx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract($result, EXTR_PREFIX_ALL, 'my');
//  $atid = api\zxid2tid($zxid);
//  $xname = "tx_transactions[x$zxid]";
  if ($tx_task == 'ok') {
    assert($my_payer == $cuid);
    assert($my_state == TX_PENDING);
    list ($message, $args) = api\pay_invoice($zxid);
    say($message, $args);
  }
  if ($tx_task == 'no') {
    list ($message, $args) = api\undo_tx($zxid);
    say($message, array_merge($args));
  }
/*  if ($tx_task == 'edit') {
    if ($my_state != TX_DONE and $my_payer != $cuid) {
      say("atid|can't change what", compact('atid'), $xname);
    } else \drupal_set_message('Editing descriptions is not yet possible');
  }*/
}

function tx_details($zxid) {
  $uid = r\cuid(); // this could be expanded later to handle other users
  if (!($result = api\last_tx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract($result);
  $amount = u\format_amount($amount);
  $taking = ($payee == $uid);
  $tofrom = $taking ? 'from' : 'to';
  $other_uid = $payer == $uid ? $payee : $payer;
  $who = api\user_field('full_name', $other_uid);
  $role = $payer == $uid ? 'payer' : 'payee';
  $purpose = $payer == $uid ? $payer_for : $payee_for;
  $tid = api\zxid2tid($zxid, $uid);
  $created = date('m-d-Y', $created);
  $summary = "Transaction #$tid: $amount $tofrom $who for \"$purpose\" on $created";
  return compact(u\ray('amount tofrom taking who payee role purpose tid created summary state'));
}

function tx_edit($zxid) {
  if (!($details = tx_details($zxid))) return FALSE; // bail on hack attempt
  extract($details, EXTR_PREFIX_ALL, 'my');
  
  if (!api\access($my_taking ? 'sell' : 'buy and sell')) return FALSE;
  
  $form = array(
    'title' => form_field('item', t('Edit Description')),
//    'review' => form_field('item', $tx_summary),
    'purpose' => form_field('textfield', t('Purpose'), '', dft($my_purpose)),
    'go' => form_field('submit', NULL, t('Submit button'), 'Update'),
    'zxid' => form_field('hidden', $zxid),
    'field' => form_field('hidden', $my_role . '_for'),
    
    '#validate' => array('rCredits\\Web\\tx_edit_validate'),
    '#submit' => array('rCredits\\Web\\tx_edit_submit'),
  );
  return $form;
}

function tx_edit_validate($form, &$form_state) {
}

function tx_edit_submit($form, &$form_state) {
  extract($form_state['values']);
  r\set_tx_field($field, $purpose, $zxid);
  $form_state['redirect'] = 'transactions';
}

function tx_bal($bal, $plus = '', $minus = '- ') {return ($bal < 0 ? $minus : $plus) . '$' . number_format(abs($bal), 2);}

function tx_summary($starting, $ending) {
  $cuid = r\cuid();
  $previous_ending = strtotime('-1 days', $starting);
  extract($now_info = (array) api\credit_info(array('asof' => $ending)), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) api\credit_info(array('state' => TX_PENDING, 'asof' => $ending)), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) api\credit_info(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (u\ray('gross_in_proper gross_out rewards') as $key) {
    $dif[$key] = number_format($now_info[$key] - $old_info[$key], 2);
    $pfancy[$key] = number_format($pending_info[$key], 2);
  }
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pfancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = date('d-M-Y', $starting);
  $ending = date('d-M-Y', $ending);
  $pending_balance = tx_bal($pending_gross_in_proper - $pending_gross_out + $pending_rewards);
  $now_balance = tx_bal($now_balance);
  $old_balance = tx_bal($old_balance);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefix_values('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Starting balance,,To You,,From You,,Rewards,,Ending balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old_balance | + | $dif_gross_in_proper | - | $dif_gross_out | + | $dif_rewards | = | $now_balance"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>PENDING:</b> | + | $pfancy_gross_in_proper | - | $pfancy_gross_out | + | $pfancy_rewards | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}
