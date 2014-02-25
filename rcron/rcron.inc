<?php
/**
 * @file
 * Functions to run periodically.
 
 * = written
 ** = tested
 
DAILY (every 5 mins for 5 mins, starting at 2am)
- * usdTxsThere() rollback lost USD transactions, mark completed deposits/withdrawals
- ** recache() check / recalc cached r, rewards, and usd
- checkAccts() check for consistency, security, scams
- ** stats(): calculate daily statistics and record in r_stats and in the stats field of user records
- ** paySuppliers(): automatic payment exchanges to suppliers
- UNUSED ** payEmployees(): Each employee (and supplier) gets relieved of their excess rCredits immediately if they don't have enough USD to trade the employer for them.
- ** gifts(): contributions to CGF (not shares)
- UNUSED * redistribute(): (selling of unwanted rCredits) looks at accounts over their maximum (maximum is ignored for companies that do payment exchanges)
- homogenize: spread the USD out a little in the system
- pool: hold some USD in the community account
- ** bank(): automatic transfers from bank accounts
- (later) calculate credit line (for overdrafts, Dwolla outage (eg 503), etc.): give the payer (r)credit up to the amount they earned or bought in the past 5 days if they have been a member for a month, 30 days for 6 months
- [later] notify and maybe fine late borrowers (might figure interest at the same time, since it requires examining each balance -- but only actually credit the interest monthly, otherwise discard it)
- [later] things to do 30/60/90 (etc) days after opening account:
  . 1mo: notice - suggest sending invitations
      $email = $mya->mail;
      $invited = ($nonperson or db\lookup('COUNT(*)', 'r_invites', 'inviter=:myid AND email<>:email', compact('myid', 'email')) >= 1);
          $step4 = t('<a>Invite someone</a> to sign up for rCredits! No one can join without an invitation. You can help by sending one. You might particularly want to invite companies and individuals you buy from or sell to (for example your employer or employees).');

  . 1mo: suggest another contribution if first was small
  . 6mo: allow cashing out rewards if system stable
- [later] statements for those who want daily statements
- (later) weekly / monthly / quarterly / annual stuff
- * notices(): send notices (daily, weekly, or monthly digest)

WEEKLY
- soldOut() and missedOut() tell members what they did that was dumb (sold rC or missed a chance to buy them)
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
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;

include_once __DIR__ . '/../usd.class';

define('R_DAILY_TASKS', 'START usdTxsThere recache checkAccts stats lessOften gifts homogenize pool bank notices END');
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
  if ($row = db\q('SELECT * FROM queue WHERE expire<>0 LIMIT 1')->fetchAssoc()) {
    r\tellStaff('task timed out', $row); // shouldn't happen. Drupal's queue processing hangs until we delete.
    db\q('DELETE FROM queue WHERE item_id=:item_id', u\just('item_id', $row));
  }
  often('hour', 'M', '00', HOUR_SECS);
  often('day', 'H', R_DAILY_HOUR, DAY_SECS); // longer periods get queued in lessOften(), so notices are last
//  $lastDaily = \variable_get('daily', 0);
//  if (strftime('%H') >= R_DAILY_HOUR and strftime('%d') != strftime('%d', $lastDaily)) everyDay();
//  if (strftime('%M') == '00') everyHour();
}

function everyHour() {
  // stuff to do every hour
  queueEach('acctHourly', "SELECT uid FROM users WHERE uid<>0");
  // r\tellStaff('hourly cron');
}

/**
 * Account checks to do every hour.
 */
function acctHourly($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  $a->shortfall('buy', PHP_INT_MAX); // notice recent deposits, if any
}

function everyDay() {
  queueNextAfter('START');
//  \variable_set('daily', REQUEST_TIME);
  // miscellaneous quick stuff to do every day
  $time = time() - 7 * DAY_SECS;
  db\q('DELETE FROM r_log WHERE time<:time', compact('time')); // zap log except for the past few days
}

function lessOften() {
  often('week', 'A', R_WEEKLY_DAY, 7 * DAY_SECS);
  often('month', 'd', R_MONTHLY_DAY, 30 * DAY_SECS);
  often('year', 'm', '01', 365 * DAY_SECS);  
//  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
//  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
  queueNextAfter('lessOften');
}

/**
 * Do something periodically if it's time and hasn't been done yet.
 * That is, if the chosen point in the period has been reached AND
 *   (it has been at least a whole period since the last run OR
 *   (it has been at least half a period and we're at no later a point than last time)
 * @param string $period: the period
 * @param string $code: the strftime code for the smaller period to match or exceed
 * @param string $value: the value to match or exceed
 * @param int $periodLen: approximate length of the period, in seconds
 */
function often($period, $code, $value, $periodLen) {
  $lastCron = \variable_get('r_last_cron', array());
  $lastTime = @$lastCron[$period];
  $elapsed = REQUEST_TIME - $lastTime;
  list ($point, $lastPoint) = array(strftime("%$code", REQUEST_TIME), strftime("%$code", $lastTime));
  if ($point >= $value and ($elapsed > $periodLen or ($elapsed > $periodLen / 2 and $point <= $lastPoint))) {
    $lastCron[$period] = REQUEST_TIME;
    \variable_set('r_last_cron', $lastCron); // remember that this periodic activity has been done
    $function = 'rCredits\\Cron\\every' . ucwords($period);
    if ($period != 'hour' and $period != 'day') r\tellStaff("Running cron for $period at " . strftime('%I:%M%P', REQUEST_TIME));
    $function(); // do it
  }
}

function everyWeek() {
// This is no longer relevant  queueEach('soldOut', 'SELECT payer, SUM(r) AS total FROM r_txs WHERE payer>0 AND amount=0 AND type=:TX_TRANSFER AND NOT goods AND state=:TX_DONE AND completed>:REQUEST_TIME-:WEEK_SECS GROUP BY payer');
// queueEach('missedOut', ??
}

/**
 * Do monthly tasks.
 */
function everyMonth() {
  $start = u\monthDay1($end = u\monthDay1() - 1);
  if (t\est()) $start = strtotime('-30 days', $end = strtotime('today') - 1) + 1; // the past 30 days
  queueEach('inflationAdjust', "SELECT uid, $start AS start, $end AS end FROM users WHERE :IS_OK AND uid>1");  
  queueEach('shareCommitted', 'SELECT uid, committed, share FROM users WHERE :IS_OK AND committed>0');
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
  if (t\est()) $txid = t\seq() - 2; else $txid--; // get the payee's txid
  if (!$fee = $usA->fee($txid)) return;
  if ($fee == '?') {
    r\tellStaff('no Dwolla for coverFee', $item);
    sleep(60); // wait a minute, so we don't tie up our server or Dwolla's
    return r\queue('coverFee', $item); // Dwolla is down -- keep trying
  }
  $for = t('Dwolla fee');
  $keys = 'payer payee type amount for taking';
  
  r\usd::beginAtom();
  r\transact(u\ray($keys, $a->community, $payee, TX_REFUND, $fee, $for, 0));
  r\usd::addToAtom($txid + 2, $fee, $payee, -1, REQUEST_TIME); // -1 to distinguish from bank transfers
  $a->actualUsd(-$fee);
  r\usd::commit();
}

//****************************************************

/**
 * Tell a member about missed opportunities to keep their rCredits and spend them for rebates.
 * Suggest a specific easy remedy.
 * UNUSED
 */
function soldOut($item) {
  return; // UNUSED
  /*
  extract(u\just('payer total', $item));
  $a = r\acct($payer);
  if (!$rebatePct = $a->rebate) return; // no potential rebates (later suggest reaching out)
  if (db\exists('r_usd', 'payer=:payer AND payee=0 AND amount>0 AND created>:REQUEST_TIME-:WEEK_SECS', compact('payer'))) return; // member moved money to bank account this week
  $rebates = u\fmtAmt($rebatePct * $total / 100);
  $total = u\fmtAmt($total);
  suggestMin($a);
  */
}

/**
 * Calculate and credit inflation adjustment to each member account.
 */
function inflationAdjust($item) {
  extract(u\just('uid start end', $item));
  $avg = r\averageBalance($uid, $start, $end);
  $adjustment = round($avg * R_INFLATION_RATE / 12, 2);
  if ($adjustment > 0) be\fund($uid, TX_INFLATION, $adjustment, t('inflation adjustment')); // don't adjust debts
}

/**
 * Share committed rewards with CGF.
 */
function shareCommitted($item) {
  extract(u\just('uid committed share', $item));
  $share = number_format($share, 1);
  r\notify($uid, 'share gift', compact('share'));
  $info = u\ray('giftDate uid amount often honor share', REQUEST_TIME, $uid, $committed, 1, 'share', -1);
  $DBTX = \db_transaction();
  db\insert('r_gifts', $info);
  r\acct($uid)->update(array('committed' => 0));
  unset($DBTX);
}

/**
 * (1) Identify inconsistencies caused by system crashes and reconcile them.
 * (2) Notice and record completion of EFTs.
 */
function usdTxsThere() {
  queueEach('usdTxThere', 'SELECT uid FROM users WHERE :IS_OK AND :IS_DW');
  r\queue('usdTxsThereEND');
}
function usdTxsThereEND() {queueNextAfter('usdTxsThere');}

function usdTxThere($item) {
  $uid = $item['uid'];
  if (t\est() and realId($uid)) return; // ignore real accounts in tests
//  $cgf = variable_get('cgf_uid');
  $usAcct = new r\usd($acct = r\acct($uid));
///  debug(compact('uid','cgf','item'));
  $since = strtotime(R_ANCIENT_EFT. ' days ago'); // bank transfers sometimes take many days
  $usAcct->each('rCredits\\Cron\\usdTxQ', $since, u\ray('money_sent deposit withdrawal'));
}

function usdTxQ($info) {r\queue($info['Type'] == 'money_sent' ? 'lostUsdTx' : 'bankFollowup', $info);}

/**
 * Record any unrecorded USD transaction (any that has no record in r_usd).
 * @param assoc $tx: the lost transaction
 */
function lostUsdTx($tx) {
  extract(u\just('Id Notes Amount SourceId DestinationId Date', $tx));
  if (!u\abbreviates(t\est() . USD_NOTE_PREFIX, $Notes)) return; // not one of ours
  if (db\exists('r_usd', 'txid=:Id', compact('Id'))) return;
  $created = strtotime($Date);
  if (!t\est() and time() - $created < 60) return; // too recent, maybe still in process? (handle tomorrow)
  list ($txid, $amount) = array($Id, $Amount);
  $payer = r\whoseUsd($SourceId);
  $payee = r\whoseUsd($DestinationId);
  if (t\est() and (!$payer or !$payee)) return;
  u\EXPECT($payer and $payee, "missing person in lost tx: payer=$payer payee=$payee");
  r\tellStaff("lost USD tx: #$txid $amount ($payer to $payee)", $tx, FALSE);
  r\usd::beginAtom();
  r\usd::addToAtom($txid, $amount, $payer, $payee, $created, $created);
  r\usd::commit();
}

/**
 * Mark recently completed bank transfers complete.
 * @param assoc $tx: the USD deposit or withdrawal transaction
 * Note that transfers to the bank were marked complete immediately, but not with the correct ClearingDate.
 */
function bankFollowup($tx) {
  r\usdTransferDone($tx);
}

/**
 * Recalculate and update cached financial balances for each account.
 * Also give signup bonus, if appropriate.
 */
function recache() {
  $fields = 'uid, r, usd, rewards, minimum, floor';
  queueEach('recache1', "SELECT $fields FROM users WHERE :IS_OK"); // make this IS_MEMBER if early gift
  queueNextAfter('recache');
}

function recache1($item) {
  extract($item);
  if (t\est() and realId($uid)) return; // ignore real accounts in tests
  $acct = r\acct($uid);
  $acct->cacheOk();
  //$acct->update(compact(array_keys($item)));
}

/**
 * Check accounts for consistency, scams, whatever.
 */
function checkAccts() {
  queueEach('checkAcct', "SELECT uid, minimum, rewards FROM users WHERE uid<>0");
  queueNextAfter('checkAccts');
}

function checkAcct($item) {
  extract($item); // (prepared for additional tasks)
  $a = r\acct($uid);
  if (r\membershipEvent($a, 'bona', $a->usd > 0)) r\tellStaff('noticed BONA late', NULL, $uid);
  //look at agreement date and first transaction date and last transaction date. queue appropriate funcs
}

/**
 * Update system-wide and community-wide stats and individual user status
 */
function stats() {
  queueEach('stats1', 'SELECT uid FROM users WHERE uid>1 AND :IS_OK');
  r\getStats(r\serverUid());
//  $result = db\q('SELECT uid FROM users WHERE uid<0');
//  while ($uid = $result->fetchColumn()) r\totals($uid, TRUE);
  queueNextAfter('stats');
}

/**
 * Update stats for one account.
 */
function stats1($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  $today = strtotime('today');
  $monthAgo = strtotime('-1 month', $today);
  $avgBalPastMo = @number_format(r\averageBalance($uid, $monthAgo, $today), 2);
  $avgBalEver = @number_format(r\averageBalance($uid, strtotime('today', $a->created), $today), 2);
  $subs = compact('uid', 'monthAgo');
  
  $sql = <<<EOF
    SELECT SUM(amount) AS giftsEver, SUM(IF(completed<:monthAgo, 0, amount)) AS giftsPastMo
    FROM r_gifts WHERE uid=:uid AND completed
EOF;
  $row1 = db\q($sql, $subs)->fetchAssoc() ?: array(); // giftsEver, giftsPastMo
  
  $sql = <<<EOF
    SELECT SUM(IF(type IN (:TX_REBATE,:TX_BONUS), amount, 0)) AS beneEver,
    SUM(IF(type IN (:TX_REBATE,:TX_BONUS) AND completed>=:monthAgo, amount, 0)) AS benePastMo,
    SUM(IF(type IN (:TX_SIGNUP,:TX_INFLATION,:TX_GRANT), amount, 0)) AS extraEver,
    SUM(IF(type IN (:TX_SIGNUP,:TX_INFLATION,:TX_GRANT) AND completed>=:monthAgo, amount, 0)) AS extraPastMo
    FROM r_txs WHERE payee=:uid AND state=:TX_DONE
EOF;
  $row2 = db\q($sql, $subs)->fetchAssoc() ?: array(); // get beneEver, benePastMo
  
  $sql = <<<EOF
    SELECT SUM(amount) AS bankedEver, SUM(IF(completed<:monthAgo, 0, amount)) AS bankedPastMo
    FROM r_usd WHERE payer=:uid AND completed
EOF;
  $row3 = db\q($sql, $subs)->fetchAssoc() ?: array(); // get bankedEver, bankedPastMo
  $newData = compact(u\ray('avgBalPastMo avgBalEver'));
  $a->update(u\ray('stats', $row1 + $row2 + $row3 + $newData));
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

/**
 * Spread the USD out somewhat equally among the membership
 * Any account that might have more than the average after sending a chunk to the ctty, gives a chunk to others.
 * Note that an account's rCredits balance (r) can go negative if merely trading for USD
 */
function homogenize() {
  $result = db\q('SELECT AVG(usd) AS avg, community FROM users WHERE uid>1 AND :IS_OK GROUP BY community');
  while ($row = $result->fetchAssoc()) {
    extract($row);
    queueEach('homogenize1', 'SELECT uid, usd, community AS ctty FROM users WHERE community=:community AND uid>1 AND usd>:avg AND :IS_OK', compact('avg', 'community'));
  }
  queueNextAfter('homogenize');
}

/**
 * Offload some USD from an account, to others with less.
 * Avoid lending an account USD beyond its credit limit (keep r >= floor).
 */
function homogenize1($item) {
  extract(u\just('uid usd ctty', $item));
  $a = r\acct($uid);
  $result = db\q('SELECT uid AS uid2 FROM users WHERE community=:ctty AND usd<:usd-:R_CHUNK AND uid>1 AND :IS_OK AND :IS_DW AND r>=floor+:R_CHUNK ORDER BY usd', compact('ctty', 'usd'));
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $a2 = r\acct($uid2); // use the latest variables, because tradeR might have changed the values
    if ($a2->usd < $a->usd - R_CHUNK and $a2->r >= $a2->floor + R_CHUNK) r\tradeR(R_CHUNK, $uid2, $uid);
    // This trade might fail (gently) if a bank transfer goes through in between
  }
}

/**
 * Hold some of the community's USD in the community account.
 * Leave each contributing account with at least R_CHUNK so it can help someone cash out the next day.
 */
function pool() {
  $result = db\q('SELECT SUM(usd) AS total, SUM(1) AS sources, community FROM users WHERE uid>1 AND :IS_OK AND :IS_DW AND usd>0 GROUP BY community');
  while ($row = $result->fetchAssoc()) { // for each community
    extract($row); // total, sources, community
    $ctty = r\acct($community);
    if (!$ctty->usdAccount) continue; // not-yet-active region or community account
    if ($ctty->usd / $total < R_CTTY_USD_SHARE) queueEach('pool1', 'SELECT uid, usd, community FROM users WHERE uid>1 AND :IS_OK AND community=:community AND usd>=2*:R_CHUNK ORDER BY usd DESC', compact('community')); // okay if ctty has a bit too much
  }
  queueNextAfter('pool');
}

/**
 * Trade $10us to the community, leaving the account with at least $10us (if possible).
 */
function pool1($item) {
  extract(u\just('uid usd community', $item));
  if ($usd >= R_CHUNK) r\tradeR(R_CHUNK, $community, $uid); // might fail (gently) if a bank transfer goes through in between
}

/**
 * Transfer funds from Dwolla to bank account or vice versa, according to account's settings
 */
function bank() {
//  $excess = 'LEAST(usd, :R_EXCESS)';
  $sql = <<<EOF
  SELECT uid, :R_SHORTFALL AS shortfall
  FROM users 
  WHERE :R_SHORTFALL>0 AND uid>1 AND :IS_OK AND :IS_DW
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
  $where = $dailySql = '(NOT :IS_NOTICE_WEEKLY AND NOT :IS_NOTICE_MONTHLY)';
  if ($weekly) $where .= ' OR :IS_NOTICE_WEEKLY';
  if ($monthly) $where .= ' OR :IS_NOTICE_MONTHLY';
  $sql = "SELECT DISTINCT u.uid, :IS_NOTICE_WEEKLY AS weekly, :IS_NOTICE_MONTHLY AS monthly, $dailySql AS daily FROM r_notices n JOIN users u ON u.uid=n.uid WHERE NOT sent AND ($where)";
  queueEach('notice', $sql);
  queueNextAfter('notices');
}

function notice($item) {
  extract(u\just('uid daily weekly monthly', $item));
  $a = r\acct($uid);
  $result = db\q('SELECT message, created FROM r_notices WHERE uid=:uid AND NOT sent', compact('uid'));
  $messages = '';
  while ($row = $result->fetch()) {
    $date = $daily ? '' : (strftime('%b %d', $row->created) . ': ');
    $messages .= "<li>$date$row->message</li>\n";
  }
  if ($messages) {
    $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
    $yestertime = u\fmtDate($start = strtotime("-1 $unit", REQUEST_TIME));
    $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
    extract(u\just('return', $a->stats()));
    list ($balance, $rewards, $ourEmail) = array(u\fmtAmt($a->usd + $a->r), u\fmtAmt($a->rewards), R_REGION_EMAIL);
    r\notify($uid, 'notices', compact(u\ray('messages unit range yestertime balance rewards return ourEmail')), TRUE);
    db\q('UPDATE r_notices SET sent=:REQUEST_TIME WHERE uid=:uid AND NOT sent', compact('uid'));
  }
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  $lastCron = \variable_get('r_last_cron', array());
  $elapsed = time() - @$lastCron['day'];
//  echo "Daily cron done after $elapsed seconds"; // keep
  r\tellStaff("At " . strftime('%I:%M%P') . ", after $elapsed secs, daily cron ended successfully.", '', FALSE);
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

function paymentExchangeError($acct1, $acct2) {
  paymentExchangeError1($acct1, 'to', $acct2);
  paymentExchangeError1($acct2, 'from', $acct1);
  return FALSE;
}

function paymentExchangeError1($acct1, $way, $acct2) {
  $other = $acct2->fullName;
  r\notify($acct1->id, 'payment exchange error', compact('way', 'other'));
}

function queueEach($op, $sql, $subs = array()) {
  $result = db\q($sql, $subs);
  while ($row = $result->fetchAssoc()) r\queue($op, $row);
}

function realId($uid) {return ($uid <= variable_get('cgf_uid'));}
