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

define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows');
define('API_URL', REAL ? (R_SITE_URL . '/api') : 'http://localhost/devcore/api');

function send($data) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, API_URL);
  curl_setopt($ch, CURLOPT_POST, 1); //post the data
  curl_setopt($ch, CURLOPT_POSTFIELDS, prep($data));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); //enable RETURN_TRANSFER so curl_exec() returns result of the request
  $result = curl_exec($ch);
  curl_close($ch);
  echo $result; // KEEP
  exit();
}

function prep($data) {
  global $my_id, $code;
  return array('json' => u\jsonEncode(compact('my_id', 'code') + $data));
}

function doOp($op0, $args, $interface = 'API', $success = 1) {
  if (!@$op0) return error('no op');
  $op = "rCredits\\$interface\\$op0";
  $message = $result = function_exists($op) ? $op($args) : error('unknown op');
  if (!is_array($result)) $result = compact('message');
  return ($op0 == 'photo' or (!is_array($message) and strpos($message, '"success":"0"'))) ?
      $message 
    : apiOut(array_merge(compact('success'), $result)); // args can override success
}

function error($message, $subs = '', $success = 0) {
  $message = u\tt($message, $subs);
  return apiOut(compact('success', 'message'));
}

function apiOut($args) {
  $result = u\jsonEncode($args);
  u\loga('out', $args);
  if (t\est()) {t\output($args, 'output'); return $result;}
  exit($result);
}

/**
 * Return the user's bank transactions for the given period (inclusive)
 * @param int $starting: starting time (the beginning of some day)
 * @param int $ending: ending time (the end of some day)
 * @param int $uid: account id for which to return transactions (defaults to current account)
 * @param assoc $opts: array of options
 *   bool $dones: include completed transactions (default TRUE)
 *   bool $pendings: include open invoices (default TRUE)
 * @return the recordset (FALSE if none)
 */
function bankTransfers($starting, $ending, $uid = NULL, $opts = array()) {
  extract(u\just('dones pendings', $opts));
  u\setDft($dones, TRUE);
  u\setDft($pendings, TRUE);
  $mya = r\acct();
  if (!@$uid) $uid = $mya->id; elseif ($uid == 'ctty') $uid = $mya->community;
  $date = $pendings ? 'GREATEST(t.created, t.completed)' : 't.completed';
  $where = "$date BETWEEN :starting AND :ending ORDER BY $date DESC";
  if (!$dones) $where = "NOT completed AND $where";
  $fields = "t.txid AS usdXid, CONCAT('b', tid) AS xid, amount, t.payer, t.payee, :TX_BANK AS type, t.created, t.completed, IF(t.completed, :TX_DONE, :TX_PENDING) AS state, IF(t.amount<0, 1, 0) AS toMe";
  
  if ($uid < 0) { // community or region
    $sql = <<<EOF
      SELECT $fields
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
 * @param int $id: account id for which to return transactions (defaults to current account)
 * @param assoc $opts: array of options
 *   bool $dones: include completed transactions (default TRUE)
 *   bool $pendings: include open invoices (default TRUE)
 *   bool $bonuses: include rebate and bonus transactions (default FALSE)
 *   bool $exchanges: include exchanges of rCredits credit for USD credit (default FALSE)
 *    (but ALWAYS include other rewards/loans etc)
 *   bool $pexchanges: include payment exchanges (to employees/suppliers or from company) (default FALSE)
 * @return the recordset (FALSE if none)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function myTransactions($starting, $ending, $id = NULL, $opts = array()) {
  extract(u\just('dones pendings bonuses exchanges pexchanges', $opts));
  u\setDft($dones, TRUE);
  u\setDft($pendings, TRUE);
  u\setDft($bonuses, FALSE);
  u\setDft($exchanges, FALSE);
  u\setDft($pexchanges, FALSE);
  $mya = r\acct();
  if (!@$id) $id = $mya->id; elseif ($id == 'ctty') $id = $mya->community;
  $where = 't.created>=:starting AND t.created<=:ending';
  if (!$bonuses) $where .= ' AND t.type>=0';
  if (!$exchanges or !$pexchanges) {
    $maybe = $exchanges ? 'NOT' : '';
    $where .= ($exchanges or $pexchanges) ? " AND (amount>0 OR t.data $maybe LIKE '%rebate%')" : ' AND amount>0';
  }
  $where .= ' ORDER BY t.xid DESC';
  $states = $dones ? array(TX_DONE, TX_DISPUTED) : array();
  if ($pendings) $states[] = TX_PENDING;
  $states = 't.state IN (' . join(',', $states) . ')';
  
  if ($id < 0) { // community or region
    $sql = <<<EOF
      SELECT t.*, IF(taking, t.payeeFor, t.payerFor) AS purpose
      FROM r_txs t LEFT JOIN users u1 ON u1.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
      WHERE (u1.community=:id OR u2.community=:id)
      AND $states
      AND $where
EOF;
  } else { // normal account
    if ($pendings) $states .= ' OR (t.state=:TX_DENIED AND t.payee=:id)';
    // Denied invoices are not exactly pending, but user will want to see them, before deleting
    $sql = <<<EOF
      SELECT t.*,
      (SELECT fullName FROM users WHERE uid=IF(t.payer=:id, t.payee, t.payer)) AS name,
      IF(t.payer=:id, t.payerFor, t.payeeFor) AS purpose
      FROM r_txs t 
      WHERE :id IN (t.payer, t.payee)
      AND ($states)
      AND $where
EOF;
  }

  return db\q($sql, compact(u\ray('starting ending id'))); 
}

/**
 * Update relations
 * @param array $updates: list of records to update, indexed by reid
 *   each element is an associative array of new values, keyed by field name
 *   if a record is to be updated to all zeros, it is simply deleted.
 * @return string: a report of what got updated
 * employeeOk can be updated only by other, all else only by main
 */
function updateRelations($updates) {
///  debug($updates);
  $myid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(u\just('main other fullName', $data['original'])); 
    unset($data['original']);
///    debug(compact('updates','data','employeeOk2'));
    
    if ($isAgent = ($myid != $main)) {
      r\reverseRelations($main, $other, $data, 'permission employerOk isOwner draw'); // giving employer permission?(!) -- create (or update?) a proper record (originally was just the employer's record)
    } else r\reverseRelations($main, $other, $data, 'employeeOk'); // claiming to be an employee?
    
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

    $msg = 'updated relation';
    if (!r\relation('permission>0 OR employerOk OR employeeOk OR isOwner OR amount OR draw', $where, $reidSub)
      and (!r\relation('employerOk OR employeeOk', 'main=:other AND other=:main', compact('main', 'other')))) {
      setCttyAdmin(FALSE, $main, $other);
      db\q("DELETE FROM r_relations WHERE $where", $reidSub);
      $pot = '(:main, :other)';
      if (!r\relation(1, "main IN $pot AND other IN $pot", compact('main', 'other'))) $msg = 'deleted relation';
    } else {
      setCttyAdmin(@$data['permission'] >= r\perm(B_MANAGE), $main, $other);
    }    
    $otherName = $fullName;
    $report .= u\tt($msg, compact('otherName')) . '<br>';
  }
  return $report;
}

function setCttyAdmin($value, $main, $other) {
  if ($main < 0 and $main == r\acct($other)->community) r\acct($other)->setBit(B_CTTY_ADMIN, $value);
}

/**
 * Undo the given transaction.
 * Situations:
 *   Transaction PENDING
 *     I invoiced someone:        CANCELED
 *     Someone invoiced me:       DENIED
 *     I offered to pay someone:  CANCELED
 *     Someone offered to pay me: DENIED
 *   Transaction DENIED (same as for PENDING)
 *     I invoiced someone:        CANCELED
 *     Someone invoiced me:       DENIED
 *     I offered to pay someone:  CANCELED
 *     Someone offered to pay me: DENIED
 *   Transaction DONE or DISPUTED
 *     I paid someone:            REVERSE (with reverse rewards)
 *     I charged someone:         REVERSE
 *     Someone paid me:           REVERSE
 *     Someone invoiced me and I approved it: DISPUTED
 *     Someone charged me unilaterally: DISPUTED
 * @param int $xid: the transaction number
 * @return array [message, subs] where subs is an assoc of [
 */
function undoTx($xid) {
  $mya = r\acct();
  $oldRecord = lastTx('xid=:xid', compact('xid'));
  if ($error = r\nonoUndo($oldRecord, $subs)) return array($error, $subs); // check nono before confirm AND here

  extract($oldRecord, EXTR_PREFIX_ALL, 'old');
  $data = unserialize($old_data);

  if ($old_state == TX_PENDING or $old_state == TX_DENIED) { // it's an invoice - cancel simply by changing the transaction state
// function undo_invoice($xid, $oldRecord, $old_otherUid, $byMe) // return $solution
///  debug(compact('old_byme','old_state','oldRecord','data'));
    if ($old_byMe) {
      list ($new_state, $solution) = array(TX_DELETED, t('deleted')); // abandon the attempt to invoice
      r\notify($old_otherUid, $old_toMe ? 'invoice canceled' : 'offer canceled', r\txReportArgs($mya, $oldRecord));
    } else {
      list ($new_state, $solution) = array(TX_DENIED, t('marked "denied"'));
      r\notify($old_otherUid, $old_toMe ? 'offer refused' : 'invoice denied', r\txReportArgs($mya, $oldRecord));
    }
    r\setTxState($new_state, $xid);
  } elseif (!$old_toMe and !$old_byMe) { // someone charged (either unilaterally or I approved it)
// function dispute($xid, $oldRecord, $old_otherUid)
    r\setTxState(TX_DISPUTED, $xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($old_otherUid, 'charge disputed', r\txReportArgs($mya, $oldRecord));
  } else {
    $otherAcct = $old_toMe ? r\acct($old_payer, $old_payerAgent) : r\acct($old_payee, $old_payeeAgent);
    $type = $old_toMe ? 'payment' : 'charge';
    $otherTid = $old_toMe ? $old_payerTid : $old_payeeTid;
    return reverse($xid, $old_amount, $otherAcct, $data, $old_goods, $old_tid, $type, $otherTid); // create an offsetting transaction
  }

  // handle undo of a partial unilateral charge (let the merchant undo the invoice, then the partial payment)
  global $lastXid; $lastXid = @$old_data['undoes']; // previous related transaction or FALSE
  
  $amount = u\fmtAmt($old_amount);
  $tofrom = !$old_toMe ? t('to') : t('from');
  $otherUid = $old_otherUid;
  $other = r\userField('fullName', $otherUid);
  $success = TRUE;
  return array("report undo", compact(u\ray('success old_tid new_tid amount other tofrom solution otherUid')));
}

/**
 * Create an offsetting transaction.
 */
function reverse($xid, $amount, $acct, $data, $old_goods, $tid, $type, $otherTid) {
//    print_r(compact(u\ray('xid amount acct data tid toMe'))); die();
  u\EXPECT(compact(u\ray('xid amount acct data tid type otherTid')), 'id float acct assoc|empty string string id');

  if (!is_array($data)) $data = array();
  $old_data = $data; // remember original data from transaction we're reversing
  $myFor = "reverses #$tid";
  $otherFor = str_replace($tid, $otherTid, $myFor);
  $what = u\order($type == 'payment', $myFor, $otherFor);
  if (@$data['rebate'] or @$data['bonus']) list ($data['rebate'], $data['bonus']) = array(-$data['bonus'], -$data['rebate']);
  $data['undoes'] = $xid; // remember what we're reversing (especially to get the realness right)
  list ($message, $args, $confirm) = transfer($type, r\acct(), $acct, $amount, $what, $old_goods, TRUE, $data);

  if (@$args['success']) {
    extract(u\just('xid tid otherTid', $args), EXTR_PREFIX_ALL, 'new');
    $old_data['undoneBy'] = $new_xid;
    $old_data['changes'][] = array(REQUEST_TIME, r\acct()->agentId, 'undo', $new_xid);
    $old_data = serialize($old_data);
    $myFor = "(reversed by #$new_tid) ";
    $otherFor = str_replace($new_tid, $new_otherTid, $myFor);
    list ($payerFor, $payeeFor) = u\order($type == 'payment', $otherFor, $myFor);
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
 * Pay the given invoice
 * @param int $xid: id of invoice to pay
 * @return array(string $message, array $args)
 */
function payInvoice($xid) {
  $mya = r\acct();
  if (!$mya->can(B_BUY)) return array('no buy', array());
  if (!$result = lastTx('xid=:xid AND state IN (:TX_PENDING,:TX_DISPUTED)', compact('xid'))) return array('no such tx', array());
  extract($result, EXTR_PREFIX_ALL, 'old');
  $acct = r\acct($old_payee, $old_payeeAgent);
  $undoes = $xid;
  
  r\usd::beginAtom();
  list ($message, $args, $confirm) = transfer('invoice payment', $mya, $acct, $old_amount, $old_payeeFor, $old_goods, TRUE, compact('undoes'));
  if (@$args['success']) r\setTxState(TX_DELETED, $xid); // delete the invoice
  r\usd::commit();

  return array($message, $args);
}

/**
 * Create a transaction
 * call by: list ($msg, $args, $confirm) = be\transfer(...);
 * @param string $ttype: payment, charge, or invoice payment
 * @param acct $a1: acct object for first party (usually current user)
 * @param acct $a2: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param numeric $requestAmount: the transaction amount
 * @param string $for OR array($payerFor, $payeeFor): description of the transaction's purpose (user input)
 * @param bool $goods: is this for real goods and services
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE)
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount) [rebate and bonus are always both set or neither]
 *   'undoes': transaction id of the related transaction (transaction being undone / invoice being paid)
 *   'preapproved': the transaction is preapproved (from some other region)
 *   'bilateral': set TRUE if a company with B_CHARGE wants to pretend it doesn't
 *      (Typically upon recursion, to handle shortfall invoice -- currently UNUSED)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $paying_invoice, $goods (the transaction is an exchange for real stuff),
 * $shortfall (how inadequate the payer's balance is)
 * @todo: write feature tests for partial transactions (paying/taking unilateral/not undoing/not broke/not)
 */
function transfer($ttype, $a1, $a2, $requestAmount, $for, $goods = FALSE, $confirmed = TRUE, $data = array()) {
  include_once __DIR__ . '/usd.class';
  global $lastXid, $scanned, $channel; 

  u\EXPECT(in_array($ttype, u\ray('payment,charge,invoice payment')), 'bad transaction type');
// disabled for now  if ($a2 == 'ALL') return $ttype == 'payment' ? r\payEmployees($requestAmount, $for, $confirmed) : array('no ALL', array(), FALSE);
  list ($myid, $uid) = array($a1->id, $a2->id);
  $taking = ($ttype == 'charge');

  // Check permissions
  u\EXPECT(compact(u\ray('ttype a1 a2 confirmed data')), 'string acct acct bool assoc');
  $otherName = $a2->fullName;
  if (!$a2->ok()) return array('non member', compact('otherName'), FALSE);
  list ($myAccess, $otherAccess) = u\order($taking, $scanned ? B_SCAN : B_SELL, B_BUY);
  list ($myAccessErr, $otherAccessErr) = u\order($taking, 'sell', 'buy');
  if (!$a1->can($myAccess)) return array("no $myAccessErr", array(), FALSE);
  if (!$a2->can($otherAccess)) return array("other no $otherAccessErr", compact('otherName'), FALSE);
  
  // Analyze who's buying or selling
  if ($uid == $myid) return array('no self-trading', array(), FALSE);
  list ($payer, $payee) = u\order($taking, $uid, $myid);
  list ($payerAgent, $payeeAgent) = u\order($taking, $a2->agentId, $a1->agentId);

  // Sort out what type of transfer this is
  $paying_invoice = ($ttype == 'invoice payment');
  $undoing = isset($data['undoes']);
  $type = TX_TRANSFER;
  $unilateral = isset($data['bilateral']) ? FALSE 
  : ($taking // selling
      and $a1->can(B_CHARGE) // seller can charge unilaterally
      and !$undoing // not reversing a payment
      and !(!$a2->person() and $a2->proSe()) // not selling to a company directly (without its agent)
      and ($channel == TX_SMART or @$scanned or $channel == TX_SMS) // in person: scanned (web or smart) or by SMS
  );
  $preapproved = (@$data['preapproved'] or !$taking or $unilateral); // paying OR permitted to charge unilaterally
//  $state = ($preapproved and !@$data['pay_all'])? TX_DONE : TX_PENDING;
//  $virtual = ($goods and !$taking and $a1->can(B_PAYEX) and r\isEmployee($uid));
  $virtual = FALSE; // this is done only automatically
  $state = ($preapproved and !$virtual)? TX_DONE : TX_PENDING; // may be changed to TX_TENTATIVE below
  
  // Check the amount and calculate any balance shortfall
  if ($error = u\badAmount($requestAmount, '>0')) return array($error, array(), FALSE);
  $totalHit = $requestAmount - @$data['rebate']; // leave room for reversing rewards (minus a negative)
  
  $usd = r\usdPart($totalHit, $taking ? $a2 : $a1, $preapproved, $goods, $confirmed, $shortfall); // short payment or unilateral charge

  if ($shortfall) {
    $short = u\fmtAmt($shortfall);
    if (is_null($usd)) {
      $msg = $taking ? 'shortr from' : 'shortr to';
      $how = 'exchange'; // for 'shortr to' message on web interface
    } else $msg = $taking ? 'short from' : 'short to';
    return array($msg, compact(u\ray('otherName short how')), FALSE);
  }
  $amount = $requestAmount;
  $r = round($amount - $usd, 2);
  
  // Figure rewards and other extra info
  if (!$for) $for = $goods ? 'goods and services' : 'cash/loan/etc.';

  list ($fromAcct, $toAcct) = u\order($taking, $a2, $a1);
  list ($fromUsAcct, $toUsAcct) = array(new r\usd($fromAcct), new r\usd($toAcct));
  $txArgs = compact(u\ray('type amount r payer payee payerAgent payeeAgent for data taking goods state'));
  r\usd::beginAtom();
  if ($confirmed) {
    $payerFee = R_FEE_FOR_CASH ? (!$goods) : FALSE; // cash-getter pays the transaction fee
    $note = is_array($for) ? $for[$taking ? 1 : 0] : $for;
    if ($state == TX_DONE and $usd > 0 and !$usdXid = $fromUsAcct->send($usd, $toUsAcct, $note, $err, $payerFee)) {
      r\usd::rollback();
      $fromAcct->actualUsd(); // fix cached USD, if needed (after rollback of course)
      return array('usd failed', array(), FALSE);
    }
    $txs = r\transact($txArgs + compact('usdXid'));
    $lastXid = $txs[0]['xid'];
    r\usd::commit();
  } else {
    $txs = r\transact($txArgs);
    r\usd::rollback();
  }
  
  // Report results
  $args = r\txReportArgs($a1, $txs[0], TRUE, $shortfall);

  if ($confirmed) { // notify other party by email about any confirmed transaction
    $notice = $taking ? ($preapproved ? 'new charge' : 'new invoice')
      : ($virtual ? 'new offer' : 'new payment');
    if (@$args['otherRewardAmount']) $notice .= '|reward other';
    if (u\abbreviates('new payment', $notice)) {
    //and $payee == \variable_get('cgf_uid')) { // should be any nonprofit? anyone?
//      $a1 = 
//      $notice .= u\tt('address info', compact('a1'));
    }
    r\notify($uid, $notice, $args);
    $message = $goods ?
        ($preapproved ? ($virtual ? 'offer' : 'transaction') : 'invoice')
      : ($preapproved ? 'exchange' : 'exchange request'); // no possible shortfall on confirmed cash exchange (see above)
    $message = "report $message";
  } else {
    $message = $goods ? $ttype : 'exchange';
  }
  return array($message, $args, !$confirmed);
}

/**
 * Record a transfer (of rCredits) to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer
 * @param int $amount: how much to transfer
 * @param string $for: purpose of funding
 * @param bool $confirmed: actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE if the current account does not have permission to do this)
 */
function fund($uid, $type, $amount, $for, $confirmed = TRUE) {
  $payeeAgent = $payee = $uid;
  $mya = r\acct(); // (null, if just registering or running as cron)
  u\EXPECT($type <= TX_REWARDS or $mya->can(B_MANAGING_CTTY), 'non-automatic reward by non ctty agent');
  if (!$confirmed) return $amount;
  
  $payer = r\acct($payee)->community;
  $payerAgent = (@$mya and $mya->id < 0) ? $mya->agentId : $payer; // take note if funded by community admin
  $taking = ($type >= TX_FINE);
  r\transact(compact(u\ray('type amount payer payee payerAgent payeeAgent for taking')));
  $amount = u\fmtAmt($amount);
  r\notify($uid, 'got funding', u\ray('amount type', $amount, $for));
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
    if ($result = db\q('SELECT * FROM r_sms WHERE number=:numberInternal', compact('numberInternal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return array('already cell', $subs);
      $accountName = accountName($old_uid);
      if (!isTempAccount($old_uid)) return array('cell taken', compact(u\ray('number accountName')));
    }
    return NULL;
  }
  
  $uid = $myid;
  $status = SMS_PRIMARY;
  $phone = $number = $numberInternal;
  $info = compact(u\ray('uid number status'));
  //if (!mergeCheck(compact('number'))) 
  db\insert('r_sms', $info);
  if (!$mya->phone) $mya->update(compact('phone')); // use it as contact phone, if none yet
  $number = $numberPretty;
  return array('report new cell', compact('number'));
}

function deleteCell($number) {
  $number = u\fmtPhone($number, '+n'); // internal format
  db\q('DELETE FROM r_sms WHERE number=:number LIMIT 1', compact('number'));
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
    SELECT DISTINCT u.uid, u.fullName FROM users u LEFT JOIN r_sms s ON u.uid = s.uid 
    WHERE (:who IN (s.number, u.phone, u.mail) OR INSTR(u.name, :short)) AND u.uid>0
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
 *   pending: show pending transaction totals instead
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

  $state = @$pending ? '=:TX_PENDING' : '>=:TX_DONE';
  u\setDft($asof, REQUEST_TIME + DAY_SECS);
  $subs = compact(u\ray('uid asof'));
  
  $dateField = 'GREATEST(t.created,IF(ISNULL(t.completed), 0, t.completed))';
  $sql = <<<EOF
    SELECT 
    SUM(IF(t.payee=:uid, amount, 0)) AS `in`,
    SUM(IF(t.payer=:uid, amount, 0)) AS `out`, 
    SUM(IF(t.type=:TX_TRANSFER OR t.type > :TX_REWARDS, 0, r)) AS rewards,
    SUM(IF(t.type=:TX_REFUND, r, 0)) AS fees,
    
    SUM(IF(t.payee=:uid, r, 0)) AS rIn,
    SUM(IF(t.payer=:uid, r, 0)) AS rOut 
    
    FROM r_txs t
    WHERE (t.payer=:uid OR t.payee=:uid) AND t.state$state AND $dateField<=:asof
EOF;
///  debug(str_replace(':uid', $uid, u\SUBS($sql)));
  // rewards is based on r, not amount, because REFUNDs have amount=0
  
  $sums = db\q($sql, $subs)->fetchAssoc();

  $dates = @$pending ? 'NOT completed AND created <=:asof' : 'completed AND completed<=:asof';
  $sql = "SELECT SUM(amount) AS bank FROM r_usd WHERE payer=:uid AND payee=0 AND $dates";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $key => $value) $sums[$key] = $value + 0; // no nulls
  extract($sums);

  $bank = -$bank; // make it "from bank" instead of "to bank"
  $balance = $bank + $in - $out;
  $outProper = $out + $fees;
  $inProper = $in - ($rewards - $fees);

  $r = $rIn - $rOut;
  $rOutProper = $rOut + $fees;
  $rInProper = $rIn - ($rewards - $fees);

  $usdIn = $in - $rIn;
  $usdOut = $out - $rOut;
  $usd = $usdIn - $usdOut;
  
  $resultKeys = 'in out rewards fees rIn rOut inProper outProper balance rInProper rOutProper r usdIn usdOut usd bank';
  $result = compact(u\ray($resultKeys));
  
  $fancy = array();
  foreach ($result as $key => $value) $fancy[$key] = u\fmtAmt(@$value + 0);
  $result['fancy'] = $fancy;

  return (object) $result;
}

/**
 * Return a list of account choices for the current user (NOT the current credit account)
 * @return an array of account ids (FALSE if none)
 */
function accountChoices() {
  if (!$mya = r\acct()) return FALSE;
  $result = array($aid = $mya->agentId);
  if ($mya->admin()) {
    $result += @db\q('SELECT uid FROM users WHERE uid<:R_CTTY_UID0 ORDER BY fullName')->fetchAllKeyed(0, 0);
    $result += @db\q('SELECT uid FROM users WHERE uid>1 ORDER BY fullName')->fetchAllKeyed(0, 0);
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
    return (object) db\lookup('nonce,todo', 'r_sms', 'number=:number', compact('number'));
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('r_sms')->fields(compact('nonce', 'todo'))->condition('number', $number)->execute();
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
  $basic = "$myid IN (payer, payee) AND type>=0 AND state<>:TX_DELETED"; // make sure it's for $myid
  $sql = "SELECT * FROM r_txs WHERE ($basic) AND ($where) ORDER BY xid DESC LIMIT 1";
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

/**
 * @return array of record objects
 */
function getRecords($table, $criteria, $subs = array(), $fields = '*', $cache = FALSE) {
/*  $sql = <<<EOF
    SELECT COLUMN_NAME FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA=:db AND TABLE_NAME=:table AND COLUMN_KEY='PRI'
EOF;
  $db = 'devcore'; // @todo: get proper db name
  $key = db\q($sql, compact('table', 'db')); */
//  if (!$cache) die('no cache getRecords');
//  echo " SELECT $fields FROM $table WHERE $criteria ";
  return db\q("SELECT $fields FROM $table WHERE $criteria", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

function isTempAccount($uid) {
  return !r\userField('pass', $uid);
}

function isTempName($name) {
  return u\abbreviates(R_TEMP_NAME_PREFIX, $name);
}
