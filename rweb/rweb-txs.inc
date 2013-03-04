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
  $maxRows = 20;
  $cacct = r\acct();
  $cuid = $cacct->id;
  $cttyAdmin = $cacct->can(PERM_CTTY_ADMIN);
  
  extract(u\just('real agents channels period starting ending do zxid download downloaded variety', $args)); 
  $report = $cttyAdmin ? (@$variety == 'report') : FALSE; // no community reports yet for non-admins
  $real = r\isRTrader() ? (isset($real) ? $real : TRUE) : FALSE;
  
  if ($report) { // community transaction history
    $page = 'reports';
    $classes = 'tid date from to amount rpct state purpose reward';
    $headers = t('#,Date,From,To,Amount,r%,Status,Purpose,Rewards');
  } else { // individual account transaction history
    $page = 'transactions';
    $do = in_array(@$do, u\ray('ok no edit')) ? $do : '';
    $zxid = (int) (@$zxid ?: 0);
    if ($do == 'edit') return tx_edit($zxid);
    if ($do == 'ok' or $do == 'no') {
      if ($confirm = txConfirm($do, $zxid, $sta)) return $confirm;
      if (!$sta['rebuild']) txOKorNO($do, $zxid); // don't do anything if canceling
      $url = str_replace('&do=', '&did=', "transactions/$args"); // don't do it again
      r\go($url); // back to the normal form
    }
    $classes = 'tid date name fromyou toyou rpct state buttons purpose reward';
    $headers = t('#,Date,Name,From you,To you,r%,Status,Action,Purpose,Reward');
    if (@$agents) {$classes .= ' agent'; $headers .= t(',Agent');}
    list ($canSell, $canBuy) = array($cacct->can(PERM_SELL), $cacct->can(PERM_BUY));
  }
  $url0 = @"$base_url/$page/real=$real&period=$period&agents=$agents&channels=$channels&variety=$variety";
  if (@$channels) {$classes .= ' channel'; $headers .= t(',Via');}
  $classes = u\ray($classes);
  
  $txDays = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  list ($starting, $ending) = txDates($txDays, @$period, @$starting, @$ending);


  $txChannels = u\ray(TX_CHANNELS);
  $txStates = u\ray(TX_STATES);
  $title = "<div id='txs-title'>TRANSACTIONS</div>";
  $header = u\ray($headers);

  if (@$download) {
    header('Content-type: text/plain');
    $dt1 = strftime('%Y%m%d', $starting);
    $dt2 = strftime('%Y%m%d', $ending);
    $asif = $real ? '' : 'asif';
    $filename = "rcredits$dt1-$dt2$asif.csv";
    header("Content-Disposition: attachment; filename=\"$filename\"");
    $csv = fopen('php://output', 'w');
    
    $header = array_combine($classes, $header); // make splice cleaner
    u\splice($header, 'buttons', 1); // don't download buttons
    u\splice($header, 'date', 0, t('Created')); // insert creation date
    $header = array_values($header);
    fputcsv($csv, $header);
  }

//  $records = be\myTransactions(compact(u\ray('starting ending real'))); // we use $rows for another purpose, below
  $result = be\myTransactions(compact(u\ray('starting ending real'))); // we use $rows for another purpose, below
  $rows = array();
  for ($rowNum = 0; $rowNum < $maxRows; $rowNum++) {
    if (!$record = $result->fetchAssoc()) break;
//  foreach ($records as $record) {
    extract($record, EXTR_PREFIX_ALL, 'tx');
    $zxid = $real ? $tx_xid : -$tx_xid;
    $data = unserialize($tx_data);
    $is_reward = ($tx_type != TX_TRANSFER);
    $amount = number_format($tx_r, 2);
    $total = number_format($tx_amount, 2);
    $usd = number_format($tx_amount - $tx_r, 2);
    $rpct = number_format(100 * $tx_r / $tx_amount, 1);
    if ($rpct == '100.0') $rpct = '100'; // keep the field narrow (4 chars max)
    $rpct = "<span title=\"r$$amount + us$$usd = $$total\">$rpct</span>";
    $date = @$tx_completed ?: $tx_created;
    
    if ($report) { // community transaction history
      $reward = @$data['bonus'] + @$data['rebate'];
      txNeatAmounts($amount, $reward, $is_reward);
      $state = txColoredState($tx_state);
      $tid = str_replace('-', 'a', $zxid);
      $row = array($tid, strftime('%d-%b-%Y', $date), r\acct($tx_payer)->full_name, r\acct($tx_payee)->full_name, $amount, $rpct, $state, $tx_purpose, $reward);
    } else { // individual account transaction history
      $url = $url0 . "&zxid=$zxid";
      $tid = be\zxid2tid($zxid);
      $toMe = ($tx_payee == $cuid);
      $byMe = ($tx_taking == $toMe);
      $agent = $toMe ? $tx_payee_agent : $tx_payer_agent;
      $state = txColoredState($tx_state, $byMe);
      $reward = @$data[$toMe ? 'bonus' : 'rebate'];
      txNeatAmounts($amount, $reward, $is_reward);
      list ($to_you, $from_you) = $toMe ? array($amount, R_NONE) : array(R_NONE, $amount);
      $button1 = (!$byMe and $tx_state == TX_PENDING) ? button('OK', "$url&do=ok", tx_action('ok', $tx_state, $toMe, $byMe)) : '';
      $button2 = ((!$is_reward or $cttyAdmin) and !@$data['undone']) ? button(' X ', "$url&do=no", tx_action('no', $tx_state, $toMe, $byMe)) : '';
      $button3 = (($tx_taking ? $canSell : $canBuy) and !$is_reward) ? "<a href='$url&do=edit' title='Edit Purpose'><img src='$base_url/sites/all/modules/rcredits/images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
      $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
      $row = array($tid, strftime('%d-%b', $date), $tx_name, $from_you, $to_you, $rpct, $state, $buttons, $tx_purpose, $reward);
      if (@$agents) $row[] = ($agent == $cuid or $agent < 0) ? R_NONE : (@$download ? r\qid($agent) : u\n2a($agent, 3));
      // might optionally show both agents for reports someday
    }

    if (@$channels) $row[] = $txChannels[$tx_channel];

    if (@$download) {
      $row = array_combine($classes, $row); // associate to make deletions cleaner
      u\splice($row, 'buttons', 1); // remove buttons if they exist
      u\splice($row, 'date', 0, strftime('%Y-%m-%d', $tx_created)); // insert full creation date
      $row['date'] = @$tx_completed ? strftime('%Y-%m-%d', $tx_completed) : ''; // show FULL date completed
      $row['state'] = $txStates[$tx_state]; // transaction state
      foreach ($row as $key => $value) $row[$key] = $value == R_NONE ? '' : strip_tags($value);
      $row = array_values($row);
      fputcsv($csv, $row); // not STDOUT
    } else $rows[] = $row;
  }

  if (@$download) exit();
  if ($record and $result->fetchAssoc()) say('transaction list truncated'); // there was more than we could show

  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  $channels = formField('checkbox', '', t('channels'), dft(@$channels));

  if ($report) {
    $channels['#title'] = t('Show:');
    $agents = $do = $summary = NULL;
  } else {
    $agents = formField('checkbox', t('Show:'), t('agents'), dft(isset($agents) ? $agents : $cacct->can(PERM_COMPANY)));
    $do = formField('hidden', $do);
    $summary = formField('markup', '', '', array('markup' => tx_summary($starting, $ending)));
  }

  $real = !r\isRTrader() ? item('') 
    : formField('radios', t('Show:'), '', dft($real), array(R_ASIF_TEXT . t(' Transactions'), t('Real Transactions')));
  $period = formField('select', t('For the past:'), '', dft(@$period ?: TX_DEFAULT_PERIOD), $txDays);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  $downloadPeriod = submit(t('Download'));
  $starting = textField(t('OR Starting:'), '', dft(u\formatDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\formatDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $downloadDates = submit(t('Download'));
  $transactions = formField('markup', '', '', array('markup' => $title . $list . '<br>&nbsp;'));
  $title = item(t('rCredits Transaction History'));
  $subtext = item(t('Only the rCredits part of each transaction is shown. The "r%" column tells what percentage this represents of the total transaction amount. Hover over the figure in the "r%" column to see the total and US Dollar amounts.'));

  $form = compact(u\ray('title subtext agents channels real period submitPeriod downloadPeriod starting ending submitDates downloadDates summary transactions'));
  if (@$downloaded) js("window.open(\"$url0&download=1\")", 'inline', 'footer');
  return labeled($form);
}

function txConfirm($do, $zxid, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return FALSE;
  if (!$details = be\lastTx('xid=:zxid', compact('zxid'), TRUE)) return FALSE; // bail on hack attempt
  extract(r\txReportArgs($details), EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_toMe, $my_byMe);
  confirm(t('Are you sure you want to ') . "$action?<br>($my_summary)", $sta); // sets 'confirm'
  return sureForm($sta);
}

function formTxs_validate($form, &$sta) {
  $txDays = @$form['period']['#options'];
  extract(u\just('real period starting ending op', $sta['values']));
  list ($sta['values']['starting'], $sta['values']['ending']) = strpos(op($sta), 'Period') ? txDates($txDays, @$period) : txDates($txDays, '', @$starting, @$ending);
}

function formTxs_submit($form, &$sta, $page = 'transactions') {
  global $base_url;
  if (confirming_s($sta)) return;
  extract(u\just('real period starting ending agents channels op', $sta['values']));
  $real = @$real; // avoid warning, in case it was hidden
  $url = @"$page/real=$real&period=$period&starting=$starting&ending=$ending&agents=$agents&channels=$channels";
  if ($op == t('Download')) {
    say('downloading');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

function txDates($txDays, $period, $starting = '', $ending = '') {
  if (@$starting and !is_numeric($starting) and !($starting = strtotime(@$starting))) say('bad date', 'starting');
  if (@$starting > REQUEST_TIME) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and !($ending = strtotime($ending))) say('bad date', 'ending');
  if (!@$ending or $ending > REQUEST_TIME) $ending = REQUEST_TIME;
  $ending = strtotime('tomorrow', $ending) - 1;

  $period = (int) (@$period ?: TX_DEFAULT_PERIOD);
  $ago = @$txDays[$period] ?: ($period . ' days');
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : "$ago ago");
  return array($starting, $ending);
}  

function txNeatAmounts(&$amount, &$reward, $is_reward) {
  $reward = $reward ? number_format($reward, 2) : R_NONE;
  if ($is_reward) list ($amount, $reward) = array(R_NONE, $amount); // show inflation, grants, fines, and signup bonus as rewards
}
function txColoredState($state, $sayPending = TRUE) {
  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'denied', TX_DISPUTED => 'disputed');
  $colors = array('&#10004;' => 'green', 'ok?' => 'orange', 'pending' => 'darkgreen', 'denied' => 'red', 'disputed' => 'red');
  $state = ($state == TX_PENDING and $sayPending) ? 'pending' : $states[$state];
  $color = $colors[$state];
  return "<span style='color:$color;'>$state</span>";
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

/**
 * Handle click on 'OK' or 'NO' (approving or undoing a transaction)
 */
function txOKorNO($tx_task, $zxid) {
  $cuid = r\acct()->id;
  if (!$result = be\lastTx('xid=:zxid', compact('zxid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('state amount toMe', $result));
  if ($tx_task == 'ok') { // OK
    u\EXPECT($state == TX_PENDING);
    if ($toMe) {
      r\setTxState(TX_DONE, $zxid); // accept the payment
      // DWOLLA: initiate transfer from user to community
      list ($message, $args) = array('payment accepted', compact('amount'));
    } else list ($message, $args) = be\payInvoice($zxid);
  } else list ($message, $args) = be\undoTx($zxid); // NO
  say($message, $args);
}

function tx_edit($zxid) {
  if (!$details = be\lastTx('xid=:zxid', compact('zxid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('taking purpose role', r\txReportArgs($details)));
  
  if (!r\acct()->can($taking ? PERM_SELL : PERM_BUY)) return FALSE;
  
  $form = array(
    'title' => formField('item', t('Edit Description')),
//    'review' => formField('item', $tx_summary),
    'purpose' => textField(t('Purpose'), '', dft($purpose)),
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
  extract($pending_info = (array) be\creditInfo(array('state' => ':TX_PENDING', 'asof' => $ending)), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (u\ray('grossInProper grossOut rewards committed') as $key) {
    $dif[$key] = number_format($now_info[$key] - $old_info[$key], 2);
    $pfancy[$key] = number_format($pending_info[$key], 2);
  }
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pfancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = u\formatDate($starting);
  $ending = u\formatDate($ending);
  $pending_balance = txBal($pending_grossInProper - $pending_grossOut + $pending_rewards - $pending_committed, '+ ');
  $now_balance = txBal($now_balance);
  $old_balance = txBal($old_balance);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards tithesign tithe balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Starting balance,,To You,,From You,,Rewards,,To CGF,,Ending balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old_balance | + | $dif_grossInProper | - | $dif_grossOut | + | $dif_rewards | - | $dif_committed | = | $now_balance"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>PENDING:</b> | + | $pfancy_grossInProper | - | $pfancy_grossOut | + | $pfancy_rewards | - | $pfancy_committed | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}
