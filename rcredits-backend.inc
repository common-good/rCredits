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
 * @param mixed $uid: account id for which to return transactions (defaults to current account)
 *   'ctty' means return transactions for the current account's whole community
 * @param assoc $opts: array of options
 *   bool $dones: include completed transactions (default TRUE)
 *   bool $pendings: include open invoices (default TRUE)
 * @return the recordset (FALSE if none)
 */
function bankTransfers($starting, $ending, $uid = NULL) {
  $mya = r\acct();

  if ($uid == 'ctty') { // otherwise $uid can be positive for a member account or negative for a ctty account
    $uid = $ctty = $mya->community;
  } elseif (!@$uid) $uid = $mya->id;
  
  $where = "t.completed BETWEEN :starting AND :ending ORDER BY t.completed DESC";
  $fields = "CONCAT('b', tid) AS xid, amount, t.payer, t.payee, :TX_BANK AS type, t.completed AS created, '' AS data, IF(t.amount<0, 1, 0) AS toMe";
  
  if (@$ctty) { // community or region
    $sql = <<<EOF
      SELECT $fields, '' AS purpose
      FROM r_usd t LEFT JOIN users u1 ON u1.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
      WHERE (u1.community=:uid OR u2.community=:uid) AND $where
EOF;
  } else $sql = "SELECT $fields FROM r_usd t WHERE payer=:uid AND payee=0 AND $where"; // normal account

  return db\q($sql, compact(u\ray('starting ending uid'))); 
}

/**
 * Return the user's transactions for the given period (inclusive)
 * @param int $starting: starting time (the beginning of some day)
 * @param int $ending: ending time (the end of some day)
 * @param mixed $id: account id for which to return transactions (defaults to current account)
 *   'ctty' means return transactions for the current account's whole community
 * @return the recordset (FALSE if none)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function myTransactions($starting, $ending, $id = NULL) {
  $mya = r\acct();
  
  if ($id == 'ctty') { // otherwise $id can be positive for a member account or negative for a ctty account
    $id = $ctty = $mya->community;
  } elseif (!@$id) $id = $mya->id;
  
  $where = 't.created>=:starting AND t.created<=:ending'
  . ' AND t.type>=0' // omit invisible transactions
  . ' ORDER BY t.created DESC, t.xid DESC';
  
  if (@$ctty) { // all transactions in the community or region
    $sql = <<<EOF
      SELECT t.*, IF(taking, t.payeeFor, t.payerFor) AS purpose
      FROM r_txs t LEFT JOIN users u1 ON u1.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
      WHERE (u1.community=:id OR u2.community=:id)
      AND $where
EOF;
  } else { // normal account (member or community)
    $sql = <<<EOF
      SELECT t.*,
      (SELECT fullName FROM users WHERE uid=IF(t.payer=:id, t.payee, t.payer)) AS name,
      IF(t.payer=:id, t.payerFor, t.payeeFor) AS purpose
      FROM r_txs t 
      WHERE :id IN (t.payer, t.payee)
      AND $where
EOF;
  }

  return db\q($sql, compact(u\ray('starting ending id'))); 
}

/**
 * Return the next relevant transaction for form 1099b, for an account.
 * @param int $payee: the account record ID
 * @return the next relevant row (NULL if no more)
 * Note that the IRS prohibits reporting non-positive transactions (but requires all the others). So we compromise by weeding out the transactions with "undoneBy" or "undoes" in the data field. But we return other negative transactions so we can warn the member to report the total negative amount on line 2, Form C.
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
      AND (goods OR (type NOT IN (:TX_TRANSFER, :TX_REFUND)))
      ORDER BY created
EOF;
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
      setCttyAdmin(FALSE, $main, $other);
      db\q("DELETE FROM r_relations WHERE $where", $reidSub);
//      $pot = '(:main, :other)';
//      if (!r\relation(1, "main IN $pot AND other IN $pot", compact('main', 'other'))) 
      $msg = 'deleted relation';
    } else {
      setCttyAdmin(@$data['permission'] >= r\perm(B_MANAGE), $main, $other);
      $msg = 'updated relation';
    }    
    $otherName = $fullName;
    $report .= u\tt($msg, compact('otherName')) . '<br>';
  }
  return $report;
}

function setCttyAdmin($value, $main, $other) {
  if ($main < 0 and $main == r\acct($other)->community) r\acct($other)->setBit(B_CADMIN, $value);
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
function undoTx($xid, $where = '1', $subs = array(), $force = FALSE) {
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
  return array("report undo", compact(u\ray('success old_tid new_tid amount other tofrom solution otherUid')));
}

/**
 * Create an offsetting transaction (nearly the same as the original but with a negative amount).
 */
function reverse($xid, $amount, $acct, $data, $old_goods, $tid, $type, $otherTid, $force) {
  $mya = r\acct();
///    print_r(compact(u\ray('xid amount acct data tid toMe'))); die();
  u\EXPECT(compact(u\ray('xid amount acct data tid type otherTid')), 'id float acct assoc|empty string string id');

  if (!is_array($data)) $data = array();
  $old_data = $data; // remember original data from transaction we're reversing
  extract(u\just('rebate bonus xfee', $data));
  $myFor = t('reverses #') . $tid;
  $otherFor = str_replace($tid, $otherTid, $myFor);
  $what = u\order($type == 'payment', $myFor, $otherFor);
  foreach (u\ray('rebate bonus xfee') as $k) if (@$$k) $data[$k] = -$$k;
  $data['undoes'] = $xid; // remember what we're reversing (especially to get the realness right)
  if ($force) $data += compact('force');
  list ($message, $args, $confirm) = transfer($type, $mya, $acct, -$amount, $old_goods, $what, NULL, TRUE, $data);

  if (@$args['success']) {
    extract(u\just('xid tid otherTid', $args), EXTR_PREFIX_ALL, 'new');
    $old_data['undoneBy'] = $new_xid;
    $old_data['changes'][] = array(REQUEST_TIME, $mya->agentId, 'undo', $new_xid);
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
  extract(u\just('payer payee amount purpose', $inv));
  
  $DBTX = \db_transaction();
  $purpose .= " (inv#$nvid)";
  list ($index, $subs) = be\transfer('payment', r\acct($payer), r\acct($payee), $amount, TRUE, $purpose);
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
 * @param bool $goods: is this for real goods and services
 * @param string $for OR array($payerFor, $payeeFor): description of the transaction's purpose (user input)
 * @param int $created: time of transaction (defaults to current time)
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE)
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount) [rebate and bonus are always both set or neither]
 *   'undoes': transaction id of the related transaction (transaction being undone / invoice being paid)
 *   'preapproved': the transaction is preapproved (from some other region)
 *   'bilateral': set TRUE if a company with B_CHARGE wants to pretend it doesn't
 *      (Typically upon recursion, to handle shortfall invoice -- currently UNUSED)
 *   'force': <do the transaction despite any shortfall (1 for original transaction, -1 for forced reversal)>
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $paying_invoice, $goods (the transaction is an exchange for real stuff),
 * $shortfall (how inadequate the payer's balance is)
  */
function transfer($ttype, $a1, $a2, $amount, $goods = FALSE, $for, $created = NULL, $confirmed = TRUE, $data = array()) {
  global $scanned, $channel;

  u\EXPECT(in_array($ttype, u\ray('payment,charge,invoice payment')), 'bad transaction type');
  u\setDft($for, $goods ? R_FOR_GOODS : R_FOR_CASH);
  u\setDft($created, time());
  u\EXPECT(compact(u\ray('a1 a2 goods confirmed data')), 'acct acct bool bool assoc');

  list ($myid, $uid) = array($a1->id, $a2->id);
  $erInfo = compact(u\ray('ttype amount goods for')) + u\ray('a1 a2', $a1->fullName, $a2->fullName);
  $taking = ($ttype == 'charge');
  if (r\dupTx($uid, $myid, $taking ? $amount : -$amount)) return r\txRet('duplicate transaction', u\ray('op', $taking ? t('charged') : t('paid')));
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  // Check permissions
//  if ($err = r\txPermErr($a1, $a2, $reallyTaking)) return r\txRet($err, array());
  if ($err = r\txPermErr($a1, $a2, $taking, $amount < 0)) return r\txRet($err, $erInfo);
   
  // Analyze who's buying or selling
  list ($payer, $payee) = u\order($taking, $uid, $myid);
  list ($payerAgent, $payeeAgent) = u\order($taking, $a2->agentId, $a1->agentId);

  // Sort out what type of transfer this is
  //$undoing = isset($data['undoes']);
  $type = TX_TRANSFER;
  $unilateral = isset($data['bilateral']) ? FALSE 
  : ($taking // selling
      // (need this at POS!) and !$undoing // not reversing a payment
      and !($a2->co and $a2->proSe) // not selling to a company directly (without its agent)
      and (@$scanned // in person: scanned (web or smart)
//        or ($channel == TX_SMS and $a1->can(B_CHARGE)) // or by SMS when qualified
      )
  );
  $preapproved = (@$data['preapproved'] or !$taking or $unilateral); // paying OR permitted to charge unilaterally

  // Check the amount and calculate any balance shortfall
  if ($error = u\badAmount($amount)) return r\txRet($error, $erInfo);
  $totalHit = $amount - @$data['rebate']; // leave room for reversing rewards (minus a negative)
  
  if ($shortfall = @$data['force'] ? 0 : r\shortfall(abs($totalHit), $reallyTaking ? $a2 : $a1, $preapproved, $goods, $confirmed)) {
    $short = u\fmtAmt($shortfall);
    $otherName = $a2->fullName;
    $msg = ($goods ? 'short ' : 'short cash ') 
    . ($reallyTaking ? ($a2->can(B_SECRET_BAL) ? 'from vague' : 'from') : 'to');
    r\suggestMin($reallyTaking ? $a2 : $a1);
    return r\txRet($msg, compact(u\ray('otherName short how')));
  }
  
  $info = compact(u\ray('type created amount payer payee payerAgent payeeAgent for data taking goods'));
  if (!$x = new r\x($info, $confirmed)) return r\txRet('tx save err', $erInfo);
  
  // Report results
  $args = r\txReportArgs($a1, $x->ray, TRUE, $shortfall);

  if ($confirmed) { // notify other party by email about any confirmed transaction
    $notice = $reallyTaking ? ($preapproved ? 'new charge' : 'new invoice')
      : (($amount < 0 and $goods) ? 'new refund' : 'new payment');
    if ($notice == 'new payment' and $a2->co) { // not "new charge" (the most common)
      $href = r\makeDo('op id expires', 'addr', $myid, 90 * DAY_SECS); // add a link to address, so merchant can ship or nonprofit can thank
      $args['myName'] = "<a href=\"$href\">$args[myName]</a>";
    }
    if (@$args['otherRewardAmount']) $notice .= '|reward other';
    r\notify($uid, $notice, $args);
    $message = $goods ?
        ($preapproved ? 'transaction' : 'invoice')
      : ($preapproved ? 'exchange' : 'exchange request'); // no possible shortfall on confirmed cash exchange (see above)
    $message = "report $message";
  } else {
    $message = $goods ? $ttype : 'exchange';
  }
  return r\txRet($message, $args, !$confirmed);
}

/**
 * Create an ivoice
 * call by: list ($msg, $args, $confirm) = be\invoice(...);
 * @param acct $a1: acct object for first party (usually current user)
 * @param acct $a2: acct object for the other party
 * @param numeric $amount: the invoiced amount
 * @param bool $goods: is this for real goods and services
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param bool $confirmed: has this invoice been confirmed by the user? (default TRUE)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *    confirm: boolean "this needs to be confirmed"
 */
function invoice($a1, $a2, $amount, $goods = FALSE, $purpose = NULL, $confirmed = TRUE) {
  u\setDft($purpose, $goods ? R_FOR_GOODS : R_FOR_CASH);
  list ($payee, $payer) = array($a1->id, $a2->id);
  if ($err = r\txPermErr($a1, $a2, TRUE)) return array($err, array(), FALSE);
  if ($err = u\badAmount($amount, '>0')) return array($err, array(), FALSE);
  u\EXPECT(compact(u\ray('a1 a2 amount goods purpose confirmed')), 'acct acct float bool string bool');
  if (r\dupTx($payee, $payer, -$amount)) return array('duplicate transaction', u\ray('op', t('charged')), FALSE);

  $subs = u\ray('myName otherName amount purpose did', $a1->fullName, $a2->fullName, u\fmtAmt($amount), $purpose, t('charged'));

  if ($confirmed) {
    $created = REQUEST_TIME;
    $info = compact(u\ray('amount payer payee goods purpose created'));
    $nvid = db\insert('r_invoices', $info);
    $a1->update(u\ray('lastTx', array($nvid, $payer - $payee, round(-$amount, 2), $created, 'inv')));

    $href = r\makeDo('op id expires', 'inv', $nvid, 90 * DAY_SECS);
    $subs['a'] = "a href=\"$href\""; // link to pay/deny
    //$a2->update(u\ray('lastTx', array($nvid, $payee - $payer, round($amount, 2), $created, 'inv')));
    r\notify($payer, 'new invoice', $subs); // notify other party by email about this invoice
    return array('report invoice', $subs + u\ray('success', TRUE), FALSE);
  } else return array('charge', $subs, TRUE);
}

/**
 * Record a transfer (of rCredits) to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer
 * @param int $amount: how much to transfer
 * @param string $for: purpose of funding
 * @param bool $confirmed: actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE on failure)
 */
function fund($uid, $type, $amount, $for, $confirmed = TRUE) {
  $payeeAgent = $payee = $uid;
  $mya = r\acct(); // (null, if just registering or running as cron)
  u\EXPECT(r\isReward($type) or $mya->can(B_MANAGING_CTTY), 'non-automatic reward by non ctty agent');
  if ($type == TX_FINE) $amount = -$amount;
  if (!$confirmed) return $amount;
  
  $payer = r\acct($payee)->community;
  $payerAgent = (@$mya and $mya->id < 0) ? $mya->agentId : $payer; // take note if funded by community admin
  if (!new r\x(compact(u\ray('type amount payer payee payerAgent payeeAgent for taking')))) return FALSE;
  //$types = u\ray(TX_TYPES);
  $subs = u\ray('amount purpose', u\fmtAmt(abs($amount)), $for);
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
  if (!$mya->phone) $mya->update(u\ray('phone', $numberInternal)); // use it as contact phone, if none yet
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
 * @param string $who: how the user referred to the trader
 * @param string $self_message: index to error message for identifying oneself
 * UNUSED @param boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible fullNames (0 or more) indexed by uid)
 *   (the caller can test which type of return, with "if (isAcct($returned_value))
 */
function identify($who, $self_message = 'no self-trading') {
  $create = FALSE; // not currently used
  $who = trim($who);
//  if (($all = strtoupper($who)) == 'ALL') return $allow_ALL ? $all : array('no ALL', array(), array());
  $myid = r\acct()->id;

  if ($phone = u\fmtPhone($who, '+n')) $who = $phone;
  
  $quid = strtoupper($who); // in case this is a quid (abbreviated or not)
  if (preg_match('/^([A-Z]{3})?[\.\:=][A-Z]{3,4}( |$)/i', $quid, $match)) { // allow quids or abbrevs within this region
    $pre = $match[1] ? '' : R_SERVER_ID; // region name was omitted
    list($quid) = explode(' ', "$pre$quid ", 2); // take just the first word, in case this is from a list showing QUID and fullName
  }

  if (u\isQid($quid)) {
    if (!$acct = r\acct($quid)) return array('unknown member', compact('who'), array());
    if ($acct->id == $myid) return array($self_message, array(), array());
    return $acct; // qids are easy
  }

  if (($short = u\shortName($who)) === '') $short = '-'; // abbreviations must not be null
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, u.fullName FROM users u LEFT JOIN r_boxes s ON u.uid = s.uid 
    WHERE (:who IN (s.code, u.phone, u.mail) OR INSTR(u.name, :short)) AND u.uid>0
    ORDER BY u.fullName
EOF;
///  debug(compact('sql', 'who', 'phone', 'short'));
  $result = db\q($sql, compact(u\ray('who short')))->fetchAllKeyed(0, 1);
//  $list = '';
  foreach ($result as $uid => $fullName) {
    if ($uid != $myid) { // not self
//      $list .= " &nbsp; &nbsp; &nbsp; &nbsp; $fullName<br>";
    } elseif (count($result) == 1) { // self is the only result
      return array($self_message, array(), array()); 
    } else unset($result[$uid]); // self is one of many results, so ignore it
  }

  if(count($result) > 1) return array('ambiguous other', compact(u\ray('who')), $result);
  if(count($result) == 1) {
    foreach ($result as $uid => $a); // get the one entry
    return r\acct($uid);
  }

//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (\valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown member', compact('who'), array());
  return new r\acct(array($key => $who));
}

/**
 * Return financial information about the given account on this server
 * @param assoc $args: options:
 *   uid: uid for the account to return information for (DEFAULTS to myid)
 *   pending: (UNUSED) show pending transaction totals instead
 *   asof: last date to include (defaults to tomorrow, meaning unrestricted)
 * @return
 *   standard object of credit account information (see $resultKeys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function creditInfo($args = array()) {
  extract(u\just('uid pending asof', $args));
  if ($mya = r\acct()) u\setDft($uid, $mya->id);
  if (!$acct = r\acct(@$uid)) return NULL;

  u\setDft($asof, REQUEST_TIME + DAY_SECS);
  $subs = compact(u\ray('uid asof'));
  
  $sql = <<<EOF
    SELECT 
    SUM(IF(t.payee=:uid, amount, 0)) AS `in`,
    SUM(IF(t.payer=:uid, amount, 0)) AS `out`, 
    SUM(IF(t.payee=:uid AND t.type=:TX_REFUND, amount, 0)) AS refunds,
    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rewards,
    SUM(IF(t.type=:TX_XFEE, IF(t.payer=:uid, -amount, amount), 0)) AS xfees,
    SUM(IF(:HAS_XFEE, IF(t.payer=:uid, -amount, amount), 0)) AS xin
    FROM r_txs t
    WHERE (t.payer=:uid OR t.payee=:uid) AND t.created<:asof
EOF;
  
  $sums = db\q($sql, $subs)->fetchAssoc();

  $dates = @$pending ? 'NOT completed AND created <:asof' : 'completed AND completed<:asof';
  $sql = <<<EOF
    SELECT 
    SUM(IF(payee=0, amount, 0)) AS bank,
    SUM(IF(payee=-1, amount, 0)) AS fees,
    SUM(IF(payee IN (0, -1, :uid), 0, amount)) AS usdOut,
    SUM(IF(payer=:uid, 0, amount)) AS usdIn
    FROM r_usd WHERE :uid IN (payer, payee) AND $dates
EOF;
  $sums += db\q($sql, $subs)->fetchAssoc();
///if ($sums['usdOut'] or $sums['usdIn']) debug(compact('sql','subs','sums'));

  foreach ($sums as $key => $value) $sums[$key] = $value + 0; // no nulls
  extract($sums);

  if (!@$pending and $refunds != $fees) {
    $info = compact(u\ray('uid fees refunds'));
/**/ t\output('ERROR: fee/refund mismatch: ' . print_r($info, 1));
    r\tellStaff('fee/refund mismatch', $subs+$args+$info);
  }
  $rewards -= $refunds; // Dwolla fee refunds are a hidden reward
  $r = ($in - $out) + ($usdOut - $usdIn);
  $in -= $refunds; // this includes (unhidden) rewards
  $in -= $xin; $out -= $xin;

  $bank = -$bank; // make it "from bank" instead of "to bank"
  $usd = $bank - $fees + $usdIn - $usdOut;
  $r += $usd; $usd = 0; // phase out usd field
  $balance = $bank + $in - $out;
  $outProper = $out;
  $inProper = $in - $rewards;

  $resultKeys = 'in out rewards refunds fees xfees xin inProper outProper balance r usdIn usdOut usd bank';
  $result = compact(u\ray($resultKeys));
  
  $fancy = array();
  foreach ($result as $k => $v) {
    $result[$k] = $v = round($v, 2);
    $fancy[$k] = u\fmtAmt($v);
  }
  $result['fancy'] = $fancy;
///debug($result);
  return (object) $result;
}

/**
 * Return a list of account choices for the current user (NOT the current credit account)
 * @return an array of account ids (FALSE if none)
 */
function accountChoices() {
  if (!$mya = r\acct()) return FALSE;
  $result = array($aid = $mya->agentId);
  if ($mya->admin) {
    $result += @db\q('SELECT uid FROM users WHERE ABS(uid)>1 ORDER BY (uid<0) DESC, fullName')->fetchAllKeyed(0, 0);
  } elseif ($mya->cAdmin) {
    $result += @db\q('SELECT uid FROM users WHERE community=:ctty ORDER BY (uid<0) DESC, fullName', u\ray('ctty', $mya->community))->fetchAllKeyed(0, 0);
  } else $result += @db\q('SELECT main FROM r_relations WHERE other=:aid AND permission>0', compact('aid'))->fetchAllKeyed(0, 0);
  return array_values($result);
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
function lastTx($where = 1, $subs = array(), $cache = FALSE) {
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
