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
  require_once 'rcredits.inc'; // Drupal bug. hook_schema needs these explicit, both uninstalling and installing.
  require_once 'rcredits-settings.inc';
  require_once 'rcredits-util.inc';
  // fix uid size in all Drupal tables
  foreach (u\ray(DRUPAL_UID_TABLES) as $table) {
    $desc = @$schema[$table]['fields']['uid']['description'] ?: 'users record id';
    $schema[$table]['fields']['uid'] = setupField($desc, 'int big');
  }
  
  // table user (additions to Drupal standard)
  $fields = array( // additional/changed users fields
    'flags' => setupField('permissions and boolean state flags', 'int big', 0, '010'),
    'community' => setupField("uid of this account's Common Good Community", 'int 11'),
    'question' => setupField('security question', 'varchar 255'),
    'usdAccount' => setupField('USD account number (hex of encrypted)', 'varchar 255'), // searchable
    'secure' => setupField('other non-personal encrypted data', 'blob normal'), 
    'offsite' => setupField('index to encrypted data stored off-site', 'int big', NULL, '010'),
    'fullName' => setupField('full name of the individual or entity', 'varchar 60'),
    'phone' => setupField('contact phone (no country code, no punctuation)', 'varchar 255'),
    'faxetc' => setupField('other contact details', 'varchar 255'),
    'address' => setupField('physical street address', 'varchar 60'),
    'city' => setupField('municipality', 'varchar 60'),
    'state' => setupField('state/province index', 'int 5'),
    'postalCode' => setupField('postal code for physical address (no punctuation)', 'varchar 20'),
    'country' => setupField('country index', 'int 4'),
    'postalAddr' => setupField('complete postal address', 'varchar 255'),
    'notes' => setupField('miscellaneous notes about the user or the account', 'text medium'),
    'signed' => setupField('when did this person sign the rCredits Agreement', 'int 11', 0),
    'signedBy' => setupField('who signed the agreement (on behalf of the account)', 'varchar 60'),
    'rebate' => setupField('current rebate percentage (sales bonus is double)', 'numeric 4,3', 5),
    'floor' => setupField('how low can balance go (normally zero or less)', 'numeric 11,2', R_FLOOR),
    'minimum' => setupField('chosen minimum balance (savings + 3 days, fill automatically)', 'numeric 11,2', NULL), // NULL means preferences not set yet
//    'chunk' => setupField('granularity of transfers from bank account', 'numeric 11,2', R_BANK_CHUNK),
//    'maximum' => setupField('maximum rCredits balance (send excess to bank), 0=none)', 'numeric 11,2', -1), // -1 means no limit
    'share' => setupField('percentage of rebates/bonuses to contribute to CGF', 'numeric 6,3', NULL),
    'r' => setupField('rCredits balance, including rewards (cached)', 'numeric 11,2', 0),
    'usd' => setupField('balance in related USD account (probable)', 'numeric 11,2', 0),
    'rewards' => setupField('total incentive rewards to date (cached)', 'numeric 11,2', 0),
    'committed' => setupField('amount committed (for donations to CGF)', 'numeric 11,2', 0),
//    'frozen' => setupField('amount of rCredits currently unavailable until specific dates or indefinitely', 'text medium'),
  );
  // need foreign keys here
  $schema['users']['fields'] = (@$schema['users']['fields'] ?: array()) + $fields;
  $schema['users']['fields']['uid'] = setupField('Unique user ID', 'int big'); // not unsigned
  $schema['users']['fields']['data'] = setupField('serialized array of miscellaneous data', 'text medium');
// kills Drupal: $schema['users']['fields']['data']['description'] = 'incidental data (a serialized array of name value pairs)';

// 'nym' => setupField('unique identifier for cryptographic signing', 'text tiny'), 
//    'demand' => setupField('waiting to trade this much USD for rCredits (or vice-versa)', 'numeric 11,2', 0),
//    'demand_date' => setupField('date/time of request for rCredits (or USD)', 'int 11'),
//    'counts' => setupField('counts of command use, etc.', 'int big', 0),
/*  MOVE THESE TO offsite field:
    'federalId' => setupField('social security number or employer id number, for reporting', 'int 9'), // secret
    'dob' => setupField('date of birth, founding, or incorporation', 'int 11'), // secret
    'idProof' => setupField('proof of identity encryption key', 'varchar ' . R_SECRET_ID_LEN), // secret
    'answer' => setupField('answer to security question', 'varchar ' . R_SECRET_ID_LEN), // secret
  */

  // table r_companies
  $fields = array(
    'uid' => setupField('account record id', 'int big'),
    'website' => setupField('primary website', 'varchar 255'),
    'description' => setupField('markup text describing the company', 'text medium'),
    'selling' => setupField('typical description of what this company sells (default transaction purpose)', 'varchar 255'),
    'structure' => setupField('organizational structure bits', 'int big', 0, '010'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = NULL;
  $schema['r_companies'] = setupTable('Information about non-personal accounts', $fields, 'uid', $foreignKeys, $indexes);

  // table r_stats
  $fields = array(
    'id' => setupField('statistics record id', 'serial big'),
    'ctty' => setupField('community or region record id', 'int big'),
    'r' => setupField('amount of rCredits issued to date', 'numeric 11,2', 0),
    'usd' => setupField('amount of US Dollars in the system', 'numeric 11,2', 0),
    'signups' => setupField('amount of signup bonuses ever', 'numeric 11,2', 0),
    'inflations' => setupField('amount of inflation adjustments ever', 'numeric 11,2', 0),
    'refunds' => setupField('amount of Dwolla fee reimbursements ever', 'numeric 11,2', 0),
    'rebates' => setupField('amount of rebate rewards ever', 'numeric 11,2', 0),
    'bonuss' => setupField('amount of transaction bonus rewards ever', 'numeric 11,2', 0),
    'grants' => setupField('amount of grants ever', 'numeric 11,2', 0), // drupal bug in addfield: this field cannot be named "grant" (gets "Duplicate column name" mysql error)
    'loans' => setupField('amount of loans ever', 'numeric 11,2', 0),
    'fines' => setupField('amount of fines ever', 'numeric 11,2', 0),
    'shared' => setupField('amount of rewards shared ever', 'numeric 11,2', 0),
    'pAccts' => setupField('number of personal accounts', 'int medium'),
    'bAccts' => setupField('number of company accounts', 'int medium'),
    'bankInPerMo' => setupField('amount transferred into the system in the past month', 'numeric 11,2', 0),
    'bankOutPerMo' => setupField('amount transferred out of the system in the past month', 'numeric 11,2', 0),
    'xchPerMo' => setupField('exchange volume within system in the past month', 'numeric 11,2', 0),
    'cashPerMo' => setupField('exchange volume for cash in the past month', 'numeric 11,2', 0),
    'perMo' => setupField('purchase volume in the past month', 'numeric 11,2', 0), // omits r/USD exchanges
    'p2bPerMo' => setupField('customer purchase volume in the past month', 'numeric 11,2', 0),
    'b2bPerMo' => setupField('b2b purchase volume in the past month', 'numeric 11,2', 0),
    'b2pPerMo' => setupField('payroll volume in the past month', 'numeric 11,2', 0),
    'xchTxsPerMo' => setupField('number of exchanges within system in the past month', 'int medium', 0),
    'cashTxsPerMo' => setupField('number of exchanges for cash in the past month', 'int medium', 0),
    'txsPerMo' => setupField('number of purchases in the past month', 'int medium', 0), // omits r/USD exchanges
    'p2bTxsPerMo' => setupField('customer purchases in the past month', 'int medium', 0),
    'b2bTxsPerMo' => setupField('b2b purchases in the past month', 'int medium', 0),
    'b2pTxsPerMo' => setupField('payments to employees and sole proprietors in the past month', 'int medium', 0),
    'created' => setupField('Unixtime record was created', 'int 11', 0),
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_stats'] = setupTable('Operating statistics for commununities and overall', $fields, 'id', $foreignKeys, $indexes);
  
  // table r_boxnames
  $fields = array(
    'id' => setupField('box name record id', 'serial big'),
    'boxid' => setupField('machine id', 'int big'),
    'uid' => setupField('account record id', 'int big'),
    'boxName' => setupField('chosen name for this machine, for this account', 'varchar 255'),
    'created' => setupField('Unixtime record was created', 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_boxnames'] = setupTable('Names for devices', $fields, 'id', $foreignKeys, $indexes);
  
  // table offsite
  $fields = array(
    'id' => setupField('key to secret data', 'int big', 0, '010'),
    'data' => setupField('true value of some secret data field on some other server (encrypted)', 'text medium'), // blob for encrypted data (up to 255*255 chars)
  );
//  $indexes = index('data');
  $schema['offsite'] = setupTable('secret data on some other server', $fields, 'id', NULL, NULL);

  // table r_do
  $fields = array(
    'doid' => setupField('record id', 'serial 11'),
    'uid' => setupField('account record id', 'int big'),
    'box' => setupField('machine id', 'int big'),
    'created' => setupField('Unixtime record was created', 'int 11', 0),
    'completed' => setupField('Unixtime action was completed', 'int 11', 0), // expiration time if <0
    'action' => setupField('serialized array describing action to take', 'text medium'),
    'code' => setupField('security code', 'varchar 255'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_do'] = setupTable('Clickable actions with no signin', $fields, 'doid', $foreignKeys, $indexes);
    
  // table r_proxies
  $fields = array(
    'id' => setupField('record id', 'serial 11'),
    'person' => setupField('account record id', 'int big'),
    'proxy' => setupField('account record id of proxy', 'int big'),
    'priority' => setupField('precedence of this proxy (1=top priority)', 'int tiny'),
  );
  $foreignKeys = foreignKey('person') + foreignKey('proxy');
  $indexes = index('person');
  $schema['r_proxies'] = setupTable('Who represents whom', $fields, 'id', $foreignKeys, $indexes);
  
  // table r_sms 
  $fields = array(
    'number' => setupField('international phone number (eg +12223334444)', 'varchar 16'),
    'uid' => setupField('uid of cell phone owner (one owner per number)', 'int big'),
    'status' => setupField('phone status', 'int tiny'),
    'todo' => setupField('waiting for confirmation to complete this operation', 'text medium'),
    'nonce' => setupField('waiting for this nonce, for confirmation', 'varchar 10'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_sms'] = setupTable('SMS cell numbers corresponding to user accounts', $fields, 'number', $foreignKeys, $indexes);
  
  // table r_txs
  $fields = array(
    'xid' => setupField('the unique transaction ID', 'serial 11'), 
    'serial' => setupField('serial number of related transactions (=xid of first transaction in the group)', 'int 11'), 
    'usdXid' => setupField('the unique transaction ID for USD part', 'varchar 255'), 
    'type' => setupField('transaction type (transfer, rebate, etc.)', 'int tiny'),
    'taking' => setupField('was the transaction initiated by the payee', 'int tiny'), 
    'goods' => setupField('is this transfer an exchange for real goods and services?', 'int tiny', 0), 
    'state' => setupField('done, pending, disputed, etc', 'int tiny'), 
    'amount' => setupField('amount in rCredits', 'numeric 11,2'), 
    'r' => setupField('amount in rCredits', 'numeric 11,2'), 
    'payer' => setupField('user id of the payer', 'int big'), 
    'payee' => setupField('user id of the payee', 'int big'), 
    'payerAgent' => setupField("user id of payer's agent (who approved this transaction for the payer)", 'int big'), 
    'payeeAgent' => setupField("user id of payee's agent (who approved this transaction for the payee)", 'int big'), 
    'payerFor' => setupField("payer's description", 'varchar 60'), 
    'payeeFor' => setupField("payee's description", 'varchar 60'), 
    'payerTid' => setupField("payer's transaction ID", 'int 11'),
    'payeeTid' => setupField("payee's transaction ID", 'int 11'),
    'data' => setupField('info stored elsewhere, duplicated here for convenience (serialized array)', 'text big'), 
    'channel' => setupField('through what medium was the transaction entered', 'int tiny'),
    'box' => setupField('on what machine was the transaction entered', 'int 11', 0), // 0 = none
    'created' => setupField('Unixtime transaction was created', 'int 11', 0),
    'completed' => setupField('Unixtime transaction was completed', 'int 11', 0),
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee') + foreignKey('payerAgent') + foreignKey('payeeAgent');
  $indexes = index('payer') + index('payee') + index('created') + index('completed'); // also serial/type/taking/goods/state/payerAgent/payeeAgent/channel/?
  $schema['r_txs'] = setupTable('Record of all rCredits transactions in the region', $fields, 'xid', $foreignKeys, $indexes);

//  foreach (u\ray('type taking goods payerAgent payeeAgent payerFor payeeFor') as $one) unset($fields[$one]);

  // table r_usd
  $fields = array(
    'txid' => setupField('the unique transaction ID', 'varchar 255'), 
    'tid' => setupField("payer's transaction ID", 'int 11'), // used only for bank transfers
    'amount' => setupField('amount in rCredits', 'numeric 11,2'), 
    'payer' => setupField('user id of the payer', 'int big'), 
    'payee' => setupField('user id of the payee', 'int big', 0), // 0 for bank transfers
    'created' => setupField('Unixtime transaction was created', 'int 11', 0),
    'completed' => setupField('Unixtime transaction was completed', 'int 11', 0), // for deposits / withdrawals
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee');
  $indexes = index('created');
  $schema['r_usd'] = setupTable('Record of USD (Dwolla) transactions in the region', $fields, 'txid', $foreignKeys, $indexes);

  // table r_usderror
  $fields = array(
    'id' => setupField('record ID', 'serial 11'),
    'error' => setupField('error message from USD server', 'varchar 255'),
    'op' => setupField('operation that caused the error', 'varchar 60'),
    'ok' => setupField('is it okay to get this error?', 'int tiny', 0),
    'created' => setupField('Unixtime error happened', 'int 11', 0),
  );
  $foreignKeys = array();
  $indexes = array();
  $schema['r_usderror'] = setupTable('USD error messages from USD gateway', $fields, 'id', $foreignKeys, $indexes);
  
  // table r_gifts
  $fields = array(
    'donid' => setupField('gift record id', 'serial 11'),
    'giftDate' => setupField('date/time of gift', 'int 11', 0),
    'completed' => setupField('Unixtime donation was completed', 'int 11', 0),
    'uid' => setupField('uid of account that made the gift', 'int big'),
    'amount' => setupField('amount of gift', 'numeric 11,2'),
    'often' => setupField('recurring how often (Y, Q, M, 1)', 'varchar 1'),
    'honor' => setupField('what type of honor', 'varchar 10'), // "share" = gift is "sharing" rewards with CGF
    'honored' => setupField('who is honored', 'text medium'),
    'share' => setupField('percentage of rebates/bonuses to contribute to CGF', 'numeric 6,3'), // -1 = /A
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_gifts'] = setupTable('Membership gift details', $fields, 'donid', $foreignKeys, $indexes);

  // table r_log
  $fields = array(
    'logid' => setupField('log record id', 'serial 11'),
    'time' => setupField('date/time logged', 'int 11', 0),
    'channel' => setupField('logged from what interface module', 'int tiny'),
    'type' => setupField('what type of log entry', 'varchar 60'),
    'myid' => setupField('current account uid', 'int big'),
    'agent' => setupField('agent account uid', 'int big'),
    'info' => setupField('arbitrary serialized data', 'text medium'),
    'special' => setupField('special value if any', 'text medium'), // eg cell number, nonce, or sql
  );
  $foreignKeys = foreignKey('myid') + foreignKey('agent');
  $indexes = index('type') + index('channel') + index('myid') + index('agent');  
  $schema['r_log'] = setupTable('Development and error log', $fields, 'logid', $foreignKeys, $indexes);
  
  // table r_demand
  /*
  $fields = array(
    'id' => setupField('demand record id', 'serial 11'),
    'time' => setupField('date/time requested', 'int 11'),
    'amount' => setupField('request amount', 'numeric 11,2'), 
    'who' => setupField('uid of the account requesting rCredits', 'int 11'),
  );
  $foreignKeys = foreignKey('who');
  $indexes = index('who');
  $schema['r_demand'] = setupTable('Who requested rCredits when', $fields, 'id', $foreignKeys, $indexes);
  */
  
  // table r_relations
  $fields = array(
    'reid' => setupField('relationship record id', 'serial 11'),
    'main' => setupField('uid of the account to which others are related', 'int big'),
    'other' => setupField('uid of a user related to that account', 'int big'), // eg owner, agent, employee, supplier
    'amount' => setupField('average monthly amount paid to other', 'numeric 11,2', 0), // typically to employee or supplier
    'draw' => setupField('whether credit can flow from main to agent', 'int tiny', 0),
    'permission' => setupField('what type of permission the agent has on the main account', 'int tiny', 0),
    'employerOk' => setupField('employer says this agent is an employee', 'int tiny', 0),
    'employeeOk' => setupField('employee says this agent is an employee', 'int tiny', 0),
    'isOwner' => setupField('this agent is an owner (or part owner)', 'int tiny', 0),
  );
  $foreignKeys = foreignKey('main') + foreignKey('other') + foreignKey('foreignUid');
  $indexes = index('main') + index('other');
  $schema['r_relations'] = setupTable('Who can manage which accounts, and how', $fields, 'reid', $foreignKeys, $indexes);
    
  // table r_candidates (almost same as r_relations, but with extra fields -- used for candidates in rcredits-circles.inc)
  /*
  $fields = array(
    'uid' => setupField('candidate record id', 'serial 11'),
    'isCompany' => setupField('is this a company', 'int tiny', 0),
    'ignor' => setupField('temporarily omit this record from consideration', 'int tiny', 0), // IGNORE is reserved in MySQL
    'score' => setupField('how good is this candidate', 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid');
  $schema['r_candidates'] = setupTable('Candidates to become rTraders', $fields, 'uid', $foreignKeys);
  */
  
/*
  $fields = array(
    'ignore' => setupField('temporarily omit this record from consideration', 'int tiny'),
    'employeeIsRTrader' => setupField('this employee is an rTrader', 'int tiny'),
    'score' => setupField('how good is this candidate', 'int 11'),
  );
  $schema['r_candidates'] = $schema['r_relations'];
  $schema['r_candidates']['fields'] += $fields;
  $schema['r_candidates']['description] = 'Candidates to become rTraders';
*/

  // table r_smarts
  $fields = array(
    'code' => setupField('permanent secret identifier for the device', 'varchar 255'),
    'owner' => setupField('uid of the smart device owner', 'int big'),
    'defaultAgent' => setupField('uid of the default user of the device', 'int big'),
    'device' => setupField('human-readable identifier for the smart device', 'varchar 60'),
    'transientId' => setupField('once-use identifier displayed on the device as a QR for the current account/agent', 'varchar 255'),
    'access' => setupField('date/time last used', 'int 11', 0), // to trigger deletion after several months
  );
  $foreignKeys = foreignKey('owner') + foreignKey('defaultAgent');
  $indexes = index('owner');
  $schema['r_smarts'] = setupTable('Who owns what smart device', $fields, 'code', $foreignKeys, $indexes);
  
  // table r_invites
  $fields = array(
    'code' => setupField('secret invitation code', 'varchar 20'),
    'email' => setupField('email of invitee', 'varchar 60'),
    'inviter' => setupField('uid of inviting member', 'int big'),
    'invitee' => setupField('uid of invited new member', 'int big'), // null if not accepted yet
    'invited' => setupField('date of invitation', 'int 11', 0),
    'subject' => setupField('email subject', 'varchar 255'),
    'message' => setupField('email message body', 'text medium'),
  );
  $foreignKeys = foreignKey('inviter') + foreignKey('invitee');
  $indexes = index('inviter');
  $schema['r_invites'] = setupTable('Who invited whom', $fields, 'code', $foreignKeys, $indexes);
  
  // table r_notices
  $fields = array(
    'msgid' => setupField('notice record id', 'serial 11'),
    'uid' => setupField('uid of member notified', 'int big'),
    'created' => setupField('date of notice', 'int 11', 0),
    'sent' => setupField('date sent (0 if not sent yet)', 'int 11', 0),
    'message' => setupField('the notice text', 'text medium'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_notices'] = setupTable('Message digest buffer', $fields, 'msgid', $foreignKeys, $indexes);
  
  // table r_nonmembers (use -make instead)
/*  $fields = array(
    'id' => setupField('non-member company record id', 'serial 11'),
    'company' => setupField('company name', 'varchar 60'),
    'potential' => setupField('number of members who shop there', 'int 6', 0),
  );
  $schema['r_nonmembers'] = setupTable('Local companies we want to recruit', $fields, 'id', NULL, NULL);
  */
  
/*
  // table r_counts
  $fields = array(
    'type' => setupField('what is being counted', 'varchar 255'),
    'channel' => setupField('through what channel did it happen', 'int 11'),
    'count' => setupField('the count', 'int big', 0),
  );
  $schema['r_counts'] = setupTable('How many this and that overall, through the various channels', $fields, array('type', 'channel'));
*/
  
  // table r_scores
  /*
  $fields = array(
    'company' => setupField('uid of company', 'int 11'),
    'count' => setupField('for the rTraders that have this many places to buy with rCredits', 'int 11', 0),
    'score' => setupField('this many of those rTraders would buy from this company', 'int 11'),
  );
  $foreignKeys = foreignKey('company');
  $indexes = index('company');
  $schema['r_scores'] = setupTable('Best companies to promote to rTrader in the current round', $fields, NULL, $foreignKeys, $indexes);
  */
  
  /*  
  setupField('r_areas', 'Area Code', 'telephone area code', 'char 3'); 
  setupField('r_areas', 'Region', 'state, province, or territory', 'varchar 24'); 
  
  setupField('r_regions', 'Region', 'state or province', 'char 2');
  setupField('r_regions', 'Credit Region', 'credit region id', 'char 2');
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

function setupTable($description, $fields, $primary_key, $foreignKeys = array(), $indexes = array()) {
  $result = compact('description', 'fields', 'indexes') + array('foreign keys' => $foreignKeys);
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

/**
 * Create country and state dropdowns.
 * This should be called just once, offline, when country/state tables change, then upload it.
 */
function makeGeoDropdowns() {
  if (!isDEV) return;
  $js = 'var cs = new Array();'; // countries
  $js .= "\n\nvar ss = new Array();"; // states
  $cq = \db_query('SELECT * FROM r_countries');
  while ($crow = $cq->fetchAssoc()) {
    $cid = $crow['id']; $cname = addslashes($crow['name']);
    $js .= "\n\ncs[$cid] = '$cname';";
    $js .= "\n\nss[$cid] = new Array();";
    $sq = dbQ('SELECT * FROM r_states WHERE country_id=:cid', compact('cid'));
    while ($srow = $sq->fetchAssoc()) {
      $sid = $srow['id']; $sname = addslashes($srow['name']);
      $js .= "\nss[$cid][$sid] = '$sname';";
    }
  }

  $js .= u\SUBS(<<<EOF
\n
function print_country(dft_country, dft_state){
  if (dft_country == "") {
    dft_country = :R_COUNTRY_ID;
    dft_state = :R_STATE_ID;
  }
  var options = document.getElementById('edit-country');
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in cs) {
    options.options[i] = new Option(cs[x], x);
    if (dft_country == x) options.selectedIndex = i;
    i++;
  }
  print_state(options[options.selectedIndex].value, dft_state);
}

function print_state(ci, dft_state){
  var options = document.getElementById('edit-state');
  options.length=0; // zap any previous list items
  var x, i = 0;
  for(x in ss[ci]) {
    options.options[i] = new Option(ss[ci][x], x);
    if (dft_state == x) options.selectedIndex = i;
    i++;
  }
}
EOF
  );
  file_put_contents(__DIR__ . '/inc/countries.js', $js);
}

/**
 * Handle adjustments to previous versions of this software
 * @param string $version: the version number (eg 1.0)
 * @param string $when: run "before" or "after" all other installation tasks, or on "uninstall"
 */
function oldVersion($version, $when) {
  require_once 'rcredits.inc'; // must be explicit for dbLookup()
  require_once 'rcredits-util.inc'; // must be explicit for dbLookup()

  //  if ($version >= '1.0b') return;
  
}