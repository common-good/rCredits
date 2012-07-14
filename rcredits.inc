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
define('RCREDITS_STARTING_BALANCE', 250); // how much people start with in the TRIAL RUN

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
  
  $scheme = compact(explode(' ', 'description type size length precision default scale serialize unsigned'));
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
  $new = compact(explode(' ', 'info human_name display zero uid'));
  $row = array(
    'currcode' => $currcode,
    'data' => (object) array_merge($cur, $new)
  );
  \drupal_write_record('mcapi_currencies', $row);
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
  deb($info);
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
  
  if ((strtolower($my_for) == 'cash') or ($my_for == '')) $my_for = 'cash';
  $tx->payer_for = $my_for;
  $tx->payee_for = $my_for;
  $tx->state = \TRANSACTION_STATE_FINISHED;
  $tx->type = $my_type;
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
  transact(compact('type', 'amount', 'payer', 'payee', 'for', 'creator'));
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
  $result = compact(explode(' ', 'balance available unavailable rewards cashable total_demand min_balance id'));
  $fancy = array();
  foreach ($result as $key => $value) {
    $fancy[$key] = is_numeric($value) ? format_amount($value) : $value;
  }
  $result['fancy'] = $fancy;
  return (object) $result;
}

/**
 * Say whether the user name is valid.
 */
function valid_name($name) {
  $words = substr_count($name, ' ') + 1;
  return (preg_match("/[^A-Z\-\.,' ]/i", $name) or $words < 2 or $words > 4) ? FALSE : TRUE;
}

/**
 * Create an appropriate credit id for the given phone number
 *
 * Callers of this function should roll it back if the id does not get used.
 * @parm string $cgc: what common good community the number's owner belongs to, if any
 * @todo: Change this to add a credit_id to the already-saved account record. The credit_id field
 *   should be UNIQUE INDEX'ed, so the UPDATE will fail in case of a collision (then just retry). 
 *   The next appropriate id should be found by SELECT MAX WHERE credit_id LIKE '$ctty____'
 *   (or '$ctty_____' -- 5-char tail for orgs), then convert to an integer and back to next id.
 *   (no need for credit_tails table).
 */
function make_credit_id($number, $cgc = '') {
  if ($number = format_phone($number, 'n')) {
    $area = substr($number, 0, 3);
    $state = db_lookup('region', 'r_area_codes', "area_code='$area'");
    $region = db_lookup('credit_region', 'r_credit_regions', "region='$state'");
    if (!$region) $region = 'Z9';
  }
//  $ctty = $area == '413' ? 'GF' : ($area == '860' ? 'NL' : 'AA'); // Greenfield, New London, or other
  $ctty = $region . 'AA'; // until the person becomes a member of a specific community, give them a generic community
  return $ctty . next_credit_id_tail($ctty);
}

/**
 * Return the next credit id tail for the given community.
 * Callers of this function should roll it back if the tail does not get used.
 */
function next_credit_id_tail($ctty, $retry = 0) {
  $tail = db_lookup('tail', 'r_credit_tails', "community='$ctty'");
  if ($tail === FALSE) {
    $sql = "INSERT INTO r_credit_tails (community, tail) VALUES ('$ctty', 0);";
    $count = \db_query($sql)->rowCount();
    $tail = -1;
  } else {
    $sql = "UPDATE r_credit_tails SET tail=tail+1 WHERE community='$ctty' AND tail='$tail'";
    $count = \db_query($sql)->rowCount();
  }
  if ($count) return n2a($tail + 1, 4);
  if ($retry > 9) return '';
  return next_credit_id_tail($ctty, $retry + 1);
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
 * in this order of preference: phone, email, website. If none, just pick a random string.
 * @parm array $info: associative array of fields, each of which uniquely identifies a user.
 */
function temp_name($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = isset($my_number) ? $my_number : (isset($my_phone) ? $my_phone : '');  
  if ($phone) return 'phone ' . format_phone($phone, 'n');
  if (isset($my_email)) return str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
  if (isset($my_website)) return str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return 'user ' . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
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
function user_field($who, $field_name) {return db_lookup($field_name, 'users', "uid = $who");}
function uid_from_crid($credit_id) {return db_lookup('uid', 'users', "credit_id = '$credit_id'");}
function tid_from_xid($who, $xid) {return db_lookup('COUNT(xid)', 'mcapi_transactions', "xid <= $xid");}
function total_demand() {return db_lookup('SUM(demand)', 'users');}

function valid_url($url) {return (\valid_url($url) and strpos($url, '.') and strpos($url, '@') === FALSE);}