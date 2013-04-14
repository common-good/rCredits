<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits as r;

/**
 * @file
 * Forms for managing transactions, and related functions
 */

define('TX_OPT_CHANNELS', 1);
define('TX_OPT_AGENTS', 2);

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();
  
  extract(u\just('agents channels period starting ending do xid download downloaded variety', $args)); 
  $report = $mya->can(B_CTTY_ADMIN) ? (@$variety == 'report') : FALSE; // no community reports yet for non-admins
  if (!$report and @$do) if ($result = handleTx($do, $xid, $sta, $args)) return $result;

  $page = $report ? 'reports' : 'transactions';
  $url0 = @"$base_url/$page/period=$period&agents=$agents&channels=$channels&variety=$variety";
  list ($classes, $header) = txFields($report, @$download, @$agents, @$channels);
 
  $txDays = array('1' => 'today', '4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  list ($starting, $ending) = txDates($txDays, @$period, @$starting, @$ending);
  $maxRows = @$download ? PHP_MAX_INT : 20; // don't overwhelm the screen
  $csv = @$download ? setupDownload($starting, $ending) : FALSE;
  if (@$download) fputcsv($csv, $header);

  // build the transaction list
  $result = be\myTransactions(compact(u\ray('starting ending')));
  $rows = array();
  for ($rowNum = 0; $rowNum < $maxRows; $rowNum++) {
    if (!$tx = $result->fetchAssoc()) break;
    $rows[] = doRow($tx, $report, $csv, $classes, @$channels, @$agents, $url0);
  }
  if ($tx and $result->fetchAssoc()) say('transaction list truncated'); // there was more than we could show
  if (@$download) exit();

  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
//  $transactions = formField('markup', '', '', array('markup' => "<div id=\"txs-title\">TRANSACTIONS</div>$list <br>&nbsp;"));
  $transactions = item("<div id=\"txs-title\">TRANSACTIONS</div>$list <br>&nbsp;");
  
  $optDefaults = array();
  $opts = array(TX_OPT_CHANNELS => 'channels');
  if (@$channels) $optDefaults[] = 'channels';
  if ($report) {
    $agents = $summary = NULL;
  } else {
    $opts[TX_OPT_AGENTS] = 'agents';
    if (isset($agents) ? $agents : !$mya->can(B_PERSONAL)) $optDefaults[] = 'agents';
    $summary = formField('markup', '', '', array('markup' => tx_summary($starting, $ending)));
  }

  $opts2 = boxField('opts', t('Show:'), '', $optDefaults, $opts);
  $divider = item('<hr>');
  $period = formField('select', t('For the past:'), '', dft(@$period ?: TX_DEFAULT_PERIOD), $txDays);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  $downloadPeriod = submit(t('Download'));
  $starting = textField(t('OR Starting:'), '', dft(u\formatDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\formatDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $downloadDates = submit(t('Download'));
  $title = "";
  $title = item(t('rCredits Transaction History'));
  $subtext = item(t('Only the rCredits part of each transaction is shown. The "r%" column tells what percentage this represents of the total transaction amount. Hover over that figure to see the total and US Dollar amounts.'));

  $form = compact(u\ray('title subtext opts2 divider period submitPeriod downloadPeriod starting ending submitDates downloadDates summary transactions'));
  if (@$downloaded) js("window.open(\"$url0&download=1\")", 'inline', 'footer');
  return labeled($form);
}

function setupDownload($starting, $ending) {
  header('Content-type: text/plain');
  $dt1 = strftime('%Y%m%d', $starting);
  $dt2 = strftime('%Y%m%d', $ending);
  $filename = "rcredits$dt1-$dt2.csv";
  header("Content-Disposition: attachment; filename=\"$filename\"");
  $csv = fopen('php://output', 'w');
}

function handleTx($do, $xid, $sta, $args) {
  if (!$xid = (int) @$xid) return FALSE;
  if ($do == 'edit') return tx_edit($xid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm = txConfirm($do, $xid, $sta)) return $confirm;
    if (!$sta['rebuild']) txOKorNO($do, $xid); // don't do anything if canceling
    $url = str_replace('&do=', '&did=', "transactions/$args"); // don't do it again
    r\go($url); // back to the normal form
  }
  return FALSE;
}
 
function txFields($report, $download, $agents, $channels) {
  if ($report) { // community transaction history
    $classes = 'tid date from to amount rpct r usd usdTid state purpose reward';
    $headers = t('#,Date,From,To,Total,r%,rCredits,USD,USD#,Status,Purpose,Rewards');
  } else { // individual account transaction history
    $classes = 'tid date name fromyou toyou rpct r usd usdTid state buttons purpose reward';
    $headers = t('#,Date,Name,From you,To you,r%,rCredits,USD,USD#,Status,Action,Purpose,Reward');
    if (@$agents) {$classes .= ' agent'; $headers .= t(',Agent');}
  }
  if (@$channels) {$classes .= ' channel'; $headers .= t(',Via');}
  $classes = u\ray($classes);
  $header = u\ray($headers);

  if (@$download) {  
    $header = array_combine($classes, $header); // make splice cleaner
    u\splice($header, 'rpct', 1); // don't download rpct
    u\splice($header, 'buttons', 1); // don't download buttons
    u\splice($header, 'date', 0, t('Created')); // insert creation date
    $header = array_values($header);
  }
  
  return array($classes, $header);
}

/**
 * Prepare one transaction row for output (or download).
 * @param assoc $tx: the transaction record
 * @param file $csv: the (open) download file or FALSE if not downloading
 *
 * @return: the row
 */
function doRow($tx, $report, $csv, $classes, $channels, $agents, $url0) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $txChannels = u\ray(TX_CHANNELS);
  $txStates = u\ray(TX_STATES);

  extract($tx, EXTR_PREFIX_ALL, 'tx');
  $xid = $tx_xid;
  $data = unserialize($tx_data);
  $is_reward = ($tx_type != TX_TRANSFER);
  $amount = number_format($tx_r, 2);
  $total = number_format($tx_amount, 2);
  $usd = number_format($tx_amount - $tx_r, 2);
  $rpct = $tx_amount ? number_format(100 * $tx_r / $tx_amount, 1) : R_NONE;
  if ($rpct == '100.0') $rpct = '100'; // keep the field narrow (4 chars max)
  $rpct = "<span title=\"r$$amount + us$$usd = $$total\">$rpct</span>";
  $date = @$tx_completed ?: $tx_created;
  
  if ($report) { // community transaction history
    $reward = @$data['bonus'] + @$data['rebate'];
    txNeatAmounts($amount, $reward, $is_reward);
    $state = txColoredState($tx_state);
    $tid = $xid; // for community use the actual transaction numbers
    $row = array($tid, strftime('%d-%b-%Y', $date), r\acct($tx_payer)->fullName, r\acct($tx_payee)->fullName, $total, $rpct, $amount, $usd, $tx_usdXid, $state, $tx_purpose, $reward);
  } else { // individual account transaction history
    $url = $url0 . "&xid=$xid";
    $toMe = ($tx_payee == $myid);
    $byMe = ($tx_taking == $toMe);
    $tid = $toMe ? $tx_payeeTid : $tx_payerTid;
    $agent = $toMe ? $tx_payeeAgent : $tx_payerAgent;
    $state = txColoredState($tx_state, $byMe);
    $reward = @$data[$toMe ? 'bonus' : 'rebate'];
    txNeatAmounts($amount, $reward, $is_reward);
    list ($to_you, $from_you) = $toMe ? array($amount, R_NONE) : array(R_NONE, $amount);
    $button1 = (!$byMe and $tx_state == TX_PENDING) ? button('OK', "$url&do=ok", tx_action('ok', $tx_state, $toMe, $byMe)) : '';
    $button2 = ((!$is_reward or $mya->can(B_CTTY_ADMIN)) and !@$data['undoneBy']) ? button(' X ', "$url&do=no", tx_action('no', $tx_state, $toMe, $byMe)) : '';
    $button3 = ($mya->can($tx_taking ? B_SELL : B_BUY) and !$is_reward) ? "<a href='$url&do=edit' title='Edit Purpose'><img src='$base_url/sites/all/modules/rcredits/images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    $row = array($tid, strftime('%d-%b', $date), $tx_name, $from_you, $to_you, $rpct, $amount, $usd, $tx_usdXid, $state, $buttons, $tx_purpose, $reward);
    if (@$agents) $row[] = ($agent == $myid or $agent < 0) ? R_NONE : ($csv ? r\qid($agent) : u\n2a($agent, 3));
    // might optionally show both agents for reports someday
  }

  if (@$channels) $row[] = $txChannels[$tx_channel];

  if ($csv) {
    $row = array_combine($classes, $row); // associate to make deletions cleaner
    u\splice($row, 'buttons', 1); // remove buttons if they exist
    u\splice($row, 'date', 0, strftime('%Y-%m-%d', $tx_created)); // insert full creation date
    $row['date'] = @$tx_completed ? strftime('%Y-%m-%d', $tx_completed) : ''; // show FULL date completed
    $row['state'] = $txStates[$tx_state]; // transaction state
    foreach ($row as $key => $value) $row[$key] = $value == R_NONE ? '' : strip_tags($value);
    $row = array_values($row);
    fputcsv($csv, $row); // not STDOUT
    return TRUE;
  } else return $row;
}

/**
 * Return a form to confirm the transaction operation.
 * @param string $do: the action to confirm
 * @param int $xid: record id of transaction to confirm an operation on
 * @param assoc $sta: Drupal's status array (gets member 'confirm' set)
 * @return: the form (FALSE on error or if already confirming)
 */
function txConfirm($do, $xid, &$sta) {
  $myid = r\acct()->id;
  if (confirming_v($sta)) return FALSE;
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(r\txReportArgs($details), EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_toMe, $my_byMe);
  confirm(t('Are you sure you want to ') . "$action?<br>($my_summary)", $sta); // sets 'confirm'
  return sureForm($sta);
}

function formTxs_validate($form, &$sta) {
  $txDays = @$form['period']['#options'];
  extract(u\just('period starting ending op', $sta['input']));
  list ($sta['input']['starting'], $sta['input']['ending']) = op($sta) == 'submitPeriod' ? txDates($txDays, @$period) : txDates($txDays, '', @$starting, @$ending);
}

function formTxs_submit($form, &$sta, $page = 'transactions') {
  if (confirming_s($sta)) return;
  extract(u\just('period starting ending opts op', $sta['input']));
  $channels = @$opts[TX_OPT_CHANNELS] ? 1 : 0;
  $agents = @$opts[TX_OPT_AGENTS] ? 1 : 0;
  $url = @"$page/period=$period&starting=$starting&ending=$ending&agents=$agents&channels=$channels";
  if ($op == t('Download')) {
    say('downloading');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

function txDates($txDays, $period, $starting = '', $ending = '') {
  if (@$starting and !is_numeric($starting) and !($starting = u\strToDate(@$starting))) say('bad date', 'starting');
  if (@$starting > REQUEST_TIME) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and !($ending = u\strToDate($ending))) say('bad date', 'ending');
  if (!@$ending or $ending > REQUEST_TIME) $ending = REQUEST_TIME;
  $ending = strtotime('tomorrow', $ending) - 1; // last second of the day

  $period = (int) (@$period ?: TX_DEFAULT_PERIOD);
  $ago = @$txDays[$period] ?: ($period . ' days');
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : "$ago ago");
  $starting = u\strToDate(date("m/d/Y", $starting)); // chop off time of day
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
function txOKorNO($tx_task, $xid) {
  $myid = r\acct()->id;
  if (!$result = be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('state amount toMe', $result));
  if ($tx_task == 'ok') { // OK
    u\EXPECT($state == TX_PENDING);
    if ($toMe) {
      r\setTxState(TX_DONE, $xid); // accept the payment
      // DWOLLA: initiate transfer from user to community
      list ($message, $args) = array('payment accepted', compact('amount'));
    } else list ($message, $args) = be\payInvoice($xid);
  } else list ($message, $args) = be\undoTx($xid); // NO
  say($message, $args);
}

/**
 * Return a form to edit the given transaction.
 * @param int $xid: the transaction id
 * @return the form (FALSE on error)
 */
function tx_edit($xid) {
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('taking purpose role', r\txReportArgs($details)));
  
  if (!r\acct()->can($taking ? B_SELL : B_BUY)) return FALSE;
  
  $form = array(
    'title' => formField('item', t('Edit Description')),
//    'review' => formField('item', $tx_summary),
    'purpose' => textField(t('Purpose'), '', dft($purpose)),
    'go' => formField('submit', NULL, t('Submit button'), 'Update'),
    'xid' => formField('hidden', $xid),
    'field' => formField('hidden', $role . '_for'),
    
    '#validate' => array('rCredits\\Web\\tx_edit_validate'),
    '#submit' => array('rCredits\\Web\\tx_edit_submit'),
  );
  return $form;
}

function tx_edit_validate($form, &$sta) {
}

function tx_edit_submit($form, &$sta) {
  extract($sta['input']);
  r\setTxField($field, $purpose, $xid);
  $sta['redirect'] = 'transactions';
}

function txBal($bal, $plus = '', $minus = '- ') {return ($bal < 0 ? $minus : $plus) . '$' . number_format(abs($bal), 2);}

/**
 * Return the transaction summary table
 */
function tx_summary($starting, $ending) {
  $myid = r\acct()->id;
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
  $pending_committed += $dif['committed']; // this will include all pending commitments since day 1 (?!)
  $pfancy_committed = u\formatAmount($pending_committed);
  
  $starting = u\formatDate($starting);
  $ending = u\formatDate($ending);
  $pending_balance = txBal($pending_grossInProper - $pending_grossOut + $pending_rewards - $pending_committed, '+ ');
  $now_balance = txBal($now_balance);
  $old_balance = txBal($old_balance);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards sharesign share balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Starting balance,,To You,,From You,,Rewards,,To CGF,,Ending balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old_balance | + | $dif_grossInProper | - | $dif_grossOut | + | $dif_rewards | - |  | = | $now_balance"),
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
