<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits as r;

/**
 * @file
 * Forms for managing transactions, and related functions
 */

define('TX_OPT_RCREDITS', 1);
define('TX_OPT_USD', 2);
define('TX_OPT_PENDINGS', 3);
define('TX_OPT_CHANNELS', 4);
define('TX_OPT_AGENTS', 5);

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();
  
  extract(u\just('rCredits USD pendings agents channels period starting ending do xid download downloaded variety', $args)); 
  $report = $mya->can(B_CTTY_ADMIN) ? (@$variety == 'report') : FALSE; // no community reports yet for non-admins
  if (!$report and @$do) if ($result = handleTx($do, $xid, $sta, $args)) return $result;

  $page = $report ? 'reports' : 'transactions';
  if (!@$period) $period = TX_DEFAULT_PERIOD;

  // set up option fields (must come before $currency and $url0)
  $optDefaults = $opts = array();
  foreach (u\ray('rCredits USD pendings channels agents') as $one) {
    $opts[constant('TX_OPT_' . strtoupper($one))] = $one; // set TX_OPT_CHANNELS etc.
    if (@$$one) $optDefaults[] = $one;
  }
  if (!isset($agents) and !$mya->can(B_PERSONAL)) {$agents = TRUE; $optDefaults[] = 'agents';}
  if (!isset($pendings)) {$pendings = TRUE; $optDefaults[] = 'pendings';}
  if (!isset($rCredits) and !isset($USD)) {
    $rCredits = $USD = TRUE; 
    $optDefaults[] = 'rCredits';
    $optDefaults[] = 'USD';
  }

  $currency = (@$download or (@$rCredits and @$USD)) ? 'both' : (@$rCredits ? 'r' : 'usd');
  $url0 = @"$base_url/$page/period=$period&rCredits=$rCredits&USD=$USD&pendings=$pendings&agents=$agents&channels=$channels&variety=$variety";
  list ($classes, $header, $downloadHeader) = txFields($report, @$download, @$agents, @$channels);
 
  $txDays = array('1' => 'today', '4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  list ($starting, $ending) = txDates($txDays, @$period, @$starting, @$ending);
  $maxRows = @$download ? PHP_INT_MAX : 20; // don't overwhelm the screen
  $csv = @$download ? setupDownload($starting, $ending) : FALSE;
  if (@$download) csv($csv, $downloadHeader);

  // build the transaction list
  $result = be\myTransactions($starting, $ending, $report ? 'ctty' : $mya->id, !@$pendings);
  $rows = array();
  for ($rowNum = 0; $rowNum < $maxRows; $rowNum++) {
    if (!$tx = $result->fetchAssoc()) break;
    $rows[] = doRow($tx, $report, $csv, $classes, @$channels, @$agents, $currency, $url0);
  }
  if ($tx and $result->fetchAssoc()) say('transaction list truncated'); // there was more than we could show
  if (@$download) {if (t\EST()) return; else exit();}

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
  
  if ($report) {
    $agents = $summary = NULL;
  } else $summary = formField('markup', '', '', array('markup' => tx_summary($starting, $ending, $currency)));

  $opts2 = boxField('opts', t('Show:'), '', $optDefaults, $opts);
  $divider = item('<hr>');
  $period = formField('select', t('For the past:'), '', dft($period), $txDays);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  $downloadPeriod = submit(t('Download'));
  $starting = textField(t('OR Starting:'), '', dft(u\formatDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\formatDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $downloadDates = submit(t('Download'));
  $title = "";
  $title = item(t('rCredits Transaction History'));
  $subtext = item(t('The "r%" column tells what percentage of the total transaction amount is rCredits. Hover over that figure to see the total, rCredits, and US Dollar amounts. Use the "rCredits" and "USD" checkboxes to control what is shown in the Summary and in the "From you" and "To you" columns. Show "pendings" to include open invoices. "Channel" is Web, Smartphone, or SMS. "Agent" is who did the transaction on your behalf.'));

  $form = compact(u\ray('title subtext opts2 divider period submitPeriod downloadPeriod starting ending submitDates downloadDates summary transactions'));
  if (@$downloaded) js("window.open(\"$url0&download=1\")", 'inline', 'footer');
  return labeled($form);
}

function setupDownload($starting, $ending) {
  global $testCSV, $testCSVname;
  $testCSV = array();
  header('Content-type: text/plain');
  $dt1 = strftime('%Y%m%d', $starting);
  $dt2 = strftime('%Y%m%d', $ending);
  $testCSVname = $filename = "rcredits$dt1-$dt2.csv";
  header("Content-Disposition: attachment; filename=\"$filename\"");
  return fopen('php://output', 'w');
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
 
/**
 * Return classes and headers for the appropriate set of fields.
 */
function txFields($report, $download, $agents, $channels) {
  if ($report) { // community transaction history
    $classes = 'tid date from to amount rpct r usd usdTid state purpose reward';
    $headers = t('#,Date,From,To,Total,r%,rCredits,USD,USD#,Status,Purpose,Rewards');
  } else { // individual account transaction history
    $classes = 'tid date name fromyou toyou rpct r usd usdTid state buttons purpose reward net';
    $headers = t('#,Date,Name,From you,To you,r%,rCredits,USD,USD#,Status,Action,Purpose,Reward,Net');
  }

  $header = array_combine(u\ray($classes), u\ray($headers)); // make splices and additions easy and clear

  if (@$agents) $header['agent'] = t('Agent'); // (not applicable for reports)
  if (@$channels) $header['channel'] = t('Via');

  if (@$download) { // downloading -- modify standard individual or community field list
    $downloadHeader = $header;
    u\splice($downloadHeader, 'buttons', 1); // don't download buttons
    u\splice($downloadHeader, 'date', 0, array('created' => t('Created'))); // insert creation date
  }

  return array(array_keys($header), array_values($header), @array_values($downloadHeader));
}

/**
 * Prepare one transaction row for output (or download).
 * @param assoc $tx: the transaction record
 * @param file $csv: the (open) download file or FALSE if not downloading
 *
 * @return: the row
 */
function doRow($tx, $report, $csv, $classes, $channels, $agents, $currency, $url0) {
  global $base_url, $rUrl;
  $mya = r\acct();
  $myid = $mya->id;
  $txChannels = u\ray(TX_CHANNELS);
  $txStates = u\ray(TX_STATES);

  extract($tx, EXTR_PREFIX_ALL, 'tx');
  $xid = $tx_xid;
  $data = unserialize($tx_data);
  $is_reward = ($tx_type != TX_TRANSFER);
  $r = number_format($tx_r, 2);
  $total = number_format($tx_amount, 2);
  $usd = number_format($tx_amount - $tx_r, 2);
  $rpct = $tx_amount ? number_format(100 * $tx_r / $tx_amount, 1) : R_NONE;
  if ($rpct == '100.0') $rpct = '100'; // keep the field narrow (4 chars max)
  $rpct = sprintf('<span title="$%sr + $%sus = $%s">%s</span>', $r, $usd, $total, $rpct);
  $date = @$tx_completed ?: $tx_created;
  $amount = $currency == 'r' ? $r : ($currency == 'usd' ? $usd : $total);
  
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
    $net = number_format(($toMe ? $total : -$total) + $reward, 2);
    txNeatAmounts($amount, $reward, $is_reward);
    list ($toYou, $fromYou) = $toMe ? array($amount, R_NONE) : array(R_NONE, $amount);
    $button1 = (!$byMe and okableState($tx_state)) ? button('OK', "$url&do=ok", tx_action('ok', $tx_state, $toMe, $byMe)) : '';
    $button2 = ((!$is_reward or $mya->can(B_CTTY_ADMIN)) and !@$data['undoneBy']) ? button(' X ', "$url&do=no", tx_action('no', $tx_state, $toMe, $byMe)) : '';
    $button3 = ($mya->can($tx_taking ? B_SELL : B_BUY) and !$is_reward) ? "<a href='$url&do=edit' title='Edit Purpose'><img src='$rUrl/images/buttons/edit.png' width=20 height=20 border=0 /></a>" : '';
    $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    $row = array($tid, strftime('%d-%b', $date), $tx_name, $fromYou, $toYou, $rpct, $amount, $usd, $tx_usdXid, $state, $buttons, $tx_purpose, $reward, $net);
  }

  if (@$agents) $row[] = ($agent == $myid or $agent < 0) ? R_NONE 
  : ($csv ? r\quid($agent) : str_replace(R_SERVER_ID . '.', '.', r\quid($agent)));
  // might optionally show both agents for reports someday
  if (@$channels) $row[] = $txChannels[$tx_channel];

  if ($csv) { // modify community or individual field list, if downloading
    $created = strftime('%Y-%m-%d', $tx_created);
    $date = @$tx_completed ? strftime('%Y-%m-%d', $tx_completed) : '';
    $state = $txStates[$tx_state - TX_STATE0]; // transaction state (human-readable)
    if ($amount == R_NONE) {
      $r = $usd = ''; // showing incentives (other than rebate/bonus) as rewards
    } elseif (!$toMe) {$r = '-' . $r; $usd = '-' . $usd;}
    
    $row = array_combine($classes, $row); // name the fields
    u\splice($row, 'date', 1, compact('created', 'date'));
    $row['r'] = $r;
    $row['usd'] = $usd;
    $row['state'] = $state;
    u\splice($row, 'buttons', 1); // don't download buttons
    foreach ($row as $key => $value) $row[$key] = $value == R_NONE ? '' : strip_tags($value);
    csv($csv, array_values($row)); // not STDOUT
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
  $mya = r\acct();
  $myid = $mya->id;
  if (confirming_v($sta)) return FALSE;
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(r\txReportArgs($mya, $details), EXTR_PREFIX_ALL, 'my');
  $action = tx_action($do, $my_state, $my_toMe, $my_byMe);
  confirm(t('Are you sure you want to ') . "$action?<br>($my_summary)", $sta); // sets 'confirm'
  return sureForm($sta);
}

function formTxs_validate($form, &$sta) {
  $txDays = @$form['period']['#options'];
  extract(u\just('period starting ending opts op', $sta['input']));
  if (!@$opts[TX_OPT_RCREDITS] and !@$opts[TX_OPT_USD]) return say('missing currency');
  list ($sta['input']['starting'], $sta['input']['ending']) = op($sta) == 'submitPeriod' ? txDates($txDays, @$period) : txDates($txDays, '', @$starting, @$ending);
}

function formTxs_submit($form, &$sta, $page = 'transactions') {
  if (confirming_s($sta)) return;
  extract(u\just('period starting ending opts op', $sta['input']));
  $channels = @$opts[TX_OPT_CHANNELS] ? 1 : 0;
  $agents = @$opts[TX_OPT_AGENTS] ? 1 : 0;
  $rCredits = @$opts[TX_OPT_RCREDITS] ? 1 : 0;
  $USD = @$opts[TX_OPT_USD] ? 1 : 0;
  $pendings = @$opts[TX_OPT_PENDINGS] ? 1 : 0;
  $url = @"$page/period=$period&starting=$starting&ending=$ending&rCredits=$rCredits&USD=$USD&pendings=$pendings&agents=$agents&channels=$channels";
  if ($op == t('Download')) {
    say('downloading');
    if (!$rCredits or !$USD) say('download always both');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

function txDates($txDays, $period, $starting = '', $ending = '') {
  if (@$starting and !is_numeric($starting) and !($starting = strtotime(@$starting))) say('bad date', 'starting');
  if (@$starting > REQUEST_TIME) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and !($ending = strtotime($ending))) say('bad date', 'ending');
  if (!@$ending or $ending > REQUEST_TIME) $ending = REQUEST_TIME;
  $ending = strtotime('tomorrow', $ending) - 1; // last second of the day

  $period = (int) (@$period ?: TX_DEFAULT_PERIOD);
  $ago = @$txDays[$period] ?: ($period . ' days');
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : "$ago ago");
  $starting = strtotime(date("m/d/Y", $starting)); // chop off time of day
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
  extract(u\just('taking purpose role', r\txReportArgs(r\acct(), $details)));
  
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
function tx_summary($starting, $ending, $currency) {
  $mya = r\acct();
  $myid = $mya->id;
  $previous_ending = $starting - 1;
  extract($nowInfo = (array) be\creditInfo(array('asof' => $ending)), EXTR_PREFIX_ALL, 'n');
  extract($pendingInfo = (array) be\creditInfo(array('state' => ':TX_PENDING', 'asof' => $ending)), EXTR_PREFIX_ALL, 'p');
  extract($oldInfo = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'o');
//  debug(compact('nowInfo','oldInfo'));

  $dif = array();
  foreach (u\ray('inProper out rInProper rOut usdIn usdOut rewards') as $key) $dif[$key] = $nowInfo[$key] - $oldInfo[$key];
  extract($dif, EXTR_PREFIX_ALL, 'd');
  $p_committed = $mya->committed; // this will include all pending commitments
  
  $fields = 'old in out rewards new';
  $pFields = 'p_inProper p_out p_rewards p_committed p_balance';
  $values =
    ($currency == 'both' ? array($o_balance, $d_inProper, $d_out, $d_rewards, $n_balance)
  : ($currency == 'r' ? array($o_r, $d_rInProper, $d_rOut, $d_rewards, $n_r)
  : ($currency == 'usd' ? array($o_usd, $d_usdIn, $d_usdOut, 0, $n_usd) : '')));
  $p_balance = $p_rInProper - $p_rOut + $p_rewards - $p_committed;
  $values = array_combine(u\ray($fields), $values) + compact(u\ray($pFields));
  foreach ($values as $key => $value) $$key = number_format($value, 2);
 
  $p_balance = txBal($p_balance, '+ '); // add +/-/$ to these, as appropriate
  $new = txBal($new);
  $old = txBal($old);

  $starting = u\formatDate($starting);
  $ending = u\formatDate($ending);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards sharesign share balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Starting balance,,To You,,From You,,Rewards,,To CGF,,Ending balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old | + | $in | - | $out | + | $rewards |  |  | = | $new"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>PENDING:</b> | + | $p_inProper | - | $p_out | + | $p_rewards | - | $p_committed | = | $p_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}

function okableState($state) {return ($state == TX_PENDING or $state == TX_DISPUTED);}

function csv($csv, $a) {
  global $testCSV; if (!@$testCSV) $testCSV = array();
  fputcsv($csv, $a);
  $testCSV[] = $a;
}
