<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rCredits.module
 *   - application interfaces to the user interfaces, which are in rCredits-api.inc (i)
 *   - utilities, which are in rCredits-util.inc (u)
 *
 * Some coding conventions:
 *   qid: a mostly-alphabetic representation of an azid, eg "XXX.YYYY..." used externally for REST and SMS queries.
 *     XXX (3 letters) represents the region's uid, YYYY... (some number of letters) represents a record ID
 *     the separator can be a colon instead of period, giving YYYY... a different meaning
 *     The region (XXX) can be omitted in SMS identifiers (eg "200 to .ABCDE for pie")
 *     For each region that exists, we have a user record with uid < 0, with name = region name followed by period (eg ABC.)
 *     (Communities within the region also have negative uids, descending from -1,000,000, and do not need qids,
 *     but could have a qid with a hyphen, like AAA-BBB, if needed.)
 *
 *   zid: a signed id (non-zero) -- meaning of negative value varies
 *  azid: array(zid, region_uid), where region_uid is the uid of the server
 *  reid: a relation id (transient, never stored in any other table)
 *   uid: an account id
 *   xid: a transaction id
 *     All transaction database operations are performed by db_q() on a virtual table {txs} with key zxid.
 *  zxid: a (signed) transaction id (>0: xid in r_txs (real), <0: -xid in r_asif)
 *  ruid: an array of the three uids representing a quid: [main_uid, agent_uid, region_uid]  
 *
 *  qxid: a qid (8 characters or more) reprersenting a zxid (: means a negative zxid)
 *  quid: a qid (7 characters or more) representing a uid acting as its own agent (. separator) OR an reid (: separator)
 *
 * Globals:
 *   cuid: current account id  
 *   cacct: the current account
 *
 * @todo: change zxid to a class. $tx->xid, table, real
 * @todo
 * - handling employees (virtual payments). This should probably be extended to suppliers too,
 *   or maybe even to everyone. The idea is to pay someone virtually in rCredits by totally paying them
 *   (either first or second) in US Dollars and (either second or first) having them buy back
 *   the appropriate amount of rCredits. Probably the best way to do this is with a simple payment transaction
 *   that requires confirmation by the payee (an "offer"). The payee cannot accept the offer without first paying
 *   that amount to the Community in US Dollars (using billpay or Dwolla). [need to integrate with Dwolla, otherwise done.]
 * @todo: we also need some kind of confirmation on foreign transactions (from the other server)
 * @todo: don't require email address for SMS signups, set name properly [NOT YET. JUST LEAVE IT.]
 * @todo: store country code rather than country, and state code rather than state
 * @todo: store city code (rather than city), with a code for "other" -- store other in data or r_other (same for country/state)
 * @todo: tx data changes(date, agent, field, oldvalue) (this gives a complete history of the transaction)
 *     (prefixes added to the payer_for or payee_for fields when changing state will not be included in changes)
 * @todo: reporting for a community or server (a form that anyone can use)
 * @todo: download transactions in standard formats
 * @todo: Finish separating backend. Front end should have no registered users. This should wait until after launch.
 * @todo: use popups for some things
 * @todo
 * - remove all direct db operations from interface modules (and their .inc files) -- use api\ and u\ there, but not r\
 *
 * Notes on transaction record system:
 *   Other servers have a single negative (<0) UID on our system, so a
 *   user doing business with someone in another region would generate a transaction between an individual and a server,
 *   with qid for the individual in that other region stored in the data field. 
 *   Servers and communities have negative uids. Servers have their qid in the name field followed by a dot, eg "NEW.".
 *
 *   Specifically: when we on the AAA regional server receive a transaction request using a qid, the first four characters are the "region" --
 *   they tell us what database to hit. For example, if AAA.AAAB pays ZZZ.AAAC, we record a payment from AAA.AAAB's uid (1)
 *   to ZZZ's uid. The data field in the transaction gets $data['foreign_uid'] = ZZZ.AAAC's uid on the ZZZ server (2).
 *   We send the request to ZZZ's server and expect a confirmation 
 *   (if no confirmation, roll back the transaction and give an error message OR queue the request).
 *
 *   On the other side of things, when we receive a request from some other community for a transaction with one of our accounts
 *   -- for example if we are the "ZZZ" community, receiving a request from the "ABC" community, for ABC.AAAAA to pay ZZZ.BBBBB --
 *   we record the payer as ABC's uid and the payee as ZZZ.BBBBB's uid, with $data['payer_id'] = 'ABC.AAAAA'.
 *
 *   Thus every inter-community transaction is stored in both communities. Both communities know their trade balance, which should be
 *   public through a RESTful query. Intra-community transactions should also be stored twice -- ideally in member's devices. Smartphones and
 *   desktop computers running an app should store all their own transactions. Encrypted backups of everything to the cloud too.
 *
 * Other thoughts:
 *   Joining by dumb cellphone is for play only. Accounts and transactions created that way get deleted after a week of inactivity.
 */
 
namespace rCredits; // typically abbreviated as "r"
use \rCredits\Util as u;
use \rCredits\API as api;

include_once __DIR__ . '/rCredits-settings.inc'; // secret settings for this community's rCredits server
include_once __DIR__ . '/acct.class';
include_once __DIR__ . '/rCredits-circles.inc'; // treat this as an extension of this file, for testing

define('R_SEPARATE_BACKEND', FALSE);
define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_ASIF_TEXT', ' "AS-IF"'); // how to identify "As If" transactions

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');

// Roles
define('R_ROLE_RTRADER', 4);

// Bank account states
define('R_BANK_VERIFIED', 1); // we have proof that the user owns the bank account

// Account types
define('R_PERSONAL', 0);

$GLOBALS['account types'] = array(t('personal'), t('commercial'), t('non-profit'));
$GLOBALS['share permissions'] = array('no access', 'read transactions', 'sell', 'buy and sell', 'manage account');

// Permissions and flags
define('BIT_PARTNER', 1); // signed the membership agreement and donated
define('BIT_STEWARD', 2); // is allowed to vote and is responsible for stewarding the Common Good Community
define('BIT_RTRADER', 4); // is allowed to use rCredits for real
define('BIT_CHARGE', 8); // can charge customers without confirmation
define('BIT_ADMIN', 32768); // is an administrator (not yet implemented)

// Transaction states
define('TX_STATES', 'deleted done pending disputed denied');

define('TX_DELETED', 0);
define('TX_DONE', 1); // here and up, the transaction counts
define('TX_PENDING', 2); // charged, waiting for confirmation
define('TX_DISPUTED', 3); // unilaterally charged, but disputed by the payer (the charge stands, until settled)
define('TX_DENIED', 4); // invoice rejected by the non-originator

// Transaction channels
define('TX_WEB', 1);
define('TX_SMS', 2); 
define('TX_SMART', 3); 
define('TX_TOUCH', 4); // touch tone phone
define('TX_CHANNELS', 'None Web SMS Smart Touch');

// Transaction types
define('TX_SIGNUP', 0); // signup incentive reward
define('TX_TRANSFER', 1); // normal fund transfer (usually for actual goods and services)
define('TX_INFLATION', 2); // reward to conpensate for inflation
define('TX_REBATE', 3);
define('TX_BONUS', 4);

define('TX_COUNTABLE', 2); // higher numbers don't show on list, so they don't get transaction numbers (tid's)

$GLOBALS['email_subjects'] = array(
  'new-invoice' => t('rCredits INVOICE from {OTHER_NAME}'),
  'new-charge' => t('rCredits CHARGE from {OTHER_NAME}'),
  'invoice-denied' => t('rCredits Invoice DENIED by {OTHER_NAME}'),
  'payment-denied' => t('rCredits Payment REJECTED by {OTHER_NAME}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {OTHER_NAME}'),
);

/**
 * Format and return args for reporting a transaction (old or new)
 * Not all returned args are used for any particular type of transaction.
 * Note that templates and tests depend on the names of some of these fields
 * @param array $tx: the transaction array (including zxid)
 * @param int $get_balance: 0=don't get any balances, 1=get my balance and tid, 2=get other party's balance and tid also
 * @param float $shortfall: how much short my balance is, for a transaction (if any)
 * @return assoc array of args
 */
function tx_report_args($tx, $get_balance = 0, $shortfall = 0) {
  global $cuid;
  extract($tx); // any or all standard transaction fields
  if ($data) if ($data = unserialize($data)) extract($data, EXTR_PREFIX_ALL, 'data');
  
  $to_me = ($payee == $cuid);
  $role = $to_me ? 'payee' : 'payer';
  list ($purpose, $other_purpose) = u\order($to_me, $payee_for, $payer_for);
  $tofrom = $to_me ? t('from') : t('to');
  $uid = $other_uid = $to_me ? $payer : $payee;
  $action = $taking ? t('charged') : ($goods ? t('paid') : t('gave'));
  $confirm_action = $taking ? t('charge') : ($goods ? t('pay') : t('give'));
  $summary_word = $taking ? t('Charge') : t('Payment');
  $amount = u\format_amount($amount);
  $short = u\format_amount($shortfall);
  if (@$data_bonus) $reward_type = $taking ? t('bonus') : t('rebate');
  $reward_amount = u\format_amount($to_me ? @$data_bonus : @$data_rebate);
  $other_name = user_field('full_name', @$data_foreign_uid ?: $other_uid);
  $created = strftime('%d-%b-%Y', $created);

  $real = ($zxid > 0);
  $asif = $real ? '' : R_ASIF_TEXT;
  if ($get_balance > 0) {
    list ($tid, $balance) = array(api\zxid2tid($zxid), api\credit_info(compact('real'))->fancy['balance']);
    if ($get_balance > 1) {
      list ($other_tid, $other_balance) = array(api\zxid2tid($zxid, $other_uid), api\credit_info(compact('real', 'uid'))->fancy['balance']);
    }
  }
  
  $summary = "$asif $summary_word of $amount $tofrom $other_name for \"$purpose\" on $created";
  $success = TRUE;
  return array_merge($tx, compact(u\ray('created zxid action real role purpose other_purpose to_me asif confirm_action amount tofrom reward_type reward_amount balance other_balance tid other_tid other_uid other_name short summary success')));
}

/**
 * Record a transaction between two participants
 * @see also transaction_setup(), r\transaction_rewards(), and api\transfer()
 */
function transact($info) {
  $txs = array(transaction_setup($info));
  transaction_rewards($txs);
  return tx_save($txs);
}

/**
 * Save the transaction set
 * @todo: don't give real rewards to the passive party in an ASIF transaction (look retroactively)
 */
function tx_save($txs) {
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  //*** Begin DBTX
  $dbtx = db_transaction();
  $table = zxs($my_payer, $my_payee);
  if (@$my_data['zxid'] < 0) $table = zxs(FALSE/2); // rare case: undoing an asif, even though both parties are now rTraders
  foreach ($txs as $key => $one) {
    if (empty($one)) continue; // skipping rebate or bonus
    $txs[$key]['serial'] = @$serial; // set serial for dependent transactions
    \drupal_write_record($table, $txs[$key]); // pass actual array element, to get xid
    if ($key == 0) {
      $txs[$key]['serial'] = $serial = $txs[0]['xid']; // set serial for main transaction
      db_q("UPDATE $table SET serial=:serial WHERE xid=:serial", compact('serial'));
    }
  }

  unset($dbtx); // commit ($dbtx->commit() is disallowed)
  //*** End DBTX

  $txs[0]['zxid'] = zxid($txs[0]['xid'], $table); // report zxid to caller
  return $txs;
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 */
function transaction_rewards(&$txs) {
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  
  if ($my_type != TX_TRANSFER) return;
  if (!$my_goods) return; // quit unless rebates and bonuses apply
  if (owning($my_payer, $my_payee)) return; // no rewards for transactions between owner and owned
  $rewards = unserialize($my_data);
  
  $txs[] = tx_reward_setup(TX_REBATE, $my_payer, $my_state, $rewards);
  $txs[] = tx_reward_setup(TX_BONUS, $my_payee, $my_state, $rewards);
}

function tx_reward_setup($type, $payee, $state, $rewards) {
  $payer = $payer_agent = $payee_agent = user_field('community_uid', $payee);
  if ($payer == $payee) return NULL; // payee is a community, so no rewards

  $taking = FALSE;
  $goods = FALSE;
  $amount = $rewards[$type == TX_REBATE ? 'rebate' : 'bonus'];
  $for = $type == TX_REBATE ? t('rebate') : t('bonus');
  return transaction_setup(compact(u\ray('type amount goods payer payee payer_agent payee_agent for taking state')));
}

/**
 * Create a new transaction and set up all the important fields.
 * @param array $info: an associative array of transaction info. The following elements are required:
 *   amount: how much to transfer
 *   payer, payee: uids
 *   payer_agent, payee_agent: uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   taking (bool): did the payee initiate the transaction
 * @return the array, with some extra field information added
 */
function transaction_setup($info) {
  global $channel; // through what medium was the transaction requested (web, SMS, etc.).
  extract($info, EXTR_PREFIX_ALL, 'my');
  $required_params = compact(u\prefix_values('my_', u\ray('amount payer payee payer_agent payee_agent for taking')));
  u\expect($required_params, 'float zid zid zid zid notempty bool');

  if (!isset($my_state)) $state = TX_DONE;
  if (!isset($my_goods)) $goods = FALSE; // set TRUE only in api\transfer()
  $created = REQUEST_TIME;
  $serial = NULL; // always set equal to xid of main transaction after saving
  
  list ($payer_for, $payee_for) = is_array($my_for) ? $my_for : array($my_for, $my_for);
  unset($info['for']);

  return array_merge($info, compact(u\ray('state goods created serial payer_for payee_for channel')));
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 * @param integer $who: uid of the user
 * @param numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($uid, $demand = 0) {
  if (!$demand) return user_field('demand', $uid);
  $demand = min(R_MAX_AMOUNT, $demand);
  user_update(compact('demand'), $uid);
  return $demand;
}

/**
 * Calculate how much too little the payer has
 */
function shortfall($uid, $requested_amount, $preapproved, $real) {
  if (!$preapproved) return 0; // if this is just an invoice, funds don't have to be there yet (no shortfall)
  $available = api\credit_info(compact('uid', 'real'))->available;
  $amount = min($requested_amount, $available);
  $shortfall = max(0, $requested_amount - $amount); // how much needs to be done in a separate currency
  if ($shortfall) {
    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
    $short = u\format_amount($shortfall);
    $shortfall = ceil($shortfall); // shortfall should be a round figure (no cents)
  }
  return $shortfall;
}

function pay_employees($amount0, $for, $confirmed) {
  global $cuid;
  $employees = employees();
  if (empty($employees)) return array('no employees', array(), array());
  if ($shortfall = shortfall($cuid, $amount0, TRUE, is_rTrader())) {
     $short = u\format_amount($shortfall);
     return array('short payment', compact('short'), FALSE); // no partial payment of employees
  }
  $employee_count = count($employees);
  $amount_unformatted = round($amount0 / $employee_count, 2);
  $total = u\format_amount($amount_unformatted * $employee_count);
  $amount = u\format_amount($amount_unformatted);
  if (!$confirmed) return array('pay employees', compact(u\ray('employee_count amount total')), TRUE);
  $pay_all = TRUE;
  $results = array();
  $successes = 0;
  foreach ($employees as $acct) {
    $results[] = list ($message, $args, $confirm) = api\transfer('payment', $acct, $amount_unformatted, $for, !u\for_cash($for), TRUE, compact('pay_all'));
    if ($args['success']) $successes++;
  }
  $success = ($successes > 0);
  return array('paid employees', compact(u\ray('total successes employee_count results success')), FALSE);
}
 
/**
 * Notify the specified user, according to their preferences
 */
function notify($uid, $message_index, $info) {
  $account = acct($uid)->account();
  extract((array) $account, EXTR_PREFIX_ALL, 'my');
  $standard_fields = array(
    'uid' => $uid,
    'full_name' => @$my_full_name ?: t('rCredits Member'),
    'email' => $my_mail, // testing uses this
  );
  if ($uid != cuid()) $standard_fields['other_name'] = user_field('full_name'); // this comes in backwards if notifying elseone
  
  $subs = array();
  foreach (array_merge($info, $standard_fields) as $key => $value) $subs['{' . strtoupper($key) . '}'] = $value;
  drupal_mail('rCredits', $message_index, $my_mail, user_preferred_language($account), $subs);
}

/**
 * Return a unique temporary name related to the given uniquely identifying parmeters 
 * in this order of preference: 
 *   phone: "unknown phone dddddddddd"
 *   email: "unknown whatever AT domain DOT ext"
 *   UNUSED (because it's hard to verify): website: "unknown domain DOT ext SLASH andsoforth"
 * If none, just pick a 10-character random string ("unknown 234h2khpsdf987")
 * @param array $info: associative array of fields, each of which uniquely identifies a user.
 * @return string: the temporary name
 */
function temp_name($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = @$my_number ?: (@$my_phone ?: '');
  if ($phone) return R_TEMP_NAME_PREFIX . u\format_phone($phone, 'n');
  if (isset($my_email)) return R_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return R_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return R_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

/**
 * Set a transaction field for the current user.
 * @param string $field_name: the field to update
 * @param mixed $new_value: what to update it to
 * @param int $zxid: transaction id & table
 * @param bool $all_related: set all related transactions too (default FALSE)
 */
function set_tx_field($field_name, $new_value, $zxid, $all_related = FALSE) {
  global $cuid;
  u\expect(compact(u\ray('field_name zxid all_related')), 'name zid bool');
  
  list ($xid, $table) = azxid($zxid);
  $crit = "xid=:xid and $cuid in (payer, payee)"; // "$cuid in" is a security measure
  list ($old_value, $data) = array_values(dbLookup("$field_name,data", $table, $crit, compact(u\ray('xid'))));
  $data = unserialize($data);
  $data['changes'][] = array(time(), acct()->agent, $field_name, $old_value);
  $data = serialize($data);
  $sql = "UPDATE $table SET $field_name=:new_value, data=:data WHERE $crit";
  $sql2 = str_replace(', data=:data', '', str_replace('xid=:xid', 'serial=:xid', $sql));
  
  /*** Begin DBTX ***/
  $dbtx = \db_transaction();
  db_q($sql, compact(u\ray('new_value data xid')));
  if ($all_related) db_q($sql2, compact(u\ray('new_value xid'))); // gotta do the auxiliary records separately so as not to affect their data field (separate query, to avoid "unbuffered query" error -- known Drupal bug)
  unset($dbtx);
  /*** End DBTX ***/
}

/**
 * Merge a temporary ("old") account into the current account (a permanent ("new") account).
 * @return 1 (for counting how many were merged)
 */
function merge_accounts($old_uid) {
  $new_uid = cuid();
  if (!api\is_temp_account($old_uid)) return; // don't merge non-temp account
  u\expect(api\access('manage account'), "unauthorized merge_accounts: $new_uid into $old_uid");
//  if (!is_temp_name(user_field('name', $old_uid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = acct($new_uid);
  $old = acct($old_uid);
  $update = array();
  foreach (u\ray('phone fax website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  
  /*** Begin DBTX ***/ $dbtx = \db_transaction();
  user_update($update, $new_uid);
  $sql = <<<EOF
    UPDATE r_asif SET payer=:new_uid WHERE payer=:old_uid
    UPDATE r_asif SET payee=:new_uid WHERE payee=:old_uid
    DELETE FROM r_asif a LEFT JOIN r_asif b ON b.serial=a.serial WHERE b.payer=b.payee
    DELETE FROM r_asif WHERE payer=payee
    UPDATE r_sms SET uid=:new_uid WHERE uid=:old_uid
    UPDATE r_user_industries SET uid=:new_uid WHERE uid=:old_uid
    DELETE FROM users WHERE uid=:old_uid
EOF;
  db_q($sql, compact('new_uid', 'old_uid'));
  /*** End DBTX ***/ unset($dbtx);
  return 1;
//  db_q("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:old_uid", compact('old_uid'));
}

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
//  $ctty_uid = server_uid();
  $sqls = array(
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE r_asif',
    'TRUNCATE TABLE r_sms',
//    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
  \rebuild_menus();
  drupal_goto('devel/cache/clear?destination=node');
}

/**
 * Return the real or ASIF transaction table name
 * @param int $uid1: uid of first member (DEFAULTs to cuid)
 *   OR boolean/2 $uid1: TRUE/2 if real (call with boolean/2 to allow administrator to do asif transactions)
 * @param int $uid2: uid of second member (DEFAULTs to cuid)
 * @return: the real table name if the one user or both users (if specified) are rTraders, otherwise the asif table name
 */
function zxs($uid1 = 'cuid', $uid2 = '') {
  global $cuid;
  if ($uid1 == 'cuid') $uid1 = $cuid;
  $real = ($uid1 >= 0 and $uid1 < 1) ? (2 * $uid1) : (is_rTrader($uid1) and is_rTrader($uid2)); // $real < 1 means boolean
  return $real ? 'r_txs' : 'r_asif';
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function dbLookup($field_name, $table_name, $criteria = '1', $subs = array()) {
  u\expect(compact(u\ray('field_name table_name criteria subs')), 'notempty notempty notempty assoc');
  $sql = "SELECT $field_name FROM $table_name WHERE $criteria";
  if (!strpos($criteria, ' LIMIT ')) $criteria .= ' LIMIT 1';
  $result = db_q($sql, $subs);
  return strpos($field_name, ',') ? $result->fetchAssoc() : $result->fetchField();
}

/**
 * Return a qid for the zid
 * @param $zid: a zid
 * @param string $region_id: the 3-character regional server ID (defaults to this server)
 * @param int $min_len: minimum length of the part of the qid after the "." or ":"
 * @return the qid, FALSE if wrong format
 */
function qid($zid = '', $region_id = R_SERVER_ID, $min_len = 1) {
  if (!u\is_zid($zid)) return FALSE;
  return $region_id . ($zid < 0 ? (':' . u\n2a(-$zid, -$min_len)) : ('.' . u\n2a($zid, -$min_len)));
}

/**
 * Return an azid and region for the qid
 * @return [zid, region_uid]
 */
function azid($qid) {
  if (!u\is_qid($qid)) return FALSE;
  $qid = str_replace('..', ':', $qid); // allow a horizontal colon
  $type = strpos($qid, '.') === FALSE ? ':' : '.';
  list ($region_id, $iid) = explode($type, $qid, 2); // separate into region and individual id
  
  $id = u\a2n($iid);
  return array($type == '.' ? $id : -$id, server_uid($region_id ?: R_SERVER_ID));
}
  
function cuid() {global $cuid; return @$cuid + 0;} // current account (may be zero, if not logged in)

/**
 * Convert ruid to quid
 * Call by:
 *   quid()
 *   quid(uid) == quid(uid, uid)
 *   quid(ruid)
 *   quid(uid, agent_uid) -- region_uid defaults to current region
 *   quid(uid, agent_uid, region_uid) -- these are the elements of an ruid array
 * @param ruid $ruid: the ruid to convert (DEFAULT current account)
 * @return the corresponding quid
 */
function quid($ruid = '', $agent_uid = '', $region_uid = '') {
  $cacct = acct();
  if ($agent_uid) {
    $uid = $ruid;
    if (!$region_uid) $region_uid = server_uid();
  } else list ($uid, $agent_uid, $region_uid) = 
      ( is_array($ruid) ? $ruid 
    : ( ($uid = $ruid) ? array($uid, $uid, server_uid()) 
    : ( array($cacct->id, $cacct->agent, $cacct->region) )));
  return qid($uid == $agent_uid ? $uid : -reid($uid, $agent_uid), u\n2a(-$region_uid, 3), 3);
}

function qxid($zxid, $region_id = R_SERVER_ID) {return qid($zxid, $region_id = R_SERVER_ID, 4);} // Convert zxid to qxid

/**
 * Convert a zuid to uid and agent uid or vice-versa
 * zuid() returns $czuid
 * zuid($zuid) returns and rid = array($uid, $agent_uid)
 * zuid($uid, $agent_uid) returns $zuid ($uid if $uid==$agent_uid, else -$reid)
 * @return as above; FALSE if no such
 */ /*
function zuid($id = '', $agent_uid = '') {
  global $czuid;
  if (!$id) return $czuid;
  if (!$agent_uid) return ($reid = -$id) > 0 ? array_values(dbLookup('main_uid,agent_uid', 'r_relations', 'reid=:reid', compact('reid'))) : array($id, $id);
  return $id == $agent_uid ? $id : -dbLookup('reid', 'r_relations', 'main_uid=:id AND agent_uid=:agent_uid', compact(u\ray('id agent_uid')));
} */

/**
 * Convert a zxid to xid and table or vice-versa
 */
function zxid($xid, $table) {return $table == zxs(TRUE/2) ? $xid : -$xid;}
function azxid($zxid) {return $zxid > 0 ? array($zxid, zxs(TRUE/2)) : array(-$zxid, zxs(FALSE/2));}

function tx2acct($id, $agent, $data) {
  if (is_array($data)) extract($data); // foreign_uid and/or foreign_agent (or neither)
  return acct(@$foreign_uid ? qid($foreign_uid, $id) : $id, @$foreign_agent ? qid($foreign_agent, $agent) : $agent);
}

/**
 * Say whether one of the accounts owns the other.
 */
function owning($uid1, $uid2) {
  $crit = 'is_owner AND ((main_uid=:uid1 AND agent_uid=:uid2) OR (main_uid=:uid2 AND agent_uid=:uid1))';
  return $uid1 == $uid2 ?: dbLookup(1, 'r_relations', $crit, compact('uid1', 'uid2'));
}

function community_uid($postal_code = '') {
  if (!$postal_code) return server_uid();
  $crit = "country=':R_COUNTRY' AND :postal_code RLIKE postal_code AND uid=community_uid";
  return user_field('uid', $crit, compact(u\ray('postal_code'))) ?: server_uid();
}

function is_foreign($uid) {
  $name = user_field('name', $uid);
  return $name == (R_SERVER_ID . '.') ? FALSE : ($uid < 0 and preg_match('/[A-Z]{3}\./', $name));
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function unique_name($full_name) {
  $name = $name0 = u\short_name($full_name);
  for ($i = 2; ; $i++) {
    if (!user_field(1, 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  global $cuid;
  $result = db_q('SELECT agent_uid, foreign_uid FROM r_relations WHERE main_uid=:cuid AND is_employee', compact('cuid'));
  $accts = array();
  foreach ($result as $key => $one) {
    extract((array) $one); // agent_uid, foreign_uid
    $accts[] = $agent_uid > 0 ? acct($agent_uid) : acct(qid($foreign_uid, $agent_uid));
  }
  return $accts;
}

/**
 * @param mixed $who: criteria or uid (defaults to $cuid)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 */
function user_field($field, $who = 'none', $subs = array()) {
  global $cuid;
  if ($who == 'none') $who = $cuid;
  if (!$who) return FALSE;
  if (is_numeric($uid = $who) and !strpos($field, '(')) { // handle numbered accounts efficiently
    if (!($acct = acct($uid))) return FALSE;
    if (strpos($field, ',')) {
      $result = array();
      foreach (u\ray($field) as $one) {
        if (strpos($one, ' AS ')) list($one, $key) = explode(' AS ', $one); else $key = $one;
        $result[$key] = $acct->$one;
      }
      return $result;
    } else return $acct->$field;
  }
//  debug(compact(u\ray('field who subs')));
  $result = api\get_records('users', is_numeric($who) ? "uid=$who" : $who, $subs, $field, TRUE);
  if (count($result) < 1) return FALSE;
  u\expect(count($result) == 1, "ambiguous user in user_field(): $who");
  u\expect(is_object($result[0]), 'bad object array in user_field(): ' . print_r($result, 1));
  return strpos($field, ',') ? ((array) $result[0]) : $result[0]->$field;
}

function user_update($info, $uid = '') {
  if ($acct = acct($uid)) {
    $acct->update($info); 
  } else {
    $info['uid'] = $uid;
    \drupal_write_record('users', $info, 'uid');
  }
}

/**
 * @todo: maybe move the processing of zxs to a wrapper function db_txq() that also does postprocessing on fetchAssoc()
 */
function db_q($sql, $subs = array(), $die = FALSE) {
  u\expect(compact(u\ray('sql subs')), 'notempty assoc');
  if (strpos($sql, '{zxs}')) {
    extract($subs, EXTR_PREFIX_ALL, 'my');
    if (@$my_zxid) {
      list ($subs['xid'], $table) = azxid($my_zxid);
      unset($subs['zxid']);
    } else {
      $table = zxs((isset($my_real) ? $my_real : is_rTrader())/2);
      unset($subs['real']);
    }
    $sql = strtr($sql, array('{zxs}' => $table, 'zxid' => 'xid'));
  }
//  debug(u\get_constants($sql));
//  debug($subs);
//  if ($die)
 //if (@$subs['info']) {print_r(u\get_constants($sql) . ' ' . print_r(u\prefix_keys(':', $subs), 1)); die();}
  return \db_query(u\get_constants($sql), u\prefix_keys(':', $subs));
}

function error($logid) {
  $info = dbLookup('info', 'r_log', 'logid=:logid', compact('logid'));
  if ($info) $info = json_decode($info); else $info = 'unknown error';
  print_r($info);
//  drupal_set_message(print_r($info, 1), 'error');
//  drupal_goto('');
}

// Single line functions that need no explanation
function rebate($uid, $amount = 1) {return round($amount * user_field('rebate_percent', $uid) / 100, 2);}
function bonus($uid, $amount = 1) {return rebate($uid, 2 * $amount);}
// UNUSED tx_field($field_name, $xid, $real) {return dbLookup($field_name, zxs($real/2), 'xid=:xid', compact('xid'));}
function set_tx_state($new_value, $zserial) {set_tx_field('state', $new_value, $zserial, TRUE);}
function total_demand() {return dbLookup('SUM(demand)', 'users');}
function server_uid($region_id = '') {return -u\a2n($region_id ?: R_SERVER_ID);}
function reid($main_uid, $agent_uid) {return dbLookup('reid', 'r_relations', 'main_uid=:main_uid AND agent_uid=:agent_uid', compact(u\ray('main_uid agent_uid')));}
function ok_agent($uid, $agent_uid) {return ($uid == $agent_uid or reid($uid, $agent_uid));}
function is_rTrader($uid = '') {return (user_field('flags', $uid ?: cuid()) & BIT_RTRADER);}
function email_template($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function email_body($key) {return str_replace('{BODY}', email_template($key), email_template(R_EMAIL_FRAME));}
