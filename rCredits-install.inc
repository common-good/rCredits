<?php

namespace rCredits;
use rCredits\Util as u;

/**
 * @file
 * Extra (non-hook) functions for rcredits.install
 */

/**
 * Setup the custom data schema.
 * This is used by both rcredits_schema() and rcredits_schema_alter(), 
 * using or ignoring each table key, depending on whether the table exists (otherwise Drupal deletes all the data)
 * @param assoc $schema: the schema to modify or empty, if creating from scratch
 */
function tableDefs($schema = array()) {
  require_once 'rcredits-settings.inc'; // Drupal bug. hook_schema needs these explicit, both uninstalling and installing.
  require_once 'rcredits-util.inc';

  // table user (additions to Drupal standard)
  $fields = array( // additional users fields
    'account_type' => setupField('personal, commercial, non-profit (or government)', 'int tiny'),
    'flags' => setupField('permissions and boolean state flags', 'int big', 0, '010'),
    'community_uid' => setupField("uid of this account's Common Good Community", 'int 11'),
    'federal_id' => setupField('social security number or employer id number, for reporting', 'varchar 9'),
    'dob' => setupField('date of birth, founding, or incorporation', 'int 11'),
    'id_proof' => setupField('proof of identity encryption key', 'text tiny'), // blob for binary (just 33 bytes)
    'question' => setupField('security question', 'varchar 255'),
    'answer' => setupField('answer to security question', 'varchar 255'),
    'full_name' => setupField('full name of the individual or entity', 'varchar 60'),
    'phone' => setupField('contact phone (no country code, no punctuation)', 'varchar 255'),
    'fax' => setupField('fax number (no country code, no punctuation)', 'varchar 255'),
    'address' => setupField('postal street address', 'varchar 60'),
    'city' => setupField('municipality', 'varchar 60'),
    'state' => setupField('full state/province name', 'varchar 60'), // varchar because char is deprecated
    'postal_code' => setupField('contact postal code (no punctuation)', 'varchar 20'),
    'country' => setupField('full country name', 'varchar 60'),
    'website' => setupField('primary website', 'varchar 255'),
    'description' => setupField('markup text describing the company (unused for personal accounts)', 'text medium'),
    'notes' => setupField('miscellaneous notes about the user or the account', 'text medium'),
    'signed_agreement' => setupField('when did this person sign the rCredits Agreement', 'int 11'),
    'rebate_percent' => setupField('current rebate percentage (sales bonus is double)', 'numeric 4,3', 5),
    'tithe' => setupField('fraction of rebates and bonuses to contribute back to Common Good Finance', 'numeric 6,3', 0),
    'demand' => setupField('waiting to buy this much credit', 'numeric 11,2', 0),
    'demand_date' => setupField('first rCredits request data', 'int 11'),
    'counts' => setupField('counts of command use, etc.', 'int big', 0),
    'min_balance' => setupField('minimum balance (normally zero or less)', 'numeric 9,2', R_MIN_BALANCE),
// separate table for this? 'nym' => setupField('unique identifier for cryptographic signing', 'text tiny'), 
    'usd_account' => setupField('official currency account number (eg Dwolla) with 2-char header', 'varchar 36'), // Dwolla acct (34 is enough for eventual IBAN)
    'usd_auth' => setupField('official currency account authorization token', 'text medium'), // Dwolla code may be any len
  );
  $schema['users']['fields'] = (@$schema['users']['fields'] ?: array()) + $fields;
  $schema['users']['fields']['uid'] = setupField('Unique user ID', 'int 11'); // not unsigned
// kills Drupal: $schema['users']['fields']['data']['description'] = 'incidental data (a serialized array of name value pairs)';

  // table r_sms 
  $fields = array(
    'number' => setupField('international phone number (eg +12223334444)', 'varchar 16'),
    'uid' => setupField('uid of cell phone owner (one owner per number)', 'int 11'),
    'status' => setupField('phone status', 'int tiny'),
    'todo' => setupField('waiting for confirmation to complete this operation', 'text medium'),
    'nonce' => setupField('waiting for this nonce, for confirmation', 'varchar 10'),
  );
  $foreign_keys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_sms'] = setupTable('SMS cell numbers corresponding to user accounts', $fields, 'number', $foreign_keys, $indexes);
  
  // table r_txs (and r_asif)
  $fields = array(
    'xid' => setupField('the unique transaction ID', 'serial 11'), 
    'serial' => setupField('serial number of related transactions (=xid of first transaction in the group)', 'int 11'), 
    'type' => setupField('transaction type (transfer, rebate, etc.)', 'int tiny'), 
    'taking' => setupField('was the transaction initiated by the payee', 'int tiny'), 
    'goods' => setupField('is this transfer and exchange for real goods and services?', 'int tiny'), 
    'state' => setupField('completed, pending, disputed, etc', 'int tiny'), 
    'amount' => setupField('transaction amount', 'numeric 11,2'), 
    'payer' => setupField('user id of the payer', 'int 11'), 
    'payee' => setupField('user id of the payee', 'int 11'), 
    'payer_agent' => setupField("user id of payer's agent (who approved this transaction for the payer)", 'int 11'), 
    'payee_agent' => setupField("user id of payee's agent (who approved this transaction for the payee)", 'int 11'), 
    'payer_for' => setupField("payer's description", 'varchar 60'), 
    'payee_for' => setupField("payee's description", 'varchar 60'), 
    'data' => setupField('info stored elsewhere, duplicated here for convenience (serialized array)', 'text big'), 
    'channel' => setupField('through what medium was the transaction entered', 'int tiny'), 
    'created' => setupField('Unixtime transaction was created', 'int 11'),
    'completed' => setupField('Unixtime transaction was completed', 'int 11'), // MYSQL reserves DATE, COMPLETED, WHEN
  );
  $foreign_keys = foreignKey('payer') + foreignKey('payee') + foreignKey('payer_agent') + foreignKey('payee_agent');
  $indexes = index('payer') + index('payee') + index('created') + index('date'); // also serial/type/taking/goods/state/payer_agent/payee_agent/channel/?
  $schema['r_txs'] = setupTable('Record of all rCredits transactions in the region', $fields, 'xid', $foreign_keys, $indexes);

  $schema['r_asif'] = setupTable('Record of pretend (asif) transactions in the region', $fields, 'xid', $foreign_keys, $indexes);
  
  foreach (u\ray('type taking goods payer_agent payee_agent payer_for payee_for') as $one) unset($fields[$one]);
  $foreign_keys = foreignKey('payer') + foreignKey('payee');
  $schema['r_usd'] = setupTable('Record of USD (Dwolla) transactions in the region', $fields, 'xid', $foreign_keys, $indexes);

  // table r_donations
  $fields = array(
    'donid' => setupField('donation record id', 'serial 11'),
    'created' => setupField('date/time of donation', 'int 11'),
    'uid' => setupField('uid of account that made the donation', 'int 11'),
    'amount' => setupField('amount of donation', 'numeric 11,2'),
    'type' => setupField('donation type (cc, check, cash, rCredits pledge, rCredits)', 'int tiny'),
    'how_often' => setupField('recurring how often (Y, Q, M, 1)', 'varchar 1'), // this is for information only
    'check_number' => setupField('check number, if any', 'varchar 60'),
    'received' => setupField('date actually received', 'int 11'),
  );
  $foreign_keys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_donations'] = setupTable('Donation details (for rCredits "Partners")', $fields, 'donid', $foreign_keys, $indexes);

  // table r_log
  $fields = array(
    'logid' => setupField('log record id', 'serial 11'),
    'time' => setupField('date/time logged', 'int 11'),
    'channel' => setupField('logged from what interface module', 'int tiny'),
    'type' => setupField('what type of log entry', 'varchar 60'),
    'cuid' => setupField('current account uid', 'int 11'),
    'agent' => setupField('agent account uid', 'int 11'),
    'info' => setupField('arbitrary serialized data', 'text medium'),
    'special' => setupField('special value if any', 'varchar 20'), // eg cell number or nonce
  );
  $foreign_keys = foreignKey('cuid') + foreignKey('agent');
  $indexes = index('type') + index('channel') + index('cuid') + index('agent');  
  $schema['r_log'] = setupTable('Development and error log', $fields, 'logid', $foreign_keys, $indexes);
  
  // table r_demand
  $fields = array(
    'id' => setupField('demand record id', 'serial 11'),
    'time' => setupField('date/time requested', 'int 11'),
    'amount' => setupField('request amount', 'numeric 11,2'), 
    'who' => setupField('uid of the account requesting rCredits', 'int 11'),
    'is_real' => setupField('was the account an rTrader at the time of the request', 'int tiny'), // "real" is a reserved word
  );
  $foreign_keys = foreignKey('who');
  $indexes = index('who');
  $schema['r_demand'] = setupTable('Who requested rCredits when', $fields, 'id', $foreign_keys, $indexes);
  
  // table r_relations
  $fields = array(
    'reid' => setupField('relationship record id', 'serial 11'),
    'main_uid' => setupField('uid of the account to which others are related', 'int 11'),
    'agent_uid' => setupField('uid of a user related to that account', 'int 11'),
    'foreign_uid' => setupField('uid of the foreign main account or foreign agent in their own region (main_uid or agent_uid is then the region)', 'int 11'),
    'foreign_name' => setupField('full name of the foreign main account or foreign agent', 'varchar 60'),
    'permission' => setupField('what type of permission the agent has on the main account', 'int tiny', 0),
    'employer_ok' => setupField('employer says this agent is an employee', 'int tiny', 0),
    'employee_ok' => setupField('employee says this agent is an employee', 'int tiny', 0),
    'is_owner' => setupField('this agent is an owner (or part owner)', 'int tiny', 0),
  );
  $foreign_keys = foreignKey('main_uid') + foreignKey('agent_uid') + foreignKey('foreign_uid');
  $indexes = index('main_uid') + index('agent_uid');
  $schema['r_relations'] = setupTable('Who can manage which accounts, and how', $fields, 'reid', $foreign_keys, $indexes);
    
  // table r_candidates (almost same as r_relations, but with extra fields -- used for candidates in rcredits-circles.inc)
  $fields = array(
    'uid' => setupField('candidate record id', 'serial 11'),
    'is_company' => setupField('is this a company', 'int tiny', 0),
    'ignor' => setupField('temporarily omit this record from consideration', 'int tiny', 0), // IGNORE is reserved in MySQL
    'score' => setupField('how good is this candidate', 'int 11', 0),
  );
  $foreign_keys = foreignKey('uid');
  $schema['r_candidates'] = setupTable('Candidates to become rTraders', $fields, 'uid', $foreign_keys);
/*
  $fields = array(
    'ignore' => setupField('temporarily omit this record from consideration', 'int tiny'),
    'employee_isrtrader' => setupField('this employee is an rTrader', 'int tiny'),
    'score' => setupField('how good is this candidate', 'int 11'),
  );
  $schema['r_candidates'] = $schema['r_relations'];
  $schema['r_candidates']['fields'] += $fields;
  $schema['r_candidates']['description] = 'Candidates to become rTraders';
*/

  // table r_smarts
  $fields = array(
    'code' => setupField('permanent secret identifier for the device', 'varchar 255'),
    'owner' => setupField('uid of the smart device owner', 'int 11'),
    'default_agent' => setupField('uid of the default user of the device', 'int 11'),
    'device' => setupField('human-readable identifier for the smart device', 'varchar 60'),
    'transient_id' => setupField('once-use identifier displayed on the device as a QR for the current account/agent', 'varchar 255'),
    'accessed' => setupField('date/time last used', 'int 11'), // to trigger deletion after several months
  );
  $foreign_keys = foreignKey('owner') + foreignKey('default_agent');
  $indexes = index('owner');
  $schema['r_smarts'] = setupTable('Who owns what smart device', $fields, 'code', $foreign_keys, $indexes);
/*
  // table r_counts
  $fields = array(
    'type' => setupField('what is being counted', 'varchar 255'),
    'channel' => setupField('through what channel did it happen', 'int 11'),
    'real' => setupField('is it for real (or just asif)', 'int tiny'),
    'count' => setupField('the count', 'int big', 0),
  );
  $schema['r_counts'] = setupTable('How many this and that overall, through the various channels', $fields, array('type', 'channel'));
*/
  
  // table r_scores
  $fields = array(
    'company' => setupField('uid of company', 'int 11'),
    'count' => setupField('for the rTraders that have this many places to buy with rCredits', 'int 11', 0),
    'score' => setupField('this many of those rTraders would buy from this company', 'int 11'),
  );
  $foreign_keys = foreignKey('company');
  $indexes = index('company');
  $schema['r_scores'] = setupTable('Best companies to promote to rTrader in the current round', $fields, NULL, $foreign_keys, $indexes);
  
  /*  
  setupField($schema, 'r_areas', 'Area Code', 'telephone area code', 'char 3'); 
  setupField($schema, 'r_areas', 'Region', 'state, province, or territory', 'varchar 24'); 
  
  setupField($schema, 'r_regions', 'Region', 'state or province', 'char 2');
  setupField($schema, 'r_regions', 'Credit Region', 'credit region id', 'char 2');
  */
  
  return $schema;
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

function setupTable($description, $fields, $primary_key, $foreign_keys = array(), $indexes = array()) {
  $result = compact('description', 'fields', 'indexes') + array('foreign keys' => $foreign_keys);
  return is_null($primary_key) ? $result : ($result + array('primary key' => (is_array($primary_key) ? $primary_key : array($primary_key))));
}

function foreignKey($local, $foreign = 'uid', $table = 'users') {
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
function setupField($description = '', $type = 'varchar 255', $default = '', $SUN = '000') {
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
  
  $scheme = compact(explode(' ', 'description type size length precision default scale serialize unsigned')); // don't use u\ray
  foreach ($scheme as $key => $value) if ($value === '') unset($scheme[$key]);

  $scheme['not null'] = $notnull;
//  ['fields'][$field_name] = $scheme;
  return $scheme;
}
