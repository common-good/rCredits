<?php
/**
 * @file
 * Functions to run periodically.
 
DAILY (every 5 mins for 5 mins, starting at 2am)
'START lostUsdTxs lostUsdTx beginVirtualPayments virtualPayments endVirtualPayments virtualPayment redistribute bank lessOften checkAccount');
- rollback lost USD transactions
- check / recalc cached available, rewards
- r\unfreeze($available, $frozen);
- automatic virtual payments: Each employee gets relieved of their excess rCredits immediately if they are over their "maximum" after getting paid (even if they get paid zero).
- redistribution (selling of unwanted rCredits) looks only at non non-employees over their maximum (maximum is ignored for companies that pay virtually)
- automatic transfers to/from bank accounts
- statements for those who want daily statements
- weekly / monthly / quarterly / annual stuff
other security / scam checks (often)

MONTHLY
- interest (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted.
- share to CGF
- archive the 13th previous month (13 is a parameter -- transactions are accessible for ever, but slower)

YEARLY
- 1099s

(just auto pay and redist happen in a chunk)
periodic queue adds all appropriate accounts to either the autopay or redistrib queue (never both and mostly neither)

- Companies with the "pay everyone virtually" option don't have to pay anyone consciously. Their rCredits are offered proportionately to all employees and suppliers. Amounts are truncated to the penny. All payment offers are accepted automatically, up to the amount in the accepter's Dwolla account. Unaccepted amounts are redistributed to others (not recursively, if the company is unable to distribute its funds). Companies must list for each employee and supplier the average amount they pay them per month ($/mo) on the relations screen.

 */
namespace rCredits\Cron;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Testing as t;

define ('R_DAILY_TASKS', 'START lostUsdTxs lostUsdTx beginVirtualPayments virtualPayments endVirtualPayments virtualPayment redistribute bank lessOften checkAccount');

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

function lostUsdTxs() {
	$result = r\dbQ('SELECT uid FROM users WHERE flags&(1<<:B_RTRADER)');
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
  extract(u\just('Id Amount SourceId DestinationId Date', $info));
  if (r\dbExists('r_usd', 'txid=:Id', compact('Id'))) return;
  
  list ($txid, $amount) = array($Id, $Amount);
  $payer = r\whoseUsd($SourceId);
  $payee = r\whoseUsd($DestinationId);
  $created = strtotime($Date);
  r\notifyStaff('lost USD tx: #@txid $@amount', compact('txid', 'amount'));
  r\usd::rollback(array(compact(u\ray('txid amount payer payee created'))));
}

function beginVirtualPayments() {
  // queue up companies that pay automatically
  doCompanies(B_VIRTUAL_ALL); // suppliers first
  doCompanies(B_VIRTUAL_EMPLOYEES); // then employees
  r\queue('virtualPayments', FALSE); // mark end of virtualPayments queue, so we can do what must follow
}

/**
 * Identify companies that have the given payment option and queue them up for virtualPayments.
 * @param int $which: B_VIRTUAL_ALL (queue up payments to suppliers) or B_VIRTUAL_EMPLOYEES (queue up payments to staff)
 */
function doCompanies($which) {
  $these = $which == B_VIRTUAL_EMPLOYEES ? 'r.employer_ok' : 'NOT r.employer_ok';
  $sql = <<< EOF
    SELECT u.uid, u.minimum, u.available, 
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
	  $kitty = ($available - $minimum) * $total / $amounts; // amount to pay out today
	  if ($kitty > 0 and $total > 0) r\queue('virtualPayments', compact(u\ray('uid kitty total slack these'))); 
	}
}

/**
 * Pay suppliers or employees
 * @param assoc $item:
 *   uid: company account id
 *   kitty: amount to pay out
 *   total: total amounts paid to this type of recipient per month
 *   UNUSED slack: how much capacity the recipients have (collectively) to be paid, before reaching their max
 *   these: employer_ok (for employees) or NOT employer_ok (for vendors)
 */
function virtualPayments($item) {
  if (!$item) return doNextInQueueAfter(__FUNCTION__);
  extract(u\just('uid kitty total which', $item));
  //$limit = ($kitty < $slack); // if there is enough slack, don't put anyone over their max
  //  SUM(IF($these, GREATEST(o.maximum, o.rewards)-o.available, 0)) AS slack, 
  
  $main = $uid;
  $sql = "SELECT other, amount FROM r_relations WHERE main=:main AND amount>0 AND $these";
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
  
  // if employee is also an employer that pays people virtually, no maximum
  $max2 = ($acct2->hasBit(B_VIRTUAL_ALL) or $acct2->hasBit(B_VIRTUAL_EMPLOYEES)) ? 0 : $acct2->maximum;
  $newAvailable = $acct2->available + $offer; // $acct2 balance after the offer is received
  $excess = round(max(0, $max2 ? ($newAvailable - $max2) : 0, $offer - $usAcct2->balance()), 2); // rCredits to sell off
  
  r\usd::beginAtom();
  $traded = $excess ? getUsd($excess, $acct2, $usAcct1) : 0; // give company partial USD through rCredits sale to 3rd party
  if ($traded < $excess or !$txid = $usAcct2->send($offer - $traded, $usAcct1, 'virtual payment exchange')) { // other partially pays main in USD
    r\usd::rollback();
    virtualPaymentError($acct1, $acct2);
  } else {
    $info = u\assoc('amount r usd payer payee for goods usd_xid', 0, $offer, -$offer, $main, $other, t('virtual payment'), TRUE, $txid);
    r\transact($info); // total rCredits from company to other
    r\usd::commit();
  }
}

function redistribute() {
  $result = r\dbQ('SELECT uid, :R_RBUY_QUANTUM*CEILING((available-maximum)/:R_RBUY_QUANTUM) AS excess FROM users WHERE available>=maximum+:R_RBUY_QUANTUM');
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
    SELECT u.uid, u.minimum-u.balance AS wants FROM users u
    LEFT JOIN r_txs t ON t.payee=:u.uid
    WHERE wants>0 AND t.type=:TX_TRANSFER AND NOT t.goods
    ORDER BY (wants+:R_RBUY_QUANTUM>:amount) DESC, t.completed LIMIT 1
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
  if ($wants > R_RBUY_QUANTUM) $wants = R_RBUY_QUANTUM * floor($wants / R_RBUY_QUANTUM); // truncate to nearest chunk
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
