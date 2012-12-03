<?php
/**
 * @file
 * All data access for the rCredits interfaces (SMS, web, smartphone, etc.)
 * These are the only rCredits functions that the individual interfaces can call,
 * apart from the utilities, which can be duplicated for those modules.
 * The point of this separation is to prepare to run the database on its own VPS, with a well-defined API,
 * separate from the individual user interfaces.
 *
 * Here's where we're headed:
 * rWeb calls to the Back End will be changed to calls to the "backend" function.
 * For example, 
 *   old: undoTx($zxid);
 *   new: backend('undoTx', $zxid);
 * backend() simply posts JSON data to the Back End and receives a JSON response, which it decodes and returns.
 * The mobile app will have a similar backend function.
 * rSMS is already a purely backend module
 *
 * In the Back End, we will receive a POST to the /api page, which will run external_request().
 * The external_request() function will decode the JSON request, then will call the appropriate function.
 * For example:
 * {"op":"undoTx","zxid":123}
 * becomes r\undoTx(123, 'web');
 * which then echos a JSON reply by calling r\frontend($info);
 * The Back End will accept web requests only from the right IP.
 * Eventually the Back End will listen only on a certain port (not 80).
 *
 * No Drupal login is required in Back End. (just pass quid(cuid) and quid(user_id),password)
 * All uids are passed as qids (qid = r\quid(uid) = R_SERVER_ID . n2a(uid))
 *
 * @todo: either make a backend or don't. Another way to look at it is to isolate the Web frontend, since rSmart and rSMS are already pretty secure and separate. May want to separate lowish-level db functions as db\ (or not).
 */

namespace rCredits\Backend; // typically abbreviated as "be"
use rCredits as r;
use rCredits\Util as u;
use rCredits\Testing as t;

define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows');
define('API_URL', REAL ? 'http://devcore.rCredits.org/api' : 'http://localhost/devcore/api');

function send($data) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, API_URL);
  curl_setopt($ch, CURLOPT_POST, 1); //post the data
  curl_setopt($ch, CURLOPT_POSTFIELDS, prep($data));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); //enable RETURN_TRANSFER so curl_exec() returns result of the request
  $result = curl_exec($ch);
  curl_close($ch);
  echo $result;
  exit();
}

function prep($data) {
  global $my_id, $code;
  return array('json' => json_encode(compact('my_id', 'code') + $data));
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
  $result = json_encode2($args, JSON_UNESCAPED_SLASHES);
  u\log('out', $args);
  if (defined('TESTING')) {t\testOutput($args, 'output'); return $result;}
  exit($result);
}

/**
 * Receive and process a REST request from the website frontend
 */
function backend2($args = FALSE) {
  global $firewall; // is this useful?

  if (!$args) $args = $_POST;
  extract($args, EXTR_PREFIX_ALL, 'my'); // $cuid, $firewall, $agent, $my_id, $op
  unset($args['cuid']); // unset because it is passed as part of r\acct(), not as an arg

  if (@$my_firewall != R_FIREWALL) error('firewall system failure'); // give hackers some satisfaction by saying 'failure'
  $firewall = $my_firewall; // is this useful?
  if (!@$my_cuid or !@$my_agent) error('user validation error');
  if (!r\agentCan($my_cuid, $my_agent)) error('sharing error');
  r\acct($my_cuid, $my_agent);
  doOp($my_op, $args);
}

/**
 * Return the user's transactions for the given period (inclusive)
 * @param array $args: associative array of options. 
 *   Required arguments:
 *     starting (numeric): starting date
 *     ending (numeric): ending date
 *     real (boolean): show real transactions? (or asif)
 *   Optional arguments:
 *     include_bonuses (boolean)
 * @return array: associative list of transactions (with negative zxids, if they are not real transactions)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function myTransactions($args) {
  $cuid = r\acct()->id;
  
  if (isFrontend()) return backend(__FUNCTION__, $args);
  extract($args);
  
  $table = r\zxs($real/2);
  $omit_bonuses = @$include_bonuses ? '' : 'WHERE t.type NOT IN (:TX_REBATE, :TX_BONUS)';
  
  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid=IF(t.payer=$cuid, t.payee, t.payer)) AS name,
      IF(t.payer=$cuid, t.payer_for, t.payee_for) AS purpose
    FROM $table t 
    $omit_bonuses
    AND $cuid IN (t.payer, t.payee) 
    AND (t.state IN(:TX_DONE, :TX_PENDING, :TX_DISPUTED) OR (t.state=:TX_DENIED AND t.payee=$cuid))
    AND t.created>=:starting AND t.created<=:ending
    ORDER BY t.xid DESC
EOF;
  
  $result = r\dbQ($sql, compact(u\ray('starting ending')))->fetchAllAssoc('xid', \PDO::FETCH_ASSOC);
  
  if (!$real) foreach ($result as $xid => $one) { // negate the keys for asif transactions
    $zxid = -$xid;
    $one['zxid'] = $zxid;
    $result[$zxid] = $one;
    unset($result[$xid]);
  }
//  return array_flip(array_map(function($one) {return -$one;}, array_flip($result))); // negate the keys

  return $result;
}

/**
 * Update relations
 * @param array $updates: list of records to update, indexed by reid
 *   each element is an associative array of new values, keyed by field name
 *   if a record is to be updated to all zeros, it is simply deleted.
 * @return string: a report of what got updated
 * Permissions and employer_ok cannot be updated by the agent and employee_ok cannot be updated by the employer
 */
function updateRelations($updates) {
//debug($updates);
  $cuid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(u\just('main_uid agent_uid permission employee_ok full_name', $data['original'])); 
    unset($data['original']);
    $isAgent = ($cuid != $main_uid);
    extract(u\just('employee_ok employer_ok permission is_owner', $data), EXTR_PREFIX_ALL, 'd');
    
    if (!$isAgent) r\reverseRelations($main_uid, $agent_uid, $data, 'employee_ok'); // claiming to be an employee
    if ($isAgent) r\reverseRelations($main_uid, $agent_uid, $data, 'permission employer_ok is_owner'); // giving employer permission -- create a proper record
    
    $reidSub = $subs = compact('reid');
    $changes = '';
    foreach ($data as $field => $value) {
      if ($isAgent and $field != 'employee_ok') return u\tt('bad relations field'); // prevent hack
      $changes .= ($changes ? ',' : '') . "$field=:$field";
      $subs += array($field => $value);
    }
    $crit = 'reid=:reid';
    if (!empty($data)) r\dbQ("UPDATE r_relations SET $changes WHERE $crit", $subs);

    $msg = 'updated relation';
    if (!r\dbLookup('permission>0 OR employer_ok OR employee_ok OR is_owner', 'r_relations', $crit, $reidSub)) {
      r\dbQ("DELETE FROM r_relations WHERE $crit", $reidSub);
      $pot = '(:main_uid, :agent_uid)';
      if (!r\dbLookup(1, 'r_relations', "main_uid IN $pot AND agent_uid IN $pot", compact('main_uid', 'agent_uid'))) $msg = 'deleted relation';
    }
    $otherName = $full_name;
    $report .= u\tt($msg, compact('otherName')) . '<br>';
  }
  return $report;
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
 * @param int $zxid: the transaction number (negative for asif transactions)
 */
function undoTx($zxid) {
  if (isFrontend()) return (array) backend('undoTx', compact(u\ray('zxid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend

  $old_tid = zxid2tid($zxid);
  $oldRecord = lastTx('xid=:zxid', compact('zxid'));
  if ($error = r\nonoUndo($oldRecord, $subs)) return array($error, $subs); // check nono before confirm AND here

  extract($oldRecord, EXTR_PREFIX_ALL, 'old');
  $data = unserialize($old_data);

  if ($old_state == TX_PENDING or $old_state == TX_DENIED) { // it's an invoice - cancel simply by changing the transaction state
// function undo_invoice($zxid, $oldRecord, $old_otherUid, $by_me) // return $solution
    if ($old_by_me) {
      list ($new_state, $solution) = array(TX_DELETED, t('deleted')); // abandon the attempt to invoice
      r\notify($old_otherUid, $old_toMe ? 'invoice-canceled' : 'offer-canceled', r\txReportArgs($oldRecord));
    } else {
      list ($new_state, $solution) = array(TX_DENIED, t('marked "denied"'));
      r\notify($old_otherUid, $old_toMe ? 'offer-refused' : 'invoice-denied', r\txReportArgs($oldRecord));
    }
    r\setTxState($new_state, $zxid);
  } elseif (!$old_toMe and !$old_by_me) { // someone charged (either unilaterally or I approved it)
// function dispute($zxid, $oldRecord, $old_otherUid)
    r\setTxState(TX_DISPUTED, $zxid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($old_otherUid, 'charge-disputed', r\txReportArgs($oldRecord));
  } else {
    $otherAcct = r\tx2acct($old_toMe ? array($old_payer, $old_payer_agent, $data) : array($old_payee, $old_payee_agent, $data));
    return reverse($zxid, $old_amount, $otherAcct, $data, $old_goods, $old_tid, $old_toMe); // create an offsetting transaction
  }

  $amount = u\formatAmount($old_amount);
  $tofrom = !$old_toMe ? t('to') : t('from');
  $otherUid = $old_otherUid;
  $other = r\userField('full_name', $otherUid);
  $success = TRUE;
  return array("report undo", compact(u\ray('success old_tid new_tid amount other tofrom solution otherUid')));
}

/**
 * Create an offsetting transaction.
 */
function reverse($zxid, $amount, $acct, $data, $old_goods, $old_tid, $toMe) {
//    print_r(compact(u\ray('zxid amount acct data old_tid toMe'))); die();
  u\EXPECT(compact(u\ray('zxid amount acct data old_tid toMe')), 'zid float acct assoc|empty string bool');

  if (!is_array($data)) $data = array();
  $old_data = $data; // remember original data from transaction we're reversing
  $type = $toMe ? 'payment' : 'charge';
  $my_note = "reverses #$old_tid";
  $other_note = str_replace($old_tid, zxid2tid($zxid, $acct->id), $my_note);
  $what = u\order($type == 'payment', $my_note, $other_note);
  if (@$data['rebate'] or @$data['bonus']) list ($data['rebate'], $data['bonus']) = array(-$data['bonus'], -$data['rebate']);
  $data['zxid'] = $zxid; // remember what we're reversing (especially to get the realness right)
  list ($message, $args, $confirm) = transfer($type, $acct, $amount, $what, $old_goods, TRUE, $data);

  if (@$args['success']) {
    list ($new_tid, $new_zxid) = array($args['tid'], $args['zxid']);
    u\EXPECT($new_zxid * $zxid > 0, 'reverse: wrong reality');
    $otherTid = zxid2tid($new_zxid, $acct->id);
    $old_data['undone'] = TRUE;
    $old_data['changes'][] = array(time(), r\acct()->agent, 'undo', $new_zxid);
    $old_data = serialize($old_data);
    $my_note = "(reversed by #$new_tid) ";
    $other_note = str_replace($new_tid, $otherTid, $my_note);
    list ($payer_note, $payee_note) = u\order($type == 'payment', $other_note, $my_note);
    $sql = <<<EOF
      UPDATE {zxs} SET 
        payer_for=CONCAT(:payer_note, payer_for), 
        payee_for=CONCAT(:payee_note, payee_for),
        data=:old_data
      WHERE xid=:zxid
EOF;
    r\dbQ($sql, compact(u\ray('payer_note payee_note zxid old_data')));
    $message = 'report undo|' . $message;
    $args['solution'] = 'reversed';
  }
  
  return array($message, $args);
}

/**
 * Request US Dollars for rCredits or vice versa
 * @param string $currency: 'r' or 'usd'
 * @param numeric $amount: how much
 * @param boolean $confirmed: has user confirmed this request
 * @return array(message index, subs, needs confirmation)
 * @todo: require Dwolla account, rather than bank account
 */
function get($currency, &$amount, $confirmed) {
  $cuid = r\acct()->id;
  if (isFrontend()) return (array) backend('get', compact(u\ray('currency amount confirmed')));
  if (is_array($currency)) extract($currency); // called from api or backend
  
  if (!access('manage account')) return array('no access get', array(), FALSE);
  $amount = str_replace(',', '', trim($amount)); // ignore commas
  if (!is_numeric($amount)) return array('bad amount', array(), FALSE);
  if ($amount < 0) return array('negative transfer', array(), FALSE);
  if ($amount > R_MAX_AMOUNT) return array('amount too big', array(), FALSE);
  if ($currency == 'r') $amount = min(R_MAX_AMOUNT, $amount);
  $request = u\formatAmount($amount);
  $success = TRUE;
  if ($currency == 'r') {
// (catch this only when demand is high)      if ($my_amount < R_MIN_DEMAND) return array('demand too small');
    r\demand($cuid, $amount);
    return array('your demand', compact(u\ray('request success')), FALSE);
  }

  // $currency == 'usd'
  extract((array) creditInfo(), EXTR_PREFIX_ALL, 'info');
  if ($amount > $info_cashable) return array("can't cash incentives", $info_fancy, FALSE);
  if (r\userField('bank_account_status') != R_BANK_VERIFIED) return array('bank unverified', array(), FALSE);
  if (!r\isRTrader()) return array('get usd inactive', $info_fancy, FALSE);
  
  $type = TX_TRANSFER;
  $payer = $cuid;
  $payee = $payer_agent = $payee_agent = r\userField('community_uid', $payer);

  $taking = FALSE;
  $for = t('USD');

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\transact(compact(u\ray('type amount payer payee payer_agent payee_agent for taking')));
  $balance = u\formatAmount($info_balance - $amount);
  if ($confirmed) return array('report get usd', compact(u\ray('request balance success')), FALSE);
  
  $dbtx->rollback();
  return array('confirm get usd', compact(u\ray('request success')), TRUE);
  //*** End DBTX
}

/**
 * Pay the given invoice
 * @return array(string $message, array $args)
 */
function payInvoice($zxid) {
  if (isFrontend()) return (array) backend('payInvoice', compact(u\ray('zxid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend

  if (!access('buy and sell')) return array('no buy and sell', array());
  if (!($result = lastTx('xid=:zxid AND state=:TX_PENDING', compact('zxid')))) return array('no such tx', array());
  extract($result, EXTR_PREFIX_ALL, 'old');
// (not needed)  if ($zxid < 0 and (r\isRTrader($old_payer) and r\isRTrader($old_payee))) return array("can't pay asif invoice");
  $acct = r\acct($old_payee, $old_payee_agent);
  
  /*** Begin DBTX ***/ $dbtx = db_transaction();
  list ($message, $args, $confirm) = transfer('invoice payment', $acct, $old_amount, $old_payee_for, $old_goods, TRUE, compact('zxid'));
  if (@$args['success']) r\setTxState(TX_DELETED, $zxid); // delete the invoice
  /*** End DBTX ***/ unset($dbtx); // $dbtx->commit() is disallowed

  return array($message, $args);
}

/**
 * Create a transaction
 * @param string $ttype: payment, charge, or invoice payment
 * @param array $acct: acct object for the other party
 *   OR 'ALL' (paying employees)
 * @param numeric $requestAmount: the transaction amount
 * @param string $for OR array($payer_for, $payee_for): description of the transaction's purpose (user input)
 * @param bool $goods: is this for real goods and services
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE)
 * @param array $data: associative array of info about the transaction this request is reversing (if any), indexed by any of:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount) [rebate and bonus are always both set or neither]
 *   'zxid': the signed transaction id of the related transaction (invoice or transaction being undone)
 *   'preapproved': the transaction is prapproved (from some other region)
 *   'pay_all': the transaction is part of a "pay all" (bulk employee payment)
// * @param bool $force_asif: make this an As-If transaction, even if both parties are rTraders
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $paying_invoice, $goods (the transaction is an exchange for real stuff),
 * $shortfall (how inadequate the payer's balance is)
 */
function transfer($ttype, $acct, $requestAmount, $for, $goods, $confirmed = TRUE, $data = array()) {
  if (isFrontend()) return (array) backend('transfer', compact(u\ray('ttype acct requestAmount for confirmed data')));
  if (is_array($ttype)) extract($ttype); // called from api or backend

  u\EXPECT(in_array($ttype, u\ray('payment,charge,invoice payment')), 'bad transaction type');
  if ($acct == 'ALL') return $ttype == 'payment' ? r\payEmployees($requestAmount, $for, $confirmed) : array('no ALL', array(), FALSE);
  $cacct = r\acct();
  $cuid = $cacct->id;
  $taking = ($ttype == 'charge');

  // Check permissions
  u\EXPECT(compact(u\ray('ttype acct requestAmount confirmed data')), 'string acct float bool assoc');
  list ($my_access, $other_access) = u\order($taking, 'sell', 'buy and sell');
  if (!access($my_access)) return array("no $my_access", array(), FALSE);
  if (!access($other_access, $acct)) return array("other no $other_access", array(), FALSE);
  
  // Analyze who's buying or selling
  list ($uid, $foreign_uid) = txPerson($acct->id, $acct->region);
  list ($agent_uid, $foreign_agent) = txPerson($acct->agent, $acct->agentRegion);
  $foreign = compact($foreign_uid ? 'foreign_uid' : '', $foreign_agent ? 'foreign_agent' : '');
  if ($uid == $cuid) return array('no self-trading', array(), FALSE);
  list ($payer, $payee) = u\order($taking, $uid, $cuid);
  list ($payer_agent, $payee_agent) = u\order($taking, $agent_uid, r\acct()->agent);

  // Sort out what type of transfer this is
  $paying_invoice = ($ttype == 'invoice payment');
  $type = TX_TRANSFER;
  $preapproved = (@$data['preapproved'] or !$taking or access('charge others')); // paying OR permitted to charge unilaterally
//  $state = ($preapproved and !@$data['pay_all'])? TX_DONE : TX_PENDING;
  $virtual = ($goods and !$taking and ($cacct->hasBit(BIT_VIRTUAL_ALL) or ($cacct->hasBit(BIT_VIRTUAL_EMPLOYEES) and r\isEmployee($uid))));
  $state = ($preapproved and !$virtual)? TX_DONE : TX_PENDING;
  $real = (!(@$data['zxid'] < 0) and r\isRTrader() and r\isRTrader($uid));
  
  // Check the amount and calculate any balance shortfall
  if ($requestAmount <= 0) return array('nonpositive transfer', array(), FALSE);
  if ($requestAmount > R_MAX_AMOUNT) return array('amount too big', array(), FALSE);

  $totalHit = $requestAmount - @$data['rebate']; // leave room for reversing rewards (minus a negative)
  if ($shortfall = r\shortfall($taking ? $uid : $cuid, $totalHit, $preapproved, $real)) {
    $short = u\formatAmount($shortfall);
    $otherName = r\userField('full_name', $uid);
    if ($paying_invoice) return array('short payment', compact('short'), FALSE); // no partial payment of invoice
    if (!$goods) return array($taking ? 'short cash from' : 'short cash to', compact('short', 'otherName'), FALSE); // no partial cash trans
    if ($shortfall >= $requestAmount) return array($taking ? 'broke from' : 'broke to', compact('otherName'), FALSE);
  }
  $amount = $requestAmount - $shortfall;
  
  // Figure rewards and other extra info
  if (!$for) $for = $goods ? 'goods and services' : 'cash/loan/etc.';
  $rebate = (string) ($goods ? r\rebate($payer, $amount) : 0); // force string, to avoid long serializations
  $bonus = (string) ($goods ? r\bonus($payee, $amount) : 0);
  if (!isset($data['rebate'])) $data += compact('rebate', 'bonus');
  $data += $foreign;
  unset($data['preapproved']); // probably don't need this (but keep zxid of related transaction, if any)
  $data = serialize($data);

  /*** Begin DBTX ***/ $dbtx = db_transaction();
  $txs = r\transact(compact(u\ray('type amount payer payee payer_agent payee_agent for data taking goods state')));
  $zxid = $txs[0]['zxid'];
  if (!$confirmed) $dbtx->rollback();
  /*** End DBTX ***/ unset($dbtx); // $dbtx->commit() is disallowed
  
  // Report results
  $args = r\txReportArgs($txs[0], 1, $shortfall);
  
  if ($confirmed) { // notify other party by email about any confirmed transaction
    $notice = $taking ? ($preapproved ? 'new-charge' : 'new-invoice')
      : ($virtual ? 'new-offer' : 'new-payment');
    r\notify($uid, $notice, $args);
    $message = $goods ?
        ($shortfall ? 'report short transaction' 
          : ($preapproved ? ($virtual ? 'report virtual' : 'report transaction')
          : 'report invoice'))
      : ($preapproved ? 'report exchange' : 'report exchange request'); // no possible shortfall on confirmed cash exchange (see above)
  } else {
    $message = $goods ? $ttype : 'exchange';
    if ($shortfall) $args['amount'] = u\formatAmount($requestAmount); // restore original amount before getting confirmation
  }
  return array($message, $args, !$confirmed);
}

function txPerson($id, $region) {return r\isForeign($region) ? array($region, $id) : array($id, NULL);}

/**
 * Record a transfer to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer (defaults to signup)
 * @param int $amount: how much to transfer
 * @param bool $forReal: actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE if the current account does not have permission to do this)
 */
function fund($uid, $type = TX_SIGNUP, $amount = R_STARTING_BALANCE, $confirmed = TRUE) {
  if (isFrontend()) return backend('fund', compact(u\ray('uid type amount')));
  if (is_array($uid)) extract($uid); // called from api or backend

  $payee_agent = $payee = $uid;
  $cacct = r\acct(); // null, if just registering
  $ctty = r\userField('community_uid', $payee);
  if ($type > TX_AUTOMATIC and !access('community admin')) return FALSE; // incentives given only automatically or by community agent
  if (!$confirmed) return TRUE;
  
  $payer = $ctty;
  $payer_agent = @$cacct->id < 0 ? $cacct->agent : $ctty;
  $taking = ($type >= TX_FINE);
  $for = $GLOBALS['txTypes'][$type];
  r\transact(compact(u\ray('type amount payer payee payer_agent payee_agent for taking')));
  return $amount;
}

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function addCell($number, $validate = FALSE) {
  $cuid = r\acct()->id;
  if (isFrontend()) return backend('addCell', compact(u\ray('number validate')));
  if (is_array($number)) extract($number); // called from api or backend

  $numberInternal = u\formatPhone($number, '+n');
  $numberPretty = u\formatPhone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$numberPretty) return array('bad phone', $subs);
    if ($result = r\dbQ('SELECT * FROM r_sms WHERE number=:numberInternal', compact('numberInternal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $cuid) return array('already cell', $subs);
      $accountName = accountName($old_uid);
      if (!isTempAccount($old_uid)) return array('cell taken', compact(u\ray('number accountName')));
    }
    return NULL;
  }
  
  $uid = $cuid;
  $status = SMS_PRIMARY;
  $number = $numberInternal;
  $info = compact(u\ray('uid number status'));
  if (!mergeCheck(compact('number'))) \drupal_write_record('r_sms', $info);
  if (!r\acct()->phone) r\userUpdate(array('phone' => $number)); // use it as contact phone, if none yet
  $number = $numberPretty;
  return array('report new cell', compact('number'));
}

function deleteCell($number) {
  if (isFrontend()) return backend('deleteCell', compact(u\ray('number')));
  if (is_array($number)) extract($number); // called from api or backend

  $number = u\formatPhone($number, '+n'); // internal format
  r\dbQ('DELETE FROM r_sms WHERE number=:number LIMIT 1', compact('number'));
  $number = u\formatPhone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant.
 *
 * @param string $who: how the user referred to the trader
 * @param boolean $allow_ALL: all $who to be "all" (meaning all employees)
 * @param string $self_message: index to error message for identifying oneself
 * UNUSED @param boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   'ALL', if the user typed "all"
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible full_names (0 or more) indexed by uid)
 *   (the caller can test which type of return, with "if (isAcct($returned_value))
 */
function identify($who, $allow_ALL = FALSE, $self_message = 'no self-trading') {
  if (isFrontend()) {
    $result = backend('identify', compact(u\ray('who allow_ALL self_message')));
    return u\isAcct($result) ? $result : ((array) $result);
  }
  if (is_array($who)) extract($who); // called from api or backend

  $create = FALSE; // not currently used
  $who = trim($who);
  if (($all = strtoupper($who)) == 'ALL') return $allow_ALL ? $all : array('no ALL', array(), array());
  $cuid = r\acct()->id;

  if ($phone = u\formatPhone($who, '+n')) $who = $phone;
  
  $quid = strtoupper($who); // in case this is a quid (abbreviated or not)
  if (strpos('.:', substr($quid, 0, 1)) !== FALSE) { // allow abbreviations within this region
    list($quid) = explode(' ', R_SERVER_ID . $quid, 2); // take just the first word, in case this is from a list showing QUID and FULL_NAME
  }

  if (u\isQid($quid)) {
    if (!($acct = r\acct($quid))) return array('unknown member', compact('who'), array());
    if ($acct->id == $cuid) return array($self_message, array(), array());
    return $acct; // qids are easy
  }

  if (($short = u\shortName($who)) === '') $short = '-'; // abbreviations must not be null
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, u.full_name FROM {users} u LEFT JOIN {r_sms} s ON u.uid = s.uid 
    WHERE :who IN (s.number, u.phone, u.mail) 
    OR INSTR(u.name, :short)
    ORDER BY u.full_name
EOF;
//debug(compact('sql', 'who', 'phone', 'short'));
  $result = r\dbQ($sql, compact(u\ray('who short')))->fetchAllKeyed(0, 1);
//  $list = '';
  foreach ($result as $uid => $full_name) {
    if ($uid != $cuid) { // not self
//      $list .= " &nbsp; &nbsp; &nbsp; &nbsp; $full_name<br>";
    } elseif (count($result) == 1) { // self is the only result
      return array($self_message, array(), array()); 
    } else unset($result[$uid]); // self is one of many results, so ignore it
  }

  if(count($result) > 1) return array('ambiguous other', compact(u\ray('who')), $result);
  if(count($result) == 1) {
    foreach ($result as $uid => $other); // get the one entry
    return r\acct($uid);
  }

//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (\valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown member', compact('who'), array());
  return new r\acct(array($key => $who));
}

/**
 * Return financial information about the given account on this server
 * param array $args: associative array of non-default options:
 *  uid: uid for the account to return information for (DEFAULTS to cuid)
 *  state: string list of transaction states to return (DEFAULT ':TX_DONE,:TX_DISPUTED')
 *  real: return real transactions instead of asif (DEFAULT real for rTraders, asif for non-rTraders)
 *  asof: last date to include (defaults to '', meaning unrestricted)
 *
 * @return
 *   standard object of credit account information (see $result_keys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function creditInfo($args = array()) {
  if (isFrontend()) {
    $info = backend('creditInfo', $args);
    $info->fancy = (array) $info->fancy; // backend turns the whole shebang into an object
  }
  extract($args, EXTR_PREFIX_ALL, 'my'); // uid, state, real, asof

  $cuid = r\acct()->id;
  $uid = @$my_uid ?: $cuid;
  $state = @$my_state ?: ':TX_DONE,:TX_DISPUTED';
  $real = isset($my_real) ? $my_real : r\isRTrader($uid);
  $asof = @$my_asof ?: '';
  $quid = r\quid($cuid, $cuid);
  
  $result_keys = 'grossIn grossOut grossInProper balance available unavailable rewards cashable totalDemand minBalance quid';
  $minBalance = r\userField('min_balance', $uid);

  $sql = <<<EOF
    SELECT 
    SUM(IF(t.payer=:uid, t.amount, 0)) AS grossOut, 
    SUM(IF(t.payee=:uid, t.amount, 0)) AS grossIn,
    SUM(IF(t.type=:TX_TRANSFER, 0, t.amount)) AS rewards
    FROM {zxs} t
    WHERE (t.payer=:uid OR t.payee=:uid) AND t.state IN($state)
EOF;
  if ($asof) $sql .= " AND t.created<=:asof";

  extract(r\dbQ($sql, compact(u\ray('uid asof real')))->fetchAssoc());
  $grossInProper = $grossIn - $rewards;
  $balance = $grossIn - $grossOut;
  $unavailable = 0; // for now
  $available = ($balance - $minBalance) - $unavailable;
  $cashable = $balance - $rewards;
  $totalDemand = r\totalDemand();
  $result = compact(u\ray($result_keys));
  $fancy = array();
  foreach ($result as $key => $value) $fancy[$key] = is_numeric($value ?: 0) ? u\formatAmount($value) : $value;
  if ($totalDemand >= 1000000) $fancy['totalDemand'] = '$' . number_format(($totalDemand - 9999) / 1000000, 2) . ' million';
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * Find and merge into the current account any temporary accounts with the same shortname or specified criteria
 * @return int: the number of accounts that were merged.
 */
function mergeCheck($info) {
  if (isFrontend()) return backend('mergeCheck', $info);
  
  $cuid = r\acct()->id;
  $info['name'] = r\userField('name');
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $where = 'name=:name';
  foreach (u\ray('number website phone fax') as $field) {
    if (@$info[$field]) $where .= " OR u.$field=:$field"; else unset($info[$field]);
  }
  $where = str_replace('u.number', 'sms.number', $where);
  $sql = "SELECT DISTINCT u.uid FROM users u LEFT JOIN r_sms sms ON sms.uid=u.uid WHERE u.uid<>:cuid AND (NOT u.pass) AND ($where)";
  $result = r\dbQ($sql, $info + compact('cuid'));
  $found = 0;
  while ($row = $result->fetchAssoc()) $found += r\mergeAccounts($row['uid']);
  return $found;
}

/**
 * Return a list of account choices for the current user (NOT the current credit account)
 * @return an array of account names, indexed by uid (FALSE if none)
 * @todo: recurse, to include access to the accounts that the shared accounts have access to
 *   maybe like this: SELECT main_uid, MAX(permission) WHERE agent_uid=:agent 
 *     OR agent_uid IN (SELECT main_uid FROM r_relations WHERE agent_uid=:agent) GROUP BY main_uid
 */
function accountChoices() {
  if (isFrontend()) return (array) backend('accountChoices');
  
  if (!($cacct = r\acct())) return FALSE;
  $agent = $cacct->agent;
  $primary = array((string) $agent => (count($GLOBALS['share permissions']) - 1)); // max access for primary user account

  $result = r\dbQ("SELECT main_uid, permission FROM r_relations WHERE agent_uid=:agent", compact('agent'));
  return $primary + $result->fetchAllKeyed(0, 1);
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
  if (isFrontend()) return (array) backend('todo', compact(u\ray('number todo nonce')));
  if (is_array($number)) extract($number); // called from api or backend
  
  if ($todo == 'nothing') {
    return (object) r\dbLookup('nonce,todo', 'r_sms', 'number=:number', compact('number'));
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
  $cuid = r\acct()->id;
  if (isFrontend()) return backend('lastTx', compact(u\ray('where subs')), $cache);
  if (is_array($where)) extract($where); // called from api or backend

  $basic = "$cuid IN (payer, payee) AND type NOT IN (:TX_DELETED, :TX_REBATE, :TX_BONUS)";
  $sql = "SELECT * FROM {zxs} WHERE ($basic) AND ($where) ORDER BY zxid DESC LIMIT 1"; // make sure it's for $cuid
  if (!($result = r\dbQ($sql, $subs)->fetchAssoc())) return FALSE;
  $toMe = ($result['payee'] == $cuid);
  $by_me = ($result['taking'] == $toMe);
  $otherUid = $result[$toMe ? 'payer' : 'payee'];
  $zxid = r\zxid($result['xid'], r\zxs());
  unset($result['xid']);
  return $result + compact(u\ray('by_me toMe otherUid zxid'));
}

/**
 * Return the user's sequential transaction number
 * @param int $zxid: the transaction record number (negative for asif)
 * @param int $uid: the user's uid (DEFAULTs to current account)
 * @return string: the user's transaction number, preceded by "a" for asif transactions
 */
function zxid2tid($zxid, $uid = '') {
  if (!$uid) $uid = r\acct()->id;
  if (isFrontend()) return backend('zxid2tid', compact(u\ray('zxid uid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend
  
  $crit =  ":uid IN (payer, payee) AND xid<=:zxid AND type NOT IN (:TX_REBATE, :TX_BONUS)";
  return ($zxid < 0 ? 'a' : '') . r\dbLookup('COUNT(xid)', '{zxs}', $crit, compact('uid', 'zxid'));
}

function accountName($uid) {
  if (isFrontend()) return backend('accountName', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid); // called from api or backend
  
  $acct = r\acct($uid);
  return $acct->full_name . ' ' . u\hug($acct->mainQid);
}

/**
 * Return all the permissions for the specified account
 * @param array $acct: acct object for the other party
 *   OR a qid or simple uid on the current server
 * @return simple array of that account's permissions
 * @todo: move this to acct class
 */
function accesses($acct = '') {  
  if (isFrontend()) {
    $result = backend('accesses', compact(u\ray('acct')), (bool) $acct);
    return is_array($result) ? $result : array();
  }
  if (is_array($acct)) extract($acct); // called from api or backend

  if (!($cacct = r\acct())) return array(); // no access for ANYONE, if anonymous user
  if (!$acct) $acct = $cacct;
  if (!u\isAcct($acct)) {$uid = $acct; $acct = r\acct($uid);}
  if (!$acct) return array();
  $perms = array();

  $cans = $GLOBALS['share permissions'];
  if (!$acct->proSe()) {
    extract((array) $acct, EXTR_PREFIX_ALL, 'my'); // especially id and agent
    $can = r\dbLookup('permission', 'r_relations', 'main_uid=:my_id AND agent_uid=:my_agent', compact('my_id', 'my_agent'));
  } else $can = count($cans) - 1;
  for ($i = 1; $i <= $can; $i++) $perms[] = $cans[$i];

  if ($acct->hasBit(BIT_ADMIN) or $acct->id == 1) $perms[] = 'admin';
  
  if ($acct->id < 0 and !$acct->proSe() and $can >= PERM_MANAGE) $perms[] = 'community admin'; // managing community account
  if ($acct->hasBit(BIT_ADMIN) or $acct->id == 1) $perms[] = 'community admin'; // temporary

  if ($acct->hasBit(BIT_PARTNER)) $perms[] = 'partner';
  if ($acct->hasBit(BIT_STEWARD)) $perms[] = 'steward';
  if ($acct->hasBit(BIT_RTRADER)) $perms[] = 'rtrader';
  if ($acct->hasBit(BIT_CHARGE)) $perms[] = 'charge others';
  if ($acct->account_type != R_PERSONAL) $perms[] = 'company'; // ad hoc permissions

  return $perms;
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
  $key = r\dbQ($sql, compact('table', 'db')); */
//  if (!$cache) die('no cache getRecords');
  if (isFrontend()) return (array) backend('getRecords', compact(u\ray('table criteria subs fields')), $cache);
  if (is_array($table)) extract($table); // called from api or backend
  
//  echo " SELECT $fields FROM $table WHERE $criteria ";
  return r\dbQ("SELECT $fields FROM $table WHERE $criteria", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

function isTempAccount($uid) {
  if (isFrontend()) return backend('isTempAccount', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid); // called from api or backend
  return !r\userField('pass', $uid);
}

function isTempName($name) {
  if (isFrontend()) return backend('is_temp_name', compact(u\ray('name')));
  if (is_array($name)) extract($name); // called from api or backend
  return u\abbreviates(R_TEMP_NAME_PREFIX, $name);
}

function testbe($function = '', $arg1 = '', $arg2 = '', $arg3 = '') {
  global $testing_backend, $firewall;
  $firewall = R_FIREWALL; // this is not currently necessary
  if (!$function) return $testing_backend;
  $function = "rCredits\\be\\$function";
  $testing_backend = TRUE;
  debug($function($arg1, $arg2, $arg3)); // keep this
}

/**
 * Determine whether we are in the web browser frontend or not.
 * @todo: also check the rSMS path (another backend)
 */
function isFrontend() {
  if (!R_SEPARATE_BACKEND) return FALSE; // disable separation of frontend and backend
  return (!testbe() and (strpos($_SERVER['REQUEST_URI'], 'rcredits/api') === FALSE));
  // (FAILS in menu system of backend) global $is_backend; return !@$is_backend;
}

function backend($op, $info = array(), $cache = FALSE) {
  global $base_url, $firewall;
  u\EXPECT(isFrontend(), 'backend recursion');
  if (!user_is_logged_in()) return FALSE; // probably checking permission
//  if ($op == 'getRecords' and !$cache) die(print_r(debug_backtrace(), 1));

  $cuid = r\acct()->id;
  u\EXPECT(@$cuid, 'no cuid in backend!');
  if (empty($info)) $cache = TRUE;
  
  $agent = r\acct()->agent;
//  $cache = FALSE; // debug
//  debug("cache=$cache cachekey=rcredits_$op$cuid:" . serialize($info));
  $skey = "{$op}_{$agent}_{$cuid}_" . base64_encode(serialize($info)); // htmlspecialchars prevents session key failure (PHP bug?)
  if ($cache and (($result = r\Web\svar($skey)) !== NULL)) return $result;
//  debug($skey . $result);
//  debug("cache:$cache $op: " . json_encode2($info) . ($cache ? " result:$result" : ''));
//  debug($result);
  $password = r\acct($agent)->pass;
  $firewall = R_FIREWALL;
//  foreach ($info as $key => $value) if (strpos($key, 'uid') !== FALSE) $info[$key] = r\quid($value); // convert to qid
//  $basic = array('agent' => r\quid($agent), 'cuid' => r\quid($cuid));
  $info += compact(u\ray('cuid agent op password firewall')); // operation, credentials, and account
  $host = strpos($base_url, 'localhost/') !== FALSE ? 'http://localhost/devcore' : 'http://devcore.rCredits.org';
  $api_result = R_SEPARATE_BACKEND ? u\httpRequest("$hostrcredits/api/", $info) : backend2($info);
//debug($api_result);
  u\log('out', $info + compact('api_result'));
  if ($cache) r\Web\svar($skey, $api_result);
  return $api_result;
}

// Shortcut functions

/**
 * Say whether the account has the specified permission
 * @param string $access: the queried permission. This could be any of three types:
 *   a standard Drupal permission, queryable through user_access()
 *   a shared account permission, stored in r_relations (cached in $_SESSION['rcredits_accountChoices'], but we're not trusting that)
 *   an ad hoc permission handled directly by this function
 * @param array $acct: acct object for the other party (DEFAULT current account)
 *   OR a simple uid on the current server
 * @return boolean: TRUE if the user has the permission
 */
function access($access, $acct = '') {
  u\EXPECT(compact(u\ray('access acct')), 'string&notempty acct|empty|id|qid');
  return in_array($access, accesses($acct));
}
