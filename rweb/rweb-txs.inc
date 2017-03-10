<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Txs as x;
use rCredits\Web as w;

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

$GLOBALS['TX_DAYS'] = array(1 => 'day', 7 => '7 days', 30 => '30 days', 90 => '90 days', 180 => '6 months', 365 => '12 months', -1 => 'Year to date');

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  w\downloadCheck('history/transactions');
  
  global $agtViaBoxHelp, $base_url;
  $agtViaBoxHelp = t('||Agent is who did the transaction on your behalf.|Channel (Via) is where the transaction originated -- what type of device: Web, Smartphone, SMS, or Automatic.|Box is where the transaction originated -- which specific device.');
  $optHelp = $agtViaBoxHelp . t('||Exchanges are 1-to-1 trades of US Dollars for %RCREDITS (usually automatic).|PExchanges are daily automatic payment exchanges of %RCREDITS between company and employee. The company pays the employee in US Dollars, then the employee trades back some US Dollars 1-to-1 for %RCREDITS. The net effect is that the employee gets paid partly in US Dollars, partly in %RCREDITS.');
  $mya = r\acct();

  extract(u\just('options period starting ending do xid download downloaded', $args)); 
  
  global $testPeriod; $testPeriod = @$period ?: TX_DEFAULT_PERIOD; // remember for tests
  if (@$do) if ($result = handleTx($do, $xid, $sta, $args)) return $result;
  // Setup option array, including defaults and help
  /*
  $optDefaults = $opts = [];
  $optHelp = array_combine($keys = ray(TX_OPT_KEYS), ray($optHelp));
  foreach ($keys as $one) {
    $value = $opts[u\consta('tx_opt', $one)] = w\popHelp($one, $optHelp[$one]); // set TX_OPT_CHANNELS etc.
    if (@$$one) $optDefaults[] = $value; // set up option field values
  }
  */
  
  extract(u\just('dateRange url starting ending', dateRange($args, 'history/transactions')));

//  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  $descending = TRUE;
  $optionSet = compact(ray('download agents channels boxes descending'));
//  list ($header, $classes, $rows, $totals) = x\getTxRows($starting, $ending, $optionSet, $url, $maxRows);
  $txRows = x\getTxRows($starting, $ending, $optionSet, $url);
  if (u\test() and @$download) return; // simulate exit, for tests
  list ($header, $classes, $rows, $totals) = $txRows;
  
  // prepare screen display
//  $classes = u\prefixValues('tx-', $classes);
//  $attributes = array('id' => 'txlist');
//  $cols = [];
//  foreach ($classes as $class) $cols[] = compact('class');
//  $colgroups = array($cols);
//  $caption = $sticky = '';
//  $list = theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));
  array_unshift($rows, $header); // put header at start
  
  $list = '';
  $count = -2;
  
  foreach ($rows as $row) {
    $count++;
    $row = array_combine($classes, $row);
    extract($row);
    $class = 'row txRow';
    if (!$list) $class .= ' head';
    $class .= ' page-' . floor($count / MIN_TX_ROWS);
    $class .= ' PAGE-' . floor($count / NORMAL_TX_ROWS);
    list ($c1, $c2, $c3, $c4, $reward) = $mya->rewarding ? [6, 3, 7, 5, $reward] : [7, 2, 12, 0, ''];
    
    $list .= <<<EOF
<div class="$class">
  <div class="tid-date col-xs-2">
    <div class="tid col-sm-5">$tid</div>
    <div class="date col-sm-7">$date</div>
  </div>
  <div class="name-purpose col-xs-$c1">
    <div class="name col-sm-6">$name</div>
    <div class="purpose col-sm-6">$purpose</div>
  </div>
  <div class="amount-reward col-xs-$c2">
    <div class="amount col-sm-$c3">$amount</div>
    <div class="reward col-sm-$c4">$reward</div>
  </div>
  <div class="buttons col-xs-1">
    $buttons
  </div>
</div>
EOF;
  }
  
  if ($count < 0) $list .= t('no txs');
  $pgFactor = MIN_TX_ROWS / NORMAL_TX_ROWS;
  
  $links = <<<EOF
<div id="txs-links" class="row">
  <div class="showMore col-xs-3">
    <a onclick="javascript:showMore($pgFactor);" title="Show more transactions per page"><span class="glyphicon glyphicon-plus"></span>Show more</a>
  </div>
  <div class="dates col-xs-2">
    <a onclick="javascript:$('#dateRange, #edit-submitPeriod, #edit-submitDates').show(); $('#edit-downloadPeriod, #edit-downloadDates').hide();" title="Select dates to show"><span class="glyphicon glyphicon-calendar"></span>Dates</a>
  </div>
  <div class="download col-xs-3">
    <a onclick="javascript:$('#dateRange, #edit-downloadPeriod, #edit-downloadDates').show(); $('#edit-submitPeriod, #edit-submitDates').hide();" title="Select dates and download as CSV file"><span class="glyphicon glyphicon-download-alt"></span>Download</a>
  </div>
  <div class="totals col-xs-2">
    <a data-toggle="modal" data-target="#txs-totals" title="Show totals for the selected period"><span class="glyphicon glyphicon-usd"></span>Totals</a>
  </div>
  <div class="nav col-xs-1">
    <a class="prevPage" onclick="javascript:showPage(-1);" title="Previous Page"><span class="glyphicon glyphicon-triangle-left"></span></a>
    <a class="nextPage" onclick="javascript:showPage(+1);" title="Next Page"><span class="glyphicon glyphicon-triangle-right"></span></a>
  </div>
</div>
EOF;

  $transactions = item("<div id=\"txlist\" class=\"table table-striped\">$links$list</div>");
  $summary = fld('markup', '', '', array('markup' => txSummary($starting, $ending)));

  $divider = item('<hr>');
  $title = item(t('Transaction History'));
  extract(u\just('simple advanced', $dateRange));
  $dateRange = w\fieldSet('dateRange', compact('simple', 'advanced'), BARE);
  $form = compact('title') + compact(ray('dateRange summary transactions states'));
  return labeled($form);
}

function formTxs_validate($form, &$sta) {w\dateRangeValidate($sta);}

function formTxs_submit($form, &$sta) {
  if (confirming_s($sta)) return;
    
  w\dateRangeRedirect('history/transactions', $sta);
}

/**
 * List the available account statement dates, with links.
 */
function formStatements($form, $sta, $month = '') {
  if ($month) return r\go("history/statement/$month");
  
  $mo = date('F');
  $subtext = date('d') < 10 ? t('note|Statements are not final until the 10th of the next month.', compact('mo')) : '';
  return monthChoices(t('Statements'), 'statements', $subtext);
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
  if (u\test() and @$sta['confirm']) return FALSE;
  if (confirming_v($sta)) return FALSE;
  if (!$myx = r\MyX::lastTx($mya, 'xid=:xid', compact('xid'), TRUE)) return 'bad tx id'; // bail on hack attempt
//  $myx = r\x($xid, $mya);
  $txAction = $do == 'ok' ? t('accept this charge') : $myx->msg('reverse');
  $summary = $myx->msg('actor', 'full', TRUE);
//  $summary = $byMe ? 'tx desc active' : 'tx desc passive';
  confirm("$summary. " . t('Okay to') . " $txAction?", $sta); // sets 'confirm'
  return sureForm($sta);
}

function handleTx($do, $xid, &$sta, $args) {
  $url = str_replace('&do=', '&did=', "history/transactions/$args"); // don't do it again

  if (substr($xid, 0, 1) == 'b') { // reversing bank transfer
    $mya = r\acct();
    if (!$mya->superAdmin) return FALSE; // only top admin can do this currently
    u\EXPECT($do == 'no', 'trying to edit bank transfer');
    $tx = db\lookup('*', 'r_usd', 'txid=:xid', ['xid'=>substr($xid, 1)]);
    foreach (u\just('txid amount tid', $tx) as $k => $v) $tx[$k] = -$v; // reverse transfer
    extract($tx);
    $mya->bank($amount, $zot, $tx);
    r\go($url, t('Reversed %amount bank transfer for %who.', 'amount who', u\fmtAmt(-$amount), $mya->fullName));
  }
  
  if (!$xid = (int) @$xid) return FALSE;
  if ($do == 'edit') return txEdit($xid);
  if ($do == 'ok' or $do == 'no') {
    if ($confirm = txConfirm($do, $xid, $sta)) return $confirm;
    if (!$sta['rebuild']) txOKorNO($do, $xid); // don't do anything if canceling
    return r\go($url); // back to the normal form
  }
  return FALSE;
}
 
/**
 * Handle click 'OK' or 'NO' (approving or undoing a transaction)
 */
function txOKorNO($task, $xid) {
  if (!r\MyX::lastTx(r\acct(), 'xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
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
  if (!$myx = r\MyX::lastTx($mya, 'xid=:xid', compact('xid'), TRUE)) return FALSE; // bail on hack attempt
  extract(u\just('taking toMe payer payee amount goods payerFor payeeFor created', $myx->ray));
  
  if (!$mya->can($taking ? B_SELL : B_BUY)) return FALSE;
  
  $form = array(    
    '#validate' => array('rCredits\\Web\\txEdit_validate'),
    '#submit' => array('rCredits\\Web\\txEdit_submit'),
  );

  $purpose = $toMe ? $payeeFor : $payerFor;
  $orig = hidFld(htmlspecialchars(serialize($myx->ray)));
  $title = item(t('Edit Transaction'));
  $to = item(r\acct($toMe ? $payer : $payee)->fullName, $toMe ? t('From:') : t('To:'));
  $amount = textFld(t('Amount: $'), '', dft(number_format($amount,2)));
  $goods = $mya->rewarding ? radiosFld('&nbsp;', '', dft((int) $goods), ray(R_WHYS)) : w\hidfld($goods);
  $purpose = textFld(t('For:'), '', dft(u\nonish(@$purpose) ? '' : $purpose));
  if ($mya->superAdmin) $created = textFld(t('Date:'), '', dft(u\fmtDate($created))); else unset($created);
  $submit = submit(t('Update'));

  return labeled($form + compact(ray('title to amount goods purpose created submit orig')));
}

function txEdit_validate($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('created amount goods purpose orig', $sta['input']));
  extract($orig = unserialize(htmlspecialchars_decode($orig)), EXTR_PREFIX_ALL, 'x');
  
  if ($goods == FOR_GOODS and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
///  debug(compact('x_toMe','amount','x_amount'));
  if (!$mya->cAdmin and ($x_toMe ? ($amount > $x_amount) : ($amount < $x_amount))) {
    $who = r\acct($x_toMe ? $x_payer : $x_payee)->name;
    $dif = abs($amount - $x_amount);
    $action = $x_toMe ? t('increase') : t('decrease');
    $date = u\fmtDate($x_created);
    $amount = u\fmtAmt($amount);
    $purpose = urlencode($purpose);
    $_a = w\atag("/charge/who=$who&amount=$dif&goods=$goods&purpose=$purpose ($action amount of $date payment to $amount)");
    $amount = u\fmtAmt($x_amount);
    return say(t('illegal amount change', compact(ray('amount action _a'))), 'amount');
  }
  if (@$created and $err = u\badDate($created)) return say($err, 'created');
  u\preray(compact('amount', 'created'), $sta['input']);
}

function txEdit_submit($form, &$sta) {
  extract(u\just('created amount goods purpose orig', $sta['input']));
  extract($orig = unserialize(htmlspecialchars_decode($orig)), EXTR_PREFIX_ALL, 'x');
  $a = r\acct(); $id = $a->id;
  $x = r\x($x_xid, $a);
  u\EXPECT($x, 'submitted non-tx!');
  u\EXPECT($a->cAdmin or $x->payer == $id or $x->payee == $id, "not our xid:$x_xid id=$id");

  $forField = $x_toMe ? 'payeeFor' : 'payerFor';
  if ($purpose != $orig[$forField]) $x->setField($forField, $purpose);
  if ($goodsChange = ($goods != $x_goods)) $x->setField('goods', $goods);
  if ($amtChange = ($amount != $x_amount)) $x->setField('amount', $amount);
  if (isset($created) and $created != $x_created) $x->setField('created', $created);
  if ($amtChange or $goodsChange) {
    $agent = r\agent();
    $msg = 'tx edited' . ($goodsChange ? '|new tx goods' : '') . ($amtChange ? '|new tx amount' : '');
    $what = ray(R_WHYS)[$goods];
    $amount = u\fmtAmt($amount);

    foreach ([$x_payer => $x_payerTid, $x_payee => $x_payeeTid] as $uid => $tid) {
      $who = $uid == $agent->id ? t('you') : $agent->fullName;
      r\notify($uid, $msg, compact(ray('who tid amount what')));
    }
  }
  return r\go('history', 'info saved');
}

/*
function txBal($bal, $plus = '', $minus = '- ') {
  $bal = str_replace(',', '', $bal);
  return ($bal < 0 ? $minus : $plus) . '$' . number_format(abs($bal), 2);
}
*/

/**
 * Return the transaction summary table
 * @param int $starting: starting time (the start of some day)
 * @param int $ending: ending time (the end of some day)
 */
function txSummary($starting, $ending) {
  $mya = r\acct();
  $myid = $mya->id;
  
  $endingNow = ($ending > r\rTime());
  $previous_ending = $starting - 1;
  extract($nowInfo = (array) be\creditInfo(array('asof' => $ending)), EXTR_PREFIX_ALL, 'n');
  extract($pendingInfo = (array) be\creditInfo(ray('pending asof', TRUE, $ending)), EXTR_PREFIX_ALL, 'p');
  extract($oldInfo = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'o');
///  debug(compact('nowInfo','oldInfo'));

  $dif = [];
  foreach (ray('inProper outProper rewards bank savings roundups') as $key) $dif[$key] = $nowInfo[$key] - $oldInfo[$key];
  extract($dif, EXTR_PREFIX_ALL, 'd');
  $p_committed = $mya->committed; // this will include all pending commitments
  $d_savings -= $d_rewards;
  list ($toSavings, $fromSavings) = u\order($d_savings >= 0, abs($d_savings), 0);
  $d_outProper += $toSavings;
  $d_inProper += $fromSavings;
  
  $fields = 'old bank in out rewards new';
  $values = array($o_balance, $d_bank, $d_inProper, $d_outProper, $d_rewards, $n_balance); // both
  $values = array_combine(ray($fields), $values);

  $p_inProper = $p_outProper = $p_rewards = 0; // temporary until we include unpaid invoice data here
  $p_balance = $p_bank - $p_committed;
  $values += compact(ray('p_inProper p_bank p_outProper p_rewards p_committed p_balance n_savings o_savings fromSavings toSavings d_roundups'));

  foreach ($values as $key => $value) $$key = number_format($value, 2);

  $starting = u\fmtDate($starting);
  $ending = u\fmtDate($ending);
  
//  $classes = u\prefixValues('txsum-', ray('one banksign bank fromsign fromyou tosign toyou sharesign share balancesign balance'));
  $classes = u\prefixValues('txsum-', ray('label sign1 amount sign2 savings other'));
/*
  $heads = ray(t('Starting balance,,From Bank,,From You,,To You,,To CGF,,Ending balance'));
  $headHelp = ray(t('Your account balance at the start of this time period||How much was transferred from your bank account to your %PROJECT Account||How much you spent||How much you received||Amount of rewards you committed to share with Common Good Finance||Your account balance at the end of this time period'));
  foreach ($heads as $key => $value) if ($value) $heads[$key] = w\popHelp($value, $headHelp[$key]);
  */
  
  $PRIMARY = w\popHelp(t('Balance'), t('Your balance is what you use for spending and for receiving payments.'));
  $STARTING = w\popHelp(t('Starting Balance'), t('Your %PROJECT Account balance at the start of this time period'));
  $BANK = w\popHelp(t('From Bank'), t('How much was transferred from your bank account to your %PROJECT Account'));
  $IN = w\popHelp(t('Funds Received'), t('How many %RCREDITS you received, in this period'));
  $OUT = w\popHelp(t('Funds Paid Out*'), t('How many %RCREDITS you paid out, in this period'));
  $REWARDS = w\popHelp(t('Rewards'), t('Incentive rewards you received, in this period (rewards go automatically into your Credit Reserve)'));
  $ENDING = w\popHelp(t('Ending Balance'), t('Your %PROJECT Account balance at the end of this time period'));
  $SAVINGS = w\popHelp(t('Credit Reserve'), t('Amount in your Credit Reserve, to spend in a pinch (once your %PROJECT community declares itself stable, this additional amount is yours to keep)'));
  $PENDING = w\popHelp(t('pending'), t('Pending transfers from your bank account to your %PROJECT Account'));
  if ($p_bank) $p_bank = $endingNow ? "+ $p_bank $PENDING" : '';
  
  $p_balance = u\starts($p_balance, '-') ? '- ' . substr($p_balance, 1) : "+ $p_balance";
/*  
  $rows = array(
    array('id' => 'txsum-dates', 'data' => ray("$starting,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => $heads),
    array(
      'id' => 'txsum-now', 
      'data' => ray("$$old | + | $bank | - | $out | + | $in |  |  | = | $$new"),
    ),
    ['id' => 'txsum-savings', 'data' => ray("<b>$SAVINGS:</b> + $o_savings||||| + | $rewards |($REWARDS)|||+ $n_savings")],
    array(
      'id' => 'txsum-pending',
      'data' => ray("<b>$PENDING:</b> || $p_bank || || | - | $p_committed || $p_balance"),
    ),
  );
  */

  $toSavings = $fromSavings = '';
  $rows = $mya->rewarding 
  ? [  
    ray("||$PRIMARY||$SAVINGS|"),
    ray("$STARTING||$old||$o_savings|$starting"),
    ray("$BANK|+|$bank|||$p_bank"),
    ray("$IN|+|$in||$toSavings|"),
    ray("$OUT|-|$out||$fromSavings|"),
    ray("$REWARDS||||+ &nbsp; $rewards|"),
    ray("$ENDING||$new||$n_savings|$ending"),
  ]
  : [
    ray("||$PRIMARY|"),
    ray("$STARTING||$old|$starting"),
    ray("$BANK|+|$bank|"),
    ray("$IN|+|$in|"),
    ray("$OUT|-|$out|"),
    ray("$ENDING||$new|$ending"),
  ];  
/*
    ray("||$PRIMARY||$SAVINGS|"),
    ray("$STARTING||$old|+|$o_savings|$starting"),
    ray("$BANK|+|$bank|||$p_bank"),
    ray("$IN|+|$in|+|$toSavings|"),
    ray("$OUT|-|$out|-|$fromSavings|"),
    ray("$REWARDS|||+|$rewards|"),
    ray("$ENDING||$new|+|$n_savings|$ending"),
*/    
  
  $attributes = ['id' => 'txsum'];
  $header = $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
//  $colgroups = array($cols);
  $colgroups = [];
  $caption = $sticky = $empty = '';
  $txsum = \theme_table(compact(ray('header rows attributes caption colgroups sticky empty')))
  . '<small><br> &nbsp;' . t('* Including $%roundups in rounded-up change donated to the community fund', 'roundups', $d_roundups) . '</small>';
//  js("$('#txsum0').modal('show');", 'inline', 'footer', 9999);
  return modal('txs-totals', t('Period Totals'), '', $txsum, 'X');
}

/**
 * List Invoices
 */
function formInvoices($form, &$sta, $toFrom, $args = '') {
  w\downloadCheck('history/invoices-' . $toFrom);

  $toMe = ($toFrom == 'to');
  extract(u\just('period starting ending download downloaded states nvid do', $args)); 

  $choices = array(t('open'), t('paid'), t('denied'));
  extract(u\just('dateRange url starting ending states', dateRange($args, "history/invoices-$toFrom", $choices, '0,1,2')));

  if (@$do == 'ok') {$ok = be\payInvoice($nvid, $msg); say($msg, $ok ? '' : 'ERR');}
  if (@$do == 'no') return r\go("handle-invoice/nvid=$nvid&toMe=$toMe&ret=" . u\urlify($url));
  
  $list = item(invoiceList($toMe, $starting, $ending, $states, $url, @$download)); // must precede starting & ending

  $title = item(t('Invoices ') . ($toMe ? t('TO You') : t('FROM You')));
  $toFrom = hidFld($toFrom);

  //simple advanced
  $form = compact('title') + $dateRange + compact(ray('list toFrom'));
  if (@$downloaded) js("window.open(\"$url&download=1\", \"download\");", 'inline', 'footer');
  return labeled($form);
}

function formInvoices_validate($form, &$sta) {w\dateRangeValidate($sta);}

function formInvoices_submit($form, &$sta) {
  dateRangeRedirect('history/invoices-' . $sta['input']['toFrom'], $sta);
}

/**
 * Prepare the list of invoices
 * @return string table-based HTML listing the relevant invoices
 */
function invoiceList($toMe, $starting, $ending, $states, $url, $download) {

  $headHelp = t('Invoice number,Date invoice was created,Who sent you the invoice,What is the the invoice for,Total invoice amount,Click a symbol to approve or deny an invoice');
  $classes = ray('nvid date name purpose amount buttons');
  $headers = t('Inv#,Date,Name,Purpose,Amount,Status');
  $headHelp = array_combine($classes, ray($headHelp));

  $header = array_combine($classes, ray($headers)); // make splices and additions easy and clear
  $downloadHeader = $header;
  foreach ($header as $key => $value) $header[$key] = w\popHelp($value, $headHelp[$key]);

  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  if (@$download) {
    $csv = x\setupDownload($starting, $ending);
    u\csv($downloadHeader);
  } else $csv = FALSE;

  $rows = [];
  list ($yu, $me) = u\order($toMe, 'payee', 'payer');
  $where = ":uid=i.$me AND i.created BETWEEN :starting AND :ending";
  /*
  This is broken anyway
  if (strpos($states, '0') === FALSE) $where .= ' AND i.status<>:TX_PENDING';
  if (strpos($states, '1') === FALSE) $where .= ' AND i.status<=0';
  if (strpos($states, '2') === FALSE) $where .= ' AND i.status<>:TX_DENIED';
  */
  $sql = <<<EOF
    SELECT nvid, i.created, u.fullName AS other, i.purpose, i.amount, i.status, t.{$me}Tid AS tid
    FROM r_invoices i 
    LEFT JOIN users u ON u.uid=i.$yu
    LEFT JOIN r_txs t ON t.xid=i.status
    WHERE $where
    ORDER BY i.status=:TX_PENDING, nvid DESC
EOF;

  $mya = r\acct();
  $q = db\q($sql, ray('uid starting ending', $mya->id, $starting, $ending));
///   debug(compact('sql','starting','ending', 'mya'));
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

  if (@$download) {if (u\test()) return; else exit();}

  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = t('no invoices');
  return theme_table(compact(ray('header rows attributes caption colgroups sticky empty')));
}
