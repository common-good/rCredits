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
 *   old: undo_tx($zxid);
 *   new: backend('undo_tx', $zxid);
 * backend() simply posts JSON data to the Back End and receives a JSON response, which it decodes and returns.
 * The mobile app will have a similar backend function.
 * rSMS is already a purely backend module
 *
 * In the Back End, we will receive a POST to the /api page, which will run external_request().
 * The external_request() function will decode the JSON request, then will call the appropriate function.
 * For example:
 * {"op":"undo_tx","zxid":123}
 * becomes r\undo_tx(123, 'web');
 * which then echos a JSON reply by calling r\frontend($info);
 * The Back End will accept web requests only from the right IP.
 * Eventually the Back End will listen only on a certain port (not 80).
 *
 * No Drupal login is required in Back End. (just pass quid(cuid) and quid(user_id),password)
 * All uids are passed as qids (qid = r\quid(uid) = R_SERVER_ID . n2a(uid))
 */

namespace rCredits\API; // typically abbreviated as "api"
use \rCredits as r;
use \rCredits\Utilities as u;

function api2($op, $args, $success = 1) {
  $op = 'rCredits\\API\\' . $op;
  if (function_exists($op)) api_out(compact('success') + $op($args)); else api_error('unknown op'); // args can override success
}

function api_error($message, $success = 0) {api_out(compact('success', 'message'));}

function api_out($args) {
  u\log('out', $args);
  exit(json_encode($args));
}

/**
 * Make sure the current user ($auid) can at least read transactions for the specified account
 */
function be_check($uid) {
  global $auid;
  $crit = 'main_uid=:uid AND agent_uid=:auid';
  if (!($uid == $auid or r\db_lookup(1, 'r_relations', $crit, compact('uid', 'auid')))) api_error('sharing error');
}

/**
 * Receive and process a REST request from a smart device
 * @param array $args: an associative array that normally includes:
 *   op: what to do (determines what function to call)
 *   my_id: qid supplied by user once, in First Time request (used to set $cuid)
 *   code: permanent code the we sent to the device in First Time response (used to set $auid)
 *     if $op = 'first_time', expect $password instead
 *   account_id: the qid for the other party, if any 
 *     This gets passed to the specific op functions as $ruid = array($uid, $agent_uid)
 * @return (normally by calling either api_error or api2):
 *   $success=1 if api2 is called, otherwise 0 ($op = 'photo' is an exception; see below)
 *   $message: an error message or help message to display on the device; or not set
 */
function smart_api() {
  global $auid, $cuid, $channel;
  $channel = TX_SMART;
  u\log('in', $_POST);
  if (!(@$args = (array) json_decode($_POST['json']))) api_error('no json');
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (@$q_op != 'reset') // DEBUG
  if (@$q_op != 'first_time') {
    if (!@$q_code) api_error('no code given');
    if (!($cuid = r\db_lookup('uid', 'r_smarts', 'code=:q_code', compact('q_code')))) api_error('unknown device');
    be_check($auid = r\quid(@$q_my_id)); // make sure agent has permission
  }
  $op = 'smart_' . @$q_op;
  if (@$q_account_id) {
    $zuid = r\quid($q_account_id);
    if (!$zuid) api_error('bad account_id');
    list ($uid, $agent_uid) = $ruid = zuid($zuid);
    $args += compact(u\ray('ruid uid agent_uid'));
  }
  api2($op, $args);
}

function smart_reset($args) {
  r\db_q('DELETE FROM r_smarts WHERE 1');
  return array();
}

/*
 * Requesting a permanent code from the server.
 * @param array $args:
 *   password: the user's Drupal account password
 * @return:
 *   code: permanent code for app to prove the identity of the device
 *   owner_id: the device owner's account ID
 *   update_link: URL of updated app; not set if no update is available
 */
function smart_first_time($args) {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  extract($args, EXTR_PREFIX_ALL, 'q');
//  if (!($account = \user_load_by_name(@$q_my_id))) api_error('unknown user');
  if (!($account = \user_load(r\quid(@$q_my_id)))) api_error('unknown user');
  if (@!$q_password) api_error('login error');
  if (!user_check_password($q_password, $account)) api_error('bad login'); // make these two error messages distinct but obscure
  $code = user_hash_password(uniquid($q_my_id, TRUE)) . '/'; // add a slash to make sure slashes get handled properly
  $uid = $account->uid;
  $device = t('Smart Device');
  \drupal_write_record('r_smarts', compact(u\ray('uid code device'))); // no way to tell if it's the same device as others
  return compact(u\ray('code owner_id')) + smart_startup($args);
}

/*
 * Identify the scanned QR code.
 * @param array $args: (none)
 * @return:
 *   update_link (URL of updated app; not set if no update is available)
 */
function smart_startup($args) {
  if (rand() < getrandmax() / 2) $update_link = 'http://rCredits.org/files/smart.apk';
  $allow_change_account = 0;
  $allow_change_agent = TRUE;
  $require_agent = access('company');
  return compact('update_link allow_change_account allow_change_agent require_agent');
}

/*
 * Identify the other party.
 * @param array $args:
 *   account_id (converted to ruid): the qid for an individual or company and rep
 * @return:
 *   full_name: the individual's name
 *   company_name: the company name (if any)
 *   location: the location of the company if it's a company, otherwise of the individual (typically city, state)
 *   other_balance: the current balance of the company if it's a company, otherwise of the individual
 */
function smart_identify($args) {
  $cuid = r\cuid();
  extract($args, EXTR_PREFIX_ALL, 'q');
  $full_name = \user_load($q_uid)->full_name;
  if ($q_uid == $cuid) api_error(r\Web\tt('no self-trading'));
  $account = \user_load($q_uid); // the individual or company account
  $country = $account->country;
  $location = $account->city . ', ' . ($country == 'United States' ? u\state_abbrev($account->state) : $country);
  if (@$q_company_uid) $company_name = user_field('full_name', $q_company_uid);
  $uid = $q_uid;
  $other_balance = credit_info(compact('uid'))->balance;
  return compact(u\ray('full_name location company_name other_balance'));
}

/*
 * Get an individual's photo.
 * @param array $args:
 *   account_id (converted to ruid): the qid for an individual or company and rep
 * @return:
 *   just the photo data (no json)
 */
function smart_photo($args) {
  extract($args, EXTR_PREFIX_ALL, 'q');
  $pic_filename = r\Web\profile_picture(@$q_uid);
  $pic = file_get_contents($pic_filename);
  exit($pic);
}

/*
 * Request a transaction.
 * @param array $args:
 *   account_id (converted to uid or company_uid & rep_id): the qid for an individual or company and rep
 *   type: “charge” or “payment”
 *   amount: numeric dollar amount
 *   goods: true unless user checks “cash, loan, etc.”
 *   purpose: description of goods and services
 * @return:
 *   tx_id: transaction ID number, if success, otherwise empty string
 *   my_balance: device owner's new balance
 *   other_balance: the new balance of the company if it's a company, otherwise of the individual
 */
function smart_transact($args) {
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (!$q_goods) $q_purpose = 'cash'; // tell transfer no rewards
  list ($index, $subs, $zot) = transfer($q_type, $q_ruid, $q_amount, $q_purpose, TRUE);
  $tx_id = r\qxid($subs['zxid']); // return the transaction id as a qid (globally unique transaction id) (':' means As-If)
  list ($my_balance, $other_balance) = smart_balances($q_uid, $subs['real']);
  $message = r\Web\tt($index, $subs);
  //if (!@$subs['success'])
  return compact(u\ray('tx_id my_balance other_balance message'));
}

function smart_balances($uid, $real) {
  $my_balance = credit_info(compact('real'))->balance;
  if (access('charge others')) $other_balance = credit_info(compact('real', 'uid'))->balance;
  return array($my_balance, $other_balance);
}

/*
 * Undo the specified transaction (perhaps tentatively)
 * @param array $args:
 *   account_id: the qid for an individual or company and rep
 *   tx_id: qid of the transaction to undo
 *   confirmed: TRUE if user has already okayed this undo
 * @return (only if confirmed is TRUE):
 *   tx_id: qid of offsetting transaction, if any (which could in turn be undone). Undefined if failure. 
 *     Otherwise, tx_id not set means transaction was simply deleted, so there is no longer any transaction that can be undone.)
 *   my_balance: device owner's new balance
 *   other_balance: the new balance of the company if it's a company, otherwise of the individual
 */
function smart_undo($params) {
  extract($params, EXTR_PREFIX_ALL, 'q');
  $zxid = r\qxid($q_tx_id);
  if ($q_confirmed) {
    list ($message, $args) = undo_tx($zxid);
    $message = r\Web\tt($message, $args);
    if (!@$args['success']) api_error($message);
    if ($new_zxid = @$args['zxid']) $tx_id = r\qxid($new_zxid); // get offsetting transaction id, if any (otherwise state changed)
    list ($my_balance, $other_balance) = smart_balances($args['other_uid'], $zxid > 0);
  } else {
    if (!($result = last_tx('xid=:zxid', compact('zxid')))) api_error(r\Web\tt('undo no match'));
    $message = r\Web\tt('confirm undo', r\tx_args($result));
  }
  return compact(u\ray('message tx_id my_balance other_balance'));
}

/**
 * Receive and process a REST request from the website frontend
 */
function backend2($args = FALSE) {
  global $auid, $cuid, $firewall;

  if (!$args) $args = $_POST;
  extract($args); // $cuid, $auid, $firewall (those first three are all extracted into globals), $my_id, $op
  $cuid = r\quid(@$my_id ?: r\cuid()); unset($args['cuid']); // unset because it is passed as a global, not as an arg
  $auid = r\quid(@$auid); unset($args['auid']); // unset because it is passed as a global, not as an arg

  if (@$firewall != R_FIREWALL) api_error('firewall system failure'); // give hackers some satisfaction by saying 'failure'
  if (!$cuid or !$auid) api_error('user validation error');
  be_check($cuid);
  api2($op, $args);
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
  $short_name = @$my_full_name ? u\short_name($full_name) : '';
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
 * @return array: associative list of transactions (with negative zxids, if they are not real transactions)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function my_transactions($args) {
  $cuid = r\cuid();
  
  if (is_frontend()) return backend(__FUNCTION__, $args);
  extract($args);
  
  $table = r\zxs((bool) $real);
  $omit_bonuses = @$include_bonuses ? '' : 'WHERE t.type NOT IN (:TX_REBATE, :TX_BONUS)';
  
  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid = IF(t.payer = $cuid, t.payee, t.payer)) AS name,
      IF(t.payer = $cuid, t.payer_for, t.payee_for) AS purpose
    FROM $table t 
    $omit_bonuses
    AND $cuid IN (t.payer, t.payee) 
    AND (t.state IN(:TX_DONE, :TX_PENDING, :TX_DISPUTED) OR (t.state = :TX_DENIED AND t.payee = $cuid))
    AND t.created>=:starting AND t.created<=:ending
    ORDER BY t.xid DESC
EOF;
  
  $result = r\db_q($sql, compact(u\ray('starting ending')))->fetchAllAssoc('xid', \PDO::FETCH_ASSOC);
  
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
 */
function update_relations($updates) {
  $report = '';
  foreach ($updates as $reid => $data) {
    $full_name = $data['full_name'];
    unset($data['full_name']); // don't try to sum full_name
    $changes = '';
    $reid_sub = $subs = compact('reid');
//    $full_name = r\db_lookup('u.full_name', 'r_relations r INNER JOIN users u ON r.agent_uid=u.uid', $crit, $reid_sub);
    foreach ($data as $field => $value) {
      $changes .= ($changes ? ',' : '') . "$field=:$field";
      $subs += array($field => $value);
    }
    $crit = 'reid=:reid';
    r\db_q("UPDATE r_relations SET $changes WHERE $crit", $subs);

    if (!array_sum($data) and !r\db_lookup('permission+is_employee+is_owner', 'r_relations', $crit, $reid_sub)) {
      r\db_q("DELETE FROM r_relations WHERE $crit", $reid_sub);
      $report .= t('Deleted relation with full_name<br>', compact('full_name'));
    } else $report .= t('Updated settings for full_name<br>', compact('full_name'));
  }
  return $report;
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
 * @param int $zxid: the transaction number (negative for asif transactions)
 * @param boolean $real: is the transaction real, that is to be undone
 */
function undo_tx($zxid, $real = 'bug') {
  $cuid = r\cuid();
  if (is_frontend()) return (array) backend('undo_tx', compact(u\ray('zxid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend
  
  $old_record = last_tx('xid=:zxid', compact('zxid'));
  if (empty($old_record)) return array('no such tx', array());
  extract($old_record, EXTR_PREFIX_ALL, 'old');
  $old_tid = zxid2tid($zxid);
  $data = unserialize($old_data);
  if (@$data['undone']) return array('already undone', compact('old_tid'));
  $other_ruid = $old_payer == $cuid ? array($old_payee, $old_payee_agent) : array($old_payer, $old_payer_agent);
  $other_uid = $other_ruid[0];
  $to_me = ($old_payee == $cuid);  
//  $by_me = ($taking xor !$to_me);
  
  if ($old_state == TX_PENDING or $old_state == TX_DENIED) { // it's an invoice - cancel simply by changing the transaction state
// function undo_invoice($zxid, $old_record, $other_uid, $by_me) // return $action
    if ($to_me) {
      list ($new_state, $action) = array(TX_DELETED, t('deleted')); // abandon the attempt to invoice
    } else {
      list ($new_state, $action) = array(TX_DENIED, t('marked "denied"'));
      $payer_name = user_field('full_name'); // my name, for message to payee
      r\notify($other_uid, 'invoice-denied', $old_record + compact('payer_name'));
    }
    r\set_tx_state($new_state, $zxid);
  } elseif (!$to_me and $old_taking and access('charge others', $old_payee)) { // someone charged me unilaterally
// function dispute($zxid, $old_record, $other_uid)
    r\set_tx_state(TX_DISPUTED, $zxid); // mark it disputed
    $action = t('marked "disputed"');
    $payer_name = user_field('full_name'); // my name, for message to payee
    r\notify($other_uid, 'charge-disputed', $old_record + compact('payer_name'));
  } else return reverse($zxid, $old_amount, $other_ruid, $data, $old_tid, $to_me); // create an offsetting transaction

  $amount = u\format_amount($old_amount);
  $tofrom = !$to_me ? t('to') : t('from');
  $other = user_field('full_name', $other_uid);
  return array("report undo", compact(u\ray('old_tid new_tid amount other tofrom action other_uid')));
}

/**
 * Create an offsetting transaction.
 */
function reverse($zxid, $amount, $other_ruid, $data, $old_tid, $to_me) {
  global $auid;
  u\expect(compact(u\ray('zxid amount other_ruid data old_tid to_me')), 'zid float rid assoc id bool');
  $old_data = $data; // remember original data from transaction we're reversing
  $type = $to_me ? 'payment' : 'charge';
  $my_note = "reverses #$old_tid";
  $other_note = str_replace($old_tid, zxid2tid($zxid, $other_ruid[0]), $my_note);
  $what = u\order($type == 'payment', $my_note, $other_note);
  list ($data['rebate'], $data['bonus']) = array(-$data['bonus'], -$data['rebate']);
  $data['old_zxid'] = $zxid; // remember what we're reversing (especially to get the realness right)
  list ($message, $args, $confirm) = transfer($type, $other_ruid, $amount, $what, TRUE, $data);

  if (@$args['success']) {
    list ($new_tid, $new_zxid) = array($args['tid'], $args['zxid']);
    u\expect($new_zxid * $zxid > 0, 'reverse: wrong reality');
    $other_tid = zxid2tid($new_zxid, $other_uid);
    $old_data['undone'] = TRUE;
    $old_data['changes'][] = array(time(), $auid, 'undo', $new_zxid);
    $old_data = serialize($old_data);
    $my_note = "(reversed by #$new_tid) ";
    $other_note = str_replace($new_tid, $other_tid, $my_note);
    list ($payer_note, $payee_note) = u\order($type == 'payment', $other_note, $my_note);
    $sql = <<<EOF
      UPDATE {zxs} SET 
        payer_for=CONCAT(:payer_note, payer_for), 
        payee_for=CONCAT(:payee_note, payee_for),
        data=:old_data
      WHERE xid=:zxid
EOF;
    r\db_q($sql, compact(u\ray('payer_note payee_note zxid old_data')));
    $message = 'report undo|' . $message;
    $args += array('action' => t('reversed'));
  }
  
  return array($message, $args);
}

/**
 * Request US Dollars for rCredits or vice versa
 * @param string $currency: 'r' or 'usd'
 * @param numeric $amount: how much
 * @param boolean $confirmed: has user confirmed this request
 * @return array(message index, subs, needs confirmation)
 */
function get($currency, &$amount, $confirmed) {
  $cuid = r\cuid();
  if (is_frontend()) return (array) backend('get', compact(u\ray('currency amount confirmed')));
  if (is_array($currency)) extract($currency); // called from api or backend
  
  if (!access('manage account')) return array('no access get', array(), FALSE);
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return array('bad amount', array(), FALSE);
  if ($amount < 0) return array('negative transfer', array(), FALSE);
  if ($currency == 'r') $amount = min(R_MAX_DEMAND, $amount);
  $request = u\format_amount($amount);
  $success = TRUE;
  if ($currency == 'r') {
// (catch this only when demand is high)      if ($my_amount < R_MIN_DEMAND) return array('demand too small');
    r\demand($cuid, $amount);
    return array('your demand', compact(u\ray('request success')), FALSE);
  }

  // $currency == 'usd'
  extract((array) credit_info(), EXTR_PREFIX_ALL, 'info');
  if ($amount > $info_cashable) return array("can't cash incentives", $info_fancy, FALSE);
  if (user_field('bank_account_status') != R_BANK_VERIFIED) return array('bank unverified', array(), FALSE);
  if (!r\is_rTrader()) return array('get usd inactive', $info_fancy, FALSE);
  
  $type = TX_TRANSFER;
  $payer = $cuid;
  $payee = variable_get('rcredits_community_uid');
  $taking = FALSE;
  $for = t('USD');
  $goods = FALSE;

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\transact(compact(u\ray('type amount payer payee for taking goods')));
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
function pay_invoice($zxid) {
  if (is_frontend()) return (array) backend('pay_invoice', compact(u\ray('zxid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend

  if (!access('buy and sell')) return array('no buy and sell', array());
  $state = TX_PENDING;
  $result = last_tx('xid=:zxid AND state=:state', compact('zxid', 'state'));
  extract($result, EXTR_PREFIX_ALL, 'old');
  if ($old_zxid < 0 and (r\is_rTrader($old_payer) and r\is_rTrader($old_payee))) return array("can't pay asif invoice");

  //*** Begin DBTX
  $dbtx = \db_transaction();
  list ($message, $args, $confirm) = transfer('invoice payment', array($old_payee, $old_payee_agent), $old_amount, $old_payee_for, TRUE);
  if (@$args['success']) {
    r\set_tx_state(TX_DELETED, $old_zxid);
  } else $dbtx->rollback();
  
  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  return array($message, $args);
}

/**
 * Create a transaction
 * @param string $ttype: payment, charge, or invoice payment
 * @param array $ruid: array(main_uid, agent_uid)
 * @param numeric $requested_amount: the transaction amount
 * @param string $for OR array($payer_for, $payee_for): description of the transaction's purpose (user input)
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE, used only for SMS interface)
 * @param array $rewards: associative array of info about the transaction this request is reversing (if any), indexed by:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount)
 *   'zxid': the record id of the transaction to reverse (negative if that transaction was As-If)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $paying_invoice, $goods (the transaction is an exchange for real stuff),
 * $shortfall (how inadequate the payer's balance is)
 */
function transfer($ttype, $ruid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  if (is_frontend()) return (array) backend('transfer', compact(u\ray('ttype ruid requested_amount for confirmed rewards')));
  if (is_array($ttype)) extract($ttype); // called from api or backend
  global $cuid, $auid;
  
  list ($uid, $agent_uid) = $ruid;
  if ($uid == $cuid) return array('no self-trading', array(), FALSE);
  $real = @$rewards['zxid'] < 0 ? FALSE : (r\is_rTrader() and r\is_rTrader($uid));
  $taking = ($ttype == 'charge');
  list ($my_access, $other_access) = u\order($taking, 'sell', 'buy and sell');
  if (!access($my_access)) return array("no $my_access", array(), FALSE);
  if (!access($other_access, $ruid)) return array("other no $other_access", array(), FALSE);
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
  
  $available = credit_info(array('real' => $real, 'uid' => $taking ? $uid : $cuid))->available;
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
  
  list ($payer, $payee) = u\order($taking, $uid, $cuid);
  list ($payer_agent, $payee_agent) = u\order($taking, $agent_uid, $auid);
  $rebate = $goods ? ('' . r\rebate($payer, $amount)) : '0'; // force string, to avoid long serializations
  $bonus = $goods ? ('' . r\bonus($payee, $amount)) : '0';
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;
  $state = $preapproved ? TX_DONE : TX_PENDING;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $txs = r\transact(compact(u\ray('type amount payer payee payer_agent payee_agent for data state goods')));
  $zxid = $txs[0]['zxid'];
  if (!$confirmed) $dbtx->rollback();
  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  $reward_index = $taking ? 2 : 1;
  $reward = @$txs[$reward_index]['amount'] ?: 0;

  $args = r\tx_report_args($taking, $amount, $shortfall, $reward, $uid, $zxid, $goods);
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
  if (is_array($uid)) extract($uid); // called from api or backend

  $type = TX_SIGNUP;
  $amount = R_STARTING_BALANCE;
  $payer = \variable_get('rcredits_community_uid');;
  $payee = $uid;
  $taking = FALSE;
  $goods = FALSE;
  $for = t('signup');
  r\transact(compact(u\ray('type amount payer payee for taking goods')));
  return $amount;
}

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function add_cell($number, $validate = FALSE) {
  if (is_frontend()) return backend('add_cell', compact(u\ray('number validate')));
  if (is_array($number)) extract($number); // called from api or backend

  $cuid = r\cuid();
  $number_internal = u\format_phone($number, '+n');
  $number_pretty = u\format_phone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$number_pretty) return array('bad phone', $subs);
    if ($result = r\db_q('SELECT * FROM sms_user WHERE number=:number_internal', compact('number_internal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $cuid) return array('already cell', $subs);
      //extract(r\db_q('SELECT full_name, credit_id FROM users WHERE uid=:old_uid', compact('old_uid'))->fetchAssoc());
      extract(user_field('full_name', $old_uid));
      $qid = r\quid($old_uid);
      $account_name = "$full_name ($qid)"; // maybe make the previous line and this a function
      if (!is_temp_account($old_uid)) return array('cell taken', compact(u\ray('number account_name')));
    }
    return NULL;
  }
  
  $uid = $cuid;
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
  if (is_array($number)) extract($number); // called from api or backend

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
 *   the other trader's ruid = array(uid, agent_uid)
 *   array(error index, array of substitutions, array of possible full_names (0 or more) indexed by uid)
 *   (the caller can test which type of return, with "if (is_rid($returned_value))
 */
function identify($who, $create = FALSE, $self_message = 'no self-trading') {
  $cuid = r\cuid();
  if (is_frontend()) {
    $result = backend('identify', compact(u\ray('who cuid create self_message')));
    return is_object($result) ? ((array) $result) : $result;
  }
  if (is_array($who)) extract($who); // called from api or backend

  if ($phone = u\format_phone($who, '+n')) $who = $phone;
  if (substr($who, 0, 1) == '.' or substr($who, 0, 1) == ':') $who = R_SERVER_ID . $who; // allow abbreviations within this region
//  if (substr($who, 0, 7) == 'http://') $who = substr($who, 7);
//  if (substr($who, 0, 4) == 'www.') $who = substr($who, 4);

  if (($short = u\short_name($who)) === '') $short = '-'; // abbreviations must not be null
  if ($zuid = r\quid(strtoupper($who))) return $zuid == $cuid ? array($self_message, array(), array()) : r\zuid($zuid); // qids are easy
  
  $sql = <<<EOF
    SELECT DISTINCT u.uid, u.full_name FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
    WHERE :who IN (s.number, u.phone, u.mail) 
    OR INSTR(u.name, :short)
    ORDER BY u.full_name
EOF;

  $result = r\db_q($sql, compact(u\ray('who short')))->fetchAllKeyed(0, 1);
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
    return (access('member', $uid)) ? array($uid, $uid) : array('non trader', compact('other'), $result);
  }

//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (\valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown user', compact('who'), array());
  return create_user(array($key => $who));
}

/**
 * Return financial information about the given account
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
function credit_info($args = array()) {
  if (is_frontend()) {
    $info = backend('credit_info', $args);
    $info->fancy = (array) $info->fancy; // backend turns the whole shebang into an object
  }
  extract($args);

  if (!@$uid) $uid = r\cuid();
  if (!@$state) $state = ':TX_DONE,:TX_DISPUTED';
  if (!@$real) $real = r\is_rTrader($uid);
  if (!@$asof) $asof = '';
  
  $table = r\zxs((bool) $real);
  $result_keys = 'gross_in gross_out gross_in_proper balance available unavailable rewards cashable total_demand min_balance qid';
  $sql = "SELECT min_balance FROM users WHERE uid=:uid";
  extract(r\db_q($sql, compact('uid'))->fetchAssoc()); // gets $min_balance
  $qid = r\quid($uid);

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
  
  $info['name'] = user_field('name');
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $where = 'name=:name';
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
 *   maybe like this: SELECT main_uid, MAX(permission) WHERE agent_uid=:auid 
 *     OR agent_uid IN (SELECT main_uid FROM r_relations WHERE agent_uid=:auid) GROUP BY main_uid
 */
function account_choices() {
  if (is_frontend()) return (array) backend('account_choices');
  
  global $auid;
  if (!$auid) return FALSE;
  $primary = array((string) $auid => (count($GLOBALS['share permissions']) - 1)); // max access for primary user account

  $result = r\db_q("SELECT main_uid, permission FROM r_relations WHERE agent_uid=:auid", compact('auid'));
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
  if (is_array($number)) extract($number); // called from api or backend
  
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
  $cuid = r\cuid();
  if (is_frontend()) return backend('last_tx', compact(u\ray('where subs')), $cache);
  if (is_array($where)) extract($where); // called from api or backend

//  $table = r\zxs();
  $sql = "SELECT * FROM {zxs} WHERE $cuid IN (payer, payee) AND ($where) ORDER BY zxid DESC LIMIT 1"; // make sure it's for $cuid
  if ($result = r\db_q($sql, $subs)->fetchAssoc()) {
    $result['zxid'] = r\zxid($result['xid'], r\zxs());
    unset($result['xid']);
  }
  return $result;
}

/**
 * Return the user's sequential transaction number
 * @param int $zxid: the transaction record number (negative for asif)
 * @param int $uid: the user's uid (DEFAULTs to cuid)
 * @return string: the user's transaction number, preceded by "a" for asif transactions
 */
function zxid2tid($zxid, $uid = '') {
  if (!$uid) $uid = r\cuid();
  if (is_frontend()) return backend('zxid2tid', compact(u\ray('zxid uid')));
  if (is_array($zxid)) extract($zxid); // called from api or backend

  return ($zxid < 0 ? 'a' : '') . r\db_lookup('COUNT(xid)', '{zxs}', "(payer=:uid OR payee=:uid) AND xid<=:zxid AND type<=" . TX_COUNTABLE, compact('uid', 'zxid'));
}

function account_name($uid) {
  if (is_frontend()) return backend('account_name', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid); // called from api or backend
  
  extract((array) \user_load($uid), EXTR_PREFIX_ALL, 'my');
  $qid = r\quid($uid);
//  return ": $my_full_name ($qid) $my_mail";
  return ": $my_full_name ($qid)";
}

/**
 * Return all the permissions for the specified account
 * @param mixed $zuid: rid or zuid of the account/agent to ask about (defaults to current account)
 * @return simple array of that account's permissions
 */
function accesses($zuid = '') {  
  global $czuid;
  if (is_frontend()) {
    $result = backend('accesses', compact(u\ray('zuid')), $zuid);
    return is_array($result) ? $result : array();
  }
  if (is_array($zuid) and !u\is_rid($zuid)) extract($zuid); // called from api or backend

  $a = u\is_rid($zuid) ? $zuid : r\zuid($zuid ?: $czuid);
  if ($a) list ($main_uid, $agent_uid) = $a; else return array(); // anonymous user has no access

  $perms = array();
  $role_perms = (\user_role_permissions(\user_load($main_uid)->roles)); // Drupal permissions for the company
//  $perms[] = array_values(\user_load($agent_uid)->roles); // include things like administrator and rTrader
  foreach ($role_perms as $key => $value) $perms += array_keys($role_perms[$key]);

  if (r\is_rTrader($main_uid)) $perms[] = 'rTrader';
  if (\user_load($main_uid)->account_type != R_PERSONAL) $perms[] = 'company'; // ad hoc permissions

  $cans = $GLOBALS['share permissions'];
  $crit = 'agent_uid=:agent_uid AND main_uid=:main_uid';
  $can = $main_uid == $agent_uid ? count($cans) - 1 : r\db_lookup('permission', 'r_relations', $crit, compact('main_uid', 'agent_uid'));
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
  if (is_array($table)) extract($table); // called from api or backend
  
  return r\db_q("SELECT $fields FROM $table WHERE $criteria", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

function is_temp_account($uid) {
  if (is_frontend()) return backend('is_temp_account', compact(u\ray('uid')));
  if (is_array($uid)) extract($uid); // called from api or backend
  return !user_field('pass', $uid);
}

function is_temp_name($name) {
  if (is_frontend()) return backend('is_temp_name', compact(u\ray('name')));
  if (is_array($name)) extract($name); // called from api or backend
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
  global $base_url, $auid, $cuid, $firewall;
  if (!is_frontend()) die('backend recursion');
  if (!user_is_logged_in()) return FALSE; // probably checking permission
//  if ($op == 'get_records' and !$cache) die(print_r(debug_backtrace(), 1));

  if (!@$cuid) die('no cuid in backend!');
  if (empty($info)) $cache = TRUE;
//  $cache = FALSE; // debug
//  debug("cache=$cache cachekey=rcredits_$op$cuid:" . serialize($info));
  $skey = "{$op}_{$auid}_{$cuid}_" . base64_encode(serialize($info)); // htmlspecialchars prevents session key failure (PHP bug?)
  if ($cache and (($result = r\Web\svar($skey)) !== NULL)) return $result;
//  debug($skey . $result);
//  debug("cache:$cache $op: " . json_encode($info) . ($cache ? " result:$result" : ''));
//  debug($result);
  $password = \user_load($auid)->pass;
  $firewall = R_FIREWALL;
//  foreach ($info as $key => $value) if (strpos($key, 'uid') !== FALSE) $info[$key] = r\quid($value); // convert to qid
//  $basic = array('auid' => r\quid($auid), 'cuid' => r\quid($cuid));
  $info += compact(u\ray('cuid auid op password firewall')); // operation, credentials, and account
  $host = strpos($base_url, 'localhost/') !== FALSE ? 'http://localhost/devcore' : 'http://devcore.rcredits.org';
  $api_result = R_SEPARATE_BACKEND ? u\http_request("$host/rCredits/api/", $info) : backend2($info);
//debug($api_result);
  u\log('out', $info + compact('api_result'));
  if ($cache) r\Web\svar($skey, $api_result);
  return $api_result;
}

// Shortcut functions

/**
 * @param mixed $who: criteria or uid (defaults to $cuid)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 */
function user_field($field_name, $who = FALSE, $subs = array()) {
  if (!$who) $who = r\cuid();
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
 *   a shared account permission, stored in r_relations (cached in $_SESSION['rcredits_account_choices'], but we're not trusting that)
 *   an ad hoc permission handled directly by this function
 * @param mixed $zuid: rid or zuid of the account to ask about (defaults to czuid, meaning the current account)
 * @return boolean: TRUE if the user has the permission
 */
function access($access, $zuid = '') {
  u\expect(compact(u\ray('access zuid')), 'string&notempty zid|rid|empty');
  return in_array($access, accesses($zuid));
}
