<?php
/**
 * @file
 * Functions to run periodically.
 
automatic payments (just virtual payments for now) (daily)
redistribution (selling of unwanted rCredits) (daily)
transfers to/from bank (daily)

interest (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted.
statements (daily, weekly, monthly -- at user's option)
1099s (annual)

update / check balances (often -- 5 minutes?) -- restart daily (start with random account?)
  recalc balance, rewards
  unset data[frozen[today]]
other security / scam checks (often)

(just auto pay and redist happen in a chunk)
periodic queue adds all appropriate accounts to either the autopay or redistrib queue (never both and mostly neither)
autopay starts at 2am, queue runs every 5 mins with time limit of 5 minutes. Each employee gets relieved of their excess rCredits immediately if they are over their "maximum" after getting paid (even if they get paid zero).
redistrib also starts at 2am (every 5 mins for 5 mins) and looks only at non non-employees over their maximum (maximum is ignored for companies that pay virtually)

- ** Default for companies is "pay everyone virtually", for personal is "pay everyone directly". Companies with the "pay everyone virtually" option don't have to pay anyone consciously. A "pay ALL" is done for them at midnight daily. Their rCredits are offered equally to all employees and to their suppliers in proportion to how many employees and vendors that supplier has (where each supplier counts as half the average number of employees that any supplier has, divided by the average number of supplieds that any supplier has), divided by its supplied count, divided by 2. All amounts are truncated to the penny. All payment offers are accepted automatically, up to the amount in the accepter's Dwolla account. Unaccepted amounts are redistributed to others (not recursively, if the company is unable to distribute its funds). A company's payments are tentative until we know what all of them will be. Companies must list for each employee and supplier the average amount they pay them per month ($/mo) on the relations screen.

 */
namespace rCredits\Cron;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Testing as t;

define ('R_DAILY_TASKS', ' lostUsdTxs lostUsdTx beginVirtualPayments virtualPayments endVirtualPayments virtualPayment redistribute bank lessOften checkAccount');

//require_once 'rcredits.inc';
//require_once 'rcredits-backend.inc';

//$intensiveQueues = 'virtualPayment redistribute'; // these want to be done ASAP once they start
//$queues = u\ray('periodic checkBalance checkScam inflation statement f1099 ' . $intensiveQueues);

/**
 * Queue: Check to see if it's time for a less-often periodic activity.
 */
function periodic() {
  if (strftime('%M') == '00') everyHour();
  // stuff to do every time cron is called (normally every 5 minutes)
}

function everyHour() {
  if (strftime('%H') == R_DAILY_HOUR) everyDay();
  // stuff to do every hour
}

function everyDay() {
  r\notifyStaff('Daily cron begins');
  // stuff to do every day
  // use maintenance mode? no
  // doPendingPayments(); // not yet implemented (if ever)
  return doNextInQueueAfter('START');
}

function lessOften() {
  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
}

function everyWeek() {
}

function everyMonth() {
  if (strftime('%m') == '01') everyYear();
  // stuff to do every month
}

function everyYear() {
}

function lostUsdTxs() {
	$result = r\dbQ('SELECT uid FROM users WHERE flags&BIT_RTRADER');
	while($uid = $result->fetchColumn()) r\queue('lostUsdTx', compact('uid'));
  r\queue('lostUsdTx', FALSE);
}

function lostUsdTx($item) {
  if (!$item) return doNextInQueueAfter(__FUNCTION__);
  $uid = $item['uid'];
  $usAcct = new r\usd($acct = r\acct($uid));
  $usAcct->each('rCredits\\lostUsdTx1', strtotime('3 days ago'));
}

function lostUsdTx1($info) {
  extract(u\just('txid amount dateOfTransaction', $info));
  if (r\dbLookup(1, 'r_usd', 'txid=:txid', compact('txid'))) return;
  $payer = $payee = 0; // irrelevant
  $created = $dateOfTransaction; // need proper param here
  r\usd::rollback(array(compact(u\ray('txid amount payer payee created'))));
}

function beginVirtualPayments() {
  // queue up companies that pay automatically
  doCompanies(BIT_VIRTUAL_ALL); // suppliers first
  doCompanies(BIT_VIRTUAL_EMPLOYEES); // then employees
  r\queue('virtualPayments', FALSE); // mark end of virtualPayments queue, so we can do what must follow
}

/**
 * Identify companies that have the given payment option and queue them up for virtualPayments.
 * @param int $which: BIT_VIRTUAL_ALL (queue up payments to suppliers) or BIT_VIRTUAL_EMPLOYEES (queue up payments to staff)
 */
function doCompanies($which) {
  $sql = <<< EOF
    SELECT u.uid, SUM(IF(r.employer_ok, r.amount, 0)) AS salaries, SUM(r.amount) AS amounts, u.data
    FROM relations r LEFT JOIN users u ON u.uid=r.main 
    GROUP ON r.main
    WHERE amount>0 and (u.flags&$which)>0
EOF;

	$result = r\dbQ($sql);

	while($row = $result->fetchAssoc()) {
	  extract(u\just('uid data amounts salaries', $row));
	  extract(u\just('minimum', $data));
    $available = r\available(acct($uid), TRUE);
	  $total = $which == BIT_VIRTUAL_ALL ? ($amounts - $salaries) : $salaries; // total amounts paid per month
	  $kitty = ($available - $minimum) * $total / $amounts; // amount to pay out today
	  if ($kitty > 0 and $total > 0) r\queue('virtualPayments', compact(u\ray('uid kitty total which'))); 
	}
}

/**
 * Pay suppliers or employees
 * @param assoc $item:
 *   uid: company account id
 *   kitty: amount to pay out
 *   total: total amounts paid to this type of recipient per month
 *   which: employees (BIT_VIRTUAL_EMPLOYEES) or suppliers (BIT_VIRTUAL_ALL)
 */
function virtualPayments($item) {
  if (!$item) return doNextInQueueAfter(__FUNCTION__);
  extract(u\just('uid kitty total which', $item));
  
  $main = $uid;
  $not = $which == BIT_VIRTUAL_ALL ? 'NOT' : '';
  $sql = "SELECT other, amount FROM r_relations WHERE main=:main AND amount>0 AND $not employer_ok";
  $result = r\dbQ($sql, compact('main')); // get each recipient's proportionate share of $total
  while($row = $result->fetchAssoc());
    extract($row);
    $offer = round($amount * $kitty / $total, 2);
    if ($offer >= 0.01) r\queue('virtualPayment', compact(u\ray('main other offer')));
}

function endVirtualPayments() {
  r\queue('virtualPayment', FALSE); // mark end of virtualPayment queue
}

/**
 * Pay one employee or supplier virtually.
 * We use "acct1" for the company (the "from" account) and "acct2" for the employee or supplier (the "to" account).
 */
function virtualPayment($item) {
  if (!$item) return doNextInQueueAfter(__FUNCTION__);
  extract(u\just('main other offer', $item));
  global $channel; $channel = TX_SYSTEM;
  
  $usAcct1 = new usd($acct1 = r\acct($main)); // from
  $usAcct2 = new usd($acct2 = r\acct($other)); // to
  
  $max2 = ($acct2->hasBit(BIT_VIRTUAL_ALL) or $acct2->hasBit(BIT_VIRTUAL_EMPLOYEES)) ? 0 : $acct2->maximum;
  $newBalance = $acct2->balance + $offer; // $acct2 balance after the offer is received
  $excess = round(max(0, $max2 ? ($newBalance - $max2) : 0, $offer - $usAcct2->balance), 2); // rCredits to sell off
  
  /*!*/ r\usd::beginAtom();
  $traded = $excess ? getUsd($excess, $acct2, $usAcct1) : 0; // give company partial USD through rCredits sale to 3rd party
  r\exchange($offer, $main, $other, 'rCredits/USD exchange'); // total rCredits from company to other
  if ($traded < $excess or !$usAcct2->transfer($offer - $traded, $usAcct1)) { // other partially pays main in USD
    /*!*/ r\usd::rollback();
    virtualPaymentError($acct1, $acct2);
    return;
  }
  /*!*/ r\usd::commit();
}

function redistribute() {
  $result = r\dbQ('SELECT uid, 10*CEILING((balance-maximum)/10) AS excess FROM users WHERE balance>=maximum+10');
  while ($uid = $result->fetchColumn()) r\queue('sellR', compact(u\ray('uid excess')));
  r\queue('sellR', FALSE);
}

function sellR($item) {
  if (!$item) return doNextInQueueAfter(__FUNCTION__); 
  extract(u\just('uid excess', $item));
  $usAcct = new usd($acct = r\acct($uid));
  if (!getUsd($excess, $usAcct, $uid)) r\notify($acct, 'cannot offload');
}

/**
 * Trade some rCredits for USD, from somebody. 
 * The rCredits always travel by way of the community.
 * @param object $dbtx: active db transaction object
 * @param float $request: how much to try to get
 * @param acct $acct: rCredits account that is trading rCredits to third parties
 * @param usd $usAcct: USD account that is receiving USD from the third parties (might be different owner from $acct)
 * @return how much USD we actually delivered
 */
function getUsd($request, $acct, $usAcct) {
  $need = $request;
  while ($need > 0) {
    $part = $need;
    if (!$acct3 = nextRBuyer($part)) break; // asks for what we need, returns the part we got in $part
    $usAcct3 = new usd($acct3); // "third party" Dwolla account
    $dbtx = \db_transaction(); // we cannot easily rollback a USD transfer, so rCredits transactions must come first
    r\exchange($amount, $acct->id, r\community(), 'rCredits/USD exchange');
    r\exchange($amount, r\community(), $acct3->id, 'rCredits/USD exchange');
    if ($usAcct3->transfer($amount, $usAcct)) {
      $need = round($need - $part, 2);
    } else $dbtx->rollback();
  }
  return round($request - $need, 2);
}

/**
 * Return the uid of the next account in line that wants to trade enough US Dollars for rCredits.
 * @param float $amount: the amount we're looking for. Returned with the amount actually available from the returned account.
 * return: the best account to handle the trade (there may be none that wants so many)
 *    if no account wants to buy any, return FALSE (this would be really bad and should never ever happen)
 */
function nextRBuyer(&$amount) {
  $sql1 = <<<EOF
    SELECT u.uid, u.minimum-u.balance AS wants FROM users u
    LEFT JOIN r_txs t ON t.payee=:u.uid
    WHERE wants>0 AND t.type=:TX_TRANSFER AND NOT t.goods
    ORDER BY (wants+10>:amount) DESC, t.completed LIMIT 1
EOF;
  $sql2 = 'SELECT u.uid, u.minimum-u.balance AS wants WHERE wants>0 ORDER BY wants DESC LIMIT 1';
  if (!$result = r\dbQ($sql1, compact('amount'))) { // find out who (preferably who who wants enough) has waited the longest
    $result = r\dbQ($sql2, compact('amount')); // nobody wanted even $10! find out who wants the most
  }
  if (!$result) {
    r\notifyStaff('no buyers', compact('amount')); // this is a really bad thing
    return FALSE;
  }

  extract($result->fetchAssoc());
  if ($wants > 10) $wants = 10 * floor($wants / 10); // truncate to nearest 10
  $amount = min($amount, round($wants, 2));
  return r\acct($uid);
}

/**
 * Run the next daily function.
 */
function doNextInQueueAfter($task) {
  $tasks = u\ray(R_DAILY_TASKS);
  if ($function = $tasks[array_search($task) + 1]) $function();
}

function lastInQueue($queue) {return (r\dbLookup('COUNT(*)', 'queue', 'name=:queue', compact('queue')) == 1);}

function virtualPaymentError($acct1, $acct2) {
  virtualPaymentError1($acct1, 'to', $acct2);
  virtualPaymentError1($acct2, 'from', $acct1);
  return FALSE;
}

function virtualPaymentError1($acct1, $way, $acct2) {
  $other = $acct2->full_name;
  r\notify($acct->id, 'pay virtual error', compact('way', 'other'));
}
