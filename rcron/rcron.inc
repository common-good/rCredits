<?php
namespace rCredits\Cron;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;
use rCredits\Risk as k;
use rCredits\Cron as cron;

/**
 * @file
 * Functions to run periodically.
 
 f('cr.zot', arg1,...);
 
 * = written
 ** = tested
 
DAILY (every 5 mins for 5 mins, starting at 2am)
- * usdTxsThere() rollback lost USD transactions, mark completed deposits/withdrawals
- ** recache() check / recalc cached r, rewards, and usd
- checkAccts() check for consistency, security, scams
- ** stats(): calculate daily statistics and record in r_stats and in the stats field of user records
- ** paySuppliers(): automatic payment exchanges to suppliers
- UNUSED ** payEmployees(): Each employee (and supplier) gets relieved of their excess rCredits immediately if they don't have enough USD to trade the employer for them.
- ** gifts(): donations to CGF (not shares)
- UNUSED * redistribute(): (selling of unwanted rCredits) looks at accounts over their maximum (maximum is ignored for companies that do payment exchanges)
- ** homogenize(): spread the USD out a little in the system (just until we can do ACH debits)
- ** pool(): hold some USD in the community account (just until we can do ACH debits)
- ** bank(): automatic transfers from bank accounts
- [later] notify and maybe fine late borrowers (might figure interest at the same time, since it requires examining each balance -- but only actually credit the interest monthly, otherwise discard it)
- ** tickle(): things to do 2/30/60/90 (etc) days after login/access/created date
  . 1mo: notice - suggest sending invitations
      $email = $mya->mail;
      $invited = ($nonperson or db\lookup('COUNT(*)', 'r_invites', 'inviter=:myid AND email<>:email', compact('myid', 'email')) >= 1);
          $step4 = t('<a>Invite someone</a> to sign up for rCredits! No one can join without an invitation. You can help by sending one. You might particularly want to invite companies and individuals you buy from or sell to (for example your employer or employees).');

  . 1mo: suggest another donation if first was small
  . 6mo: allow cashing out rewards if system stable? Probably not.
- [later] statements for those who want daily statements
- weekly / monthly / quarterly / annual stuff
- ** notices(): send notices (daily, weekly, or monthly digest)

WEEKLY
- soldOut() and missedOut() tell members what they did that was dumb (sold rC or missed a chance to buy them)
- after first month, lower floor according to sales (only if B_DEBT)

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

define('R_DAILY_TASKS', 'START recache checkAccts setNear acctStats cttyStats trust acctRisk txRisk achRisk tickle lessOften gifts bank bona employees notices END'); // had homogenize, pool 
//define ('R_DAILY_TASKS', 'START redistribute END');
include_once __DIR__ . '/rcron-subs.inc';

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
  if (strftime('%H') == (R_DAILY_HOUR + 6) % 24) {
    $lastCron = \variable_get('r_last_cron', []);
    $lastTime = @$lastCron['day'];
    if (REQUEST_TIME - $lastTime > DAY_SECS) r\tellStaff('daily cron FAILED');
  }
  // r\tellStaff('hourly cron');
}

/**
 * Account checks to do every hour.
 */
function acctHourly($item) {
  return; // nothing to do, currently
  extract(u\just('uid', $item));
  $a = r\acct($uid);
//  $a->shortfall(R_FOR_GOODS, PHP_INT_MAX); // notice recent deposits, if any
}

function everyDay() {
  queueNext('START');
//  \variable_set('daily', REQUEST_TIME);
  // miscellaneous quick stuff to do every day
  $time = time() - 7 * DAY_SECS;
  db\q('DELETE FROM r_log WHERE time<:time', compact('time')); // zap log except for the past few days
  db\q('DELETE FROM r_do WHERE completed<0 AND :now>-completed', ['now'=>time()]); // completed<0 is -expiry time
}

/**
 * Do things a certain number of days after the created, signed, invite, firstTx, last login, or last access date.
 */
function tickle() {
//  $delays = '1,2,3,5,8,13,21,34,55,89,144'; // Fibonacci progression of delays
  $today = strtotime('today');
  $timeSince = "(FLOOR(($today-@DATE)/:DAY_SECS)+1)"; // $today works better in tests than :REQUEST_TIME or time()

  // Remind invitee to accept
  $delays = '3,8,21';
  $elapsed = str_replace('@DATE', 'invited', $timeSince);
  $sql = <<<EOF
    SELECT email, inviter, code, $elapsed AS elapsed FROM r_invites i
    LEFT JOIN users u ON u.mail=i.email
    WHERE $elapsed IN ($delays) AND u.uid IS NULL AND invitee=0
EOF;
  queueEach('tickleSignup', $sql);
    
  // Remind newbie to take another step (or admin to activate it)
  $delays = '1,2,3,5,8'; // Fibonacci progression of delays (followed by every week)
  $elapsed = str_replace('@DATE', 'access', $timeSince);
  $sql = <<<EOF
    SELECT uid, $elapsed AS elapsed FROM users 
    WHERE ($elapsed IN ($delays) OR ($elapsed>8 AND $elapsed%7=0))
    AND uid>1 AND NOT :IS_OK AND NOT :IS_NONUDGE AND NOT :IS_CLOSED
EOF;
  queueEach('tickleStep', $sql);

  // Tickle sometime after first Tx (activation date)
  $elapsed = str_replace('@DATE', 'date0', $timeSince);
  $months = "ROUND($elapsed/:MONTH_DAYS)";
  $start = strtotime('-6 months', strtotime('today', time()));
  $day = date('d');
  $tomorrow = date('d', strtotime('+1'));
  $sql = <<<EOF
    SELECT uid, $months AS months, monthly
    FROM users u LEFT JOIN (
      SELECT payer, MIN(created) AS date0, SUM(IF(created<:start, 0, amount))/6 AS monthly,
      DAY(FROM_UNIXTIME(MIN(created))) AS day0
      FROM r_txs GROUP BY payer
    ) t ON t.payer=u.uid
    WHERE $elapsed>15 AND uid > 1 AND :IS_BONA
    AND ($day=day0 OR (day0>$day AND $tomorrow=1))
EOF;
  queueEach('tickleActive', $sql, compact('start'));
  
  queueNext();
}

/**
 * Remind invitee to accept invitation.
 */
function tickleSignup($item) {
  global $base_url;
  extract(u\just('email inviter code elapsed', $item));
  $inviterName = r\acct($inviter)->fullName;
  $count = $elapsed < 7 ? t('two') : ($elapsed < 14 ? t('one') : 0);
  $nudge = $count ? u\tt("We will send you just @count more reminder(s)", compact('count')) : t('This is your last reminder');
  r\rMail('nonmember', $email, u\ray('inviterName nudge code noFrame', $inviterName, $nudge, $code, TRUE));
}

/**
 * Remind newb to take another step.
 */
function tickleStep($item) {
  extract(u\just('uid elapsed', $item));
  $a = r\acct($uid);
  if (!$a->member and $step = $a->nextStep()) {
//    $msg = $step == 'verify' ? 'call bank' : (($step == 'sign' or $step == 'contact') ? 'do step one' : 'take another step');
    $msg = ($step == 'sign' or $step == 'contact') ? 'do step one' : 'take another step';
/*    $when = $elapsed < 3 ? 
      (u\nextBusDay(+1) . t(' (how about 10am?)'))
    : (u\nextBusDay(0) . t(' between 9am and 4pm')); // for verify
    */
//    if ($step != 'verify' or $elapsed > 1) r\notify($a->id, "$msg|sign in", compact('when'));
    r\notify($a->id, "$msg|sign in");
  } elseif (!$a->ok) {
    return r\tellStaff('activate account', '', $a->uid);
  }
}

/**
 * Calculate credit line (for overdrafts etc.): give the member (r)credit up to their average monthly throughput over the past 6 months (which comes to 1/6 their monthly throughput if they've been in for a month, 2/6 for 2 months, etc, up to 100% at 6 months)
 */
function tickleActive($item) {
  extract(u\just('uid months monthly', $item));
  $a = r\acct($uid);
  $ratio = $monthly ? $a->floor / $monthly : 1;
  if ($ratio > 1.1 or $ratio < .9) { // big change (or any change in sign)
    $a->update('floor', round(-$monthly, 2));
    $msg = 'new floor' . ($a->can(B_DEBT) ? '' : '|no floor effect');
    r\notify($uid, $msg, ['limit' => u\fmtAmt($monthly)]);
  }
}
 
/**
 * Do periodic activities where the period is longer than a day.
 */
function lessOften() {
  often('week', 'A', R_WEEKLY_DAY, 7 * DAY_SECS);
  often('month', 'd', R_MONTHLY_DAY, 30 * DAY_SECS);
  often('year', 'm', '01', 365 * DAY_SECS);  
//  if (strftime('%A') == R_WEEKLY_DAY) everyWeek();
//  if (strftime('%d') == R_MONTHLY_DAY) everyMonth();
  queueNext();
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
  $lastCron = \variable_get('r_last_cron', []);
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
// This is no longer relevant  queueEach('soldOut', 'SELECT payer, SUM(r) AS total FROM r_txs WHERE payer>0 AND amount=0 AND type=:TX_TRANSFER AND NOT goods AND created>:REQUEST_TIME-:WEEK_SECS GROUP BY payer');
// queueEach('missedOut', ??
}

/**
 * Do monthly tasks.
 */
function everyMonth() {
  $start = u\monthDay1($end = u\monthDay1() - 1);
  if (u\test()) $start = strtotime('-30 days', $end = strtotime('today') - 1) + 1; // the past 30 days
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
 */ /*
function coverFee($item) {
  extract(u\just('txid payee', $item));
  $usA = new r\usd($a = r\acct($payee));
  if (u\test()) $txid = t\seq() - 2; else $txid--; // get the payee's txid
  if (!$fee = $usA->fee($txid)) return;
  if ($fee == '?') {
    r\tellStaff('no Dwolla for coverFee', $item);
    sleep(60); // wait a minute, so we don't tie up our server or Dwolla's
    return r\queue('coverFee', $item); // Dwolla is down -- keep trying
  }
  $for = t('Dwolla fee');
  $keys = 'payer payee type amount for taking';
  
  r\usd::beginAtom();
  new r\x(u\ray($keys, $a->community, $payee, TX_REFUND, $fee, $for, 0));
  r\usd::addToAtom($txid + 2, $fee, $payee, -1, REQUEST_TIME); // -1 to distinguish from bank transfers
  $a->actualUsd(-$fee);
  r\usd::commit();
}
*/
//****************************************************

/**
 * Calculate and credit inflation adjustment to each member account.
 */
function inflationAdjust($item) {
  extract(u\just('uid start end', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slave in joint account gets no inflation (master gets inflation for both)
  $avg = r\averageBalance($uid, $start, $end);
  $adjustment = round($avg * R_INFLATION_RATE / 12, 2);
  if ($adjustment > 0) be\fund($uid, TX_INFLATION, $adjustment, t('inflation adjustment')); // don't adjust debts
}

/**
 * Share committed rewards with CGF.
 */
function shareCommitted($item) {
  extract(u\just('uid committed share', $item));
  $share = round($share, 1);
  r\notify($uid, 'share gift', compact('share'));
  $info = u\ray('giftDate uid amount often honor share', REQUEST_TIME, $uid, $committed, 1, 'share', -1);
  $DBTX = \db_transaction();
  db\insert('r_gifts', $info);
  r\acct($uid)->update('committed', 0);
  unset($DBTX);
}

/**
 * (1) Identify inconsistencies caused by system crashes and reconcile them.
 * (2) Notice and record completion of EFTs.
 *//*
function usdTxsThere() {
  queueEach('usdTxThere', 'SELECT uid FROM users WHERE :IS_OK AND :HAS_BANK');
  r\queue('usdTxsThereEND');
}
function usdTxsThereEND() {queueNext();}

function usdTxThere($item) {
  $uid = $item['uid'];
  if (u\test() and realId($uid)) return; // ignore real accounts in tests
  $usAcct = new r\usd($a = r\acct($uid));
  $since = strtotime(R_ANCIENT_EFT. ' days ago'); // bank transfers sometimes take many days
  $usAcct->each('rCredits\\Cron\\usdTxQ', $since, u\ray('money_sent deposit withdrawal'));
}

function usdTxQ($info) {r\queue($info['Type'] == 'money_sent' ? 'lostUsdTx' : 'bankFollowup', $info);}
*/
/**
 * Record any unrecorded USD transaction (any that has no record in r_usd).
 * @param assoc $tx: the lost transaction
 *//*
function lostUsdTx($tx) {
  extract(u\just('Id Notes Amount SourceId DestinationId Date', $tx));
  if (!u\abbreviates(u\test() . USD_NOTE_PREFIX, $Notes)) return; // not one of ours
  if (db\exists('r_usd', 'txid=:Id', compact('Id'))) return;
  $created = strtotime($Date);
  if (!u\test() and time() - $created < 60) return; // too recent, maybe still in process? (handle tomorrow)
  list ($txid, $amount) = array($Id, $Amount);
  $payer = r\whoseUsd($SourceId);
  $payee = r\whoseUsd($DestinationId);
  if (u\test() and (!$payer or !$payee)) return;
  u\EXPECT($payer and $payee, "missing person in lost tx: payer=$payer payee=$payee");
  r\tellStaff("lost USD tx: #$txid $amount ($payer to $payee)", $tx, FALSE);
  r\usd::beginAtom();
  r\usd::addToAtom($txid, $amount, $payer, $payee, $created, $created);
  r\usd::commit();
}
*/

/**
 * Mark recently completed bank transfers complete.
 * @param assoc $tx: the USD deposit or withdrawal transaction
 * Note that transfers to the bank were marked complete immediately, but not with the correct ClearingDate.
 *//*
function bankFollowup($tx) {
  r\usdTransferDone($tx);
}
*/

/**
 * Recalculate and update cached financial balances for each account.
 * Also give signup bonus, if appropriate.
 */
function recache() {
  $fields = 'uid, r, usd, rewards, minimum, floor';
  queueEach('recache1', "SELECT $fields FROM users WHERE :IS_OK"); // make this IS_MEMBER if early gift
  queueNext();
}

function recache1($item) {
  extract($item);
//  if (u\test() and realId($uid)) return; // ignore real accounts in tests
  $a = r\acct($uid);
  $a->cacheOk();
  //$a->update(compact(array_keys($item)));
}

/**
 * Check accounts for consistency, scams, whatever.
 */
function checkAccts() {
  queueEach('checkAcct', "SELECT uid, minimum, rewards FROM users WHERE uid<>0");
  queueNext();
}

function checkAcct($item) {
  extract($item); // (prepared for additional tasks)
  $a = r\acct($uid);
// NO  if (r\membershipEvent($a, 'bona', $a->j_r > 0)) r\tellStaff('noticed BONA late', NULL, $uid);
  //look at agreement date and first transaction date and last transaction date. queue appropriate funcs
}

/**
 * Analyze anew how each member relates to other members.
 */
function setNear() {
  db\q('TRUNCATE r_near'); // start from scratch
  $queries = join(' UNION ', [
    'SELECT DISTINCT inviter AS id1, invitee AS id2, 1 AS w FROM r_invites WHERE invitee',
    'SELECT DISTINCT person AS id1, proxy AS id2, 1 AS w FROM r_proxies',
    'SELECT DISTINCT payer AS id1, payee AS id2, 1 AS w FROM r_txs WHERE payer>0 AND payee>0',
    'SELECT DISTINCT main AS id1, other AS id2, 1 AS w FROM r_relations',
    'SELECT DISTINCT u1.uid AS id1, u2.uid AS id2, 0.5 AS w FROM users u1 INNER JOIN users u2 ON (u2.city=u1.city OR u2.postalCode=u1.postalCode) WHERE u1.uid<>u2.uid',
  ]); // each type of relationship counts as one point
  $sql = <<<EOF
    INSERT INTO r_near
    SELECT LEAST(q.id1, q.id2) AS uid1, GREATEST(q.id1, q.id2) AS uid2, SUM(w) AS weight 
    FROM ($queries) q 
    LEFT JOIN users u ON u.uid=q.id1
    LEFT JOIN users v ON v.uid=q.id2
    WHERE NOT u.:IS_CO AND NOT v.:IS_CO
    GROUP BY LEAST(q.id1, q.id2), GREATEST(q.id1, q.id2)
EOF;
  db\q($sql);
  queueNext();
}

/**
 * Update individual account stats
 */
function acctStats() {
  db\q('UPDATE users SET picture=0'); // clear temp field
  queueEach('acctStats1', 'SELECT uid FROM users WHERE uid>1 AND :IS_OK');
  queueNext();
}

/**
 * Update stats for one account (joint accounts are aggregated in acct::j_stats(), not here)
 * stats will be an assoc: [avgBalPastMo, avgBalEver, minPastMo, etc.]
 * picture (an otherwise unused field) will be minPastMo in cents (ready for r\stats to SUM)
 */
function acctStats1($item, $asof = REQUEST_TIME) {
  extract(u\just('uid asof', $item));
  $a = r\acct($uid);
  $asof = strtotime('today', $asof);
  $monthAgo = u\plusMonths(-1, $asof);
  $avgBalPastMo = r\averageBalance($uid, $monthAgo, $asof, TRUE, $minPastMo);
  $avgBalEver = r\averageBalance($uid, strtotime('today', $a->created), $asof, TRUE);
  $subs = compact('uid', 'monthAgo', 'asof');
  
  $sql = <<<EOF
    SELECT SUM(amount) AS giftsEver, SUM(IF(completed<:monthAgo, 0, amount)) AS giftsPastMo
    FROM r_gifts WHERE uid=:uid AND completed AND completed<:asof
EOF;
  $row1 = db\q($sql, $subs)->fetchAssoc() ?: []; // giftsEver, giftsPastMo
  
  $sql = <<<EOF
    SELECT SUM(IF(type IN (:TX_REBATE,:TX_BONUS), amount, 0)) AS beneEver,
    SUM(IF(type IN (:TX_REBATE,:TX_BONUS) AND created>=:monthAgo, amount, 0)) AS benePastMo,
    SUM(IF(type IN (:TX_SIGNUP,:TX_INFLATION,:TX_GRANT), amount, 0)) AS extraEver,
    SUM(IF(type IN (:TX_SIGNUP,:TX_INFLATION,:TX_GRANT) AND created>=:monthAgo, amount, 0)) AS extraPastMo
    FROM r_txs WHERE payee=:uid AND created<:asof
EOF;
  $row2 = db\q($sql, $subs)->fetchAssoc() ?: []; // get beneEver, benePastMo
  
  $sql = <<<EOF
    SELECT SUM(amount) AS bankedEver, SUM(IF(completed<:monthAgo, 0, amount)) AS bankedPastMo
    FROM r_usd WHERE payer=:uid AND completed AND completed<:asof
EOF;
  $row3 = db\q($sql, $subs)->fetchAssoc() ?: []; // get bankedEver, bankedPastMo
  $newData = $row1 + $row2 + $row3 + compact(u\ray('avgBalPastMo avgBalEver minPastMo'));
  foreach ($newData as $k => $v) $newData[$k] = '' . round($v, 2); // avoid long decimals
  $a->update('stats picture', serialize($newData), $minPastMo * 100);
}

/**
 * Update system-wide and community-wide stats
 */
function cttyStats() {
  r\getStats(0); // for all our communities at once
  queueEach('cttyStats1', 'SELECT DISTINCT community AS ctty FROM users WHERE community IS NOT NULL');
  queueNext();
}

function cttyStats1($item) {
  extract(u\just('ctty', $item));
  r\getStats($ctty); // record stats for one community on the regional server

  $companies = \variable_get('companies', []);
  $companies[$ctty] = cron\companies($ctty);
  \variable_set('companies', $companies);
}

/**
 * Calculate how much each member is trusted by the others.
 */
function trust() {
  db\q('UPDATE users SET trust=IF(:IS_CO,NULL,1)');
  $sql = <<<EOF
    UPDATE users u 
    INNER JOIN
      (SELECT proxy AS uid,SUM(straw) AS bale
       FROM 
         (SELECT proxy,IFNULL(up.trust,0)/p.priority AS straw
          FROM r_proxies p 
          LEFT JOIN users up ON up.uid=p.person) s
       GROUP BY proxy) b
    ON b.uid=u.uid
    SET u.trust=u.trust+:weight*b.bale WHERE NOT :IS_CO
EOF;
  for ($i = 0, $weight = 1; $i < 32; $i++, $weight /= 2) db\q($sql, compact('weight'));
  if (u\test()) db\q('UPDATE users SET trust=ROUND(trust, 2)'); // make the results easily testable
  queueNext();
}

/**
 * Calculate risks of financial misbehavior for each account.
 * Do companies first, so their risk can be used to help assess individual risk.
 * Do the whole thing twice, to take into account relations with suspicious individuals.
 */
function acctRisk() {
  db\q('UPDATE users SET risk=NULL'); // avoid unnecessary badConx. don't touch risks though!
  for ($i=0; $i<2; $i++) { // do it twice (for good and bad Conx)
    queueEach('acctRisk1', 'SELECT uid FROM users WHERE uid>1 and :IS_OK ORDER BY :IS_CO DESC');
  }
  queueNext();
}

function acctRisk1($item) {
  include_once __DIR__ . '/../rcredits-risk.inc';
  k\acctRisk1($item['uid']);
}

/**
 * Calculate risks of financial misbehavior for each recent transaction.
 * Include any substantial payments from the community (over $100).
 */
function txRisk() {
  queueEach('txRisk1', 'SELECT * FROM r_txs WHERE risk IS NULL AND ABS(amount)>0 AND (payer>0 OR ABS(amount)>:K_CTTY_THRESHOLD) ORDER BY created');
  queueNext();
}

function txRisk1($item) {
  include_once __DIR__ . '/../rcredits-risk.inc';
  k\txRisk1($item);
}

/**
 * Calculate risks of financial misbehavior for each recent ACH.
 */
function achRisk() {
  queueEach('achRisk1', 'SELECT * FROM r_usd WHERE risk IS NULL ORDER BY created');
  queueNext();
}

function achRisk1($item) {
  include_once __DIR__ . '/../rcredits-risk.inc';
  k\achRisk1($item);
}

/**
 * Handle one-time and recurring gifts that it's time to handle.
 */
function gifts() {
  queueEach('gift1', 'SELECT g.* FROM r_gifts g INNER JOIN users u ON u.uid=g.uid WHERE NOT completed and giftDate<=:REQUEST_TIME AND :IS_OK');
  queueNext();
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
 *//*
function homogenize() {
  $result = db\q('SELECT AVG(usd) AS avg, community FROM users WHERE uid>1 AND :IS_OK GROUP BY community');
  while ($row = $result->fetchAssoc()) {
    extract($row);
    queueEach('homogenize1', 'SELECT uid, usd, community AS ctty FROM users WHERE community=:community AND uid>1 AND usd>:avg AND :HAS_BANK AND :IS_OK', compact('avg', 'community'));
  }
  queueNext();
}
*/

/**
 * Offload some USD from an account, to others with less.
 * Avoid lending an account USD beyond its credit limit (keep r >= floor).
 *//*
function homogenize1($item) {
  extract(u\just('uid usd ctty', $item));
  $a = r\acct($uid);
  $result = db\q('SELECT uid AS uid2 FROM users WHERE community=:ctty AND usd<:usd-:R_CHUNK AND uid>1 AND :IS_OK AND :HAS_BANK AND r>=floor+:R_CHUNK ORDER BY usd', compact('ctty', 'usd'));
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $a2 = r\acct($uid2); // use the latest variables, because tradeR might have changed the values
    if ($a2->usd < $a->usd - R_CHUNK and $a2->r >= $a2->floor + R_CHUNK) r\tradeR(R_CHUNK, $uid2, $uid);
    // This trade might fail (gently) if a bank transfer goes through in between
  }
}
*/

/**
 * Hold some of the community's USD in the community account.
 * Leave each contributing account with at least R_CHUNK so it can help someone cash out the next day (NOPE -- not doing that anymore).
 *//*
function pool() {
  $result = db\q('SELECT SUM(usd) AS total, SUM(1) AS sources, community FROM users GROUP BY community');
  while ($row = $result->fetchAssoc()) { // for each community
    extract($row); // total, sources, community
    $ctty = r\acct($community);
    if (!$ctty->usdAccount) continue; // not-yet-active region or community account
//    $sql = 'SELECT uid, usd, community FROM users WHERE uid>1 AND :IS_OK AND community=:community AND usd>=2*:R_CHUNK ORDER BY usd DESC';
    $sql = 'SELECT uid, usd, community FROM users WHERE uid>1 AND :HAS_BANK AND :IS_OK AND :community=:community AND usd>0 ORDER BY usd DESC'; // just one community for now
    //if ($ctty->usd / $total < R_CTTY_USD_SHARE) 
    queueEach('pool1', $sql, compact('community')); // okay if ctty has a bit too much
  }
  queueNext();
}
*/

/**
 * Trade $10us to the community, leaving the account with at least $10us (if possible).
 *//*
function pool1($item) {
  extract(u\just('uid usd community', $item));
//  if ($usd >= R_CHUNK) r\tradeR(R_CHUNK, $community, $uid); // might fail (gently) if a bank transfer goes through in between
  $community = r\serverUid(); // just one community for now
  if ($usd > 0) r\tradeR(min($usd, R_CHUNK), $community, $uid); // might fail (gently) if a bank transfer goes through in between
}
*/

/**
 * 1. Mark deposited transfers complete R_USDTX_DAYS business days after deposit (if not already marked complete)
 * 2. Transfer funds from member bank accounts to rCredits or vice versa, according to account's settings
 * 3. Transfer funds from a connected account, if appropriate.
 */
function bank() {
//  $excess = 'LEAST(usd, :R_EXCESS)';
  $weekday = strftime('%w'); // 0-6 Sun-Sat (use %w because %u fails on Windows and maybe linux)
  if (u\test() or in_array($weekday, [2, 3, 4, 5])) { // no completions on Saturday, Sunday, or Monday
    $sql = 'SELECT * FROM r_usd WHERE amount<0 AND NOT completed AND :REQUEST_TIME>=created+:R_USDTX_DAYS*:DAY_SECS'; // this allows us to wait arbitrarily long to deposit chks
//    $sql = 'SELECT * FROM r_usd WHERE amount<0 AND NOT completed AND deposit AND :REQUEST_TIME>=deposit+:R_USDTX_DAYS*:DAY_SECS'; // this depends on deposits almost every day
    queueEach('completeUsdTx', $sql);
  }
  
  $sql = <<<EOF
  SELECT uid
  FROM users 
  WHERE minimum>0 AND r<minimum AND uid>1 AND :IS_OK AND :HAS_BANK
EOF;
  queueEach('bank1', $sql); // Note: joint slave r values cannot be negative unless both are (see r\cache)
  
  $sql = <<<EOF
  SELECT uid, r.main as ma
  FROM users u INNER JOIN r_relations r ON r.other=u.uid
  WHERE minimum>0 AND r<minimum AND uid>1 AND :IS_OK AND NOT :HAS_BANK AND r.draw 
  AND r.permission<>:B_JOINT-:B_RELATED
EOF;
  queueEach('draw1', $sql);
  
  
  queueNext();
}

function completeUsdTx($item) {
  extract(u\just('txid payer amount created', $item));
  r\acct($payer)->completeUsdTx($txid, -$amount, $created);
}

function bank1($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  if ($a->j_r >= $a->j_minimum) return; // joint account balance is sufficient
  if (!$amount = getHowMuchR($a)) return; // check shortfall here NOT in query, balance may change
  $action = 'draw from';
  if (!$checkNum = $a->bank(-$amount)) return; // may never happen, but if mysql is unavailable just redo next day
  $amount = u\fmtAmt($amount);
  r\notify($uid, 'under min|banked|bank tx number', compact(u\ray('action amount checkNum')));
}

function draw1($item) {
  extract(u\just('uid ma', $item)); // ma is UNUSED
  $a = r\acct($uid);
  if ($a->jid) return; // don't draw within a joint account
  if (!$amount = getHowMuchR($a, FALSE)) return;

  if ($a->draw($amount)) {
    $amount = u\fmtAmt($amount);
    r\notify($uid, 'under min|drew', compact('amount'));
  } else r\notify($uid, 'under min|cannot draw');
}

/**
 * Given an account's shortfall, return how much to actually transfer from wherever to the rCredits account.
 * @param acct $a: which account
 * @param bool $banky: the account has a connected bank account
 * @return: the amount to transfer from the connected bank account or drawn for other rCredits accts (0 if none).
 */
function getHowMuchR($a, $banky = TRUE) {
  if (!$need = max(0, $a->minimum - $a->j_r)) return 0;
  if ($banky) {
    $waiting = -$a->waitingToBank(); // amount already requested (if any)
    $need = round($need - $waiting, 2); // avoid a re-request
  }
  return $need <=0 ? 0 : max($a->achMin, round(R_ACHMIN * ceil($need / R_ACHMIN)), 2);
}

/**
 * Give signup bonuses.
 */
function bona() {
  $sql = 'SELECT DISTINCT u.uid FROM users u LEFT JOIN r_txs t ON t.payer=u.uid WHERE NOT :IS_BONA AND NOT :IS_CO AND t.taking'; // new human member who has completed an rCard purchase
  queueEach('bona1', $sql);
  queueNext();
}

function bona1($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  
  $DBTX = \db_transaction();
  r\membershipEvent($a, 'bona');
  be\fund($a->id, TX_SIGNUP, R_SIGNUP_BONUS, t('signup bonus'));
  
  if (!r\acct($a->helper)->cAdmin) { // helper is not admin
    $fullName = $a->fullName;
    be\fund($a->helper, TX_HELPER, R_HELPER_BONUS, u\tt('invite reward', compact('fullName')));
  }
  unset($DBTX);
}
  
/**
 * For each new employee, reward the inviter of their company (if nobody has been rewarded for that employee yet).
 */
function employees() {
  $sql = 'SELECT u.uid, r.main AS company FROM r_relations r INNER JOIN users u ON u.uid=r.other WHERE r.employee AND :IS_BONA AND NOT :IS_COUNTED';
  queueEach('employee', $sql);
  queueNext();
}

function employee($item) {
  extract(u\just('uid company', $item));
  $co = r\acct($company);
  if ($coHelper = $co->helper and $helper = r\acct($coHelper)->helper) { // if is for tests (handles no helper)
    $h = r\acct($helper);
    if ($helper != $uid and !$h->cAdmin) {
      $fullName = $co->fullName;
      be\fund($helper, TX_HELPER, R_COUNTED_BONUS, u\tt('employee invite reward', compact('fullName')));
      r\acct($uid)->setBit(B_COUNTED);
    }
  }
}
  
/**
 * Send daily, weekly, or monthly message digests. (Newbies are always daily)
 */
function notices() {
  global $timeFor;
  $weekly = (strftime('%A') == R_WEEKLY_DAY or @$timeFor['weekly']);
  $monthly = (strftime('%d') == R_MONTHLY_DAY or @$timeFor['monthly']);

  $where = $dailySql = '((NOT :IS_WEEKLY AND NOT :IS_MONTHLY) OR NOT :IS_MEMBER)'; // daily
  if ($weekly) $where .= ' OR :IS_WEEKLY';
  if ($monthly) $where .= ' OR :IS_MONTHLY';
  $sql = "SELECT DISTINCT u.uid, :IS_WEEKLY AS weekly, :IS_MONTHLY AS monthly FROM r_notices n JOIN users u ON u.uid=n.uid WHERE NOT sent AND ($where)";
  queueEach('notice', $sql);
  queueNext();
}

function notice($item) {
  extract(u\just('uid daily weekly monthly', $item));

  if ($args = r\noticeArgs($uid, 'NOT sent', $weekly, $monthly)) {
    r\notify($uid, 'notices', $args, TRUE);
    db\q('UPDATE r_notices SET sent=:REQUEST_TIME WHERE uid=:uid AND NOT sent', compact('uid'));
  }
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  $lastCron = \variable_get('r_last_cron', []);
  $elapsed = time() - @$lastCron['day'];
///  echo "Daily cron done after $elapsed seconds"; // keep
  r\tellStaff("At " . strftime('%I:%M%P') . ", after $elapsed secs, daily cron ended successfully.", '', FALSE);
}

/**
 * Run the next daily function.
 */
function queueNext($task = NULL) {
  global $cronOp; 
  if ($cronOp == ONE_CRON_OP) return; // doing just one, so do nothing more

  u\setDft($task, str_replace(__NAMESPACE__ . '\\', '', u\caller()));

  if (u\test() and @$cronOp and $cronOp != 'ALL') {
    r\queue($task == $cronOp ? 'END' : $cronOp);
  } else r\queue(u\nextAfter($task, u\ray(R_DAILY_TASKS)));
//  } else r\queue($tasks[array_search($task, $tasks) + 1]);
}

function queueEach($op, $sql, $subs = []) {
  global $cronOp; // this is set to ONE_CRON_OP when calling cron() from PHP window on test server
  $result = db\q($sql, $subs);
  while ($row = $result->fetchAssoc()) {
    $oneOp = "rCredits\\Cron\\$op";
    if (@$cronOp == ONE_CRON_OP) $oneOp($row); else r\queue($op, $row);
  }
}

//function realId($uid) {return ($uid <= r\cgfId());}
