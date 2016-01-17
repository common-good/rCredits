<?php
/**
 * @file
 * Top level application interfaces (SMS, web, smartphone, etc.) to rCredits.
 * These high-level business functions are called by more than one interface.
 */

namespace rCredits\Backend; // typically abbreviated as "be"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Backend as be;

define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows'); // used?

/**
 * Return the user's bank transactions for the given period (inclusive)
 * @param int $starting: starting time (the beginning of some day)
 * @param int $ending: ending time (the end of some day)
 * @param mixed $uid: account id for which to return transactions
 *   'ctty' means return transactions for the current account's whole community
 * @param int $jid: account id for joined account, if any
 * @param assoc $opts: array of options
 *   bool $dones: include completed transactions (default TRUE)
 *   bool $pendings: include open invoices (default TRUE)
 * @return the recordset (FALSE if none)
 */
function bankTransfers($starting, $ending, $uid, $jid = 0) {
  $mya = r\acct();

  $where = "t.completed BETWEEN :starting AND :ending ORDER BY t.completed DESC";
  $fields = "txid AS xid, amount, t.payer, :TX_BANK AS type, t.completed AS created, '' AS data, IF(t.amount<0, 1, 0) AS toMe";
  
  if ($uid == 'ctty') { // community or region
    $uid = $mya->community;
    $sql = <<<EOF
      SELECT $fields, '' AS purpose
      FROM r_usd t LEFT JOIN users u ON u.uid=t.payer
      WHERE u.community=:uid AND :jid=0 AND $where
EOF;
  } else $sql = "SELECT $fields FROM r_usd t WHERE payer IN (:uid, :jid) AND $where"; // normal account

  return db\q($sql, compact(u\ray('starting ending uid jid'))); 
}

/**
 * Return the user's transactions for the given period (inclusive)
 * @param int $starting: starting time (the beginning of some day)
 * @param int $ending: ending time (the end of some day)
 * @param mixed $uid: account id for which to return transactions
 *   'ctty' means return transactions for the current account's whole community
 * @param int $jid: account id for joined account, if any
 * @return the recordset (FALSE if none)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function myTransactions($starting, $ending, $uid, $jid = 0) {
  $mya = r\acct();
  $subs = compact(u\ray('starting ending uid jid'));
  $where = 't.created>=:starting AND t.created<=:ending'
  . ' AND t.type>=0' // omit invisible transactions
  . ' ORDER BY t.created DESC, t.xid DESC';
  
  if ($uid == 'ctty') { // all transactions in the community or region
    $uid = $mya->community;
    $sql = <<<EOF
      SELECT t.*, IF(taking, t.payeeFor, t.payerFor) AS purpose
      FROM r_txs t LEFT JOIN users u1 ON u1.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
      WHERE :uid IN (u1.community, u2.community) AND :jid=0
      AND $where
EOF;
  } else { // normal account (member or community)
    $isPayer = '(t.payer IN (:uid, :jid) AND t.payee<>:uid)';

    $sql = <<<EOF
      SELECT t.*,
      (SELECT fullName FROM users WHERE uid=IF($isPayer, t.payee, t.payer)) AS name,
      IF($isPayer, t.payerFor, t.payeeFor) AS purpose
      FROM r_txs t 
      WHERE ($isPayer OR t.payee IN (:uid, :jid))
      AND $where
EOF;
  }

  return db\q($sql, $subs); 
}

/**
 * Return the next relevant transaction for form 1099b, for an account.
 * @param int $payee: the account record ID
 * @return the next relevant row (NULL if no more)
 * Note that the IRS prohibits reporting non-positive transactions (but requires all the others). So we compromise by weeding out the transactions with "undoneBy" or "undoes" in the data field. But we return other negative transactions so we can warn the member to report the total negative amount on line 2, Form C.
 * Also note that this report is always for just one account (and for just one SSN or EIN) 
 *   even if the account is joint.
 */
function get1099b($payee) {
  global $q1099;
  
  if (!@$q1099) {
    $tome = ':payee=payee';
    $end = strtotime('january 1') - 1;
    $start = strtotime('-1 year', $end) + 1;
    $sql = <<<EOF
      SELECT created, data, serial,
        IF($tome, amount, -amount) AS amount, 
        IF($tome, payer, payee) AS uid, 
        IF($tome, payeeFor, payerFor) AS `for` ,
        $tome AS tome
      FROM r_txs WHERE :payee IN (payer, payee) AND created BETWEEN $start AND $end 
      AND (goods=:R_FOR_GOODS OR type<>:TX_TRANSFER)
      ORDER BY created
EOF;
//      AND (goods OR (type NOT IN (:TX_TRANSFER, :TX_REFUND)))
    $q1099 = db\q($sql, compact('payee'));
  }

  while ($row = $q1099->fetchAssoc()) {
    extract(u\just('data serial tome', $row));
    $data = unserialize($data);
    if (@$data['undoneBy'] or @$data['undoes'] or $serial == @$ignore) {
      $ignore = $serial; // ignore undone txs and their rewards
      continue;
    }
    if ($tome) return $row; // have to look at expenses, in case there's a reversed rebate, but don't return em
  }

  return $q1099 = NULL;
}

/**
 * Update relations
 * @param array $updates: list of records to update, indexed by reid
 *   each element is an associative array of new values, keyed by field name
 *   if a record is to be updated to all zeros, it is simply deleted.
 * @return string: a report of what got updated
 */
function updateRelations($updates) {
///  debug($updates);
  $myid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(u\just('main other fullName', $data['original'])); 
    unset($data['original']);
///    debug(compact('updates','data','employeeOk2'));
    
/*    if ($isAgent = ($myid != $main)) {
      r\reverseRelations($main, $other, $data, 'permission employee isOwner draw'); // giving employer permission?(!) -- create (or update?) a proper record (originally was just the employer's record)
    } else r\reverseRelations($main, $other, $data, 'employeeOk'); // claiming to be an employee?
*/    
    $reidSub = $subs = compact('reid');
    $changes = '';
    foreach ($data as $field => $value) {
//      if ($isAgent and $field != 'employeeOk') return u\tt('bad relations field'); // prevent hack
      $changes .= ($changes ? ',' : '') . "$field=:$field";
      $subs += array($field => $value);
      if ($field == 'draws' and $value) {
//        r\acct($main)->setBit(B_DRAWN);
        r\acct($other)->setBit(B_DRAWS);
      }
    }
    $where = 'reid=:reid';
    if (!empty($data)) db\q("UPDATE r_relations SET $changes WHERE $where", $subs);

    if (!r\relation('permission>0 OR employee OR isOwner OR draw', $where, $reidSub)) {
//      and (!r\relation('employee', 'main=:other AND other=:main', compact('main', 'other')))) {
      db\q("DELETE FROM r_relations WHERE $where", $reidSub);
//      $pot = '(:main, :other)';
//      if (!r\relation(1, "main IN $pot AND other IN $pot", compact('main', 'other'))) 
      $msg = 'deleted relation';
    } else $msg = 'updated relation';
    $otherName = $fullName;
    $report .= u\tt($msg, compact('otherName')) . '<br>';
  }
  return $report;
}

/**
 * Undo the given transaction.
 * Situations:
 *     I paid someone:            REVERSE (with reverse rewards)
 *     I charged someone:         REVERSE
 *     Someone paid me:           REVERSE
 *     Someone invoiced me and I approved it: DISPUTED
 *     Someone charged me unilaterally: DISPUTED
 * @param int $xid: the transaction number
 * @param string $where: additional (security check) criteria, if any
 * @param assoc $subs: additional substitions for $where
 * @param bool $force: <reverse despite any shortfall>
 * @return array [message, subs] where subs is an assoc of [
 */
function undoTx($xid, $where = '1', $subs = [], $force = FALSE) {
  $mya = r\acct();
  $oldRecord = lastTx("xid=:xid AND $where", compact('xid') + $subs);
///  debug(compact('xid','where','subs','oldRecord'));
  if ($error = r\nonoUndo($oldRecord, $subs)) return array($error, $subs); // check nono before confirm AND here

  extract($oldRecord, EXTR_PREFIX_ALL, 'old');
  $data = unserialize($old_data);

/* function undo_invoice($xid, $oldRecord, $old_otherUid, $byMe) // return $solution

    if ($old_byMe) {
      list ($new_state, $solution) = array(TX_DELETED, t('deleted')); // abandon the attempt to invoice
      r\notify($old_otherUid, $old_toMe ? 'invoice canceled' : 'offer canceled', r\txReportArgs($mya, $oldRecord));
    } else {
      list ($new_state, $solution) = array(TX_DENIED, t('marked "denied"'));
      r\notify($old_otherUid, $old_toMe ? 'offer refused' : 'invoice denied', r\txReportArgs($mya, $oldRecord));
    }
    r\setTxState($new_state, $xid);
    */
  if (!$old_toMe and !$old_byMe) { // someone charged (either unilaterally or I approved it)
// function dispute($xid, $oldRecord, $old_otherUid)
    r\disputeTx($xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($old_otherUid, 'charge disputed', r\txReportArgs($mya, $oldRecord));
  } else {
    $otherAcct = $old_toMe ? r\acct($old_payer, $old_payerAgent) : r\acct($old_payee, $old_payeeAgent);
    $type = $old_toMe ? 'charge' : 'payment';
    $otherTid = $old_toMe ? $old_payerTid : $old_payeeTid;
    return reverse($xid, $old_amount, $otherAcct, $data, $old_goods, $old_tid, $type, $otherTid, $force); // create an offsetting transaction
  }

  $amount = u\fmtAmt($old_amount);
  $tofrom = !$old_toMe ? t('to') : t('from');
  $otherUid = $old_otherUid;
  $other = r\acct($otherUid)->fullName;
  $success = TRUE;
  return ['report undo', compact(u\ray('success old_tid new_tid amount other tofrom solution otherUid'))];
}

/**
 * Create an offsetting transaction (nearly the same as the original but with a negative amount).
 */
function reverse($xid, $amount, $acct, $data, $old_goods, $tid, $type, $otherTid, $force) {
  $mya = r\acct();
///    print_r(compact(u\ray('xid amount acct data tid toMe'))); die();
  u\EXPECT(compact(u\ray('xid amount acct data tid type otherTid')), 'id float acct assoc|empty string string int'); // otherTid may be 0 (for reversing rewards)
  
  if (!is_array($data)) $data = [];
  $old_data = $data; // remember original data from transaction we're reversing
  extract(u\just('rebate bonus xfee', $data));
  $myFor = t('reverses #') . $tid;
  $otherFor = str_replace($tid, $otherTid, $myFor);
  $what = u\order($type == 'payment', $myFor, $otherFor);
  foreach (u\ray('rebate bonus xfee') as $k) if (@$$k) $data[$k] = -$$k;
  $data['undoes'] = $xid; // remember what we're reversing (especially to get the realness right)
  if ($force) u\preray(compact('force'), $data);

  list ($message, $args, $confirm) = transfer($type, $mya, $acct, -$amount, $old_goods, $what, NULL, TRUE, $data);

  if (@$args['success']) {
    extract(u\just('xid tid otherTid', $args), EXTR_PREFIX_ALL, 'new');
    $old_data['undoneBy'] = $new_xid;
    $old_data['changes'][] = array(r\rTime(), $mya->agentId, 'undo', $new_xid);
    $old_data = serialize($old_data);
    $myFor = "(reversed by #$new_tid) ";
    $otherFor = str_replace($new_tid, $new_otherTid, $myFor);
    list ($payerFor, $payeeFor) = u\order($type == 'payment', $myFor, $otherFor);
    $sql = <<<EOF
      UPDATE r_txs SET 
        payerFor=CONCAT(:payerFor, payerFor), 
        payeeFor=CONCAT(:payeeFor, payeeFor),
        data=:old_data
      WHERE xid=:xid
EOF;
    db\q($sql, compact(u\ray('payerFor payeeFor xid old_data')));
    $message = 'report undo|' . $message;
    $args['solution'] = 'reversed';
  }
  
  return array($message, $args);
}


/**
 * Pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @param string $msg: success or error message
 * @return TRUE if the payment was successful
 */
function payInvoice($nvid, &$msg = '') {
  if ($msg = r\badUnpaidInv($nvid, $inv)) return FALSE;
  extract(u\just('payer payee amount goods purpose', $inv));
  
  $DBTX = \db_transaction();
  $purpose .= " (rCredits inv#$nvid)";
  $data['inv'] = $nvid;
  list ($index, $subs) = be\transfer('payment', r\acct($payer), r\acct($payee), $amount, $goods, $purpose, NULL, TRUE, $data);
  if ($ok = @$subs['success']) db\q('UPDATE r_invoices SET status=:xid WHERE nvid=:nvid', u\ray('xid nvid', @$subs['xid'], $nvid));
  unset($DBTX);
  
  $msg = u\tt($index, $subs);
  return $ok;
}

/**
 * Create a transaction
 * call by: list ($msg, $args, $confirm) = be\transfer(...);
 * @param string $ttype: payment, charge, or invoice payment
 * @param acct $a1: acct object for first party (usually current user)
 * @param acct $a2: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param numeric $amount: the transaction amount
 * @param int $goods: 0=for USD, 1=for real goods and services, 2=other
 * @param string $for OR array($payerFor, $payeeFor): description of the transaction's purpose (user input)
 * @param int $created: time of transaction (defaults to current time)
 * @param bool $oked: has this transaction been confirmed by the user? (default TRUE)
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount) [rebate and bonus are always both set or neither]
 *   'undoes': transaction id of the related transaction (transaction being undone / invoice being paid)
 *   'force': <do the transaction despite any shortfall (1 for original transaction, -1 for forced reversal)>
 *   'inv': record number of the invoice for which this is payment
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: 
 *   $taking (charge vs. payment), 
 *   $oked (user confirmed the transaction request),
 *   $goods (the transaction is an exchange for real stuff),
 *   $shortfall (how inadequate the payer's balance is)
  */
function transfer($ttype, $a1, $a2, $amount, $goods, $for, $created = NULL, $oked = TRUE, $data = []) {
  global $scanned, $channel;

  u\EXPECT(in_array($ttype, u\ray('payment,charge,invoice payment')), 'bad transaction type');
  u\setDft($for, u\ray(R_WHYS)[$goods]);
  u\setDft($created, r\rTime());
  u\EXPECT(compact(u\ray('a1 a2 goods oked data')), 'acct acct int bool assoc');

  $type = TX_TRANSFER;
  list ($myid, $uid) = array($a1->id, $a2->id);
  $force = @$data['force'];
  $erInfo = compact(u\ray('ttype amount goods for force')) + u\ray('a1 a2', $a1->fullName, $a2->fullName);
  $taking = ($ttype == 'charge');
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loserA, $gainerA) = u\order($reallyTaking, $a2, $a1);
  list ($payer, $payee) = u\order($taking, $uid, $myid); // Analyze who's buying or selling
  list ($payerAgent, $payeeAgent) = u\order($taking, $a2->agentId, $a1->agentId);

  if (!$force and !@$data['inv'] and r\dupTx($uid, $myid, $taking ? $amount : -$amount)) return r\txRet('duplicate transaction', u\ray('op', $taking ? t('charged') : t('paid')));

  if ($err = r\txPermErr($a1, $a2, $taking, $amount < 0)) { // Check permissions
    if ($force) {
      $msg = 'forced without perm';
      foreach (['agent1'=>$a1, 'agent2'=>$a2] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$a1, $a2] as $a) {
        r\message($a->id, $msg, $erInfo);
        if (!$a->proSe) r\message($a->agentId, $msg, $erInfo);
      }
      r\tellStaff($msg, $erInfo);
    } else return r\txRet($err[0], $erInfo + $err[1]);
  }
  
  foreach ([$a1, $a2] as $a) if (!$a->co and !($channel == TX_POS ? $a->agent->photoId : $a->photoId)) {
    if ($force) {
      r\tellStaff('forced without photoId', $erInfo); 
    } elseif ($gainerA->id != r\cgfId()) { // and !$gainerA->coCan(CO_FAST)
      r\tellStaff('tx without photoId', $erInfo); 
//      return r\txRet($a == $a1 ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $a2->fullName]);
    }
  }
  
  // Check the amount and calculate any balance shortfall
  if ($error = u\badAmount($amount)) return r\txRet($error, $erInfo);
  $need = $amount - @$data['rebate']; // leave room for reversing rewards (minus a negative)

  $shortfall = (@$data['force'] or $loserA->id < 0) ? 0 : $loserA->shortfall($goods, abs($need), $oked, $a1);
  if ($shortfall) {
    $short = u\fmtAmt($shortfall);
    $otherName = $a2->fullName;
    $msg = $reallyTaking ? ($a2->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
    if ($channel == TX_WEB) {
      if ($msg == 'short to' and r\signedIn()) $msg .= '|increase min';
      if ($goods != R_FOR_GOODS) $msg .= '|short cash help|maybe not cash';
// disabled for now      if (!$loserA->can(B_DEBT) and $loserA->floor < 0) $msg .= '|try debt';
    }
    r\suggestMin($loserA);
    return r\txRet($msg, compact(u\ray('otherName short how')));
  }
  
  $info = compact(u\ray('type created amount payer payee payerAgent payeeAgent for data taking goods'));
  if (!$x = new r\x($info, $oked)) return r\txRet('tx save err', $erInfo);
  
  // Report results
  $args = r\txReportArgs($a1, $x->ray, TRUE, $shortfall);

  if ($oked) { // notify other party by email about any completed transaction
    foreach ([$a1, $a2] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED);
    $notice = $reallyTaking ? 'new charge'
      : (($amount < 0 and $goods == R_FOR_GOODS) ? 'new refund' : 'new payment');
    if ($notice == 'new payment' and $a2->co) { // not "new charge" (the most common)
      $atag = $a2->makeDo('addr', $myid, 0); // add a link to address, so merchant can ship or nonprofit can thank
      $args['myName'] = "<$atag>$args[myName]</a>";
    }
    if (@$args['otherRewardAmount']) $notice .= '|reward other';
    r\notify($uid, $notice, $args);
    $message = 'report tx' . ($goods == R_FOR_GOODS ? '|reward' : '');
  } else $message = $ttype;

  return r\txRet($message, $args, !$oked);
}

/**
 * Create an ivoice
 * call by: list ($msg, $args, $confirm) = be\invoice(...);
 * @param acct $a1: acct object for first party (usually current user)
 * @param acct $a2: acct object for the other party
 * @param numeric $amount: the invoiced amount
 * @param int $goods: 0=for USD, 1=for real goods and services, 2=other
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param bool $oked: has this invoice been confirmed by the user? (default TRUE)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *    confirm: boolean "this needs to be confirmed"
 */
function invoice($a1, $a2, $amount, $goods = R_FOR_USD, $purpose = NULL, $oked = TRUE) {
  if ($err = u\badAmount($amount, '>0')) return r\txRet($err);
  u\EXPECT(compact(u\ray('a1 a2 amount goods purpose oked')), 'acct acct float int string bool');
  u\setDft($purpose, $why = u\ray(R_WHYS)[$goods]);
  list ($payee, $payer) = array($a1->id, $a2->id);
  if ($err = r\txPermErr($a1, $a2, TRUE)) return r\txRet($err);
  if (r\dupTx($payee, $payer, -$amount)) return r\txRet('duplicate transaction', ['op' => t('charged')]);
  if (!$a1->co and !$a1->photoId) return r\txRet('no photoid');
  if (!$a2->co and !$a2->photoId) return r\txRet('other no photoid', ['who' => $a2->fullName]);
  
  $subs = u\ray('myName otherName amount why purpose did', $a1->fullName, $a2->fullName, u\fmtAmt($amount), $why, $purpose, t('charged'));

  if ($oked) {
    $created = r\rTime();
    $info = compact(u\ray('amount payer payee goods purpose created'));
    $nvid = db\insert('r_invoices', $info);
    $a1->update('lastTx', [$nvid, $payer - $payee, round(-$amount, 2), $created, 'inv']);

    $subs['a1'] = $a1->makeDo('inv', $nvid); // link to pay/deny
    //$a2->update(u\ray('lastTx', array($nvid, $payee - $payer, round($amount, 2), $created, 'inv')));
    r\message($payer, 'new invoice', $subs, TRUE); // tell other party by email about this invoice
    $msg = 'report tx' . ($amount > 0 ? '|balance unchanged' : '');
    return array($msg, $subs + ['success'=>TRUE], FALSE);
  } else return array('charge', $subs, TRUE);
}

/**
 * Record a transfer (of rCredits) to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer
 * @param int $amount: how much to transfer
 * @param string $for: purpose of funding
 * @param bool $oked: actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE on failure)
 */
function fund($uid, $type, $amount, $for, $oked = TRUE) {
  $payeeAgent = $payee = $uid;
  $mya = r\acct(); // (null, if just registering or running as cron)
  $isReward = r\isReward($type);
  u\EXPECT($isReward or $mya->can(B_MANAGING_CTTY), 'non-automatic reward by non ctty agent');
  if ($type == TX_FINE) $amount = -$amount;
  if (!$oked) return $amount;
  
  $payer = r\acct($payee)->community;
  $payerAgent = (@$mya and $mya->id < 0) ? $mya->agentId : $payer; // take note if funded by community admin
  if (!new r\x(compact(u\ray('type amount payer payee payerAgent payeeAgent for taking')))) return FALSE;
  $types = u\ray(TX_TYPES);
  $thing = $isReward ? t('reward') : $types[$type];
  $subs = u\ray('amount purpose thing', u\fmtAmt(abs($amount)), $for, $thing);
  $index = $type == TX_FINE ? 'got fine' : 'got funding';
  if ($type == TX_INFLATION) $index .= '|inflation details';
  r\notify($uid, $index, $subs);
  return $amount;
}

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function addCell($number, $validate = FALSE) {
  $mya = r\acct();
  $myid = $mya->id;

  $numberInternal = u\fmtPhone($number, '+n');
  $numberPretty = u\fmtPhone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$numberPretty) return array('bad phone', $subs);
    if ($result = db\q('SELECT * FROM r_boxes WHERE code=:numberInternal', compact('numberInternal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return array('already cell', $subs);
      $accountName = accountName($old_uid);
      if (!isTempAccount($old_uid)) return array('cell taken', compact(u\ray('number accountName')));
    }
    return NULL;
  }
  
  //$status = SMS_PRIMARY;
  r\makeDevice($myid, $numberInternal, TX_SMS);
  if (!$mya->phone) $mya->update('phone', $numberInternal); // use it as contact phone, if none yet
  return array('report new cell', u\ray('number', $numberPretty));
}

function deleteCell($number) {
  $number = u\fmtPhone($number, '+n'); // internal format
  db\q('DELETE FROM r_boxes WHERE code=:number LIMIT 1', compact('number'));
  $number = u\fmtPhone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant.
 *
 * @param string $who: how the user referred to the trader (name, phone, email, uid, or temporary code)
 * @param string $self_message: index to error message for identifying oneself
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible fullNames (0 or more) indexed by uid)
 *   (the caller can test which type of return, with "if (isAcct($returned_value))
 */
function identify($who, $self_message = 'no self-trading') {
  $create = FALSE; // not currently used
  $who = trim($who);
  $myid = r\acct()->id;

  if ($phone = u\fmtPhone($who, '+n')) {
    $who = $phone; $cphone = u\cryptN($phone);
  } else $cphone = '(none)';
  
  $quid = strtoupper($who); // in case this is a quid (abbreviated or not)
  if (preg_match('/^([A-Z]{3})?[\.\:=][A-Z]{3,4}( |$)/i', $quid, $match)) { // allow quids or abbrevs within this region
    $pre = $match[1] ? '' : R_SERVER_ID; // region name was omitted
    list($quid) = explode(' ', "$pre$quid ", 2); // take just the first word, in case this is from a list showing QUID and fullName
  }

  if (u\isQid($quid)) {
    if (!$acct = r\acct($quid)) return array('unknown member', compact('who'), []);
    if ($acct->id == $myid) return array($self_message, [], []);
    return $acct; // qids are easy
  }

  if (($short = u\shortName($who)) === '') $short = '-'; // abbreviations must not be null
  $cmail = strpos($who, '@') ? u\cryptN($who) : '(none)';
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid,u.fullName
    FROM users u LEFT JOIN r_boxes s ON u.uid = s.uid 
    WHERE (:who=s.code OR INSTR(u.name, :short) OR (!:IS_NOSEARCH AND (:cphone=u.phone OR :cmail=u.mail)))
    AND u.uid>0
    ORDER BY u.fullName
EOF;

  $result = db\q($sql, compact(u\ray('who short cphone cmail')))->fetchAllKeyed(0, 1);
  if (count($result) == 1) {
    if (@$result[$myid]) return [$self_message, [], []];
    return r\acct(key($zot = $result)); // get the one entry (zot resets the pointer)
  } else unset($result[$myid]); // self is one of many results, so ignore it

  $msg = $result ? 'ambiguous other' 
  : ('unknown member' . ((u\starts($cphone, CRYPTN_FLAG) or u\starts($cmail, CRYPTN_FLAG)) ? '|no search' : ''));
  return [$msg, compact('who'), $result];
}

/**
 * Return financial information about the given account on this server
 * @param assoc $args: options:
 *   uid: uid for the account to return information for (DEFAULTS to myid)
 *   pending: show pending transactions (applicable only to bank transfers)
 *   asof: last date to include (defaults to 24 hours from now, meaning unrestricted)
 *   noj: ignore any joined account
 * @return
 *   standard object of credit account information (see $resultKeys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function creditInfo($args = []) {
  extract(u\just('uid pending asof noj', $args));
  if (!$a = r\acct(@$uid)) return NULL; else $uid = $a->id;
  $jid = @$noj ? 0 : ($a->jid ?: 0);

  u\setDft($asof, $asof0 = r\rTime() + DAY_SECS);
  $subs = compact(u\ray('uid jid asof'));
  $isPayer = 't.payer IN (:uid, :jid)';
  $isPayee = 't.payee IN (:uid, :jid)'; // account can be both payer and payee, if joint account
  
  $sql = <<<EOF
    SELECT 
    SUM(IF($isPayee, amount, 0)) AS `in`,
    SUM(IF($isPayer, amount, 0)) AS `out`, 
    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rewards,
    SUM(IF(t.type=:TX_XFEE, IF($isPayer, -amount, amount), 0)) AS xfees,
    SUM(IF(:HAS_XFEE, IF($isPayer, -amount, amount), 0)) AS xin
    FROM r_txs t
    WHERE ($isPayer OR $isPayee) AND t.created<=:asof
EOF;
  
  $sums = @$pending ? u\ray('in out rewards xfees xin', 0, 0, 0, 0, 0) : db\q($sql, $subs)->fetchAssoc();

  $where = @$pending ? 'created<=:asof AND NOT completed' : 'completed AND completed<=:asof';
  $sql = "SELECT SUM(amount) AS bank FROM r_usd t WHERE $isPayer AND $where";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $key => $value) $sums[$key] = $value + 0; // no nulls
  extract($sums);

  $bank = -$bank; // make it "from bank" instead of "to bank"
  $r = ($in - $out) + $bank;
  $in -= $xin; $out -= $xin;
  $outProper = $out;
  $inProper = $in - $rewards;
  
  // figure savings
  if ($asof == $asof0) {
    $savings = $a->j_savings;
  } else {
    $savings = $rewards; // tentative savings before additions ( = oldSavings - oldSavingsAdd)
    foreach ($jid ? [$a, r\acct($jid)] : [$a] as $ai) if ($chs = $ai->changes) {
      $addOLD = $ai->savingsAdd; // start with present
      foreach ($chs as $dt => $info) if ($dt > $asof) $addOLD = @$info['savingsAdd']; else break;
      $savings += $addOLD;
    }
  }
  $balance = $r - $savings;

  $resultKeys = 'in out rewards refunds fees xfees xin inProper outProper balance r savings bank';
  $result = compact(u\ray($resultKeys));
  
  $fancy = [];
  foreach ($result as $k => $v) {
    $result[$k] = $v = round($v, 2);
    $fancy[$k] = u\fmtAmt($v);
  }
  $result['fancy'] = $fancy;
///debug($result);
  return (object) $result;
}

/**
 * Return a list of account choices for the current user (agent of the current account)
 * @return an array of account ids (FALSE if none)
 */
function accountChoices() {
  if (!$mya = r\acct()) return FALSE;
  $result = [$aid = $mya->agentId];
  $nameOrder = "IF(MID(fullName, 2, 2)='. ', MID(fullName, 4), IF(MID(fullName, 1, 4)='the ', MID(fullName, 5), fullName))"; // ignore "the " and initialled first name (eg "X. ")
  if ($mya->admin) {
    $q = @db\q("SELECT uid FROM users WHERE ABS(uid)>1 ORDER BY (uid<0) DESC, $nameOrder");
  } elseif ($mya->cAdmin) {
    $q = @db\q("SELECT uid FROM users WHERE community=:ctty ORDER BY (uid<0) DESC, $nameOrder", ['ctty' => $mya->community]);
  } else $q = @db\q('SELECT main FROM r_relations WHERE other=:aid AND permission>0', compact('aid'));
///  debug($result + $q->fetchAllKeyed(0, 0));
  return array_values($result + $q->fetchAllKeyed(0, 0));
}

/**
 * Remember what to do, once the user confirms.
 *
 * @parm string $number: the user's cell number (in standard format +1dddddddddd)
 * @parm string $todo: what to do once the user responds with the expected nonce (defaults to 'nothing')
 * @parm string $nonce: what to store as the nonce (defaults to 'whatever', meaning pick one at random)
 *
 * @return object(nonce, todo) if $todo is 'nothing', else the nonce
 */
function todo($number, $todo = 'nothing', $nonce = 'whatever') {
  if ($todo == 'nothing') {
    return (object) db\lookup('nonce,todo', 'r_boxes', 'code=:number', compact('number'));
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('r_boxes')->fields(compact('nonce', 'todo'))->condition('code', $number)->execute();
    u\EXPECT(todo($number)->nonce == $nonce, 'setting nonce failed');
    return $nonce;
  }
}

/**
 * Return the last transaction meeting the given criteria, for the current user.
 * Rebates, bonuses, and deleted transactions are ignored
 * @param string $where: the criteria (defaults to 1 = any)
 * @param array $subs: optional substitutions (will be prefixed with colons)
 * @return associative array: the old transaction record (FALSE if none)
 */
function lastTx($where = 1, $subs = [], $cache = FALSE) {
  $myid = r\acct()->id;
  $basic = "$myid IN (payer, payee) AND type>=0"; // make sure it's for $myid
  $sql = "SELECT * FROM r_txs WHERE ($basic) AND ($where) ORDER BY xid DESC LIMIT 1";
///  debug(compact('where','subs','myid','basic','sql'));
  if (!$result = db\q($sql, $subs)->fetchAssoc()) return FALSE;
  $toMe = ($result['payee'] == $myid);
  $byMe = ($result['taking'] == $toMe);
  list ($tid, $otherTid) = u\order($toMe, $result['payeeTid'], $result['payerTid']);
  $otherUid = $result[$toMe ? 'payer' : 'payee'];
  return $result + compact(u\ray('byMe toMe tid otherTid otherUid'));
}

function accountName($uid) {
  $acct = r\acct($uid);
  return $acct->fullName . ' ' . u\hug($acct->mainQid);
}

function isTempAccount($uid) {
  return !r\acct($uid)->pass;
}

function isTempName($name) {
  return u\abbreviates(R_TEMP_NAME_PREFIX, $name);
}
