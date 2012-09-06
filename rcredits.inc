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
 * @todo
 * - handling employees (virtual payments). This should probably be extended to suppliers too,
 *   or maybe even to everyone. The idea is to pay someone virtually in rCredits by totally paying them
 *   (either first or second) in US Dollars and (either second or first) having them buy back
 *   the appropriate amount of rCredits. Probably the best way to do this is with a simple payment transaction
 *   that requires confirmation by the payee (an "offer"). The payee cannot accept the offer without first paying
 *   that amount to the Community in US Dollars (using billpay or Dwolla).
 * - a way to track US Dollars payments (probably a separate transaction table us_transactions)
 * - remove all direct db operations from interface modules (and their .inc files) -- use api\ and u\ there, but not r\
 *
 * - The data field in r_txs should also hold additional identification information about a nonlocal payer or payee.
 *   If the mcapi intertrading module does not work for us, other servers could have a single UID on our system, so a
 *   user doing business with someone in another region would generate a transaction between an individual and a server,
 *   with credit_id for the individual in that other region stored in the data field. 
 *   We do NOT need to store credit_id for creator, payer, and payee.
 *
 *   Specifically: when we in the ZZZZ community receive a transaction request using a credit_id, the first four characters are the "community" --
 *   they tell us what database to hit. For example, if ZZZZYYYY pays AAAABBBB, we record a payment from ZZZZYYYY's uid to the uid of a user record *   with credit_id = "AAAA". The data field in the transaction gets $data['payee_id'] = 'AAAABBBB'. We send the request to AAAA's db and
 *   expect a confirmation (if no confirmation, roll back the transaction and give an error message OR queue the request).
 *
 *   On the other side of things, when we receive a request from some other community for a transaction with one of our accounts
 *   -- for example if we are the "ZZZZ" community, receiving a request from the "ABCD" community, for ABCDAAAA to pay ZZZZBBBB --
 *   we record the payer as ABCD's uid and the payee as ZZZZBBBB's uid, with $data['payer_id'] = 'ABCDAAAA'.
 *
 *   Thus every inter-community transaction is stored in both communities. Both communities know their trade balance, which should be
 *   public through a RESTful query. Intra-community transactions should also be stored twice -- ideally in member's devices. Smartphones and
 *   desktop computers running an app should store all their own transactions. Encrypted backups of everything to the cloud too.
 *
 * @todo: use user_load or user_load_by_name or perhaps entity_load($entity_type, $ids = FALSE, $conditions = array(), $reset = FALSE) 
 * consistently, to take advantage of caching (but test, to make sure it works for non-$user accounts)
 *
 * @todo: use name field for credit_id.
 */
 
namespace rCredits; // typically abbreviated as "r"
use \rCredits\Utilities as u;
use \rCredits\API as api;

include_once __DIR__ . '/rcredits-circles.inc'; // treat this as an extension of this file, for testing

define('RCREDITS_SEPARATE_BACKEND', FALSE);
define('RCREDITS_COMMUNITY_NAME', 'Community');
define('RCREDITS_COMMUNITY_ID', 'community'); // credit_id for the community account
define('RCREDITS_COMMUNITY_POSTAL_AREA', '01%'); // anyone with a postal code that matches this is in
define('RCREDITS_INITIAL_DEMAND', 25000); // total escrowed rCredits during TRIAL RUN
//define('RCREDITS_CURRENCY_CODE', 'r'); // Pretend rCredits, pre-launch
define('RCREDITS_REBATE', 0.05);
define('RCREDITS_BONUS', 0.10);
define('RCREDITS_MAX_DEMAND', 1000000); // you can't ask for more rCredits than this
define('RCREDITS_MIN_DEMAND', 100); // you can't buy less rCredits than this at a time
define('RCREDITS_STARTING_BALANCE', 250); // how much people start with in the TRIAL RUN
define('RCREDITS_MIN_BALANCE', -10000); // how low can user's balance go, by default (be generous in TRIAL RUN)
define('RCREDITS_TEMP_NAME_PREFIX', 'unknown ');

define('RCREDITS_ROLE_RTRADER', 4);

// Account types
define('RCREDITS_PERSONAL', 0);

$GLOBALS['account types'] = array(t('personal'), t('commercial'), t('non-profit'));
$GLOBALS['share permissions'] = array('no access', 'can read transactions', 'can create transactions', 'can manage account');

// Transaction states (greater than 0 means it counts)
define('TX_DENIED', -2); // invoice rejected by payer
define('TX_DELETED', -1);
define('TX_PENDING', 0); // charged, waiting for confirmation
define('TX_DONE', 1); // here and up, the transaction counts
define('TX_CONTESTED', 2); // unilaterally charged, but contested by the payer (the charge stands, until settled)

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
function tx_report_args($taking, $amount, $shortfall, $reward, $my_uid, $other_uid, $xid, $for_cash) {
  $type = $for_cash ? 'traded' : ($taking ? t('charged') : t('paid'));
  $amount = u\format_amount($amount);
  $short = u\format_amount($shortfall);
  $tofrom = $taking ? t('from') : t('to');
  $reward_type = $taking ? t('bonus') : t('rebate');
  $reward_amount = u\format_amount($reward);
  $other = user_field('full_name', $other_uid);

  $balance = api\credit_info($my_uid)->fancy['balance'];
  $tid = api\tid_from_xid($my_uid, $xid);

  $way = $taking ? 'cash for rCredits' : 'rCredits for cash'; // use as both text and message index (might wanna separate out way_index)
  $success = TRUE;
  
  return compact(u\ray('type amount tofrom reward_type reward_amount balance xid tid other short way success'));
}

/**
 * Record a transaction between two participants
 * @see also r\transaction_rewards() and r\transfer()
 */
function transact($info) {
  $txs = array(transaction_setup($info));
  transaction_rewards($txs);
  return tx_save($txs);
}

/**
 * Save the transaction set
 */
function tx_save($txs) {
  //*** Begin DBTX
  $dbtx = db_transaction();
  foreach ($txs as $key => $zot) { // $zot is unused
    $txs[$key]['serial'] = @$serial; // set serial for dependent transactions
//    $txs[$key]['type'] = tx_type($txs[$key]['type']); // translate from string to numeric at the last moment
    \drupal_write_record('r_txs', $txs[$key]); // pass actual array element, to get xids
    if ($key == 0) {
      $txs[$key]['serial'] = $serial = $txs[0]['xid']; // set serial for main transaction
      db_q('UPDATE r_txs SET serial=:serial WHERE xid=:serial', compact('serial'));
    }
  }

  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  return $txs;
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 */
function transaction_rewards(&$txs) {
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  
//  if (($my_type != 'payment') and ($my_type != 'charge')) return;
  if ($my_type != TX_TRANSFER) return;
  if ($my_payer_for == 'cash') return; // quit unless rebates and bonuses apply
  $rewards = unserialize($my_data);
  
  $fields = u\ray('type amount payer payee for creator state channel');
  $payer = $creator = \variable_get('rcredits_community_uid');
//  $payer_id = $creator_id = RCREDITS_COMMUNITY_ID;
  $state = $my_state; // match rewards state to main transaction

  $type = TX_REBATE;
  $payee = $my_payer;
//  $payee_id = $my_payer_id;
  $amount = $rewards['rebate'];
  $for = t('rebate');
  $txs[] = transaction_setup(compact($fields));

  $type = TX_BONUS;
  $payee = $my_payee;
//  $payee_id = $my_payee_id;
  $amount = $rewards['bonus'];
  $for = t('bonus');
  $txs[] = transaction_setup(compact($fields));
}

/**
 * Create a new transaction and set up all the important fields.
 * @todo: set real to unreal if either party is a non-rTrader (never change the value of real if it's passed, because it is an undo)
 * @todo: don't give real rewards to the passive party in an ASIF transaction (look retroactively at creator)
 * (and no reward transaction if they are an rTrader)
 */
function transaction_setup($info) {
  global $cid;
  extract($info);

  if (!isset($state)) $state = TX_DONE; // don't use @$my_state here (might be zero)
  if (!@$my_creator) $creator = $cid;
  $created = REQUEST_TIME;
  $serial = NULL; // always set equal to xid of main transaction after saving
  $data = serialize(@$data ?: array());
  if (!isset($real)) $real = is_rTrader($payer) + 2 * is_rTrader($payee); // mark real versus AS IF transaction

  
  list ($payer_for, $payee_for) = is_array($for) ? $for : array($for, $for);
  unset($info['for']);

  return array_merge($info, compact(u\ray('state creator created serial data payer_for payee_for channel')));
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 *
 * @param integer $who: uid of the user
 * @param numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($who, $demand = 0) {
  if (!$demand) return user_field('demand', $who);
  $demand = min(RCREDITS_MAX_DEMAND, $demand);
  update_user(compact('demand'), $who);
  return $demand;
}

/**
 * Return the next credit id for the given community.
 * @param string $ctty: 4-character community header
 * @param bool $business: return a credit id for a business? (as opposed to for an individual)
 */
function next_credit_id($ctty, $business = FALSE) {
  $pattern = $business ? '_____' : '____';
  $next = db_lookup('MAX(credit_id)', 'users', "credit_id LIKE '$ctty$pattern'");
  $tail = $next ? u\a2n(substr($next, 4)) : -1;
  return $ctty . u\n2a($tail + 1, 4);
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
  if ($phone) return RCREDITS_TEMP_NAME_PREFIX . u\format_phone($phone, 'n');
  if (isset($my_email)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return RCREDITS_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

function set_tx_state($new_value, $serial) {
  return db_q("UPDATE r_txs SET state=:new_value WHERE serial=:serial", compact(u\ray('new_value serial')));
}

function set_tx_field($field_name, $new_value, $xid) {
  return db_q("UPDATE r_txs SET $field_name=:new_value WHERE xid=:xid", compact(u\ray('new_value xid')));
}
//return \db_update('r_txs')->fields(array($field_name => $new_value))->condition('xid', $xid);

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
  if(!($ctty_uid = \variable_get('rcredits_community_uid'))) die("No community uid!");
  $sqls = array(
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE sms_user',
    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function db_lookup($field_name, $table_name, $criteria = '1', $subs = array()) {
  $sql = "SELECT $field_name FROM $table_name WHERE $criteria";
  if (!strpos($criteria, ' LIMIT ')) $criteria .= ' LIMIT 1';
  return db_q($sql, $subs)->fetchField();
}

function user_field($field_name, $who, $subs = array()) {
  return db_lookup($field_name, 'users', is_numeric($who) ? "uid=$who" : $who, $subs);
}

// Single line functions that need no explanation
function db_q($sql, $args = array()) {return \db_query($sql, u\prefix_keys(':', $args));}
function rebate($who, $amount = 1) {return round($amount * db_lookup('rebate_percent', 'users', 'uid=:who', compact('who')) / 100, 2);}
function bonus($who, $amount = 1) {return rebate($who, 2 * $amount);}
function update_user($info, $who) {return \db_update('users')->fields($info)->condition('uid', $who)->execute();}
function tx_field($field_name, $xid) {return db_lookup($field_name, 'r_txs', 'xid=:xid', compact('xid'));}
function uid_from_crid($credit_id) {return db_lookup('uid', 'users', 'credit_id=:credit_id', compact('credit_id'));}
function total_demand() {return db_lookup('SUM(demand)', 'users');}
function is_rTrader($uid) {return db_lookup(1, 'users_roles', 'uid=:uid AND rid=' . RCREDITS_ROLE_RTRADER, compact('uid'));}
