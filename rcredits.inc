<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in rcredits-api.inc (i)
 *   - utilities, which are in rcredits-util.inc (u)
 *
 * Some coding conventions:
 *  reid: a relation id (transient, never stored in any other table)
 *   uid: an account id         zuid: a (signed) account id (positive: uid, <0: -rid)
 *   xid: a transaction id      zxid: a (signed) transaction id (>0: xid in r_txs (real), <0: -xid in r_asif)
 *     All transaction database operations are performed by db_q() on a virtual table {txs} with key zxid.
 *  ruid: an array of the two uids represented by a zuid
 *
 *   cuid: current account id  
 *   auid: the user's logged-in account id (agent uid)
 *  czuid: the zuid for cuid + auid
 *
 *   a "q" id is a mostly-alphabetic representation of a signed id (zuid or zxid) plus the server id. 
 *   It is used externally for REST and SMS queries. The qid takes two forms:
 *     XXX.YYYY... or XXX:ZZZZ... where
 *       XXX is R_SERVER_ID (the region)
 *       YYYY... is n2a(id) for a positive id
 *       ZZZZ... is n2a(-id) for a negative id
 *   The region (XXX) can be omitted in SMS identifiers (eg "200 to .ABCDE for pie")
 *   quids are (at least) 7 characters (3, 1, 3)
 *   qxids are (at least) 8 characters (3, 1, 4)
 *
 * @todo: pass ruids and record that info in transactions as agent (done, but test it!!)
 * @todo: throughout, do something better than die()
 * @todo: rebates and bonuses only when the two parties are in the same community (rewards for buy local)
 * @todo: separate smart api to its own module
 * @todo
 * - handling employees (virtual payments). This should probably be extended to suppliers too,
 *   or maybe even to everyone. The idea is to pay someone virtually in rCredits by totally paying them
 *   (either first or second) in US Dollars and (either second or first) having them buy back
 *   the appropriate amount of rCredits. Probably the best way to do this is with a simple payment transaction
 *   that requires confirmation by the payee (an "offer"). The payee cannot accept the offer without first paying
 *   that amount to the Community in US Dollars (using billpay or Dwolla).
 * @todo: run acceptance tests without drupal's interface (use u\expect())
 * @todo: don't require email address for SMS signups, set name properly
 * @todo: add community field to users (uid of user's CGC)
 * @todo: store country code rather than country, and state code rather than state
 * @todo: store city code (rather than city), with a code for "other" -- store other in data or r_other (same for country/state)
 * @todo: tx data changes(date, agent, field, oldvalue) (this gives a complete history of the transaction)
 *     (prefixes added to the payer_for or payee_for fields when changing state will not be included in changes)
 * @todo: reporting for a community or server (a form that anyone can use)
 * @todo: download transactions in standard formats
 * @todo: Finish separating backend. Front end should have no registered users. This should wait until after launch.
 * @todo: use popups for some things
 * @todo: use user_load or user_load_by_name or perhaps entity_load($entity_type, $ids = FALSE, $conditions = array(), $reset = FALSE) 
 * consistently, to take advantage of caching (but test, to make sure it works for non-$user accounts)
 * @todo
 * - NO a way to track US Dollars payments (probably a separate transaction table us_transactions)
 * - remove all direct db operations from interface modules (and their .inc files) -- use api\ and u\ there, but not r\
 *
 * - The data field in r_txs should also hold additional identification information about a nonlocal payer or payee.
 *     (exopayer or exopayee)
 *   Other servers will have a single UID on our system, so a
 *   user doing business with someone in another region would generate a transaction between an individual and a server,
 *   with qid for the individual in that other region stored in the data field. 
 *
 *   Specifically: when we in the AAAA community receive a transaction request using a qid, the first four characters are the "community" --
 *   they tell us what database to hit. For example, if AAAA.AAAB pays ZZZZ.AAAC, we record a payment from AAAA.AAAB's uid (1)
 *   to ZZZZ's uid. The data field in the transaction gets $data['external_payee'] = ZZZZ.AAAC's uid on the ZZZZ server (2).
 *   We send the request to ZZZZ's server and expect a confirmation 
 *   (if no confirmation, roll back the transaction and give an error message OR queue the request).
 *
 *   On the other side of things, when we receive a request from some other community for a transaction with one of our accounts
 *   -- for example if we are the "ZZZ" community, receiving a request from the "ABC" community, for ABC.AAAAA to pay ZZZ.BBBBB --
 *   we record the payer as ABC's uid and the payee as ZZZ.BBBBB's uid, with $data['payer_id'] = 'ABC.AAAAA'.
 *
 *   Thus every inter-community transaction is stored in both communities. Both communities know their trade balance, which should be
 *   public through a RESTful query. Intra-community transactions should also be stored twice -- ideally in member's devices. Smartphones and
 *   desktop computers running an app should store all their own transactions. Encrypted backups of everything to the cloud too.
 */
 
namespace rCredits; // typically abbreviated as "r"
use \rCredits\Utilities as u;
use \rCredits\API as api;

include_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server
include_once __DIR__ . '/rcredits-circles.inc'; // treat this as an extension of this file, for testing

define('R_SEPARATE_BACKEND', FALSE);
define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_ASIF_TEXT', ' "AS-IF"'); // how to identify "As If" transactions

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', '/email-frame.html');

// Roles
define('R_ROLE_RTRADER', 4);

// Bank account states
define('R_BANK_VERIFIED', 1); // we have proof that the user owns the bank account

// Account types
define('R_PERSONAL', 0);

$GLOBALS['account types'] = array(t('personal'), t('commercial'), t('non-profit'));
$GLOBALS['share permissions'] = array('no access', 'read transactions', 'sell', 'buy and sell', 'manage account');

// Transaction states
define('TX_STATES', 'deleted done pending disputed denied');

define('TX_DELETED', 0);
define('TX_DONE', 1); // here and up, the transaction counts
define('TX_PENDING', 2); // charged, waiting for confirmation
define('TX_DISPUTED', 3); // unilaterally charged, but disputed by the payer (the charge stands, until settled)
define('TX_DENIED', 4); // invoice rejected by payer

// Transaction channels
define('TX_WEB', 1);
define('TX_SMS', 2); 
define('TX_SMART', 3); 
define('TX_TOUCH', 4); // touch tone phone

// Transaction types
define('TX_SIGNUP', 0); // signup incentive reward
define('TX_TRANSFER', 1); // normal fund transfer (usually for actual goods and services)
define('TX_INFLATION', 2); // reward to conpensate for inflation
define('TX_REBATE', 3);
define('TX_BONUS', 4);

define('TX_COUNTABLE', 2); // higher numbers don't show on list, so they don't get transaction numbers (tid's)

$GLOBALS['email_subjects'] = array(
  'new-invoice' => t('rCredits INVOICE from {PAYEE_NAME}'),
  'invoice-denied' => t('rCredits Invoice DENIED by {PAYER_NAME}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {PAYER_NAME}'),
);

/**
 * Make Fields
 *
 * Make (or unmake) database fields, according to the schema
 * @param boolean $unmake: whether to unmake, rather than make
 * @param array $extras: associative array of table_name (only for unmaking)
 */
function make_fields($unmake = FALSE, $extras = array()) {
  $schema = array();
  \rcredits_schema_alter($schema); // get the fields to add (or drop)
  foreach ($schema as $table_name => $one) {
    if ($unmake and isset($extras[$table_name])) $one += $extras[$table_name];
    foreach ($one['fields'] as $field_name => $spec) {
      if (\db_field_exists($table_name, $field_name)) {
        if ($unmake) \db_drop_field($table_name, $field_name);
        if (!$unmake) \db_change_field($table_name, $field_name, $field_name, $spec);
      } else {
        if (!$unmake) \db_add_field($table_name, $field_name, $spec);
      }
    }
  }
}

function setup_table($description, $fields, $primary_key, $foreign_keys = array(), $indexes = array()) {
  $result = compact('description', 'fields', 'indexes') + array('foreign keys' => $foreign_keys);
  return is_null($primary_key) ? $result : ($result + array('primary key' => (is_array($primary_key) ? $primary_key : array($primary_key))));
}

function foreign_key($local, $foreign = 'uid', $table = 'users') {
  $columns = array($local => $foreign);
  return array($local => compact('table', 'columns'));
}

function index($field) {return array($field => array($field));}

/**
 * Setup a schema field
 *
 * @param string $type: data_type and size, separated by a space (a numeric size is interpreted as a string's maximum length)
 * @param string of boolean $SUN: $serialize, $unsigned, $notnull (one binary digit for each)
 * @param string $precision: precision and scale, separated by a comma
 *
 * @return array: the field's schema (UNUSED?)
 */
function setup_field($description = '', $type = 'varchar 255', $default = '', $SUN = '000') {
  $size = '';
  $precision = ',';

  if (strpos($type, ' ')) list ($type, $size) = explode(' ', $type);
  if ($type == 'numeric') {
    $precision = $size;
    $size = '';
  } elseif (is_numeric($size)) {
    $length = $size;
    $size = '';
  }
  list ($serialize, $unsigned, $notnull) = str_split($SUN);
  list ($precision, $scale) = explode(',', $precision); // don't use u\ray here
  
  $scheme = compact(u\ray('description type size length precision default scale serialize unsigned'));
  foreach ($scheme as $key => $value) if ($value === '') unset($scheme[$key]);

  $scheme['not null'] = $notnull;
//  ['fields'][$field_name] = $scheme;
  return $scheme;
}

/**
 * Add a single entry to the permission array for hook_permission()
 */
function permission(&$permissions, $title, $description) {
  $permissions[strtolower($title)] = compact('title', 'description');
}

/**
 * Format and return args for reporting a transaction
 * Not all returned args are used for any particular type of transaction.
 */
function tx_report_args($taking, $amount, $shortfall, $reward, $other_uid, $zxid, $goods) {
  $type = $taking ? t('charged') : ($goods ? t('paid') : t('gave'));
  $amount = u\format_amount($amount);
  $short = u\format_amount($shortfall);
  $tofrom = $taking ? t('from') : t('to');
  $reward_type = $taking ? t('bonus') : t('rebate');
  $reward_amount = u\format_amount($reward);
  $other = user_field('full_name', $other_uid);

  $real = ($zxid > 0);
  $asif = $real ? '' : R_ASIF_TEXT;
  $balance = api\credit_info(compact('real'))->fancy['balance'];
  $tid = api\zxid2tid($zxid);
  
  $success = TRUE;
  
  return compact(u\ray('type amount tofrom reward_type reward_amount balance zxid tid other_uid other short success asif'));
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
  if (@$my_data['zxid'] < 0) $table = zxs(FALSE); // special case for undoing an asif, even though both parties are now rTraders
  foreach ($txs as $key => $zot) { // $zot is unused
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
  
  // set up fields common to both rebate and bonus
  $fields = u\ray('type amount payer payee payer_agent payee_agent for creator state');
  $payer = $payer_agent = $payee_agent = \variable_get('rcredits_community_uid');
  $state = $my_state; // match rewards state to main transaction
  $taking = FALSE;

  $type = TX_REBATE;
  $payee = $my_payer;
  $amount = $rewards['rebate'];
  $for = t('rebate');
  $txs[] = transaction_setup(compact($fields));

  $type = TX_BONUS;
  $payee = $my_payee;
  $amount = $rewards['bonus'];
  $for = t('bonus');
  $txs[] = transaction_setup(compact($fields));
}

/**
 * Create a new transaction and set up all the important fields.
 * @param array $info: an associative array of transaction info. The following elements are required:
 *   amount: how much to transfer
 *   payer: zuid of the payer and payer_agent
 *   payee: zuid of the payee and payee_agent
 *   taking (bool): did the payee initiate the transaction
 *   for: purpose of the transaction
 *   goods (bool): is the transfer in exchange for goods and services
 * @return the array, with some extra field information added
 */
function transaction_setup($info) {
  global $channel, $auid; // through what medium was the transaction requested (web, SMS, etc.).
  extract($info);

  if (!isset($state)) $state = TX_DONE;
  $created = REQUEST_TIME;
  $serial = NULL; // always set equal to xid of main transaction after saving
  $data = serialize(@$data ?: array());
  
  list ($payer_for, $payee_for) = is_array($for) ? $for : array($for, $for);
  @$goods += 0; // avoid NULL
  unset($info['for']);

  return array_merge($info, compact(u\ray('state taking created serial data payer_for payee_for channel goods')));
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 * @param integer $who: uid of the user
 * @param numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($who, $demand = 0) {
  if (!$demand) return user_field('demand', $who);
  $demand = min(R_MAX_DEMAND, $demand);
  update_user(compact('demand'), $who);
  return $demand;
}

/**
 * Return the next credit id for the given community.
 * @param string $ctty: 4-character community header
 * @param bool $business: return a credit id for a business? (as opposed to for an individual)
 */
/*
function next_credit_id($ctty, $business = FALSE) {
  $pattern = $business ? '_____' : '____';
  $next = db_lookup('MAX(credit_id)', 'users', "credit_id LIKE '$ctty$pattern'");
  $tail = $next ? u\a2n(substr($next, 4)) : -1;
  return $ctty . u\n2a($tail + 1, 4);
}
*/

/**
 * Notify the specified user, according to their preferences
 */
function notify($uid, $message_index, $info) {
  $account = \user_load($uid);
  extract((array) $account, EXTR_PREFIX_ALL, 'my');
  $standard_fields = array(
    'uid' => $uid,
    'full_name' => $my_full_name,
  );
  
  $subs = array();
  foreach ($info + $standard_fields as $key => $value) $subs['{' . strtoupper($key) . '}'] = $value;
  drupal_mail('rcredits', $message_index, $my_mail, user_preferred_language($account), $subs);
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
  global $cuid, $auid;
  u\expect(compact(u\ray('field_name zxid all_related')), 'name zid bool');
  
  list ($xid, $table) = zxid($zxid);
  $crit = "xid=:xid and $cuid in (payer, payee)"; // "$cuid in" is a security measure
  list ($old_value, $data) = array_values(db_lookup("$field_name,data", $table, $crit, compact(u\ray('xid'))));
  $data = unserialize($data);
  $data['changes'][] = array(time(), $auid, $field_name, $old_value);
  $data = serialize($data);
  $sql = "UPDATE $table SET $field_name=:new_value,data=:data WHERE $crit";
  if ($all_related) { // gotta do the auxiliary records separately so as not to affect their data field
    $sql .= ';' . str_replace(',data=:data', '', str_replace('xid=:xid', 'serial=:xid', $sql));
  }
  /*** Begin DBTX ***/
  $dbtx = \db_transaction();
  $result = db_q($sql, compact(u\ray('new_value data xid')));
  unset($dbtx);
  /*** End DBTX ***/
  return $result;
}

/**
 * Merge a temporary ("old") account into the current account (a permanent ("new") account).
 * @return 1 (for counting how many were merged)
 */
function merge_accounts($old_uid) {
  $new_uid = cuid();
  if (!api\is_temp_account($old_uid)) return; // don't merge non-temp account
  if (!api\access('manage account')) die("unauthorized merge_accounts: $new_uid into $old_uid");
//  if (!is_temp_name(user_field('name', $old_uid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = \user_load($new_uid);
  $old = \user_load($old_uid);
  $update = array();
  foreach (u\ray('phone fax website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  
  /*** Begin DBTX ***/
  $dbtx = \db_transaction();
  \user_save($new, $update);
  $sql = <<<EOF
    UPDATE r_asif SET payer=:new_uid WHERE payer=:old_uid
    UPDATE r_asif SET payee=:new_uid WHERE payee=:old_uid
    DELETE FROM r_asif a LEFT JOIN r_asif b ON b.serial=a.serial WHERE b.payer=b.payee
    DELETE FROM r_asif WHERE payer=payee
    UPDATE sms_user SET uid=:new_uid WHERE uid=:old_uid
    UPDATE r_user_industries SET uid=:new_uid WHERE uid=:old_uid
    DELETE FROM users WHERE uid=:old_uid
EOF;
  db_q($sql, compact('new_uid', 'old_uid'));
  unset($dbtx);
  /*** End DBTX ***/
  return 1;
//  db_q("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:old_uid", compact('old_uid'));
}

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
  if(!($ctty_uid = \variable_get('rcredits_community_uid'))) die("No community uid!");
  $sqls = array(
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE r_asif',
    'TRUNCATE TABLE sms_user',
    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
}

/**
 * Format old transaction info (for confirmation)
 */
function tx_args($info) {
  $cuid = cuid();
  extract($info, EXTR_PREFIX_ALL, 'old');

  $tofrom = $old_payer == $cuid ? t('to') : t('from');
  $other = user_field('full_name', $old_payer == $cuid ? $old_payee : $old_payer);
  $amount = u\format_amount($old_amount);
  $date = strftime('%m/%d/%Y', $old_created);
  $real = ($old_zxid > 0);
  return compact(u\ray('tofrom other amount date real'));
}

/**
 * Return the real or ASIF transaction table name
 * @param int $uid1: uid of first member (DEFAULTs to cuid)
 *   OR boolean $uid1: TRUE if real
 * @param int $uid2: uid of second member (DEFAULTs to cuid)
 * @return: the real table name if the one user or both users (if specified) are rTraders, otherwise the asif table name
 */
function zxs($uid1 = 'cuid', $uid2 = '') {
  global $cuid;
  if ($uid1 == 'cuid') $uid1 = (int) $cuid;
  $real = is_bool($uid1) ? $uid1 : (is_rTrader($uid1) and is_rTrader($uid2));
  return $real ? 'r_txs' : 'r_asif';
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function db_lookup($field_name, $table_name, $criteria = '1', $subs = array()) {
  $sql = "SELECT $field_name FROM $table_name WHERE $criteria";
  if (!strpos($criteria, ' LIMIT ')) $criteria .= ' LIMIT 1';
  $result = db_q($sql, $subs);
  return strpos($field_name, ',') ? $result->fetchAssoc() : $result->fetchField();
}

function user_field($field_name, $who, $subs = array()) {
  return db_lookup($field_name, 'users', is_numeric($who) ? "uid=$who" : $who, $subs);
}

/**
 * Return a qid for the id or vice versa
 * @param mixed $id:
 *   positive integer: return XXX.YYYY...
 *   negative integer: return XXX:YYYY...
 *   XXX.YYYY...: return a positive integer
 *   XXX:YYYY...: return a negative integer (the colon can also be presented as "deux points" -- "..")
 *   (XXX is the 3-character server ID and defaults to the current server, the Ys are letters)
 * @param int $len: minimum length of the YYYY... part
 * @return FALSE if zero, empty, or wrong format -- otherwise as above
 */
function qid($id = '', $len = 0) {
  if (!@$id) return FALSE; // don't convert 0, empty, or NULL
  if (is_numeric($id)) return R_SERVER_ID . ($id < 0 ? (':' . u\n2a(-$id, -$len)) : ('.' . u\n2a($id, -$len))); 

  if (!u\is_qid($id)) return FALSE;
  $qid = str_replace('..', ':', $id); // allow a horizontal colon
  $type = strpos($qid, '.') === FALSE ? ':' : '.';
  list ($region, $iid) = explode($type, $qid, 2); // separate into region and individual id
  if ($region != R_SERVER_ID) return FALSE; // for now only one server
  if (strlen($iid) < $len) return FALSE;

  $id = u\a2n($iid);
  if ($type == '.') return $id;
  if ($type == ':') return -$id;
  return FALSE;
}

function cuid() {global $cuid; return @$cuid + 0;} // current account (may be zero, if not logged in)

function quid($id = '') {return qid($id ?: zuid(), 3);} // Convert zuid to quid or vice versa
function qxid($id) {return quid($id, FALSE, 4);} // Convert zxid to qxid or vice versa

/**
 * Convert a zuid to uid and agent uid or vice-versa
 * zuid() returns $czuid
 * zuid($zuid) returns and rid = array($uid, $agent_uid)
 * zuid($uid, $agent_uid) returns $zuid ($uid if $uid==$agent_uid, else -$reid)
 * @return as above; FALSE if no such
 */
function zuid($id = '', $agent_uid = '') {
  global $czuid;
  if (!$id) return $czuid;
  if (!$agent_uid) return ($reid = -$id) > 0 ? array_values(db_lookup('main_uid,agent_uid', 'r_relations', 'reid=:reid', compact('reid'))) : array($id, $id);
  return $id == $agent_uid ? $id : -db_lookup('reid', 'r_relations', 'main_uid=:id AND agent_uid=:agent_uid', compact(u\ray('id agent_uid')));
}

/**
 * Convert a zxid to xid and table or vice-versa
 */
function zxid($id, $table = '') {
  if ($table) return $table == zxs(TRUE) ? $id : -$id;
  if (!$table) return $id > 0 ? array($id, zxs(TRUE)) : array(-$id, zxs(FALSE));
}

/**
 * Say whether one of the accounts owns the other.
 */
function owning($uid1, $uid2) {
  $crit = 'is_owner AND ((main_uid=:uid1 AND agent_uid=:uid2) OR (main_uid=:uid2 AND agent_uid=:uid1))';
  return $uid1 == $uid2 ?: db_lookup(1, 'r_relations', $crit, compact('uid1', 'uid2'));
}

function is_rTrader($uid = '') {
  if (!$uid) $uid = cuid();
  return db_lookup(1, 'users_roles', 'uid=:uid AND rid=' . R_ROLE_RTRADER, compact('uid'));
}

/**
 * @todo: maybe move the processing of zxs to a wrapper function db_txq() that also does postprocessing on fetchAssoc()
 */
function db_q($sql, $args = array()) {
  if (strpos($sql, '{zxs}')) {
    extract($args, EXTR_PREFIX_ALL, 'my');
    if (@$my_zxid) {
      list ($args['xid'], $table) = zxid($my_zxid);
      unset($args['zxid']);
    } else {
      $table = zxs((bool) (isset($my_real) ? $my_real : is_rTrader()));
      unset($args['real']);
    }
    $sql = strtr($sql, array('{zxs}' => $table, 'zxid' => 'xid'));
  }
  return \db_query(u\get_constants($sql), u\prefix_keys(':', $args));
}

// Single line functions that need no explanation
function rebate($who, $amount = 1) {return round($amount * db_lookup('rebate_percent', 'users', 'uid=:who', compact('who')) / 100, 2);}
function bonus($who, $amount = 1) {return rebate($who, 2 * $amount);}
function update_user($info, $who) {return \db_update('users')->fields($info)->condition('uid', $who)->execute();}
// UNUSED tx_field($field_name, $xid, $real) {return db_lookup($field_name, zxs((bool) $real), 'xid=:xid', compact('xid'));}
function set_tx_state($new_value, $zserial) {set_tx_field('state', $new_value, $zserial, 'serial');}
function uid_from_crid($qid) {return db_lookup('uid', 'users', 'qid=:qid', compact('qid'));}
function total_demand() {return db_lookup('SUM(demand)', 'users');}

