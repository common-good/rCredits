CURRENTLY UNUSED (Dec 2014)

/**
 * Create an ACH request file (debits only, for now).
 */
function achify() {
  $count = $hash = $credits = $debits = 0;
  
  $tdRoute = '21127445'; // sending bank's routing number without the final "0"
  $now = strftime('%y%m%d%H%M');
  $fileId = 'A'; // distinguishes files on the same date and between the same participants (A-Z, 0-9)
  $dest = pad('TD BANK NA', 23);
  $source = pad('SOC TO BENEFIT EVERYONE', 23);
  $ref = pad('', 8);
  $recs[] = "101 {$tdRoute}0" . '1' . CGF_EIN . "$now{$fileId}094101$dest$source$ref"; // file header
  
  $class = '200'; // mixed credits and debits (225 for debits only, 220 for credits only)
  $coName = pad(strtoupper('RCREDITS SYSTEM'), 16); // 'SOC BENFT EVRYON'? 'COM GOOD FINANCE'? 'COMMON GOOD FIN'?
  $extra = pad('', 20); // discretionary (we can use this space for anything)
  $code = 'PPD'; // prearranged payment and deposit
  //$purpose = pad($amount > 0 ? t('rCredits') : t('money out'), 10);
  $purpose = pad(t('rCredits+-'), 10);
  $when = pad(strftime('%b %d'), 6); // descriptive date
  $effective = strftime('%y%m%d'); // requested settlement date
  $batch = lpad(1, 7); // should this be unique for all our batches ever? or just within the file?
  $recs[] = "5$class$coName$extra" . '1' . CGF_EIN . "$code$purpose$when$effective" . "0001$tdRoute$batch"; // co batch header

  $longAgo = REQUEST_TIME - DAY_SECS * 10;
  db\q('UPDATE r_ach SET completed=-9 WHERE created<=:longAgo', compact('longAgo')); // stop trying if too old
  $q = db\q('SELECT txid,uid,amount FROM r_ach WHERE completed<=0 AND completed>-:R_ACH_TRIES ORDER BY txid');
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($amount < 0) continue; // ignore these while the system is young (admin may want to dissuade the member)
    $a = r\acct($uid);
    if (!$a->bank or !$a->ok) continue;
    extract(u\just('bankAccount bankAccountType', $a->secure));
    if (!@$bankAccount) continue;
    list ($route, $account) = u\parseBankAccount($bankAccount);
    $code = @$bankAccountType == 'checking' ? ($amount < 0 ? 22 : 27) : ($amount < 0 ? 32 : 37);
    $account = pad($account, 17);
    if ($amount < 0) $credits -= $amount; else $debits += $amount;
    $amount = padnum($amount, 10);
    $acctId = pad(($a->co ? 'EIN #' : 'SSN #') . $a->federalId, 15);
    $name = pad($a->legalName, 22);
    $extra = pad('', 2); // discretionary
    $trace = $tdRoute . lpad($txid, 7); // 15 chars total
    $recs[] = "6$code$route$account$amount$acctId$name$extra" . "0$trace"; // one ach request
    $count += 1;
    $hash = substr($hash + $route, 10);
  }
  
  if (!$count) return; // must be separate line from padding
  
  $count = lpad($count, 6);
  $hash = lpad($hash, 10);
  $debits = padnum($debits, 12);
  $credits = padnum($credits, 12);
  $auth = lpad('?', 8) . lpad('', 11); //
  $b6 = pad('', 6);
  $recs[] = "8$class$count$hash$debits$credits" . '1' . CGF_EIN . "$auth$b6$tdRoute$batch";  // batch control

  $batches = lpad(1, 6);
  $blocks = lpad(ceil((2 + $count + 2) / 10), 6); // how many blocks of 10 lines, rounded up
  $count = lpad($count, 8);
  $b39 = lpad('', 39);
  $recs[] = "9$batches$blocks$count$hash$debits$credits$b39"; // file control
  
  //foreach ($recs as $line) u\EXPECT(strlen($line) == 94, 'bad ach line len: ' . $line);
  foreach ($recs as $line) if (strlen($line) != 94) deburg('bad ach line len: ' . strlen($line) . " - $line");
  file_put_contents(__DIR__ . "/../ach-$now.txt", join("\n", $recs));
  r\tellStaff('ach ready');
}

function pad($s, $n) {return substr(str_pad($s, $n), 0, $n);}
function lpad($s, $n) {return str_pad(substr($s, 0, $n), $n, '0', STR_PAD_LEFT);}
function padnum($n, $len) {return lpad(str_replace('.', '', abs($n)), $len);}
/**/ function log($msg) {file_put_contents('cron.log', print_r($msg, 1) . "\n" . @file_get_contents('cron.log'));}