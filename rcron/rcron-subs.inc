<?php
namespace rCredits\Cron;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;
use rCredits\Risk as k;
use rCredits\Cron as cr;

/**
 * @file
 * Subroutines for Cron activities.
 
 f('cr.zot', arg1, ...);
*/

function queueEach($op, $sql, $subs = []) {
  $result = db\q($sql, $subs);
  $sql = u\SUBS($sql);

  while ($row = $result->fetchAssoc()) {
    cr\queue($op, $row);
  }
}

/**
 * Run the next daily function.
 */
function queueNext($task = NULL) {
  global $cronOp; 
  if ($cronOp == ONE_CRON_OP) {
    if (isDEV) while (db\exists('queue')) \drupal_cron_run();
    return; // doing just one, so do nothing more
  }

  u\setDft($task, str_replace(__NAMESPACE__ . '\\', '', u\caller()));

  if (u\test() and @$cronOp and $cronOp != 'ALL') {
    cr\queue($task == $cronOp ? 'END' : $cronOp);
  } else cr\queue(u\nextAfter($task, ray(R_DAILY_TASKS)));
//  } else cr\queue($tasks[array_search($task, $tasks) + 1]);
}

/**
 * Say whether it's time to run a periodic function.
 * @param int $time: current time (or time to test)
 * @param string $period: name of period type (day, hour, etc.)
 * @param int $chosen: at which point should we do the function
 * @return <it's now a different period than when the function was last run AND we're at or past the chosen point>
 */
define('TIME_CODES', 'year:Ym,month:md,week:Ua,day:dH,hour:HM');

function timeFor($time, $period, $chosen) {
  global $timeFor; if (u\test() and isset($timefor)) return @$timefor[$period];
  
  $lastCron = \variable_get('r_last_cron') ?: [];
  $lastTime = @$lastCron[$period];
  $codes = ray(TIME_CODES);
  list ($periodCode, $withinCode) = str_split($codes[$period]);
  $point = strftime("%$withinCode", $time);
  if ($withinCode == 'a') $point = u\day2n($point); // %u and %w fail on dev machine (among others)
  list ($periodNum, $lastPeriodNum) = [strftime("%$periodCode", $time), strftime("%$periodCode", $lastTime)];
  return ($periodNum != $lastPeriodNum and $point >= $chosen);
}

define('OFTEN_TESTS', 'year 01 1/1/2000 12/31/2000 0, year 01 1/1/2000 1/1/2001 1, year 01 1/1/2000 1/2/2001 1, '
. 'year 01 1/3/2000 12/31/2000 0, year 01 1/3/2000 1/1/2001 1, year 01 1/3/2000 1/4/2001 1, ' // two days late last time
. 'month 01 1/1/2000 1/31/2000 0, month 01 1/1/2000 2/1/2000 1, month 01 1/1/2000 2/2/2000 1, '
. 'month 01 1/3/2000 1/31/2000 0, month 01 1/3/2000 2/1/2000 1, month 01 1/3/2000 2/4/2000 1, ' // two days late last time
. 'week 0 1/2/2000 1/8/2000 0, week 0 1/2/2000 1/9/2000 1, week 0 1/2/2000 1/10/2000 1, '
. 'week 0 1/4/2000 1/8/2000 0, week 0 1/4/2000 1/9/2000 1, week 0 1/4/2000 1/12/2000 1, ' // two days late last time
);

function oftenTests() {
  $ok = TRUE;
  foreach (ray(OFTEN_TESTS) as $one) {
    if (!$one) break;
    list ($period, $chosen, $last, $time, $want) = ray($one);
    \variable_set('r_last_cron', [$period => strtotime($last)]);
    if (timeFor(strtotime($time), $period, $chosen) != $want) $ok = t\output("test failed: $one");
  }
  return $ok;
}

function queue($op, $item = []) {
  global $cronOp; // this is set to ONE_CRON_OP when calling cron() from PHP window on test server
  global $mya;

  $info = $item;
  if ($uid = @$info['uid']) $mya = r\acct($uid); // fudge current account
//  if (!strpos($op, '1')) {

  if (!in_array($op, ray('acctHourly acctStats acctStats1 acctRisk acctRisk1'))) u\log($op, 'cr', u\justNot('uid', $item));
  
  if (@$cronOp == ONE_CRON_OP or u\test()) {
    $oneOp = "rCredits\\Cron\\$op";
    $oneOp($item);
  } else \DrupalQueue::get('q')->createItem(compact('op') + $item);
}
