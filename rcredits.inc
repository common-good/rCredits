<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT the Drupalish callbacks, which are in rcredits.module
 *
 * @todo
 * - the undo feature
 * - handling invoices (required confirmation of charges)
 * - handling employees (virtual payments). This should probably be extended to suppliers too,
 *   or maybe even to everyone. The idea is to pay someone virtually in rCredits by totally paying them
 *   (either first or second) in US Dollars and (either second or first) having them buy back
 *   the appropriate amount of rCredits. Probably the best way to do this is with a simple payment transaction
 *   that requires confirmation by the payee (an "offer"). The payee cannot accept the offer without first paying
 *   that amount to the Community in US Dollars (using billpay or Dwolla).
 * - a way to track US Dollars payments (probably a separate transaction table us_transactions)
 */
 
namespace rCredits;

//define('TRANSACTION_STATE_STRAW', -3); // add this to states defined in mcapi.module
define('RCREDITS_COMMUNITY_NAME', 'Community');
define('RCREDITS_COMMUNITY_ID', 'community');
define('RCREDITS_INITIAL_DEMAND', 25000); // total escrowed rCredits during TRIAL RUN
define('RCREDITS_CURRENCY_CODE', 'r'); // Pretend rCredits, pre-launch
define('RCREDITS_REBATE', 0.05);
define('RCREDITS_BONUS', 0.10);
define('RCREDITS_COMMUNITY_POSTAL_AREA', '01%'); // anyone with a postal code that matches this is in
define('RCREDITS_MAX_DEMAND', 1000000000);
define('RCREDITS_STARTING_BALANCE', 250); // how much people start with in the TRIAL RUN
define('RCREDITS_MIN_BALANCE', -10000); // how low can user's balance go, by default (be generous in TRIAL RUN)
define('RCREDITS_TEMP_NAME_PREFIX', 'unknown ');
define('TRANSACTION_STATE_PENDING', 3); // charged, waiting for confirmation

/**
 * Show a debbugging statement (without a lot of typing)
 */
function deb($message) {
  drupal_set_message(print_r($message, TRUE), 'error');
}

/**
 * Make Fields
 *
 * Make (or unmake) database fields, according to the schema
 * @parm boolean $unmake: whether to unmake, rather than make
 * @parm array $extras: associative array of table_name (only for unmaking)
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

/**
 * Setup a schema field
 *
 * @parm string $type: data_type and size, separated by a space (a numeric size is interpreted as a string's maximum length)
 * @parm string of boolean $SUN: $serialize, $unsigned, $notnull (one binary digit for each)
 * @parm string $precision: precision and scale, separated by a comma
 *
 * @return array: the field's schema (UNUSED?)
 */
function setup_field(&$schema, $table_name, $label, $description = '', $type = 'varchar 255', $default = '', $SUN = '000') {
  $size = '';
  $precision = ',';

  list ($label, $field_name) = is_array($label) ? $label : array($label, str_replace(' ', '_', strtolower($label)));
  if (strpos($type, ' ')) list ($type, $size) = ray($type);
  if ($type == 'numeric') {
    $precision = $size;
    $size = '';
  } elseif (is_numeric($size)) {
    $length = $size;
    $size = '';
  }
  list ($serialize, $unsigned, $notnull) = str_split($SUN);
  list ($precision, $scale) = explode(',', $precision);
  
  $scheme = compact(ray('description type size length precision default scale serialize unsigned'));
  foreach ($scheme as $key => $value) {
    if ($value === '') unset($scheme[$key]);
  }
  $scheme['not null'] = $notnull;
  $schema[$table_name]['fields'][$field_name] = $scheme;
  return $scheme;
}

/**
 * Initialize the rCredits currency information
 *
 * @todo: get a standard schema from mcapi (ask Matthew)
 *   (mcapi_currency.inc is merely a stand-in)
 */
function make_currency() {
  require __DIR__ . '/mcapi_currency.inc';
  $currcode = RCREDITS_CURRENCY_CODE;
  if (db_lookup(1, 'mcapi_currencies', "currcode = '$currcode'")) return;
  $cur = mcapi\currency();
  $info = compact('currcode');
  $human_name = 'rCredits';
  $display = array('format' => '$@minus@integer.@subdivision', 'divisions' => 1);
  $zero = 1;
  $uid = 1;
  $new = compact(ray('info human_name display zero uid'));
  $row = array(
    'currcode' => $currcode,
    'data' => (object) array_merge($cur, $new)
  );
  \drupal_write_record('mcapi_currencies', $row);
}

/**
 * Add a single entry to the permission array for hook_permission()
 */
function permission(&$permissions, $title, $description) {
  $permissions[strtolower($title)] = compact('title', 'description');
}

/**
 * Create a user account for transactions and/or contact
 *
 * @parm array $info associative array of fields and values
 *   This function assumes that the info is not sufficient to identify an existing user
 * @return the user account object
 */
function create_user($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $info['mail'] = $info['init'] = isset($my_email) ? $my_email : '';
  $info['pass'] = \user_password();
  $info['name'] = isset($my_name) ? $my_name : temp_name($info);
  $info['status'] = TRUE;
  $info['is_new'] = TRUE;
  
  if (isset($my_number)) {
    $number = format_phone($my_number, '+n');
    $status = SMS_USER_CONFIRMED; // ws: for now, with everything through SMS, we want to be able to send to anyone
    $info['sms_user'] = compact('number', 'status');
  }
  $account = \user_save(array(), $info);
  if (!$account) throw new \Exception('failed to save user record');
  return $account;
}

/**
 * Create a transaction
 * @parm bool $taking: true if the money is going TO the initiating user
 * @parm int $my_uid: user id of the initiating user
 * @parm int $other_uid: user id of the other party
 * @parm numeric $requested_amount: the transaction amount
 * @parm string $for: description of the transaction's purpose (user input)
 * @parm bool $confirmed: has this transaction been confirmed by the user? (default TRUE, used only for SMS interface)
 * @parm array $rewards: associative array of reward percentages, indexed by 'rebate' and 'bonus' (default empty means standard rewards)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 */
function transfer($taking, $my_uid, $other_uid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  $type = $confirm_type = $taking ? 'charge' : 'payment';
  $approved = $taking ? TRUE : access('charge others', $my_uid);
  if ($for_cash = for_cash($for)) $for = 'cash';
  $other = user_field('name', $other_uid);
  $available = credit_info($taking ? $other_uid : $my_uid)->available;
  $amount = min($requested_amount, $available);
  $shortfall = $requested_amount - $amount; // how much needs to be done in a separate currency
  $short = format_amount($shortfall);
  if ($shortfall > 0 and $approved) {
    if ($for_cash) return array($taking ? 'short cash from' : 'short cash to', compact('short', 'other'), FALSE); // no partial cash trans
    if (ceil($shortfall) >= $requested_amount) return array($taking ? 'broke from' : 'broke to', compact('other'), FALSE);
    $shortfall = ceil($shortfall); // for goods and services, shortfall should be a round figure (no cents)
    $amount = $requested_amount - $shortfall;
    $short = format_amount($shortfall);
  }
  
  list ($payer, $payee) = $taking ? array($other_uid, $my_uid) : array($my_uid, $other_uid);
  list ($rebate, $bonus) = $for_cash ? array(0, 0) : array(rebate($payer), bonus($payee));
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;
  $state = $approved ? TRANSACTION_STATE_FINISHED : TRANSACTION_STATE_PENDING;

  $dbtx = db_transaction();
  try {
    $txs = transact(compact(ray('type amount payer payee for data state')));
  }
  catch (\Exception $e) {return array($e->getMessage(), array(), FALSE);}
  
  $totals = \transaction_totals($my_uid);
  $tid = tid_from_xid($my_uid, $txs[0]->xid);
  $reward_index = $taking ? 2 : 1;
  $reward = isset($txs[$reward_index]) ? $txs[$reward_index]->worth['und'][0]['quantity'] : 0;
  
  $type = $taking ? t('charged') : t('paid');
  $amount = format_amount($amount);
  $tofrom = $taking ? t('from') : t('to');
  $reward_type = $taking ? t('bonus') : t('rebate');
  $reward_amount = format_amount($reward);
  $balance = format_amount($totals->balance);
  $args = compact(ray('type amount tofrom reward_type reward_amount balance tid other short'));
  
  if (!$confirmed) $dbtx->rollback();

  if (!$for_cash) {
    if ($confirmed) return array($shortfall > 0 ? 'report short transaction' : 'report transaction', $args, FALSE);
    if ($shortfall > 0) $args['amount'] = format_amount($requested_amount); // restore original amount before getting confirmation
    return array($confirm_type, $args, TRUE);
  }
  
  $args['type'] = 'traded';
  $args['way'] = $taking ? t('cash for credit') : t('credit for cash');
  return array($confirmed ? 'report exchange' : $args['way'], $args, !$confirmed);
}

/**
 * Record a transaction between two participants
 * @see also r\transaction_rewards() and r\transfer()
 */
function transact($info) {
//  foreach (array('creator', 'payer', 'payee') as $one) $info[$one. '_id'] = user_field('credit_id', $info[$one]);
  $tx = transaction_setup($info);

  try {
    return \transaction_insert_new($tx, TRUE);
  } catch (Exception $e) {throw $e;}
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 * @todo: Note that the data field might also hold additional identification information about a nonlocal payer or payee.
 *   If the mcapi intertrading module does not work for us, other servers could have a single UID on our system, so a
 *   user doing business with someone in another region would generate a transaction between an individual and a server,
 *   with credit_id for the individual in that other region stored in the data field. 
 *   We do NOT need creator_id, payer_id, and payee_id (right?).
 */
function transaction_rewards(&$txs) {
  extract((array) $txs[0], EXTR_PREFIX_ALL, 'my');
  
  if (($my_type != 'payment') and ($my_type != 'charge')) return;
  if ($my_payer_for == 'cash') return; // quit unless rebates and bonuses apply
  
  $fields = ray('type amount payer payee for creator');
  $payer = $creator = variable_get('rcredits_community_uid');
//  $payer_id = $creator_id = RCREDITS_COMMUNITY_ID;

  $type = 'rebate';
  $payee = $my_payer;
//  $payee_id = $my_payer_id;
  $amount = $my_amount * $my_data[$type]; // was $my_worth['und'][0]['quantity'] *
  $for = t('rebate');
  $txs[] = transaction_setup(compact($fields));

  $type = 'bonus';
  $payee = $my_payee;
//  $payee_id = $my_payee_id;
  $amount = $my_amount * $my_data[$type];
  $for = t('bonus');
  $txs[] = transaction_setup(compact($fields));
}

/**
 * Create a new transaction and set up all the important fields.
 */
function transaction_setup($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');

  $tx = new_transaction();
  unset($tx->xid);
  $tx->payer = $my_payer;
  $tx->payee = $my_payee;
  $tx->amount = $my_amount; // possible (simple) replacement for worth array
  $tx->worth['und'][0]['currcode'] = RCREDITS_CURRENCY_CODE;
//  $tx->worth['und'][0]['currcode'] = isset($values['currency']) ? $values['currency'] : key(currencies_load());
  $tx->worth['und'][0]['quantity'] = $my_amount;
  
  $tx->payer_for = $my_for;
  $tx->payee_for = $my_for;
  $tx->state = isset($my_state) ? $my_state : \TRANSACTION_STATE_FINISHED;
  $tx->type = $my_type;
  if (isset($my_data)) $tx->extra = $my_data;
  if (isset($my_creator)) $tx->creator = $my_creator; // defaults to current user
  return $tx;
}

/**
 * Record an incentive transaction for signing up
 * @parm int who: uid of the person signing up
 * @return int: the incentive amount
 */
function give_signup_incentive($who) {
  $type = 'signup';
  $amount = RCREDITS_STARTING_BALANCE;
  $payer = $creator = variable_get('rcredits_community_uid');;
  $payee = $who;
  $for = t('signup');
  transact(compact(ray('type amount payer payee for creator')));
  return $amount;
}

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * @param boolean $simplify: Omit the cents for whole dollar amounts
 *
 * @return
 *   the formatted amount
 */
function format_amount($amount, $simplify = TRUE) {
  $amount = '$' . number_format($amount, 2);
  return $simplify ? str_replace('.00', '', $amount) : $amount;
}

/** 
 * Log the message and return it.
 * This makes it easy for tests to see what was done
 *
 * @param string $type
 *   type of log entry. If null (the default), don't log anything
 *
 * @return the (possibly updated) log array
 */  
function r_log($type='', $message='', $number='', $nonce='') {
  $log_filename = 'rcredits.log';
//    $log = file_exists($log_filename) ? file_get_contents($log_filename) : '';
//    file_put_contents($log_filename, $log . "$type, $number, $message\n");
  $log = file_exists($log_filename) ? unserialize(file_get_contents($log_filename)) : '';
  if ($type) {
    $log[] = compact('type', 'message', 'number');
    if ($nonce) {
      $message = str_replace($nonce, '@nonce', $message); // also store without the nonce for testing, because the nonce is unpredictable
      $log[] = compact('type', 'message', 'number');
    }
    file_put_contents($log_filename, serialize($log) . "\n");
  }
  return $log;
}  

/**
 * Output the log, human-readably
 */
function showlog() {
  print_r(r_log());
}

/**
 * Create a transaction
 * @parm bool $tome: true if the money is going TO the initiating user
 * @parm int $my_uid: user id of the initiating user
 * @parm int $other_uid: user id of the other party
 * @parm numeric $requested_amount: the transaction amount
 * @parm string $for: description of the transaction's purpose (user input)
 * @parm bool $confirmed: has this transaction been confirmed by the user? (default TRUE, used only for SMS interface)
 * @parm array $rewards: associative array of reward percentages, indexed by 'rebate' and 'bonus' (default empty means standard rewards)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 */
function transfer($tome, $my_uid, $other_uid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  $type = $confirm_type = $tome ? 'charge' : 'payment';
  if ($for_cash = for_cash($for)) $for = 'cash';
  $other = user_field('name', $other_uid);
  $available = credit_info($tome ? $other_uid : $my_uid)->available;
  $amount = min($requested_amount, $available);
  $shortfall = $requested_amount - $amount; // how much needs to be done in a separate currency
  $short = format_amount($shortfall);
  if ($shortfall > 0) {
    if ($for_cash) return array($tome ? 'short cash from' : 'short cash to', compact('short', 'other'), FALSE); // no partial cash trans
    if (ceil($shortfall) >= $requested_amount) return array($tome ? 'broke from' : 'broke to', compact('other'), FALSE);
    $shortfall = ceil($shortfall); // for goods and services, shortfall should be a round figure (no cents)
    $amount = $requested_amount - $shortfall;
    $short = format_amount($shortfall);
  }
  
  list ($payer, $payee) = $tome ? array($other_uid, $my_uid) : array($my_uid, $other_uid);
  list ($rebate, $bonus) = $for_cash ? array(0, 0) : array(rebate($payer), bonus($payee));
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;

  $dbtx = db_transaction();
  try {
    $txs = transact(compact(explode(' ', 'type amount payer payee for data')));
  }
  catch (\Exception $e) {return array($e->getMessage(), array(), FALSE);}
  
  $totals = \transaction_totals($my_uid);
  $tid = tid_from_xid($my_uid, $txs[0]->xid);
  $reward_index = $tome ? 2 : 1;
  $reward = isset($txs[$reward_index]) ? $txs[$reward_index]->worth['und'][0]['quantity'] : 0;
  
  $type = $tome ? t('charged') : t('paid');
  $amount = format_amount($amount);
  $tofrom = $tome ? t('from') : t('to');
  $reward_type = $tome ? t('bonus') : t('rebate');
  $reward_amount = format_amount($reward);
  $balance = format_amount($totals->balance);
  $args = compact(explode(' ', 'type amount tofrom reward_type reward_amount balance tid other short'));
  
  if (!$confirmed) $dbtx->rollback();

  if (!$for_cash) {
    if ($confirmed) return array($shortfall > 0 ? 'report short transaction' : 'report transaction', $args, FALSE);
    if ($shortfall > 0) $args['amount'] = format_amount($requested_amount); // restore original amount before getting confirmation
    return array($confirm_type, $args, TRUE);
  }
  
  $args['type'] = 'traded';
  $args['way'] = $tome ? t('cash for credit') : t('credit for cash');
  return array($confirmed ? 'report exchange' : $args['way'], $args, !$confirmed);
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 *
 * @parm integer $who: uid of the user
 * @parm numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($who, $demand = 0) {
  if (!$demand) return db_lookup('demand', 'users', "uid = $who");
  $demand = min(RCREDITS_MAX_DEMAND, $demand);
//  \db_query("UPDATE users SET demand = $demand WHERE uid = $who");
  \db_update('users')->fields(compact('demand'))->condition('uid', $who, '=')->execute();
  return $demand;
}

/**
 * @param int $who
 *   uid of the credit account to return information for
 *
 * @return
 *   standard object of credit account information (see keys in the return line below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function credit_info($who) {
  $sql = "SELECT credit_id AS id, min_balance FROM users WHERE uid = $who";
  extract(db_query($sql)->fetchAssoc());
  $sql = "SELECT 
    SUM(IF(t.payer = $who, w.worth_quantity, 0)) AS gross_out, 
    SUM(IF(t.payee = $who, w.worth_quantity, 0)) AS gross_in,
    SUM(IF(t.type NOT IN('payment', 'charge'), w.worth_quantity, 0)) AS rewards
    FROM mcapi_transactions t LEFT JOIN field_data_worth w ON w.entity_id = t.xid
    WHERE bundle = 'transaction' AND (t.payer = $who OR t.payee = $who)";
  extract(db_query($sql)->fetchAssoc());
  $balance = $gross_in - $gross_out;
  $unavailable = 0; // for now
  $available = ($balance - $min_balance) - $unavailable;
  $cashable = $balance - $rewards;
  $total_demand = total_demand();
  $result = compact(ray('balance available unavailable rewards cashable total_demand min_balance id'));
  $fancy = array();
  foreach ($result as $key => $value) {
    $fancy[$key] = is_numeric($value) ? format_amount($value) : $value;
  }
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * Say whether the user name is valid.
 * @parm string $name: the name to test
 * @parm boolean $picky: whether to allow only 2-4 word names (default TRUE)
 */
function valid_name($name, $picky = TRUE) {
  $words = substr_count($name, ' ') + 1;
  if (preg_match("/[^\w\-\.,' ]/iu", $name)) return FALSE; // allows unicode chars and common punctuation
  if ($picky and ($words < 2 or $words > 4)) return FALSE;
  return TRUE;
}

/**
 * Create a unique username for the given account, based on the given name.
 */
function make_username($account, $name) {
  $original = $name;
  for ($locktry = 0; ($locktry < 99) and !\lock_acquire(__FUNCTION__); $locktry++) \lock_wait();

  for ($try = 1; $try < 99; $try++) {
    if (!db_lookup(1, 'users', "name = :name", compact('name'))) {
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
 * Create an appropriate credit id for the given account
 */
function make_credit_id($account) {
  if (isset($account->credit_id) and $account->credit_id) return;
  if ($phone = format_phone($account->phone, 'n')) {
    $area = substr($phone, 0, 3);
    $state = db_lookup('region', 'r_area_codes', "area_code='$area'");
    $region = db_lookup('credit_region', 'r_credit_regions', "region='$state'");
    if (!$region) $region = 'Z9';
  }
  $ctty = $region . 'AA'; // until the person becomes a member of a specific community, give them a generic community in their region

  for ($try = 0; $try < 99; $try++) {
    $credit_id = next_credit_id($ctty);
    \user_save($account, compact('credit_id'));
    if (!db_lookup('uid', 'users', "credit_id = '$credit_id' AND uid < " . $account->uid)) return;
  }
  die("Too many collisions in make_credit_id() -- ctty = $ctty");
}

/**
 * Return the next credit id for the given community.
 * @parm string $ctty: 4-character community header
 * @parm bool $business: return a credit id for a business? (as opposed to for an individual)
 */
function next_credit_id($ctty, $business = FALSE) {
  $pattern = $business ? '_____' : '____';
  $next = db_lookup('MAX(credit_id)', 'users', "credit_id LIKE '$ctty$pattern'");
  $tail = $next ? a2n(substr($next, 4)) : -1;
  return $ctty . n2a($tail + 1, 4);
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function for_cash($for) {
  return ($for == '' or preg_match('/cash( |$)/i', $for));
}

/**
 * Return an alphabetic representation of the given integer.
 * A is the zero digit, B is 1, etc.
 * @parm int $n: the integer to represent
 * @parm int $len: the length of the string to return
 * @parm int $base: the radix (defaults to 26).
 */
function n2a($n, $len, $base = 26) {
  $result = '';
  for ($i = 0; $i < $len; $i++) {
    $digit = $n % $base;
    $result = chr(ord("A") + $digit) . $result;
    $n = (int) ($n / $base);
  }
  return $result;
}

/**
 * Return the numeric equivalent of the given alphabetic string.
 * @see n2a
 */
function a2n($string, $base = 26) {
  $result = 0;
  for ($i = 0; $i < strlen($string); $i++) {
    $result = $result * $base + ord($string[$i]) - ord("A");
  }
  return $result;
}

/**
 * Return a unique temporary name related to the given uniquely identifying parmeters 
 * in this order of preference: 
 *   phone: "unknown phone dddddddddd"
 *   email: "unknown whatever AT domain DOT ext"
 *   UNUSED (because it's hard to verify): website: "unknown domain DOT ext SLASH andsoforth"
 * If none, just pick a 10-character random string ("unknown 234h2khpsdf987")
 * @parm array $info: associative array of fields, each of which uniquely identifies a user.
 * @return string: the temporary name
 */
function temp_name($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = isset($my_number) ? $my_number : (isset($my_phone) ? $my_phone : '');
  if ($phone) return RCREDITS_TEMP_NAME_PREFIX . format_phone($phone, 'n');
  if (isset($my_email)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return RCREDITS_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

/**
 * Format the given phone number as wanted
 * @parm string $phone
 * @parm string $how: how to format it
 *   '+n' +1dddddddddd
 *   'n'  dddddddddd
 *   '+'  (+1) ddd-ddd-dddd
 *   ''   ddd-ddd-dddd (the default)
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
function format_phone($phone, $how = '') {
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // possible phone number for the other party
  if (substr($phone, 0, 2) == '+1') $phone = substr($phone, 2);
//  if (!is_numeric($phone) or strlen($phone) != 10) return FALSE;
  if (!is_numeric($phone)) return FALSE; // temporary for debugging
  if ($how == 'n') return $phone;
  if ($how == '+n') return '+1' . $phone;
  $phone = substr($phone, 0, 3) . '-' . substr($phone, 3, 3) . '-' . substr($phone, 6);
  if ($how == '') return $phone;
  if ($how == '+') return '(+1) ' . $phone;
  return FALSE;
}

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
  if(!($ctty_uid = variable_get('rcredits_community_uid'))) die("No community uid!");
  $sqls = array(
    'TRUNCATE TABLE mcapi_transactions',
    'TRUNCATE TABLE field_data_worth',
    'TRUNCATE TABLE field_revision_worth',
    'TRUNCATE TABLE sms_user',
    "DELETE FROM users WHERE uid > $ctty_uid",
  );
  foreach ($sqls as $sql) \db_query($sql);
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 */
function db_lookup($field_name, $table_name, $criteria = '', $subs = array()) {
  if ($criteria) $criteria = "WHERE $criteria";
  $sql = "SELECT $field_name FROM $table_name $criteria";
  $new_subs = array();
  if ($subs) foreach ($subs as $key => $value) $new_subs[':' . $key] = $value;
//  deb($sql . print_r($new_subs, TRUE));
  return \db_query($sql, $new_subs)->fetchField();
}

// Single line functions that need no explanation
function rebate($who) {return db_lookup('rebate_percent', 'users', "uid = $who")/100;}
function bonus($who) {return 2 * rebate($who);}
function user_field($field_name, $who) {return db_lookup($field_name, 'users', "uid = $who");}
function uid_from_crid($credit_id) {return db_lookup('uid', 'users', "credit_id = '$credit_id'");}
function tid_from_xid($who, $xid) {return db_lookup('COUNT(xid)', 'mcapi_transactions', "xid <= $xid");}
function total_demand() {return db_lookup('SUM(demand)', 'users');}

function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function is_temp_name($name) {return abbreviates(RCREDITS_TEMP_NAME_PREFIX, $name);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
function ray($s) {return explode(' ', $s);}
function access($access, $who) {return \user_access($access, \user_load($who));}