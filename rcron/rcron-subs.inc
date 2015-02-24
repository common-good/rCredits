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
 
 f('cr.zot', arg1,...);
 
/**
 * Return a list of participating companies for the given community.
 */
function companies($ctty) {
  $res = '';
  $q = db\q('SELECT uid FROM users WHERE :IS_CO AND :IS_OK AND uid>1 AND community=:ctty', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    $a = r\acct($row['uid']);
    list ($selling) = explode("\n", $a->selling);
    $res .= "<tr><td width=\"200\">$a->fullName</td><td style=\"margin-left:10px;\">$selling</td></tr>\n";
  }
  return $res;
}