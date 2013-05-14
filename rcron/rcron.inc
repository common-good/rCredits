<?php
/**
 * @file
 * Functions to run periodically.
 
 * = written
 ** = tested
 
DAILY (every 5 mins for 5 mins, starting at 2am)
- * usdTxs() rollback lost USD transactions, mark completed deposits/withdrawals
- ** recache() check / recalc cached r, rewards, and usd
- [later] checkAccts() thorough security / scam checks
- [later] * totals() get community and system totals: variable_set('totals', )
- ** paySuppliers(): automatic virtual payments to suppliers
- ** payEmployees(): Each employee gets relieved of their excess rCredits immediately if they are over their "maximum" after getting paid.
- ** gifts(): contributions to CGF (not shares)
- * redistribute(): (selling of unwanted rCredits) looks at accounts over their maximum (maximum is ignored for companies that pay virtually)
- ** bank(): automatic transfers to/from bank accounts
- (later) calculate credit line (for overdrafts, Dwolla outage (eg 503), etc.): give the payer (r)credit up to the amount they earned or bought in the past 5 days if they have been a member for a month, 30 days for 6 months
- [later] notify and maybe fine late borrowers (might figure interest at the same time, since it requires examining each balance -- but only actually credit the interest monthly, otherwise discard it)
- [later] things to do 30/60/90 (etc) days after opening account:
  . 1mo: notice - suggest sending invitations
      $email = $mya->mail;
      $invited = ($nonperson or r\dbLookup('COUNT(*)', 'r_invites', 'inviter=:myid AND email<>:email', compact('myid', 'email')) >= 1);
          $step4 = t('<a>Invite someone</a> to sign up for rCredits! No one can join without an invitation. You can help by sending one. You might particularly want to invite companies and individuals you buy from or sell to (for example your employer or employees).');

  . 1mo: suggest another contribution if first was small
  . 6mo: allow cashing out rewards if system stable
- [later] statements for those who want daily statements
- * notices(): send notices (daily digest)
- (later) weekly / monthly / quarterly / annual stuff

MONTHLY
- inflation adjustment (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted. (see daily notify and fine, above)
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

include_once __DIR__ . '/../usd.class';

define('R_DAILY_TASKS', 'START usdTxs recache checkAccts totals paySuppliers payEmployees gifts redistribute bank notices lessOften END');
//define ('R_DAILY_TASKS', 'START redistribute END');
define('R_EXCESS', 'GREATEST(0, IF(maximum=0, 0, r+usd-GREATEST(floor, maximum)))'); // sql for excess in account
define('R_SHORTFALL', 'GREATEST(0, minimum-(r+usd))'); // sql for shortfall in account

/**
 * Do a queued task
 * @param assoc $item: task arguments, including 'op' -- the function to handle the task
 */
function doTask($item) {
  global $channel; $channel = TX_CRON;
  $op = 'rCredits\\Cron\\' . array_shift($item);
  $op($item);
}

/**
 * Queue: Check to see if it's time for a less-often periodic activity.
 */
function periodic() {
  if (!isDEV) return; // DEBUG (don't activate this until we're sure it will work smoothly)
  $lastDaily = \variable_get('daily', 0);
//  $lastDaily = 0; // DEBUG
  r\dbQ("DELETE FROM queue WHERE name='q'");
  if (strftime('%H') >= R_DAILY_HOUR and strftime('%d') != strftime('%d', $lastDaily)) everyDay();
  if (strftime('%M') == '00') everyHour();
  // stuff to do every time cron is called (normally every 5 minutes)
}

function everyHour() {
  // stuff to do every hour
}

function everyDay() {
  queueNextAfter('START');
  \variable_set('daily', REQUEST_TIME);
  // miscellaneous quick stuff to do every day
  $time = time() - 7 * DAY_SECS;
  r\dbQ('DELETE FROM r_log WHERE time<:time', compact('time')); // zap log except for the past few days
}

function lessOften() {
  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
  queueNextAfter('lessOften');
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


function usdTxs() {
  queueEach('usdTx', 'SELECT uid FROM users WHERE :IS_OK');
  queue('usdTxsEND');
}
function usdTxsEND() {queueNextAfter('usdTxs');}

function usdTx($item) {
  $uid = $item['uid'];
  $usAcct = new r\usd($acct = r\acct($uid));
  $usAcct->each('rCredits\\Cron\\usdTxQ', strtotime('4 days ago'), array('money_sent','deposit','withdrawal'));
}

function usdTxQ($info) {queue($info['Type'] == 'money_sent' ? 'lostUsdTx' : 'bankFollowup', $info);}

function lostUsdTx($info) {
  extract(u\just('Id Notes Amount SourceId DestinationId Date', $info));
  if (!u\abbreviates(USD_NOTE_PREFIX, $Notes)) return; // not one of ours
  if (r\dbExists('r_usd', 'txid=:Id', compact('Id'))) return;
  
  list ($txid, $amount) = array($Id, $Amount);
  $payer = r\whoseUsd($SourceId);
  $payee = r\whoseUsd($DestinationId);
  $created = strtotime($Date);
  r\tellStaff('lost USD tx: #@txid $@amount', compact('txid', 'amount'), FALSE);
  r\usd::rollback(array(compact(u\ray('txid amount payer payee created'))));
}

function bankFollowup($info) {
  extract(u\just('Id Amount ClearingDate Status', $info));
  list ($txid, $amount, $completed) = array($Id, $Amount, strtotime($ClearingDate));
  if ($Status == 'processed') r\dbUpdate('r_usd', compact(u\ray('txid amount completed')), 'txid');
}

/**
 * Recalculate and update cached financial balances for each account.
 */
function recache() {
  $fields = 'uid, r, usd, rewards, maximum, minimum';
  queueEach('recache1', "SELECT $fields FROM users WHERE uid>0 AND :IS_MEMBER");
  queueNextAfter('recache');
}

function recache1($item) {
  extract($item);

  $info = be\creditInfo(compact('uid'));
  foreach (u\just('r rewards', $item) as $key => $value) if ($value != $info->$key) {
    r\tellStaff("Cron: Cache <> calc $key: $value <> " . $info->$key, array(), $uid);
    $$key = $info->$key;
  }
  $usAcct = new r\usd($acct = r\acct($uid));
  $usdActual = $usAcct->bal($err);
  if (!$err) $usd = $usdActual;
  
  if ($maximum and $maximum < max($rewards, $minimum)) {
    r\tellStaff('corrupt max', $item);
    $maximum = max($rewards, $minimum);
  }

  $acct->update(compact(array_keys($item)));
}

/**
 * Check accounts for consistency, scams, whatever.
 */
function checkAccts() {
  queueEach('checkAcct', "SELECT uid, maximum, minimum, rewards FROM users WHERE uid>0 AND :IS_MEMBER");
  queueNextAfter('checkAccts');
}

function checkAcct($item) {
  extract($item); // (prepared for additional tasks)
  $a = r\acct($uid);
}

/**
 * Update system-wide and community-wide totals
 */
function totals() {
  r\totals('ALL', TRUE);
  $result = r\dbQ('SELECT uid FROM users WHERE uid<0');
  while ($uid = $result->fetchColumn()) r\totals($uid, TRUE);
  queueNextAfter('totals');
}

/**
 * Queue up companies that pay automatically, to pay suppliers.
 */
function paySuppliers() {
  doCompanies('SUPPLIERS');
  queue('paySuppliersEND'); // let specific tasks go on queue before queueing more stuff 
}
function paySuppliersEND() {queueNextAfter('paySuppliers');}

/**
 * Queue up companies that pay automatically, to pay employees.
 */
function payEmployees() {
  doCompanies('EMPLOYEES');
  queue('payEmployeesEND'); // let specific tasks go on queue before queueing more stuff 
}
function payEmployeesEND() {queueNextAfter('payEmployees');}

/**
 * Identify companies that have the given payment option and queue them up for virtualPayments.
 * @param int $which: SUPPLIERS (queue up payments to suppliers) or EMPLOYEES (queue up payments to staff)
 */
function doCompanies($which) {
  $rightType = $which == 'EMPLOYEES' ? 'employerOk' : 'NOT employerOk';
  $sql = <<< EOF
    SELECT u.uid, u.minimum, u.r, u.floor,
      SUM(r.amount) AS amounts, 
      SUM(IF($rightType, r.amount, 0)) AS tothese 
    FROM r_relations r 
    LEFT JOIN users u ON u.uid=r.main
    WHERE amount>0 and :IS_VIRTUAL and :IS_OK
    GROUP BY r.main
EOF;
	$result = r\dbQ($sql);

	while($row = $result->fetchAssoc()) {
	  extract($row);
	  $kitty = ($r - $minimum - max(0, $floor)); // amount to pay out today for this group
    if ($which != 'EMPLOYEES') $kitty *= $tothese / $amounts; // limit suppliers to their share
	  if ($kitty > 0 and $tothese > 0) queue('virtualPayments1', compact(u\ray('uid kitty tothese slack rightType'))); 
	}
}

/**
 * Pay suppliers or employees
 * @param assoc $item:
 *   uid: company account id
 *   kitty: amount to pay out
 *   tothese: total amounts paid to this type of recipient per month
 *   UNUSED slack: how much capacity the recipients have (collectively) to be paid, before reaching their max
 *   rightType: employerOk (for employees) or NOT employerOk (for vendors)
 */
function virtualPayments1($item) {
  extract(u\just('uid kitty tothese rightType', $item));
  //$limit = ($kitty < $slack); // if there is enough slack, don't put anyone over their max
  //  SUM(IF($which, GREATEST(o.maximum, o.rewards)-o.r, 0)) AS slack, 
  
  $offers = $total = 0;
  $main = $uid;
  $sql = "SELECT other, amount FROM r_relations WHERE main=:main AND amount>0 AND $rightType";
  $result = r\dbQ($sql, compact('main')); // get each recipient's proportionate share of $tothese
  while($row = $result->fetchAssoc()) {
    extract($row);
    $offer = round($amount * $kitty / $tothese, 2);
    if ($offer > 0) {
      queue('virtualPayment', compact(u\ray('main other offer')));
      $offers += 1;
      $total += $offer;
    }
  }
  $whom = $rightType == 'employerOk' ? 'employees' : 'suppliers';
  $total = u\formatAmount($total, 's$r');
  if ($offers) r\notify($uid, 'virtual payments offered', compact(u\ray('offers total whom')));
}

/**
 * Pay one employee or supplier virtually.
 * We use "acct1" for the company (the "from" account) and "acct2" for the employee or supplier (the "to" account).
 */
function virtualPayment($item) {
  extract(u\just('main other offer', $item));
  
  $usAcct1 = new r\usd($acct1 = r\acct($main)); // from
  $usAcct2 = new r\usd($acct2 = r\acct($other)); // to
  
  // if recipient is also an employer that pays people virtually, no maximum
  $max2 = $acct2->hasBit(B_VIRTUAL) ? 0 : $acct2->maximum;
  $newR = $acct2->r + $offer; // what $acct2 r balance will be after the offer is received
  $excess = round(max(0, $max2 ? ($newR - $max2) : 0, $offer - $acct2->usd), 2); // r excess or USD shortfall (= r to exchange for USD in a 3-way trade)
  r\usd::beginAtom();
  $traded = $excess ? getUsd($excess, $acct2, $usAcct1) : 0; // give company partial USD through rCredits sale to 3rd party
  //debug(compact(u\ray('main other offer max2 newR excess traded')));
  if ($traded >= $excess and ($offer == $traded or $txid = $usAcct2->send($offer - $traded, $usAcct1, 'virtual payment exchange'))) { // other partially pays main in USD if not totally left to 3rd parties
    $info = u\assoc('amount r payer payee for goods usdXid', 0, $offer, $main, $other, t('virtual payment'), TRUE, @$txid);
    r\transact($info); // total rCredits from company to other
    $amount = u\formatAmount($offer, 's$r');
    $fullName = $acct1->fullName;
    $bonus = u\formatAmount(r\bonus($other, $offer), '$');
    r\notify($other, 'virtual payment received', compact(u\ray('amount fullName bonus')));
    r\usd::commit();
  } else {
    r\usd::rollback();
    virtualPaymentError($acct1, $acct2);
  }
}

/**
 * Handle one-time and recurring gifts that it's time to handle.
 */
function gifts() {
  queueEach('gift1', "SELECT * FROM r_gifts g INNER JOIN users u ON u.uid=g.uid WHERE NOT completed and giftDate<=:REQUEST_TIME AND :IS_OK");
  queueNextAfter('gifts');
}

/**
 * Handle a gift, if possible.
 * Repeating gifts happen on the same day each month (but earlier for gifts on the 29th-31st)
 * Each instance of a repeating gift creates a repeating post-dated gift record.
 * Gifts happen as soon as the money is there, on or after the gift date.
 */
function gift1($item) {r\acceptGift($item);}

function redistribute() {
  $sql = 'SELECT uid, :R_CHUNK*FLOOR((:R_EXCESS-usd)/:R_CHUNK) AS excess FROM users WHERE :R_EXCESS-usd>=:R_CHUNK AND NOT :IS_VIRTUAL AND uid>0 AND :IS_OK';
  queueEach('sellR', $sql);
  queueNextAfter('redistribute');
}

function sellR($item) {
  extract(u\just('uid excess', $item));
  $usAcct = new r\usd($acct = r\acct($uid));
  if (!$got = getUsd($excess, $acct, $usAcct) or $got < $excess) r\notify($uid, 'cannot offload');
}

/**
 * Transfer funds from Dwolla to bank account or vice versa, according to account's settings
 */
function bank() {
  $sql = <<<EOF
  SELECT uid, LEAST(usd, :R_EXCESS) AS excess, :R_SHORTFALL AS shortfall
  FROM users 
  WHERE ((LEAST(usd, :R_EXCESS)>=:R_BANK_MIN AND :IS_TO_BANK AND NOT :IS_VIRTUAL) OR :R_SHORTFALL>0)
    AND uid>0 AND :IS_OK
EOF;
  // AND (flags&(1<<:B_BANKABLE))
  queueEach('bank1', $sql);
  queueNextAfter('bank');
}

function bank1($item) {
  extract(u\just('uid excess shortfall', $item));
  if ($excess > 0 and $shortfall > 0) return r\tellStaff('min > max', $item, $uid); // should never happen
  $acct = r\acct($uid);
  $waiting = $acct->waitingToBank(); // amount already requested (if any)
  $amount = ($excess ?: -$shortfall) - $waiting;
  if ($excess > 0) {
    if ($amount < R_BANK_MIN or $waiting < 0) return; // too little, considering what's waiting, or flip-flop
  } else { // shortfall
    if ($amount >= 0 or $waiting > 0) return; // already plenty or flip-flop
    $amount = min($amount, -R_BANK_MIN);
  }
  $action = $excess > 0 ? 'deposit to' : 'draw from';

  $usAcct = new r\usd($acct);
  if ($usAcct->bank($amount, $error)) {
    $status = @$excess > 0 ? 'over the maximum you set' : 'under the minimum you set';
    $amount = u\formatAmount(abs(@$amount));
    r\notify($uid, 'banked', compact(u\ray('action status amount')));
  } elseif (time() - $acct->created > 3 * DAY_SECS) r\notify($uid, 'cannot bank', compact('action', 'error'));
}

/**
 * Send daily message digests. (no other choice at this time)
 */
function notices() {
  queueEach('notice', 'SELECT DISTINCT uid FROM r_notices WHERE NOT sent');
  queueNextAfter('notices');
}

function notice($item) {
  $uid = $item['uid'];
  $result = r\dbQ('SELECT message FROM r_notices WHERE uid=:uid AND NOT sent', $item);
  $messages = '';
  while ($row = $result->fetch()) $messages .= "<li>$row->message</li>\n";
  if ($messages) {
    $yesterday = u\formatDate(REQUEST_TIME - 1 * DAY_SECS);
    r\notify($uid, 'notices', compact('messages', 'yesterday'), TRUE);
    r\dbQ('UPDATE r_notices SET sent=:REQUEST_TIME WHERE uid=:uid AND NOT sent', $item);
  }
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
  $threeway = ($acct->id != $usAcct->acct->id);
  r\usd::beginAtom();
  while ($need > 0) {
    $part = $need;
    if (!$acct3 = nextRBuyer($part, $acct->id)) break; // asks for what we need, returns the part we got in $part
    u\EXPECT($part > 0, 'got nothing');
    $usAcct3 = new r\usd($acct3); // "third party" Dwolla account
    r\usd::beginAtom();
    if ($txid = $usAcct3->send($part, $usAcct)) { // $part NOT R_CHUNK
      r\exchange($part, $acct->community, $acct3->id, $txid); // use first party's community as go-between
      r\usd::commit();
      $need = round($need - $part, 2);
    } else r\usd::rollback();
  }
  if ($got = round($request - $need, 2)) r\exchange($got, $acct->id, $acct->community, $txid);
  r\usd::commit($got); // accept partial success
  return $got;
}

/**
 * Return the uid of the next account in line willing and able to trade enough US Dollars for rCredits.
 * @param float $amount: the amount we're looking for. Returned with the amount actually available from the returned account. The amount sought depends on the $amount passed as follows:
 *   $amount < R_CHUNK*1.5 -- seek $amount
 *   otherwise -- seek R_CHUNK (leaving R_CHUNK/2 or more for other buyers
 * return: the best account to handle the trade (there may be none that wants so many)
 *    if no account wants to buy any, return FALSE (this would be really bad and should never ever happen)
 */
function nextRBuyer(&$amount, $except) {
  if ($amount > R_CHUNK * 1.5) $amount = R_CHUNK;
  $offer = 'LEAST(u.minimum-u.r, u.usd)'; // how much they want or how much they can buy, whichever is less
  $where = "($offer>0 AND u.uid>0 AND u.uid<>:except AND :IS_OK)";
  $sql1 = <<<EOF
    SELECT u.uid, $offer AS got FROM users u
    LEFT JOIN r_txs t ON t.payee=u.uid
    WHERE $where AND ((t.type=:TX_TRANSFER AND NOT t.goods) OR t.xid IS NULL)
    ORDER BY ($offer>=:amount) DESC, t.completed LIMIT 1
EOF;
  if (!$row = r\dbQ($sql1, compact('amount', 'except'))->fetchAssoc()) { // find out who (preferably who who wants enough) has waited the longest
    r\tellStaff('no buyers', compact('amount')); // this is a really bad thing (nobody wants any at all)
    return FALSE;
  }

  extract($row); // uid and got
  $amount = min($amount, round($got, 2)); // take what we could get
  return r\acct($uid);
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  $elapsed = time() - REQUEST_TIME;
  echo "Daily cron done after $elapsed seconds"; // keep
  r\tellStaff('Daily cron ended successfully', '', FALSE);
}

/**
 * Run the next daily function.
 */
function queueNextAfter($task) {
  global $cronOp;
  $tasks = u\ray(R_DAILY_TASKS);

  if (t\EST() and @$cronOp) {
    queue($task == $cronOp ? 'END' : $cronOp);
  } else queue($tasks[array_search($task, $tasks) + 1]);
}

function virtualPaymentError($acct1, $acct2) {
  virtualPaymentError1($acct1, 'to', $acct2);
  virtualPaymentError1($acct2, 'from', $acct1);
  return FALSE;
}

function virtualPaymentError1($acct1, $way, $acct2) {
  $other = $acct2->fullName;
  r\notify($acct1->id, 'pay virtual error', compact('way', 'other'));
}

function queue($op, $item = array()) {
  \DrupalQueue::get('q')->createItem(compact('op') + $item);
}

function queueEach($op, $sql, $subs = array()) {
  $result = r\dbQ($sql, $subs);
  while ($row = $result->fetchAssoc()) queue($op, $row);
}
