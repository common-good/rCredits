<?php
/**
 * @file
 * Smart phone interface to the rCredits system.
 */

namespace rCredits\Smart; // typically abbreviated as "smart"
use \rCredits as r;
use \rCredits\Util as u;
use \rCredits\API as api;

/**
 * Receive and process a REST request from a smart device
 * @param array $args: an associative array that normally includes:
 *   op: what to do (determines what function to call)
 *   my_id: agent qid
 *   code: permanent code that we sent to the device in First Time response (used to set $cuid)
 *     for now, user cannot change the active account, just the agent (eg sales clerks)
 *     if $op = 'first_time', expect $password instead
 *   account_id: the qid for the other party, if any 
 *     This gets passed to the specific op functions as $ruid = array($uid, $agent_uid, $region_uid)
 * @return (normally by calling either apiError or api2):
 *   $success=1 if api2 is called, otherwise 0 ($op = 'photo' is an exception; see below)
 *   $message: an error message or help message to display on the device; or not set
 */
function api() {
  global $cuid, $cacct, $channel;
  $channel = TX_SMART;
  u\log('in', $_POST);
  if (!(@$args = (array) json_decode($_POST['json']))) apiError('no json');
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (@$q_op != 'reset') // DEBUG
  if (@$q_op != 'first_time') {
    if (!@$q_code) apiError('no code given');
    if (!($cuid = r\db_lookup('uid', 'r_smarts', 'code=:q_code', compact('q_code')))) apiError('unknown device');
    if (!is_qid($q_my_id)) apiError('bad id format');
//    if (!($cruid = r\ruid($q_my_id))) apiError('bad id');
//    if ($cruid[0] != $cruid[1]) apiError('agent agent not allowed'); // avoid errors in r\acct
    if (!r\acct::ok($cuid, $q_my_id, $error)) apiError($error); // avoid errors in r\acct
    $cacct = new r\acct($cuid, $q_my_id);
    
//    $cruid = r\ruid(@$q_my_id);
//    if (!@$cruid) apiError('bad id');
//    list ($auid, $auid, $region_uid) = $cruid;
    if ($cacct->region != r\server_uid()) apiError('wrong server');
    if ($cacct->agentRegion != r\server_uid()) apiError('wrong server for agent');
    if (!r\ok_agent($cuid, $cacct->agent)) apiError('You do not have permission to use that account.'); // make sure agent has permission
  }

  if (u\is_qid(@$q_account_id)) {
    
    $ruid = r\ruid($q_account_id);
    if (!$ruid) apiError('bad account_id');
    list ($uid, $agent_uid, $region_uid) = $ruid;
    $args = array_merge($args, compact(u\ray('ruid uid agent_uid region_uid'))); // use array_merge (not +) to assure overwrite
  }
  api2(@$q_op, $args, 'Smart');
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
//  if (!($account = \user_load_by_name(@$q_my_id))) apiError('unknown user');
  if (!($ruid = r\ruid(@$q_my_id))) apiError('bad id');
  $uid = $ruid[0];
  if (!($account = \user_load($uid))) apiError('unknown user');
  if (@!$q_password) apiError('login error');
  if (!user_check_password($q_password, $account)) apiError('bad login'); // make these two error messages distinct but obscure
  $code = user_hash_password(uniqid($q_my_id, TRUE)) . '/'; // add a slash to make sure slashes get handled properly
  $owner_id = r\quid($uid, $uid);
  $device = t('Smart Device');
  \drupal_write_record('r_smarts', compact(u\ray('uid code device'))); // no way to tell if it's the same device as others
  return compact(u\ray('code owner_id')) + startup($args);
}

/*
 * Respond to the application being run.
 * @param array $args: (none)
 * @return:
 *   update_link (URL of updated app; not set if no update is available)
 */
function startup($args) {
  if (rand() < getrandmax() / 2) $update_link = 'http://rCredits.org/files/smart.apk';
  $allow_change_account = 0;
  $allow_change_agent = TRUE;
  $require_agent = api\access('company');
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
function identify($args) {
  $cuid = r\cuid();
  extract($args, EXTR_PREFIX_ALL, 'q');
  $full_name = \user_load($q_uid)->full_name;
  if ($q_uid == $cuid) apiError(r\Web\tt('no self-trading'));
  $account = \user_load($q_uid); // the individual or company account
  $country = $account->country;
  $location = $account->city . ', ' . ($country == 'United States' ? u\state_abbrev($account->state) : $country);
  if (@$q_company_uid) $company_name = r\user_field('full_name', $q_company_uid);
  $uid = $q_uid;
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
function transact($args) {
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (!$q_goods) $q_purpose = 'cash'; // tell transfer no rewards
  list ($index, $subs, $zot) = api\transfer($q_type, $q_acct, $q_amount, $q_purpose, $q_goods, TRUE);
  $tx_id = r\qxid($subs['zxid']); // return the transaction id as a qid (globally unique transaction id) (':' means As-If)
  list ($my_balance, $other_balance) = balances($q_uid, $subs['real']);
  $message = r\Web\tt($index, $subs);
  //if (!@$subs['success'])
  return compact(u\ray('tx_id my_balance other_balance message'));
}

function balances($uid, $real) {
  $my_balance = api\credit_info(compact('real'))->balance;
  if (api\access('charge others')) $other_balance = api\credit_info(compact('real', 'uid'))->balance;
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
function undo($params) {
  extract($params, EXTR_PREFIX_ALL, 'q');
  
  list ($zxid, $region_uid) = r\azid($q_tx_id);
  if ($region_uid != r\server_uid()) apiError('That is not a transaction on this server.');
  
  if ($q_confirmed) {
    list ($message, $args) = api\undo_tx($zxid);
    $message = r\Web\tt($message, $args);
    if (!@$args['success']) apiError($message);
    if ($new_zxid = @$args['zxid']) $tx_id = r\qxid($new_zxid); // get offsetting transaction id, if any (otherwise state changed)
    list ($my_balance, $other_balance) = balances($args['other_uid'], $zxid > 0);
  } else {
    if (!($result = api\last_tx('xid=:zxid', compact('zxid')))) apiError(r\Web\tt('undo no match'));
    $message = r\Web\tt('confirm undo', r\tx_report_args($result));
  }
  return compact(u\ray('message tx_id my_balance other_balance'));
}
