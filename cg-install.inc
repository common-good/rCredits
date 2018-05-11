<?php

namespace CG\Install;
use CG as r;
use CG\Util as u;
use CG\DB as db;

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
function tableDefs($schema = []) {
  require_once 'cg.inc'; // Drupal bug. hook_schema needs these explicit, both uninstalling and installing.
  require_once 'cg-settings.inc';
  require_once 'cg-util.inc';
  // fix uid size in all Drupal tables
  foreach (ray(DRUPAL_UID_TABLES) as $table) {
    $desc = @$schema[$table]['fields']['uid']['description'] ?: 'users record id';
    $schema[$table]['fields']['uid'] = setupField($desc, 'int big');
  }
  
  // table users
  $fields = array(
    // standard Drupal fields
    'uid' => setupField(t('account record ID'), 'int big'),
    'name' => setupField(t('unique user name'), 'varchar 60', ''),
    'pass' => setupField(t('account password'), 'varchar 128', ''),
    'mail' => setupField(t('encrypted email address'), 'varchar 254', ''),
    'created' => setupField(t('Unix date and time record was created'), 'int 11', 0),
    'access' => setupField(t('Unix date and time account was last accessed'), 'int 11', 0),
    'login' => setupField(t('Unix date and time user last signed in'), 'int 11', 0),
    'picture' => setupField(t('used for temporary storage when generating statistics'), 'int 11', 0),
    'data' => setupField(t('serialized associative array of miscellaneous fields (not encrypted)'), 'text big'),
    
    // additions to Drupal fields
    'flags' => setupField(t('boolean permissions and state flags'), 'int big', 0, TRUE),
    'jid' => setupField(t('account ID of joined account (0 if none)'), 'int big', 0),
    'changes' => setupField(t('changes made to the account'), 'blob big'),
    'community' => setupField("uid of this account's Common Good Community", 'int big'),
//    'question' => setupField(t('security question'), 'varchar 255'),
//    'usdAccount' => setupField(t('USD (Dwolla) account number (hex of encrypted)'), 'varchar 255'), // searchable
    'secure' => setupField(t('encrypted data'), 'blob medium'), 
    'vsecure' => setupField(t('hyper-encrypted data'), 'blob'), 

//    'offsite' => setupField(t('index to encrypted data stored off-site'), 'int big', NULL, TRUE),
    'fullName' => setupField(t('full name of the individual or entity'), 'varchar 255'),
    'phone' => setupField(t('contact phone (no country code, no punctuation)'), 'varchar 255'), // encrypt/searchable
//    'faxetc' => setupField(t('other contact details'), 'varchar 255'),
//    'address' => setupField(t('physical street address'), 'varchar 60'),
    'city' => setupField(t('municipality'), 'varchar 60'),
    'state' => setupField(t('state/province index'), 'int 5'),
    'zip' => setupField(t('postal code for physical address (no punctuation)'), 'varchar 255'),
    'country' => setupField(t('country index'), 'int 4'),
    'notes' => setupField(t('miscellaneous notes about the user or the account'), 'text big'),
    'tickle' => setupField(t('Unixtime to tickle an admin about this account'), 'int 11', 0),
    'activated' => setupField(t('when was the account activated'), 'int 11', 0),
    'helper' => setupField(t('who invited this person or company'), 'int big'),
    'iCode' => setupField(t('sequence number of helper invitation'), 'int'),
    'signed' => setupField(t('when did this person sign the %PROJECT Agreement'), 'int 11', 0),
    'signedBy' => setupField(t('who signed the agreement (on behalf of the account)'), 'varchar 60'),
    'rebate' => setupField(t('current rebate percentage (sales bonus is proportionate)'), 'numeric 5,3', R_REBATE),
    'savingsAdd' => setupField(t('chosen amount to hold as savings, beyond rewards'), 'numeric 11,2', 0),
    'saveWeekly' => setupField(t('chosen amount to increase savingsAdd by, weekly'), 'numeric 11,2', 0),
    'floor' => setupField(t('negative credit line'), 'numeric 11,2', R_FLOOR),
    'minimum' => setupField(t('chosen target balance (for automatic refills)'), 'numeric 11,2', NULL), // NULL means preferences not set yet
//    'chunk' => setupField(t('granularity of transfers from bank account'), 'numeric 11,2', R_BANK_CHUNK),
//    'maximum' => setupField(t('maximum balance (send excess to bank)'), 'numeric 11,2', -1), // -1 means no limit (unused)
    'share' => setupField(t('percentage of rebates/bonuses to donate to CG'), 'numeric 6,3', 0),
    'crumbs' => setupField(t('percentage of each transaction to donate to CG (half to community)'), 'numeric 6,3', NULL),
    'balance' => setupField(t('balance, not including rewards (cached)'), 'numeric 11,2', 0),
//    'r' => setupField(t('balance, including rewards (cached)'), 'numeric 11,2', 0),
//    'usd' => setupField(t('balance in related USD account (probable)'), 'numeric 11,2', 0),
    'rewards' => setupField(t('total incentive rewards to date (cached)'), 'numeric 11,2', 0),
    'committed' => setupField(t('amount committed (for donations to CGF)'), 'numeric 11,2', 0),
//    'frozen' => setupField(t('amount of rCredits currently unavailable until specific dates or indefinitely'), 'text medium'),
//    'photo' => setupField(t('member photo'), 'blob big'), // encrypted
    'risk' => setupField(t('today\'s suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => setupField(t('list of risk factors'), 'int big', 0, TRUE),
    'trust' => setupField(t('how much this person is trusted by others in the community'), 'float medium', NULL),
    'stats' => setupField(t('account statistics'), 'text medium'),
    'notices' => setupField(t('when to send what kind of notice'), 'text tiny'),
    'lastip' => setupField(t('latest IP address used'), 'varchar 39'),
//    'members' => isGAME ? setupField(t('number of members represented'), 'int medium', 1) : NULL,
    'special' => setupField(t('special transient data'), 'text big'),
  );
  // need foreign keys here
//  $schema['users']['fields']['uid'] = setupField(t('Unique user ID'), 'int big'); // not unsigned (must be first field)
//  $schema['users']['fields']['data'] = setupField(t('serialized array of miscellaneous data'), 'text big');
//  $schema['users']['fields'] = $fields;
  $foreignKeys = foreignKey('community');
  $indexes = index('name') + index('mail') + index('created') + index('community') + index('zip');
  $schema['users'] = setupTable(t('Account data'), $fields, 'uid', $foreignKeys, $indexes);
// kills Drupal: $schema['users']['fields']['data']['description'] = 'incidental data (a serialized array of name value pairs)';

  // table sessions (additions to Drupal standard)
  $fields = array( // additional/changed sessions fields
    'acct' => setupField(t('currently viewing/managing this account ID'), 'int big'),
  );
  $schema['sessions']['fields'] = (@$schema['sessions']['fields'] ?: []) + $fields;

  // table r_photos (separate from user table for efficiency -- this table gets big)
  $fields = array(
    'uid' => setupField(t('account record id'), 'int big'),
    'photo' => setupField(t('member photo'), 'blob big'), // encrypted for members, filename for companies
  );
  $foreignKeys = foreignKey('uid');
  $indexes = [];
  $schema['r_photos'] = setupTable(t('one photo for each account'), $fields, 'uid', $foreignKeys, $indexes);

  // table r_notes (NO. use users fields instead)
/*  $fields = array(
    'id' => setupField(t('record id'), 'serial big'),
    'uid' => setupField(t('id of account the note is about'), 'int big'),
    'agent' => setupField(t('id of account for the note writer'), 'int big'),
    'tickle' => setupField(t('Unixtime to tickle an admin about this note (or negative the date/time completed)'), 'int 11'),
    'created' => setupField(t('Unixtime record was created'), 'int 11'),
  );
  $foreignKeys = [];
  $indexes = [];
  $schema['r_notes'] = setupTable('notes about accounts'), $fields, 'id', $foreignKeys, $indexes);
  */
  
  // table queue
  $fields = array(
    'id' => setupField(t('primary key: Unique item ID'), 'serial big'),
    'item' => setupField(t('arbitrary data for the item.'), 'blob big'),
    'created' => setupField(t('Unixtime record was created'), 'int 11'),
  );
  $foreignKeys = [];
  $indexes = [];
  $schema['queue'] = setupTable(t('Cron queue'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_stats
  $fields = array(
    'id' => setupField(t('statistics record id'), 'serial big'),
    'created' => setupField(t('Unixtime record was created'), 'int 11', 0),
    
    'ctty' => setupField(t('community or region record id'), 'int big'),
    'pAccts' => setupField(t('number of personal accounts'), 'int medium', 0),
    'bAccts' => setupField(t('number of company accounts'), 'int medium', 0),
    'newbs' => setupField(t('number of not-yet-active accounts'), 'int medium', 0),
    'aAccts' => setupField(t('number of active personal accounts'), 'int medium', 0),
    'conx' => setupField(t('number of connections per personal account'), 'numeric 10,3', 0),
    'conxLocal' => setupField(t('number of local connections per personal account'), 'numeric 10,3', 0),
    
    'balsPos' => setupField(t('amount of positive balances'), 'numeric 11,2', 0),
    'balsNeg' => setupField(t('amount of negative balances'), 'numeric 11,2', 0),
    'balsPosCount' => setupField(t('number of positive balances'), 'int medium', 0),
    'balsNegCount' => setupField(t('number of negative balances'), 'int medium', 0),
    'topN' => setupField(t('top N or N% of balances, whichever is greater'), 'numeric 11,2', 0),
    'botN' => setupField(t('bottom N or N% of balances, whichever is less'), 'numeric 11,2', 0),
    'floors' => setupField(t('credit lines'), 'numeric 11,2', 0),
    
    'p2b' => setupField(t('customer purchase volume'), 'numeric 11,2', 0),
    'b2b' => setupField(t('b2b purchase volume'), 'numeric 11,2', 0),
    'b2p' => setupField(t('payroll volume'), 'numeric 11,2', 0),
    'p2p' => setupField(t('person-to-person purchase volume'), 'numeric 11,2', 0),
    'p2bCount' => setupField(t('customer purchase volume'), 'int medium', 0),
    'b2bCount' => setupField(t('b2b purchase volume'), 'int medium', 0),
    'b2pCount' => setupField(t('payroll volume'), 'int medium', 0),
    'p2pCount' => setupField(t('person-to-person purchase volume'), 'int medium', 0),
    'cashs' => setupField(t('amount of exchanges for cash'), 'numeric 11,2', 0),
    'cashsCount' => setupField(t('number of exchanges for cash'), 'int medium', 0),
    
    'cgIn' =>setupField(t('amount of %RCREDITS coming into this community'), 'numeric 11,2', 0),
    'cgOut' =>setupField(t('amount of %RCREDITS leaving this community'), 'numeric 11,2', 0),
    'cgInCount' =>setupField(t('number of transfers into this community'), 'int medium', 0),
    'cgOutCount' =>setupField(t('number of transfers out of this community'), 'int medium', 0),
    
    'usdIn' => setupField(t('amount of US Dollars brought into the system'), 'numeric 11,2', 0),
    'usdOut' => setupField(t('amount of US Dollars taken out of the system'), 'numeric 11,2', 0),
    'usdInCount' => setupField(t('number of incoming bank transfers'), 'int medium', 0),
    'usdOutCount' => setupField(t('number of outgoing bank transfers'), 'int medium', 0),
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_stats'] = setupTable(t('Operating statistics for communities and overall'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_boxes
  $fields = array(
    'id' => setupField(t('device record id'), 'serial big'),
    'channel' => setupField(t('channel'), 'int tiny'), // web browser, rPOS app, or SMS
    'code' => setupField(t('device id'), 'varchar 255'), // device identifier (unique to each device/type combo)
    'boxnum' => setupField(t('sequential device number for the account'), 'int 11', 0),
    'uid' => setupField(t('account record id'), 'int big'),
    'boxName' => setupField(t('member\'s chosen name for this device, for this account'), 'varchar 255'),
    'todo' => setupField(t('waiting for confirmation to complete this operation'), 'text medium'),
    'nonce' => setupField(t('waiting for this nonce, for confirmation'), 'varchar 255'), // also transient QR ID
//    'restricted' => setupField(t('permit no new users of this device'), 'int tiny', 0),
    'access' => setupField(t('date/time last used'), 'int 11', 0), // to trigger deletion after several months
    'created' => setupField(t('Unixtime record was created'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid') + index('boxnum');
  $schema['r_boxes'] = setupTable(t('Devices info1'), $fields, 'id', $foreignKeys, $indexes);

  // table r_bad
  $fields = array(
    'qid' => setupField(t('phoney customer qid'), 'varchar 255'),
    'code' => setupField(t('phoney card security code'), 'varchar 255'),
    'created' => setupField(t('Unixtime record was created'), 'int 11'),
  );
  $foreignKeys = [];
  $indexes = [];
  $schema['r_bad'] = setupTable(t('lost, stolen, or faked rCard codes'), $fields, 'created', $foreignKeys, $indexes);
  
  // table r_do
  $fields = array(
    'doid' => setupField(t('record id'), 'serial'),
    'expires' => setupField(t('Unixtime expiration'), 'int 11', 0), // 0 means never expires
    'data' => setupField(t('serialized array of parameters'), 'text medium'),
    'uid' => setupField(t('related account record ID'), 'int big', 0), // needed only for changeUid
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_do'] = setupTable(t('Clickable actions with no signin'), $fields, 'doid', $foreignKeys, $indexes);
    
  // table r_proxies
  $fields = array(
    'id' => setupField(t('record id'), 'serial'),
    'person' => setupField(t('account record id'), 'int big'),
    'proxy' => setupField(t('account record id of proxy'), 'int big'),
    'priority' => setupField(t('precedence of this proxy (1=top priority)'), 'int tiny'),
  );
  $foreignKeys = foreignKey('person') + foreignKey('proxy');
  $indexes = index('person');
  $schema['r_proxies'] = setupTable(t('Who represents whom'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_txs
  $fields = array(
    'xid' => setupField(t('the unique transaction ID'), 'serial big'), 
    'serial' => setupField(t('serial number of related transactions (=xid of first transaction in the group)'), 'int 11'), 
    'type' => setupField(t('transaction type (transfer, rebate, etc.)'), 'int tiny'),
//    'taking' => setupField(t('was the transaction initiated by the payee'), 'int tiny'), 
    'goods' => setupField(t('is this transfer an exchange for real goods and services?'), 'int tiny', 0), 
    'amount' => setupField(t('amount transferred'), 'numeric 11,2'), 
    'payer' => setupField(t('user id of the payer'), 'int big'), 
    'payee' => setupField(t('user id of the payee'), 'int big'), 
    'payerAgent' => setupField("user id of payer's agent (who approved this transaction for the payer)", 'int big'), 
    'payeeAgent' => setupField("user id of payee's agent (who approved this transaction for the payee)", 'int big'), 
    'payerFor' => setupField("payer's description", 'varchar 255'), 
    'payeeFor' => setupField("payee's description", 'varchar 255'), 
    'payerReward' => setupField(t('incentive reward for payer'), 'numeric 11,2', 0), 
    'payeeReward' => setupField(t('incentive reward for payee'), 'numeric 11,2', 0), 
    'payerTid' => setupField("payer's transaction ID", 'int 11'),
    'payeeTid' => setupField("payee's transaction ID", 'int 11'),
    'data' => setupField(t('miscellaneous non-searchable data (serialized array)'), 'text big'), 
    'flags' => setupField(t('boolean characteristics and state flags'), 'int big', 0, TRUE),
    'channel' => setupField(t('through what medium was the transaction entered'), 'int tiny'),
    'box' => setupField(t('on what machine was the transaction entered'), 'int 11', 0), // 0 = none
    'created' => setupField(t('Unixtime transaction was created'), 'int 11', 0),
    'risk' => setupField(t('suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => setupField(t('list of risk factors'), 'int big', 0, TRUE),
//    'members' => isGAME ? setupField(t('number of members represented'), 'int medium', 1) : NULL,
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee') + foreignKey('payerAgent') + foreignKey('payeeAgent');
  $indexes = index('payer') + index('payee') + index('created') + index('payerTid') + index('payeeTid'); // also serial/type/goods/payerAgent/payeeAgent/channel/?
  $schema['r_txs'] = setupTable(t('Record of all transactions in the region'), $fields, 'xid', $foreignKeys, $indexes);

  // table r_invoices
  $fields = array(
    'nvid' => setupField(t('the unique invoice ID'), 'serial big'), 
    'status' => setupField(t('transaction record ID or status (pending or denied)'), 'int 11', TX_PENDING), // positive values are record IDs in r_txs (for paid invoices), non-positive values are status
    'amount' => setupField(t('amount to charge'), 'numeric 11,2'), 
    'payer' => setupField(t('user id of the payer'), 'int big'), 
    'payee' => setupField(t('user id of the payee'), 'int big'), 
    'goods' => setupField(t('is this an invoice for real goods and services?'), 'int tiny', 0), 
    'purpose' => setupField("payee's description", 'text big'), 
// not needed yet    'flags' => setupField(t('boolean characteristics and state flags'), 'int big', 0, TRUE),
    'data' => setupField(t('miscellaneous non-searchable data (serialized array)'), 'text big'), 
    'created' => setupField(t('Unixtime invoice was created'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee');
  $indexes = index('payer') + index('payee') + index('created') + index('status');
  $schema['r_invoices'] = setupTable(t('Record of all invoices in the region'), $fields, 'nvid', $foreignKeys, $indexes);
  
//  foreach (ray('type taking goods payerAgent payeeAgent payerFor payeeFor') as $one) unset($fields[$one]);

  // table r_usd
  $fields = array(
    'txid' => setupField(t('the unique transaction ID'), 'serial big'), 
    'tid' => setupField("payer's transaction ID", 'int 11'), // set, but currently UNUSED
    'amount' => setupField(t('amount of transfer'), 'numeric 11,2'), 
    'payee' => setupField(t('CG account record ID'), 'int big'), 
    'created' => setupField(t('Unixtime transaction was created'), 'int 11', 0),
    'completed' => setupField(t('Unixtime transaction was completed'), 'int 11', 0), // for deposits / withdrawals
    'deposit' => setupField(t('Unixtime transfer check was printed and deposited'), 'int 11', 0),
    'bankAccount' => setupField(t('Bank account for the transfer'), 'blob'), // "blob tiny" does not exist
    'risk' => setupField(t('suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => setupField(t('list of risk factors'), 'int big', 0, TRUE),
    'channel' => setupField(t('through what medium was the transaction entered'), 'int tiny'),
//    'members' => isGAME ? setupField(t('number of members represented'), 'int medium', 1) : NULL,
  );
  $foreignKeys = foreignKey('payee');
  $indexes = index('created') + index('payee') + index('deposit');
  $schema['r_usd'] = setupTable(t('Record of transfers to or from a bank account'), $fields, 'txid', $foreignKeys, $indexes);

  // table r_banks
  $fields = array(
    'route' => setupField(t('routing number'), 'int 9'),
    'branch' => setupField(t('is this a branch office'), 'int tiny'),
    'fedRoute' => setupField(t('routing number of servicing Fed'), 'int 9'),
    'type' => setupField(t('bank type'), 'int tiny'), // 0=fed bank 1=use routing 2=use newRouting
    'modified' => setupField(t('date modified'), 'char 6'),
    'newRoute' => setupField(t('new routing number'), 'int 9'),
    'name' => setupField(t('bank name'), 'varchar 36'),
    'address' => setupField(t('bank address'), 'varchar 36'),
    'city' => setupField(t('bank city'), 'varchar 20'),
    'state' => setupField(t('bank state'), 'char 2'),
    'zip' => setupField(t('bank zipcode'), 'char 9'),
    'phone' => setupField(t('bank phone'), 'char 10'),
    'status' => setupField(t('status'), 'char 1'), // 1
    'view' => setupField(t('status'), 'char 1'), // 1
  );
  $foreignKeys = [];
  $indexes = index('newroute');
  $schema['r_banks'] = setupTable(t('Bank routing numbers'), $fields, 'route', $foreignKeys, $indexes);
  
  // table r_stakes
  $fields = array(
    'stakeid' => setupField(t('record ID'), 'serial big'),
    'uid' => setupField(t('member record ID'), 'int big'), 
    'clubid' => setupField(t('investment club record ID'), 'int big'), 
    'stake' => setupField(t('member stake in the club'), 'numeric 11,2', 0),
    'request' => setupField(t('request to change stake by this amount'), 'numeric 11,2', 0),
    'joined' => setupField(t('when this member joined the club'), 'int big'),
  );
  $foreignKeys = foreignKey('uid') + foreignKey('clubid');
  $indexes = index('uid');
  $schema['r_stakes'] = setupTable(t('member stakes in an investment club'), $fields, 'stakeid', $foreignKeys, $indexes);
  
  // table r_shares
  $fields = array(
    'shid' => setupField(t('record ID'), 'serial big'),
    'vestid' => setupField(t('investment record ID'), 'int'), 
    'shares' => setupField(t('club shares in the investment'), 'int 11'), 
    'sell' => setupField(t('number of shares to sell ASAP'), 'int 11'),
    'bought' => setupField(t('Unixtime investment made'), 'int 11'),
    'sold' => setupField(t('Unixtime investment totally sold'), 'int 11'),
  );
  $foreignKeys = foreignKey('vestid');
  $indexes = index('vestid');
  $schema['r_shares'] = setupTable(t('club stakes in investments'), $fields, 'shid', $foreignKeys, $indexes);
  
  // table r_investments
  $fields = array(
    'vestid' => setupField(t('record ID'), 'serial big'),
    'coid' => setupField(t('member company record ID'), 'int big'), 
    'clubid' => setupField(t('investment club record ID'), 'int big'), 
    'proposedBy' => setupField(t('account record ID of proposer'), 'int big'),
    'investment' => setupField(t('description of investment'), 'text medium'),
    'return' => setupField(t('predicted or actual APR'), 'numeric 10,3'), // see flags
    'terms' => setupField(t('investment terms'), 'text'),
    'assets' => setupField(t('company assets, bond, or collateral'), 'numeric 11,2'), 
    'offering' => setupField(t('size of offering'), 'numeric 11,2'), 
    'price' => setupField(t('price per share'), 'numeric 11,2'), 
    'character' => setupField(t('assessment of the integrity and determination of the owners'), 'text medium'),
    'strength' => setupField(t('company\'s financial strength (0 to 100)'), 'int tiny'), 
    'web' => setupField(t('impression on the web (0 to 100)'), 'int tiny'),
    'history' => setupField(t('past repayment success (0 to 100)'), 'int tiny'), // 50=none
    'soundness' => setupField(t('overall how sound is this investment (0 to 100)'), 'int tiny'), 
// NO use only r_shares:bought    'approved' => setupField(t('Unixtime investment approved (negative if rejected)'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('coid') + foreignKey('clubid') + foreignKey('proposedBy');
  $indexes = index('coid') + index('proposedBy');
  $schema['r_investments'] = setupTable(t('potential and actual investments'), $fields, 'vestid', $foreignKeys, $indexes);

  // table r_ratings
  $fields = array(
    'ratingid' => setupField(t('record ID'), 'serial big'),
    'vestid' => setupField(t('investment record ID'), 'int'), 
    'uid' => setupField(t('member record ID'), 'int big'), 
    'good' => setupField(t('how well this investment serves the common good (0-100)'), 'int tiny'), 
//    'internet' => setupField(t('how good the company looks on the web (0-100)'), 'int tiny'), 
    'patronage' => setupField(t('how much the member will spend here monthly'), 'numeric 11,2'),
    'comments' => setupField(t('description of investment'), 'text medium'),
  );
  $foreignKeys = foreignKey('vestid') + foreignKey('uid');
  $indexes = index('vestid') + index('uid');
  $schema['r_ratings'] = setupTable(t('how a member rates an investment'), $fields, 'ratingid', $foreignKeys, $indexes);

  // table r_ips
  $fields = array(
    'ip' => setupField(t('ip address'), 'varchar 39'),
    'uid' => setupField(t('account record ID'), 'int big'), 
    'device' => setupField(t('device code'), 'varchar 255'), // device identifier (unique to each device/type combo)
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_ips'] = setupTable(t('IP addresses of approved accounts'), $fields, 'ip', $foreignKeys, $indexes);
  
  // table r_gifts
  $fields = array(
    'donid' => setupField(t('gift record id'), 'serial'),
    'uid' => setupField(t('uid of account that made the gift'), 'int big'),
    'amount' => setupField(t('amount of gift'), 'numeric 11,2'),
    'often' => setupField(t('recurring how often (Y, Q, M, 1)'), 'varchar 1'),
    'honor' => setupField(t('what type of honor'), 'varchar 10'), // "share" = gift is "sharing" rewards with CGF
    'honored' => setupField(t('who is honored'), 'text medium'),
    'completed' => setupField(t('Unixtime donation was completed'), 'int 11', 0),
    'giftDate' => setupField(t('date/time of gift'), 'int 11', 0),
    'share' => setupField(t('percentage of rebates/bonuses to donate to CGF'), 'numeric 6,3'), // -1 = N/A
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_gifts'] = setupTable(t('Membership gift details'), $fields, 'donid', $foreignKeys, $indexes);

  // table r_user_industries
  $fields = array(
    'id' => setupField(t('user industry record id'), 'serial'),
    'iid' => setupField(t('industry id'), 'int 11', 0),
    'uid' => setupField(t('industry id'), 'int big', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid') + index('iid');
  $schema['r_user_industries'] = setupTable(t('industries for each company'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_relations
  $fields = array(
    'reid' => setupField(t('relationship record id'), 'serial big'),
    'main' => setupField(t('uid of the account to which others are related'), 'int big'),
    'other' => setupField(t('uid of an other account related to this account'), 'int big'), // eg owner, agent, employee, supplier (0 for a partner's customer with no account yet)
    'otherNum' => setupField(t('sequence number of the other (starts with 1)'), 'int'), // used only for company cards, otherwise 0
//    'amount' => setupField(t('average monthly amount paid to other'), 'numeric 11,2', 0), // typically to employee or supplier
    'draw' => setupField(t('whether credit can flow from main to other account'), 'int tiny', 0),
    'permission' => setupField(t('what type of permission the other has on the main account'), 'int tiny', 0),
    'employee' => setupField(t('this other is an employee according to the main account (employer)'), 'int tiny', 0),
    'isOwner' => setupField(t('this other is an owner (or part owner) or family relation'), 'int tiny', 0),
    'isCustomer' => setupField(t('this other is a customer'), 'int tiny', 0), // 2 means <pay invoices automatically>
    'code' => setupField(t('the (main) company\'s account ID for this other'), 'varchar 50'),
    'data' => setupField(t('serialized array of parameters'), 'text medium'), // used for partner's customer data while other=0
    'created' => setupField(t('Unixtime record created'), 'int 11'),
  );
  $foreignKeys = foreignKey('main') + foreignKey('other') + foreignKey('foreignUid');
  $indexes = index('main') + index('other');
  $schema['r_relations'] = setupTable(t('Who can manage which accounts, and how'), $fields, 'reid', $foreignKeys, $indexes);
    
  // table r_coupons
  $fields = array(
    'coupid' => setupField(t('record id'), 'serial big'),
    'fromId' => setupField(t('account ID of company offering the coupon'), 'int big', 0),
    'start' => setupField(t('Unixtime coupon is first valid'), 'int 11', 0),
    'end' => setupField(t('Unixtime after which coupon is no longer valid'), 'int 11', 0),
    'amount' => setupField(t('amount of discount'), 'numeric 11,2'), // amount or percentage (if < 0)
    'minimum' => setupField(t('minimum purchase amount to get the discount'), 'numeric 11,2'),
    'ulimit' => setupField(t('maximum number of uses per member'), 'int 11', 0),
    'flags' => setupField(t('boolean type flags'), 'int big', 0, TRUE),
  );
  $foreignKeys = foreignKey('fromId');
  $indexes = index('fromId');
  $schema['r_coupons'] = setupTable(t('Coupons'), $fields, 'coupid', $foreignKeys, $indexes);

  // table r_coupated
  $fields = array(
    'id' => setupField(t('record id'), 'serial big'),
    'uid' => setupField(t('account ID of account that used the coupon'), 'int big', 0),
    'coupid' => setupField(t('coupon ID'), 'int big', 0),
    'uses' => setupField(t('number of times this account has used this coupon OR iCode'), 'int', 0), // gift card uses this field for iCode of the card code
    'when' => setupField(t('Unixtime the coupon was used'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid') + foreignKey('coupid');
  $indexes = index('uid') + index('coupid');
  $schema['r_coupated'] = setupTable(t('Coupons used'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_invites
  $fields = array(
    'id' => setupField(t('record id'), 'serial big'),
    'code' => setupField(t('secret invitation code'), 'varchar 64'),
    'email' => setupField(t('email of invitee'), 'varchar 255'), // encrypted/searchable
    'inviter' => setupField(t('uid of inviting member'), 'int big', 0),
    'invitee' => setupField(t('uid of invited new member'), 'int big', 0), // 0 if not accepted yet
    'invited' => setupField(t('date of invitation'), 'int 11', 0),
    'subject' => setupField(t('email subject'), 'varchar 255'),
    'message' => setupField(t('email message body'), 'text medium'),
  );
  $foreignKeys = foreignKey('inviter') + foreignKey('invitee');
  $indexes = index('inviter') + index('code');
  $schema['r_invites'] = setupTable(t('Who invited whom'), $fields, 'id', $foreignKeys, $indexes);

  // table r_request
  $fields = array(
    'listid' => setupField(t('record id'), 'int big'), // pointer into origin table
    'created' => setupField(t('Unixtime record created'), 'int 11'),
    'first' => setupField(t('first name of the individual'), 'varchar 60'),
    'last' => setupField(t('last name of the individual'), 'varchar 60'),
    'phone' => setupField(t('contact phone (no punctuation)'), 'varchar 255'), // encrypted/searchable
    'email' => setupField(t('email of invitee'), 'varchar 255'), // encrypted/searchable
    'zip' => setupField(t('postal code (no punctuation)'), 'varchar 60'),
    'ctty' => setupField(t('uid of this requester\'s Common Good Community'), 'int big'), // gleaned from zip
    'done' => setupField(t('are we done with this request'), 'int tiny', 0), 
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_request'] = setupTable(t('Requests to be invited'), $fields, 'listid', $foreignKeys, $indexes);
      
  // table r_notices
  $fields = array(
    'msgid' => setupField(t('notice record id'), 'serial'),
    'uid' => setupField(t('account record ID of member notified'), 'int big'),
    'created' => setupField(t('date of notice'), 'int 11', 0),
    'sent' => setupField(t('date sent (0 if not sent yet)'), 'int 11', 0),
    'message' => setupField(t('the notice text'), 'text medium'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_notices'] = setupTable(t('Message digest buffer'), $fields, 'msgid', $foreignKeys, $indexes);

  // table r_regions
  /*
  $fields = array(
    'region' => setupField(t('region id'), 'char 3'),
    'state' => setupField(t('state or province abbreviation'), 'char 2'),
  );
  $foreignKeys = [];
  $indexes = index('state');
  $schema['r_regions'] = setupTable(t('rCredits regions'), $fields, 'region', $foreignKeys, $indexes);
  */

  // table r_near
  $fields = [
    'uid1' => setupField(t('account record ID of one account'), 'int big'),
    'uid2' => setupField(t('account record ID of other account'), 'int big'),
    'weight' => setupField(t('number of connections'), 'int medium'),
  ];
  $foreignKeys = foreignKey('uid1', 'uid2');
  $indexes = [];
  $schema['r_near'] = setupTable(t('How members are connected'), $fields, ['uid1', 'uid2'], $foreignKeys, $indexes);

  /*
  // table r_votes
  $fields = [
    'vid' => setupField(t('vote record id'), 'serial'),
    'uid' => setupField(t('account record ID'), 'int big'),
    'issue' => setupField(t('name of the question being voted on'), 'varchar 255'),
    'vote' => setupField(t('how the account voted on this issue'), 'int medium'),
  ];
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_votes'] = setupTable(t('How members voted on issues'), $fields, 'vid', $foreignKeys, $indexes);
*/

  // table r_tous (messages to us)
  $fields = [
    'id' => setupField(t('vote record id'), 'serial'),
    'uid' => setupField(t('account record ID'), 'int big'),
    'time' => setupField(t('date/time of message'), 'int 11', 0),
    'message' => setupField(t('the message'), 'blob'),
  ];
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_tous'] = setupTable(t('Secure messages sent from member to us.'), $fields, 'id', $foreignKeys, $indexes);

  // VOTING

  // table r_events
  $fields = array(
    'id' => setupField(t('record id'), 'serial'),
    'ctty' => setupField(t('what community the event is in'), 'int big'),
    'type' => setupField(t('event type (I=in person, V=vote, G=grading, P=RFP)'), 'char 1'),
    'event' => setupField(t('name of event'), 'varchar 255'),
    'details' => setupField(t('event details'), 'text big'), 
    'start' => setupField(t('Unixtime event begins'), 'int 11', 0),
    'end' => setupField(t('Unixtime event ends'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_events'] = setupTable(t('Events in each community\'s democratic process'), $fields, 'id', $foreignKeys, $indexes);
    
  // table r_questions
  $fields = [
    'id' => setupField(t('question record id'), 'serial big'),
    'event' => setupField(t('what event this question is part of'), 'int big'),
//    'ctty' => setupField(t('community or region record id'), 'int big'),
//	`phase` CHAR(1) DEFAULT 'I'), // current phase: I=ideas P=proposals, [D=discussion], V=voting, R=results
    'repeats' => setupField(t('pointer to question that this is a revote on (0=none)'), 'int big', 0),
    'repeatedBy' => setupField(t('pointer to question that is a revote of this one (0=none))'), 'int big', 0),
//    'endIdeas' => setupField(t('when does the Ideas phase end'), 'int 11', 0),
//    'endProposals' => setupField(t('when does the Proposals phase end'), 'int 11', 0), // not currently used
//    'endVoting' => setupField(t('when does the Voting phase end'), 'int 11', 0),
//    'endGrading' => setupField(t('when does the Grading phase end'), 'int 11', 0), // grading specific proposals in Budget questions
    'text' => setupField(t('text of the question'), 'text medium'),
    'detail' => setupField(t('additional detail about the question'), 'text medium'),
//    'linkIdeas' => setupField(t('where to brainstorm ideas (and see the list)'), 'text tiny'),
//    'linkProposals' => setupField(t('where to propose options (and see the discussion)'), 'text tiny'),
    'linkDiscussion' => setupField(t('link to online discussion of the issue'), 'text tiny'),
    'type' => setupField(t('vote type M=multiple choice, B=budget (penny vote), R=range, E=essay'), 'char 1', 'M'),
    'units' => setupField(t('budget units (defaults to money, measured in the community\'s national currency)'), 'varchar 255'),
    'budget' => setupField(t('how much (money) is to be budgeted'), 'numeric 11,0', 0),
    'minVeto' => setupField(t('minimum veto fraction of vote, to force reconsideration'), 'numeric 5,3'), // set by ctty, here for history (no effect on M results)
    'optOrder' => setupField(t('option order'), 'char 1', 'S'), // S=shuffle (randomize), Z=reverse sometimes, N=no change (static)
    'voteCount' => setupField(t('total number of votes'), 'int 11', 0),
//    'modified' => setupField(t('date/time last modified'), 'int 11', 0),
    'result' => setupField(t('results of the vote or grading'), 'text big'),
    'created' => setupField(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_questions'] = setupTable(t('Questions to be voted on'), $fields, 'id', $foreignKeys, $indexes);

  // table r_options
  $fields = [
    'id' => setupField(t('option record id'), 'serial big'),
    'question' => setupField(t('question for which this is an option'), 'int big', 0),
    'text' => setupField(t('text of the option'), 'text tiny'),
    'detail' => setupField(t('additional detail about the option'), 'text medium'),
    'displayOrder' => setupField(t('where to display this option in the order'), 'int tiny', 0),
    'minimum' => setupField(t('the least (money) to budget for this option'), 'int big', 0),
    'maximum' => setupField(t('the most (money) to budget for this option'), 'int big', 0), // (0=no maximum) */
	  'mandatory' => setupField(t('is the minimum required?'), 'int tiny', 0),
	  'averageGrade' => setupField(t('average grade (in a penny vote, the fraction of all votes)'), 'numeric 5,3', 0),
	  'averageMax' => setupField(t('average maximum grade (for range votes)'), 'numeric 5,3', 0),
    'vetoes' => setupField(t('number of vetoes for this option'), 'int', 0),
    'modified' => setupField(t('date/time last modified'), 'int 11', 0),
    'created' => setupField(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('question', 'id', 'r_questions');
  $indexes = index('question');
  $schema['r_options'] = setupTable(t('Options for a question to be voted on'), $fields, 'id', $foreignKeys, $indexes);

  // table r_pairs (not currently used)
  $fields = [
    'id' => setupField(t('pairs record id'), 'serial big'),
    'option1' => setupField(t('record id of one option'), 'int big', 0),
    'option2' => setupField(t('record id of the other option'), 'int big', 0),
    'prefer1' => setupField(t('how many voters prefer the option1'), 'int 11', 0),
    'prefer2' => setupField(t('how many voters prefer the option2'), 'int 11', 0),
    'nopreference' => setupField(t('how many voters had no preference between the two options'), 'int 11', 0),
    'raw' => setupField(t('true if this record is calculated without counting proxies'), 'int tiny', 0),
	  'created' => setupField(t('date/time created'), 'int 11', 0), // normally created just once, when election is over (except for demos)
  ];
  $foreignKeys = foreignKey('option1', 'id', 'r_options') + foreignKey('option2', 'id', 'r_options');
  $indexes = index('option1') + index('option2');
  $schema['r_pairs'] = setupTable(t('Counts of preferences of one option over another'), $fields, 'id', $foreignKeys, $indexes);

  // table r_ballots
  $fields = [
    'id' => setupField(t('ballot record id'), 'serial big'),
    'question' => setupField(t('question or proposal voted on by a particular voter'), 'int big', 0), // <0 for proposals
    'voter' => setupField(t('record id of voter whose ballot this is'), 'int big', 0),
    'proxy' => setupField(t('record id of voter who actually voted on behalf of the voter'), 'int big', 0),
    'modified' => setupField(t('date/time last modified'), 'int 11', 0),
    'created' => setupField(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('question', 'id', 'r_questions') + foreignKey('voter') + foreignKey('proxy');
  $indexes = index('question') + index('voter') + index('proxy');
  $schema['r_ballots'] = setupTable(t('A votable question addressed by a particular voter'), $fields, 'id', $foreignKeys, $indexes);

  // table r_votes
  $fields = [
    'id' => setupField(t('vote record id'), 'serial big'),
    'ballot' => setupField(t('ballot on which this option is being graded'), 'int big', 0),
    'option' => setupField(t('option being graded'), 'int big', 0),
    'grade' => setupField(t('grade given by a particular voter for this option'), 'int', -1), // In a B vote, this is 10 * the pct (ddd.d) else 3*letter (-1=blank -2=veto); in range-type vote this is the voter's chosen min
    'gradeMax' => setupField(t('maximum grade given by a particular voter for this range-type option'), 'int', -1),
    'displayOrder' => setupField(t('what order options were shown in to this voter'), 'int', 0), // 0 is first
    'text' => setupField(t('what was voter\'s comment or moral objection to this option'), 'text big'),
    'isVeto' => setupField(t('this is a veto (not a canceled veto or mere comment)'), 'int tiny', 0),
    'modified' => setupField(t('date/time last modified'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('ballot', 'id', 'r_ballots') + foreignKey('option', 'id', 'r_options');
  $indexes = index('ballot') + index('option');
  $schema['r_votes'] = setupTable(t('What grade a particular voter gave a particular option (for a particular question) on a particular question ballot'), $fields, 'id', $foreignKeys, $indexes);

  // table r_proposals
  $fields = [
    'id' => setupField(t('proposal record id'), 'serial big'),
    'event' => setupField(t('what event this question is part of'), 'int big'),
    'project' => setupField(t('project title'), 'varchar 255'),
//    'ctty' => setupField(t('community or region record id'), 'int big'),
    'categories' => setupField(t('funding categories (space-separated list)'), 'text tiny'),
    'overview' => setupField(t('project overview'), 'text medium'),
    'purpose' => setupField(t('why the project is needed'), 'text medium'),
    'where' => setupField(t('zipcode where the project will take place'), 'text tiny'),
    'when' => setupField(t('project start date'), 'int 11'),
    'until' => setupField(t('project end date'), 'int 11'),
    'how' => setupField(t('how the project will be accomplished'), 'text medium'),
    'amount' => setupField(t('amount of funding requested'), 'numeric 11,2'),
    'type' => setupField(t('type of funding'), 'int tiny'), // 0=investment 1=loan 2=incentive 3=grant 4=other
    'budgetTotal' => setupField(t('total expense budget'), 'numeric 11,2'),
    'budget' => setupField(t('detailed project budget'), 'text medium'),
    'contingency' => setupField(t('how the project organizers will cope with less funding than requested'), 'text medium'),
    'qualifications' => setupField(t('qualifications of project staff'), 'text medium'),
    'evaluation' => setupField(t('how the success of the project will be evaluated'), 'text medium'),
    'name' => setupField(t('individual or organization making the proposal'), 'varchar 255'),
    'contact' => setupField(t('contact person (or "self")'), 'varchar 255'),
    'phone' => setupField(t('contact phone'), 'varchar 255'),
    'email' => setupField(t('contact email'), 'varchar 255'),
    'sponsor' => setupField(t('member(s) sponsoring the project proposal'), 'text tiny'),
  ];
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty') + index('name');
  $schema['r_proposals'] = setupTable(t('funding proposals'), $fields, 'id', $foreignKeys, $indexes);
  

  
  // table r_votenotes
  /*
  $fields = [
    'id' => setupField(t('veto record id'), 'serial big'),
    'vote' => setupField(t('what option (on a particular ballot) did the voter veto or comment on'), 'int big', 0),
    'modified' => setupField(t('date/time last modified'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('vote'), 'id', 'r_votes');
  $indexes = index('vote');
  $schema['r_votenotes'] = setupTable('Details about a voter\'s veto of a particular option'), $fields, 'id', $foreignKeys, $indexes);
  */
  
  // table r_nonmembers (use -make instead)
/*  $fields = array(
    'id' => setupField(t('non-member company record id', 'serial'),
    'company' => setupField(t('company name', 'varchar 60'),
    'potential' => setupField(t('number of members who shop there', 'int 6', 0),
  );
  $schema['r_nonmembers'] = setupTable('Local companies we want to recruit', $fields, 'id', NULL, NULL);
  */
  
/*
  // table r_counts
  $fields = array(
    'type' => setupField(t('what is being counted', 'varchar 255'),
    'channel' => setupField(t('through what channel did it happen', 'int 11'),
    'count' => setupField(t('the count', 'int big', 0),
  );
  $schema['r_counts'] = setupTable('How many this and that overall, through the various channels', $fields, array('type', 'channel'));
*/
  
  // table r_scores
  /*
  $fields = array(
    'company' => setupField(t('uid of company', 'int 11'),
    'count' => setupField(t('for the rTraders that have this many places to buy with rCredits', 'int 11', 0),
    'score' => setupField(t('this many of those rTraders would buy from this company', 'int 11'),
  );
  $foreignKeys = foreignKey('company');
  $indexes = index('company');
  $schema['r_scores'] = setupTable('Best companies to promote to rTrader in the current round', $fields, NULL, $foreignKeys, $indexes);
  */
  
  /*  
  setupField(t('r_areas', 'Area Code', 'telephone area code', 'char 3'); 
  setupField(t('r_areas', 'Region', 'state, province, or territory', 'varchar 24'); 
  
  */

  foreach (ray(TRACK_DELETED) as $table) {
    $xtable = $table == 'users' ? 'x_users' : str_replace('r_', 'x_', $table);
    $schema[$xtable] = $schema[$table];
/*    $fields = $schema[$xtable]['fields'];
    if ($key = @$schema[$xtable]['primary key']) {
      if ($fields[$key]['type'] == 'serial') $schema[$xtable]['fields'][$key]['type'] = 'int'; // permits dropping primary key
    } */
    $deleted = setupField(t('Unixtime record was deleted'), 'int 11', 0);
    u\preray(compact('deleted'), $schema[$xtable]['fields']);
    $schema[$xtable]['primary key'][] = 'deleted'; // allow duplicate keys in deletions tables
  }

  return (object) $schema;
}

/**
 * Make Fields
 *
 * Make database fields, according to the schema
 */
function rebuildDb() {
  global $db_name;
  $schema = tableDefs();
  foreach ($schema as $table => $scheme) {
    if (\db_table_exists($table)) {
      $previous = '';
      foreach ($scheme['fields'] as $field => $spec) {
        $spec = specString($spec);
        if (db\exists('INFORMATION_SCHEMA.COLUMNS', 'TABLE_SCHEMA=:db_name AND TABLE_NAME=:table AND COLUMN_NAME=:field', compact('db_name', 'table', 'field'))) {
          db\q("ALTER TABLE $table CHANGE `$field` `$field` $spec");
        } else db\q("ALTER TABLE $table ADD `$field` $spec " . ($previous ? "AFTER `$previous`" : 'FIRST'));
        $previous = $field;
      }
    } else \db_create_table($table, $scheme);
  }
}

/**
 * Interpret the Drupal field specification array as a MySQL field specification string.
 * @param assoc $spec: the filed specification array
 * @return the MySQL field specification string
 */
function specString($spec) {
  extract($spec);
  
  if ($type == 'numeric') {
    $type = "decimal($precision,$scale)";
  } elseif ($type == 'text' or $type == 'blob') {
    $type = (@$size == 'big' ? 'long' : @$size) . $type;
  } elseif ($type == 'float' or $type == 'double') { // no change
  } elseif ($type == 'serial') {
    $type = @$size . 'int AUTO_INCREMENT';
  } else {
    $type = (@$size ?: '') . $type . (@$length ? "($length)" : '');
  }

  $description = str_replace("'", "''", $description);
  return $type
  . (@$unsigned ? ' UNSIGNED' : '')
  . (isset($default) ? " NOT NULL DEFAULT '$default'" : '') // default is NULL by default
  . " COMMENT '$description'";
}

function setupTable($description, $fields, $primaryKey, $foreignKeys = [], $indexes = []) {
  $res = compact('description', 'fields', 'indexes') + ['foreign keys' => $foreignKeys];
  if (@$primaryKey) $res['primary key'] = is_array($primaryKey) ? $primaryKey : [$primaryKey];
  return $res;
}

function foreignKey($local, $foreign = 'uid', $table = 'users') {
  $columns = array($local => $foreign);
  return array($local => compact('table', 'columns'));
}

function index($field) {return array($field => array($field));}

/**
 * Setup a field specification
 *
 * @param string $description: description of the field
 * @param string $type: data type and optional size, separated by a space
 *   a numeric size is interpreted as a string's maximum length
 *   a size consisting of two numbers separated by a comma is interpreted as precision and scale
 * @param string $default: the field's default value, if any
 * @param bool $unsigned: <field is an unsigned number>
 * @return array: the field's specification array
 */
function setupField($description = '', $type = 'varchar 255', $default = NULL, $unsigned = FALSE) {
  $size = '';
  $precision = ',';

  if (strpos($type, ' ')) list ($type, $size) = explode(' ', $type);
  if ($type == 'numeric') {
    list ($precision, $size) = [$size, ''];
  } elseif (is_numeric($size)) list ($length, $size) = [$size, ''];
  list ($precision, $scale) = explode(',', $precision); // don't use u\ray here
  
  $spec = compact(explode(' ', 'description type size length precision default scale unsigned')); // don't use u\ray
  foreach ($spec as $k => $v) if ($v === '') unset($spec[$k]);

  return $spec;
}

function scheme($table) {return tableDefs()->$table;}
