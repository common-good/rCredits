<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits as r;

/**
 * @file
 * Forms for managing transactions, and related functions
 */

/**
 * List Transactions
 */
function formTxs($form, &$sta, $argString = '') {
  $cuid = r\acct()->id;
  global $base_url;

//  $args = explode('&', $args);
//  foreach ($args as $one) list ($keys[], $values[]) = explode('=', "$one=");
//  extract(u\just('real period do zxid', array_combine($keys, $values)));
  parse_str($argString, $args);
  extract(u\just('real period do zxid', $args)); 
  $real = r\isRTrader() ? ((bool) retSet($real) ?: TRUE) : FALSE;
  $period = (int) (@$period ?: 30);
  $do = in_array(@$do, u\ray('ok no edit')) ? $do : '';
  $zxid = (int) (@$zxid ?: 0);

  //  if ($confirm = sure_form($sta)) return $confirm;
  if ($do == 'edit') return tx_edit($zxid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm = txConfirm($do, $zxid, $sta)) return $confirm;
    if (!$sta['rebuild']) txOKorNO($do, $zxid); // don't do anything if canceling
    r\go("transactions/real=$real&period=$period"); // back to the normal form
  }

  $tx_days = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  $ago = @$tx_days[$period] ?: ($period . ' days');
  $ending = time();
  $starting = strtotime($period == '-1' ? '1jan' : "$ago ago", $ending);
  $can_sell = be\access('sell');
  $can_buy = be\access('buy and sell');

  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'denied', TX_DISPUTED => 'disputed');
  $colors = array('&#10004;' => 'green', 'ok?' => 'orange', 'pending' => 'darkgreen', 'denied' => 'red', 'disputed' => 'red');

  $records = be\myTransactions(compact(u\ray('starting ending real'))); // we use $rows for another purpose, below
  $rows = array();
  foreach ($records as $record) {
    extract($record, EXTR_PREFIX_ALL, 'one');
    $tid = be\zxid2tid($one_zxid);
    $data = unserialize($one_data);
    $is_reward = ($one_type != TX_TRANSFER);
    $to_me = ($one_payee == $cuid);
    $by_me = ($one_taking == $to_me);
    $reward = @$data[$to_me ? 'bonus' : 'rebate'];
    $reward = $reward ? number_format($reward, 2) : R_NONE;
    $amount = number_format($one_amount, 2);
    if ($is_reward) list ($amount, $reward) = array(R_NONE, $amount); // show inflation, grants, fines, and signup bonus as rewards
    list ($to_you, $from_you) = $to_me ? array($amount, R_NONE) : array(R_NONE, $amount);
    $state = ($one_state == TX_PENDING and $by_me) ? 'pending' : $states[$one_state];
//    debug("taking=$one_taking to=$to_me by=$by_me state=$state");
    $color = $colors[$state];
    $state = "<span style='color:$color;'>$state</span>";
    $button1 = (!$by_me and $one_state == TX_PENDING) ? button('OK', "$base_url/transactions/real=$real&period=$period&do=ok&zxid=$one_zxid", tx_action('ok', $one_state, $to_me, $by_me)) : '';
    $button2 = (!$is_reward and !@$data['undone']) ? button(' X ', "$base_url/transactions/real=$real&period=$period&do=no&zxid=$one_zxid", tx_action('no', $one_state, $to_me, $by_me)) : '';
    $button3 = (($one_taking ? $can_sell : $can_buy) and !$is_reward) ? "<a href='$base_url/transactions/real=$real&period=$period&do=edit&zxid=$one_zxid' title='Edit Purpose'><img src='$base_url/images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    $rows[] = array($tid, strftime('%d-%b', $one_created), $one_name, $from_you, $to_you, $state, $buttons, $one_purpose, $reward);
  }

  $title = "<div id='txs-title'>TRANSACTIONS</div>";
  $classes = u\prefixValues('tx-', u\ray('tid date name fromyou toyou state buttons purpose reward'));
  $attributes = array('id' => 'txlist');
  $header = u\ray('#,Date,Name,From you,To you,Status,,Purpose,Rewards');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));

  $form = u\prefixKeys('tx_', array(
    'period' => formField('select', t('Show transactions for the past'), '', dft($period), $tx_days),
    'real' => !r\isRTrader() ? formField('hidden', FALSE) 
      : formField('radios', t('Show:'), '', dft($real) + array(u\getConstants(t('Show :R_ASIF_TEXT Transactions')), t('Show Real Transactions'))),
    'show' => formField('submit', NULL, t('Submit button'), 'Show'),
    'do' => formField('hidden', $do),
    'summary' => formField('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => formField('markup', '', '', array('markup' => $title . $list . '<br>&nbsp;')),
  ));
  return $form;
}

function txConfirm($do, $zxid, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return FALSE;
  if (!($details = be\lastTx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract(r\txReportArgs($details), EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_to_me, $my_by_me);
  confirm("Are you sure you want to $action?<br>($my_summary)", $sta); // sets 'confirm'
  return sureForm($sta);
}

/**
 * Return the action option for a transaction
 * (This has to match what be\undoTx() actually does.)
 * @param string $do: which major type of action ('ok' or 'no')
 * @param int $state: the current transaction state
 * @param bool $to_me: user is the payee
 * @param bool $by_me: user originated the transaction
 * @return string: how to describe the ok or no
 */
function tx_action($do, $state, $to_me, $by_me) {
  if ($do == 'ok') return $to_me ? t('ACCEPT this payment') : t('APPROVE this charge');
  
  if ($to_me) {
    if ($state == TX_DISPUTED) return t('REVERSE this disputed transaction'); // let the customer win
    if ($state == TX_PENDING) return $by_me ? t('CANCEL this invoice') : t('REFUSE this payment');
    return t('REVERSE this charge'); // let the customer win
  } else {
    if ($state == TX_PENDING) return $by_me ? t('CANCEL this payment') : t('DECLINE to pay this invoice');
    return $by_me ? t('REVERSE this payment') : t('DISPUTE this charge');
  }
}

function formTxs_validate($form, &$sta) {
/*
  if ($op == 'Go') {
    extract($sta['values']);
    if (no_selection($tx_transactions)) return say('nothing selected');
    if (confirming_v($sta)) return;
    if ($tx_task != 'edit') confirm("Are you sure you want to $tx_task the selected transactions?", $sta);
  }
*/
}

function formTxs_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  extract($sta['values']);
  if ($op == 'Show') $sta['redirect'] = "transactions/real=$tx_real&period=$tx_period";
}

function txOKorNO($tx_task, $zxid) {
  $cuid = r\acct()->id;
  if (!($result = be\lastTx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract($result, EXTR_PREFIX_ALL, 'my');
//  $atid = be\zxid2tid($zxid);
//  $xname = "tx_transactions[x$zxid]";
  if ($tx_task == 'ok') {
// (no. allow pay-all and external transaction confirmation)    assert($my_payer == $cuid);
    u\EXPECT($my_state == TX_PENDING);
    if ($my_to_me) {
      r\setTxState(TX_DONE, $zxid); // accept the payment
    } else {
      list ($message, $args) = be\payInvoice($zxid);
      say($message, $args);
    }
  }
  if ($tx_task == 'no') {
    list ($message, $args) = be\undoTx($zxid);
    say($message, $args);
  }
}

function tx_edit($zxid) {
  if (!($details = be\lastTx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract(u\just('taking purpose role', r\txReportArgs($details)));
  
  if (!be\access($taking ? 'sell' : 'buy and sell')) return FALSE;
  
  $form = array(
    'title' => formField('item', t('Edit Description')),
//    'review' => formField('item', $tx_summary),
    'purpose' => formField('textfield', t('Purpose'), '', dft($purpose)),
    'go' => formField('submit', NULL, t('Submit button'), 'Update'),
    'zxid' => formField('hidden', $zxid),
    'field' => formField('hidden', $role . '_for'),
    
    '#validate' => array('rCredits\\Web\\tx_edit_validate'),
    '#submit' => array('rCredits\\Web\\tx_edit_submit'),
  );
  return $form;
}

function tx_edit_validate($form, &$sta) {
}

function tx_edit_submit($form, &$sta) {
  extract($sta['values']);
  r\setTxField($field, $purpose, $zxid);
  $sta['redirect'] = 'transactions';
}

function txBal($bal, $plus = '', $minus = '- ') {return ($bal < 0 ? $minus : $plus) . '$' . number_format(abs($bal), 2);}

function tx_summary($starting, $ending) {
  $cuid = r\acct()->id;
  $previous_ending = strtotime('-1 days', $starting);
  extract($now_info = (array) be\creditInfo(array('asof' => $ending)), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) be\creditInfo(array('state' => TX_PENDING, 'asof' => $ending)), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (u\ray('gross_in_proper gross_out rewards') as $key) {
    $dif[$key] = number_format($now_info[$key] - $old_info[$key], 2);
    $pfancy[$key] = number_format($pending_info[$key], 2);
  }
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pfancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = strftime('%d-%b-%Y', $starting);
  $ending = strftime('%d-%b-%Y', $ending);
  $pending_balance = txBal($pending_gross_in_proper - $pending_gross_out + $pending_rewards, '+ ');
  $now_balance = txBal($now_balance);
  $old_balance = txBal($old_balance);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards balancesign balance'));
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
