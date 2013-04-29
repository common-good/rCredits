<?php
/**
 * @file
 * Functions to run periodically.
 
DAILY (every 5 mins for 5 mins, starting at 2am)
- * rollback lost USD transactions
- * check / recalc cached r, rewards, and usd
- * get community and system totals: variable_set('totals', )
- * automatic virtual payments: Each employee gets relieved of their excess rCredits immediately if they are over their "maximum" after getting paid (even if they get paid zero).
- * redistribution (selling of unwanted rCredits) looks only at non non-employees over their maximum (maximum is ignored for companies that pay virtually)
- * automatic transfers to/from bank accounts
- calculate credit line (for overdrafts, Dwolla outage (eg 503), etc.): give the payer (r)credit up to the amount they earned or bought in the past 5 days if they have been a member for a month, 30 days for 6 months
- notify and maybe fine late borrowers (might figure interest at the same time, since it requires examining each balance -- but only actually credit the interest monthly, otherwise discard it)
- statements for those who want daily statements
- weekly / monthly / quarterly / annual stuff
- other security / scam checks (often)

MONTHLY
- interest (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted. (see daily notify and fine, above)
- share to CGF
- adjust rewards pct for each account (based on growth of account's wider economic ctty)
- archive the 13th previous month (13 is a parameter -- transactions are accessible for ever, but slower)

YEARLY
- 1099s

 */
namespace rCredits\Cron;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;

define ('R_DAILY_TASKS', 'START lostUsdTxs recache totals beginVirtualPayments endVirtualPayments redistribute bank lessOften checkAccount END');

//require_once 'rcredits.inc';
//require_once 'rcredits-backend.inc';

//$intensiveQueues = 'virtualPayment redistribute'; // these want to be done ASAP once they start
//$queues = u\ray('periodic checkBalance checkScam inflation statement f1099 ' . $intensiveQueues);

/**
 * Do a queued task
 * @param assoc $item: task arguments, including 'op' -- the function to handle the task
 */
function doTask($item) {
  $op = 'rCredits\\Cron\\' . $item['op'];
  $op($item);
}

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
  r\notifyStaff('Daily cron begins', '', FALSE);
  return queueNextAfter('START');
}

function lessOften() {
  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
}

function everyWeek() {
}

/**
 * inflation (calculate and credit)
 */
function everyMonth() {
  if (strftime('%m') == '01') everyYear();
  // stuff to do every month
}

/**
 * 1099s
 */
function everyYear() {
}

//****************************************************


function lostUsdTxs() {
	$result = r\rTraders();
	while($uid = $result->fetchColumn()) queue('lostUsdTx', compact('uid'));
  queueNextAfter('lostUsdTx');
}

function lostUsdTx($item) {
  $uid = $item['uid'];
  $usAcct = new r\usd($acct = r\acct($uid));
  $usAcct->each('rCredits\\lostUsdTx1', strtotime('3 days ago'));
}

function lostUsdTx1($info) {
  extract(u\just('Id Amount SourceId DestinationId Date', $info));
  if (r\dbExists('r_usd', 'txid=:Id', compact('Id'))) return;
  
  list ($txid, $amount) = array($Id, $Amount);
  $payer = r\whoseUsd($SourceId);
  $payee = r\whoseUsd($DestinationId);
  $created = strtotime($Date);
  r\notifyStaff('lost USD tx: #@txid $@amount', compact('txid', 'amount'), FALSE);
  r\usd::rollback(array(compact(u\ray('txid amount payer payee created'))));
}

function recache() {
  $result = r\rTraders('uid, r, usd, rewards');
  while ($row = $result->fetchAssoc()) queue('recache1', $row);
}

function recache1($item) {
  $uid = array_shift($item);
  $info = be\creditInfo(compact('uid'));
  foreach ($item as $key => $value) if ($value != $info->$key) {
    r\notifyStaff("Cron: Cache <> calc $key: $value <> " . $info->$key, array(), $uid);
    r\acct($uid)->update(array($key => $info->$key));
  } 
}

/**
 * Update system-wide and community-wide totals
 */
function totals() {
  r\totals('ALL', TRUE);
  $result = r\dbQ('SELECT uid FROM users WHERE uid<0');
  while ($uid = $result->fetchColumn()) r\totals($uid, TRUE);
}

function beginVirtualPayments() {
  // queue up companies that pay automatically
  doCompanies('SUPPLIERS'); // suppliers first
  doCompanies('EMPLOYEES'); // then employees
  queue('endVirtualPayments'); // let virtualPayment tasks go on queue before queueing more stuff 
}

function endVirtualPayments() {queueNextAfter('endVirtualPayments');}

/**
 * Identify companies that have the given payment option and queue them up for virtualPayments.
 * @param int $which: SUPPLIERS (queue up payments to suppliers) or EMPLOYEES (queue up payments to staff)
 */
function doCompanies($which) {
  $these = $which == 'EMPLOYEES' ? 'r.employerOk' : 'NOT r.employerOk';
  $sql = <<< EOF
    SELECT u.uid, u.minimum, u.r, u.floor,
      SUM(r.amount) AS amounts, 
      SUM(IF($these, r.amount, 0)) AS total, 
    FROM relations r 
    LEFT JOIN users u ON u.uid=r.main
    GROUP ON r.main
    WHERE amount>0 and (u.flags&$which)>0
EOF;

	$result = r\dbQ($sql);

	while($row = $result->fetchAssoc()) {
	  extract($row);
	  $kitty = ($r - $minimum - max(0, $floor)) * $total / $amounts; // amount to pay out today
	  if ($kitty > 0 and $total > 0) queue('virtualPayments', compact(u\ray('uid kitty total slack these'))); 
	}
}

/**
 * Pay suppliers or employees
 * @param assoc $item:
 *   uid: company account id
 *   kitty: amount to pay out
 *   total: total amounts paid to this type of recipient per month
 *   UNUSED slack: how much capacity the recipients have (collectively) to be paid, before reaching their max
 *   which: employerOk (for employees) or NOT employerOk (for vendors)
 */
function virtualPayments($item) {
  extract(u\just('uid kitty total which', $item));
  //$limit = ($kitty < $slack); // if there is enough slack, don't put anyone over their max
  //  SUM(IF($which, GREATEST(o.maximum, o.rewards)-o.r, 0)) AS slack, 
  
  $main = $uid;
  $sql = "SELECT other, amount FROM r_relations WHERE main=:main AND amount>0 AND $which";
  $result = r\dbQ($sql, compact('main')); // get each recipient's proportionate share of $total
  while($row = $result->fetchAssoc());
    extract($row);
    $offer = round($amount * $kitty / $total, 2);
    if ($offer >= 0.01) queue('virtualPayment', compact(u\ray('main other offer')));
}

/**
 * Pay one employee or supplier virtually.
 * We use "acct1" for the company (the "from" account) and "acct2" for the employee or supplier (the "to" account).
 */
function virtualPayment($item) {
  extract(u\just('main other offer', $item));
  global $channel; $channel = TX_SYSTEM;
  
  $usAcct1 = new usd($acct1 = r\acct($main)); // from
  $usAcct2 = new usd($acct2 = r\acct($other)); // to
  
  // if employee is also an employer that pays people virtually, no maximum
  $max2 = $acct2->hasBit(B_VIRTUAL) ? 0 : $acct2->maximum;
  $newR = $acct2->r + $offer; // $acct2 balance after the offer is received
  $excess = round(max(0, $max2 ? ($newR - $max2) : 0, $offer - $usAcct2->balance()), 2); // rCredits to sell off
  
  r\usd::beginAtom();
  $traded = $excess ? getUsd($excess, $acct2, $usAcct1) : 0; // give company partial USD through rCredits sale to 3rd party
  if ($traded < $excess or !$txid = $usAcct2->send($offer - $traded, $usAcct1, 'virtual payment exchange')) { // other partially pays main in USD
    r\usd::rollback();
    virtualPaymentError($acct1, $acct2);
  } else {
    $info = u\assoc('amount r usd payer payee for goods usdXid', 0, $offer, -$offer, $main, $other, t('virtual payment'), TRUE, $txid);
    r\transact($info); // total rCredits from company to other
    r\usd::commit();
  }
}

function redistribute() {
  $result = r\dbQ('SELECT uid, :R_RBUY_QUANTUM*CEILING((r-maximum)/:R_RBUY_QUANTUM) AS excess FROM users WHERE r>=maximum+:R_RBUY_QUANTUM');
  while ($uid = $result->fetchColumn()) queue('sellR', compact(u\ray('uid excess')));
  queueNextAfter('redistribute');
}

function sellR($item) {
  extract(u\just('uid excess', $item));
  $usAcct = new usd($acct = r\acct($uid));
  if (!getUsd($excess, $usAcct, $uid)) r\notify($acct, 'cannot offload');
}

/**
 * Transfer funds from Dwolla to bank account or vice versa, according to account's settings
 */
function bank($item) {
}

/**
 * Check account for consistency, scams, whatever.
 */
function checkAccount($item) {
}

/**
 * Trade some rCredits for USD, from somebody. 
 * The rCredits always travel by way of the community.
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
    r\usd::beginAtom();
    if ($txid = $usAcct3->send(R_RBUY_QUANTUM, $usAcct)) {
      r\exchange(R_RBUY_QUANTUM, $acct->id, r\community(), $txid);
      r\exchange(R_RBUY_QUANTUM, r\community(), $acct3->id, $txid);
      r\usd::commit();
      $need = round($need - $part, 2);
    } else r\usd::rollback();
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
    SELECT u.uid, u.minimum-u.r AS wants FROM users u
    LEFT JOIN r_txs t ON t.payee=:u.uid
    WHERE wants>0 AND t.type=:TX_TRANSFER AND NOT t.goods
    ORDER BY (wants+:R_RBUY_QUANTUM>:amount) DESC, t.completed LIMIT 1
EOF;
  $sql2 = 'SELECT u.uid, u.minimum-u.r AS wants WHERE wants>0 ORDER BY wants DESC LIMIT 1';
  if (!$result = r\dbQ($sql1, compact('amount'))) { // find out who (preferably who who wants enough) has waited the longest
    $result = r\dbQ($sql2, compact('amount')); // nobody wanted even $10! find out who wants the most
  }
  if (!$result) {
    r\notifyStaff('no buyers', compact('amount')); // this is a really bad thing
    return FALSE;
  }

  extract($result->fetchAssoc());
  if ($wants > R_RBUY_QUANTUM) $wants = R_RBUY_QUANTUM * floor($wants / R_RBUY_QUANTUM); // truncate to nearest chunk
  $amount = min($amount, round($wants, 2));
  return r\acct($uid);
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  r\notifyStaff('Daily cron ended successfully', '', FALSE);
}

/**
 * Run the next daily function.
 */
function queueNextAfter($task) {
  $tasks = u\ray(R_DAILY_TASKS);
  queue($tasks[array_search($task) + 1]);
}

function virtualPaymentError($acct1, $acct2) {
  virtualPaymentError1($acct1, 'to', $acct2);
  virtualPaymentError1($acct2, 'from', $acct1);
  return FALSE;
}

function virtualPaymentError1($acct1, $way, $acct2) {
  $other = $acct2->fullName;
  r\notify($acct->id, 'pay virtual error', compact('way', 'other'), TRUE);
}

function queue($op, $item = array()) {
  $op = "rCredits\\Cron\\$op";
  \DrupalQueue::get('q')->createItem(compact('op') + $item);
}
