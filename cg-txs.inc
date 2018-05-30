<?php
namespace CG\Txs; // typically abbreviated as "x"
use CG\Backend as be;
use CG\Util as u;
use CG as r;
use CG\DB as db;
use CG\Txs as x;
use CG\Web as w;

/**
 * @file
 * Auxiliary functions for listing transactions
 */

define('DT_MAX', 100000000000); // bigger than any date
define('TX_TID_MAX', 100000000); // zero-filler
 
define('TX_OPT_KEYS' , 'agents channels boxes exchanges pexchanges'); // these are all placeholders now
define('TX_URL_OPTS', 'states ' . TX_OPT_KEYS);
define('TX_OPT_AGENTS', 1);
define('TX_OPT_CHANNELS', 2);
define('TX_OPT_BOXES', 3);
define('TX_OPT_EXCHANGES', 4);
define('TX_OPT_PEXCHANGES', 5);

/**
 * Get a formatted list of transaction records.
 * @param int $starting: starting date
 * @param int $ending: ending date
 * @param assoc $options: miscellaneous filters and flags
 * @param string $url0: original URL for the page, including parameters
 * @param numeric $run: (also RETURNED) running total
 * @return [$header, $classes, $rows, $totals]
 */
function getTxRows($starting, $ending, $options = [], $url0 = '', &$run = '') {
  global $mya;
  $maxRows = PHP_INT_MAX;
  
  extract(just('download agents channels boxes descending', $options));
  if ($mya->jid) $agents = TRUE; // show which partner did it, on joint account
  list ($classes, $header, $downloadHeader) = txFields(@$download, @$agents, @$channels, @$boxes);

  if (@$download) {
    setupDownload($starting, $ending);
    u\csv($downloadHeader);
  }
  $totDone = $totPending = array_combine($classes, array_fill(0, count($classes), FALSE));
  $totDone['roundups'] = 0; // track total donated change
  
  // build the transaction list
//  $dones = ($states != STATES_PENDING);
//  $pendings = ($states != STATES_DONE);
//  $txOpts = compact(ray('dones pendings exchanges pexchanges'));
  $txOpts = compact(ray('dones pendings', TRUE, FALSE));
  $rows = [];
  
//    $q = $func($starting, $ending, $report ? 'ctty' : $mya->id, $txOpts);
  $q = be\myTransactions($starting, $ending, $mya->id, $mya->jid);
  while (count($rows) < $maxRows and $tx = $q->fetchAssoc()) {
    doRow($rows, $tx, $totDone, $classes, @$download, @$agents, @$channels, @$boxes, $url0, $run);
  }

  if (@$tx and $q->fetchAssoc()) r\Web\say('transaction list truncated'); // there was more than we could show
  if (@$download) {
    unset($totDone['roundups']); // donated change doesn't show in downloaded data
    u\splice($totDone, 'buttons', 1); // don't download buttons
    u\splice($totDone, 'amount', 1, ['frombank' => 0]);
    $totDone['date'] = '';
    $totDone['name'] = 'TOTALS';
    foreach(just(TX_DOWNLOAD_MONEY_FIELDS, $totDone) as $key => $value) $totDone[$key] = round($value, 2);
    //if ($value === FALSE) $totDone[$key] = '-';
    if (!$mya->cttyRewardy) $totDone['reward'] = ''; // this is running balance when not rewards
    u\csv(array_values($totDone));
    
    global $testCSV; if (u\test()) return FALSE;
    exit();
  }
  
  return [$header, $classes, sortRows($rows, @$descending), $totDone];
}

/**
 * Prepare one transaction row for output (or download) and add it to the rows array.
 * @param assoc $tx: the transaction record
 */
function doRow(&$rows, $tx, &$tot, $classes, $download = FALSE, $agents = FALSE, $channels = FALSE, $boxes = FALSE, $url0 = '', &$run = '') {
  global $rUrl, $mya;
  
//  foreach (ray('xid amount byMe toMe reallyToMe agent data sharing tid') as $k) $$k = $myx->$k;
  extract(just('xid fromyou toyou reward byMe toMe reallyToMe agent data flags roundup disputed sharing tid name type created description', $tx));
//  $myx = r\x($tx, $mya);

  if (@$data) $data = unserialize($data);
  if (u\getBit(@$flags, B_ROUNDUPS) and !$toMe) return; // don't show contributions of roundups except to recipient (CGF)

  $change = ceil($fromyou) - $fromyou;
  $fromyou0 = $fromyou;
  if (@$roundup and !$toMe and $change > .005) $fromyou = ceil($fromyou); else $change = 0;
  $tx['amount'] = $amt = $toMe ? $toyou : -$fromyou; // this may be different from $tx[$amount]
  
  $reallyToMe = ($toMe xor $toyou < 0);
  $banking = ($type == TX_BANK);
//  if ($banking and ($amount < 0 xor $xid < 0)) $amount = -$amount; // show as positive unless reversing
//  $isExtra = ($mya->cttyRewardy and $type != TX_TRANSFER and !$banking);
  $isExtra = (in($type, TX_REWARDS) and !$banking);
  if ($isExtra or $sharing += 0) {
    if (!$mya->cttyRewardy) {
      return;
//      if ($download) $fromyou = $toyou = 0; else return;
    }
//    list ($amt, $reward) = [0, $amt];
  }
//  $state = txColoredState($data);
  
//  if ($sharing) list ($reward, $amount) = [$toMe ? $amount : -$amount, 0]; else $reward = 0;
//  $reward += $banking ? 0 : ($toMe ? (@$data['bonus'] + @$data['xfee']) : (@$data['rebate'] - @$data['xfee']));
//  if ($type == TX_TRANSFER) $reward += $toMe ? @$payeeReward : @$payerReward;
//  $net = number_format(($toMe ? $amount : -$amount) + $reward, 2);
  $net = number_format($mya->cttyRewardy ? $amt + $reward : $amt, 2);
  
//  if (isset($data['xfee'])) {$amount = -$amount; $toMe = !$toMe;} // ???
/*  if ($toMe and $fromMe) {
    $toYou = $fromYou = "<span class=\"virtual\">$amount</span>"; // make old draws clear
  } else 
  */
//  list ($toYou, $fromYou) = u\order($toMe, $amount, R_NONE);
  
  $url = "$url0&xid=" . ($banking ? 'b' : '') . $xid;
  $button1 = ($mya->can(@$reallyToMe ? B_SELL : B_BUY) and !$isExtra and !$banking) ? "<a href=\"$url&do=edit\" title=\"Edit Transaction\"><img src=\"$rUrl/images/icons/edit.png\" width=16 height=16 border=0 /></a>" : '';
//  list ($msg, $title) = [t('Reverse this transaction?'), t('Reverse')];
  $title = t('Reverse');
//  $onclick = "yesno('$msg', function () {location.href='$url&do=no';}); return false;";
//  $button2 = ((($mya->can(@$reallyToMe ? B_BUY : B_SELL) and !$isExtra and !$banking) or $mya->superAdmin) and !@$data['undoneBy']) ? w\button(' X ', "$url&do=no", r\MyX::msg0($tx, 'reverse')) : ''; // (admin not cAdmin)
//  $button2 = ((($mya->can(@$reallyToMe ? B_BUY : B_SELL) and !$isExtra and !$banking) or $mya->superAdmin) and !@$data['undoneBy']) ? w\btn("$url&do=no", ' X ', 'primary', 'xs', compact('onclick', 'title')) : ''; // (admin not cAdmin)
  $button2 = ((($mya->can(@$reallyToMe ? B_BUY : B_SELL) and !$isExtra and !$banking) or $mya->superAdmin) and !@$data['undoneBy']) ? w\btn("$url&do=no", ' X ', 'primary', 'xs', compact('title')) : ''; // (admin not cAdmin)
  //was txAct($toMe, $byMe, $reallyToMe)
  $button3 = (!$byMe and $disputed) ? w\button('OK', "$url&do=ok", r\MyX::msg0($tx, 'theyDid')) : ''; // w\txAct($toMe, $byMe, $reallyToMe)
//  if (!$mya->can(B_MANAGE)) $button1 = $button2 = $button3 = '';
//  $buttons = "<div class='txs-buttons'><div>$button1</div><div>$button2</div><div>$button3</div></div>";
//  $buttons = "<div class=\"col-sm-4\">$button1</div><div class=\"col-sm-4\">$button2</div><div class=\"col-sm-4\">$button3</div>";
  $buttons = "<div class=\"col-xs-6\">$button1</div><div class=\"col-xs-6\">$button2</div>";
//    $row = array($tid, strftime('%d-%b', $tx_created), @$myx->name, $fromYou, $toYou, $rpct, $amount, $usd, $buttons, $myx->for, $reward, $net);
  $date = strftime($download ? '%Y-%m-%d' : '%m/%d/%y', $created); // %d-%b
  if (strlen($tid) > 4) $tid = "<small>$tid</small>";

  if (!$mya->cttyRewardy) list ($reward, $run) = [$run, $run - $amt];
  foreach (ray('amt fromyou fromyou0 toyou reward') as $k) $$k = $$k ? number_format($$k, 2) : R_NONE;
  if ($mya->cttyRewardy) if (strpos($reward, '-') === FALSE) $reward = '+' . $reward;
  if ($change and !$download) $amt = w\popHelp($amt, t('Rounded up from %fromyou0', compact('fromyou0')));
  $row = array($tid, $date, @$name, $description, $amt, $fromyou, $toyou, $reward, $net, $buttons);
//  $row = array($tid, strftime($download ? '%Y-%m-%d' : '%d-%b', $myx->created), @$myx->name2, $fromYou, $toYou, $state, $buttons, $myx->for, $reward, $net);

  if (@$agents) $row[] = (!@$agent or ($mya->isMe($agent) and $mya->co) or $agent < 0) ? R_NONE 
  : ($download ? r\qid($agent) : substr(r\qid($agent), 3));
  if (@$channels and $txChannels = ray(TX_CHANNELS)) $row[] = $txChannels[$channel];
  if (@$boxes) $row[] = db\get('boxnum', 'r_boxes', 'id=:tx_box', compact('tx_box'));
  addRow($tot, array_combine($classes, $row), $tx, $change, $csvRow);
  if ($download) return u\csv($csvRow); // not STDOUT
//  finishCsvRow($csv, array_combine($classes, $row), $tx, $tot);
  
  $sort = (string) (DT_MAX + $created) . (TX_TID_MAX + $tid) . ($banking ? 5 : 0) . u\randomFloat(.0001, .9999); // make it a string so as not to lose precision
//  array_unshift($row, $sort); // sort by date (fails around the year 2260), then by tid
  
  $rows[$sort] = $row;
}

/**
 * Sort the rows descending by the first column and discard it.
 * @param array $rows: the unsorted rows
 * @return array: those rows without the sort column, sorted
 */
function sortRows($rows, $descending) {
  if (!@$rows) return []; // no records
  $count = count($rows);
  
  $func = $descending ? 'krsort' : 'ksort';
  $func($rows, SORT_STRING); // SORT_STRING is a PHP constant
  u\EXPECT(count($rows) == $count, "ksort lost rows: was $count, now " . count($rows));
  return array_values($rows);
}

function setupDownload($starting, $ending) {
  global $mya;
  $dt1 = strftime('%Y%m%d', $starting);
  $dt2 = strftime('%Y%m%d', $ending);
  u\csvStart(($mya->cAdmin ? $mya->mainQid : PROJECT_ID) . "$dt1-$dt2.csv");
}

/**
 * Return classes and headers for the appropriate set of fields.
 */
function txFields($download = FALSE, $agents = FALSE, $channels = FALSE, $boxes = FALSE) {
  global $mya, $agtViaBoxHelp;
//  $headHelp = t('Transaction number|Date transaction was completed|Payer name|Payee name|Transaction amount|Current state of transaction -- check mark means completed.|What is the purpose of the transaction (usually the item or service purchased)|How much incentive rewards were awarded for this transaction|Who did you pay or receive from|Amounts you spent|Amounts you received|Click a symbol to reverse a transaction or to edit the purpose|The net change in account balance') . @$agtViaBoxHelp;
  $headHelp = t('|Transaction number|Date transaction was completed|Payer name|Payee name|Current state of transaction -- check mark means completed.|What is the purpose of the transaction (usually the item or service purchased)|How much incentive rewards were awarded for this transaction|Who did you pay or receive from|Amounts you spent or received|fromyou|toyou|Click a symbol to reverse a transaction or to edit the purpose|The net change in account balance') . @$agtViaBoxHelp;
//  $classes = 'tid date from to amount state purpose reward';
  $classes = 'tid date from to state purpose reward';
//  $helpFields = $classes . ' name fromyou toyou buttons net' . (@$agtViaBoxHelp ? ' agent channel box' : '');
  $helpFields = $classes . ' name amount fromyou toyou buttons net' . (@$agtViaBoxHelp ? ' agent channel box' : '');
  $headHelp = array_combine(ray($helpFields), ray($headHelp));
/*  if ($report) { // community transaction history
    $headers = t('Tx#,Date,From,To,Total,Status,Purpose,Rewards');
  } else { // individual account transaction history
  */
//  $classes = 'tid date name fromyou toyou state buttons purpose reward net';
  $classes = 'tid date name purpose amount fromyou toyou reward net buttons';
//  $headers = t('Tx#,Date,Name,From you,To you,Status,Action,Purpose,Reward/Fee,Net');
  $headers = t('Tx#,Date,Name,Purpose,Amount,From You, To You,Reward,Net,Action');
//  }

  $header = array_combine(ray($classes), ray($headers)); // make splices and additions easy and clear
  if (!@$mya->cttyRewardy) list ($header['reward'], $headHelp['reward']) = [t('Balance'), t('Running Balance')];
  if (@$agents) $header['agent'] = t('Agent');
  if (@$channels) $header['channel'] = t('Via');
  if (@$boxes) $header['box'] = t('Box');

  if (@$download) { // downloading -- modify standard individual or community field list
    $downloadHeader = $header;
    u\splice($downloadHeader, 'buttons', 1); // don't download buttons
    u\splice($downloadHeader, 'amount', 1, ['bank' => t('From Bank')]); // don't download amount
  } else foreach ($header as $k => $v) $header[$k] = w\popHelp($v, $headHelp[$k]);
  
  return array(array_keys($header), array_values($header), @array_values($downloadHeader));
}

/**
 * Update the totals with values from this row and prepare an exportable CSV row.
 * @param assoc $tot: (UPDATED) the running totals
 * @param assoc $row: keyed values for the current record (changes herein to $row are just for calculating $tot and $csvRow)
 * @param assoc $tx: the transaction record
 * @param numeric $change: change being donated to the community
 * @param array $csvRow: (RETURNED) the values, formatted for CSV export
 */
function addRow(&$tot, $row, $tx, $change, &$csvRow = '') {
  extract(just('xid type toMe amount', $tx));

  if ($type == TX_BANK) {
    $frombank = $amount;
    $row['fromyou'] = $row['toyou'] = '';
  } else {
    $frombank = '';
    if (in($type, TX_REWARDS)) $row['fromyou'] = $row['toyou'] = '';
  }
  
  unset($row['buttons']); // don't download status or buttons
  u\splice($row, 'amount', 0, compact('frombank')); // insert a bank row

  foreach ($row as $k => $v) $row[$k] = $v == R_NONE ? '' : strip_tags($v);
//  if ($row['fromyou'] == $row['toyou']) $row['fromyou'] = $row['toyou'] = ''; // don't report gross on payex
  foreach (ray(TX_DOWNLOAD_MONEY_FIELDS) as $k) {
    $row[$k] = u\cleanAmount(@$row[$k]) + 0;
    $tot[$k] = (@$tot[$k] ?: 0) + $row[$k]; // formatting value is useless
  }
  $tot['roundups'] += $change;
  unset($row['amount']); // fromyou and toyou are enough
  $csvRow = array_values($row);
}

function txColoredState($disputed) {
  $states = array('&#10004;', 'disputed');
  $colors = array('ok', 'disputed');
  $state = $disputed ? 1 : 0;
  $color = $colors[$state];
  $state = $states[$state];
  return "<span class=\"state-$color\">$state</span>";
}

/**
 * Sum the amounts in each transaction category, based on description.
 * @param array $cats: (MODIFIED) counts and amount totals, indexed by category description
 * @param string $purpose: description of transaction
 * @param numeric $amount: transaction amount
 * @param bool $dups: (RETURNED) set TRUE if this category has at least two transactions in it
 */
function addCat(&$cats, $purpose, $amount, &$dups) {    
  if (strpos($purpose, '(') === 0 and ($i = strpos($purpose, ')')) and $i < strlen($purpose) - 1) {
    $purpose = trim(substr($purpose, $i + 1)); // remove reversal info, if any
  }
  if ($m = strpbrk($purpose, ':-;(/[{~#*') and $i = strpos($purpose, $m) and $i < strlen($purpose) - 1) {
    $cat = trim(substr($purpose, 0, $i));
  } else $cat = $purpose;
  
  if ($one = @$cats[$cat]) $dups = TRUE; else $one = array(0, 0);
  list ($oldCount, $oldAmount) = $one;
  $cats[$cat] = array($oldCount + 1, $oldAmount + $amount);
}

/**
 * Adjust the categories list before displaying.
 */
function finishCats(&$cats) {
  $otherInCount = $otherIn = $otherOutCount = $otherOut = 0;
  foreach ($cats as $cat => $one) if ($one[0] == 1 and !u\starts($cat, INFLATION_DESC)) { // show inflation in summary
    if ($one[1] < 0) {
      $otherOutCount++; $otherOut += $one[1];
    } else {$otherInCount++; $otherIn += $one[1];}
    
    unset($cats[$cat]);
  }
  
  ksort($cats);
  if ($otherInCount) $cats[t('other funds IN')] = array($otherInCount, $otherIn);
  if ($otherOutCount) $cats[t('other funds OUT')] = array($otherOutCount, $otherOut);
}
