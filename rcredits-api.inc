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
 *   old: undo_tx($xid);
 *   new: backend('undo_tx', $xid);
 * backend() simply posts JSON data to the Back End and receives a JSON response, which it decodes and returns.
 * The mobile app will have a similar backend function.
 * rSMS is already a purely backend module
 *
 * In the Back End, we will receive a POST to the /api page, which will run external_request().
 * The external_request() function will decode the JSON request, then will call the appropriate function.
 * For example:
 * {"op":"undo_tx","xid":123}
 * becomes r\undo_tx(123, 'web');
 * which then echos a JSON reply by calling r\frontend($info);
 * The Back End will accept web requests only from the right IP.
 * Eventually the Back End will listen only on a certain port (not 80).
 *
 * No Drupal login is required in Back End. (just pass qid(cid) and qid(user_id),password)
 * All uids are passed as qids (qid = r\qid(uid) = R_COMMUNITY_ID . n2a(uid))
 */

namespace rCredits\API; // typically abbreviated as "api"
use \rCredits as r;
use \rCredits\Utilities as u;

define('R_FIREWALL', 'lkj234lisufsd89072osi%!@J@$K'); // nobody can use the backend without knowing this code

function api2($result) {die(json_encode($result));}
function api_error($error) {api2(compact('error'));}

function api($args = FALSE) {
  global $cid, $userid, $firewall;

  if (!$args) $args = $_POST;
  extract($args);
  if ($firewall != R_FIREWALL) api_error('firewall system failure'); // give hackers some satisfaction by saying 'failure'
  $cid = r\qid($cid); unset($args['cid']);
  $userid = r\qid($userid); unset($args['userid']);
  if (!$cid or !$userid) api_error('user validation error');
  $crit = 'shared_uid=:cid AND share_with=:userid';
  if ($cid != $userid and !r\db_lookup(1, 'r_shared_accounts', $crit, compact('cid', 'userid'))) api_error('sharing error');
  if (isset($uid)) $args['uid'] = $uid = r\qid($uid);
  
  $account = \user_load(@$userid);
  u\log('rAPI', 'in', $_POST);
  if (@!$password) api_error('login error');
  if (@$account->pass != $password) api_error('bad login'); // make these two error messages distinct but obscure

  if (!function_exists($op) or strpos($op, '\\')) api2('unknown op');
  api2($op($args));
  /*
  switch ($op) {
    case 'create_user': api2(create_user($info));
    case 'my_transactions': api2(my_transactions($args));
    case 'undo_tx': api2(undo_tx($xid));
    case 'get': api2(get($currency, $amount, $confirmed));
    case 'pay_invoice': api2(pay_invoice($xid));
    case 'transfer': api2(transfer($ttype, $uid, $requested_amount, $for, $confirmed, $rewards));
    case 'give_signup_incentive': api2(give_signup_incentive($uid));

    case 'add_cell': api2(add_cell($number, $validate));
    case 'delete_cell': api2(delete_cell($number));
    
    case 'identify': api2(identify($who, $create, $self_message));
    case 'credit_info': api2(credit_info($args));
    case 'make_username': api2(make_username($account, $name));
    case 'merge_check': api2(merge_check($info));
//    case 'make_credit_id': api2(make_credit_id($uid));
    case 'account_choices': api2(account_choices());
    case 'todo': api2(todo($number, $todo, $nonce));
    case 'last_tx': api2(last_tx($where, $subs = array()));
    case 'xid2tid': api2(xid2tid($xid, $uid));
    case 'account_name': api2(account_name($uid));
    case 'accesses': api2(accesses($uid));
    case 'get_records': api2(get_records($table, $criteria, $subs));
    case 'user_field': api2(user_field($field_name, $who, $subs));
    case 'is_temp_account': api2(is_temp_account($uid));
    case 'is_temp_name': api2(is_temp_name($name));
  }
*/
}

/**
 * Create a user account for transactions and/or contact
 *
 * @parm array $info: associative array of information about the user
 *   'phone' is contact phone (in standard +ddddddddddd format)
 *   'number' is cell phone (in standard +ddddddddddd format -- use for phone also)
 *   'email' is email address
 *   UNUSED (because it's hard to verify): 'website' is website address
 *   (This function assumes that the info is not sufficient to identify an existing user)
 * @return the uid of the new account
 */
function create_user($info) {
  if (is_frontend()) return backend('create_user', $info);
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  if (isset($my_number) and !isset($my_phone)) $my_phone = $my_number;
  if (isset($my_phone)) $info['phone'] = u\format_phone($my_phone);
  
  $info['mail'] = $info['init'] = @$my_email ?: '';
// Don't create a password (that's how we know it's a temporary account) $info['pass'] = \user_password();
  $info['full_name'] = @$my_full_name ?: '';
  $info['short_name'] = $short_name = ($my_short_name ?: (@$my_full_name ? u\short_name($full_name) : ''));
  $info['name'] =  @$my_name ?: (@$short_name ?: r\temp_name($info));
  $info['status'] = TRUE;
  $info['is_new'] = TRUE;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $account = \user_save(array(), $info);
//  make_credit_id($account->uid); // this is done separately, to assure uniqueness
  if (isset($my_number)) r\SMS\create($my_number, $account->uid, SMS_USER_CONFIRMED);
  unset($dbtx);
  //*** End DBTX

  if (!$account) throw new \Exception('failed to save user record');
  return $account->uid;
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
 * @return array: associative list of transactions (with negative xids, if they are not real transactions)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function my_transactions($args) {
  global $cid;
  
  if (is_frontend()) return backend(__FUNCTION__, $args);
  extract($args);
  
  $table = r\t_table($real);
  $omit_bonuses = @$include_bonuses ? '' : 'WHERE t.type NOT IN (:TX_REBATE, :TX_BONUS)';
  
  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid = IF(t.payer = $cid, t.payee, t.payer)) AS name,
      IF(t.payer = $cid, t.payer_for, t.payee_for) AS purpose
    FROM $table t 
    $omit_bonuses
    AND $cid IN (t.payer, t.payee) 
    AND (t.state IN(:TX_DONE, :TX_PENDING, :TX_DISPUTED) OR (t.state = :TX_DENIED AND t.payee = $cid))
    AND t.created>=:starting AND t.created<=:ending
    ORDER BY t.xid DESC
EOF;
  
  $result = r\db_q($sql, compact(u\ray('starting ending')))->fetchAllAssoc('xid', \PDO::FETCH_ASSOC);
  
  if (!$real) foreach ($result as $xid => $one) { // negate the keys for asif transactions
    $one['xid'] = -$xid;
    $result[-$xid] = $one;
    unset($result[$xid]);
  }
//  return array_flip(array_map(function($one) {return -$one;}, array_flip($result))); // negate the keys

  return $result;
}

/**
 * Undo the given transaction.
 * Situations:
 *   Transaction PENDING or DENIED
 *     I invoiced someone: change state to CANCELED
 *     Someone invoiced me: change state to DENIED
 *   Transaction DONE
 *     I paid someone: create an opposite transaction (with reverse rewards)
 *     I charged someone: create an opposite transaction
 *     Someone paid me: create an opposite transaction
 *     Someone invoiced me and I approved it: create an opposite transaction
 *     Someone charged me unilaterally: change the transaction state to DISPUTED (?)
 * @param int $xid: the transaction number (negative for asif transactions)
 * @param boolean $real: is the transaction real, that is to be undone
 */
function undo_tx($xid, $real = 'bug') {
  global $cid;
  if (is_frontend()) return (array) backend('undo_tx', compact(u\ray('xid')));
  if (is_array($xid)) extract($xid);
  
  $fields = 'state, creator, amount, payer, payee, data';
  $old_record = last_tx('xid=:xid', compact('xid'));
  if (empty($old_record)) return array('no such tx', array()); // $cid IN is a double check (might be an xid in r_asif)
  extract($old_record);
  $old_tid = xid2tid($xid);
  $tofrom = $payer == $cid ? t('to') : t('from');
  $other_uid = $payer == $cid ? $payee : $payer;
  $other = user_field('full_name', $other_uid);
  $old_data = $data = unserialize($data);
  if (@$data['undone']) return array('already undone', compact('old_tid'));
  
  if ($state == TX_PENDING or $state == TX_DENIED) { // it's an invoice - cancel simply by changing the transaction state
    if ($creator == $cid) { // created by me
      list ($new_state, $action) = array(TX_DELETED, t('deleted'));
    } else {
      list ($new_state, $action) = array(TX_DENIED, t('marked "denied"'));
      $payer_name = user_field('full_name'); // my name, for message to payee
      r\notify($other_uid, 'invoice-denied', $old_record + compact('payer_name'));
    }
    r\set_tx_state($new_state, $xid);
  } elseif ($creator != $cid and $payee == $cid and access('charge others', $creator)) { // someone charged me unilaterally
    r\set_tx_state(TX_DISPUTED, $xid); // mark it disputed
    $action = t('marked "disputed"');
    $payer_name = user_field('full_name'); // my name, for message to payee
    r\notify($other_uid, 'charge-disputed', $old_record + compact('payer_name'));
  } else { // create an offsetting transaction
    $type = $cid == $payee ? 'payment' : 'charge';
    $my_note = "reverses #$old_tid";
    $other_note = str_replace($old_tid, xid2tid($xid, $other_uid), $my_note);
    $what = u\order($type == 'payment', $my_note, $other_note);
    list ($data['rebate'], $data['bonus']) = array(-$data['bonus'], -$data['rebate']);
    $data['old_xid'] = $xid; // remember what we're reversing (especially to get the realness right)
    list ($message, $args, $confirm) = transfer($type, $other_uid, $amount, $what, TRUE, $data);
    if (@$args['success']) {
      $old_data['undone'] = TRUE;
      $old_data = serialize($old_data);
      list ($new_tid, $other_tid) = array($args['tid'], xid2tid($args['xid'], $other_uid));
      $my_note = "(reversed by #$new_tid) ";
      $other_note = str_replace($new_tid, $other_tid, $my_note);
      list ($payer_note, $payee_note) = u\order($type == 'payment', $other_note, $my_note);
      $sql = <<<EOF
        UPDATE r_txs SET 
          payer_for=CONCAT(:payer_note, payer_for), 
          payee_for=CONCAT(:payee_note, payee_for),
          data=:old_data
        WHERE xid=:xid
EOF;
      r\db_q($sql, compact(u\ray('payer_note payee_note xid old_data')));
      $message = 'report undo|' . $message;
      $args += array('action' => t('reversed'));
    }
    return array($message, $args);
  }

  $amount = u\format_amount($amount);
  return array("report undo", compact(u\ray('old_tid new_tid amount other tofrom action')));
}

/**
 * Request US Dollars for rCredits or vice versa
 * @param string $currency: 'r' or 'usd'
 * @param numeric $amount: how much
 * @param boolean $confirmed: has user confirmed this request
 * @return array(message index, subs, needs confirmation)
 */
function get($currency, &$amount, $confirmed) {
  global $cid;
  if (is_frontend()) return (array) backend('get', compact(u\ray('currency amount confirmed')));
  if (is_array($currency)) extract($currency);
  
  if (!access('can manage account')) return array('no access get', array(), FALSE);
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return array('bad amount', array(), FALSE);
  if ($amount < 0) return array('negative transfer', array(), FALSE);
  if ($currency == 'r') $amount = min(R_MAX_DEMAND, $amount);
  $request = u\format_amount($amount);
  $success = TRUE;
  if ($currency == 'r') {
// (catch this only when demand is high)      if ($my_amount < R_MIN_DEMAND) return array('demand too small');
    r\demand($cid, $amount);
    return array('your demand', compact(u\ray('request success')), FALSE);
  }

  // $currency == 'usd'
  extract((array) credit_info(), EXTR_PREFIX_ALL, 'info');
  if ($amount > $info_cashable) return array("can't cash incentives", $info_fancy, FALSE);
  if (user_field('bank_account_status') != R_BANK_VERIFIED) return array('bank unverified', array(), FALSE);
  if (!r\is_rTrader()) return array('get usd inactive', $info_fancy, FALSE);
  
  $type = TX_TRANSFER;
  $payer = $creator = $cid;
  $payee = variable_get('rcredits_community_uid');
  $for = t('cash/loan/etc.');
  $goods = FALSE;

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\transact(compact(u\ray('type amount payer payee for creator goods')));
  $balance = u\format_amount($info_balance - $amount);
  if ($confirmed) return array('report get usd', compact(u\ray('request balance success')), FALSE);
  
  $dbtx->rollback();
  return array('confirm get usd', compact(u\ray('request success')), TRUE);
  //*** End DBTX
}

/**
 * Pay the given invoice
 * @return array(string $message, array $args)
 */
function pay_invoice($xid) {
  if (is_frontend()) return (array) backend('pay_invoice', compact(u\ray('xid')));
  if (is_array($xid)) extract($xid);

  if (!access('can create transactions')) return array('no access create', array());
  $state = TX_PENDING;
  $result = last_tx('xid=:xid AND state=:state', compact('xid', 'state'));
  extract($result, EXTR_PREFIX_ALL, 'old');
  if ($old_xid < 0 and (r\is_rTrader($old_payer) and r\is_rTrader($old_payee))) return array("can't pay asif invoice");

  //*** Begin DBTX
  $dbtx = \db_transaction();
  list ($message, $args, $confirm) = transfer('invoice payment', $old_payee, $old_amount, $old_payee_for, TRUE);
  if (@$args['success']) {
    r\set_tx_state(TX_DELETED, $old_xid);
  } else $dbtx->rollback();
  
  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  return array($message, $args);
}

/**
 * Create a transaction
 * @param bool $ttype: payment, charge, or invoice payment
 * @param int $other_uid: user id of the other party
 * @param numeric $requested_amount: the transaction amount
 * @param string $for OR array($payer_for, $payee_for): description of the transaction's purpose (user input)
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE, used only for SMS interface)
 * @param array $rewards: associative array of reward percentages, indexed by 'rebate' and 'bonus' (default empty means standard rewards)
 *    $rewards is for undoing ($amount should also be negative)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $paying_invoice, $goods (the transaction is an exchange for real stuff),
 * $shortfall (how inadequate the payer's balance is)
 */
function transfer($ttype, $uid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  if (is_frontend()) return (array) backend('transfer', compact(u\ray('ttype uid requested_amount for confirmed rewards')));
  if (is_array($ttype)) extract($ttype);
  
  global $cid;
  if (!access('can create transactions')) return array('no access create', array(), FALSE);
  $real = @$rewards['xid'] < 0 ? FALSE : (r\is_rTrader() and r\is_rTrader($uid));
  $taking = ($ttype == 'charge');
//  if ($paying_invoice = ($ttype == 'invoice payment')) $ttype = 'payment';
  $paying_invoice = ($ttype == 'invoice payment');
  $confirm_type = $ttype;
  $type = TX_TRANSFER;
//  $preapproved = ($taking xor $requested_amount < 0) ? access('charge others') : TRUE;
  $preapproved = $taking ? access('charge others') : TRUE;
//  if ($requested_amount < 0 and empty($rewards)) return array('negative transfer', array(), FALSE);
  if ($requested_amount < 0) return array('negative transfer', array(), FALSE);
  $goods = (@$rewards['rebate'] or @$rewards['bonus']) ?: !u\for_cash($for);
  if (!$goods and !is_array($for)) $for = t('cash/loan/etc.');
  
  $available = credit_info(array('real' => $real, 'uid' => $taking ? $uid : $cid))->available;
  $amount = min($requested_amount, $available);
  $shortfall = max(0, $requested_amount - $amount); // how much needs to be done in a separate currency
  if ($shortfall and $preapproved) {
    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
    $short = u\format_amount($shortfall);
    $other = user_field('full_name', $uid);
    if ($paying_invoice) return array('short invoice', compact('short', 'other'), FALSE); // no partial payment of invoice
    if (!$goods) return array($taking ? 'short cash from' : 'short cash to', compact('short', 'other'), FALSE); // no partial cash trans
    if (ceil($shortfall) >= $requested_amount) return array($taking ? 'broke from' : 'broke to', compact('other'), FALSE);
    $shortfall = ceil($shortfall); // for goods and services, shortfall should be a round figure (no cents)
    $amount = $requested_amount - $shortfall;
  }
  
  list ($payer, $payee) = $taking ? array($uid, $cid) : array($cid, $uid);
  $rebate = $goods ? ('' . r\rebate($payer, $amount)) : '0'; // force string, to avoid long serializations
  $bonus = $goods ? ('' . r\bonus($payee, $amount)) : '0';
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;
  $state = $preapproved ? TX_DONE : TX_PENDING;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $txs = r\transact(compact(u\ray('type amount payer payee for data state goods')));
  $xid = $txs[0]['xid'];
  if (!$confirmed) $dbtx->rollback();
  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  $reward_index = $taking ? 2 : 1;
  $reward = @$txs[$reward_index]['amount'] ?: 0;

  $args = r\tx_report_args($taking, $amount, $shortfall, $reward, $uid, $xid, $goods);
  if (!$goods) {
    $message = !$confirmed ? 'exchange' : ($preapproved ? 'report exchange' : 'report exchange request');
    return array($message, $args, !$confirmed);
  }
  
  if ($confirmed) {
    $message = $shortfall ? 'report short transaction' : ($preapproved ? 'report transaction' : 'report invoice');
    if (!$shortfall and !$preapproved) {
      $payee_name = user_field('full_name'); // my name, for message to payer
      r\notify($uid, 'new-invoice', $txs[0] + compact('payee_name'));
    }
    return array($message, $args, FALSE);
  }
  if ($shortfall) $args['amount'] = u\format_amount($requested_amount); // restore original amount before getting confirmation
  return array($confirm_type, $args, TRUE);
}

/**
 * Record an incentive transaction for signing up
 * @param int uid: uid of the person signing up
 * @return int: the incentive amount
 */
function give_signup_incentive($uid) {
  if (is_frontend()) return backend('give_signup_incentive', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid);

  $type = TX_SIGNUP;
  $amount = R_STARTING_BALANCE;
  $payer = $creator = \variable_get('rcredits_community_uid');;
  $payee = $uid;
  $for = t('signup');
  r\transact(compact(u\ray('type amount payer payee for creator')));
  return $amount;
}

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function add_cell($number, $validate = FALSE) {
  if (is_frontend()) return backend('add_cell', compact(u\ray('number validate')));
  if (is_array($number)) extract($number);

  global $cid;
  $number_internal = u\format_phone($number, '+n');
  $number_pretty = u\format_phone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$number_pretty) return array('bad phone', $subs);
    if ($result = r\db_q('SELECT * FROM sms_user WHERE number=:number_internal', compact('number_internal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $cid) return array('already cell', $subs);
      //extract(r\db_q('SELECT full_name, credit_id FROM users WHERE uid=:old_uid', compact('old_uid'))->fetchAssoc());
      extract(user_field('full_name', $old_uid));
      $qid = r\qid($old_uid);
      $account_name = "$full_name ($qid)"; // maybe make the previous line and this a function
      if (!is_temp_account($old_uid)) return array('cell taken', compact(u\ray('number account_name')));
    }
    return NULL;
  }
  
  $uid = $cid;
  $status = SMS_USER_CONFIRMED;
  $number = $number_internal;
//  debug(!merge_check(compact('number')));
  if (!merge_check(compact('number'))) \drupal_write_record('sms_user', compact(u\ray('uid number status')));
  $account = \user_load($uid);
  if (!$account->phone) \user_save($account, array('phone' => $number)); // use it as contact phone, if none yet
  $number = $number_pretty;
  return array('report new cell', compact('number'));
}

function delete_cell($number) {
  if (is_frontend()) return backend('delete_cell', compact(u\ray('number')));
  if (is_array($number)) extract($number);

  $number = u\format_phone($number, '+n'); // internal format
  r\db_q('DELETE FROM sms_user WHERE number=:number LIMIT 1', compact('number'));
  $number = u\format_phone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant.
 *
 * @param string $who: how the user referred to the trader
 * @param boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
 *   If true, the other trader will be created only if specified by email or phone
 * @param string $self_message: index to error message for identifying oneself
 * @return one of the following:
 *   the other trader's uid
 *   an array whose elemnts are: an error index, an array of substitutions, an array of possible uids (0 or more)
 */
function identify($who, $create = FALSE, $self_message = 'no self-trading') {
  global $cid;
  if (is_frontend()) {
    $result = backend('identify', compact(u\ray('who cid create self_message')));
    return is_object($result) ? ((array) $result) : $result;
  }
  if (is_array($who)) extract($who);

  if ($phone = u\format_phone($who, '+n')) $who = $phone;
//  if (substr($who, 0, 7) == 'http://') $who = substr($who, 7);
//  if (substr($who, 0, 4) == 'www.') $who = substr($who, 4);

  if (($short = u\short_name($who)) === '') $short = '-'; // abbreviations must not be null
  if ($uid = r\qid($who)) return $uid == $cid ? array($self_message, array(), array($uid)) : $uid; // is it a qid?
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, u.full_name FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
    WHERE :who IN (s.number, u.phone, u.mail) 
    OR INSTR(u.short_name, :short)
    ORDER BY u.full_name
EOF;

  $result = r\db_q($sql, compact(u\ray('who short')))->fetchAllAssoc('uid', \PDO::FETCH_ASSOC);
  $list = '';
  foreach ($result as $uid => $one) {
    if ($uid != $cid) { // not self
      extract($one); // get full_name (and uid again)
      $qid = r\qid($uid);
      $list .= " &nbsp; &nbsp; &nbsp; &nbsp; $qid &nbsp;$full_name<br>";
    } elseif (count($result) == 1) { // self is the only result
      return array($self_message, array(), array()); 
    } else unset($result[$uid]); // self is one of many results, so ignore it
  }

  if(count($result) > 1) return array('ambiguous other', compact(u\ray('who list')), array_keys($result));
  if(count($result) == 1) {
    foreach ($result as $uid => $one) { // just one
      $other = $one['full_name'];
      return (access('member', $uid)) ? $uid : array('non trader', compact('other'), array($uid));
    }
  }

//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (\valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown user', compact('who'), array());
  return create_user(array($key => $who));
}

/**
 * Return financial information about the given account
 * param array $args: associative array of non-default options:
 *  uid: uid for the account to return information for (DEFAULTS to cid)
 *  state: string list of transaction states to return (DEFAULT ':TX_DONE,:TX_DISPUTED')
 *  real: return real transactions instead of asif (DEFAULT real for rTraders, asif for non-rTraders)
 *  asof: last date to include (defaults to '', meaning unrestricted)
 *
 * @return
 *   standard object of credit account information (see $result_keys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function credit_info($args = array()) {
  if (is_frontend()) {
    $info = backend('credit_info', $args);
    $info->fancy = (array) $info->fancy; // backend turns the whole shebang into an object
  }
  extract($args);

  if (!@$uid) $uid = u\cid();
  if (!@$state) $state = ':TX_DONE,:TX_DISPUTED';
  if (!@$real) $real = r\is_rTrader($uid);
  if (!@$asof) $asof = '';
  
  $table = r\t_table($real);
  $result_keys = 'gross_in gross_out gross_in_proper balance available unavailable rewards cashable total_demand min_balance qid';
  $sql = "SELECT min_balance FROM users WHERE uid=:uid";
  extract(r\db_q($sql, compact('uid'))->fetchAssoc()); // gets $min_balance
  $qid = r\qid($uid);

  $sql = <<<EOF
    SELECT 
    SUM(IF(t.payer=:uid, t.amount, 0)) AS gross_out, 
    SUM(IF(t.payee=:uid, t.amount, 0)) AS gross_in,
    SUM(IF(t.type=:TX_TRANSFER, 0, t.amount)) AS rewards
    FROM $table t
    WHERE (t.payer=:uid OR t.payee=:uid) AND t.state IN($state)
EOF;
  if ($asof) $sql .= " AND t.created<=:asof";

  extract(r\db_q($sql, compact(u\ray('uid asof')))->fetchAssoc());
  $gross_in_proper = $gross_in - $rewards;
  $balance = $gross_in - $gross_out;
  $unavailable = 0; // for now
  $available = ($balance - $min_balance) - $unavailable;
  $cashable = $balance - $rewards;
  $total_demand = r\total_demand();
  $result = compact(u\ray($result_keys));
  $fancy = array();
  foreach ($result as $key => $value) $fancy[$key] = is_numeric($value ?: 0) ? u\format_amount($value) : $value;
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * Create a unique username for the given account, based on the given name.
 */
 /*
function make_username($account, $name) {
die('UNUSED');
  if (is_frontend()) return backend('make_username', compact(u\ray('account name')));

  $original = $name;
  for ($locktry = 0; ($locktry < 99) and !\lock_acquire(__FUNCTION__); $locktry++) \lock_wait();

  for ($try = 1; $try < 99; $try++) {
    if (!db_lookup(1, 'users', "name=:name", compact('name'))) {
      \user_save($account, compact('name'));
      \lock_release(__FUNCTION__);
      return $name;
    }
    deb("Account with username $name already exists, retrying...");
    $name = $original . $try;
  }
  \lock_release(__FUNCTION__);
  die("Too many collisions in make_make_shortname) -- original = $original");
}
*/

/**
 * Find and merge into the current account any temporary accounts with the same shortname or specified criteria
 * @return int: the number of accounts that were merged.
 */
function merge_check($info) {
  if (is_frontend()) return backend('merge_check', $info);
  
  $info['short_name'] = user_field('short_name');
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $where = 'short_name=:short_name';
  if (@$my_number) $where .= ' OR sms.number=:number';
  if (@$my_website) $where .= ' OR u.website=:website';
  if (@$my_phone) $where .= ' OR u.phone=:phone';
  if (@$my_fax) $where .= ' OR u.fax=:fax';
  $sql = "SELECT DISTINCT u.uid FROM users u LEFT JOIN sms_user sms ON sms.uid=u.uid WHERE (NOT u.pass) AND ($where)";
  $result = r\db_q($sql, $info);
  $found = 0;
  while ($row = $result->fetchAssoc()) $found += r\merge_accounts($row['uid']);
  return $found;
}

/**
 * Create an appropriate credit id for the given account
 * @return the credit id (FALSE if failure)
 */
/*
function make_credit_id($uid) {
  if (is_frontend()) return backend('make_credit_id', compact(u\ray('uid')));
  
  $account = \user_load($uid);
  if (@$account->credit_id) return $account->credit_id;
  if ($state = $account->state) { // true for web interface, not for SMS
    $country = $account->country;
    if ($country == 'United States') $state = u\state_abbrev($state);
  } else { // SMS, not web
    $phone = u\format_phone($account->phone, 'n');
    $area = substr($phone, 0, 3);
    $state = r\db_lookup('region', 'r_area_codes', 'area_code=:area', compact('area'));
    $country = 'United States';
  }
  if ($country == 'United States') {
    $region = r\db_lookup('credit_region', 'r_credit_regions', 'region=:state', compact('state'));
  } else $region = u\country_abbrev($country);
  
  if (!@$region) $region = 'Z9';
  $ctty = $region . 'AA'; // until the person becomes a member of a specific community, give them a generic community in their region

  for ($try = 0; $try < 99; $try++) {
    $credit_id = r\next_credit_id($ctty);
    \user_save($account, compact('credit_id'));
    if (!r\db_lookup('uid', 'users', 'credit_id=:credit_id AND uid<:uid', compact('credit_id', 'uid'))) return $credit_id;
  }
  die("Too many collisions in make_credit_id() -- ctty = $ctty");
}
*/

/**
 * Return a list of account choices for the current user (NOT the current credit account)
 * @return an array of account names, indexed by uid (FALSE if none)
 * @todo: recurse, to include access to the accounts that the shared accounts have access to
 *   maybe like this: SELECT shared_uid, MAX(permission) WHERE shared_with=:userid 
 *     OR shared_with IN (SELECT shared_uid FROM r_shared_accounts WHERE shared_with=:userid) GROUP BY shared_uid
 */
function account_choices() {
  if (is_frontend()) return (array) backend('account_choices');
  
  global $userid;
  if (!$userid) return FALSE;
  $primary = array((string) $userid => (count($GLOBALS['share permissions']) - 1)); // max access for primary user account

  $result = r\db_q("SELECT shared_uid, permission FROM r_shared_accounts WHERE shared_with=:userid", compact('userid'));
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
  if (is_frontend()) return (array) backend('todo', compact(u\ray('number todo nonce')));
  if (is_array($number)) extract($number);
  
  if ($todo == 'nothing') {
    return (object) r\db_q("SELECT nonce, todo FROM sms_user WHERE number=:number", compact('number'))->fetchAssoc();
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('sms_user')->fields(compact('nonce', 'todo'))->condition('number', $number)->execute();
    return $nonce;
  }
}

/**
 * Return the last transaction meeting the given criteria, for the current user.
 * @param string $where: the criteria (defaults to 1 = any)
 * @param array $subs: optional substitutions (will be prefixed with colons)
 */
function last_tx($where = 1, $subs = array(), $cache = FALSE) {
  global $cid;
  if (is_frontend()) return backend('last_tx', compact(u\ray('where subs')), $cache);
  if (is_array($where)) extract($where);

  $table = r\t_table();
  $sql = "SELECT * FROM $table WHERE $cid IN (payer, payee) AND ($where) ORDER BY xid DESC LIMIT 1"; // make sure it's for $cid
  $result = r\db_q($sql, $subs)->fetchAssoc();
  if ($result and $table == 'r_asif') $result['xid'] *= -1; // flag unrealness
  return $result;
}

/**
 * Return the user's sequential transaction number
 * @param int $xid: the transaction record number (negative for asif)
 * @param int $uid: the user's uid (DEFAULTs to cid)
 * @return string: the user's transaction number, preceded by "a" for asif transactions
 */
function xid2tid($xid, $uid = '') {
  if (!$uid) $uid = u\cid();
  if (is_frontend()) return backend('xid2tid', compact(u\ray('xid uid')));
  if (is_array($xid)) extract($xid);

  return ($xid < 0 ? 'a' : '') . r\db_lookup('COUNT(xid)', 'r_txs', "(payer=:uid OR payee=:uid) AND xid<=:xid AND type<=" . TX_COUNTABLE, compact('uid', 'xid'));
}

function account_name($uid) {
  if (is_frontend()) return backend('account_name', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid);
  
  extract((array) \user_load($uid), EXTR_PREFIX_ALL, 'my');
  $qid = r\qid($uid);
  return ": $my_full_name ($qid) $my_mail";
}

/**
 * Return all the permissions for the specified account
 * This function gets called before anything important happens, so it calls cid() to set up.
 * @param numeric $uid: uid of the account to ask about (defaults to '', meaning the current account)
 * @return simple array of that account's permissions
 */
function accesses($uid = '') {  
  global $userid, $cid;
  if (!$cid) return array(); // anonymous user has no access

  if (is_frontend()) {
    $result = backend('accesses', compact(u\ray('uid')), $uid);
    return is_array($result) ? $result : array();
  }
  if (is_array($uid)) extract($uid);
  if (!$uid) $uid = $cid;
  if (!$uid) return array(); // make it easy. no permissions for anonymous user.
  
  $role_perms = (\user_role_permissions(\user_load($uid)->roles)); // Drupal permissions
  $perms = array();
  $perms[] = array_values(\user_load($uid)->roles); // include things like administrator and rTrader
  foreach ($role_perms as $key => $value) $perms += array_keys($role_perms[$key]);

  if (\user_load($uid)->account_type != R_PERSONAL) $perms[] = 'company'; // ad hoc permissions

  $cans = $GLOBALS['share permissions'];
  $crit = 'shared_with=:userid AND shared_uid=:uid';
  $can = $uid == $userid ? count($cans) - 1 : r\db_lookup('permission', 'r_shared_accounts', $crit, compact('uid', 'userid'));
  for ($i = 1; $i <= $can; $i++) $perms[] = $cans[$i];

  return $perms;
}

/**
 * @return array of record objects
 */
function get_records($table, $criteria, $subs, $fields = '*', $cache = FALSE) {
/*  $sql = <<<EOF
    SELECT COLUMN_NAME FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA=:db AND TABLE_NAME=:table AND COLUMN_KEY='PRI'
EOF;
  $db = 'devcore'; // @todo: get proper db name
  $key = r\db_q($sql, compact('table', 'db')); */
//  if (!$cache) die('no cache get_records');
  if (is_frontend()) return (array) backend('get_records', compact(u\ray('table criteria subs fields')), $cache);
  if (is_array($table)) extract($table);
  
  return r\db_q("SELECT $fields FROM $table WHERE $criteria", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

function is_temp_account($uid) {
  if (is_frontend()) return backend('is_temp_account', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid);
  return !user_field('pass', $uid);
}

function is_temp_name($name) {
  if (is_frontend()) return backend('is_temp_name', compact(u\ray('name')));
  if (is_array($name)) extract($name);
  return u\abbreviates(R_TEMP_NAME_PREFIX, $name);
}

function testbe($function = '', $arg1 = '', $arg2 = '', $arg3 = '') {
  global $testing_backend, $firewall;
  $firewall = R_FIREWALL; // this is not currently necessary
  if (!$function) return $testing_backend;
  $function = "rCredits\\API\\$function";
  $testing_backend = TRUE;
  debug($function($arg1, $arg2, $arg3));
}

/**
 * Determine whether we are in the web browser frontend or not.
 * @todo: also check the rSMS path (another backend)
 */
function is_frontend() {
  if (!R_SEPARATE_BACKEND) return FALSE; // disable separation of frontend and backend
  return (!testbe() and (strpos($_SERVER['REQUEST_URI'], 'rCredits/api') === FALSE));
  // (FAILS in menu system of backend) global $is_backend; return !@$is_backend;
}

function backend($op, $info = array(), $cache = FALSE) {
  global $base_url, $userid, $cid, $firewall;
  if (!is_frontend()) die('backend recursion');
  if (!user_is_logged_in()) return FALSE; // probably checking permission
//  if ($op == 'get_records' and !$cache) die(print_r(debug_backtrace(), 1));

  if (!@$cid) die('no cid in backend!');
  if (empty($info)) $cache = TRUE;
//  $cache = FALSE; // debug
//  debug("cache=$cache cachekey=rcredits_$op$cid:" . serialize($info));
  $skey = "rcredits_{$op}_{$userid}_{$cid}_" . base64_encode(serialize($info)); // htmlspecialchars prevents session key failure (PHP bug?)
  if ($cache and (($result = @$_SESSION[$skey]) !== NULL)) return $result;
//  debug($skey . $result);
//  debug("cache:$cache $op: " . json_encode($info) . ($cache ? " result:$result" : ''));
//  debug($result);
  $password = \user_load($userid)->pass;
  $firewall = R_FIREWALL;
  foreach ($info as $key => $value) if (strpos($key, 'uid') !== FALSE) $info[$key] = r\qid($value); // convert to qid
  $basic = array('userid' => r\qid($userid), 'cid' => r\qid($cid));
  $info += compact(u\ray('op password firewall') + $basic); // operation, credentials, and account
  $host = strpos($base_url, 'localhost/') !== FALSE ? 'http://localhost/devcore' : 'http://devcore.rcredits.org';
  $api_result = R_SEPARATE_BACKEND ? u\http_request("$host/rCredits/api/", $info) : api($info);
//debug($api_result);
  u\log('rAPI', 'out', $info + compact('api_result'));
  if ($cache) $_SESSION[$skey] = $api_result;
//  if ($cache) debug($_SESSION["rcredits_$op$cid:" . $info]);
  return $api_result;
}

// Shortcut functions

/**
 * @param mixed $who: criteria or uid (defaults to $cid)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 */
function user_field($field_name, $who = FALSE, $subs = array()) {
  if (!$who) $who = u\cid();
  $result = get_records('users', is_numeric($who) ? "uid=$who" : $who, $subs, $field_name, TRUE);
  if (count($result) < 1) return FALSE;
  if (count($result) > 1) die("ambiguous user in user_field: $who");
  if (!is_object($result[0])) die('bad object array in user_field: ' . print_r($result, 1));
  return strpos($field_name, ',') ? ((array) $result[0]) : $result[0]->$field_name;
//  return r\db_lookup($field_name, 'users', is_numeric($who) ? "uid=$who" : $who, $subs);
}

/**
 * Say whether the account has the specified permission
 * @param string $access: the queried permission. This could be any of three types:
 *   a standard Drupal permission, queryable through user_access()
 *   a shared account permission, stored in r_shared_accounts (cached in $_SESSION['rcredits_account_choices'], but we're not trusting that)
 *   an ad hoc permission handled directly by this function
 * @param numeric $uid: uid of the account to ask about (defaults to '', meaning the current account)
 * @return boolean: TRUE if the user has the permission
 */
function access($access, $uid = '') {return in_array($access, accesses($uid));}
