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
 * @todo: restrict how much can be displayed on screen (avoid memory overflow)
 */
function formTxs($form, &$sta, $args = '') {
  global $base_url;
  $cacct = r\acct();
  $cuid = $cacct->id;

  extract(u\just('real agents channels period starting ending do zxid download downloaded', $args)); 
  $do = in_array(@$do, u\ray('ok no edit')) ? $do : '';
  $zxid = (int) (@$zxid ?: 0);

  //  if ($confirm = sure_form($sta)) return $confirm;
  if ($do == 'edit') return tx_edit($zxid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm = txConfirm($do, $zxid, $sta)) return $confirm;
    if (!$sta['rebuild']) txOKorNO($do, $zxid); // don't do anything if canceling
    $url = str_replace('&do=', '&did=', "transactions/$args"); // don't do it again
    r\go($url); // back to the normal form
  }

  $real = r\isRTrader() ? (isset($real) ? $real : TRUE) : FALSE;
  $txDays = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  list ($starting, $ending) = txDates($txDays, @$period, @$starting, @$ending);
  list ($canSell, $canBuy) = array($cacct->can(PERM_SELL), $cacct->can(PERM_BUY));

  $txChannels = u\ray(TX_CHANNELS);
  $txStates = u\ray(TX_STATES);
  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'denied', TX_DISPUTED => 'disputed');
  $colors = array('&#10004;' => 'green', 'ok?' => 'orange', 'pending' => 'darkgreen', 'denied' => 'red', 'disputed' => 'red');
  $url = @"$base_url/transactions/real=$real&period=$period&zxid=$one_zxid&agents=$agents&channels=$channels";
  $title = "<div id='txs-title'>TRANSACTIONS</div>";
  $classes = 'tid date name fromyou toyou state buttons purpose reward';
  $headers = t('#,Date,Name,From you,To you,Status,Action,Purpose,Reward');
  if (@$agents) {$classes .= ' agent'; $headers .= t(',Agent');}
  if (@$channels) {$classes .= ' channel'; $headers .= t(',Chan');}
  $header = u\ray($headers);

  if (@$download) {
    header('Content-type: text/plain');
    $dt1 = strftime('%Y%m%d', $starting);
    $dt2 = strftime('%Y%m%d', $ending);
    $asif = $real ? '' : 'asif';
    $filename = "rcredits$dt1-$dt2$asif.csv";
    header("Content-Disposition: attachment; filename=\"$filename\"");
    $csv = fopen('php://output', 'w');
    array_splice($header, 6, 1);
    fputcsv($csv, $header);
  }

  $records = be\myTransactions(compact(u\ray('starting ending real'))); // we use $rows for another purpose, below
  $rows = array();
  foreach ($records as $record) {
    extract($record, EXTR_PREFIX_ALL, 'one');
    $tid = be\zxid2tid($one_zxid);
    $data = unserialize($one_data);
    $is_reward = ($one_type != TX_TRANSFER);
    $toMe = ($one_payee == $cuid);
    $byMe = ($one_taking == $toMe);
    $agent = $toMe ? $one_payee_agent : $one_payer_agent;
    $reward = @$data[$toMe ? 'bonus' : 'rebate'];
    $reward = $reward ? number_format($reward, 2) : R_NONE;
    $amount = number_format($one_amount, 2);
    if ($is_reward) list ($amount, $reward) = array(R_NONE, $amount); // show inflation, grants, fines, and signup bonus as rewards
    list ($to_you, $from_you) = $toMe ? array($amount, R_NONE) : array(R_NONE, $amount);
    $state = ($one_state == TX_PENDING and $byMe) ? 'pending' : $states[$one_state];
//    debug("taking=$one_taking to=$toMe by=$byMe state=$state");
    $color = $colors[$state];
    $state = "<span style='color:$color;'>$state</span>";
    $button1 = (!$byMe and $one_state == TX_PENDING) ? button('OK', "$url&do=ok", tx_action('ok', $one_state, $toMe, $byMe)) : '';
    $button2 = (!$is_reward and !@$data['undone']) ? button(' X ', "$url&do=no", tx_action('no', $one_state, $toMe, $byMe)) : '';
    $button3 = (($one_taking ? $canSell : $canBuy) and !$is_reward) ? "<a href='$url&do=edit' title='Edit Purpose'><img src='$base_url/sites/all/modules/rcredits/images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    $row = array($tid, strftime('%d-%b', $one_created), $one_name, $from_you, $to_you, $state, $buttons, $one_purpose, $reward);
    if (@$agents) $row[] = ($agent == $cuid or $agent < 0) ? R_NONE : (@$download ? r\qid($agent) : u\n2a($agent, 3));
    if (@$channels) $row[] = $txChannels[$one_channel];
    if (@$download) {
      array_splice($row, 6, 1); // remove buttons
      $row[1] = strftime('%Y-%m-%d', $one_created); // date created
      $row[5] = $txStates[$one_state]; // transaction state
      foreach ($row as $key => $value) $row[$key] = $value == R_NONE ? '' : strip_tags($value);
      fputcsv($csv, $row); // not STDOUT
    } else $rows[] = $row;
  }
  if (@$download) exit();

  $classes = u\prefixValues('tx-', u\ray($classes));
  $attributes = array('id' => 'txlist');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));

  $form = array(
    'title' => item(t('Transaction History')),
    'agents' => formField('checkbox', t('Show:'), t('agents'), @$agents ?: dft($cacct->can(PERM_COMPANY))),
    'channels' => formField('checkbox', '', t('channels'), dft(@$channels)), 
    'real' => !r\isRTrader() ? formField('hidden', FALSE) 
      : formField('radios', t('Show:'), '', dft($real), array(R_ASIF_TEXT . t(' Transactions'), t('Real Transactions'))),
    'period' => formField('select', t('For the past:'), '', dft(@$period), $txDays),
    'submitPeriod' => submit(t('Show ')), // label has to be different from submitDates (see below)
    'downloadPeriod' => submit(t('Download')),
    'starting' => formField('textfield', t('OR Starting:'), '', dft(u\formatDate($starting))),
    'ending' => formField('textfield', t('Ending:'), '', dft(u\formatDate($ending))),
    'submitDates' => submit(t(' Show')), // label has to be different (Drupal bug -- else can't tell which is clicked)
    'downloadDates' => submit(t('Download')),
    'do' => formField('hidden', $do),
    'summary' => formField('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => formField('markup', '', '', array('markup' => $title . $list . '<br>&nbsp;')),
  );
  if (@$downloaded) js("window.open(\"$url&download=1\")", 'inline', 'footer');
  return labeled($form);
}

function txConfirm($do, $zxid, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return FALSE;
  if (!($details = be\lastTx('xid=:zxid', compact('zxid'), TRUE))) return FALSE; // bail on hack attempt
  extract(r\txReportArgs($details), EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_toMe, $my_byMe);
  confirm(t('Are you sure you want to ') . "$action?<br>($my_summary)", $sta); // sets 'confirm'
  return sureForm($sta);
}

function formTxs_validate($form, &$sta) {
  $txDays = $form['period']['#options'];
  extract(u\just('real period starting ending op', $sta['values']));
  list ($sta['values']['starting'], $sta['values']['ending']) = strpos(op($sta), 'Period') ? txDates($txDays, $period) : txDates($txDays, '', $starting, $ending);
}

function formTxs_submit($form, &$sta) {
  global $base_url;
  if (confirming_s($sta)) return;
  extract(u\just('real period starting ending agents channels op', $sta['values']));
  $url = "transactions/real=$real&period=$period&starting=$starting&ending=$ending&agents=$agents&channels=$channels";
  if ($op == t('Download')) {
    say('downloading');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

function txDates($txDays, $period, $starting = '', $ending = '') {
  $period = (int) (@$period ?: 30);
  $ago = @$txDays[$period] ?: ($period . ' days');
  if (@$ending and !is_numeric($ending) and !($ending = strtotime($ending))) say('bad date', 'starting');
  if (!@$ending) $ending = time();
  $ending = strtotime('tomorrow', $ending) - 1;
  if (@$starting and !is_numeric($starting) and !($starting = strtotime(@$starting))) say('bad date', 'starting');
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : "$ago ago");
  return array($starting, $ending);
}  

/**
 * Return the action option for a transaction
 * (This has to match what be\undoTx() actually does.)
 * @param string $do: which major type of action ('ok' or 'no')
 * @param int $state: the current transaction state
 * @param bool $toMe: user is the payee
 * @param bool $byMe: user originated the transaction
 * @return string: how to describe the ok or no
 */
function tx_action($do, $state, $toMe, $byMe) {
  if ($do == 'ok') return $toMe ? t('ACCEPT this payment') : t('APPROVE this charge');
  
  if ($toMe) {
    if ($state == TX_DISPUTED) return t('REVERSE this disputed transaction'); // let the customer win
    if ($state == TX_PENDING) return $byMe ? t('CANCEL this invoice') : t('REFUSE this payment');
    return t('REVERSE this charge'); // let the customer win
  } else {
    if ($state == TX_PENDING) return $byMe ? t('CANCEL this payment') : t('DECLINE to pay this invoice');
    return $byMe ? t('REVERSE this payment') : t('DISPUTE this charge');
  }
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
    if ($my_toMe) {
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
  
  if (!r\acct()->can($taking ? PERM_SELL : PERM_BUY)) return FALSE;
  
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
  $previous_ending = $starting - 1;
  extract($now_info = (array) be\creditInfo(array('asof' => $ending)), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) be\creditInfo(array('state' => TX_PENDING, 'asof' => $ending)), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (u\ray('grossInProper grossOut rewards') as $key) {
    $dif[$key] = number_format($now_info[$key] - $old_info[$key], 2);
    $pfancy[$key] = number_format($pending_info[$key], 2);
  }
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pfancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = u\formatDate($starting);
  $ending = u\formatDate($ending);
  $pending_balance = txBal($pending_grossInProper - $pending_grossOut + $pending_rewards, '+ ');
  $now_balance = txBal($now_balance);
  $old_balance = txBal($old_balance);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Starting balance,,To You,,From You,,Rewards,,Ending balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old_balance | + | $dif_grossInProper | - | $dif_grossOut | + | $dif_rewards | = | $now_balance"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>PENDING:</b> | + | $pfancy_grossInProper | - | $pfancy_grossOut | + | $pfancy_rewards | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}
