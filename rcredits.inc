<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT the Drupalish callbacks, which are in rcredits.module
 *
 * @todo
 * - the undo feature
 * - handling employees (virtual payments). This should probably be extended to suppliers too,
 *   or maybe even to everyone. The idea is to pay someone virtually in rCredits by totally paying them
 *   (either first or second) in US Dollars and (either second or first) having them buy back
 *   the appropriate amount of rCredits. Probably the best way to do this is with a simple payment transaction
 *   that requires confirmation by the payee (an "offer"). The payee cannot accept the offer without first paying
 *   that amount to the Community in US Dollars (using billpay or Dwolla).
 * - a way to track US Dollars payments (probably a separate transaction table us_transactions)
 * - rewards in "data" field get serialized to long strings of digits. Be more concise (store a int number of cents?).
 */
 
namespace rCredits;

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

$GLOBALS['account types'] = array(t('personal'), t('commercial'), t('non-profit'));
$GLOBALS['share permissions'] = array('no access', 'can read transactions', 'can create transactions', 'can manage account');
  
// Transaction states
define('TX_DONE', 1);
define('TX_PENDING', 0); // charged, waiting for confirmation
define('TX_DELETED', -1);
define('TX_DENIED', -2); // invoice disputed by payer

// Transaction channels
define('TX_WEB', 0);
define('TX_SMS', 1); 
define('TX_SMART', 2); 
define('TX_TOUCH', 3); // touch tone phone

define('TX_TYPES', 'signup payment charge rebate bonus interest');
define('TX_REWARD_OFFSET', 2); // how much to add to payment to get to rebate (and to charge to get to bonus)

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

/**
 * Setup a schema field
 *
 * @param string $type: data_type and size, separated by a space (a numeric size is interpreted as a string's maximum length)
 * @param string of boolean $SUN: $serialize, $unsigned, $notnull (one binary digit for each)
 * @param string $precision: precision and scale, separated by a comma
 *
 * @return array: the field's schema (UNUSED?)
 */
function setup_field(&$schema, $table_name, $label, $description = '', $type = 'varchar 255', $default = '', $SUN = '000') {
  $size = '';
  $precision = ',';

  list ($label, $field_name) = is_array($label) ? $label : array($label, str_replace(' ', '_', strtolower($label)));
  if (strpos($type, ' ')) list ($type, $size) = explode(' ', $type);
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
 * Add a single entry to the permission array for hook_permission()
 */
function permission(&$permissions, $title, $description) {
  $permissions[strtolower($title)] = compact('title', 'description');
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
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  if (isset($my_number) and !isset($my_phone)) $my_phone = $my_number;
  if (isset($my_phone)) $info['phone'] = format_phone($my_phone);
  
  $info['mail'] = $info['init'] = @$my_email ?: '';
// Don't create a password (that's how we know it's a temporary account) $info['pass'] = \user_password();
  $info['name'] =  @$my_name ?: temp_name($info);
  $info['status'] = TRUE;
  $info['is_new'] = TRUE;

  //*** Begin DBTX
  $account = \user_save(array(), $info);
  make_credit_id($account->uid); // this is done separately, to assure uniqueness
  if (isset($my_number)) SMS\create($my_number, $account->uid, SMS_USER_CONFIRMED);
  unset($dbtx);
  //*** End DBTX

  if (!$account) throw new \Exception('failed to save user record');
  return $account->uid;
}

function get($currency, &$amount, $uid, $confirmed) {    
  if (!access('can manage account')) return array('no access get', array(), FALSE);
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return array('bad amount', '', FALSE);
  $amount = min(RCREDITS_MAX_DEMAND, $amount);
  $request = format_amount($amount);
  $success = TRUE;
  if ($currency == 'r') {
// (catch this only when demand is high)      if ($my_amount < RCREDITS_MIN_DEMAND) return array('demand too small');
    demand($uid, $amount);
    return array('your demand', compact(ray('request success')), FALSE);
  }

  // $currency == 'usd'
  extract((array) credit_info($uid), EXTR_PREFIX_ALL, 'info');
  if ($amount > $info_cashable) return array("can't cash incentives", $info_fancy, FALSE);
  $type = 'payment';
  $payer = $creator = $uid;
  $payee = variable_get('rcredits_community_uid');
  $for = t('cash');

  //*** Begin DBTX
  $dbtx = db_transaction();
  transact(compact(ray('type amount payer payee for creator')));
  $balance = format_amount($info_balance - $amount);
  $amount = format_amount($amount);
  if ($confirmed) return array('report get usd', compact(ray('amount balance success')), FALSE);
  
  $dbtx->rollback();
  return array('get usd', compact(ray('amount success')), TRUE);
  //*** End DBTX
}

function pay_invoice($xid) {
  if (!access('can create transactions')) return array('no access create', array());
  $sql = "SELECT * FROM r_txs WHERE xid = $xid AND state = :state LIMIT 1";
  $result = db_query($sql, array(':state' => TX_PENDING))->fetchAssoc();
  if (!$result) die('bad call to ' . __FUNCTION__);
  extract($result, EXTR_PREFIX_ALL, 'old');

  //*** Begin DBTX
  $dbtx = \db_transaction();
  list ($message, $args, $confirm) = transfer('invoice payment', $old_payer, $old_payee, $old_amount, $old_payee_for, TRUE);
  if (@$args['success']) {
    // FUNCTION ALWAYS FAILS (why?) r\set_tx_field('state', TX_DELETED, $old_xid);
    $subs = array(':state' => TX_DELETED, ':serial' => $old_serial);
    \db_query('UPDATE r_txs SET state=:state WHERE serial=:serial', $subs);
  } else $dbtx->rollback();
  
  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  return array($message, $args);
}

/**
 * Format and return args for reporting a transaction
 * Not all returned args are used for any particular type of transaction.
 */
function tx_report_args($taking, $amount, $shortfall, $reward, $my_uid, $other_uid, $xid, $for_cash) {
  $type = $for_cash ? 'traded' : ($taking ? t('charged') : t('paid'));
  $amount = format_amount($amount);
  $short = format_amount($shortfall);
  $tofrom = $taking ? t('from') : t('to');
  $reward_type = $taking ? t('bonus') : t('rebate');
  $reward_amount = format_amount($reward);
  $other = user_field('full_name', $other_uid);

  $balance = credit_info($my_uid)->fancy['balance'];
  $tid = tid_from_xid($my_uid, $xid);

  $way = $taking ? 'cash for rCredits' : 'rCredits for cash'; // use as both text and message index (might wanna separate out way_index)
  $success = TRUE;
  
  return compact(ray('type amount tofrom reward_type reward_amount balance tid other short way success'));
}

/**
 * Create a transaction
 * @param bool $type: payment, charge, or invoice payment
 * @param int $my_uid: user id of the initiating user
 * @param int $other_uid: user id of the other party
 * @param numeric $requested_amount: the transaction amount
 * @param string $for: description of the transaction's purpose (user input)
 * @param bool $confirmed: has this transaction been confirmed by the user? (default TRUE, used only for SMS interface)
 * @param array $rewards: associative array of reward percentages, indexed by 'rebate' and 'bonus' (default empty means standard rewards)
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: $taking (charge vs. payment), $preapproved (paying or authorized to charge), 
 * $confirmed (user confirmed the transaction request), $xid (approving an invoice), $for_cash (the transaction is an exchange for USD),
 * $shortfall (how inadequate the payer's balance is)
 */
function transfer($type, $my_uid, $other_uid, $requested_amount, $for, $confirmed = TRUE, $rewards = array()) {
  if (!access('can create transactions', $my_uid)) return array('no access create', array(), FALSE);
  $taking = ($type == 'charge');
  if ($paying_invoice = ($type == 'invoice payment')) $type = 'payment';
  $confirm_type = $type;
  $preapproved = $taking ? access('charge others', $my_uid) : TRUE;
  if ($for_cash = for_cash($for)) $for = 'cash';
  
  $available = credit_info($taking ? $other_uid : $my_uid)->available;
  $amount = min($requested_amount, $available);
  $shortfall = max(0, $requested_amount - $amount); // how much needs to be done in a separate currency
  if ($shortfall and $preapproved) {
    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
    $short = format_amount($shortfall);
    $other = user_field('full_name', $other_uid);
    if ($paying_invoice) return array('short invoice', compact('short', 'other'), FALSE); // no partial payment of invoice
    if ($for_cash) return array($taking ? 'short cash from' : 'short cash to', compact('short', 'other'), FALSE); // no partial cash trans
    if (ceil($shortfall) >= $requested_amount) return array($taking ? 'broke from' : 'broke to', compact('other'), FALSE);
    $shortfall = ceil($shortfall); // for goods and services, shortfall should be a round figure (no cents)
    $amount = $requested_amount - $shortfall;
  }
  
  list ($payer, $payee) = $taking ? array($other_uid, $my_uid) : array($my_uid, $other_uid);
  list ($rebate, $bonus) = $for_cash ? array(0, 0) : array(rebate($payer), bonus($payee));
  $data = empty($rewards) ? compact('rebate', 'bonus') : $rewards;
  $state = $preapproved ? TX_DONE : TX_PENDING;

  //*** Begin DBTX
  $dbtx = db_transaction();
  $txs = transact(compact(ray('type amount payer payee for data state')));
  $xid = $txs[0]['xid'];
  if (!$confirmed) $dbtx->rollback();

  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  $reward_index = $taking ? 2 : 1;
  $reward = @$txs[$reward_index]['amount'] ?: 0;

  $args = tx_report_args($taking, $amount, $shortfall, $reward, $my_uid, $other_uid, $xid, $for_cash);
  if ($for_cash) {
    $message = $confirmed ? (($taking and !$preapproved) ? 'report exchange request' : 'report exchange') : $args['way'];
    return array($message, $args, !$confirmed);
  }
  
  if ($confirmed) {
    $message = $shortfall ? 'report short transaction' : (($taking and !$preapproved) ? 'report invoice' : 'report transaction');
    return array($message, $args, FALSE);
  }
  if ($shortfall) $args['amount'] = format_amount($requested_amount); // restore original amount before getting confirmation
  return array($confirm_type, $args, TRUE);
}

/**
 * Record a transaction between two participants
 * @see also r\transaction_rewards() and r\transfer()
 * type is passed as a string
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
    $txs[$key]['type'] = tx_type($txs[$key]['type']); // translate from string to numeric at the last moment
    drupal_write_record('r_txs', $txs[$key]); // pass actual array element, to get xids
    if ($key == 0) {
      $txs[$key]['serial'] = $serial = $txs[0]['xid']; // set serial for main transaction
      db_query('UPDATE r_txs SET serial=:serial WHERE xid=:serial', array(':serial' => $serial));
    }
  }

  unset($dbtx); // $dbtx->commit() is disallowed
  //*** End DBTX
  
  return $txs;
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
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  
  if (($my_type != 'payment') and ($my_type != 'charge')) return;
  if ($my_payer_for == 'cash') return; // quit unless rebates and bonuses apply
  $rewards = unserialize($my_data);
  
  $fields = ray('type amount payer payee for creator state');
  $payer = $creator = variable_get('rcredits_community_uid');
//  $payer_id = $creator_id = RCREDITS_COMMUNITY_ID;
  $state = $my_state; // match rewards state to main transaction
  $type = 'rebate';
  $payee = $my_payer;
//  $payee_id = $my_payer_id;
  $amount = $my_amount * $rewards[$type];
  $for = t('rebate');
  $txs[] = transaction_setup(compact($fields));

  $type = 'bonus';
  $payee = $my_payee;
//  $payee_id = $my_payee_id;
  $amount = $my_amount * $rewards[$type];
  $for = t('bonus');
  $txs[] = transaction_setup(compact($fields));
}

/**
 * Create a new transaction and set up all the important fields.
 */
function transaction_setup($info) {
  extract($info);

  if (!isset($state)) $state = TX_DONE; // don't use @$my_state here (might be zero)
  if (!@$my_creator) $creator = be_who();
  $created = REQUEST_TIME;
  $serial = NULL; // always set equal to xid of main transaction after saving
  $data = serialize(@$data ?: array());
  
  $payer_for = $payee_for = $for;
  unset($info['for']);

  return array_merge($info, compact(ray('state creator created serial data payer_for payee_for')));
}

/**
 * Interpret the given transaction type
 * @param mixed $type: a number or string indicating the transaction type
 * @return a corresponding string or number (whatever the $type isn't)
 */
function tx_type($type) {
  $types = ray(TX_TYPES);
  return ($type = (is_numeric($type)) ? $types[$type] : array_search($type, $types));
}
function tx_types($string) {return strtr($string, array_flip(ray(TX_TYPES)));} // just string, just one way, for now

/**
 * Record an incentive transaction for signing up
 * @param int who: uid of the person signing up
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
 * Figure out who the user meant.
 *
 * @param string $who: how the user referred to the trader
 * @param numeric $my_uid
 * @param boolean $create: whether to create a record, if the other trader cannot be identified (default FALSE)
 * @param string $self_message: index to error message for identifying oneself
 * @return one of the following:
 *   the other trader's uid
 *   an array whose elemnts are: an error index, an array of substitutions, an array of possible uids (0 or more)
 */
function identify($who, $my_uid, $create = FALSE, $self_message = 'no self-trading') {
  if ($phone = format_phone($who, '+n')) $who = $phone;
//  if (substr($who, 0, 7) == 'http://') $who = substr($who, 7);
//  if (substr($who, 0, 4) == 'www.') $who = substr($who, 4);

  $who = strtolower(str_replace("'", '', $who)); // avoid SQL errors
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
          WHERE '$who' IN (CONCAT('.', u.short_name), u.credit_id, s.number, u.phone, u.mail)";
  $result = db_query($sql)->fetchCol();
  if(count($result) > 1) return array('ambiguous other', compact('who'), $result);
  if (isset($result[0])) {
    $other_uid = $result[0];
    if ($other_uid == $my_uid) return array($self_message, array(), $result);
    $other_account = \user_load($other_uid);
    $other = $other_account->full_name;
    if (!access('transact', $other_uid)) return array('non trader', compact('other'), $result);

    return $other_uid;
  }
  
//  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : (r\valid_url("http://$who") ? 'website' : ''));
  $key = $who == $phone ? 'phone' : (valid_email_address($who) ? 'email' : '');
  if ((!$key) or !$create) return array('unknown user', compact('who'), $result);
  return create_account(array($key => $who));
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 * @todo: move this to rcredits.inc or utils.inc
 */
function nonce() {
  $offset = db_lookup('FLOOR(RAND() * COUNT(*))', 'r_nonces');
  $nonce = db_lookup('nonce', 'r_nonces', "1 LIMIT $offset, 1");

  $impossible = join(' ', $GLOBALS['SMS not a nonce']);
  if (strpos(". $impossible ", " $nonce ")) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long
  return $nonce;
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
 * Return information about the given account
 * @param int $who: uid of the credit account to return information for
 * @param boolean $state: TX_DONE (default) or TX_PENDING (return info about what kind of transactions)
 * @param numeric $asof: last date to include (defaults to NULL, meaning unrestricted)
 *
 * @return
 *   standard object of credit account information (see $result_keys, below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 */
function credit_info($who, $state = TX_DONE, $asof = '') {
  $result_keys = 'gross_in gross_out gross_in_proper balance available unavailable rewards cashable total_demand min_balance id';
  $sql = "SELECT credit_id AS id, min_balance FROM users WHERE uid = $who";
  extract(db_query($sql)->fetchAssoc()); // gets $id, $min_balance
  $types = tx_types("'payment', 'charge'");
  $sql = "SELECT 
    SUM(IF(t.payer = $who, t.amount, 0)) AS gross_out, 
    SUM(IF(t.payee = $who, t.amount, 0)) AS gross_in,
    SUM(IF(t.type NOT IN($types), t.amount, 0)) AS rewards
    FROM r_txs t
    WHERE (t.payer = $who OR t.payee = $who) AND t.state = $state";
  if ($asof) $sql .= " AND t.created <= $asof";

  extract(db_query($sql)->fetchAssoc());
  $gross_in_proper = $gross_in - $rewards;
  $balance = $gross_in - $gross_out;
  $unavailable = 0; // for now
  $available = ($balance - $min_balance) - $unavailable;
  $cashable = $balance - $rewards;
  $total_demand = total_demand();
  $result = compact(ray($result_keys));
  $fancy = array();
  foreach ($result as $key => $value) $fancy[$key] = is_numeric($value ?: 0) ? format_amount($value) : $value;
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * Say whether the user name is valid.
 * @param string $name: the name to test
 * @param boolean $picky: whether to allow only 2-4 word names (default TRUE)
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
 * Merge a temporary ("old") account into a permanent ("new") account.
 * @todo: handle resultant self-transactions
 */
function merge_accounts($old_uid, $new_uid) {
  if (!access('can manage account')) die('unauthorized merge_accounts');
//  if (!is_temp_name(user_field('name', $old_uid))) die('merging non-temp account');
  if (!is_temp_account($old_uid)) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below
  /*** Begin DBTX ***/ $dbtx = db_transaction();
  \db_query("UPDATE r_txs SET payer=$new_uid WHERE payer=$old_uid");
  \db_query("UPDATE r_txs SET payee=$new_uid WHERE payee=$old_uid");
  \db_query("UPDATE sms_user SET uid=$new_uid WHERE uid=$old_uid");
  \db_query("UPDATE users SET uid=-uid, status=0 WHERE uid=$old_uid");
  /*** End DBTX ***/ unset($dbtx);
}

/**
 * Return the standard 2-letter abbreviation for the country or vice versa
 * See the latest data on Wikipedia.
 * @todo: get this in agreement with countries.js (sorted by country name)
 */
function country_abbrev($country) {
  global $base_path;
//  include_once $base_path . 'inc/countries.php';
  include_once __DIR__ . '/../../../../inc/countries.php';
  return strlen($country) == 2 ? $countries[$country] : array_search($country, $countries);
}

/**
 * Return the standard 2-letter abbreviation for the (US) state or vice versa
 * @todo: add Canadian provinces
 */
function state_abbrev($state) {
  global $base_path;
  include_once __DIR__ . '/../../../../inc/states.php';
  return strlen($state) == 2 ? $states[$state] : array_search($state, $states);
}

/**
 * Create an appropriate credit id for the given account
 * @return the credit id (FALSE if failure)
 */
function make_credit_id($uid) {
  $account = user_load($uid);
  if (@$account->credit_id) return $account->credit_id;
  if ($state = $account->state) { // true for web interface, not for SMS
    $country = $account->country;
    if ($country == 'United States') $state = state_abbrev($state);
  } else { // SMS, not web
    $phone = format_phone($account->phone, 'n');
    $area = substr($phone, 0, 3);
    $state = db_lookup('region', 'r_area_codes', "area_code='$area'");
    $country = 'United States';
  }
  if ($country == 'United States') {
    $region = db_lookup('credit_region', 'r_credit_regions', "region='$state'");
  } else $region = country_abbrev($country);
  
  if (!@$region) $region = 'Z9';
  $ctty = $region . 'AA'; // until the person becomes a member of a specific community, give them a generic community in their region

  for ($try = 0; $try < 99; $try++) {
    $credit_id = next_credit_id($ctty);
    \user_save($account, compact('credit_id'));
    if (!db_lookup('uid', 'users', "credit_id = '$credit_id' AND uid < " . $uid)) return $credit_id;
  }
  die("Too many collisions in make_credit_id() -- ctty = $ctty");
}

/**
 * Return the next credit id for the given community.
 * @param string $ctty: 4-character community header
 * @param bool $business: return a credit id for a business? (as opposed to for an individual)
 */
function next_credit_id($ctty, $business = FALSE) {
  $pattern = $business ? '_____' : '____';
  $next = db_lookup('MAX(credit_id)', 'users', "credit_id LIKE '$ctty$pattern'");
  $tail = $next ? a2n(substr($next, 4)) : -1;
  return $ctty . n2a($tail + 1, 4);
}

/**
 * Use the specified account or say which is in use.
 * @param numeric $uid: (optional) uid of account to use (must be in $_SESSION['account_choices'])
 * @param numeric $permission: (optional returned) permission for the current account
 * @return numeric: uid of account currently in use.
 */
function be_who($uid = '', &$permission = 0) {
  global $user;

  if (!$uid) $uid = @$_SESSION['being'] ?: $user->uid;
  if (!($accounts = @unserialize($_SESSION['account_choices'])) or !@$accounts[$uid]) {
    $_SESSION['account_choices'] = serialize(array($user->uid => count($GLOBALS['share permissions']) - 1));
  }
  $accounts = unserialize($_SESSION['account_choices']);
  if (!($permission = $accounts[$uid])) die('bad be_who: ' . $uid);
  return ($_SESSION['being'] = $uid);
}

/*  if (!isset($accounts[$uid])) die('bad be_who: ' . $uid);
  $permission = $accounts[$uid];*/

function access($access, $uid = '') {
  if (!$uid) $uid = be_who('', $permission);
  if (($acct_perm = array_search($access, $GLOBALS['share permissions'])) !== FALSE) {
    $accounts = unserialize($_SESSION['account_choices']);
    return ($accounts[$uid] >= $acct_perm);
  } else return \user_access($access, \user_load($uid));
}

/**
 * Calculate a short_name from the given full_name.
 */
function short_name($full_name) {
  return strtolower(preg_replace('/\W/', '', iconv('UTF-8', 'ASCII//TRANSLIT', $full_name)));
}

function account_name($uid) {
  extract((array) \user_load($uid), EXTR_PREFIX_ALL, 'my');
  return @$my_credit_id ? ": $my_full_name ($my_credit_id) $my_mail" : ": $my_mail";
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function for_cash($for) {
  return ($for == '' or preg_match('/(cash|credit|usd|gift)( |$)/i', $for));
}

/**
 * Return an alphabetic representation of the given integer.
 * A is the zero digit, B is 1, etc.
 * @param int $n: the integer to represent
 * @param int $len: the length of the string to return
 * @param int $base: the radix (defaults to 26).
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
 * @param array $info: associative array of fields, each of which uniquely identifies a user.
 * @return string: the temporary name
 */
function temp_name($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = @$my_number ?: (@$my_phone ?: '');
  if ($phone) return RCREDITS_TEMP_NAME_PREFIX . format_phone($phone, 'n');
  if (isset($my_email)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return RCREDITS_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return RCREDITS_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

/**
 * Format the given phone number as wanted
 * @param string $phone
 * @param string $how: how to format it
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
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE sms_user',
    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
}

/**
 * Return data from a single record
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function db_get_record($table, $where, $fields = '*', $subs = array()) {
  return db_query("SELECT $fields FROM $table WHERE $where LIMIT 1", prefix_keys(':', $subs))->fetchAssoc();
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function db_lookup($field_name, $table_name, $criteria = '1', $subs = array()) {
  $sql = "SELECT $field_name FROM $table_name WHERE $criteria";
  return \db_query($sql, prefix_keys(':', $subs))->fetchField();
}

/**
 * Return the array with its keys prefixed by the given string
 */
function prefix_keys($prefix, $array) {
  if ($prefix == '') return $array;
  $new_array = array();
  foreach ($array as $key => $value) $new_array[$prefix . $key] = $value;
  return $new_array;
}

function array_prefix($prefix, $array) {
  foreach ($array as $key => $value) $array[$key] = $prefix . $value;
  return $array;
}

/**
 * Return the difference between two arrays (better than array_diff_assoc)
 */
function diff($a, $b) {
  $result = array();
  foreach ($a as $key => $one) if (!is_null($a[$key])) {
    if ($a[$key] != @$b[$key] or !isset($b[$key])) $result[] = $key . (isset($b[$key]) ? '' : ': missing in #2');
  }
  foreach ($b as $key => $one) if (!is_null($b[$key]) and !isset($a[$key])) $result[] = $key . ': missing in #1';
  return $result;
}

/**
 * Split the given string into an array
 * delimited by space, comma, or semi-colon, followed by any number of spaces, or by a bar surrounded by optional spaces
 * Entries in a comma-delimited list that contain a colon will be split into key and value.
 */
function ray($s) {
  $pattern = strpos($s, '|') ? ' *\| *' : (strpos($s, ',') ? '\, *' : (strpos($s, ';') ? '; *' : (strpos($s, ' ') ? ' +' : '\, *')));
  $simple = mb_split($pattern, $s);
  if (strpos($s, '|') or !strpos($s, ',') or !strpos($s, ':')) return $simple;

  $array = array();
  foreach ($simple as $one) {
    list ($key, $value) = explode(':', $one);
    $array[$key] = $value;
  }
  return $array;
}

/**
 * Get the long version of the indicated message(s), make any substitutions,
 * and return the result.
 *
 * @param string $message
 *   name of the message (index into $GLOBALS['SMS messages'])
 *   or a list of such message indices, separated by "|"
 *   or a two-element array: array(message, subs)
 * @param array $subs (optional). '@' will be prepended to the keys.
 * @param string $interface: which message set to use (SMS messages provide a fallback)
 */   
function tt($message, $subs = array(), $interface = 'SMS') {
  if (is_array($message)) list ($message, $subs) = $message;
  if (!$subs) $subs = array();
  $messages = explode('|', $message);
  foreach ($messages as $i => $message) {
    if (isset($GLOBALS["$interface messages"][$message])) {
      $model = $GLOBALS["$interface messages"][$message];
    } else {    
      $model = $GLOBALS['SMS messages'][$message];
      if ($p = strpos($model, '. Type ')) $model = substr($model, 0, $p + 1); // don't tell the user to type something unless it's SMS
    }
    $messages[$i] = strtr($model, prefix_keys('@', $subs));
  }
  return join(' ', $messages);
}

// Single line functions that need no explanation
function rebate($who) {return round(db_lookup('rebate_percent', 'users', "uid = $who")/100, 2);}
function bonus($who) {return 2 * rebate($who);}
function user_field($field_name, $who) {return db_lookup($field_name, 'users', "uid = $who");}
function update_user($info, $who) {return \db_update('users')->fields($info)->condition('uid', $who)->execute();}
function tx_field($field_name, $xid) {return db_lookup($field_name, 'r_txs', "xid = $xid");}
function set_tx_field($field_name, $new_value, $xid) {\db_update('r_txs', array($field_name, $new_value))->condition('xid', $xid);}
function uid_from_crid($credit_id) {return db_lookup('uid', 'users', 'credit_id=:credit_id', compact('credit_id'));}
function tid_from_xid($who, $xid) {return db_lookup('COUNT(xid)', 'r_txs', "xid <= $xid AND (type = 'payment' OR type = 'charge')");}
function total_demand() {return db_lookup('SUM(demand)', 'users');}
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
//function is_temp_name($name) {return abbreviates(RCREDITS_TEMP_NAME_PREFIX, $name);}
function is_temp_account($uid) {return !user_field('pass', $uid);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
