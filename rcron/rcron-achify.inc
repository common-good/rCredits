<?php
use rCredits as r;
use rCredits\Util as u;
use rCredits\DB as db;

define('F_FILEHDR', 'recType:1, priority:2, destNum:10, originNum:10, datetime:10, fileIdModifier:1, recSize:3, blocking:2, format:1, destName:23, originName:23, ref:8'); // header format
define('F_BATCHHDR', 'recType:1, class:3, originName:16, data:20, originNum:10, secCode:3, purpose:10, when:6, entryDate:6, settle:3, status:1, destNum:8, batchNum:7'); // batch header format
define('F_PPD', 'recType:1, txCode:2, routing:9, account:17, amount:10, id:15, name:22, data:2, addendaFlag:1, destNum:8, count:7'); // PPD (Prearranged Payment and Deposit) entry detail format
define('F_BATCHFTR', 'recType:1, class:3, count:6, hash:10, debits:12, credits:12, originNum:10, auth:19, reserve:6, destNum:8, batchNum:7'); // end of batch (control record)
define('F_FILEFTR', 'recType:1, batches:6, blocks:6, entries:8, hash:10, debits:12, credits:12, reserve:39'); // end of file (control record)
define('DEST_NUM', '011500120'); // usually a routing number for the bank providing ACH service

/**
 * Create an ACH request file (debits only, for now).
 */
function achify($where = 'deposit=0') {
	$CGCODE = 'ASOBENE1'; // company code assigned by Citizens Bank for upload filename
  $reserve = ''; // leave reserved fields empty
  $count = $hash = $credits = $debits = 0;

  list ($recType, $priority, $recSize, $blocking, $format) = [1, 1, 94, 10, 1];
  $fileIdModifier = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9); never send two files on the same day, so this can be static
  list ($destNum, $destName) = [' ' . DEST_NUM, 'Citizens Bank'];
  $datetime = strftime('%y%m%d%H%M');
  list ($originNum, $originName, $ref) = ['1' . CGF_EIN, PROJECT, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEHDR))), $ray);
  
  $destNum = substr(DEST_NUM, 0, 8); // BATCHHDR, PPD, and BATCHFTR omit last digit (check digit) of routing number
  list ($recType, $class, $secCode, $settle, $status) = [5, 200, 'PPD', '', 1]; // 200=mixed (maybe), 225=debits, 220=credits
  $entryDate = strftime('%y%m%d', strtotime('tomorrow'));
  $batchNum = 1; // batch (company) number within file
  list ($data, $purpose, $when) = ['', t('CG Credits'), strftime('%b%d')];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHHDR))), $ray);

/*
  $tdRoute = '21127445'; // sending bank's routing number without the final "0"
  $now = strftime('%y%m%d%H%M');
  $fileId = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9)
  $dest = pad('RBS CITIZENS NA', 23);
  $source = pad(PROJECT, 23);
  $ref = pad('', 8);
  $recs[] = "101 {$tdRoute}0" . '1' . CGF_EIN . "$now{$fileId}094101$dest$source$ref"; // file header
  
  $class = '200'; // mixed credits and debits (225 for debits only, 220 for credits only)
  $coName = pad(strtoupper(PROJECT), 16); // 'SOC BENFT EVRYON'? 'COM GOOD FINANCE'? 'COMMON GOOD FIN'?
  $extra = pad('', 20); // discretionary (we can use this space for anything)
  $code = 'PPD'; // prearranged payment and deposit
  //$purpose = pad($amount > 0 ? t('rCredits') : t('money out'), 10);
  $purpose = pad(t('CG Credits'), 10);
  $when = pad(strftime('%b %d'), 6); // descriptive date
  $effective = strftime('%y%m%d'); // requested settlement date
  $batch = u\lpad(1, 7); // batch (company) number within file
  $recs[] = "5$class$coName$extra" . '1' . CGF_EIN . "$code$purpose$when$effective" . "0001$tdRoute$batch"; // co batch header
*/

//  $where = TRUE ? 1 : 'amount<=0';
  $q = db\q("SELECT txid,payee,amount,deposit FROM r_usd WHERE $where ORDER BY txid");
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($payee);
    $bankAccountType = 'checking'; // we handle only checking accounts at this time
    if ($a->mainQid == 'NEWAFX') $bankAccountType = 'savings';
    $count++;
    list ($recType, $id, $name, $addendaFlag) = [6, $a->mainQid, $a->legalName, 0];
    list ($routing, $account) = u\parseBankAccount($a->bankAccount);
/// TEST    if ($a->mainQid == 'NEWAIO') $amount = 0; // debug(
		
		$amount *= 100;
    if ($amount <= 0) $credits -= $amount; else $debits += $amount;
    $txCode = $amount == 0 ? 23 
    : (@$bankAccountType == 'checking' ? ($amount < 0 ? 22 : 27) : ($amount < 0 ? 32 : 37));
		$amount = abs($amount);

    $recs[] = achRecord(compact(array_keys($ray = ray(F_PPD))), $ray);

    $hash += substr($routing, 0, 8);
    if (!$deposit) db\update('r_usd', ray('deposit txid', strtotime('today'), $txid), 'txid');
    
    /*    
    list ($route, $account) = u\parseBankAccount($a->bankAccount);
    $bankAccountType = 'checking'; // we handle only checking accounts at this time
    $code = @$bankAccountType == 'checking' ? ($amount < 0 ? 22 : 27) : ($amount < 0 ? 32 : 37);
    $account = pad($account, 17);
    if ($amount < 0) $credits -= $amount; else $debits += $amount;
    $amount = padnum($amount, 10);
    $acctId = pad(($a->co ? 'EIN #' : 'SSN #') . $a->federalId, 15);
    $name = pad($a->legalName, 22);
    $extra = pad('', 2); // discretionary
    $trace = $tdRoute . u\lpad($txid, 7); // 15 chars total
    $recs[] = "6$code$route$account$amount$acctId$name$extra" . "0$trace"; // one ach request
    $count += 1;
    $hash = substr($hash + $route, 10);
    */
  }
  
  if (!$count) return 0;
  
  list ($recType, $auth) = [8, ''];
  $recs[] = achRecord(compact(array_keys($ray = ray(F_BATCHFTR))), $ray);

  list ($recType, $batches, $entries) = [9, 1, $count];
  $blocks = ceil((2 + $count + 2) / 10); // how many blocks of 10 lines, rounded up (2 hdrs, "count" entries, 2 ftrs)
  $recs[] = achRecord(compact(array_keys($ray = ray(F_FILEFTR))), $ray);

  /*
  $count = u\lpad($count, 6);
  $hash = u\lpad($hash, 10);
  $debits = padnum($debits, 12);
  $credits = padnum($credits, 12);
  $auth = u\lpad('?', 8) . u\lpad('', 11); //
  $b6 = pad('', 6);
  $recs[] = "8$class$count$hash$debits$credits" . '1' . CGF_EIN . "$auth$b6$tdRoute$batch";  // batch control

  $batches = u\lpad(1, 6);
  $blocks = u\lpad(ceil((2 + $count + 2) / 10), 6); // how many blocks of 10 lines, rounded up
  $count = u\lpad($count, 8);
  $b39 = u\lpad('', 39);
  $recs[] = "9$batches$blocks$count$hash$debits$credits$b39"; // file control
  */
/**/  foreach ($recs as $line) if (strlen($line) != $recSize) debug('bad ach line len: ' . strlen($line) . " - $line");

  $datetime = strftime('%Y%m%d%H%M'); // use full year (unlike ach record format)
  header('Content-type: text/plain');
  header("Content-Disposition: attachment; filename=$CGCODE.ach-$datetime.txt");
/**/  echo join("\n", $recs);
  exit();
}

/**
 * Return an ACH record for the given data and format.
 * @param assoc $values: field values, in order
 * @param assoc $lens: field lengths, in order
 * @return the field values, justified / filled / truncated appropriately, then concatenated.
 */
function achRecord($values, $lens) {
  $res = '';
  foreach ($lens as $k => $len) {
    $v = $values[$k];
    $res .= preg_match('/^[-,\\.0-9]+$/', $v) ? padnum($v, $len) : pad($v, $len); // (is_numeric would allow numbers with spaces)
  }
  return $res;
}

function pad($s, $n) {return substr(str_pad($s, $n), 0, $n);}
function padnum($n, $len) {return u\lpad(str_replace(',', '', number_format($n, 0)), $len);}
