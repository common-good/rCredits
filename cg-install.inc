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
  $table = 'sessions';
  $desc = @$schema[$table]['fields']['uid']['description'] ?: 'users record id';
  $schema[$table]['fields']['uid'] = dbFld($desc, 'int big');
  
  // table users
  $fields = array(
    // standard Drupal fields
    'uid' => dbFld(t('account record ID'), 'int big'),
    'name' => dbFld(t('unique user name'), 'varchar 60', ''),
    'pass' => dbFld(t('account password'), 'varchar 128', ''),
    'mail' => dbFld(t('encrypted email address'), 'varchar 254', ''),
    'created' => dbFld(t('Unix date and time record was created'), 'int 11', 0),
    'access' => dbFld(t('Unix date and time account was last accessed'), 'int 11', 0),
    'login' => dbFld(t('Unix date and time user last signed in'), 'int 11', 0),
    'picture' => dbFld(t('used for temporary storage when generating statistics'), 'int 11', 0),
    'data' => dbFld(t('serialized associative array of miscellaneous fields (not encrypted)'), 'text big'),
    
    // additions to Drupal fields
    'flags' => dbFld(t('boolean permissions and state flags'), 'int big', 0, TRUE),
    'jid' => dbFld(t('account ID of joined account (0 if none)'), 'int big', 0),
    'changes' => dbFld(t('changes made to the account'), 'blob big'),
    'community' => dbFld("uid of this account's Common Good Community", 'int big'),
//    'question' => dbFld(t('security question'), 'varchar 255'),
//    'usdAccount' => dbFld(t('USD (Dwolla) account number (hex of encrypted)'), 'varchar 255'), // searchable
    'secure' => dbFld(t('encrypted data'), 'blob medium'), 
    'vsecure' => dbFld(t('hyper-encrypted data'), 'blob'), 

//    'offsite' => dbFld(t('index to encrypted data stored off-site'), 'int big', NULL, TRUE),
    'fullName' => dbFld(t('full name of the individual or entity'), 'varchar 255'),
    'phone' => dbFld(t('contact phone (no country code, no punctuation)'), 'varchar 255'), // encrypt/searchable
//    'faxetc' => dbFld(t('other contact details'), 'varchar 255'),
//    'address' => dbFld(t('physical street address'), 'varchar 60'),
    'city' => dbFld(t('municipality'), 'varchar 60'),
    'state' => dbFld(t('state/province index'), 'int 5'),
    'zip' => dbFld(t('postal code for physical address (no punctuation)'), 'varchar 255'),
    'country' => dbFld(t('country index'), 'int 4'),
    'notes' => dbFld(t('miscellaneous notes about the user or the account'), 'text big'),
    'tickle' => dbFld(t('Unixtime to tickle an admin about this account'), 'int 11', 0),
    'activated' => dbFld(t('when was the account activated'), 'int 11', 0),
    'helper' => dbFld(t('who invited this person or company'), 'int big'),
    'iCode' => dbFld(t('sequence number of helper invitation'), 'int'),
    'signed' => dbFld(t('when did this person sign the %PROJECT Agreement'), 'int 11', 0),
    'signedBy' => dbFld(t('who signed the agreement (on behalf of the account)'), 'varchar 60'),
    'rebate' => dbFld(t('current rebate percentage (sales bonus is proportionate)'), 'numeric 5,3', R_REBATE),
    'savingsAdd' => dbFld(t('chosen amount to hold as savings, beyond rewards'), 'numeric 11,2', 0),
    'saveWeekly' => dbFld(t('chosen amount to increase minimum (target balance) by, weekly'), 'numeric 11,2', 0),
    'floor' => dbFld(t('negative credit line'), 'numeric 11,2', R_FLOOR),
    'minimum' => dbFld(t('chosen target balance (for automatic refills)'), 'numeric 11,2', NULL), // NULL means preferences not set yet
//    'chunk' => dbFld(t('granularity of transfers from bank account'), 'numeric 11,2', R_BANK_CHUNK),
//    'maximum' => dbFld(t('maximum balance (send excess to bank)'), 'numeric 11,2', -1), // -1 means no limit (unused)
    'share' => dbFld(t('percentage of rebates/bonuses to donate to CG'), 'numeric 6,3', 0),
    'crumbs' => dbFld(t('percentage of each transaction to donate to CG (half to community)'), 'numeric 6,3', NULL),
    'balance' => dbFld(t('balance, not including rewards (cached)'), 'numeric 11,2', 0),
//    'r' => dbFld(t('balance, including rewards (cached)'), 'numeric 11,2', 0),
//    'usd' => dbFld(t('balance in related USD account (probable)'), 'numeric 11,2', 0),
    'rewards' => dbFld(t('total incentive rewards to date (cached)'), 'numeric 11,2', 0),
    'committed' => dbFld(t('amount committed (for donations to CGF)'), 'numeric 11,2', 0),
//    'frozen' => dbFld(t('amount of rCredits currently unavailable until specific dates or indefinitely'), 'text medium'),
//    'photo' => dbFld(t('member photo'), 'blob big'), // encrypted
    'risk' => dbFld(t('today\'s suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => dbFld(t('list of risk factors'), 'int big', 0, TRUE),
    'trust' => dbFld(t('how much this person is trusted by others in the community'), 'float medium', NULL),
    'stats' => dbFld(t('account statistics'), 'text medium'),
    'notices' => dbFld(t('when to send what kind of notice'), 'text tiny'),
    'lastip' => dbFld(t('latest IP address used'), 'varchar 39'),
//    'members' => isGAME ? dbFld(t('number of members represented'), 'int medium', 1) : NULL,
    'special' => dbFld(t('special transient data'), 'text big'),
  );
  // need foreign keys here
//  $schema['users']['fields']['uid'] = dbFld(t('Unique user ID'), 'int big'); // not unsigned (must be first field)
//  $schema['users']['fields']['data'] = dbFld(t('serialized array of miscellaneous data'), 'text big');
//  $schema['users']['fields'] = $fields;
  $foreignKeys = foreignKey('community');
  $indexes = index('name') + index('mail') + index('created') + index('community') + index('zip');
  $schema['users'] = dbTable(t('Account data'), $fields, 'uid', $foreignKeys, $indexes);
// kills Drupal: $schema['users']['fields']['data']['description'] = 'incidental data (a serialized array of name value pairs)';

  // table sessions (additions to Drupal standard)
  $fields = array( // additional/changed sessions fields
    'acct' => dbFld(t('currently viewing/managing this account ID'), 'int big'),
  );
  $schema['sessions']['fields'] = (@$schema['sessions']['fields'] ?: []) + $fields;

  // table r_photos (separate from user table for efficiency -- this table gets big)
  $fields = array(
    'uid' => dbFld(t('account record id'), 'int big'),
    'photo' => dbFld(t('member photo'), 'blob big'), // encrypted for members, filename for companies
  );
  $foreignKeys = foreignKey('uid');
  $indexes = [];
  $schema['r_photos'] = dbTable(t('one photo for each account'), $fields, 'uid', $foreignKeys, $indexes);

  // table r_company
  $fields = array(
    'uid' => dbFld(t('account record ID'), 'serial big'),
    'coType' => dbFld(t('type of entity'), 'int tiny'),
    'website' => dbFld(t('company website domain'), 'text tiny'),
    'description' => dbFld(t('long markdown description'), 'text medium'),
    'shortDesc' => dbFld(t('one line description'), 'text tiny'),
    'selling' => dbFld(t('list of typical transaction descriptions'), 'text medium'),
    'coFlags' => dbFld(t('miscellaneous flag bits'), 'int big', 0, TRUE),
    'gross' => dbFld(t('average annual gross receipts'), 'numeric 11,2', 0),
    'serviceArea' => dbFld(t('geographic region the company serves'), 'text tiny'),
    'employees' => dbFld(t('number of employees'), 'int 11', 0),
    'payrollStart' => dbFld(t('Unixtime date last payroll started'), 'int 11', 0),
    'payrollEnd' => dbFld(t('Unixtime date last payroll ended'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = [];
  $schema['r_company'] = dbTable(t('Companies'), $fields, 'uid', $foreignKeys, $indexes);
  
  // table queue
  $fields = array(
    'id' => dbFld(t('primary key: Unique item ID'), 'serial big'),
    'item' => dbFld(t('arbitrary data for the item.'), 'blob big'),
    'created' => dbFld(t('Unixtime record was created'), 'int 11'),
  );
  $foreignKeys = [];
  $indexes = [];
  $schema['queue'] = dbTable(t('Cron queue'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_stats
  $fields = array(
    'id' => dbFld(t('statistics record id'), 'serial big'),
    'created' => dbFld(t('Unixtime record was created'), 'int 11', 0),
    
    'ctty' => dbFld(t('community or region record id'), 'int big'),
    'pAccts' => dbFld(t('number of personal accounts'), 'int medium', 0),
    'bAccts' => dbFld(t('number of company accounts'), 'int medium', 0),
    'newbs' => dbFld(t('number of not-yet-active accounts'), 'int medium', 0),
    'aAccts' => dbFld(t('number of active personal accounts'), 'int medium', 0),
    'conx' => dbFld(t('number of connections per personal account'), 'numeric 10,3', 0),
    'conxLocal' => dbFld(t('number of local connections per personal account'), 'numeric 10,3', 0),
    
    'balsPos' => dbFld(t('amount of positive balances'), 'numeric 11,2', 0),
    'balsNeg' => dbFld(t('amount of negative balances'), 'numeric 11,2', 0),
    'balsPosCount' => dbFld(t('number of positive balances'), 'int medium', 0),
    'balsNegCount' => dbFld(t('number of negative balances'), 'int medium', 0),
    'topN' => dbFld(t('top N or N% of balances, whichever is greater'), 'numeric 11,2', 0),
    'botN' => dbFld(t('bottom N or N% of balances, whichever is less'), 'numeric 11,2', 0),
    'floors' => dbFld(t('credit lines'), 'numeric 11,2', 0),
    
    'p2b' => dbFld(t('customer purchase volume'), 'numeric 11,2', 0),
    'b2b' => dbFld(t('b2b purchase volume'), 'numeric 11,2', 0),
    'b2p' => dbFld(t('payroll volume'), 'numeric 11,2', 0),
    'p2p' => dbFld(t('person-to-person purchase volume'), 'numeric 11,2', 0),
    'p2bCount' => dbFld(t('customer purchase volume'), 'int medium', 0),
    'b2bCount' => dbFld(t('b2b purchase volume'), 'int medium', 0),
    'b2pCount' => dbFld(t('payroll volume'), 'int medium', 0),
    'p2pCount' => dbFld(t('person-to-person purchase volume'), 'int medium', 0),
    'cashs' => dbFld(t('amount of exchanges for cash'), 'numeric 11,2', 0),
    'cashsCount' => dbFld(t('number of exchanges for cash'), 'int medium', 0),
    
    'cgIn' =>dbFld(t('amount of %RCREDITS coming into this community'), 'numeric 11,2', 0),
    'cgOut' =>dbFld(t('amount of %RCREDITS leaving this community'), 'numeric 11,2', 0),
    'cgInCount' =>dbFld(t('number of transfers into this community'), 'int medium', 0),
    'cgOutCount' =>dbFld(t('number of transfers out of this community'), 'int medium', 0),
    
    'usdIn' => dbFld(t('amount of US Dollars brought into the system'), 'numeric 11,2', 0),
    'usdOut' => dbFld(t('amount of US Dollars taken out of the system'), 'numeric 11,2', 0),
    'usdInCount' => dbFld(t('number of incoming bank transfers'), 'int medium', 0),
    'usdOutCount' => dbFld(t('number of outgoing bank transfers'), 'int medium', 0),
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_stats'] = dbTable(t('Operating statistics for communities and overall'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_boxes
  $fields = array(
    'id' => dbFld(t('device record id'), 'serial big'),
    'channel' => dbFld(t('channel'), 'int tiny'), // web browser, rPOS app, or SMS
    'code' => dbFld(t('device id'), 'varchar 255'), // device identifier (unique to each device/type combo)
    'boxnum' => dbFld(t('sequential device number for the account'), 'int 11', 0),
    'uid' => dbFld(t('account record id'), 'int big'),
    'boxName' => dbFld(t('member\'s chosen name for this device, for this account'), 'varchar 255'),
    'todo' => dbFld(t('waiting for confirmation to complete this operation'), 'text medium'),
    'nonce' => dbFld(t('waiting for this nonce, for confirmation'), 'varchar 255'), // also transient QR ID
//    'restricted' => dbFld(t('permit no new users of this device'), 'int tiny', 0),
    'access' => dbFld(t('date/time last used'), 'int 11', 0), // to trigger deletion after several months
    'created' => dbFld(t('Unixtime record was created'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid') + index('boxnum');
  $schema['r_boxes'] = dbTable(t('Devices info1'), $fields, 'id', $foreignKeys, $indexes);

  // table r_bad
  $fields = array(
    'qid' => dbFld(t('phoney customer qid'), 'varchar 255'),
    'code' => dbFld(t('phoney card security code'), 'varchar 255'),
    'created' => dbFld(t('Unixtime record was created'), 'int 11'),
  );
  $foreignKeys = [];
  $indexes = [];
  $schema['r_bad'] = dbTable(t('lost, stolen, or faked rCard codes'), $fields, 'created', $foreignKeys, $indexes);
  
  // table r_do
  $fields = array(
    'doid' => dbFld(t('record id'), 'serial'),
    'expires' => dbFld(t('Unixtime expiration'), 'int 11', 0), // 0 means never expires
    'data' => dbFld(t('serialized array of parameters'), 'text medium'),
    'uid' => dbFld(t('related account record ID'), 'int big', 0), // needed only for changeUid
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_do'] = dbTable(t('Clickable actions with no signin'), $fields, 'doid', $foreignKeys, $indexes);
    
  // table r_proxies
  $fields = array(
    'id' => dbFld(t('record id'), 'serial'),
    'person' => dbFld(t('account record id'), 'int big'),
    'proxy' => dbFld(t('account record id of proxy'), 'int big'),
    'priority' => dbFld(t('precedence of this proxy (1=top priority)'), 'int tiny'),
  );
  $foreignKeys = foreignKey('person') + foreignKey('proxy');
  $indexes = index('person');
  $schema['r_proxies'] = dbTable(t('Who represents whom'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_txs
  $fields = array(
    'xid' => dbFld(t('the unique transaction ID'), 'serial big'), 
    'serial' => dbFld(t('serial number of related transactions (=xid of first transaction in the group)'), 'int 11'), 
    'type' => dbFld(t('transaction type (transfer, rebate, etc.)'), 'int tiny'),
//    'taking' => dbFld(t('was the transaction initiated by the payee'), 'int tiny'), 
    'goods' => dbFld(t('is this transfer an exchange for real goods and services?'), 'int tiny', 0), 
    'amount' => dbFld(t('amount transferred'), 'numeric 11,2'), 
    'payer' => dbFld(t('user id of the payer'), 'int big'), 
    'payee' => dbFld(t('user id of the payee'), 'int big'), 
    'payerAgent' => dbFld("user id of payer's agent (who approved this transaction for the payer)", 'int big'), 
    'payeeAgent' => dbFld("user id of payee's agent (who approved this transaction for the payee)", 'int big'), 
    'payerFor' => dbFld("payer's description", 'varchar 255'), 
    'payeeFor' => dbFld("payee's description", 'varchar 255'), 
    'payerReward' => dbFld(t('incentive reward for payer'), 'numeric 11,2', 0), 
    'payeeReward' => dbFld(t('incentive reward for payee'), 'numeric 11,2', 0), 
    'payerTid' => dbFld("payer's transaction ID", 'int 11'),
    'payeeTid' => dbFld("payee's transaction ID", 'int 11'),
    'data' => dbFld(t('miscellaneous non-searchable data (serialized array)'), 'text big'), 
    'flags' => dbFld(t('boolean characteristics and state flags'), 'int big', 0, TRUE),
    'channel' => dbFld(t('through what medium was the transaction entered'), 'int tiny'),
    'box' => dbFld(t('on what machine was the transaction entered'), 'int 11', 0), // 0 = none
    'created' => dbFld(t('Unixtime transaction was created'), 'int 11', 0),
    'risk' => dbFld(t('suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => dbFld(t('list of risk factors'), 'int big', 0, TRUE),
//    'members' => isGAME ? dbFld(t('number of members represented'), 'int medium', 1) : NULL,
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee') + foreignKey('payerAgent') + foreignKey('payeeAgent');
  $indexes = index('payer') + index('payee') + index('created') + index('payerTid') + index('payeeTid'); // also serial/type/goods/payerAgent/payeeAgent/channel/?
  $schema['r_txs'] = dbTable(t('Record of all transactions in the region'), $fields, 'xid', $foreignKeys, $indexes);

  // table r_invoices
  $fields = array(
    'nvid' => dbFld(t('the unique invoice ID'), 'serial big'), 
    'status' => dbFld(t('transaction record ID or status (pending or denied)'), 'int 11', TX_PENDING), // positive values are record IDs in r_txs (for paid invoices), non-positive values are status
    'amount' => dbFld(t('amount to charge'), 'numeric 11,2'), 
    'payer' => dbFld(t('user id of the payer'), 'int big'), 
    'payee' => dbFld(t('user id of the payee'), 'int big'), 
    'goods' => dbFld(t('is this an invoice for real goods and services?'), 'int tiny', 0), 
    'purpose' => dbFld("payee's description", 'text big'), 
// not needed yet    'flags' => dbFld(t('boolean characteristics and state flags'), 'int big', 0, TRUE),
    'data' => dbFld(t('miscellaneous non-searchable data (serialized array)'), 'text big'), 
    'created' => dbFld(t('Unixtime invoice was created'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('payer') + foreignKey('payee');
  $indexes = index('payer') + index('payee') + index('created') + index('status');
  $schema['r_invoices'] = dbTable(t('Record of all invoices in the region'), $fields, 'nvid', $foreignKeys, $indexes);
  
//  foreach (ray('type taking goods payerAgent payeeAgent payerFor payeeFor') as $one) unset($fields[$one]);

  // table r_usd
  $fields = array(
    'txid' => dbFld(t('the unique transaction ID'), 'serial big'), 
    'tid' => dbFld("payer's transaction ID", 'int 11'), // set, but currently UNUSED
    'amount' => dbFld(t('amount of transfer'), 'numeric 11,2'), 
    'payee' => dbFld(t('CG account record ID'), 'int big'), 
    'created' => dbFld(t('Unixtime transaction was created'), 'int 11', 0),
    'completed' => dbFld(t('Unixtime transaction was completed'), 'int 11', 0), // for deposits / withdrawals
    'deposit' => dbFld(t('Unixtime transfer check was printed and deposited'), 'int 11', 0),
    'bankAccount' => dbFld(t('Bank account for the transfer'), 'blob'), // "blob tiny" does not exist
    'risk' => dbFld(t('suspiciousness rating'), 'float medium', NULL), // NULL means never set yet
    'risks' => dbFld(t('list of risk factors'), 'int big', 0, TRUE),
    'channel' => dbFld(t('through what medium was the transaction entered'), 'int tiny'),
//    'members' => isGAME ? dbFld(t('number of members represented'), 'int medium', 1) : NULL,
  );
  $foreignKeys = foreignKey('payee');
  $indexes = index('created') + index('payee') + index('deposit');
  $schema['r_usd'] = dbTable(t('Record of transfers to or from a bank account'), $fields, 'txid', $foreignKeys, $indexes);

  // table r_banks
  $fields = array(
    'route' => dbFld(t('routing number'), 'int 9'),
    'branch' => dbFld(t('is this a branch office'), 'int tiny'),
    'fedRoute' => dbFld(t('routing number of servicing Fed'), 'int 9'),
    'type' => dbFld(t('bank type'), 'int tiny'), // 0=fed bank 1=use routing 2=use newRouting
    'modified' => dbFld(t('date modified'), 'char 6'),
    'newRoute' => dbFld(t('new routing number'), 'int 9'),
    'name' => dbFld(t('bank name'), 'varchar 36'),
    'address' => dbFld(t('bank address'), 'varchar 36'),
    'city' => dbFld(t('bank city'), 'varchar 20'),
    'state' => dbFld(t('bank state'), 'char 2'),
    'zip' => dbFld(t('bank zipcode'), 'char 9'),
    'phone' => dbFld(t('bank phone'), 'char 10'),
    'status' => dbFld(t('status'), 'char 1'), // 1
    'view' => dbFld(t('status'), 'char 1'), // 1
  );
  $foreignKeys = [];
  $indexes = index('newroute');
  $schema['r_banks'] = dbTable(t('Bank routing numbers'), $fields, 'route', $foreignKeys, $indexes);
  
  // table r_stakes
  $fields = array(
    'stakeid' => dbFld(t('record ID'), 'serial big'),
    'uid' => dbFld(t('member record ID'), 'int big', 0), 
    'clubid' => dbFld(t('investment club record ID'), 'int big', 0), 
    'stake' => dbFld(t('member stake in the club'), 'numeric 11,2', 0),
    'request' => dbFld(t('request to change stake by this amount'), 'numeric 11,2', 0),
    'joined' => dbFld(t('when this member joined the club'), 'int 11', 0),
    'requestedOut' => dbFld(t('when this member last requested cash out'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid') + foreignKey('clubid');
  $indexes = index('uid') + index('requestedOut');
  $schema['r_stakes'] = dbTable(t('member stakes in an investment club'), $fields, 'stakeid', $foreignKeys, $indexes);
  
  // table r_shares
  $fields = array(
    'shid' => dbFld(t('record ID'), 'serial big'),
    'vestid' => dbFld(t('investment record ID'), 'int'), 
    'shares' => dbFld(t('club shares in the investment bought or (if <0) sold'), 'int 11', 0), 
    'pending' => dbFld(t('number of shares to buy or (if <0) sell ASAP'), 'int 11', 0),
    'when' => dbFld(t('Unixtime investment made'), 'int 11'),
//    'sold' => dbFld(t('Unixtime investment totally sold'), 'int 11'),
  );
  $foreignKeys = foreignKey('vestid');
  $indexes = index('vestid');
  $schema['r_shares'] = dbTable(t('club stakes in investments'), $fields, 'shid', $foreignKeys, $indexes);
  
  // table r_investments
  $fields = array(
    'vestid' => dbFld(t('record ID'), 'serial big'),
    'coid' => dbFld(t('member company record ID'), 'int big'), 
    'clubid' => dbFld(t('investment club record ID'), 'int big'), 
    'proposedBy' => dbFld(t('account record ID of proposer'), 'int big'),
    'investment' => dbFld(t('description of investment'), 'text medium'),
    'return' => dbFld(t('predicted or actual APR'), 'numeric 10,3'), // see flags
    'types' => dbFld(t('D=dividends I=interest T=tax-exempt interest'), 'varchar 4'),
    'terms' => dbFld(t('investment terms'), 'text'),
    'assets' => dbFld(t('company assets, bond, or collateral'), 'numeric 11,2'), 
    'offering' => dbFld(t('size of offering'), 'numeric 11,2'), 
    'price' => dbFld(t('price per share'), 'numeric 11,2'), 
    'character' => dbFld(t('assessment of the integrity and determination of the owners'), 'text medium'),
    'strength' => dbFld(t('company\'s financial strength (0 to 100)'), 'int tiny'), 
    'web' => dbFld(t('impression on the web (0 to 100)'), 'int tiny'),
    'history' => dbFld(t('past repayment success (0 to 100)'), 'int tiny'), // 50=none
    'soundness' => dbFld(t('overall how sound is this investment (0 to 100)'), 'int tiny'), 
// NO use only r_shares:bought    'approved' => dbFld(t('Unixtime investment approved (negative if rejected)'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('coid') + foreignKey('clubid') + foreignKey('proposedBy');
  $indexes = index('coid') + index('proposedBy');
  $schema['r_investments'] = dbTable(t('potential and actual investments'), $fields, 'vestid', $foreignKeys, $indexes);

  // table r_ratings
  $fields = array(
    'ratingid' => dbFld(t('record ID'), 'serial big'),
    'vestid' => dbFld(t('investment record ID'), 'int'), 
    'uid' => dbFld(t('member record ID'), 'int big'), 
    'good' => dbFld(t('how well this investment serves the common good (0-100)'), 'int tiny'), 
//    'internet' => dbFld(t('how good the company looks on the web (0-100)'), 'int tiny'), 
    'patronage' => dbFld(t('how much the member will spend here monthly'), 'numeric 11,2'),
    'comments' => dbFld(t('description of investment'), 'text medium'),
  );
  $foreignKeys = foreignKey('vestid') + foreignKey('uid');
  $indexes = index('vestid') + index('uid');
  $schema['r_ratings'] = dbTable(t('how a member rates an investment'), $fields, 'ratingid', $foreignKeys, $indexes);

  // table r_ips
  $fields = array(
    'ip' => dbFld(t('ip address'), 'varchar 39'),
    'uid' => dbFld(t('account record ID'), 'int big'), 
    'device' => dbFld(t('device code'), 'varchar 255'), // device identifier (unique to each device/type combo)
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_ips'] = dbTable(t('IP addresses of approved accounts'), $fields, 'ip', $foreignKeys, $indexes);
  
  // table r_gifts
  $fields = array(
    'donid' => dbFld(t('gift record id'), 'serial'),
    'uid' => dbFld(t('uid of account that made the gift'), 'int big'),
    'amount' => dbFld(t('amount of gift'), 'numeric 11,2'),
    'often' => dbFld(t('recurring how often (Y, Q, M, 1)'), 'varchar 1'),
    'honor' => dbFld(t('what type of honor'), 'varchar 10'), // "share" = gift is "sharing" rewards with CGF
    'honored' => dbFld(t('who is honored'), 'text medium'),
    'completed' => dbFld(t('Unixtime donation was completed'), 'int 11', 0),
    'giftDate' => dbFld(t('date/time of gift'), 'int 11', 0),
    'share' => dbFld(t('percentage of rebates/bonuses to donate to CGF'), 'numeric 6,3'), // -1 = N/A
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_gifts'] = dbTable(t('Membership gift details'), $fields, 'donid', $foreignKeys, $indexes);

  // table r_user_industries
  $fields = array(
    'id' => dbFld(t('user industry record id'), 'serial'),
    'iid' => dbFld(t('industry id'), 'int 11', 0),
    'uid' => dbFld(t('industry id'), 'int big', 0),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid') + index('iid');
  $schema['r_user_industries'] = dbTable(t('industries for each company'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_relations
  $fields = array(
    'reid' => dbFld(t('relationship record id'), 'serial big'),
    'main' => dbFld(t('uid of the account to which others are related'), 'int big'),
    'other' => dbFld(t('uid of an other account related to this account'), 'int big'), // eg owner, agent, employee, supplier (0 for a partner's customer with no account yet)
    'otherNum' => dbFld(t('sequence number of the other (starts with 1)'), 'int', 0), // used only for company cards, otherwise 0
//    'amount' => dbFld(t('average monthly amount paid to other'), 'numeric 11,2', 0), // typically to employee or supplier
//    'draw' => dbFld(t('whether credit can flow from main to other account'), 'int tiny', 0),
    'permission' => dbFld(t('what type of permission the other has on the main account'), 'int tiny', 0),
//    'employee' => dbFld(t('this other is an employee according to the main account (employer)'), 'int tiny', 0),
//    'isOwner' => dbFld(t('this other is an owner (or part owner) or family relation'), 'int tiny', 0),
//    'isCustomer' => dbFld(t('this other is a customer'), 'int tiny', 0), // 2 means <pay invoices automatically>
    'code' => dbFld(t('the (main) company\'s account ID for this other'), 'varchar 50'),
    'data' => dbFld(t('serialized array of parameters'), 'text medium'), // used for partner's customer data while other=0
    'flags' => dbFld(t('boolean type flags'), 'int big', 0, TRUE),
    'created' => dbFld(t('Unixtime record created'), 'int 11'),
  );
  $foreignKeys = foreignKey('main') + foreignKey('other');
  $indexes = index('main') + index('other');
  $schema['r_relations'] = dbTable(t('Who can manage which accounts, and how'), $fields, 'reid', $foreignKeys, $indexes);
    
  // table r_coupons
  $fields = array(
    'coupid' => dbFld(t('record id'), 'serial big'),
    'fromId' => dbFld(t('account ID of company offering the coupon'), 'int big', 0),
    'start' => dbFld(t('Unixtime coupon is first valid'), 'int 11', 0),
    'end' => dbFld(t('Unixtime after which coupon is no longer valid'), 'int 11', 0),
    'amount' => dbFld(t('amount of discount'), 'numeric 11,2'), // amount or percentage (if < 0)
		'on' => dbFld(t('discount on what'), 'varchar 255'),
    'minimum' => dbFld(t('minimum purchase amount to get the discount'), 'numeric 11,2', 0),
    'ulimit' => dbFld(t('maximum number of uses per member'), 'int 11', 0),
    'flags' => dbFld(t('boolean type flags'), 'int big', 0, TRUE),
  );
  $foreignKeys = foreignKey('fromId');
  $indexes = index('fromId');
  $schema['r_coupons'] = dbTable(t('Coupons'), $fields, 'coupid', $foreignKeys, $indexes);

  // table r_coupated
  $fields = array(
    'id' => dbFld(t('record id'), 'serial big'),
    'uid' => dbFld(t('account ID of account that used the coupon'), 'int big', 0),
    'coupid' => dbFld(t('coupon ID'), 'int big', 0),
    'uses' => dbFld(t('number of times this account has used this coupon OR iCode'), 'int', 0), // gift card uses this field for iCode of the card code
    'when' => dbFld(t('Unixtime the coupon was used'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('uid') + foreignKey('coupid');
  $indexes = index('uid') + index('coupid');
  $schema['r_coupated'] = dbTable(t('Coupons used'), $fields, 'id', $foreignKeys, $indexes);
  
  // table r_invites
  $fields = array(
    'id' => dbFld(t('record id'), 'serial big'),
    'code' => dbFld(t('secret invitation code'), 'varchar 64'),
    'email' => dbFld(t('email of invitee'), 'varchar 255'), // encrypted/searchable
    'inviter' => dbFld(t('uid of inviting member'), 'int big', 0),
    'invitee' => dbFld(t('uid of invited new member'), 'int big', 0), // 0 if not accepted yet
    'invited' => dbFld(t('date of invitation'), 'int 11', 0),
    'subject' => dbFld(t('email subject'), 'varchar 255'),
    'message' => dbFld(t('email message body'), 'text medium'),
  );
  $foreignKeys = foreignKey('inviter') + foreignKey('invitee');
  $indexes = index('inviter') + index('code');
  $schema['r_invites'] = dbTable(t('Who invited whom'), $fields, 'id', $foreignKeys, $indexes);

  // table r_request
  $fields = array(
    'listid' => dbFld(t('record id'), 'int big'), // pointer into origin table
    'created' => dbFld(t('Unixtime record created'), 'int 11'),
    'first' => dbFld(t('first name of the individual'), 'varchar 60'),
    'last' => dbFld(t('last name of the individual'), 'varchar 60'),
    'phone' => dbFld(t('contact phone (no punctuation)'), 'varchar 255'), // encrypted/searchable
    'email' => dbFld(t('email of invitee'), 'varchar 255'), // encrypted/searchable
    'zip' => dbFld(t('postal code (no punctuation)'), 'varchar 60'),
    'ctty' => dbFld(t('uid of this requester\'s Common Good Community'), 'int big'), // gleaned from zip
    'done' => dbFld(t('are we done with this request'), 'int tiny', 0), 
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_request'] = dbTable(t('Requests to be invited'), $fields, 'listid', $foreignKeys, $indexes);
      
  // table r_notices
  $fields = array(
    'msgid' => dbFld(t('notice record id'), 'serial'),
    'uid' => dbFld(t('account record ID of member notified'), 'int big'),
    'created' => dbFld(t('date of notice'), 'int 11', 0),
    'sent' => dbFld(t('date sent (0 if not sent yet)'), 'int 11', 0),
    'message' => dbFld(t('the notice text'), 'text medium'),
  );
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_notices'] = dbTable(t('Message digest buffer'), $fields, 'msgid', $foreignKeys, $indexes);

  // table r_regions
  /*
  $fields = array(
    'region' => dbFld(t('region id'), 'char 3'),
    'state' => dbFld(t('state or province abbreviation'), 'char 2'),
  );
  $foreignKeys = [];
  $indexes = index('state');
  $schema['r_regions'] = dbTable(t('rCredits regions'), $fields, 'region', $foreignKeys, $indexes);
  */

  // table r_near
  $fields = [
    'uid1' => dbFld(t('account record ID of one account'), 'int big'),
    'uid2' => dbFld(t('account record ID of other account'), 'int big'),
    'weight' => dbFld(t('number of connections'), 'int medium'),
  ];
  $foreignKeys = foreignKey('uid1', 'uid2');
  $indexes = [];
  $schema['r_near'] = dbTable(t('How members are connected'), $fields, ['uid1', 'uid2'], $foreignKeys, $indexes);

  /*
  // table r_votes
  $fields = [
    'vid' => dbFld(t('vote record id'), 'serial'),
    'uid' => dbFld(t('account record ID'), 'int big'),
    'issue' => dbFld(t('name of the question being voted on'), 'varchar 255'),
    'vote' => dbFld(t('how the account voted on this issue'), 'int medium'),
  ];
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_votes'] = dbTable(t('How members voted on issues'), $fields, 'vid', $foreignKeys, $indexes);
*/

  // table r_tous (messages to us)
  $fields = [
    'id' => dbFld(t('vote record id'), 'serial'),
    'uid' => dbFld(t('account record ID'), 'int big'),
    'time' => dbFld(t('date/time of message'), 'int 11', 0),
    'message' => dbFld(t('the message'), 'blob'),
  ];
  $foreignKeys = foreignKey('uid');
  $indexes = index('uid');
  $schema['r_tous'] = dbTable(t('Secure messages sent from member to us.'), $fields, 'id', $foreignKeys, $indexes);

  // VOTING

  // table r_events
  $fields = array(
    'id' => dbFld(t('record id'), 'serial'),
    'ctty' => dbFld(t('what community the event is in'), 'int big'),
    'type' => dbFld(t('event type (I=in person, V=vote, G=grading, P=RFP)'), 'char 1'),
    'event' => dbFld(t('name of event'), 'varchar 255'),
    'details' => dbFld(t('event details'), 'text big'), 
    'start' => dbFld(t('Unixtime event begins'), 'int 11', 0),
    'end' => dbFld(t('Unixtime event ends'), 'int 11', 0),
  );
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_events'] = dbTable(t('Events in each community\'s democratic process'), $fields, 'id', $foreignKeys, $indexes);
    
  // table r_questions
  $fields = [
    'id' => dbFld(t('question record id'), 'serial big'),
    'event' => dbFld(t('what event this question is part of'), 'int big'),
//    'ctty' => dbFld(t('community or region record id'), 'int big'),
//	`phase` CHAR(1) DEFAULT 'I'), // current phase: I=ideas P=proposals, [D=discussion], V=voting, R=results
    'repeats' => dbFld(t('pointer to question that this is a revote on (0=none)'), 'int big', 0),
    'repeatedBy' => dbFld(t('pointer to question that is a revote of this one (0=none))'), 'int big', 0),
//    'endIdeas' => dbFld(t('when does the Ideas phase end'), 'int 11', 0),
//    'endProposals' => dbFld(t('when does the Proposals phase end'), 'int 11', 0), // not currently used
//    'endVoting' => dbFld(t('when does the Voting phase end'), 'int 11', 0),
//    'endGrading' => dbFld(t('when does the Grading phase end'), 'int 11', 0), // grading specific proposals in Budget questions
    'text' => dbFld(t('text of the question'), 'text medium'),
    'detail' => dbFld(t('additional detail about the question'), 'text medium'),
//    'linkIdeas' => dbFld(t('where to brainstorm ideas (and see the list)'), 'text tiny'),
//    'linkProposals' => dbFld(t('where to propose options (and see the discussion)'), 'text tiny'),
    'linkDiscussion' => dbFld(t('link to online discussion of the issue'), 'text tiny'),
    'type' => dbFld(t('vote type M=multiple choice, B=budget (penny vote), R=range, E=essay'), 'char 1', 'M'),
    'units' => dbFld(t('budget units (defaults to money, measured in the community\'s national currency)'), 'varchar 255'),
    'budget' => dbFld(t('how much (money) is to be budgeted'), 'numeric 11,0', 0),
    'minVeto' => dbFld(t('minimum veto fraction of vote, to force reconsideration'), 'numeric 5,3'), // set by ctty, here for history (no effect on M results)
    'optOrder' => dbFld(t('option order'), 'char 1', 'S'), // S=shuffle (randomize), Z=reverse sometimes, N=no change (static)
    'voteCount' => dbFld(t('total number of votes'), 'int 11', 0),
//    'modified' => dbFld(t('date/time last modified'), 'int 11', 0),
    'result' => dbFld(t('results of the vote or grading'), 'text big'),
    'created' => dbFld(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty');
  $schema['r_questions'] = dbTable(t('Questions to be voted on'), $fields, 'id', $foreignKeys, $indexes);

  // table r_options
  $fields = [
    'id' => dbFld(t('option record id'), 'serial big'),
    'question' => dbFld(t('question for which this is an option'), 'int big', 0),
    'text' => dbFld(t('text of the option'), 'text tiny'),
    'detail' => dbFld(t('additional detail about the option'), 'text medium'),
    'displayOrder' => dbFld(t('where to display this option in the order'), 'int tiny', 0),
    'minimum' => dbFld(t('the least (money) to budget for this option'), 'int big', 0),
    'maximum' => dbFld(t('the most (money) to budget for this option'), 'int big', 0), // (0=no maximum) */
	  'mandatory' => dbFld(t('is the minimum required?'), 'int tiny', 0),
	  'averageGrade' => dbFld(t('average grade (in a penny vote, the fraction of all votes)'), 'numeric 5,3', 0),
	  'averageMax' => dbFld(t('average maximum grade (for range votes)'), 'numeric 5,3', 0),
    'vetoes' => dbFld(t('number of vetoes for this option'), 'int', 0),
    'modified' => dbFld(t('date/time last modified'), 'int 11', 0),
    'created' => dbFld(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('question', 'id', 'r_questions');
  $indexes = index('question');
  $schema['r_options'] = dbTable(t('Options for a question to be voted on'), $fields, 'id', $foreignKeys, $indexes);

  // table r_pairs (not currently used)
  $fields = [
    'id' => dbFld(t('pairs record id'), 'serial big'),
    'option1' => dbFld(t('record id of one option'), 'int big', 0),
    'option2' => dbFld(t('record id of the other option'), 'int big', 0),
    'prefer1' => dbFld(t('how many voters prefer the option1'), 'int 11', 0),
    'prefer2' => dbFld(t('how many voters prefer the option2'), 'int 11', 0),
    'nopreference' => dbFld(t('how many voters had no preference between the two options'), 'int 11', 0),
    'raw' => dbFld(t('true if this record is calculated without counting proxies'), 'int tiny', 0),
	  'created' => dbFld(t('date/time created'), 'int 11', 0), // normally created just once, when election is over (except for demos)
  ];
  $foreignKeys = foreignKey('option1', 'id', 'r_options') + foreignKey('option2', 'id', 'r_options');
  $indexes = index('option1') + index('option2');
  $schema['r_pairs'] = dbTable(t('Counts of preferences of one option over another'), $fields, 'id', $foreignKeys, $indexes);

  // table r_ballots
  $fields = [
    'id' => dbFld(t('ballot record id'), 'serial big'),
    'question' => dbFld(t('question or proposal voted on by a particular voter'), 'int big', 0), // <0 for proposals
    'voter' => dbFld(t('record id of voter whose ballot this is'), 'int big', 0),
    'proxy' => dbFld(t('record id of voter who actually voted on behalf of the voter'), 'int big', 0),
    'modified' => dbFld(t('date/time last modified'), 'int 11', 0),
    'created' => dbFld(t('date/time created'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('question', 'id', 'r_questions') + foreignKey('voter') + foreignKey('proxy');
  $indexes = index('question') + index('voter') + index('proxy');
  $schema['r_ballots'] = dbTable(t('A votable question addressed by a particular voter'), $fields, 'id', $foreignKeys, $indexes);

  // table r_votes
  $fields = [
    'id' => dbFld(t('vote record id'), 'serial big'),
    'ballot' => dbFld(t('ballot on which this option is being graded'), 'int big', 0),
    'option' => dbFld(t('option being graded'), 'int big', 0),
    'grade' => dbFld(t('grade given by a particular voter for this option'), 'int', -1), // In a B vote, this is 10 * the pct (ddd.d) else 3*letter (-1=blank -2=veto); in range-type vote this is the voter's chosen min
    'gradeMax' => dbFld(t('maximum grade given by a particular voter for this range-type option'), 'int', -1),
    'displayOrder' => dbFld(t('what order options were shown in to this voter'), 'int', 0), // 0 is first
    'text' => dbFld(t('what was voter\'s comment or moral objection to this option'), 'text big'),
    'isVeto' => dbFld(t('this is a veto (not a canceled veto or mere comment)'), 'int tiny', 0),
    'modified' => dbFld(t('date/time last modified'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('ballot', 'id', 'r_ballots') + foreignKey('option', 'id', 'r_options');
  $indexes = index('ballot') + index('option');
  $schema['r_votes'] = dbTable(t('What grade a particular voter gave a particular option (for a particular question) on a particular question ballot'), $fields, 'id', $foreignKeys, $indexes);

  // table r_proposals
  $fields = [
    'id' => dbFld(t('proposal record id'), 'serial big'),
    'event' => dbFld(t('what event this question is part of'), 'int big'),
    'project' => dbFld(t('project title'), 'varchar 255'),
//    'ctty' => dbFld(t('community or region record id'), 'int big'),
    'overview' => dbFld(t('project overview'), 'text medium'),
    'categories' => dbFld(t('funding categories (space-separated list)'), 'text tiny'),
    'purpose' => dbFld(t('why the project is needed'), 'text medium'),
		'systemic' => dbFld(t('how the project promotes systemic change'), 'text medium'),
    'where' => dbFld(t('zipcode where the project will take place'), 'text tiny'),
    'when' => dbFld(t('project start date'), 'int 11'),
    'until' => dbFld(t('project end date'), 'int 11'),
    'how' => dbFld(t('how the project will be accomplished'), 'text medium'),
    'amount' => dbFld(t('amount of funding requested'), 'numeric 11,2'),
    'type' => dbFld(t('type of funding'), 'int tiny'), // 0=investment 1=loan 2=incentive 3=grant 4=other (no longer used)
		'recovery' => dbFld(t('how funds will return to the Community Fund'), 'int medium', 0),
    'budgetTotal' => dbFld(t('total expense budget'), 'numeric 11,2'),
    'budget' => dbFld(t('detailed project budget'), 'text medium'),
    'contingency' => dbFld(t('how the project organizers will cope with less funding than requested'), 'text medium'),
    'qualifications' => dbFld(t('qualifications of project staff'), 'text medium'),
    'evaluation' => dbFld(t('how the success of the project will be evaluated'), 'text medium'),
    'name' => dbFld(t('individual or organization making the proposal'), 'varchar 255'),
    'contact' => dbFld(t('contact person (or "self")'), 'varchar 255'),
    'phone' => dbFld(t('contact phone'), 'varchar 255'),
    'email' => dbFld(t('contact email'), 'varchar 255'),
    'sponsor' => dbFld(t('member(s) sponsoring the project proposal'), 'text tiny'),
  ];
  $foreignKeys = foreignKey('ctty');
  $indexes = index('ctty') + index('name');
  $schema['r_proposals'] = dbTable(t('funding proposals'), $fields, 'id', $foreignKeys, $indexes);
  

  
  // table r_votenotes
  /*
  $fields = [
    'id' => dbFld(t('veto record id'), 'serial big'),
    'vote' => dbFld(t('what option (on a particular ballot) did the voter veto or comment on'), 'int big', 0),
    'modified' => dbFld(t('date/time last modified'), 'int 11', 0),
  ];
  $foreignKeys = foreignKey('vote'), 'id', 'r_votes');
  $indexes = index('vote');
  $schema['r_votenotes'] = dbTable('Details about a voter\'s veto of a particular option'), $fields, 'id', $foreignKeys, $indexes);
  */
  
  // table r_nonmembers (use -make instead)
/*  $fields = array(
    'id' => dbFld(t('non-member company record id', 'serial'),
    'company' => dbFld(t('company name', 'varchar 60'),
    'potential' => dbFld(t('number of members who shop there', 'int 6', 0),
  );
  $schema['r_nonmembers'] = dbTable('Local companies we want to recruit', $fields, 'id', NULL, NULL);
  */
  
/*
  // table r_counts
  $fields = array(
    'type' => dbFld(t('what is being counted', 'varchar 255'),
    'channel' => dbFld(t('through what channel did it happen', 'int 11'),
    'count' => dbFld(t('the count', 'int big', 0),
  );
  $schema['r_counts'] = dbTable('How many this and that overall, through the various channels', $fields, array('type', 'channel'));
*/
  
  // table r_scores
  /*
  $fields = array(
    'company' => dbFld(t('uid of company', 'int 11'),
    'count' => dbFld(t('for the rTraders that have this many places to buy with rCredits', 'int 11', 0),
    'score' => dbFld(t('this many of those rTraders would buy from this company', 'int 11'),
  );
  $foreignKeys = foreignKey('company');
  $indexes = index('company');
  $schema['r_scores'] = dbTable('Best companies to promote to rTrader in the current round', $fields, NULL, $foreignKeys, $indexes);
  */
  
  /*  
  dbFld(t('r_areas', 'Area Code', 'telephone area code', 'char 3'); 
  dbFld(t('r_areas', 'Region', 'state, province, or territory', 'varchar 24'); 
  
  */

  foreach (ray(TRACK_DELETED) as $table) {
    $xtable = $table == 'users' ? 'x_users' : str_replace('r_', 'x_', $table);
    $schema[$xtable] = $schema[$table];
/*    $fields = $schema[$xtable]['fields'];
    if ($key = @$schema[$xtable]['primary key']) {
      if ($fields[$key]['type'] == 'serial') $schema[$xtable]['fields'][$key]['type'] = 'int'; // permits dropping primary key
    } */
    $deleted = dbFld(t('Unixtime record was deleted'), 'int 11', 0);
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

/**
 * Set up a database table specification.
 */
function dbTable($description, $fields, $primaryKey, $foreignKeys = [], $indexes = []) {
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
 * Setup a database field specification
 *
 * @param string $description: description of the field
 * @param string $type: data type and optional size, separated by a space
 *   a numeric size is interpreted as a string's maximum length
 *   a size consisting of two numbers separated by a comma is interpreted as precision and scale
 * @param string $default: the field's default value, if any
 * @param bool $unsigned: <field is an unsigned number>
 * @return array: the field's specification array
 */
function dbFld($description = '', $type = 'varchar 255', $default = NULL, $unsigned = FALSE) {
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
