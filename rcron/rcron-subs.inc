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

  if ($cronOp) {
    if ($cronOp != 'ALL') cr\queue('END');
    if (u\test()) while (db\exists('queue')) cr\run();
    return; // doing just one, so do nothing more
  }

  u\setDft($task, str_replace(__NAMESPACE__ . '\\', '', u\caller()));
  u\loga('queueNext', compact('task'));

//  if (u\test() and @$cronOp and $cronOp != 'ALL') {
//    cr\queue($task == $cronOp ? 'END' : $cronOp);
  //} else 
  cr\queue(u\nextAfter($task, ray(R_DAILY_TASKS)));
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

function queue($op, $item = []) {
  global $cronOp; // this is set when calling cron() from PHP window on test server

  $item = compact('op') + $item;
  if (@$cronOp or u\test()) {
    cr\doTask($item);
  } else db\insert('queue', ray('item created', serialize($item), time()));
}
