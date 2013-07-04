<?php
/**
 * @file
 * Functions to run periodically.
 
 * = written
 ** = tested
 
DAILY (every 5 mins for 5 mins, starting at 2am)
- ** usdTxsHere() complete USD transactions that didn't go through because of Dwolla outage
- * usdTxsThere() rollback lost USD transactions, mark completed deposits/withdrawals
- ** recache() check / recalc cached r, rewards, and usd
- [later] checkAccts() thorough security / scam checks
- ** totals() get community and system totals: variable_set('totals', )
- ** paySuppliers(): automatic virtual payments to suppliers
- ** payEmployees(): Each employee (and supplier) gets relieved of their excess rCredits immediately if they don't have enough USD to trade the employer for them.
- ** gifts(): contributions to CGF (not shares)
- NO! * redistribute(): (selling of unwanted rCredits) looks at accounts over their maximum (maximum is ignored for companies that pay virtually)
- ** bank(): automatic transfers from bank accounts
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
- (later) weekly / monthly / quarterly / annual stuff
- * notices(): send notices (daily, weekly, or monthly digest)

WEEKLY
- after first month, lower floor according to sales (only if B_DEBT_OK)

MONTHLY
- inflation adjustment (monthly, calculating daily bals from history) is credited to the new month. calculated as the sum of t*bal*APR, where bal is each balance they had during the preceding month and t is the number of years that balance lasted. (see daily notify and fine, above)
- share to CGF
- adjust rewards pct for each account (based on growth of account's wider economic ctty)
- archive the 13th previous month (13 is a parameter -- transactions are accessible for ever, but slower)

YEARLY
- 1099s

NOTES:
- Timer Cron is disabled on the staging site. Here's what the cron job would be:
  <star>/5 * * * * /usr/bin/lynx -source http://ws.rcredits.org/cron.php?cron_key=w8x3lfzNQK1bxMz6RMTuF54mMInAs4b8SiVJO9zvWYg
  where <star> means * (not the same as for the production site)
 */
namespace rCredits\Cron;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;

include_once __DIR__ . '/../usd.class';

define('R_DAILY_TASKS', 'START usdTxsHere usdTxsThere recache checkAccts totals paySuppliers payEmployees gifts bank lessOften notices END');
//define ('R_DAILY_TASKS', 'START redistribute END');

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
 *  Do this stuff every time cron is called (normally every 5 minutes)
 *  Check to see if it's time for a less-often periodic activity (and if so do it).
 */
function periodic() {
  if ($row = r\dbQ('SELECT * FROM queue WHERE expire<>0 LIMIT 1')->fetchAssoc()) {
    r\tellStaff('task timed out', $row); // shouldn't happen. Drupal's queue processing hangs until we delete.
    r\dbQ('DELETE FROM queue WHERE item_id=:item_id', u\just('item_id', $row));
  }
  
  $lastDaily = \variable_get('daily', 0);
  if (strftime('%H') >= R_DAILY_HOUR and strftime('%d') != strftime('%d', $lastDaily)) everyDay();
  if (strftime('%M') == '00') everyHour();
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

//************
// Ad hoc (soon, as needed) tasks

/**
 * Reimburse the Dwolla fee (if any) of a recent transaction.
 * @param assoc $item:
 *   txid: payer's USD transaction id (payee's txid is one less)
 *   payee: uid of recipient
 */ 
function coverFee($item) {
  extract(u\just('txid payee', $item));
  $usA = new r\usd($a = r\acct($payee));
  $txid--; // get the payee's txid
  if (!$fee = $usA->fee($txid)) return;
  if ($fee == '?') {
    r\tellStaff('no Dwolla for coverFee', $item);
    sleep(60); // wait a minute, so we don't tie up our server or Dwolla's
    return r\queue(__FUNCTION__, $item); // Dwolla is down -- keep trying
  }
  $for = t('Dwolla fee (reimbursed)');
  $keys = 'payer payee type amount r for taking usdXid';
  $info = u\assoc($keys, $a->community, $payee, TX_REFUND, 0, $fee, $for, 0, $txid);
  r\transact($info);
}

//****************************************************

/**
 * Complete queued USD transactions that have not gone through yet.
 * Maybe this should be the only time they are done!
 */
function usdTxsHere() {
  queueEach('usdTxHere', 'SELECT * FROM r_txs WHERE usdXid=-1 AND r<amount AND state IN(:TX_DONE,:TX_DISPUTED)');
  r\queue('usdTxsHereEND');
}
function usdTxsHereEND() {queueNextAfter('usdTxsHere');}

function usdTxHere($item) {
  extract($item); // tx with not-yet-done USD transfer
  $usa1 = new r\usd($fromAcct = r\acct($payer));
  $usa2 = new r\usd($toAcct = r\acct($payee));
  r\usd::beginAtom();
  if (($usdXid = $usa1->send($amount - $r, $usa2, $taking ? $payeeFor : $payerFor, $err)) and $usdXid != -1) {
    r\dbUpdate('r_txs', compact('xid', 'usdXid'), 'xid');
    r\usd::commit();
  } else r\usd::rollback(); // if ($err == 'not enough') (or any other err) just wait for more USD to be there
}

/**
 * (1) Identify inconsistencies caused by system crashes and reconcile them.
 * (2) Notice and record completion of EFTs.
 */
function usdTxsThere() {
  queueEach('usdTxThere', 'SELECT uid FROM users WHERE :IS_OK');
  r\queue('usdTxsThereEND');
}
function usdTxsThereEND() {queueNextAfter('usdTxsThere');}

function usdTxThere($item) {
  $uid = $item['uid'];
  if (t\est() and realId($uid)) return; // ignore real accounts in tests
//  $cgf = variable_get('cgf_uid');
  $usAcct = new r\usd($acct = r\acct($uid));
//  debug(compact('uid','cgf','item'));
  $since = strtotime('10 days ago'); // bank transfers sometimes take 7 days or more
  $usAcct->each('rCredits\\Cron\\usdTxQ', $since, u\ray('money_sent deposit withdrawal'));
}

function usdTxQ($info) {r\queue($info['Type'] == 'money_sent' ? 'lostUsdTx' : 'bankFollowup', $info);}

/**
 * Reverse any uncommitted USD transaction (any that has no record in r_usd).
 */
function lostUsdTx($info) {
//print_r($info); die();
  extract(u\just('Id Notes Amount SourceId DestinationId Date', $info));
  if (!u\abbreviates(t\est() . USD_NOTE_PREFIX, $Notes)) return; // not one of ours
  if (r\dbExists('r_usd', 'txid=:Id', compact('Id'))) return;
  $created = strtotime($Date) + DW_LAG;
  if (REQUEST_TIME - $created < 60) return; // too recent, maybe still in process? (handle tomorrow)
  list ($txid, $amount) = array($Id, $Amount);
  $omit = t\est() ? (variable_get('cgf_uid') - 1) : NULL; // ignore tester's account
  $payer = r\whoseUsd($SourceId, $omit);
  $payee = r\whoseUsd($DestinationId, $omit);
  if (t\est() and (!$payer or !$payee)) return;
  u\EXPECT($payer and $payee, "missing person in lost tx: payer=$payer payee=$payee");
//  debug(compact(u\ray('info txid amount omit payer payee created'))); return;
  r\tellStaff("lost USD tx: #$txid $amount ($payer to $payee)", $info, FALSE);
  r\usd::rollback(array(compact(u\ray('txid amount payer payee created'))));
}

/**
 * Mark recently completed bank transfers complete.
 */
function bankFollowup($info) {
  extract(u\just('Id Amount ClearingDate Status', $info));
  if ($Status != 'processed') return; // not 'pending'
  r\dbUpdate('r_usd', u\assoc('txid amount completed', $Id, -$Amount, strtotime($ClearingDate) + DW_LAG), 'txid');
}

/**
 * Recalculate and update cached financial balances for each account.
 * Also give signup bonus, if appropriate.
 */
function recache() {
  $fields = 'uid, r, usd, rewards, minimum, floor';
  queueEach('recache1', "SELECT $fields FROM users WHERE uid>0 AND :IS_OK"); // make this IS_MEMBER if early gift
  queueNextAfter('recache');
}

function recache1($item) {
  extract($item);
  if (t\est() and realId($uid)) return; // ignore real accounts in tests
  $acct = r\acct($uid);

  $info = be\creditInfo(compact('uid'));
  foreach (u\just('r usd rewards minimum', $item) as $key => $is) {
    $shouldBe = $key == 'usd' ? $acct->actualUsd()
      : ($key == 'minimum' ? max(0, $floor, $minimum) 
      : $info->$key);
    if ($is != round($shouldBe, 2)) {
      r\tellStaff('cache mismatch', compact(u\ray('key is shouldBe')), $uid);
      $$key = round($shouldBe, 2);
    }
  }

  $DBTX = \db_transaction();
  $acct->update(compact(array_keys($item)));
  r\membershipEvent($acct, 'bona', $usd > 0); // this works for testing too
  unset($DBTX);
}

/**
 * Check accounts for consistency, scams, whatever.
 */
function checkAccts() {
  queueEach('checkAcct', "SELECT uid, minimum, rewards FROM users WHERE uid>0 AND :IS_OK"); //IS_MEMBER?
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
  r\queue('paySuppliersEND'); // let specific tasks go on queue before queueing more stuff 
}
function paySuppliersEND() {queueNextAfter('paySuppliers');}

/**
 * Queue up companies that pay automatically, to pay employees.
 */
function payEmployees() {
  doCompanies('EMPLOYEES');
  r\queue('payEmployeesEND'); // let specific tasks go on queue before queueing more stuff 
}
function payEmployeesEND() {queueNextAfter('payEmployees');}

/**
 * Identify companies that have the given payment option and queue them up for virtualPayments.
 * @param int $which: SUPPLIERS (queue up payments to suppliers) or EMPLOYEES (queue up payments to staff)
 */
function doCompanies($which) {
  $rightType = $which == 'EMPLOYEES' ? 'employerOk' : 'NOT employerOk';
  $sql = <<< EOF
    SELECT u.uid, u.minimum, u.r, u.floor, u.committed,
      SUM(r.amount) AS amounts, 
      SUM(IF($rightType, r.amount, 0)) AS tothese 
    FROM r_relations r 
    LEFT JOIN users u ON u.uid=r.main
    LEFT JOIN users u2 ON u2.uid=r.other
    WHERE r.amount>0 and (u.flags&(1<<:B_VIRTUAL)) AND (u.flags&(1<<:B_OK)) AND (u2.flags&(1<<:B_OK))
    GROUP BY r.main
EOF;
	$result = r\dbQ($sql); // (can't use IS_ in query because flags field is ambiguous)

	while($row = $result->fetchAssoc()) {
	  extract($row);
	  $kitty = max(0, $r - max($minimum, $committed) - max(0, $floor)); // amount to pay out today for this group
    if ($which != 'EMPLOYEES') $kitty *= $tothese / $amounts; // limit suppliers to their share
	  if ($kitty > 0 and $tothese > 0) r\queue('virtualPayments1', compact(u\ray('uid kitty tothese slack rightType'))); 
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
  $sql = "SELECT other, amount FROM r_relations r INNER JOIN users u ON u.uid=r.other WHERE :IS_OK AND main=:main AND amount>0 AND $rightType";
  $result = r\dbQ($sql, compact('main')); // get each recipient's proportionate share of $tothese
  while($row = $result->fetchAssoc()) {
    extract($row);
    $offer = round($amount * $kitty / $tothese, 2);
    if ($offer >= R_VPAYMENT_MIN) {
      r\queue('virtualPayment', compact(u\ray('main other offer')));
      $offers += 1;
      $total += $offer;
    }
  }
  $whom = $rightType == 'employerOk' ? 'employees' : 'suppliers';
  $total = u\fmtAmt($total, 's$r');
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
  
  //$max2 = $acct2->hasBit(B_VIRTUAL) ? 0 : $acct2->maximum;
  $newR = $acct2->r + $offer; // what $acct2 r balance will be after the offer is received
  $excess = round(max(0, $offer - $acct2->usd), 2); // USD shortfall = r to exchange for USD in a 3-way trade
  r\usd::beginAtom();
  $traded = $excess ? r\getUsd($excess, $acct2, $usAcct1) : 0; // give company partial USD through rCredits sale to 3rd party
//debug(compact(u\ray('main other offer newR excess traded')));
  if ($traded >= $excess and ($offer == $traded or $txid = $usAcct2->send($offer - $traded, $usAcct1, 'virtual payment exchange'))) { // other partially pays main in USD if not totally left to 3rd parties
    $info = u\assoc('amount r payer payee for goods usdXid', 0, $offer, $main, $other, t('virtual payment'), TRUE, @$txid);
    r\transact($info); // total rCredits from company to other
    $amount = u\fmtAmt($offer, 's$r');
    $fullName = $acct1->fullName;
    $bonus = u\fmtAmt(r\bonus($other, $main, $offer));
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
  queueEach('gift1', "SELECT g.* FROM r_gifts g INNER JOIN users u ON u.uid=g.uid WHERE NOT completed and giftDate<=:REQUEST_TIME AND :IS_OK");
  queueNextAfter('gifts');
}

/**
 * Handle a gift, if possible.
 * Repeating gifts happen on the same day each month (but earlier for gifts on the 29th-31st)
 * Each instance of a repeating gift creates a repeating post-dated gift record.
 * Gifts happen as soon as the money is there, on or after the gift date.
 */
function gift1($item) {r\acceptGift($item);}

/*
function redistribute() {
  $excess = ':R_CHUNK*FLOOR((:R_EXCESS-usd)/:R_CHUNK)';
  $sql = "SELECT uid, $excess AS excess FROM users WHERE $excess>0 AND NOT :IS_VIRTUAL AND uid>0 AND :IS_OK";
  queueEach('sellR', $sql);
  queueNextAfter('redistribute');
}

function sellR($item) {
  extract(u\just('uid excess', $item));
  $usAcct = new r\usd($acct = r\acct($uid));
  if (!$got = r\getUsd($excess, $acct, $usAcct) or $got < $excess) r\notify($uid, 'cannot offload');
}*/

/**
 * Transfer funds from Dwolla to bank account or vice versa, according to account's settings
 */
function bank() {
//  $excess = 'LEAST(usd, :R_EXCESS)';
  $sql = <<<EOF
  SELECT uid, :R_SHORTFALL AS shortfall
  FROM users 
  WHERE :R_SHORTFALL>0 AND uid>0 AND :IS_OK
EOF;
  queueEach('bank1', $sql);
  queueNextAfter('bank');
}

function bank1($item) {
  extract(u\just('uid shortfall', $item));
  $excess = 0;
//  if ($excess > 0 and $shortfall > 0) return r\tellStaff('min > max', $item, $uid); // should never happen
  $acct = r\acct($uid);
  if (!$amount = toBank($excess > 0 ? $excess : -$shortfall, $acct)) return;
  $action = $excess > 0 ? 'deposit to' : 'draw from';

  $usAcct = new r\usd($acct);
  if ($usAcct->bank($amount, $error)) {
    $status = @$excess > 0 ? 'over the maximum you set' : 'under the minimum you set';
    $amount = u\fmtAmt(abs(@$amount));
    r\notify($uid, 'minmax status|banked', compact(u\ray('action status amount')));
  } elseif (REQUEST_TIME - $acct->created > 3 * DAY_SECS) r\notify($uid, 'cannot bank', compact('action', 'error'));
}

/**
 * Given a desired amount to transfer to the bank, return the amount to actually transfer.
 * @param float $amount: the amount wanting to go to or (if negative) from the bank
 * @param acct $acct: which account
 * @return: the amount to deposit or (if negative) withdraw (zero if none).
 */
function toBank($amount, $acct) {
  $waiting = $acct->waitingToBank(); // amount already requested (if any)
/*  if ($waiting * $amount < 0) { // flip flop -- user could choose better settings (but continue anyway)
    $suggestion = u\fmtAmt(round($acct->minimum + max($acct->minimum * .2, 50, abs($amount), abs($waiting)), -1));
    r\suggest($acct, 'min too low', compact('suggestion'));
  } */
  $amount -= $waiting;
  return $amount < 0 ? R_BANK_MIN * floor($amount / R_BANK_MIN) : 0;
/*  if ($amount > 0) { // excess
    return $amount >= R_BANK_MIN ? $amount : 0;
  } elseif ($amount < 0) { // shortfall
    return min($amount, -R_BANK_MIN);
  } else return 0; */
}

/**
 * Send daily, weekly, or monthly message digests.
 */
function notices() {
  $weekly = (strftime('%A') == R_WEEKLY_DAY);
  $monthly = (strftime('%d') == R_MONTHLY_DAY);
  $where = '(NOT :IS_NOTICE_WEEKLY AND NOT :IS_NOTICE_MONTHLY)';
  if ($weekly) $where .= ' OR :IS_NOTICE_WEEKLY';
  if ($monthly) $where .= ' OR :IS_NOTICE_MONTHLY';
  $sql = "SELECT DISTINCT u.uid, :IS_NOTICE_WEEKLY AS weekly, :IS_NOTICE_MONTHLY AS monthly FROM r_notices n JOIN users u ON u.uid=n.uid WHERE NOT sent AND ($where)";
  queueEach('notice', $sql);
  queueNextAfter('notices');
}

function notice($item) {
  extract(u\just('uid weekly monthly', $item));
  $result = r\dbQ('SELECT message FROM r_notices WHERE uid=:uid AND NOT sent', compact('uid'));
  $messages = '';
  while ($row = $result->fetch()) $messages .= "<li>$row->message</li>\n";
  if ($messages) {
    $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
    $yestertime = u\formatDate($start = strtotime("-1 $unit", REQUEST_TIME));
    $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
    r\notify($uid, 'notices', compact(u\ray('messages unit range yestertime')), TRUE);
    r\dbQ('UPDATE r_notices SET sent=:REQUEST_TIME WHERE uid=:uid AND NOT sent', compact('uid'));
  }
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  $elapsed = time() - REQUEST_TIME;
//  echo "Daily cron done after $elapsed seconds"; // keep
  r\tellStaff("Daily cron ended successfully after $elapsed seconds", '', FALSE);
}

/**
 * Run the next daily function.
 */
function queueNextAfter($task) {
  global $cronOp;
  $tasks = u\ray(R_DAILY_TASKS);

  if (t\est() and @$cronOp and $cronOp != 'ALL') {
    r\queue($task == $cronOp ? 'END' : $cronOp);
  } else r\queue($tasks[array_search($task, $tasks) + 1]);
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

function queueEach($op, $sql, $subs = array()) {
  $result = r\dbQ($sql, $subs);
  while ($row = $result->fetchAssoc()) r\queue($op, $row);
}

function realId($uid) {return ($uid <= variable_get('cgf_uid'));}
