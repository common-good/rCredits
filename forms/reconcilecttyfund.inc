<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Upload a bank statement (or two) to reconcile the community fund.
 * Handle two files, in case the Community Fund is split into INCOMING and OUTGOING
 */
function formReconcileCttyFund($form, &$sta) {
  $title = t('Reconcile Community Fund');
	$subtext = t('Upload a Community Fund bank statement file (in OFX format). Or two files, if the Community Fund is split.');
  if ($reconciledAsOf = getv('$reconciledAsOf') and !$txs = getv('ofxTxs')) $buttons = offerOffsetTransfer($reconciledAsOf);
  $file1 = fileFld();
  $file2 = fileFld();
  $submit = t('Upload and Reconcile');

  return cgform(compact(ray('title subtext buttons file1 file2 submit')));
}

function formReconcileCttyFund_validate($form, &$sta) {
 	$txKs = ray(OFX_TX_FLDS);

  foreach ([1, 2] as $i) {
    $flnm = w\uploadFile($err, "file$i");
		if (!$flnm) {
      if ($i == 1) return w\say($err, 'file1'); else break;
    }
	  $s = file_get_contents($flnm);

		if (!$acct[$i] = ofxData('bankId acctId', $s)) return w\say(t('File #%i is not a valid OFX file.', $subs = compact($i)), 'file1');
		if (!$dts[$i] = ofxData('dtStart dtEnd', $s)) return w\say(t('File #%i is not a valid OFX file.', $subs), 'file1');
		if (!$ofxBal[$i] = ofxData('balAmt dtAsOf', $s)) return w\say(t('No bank balance amount found in file #%i.', $subs), 'file1');
		if (!$txs[$i] = ofxData(OFX_TX_FLDS, $s, TRUE)) return w\say(t('No transactions found in file #%i.', $subs), 'file1');
    
    foreach (ray('acct dts ofxBal') as $k) foreach(${$k}[$i] as $k2 => $v2) ${$k2 . $i} = $v2; // bankId2, etc.
	}

  if (@$acct[2]) {
    if ($dtEnd1 != $dtEnd2) return w\say(t('Ending date must be the same for both downloaded OFX files.'), 'file1');
    foreach (ray('bankId acctId') as $k) $acct[$k] = u\unite($acct[1][$k], $acct[2][$k]);
    $dts['dtStart'] = min($dtStart1, $dtStart2);
    $dts['dtEnd'] = $ofxBal['dtAsOf'] = $dtEnd1;
    $ofxBal['balAmt'] = $balAmt1 + $balAmt2;
    $txs = array_merge($txs[1], $txs[2]); // append one set of transactions to the other
  } else foreach (ray('acct dts ofxBal txs') as $k) $$k = ${$k}[1];
  
  foreach ($txs as $t => $tx) $index[$t] = $tx[1]; // create index of dates
  asort($index); // sort index by date
  foreach ($index as $t => $zot) $txs2[] = $txs[$t]; // create single set of transactions in date order
  $sta['input'] += $acct + $dts + $ofxBal + ['txs' => serialize($txs2)];

}

// update r_usd set bankTxId=0
function formReconcileCttyFund_submit($form, &$sta) {
  global $mya;

  if (!db\exists('r_usd', 'bankTxId<>0')) { // one-time setup
  	$tot = -8045;
	  db\q('UPDATE r_usd SET bankTxId=0; TRUNCATE r_usd2;'); // debug(
	  db\q('UPDATE r_usd SET bankTxId=:bxid WHERE deposit<:y2015', ray('bxid y2015', bankTxId('13265328601201412310000001'), strtotime('1/1/2015')));
	} elseif(db\exists('r_usd', 'bankTxId<>0 AND deposit>1489550400')) {
		$tot = 59080.04;
    db\q('UPDATE r_usd SET bankTxId=0 WHERE deposit>1489550400');
		db\q('DELETE FROM r_usd2 WHERE id>49');
	} else $tot = 113788.57;

  //select sum(amount) from r_usd where deposit<1420088400
	extract(just('bankId acctId dtStart dtEnd balAmt dtAsOf txs', $sta['input']));
	$txKs = ray(OFX_TX_FLDS);
	$batchSql = "CONCAT(IF(txid<0,'B', IF(amount<0,'O','I')), deposit)";
	$sql = "SELECT $batchSql,SUM(amount) FROM r_usd where amount<>0 and deposit<>0 AND bankTxId=0 GROUP BY $batchSql ORDER BY deposit,$batchSql";
//	$sql = 'SELECT deposit AS dt,SUM(amount) AS amt FROM r_usd WHERE amount>0 AND deposit<>0 AND bankTxId=0 GROUP BY deposit,amount<0';
	$batches = db\q($sql)->fetchAllKeyed(); // summarize batches recorded in r_usd
	$lastBatch = end((array_keys($batches))); // extra parens not needed in PHP 7
	
	$txs = unserialize($txs);
  $ofxOnly = $batOnly = 0;

	foreach ($txs as $t => $tx) { // loop through txs in the OFX file downloaded from the bank
		extract($txs[$t] = $tx = array_combine($txKs, $tx)); 

		$trnAmt += 0; // convert from string
		$dt = strtotime($dtPosted);
		$extra = in($trnType, strtoupper('xfer fee srvchg other credit'));
		$amount = u\fmtAmt($trnAmt);
		$txDesc[$fitId] = tr('%trnType: #%fitId %amount (%dt) %name (%memo)', compact(ray('trnType fitId amount dt name memo')));

    if (reversing($name, $trnType)) continue; // handle reversals in a separate loop
    
		$where = 'bankTxId<>0';
  	$bal = db\sum('amount', 'r_usd', $where) + db\sum('amount', 'r_usd2', $where);
    // $errs[] = ("before amt=$trnAmt ($dtPosted): bal=$bal tot=$tot" . (round($bal - $tot) == 0 ? '' : ' DIFFERENT!'));
	  $tot += $trnAmt;


		$bankTxId = bankTxId($fitId);
		$table = '(SELECT bankTxId FROM r_usd UNION SELECT bankTxId FROM r_usd2) s';
//    $table = '(SELECT amount,bankTxId,deposit FROM r_usd UNION SELECT amount,bankTxId,completed AS deposit FROM r_usd2) t';
		if (db\exists($table, compact('bankTxId'))) { // already reconciled
      $tot -= $trnAmt; 
      unset($txs[$t]);
//       $errs[] = "Tx $fitId already done.";
      continue;
    };
		
		if ($extra) { // fee, credit, inter-accont transfer, or other activity irrelevant to Community Fund balance
      db\insert('r_usd2', ray('amount completed bankTxId type memo', $trnAmt, $dt, $bankTxId, $trnType == 'XFER' ? 'T' : 'S', $memo));
      unset($txs[$t]);
			continue;
		} elseif (!in($trnType, 'DEP DIRECTDEP DEBIT')) {
      $errs[] = t('Bad type in OFX file -- ') . $txDesc[$fitId]; 
      $tot -= $trnAmt;
      $ofxOnly += $trnAmt;
      unset($txs[$t]);
      continue;
    }
		
		foreach ($batches as $batch => $batAmt) { // find this bank deposit among our record of ctty fund deposits (transfers to CG accounts)
			$batAmt += 0; // convert from string
			$batDt = substr($batch, 1) + 0; // unix date of batch
      if ($batDt >= $dt) break; // bank can't receive a deposit before we send it
      
			if ($batAmt == $trnAmt and $batch[0] != 'B') { // reversals are handled below
				$sign = $batAmt < 0 ? '<0' : '>0';
				db\q("UPDATE r_usd SET bankTxId=:bankTxId WHERE deposit=:batDt AND amount$sign AND txid>0", compact('bankTxId', 'batDt'));
				unset($batches[$batch]); // leave only the unmatched batches in the array
        unset($txs[$t]);
				break;
			}
		}
	}

	foreach ($txs as $t => $tx) {	// go through the remaining unmatched transactions in OFX, seeking individual transfers (usually reversals)
    extract($tx);
		$dt = strtotime($dtPosted);
    $txidSign = reversing($name, $trnType) ? '<0' : '>0';
  	$where = ":dt BETWEEN deposit AND deposit+1*:MONTH_SECS AND amount=:trnAmt AND txid$txidSign AND bankTxId=0 ORDER BY deposit";
		if ($ray = db\get('txid,deposit', 'r_usd', $where, $subs = compact('dt', 'trnAmt'))) {
      extract($ray);
      db\update('r_usd', compact('bankTxId', 'txid'), 'txid');
      $bi = ($txidSign == '<0' ? 'B' : ($trnAmt < 0 ? 'O' : 'I')) . $deposit;
      $batAmt = @$batches[$bi];
      u\EXPECT($batAmt, 'logic error in reconcile reversal');
      if ($batAmt - $trnAmt) { // this was a batch with more than one transaction
        $batches[$bi] -= $trnAmt; // make the rest of this batch recognizable
      } elseif ($batAmt) unset($batches[$bi]);
      unset($txs[$t]);
    } else {
      $errs[] = tr(str_replace(':', '%', $where), $subs);
      $errs[] = t('OFX file shows a transaction with no matching batch -- ') . $txDesc[$fitId];
      $tot -= $trnAmt;
      $ofxOnly += $trnAmt;
    }
    $where = tr(str_replace(':', '%', $where), $subs);
	}
  
  foreach ($batches as $batch => $batAmt) {
		$batDt = substr($batch, 1) + 0; // unix date of batch
		$errs[] = t('USD transfer batch does not appear in OFX file: %amt (%date)', 'amt date', u\fmtAmt($batAmt), u\fmtDate($batDt, TRUE));
    $batOnly += $batAmt;
  }
	
  $asOf = strtotime($dtAsOf);
  setv('$reconciledAsOf', $asOf);
  setv('batches', $batches);
  setv('ofxTxs', $txs);
	if (@$errs) say(join('<br>', $errs));
  $where = 'bankTxId<>0 AND deposit<=:NOW+:DAY_SECS';
//  $bal = db\sum('amount', $table, $where); // <=:NOW is a placeholder for when we want earlier balances
	$bal = db\sum('amount', 'r_usd', $where);
	$bal += db\sum('amount', 'r_usd2', str_replace('deposit', 'completed', $where));
	w\say(t('<p>Reconciled Community Fund balance as of today is %bal.</p>', 'bal', u\fmtAmt($bal)));
	w\say(t('<p>Bank reports a balance of %ofxBal as of %asOf.</p>', 'ofxBal asOf', u\fmtAmt($balAmt), u\fmtDate($asOf, TRUE)));
	w\say(t('ofxOnly=%ofxOnly batOnly=%batOnly', compact(ray('tot ofxOnly batOnly'))));
  if ($batches or $txs) w\go("/sadmin/reconcile-ctty-fund/manually/asOf=$asOf");
}

/**
 * Extricate the specified parameters from the given OFX file contents.
 * @param string $ks: list of parameter names (case-insensitive)
 * @param string $s: contents of an OFX file
 * @return matches found
 */
function ofxData($ks, $s, $multi = FALSE) {
	foreach ($ks = ray($ks) as $k) $pat[] = "<$k>([^\\r\\n]*)";
	$pat = join('\s*', $pat);
	if ($multi) {
	  if (!preg_match_all("/$pat/ism", $s, $m, PREG_SET_ORDER)) return FALSE;
		foreach ($m as $i => $one) $m[$i] = array_slice($m[$i], 1);
		return $m;
	} else return preg_match("/$pat/ism", $s, $m) ? array_combine($ks, array_slice($m, 1)) : FALSE;
}

function reversing($name, $trnType) {
  return (($trnType == 'DEBIT' and strpos($name, 'DDA DEBIT') !== FALSE) or strpos($name, 'RETURN SETTLE') !== FALSE);
}    

/**
 * Display some basic facts about the Dollar Pool's reconciledness and return a button to create an offset transfer.
 */
function offerOffsetTransfer($asOf) {
  w\say(t('There are no unmatched downloaded (OFX) transactions.'));
  if ($offsets = db\sum('amount', 'r_usd2')) { // include both reconciled and unreconciled
    if ($offsets -= r\acct(DOLLAR_POOL)->waitingFromBank()) {
      $button = btn('', t('Request a transfer of %amt from Checking to cover fees and bank errors', 'amt', u\fmtAmt($offsets)), 'warning', 'xs', ray('id', 'dp-offset'));
    } else say(t('All offset transfers have been completed and reconciled.'));
  }
  $btidTomorrow = floor($asOf / DAY_SECS) + 1;
  $btidTomorrow = 0 + ($btidTomorrow . str_repeat('0', isDEV ? 5 : 12));
  $dpBal = $offsets + db\sum('amount', 'r_usd', 'bankTxId<>0 AND bankTxId<:btidTomorrow', compact('btidTomorrow'));
  say(t('Total Dollar Pool balance as of %dt is %amt', 'dt amt', u\fmtDate($asOf, TRUE), u\fmtAmt($dpBal)));
  jsx('dollar-pool-offset', 'amount', @$offsets);
 
  return @$button;
}