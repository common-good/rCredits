<?php
/**
 * @file
 * Smart phone interface to the rCredits system.
 */

namespace rCredits\Smart;
use rCredits as r;
use rCredits\Util as u;
use rCredits\API as api;

/**
 * Receive and process a REST request from a smart device
 * @param assoc $args: an associative array (defaults $_POST) of a json-encoded assoc that normally includes:
 *   op: what to do (determines what function to call)
 *   my_id: agent qid
 *   code: permanent code that we sent to the device in First Time response (used to set $cuid)
 *     for now, user cannot change the active account, just the agent (eg sales clerks)
 *     if $op = 'first_time', expect $password instead
 *   account_id: the qid for the other party, if any 
 *     This gets passed to the specific op functions as $acct
 * @return (normally by calling either api\error or doOp):
 *   $success=1 if doOp is called, otherwise 0 ($op = 'photo' is an exception; see below)
 *   $message: an error message or help message to display on the device; or not set
 */
function api($args = FALSE) {
  global $cuid, $channel;
  $channel = TX_SMART;
  if (!$args) $args = $_POST;
  u\log('in', $args);
  if (!(@$args = (array) json_decode($args['json']))) return api\error('no json');
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (@$q_op != 'reset') // DEBUG
  if (@$q_op != 'first_time') {
    if (!@$q_code) return api\error('no code given');
    if (!($cuid = r\dbLookup('uid', 'r_smarts', 'code=:q_code', compact('q_code')))) return api\error('unknown device');
    if (!u\is_qid($q_my_id)) return api\error('bad id format');
    if (!($cacct = r\acct($cuid, $q_my_id, $error))) return api\error($error);
    if ($cacct->region != r\server_uid()) return api\error('wrong server');
    if ($cacct->agentRegion != r\server_uid()) return api\error('wrong server for agent');
    if (!r\ok_agent($cuid, $cacct->agent)) return api\error('no permission'); // make sure agent has permission
    $args['cacct'] = $cacct;
    $edit = array('code' => $q_code, 'accessed' => time());
    \drupal_write_record('r_smarts', $edit, 'code'); // so we can drop the deadwood
  }

  if (u\is_qid(@$q_account_id)) {
    if (!($acct = r\acct($q_account_id, '', $error))) return api\error($error);
//    list ($uid, $agent_uid, $region_uid) = $acct->UAR();
    $args['acct'] = $acct;
//    $args = array_merge($args, compact('acct')); // use array_merge (not +) to assure overwrite
  }
  return api\doOp(@$q_op, $args, 'Smart');
}

function reset($args) {
  r\db_q('DELETE FROM r_smarts WHERE 1');
  return array();
}

/*
 * Requesting a permanent code from the server.
 * @param array $args:
 *   my_id
 *   password: the user's Drupal account password
 * @return:
 *   code: permanent code for app to prove the identity of the device
 *   owner_id: the device owner's account ID
 *   update_link: URL of updated app; not set if no update is available
 */
function first_time($args) {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (!u\is_qid(@$q_my_id)) return api\error('bad id');
  if (!($cacct = r\acct(@$q_my_id))) return api\error('unknown user');
  if (@!$q_password) return api\error('login error');
  if (!user_check_password($q_password, $cacct->account())) return api\error('bad login'); // make these two error messages distinct but obscure
  if (!($code = makeDevice(t('Smart Device'), $cacct))) return api\error("can't save device info");
  $owner_id = $cacct->qid;
  return compact(u\ray('code owner_id')) + startup($args);
}

/*
 * Respond to the application being run.
 * @param array $args: (none)
 * @return:
 *   update_link (URL of updated app; not set if no update is available)
 */
function startup($args) {
  if (rand() < getrandmax() / 2) $update_link = 'http:/rCredits.org/files/smart.apk';
  $allow_change_account = 0;
  $allow_change_agent = TRUE;
  $require_agent = api\access('company');
  return compact('update_link allow_change_account allow_change_agent require_agent');
}

/*
 * Identify the other party.
 * @param array $args:
 *   account_id: the qid for an individual or company and rep
 * @return:
 *   full_name: the individual's name
 *   company_name: the company name (if any)
 *   location: the location of the company if it's a company, otherwise of the individual (typically city, state)
 *   other_balance: the current balance of the company if it's a company, otherwise of the individual
 */
function identify($args) {
  $cuid = r\cuid();
  extract($args, EXTR_PREFIX_ALL, 'q');
  $full_name = $q_acct->full_name;
  if ($q_acct->id == $cuid) return api\error(r\Web\tt('no self-trading'));
  $country = $q_acct->country;
//  $location = $q_acct->city . ', ' . ($country == 'United States' ? u\state_abbrev($q_acct->state) : $country);
  $location = $q_acct->city . ', ' . $q_acct->state . ($country == 'United States' ? '' : ", $country");
  if (!$q_acct->proSe()) list ($company_name, $full_name) = array($full_name, r\acct($q_acct->agent)->full_name);
  $uid = $q_acct->id;
  if (api\access('charge others')) $other_balance = api\credit_info(compact('uid'))->balance;
  return compact(u\ray('full_name location company_name other_balance'));
}

/*
 * Get an individual's photo.
 * @param array $args:
 *   account_id (converted to ruid): the qid for an individual or company and rep
 * @return:
 *   just the photo data (no json)
 */
function photo($args) {
  global $base_url;
  extract($args, EXTR_PREFIX_ALL, 'q');
  $pic_filename = r\Web\profile_picture(@$q_acct->id);
  $default = "$base_url/sites/default/files/pictures/no-photo-available.jpg";
  $pic = file_get_contents($pic_filename ?: $default);
  if (!$pic) $pic = file_get_contents($default);
  $message = bin2hex(substr($pic, 0, 50));
  u\log('out', compact('message'));

  if (defined('TESTING')) {debug($message); return $message;} else exit($pic);
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
function transact($args) {
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (!$q_goods) $q_purpose = 'cash'; // tell transfer no rewards
  list ($index, $subs, $zot) = api\transfer($q_type, $q_acct, $q_amount, $q_purpose, $q_goods, TRUE);
  $tx_id = r\qxid($subs['zxid']); // return the transaction id as a qid (globally unique transaction id) (':' means As-If)
  extract(balances($q_acct->id, $subs['real'])); // my_balance, other_balance
  $message = r\Web\tt($index, $subs);
  //if (!@$subs['success'])
  return compact(u\ray('tx_id my_balance other_balance message'));
}

function balances($uid, $real) {
  $my_balance = api\credit_info(compact('real'))->balance;
  if (api\access('charge others')) $other_balance = api\credit_info(compact('real', 'uid'))->balance;
  return compact('my_balance', 'other_balance');
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
function undo($params) {
  extract($params, EXTR_PREFIX_ALL, 'q');
  
  list ($zxid, $region_uid) = r\azid($q_tx_id);
  if ($region_uid != r\server_uid()) return api\error('That is not a transaction on this server.');
  
  if ($q_confirmed) {
    list ($message, $args) = api\undo_tx($zxid);
    $message = r\Web\tt($message, $args);
    if (!@$args['success']) return api\error($message);
    if ($new_zxid = @$args['zxid']) $tx_id = r\qxid($new_zxid); // get offsetting transaction id, if any (otherwise state changed)
    $balances = balances($args['other_uid'], $zxid > 0);
  } else {
    if (!($result = api\last_tx('xid=:zxid', compact('zxid')))) return api\error(r\Web\tt('undo no match'));
    $message = r\Web\tt('confirm undo', r\tx_report_args($result));
    $balances = array();
  }
  return compact(u\ray('message tx_id')) + $balances;
}

function makeDevice($device, $acct, $code = '') {
  $uid = $acct->id;
  $code = $code ?: user_hash_password(uniqid($acct->qid, TRUE) . time()) . '/'; // add a slash to make sure slashes get handled properly
  $accessed = time();
  $info = compact(u\ray('uid code device $accessed'));
  return @\drupal_write_record('r_smarts', $info) ? $code : FALSE; // no way to tell if it's the same device as others
}
