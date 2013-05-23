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
function usdAccts($where = "postalCode LIKE '013%'") {
  if (!r\acct()->can(B_ADMIN)) return;
  $result = r\dbQ("SELECT uid FROM users WHERE $where ORDER BY fullName");
  $head = u\ray('Name Dwolla? Type Verified OK Chg Co Bal');
  while ($row = $result->fetch()) {
    $usA = new r\usd($a = r\acct($row->uid));
    $bal = $usA->bal($err);
    $hasDwolla = $err ? '-' : 'Yes';
    $source = $usA->source($err);
    $type = @$source['Type'];
    $verified = @$source['Verified'] ? 'Yes' : '-';
    $count = @$source['count'];
    $ok = $a->can(B_OK) ? 'ok' : '';
    $charge = $a->can(B_CHARGE) ? 'Chg' : '';
    $company = $a->can(B_COMPANY) ? 'Co' : '';
    $line = array($a->fullName, $hasDwolla, $type, $verified, $ok, $charge, $company, $bal);
    $lines[] = array_combine($head, $line);
  }
  return showTable($lines);
}

function showTable($ray) {
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $key => $value) {
      if ($value and in_array($key, u\ray('id uid payer payee main other inviter invitee proxy person owner defaultAgent'))) $value = $value < 0 ? 'ctty' : r\acct($value)->fullName;
      if ($value and in_array($key, u\ray(R_DATE_FIELDS))) $value = u\formatDate($value);
      if ($key == 'flags') $value = "<div style='text-transform:capitalize; font-size:7px; line-height:11px;'>" . bs($value) . '</div>';
      if ($key == 'payeeFor' and $value == $row->payerFor) $value = 'same';
      $line .= "<td>$value</td>";
    }
    $lines[] = "<tr>$line</tr>";
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>", $lines) . "</tr>\n</table>";
}

function showTableQ($sql, $subs = array()) {
  if (!strpos($sql, ' ')) {$sql = "SELECT * FROM $sql " . ($subs ?: ''); $subs = array();}
  if (!$result = r\dbQ($sql, $subs)) return \drupal_set_message('bad query');
  return showTable($result->fetchAll());
}