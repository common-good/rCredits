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
 *   old: undo_tx($xid, $cid, $channel);
 *   new: backend('undo_tx', $xid, $cid, $channel);
 * backend() simply posts JSON data to the Back End and receives a JSON response, which it decodes and returns.
 * The mobile app will have a similar backend function.
 * rSMS is already a purely backend module
 *
 * In the Back End, we will receive a POST to the /api page, which will run external_request().
 * The external_request() function will decode the JSON request, then will call the appropriate function.
 * For example:
 * {"op":"undo_tx","xid":123,"credit_id":"NEAAAAAB","channel":"web"}
 * becomes r\undo_tx(123, 'NEAAAAAB', 'web');
 * which then echos a JSON reply by calling r\frontend($info);
 * The Back End will accept web requests only from the right IP.
 * Eventually the Back End will listen only on a certain port (not 80).
 *
 * No Drupal login is required in Back End. (just pass credit_id and user_id,password)
 */

namespace rCredits\API; // typically abbreviated as "api"
use \rCredits as r;
use \rCredits\Utilities as u;

define('RCREDITS_FIREWALL', 'lkj234lisufsd89072osi%!@J@$K'); // nobody can use the backend without knowing this code

function api2($result) {die(json_encode($result));}
function api_error($error) {api2(compact('error'));}

function api($args = FALSE) {
  global $cid, $userid, $firewall;

  extract($args ?: $_POST);
  if ($firewall != RCREDITS_FIREWALL) api_error('firewall system failure'); // give hackers some satisfaction

  $account = \user_load(@$userid);
  u\log('rAPI', 'in', $_POST);
  if (@!$password) api_error('login error');
  if (@$account->pass != $password) api_error('bad login'); // make these two error messages distinct but obscure

  switch ($op) {
    case 'create_user': api2(create_user($info));
    case 'undo_tx': api2(undo_tx($xid, $cid, $channel));
    case 'get': api2(get($currency, $amount, $uid, $confirmed, $channel));
    case 'pay_invoice': api2(pay_invoice($xid, $channel));
    case 'transfer': api2(transfer($ttype, $channel, $other_uid, $requested_amount, $for, $confirmed, $rewards));
    case 'give_signup_incentive': api2(give_signup_incentive($who, $channel));

    case 'add_cell': api2(add_cell($number, $validate));
    case 'delete_cell': api2(delete_cell($number));
    
    case 'identify': api2(identify($who, $cid, $create, $self_message));
    case 'credit_info': api2(credit_info($who, $state, $asof));
    case 'make_username': api2(make_username($account, $name));
    case 'merge_check': api2(merge_check($info));
    case 'merge_accounts': api2(merge_accounts($old_uid, $new_uid));
    case 'make_credit_id': api2(make_credit_id($uid));
    case 'account_choices': api2(account_choices());
    case 'todo': api2(todo($number, $todo, $nonce));
    case 'last_tx': api2(last_tx($where, $subs = array()));
    case 'tid_from_xid': api2(tid_from_xid($who, $xid));
    case 'account_name': api2(account_name($uid));
    case 'accesses': api2(accesses($uid));
    case 'get_records': api2(get_records($table, $criteria, $subs));
    case 'user_field': api2(user_field($field_name, $who, $subs));
    case 'is_temp_account': api2(is_temp_account($uid));
    case 'is_temp_name': api2(is_temp_name($name));
  }

  api2('unknown op');
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
  if (is_frontend()) return backend('create_user', compact('info'));
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  if (isset($my_number) and !isset($my_phone)) $my_phone = $my_number;
  if (isset($my_phone)) $info['phone'] = u\format_phone($my_phone);
  
  $info['mail'] = $info['init'] = @$my_email ?: '';
// Don't create a password (that's how we know it's a temporary account) $info['pass'] = \user_password();
  $info['name'] =  @$my_name ?: r\temp_name($info);
  $info['status'] = TRUE;
  $info['is_new'] = TRUE;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $account = \user_save(array(), $info);
  make_credit_id($account->uid); // this is done separately, to assure uniqueness
  if (isset($my_number)) r\SMS\create($my_number, $account->uid, SMS_USER_CONFIRMED);
  unset($dbtx);
  //*** End DBTX

  if (!$account) throw new \Exception('failed to save user record');
  return $account->uid;
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
 *     Someone charged me unilaterally: change the transaction state to CONTESTED (?)
 * @param int $channel: through what medium was the transaction requested (web, SMS, etc.).
 */
function undo_tx($xid, $cid, $channel) {
  if (is_frontend()) return (array) backend('undo_tx', compact(u\ray('xid cid channel')));
  
  extract(r\db_q('SELECT state, creator, amount, payer, payee, data FROM r_txs WHERE xid=:xid', compact('xid'))->fetchAssoc());
  $old_tid = tid_from_xid($cid, $xid);
  $tofrom = $payer == $cid ? t('to') : t('from');
  $other_uid = $payer == $cid ? $payee : $payer;
  $other = user_field('full_name', $other_uid);
  $old_data = $data = unserialize($data);
  if (@$data['undone']) return array('already undone', compact('old_tid'));
  
  if ($state == TX_PENDING or $state == TX_DENIED) {
//    $new_state = ($amount < 0 xor $creator == $cid) ? TX_DELETED : TX_DENIED;
    $new_state = $creator == $cid ? TX_DELETED : TX_DENIED;
    r\set_tx_state($new_state, $xid);
    $action = $creator == $cid ? t('deleted') : t('marked "denied"');
//  } elseif ($creator != $cid and ($amount < 0 xor $payee == $cid) and access('charge others', $creator)) { // someone charged me unilaterally
  } elseif ($creator != $cid and $payee == $cid and access('charge others', $creator)) { // someone charged me unilaterally
    r\set_tx_state(TX_CONTESTED, $xid);
    $action = t('marked "contested"');
  } else {
    $type = $cid == $payee ? 'payment' : 'charge';
    $my_note = "reverses #$old_tid";
    $other_note = str_replace($old_tid, tid_from_xid($other_uid, $xid), $my_note);
    $what = u\order($type == 'payment', $my_note, $other_note);
    list ($data['rebate'], $data['bonus']) = array(-$data['bonus'], -$data['rebate']);
    list ($message, $args, $confirm) = transfer($type, $channel, $other_uid, $amount, $what, TRUE, $data);
    if (@$args['success']) {
      $old_data['undone'] = TRUE;
      $old_data = serialize($old_data);
      list ($new_tid, $other_tid) = array($args['tid'], tid_from_xid($other_uid, $args['xid']));
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
 * @param int $channel: through what medium was the transaction requested (web, SMS, etc.).
 */
function get($currency, &$amount, $uid, $confirmed, $channel) {    
  if (is_frontend()) return (array) backend('get', compact(u\ray('currency amount uid confirmed channel')));
  
  if (!access('can manage account')) return array('no access get', array(), FALSE);
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return array('bad amount', array(), FALSE);
  if ($amount < 0) return array('negative transfer', array(), FALSE);
  if ($currency == 'r') $amount = min(RCREDITS_MAX_DEMAND, $amount);
  $request = u\format_amount($amount);
  $success = TRUE;
  if ($currency == 'r') {
// (catch this only when demand is high)      if ($my_amount < RCREDITS_MIN_DEMAND) return array('demand too small');
    r\demand($uid, $amount);
    return array('your demand', compact(u\ray('request success')), FALSE);
  }

  // $currency == 'usd'
  extract((array) credit_info($uid), EXTR_PREFIX_ALL, 'info');
  if ($amount > $info_cashable) return array("can't cash incentives", $info_fancy, FALSE);
//  $type = 'payment';
  $type = TX_TRANSFER;
  $payer = $creator = $uid;
  $payee = variable_get('rcredits_community_uid');
  $for = t('cash');

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\transact(compact(u\ray('type amount payer payee for creator channel')));
  $balance = u\format_amount($info_balance - $amount);
  if ($confirmed) return array('report get usd', compact(u\ray('request balance success')), FALSE);
  
  $dbtx->rollback();
  return array('confirm get usd', compact(u\ray('request success')), TRUE);
  //*** End DBTX
}

/**
 * Pay the given invoice
 * @param int $channel: through what medium was the transaction requested (web, SMS, etc.).
 * @return array(string $message, array $args)
 */
function pay_invoice($xid, $channel) {
  if (is_frontend()) return (array) backend('pay_invoice', compact(u\ray('xid channel')));
  
  if (!access('can create transactions')) return array('no access create', array());
  $state = TX_PENDING;
  $sql = "SELECT * FROM r_txs WHERE xid=:xid AND state=:state LIMIT 1";
  $result = r\db_q($sql, compact('xid', 'state'))->fetchAssoc();
  if (!$result) die('bad call to ' . __FUNCTION__);
  extract($result, EXTR_PREFIX_ALL, 'old');

  //*** Begin DBTX
  $dbtx = \db_transaction();
  list ($message, $args, $confirm) = transfer('invoice payment', $channel, $old_payee, $old_amount, $old_payee_for, TRUE);
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
 * @param int $channel: through what medium was the transaction requested (web, SMS, etc.).
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
 * $confirmed (user confirmed the transaction request), $xid (approving an invoice), $for_cash (the transaction is an exchange for USD),
 * $shortfall (how inadequate the payer's balance is)
 * @todo: record cash flag in data (pass it explicitly to transact?). it comes back here in $rewards, for undo.
 * @todo: giving wrong message on successful undo
 */
function transfer($ttype, $channel, $other_uid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  if (is_frontend()) return (array) backend('transfer', compact(u\ray('ttype channel other_uid requested_amount for confirmed rewards')));
  
  global $cid;
  if (!access('can create transactions', $cid)) return array('no access create', array(), FALSE);
  $taking = ($ttype == 'charge');
//  if ($paying_invoice = ($ttype == 'invoice payment')) $ttype = 'payment';
  $paying_invoice = ($ttype == 'invoice payment');
  $confirm_type = $ttype;
  $type = TX_TRANSFER;
//  $preapproved = ($taking xor $requested_amount < 0) ? access('charge others', $cid) : TRUE;
  $preapproved = $taking ? access('charge others', $cid) : TRUE;
//  if ($requested_amount < 0 and empty($rewards)) return array('negative transfer', array(), FALSE);
  if ($requested_amount < 0) return array('negative transfer', array(), FALSE);
  $for_cash = @$rewards['rebate'] ? ($rewards['rebate'] + $rewards['bonus'] == 0) : u\for_cash($for); // @$rewards['cash'] ?:
  if ($for_cash and !is_array($for)) $for = 'cash';
  
  $available = credit_info($taking ? $other_uid : $cid)->available;
  $amount = min($requested_amount, $available);
  $shortfall = max(0, $requested_amount - $amount); // how much needs to be done in a separate currency
  if ($shortfall and $preapproved) {
    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
    $short = u\format_amount($shortfall);
    $other = user_field('full_name', $other_uid);
    if ($paying_invoice) return array('short invoice', compact('short', 'other'), FALSE); // no partial payment of invoice
    if ($for_cash) return array($taking ? 'short cash from' : 'short cash to', compact('short', 'other'), FALSE); // no partial cash trans
    if (ceil($shortfall) >= $requested_amount) return array($taking ? 'broke from' : 'broke to', compact('other'), FALSE);
    $shortfall = ceil($shortfall); // for goods and services, shortfall should be a round figure (no cents)
    $amount = $requested_amount - $shortfall;
  }
  
  list ($payer, $payee) = $taking ? array($other_uid, $cid) : array($cid, $other_uid);
  $rebate = $for_cash ? '0' : ('' . r\rebate($payer, $amount)); // force string, to avoid long serializations
  $bonus = $for_cash ? '0' : ('' . r\bonus($payee, $amount));
//  list ($rebate, $bonus) = $for_cash ? array('0', '0') : array('' . ($amount * rebate($payer)), '' . ($amount * bonus($payee)));
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;
  $state = $preapproved ? TX_DONE : TX_PENDING;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $txs = r\transact(compact(u\ray('type amount payer payee for data state channel')));
  $xid = $txs[0]['xid'];
  if (!$confirmed) $dbtx->rollback();

  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  $reward_index = $taking ? 2 : 1;
  $reward = @$txs[$reward_index]['amount'] ?: 0;

  $args = r\tx_report_args($taking, $amount, $shortfall, $reward, $cid, $other_uid, $xid, $for_cash);
  if ($for_cash) {
    $message = $confirmed ? (($taking and !$preapproved) ? 'report exchange request' : 'report exchange') : $args['way'];
    return array($message, $args, !$confirmed);
  }
  
  if ($confirmed) {
    $message = $shortfall ? 'report short transaction' : (($taking and !$preapproved) ? 'report invoice' : 'report transaction');
    return array($message, $args, FALSE);
  }
  if ($shortfall) $args['amount'] = u\format_amount($requested_amount); // restore original amount before getting confirmation
  return array($confirm_type, $args, TRUE);
}

/**
 * Record an incentive transaction for signing up
 * @param int who: uid of the person signing up
 * @param int $channel: through what medium was the transaction requested (web, SMS, etc.). Negative for ASIF transactions.
 * @return int: the incentive amount
 */
function give_signup_incentive($who, $channel) {
  if (is_frontend()) return backend('give_signup_incentive', compact(u\ray('who channel')));

  $type = TX_SIGNUP;
  $amount = RCREDITS_STARTING_BALANCE;
  $payer = $creator = \variable_get('rcredits_community_uid');;
  $payee = $who;
  $for = t('signup');
  r\transact(compact(u\ray('type amount payer payee for creator channel')));
  return $amount;
}

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function add_cell($number, $validate = FALSE) {
  if (is_frontend()) return backend('add_cell', compact(u\ray('number validate')));

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
      extract(user_field('full_name,credit_id', $old_uid));
      $account = "$full_name ($credit_id)"; // maybe make the previous line and this a function
      if (!is_temp_account($old_uid)) return array('cell taken', compact(u\ray('number account')));
    }
    return NULL;
  }
  
  $uid = $cid;
  $status = SMS_USER_CONFIRMED;
  $number = $number_internal;
  debug(!merge_check(compact('number')));
  if (!merge_check(compact('number'))) \drupal_write_record('sms_user', compact(u\ray('uid number status')));
  $number = $number_pretty;
  return array('report new cell', compact('number'));
}

function delete_cell($number) {
  if (is_frontend()) return backend('delete_cell', compact(u\ray('number')));
  $number = u\format_phone($number, '+n'); // internal format
  r\db_q('DELETE FROM sms_user WHERE number=:number LIMIT 1', compact('number'));
  $number = u\format_phone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant.
 *
 * @param string $who: how the user referred to the trader
 * @param numeric $cid
 * @param boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
 *   If true, the other trader will be created only if specified by email or phone
 * @param string $self_message: index to error message for identifying oneself
 * @return one of the following:
 *   the other trader's uid
 *   an array whose elemnts are: an error index, an array of substitutions, an array of possible uids (0 or more)
 */
function identify($who, $cid, $create = FALSE, $self_message = 'no self-trading') {
  if (is_frontend()) {
    $result = backend('identify', compact(u\ray('who cid create self_message')));
    return is_object($result) ? ((array) $result) : $result;
  }
  
  if ($phone = u\format_phone($who, '+n')) $who = $phone;
//  if (substr($who, 0, 7) == 'http://') $who = substr($who, 7);
//  if (substr($who, 0, 4) == 'www.') $who = substr($who, 4);

  if (($short = u\short_name($who)) === '') $short = '-'; // abbreviations must not be null
  
  $sql = "SELECT DISTINCT u.uid FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
          WHERE :who IN (CONCAT('.', u.short_name), u.credit_id, s.number, u.phone, u.mail) 
          OR (MID(:who, 1, 1)='.' AND INSTR(u.short_name, MID(:who, 2)))
          OR INSTR(u.short_name, :short)";
  $result = r\db_q($sql, compact(u\ray('who short')))->fetchCol();
  if(count($result) > 1) {
    $list = '';
    foreach ($result as $one) {
      extract(r\db_q("SELECT credit_id, full_name FROM users WHERE uid=:one", compact('one'))->fetchAssoc(), EXTR_PREFIX_ALL, 'my');
      $list .= " &nbsp; &nbsp; &nbsp; &nbsp; $my_credit_id &nbsp;$my_full_name<br>";
    }
    return array('ambiguous other', compact(u\ray('who list')), $result);
  }
  if (isset($result[0])) {
    $other_uid = $result[0];
    if ($other_uid == $cid) return array($self_message, array(), $result);
    $other_account = \user_load($other_uid);
    $other = $other_account->full_name;
    if (!access('member', $other_uid)) return array('non trader', compact('other'), $result);

    return $other_uid;
  }
  
//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (\valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown user', compact('who'), $result);
  return create_user(array($key => $who));
}

/**
 * Return information about the given account
 * @param int $who: uid of the credit account to return information for
 * @param boolean $state: TX_DONE or TX_CONTESTED (default) or TX_PENDING (return info about what kind of transactions)
 * @param numeric $asof: last date to include (defaults to NULL, meaning unrestricted)
 *
 * @return
 *   standard object of credit account information (see $result_keys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function credit_info($who, $state = TX_DONE, $asof = '') {
  if (is_frontend()) {
    $info = backend('credit_info', compact(u\ray('who state asof')));
    $info->fancy = (array) $info->fancy; // backend turns the whole shebang into an object
  }
  
  if ($state == TX_DONE) $state = TX_DONE . ',' . TX_CONTESTED;
  $result_keys = 'gross_in gross_out gross_in_proper balance available unavailable rewards cashable total_demand min_balance id';
  $sql = "SELECT credit_id AS id, min_balance FROM users WHERE uid=:who";
  extract(r\db_q($sql, compact('who'))->fetchAssoc()); // gets $id, $min_balance
//  $types = tx_types("'payment', 'charge'");
  $transfer = TX_TRANSFER;
  $sql = "SELECT 
    SUM(IF(t.payer=:who, t.amount, 0)) AS gross_out, 
    SUM(IF(t.payee=:who, t.amount, 0)) AS gross_in,
    SUM(IF(t.type=:transfer, 0, t.amount)) AS rewards
    FROM r_txs t
    WHERE (t.payer=:who OR t.payee=:who) AND t.state IN(:state)";
  if ($asof) $sql .= " AND t.created<=:asof";

  extract(r\db_q($sql, compact(u\ray('who transfer state asof')))->fetchAssoc());
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

/**
 * Find and merge into the current account any temporary accounts with the same shortname or specified criteria
 * @return int: the number of accounts that were merged.
 */
function merge_check($info) {
  if (is_frontend()) return backend('merge_check', compact(u\ray('info')));
  
  global $cid;
  if (!@$short_name) $info['short_name'] = user_field('short_name', $cid);
  extract($info);
  
  $where = 'short_name=:short_name';
  if (@$number) $where .= ' OR sms.number=:number';
  if (@$website) $where .= ' OR u.website=:website';
  if (@$phone) $where .= ' OR u.phone=:phone';
  if (@$fax) $where .= ' OR u.fax=:fax';
  $sql = "SELECT DISTINCT u.uid FROM users u LEFT JOIN sms_user sms ON sms.uid=u.uid WHERE (NOT u.pass) AND ($where)";
  $result = r\db_q($sql, $info);
  $found = 0;
  while ($row = $result->fetchAssoc()) $found += merge_accounts($row['uid'], $cid);
  return $found;
}

/**
 * Merge a temporary ("old") account into a permanent ("new") account.
 * @todo: handle resultant self-transactions
 * @todo: write old uids to a separate table (maybe just uid and credit_id)
 * @todo: handle shared accounts
 * @return 1 (for counting how many were merged)
 */
function merge_accounts($old_uid, $new_uid) {
  if (is_frontend()) return backend('merge_accounts', compact(u\ray('old_uid new_uid')));
  
  if (!is_temp_account($old_uid)) return; // don't merge non-temp account
  if (!access('can manage account')) die("unauthorized merge_accounts: $new_uid into $old_uid");
//  if (!is_temp_name(user_field('name', $old_uid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = \user_load($new_uid);
  $old = \user_load($old_uid);
  $update = array();
  foreach (u\ray('phone fax website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  
  /*** Begin DBTX ***/
  $dbtx = db_transaction();
  \user_save($new, $update);
  $subs = compact('new_uid', 'old_uid');
  r\db_q("UPDATE r_txs SET payer=:new_uid WHERE payer=:old_uid", $subs);
  r\db_q("UPDATE r_txs SET payee=:new_uid WHERE payee=:old_uid", $subs);
  r\db_q("UPDATE sms_user SET uid=:new_uid WHERE uid=:old_uid", $subs);
  r\db_q("UPDATE r_user_industries SET uid=:new_uid WHERE uid=:old_uid", $subs);
  r\db_q("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:old_uid", compact('old_uid'));
  /*** End DBTX ***/ unset($dbtx);
  return 1;
}

/**
 * Create an appropriate credit id for the given account
 * @return the credit id (FALSE if failure)
 */
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
  
  if ($todo == 'nothing') {
    return (object) r\db_q("SELECT nonce, todo FROM sms_user WHERE number=:number", compact('number'))->fetchAssoc();
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('sms_user')->fields(compact('nonce', 'todo'))->condition('number', $number)->execute();
    return $nonce;
  }
}

/**
 * Return the last transaction meeting the given criteria
 * @param string $where: the criteria (defaults to 1 = none)
 * @param array $subs: optional substitutions (will be prefixed with colons)
 */
function last_tx($where = 1, $subs = array(), $cache = FALSE) {
  if (is_frontend()) return backend('last_tx', compact(u\ray('where subs')), $cache);
  
  $sql = "SELECT * FROM r_txs WHERE $where ORDER BY xid DESC LIMIT 1"; 
  return r\db_q($sql, $subs)->fetchAssoc();
}

function tid_from_xid($who, $xid) {
  if (is_frontend()) return backend('tid_from_xid', compact(u\ray('who xid')));
  
  return r\db_lookup('COUNT(xid)', 'r_txs', "(payer=:who OR payee=:who) AND xid<=:xid AND type<=" . TX_COUNTABLE, compact('who', 'xid'));
}

function account_name($uid) {
  if (is_frontend()) return backend('account_name', compact(u\ray('uid')));
  
  extract((array) \user_load($uid), EXTR_PREFIX_ALL, 'my');
  return @$my_credit_id ? ": $my_full_name ($my_credit_id) $my_mail" : ": $my_mail";
}

/**
 * Return all the permissions for the specified account
 * @param numeric $uid: uid of the account to ask about (defaults to '', meaning the current account)
 * @return simple array of that account's permissions
 */
function accesses($uid = '') {  
  global $userid, $cid;
  if (!$uid) $uid = u\cid() + 0; // must precede backend caching
  if (is_frontend()) return backend('accesses', compact(u\ray('uid')), TRUE);
  
  $role_perms = (\user_role_permissions(\user_load($uid)->roles)); // Drupal permissions
  $perms = array();
  foreach ($role_perms as $key => $value) $perms += array_keys($role_perms[$key]);
  if (\user_load($uid)->account_type != RCREDITS_PERSONAL) $perms[] = 'company'; // ad hoc permissions

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
  if (is_frontend()) return (array) backend('get_records', compact(u\ray('table criteria subs')), $cache);
  
  return r\db_q("SELECT $fields FROM $table WHERE $criteria", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

function is_temp_account($uid) {return is_frontend() ? backend('is_temp_account', compact(u\ray('uid'))) : !user_field('pass', $uid);}
function is_temp_name($name) {return is_frontend() ? backend('is_temp_name', compact(u\ray('name'))) : u\abbreviates(RCREDITS_TEMP_NAME_PREFIX, $name);}

function testbe($function = '', $arg1 = '', $arg2 = '', $arg3 = '') {
  global $testing_backend, $firewall;
  $firewall = RCREDITS_FIREWALL; // this is not currently necessary
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
  if (!RCREDITS_SEPARATE_BACKEND) return FALSE; // disable separation of frontend and backend
  return (!testbe() and (strpos($_SERVER['REQUEST_URI'], 'rCredits/api') === FALSE));
  // (FAILS in menu system of backend) global $is_backend; return !@$is_backend;
}

function backend($op, $info = array(), $cache = FALSE) {
  global $base_url, $userid, $cid, $firewall;
  if (!is_frontend()) die('backend recursion');
  if (!user_is_logged_in()) return FALSE; // probably checking permission
//  if ($op == 'get_records' and !$cache) die(print_r(debug_backtrace(), 1));

  if (!@$cid) u\cid();
  if (empty($info)) $cache = TRUE;
//  $cache = FALSE; // debug
//  debug("cache=$cache cachekey=rcredits_$op$cid:" . serialize($info));
  $skey = "rcredits_{$op}_{$userid}_{$cid}_" . base64_encode(serialize($info)); // htmlspecialchars prevents session key failure (PHP bug?)
  if ($cache and (($result = @$_SESSION[$skey]) !== NULL)) return unserialize($result);
//  debug($skey . $result);
  debug("cache:$cache $op: " . json_encode($info) . ($cache ? " result:$result" : ''));
//  debug($result);
  $password = \user_load($userid)->pass;
  $firewall = RCREDITS_FIREWALL;
  $info += compact(u\ray('op userid password cid firewall')); // operation, credentials, and account
  $host = strpos($base_url, 'localhost/') !== FALSE ? 'http://localhost/devcore' : 'http://devcore.rcredits.org';
  $api_result = RCREDITS_SEPARATE_BACKEND ? u\http_request("$host/rCredits/api/", $info) : api($info);
//debug($api_result);
  u\log('rAPI', 'out', $info + compact('api_result'));
  if ($cache) $_SESSION[$skey] = serialize($api_result);
//  if ($cache) debug($_SESSION["rcredits_$op$cid:" . serialize($info)]);
  return $api_result;
}

// Shortcut functions

/**
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

