<?php
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

/**
 * Make Fields
 *
 * Make (or unmake) database fields, according to the schema
 */
function make_fields($unmake = FALSE) {
  $schema = array();
  \rcredits_schema_alter($schema); // get the fields to add (or drop)
  foreach ($schema as $table_name => $one) {
    foreach ($one['fields'] as $field_name => $spec) {
      $exists = \db_field_exists($table_name, $field_name);
      if ($unmake) {
        if($exists) \db_drop_field($table_name, $field_name);
      } else {
        if($exists) {
          \db_change_field($table_name, $field_name, $field_name, $spec);
        } else {
          \db_add_field($table_name, $field_name, $spec);
        }
      }
    }
  }
}

/**
 * Setup Field
 *
 * @parm string $type: data_type and size, separated by a space (a numeric size is interpreted as a string's maximum length)
 * @parm string of boolean $SUN: $serialize, $unsigned, $notnull (one binary digit for each)
 * @parm string $precision: precision and scale, separated by a comma
 *
 * @return array: the field schema (UNUSED?)
 */
function setup_field(&$schema, $table_name, $label, $description = '', $type = 'varchar 255', $default = '', $SUN = '000', $precision = ',') {
  list ($label, $field_name) = is_array($label) ? $label : array($label, str_replace(' ', '_', strtolower($label)));
  $size = '';
  if (strpos($type, ' ')) list ($type, $size) = explode(' ', $type);
  if (is_numeric($size)) {
    $length = $size;
    $size = '';
  }
  list ($serialize, $unsigned, $notnull) = str_split($SUN);
  list ($precision, $scale) = explode(',', $precision);
  
  $scheme = compact(explode(' ', 'description type size length precision default scale serialize unsigned'));
  foreach ($scheme as $key => $value) {
    if ($value === '') unset($scheme[$key]);
  }
  $scheme['not null'] = $notnull;
  $schema[$table_name]['fields'][$field_name] = $scheme;
//  print_r($schema); die("\n\ntnm=$table_name");
  return $scheme;
}

/**
 * Make Currency
 *
 * Initialize the rCredits currency information
 *
 * @todo: get a standard schema from mcapi (ask Matthew)
 *   (mcapi_currency.inc is merely a stand-in)
 */
function make_currency() {
  require dirname(__FILE__) . '/mcapi_currency.inc';
  $currcode = RCREDITS_CURRENCY_CODE;
  if (db_lookup(1, 'mcapi_currencies', "currcode = '$currcode'")) return;
  $cur = mcapi\currency();
  $info = compact('currcode');
  $human_name = 'rCredits';
  $display = array('format' => '$@minus@integer.@subdivision', 'divisions' => 1);
  $zero = 1;
  $uid = 1;
  $new = compact(explode(' ', 'info human_name display zero uid'));
  $row = array(
    'currcode' => $currcode,
    'data' => (object) array_merge($cur, $new)
  );
  \drupal_write_record('mcapi_currencies', $row);
}

/**
 * Create User
 *
 * @return the user account object
 */
function create_user($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  if (isset($my_number)) {
    $info['sms_user'] = array(
      'number' => $my_number,
      'status' => 0,
      'code' => '',
      'gateway' => '',
    );
  }

  // If someone already has this email address, then use it instead of creating a new user.
  if (isset($my_mail) and ($account = array_shift(user_load_multiple(array(), array('mail' => $my_mail))))) {
    return user_save((object) $account, $info);
  }

  $info['mail'] = $info['init'] = '';
  $info['pass'] = user_password();
//  $info['name'] = $my_name ?: substr(md5($number . strval(REQUEST_TIME)), 0, 10); // Pick a pseudo-random name for the user
  $info['name'] = $my_name ? $my_name : substr(md5($number . strval(REQUEST_TIME)), 0, 10); // Pick a pseudo-random name for the user
  $info['status'] = TRUE;
  $info['is_new'] = TRUE;
  return \user_save((object) $info);
}

/**
 * Transact
 *
 * Record a transaction between two participants
 */
function transact($info) {
  $tx = transaction_setup($info);

  try {
    return \transaction_insert_new($tx, TRUE);
  }
  catch (Exception $e) {throw $e;}
}

/**
 * Transaction Setup
 *
 * Create a new transaction and set up all the important fields.
 */
function transaction_setup($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');

  $tx = new_transaction();
  unset($tx->xid);
  $tx->payer = $my_payer;
  $tx->payee = $my_payee;
  $tx->worth['und'][0]['currcode'] = RCREDITS_CURRENCY_CODE;
//  $tx->worth['und'][0]['currcode'] = isset($values['currency']) ? $values['currency'] : key(currencies_load());
  $tx->worth['und'][0]['quantity'] = $my_amount;
  
  if ((strtolower($my_for) == 'cash') or ($my_for == '')) $my_for = 'cash';
  $tx->payer_for = $my_for;
  $tx->payee_for = $my_for;
  $tx->state = \TRANSACTION_STATE_FINISHED;
  $tx->type = $my_type;
  if (isset($my_creator)) $tx->creator = $my_creator; // defaults to current user
  return $tx;
}

/**
 * Amount
 *
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
 * Log (dirt simple for the moment)
 *
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
 * Demand: Set or report how much rCredits the user wants to buy with US Dollars.
 *
 * @parm integer $who: uid of the user
 * @parm numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($who, $demand = 0) {
  if (!$demand) return db_lookup('demand', 'users', "uid = $who");
  $demand = min(RCREDITS_MAX_DEMAND, $demand);
  db_query("UPDATE users SET demand = $demand WHERE uid = $who");
  return $demand;
}

/**
 * Credit Information
 *
 * @param int $who
 *   uid of the credit account to return information for
 *
 * @return
 *   standard object of credit account information (see keys in the return line below)
 *   with corresponding number-formatted information in ->fancy
 *   including the overall demand for rCredits
 *   array keys in ->fancy all begin with '@', to make use with t() easy
 */
function credit_info($who) {
  $sql = "SELECT 
    SUM(IF(t.payer = $who, w.worth_quantity, 0)) AS gross_out, 
    SUM(IF(t.payee = $who, w.worth_quantity, 0)) AS gross_in,
    SUM(IF(t.type NOT IN('payment', 'charge'), w.worth_quantity, 0)) AS rewards
    FROM mcapi_transactions t LEFT JOIN field_data_worth w ON w.entity_id = t.xid
    WHERE bundle = 'transaction' AND (t.payer = $who OR t.payee = $who)";
  extract(db_query($sql)->fetchAssoc());
  $balance = $gross_in - $gross_out;
  $unavailable = 0; // for now
  $available = $balance - $unavailable;
  $cashable = $balance - $rewards;
  $demand = total_demand();
  $result = compact(explode(' ', 'balance available unavailable rewards cashable demand'));
  $fancy = array();
  foreach ($result as $key => $value) {
    $fancy["@$key"] = format_amount($value);
  }
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * DB Lookup
 *
 * Look the field up in the table, returning the first record matching the criteria.
 */
function db_lookup($field_name, $table_name, $criteria = '') {
  if ($criteria) $criteria = "WHERE $criteria";
  $sql = "SELECT $field_name FROM $table_name $criteria";
  return \db_query($sql)->fetchField();
}

/**
 * Various handy functions based on simple calls to db_lookup()
 */
function rebate($who) {return db_lookup('rebate_percent', 'users', "uid = $who")/100;}
function bonus($who) {return 2 * rebate($who);}
function uid_from_crid($credit_id) {return db_lookup('uid', 'users', "credit_id = '$credit_id'");}
function tid_from_xid($who, $xid) {return db_lookup('COUNT(xid)', 'mcapi_transactions', "xid <= $xid");}
function total_demand() {return db_lookup('SUM(demand)', 'users');}
