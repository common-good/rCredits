<?php
namespace CG\Cron;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Risk as k;
use CG\Cron as cr;

/**
 * @file
 * Functions to run periodically.
 
 To run a single CRON function "zot" from the PHP window, use:
 f('cr.zot', arg1,...);
 
 To run all the daily functions starting with a particular function "zot", use:
 CG\Cron\zot(arg1,...); // to run all daily functions, zot=everyDay
 
 * = written
 ** = tested

TODO:
- identify pending joint accounts and alert both parties (with link to complete or cancel) select reid, u1.fullName, u2.fullName FROM r_relations r LEFT JOIN users u1 ON r.main=u1.uid LEFT JOIN users u2 ON u2.uid=r.other WHERE (SELECT reid FROM r_relations WHERE main=r.other AND other=r.main) IS NULL AND r.permission=7 


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
  <star>/5 * * * * curl https://new2.CommonGood.earth/cron.php?cron_key=w8x3lfzNQK1bxMz6RMTuF54mMInAs4b8SiVJO9zvWYg
  where <star> means * (not the same as for the production site) 
 */

define('R_DAILY_TASKS', 'START recache checkAccts requests setNear acctStats cttyStats trust acctRisk txRisk achRisk tickle lessOften gifts bank invoices invest notices END'); // had homogenize, pool 
//define ('R_DAILY_TASKS', 'START redistribute END');
include_once __DIR__ . '/rcron-subs.inc';

/**
 * Execute a cron run when called.
 */
function run() {
  global $channel; $channel = TX_CRON; // set this even if called from PHP window by admin
// NO (happens too often)  u\log('running cron');
  // Allow execution to continue even if the request gets canceled.
  @ignore_user_abort(TRUE);

  // Prevent session information from being saved while cron is running.
  $original_session_saving = \drupal_save_session();
  \drupal_save_session(FALSE);

  // Force the current user to anonymous to ensure consistent permissions on cron runs.
  $original_user = $GLOBALS['user'];
  $GLOBALS['user'] = \drupal_anonymous_user();

  \drupal_set_time_limit(4 * 60);

  if (\lock_acquire('cron', 240.0)) {
    cr\periodic();
    \lock_release('cron');
  } else u\log(t('Attempted to run cron while it was already running.'));

  $end = time() + R_CRON_PERIOD - R_MAX_TASK_TIME;
  $sql = 'SELECT id,item FROM queue ORDER BY id';
  $q = db\q($sql);
  
  while (time() < $end and $row = $q->fetchAssoc()) {
    extract($row);
    try {
      cr\doTask(unserialize($item));
      db\q('DELETE FROM queue WHERE id=:id', compact('id'));
    } catch (Exception $e) {
      u\log('cron task failed'); // In case of exception log it and leave the item in the queue to be processed again later.
    }
  }
  // Restore the user.
  $GLOBALS['user'] = $original_user;
  \drupal_save_session($original_session_saving);
}

/**
 * Do a queued task
 * @param assoc $item: task arguments, including 'op' -- the function to handle the task
 */
function doTask($item) {
  global $channel; $channel = TX_CRON;
  global $mya;
  
  $op = array_shift($item);
  $func = 'CG\\Cron\\' . $op;

  if ($uid = @$item['uid']) $mya = r\acct($uid); // fudge current account
  if (!in($op, 'acctHourly checkAcct acctStats acctRisk') and substr($op, -1, 1) != '1') u\log($op, 'cr', justNOT('uid', $item));
    
  $func($item);
}

/**
 *  Do this stuff every time cron is called (normally every 5 minutes)
 *  Check to see if it's time for a less-often periodic activity (and if so do it).
 */
function periodic() {
  if ($row = db\q('SELECT * FROM queue WHERE created<:NOW-:DAY_SECS/2 LIMIT 1')->fetchAssoc()) {
    r\tellAdmin('task timed out', $row); // shouldn't happen. Drupal's queue processing hangs until we delete.
    db\q('DELETE FROM queue WHERE id=:id', just('id', $row));
  }

  often('hour', '00');
  often('day', R_DAILY_HOUR); // longer periods get queued in lessOften(), so notices are last

  $cttys = db\q('SELECT uid FROM users WHERE uid<0')->fetchCol();
  foreach ($cttys as $ctty) { // should never happen, but it does. figure out where.
    $a = r\acct($ctty);
    foreach ([B_UP, B_CGC] as $bit) if (!$a->can($bit)) {
/**/  r\tellAdmin(t('Community bits got unset!'), ray('ctty bit time trace', $ctty, $bit, time(), trace()));
      $a->setBit($bit);
    }
  }
}

function everyHour() { // stuff to do every hour
//  queueEach('acctHourly', "SELECT uid FROM users WHERE uid<>0");
  if (strftime('%H') + 0 == (R_DAILY_HOUR + 6) % 24) {
    $lastDaily = \variable_get('last_daily_cron', 0);
    if (NOW - $lastDaily > DAY_SECS) r\tellAdmin('daily cron FAILED');
  }
  
  db\q('DELETE FROM cache_form WHERE created<:REQUEST_TIME-:HOUR_SECS'); // clean up form cache

  // r\tellAdmin('hourly cron');
}

/**
 * Account checks to do every hour.
 */
function acctHourly($item) {
  return; // nothing to do, currently
  extract(just('uid', $item));
  $a = r\acct($uid);
//  $a->shortfall(FOR_GOODS, PHP_INT_MAX); // notice recent deposits, if any
}

/**
 * Do miscellaneous stuff that needs doing every day
 */
function everyDay() {
  queueNext('START'); // eachA seems to interfere with this and processing the queue is vital, so launch it first
  db\q('DELETE FROM r_do WHERE completed<0 AND :NOW>-completed'); // completed<0 is -expiry time

  $cttys = db\q('SELECT uid AS ctty FROM users WHERE uid<0')->fetchCol();
  foreach ($cttys as $ctty) $companies[$ctty] = r\companies($ctty, TRUE);  
  \variable_set('companies', @$companies ?: []);
  
  $sql = <<<X
  SELECT u.uid, c.uid AS ctty, u.fullName AS person, c.fullName AS cttyName FROM users u LEFT JOIN users c ON u.zip RLIKE c.zip
  WHERE u.community=:seedpack AND c.uid<1 AND c.uid<>:seedpack
X;
  $q = db\q($sql, ['seedpack' => r\serverUid()]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    db\update('users', ray('uid community', $uid, $ctty), 'uid');
    r\tellAdmin(t('%person was in Seedpack; should be (now is) in %cttyName', $row));
  }

//  $where = "RIGHT(fullName, 8)=' PARTNER' COLLATE utf8_bin AND pass='' AND NOT :IS_CO AND activated=0";
  $where = "BINARY RIGHT(fullName, 8)=' PARTNER' AND pass IS NULL AND NOT :IS_CO AND activated=0";
  eachA(function ($a) { // acts weird; do this last
    if ($jA = $a->jA and $jA->jid == $a->id) $jA->update('jid', 0);
    r\deleteAccount($a);
  } , $where);
//  db\del('users', $where); // abandoned new joint acct (must postcede setting jids)
}

/**
 * Do things a certain number of days after the created, signed, invite, firstTx, last login, or last access date.
 */
function tickle() {
//  $delays = '1,2,3,5,8,13,21,34,55,89,144'; // Fibonacci progression of delays
  $today = strtotime('today');
  $timeSince = "(ROUND(($today-@DATE)/:DAY_SECS, 0))"; // $today works better in tests than :REQUEST_TIME or time()

  // Remind invitee to accept
  $delays = '8';
  $elapsed = str_replace('@DATE', 'invited', $timeSince);
  $sql = <<<X
    SELECT email, inviter, code, $elapsed AS elapsed FROM r_invites i
    LEFT JOIN users u ON u.uid=i.inviter
    WHERE $elapsed IN ($delays) AND invitee=0 AND u.:IS_OK
X;

  queueEach('tickleSignup', $sql);
/*    SELECT i.email, inviter, code, $elapsed AS elapsed FROM r_invites i
    LEFT JOIN users u ON u.email=i.email
    WHERE $elapsed IN ($delays) AND u.uid IS NULL AND invitee=0 */
    
  // Remind newbie to take another step (or admin to activate it)
//  $delays = '1,2,3,5,8'; // Fibonacci progression of delays (followed by every week)
  $delays = '1,4,9'; // delays proposed by staff (ultimately every week)
  $elapsed = str_replace('@DATE', 'access', $timeSince);
  $sql = <<<X
    SELECT uid, $elapsed AS elapsed FROM users 
    WHERE ($elapsed IN ($delays) OR ($elapsed>8 AND $elapsed%7=0))
    AND uid>:CANONIC_ACCTS AND NOT activated AND NOT :IS_NONUDGE
X;
  queueEach('tickleStep', $sql);

  $sinceActivated = 'DATEDIFF(CURDATE(), FROM_UNIXTIME(activated))';
  // Do every 30 days after activation date
  $start = strtotime(-FLOOR_DAYS . ' days', $today);
  $sql = <<<X
    SELECT uid, GREATEST(inVol, outVol) AS monthly, $sinceActivated/30 AS months
    FROM (
      SELECT uid, u.flags, activated,
      SUM(IF(payee=uid, amount, 0))/6 AS inVol,
      SUM(IF(payer=uid, amount, 0))/6 AS outVol
      FROM users u LEFT JOIN r_txs t ON uid IN (payer, payee) 
      WHERE t.created>=$start AND $sinceActivated % 30 = 0 GROUP BY uid
    ) t 
    WHERE $sinceActivated>0 AND uid > 1 AND :IS_OK
X;
  queueEach('activeMonthly', $sql); // no need for jid here (floors get summed)
  
  queueEach('activeWeekly', "SELECT uid, $sinceActivated/7 AS weeks FROM users");
  
  queueNext();
}

/**
 * Remind invitee to accept invitation.
 */
function tickleSignup($item) {
  extract(just('email inviter code elapsed', $item));
  if (r\acct($email = u\decry('P', $email))) return; // invitee already began to open an account
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
  extract(just('uid elapsed', $item));
  $a = r\acct($uid);
  if (!$a->member and $step = $a->nextStep()) {
    $msg = ($step == 'signup' or $step == 'verify') ? 'do step one' : 'take another step';
    r\notify($uid, "$msg|sign in|daily messages");
    if ($elapsed >= 9 and $elapsed < 30) {
      $sql = 'SELECT DISTINCT inviter FROM r_invites i LEFT JOIN users u ON u.uid=i.inviter WHERE invitee=:uid AND u.:IS_OK';
      $q = db\q($sql, compact('uid'));
      while ($row = $q->fetchAssoc()) {
        extract($row);
        r\notify($inviter, 'invitee slow', ray('fullName elapsed step', $a->fullName, $elapsed, $step));
      }
    }
  } elseif ($a->member and !$a->ok) {
    return r\tellAdmin('activate account', compact('uid'));
  }
}

/**
 * Do stuff monthly (every 30 days) since member has been active:
 * . Calculate credit line (for overdrafts etc.): give the member (cg)credit up to their average monthly throughput over the past 6 months (which comes to 1/6 their monthly throughput if they've been in for a month, 2/6 for 2 months, etc, up to 100% at 6 months)
 * . Cashout automatically if account chose that monthly option.
 */
function activeMonthly($item) {
  extract(just('uid monthly', $item));
  $a = r\acct($uid);
  
  // adjust credit line
  $floorX = $a->floor; // current floor
  
  // temporarily (until 2019?) limit the rate of change of the floor (while changing to new system)
  $floor = round(min(-$monthly, .9 * $floorX), 2);
  $a->update('floor', round($floor, 2));

  $ratio = $floor ? $floorX / $floor : 1;
  if ($ratio > 1.1 or $ratio < .9) { // big change (or any change in sign)
    $msg = 'new floor' . ($a->can(B_DEBT) ? '' : '|no floor effect');
  // DISABLED FOR NOW if (!$a->slave) r\notify($uid, $msg, ['limit' => u\fmtAmt(-$floor)]);
  }
  
  if ($a->cashoutM) $a->cashout();
}

function activeWeekly($item) {
  extract(just('uid', $item));
  $a = r\acct($uid);
  if ($a->cashoutW) $a->cashout();
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
  $function = 'CG\\Cron\\every' . ucwords($period);
  $topic = t('cron period', compact('period'));
  $time = strftime('%I:%M%P', r\rTime());
  $msg = t('Running cron for the %period, at %time', compact('period', 'time'));
  if ($period != 'hour' and $period != 'day') r\tellAdmin($msg, compact('topic'));
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
  $cgf = r\cgfId();
  $server = r\serverUid();
  $bene = u\test() ? $cgf : db\get('uid', 'users', "name='bene' AND uid<0");
  $start = u\monthDay1($end = u\monthDay1() - 1); // mark start and end of preceding month
  if (u\test()) $start = strtotime('-30 days', $end = strtotime('today') - 1) + 1; // test: the past 30 days
  queueEach('whatsup', "SELECT uid, u.:IS_CO as co, MAX(t.created) AS lastTx from users u LEFT JOIN r_txs t ON uid IN (payer, payee) WHERE uid>:CANONIC_ACCTS AND u.:IS_OK GROUP BY uid HAVING lastTx IS NULL OR lastTx < $start");
  queueEach('inflationAdjust', "SELECT uid, $start AS start, $end AS end FROM users WHERE :IS_OK AND uid>:CANONIC_ACCTS");  
//  queueEach('shareCommitted', 'SELECT uid, committed, share FROM users WHERE :IS_OK AND committed>0');
  queueEach('roundup', "SELECT payer as uid, SUM(1-MOD(amount, 1)) AS donate FROM r_txs WHERE :IS_ROUNDUP AND amount>0 AND MOD(amount, 1)>0 AND created BETWEEN $start AND $end GROUP BY payer");
  
  $sql = <<<X
    SELECT $cgf AS cgf, u.uid, SUM(t.amount)*u.crumbs AS donate FROM r_txs t LEFT JOIN users u ON u.uid=t.payee 
    WHERE t.payer>0 AND u.crumbs>0 AND t.created BETWEEN $start AND $end GROUP BY t.payee
X;
  queueEach('crumbs', $sql); // no crumbs on rewards

  $sql = <<<X
      SELECT $cgf AS cgf, $end AS `end`, u.community AS ctty, SUM(IF(t.payee=$cgf, t.amount, 0)) AS `in`,
        SUM(IF(t.payer=$cgf AND t.created<=$end, t.amount, 0)) AS `out`,
        (SELECT COUNT(1) FROM r_usd d LEFT JOIN users du ON du.uid=d.payee 
         WHERE d.txid<0 AND d.amount<0 AND du.community=u.community AND d.created<=$end) AS bounces
      FROM r_txs t LEFT JOIN users u ON u.uid=IF(t.payer=$cgf, t.payee, t.payer)
      WHERE ((payer=$cgf AND payee<0) OR payee=$cgf) AND t.:IS_GIFT AND community NOT IN($server, $bene) GROUP BY community
X;
  queueEach('cgc', $sql);
  
  $list = db\lst("CONCAT(fullName,' (',city,')')", 'users', ':IS_OK AND :IS_PAPER');
  r\tellAdmin(t('Send paper statements'), compact('list'));
}

/**
 * Ask the member what's up, if they haven't done anything for a month or more.
 */
function whatsup($item) {
  extract(just('uid lastTx co', $item));
  $msg = 'whats up' . ($co ? '|co promo' : '');
  r\notify($uid, $msg, ray('lastTx', u\fmtDate($lastTx)));
}

/**
 * Yearly
 */
function everyYear() {
  $topic = t('Annual task list');
  r\tellAdmin(t('Generate 1099-B forms for IRS and states'), compact('topic'));
}

/**
 * Calculate and credit inflation adjustment to each member account.
 */
function inflationAdjust($item) {
  return; // UNUSED
  extract(just('uid start end', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slave in joint account gets no inflation (master gets inflation for both)
  if (db\exists('r_txs', 'payee=:uid AND type=:TX_INFLATION AND created>=:end', compact('uid', 'end'))) return;
  list ($avg, $min, $ravg) = r\averageBalance($uid, $start, $end);
//  if ($adjustment > 0)  // don't adjust debts
//  foreach ([t('average balance') => $avg, t('credit reserve') => $ravg] as $k => $v) {
  foreach ([t('average balance') => $avg] as $k => $v) {
    $adjustment = round($v * R_INFLATION_RATE / 12, 2);
//    if ($adjustment) be\fund($uid, TX_INFLATION, $adjustment, INFLATION_DESC . t(' on your ') . $k);
  }
}

/**
 * Donate rounded change to the community. Date the transaction in the preceding month (in which the roundups originate).
 * Allow this even if account is inactive (donation has already been made, from member's point of view).
 */
function roundup($item) {
  extract(just('uid donate', $item));
  $a = r\acct($uid);
  if ($donate > 0 and $a->ok) $a->pay(r\acct(r\cgfId()), $donate, t('roundups desc'), ray('created roundups gift', u\monthDay1() - 1, TRUE, TRUE));
}

/**
 * Donate percentage of funds received to the community.
 * Allow this even if account is inactive (donation has already been made, from member's point of view).
 */
function crumbs($item) {
  extract(just('uid donate cgf', $item));
  list ($a, $cgfA) = [r\acct($uid), r\acct($cgf)];
  if ($donate > 0 and $a->ok) $a->pay($cgfA, $donate, t('crumbs desc'), ray('created crumbs gift', u\monthDay1() - 1, TRUE, TRUE));
}

/**
 * Kick half the donations originating in an organized CGC back to that community.
 */
function cgc($item) {
  extract(just('ctty in out cgf end bounces', $item));
  $kickBack = round($in / 2 - $out - R_DEPOSIT_RETURN_FEE * $bounces, 2);
  if ($kickBack > 0) {
    list ($a, $cgfA) = [r\acct($ctty), r\acct($cgf)];
    $desc = t('50% share of local donations through %dt', 'dt', u\fmtDate($end));
    if ($a->ok) $cgfA->pay($a, $kickBack, $desc, ray('goods created gift', FOR_NONGOODS, u\monthDay1() - 1, TRUE));
  }
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
  $secret = urlencode(db\get('last', 'r_request', 'listid=3894'));

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
        foreach (['email', 'phone'] as $k) $row[$k] = u\cry('P', $row[$k]);
        db\update('r_request', $row, 'listid', TRUE);
      } else $header = $row + ['ctty'];
    }
  }

  $sql = <<<X
  UPDATE r_request r LEFT JOIN users u 
  ON (
    (r.email<>'' AND u.mail=r.email) OR
    (r.phone<>'' AND u.phone=r.phone) OR
    (INSTR(u.fullName, r.first)>0 AND INSTR(u.fullName, r.last)>0 AND MID(u.zip, 1, 5)=MID(r.zip, 1, 5))
  )
  LEFT JOIN r_invites AS i ON (i.email=r.email AND i.invited>r.created)
  SET done=1 WHERE NOT done AND (u.uid IS NOT NULL OR i.id IS NOT NULL)
X;
  db\q($sql);
  
  queueNext();
}

/**
 * Recalculate and update cached financial balances for each account.
 * Also give signup bonus, if appropriate.
 */
function recache() {
  $fields = 'uid, minimum, floor'; // this isn't actually used
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
  $sql = <<<X
SELECT u.uid,u.fullName,u.jid,u2.jid AS partnerJid,r.reid,r.other AS relatedId FROM users u
LEFT JOIN r_relations r ON r.main=u.uid
LEFT JOIN users u2 ON u2.uid=u.jid
WHERE u.uid<>0 AND u.jid AND r.permission=:B_JOINT-:B_RELATED
X;
  queueEach('checkAcct', $sql);
  queueNext();
}

function checkAcct($item) {
  extract($item);
  if (!$reid or !$partnerJid or $partnerJid != $uid or $relatedId != $jid) r\tellAdmin('bad relation', $item);
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
    'SELECT DISTINCT u1.uid AS id1, u2.uid AS id2, 0.5 AS w FROM users u1 INNER JOIN users u2 ON (u2.city=u1.city OR u2.zip=u1.zip) WHERE u1.uid<>u2.uid',
  ]); // each type of relationship counts as one point
  $sql = <<<X
    INSERT INTO r_near
    SELECT LEAST(q.id1, q.id2) AS uid1, GREATEST(q.id1, q.id2) AS uid2, SUM(w) AS weight 
    FROM ($queries) q 
    LEFT JOIN users u ON u.uid=q.id1
    LEFT JOIN users v ON v.uid=q.id2
    WHERE NOT u.:IS_CO AND NOT v.:IS_CO
    GROUP BY LEAST(q.id1, q.id2), GREATEST(q.id1, q.id2)
X;
  db\q($sql);
  queueNext();
}

/**
 * Update individual account stats
 * DISABLED
 */
function acctStats() {
  /*
  db\q('UPDATE users SET picture=0'); // clear temp field
  queueEach('acctStats1', 'SELECT uid FROM users WHERE uid>:CANONIC_ACCTS AND :IS_OK');
  */
  queueNext();
}

/**
 * Update stats for one account (joint accounts are aggregated in r\Acct::j_stats(), not here)
 * stats will be an assoc: [avgBalPastMo, avgBalEver, minPastMo, etc.]
 * picture (an otherwise unused field) will be minPastMo in cents (ready for r\stats to SUM)
 */
function acctStats1($item, $asof = NULL) {
  extract(just('uid', $item));
  u\setDft($asof, r\rTime());
  $a = r\acct($uid);
  $asof = strtotime('today', $asof);
  $monthAgo = u\plusMonths(-1, $asof);
  list ($avgBalPastMo, $minPastMo) = r\averageBalance($uid, $monthAgo, $asof, TRUE);
  list ($avgBalEver) = r\averageBalance($uid, strtotime('today', $a->created), $asof, TRUE);
///  debug(compact('avgBalEver','uid','asof') + ['created' => $a->created]);
  $subs = compact('uid', 'monthAgo', 'asof');
  
  $sql = <<<X
    SELECT SUM(amount) AS giftsEver, SUM(IF(completed<:monthAgo, 0, amount)) AS giftsPastMo
    FROM r_gifts WHERE uid=:uid AND completed AND completed<:asof
X;
  $row1 = db\q($sql, $subs)->fetchAssoc() ?: []; // giftsEver, giftsPastMo
  
  $sql = <<<X
    SELECT SUM(IF(type=:TX_TRANSFER, payerReward+payeeReward, 0)) AS beneEver,
    SUM(IF(type=:TX_TRANSFER AND created>=:monthAgo, payerReward+payeeReward, 0)) AS benePastMo,
    SUM(IF(type IN (:TX_GRANT), amount, 0)) AS extraEver,
    SUM(IF(type IN (:TX_GRANT) AND created>=:monthAgo, amount, 0)) AS extraPastMo
    FROM r_txs WHERE :uid IN (payer, payee) AND created<:asof
X;
  $row2 = db\q($sql, $subs)->fetchAssoc() ?: []; // get beneEver (purchase rewards), benePastMo, extraEver, extraPastMo
  
  $sql = <<<X
    SELECT SUM(-amount) AS bankedEver, SUM(IF(completed<:monthAgo, 0, -amount)) AS bankedPastMo
    FROM r_usd WHERE payee=:uid AND completed AND completed<:asof
X;
  $row3 = db\q($sql, $subs)->fetchAssoc() ?: []; // get bankedEver, bankedPastMo

  $newData = $row1 + $row2 + $row3 + compact(ray('avgBalPastMo avgBalEver minPastMo'));

  foreach ($newData as $k => $v) $newData[$k] = '' . round($v, 2); // avoid long decimals
  $a->update('stats picture', serialize($newData), $minPastMo * 100);
}

/**
 * Update system-wide and community-wide stats
 * @param int $dt1: date to start (-1 means pick up where we left off)
 */
function cttyStats($dt1 = 0) {
  if (!$dt1) { // normally this is called without a date
    cttyStats1(['ctty' => 0]); // for all our communities total
    queueEach('cttyStats1', 'SELECT DISTINCT uid AS ctty FROM users WHERE uid<0'); // and one at a time
  } else { // recreate stats data from the beginning of time
    $sql = <<<X
      SELECT community AS ctty, MIN(t.created) AS start
      FROM users u LEFT JOIN r_txs t ON t.payee=u.uid 
      WHERE uid>:CANONIC_ACCTS AND (t.created>=:R_LAUNCH OR t.created IS NULL) GROUP BY community ORDER BY -community
X;
    $cttys = db\q($sql)->fetchAllKeyed();
    u\preray([0 => min($cttys)], $cttys); // show all dates overall

    $now = r\rTime();
    if ($dt1 < 0) $dt1 = db\max('created', 'r_stats'); // repeat last date to catch missing cttys
    if (!$dt1) $dt1 = @$cttys[0] ?: $now;
    $dt = strtotime('today', $dt1); // use first moment of the day

    while ($dt <= $now) {
      foreach ($cttys as $ctty => $start) if ($start <= $dt) {
        $item = compact('ctty', 'dt');
        if (isPRODUCTION) cr\queue('cttyStats1', $item); else cttyStats1($item);
      }
      $dt = strtotime('+1 days', $dt); // every day, not just GRAIN
    }
  }

  queueNext();
}

define('BAN_MONTHS', 3); // number of months in a row to have + or - demand, to set credit ban on/off

function cttyStats1($item) {
  extract(just('ctty dt', $item));
  u\setDft($dt, $today = strtotime('today'));

  $stats = r\stats($ctty, $dt); // record stats for one community on the regional server, for the given date
  if ($dt == $today and $ctty) { // check demand for CG in the ctty
    $allPos = $allNeg = TRUE;
    for ($i = 0; $i <= BAN_MONTHS; $i++) {
      $usdX = @$usd; // following month's Dollar Pool total
      $ago = -30 * $i;
      if ($i > 0) $stats = r\stats($ctty, strtotime($ago . ' days', $dt)); // 0 is already done
      $usd = $stats['usdIn'] - $stats['usdOut']; // this month's Dollar Pool total
      if ($i > 0) {if ($usdX - $usd < 0) $allPos = FALSE; else $allNeg = FALSE;}
    }
    $cttyA = r\acct($ctty);
    $subs = ray('months', BAN_MONTHS);
    if ($allNeg and !$cttyA->noneg) r\tellCo('credit ban on', $subs, $ctty);
    if ($allPos and $cttyA->noneg) r\tellCo('credit ban off', $subs, $ctty);
  }
}

/**
 * Calculate how much each member is trusted by the others.
 */
function trust() {
  db\q('UPDATE users SET trust=IF(:IS_CO,NULL,1)');
  $sql = <<<X
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
X;
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
  include_once __DIR__ . '/../cg-risk.inc';
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
  include_once __DIR__ . '/../cg-risk.inc';
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
  include_once __DIR__ . '/../cg-risk.inc';
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
  if (u\test() or in($weekday, [2, 3, 4, 5])) { // no completions on Saturday, Sunday, or Monday
    $now = r\rTime();
    $sql = 'SELECT * FROM r_usd WHERE amount>0 AND NOT completed AND (amount<=:R_USDTX_QUICK OR :now>=created+:R_USDTX_DAYS*:DAY_SECS)'; // this allows us to wait arbitrarily long to deposit chks
//    $sql = 'SELECT * FROM r_usd WHERE amount<0 AND NOT completed AND deposit AND :REQUEST_TIME>=deposit+:R_USDTX_DAYS*:DAY_SECS'; // this depends on deposits almost every day
    queueEach('completeUsdTx', $sql, compact('now'));
  }
  
//  $where = ':IS_REFILL AND (:IS_JOINED OR r-rewards-savingsAdd<minimum) AND uid>1 AND :IS_OK';
  $where = 'u.:IS_REFILL AND (jid OR balance<minimum) AND uid>:CANONIC_ACCTS AND u.:IS_OK';
  $sql = <<<X
  SELECT uid
  FROM users u
  WHERE $where AND :HAS_BANK
X;

  queueEach('bank1', $sql); // Note: joint slave r values cannot be negative unless both are (see r\cache)
  
  $sql = <<<X
  SELECT uid, r.main as ma
  FROM users u INNER JOIN r_relations r ON r.other=u.uid
  WHERE $where AND NOT :HAS_BANK AND r.:IS_DRAW 
  AND r.permission<>:B_JOINT-:B_RELATED
X;
  queueEach('draw1', $sql);

  queueNext();
}

function completeUsdTx($item) {
  extract(just('txid payee amount channel', $item));
  $realAmt = db\get('amount', 'r_usd', 'txid=:txid', compact('txid'));
  if ($amount != $realAmt) r\tellAdmin('bad completeUsdTx amount', $item + compact('realAmt'));
  r\acct($payee)->completeUsdTx($txid, $amount, @$channel == TX_CRON);
}

function bank1($item) {
  extract(just('uid', $item));
  $a = r\acct($uid);
  if ($a->slave) return; // slaves don't have automatic refills
  if (!$amount = getHowMuchR($a)) return; // check shortfall here NOT in query, balance may change
  if (!$a->bank($amount, $msg)) return; // may never happen, but if mysql is unavailable just redo next day
r\notify($uid, "under min|$msg");
}

function draw1($item) {
  extract(just('uid ma', $item)); // ma is UNUSED
  $a = r\acct($uid);
  if ($a->jid) return; // don't draw within a joint account
  if (!$amount = getHowMuchR($a, FALSE)) return;

  if ($a->draw($amount)) {
    $amount = u\fmtAmt($amount);
    r\notify($uid, 'under min|drew', compact('amount'));
  } else r\notify($uid, 'under min|cannot draw');
}

/**
 * Given an account's shortfall, return how much to actually transfer to the CG Account.
 * @param acct $a: which account
 * @param bool $banky: the account has a connected bank account
 * @return: positive amount to draw from the connected bank account or elsewhere (0 if none).
 */
function getHowMuchR($a, $banky = TRUE) {
  if (!$need = max(0, $a->minimum - $a->balance)) return 0; // + $a->committed
  if ($banky) {
    $waiting = $a->waitingFromBank($combinable); // net amount already requested from bank (if any)
    $need = round($need - $waiting, 2); // avoid a re-request
  }
  return ($need <= 0 ? 0 : max(@$combinable + 0 > 0 ? 0 : $a->achMin, round(R_ACHMIN * ceil($need / R_ACHMIN))));
}

/**
 * Handle pending invoices.
 */
function invoices() {
  $sql = 'SELECT * FROM r_invoices WHERE status=:TX_PENDING OR status=:TX_APPROVED';
  queueEach('invoice', $sql);
  queueNext();
}

function invoice($item) {
  extract(just('nvid payer payee amount purpose status created', $item));
  $a = r\acct($payer);
  $a2 = r\acct($payee);
  $payeeName = $a2->fullName;
  
  $masterA = $a->masterA;
  $needed = $a->roundup ? ceil($amount) : $amount;
  
  if ($status == TX_APPROVED) {
    $short = round($needed - $a->avail(), 2);
    if ($short <= 0) {
      if (!$ok = be\payInvoice($nvid, $msg)) r\tellAdmin(t('invoice payment failed in cron'), compact('msg') + $item);
    } else {
      $waiting = $a->waitingFromBank();
      if ($masterA->hasBank and $waiting <= $short) { // we can get it from the bank and haven't requested it yet
        if ($masterA->refill) $needed += getHowMuchR($masterA);
        if ($needed < 0.01) return; // (avoid floating point equality errors by using <)
        if (!$masterA->bank($needed, $msg)) return; // may never happen, but if mysql is unavailable just redo next day
        r\notify($payer, $msg);
      } elseif ($waiting <= $short) r\notify($payer, 'short invoice|when funded', ray('short payeeName', u\fmtAmt($short), $payeeName));
    }
  } elseif ($status == TX_PENDING) {
    $today = strtotime('today'); // not time(), because tests fail erratically
    $daysAgo = max(1, round(($today - $created) / DAY_SECS));
    if ($daysAgo % 7 == 0) {
      $subs = ray('daysAgo created amount purpose nvid _a1 payerName payeeName email phone', $daysAgo, u\fmtDate($created, TRUE), u\fmtAmt($amount), $purpose, $nvid, $a->makeDo('inv', $nvid), $a->fullName, $payeeName, $a2->email, $a2->phone);
      r\message($payer, 'stale invoice', $subs + ['reply' => $payee]); // tell other party by email about this invoice
      r\message($payee, 'stale invoice report', $subs); // tell the payee we're following up on it
    }
  }
}

/**
 * Handle requests to invest in an investment club.
 */
function invest() {
  queueEach('invest1', 'SELECT stakeid,uid,request FROM r_stakes WHERE request>0');
  queueNext();
}

function invest1($item) {
  extract(just('uid,stakeid,request', $item));
  $a = r\acct($uid);
  if ($a->avail($request)) {
    r\handleInvestmentRequest($stakeid, $msg);
  } elseif (!$a->waitingFromBank()) {
    $a->bank($request, $msg);
    $msg .= t('|(to fulfill your recent investment request)'); // whether success or failure
  }
  r\notify($uid, $msg);
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
  $sql = "SELECT DISTINCT u.uid, :IS_WEEKLY AS weekly, :IS_MONTHLY AS monthly FROM r_notices n JOIN users u ON u.uid=n.uid WHERE IF(activated, u.:IS_OK, NOT u.:IS_NONUDGE) AND NOT sent AND ($where)";

  queueEach('notice', $sql);
  queueNext();
}

function notice($item) {
  extract(just('uid daily weekly monthly', $item));

  $now = r\rTime();
  if ($args = r\noticeArgs($uid, 'NOT sent', $weekly, $monthly)) {
    $args['messages'] = str_replace('stule=', 'style=', $args['messages']); // revert in emails (but CSP forbids inline online)
    r\notify($uid, 'notices', $args, TRUE);
    db\q('UPDATE r_notices SET sent=:now WHERE uid=:uid AND NOT sent', compact('uid', 'now'));
  }
}

/**
 * Last task queued once the everyDay() function has gotten things started.
 */
function END() {
  global $mya; $mya = NULL; // don't tell Admin the last current member
  if (db\exists('queue')) r\tellAdmin('queue has dingleberries'); else db\q('TRUNCATE queue'); // reset serial field
  \variable_set('last_daily_cron', NOW);
  $lastCron = \variable_get('r_last_cron', []);
  $elapsed = NOW - @$lastCron['day'];
  $time = strftime('%I:%M%P');
  $msg = t('At %time, after %elapsed secs, daily cron ended successfully.', compact(ray('time elapsed')));
  $topic = t('cron end');
  r\tellAdmin($msg, compact('topic'));
}


//function realId($uid) {return ($uid <= r\cgfId());}
