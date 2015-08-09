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
 * Subroutines for Cron activities.
 
 f('cr.zot', arg1, ...);
 
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
