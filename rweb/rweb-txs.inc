<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Txs as x;

/**
 * @file
 * Forms for managing transactions, and related functions
 */
 
include __DIR__ . '/../rcredits-txs.inc';

define('TX_OPT_KEYS' , 'agents channels boxes exchanges pexchanges'); // these are all placeholders now
define('TX_URL_OPTS', 'states ' . TX_OPT_KEYS);
define('TX_OPT_AGENTS', 1);
define('TX_OPT_CHANNELS', 2);
define('TX_OPT_BOXES', 3);
define('TX_OPT_EXCHANGES', 4);
define('TX_OPT_PEXCHANGES', 5);

define('STATES_DONE', 0); // show just completed transactions
define('STATES_PENDING', 1); // show just open invoices
define('STATES_BOTH', 2); // show both completed and pending transactions

define('_N', 0); // used in tests
define('_XCH', 1);
define('_VPAY', 1);

define('RUSD_R', 0);
define('RUSD_USD', 1);
define('RUSD_BOTH', 2); // show both rCredits (0) and USD (1)

$GLOBALS['TX_DAYS'] = array(1 => 'today', 7 => '7 days', 30 => '30 days', 90 => '90 days', 180 => '6 months', 365 => '12 months', -1 => 'Year to date', -2 => 'Ever');

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  global $agtViaBoxHelp, $base_url;
  $agtViaBoxHelp = t('Agent is who did the transaction on your behalf.|Channel (Via) is where the transaction originated -- what type of device: Web, Smartphone, SMS, or Automatic.|Box is where the transaction originated -- which specific device.');
  $optHelp = $agtViaBoxHelp . '|' . t('Exchanges are 1-to-1 trades of US Dollars for rCredits (usually automatic).|PExchanges are daily automatic payment exchanges of rCredits between company and employee. The company pays the employee in US Dollars, then the employee trades back some US Dollars 1-to-1 for rCredits. The net effect is that the employee gets paid partly in US Dollars, partly in rCredits.');

  global $base_url;
  $mya = r\acct();
  
  extract(u\just('options period starting ending do xid download downloaded variety', $args)); 
  $report = $mya->admin ? (@$variety == 'stats') : FALSE; // no community txs list yet for non-admins
  if (!$report and @$do) if ($result = handleTx($do, $xid, $sta, $args)) return $result;

  $page = $report ? 'community' : 'history';
  u\setDft($period, TX_DEFAULT_PERIOD);
  if (strlen(@$options) != count($urlOpts = u\ray(TX_URL_OPTS))) $options = !$mya->co ? '200001' : '210001'; // defaults for TX_OPT_KEYS options (2=both)
  $options = array_combine($urlOpts, str_split($options));
  u\setDft($states, STATES_DONE);
//  u\setDft($currency, 2); // both
//  if (@$download) $options['currency']  = RUSD_BOTH; // always both for download
  extract($options);
  $options = join('', array_values($options));

  // Setup option array, including defaults and help
  $optDefaults = $opts = array();
  $optHelp = array_combine($keys = u\ray(TX_OPT_KEYS), u\ray($optHelp));
  foreach ($keys as $one) {
    $value = $opts[u\consta('tx_opt', $one)] = x\popHelp($one, $optHelp[$one]); // set TX_OPT_CHANNELS etc.
    if (@$$one) $optDefaults[] = $value; // set up option field values
  }
  
  list ($starting, $ending) = txDates(@$period, @$starting, @$ending);
  $url0 = @"$base_url/$page/period=$period&starting=$starting&ending=$ending&options=$options&variety=$variety";

  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  list ($header, $classes, $rows, $totals) = x\getTxRows($starting, $ending, $url0, $maxRows, compact(u\ray('download agents channels boxes')));
  
  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  $transactions = item("<div id=\"txs-title\">TRANSACTIONS</div>$list <br>&nbsp;");
  
  if ($report) {
    $agents = $summary = NULL;
  } else {
//    $reports = item(t("<a href=\"$base_url/history/1099-B\">Annual Tax Report</a>"));
    $summary = formField('markup', '', '', array('markup' => txSummary($starting, $ending)));
  }

  //$radios = array(t('rCredits'), t('USD'), t('both'));
//  $currency = hidField(2); //formField('radios', t('Currency:'), '', dft($currency), $radios); 
  $states = hidField($states);
  //$radios = array(t('completed'), t('open invoices'), t('both'));
//  $states = formField('radios', t('Types:'), '', dft($states), $radios);
  //$opts2Desc = TRUE ? '' : t('Agent is who did the transaction on your behalf. Channel ("Via") is Web, Smartphone, SMS, or Automatic. Exchanges are 1-to-1 trades of rCredits for US Dollars. PExchanges are 1-to-1 trades that earn incentive rewards.');
  // (NOT USED YET) $opts2 = boxesField('opts', t('Show:'), $opts2Desc, $optDefaults, $opts);
  
  $divider = item('<hr>');
  $period = formField('select', t('For the past:'), '', dft($period), $GLOBALS['TX_DAYS']);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  $downloadPeriod = submit(t('Download '));
  $starting = textField(t('OR Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\fmtDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $downloadDates = submit(t(' Download'));
  $title = "";
  $title = item(t('Transaction History'));

  $form = compact(u\ray('title subtext states opts2 divider period submitPeriod downloadPeriod starting ending submitDates downloadDates summary transactions'));
  if (@$downloaded) js("window.open(\"$url0&download=1\", \"download\");", 'inline', 'footer');
  return labeled($form);
}

function formTxs_validate($form, &$sta) {
  extract(u\just('period starting ending', $sta['input']));
  $op = op($sta);
  list ($starting, $ending, $period) = ($op == 'submitPeriod' or $op == 'downloadPeriod') ? txDates(@$period) : txDates('', @$starting, @$ending);
  u\preray(compact(u\ray('starting ending period')), $sta['input']);
}

function formTxs_submit($form, &$sta, $page = 'history') {
  if (confirming_s($sta)) return;
  extract(u\just('states period starting ending opts options do xid', $sta['input']));
  if (isset($states)) { // $options summarizes everything when clicking on OK or NO or EDIT
//    $options = $currency . $states;
    $options = $states;
    // (NOT USED YET) foreach (u\ray(TX_OPT_KEYS) as $one) $options .= (@$opts[u\consta('TX_OPT', $one)] ? 1 : 0);
  }
  $url = @"$page/period=$period&starting=$starting&ending=$ending&options=$options";
  $op = op($sta);
  if (u\abbreviates('download', $op)) {
    say('downloading');
//    if ($currency != 2) say('download always both');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

/**
 * List the available account statement dates, with links.
 */
function formStatements($form, $sta) {
  $mya = r\acct();
  $start = u\monthDay1($mya->created);
  $month = strtotime('-1 month', u\monthDay1(REQUEST_TIME));
  
  for (; $month >= $start; $month = strtotime('-1 month', $month)) {
    $year = date('Y', $month);
    $text = strftime('%b%Y', $month);
    $list[$year] = @$list[$year] . "<a href=\"statement/$text\" target=\"_blank\">$text</a><br>\n";
  }
  if (@$list) {
    $headers = $data = '';
    foreach ($list as $year => $v) {
      $headers .= "<th>$year</th>";
      $data .= "<td>$v</td>";
    }
    $list = "<table id=\"statements\">\n<tr>$headers</tr>\n<tr>$data</tr>\n</table>";
  } else $list = t('There are no statements available yet for this account.');
  
  $title = item(t('Account Statements'));
  $list = item($list);
  return compact('title', 'list');
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
  if (t\est() and @$sta['confirm']) return FALSE;
  if (confirming_v($sta)) return FALSE;
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return 'bad tx id'; // bail on hack attempt
  $subs = r\txReportArgs($mya, $details);
  $summary = u\tt('tx summary', $subs);
  extract(u\just('toMe byMe', $subs));
  $txAction = x\txAct($do, '', $toMe, $byMe);
  confirm(u\tt('tx summary|confirm tx action', $subs + compact('txAction')), $sta); // sets 'confirm'
  return sureForm($sta);
}

function handleTx($do, $xid, &$sta, $args) {
  if (!$xid = (int) @$xid) return FALSE;
  if ($do == 'edit') return txEdit($xid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm = txConfirm($do, $xid, $sta)) return $confirm;
    if (!$sta['rebuild']) txOKorNO($do, $xid); // don't do anything if canceling
    $url = str_replace('&do=', '&did=', "history/$args"); // don't do it again
    return r\go($url); // back to the normal form
  }
  return FALSE;
}
 
/**
 * Return usable starting and ending dates (inclusive).
 * @param int $period: index into TX_DAYS -- for calculating starting and ending if none explicitly specified
 * @param int $starting: explicit starting date (optional)
 * @param int $ending: explicit ending date (optional)
 * @return array($starting, $ending, $period), where $starting is adjusted to start of day and $ending to end of day
 */
function txDates($period, $starting = '', $ending = '') {
  if (@$starting and !is_numeric($starting) and $err = u\badDate($starting)) say($err, 'starting');
  if (@$starting > REQUEST_TIME) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and $err = u\badDate($ending)) say($err, 'ending');
  if (!@$ending or $ending > REQUEST_TIME) $ending = REQUEST_TIME;

  $period = (int) @$starting ? ($ending - $starting) / DAY_SECS : (@$period ?: TX_DEFAULT_PERIOD);
  $ago = @$GLOBALS['TX_DAYS'][$period] ?: ($period . ' days');
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : ($period == '-2' ? '1/1/2013' : "$ago ago"));
  return array(strtotime('today', $starting), strtotime('tomorrow', $ending) - 1, $period);
}  

/**
 * Handle click 'OK' or 'NO' (approving or undoing a transaction)
 */
function txOKorNO($task, $xid) {
  if (!be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  list ($message, $args) = be\undoTx($xid); // X (reverse it)
  say($message, $args);
}

/**
 * Return a form to edit the given transaction.
 * @param int $xid: the transaction id
 * @return the form (FALSE on error)
 */
function txEdit($xid) {
  $mya = r\acct();
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('taking toMe payer payee amount goods payerFor payeeFor', $details));
  
  if (!$mya->can($taking ? B_SELL : B_BUY)) return FALSE;
  $radios = array(t('cash/loan/etc.'), t('goods & services'));
  
  $form = array(    
    '#validate' => array('rCredits\\Web\\txEdit_validate'),
    '#submit' => array('rCredits\\Web\\txEdit_submit'),
  );

  $purpose = $toMe ? $payeeFor : $payerFor;
  $orig = hidField(serialize($details));
  $title = item(t('Edit Transaction'));
  $to = item(r\acct($toMe ? $payer : $payee)->fullName, $toMe ? t('From:') : t('To:'));
  $amount = textField(t('Amount: $'), '', dft(number_format($amount,2)));
  $goods = formField('radios', '&nbsp;', '', dft($goods), $radios);
  $purpose = textField(t('For:'), '', dft(u\nonish(@$purpose) ? '' : $purpose));
  $submit = submit(t('Update'));

  return labeled($form + compact(u\ray('title to amount goods purpose submit orig')));
}

function txEdit_validate($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('amount goods purpose orig', $sta['input']));
  extract($orig = unserialize($orig), EXTR_PREFIX_ALL, 'x');
  
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
///debug(compact('x_toMe','amount','x_amount'));
  if (!$mya->cAdmin and ($x_toMe ? ($amount > $x_amount) : ($amount < $x_amount))) {
    $who = r\acct($x_toMe ? $x_payer : $x_payee)->name;
    $dif = abs($amount - $x_amount);
    $action = $x_toMe ? t('increase') : t('decrease');
    $date = u\fmtDate($x_created);
    $amount = u\fmtAmt($amount);
    $purpose = urlencode($purpose);
    $a = "a href=\"$base_url/charge/who=$who&amount=$dif&goods=$goods&purpose=$purpose ($action amount of $date payment to $amount)\"";
    $amount = u\fmtAmt($x_amount);
    return say('illegal amount change', compact(u\ray('amount action a')), 'amount');
  }
  $sta['input']['amount'] = $amount;
}

function txEdit_submit($form, &$sta) {
  extract(u\just('amount goods purpose orig', $sta['input']));
  extract($orig = unserialize($orig), EXTR_PREFIX_ALL, 'x');

  $forField = $x_toMe ? 'payeeFor' : 'payerFor';
  if ($purpose != $orig[$forField]) r\setTxField($forField, $purpose, $x_xid);
  if ($goodsChange = ($goods != $x_goods)) r\setTxField('goods', $goods, $x_xid);
  if ($amtChange = ($amount != $x_amount)) r\setTxField('amount', $amount, $x_xid);
  if ($amtChange or $goodsChange) {
    $agent = r\agent();
    $msg = 'tx edited' . ($goodsChange ? '|new tx goods' : '') . ($amtChange ? '|new tx amount' : '');
    $what = $goods ? t('real goods or services') : t('cash/loan/etc');
    $amount = u\fmtAmt($amount);

    foreach (array($x_payer => $x_payerTid, $x_payee => $x_payeeTid) as $uid => $tid) {
      $who = $uid == $agent->id ? t('you') : $agent->fullName;
      r\notify($uid, $msg, compact(u\ray('who tid amount what')));
    }
  }
  return r\go('history', 'info saved');
}

function txBal($bal, $plus = '', $minus = '- ') {
  $bal = str_replace(',', '', $bal);
  return ($bal < 0 ? $minus : $plus) . '$' . number_format(abs($bal), 2);
}

/**
 * Return the transaction summary table
 * @param int $starting: starting time (the start of some day)
 * @param int $ending: ending time (the end of some day)
 */
function txSummary($starting, $ending) {
  $mya = r\acct();
  $myid = $mya->id;
  $previous_ending = $starting - 1;
  extract($nowInfo = (array) be\creditInfo(array('asof' => $ending)), EXTR_PREFIX_ALL, 'n');
  extract($pendingInfo = (array) be\creditInfo(u\ray('pending asof', TRUE, $ending)), EXTR_PREFIX_ALL, 'p');
  extract($oldInfo = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'o');
///  debug(compact('nowInfo','oldInfo'));

  $dif = array();
  foreach (u\ray('inProper outProper usdIn usdOut rewards bank fees') as $key) $dif[$key] = $nowInfo[$key] - $oldInfo[$key];
  extract($dif, EXTR_PREFIX_ALL, 'd');
  $p_committed = $mya->committed; // this will include all pending commitments
  
  $fields = 'old bank in out fees rewards new';
  $values = array($o_balance, $d_bank, $d_inProper, $d_outProper, $d_fees, $d_rewards, $n_balance); // both
  $values = array_combine(u\ray($fields), $values);

  $p_inProper = $p_outProper = $p_rewards = 0; // temporary until we include unpaid invoice data here
  $p_balance = $p_bank - $p_committed;
  $values += compact(u\ray('p_inProper p_bank p_outProper p_rewards p_committed p_balance'));
  foreach ($values as $key => $value) $$key = number_format($value, 2);
 
  $p_balance = txBal($p_balance, '+ '); // add +/-/$ to these, as appropriate
  $new = txBal($new);
  $old = txBal($old);

  $starting = u\fmtDate($starting);
  $ending = u\fmtDate($ending);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
  $classes = u\prefixValues('txsum-', u\ray('one banksign bank fromsign fromyou tosign toyou rewardssign rewards sharesign share balancesign balance'));
  $heads = u\ray(t('Starting balance,,From Bank,,From You,,To You,,Rewards,,To CGF,,Ending balance'));
  $headHelp = u\ray(t('Your account balance at the start of this time period||How much was transferred from your bank account to your rCredits account||How much you spent||How much you received||How much incentive rewards did you receive||Amount of rewards you committed to share with Common Good Finance||Your account balance at the end of this time period'));
  foreach ($heads as $key => $value) if ($value) $heads[$key] = x\popHelp($value, $headHelp[$key]);
  $PENDING = x\popHelp(t('PENDING'), t('Pending transfers, transaction requests, and commitments as of ') . $ending);
  
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => $heads),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old | + | $bank | - | $out | + | $in | + | $rewards |  |  | = | $new"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>$PENDING:</b> | + | $p_bank | - | $p_outProper | + | $p_inProper | + | $p_rewards | - | $p_committed | = | $p_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}

/**
 * List Invoices
 */
function formInvoices($form, &$sta, $toFrom, $args = '') {
  global $base_url;

  $toMe = ($toFrom == 'to');
  extract(u\just('period starting ending download downloaded states nvid do', $args)); 

  $choices = array(t('open'), t('paid'), t('denied'));
  u\setDft($states, $statesDft = '0,1,2');
  foreach (u\ray($states) as $k) $stateDfts[] = $choices[$k];
  
  $showAdvJs = 'jQuery(\'#advanced\').show(); jQuery(\'#simple\').hide();';
  if ($states != $statesDft or (@$ending and $ending != strtotime('tomorrow') - 1)) js($showAdvJs, 'inline', 'footer'); // showing advanced selection
  u\setDft($period, TX_DEFAULT_PERIOD);
  list ($starting, $ending) = txDates(@$period, @$starting, @$ending);
  $url0 = @"$base_url/history/invoices-$toFrom/period=$period&starting=$starting&ending=$ending&states=$states";

  if (@$do == 'ok') {$ok = be\payInvoice($nvid, $msg); say($msg, $ok ? '' : 'ERR');}
  if (@$do == 'no') return r\go("handle-invoice/nvid=$nvid&toMe=$toMe&ret=" . u\urlify($url0));
  
  $period = formField('select', t('For the past:'), '', dft($period), $GLOBALS['TX_DAYS']);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  $downloadPeriod = submit(t('Download '));
  $showAdvanced = item('<a class="nospin" href="javascript:' . $showAdvJs . '">advanced</a>');
  $simple = fieldSet('simple', compact(u\ray('period submitPeriod downloadPeriod showAdvanced')));
  
  $states2 = boxesField('states', t('Show:'), '', $stateDfts, $choices);
  $list = item(invoiceList($toMe, $starting, $ending, $states, $url0, @$download)); // must precede starting & ending
  $starting = textField(t('Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\fmtDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $downloadDates = submit(t(' Download'));
  $showSimple = item('<a class="nospin" href="javascript:jQuery(\'#advanced\').hide(); jQuery(\'#simple\').show();">simpler</a>');
  $advanced = fieldSet('advanced', compact(u\ray('states2 starting ending submitDates downloadDates showSimple')));

  $title = item(t('Invoices ') . ($toMe ? t('TO You') : t('FROM You')));
  $toFrom = hidField($toFrom);

  $form = compact(u\ray('title subtext simple advanced list toFrom'));
  if (@$downloaded) js("window.open(\"$url0&download=1\", \"download\");", 'inline', 'footer');
  return labeled($form);
}

/**
 * Prepare the list of invoices
 * @return string table-based HTML listing the relevant invoices
 */
function invoiceList($toMe, $starting, $ending, $states, $url, $download) {

  $headHelp = t('Invoice number|Date invoice was created|Who sent you the invoice|What is the the invoice for|Total invoice amount|Click a symbol to approve or deny an invoice');
  $classes = u\ray('nvid date name purpose amount buttons');
  $headers = t('Inv#,Date,Name,Purpose,Amount,Status');
  $headHelp = array_combine($classes, u\ray($headHelp));

  $header = array_combine($classes, u\ray($headers)); // make splices and additions easy and clear
  $downloadHeader = $header;
  foreach ($header as $key => $value) $header[$key] = x\popHelp($value, $headHelp[$key]);

  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  if (@$download) {
    $csv = x\setupDownload($starting, $ending);
    u\csv($downloadHeader);
  } else $csv = FALSE;

  $rows = array();
  list ($yu, $me) = u\order($toMe, 'payee', 'payer');
  $where = ":uid=i.$me AND i.created BETWEEN :starting AND :ending";
  if (strpos($states, '0') === FALSE) $where .= ' AND i.status<>:TX_PENDING';
  if (strpos($states, '1') === FALSE) $where .= ' AND i.status<=0';
  if (strpos($states, '2') === FALSE) $where .= ' AND i.status<>:TX_DENIED';
  
  $sql = <<<EOF
    SELECT nvid, i.created, u.fullName AS other, i.purpose, i.amount, i.status, t.{$me}Tid AS tid
    FROM r_invoices i 
    LEFT JOIN users u ON u.uid=i.$yu
    LEFT JOIN r_txs t ON t.xid=i.status
    WHERE $where
    ORDER BY i.status=:TX_PENDING, nvid DESC
EOF;

  $q = db\q($sql, u\ray('uid starting ending', r\acct()->id, $starting, $ending));
  while (count($rows) < $maxRows and $tx = $q->fetchAssoc()) {
    extract($tx);
    if (!@$download) $amount = number_format($amount, 2);
    $row = array($nvid, u\fmtDate($created), $other, $purpose, $amount);
    
    $do = @"$url&nvid=$nvid&do";

    if ($status > 0) {
      $button1 = $button2 = $button3 = '';
      $status = t('paid') . " (Tx#$tid)";
    } else {
      $button1 = $toMe ? button(t(' PAY '), "$do=ok", t('Pay this invoice')) : '';
      $button2 = (!$toMe ?: $status == TX_PENDING) ? button(' X ', "$do=no", $toMe ? t('Deny this invoice') : t('Delete this invoice')) : '';
//    $button3 = (!$toMe and $status == TX_PENDING) ? "<input type='image' title='Edit this invoice' src='$rUrl/images/icons/edit.png' />" : '';
      $button3 = '';
      $status = $status == TX_DENIED ? t('denied') : t('pending');
    }

    $buttons = "<div class='txs-buttons'><div>$status</div><div>$button1</div><div>$button2</div><div>$button3</div></div>";
    
    if (!@$download) $row[] = $buttons;
    $rows[] = $row;
  }
  if ($tx and $q->fetchAssoc()) say('transaction list truncated'); // there was more than we could show

  if (@$download) {if (t\est()) return; else exit();}

  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no invoices');
  return theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}

function formInvoices_validate($form, &$sta) {return formTxs_validate($form, $sta);}

function formInvoices_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  extract(u\just('states period starting ending do xid toFrom', $sta['input']));

  $states = @join(',', $states);
  $url = @"history/invoices-$toFrom/period=$period&starting=$starting&ending=$ending&states=$states";
  $op = op($sta);

  if (u\abbreviates('download', $op)) {
    say('downloading');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}