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
 
 To run a single CRON function "zot" from the PHP window, use:
 f('cr.zot', arg1,...);
 
 To run all the daily functions starting with a particular function "zot", use:
 rCredits\Cron\zot(arg1,...); // to run all daily functions, zot=everyDay
 
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
      $email = $mya->email;
      $invited = ($nonperson or db\count('r_invites', 'inviter=:myid AND email<>:email', compact('myid', 'email')) >= 1);
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

define('R_DAILY_TASKS', 'START recache checkAccts requests setNear acctStats cttyStats trust acctRisk txRisk achRisk tickle lessOften gifts bank bona employees notices END'); // had homogenize, pool 
//define ('R_DAILY_TASKS', 'START redistribute END');
include_once __DIR__ . '/rcron-subs.inc';

/**
 * Do a queued task
 * @param assoc $item: task arguments, including 'op' -- the function to handle the task
 */
function doTask($item) {
  global $channel; $channel = TX_CRON;
  $func = array_shift($item);
  $op = 'rCredits\\Cron\\' . $func;
// (too often)  u\log(t('starting cron.') . $func, 'state', $item);
  $op($item);
}

/**
 *  Do this stuff every time cron is called (normally every 5 minutes)
 *  Check to see if it's time for a less-often periodic activity (and if so do it).
 */
function periodic() {
  if ($row = db\q('SELECT * FROM queue WHERE expire<>0 LIMIT 1')->fetchAssoc()) {
    r\tellAdmin('task timed out', $row); // shouldn't happen. Drupal's queue processing hangs until we delete.
    db\q('DELETE FROM queue WHERE item_id=:item_id', u\just('item_id', $row));
  }
  often('hour', '00');
  often('day', R_DAILY_HOUR); // longer periods get queued in lessOften(), so notices are last
//  $lastDaily = \variable_get('daily', 0);
//  if (strftime('%H') >= R_DAILY_HOUR and strftime('%d') != strftime('%d', $lastDaily)) everyDay();
//  if (strftime('%M') == '00') everyHour();
}

function everyHour() {
  // stuff to do every hour
  queueEach('acctHourly', "SELECT uid FROM users WHERE uid<>0");
  if (strftime('%H') + 0 == (R_DAILY_HOUR + 6) % 24) {
    $lastCron = \variable_get('r_last_cron', []);
    $lastTime = @$lastCron['day'];
    if (r\rTime() - $lastTime > DAY_SECS) r\tellAdmin('daily cron FAILED');
  }
  // r\tellAdmin('hourly cron');
}

/**
 * Account checks to do every hour.
 */
function acctHourly($item) {
  return; // nothing to do, currently
  extract(u\just('uid', $item));
  $a = r\acct($uid);
//  $a->shortfall(FOR_GOODS, PHP_INT_MAX); // notice recent deposits, if any
}

function everyDay() {
  queueNext('START');
  // miscellaneous quick stuff to do every day
  $time = r\rTime() - 7 * DAY_SECS;
  db\q('DELETE FROM r_log WHERE time<:time', compact('time')); // zap log except for the past few days
  db\q('DELETE FROM r_do WHERE completed<0 AND :now>-completed', ['now'=>r\rTime()]); // completed<0 is -expiry time

  $cttys = db\q('SELECT uid AS ctty FROM users WHERE uid<0')->fetchCol();
  foreach ($cttys as $ctty) {
    $companies = \variable_get('companies', []);
    $companies[$ctty] = r\companies($ctty);
    \variable_set('companies', $companies);
  }  
}

/**
 * Do things a certain number of days after the created, signed, invite, firstTx, last login, or last access date.
 */
function tickle() {
//  $delays = '1,2,3,5,8,13,21,34,55,89,144'; // Fibonacci progression of delays
  $today = strtotime('today');
  $timeSince = "(FLOOR(($today-@DATE)/:DAY_SECS))"; // $today works better in tests than :REQUEST_TIME or time()

  // Remind invitee to accept
  $delays = '8';
  $elapsed = str_replace('@DATE', 'invited', $timeSince);
  $sql = <<<EOF
    SELECT email, inviter, code, $elapsed AS elapsed FROM r_invites
    WHERE $elapsed IN ($delays) AND invitee=0
EOF;

  queueEach('tickleSignup', $sql);
/*    SELECT i.email, inviter, code, $elapsed AS elapsed FROM r_invites i
    LEFT JOIN users u ON u.email=i.email
    WHERE $elapsed IN ($delays) AND u.uid IS NULL AND invitee=0 */
    
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
  $start = strtotime('-6 months', strtotime('today', r\rTime()));
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
  extract(u\just('email inviter code elapsed', $item));
  if (r\acct($email = u\decryptN($email))) return; // invitee already began to open an account
  $inviterName = r\acct($inviter)->fullName;
//  $count = $elapsed < 7 ? t('two') : ($elapsed < 14 ? t('one') : 0);
//  $nudge = $count ? t("We will send you just @count more reminder(s)", compact('count')) : t('This is your last reminder');
  $nudge = t($elapsed < 7 ? 'reminder coming' : 'reminder one');
  r\rMail('nonmember', $email, ray('fullName nudge code noFrame', $inviterName, $nudge, $code, TRUE));
  r\notify($inviter, 'invite languishing', compact('email', 'elapsed'));
}

/**
 * Remind newb to take another step.
 */
function tickleStep($item) {
  extract(u\just('uid elapsed', $item));
  $a = r\acct($uid);
  if (!$a->member and $step = $a->nextStep()) {
//    $msg = $step == 'verify' ? 'call bank' : (($step == 'sign' or $step == 'contact') ? 'do step one' : 'take another step');
    $msg = ($step == 'verify' or $step == 'contact') ? 'do step one' : 'take another step';
/*    $when = $elapsed < 3 ? 
      (u\nextBusDay(+1) . t(' (how about 10am?)'))
    : (u\nextBusDay(0) . t(' between 9am and 4pm')); // for verify
    */
//    if ($step != 'verify' or $elapsed > 1) r\notify($a->id, "$msg|sign in", compact('when'));
    r\notify($uid, "$msg|sign in");
    if ($elapsed >= 8) {
      $q = db\q('SELECT DISTINCT inviter FROM r_invites WHERE invitee=:uid', compact('uid'));
      while ($row = $q->fetchAssoc()) {
        extract($row);
        r\notify($inviter, 'invitee slow', ray('fullName elapsed step', $a->fullName, $elapsed, $step));
      }
    }
  } elseif (!$a->ok) {
    return r\tellAdmin('activate account', compact('uid'));
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
// DISABLED FOR NOW    r\notify($uid, $msg, ['limit' => u\fmtAmt($monthly)]);
  }
}
 
/**
 * Do periodic activities where the period is longer than a day.
 */
function lessOften() {
  often('week', R_WEEKLY_DAY);
  often('month', R_MONTHLY_DAY);
  often('year', '01');  
  queueNext();
}

/**
 * Do something periodically if it's time and hasn't been done yet.
 * That is, if the chosen point in the period has been reached AND
 *   (it has been at least a whole period since the last run OR
 *   (it has been at least half a period and we're at no later a point than last time)
 * @param string $period: the period
 * @param string $code: the strftime code for the smaller period to match or exceed
 * @param string $chosen: the chosen value to match or exceed
 */
function often($period, $chosen) {
  if (!timeFor(r\rTime(), $period, $chosen)) return;

  $lastCron = \variable_get('r_last_cron');
  $lastCron[$period] = r\rTime();
  \variable_set('r_last_cron', $lastCron); // remember that this periodic activity has been done
  $function = 'rCredits\\Cron\\every' . ucwords($period);
  if ($period != 'hour' and $period != 'day') r\tellAdmin("Running cron for the $period", ['time' => strftime('%I:%M%P', r\rTime())]);
  $function(); // do it
}

/**
 * Do weekly tasks.
 */
function everyWeek() {
  // help members increase their overall savings (or crawl out of primary account debt)
  $refillable = ':IS_OK AND :HAS_BANK AND :IS_REFILL';
  $newMin = "(minimum+GREATEST(saveWeekly, :MIN_WEEKLY_BUMP))"; // crawl out of debt by at least $1 a week
  db\q("UPDATE users SET minimum=IF(ABS($newMin)<.01, .01, $newMin) WHERE $refillable AND minimum<GREATEST(0, floor)"); // crawl out of debt
  db\q("UPDATE users SET minimum=minimum+saveWeekly WHERE $refillable AND saveWeekly>0 AND minimum>=GREATEST(0, floor)"); // build up savings
//  db\q('UPDATE users SET savingsAdd=GREATEST(0, savingsAdd+saveWeekly) WHERE :IS_OK AND :HAS_BANK AND saveWeekly<>0'); // save or unsave
}

/**
 * Do monthly tasks.
 */
function everyMonth() {
  $start = u\monthDay1($end = u\monthDay1() - 1); // mark start and end of preceding month
  if (u\test()) $start = strtotime('-30 days', $end = strtotime('today') - 1) + 1; // test: the past 30 days
  queueEach('inflationAdjust', "SELECT uid, $start AS start, $end AS end FROM users WHERE :IS_OK AND uid>1");  
//  queueEach('shareCommitted', 'SELECT uid, committed, share FROM users WHERE :IS_OK AND committed>0');
  queueEach('roundup', "SELECT payer, SUM(MOD(amount, 1)) AS roundups FROM r_txs WHERE :IS_ROUNDUP AND amount>0 AND created BETWEEN $start AND $end GROUP BY payer");
}

/**
 * Yearly
 */
function everyYear() {
  r\tellAdmin('generate 1099-B forms for IRS and states');
}

/**
 * Calculate and credit inflation adjustment to each member account.
 */
function inflationAdjust($item) {
  extract(u\just('uid start end', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slave in joint account gets no inflation (master gets inflation for both)
  if (db\exists('r_txs', 'payee=:uid AND type=:TX_INFLATION AND created>=:end', compact('uid', 'end'))) return;
  list ($avg, $min, $ravg) = r\averageBalance($uid, $start, $end);
//  if ($adjustment > 0)  // don't adjust debts
  foreach ([t('average balance') => $avg, t('credit reserve') => $ravg] as $k => $v) {
    $adjustment = round($v * R_INFLATION_RATE / 12, 2);
    if ($adjustment) be\fund($uid, TX_INFLATION, $adjustment, INFLATION_DESC . t(' on your ') . $k);
  }
}

/**
 * Share committed rewards with CGF.
 *//*
function shareCommitted($item) {
  extract(u\just('uid committed share', $item));
  $share = round($share, 1);
  r\notify($uid, 'share gift', compact('share'));
  $info = ray('giftDate uid amount often honor share', r\rTime(), $uid, $committed, 1, 'share', -1);
  $DBTX = \db_transaction();
  db\insert('r_gifts', $info);
  r\acct($uid)->update('committed', 0);
  unset($DBTX);
}
*/

/**
 * Donate rounded change to the community.
 */
function roundup($item) {
  extract(u\just('payer roundups', $item));
  $a = r\acct($payer);
  be\transfer('payment', $a, $a->cttyA, $roundups, FOR_NONGOODS, t('contribution of rounded-up payment change'), NULL, TRUE, ['roundups' => TRUE]);
}

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
 * Gather requests for invitations
 */
function requests() {
  $dlm = db\max('created', 'r_request') + 0;
  $secret = urlencode(db\lookup('last', 'r_request', 'listid=3894'));

  if ($s = file_get_contents(PROMO_URL . "/inc/requests.php?dlm=$dlm&secret=$secret")) {
    foreach ($rows = explode("\n", $s) as $row) {
      $row = explode("\t", $row);
      if (@$header) {
        $row = array_combine($header, $row);
        extract($row);
        $z2 = substr($zip, 0, 2);
        $row['ctty'] = $zip ? (r\communityUid($zip, TRUE) ?: ($z2 == '05' ? r\serverUid() : 0)) : 0;
        if (!$created) $row['created'] = strtotime('9/1/2011');
        $row['phone'] = u\fmtPhone($phone, '+n');
        foreach (['first', 'last'] as $k) $row[$k] = u\normalizeCase($$k);
        r\tellAdmin('invitation request', $row);
        foreach (['email', 'phone'] as $k) $row[$k] = u\cryptN($row[$k]);
        db\update('r_request', $row, 'listid', TRUE);
      } else $header = $row + ['ctty'];
    }
  }

  $sql = <<<EOF
  UPDATE r_request r LEFT JOIN users u 
  ON (
    (r.email<>'' AND u.mail=r.email) OR
    (r.phone<>'' AND u.phone=r.phone) OR
    (INSTR(u.fullName, r.first)>0 AND INSTR(u.fullName, r.last)>0 AND MID(u.postalCode, 1, 5)=MID(r.zip, 1, 5))
  )
  LEFT JOIN r_invites AS i ON (i.email=r.email AND i.invited>r.created)
  SET done=1 WHERE NOT done AND (u.uid IS NOT NULL OR i.id IS NOT NULL)
EOF;
  db\q($sql);
  
  queueNext();
}

/**
 * Recalculate and update cached financial balances for each account.
 * Also give signup bonus, if appropriate.
 */
function recache() {
  $fields = 'uid, r, rewards, minimum, floor';
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
 * Update stats for one account (joint accounts are aggregated in r\Acct::j_stats(), not here)
 * stats will be an assoc: [avgBalPastMo, avgBalEver, minPastMo, etc.]
 * picture (an otherwise unused field) will be minPastMo in cents (ready for r\stats to SUM)
 */
function acctStats1($item, $asof = NULL) {
  extract(u\just('uid', $item));
  u\setDft($asof, r\rTime());
  $a = r\acct($uid);
  $asof = strtotime('today', $asof);
  $monthAgo = u\plusMonths(-1, $asof);
  list ($avgBalPastMo, $minPastMo) = r\averageBalance($uid, $monthAgo, $asof, TRUE);
  list ($avgBalEver) = r\averageBalance($uid, strtotime('today', $a->created), $asof, TRUE);
///  debug(compact('avgBalEver','uid','asof') + ['created' => $a->created]);
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
  $newData = $row1 + $row2 + $row3 + compact(ray('avgBalPastMo avgBalEver minPastMo'));
  foreach ($newData as $k => $v) $newData[$k] = '' . round($v, 2); // avoid long decimals
  $a->update('stats picture', serialize($newData), $minPastMo * 100);
}

/**
 * Update system-wide and community-wide stats
 */
function cttyStats() {
  if (db\exists('r_stats')) {
    r\getStats(0); // for all our communities at once
    queueEach('cttyStats1', 'SELECT DISTINCT uid AS ctty FROM users WHERE uid<0'); // and one at a time
  } else { // recreate stats data from the beginning of time
    $grain = isDEV ? 20 : 1; // HAVE data for every day, not just every STATS_GRAIN days
    $blockAgo = strtotime(-$grain, strtotime('today'));
  
    $sql = <<<EOF
      SELECT community AS ctty, MIN(t.created) AS start
      FROM users u LEFT JOIN r_txs t ON t.payee=u.uid 
      WHERE uid>1 AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
EOF;
    $cttys = db\q($sql)->fetchAllKeyed();
    u\preray([0 => min($cttys)], $cttys); // show all dates overall

    $now = r\rTime();
    $start0 = @$cttys[0] ?: $now;
    $created = strtotime('today', $start0); // use first moment of the day
    
    while ($created <= $now) {
      foreach ($cttys as $ctty => $start) if ($created >= $start) {
        cr\queue('cttyStats1', compact('ctty', 'created'));
      }
      $delta = $created < $blockAgo ? $grain : 1;
      $created = strtotime("+$delta days", $created);
    }
  }

  queueNext();
}

function cttyStats1($item) {
  extract(u\just('ctty created', $item));
  r\getStats($ctty, @$created); // record stats for one community on the regional server, for the given date
  // this fails on cross-community txs!  if ($ctty != 0) if ($err = badStats($ctty, $created)) return 
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
  $now = r\rTime();
  queueEach('gift1', 'SELECT g.* FROM r_gifts g INNER JOIN users u ON u.uid=g.uid WHERE NOT completed and giftDate<=:now AND :IS_OK', compact('now'));
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
 * 1. Mark deposited transfers complete R_USDTX_DAYS business days after deposit (if not already marked complete)
 * 2. Transfer funds from member bank accounts to rCredits or vice versa, according to account's settings
 * 3. Transfer funds from a connected account, if appropriate.
 */
function bank() {
//  $excess = 'LEAST(usd, :R_EXCESS)';
  $weekday = strftime('%w'); // 0-6 Sun-Sat (use %w because %u fails on Windows and maybe linux)
  if (u\test() or in_array($weekday, [2, 3, 4, 5])) { // no completions on Saturday, Sunday, or Monday
    $now = r\rTime();
    $sql = 'SELECT * FROM r_usd WHERE amount<0 AND NOT completed AND (amount>=-:R_USDTX_QUICK OR :now>=created+:R_USDTX_DAYS*:DAY_SECS)'; // this allows us to wait arbitrarily long to deposit chks
//    $sql = 'SELECT * FROM r_usd WHERE amount<0 AND NOT completed AND deposit AND :REQUEST_TIME>=deposit+:R_USDTX_DAYS*:DAY_SECS'; // this depends on deposits almost every day
    queueEach('completeUsdTx', $sql, compact('now'));
  }
  
//  $where = ':IS_REFILL AND (:IS_JOINED OR r-rewards-savingsAdd<minimum) AND uid>1 AND :IS_OK';
  $where = ':IS_REFILL AND (:IS_JOINED OR r-rewards<minimum) AND uid>1 AND :IS_OK';
  $sql = <<<EOF
  SELECT uid
  FROM users 
  WHERE $where AND :HAS_BANK
EOF;

  queueEach('bank1', $sql); // Note: joint slave r values cannot be negative unless both are (see r\cache)
  
  $sql = <<<EOF
  SELECT uid, r.main as ma
  FROM users u INNER JOIN r_relations r ON r.other=u.uid
  WHERE $where AND NOT :HAS_BANK AND r.draw 
  AND r.permission<>:B_JOINT-:B_RELATED
EOF;
  queueEach('draw1', $sql);

  queueNext();
}

function completeUsdTx($item) {
  extract(u\just('txid payer amount channel', $item));
  $realAmt = db\lookup('amount', 'r_usd', 'txid=:txid', compact('txid'));
  if ($amount != $realAmt) r\tellAdmin('bad completeUsdTx amount in cron', $item + compact('realAmt'));
  r\acct($payer)->completeUsdTx($txid, -$amount, @$channel == TX_CRON);
}

function bank1($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slaves don't have automatic refills
  if (!$amount = getHowMuchR($a)) return; // check shortfall here NOT in query, balance may change
  $action = 'draw from';
  if (!$checkNum = $a->bank(-$amount)) return; // may never happen, but if mysql is unavailable just redo next day
  $amount = u\fmtAmt($amount);
  r\notify($uid, 'under min|banked|bank tx number', compact(ray('action amount checkNum')));
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
 * Given an account's shortfall, return how much to actually transfer from wherever to the %PROJECT account.
 * @param acct $a: which account
 * @param bool $banky: the account has a connected bank account
 * @return: amount to transfer from the connected bank account or draw from other %PROJECT accounts (0 if none).
 */
function getHowMuchR($a, $banky = TRUE) {
  if (!$need = max(0, $a->minimum + $a->committed - $a->balance)) return 0;
  if ($banky) {
    $waiting = -$a->waitingToBank($combinable); // amount already requested (if any)
    $need = round($need - $waiting, 2); // avoid a re-request
  }
  return ($need <= 0 ? 0 : max(@$combinable + 0 < 0 ? 0 : $a->achMin, round(R_ACHMIN * ceil($need / R_ACHMIN))));
}

/**
 * Give signup bonuses.
 * NOTE: Henceforth do this when activating account, instead.
 * But keep this function active until all legacy accounts have been given a signup bonus
 */
function bona() {
//  $sql = 'SELECT DISTINCT u.uid FROM users u LEFT JOIN r_txs t ON t.payer=u.uid WHERE NOT :IS_BONA AND NOT :IS_CO AND t.taking'; // new human member who has completed an rCard purchase
  $sql = 'SELECT DISTINCT u.uid FROM users u WHERE :IS_OK AND NOT :IS_BONA AND NOT :IS_CO'; // new human member
  queueEach('bona1', $sql);
  queueNext();
}

function bona1($item) {
  extract(u\just('uid', $item));
  $a = r\acct($uid);
  r\membershipEvent($a, 'bona');
}

/**
 * For each new employee, reward the inviter of their company (if nobody has been rewarded for that employee yet).
 */
function employees() {
  $sql = 'SELECT u.uid, r.main AS company FROM r_relations r INNER JOIN users u ON u.uid=r.other WHERE r.employee AND :IS_BONA AND NOT :IS_COUNTED';
// DISCONTINUED  queueEach('employee', $sql);
  queueNext();
}

/*
function employee($item) {
  extract(u\just('uid company', $item));
  $co = r\acct($company);
  if ($coHelper = $co->helper and $helper = r\acct($coHelper)->helper) { // if is for tests (handles no helper)
    $h = r\acct($helper);
    if ($helper != $uid and !$h->cAdmin) {
      $fullName = $co->fullName;
      be\fund($helper, TX_HELPER, R_COUNTED_BONUS, t('employee invite reward', compact('fullName')));
      r\acct($uid)->setBit(B_COUNTED);
    }
  }
}
*/
  
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

  $now = r\rTime();
  if ($args = r\noticeArgs($uid, 'NOT sent', $weekly, $monthly)) {
    r\notify($uid, 'notices', $args, TRUE);
    db\q('UPDATE r_notices SET sent=:now WHERE uid=:uid AND NOT sent', compact('uid', 'now'));
  }
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  $lastCron = \variable_get('r_last_cron', []);
  $elapsed = r\rTime() - @$lastCron['day'];
///  echo "Daily cron done after $elapsed seconds"; // keep
  r\tellAdmin("At " . strftime('%I:%M%P') . ", after $elapsed secs, daily cron ended successfully.");
}


//function realId($uid) {return ($uid <= r\cgfId());}
