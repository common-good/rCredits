<?php
/**
 * @file
 * Functions to run periodically.
 
update / check balances (often -- 5 minutes?)
  recalc balance, rewards
  unset data[frozen[today]]
other security / scam checks (often)
completion of (deliberate) pending virtual payments (daily, but not implemented yet)
automatic payments (just virtual payments for now) (daily)
redistribution (selling of unwanted rCredits) (daily)
interest (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted.
statements (daily, weekly, monthly -- at user's option)
1099s (annual)

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

//require_once 'rcredits.inc';
//require_once 'rcredits-backend.inc';

//$intensiveQueues = 'virtualPayment redistribute'; // these want to be done ASAP once they start
//$queues = u\ray('periodic checkBalance checkScam inflation statement f1099 ' . $intensiveQueues);

/**
 * Queue: Check to see if it's time for a less-often periodic activity.
 */
function periodic() {
r\notifyStaff('periodic');
  if (strftime('%M') == '00') everyHour();
  // stuff to do every time cron is called (normally every 5 minutes)
}

function everyHour() {
r\notifyStaff('hourly');
  if (strftime('%H') == R_DAILY_HOUR) everyDay();
  // stuff to do every hour
}

function everyDay() {
  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
  // stuff to do every day
  // use maintenance mode? no
  // doPendingPayments(); // not yet implemented (if ever)

  // queue up companies that pay automatically
  queueCompanies(BIT_VIRTUAL_ALL);
  queueCompanies(BIT_VIRTUAL_EMPLOYEES);
}

function everyWeek() {
}

function everyMonth() {
  if (strftime('%m') == '01') everyYear();
  // stuff to do every month
}

function everyYear() {
}

/**
 * Identify companies that have the given payment option and queue them up for processing.
 * @param int $which: BIT_VIRTUAL_ALL (queue up payments to suppliers) or BIT_VIRTUAL_EMPLOYEES (queue up payments to staff)
 */
function queueCompanies($which) {
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
	  if ($kitty > 0 and $total > 0) r\queue('virtualPayment', compact(u\ray('uid kitty total which'))); 
	}
}

function doPendingPayments() { // leave this incomplete (for now ALL virtual payments are automatic)
/*
  $sql = 'SELECT xid, payer, payee, amount FROM r_txs WHERE NOT taking and state=:TX_PENDING';
  $result = r\dbQ($sql);
  while($row = $result->fetchAssoc()) {
    extract($row);
	$dbtx = \db_transaction();
	r\setTxState(); // TX_DONE (amount should be 0, usd is negative the payment amount)
*/
}

/**
 * Pay suppliers or employees
 * @param string $which: employees (BIT_VIRTUAL_EMPLOYEES) or suppliers (BIT_VIRTUAL_ALL)
 */
function payVirtuals($kitty, $total, $row, $which) {
  extract(u\just('main', $row));
  
  $not = $which == BIT_VIRTUAL_ALL ? 'NOT' : '';
  $result = r\dbQ("SELECT other, amount FROM r_relations WHERE main=:main AND amount>0 AND $not employer_ok", compact('main'));
  while($row = $result->fetchAssoc());
    extract($row);
    $amount *= $kitty / $total;
	if ($amount >= 0.01) payVirtual($main, $other, $amount);
}

function payVirtual($uid1, $uid2, $total) {
  $usAcct1 = new usd($acct1 = r\acct($uid1));
  $usAcct2 = new usd($acct2 = r\acct($uid2));
  $excess = max(0, $acct2->balance + $total - $acct2->maxR);
  $amount = $total - $excess;
  $usBalance2 = $usAcct2->balance();
  $short = max(0, $amount - $usBalance2);
  if ($short) {
    r\notify(); // tell the supplier or employee about the missed opportunity to receive $short rCredits
	$amount -= $short;
	$excess += $short;
  }
  $dbtx = db_transaction();
  if ($amount > 0) r\transact(); // total from 1 to 2
  if (!$usAcct2->transfer($amount, $usAcct1)) {$dbtx ->rollBack(); return FALSE;}
  if ($excess) {
    $usAcct3 = new usd($acct3 = nextRBuyer($excess));
    r\transact(); // $excess from 2 to ctty
	  r\transact(); // $excess from ctty to 3
    if (!$usAcct2->transfer($excess, $usAcct3)) {$dbtx->rollBack(); return FALSE;}
  }
  unset($dbtx);
  return TRUE;
}

/**
 * Return the uid of the next account in line that wants to trade enough US Dollars for rCredits.
 * @param float $amount: the amount we're looking for
 * return: uid of the best account to handle the trade (there may be none that wants so many)
 */
function nextRBuyer($amount) {
}