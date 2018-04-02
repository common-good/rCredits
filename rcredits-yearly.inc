<?php
/**
 * @file
 * Functions that get run just once a year.
 * Assume that Common Good Finance is the report transmitter.
 */

namespace rCredits\Special; // typically abbreviated as "y"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Backend as be;
use rCredits\Web as w;

define('EOL', "\r\n"); // must be two characters long (make sure download doesn't change it)
define('STATE_CODE', '25'); // Massachusetts code, for combined Federal/State filing

/**
 * Create 1099-B reports for the IRS as a downloadable file to be uploaded to the IRS FIRE site.
 * Log in at fire.irs.gov (or fire.test.irs.gov)
 * @param mixed $testMode: must be set non-FALSE for a successful test. If >1, it is a single uid to test.
 * @param assoc $badNames: names reported incorrectly, indexed by uid
 * @param assoc $badTINs: EINs (no punctuation) reported incorrectly, indexed by uid
 * @param int $year: reporting year (defaults to most recent completed calendar year)
 * example for corrections: f('y.do1099bs', FALSE, [23947972 => 'Miss Pelled'], [29274902 => '292039482'], 2015);
 */
function do1099bs($testMode = FALSE, $badNames = [], $badTINs = [], $year = NULL) {
  global $err1099;
  u\setDft($year, date('Y') - 1);
  u\beginDownload("forms1099b-FY$year.bin$testMode"); // binary file extension: discourage automatic changes during FTP
  putRec(recT($year, $testMode));
  
  $uids = array_keys($badNames + $badTINs);
  $where = ($uids = join(',', $uids ?: [])) ? "uid IN ($uids)" : 'activated'; // making :uids a parameter fails 
  $sql = "SELECT uid FROM users WHERE $where ORDER BY uid";

  if ($uids) { // show bad old data first if correcting
    doPayer($year, $sql, $totals, 'G', $badNames, $badTINs);
    $bCount = doPayer($year, $sql, $totals, 'C'); // corrected data
  } else $bCount = doPayer($year, $sql, $totals);
  
  $aCount = $uids ? 2 : 1;
  putRec(recK($bCount * $aCount, $totals));
  putRec(recF($bCount * $aCount, $aCount));
  flush();
  if (@$err1099) return r\go('sadmin/panel'); else exit(); // shouldn't we exit in any case?
}

/**
 * Handle all appropriate payees for the payer (that is the rCredits regional server).
 * @return the number of payee ("B") records.
 */
function doPayer($year, $sql, &$totals, $mod = '', $badNames = [], $badTINs = []) {
  global $seq1099;
  putRec(recA($year));
  $bCount = 0; // IRS instructions pub 1220 call this "number of payees", but say it means "number of B records"
  $total = 0;
  $q = db\q($sql);
  while ($row = $q->fetchAssoc()) {
    extract($row); // uid
    $bCount += do1099b($year, $uid, $total, $mod, $badNames, $badTINs);
    $bFactor = ($badNames or $badTINs) ? 2 : 1; // two records for each payee, if correcting
    if ($seq1099 + 0 != $bCount * $bFactor + 2) return err1099(t('Record count %rc at odds with payee count %pc.', 'rc pc', $seq1099, $bCount));   
  }
  $totals = u\lpad(0, 6 * 18) . u\lpad(fmtAmt($total), 18) . u\lpad(0, 9 * 18);
  putRec(recC($bCount, $totals));
  return $bCount;
}

/**
 * Create B records for each income transaction, for each payee.
 * @param int $year: the relevant fiscal year
 * @param int $payee: payee's account ID
 * @param float $total: running total amount for all B forms so far
 * @param string $mod: normally ''. C=show corrected info. G=show incorrect name and/or TIN (as submitted before)
 * @param assoc $badNames: list of incorrectly reported names, indexed by uid
 * @param assoc $badTINs: list of incorrectly reported TINs, indexed by uid
 * @return number of B records created
 */
function do1099b($year, $payee, &$total, $mod = '', $badNames = [], $badTINs = []) {
//  global $amts99;
  $bCount = 0;
  while ($row = be\get1099b($payee, $year)) {
    extract(just('created amount for', $row));
    if ($amount <= 0) continue; // IRS says do NOT report non-positive transactions on 1099-B
    if ($mod == 'G') $amount = 0; // IRS says zero out amount when showing the bad old data
    $total += $amount;
//    $amts99[] = $amount;
    $bCount++;
    $a = r\acct($payee); $id = $a->id;
    putRec(recB($year, $a, $created, fmtAmt($amount), $for, $mod, @$badNames[$id], @$badTINs[$id]));
  }
  return $bCount;
}

function recT($year, $testMode) {
  //$region = r\acct(r\serverUid());
  $prior = pad(($year < date('Y') - 1) ? 'P' : '', 1); // P for prior year data, else ' '
  $TCC = '08C20';
  $testing = pad($testMode ? 'T' : ' ', 1);
  $foreign = pad(FALSE, 1); // 1 for foreign
  $coName = $transmitterName = pad(CGF_LEGALNAME, 80);
  list ($address, $city, $st, $zip) = u\parseAddr(CGF_POSTALADDR);
  $coAddr = pad($address, 40) . pad($city, 40) . $st . pad(str_replace('-', '', $zip), 9);
  $payees = u\lpad(0, 8); // not possible to know the count yet
  $contact = pad(ADMIN_NAME, 40) . pad(u\fmtPhone(R_ADMIN_PHONE,'n'), 15) . pad(R_ADMIN_EMAIL, 50);
  $source = pad('I', 1); // V=vendor I=in-house
  $vendor = sp(40) . sp(40) . sp(40) . sp(2) . sp(9);
  $vendorContact = sp(40) . sp(15);
  $foreignVendor = pad(FALSE, 1);
  
  return 'T' . $year . $prior . CGF_EIN . $TCC . sp(7) . $testing . $foreign . $transmitterName . $coName . $coAddr . sp(15) . $payees . $contact . sp(91) . seq() . sp(10) . $source . $vendor . $vendorContact . sp(35) . $foreignVendor . sp(8) . EOL;
}

function recA($year) {  
  $combo = pad(TRUE, 1); // combined Federal/State filing
  $regionTIN = '461821792';
  $nameControl = sp(4);
  $lastEver = pad(FALSE, 1);
  $type = pad('B', 2); // B=1099-B
  $amountCodes = pad('7', 16); // amount is entered on this line number on form 1099-B
  $foreign = pad(FALSE, 1); // 1 for foreign
  $payerName = pad(r\regionfield('legalName'), 80);
  $isAgent = u\lpad(0, 1); // 1 if second half of payerName is transfer agent, else 0
  $payerAddr = pad('c/o Common Good, PO Box 21', 40) . pad('Ashfield', 40) . 'MA' . pad('013300021', 9);
  $payerPhone = pad('4136281723', 15); // was 4133765021 (maybe again someday)
  
  return 'A' . $year . $combo . sp(5) . $regionTIN . $nameControl . $lastEver . $type . $amountCodes . sp(8) . $foreign . $payerName . $isAgent . $payerAddr . $payerPhone . sp(260) . seq() . sp(241) . EOL;
}

/**
 * Return a payee ("B") record for the given year, account, amount, and purpose.
 * @param string $mod: normally ''. C=show corrected info. G=show incorrect name and/or TIN (as submitted before)
 */
function recB($year, $a, $completed, $amount, $for, $mod = '', $badName = NULL, $badTIN = NULL) {
  $who = "$a->legalName ($a->mainQid)";
  if (!isset($badName) and !isset($badTIN)) {
    if ($mod == 'G') return err1099(t('Original B Record has no error to be corrected: ') . $who);
  } elseif ($mod != 'G') if (isset($badName) or isset($badTIN)) return err1099(t('Spurious badName (%badName) or badTIN (%badTIN) for payee %who.', compact('badName', 'badTIN', 'who')));
  $corrected = pad($mod, 1);
  $tinType = pad($a->co ? 1 : 2, 1);
  $payeeTIN = pad(isset($badTIN) ? $badTIN : $a->federalId, 9);
  $accountNum = pad($a->mainQid, 20);
  $payerOffice = pad('', 4);
  $amt = u\lpad($amount, 12);
  $foreignPayee = pad('', 1);
  $payeeName = pad(noBreakAt(preg_replace('/[^A-Z &\-]/i', '', isset($badName) ? $badName : $a->legalNameDpy), 40), 2 * 40);
  list ($payeeFirst, $payeeLast) = u\parseName($payeeName);
  $nameControl = pad(preg_replace('/[^A-Z]/i', '', $a->co ? $payeeName : ($payeeLast . $payeeFirst)), 4);
//  $nameControl = pad(preg_replace('/[^A-Z]/i', '', ($a->co and $mod != 'G') ? $payeeName : $payeeLast), 4); // temporary for TY2014,2015
  list ($address, $city, $st, $zip) = u\parseAddr($a->postalAddr);
  if ($st == 'ER' or !$zip or !$city) return err1099(t('Bad postalAddr for ') . $who);
  $payeeAddr = pad($address, 40) . sp(40) . pad($city, 40) . pad($st, 2) . pad(u\digits($zip), 9);
  $date = strftime('%Y%m%d', $completed);
  $desc = pad($for, 39);
  $qty = u\lpad(0, 13);
  $symbol = pad(0, 13);
  $withheld = u\lpad(0, 12) . u\lpad(0, 12);
  
  return strtoupper('B' . $year . $corrected . $nameControl . $tinType . $payeeTIN . $accountNum . $payerOffice . sp(10) . u\lpad(0, 6 * 12) . $amt . u\lpad(0, 9 * 12) . $foreignPayee . $payeeName . sp(40) . $payeeAddr .  sp(1) . seq() . sp(36) // end of the part that's the same for all kinds of 1099

  . sp(4) . $date . sp(13) . $desc . sp(8) . sp(1) . $qty . $symbol . sp(20) . sp(60) . $withheld . STATE_CODE . EOL); // special part for 1099-B
}

function recC($bCount, $totals) { 
  return 'C' . u\lpad($bCount, 8) . sp(6) . $totals . sp(196) . seq() . sp(241) . EOL;
}
  
function recK($bCount, $totals) {
  $withheld = u\lpad(0, 18) . u\lpad(0, 18);
  
  return 'K' . u\lpad($bCount, 8) . sp(6) . $totals . sp(196) . seq() . sp(199) . $withheld . sp(4) . STATE_CODE . EOL;
}

function recF($bCount, $aCount) {
  return 'F' . u\lpad($aCount, 8) . u\lpad(0, 21) . sp(19) . u\lpad($bCount, 8) . sp(442) . seq() . sp(241) . EOL;
}
    
/**
 * Make sure the string has a whole word at the end of the first chunk (typically for 40/40 names).
 * This is required for payee name.
 * @param string $s: the string to fit in two chunks
 * @param in $chunk: chunk size
 * @return the string, padded out so the second chunk starts on a word boundary (if possible)
 */
function noBreakAt($s, $chunk) {
  if (strlen($s) <= $chunk) return $s;
  $extra = 2 * $chunk - strlen($s);
  for ($i = $chunk; $i >= $chunk - $extra; $i--) {
    if (substr($s, $i, 1) == ' ') {
      return ($i < $chunk - 1) ? substr($s, 0, $i) . sp($chunk - $i) . substr($s, $i + 1) : $s;
    }
  }
  return $s;
}
 
/**
 * Show an error message if the record is the wrong length.
 * @param string $rec: the record to test
 * @return the record
 */ 
function lenCheck($rec) {
  global $seq1099;
  $type = substr($rec, 0, 1);
  $reclen = strlen($rec);
  $ideal = 750;
  if (strlen($rec) != $ideal) return err1099("ERROR: $type record #$seq1099 is $reclen chars -- should be $ideal:\n" . breakup($rec));
  return $rec;
}

function err1099($msg) {
  global $err1099;
  \drupal_set_message($msg);
  $err1099 = TRUE;
/**/  echo "\n\n********* ERROR *************\n\n";
  return '';
}

/**
 * Check the record and write it to the output file.
 * @param string $rec: the record
 */
function putRec($rec) {
  global $err1099;
  //fwrite(STDOUT, lenCheck($rec));
/**/  if (!@$err1099) echo lenCheck($rec);
}

/**
 * Return the next record sequence number, formatted.
 */
function seq() {
  global $seq1099;
  return $seq1099 = u\lpad(@$seq1099 ? $seq1099 + 1 : 1, 8);
}

/**
 * Break a line up with "|" every 10 characters, for easier debugging.
 */
function breakup($s, $len = 10) {
  for ($i = 0, $new = ''; $i < strlen($s); $i += $len) {
    $new .= substr($s, $i, $len) . '|';
  }
  return str_replace(' ', '.', $new);
}

function pad($s, $n) {return substr(str_pad($s, $n), 0, $n);}
function sp($n) {return pad('', $n);}
function fmtAmt($n) {return preg_replace('/[\,\.]/', '', number_format($n, 2));}