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

$GLOBALS['TX_DAYS'] = array(1 => 'day', 7 => '7 days', 30 => '30 days', 90 => '90 days', 180 => '6 months', 365 => '12 months', -1 => 'Year to date', -2 => 'Ever');

/**
 * List Transactions
 */
function formTxs($form, &$sta, $args = '') {
  global $agtViaBoxHelp;
  $agtViaBoxHelp = '|' . t('Agent is who did the transaction on your behalf.|Channel (Via) is where the transaction originated -- what type of device: Web, Smartphone, SMS, or Automatic.|Box is where the transaction originated -- which specific device.');
  $optHelp = $agtViaBoxHelp . '|' . t('Exchanges are 1-to-1 trades of US Dollars for rCredits (usually automatic).|PExchanges are daily automatic payment exchanges of rCredits between company and employee. The company pays the employee in US Dollars, then the employee trades back some US Dollars 1-to-1 for rCredits. The net effect is that the employee gets paid partly in US Dollars, partly in rCredits.');

  $mya = r\acct();
  
  extract(u\just('options period starting ending do xid download downloaded', $args)); 
  
  global $testPeriod; $testPeriod = @$period ?: TX_DEFAULT_PERIOD; // remember for tests
  if (@$do) if ($result = handleTx($do, $xid, $sta, $args)) return $result;
  // Setup option array, including defaults and help
  /*
  $optDefaults = $opts = [];
  $optHelp = array_combine($keys = u\ray(TX_OPT_KEYS), u\ray($optHelp));
  foreach ($keys as $one) {
    $value = $opts[u\consta('tx_opt', $one)] = w\popHelp($one, $optHelp[$one]); // set TX_OPT_CHANNELS etc.
    if (@$$one) $optDefaults[] = $value; // set up option field values
  }
  */
  
  extract(u\just('dateRange url starting ending', dateRange($args, 'history/transactions')));
  $maxRows = @$download ? PHP_INT_MAX : TX_MAX_ROWS; // don't overwhelm the screen
  list ($header, $classes, $rows, $totals) = x\getTxRows($starting, $ending, $url, $maxRows, compact(u\ray('download agents channels boxes')));
  
  // prepare screen display
  $classes = u\prefixValues('tx-', $classes);
  $attributes = array('id' => 'txlist');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no txs');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  $transactions = item("<div id=\"txs-title\">TRANSACTIONS</div>$list <br>&nbsp;");
  $summary = fld('markup', '', '', array('markup' => txSummary($starting, $ending)));

  $divider = item('<hr>');
  $title = "";
  $title = item(t('Transaction History'));
  $dataRange = '';
  $form = compact('title') + $dateRange + compact(u\ray('summary transactions states'));
  if (@$downloaded) js("window.open(\"$url&download=1\", \"download\");", 'inline', 'footer');
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
  $subtext = date('d') < 10 ? tt('note|Statements are not final until the 10th of the next month.', compact('mo')) : '';
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
  if (!$details = be\lastTx('xid=:xid', compact('xid'), TRUE)) return 'bad tx id'; // bail on hack attempt
  $myx = r\x($xid, $mya);
  $txAction = $do == 'ok' ? t('accept this charge') : $myx->msg('reverse');
  $summary = $myx->msg('actor', 'full', TRUE);
//  $summary = $byMe ? 'tx desc active' : 'tx desc passive';
  confirm("$summary. " . t('Okay to') . " $txAction?", $sta); // sets 'confirm'
  return sureForm($sta);
}

function handleTx($do, $xid, &$sta, $args) {
  $url = str_replace('&do=', '&did=', "history/transactions/$args"); // don't do it again

  if (substr($xid, 0, 1) == 'b') { // reversing bank transfer
    if (!r\acct()->admin) return FALSE; // only admin can do this currently
    u\EXPECT($do == 'no', 'trying to edit bank transfer');
    $tx = db\lookup('*', 'r_usd', 'txid=:xid', ['xid'=>substr($xid, 1)]);
    foreach (u\just('txid amount tid', $tx) as $k => $v) $tx[$k] = -$v; // reverse transfer
    extract($tx);
    $mya = r\acct();
    $mya->bank($amount, $zot, $tx);
    r\go($url, tt('Reversed @amount bank transfer for @who.', u\ray('amount who', u\fmtAmt(-$amount), $mya->fullName)));
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
  
  $form = array(    
    '#validate' => array('rCredits\\Web\\txEdit_validate'),
    '#submit' => array('rCredits\\Web\\txEdit_submit'),
  );

  $purpose = $toMe ? $payeeFor : $payerFor;
  $orig = hidFld(htmlspecialchars(serialize($details)));
  $title = item(t('Edit Transaction'));
  $to = item(r\acct($toMe ? $payer : $payee)->fullName, $toMe ? t('From:') : t('To:'));
  $amount = textFld(t('Amount: $'), '', dft(number_format($amount,2)));
  $goods = radiosFld('&nbsp;', '', dft((int) $goods), u\ray(R_WHYS));
  $purpose = textFld(t('For:'), '', dft(u\nonish(@$purpose) ? '' : $purpose));
  $submit = submit(t('Update'));

  return labeled($form + compact(u\ray('title to amount goods purpose submit orig')));
}

function txEdit_validate($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('amount goods purpose orig', $sta['input']));
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
    $a = "a href=\"$base_url/charge/who=$who&amount=$dif&goods=$goods&purpose=$purpose ($action amount of $date payment to $amount)\"";
    $amount = u\fmtAmt($x_amount);
    return say('illegal amount change', compact(u\ray('amount action a')), 'amount');
  }
  $sta['input']['amount'] = $amount;
}

function txEdit_submit($form, &$sta) {
  extract(u\just('amount goods purpose orig', $sta['input']));
  extract($orig = unserialize(htmlspecialchars_decode($orig)), EXTR_PREFIX_ALL, 'x');
  $x = r\x($x_xid);
  u\EXPECT($x, 'submitted non-tx!');

  $forField = $x_toMe ? 'payeeFor' : 'payerFor';
  if ($purpose != $orig[$forField]) $x->setField($forField, $purpose);
  if ($goodsChange = ($goods != $x_goods)) $x->setField('goods', $goods);
  if ($amtChange = ($amount != $x_amount)) $x->setField('amount', $amount);
  if ($amtChange or $goodsChange) {
    $agent = r\agent();
    $msg = 'tx edited' . ($goodsChange ? '|new tx goods' : '') . ($amtChange ? '|new tx amount' : '');
    $what = u\ray(R_WHYS)[$goods];
    $amount = u\fmtAmt($amount);

    foreach ([$x_payer => $x_payerTid, $x_payee => $x_payeeTid] as $uid => $tid) {
      $who = $uid == $agent->id ? t('you') : $agent->fullName;
      r\notify($uid, $msg, compact(u\ray('who tid amount what')));
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
  extract($pendingInfo = (array) be\creditInfo(u\ray('pending asof', TRUE, $ending)), EXTR_PREFIX_ALL, 'p');
  extract($oldInfo = (array) be\creditInfo(array('asof' => $previous_ending)), EXTR_PREFIX_ALL, 'o');
///  debug(compact('nowInfo','oldInfo'));

  $dif = [];
  foreach (u\ray('inProper outProper rewards bank savings') as $key) $dif[$key] = $nowInfo[$key] - $oldInfo[$key];
  extract($dif, EXTR_PREFIX_ALL, 'd');
  $p_committed = $mya->committed; // this will include all pending commitments
  $d_savings -= $d_rewards;
  list ($toSavings, $fromSavings) = u\order($d_savings >= 0, abs($d_savings), 0);
  $d_outProper += $toSavings;
  $d_inProper += $fromSavings;
  
  $fields = 'old bank in out rewards new';
  $values = array($o_balance, $d_bank, $d_inProper, $d_outProper, $d_rewards, $n_balance); // both
  $values = array_combine(u\ray($fields), $values);

  $p_inProper = $p_outProper = $p_rewards = 0; // temporary until we include unpaid invoice data here
  $p_balance = $p_bank - $p_committed;
  $values += compact(u\ray('p_inProper p_bank p_outProper p_rewards p_committed p_balance n_savings o_savings fromSavings toSavings'));

  foreach ($values as $key => $value) $$key = number_format($value, 2);

  $starting = u\fmtDate($starting);
  $ending = u\fmtDate($ending);
  $summary_title = "<div id='txsum-title'>SUMMARY</div>";
  
//  $classes = u\prefixValues('txsum-', u\ray('one banksign bank fromsign fromyou tosign toyou sharesign share balancesign balance'));
  $classes = u\prefixValues('txsum-', u\ray('label sign1 amount sign2 savings other'));
/*
  $heads = u\ray(t('Starting balance,,From Bank,,From You,,To You,,To CGF,,Ending balance'));
  $headHelp = u\ray(t('Your account balance at the start of this time period||How much was transferred from your bank account to your rCredits account||How much you spent||How much you received||Amount of rewards you committed to share with Common Good Finance||Your account balance at the end of this time period'));
  foreach ($heads as $key => $value) if ($value) $heads[$key] = w\popHelp($value, $headHelp[$key]);
  */
  
  $PRIMARY = w\popHelp(t('Balance'), t('Your balance is what you use for spending and for receiving payments.'));
  $STARTING = w\popHelp(t('Starting Balance'), t('Your rCredits account balance at the start of this time period'));
  $BANK = w\popHelp(t('From Bank'), t('How much was transferred from your bank account to your rCredits account'));
  $IN = w\popHelp(t('Funds Received'), t('How many rCredits you received, in this period'));
  $OUT = w\popHelp(t('Funds Paid Out'), t('How many rCredits you paid out, in this period'));
  $REWARDS = w\popHelp(t('Rewards'), t('Incentive rewards you received, in this period (rewards go automatically into your Credit Reserve)'));
  $ENDING = w\popHelp(t('Ending Balance'), t('Your rCredits account balance at the end of this time period'));
  $SAVINGS = w\popHelp(t('Credit Reserve'), t('Amount in your Credit Reserve, to spend in a pinch (once your rCredits community declares itself a success, this additional amount is yours to keep)'));
  $PENDING = w\popHelp(t('pending'), t('Pending transfers from your bank account to your rCredits account'));
  if ($p_bank) $p_bank = $endingNow ? "+ $p_bank $PENDING" : '';
  
  $p_balance = u\starts($p_balance, '-') ? '- ' . substr($p_balance, 1) : "+ $p_balance";
/*  
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => $heads),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$$old | + | $bank | - | $out | + | $in |  |  | = | $$new"),
    ),
    ['id' => 'txsum-savings', 'data' => u\ray("<b>$SAVINGS:</b> + $o_savings||||| + | $rewards |($REWARDS)|||+ $n_savings")],
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>$PENDING:</b> || $p_bank || || | - | $p_committed || $p_balance"),
    ),
  );
  */

  $rows = [  
    u\ray("||$PRIMARY||$SAVINGS|"),
    u\ray("$STARTING||$old|+|$o_savings|$starting"),
    u\ray("$BANK|+|$bank|||$p_bank"),
    u\ray("$IN|+|$in|+|$toSavings|"),
    u\ray("$OUT|-|$out|-|$fromSavings|"),
    u\ray("$REWARDS|||+|$rewards|"),
    u\ray("$ENDING||$new|+|$n_savings|$ending"),
  ];
  
  $attributes = array('id' => 'txsum');
  $header = $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
//  $colgroups = array($cols);
  $colgroups = [];
  $caption = $sticky = $empty = '';
  return $summary_title . theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}

/**
 * List Invoices
 */
function formInvoices($form, &$sta, $toFrom, $args = '') {
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
  $form = compact('title') + $dateRange + compact(u\ray('list toFrom'));
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

  $headHelp = t('Invoice number|Date invoice was created|Who sent you the invoice|What is the the invoice for|Total invoice amount|Click a symbol to approve or deny an invoice');
  $classes = u\ray('nvid date name purpose amount buttons');
  $headers = t('Inv#,Date,Name,Purpose,Amount,Status');
  $headHelp = array_combine($classes, u\ray($headHelp));

  $header = array_combine($classes, u\ray($headers)); // make splices and additions easy and clear
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
  $q = db\q($sql, u\ray('uid starting ending', $mya->id, $starting, $ending));
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
  $empty = tt('no invoices');
  return theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}
