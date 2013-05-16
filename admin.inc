<?php
/**
 * @file
 * rCredits admin
 */

use rCredits as r;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;

/**
 * List the USD account status of each account.
 */
function usdAccts($where = 'uid>1') {
  debug(r\acct());
  //if (!r\acct()->can(B_ADMIN)) return;
  $result = r\dbQ("SELECT uid FROM users WHERE $where ORDER BY fullName");
  $list = '';
  while ($row = $result->fetch()) {
    $usA = new r\usd($a = r\acct($row->uid));
    $bal = $usA->bal($err);
    $hasDwolla = $err ? '-' : 'Yes';
    $source = $usA->source($err);
    $type = @$source['Type'];
    $verified = @$source['Verified'] ? 'Yes' : '-';
    $count = @$source['count'];
    $list .= "<tr><td>$a->fullName</td><td>$count</td><td>$hasDwolla</td><td>$type</td><td>$verified</td><td>$bal</td></tr>\n";
  }
  \drupal_set_message(<<<EOF
  <table>
  <tr><th>Name</th><th>#</th><th>Dwolla?</th><th>Type</th><th>Verified</th><th>Bal</th></tr>
  $list
  </table>
EOF
  );
}