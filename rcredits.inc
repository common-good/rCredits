<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in rcredits-backend.inc (i)
 *   - utilities, which are in rcredits-util.inc (u)
 */
 
namespace rCredits; // typically abbreviated as "r"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Testing as t;
use rCredits\Util as u;
use rCredits\Backend as be;

// definitions used in settings file
define('HOUR_SECS', 3600);
define('DAY_SECS', 24 * HOUR_SECS);
define('WEEK_SECS', 7 * DAY_SECS);
define('YEAR_SECS', 365.25 * DAY_SECS);
define('US_COUNTRY_ID', 1228); // United States

require_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server
require_once __DIR__ . '/rcredits-db.inc';
require_once __DIR__ . '/acct.class';

define('MINI_LAUNCH', TRUE);
define('CGF_EMAIL', isDEV ? 'wspademan+cgf@gmail.com' : 'info@CommonGoodFinance.org'); // Common Good Finance email and other data
define('CGF_PHONE', '+14136281723');
define('CGF_ADDRESS', 'PO Box 21');
define('CGF_CITY', 'Ashfield');

define('R', '<span class="r">r</span>');
define('US', '<span class="us">us</span>');
define('R_VERSION', '1.1'); // software version
define('R_ROOT', DRUPAL_ROOT . '/sites/all/modules/rcredits');
define('R_CONCEALED_FIELDS', 'code password pass oneTimeLoginUrl'); // fields to leave out of log
define('R_DATE_FIELDS', 'created completed sent invited giftDate signed login access yesterday'); // date field names for testing
define('R_DATA_FIELDS', 'newAccount by calling wants legalName website description selling coFlags stats verifyBy oneTimePass relationsOk signupCo'); // fields stored in the account's "data" array field (serialized)
define('R_MONEY_FIELDS', 'usd r rewards floor committed'); // financial amount fields in account record
define('R_SECURE_FIELDS', 'usdType usdPhone usdEmail usdPass usdPin auth bankAccount pin cardCode cardCode2'); // non-personal encrypted fields (must be complete list)
define('R_SECRET_FIELDS', 'federalId dob idProof answer'); // personal identifying information
define('R_SECRET_URL', isPRODUCTION ? 'http://rcredits.org' : (isDEV ? 'http://localhost/rcredits-promo' : 'http://' . STAGE)); // where to store secrets
define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves
define('R_CRON_PERIOD', 5*60); // how many seconds between cron runs (must divide an hour evenly)
define('R_MAX_TASK_TIME', 1*60); // what's the longest a queued cron task might take
define('DRUPAL_UID_TABLES', 'authmap file_managed history node node_revision sessions users_roles watchdog');

define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_MIN_PHOTO_WIDTH', 240); // how wide to make profile pictures (unless too long)
define('R_MAX_PHOTO_WIDTH', 1024);
define('R_MAX_UPLOAD_SIZE', 3); // maximum upload size in MB
define('R_MAX_PHOTOID_SIZE', 2.5); // Dwolla's limit (MB)
define('R_URL_SLASH', '%_%'); // Drupal gets confused by slashes in a url, even if urlencoded
define('R_MAX_DAILY_EXCHANGE', 1000); // per Common Good Finance policy and to avoid formal FinCEN regulation
define('R_MAX_RTX', 1000); // never transfer more rCredits than this at once (self-imposed limit for safety)
define('R_INVITE_DAYS', 30); // number of days to accept invitation to membership
define('R_MINR_DEFAULT', 200); // suggested minimum amount to hold in rCredits
//define('R_MAXR_DEFAULT', -1); // suggested maximum amount to hold in rCredits (-1 = no limit)
define('R_CARDCODE_LEN', 14); // length of rCard security code (long to deter easy copying, max 14 for small QR)
define('R_CARD_COST', 5); // cost to order a company or replacement rCard
define('R_FREEZE_TIME', FALSE); // how long to freeze newly bought rCredits (was '2 weeks')
define('R_DEFAULT_PURPOSE', 'rCredits/USD exchange'); // default transaction purpose
define('R_CHUNK', 10); // chunk size for buying rCredits (sales amounts are a multiple of this)
define('R_BANK_MIN', 10); // granularity of withdrawals from bank account
define('R_SUGGEST_BUMP', 1.25); // suggested minimum multiplier, so as not to unnecessarily sell rCredits
define('R_VPAYMENT_MIN', 0.05); // minimum payment exchange (where bonus rounds to .01)
define('R_COMMERCIAL_ASPECT', '4:3'); // width-to-height ratio for commercial profile picture
define('R_PERSONAL_ASPECT', '3:4'); // width-to-height ratio for personal profile picture
define('R_PIXEL_FACTOR', 200); // what to multiply the aspect ratio components by, to get pixel dimensions
define('R_GET_AUTH_URI', 'usd-get-auth'); // where Dwolla calls us back for getting auth code
define('R_INDIVIDUAL_GIFT', 50); // suggested contribution for individuals
define('R_COMPANY_GIFT', 250); // suggested contribution for companies
define('R_BUYS_BEFORE_BONUS', MINI_LAUNCH ? 1 : 3); // number of purchases required before signup bonus
define('R_REGION_MAX', isDEV ? 46000 : 3000000000); // maximum uids in a region (46k,3T ~sqrt(PHP_INT_MAX))
define('R_ACCT_REGION', '(-FLOOR((uid-1)/' . R_REGION_MAX . ')-1)'); // SQL region expression
define('R_CTTY_UID0', -R_REGION_MAX); // community uids are less than this (farther from zero)
define('R_DW_LINK', 'http://refer.dwolla.com/a/clk/3bf8cy'); // (credits CGF for referral)
define('R_SIGNIN_HOURS', 24); // window for signing in after registering or resetting password
define('R_MEMBER_MARK', '.'); // divides region from specific identifier in member IDs
define('R_AGENT_MARK', ':'); // divides region from specific identifier in relational IDs
define('R_AGENT_URL_MARK', '-'); // use this as a substitute for R_AGENT_MARK in URLs (else domain forwarding fails)
define('R_MARKS', '[\\' . R_MEMBER_MARK . '\\' . R_AGENT_MARK . '\\' . R_AGENT_URL_MARK . ']'); // any of those
define('R_ANCIENT_EFT', 10); // how long, at most, an EFT might take (transferring funds to/from bank)
define('R_REQUIRED', ' <span class="form-required" title="This field is required.">*</span>');
define('R_CTTY_USD_SHARE', .5); // fraction of a community's total USD to hold in the community account
define('R_CC_RATE', 2.5); // typical credit card percentage rate
define('R_CODE_LEN', 20); // length of most random identifiers

define('DW_SITE', isPRODUCTION ? 'https://www.dwolla.com' : 'https://uat.dwolla.com');
//                               : 'http://new.rcredits.org/stage.php?stageWord=' . R_STAGE_WORD . '&request=');
//define('DW_TEST_ACCT', '8127139234'); // "reflector" account on www (totally reimburses anything under $10)
define('DW_TEST_ACCT', '8127419900'); // "reflector" account on www (totally reimburses anything under $10)
define('DW_TESTER_ACCT', '8127413762'); // william's account on Dwolla sandbox
define('DW_FEE_THRESHOLD', 10); // over this amount, Dwolla charges a fee
define('DW_FEE', .25); // the fee
define('DW_VERIFY_ROOF', 20); // bank verification deposits are less than this (actually less than 15 cents)
define('DW_STEPS', 'Email Phone Address Ssn Kba AuthorizedRepresentative Finished'); // registration steps

define('R_USPS_WEB_TOOLS_ID', '921COMMO5331'); // USPS API credentials

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');
define('R_PICTURE_DIR', '/sites/default/files/pictures'); // precede this by DRUPAL_ROOT (file) or $base_url (url)
define('R_PROOF_DIR', '/sites/default/files/proofs');
define('R_DFT_PICTURE', R_PICTURE_DIR . '/no-photo-available.jpg'); // default profile picture
define('R_DFT_PICTURE_WIDE', R_PICTURE_DIR . '/no-photo-available-wide.jpg'); // default profile picture

// account type bits
define('R_PERSONAL', 0);
$GLOBALS['account types'] = array(t('personal'), t('sole proprietor'), t('partnership'), t('private corporation'), t('nonprofit or government'), t('publicly-traded corporation'));

// Commercial Account flags (bits)
define('CO_SOLE_PROPRIETOR', 1);
define('CO_PARTNERSHIP', 2);
define('CO_CORPORATION', 3);
define('CO_NONPROFIT', 4);
define('CO_WORKER_COOP', 5);
define('CO_CONSUMER_COOP', 6);
define('CO_GOVERNMENT', 7);
define('CO_PUBLICLY_TRADED', 8);

define('CO_CANS', 15); // company permission bits
define('CO_u15', 15);
define('CO_REFUND', 16);
define('CO_SELL_CASH', 17);
define('CO_BUY_CASH', 18);
define('CO_u19', 19);
define('CO_u20', 20);
define('CO_REQUIRE_CASHIER', 21); // prohibit anonymous use of rPOS devices AND allow other companies to use

define('CO_CAN_BIT_DESCS', 'u0,refund,sell cash,buy cash');

// Permissions, options, and flags (flags field in account record)
// If we need more bits, create a flags2 field and make the B_ defs negative for acct::hasBit and setBit
define('B_MEMBER', 0); // signed the membership agreement, etc. (ready to be approved)
define('B_DW', 1); // has a connected Dwolla account
define('B_OK', 2); // approved to use rCredits for real (set by admin when ready)
define('B_BONA', 3); // got signup bonus (good circles - somehow got more r+usd than the initial gift)
define('B_BANK', 4); // has an effective connected bank account
define('B_PERSON', 5); // personal account
define('B_COMPANY', 6); // company account (could be personal also, for independent contractor)
define('B_PHOTOID', 7); // photoid required and not yet approved
define('B_CHARGE', 8); // UNUSED (any company can do this) can charge customers without confirmation
define('B_u9', 9); // 
define('B_u10', 10); //
define('B_u11', 11); // 
define('B_PAYROLL', 12); // automatically pay employees their total daily salary - employer chooses pay period)
define('B_NOTICE_WEEKLY', 13); // send notices only weekly
define('B_NOTICE_MONTHLY', 14); // send notices only monthly
//define('B_NOTIFY_EMAIL', 15); // notifications by email
define('B_u15', 15); //
define('B_NOTIFY_SMS', 16); // notifications by SMS
define('B_ELECTRONIC', 17); // will accept electronic statements
define('B_SECRET_BAL', 18); // don't let merchants tell me my balance on request
define('B_u19', 19); //
define('B_DEBT_OK', 20); // member chose to permit sub-zero balances
define('B_DRAWS', 21); // this account draws on one or more others
define('B_SAVINGS', 22); // the connected bank account is a savings or money market account
define('B_u23', 23); // 
define('B_u24', 24); // 
define('B_u25', 25); // 
define('B_u26', 26); // 
define('B_u27', 27); // 
define('B_u28', 28); // 
define('B_CTTY_ADMIN', 29); // is a community administrator
define('B_ADMIN', 30); // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('B_CLOSED', 31); // account is closed (data[newAccount] has new qid)
define('B_MAX', 31); // maximum bit number
//define('B_DFT', (1 << B_NOTIFY_EMAIL) | 0); // set what bits when creating a new account
define('B_DFT', 0 | 0); // set what bits when creating a new account
define('B_ADMINABLE', 'person company member dw photoid ctty_admin closed'); // for cttyAdmin to set
define('B_LIST', 'member dw ok bona bank person company photoid charge u9 u10 u11 payroll weekly monthly u15 sms electro secret u19 debt draws savings u23 u24 u25 u26 u27 u28 cadmin admin closed');

define('R_SHORTFALL', 'GREATEST(0, minimum-(r+usd))'); // sql for shortfall in account (to draw in)
define('R_DEMAND', 'LEAST(u.usd, u.minimum-u.r)'); // definitely WANTS this many rCredits (sometimes negative)
define('R_CAPACITY', 'u.usd'); // how much rCredits is each account WILLING and able to buy

// for documentation of IS_OK etc. see u\SUBS()

// Relationship permissions used in calls to acct::can() -- stored (less B_RELATED) in r_relations
define('B_RELATED', 100); // has a relationship record
define('B_READ', 101); // can view Transaction History
define('B_SCAN', 102); // can scan rCard (or view -- permissions are cummulative)
define('B_SELL', 103); // can send invoices (or scan or view)
define('B_BUY', 104); // can make purchase for the account (or sell or scan or view)
define('B_MANAGE', 105); // can manage all aspects of the account
$GLOBALS['share permissions'] = array(t('no access'), t('read transactions'), t('scan rCards'), t('send invoices'), t('buy and sell'), t('manage account'));

// Other special pseudo-bit values for acct::can()
define('B_MANAGING_CTTY', 120); // ctty_admin is actually managing a qualified account
define('B_PERSONISH', 121); // personal account or is admin
define('B_DEV', 122); // isDEV or is admin
define('B_SHOW_ALL', 999); // special signal to acct::can() to return all permissions
$GLOBALS['permissions'] = 'related read sell buy manage - company charge - member partner steward - rtrader - - managing_ctty ctty_admin - admin';

define('FLOW_NONE', 0); // which way can credit flow, with another account
define('FLOW_TO', 1);
define('FLOW_FROM', 2);
define('FLOW_BOTH', 3);

// Transaction states
define('TX_STATES', t('denied deleted pending done tentative disputed'));

define('TX_STATE0', -2); // lowest index
define('TX_DENIED', -2); // invoice rejected by the non-originator
define('TX_DELETED', -1);
define('TX_PENDING', 0); // charged, waiting for confirmation (in all other states, transaction is complete)
define('TX_DONE', 1); // transactions with a positive state count as done
define('TX_TENTATIVE', 2); // CURRENTLY UNUSED done in rCredits, but waiting for USD funds instead (amount minus r is the expected amount)
define('TX_DISPUTED', 3); // unilaterally charged, but disputed by the payer (the charge stands, until settled)

// Transaction channels (roughly in order of simplicity and generality of messages)
define('TX_SYSTEM', 0);
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_SMART', 3); // smart phone or other computer-like device
define('TX_TONE', 4); // touch tone phone
define('TX_CRON', 5);
define('TX_CHANNELS', t('System SMS Web App Tone Cron'));

// Transaction types (all but type TX_TRANSFER and TX_BANK create rCredits)
define('TX_BANK', -9); // used only internally, to mark bank transfers
define('TX_REFUND', -3); // refund of fees (Dwolla)
define('TX_BONUS', -2); // sales bonus (negative reward types are invisible on transaction history)
define('TX_REBATE', -1);
define('TX_TRANSFER', 0); // normal fund transfer (usually for actual goods and services) -- not creating rC
define('TX_SIGNUP', 1); // signup incentive reward
define('TX_INFLATION', 3); // reward to conpensate for inflation
define('TX_PIONEER', 4); // pioneer reward (special early-stage rewards) -- might change later to TX_OTHER
define('TX_REWARDS', 4); // from here on, it's not "rewards"

define('TX_u4', 4);
define('TX_GRANT', 5);
define('TX_LOAN', 6);
define('TX_INVEST', 7); // equity investment (community purchase are TX_TRANSFER)
define('TX_u8', 8);
define('TX_FINE', 9);

define('TX_DEFAULT_PERIOD', 30); // number of days to show in transaction history, by default
define('TX_MAX_ROWS', 100); // maximum number of transactions to show on one screen
define('TX_DOWNLOAD_MONEY_FIELDS', 'frombank fromyou toyou reward net');

$GLOBALS['emailSubjects'] = array(
/*  'new-payment' => t('rCredits PAYMENT from {otherName}'),
  'new-charge' => t('rCredits CHARGE from {otherName}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {otherName}'),

  'new-invoice' => t('rCredits INVOICE from {otherName}'),
  'invoice-denied' => t('rCredits Invoice DENIED by {otherName}'),
  'invoice-canceled' => t('rCredits Invoice CANCELED by {otherName}'),

  'new-offer' => t('rCredits PAYMENT OFFER from {otherName}'),
  'offer-refused' => t('rCredits Payment REJECTED by {otherName}'),
  'offer-canceled' => t('rCredits Invoice CANCELED by {otherName}'),
*/
  'welcome' => t('Welcome to rCredits!'),
  'password-reset' => t('New rCredits PASSWORD'),
  'tell-staff' => t('rCredits STAFF ALERT: {topic}'),
  'notices' => t('rCredits Notices for {range}'),
  'message' => t('rCredits {topic}'),
);
$GLOBALS['topics'] = array( // topics for 'message' email
  'account suspended' => t('Account SUSPENDED'),
);

/**
 * Format and return args for reporting a transaction (old or new) to the current account.
 * Not all returned args are used for any particular type of transaction.
 * Note that templates and tests depend on the names of some of these fields
 * @param acct $acct: the account to report to (usually the current account)
 * @param array $tx: the transaction array (including xid)
 * @param int $getBalance: 0=don't get any balances, 1=get my balance and tid, 2=get other party's balance and tid also
 * @param float $shortfall: how much short my balance is, for a transaction (if any)
 * @return assoc array of args
 */
function txReportArgs($acct, $tx, $getMyBalance = FALSE, $shortfall = 0) {
  global $channel;
  extract($tx); // any or all standard transaction fields
  if ($data) if ($data = unserialize($data)) extract(u\just('rebate bonus', $data));
  global $lastXid; $lastXid = $xid; // remember last transaction for current user, for potential undo (here and on undo)

  $toMe = ($payee == $acct->id);
  $reverses = isset($data['undoes']);
  $role = $toMe ? 'payee' : 'payer';
  u\both($tx, 'purpose', $toMe, $payeeFor, $payerFor);
  u\both($tx, 'tid', $toMe, $payeeTid, $payerTid);
  list ($payerPurpose, $payeePurpose) = array($payerFor, $payeeFor); // clearer names, for substitutions
  $tofrom = $toMe ? t('from') : t('to');
  $uid = $otherUid = ($toMe ? $payer : $payee);
  $confirmAction = $toMe ? t('charge') : ($channel == TX_SMART ? t('credit') : t('pay'));
  u\both($tx, 'did', $toMe, t('charged'), ($channel == TX_SMART ? t('credited') : t('paid')));

  If ($amount < 0) { // for POS
    $amount = -$amount; // rephrase as positive
    $tofrom = t('to');
    $confirmAction = $toMe ? t('credit') : t('charge');
    u\both($tx, 'did', $toMe, $goods ?  t('refunded') : t('credited'), ($reverses and $goods) ? t('re-charged') : t('charged'));
  }

  $amount = u\fmtAmt($amount);
  $short = u\fmtAmt($shortfall);
  if ($getMyBalance) {
    if ($acct->can(B_BUY)) $balance = u\fmtAmt($acct->rAvailable('buy', TRUE));
  }
  
  if ($state != TX_PENDING and $state != TX_DISPUTED and $goods) {
//    u\both($tx, 'rewardType', $toMe, t('bonus'), t('rebate'));
    u\both($tx, 'rewardType', $toMe, t('reward'), t('reward'));
    u\both($tx, 'rewardAmount', $toMe, u\fmtAmt(@$bonus + 0), u\fmtAmt(@$rebate + 0));
  }
  $myName = $acct->fullName;
  $otherName = r\acct($otherUid)->fullName;
  $created = u\fmtDate($created);

  $success = TRUE;
  foreach (u\ray('r payer payee') as $one) unset($tx[$one]); // else preempts all subs that start thus
  return compact(u\ray('created xid role payerPurpose payeePurpose toMe confirmAction amount tofrom balance otherUid myName otherName short success')) + $tx;
}

/**
 * Return the rCredits account id for the given USD account id.
 * @param string $usdAccountId: the Dwolla ID (with no hyphens)
 * @param int $omit: an account id to ignore (optional)
 */
function whoseUsd($usdAccountId, $omit = NULL) {
  $usdAccount = bin2hex(u\ezencrypt(str_replace('-', '', $usdAccountId), R_WORD));
  $where = 'usdAccount=:usdAccount' . (@$omit ? ' AND uid<>:omit' : '');
  $subs = compact('usdAccount') + (@$omit ? compact('omit') : array());
  return db\lookup('uid', 'users', $where, $subs);
}

/**
 * Return the user's sequential transaction number
 * @param int $xid: the transaction record number
 * @param int $uid: the user's uid
 * @return string: the user's transaction number
 */
function xid2tid($xid, $uid) {
  $where =  ":uid IN (payer, payee) AND xid<:xid AND type>=0";
  return db\lookup('COUNT(xid)', 'r_txs', $where, compact('uid', 'xid')) + 1;
}

/**
 * Trade r in one account for USD in another
 * @param float $amount: the amount to trade (limited to R_MAX_DAILY_EXCHANGE)
 * @param int $uid1: uid of account with r to trade
 * @param int $uid2: uid of account with USD to trade
 * @param string $err: (RETURNED) an error message on failure
 * @return string $txid: USD transaction ID (from Dwolla) or FALSE on failure
 * Caller is responsible for determining whether the accounts have enough r and USD for the trade.
 */
function tradeR($amount, $uid1, $uid2, &$err = '') {
  if ($amount > R_MAX_DAILY_EXCHANGE) {$err = 'rTrade amount over max'; return FALSE;}
  $us1 = new r\usd($a1 = r\acct($uid1));
  $us2 = new r\usd($a2 = r\acct($uid2));
  u\EXPECT($a1->can(B_DW) and $a2->can(B_DW), 'no dw');

  r\usd::beginAtom();
  if (!$txid = $us2->send($amount, $us1, R_DEFAULT_PURPOSE, $err)) {
    r\usd::rollback(); // pretend nothing ever happened
    return FALSE;
  }
  r\usd::commit();
  return $txid;
}

/**
 * Record a transaction between two participants
 * and create related transactions for any applicable incentive rewards
 * @param assoc $info: transaction data
 *   amount: (required) how much to transfer
 *   payer, payee: (required) uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   etc.
 * @return array: the list of transactions created (main, rebate, bonus)
 */
function transact($info) {
  $txs = array();

  $DBTX = db_transaction();
  $txs[] = txSave(transactionSetup($info));
  transactionRewards($txs);
  cacheBothTotals($txs);
  unset($DBTX); // commit

  return $txs;
}

/**
 * Save the transaction
 * @param assoc $tx: transaction info.
 *   amount: (required) how much to transfer
 *   payer, payee: (required) uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   etc.
 * @return the transaction array, with extra info added
 */
function txSave($tx) {
  $xid = db\insert('r_txs', $tx);
  if (!@$tx['serial']) { // main transaction needs some touching up, now that we know the xid
    extract(u\just('payer payee', $tx));
    $payerTid = $payer > 0 ? r\xid2tid($xid, $payer) : 0;
    $payeeTid = $payee > 0 ? r\xid2tid($xid, $payee) : 0;
    $sql = "UPDATE r_txs SET serial=:xid, payerTid=:payerTid, payeeTid=:payeeTid WHERE xid=:xid";
    db\q($sql, compact(u\ray('xid payerTid payeeTid')));
    $serial = $xid; // set serial for this and remember it for dependents
  }
  return $tx + compact(u\ray('xid serial payerTid payeeTid'));
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 */
function transactionRewards(&$txs) {
  extract(u\just('type serial goods payer payee data state payerTid payeeTid created', $txs[0]));
  
  if ($type != TX_TRANSFER) return;
  if (!$goods) return; // quit unless rebates and bonuses apply
  if (owning($payer, $payee)) return; // no rewards for transactions between owner and owned
  $rewards = unserialize($data);
  
  foreach (array('rebate', 'bonus') as $type) {
    $amount = $rewards[$type];
    $to = $type == 'rebate' ? $payer : $payee;
    $tid = $type == 'rebate' ? $payerTid : $payeeTid;
    $type = u\consta('tx', $type);
    if ($amount and $to > 0) $txs[] = txSave(txRewardSetup($serial, $type, $to, $state, $amount, $tid, $created));
  }
}

function txRewardSetup($serial, $type, $payee, $state, $amount, $tid, $created) {
  $payer = $payerAgent = userField('community', $payee);
  $payeeAgent = $payee;
  $taking = FALSE;
  $goods = FALSE;
  $for = ($type == TX_REBATE ? t('rebate') : t('bonus')) . t( ' on #') . $tid;

  return transactionSetup(compact(u\ray('serial type amount goods payer payee payerAgent payeeAgent for taking state created'))); // created is here mostly for testing
}

/**
 * Create a new transaction and set up all the important fields.
 * @param assoc $info: transaction info.
 *   amount: (required) how much to transfer
 *   payer, payee: (required) uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   etc.
 * @return the array, with some extra field information added
 * Give full rewards for contributions to CGF, whether in r or USD
 */
function transactionSetup($info) {
  global $channel, $boxUser; // through what medium was the transaction requested (web, SMS, etc.), what machine
  $fields = 'type state data taking goods amount payer payee payerAgent payeeAgent created';
  extract(u\just($fields . ' for', $info));

  $specs = " -- payer=$payer payee=$payee";
  u\EXPECT(compact(u\ray('amount payer payee')), 'float zid zid');
  u\EXPECT($payer and $payee and $payer != $payee, 'null or self trading' . $specs);
  list ($a1, $a2) = array(r\acct($payer), r\acct($payee));
  u\EXPECT($a1->ok() and $a2->ok(), 'non-member in tx' . $specs);
  
  u\setDft($type, TX_TRANSFER);
  u\setDft($state, TX_DONE);
  u\setDft($taking, FALSE);
  u\setDft($goods, FALSE);
  u\setDft($for, R_DEFAULT_PURPOSE);
  u\setDft($payerAgent, $payer);
  u\setDft($payeeAgent, $payee);
  u\setDft($created, REQUEST_TIME);
//  $rewardBasis = ($payee == \variable_get('cgf_uid') and $for == 'contribution') ? $amount : $r;
  $data = dataSetup($payer, $payee, $amount, $goods, @$data ?: array());
  if (@$state !== TX_PENDING) $completed = $created;
  $taking += 0;
  $goods += 0;
  $box = @$boxUser ?: 0;

  list ($payerFor, $payeeFor) = is_array($for) ? $for : array($for, $for);
  unset($info['for']);

  return compact(u\ray($fields . ' completed payerFor payeeFor channel box')) + $info;
}

/**
 * Set up and return the data field for a transaction.
 * @param assoc $data: forced values, if any
 */
function dataSetup($payer, $payee, $amount, $goods, $data = array()) {
  u\EXPECT(is_array($data), 'data should be array');
  if ($goods and !isset($data['rebate'])) {
    $rebate = (string) ($goods ? r\rebate($payer, $payee, $amount) : 0); // force string, to avoid long serializations
    $bonus = (string) ($goods ? r\bonus($payee, $payer, $amount) : 0);
    $data += compact('rebate', 'bonus');
  }
  unset($data['preapproved']); // don't need these (but keep 'undoes' and original rebate/bonus, if any)
  unset($data['bilateral']);
  return serialize($data);
}

function cacheBothTotals($txs) {foreach(u\ray('payer payee') as $one) cacheTotals($txs[0][$one], $txs);}

/**
 * Keep track of r balance, rewards, committed amount in the account record
 * None of these figures is authoritative except 'committed'. 
 * The r balance and rewards are (definitively) the sum of the relevant fields in transactions.
 * The USD balance is (definitively) what is actually in the Dwolla account.
 */
function cacheTotals($uid, $txs) {
  extract(u\just('amount state type', $txs[0]), EXTR_PREFIX_ALL, 'tx0');
  if ($tx0_state < TX_DONE) return; // don't change totals if transaction doesn't count (yet)
  if ($uid < 0) return; // cache only usd for community and region

  $a = r\acct($uid);
  $cache = 'r rewards committed';
  extract((array) $a->account($cache));
  
  foreach ($txs as $tx) {
    extract(u\just('payer payee amount type goods', $tx));
    if ($toOther = ($payer == $uid)) {
      $amount = -$amount;
    } elseif ($payee != $uid) continue; // neither to nor from me
    $r += $amount;
    if ($type != TX_TRANSFER and $type <= TX_REWARDS) {
      if ($type == TX_REBATE or $type == TX_BONUS) $committed = round($committed + $amount * $a->share / 100, 2);
      if ($type != TX_REFUND) $rewards = round($rewards + $amount, 2);
    }
  }
  $r = round($r, 2);
  $a->update(compact(u\ray($cache)));
  if ($type == TX_TRANSFER and $amount > 0) r\membershipEvent($a, 'bona', TRUE);
}

/**
 * Return the shortfall for the requested transaction.
 * @param float $request: how much is requested
 * @param acct $a: the payer's account
 * @param bool $preapproved: is the transaction preapproved (otherwise it's an invoice)
 * @param bool $confirmed: has the user confirmed his/her intent
 * @param bool $goods: is the transaction for real goods and services
 *
 * @return float: how much too little the payer has
 */
function shortfall($request, $a, $preapproved, $goods, $confirmed) {
  $shortfall = 0; // no shortfall if not preapproved
  if (!$preapproved) return 0; // if just an invoice, funds don't have to be there yet (no shortfall, no usd)
  return $a->shortfall($goods ? 'buy' : 'cash', $request, $confirmed);
}

/**
 * Return a link to a "no signin required" action.
 * @see also r\Web\rdo()
 */
function makeDo($op, $uid, $amount = 0, $timeLimit = WEEK_SECS, $subs = '', $sql = '') {
///print_r(compact(u\ray('op uid amount timeLimit subs sql'))); die('makedo'); 
  global $base_url, $testDoCode;
  $action = serialize(compact(u\ray('op uid sql subs amount')));
  list ($created, $completed) = array(REQUEST_TIME, -(REQUEST_TIME + $timeLimit));
  $code = (t\est() and @$testDoCode) ? $testDoCode : \user_password(20);
  $id = db\insert('r_do', compact(u\ray('uid created completed action code')));
  return "$base_url/do/id=$id&code=$code";
}

/**
 * Wrap all email, so it doesn't go out when testing.
 */
function rMail($index, $email, $info, $lingo = '') {
  $info['email'] = $email;
  if (!isPRODUCTION) if ($email != R_REGION_EMAIL and $email != R_TESTER_EMAIL) $email = 'none@example.com'; // black hole
  \drupal_mail('rCredits', $index, $email, '', $info);
}

/**
 * Email an arbitrary message to the member, now or later.
 * @param int $uid: account record ID
 * @param string $index: index to arbitrary HTML message body (between salutation and closing)
 * @param assoc $subs: message parameters
 * @param bool $now: TRUE to send the email now (otherwise later)
 */
function message($uid, $index, $subs = array(), $now = TRUE) {
  $message = u\tt($index, $subs);
  $topic = $GLOBALS['topics'][$index];
  r\notify($uid, 'message', compact('message', 'topic'), $now);
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $info: substitutions for the message
 * @param bool $force: send by email NOW
 */
function notify($uid, $index, $info = array(), $force = FALSE) {
  global $channel;
  $acct = acct($uid);
  u\EXPECT($acct, 'no account');
  $info['uid'] = $uid;
  $info['otherName'] = @$info['myName']; // notifying someone else about US, if anyone
  $fullName = $acct->fullName;
  $warnIndexes = u\ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $acct->mail, $info); // no need to pass $acct->language
  } else { //if ($acct->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(u\tt($index, $info), REQUEST_TIME);
    if (t\est()) t\output("Notice to $fullName: $message");
    db\insert('r_notices', compact(u\ray('uid message created')));
  }
  
  if ($acct->can(B_NOTIFY_SMS)) {
    if ($num = $acct->smsNumber) $smsed = SMS\send($num, u\tt($index, $info, TX_SMS));
  }
  if (FALSE and !@$mailed and !@$smsed and array_search($index, $warnIndexes) !== FALSE) {
    $notification = str_replace('-', ' ', $index);
    $warning = u\tt('cannot contact', compact('fullName', 'notification'));
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) SMS\send(r\acct()->smsNumber, $warning);
    if ($channel == TX_SMART) { } // dunno how to handle this
  }
}

/**
 * Create an rCard security code for the given account (either for an individual or a company agent).
 * @param acct $a: the account
 */
function makeCardCode($a) {
  $secure = u\ray($a->proSe() ? 'cardCode' : 'cardCode2', r\cardCode($a->qid())) + $a->agent->secure;
  $a->agent->update(compact('secure')); // resave the secure fields
}

/**
 * If appropriate, update an account's membership status and send the staff an alert.
 * @param acct $a: the account
 * @param string $status: what milestone or event to alert about
 * @param bool $do: prerequisite for the alert
 * If there is an appropriate bit to set, it will be set as appropriate.
 */
function membershipEvent($a, $status, $do = TRUE) {
  $bit = @u\consta('b', $status); // get the appropriate bit, if any
  if ($do and ($bit === FALSE or !$a->hasBit($bit))) { // met the prerequisite and hasn't previously
    $DBTX = \db_transaction();
    if ($bit !== FALSE) $a->setBit($bit); // set the bit, if any
    if ($status == 'bona' and $a->person()) {
      be\fund($a->id, TX_SIGNUP, R_SIGNUP_BONUS, t('signup bonus'));
      r\notify($a->id, 'account funded', array(), TRUE);
    }
    if ($status == 'ok') {
      u\EXPECT($a->can(B_CTTY_ADMIN), 'should be ctty admin'); // only ctty admin can approve an account
      r\notify($a->id, 'welcome', array(), TRUE);
    }
    unset($DBTX);
    
    $fullName = $a->fullName;
    $quid = $a->mainQid;
    tellStaff('event', compact(u\ray('fullName quid status')), $a->id);
	  return TRUE;
  } else return FALSE;
}

/**
 * Try to transfer funds from contributor to CGF.
 * @param assoc $info: the gift record data
 * @return TRUE if the funds were successfully transferred.
 */
function acceptGift($info) {
  extract(u\just('donid uid giftDate amount often honor', $info));
  $a = r\acct($uid);
  if ($a->shortfall('buy', $amount)) return FALSE; // wait until tomorrow, if not enough
  if ($a->shortfall('buy', $amount, TRUE)) return FALSE; // actually draw from other accounts, as needed

  $cgf = r\acct(\variable_get('cgf_uid'));

//  r\usd::beginAtom();
  $purpose = @$honor == 'share' ? t('sharing rewards with CGF') : t('contribution');
  list ($msg, $args, $confirm) = be\transfer('payment', $a, $cgf, $amount, $purpose, TRUE);
///  debug(compact('msg','args','confirm'));
  if (@$args['success']) {
    $completed = REQUEST_TIME;
    db\update('r_gifts', compact('donid', 'completed'), 'donid');
    if ($often != 1) {
      $months = $often == 'M' ? 1 : ($often == 'Q' ? 3 : 12);
      $giftDate = strtotime($months . ' months', $giftDate);
      $share = -1; // mark this field N/A (irrelevant)
      db\insert('r_gifts', compact(u\ray('uid giftDate amount often share')));
    }
//    r\usd::commit();
    r\tellStaff('gift accepted', $info + $args);
    r\notify($uid, 'gift sent', $args);
    return TRUE;
  } //else r\usd::rollback();
  return FALSE;
}

/**
 * Notify rCredits staff
 * @param string $index: the message or message index
 * @param assoc $subs: substitutions for the message OR (if no subs in message) data to show
 * @param bool/int $uid: related account record id (defaults to current account), none if FALSE
 */
function tellStaff($index, $subs = array(), $uid = TRUE) {
  if (empty($subs)) $subs = array(); // accept ''
  unset($subs['secure']); unset($subs['pass']); // nobody should see these
  ksort($subs); // put the keys in a predictable order, for testing and easy reading
  foreach (u\ray('federalId dob answer') as $one) unset($subs[$one]); // secret stuff
  $message = strpos($raw = u\tt($index), '@') === FALSE ? ($raw . u\seeAssoc($subs)) : u\tt($index, $subs);

  if ($uid) {
    if ($mya = r\acct($uid > 1 ? $uid : '')) $message = "$mya->fullName (#$mya->id): $message"; else $uid = 1;
  } else $uid = 1; // default to sysadmin

  $noFrame = TRUE;
  $topic = strlen($index) < 20 ? strip_tags($index) : '';
  $email = R_REGION_EMAIL;
  r\rMail('tell-staff', $email, compact(u\ray('email message topic noFrame uid')));
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
function tempName($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = @$my_number ?: (@$my_phone ?: '');
  if ($phone) return R_TEMP_NAME_PREFIX . u\fmtPhone($phone, 'n');
  if (isset($my_email)) return R_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return R_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return R_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

/**
 * Set a transaction field for the current user.
 * @param string $fieldName: the field to update
 * @param mixed $newValue: what to update it to
 * @param int $xid: transaction id
 * @param bool $allRelated: set all related transactions too (default FALSE)
 * NOTE: tx data changes(date, agent, field, oldvalue) (this gives a complete history of the transaction)
 *     (prefixes added to the payerFor or payeeFor fields when changing state will not be included in changes)
 */
function setTxField($fieldName, $newValue, $xid, $allRelated = FALSE) {
  $myid = acct()->id;
  u\EXPECT(compact(u\ray('fieldName xid allRelated')), 'name zid bool');
  
  $where = "xid=:xid and $myid in (payer, payee)"; // "$myid in" is a security measure
  list ($oldValue, $oldState, $data) = array_values(db\lookup("$fieldName AS f1,state,data", 'r_txs', $where, compact(u\ray('xid')))); // f1 prevent Drupal from combining state results when $fieldName is 'state'
  $data = unserialize($data);
  $data['changes'][] = array(REQUEST_TIME, acct()->agentId, $fieldName, $oldValue);
  $data = serialize($data);
  $newState = $fieldName == 'state' ? $newValue : $oldState;
  $completion = (@$newState != TX_PENDING and !@$oldState) ? ',completed=:REQUEST_TIME' : ''; // update completion time
  $sql = "UPDATE r_txs SET $fieldName=:newValue,data=:data$completion WHERE $where";
  $sql2 = "UPDATE r_txs SET $fieldName=:newValue$completion WHERE serial=:xid AND type<>:TX_TRANSFER";
//  $sql2 = str_replace(', data=:data', '', str_replace('xid=:xid', 'serial=:xid', $sql));

  $DBTX = \db_transaction();
  db\q($sql, compact(u\ray('newValue data xid')));
  if ($allRelated) db\q($sql2, compact(u\ray('newValue xid'))); // gotta do the auxiliary records separately so as not to affect their data field (separate query, to avoid "unbuffered query" error -- known Drupal bug)
  unset($DBTX); // commit
}

/**
 * Merge a temporary ("old") account into the current account (a permanent ("new") account).
 * @return 1 (for counting how many were merged)
 */ /*
function mergeAccounts($oldUid) {
  $newUid = acct()->id;
  if (!be\isTempAccount($oldUid)) return; // don't merge non-temp account
//  u\EXPECT(be\access('manage account'), "unauthorized mergeAccounts: $newUid into $oldUid");
///  if (!isTempName(userField('name', $oldUid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = acct($newUid);
  $old = acct($oldUid);
  $update = array();
  foreach (u\ray('phone faxetc website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  acct()->update($update);
  newMemberId($oldUid, $newUid, TRUE);
  db\q('DELETE FROM users WHERE uid=:oldUid', compact('oldUid')); // must be last, for referential integrity
  return 1;
//  db\q("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:oldUid", compact('oldUid'));
}
*/

/**
 * Return the rough (physical) location of the entity with the given account (city, state, non-US country).
 * @param acct $acct: the account
 * @param bool $complete: if TRUE, include the street address
 */
function location($acct, $complete = FALSE) {
  $country = realCountry($acct->country);
  $state = realState($acct->state, $acct->country);
  $result = array();
  if ($complete) $result[] = (string) $acct->address; // (string) in case address is NULL
  $result[] = $acct->city;
  $result[] = $state . (($complete and $acct->postalCode) ? ' ' . $acct->postalCode : '');
  if ($acct->country != R_COUNTRY_ID) $result[] = strtoupper($country);
  foreach ($result as $key => $value) if ($value === '') unset($result[$key]);
  return join(', ', $result);
}

/**
 * Return a qid for the zid
 * @param $zid: a zid
 * @param string $regionId: the 3-character regional server ID (defaults to this server)
 * @param int $min_len: minimum length of the part of the qid after the R_MEMBER_MARK or R_AGENT_MARK
 * @return the qid, FALSE if wrong format
 */
function qid($zid = '', $regionId = R_SERVER_ID, $min_len = 1) {
  if (!u\isZid($zid)) return FALSE;
  return $regionId . ($zid < 0 ? (R_AGENT_MARK . u\n2a(-$zid, -$min_len)) : (R_MEMBER_MARK . u\n2a($zid, -$min_len)));
}

/**
 * Return the short qid and full name of the specified account.
 * For example, return ".ZZA Abe One".
 */
function definiteName($uid) {
  if ($uid <= 0) return '';
  $acct = acct($uid);
  return r\quid($uid) . ' ' . $acct->fullName;
}

/**
 * Return a zid and region for the qid
 * @param string $qid: a qid of any length or type (the region part defaults to the server region)
 * @return [zid, regionUid]
 */
function unQid($qid) {
  if (!u\isQid($qid)) {
    $qid = R_SERVER_ID . $qid; // if local, complete it
    if (!u\isQid($qid)) return FALSE;
  }
  list ($regionId, $type, $iid) = preg_split('/(' . R_MARKS . ')/U', $qid, 0, PREG_SPLIT_DELIM_CAPTURE);
  $id = u\a2n($iid);
  return array($type == R_MEMBER_MARK ? $id : -$id, serverUid($regionId));
}
  
/**
 * Convert uid to quid
 * Call by:
 *   quid(uid)
 *   quid(uid, agentId)
 * @param int uid: record id of the account
 * @param int agentId: uid of agent (defaults to uid)
 * @return the corresponding quid
 */
function quid($uid, $agentId = '') {
  if (!$agentId or $agentId == $uid) { // proSe
    if ($uid < 0) return u\n2a(-$uid - 1, -3); // -1 because region and ctty uids are bumped down by 1
    $region = -uidRegion($uid) - 1;
    $i = ($uid - 1) - $region * R_REGION_MAX; // -1 because uids are all bumped up by 1
    return u\n2a($region, -3) . R_MEMBER_MARK . u\n2a($i, -3);
  } else {
    $uid = relation('reid', $uid, $agentId); // compound quid
    return R_SERVER_ID . R_AGENT_MARK . u\n2a($uid, -3);
  }
}

/**
 * Return the uid for a pro se quid (local or complete).
 */
function unQuid($quid, $zid = NULL, $region = NULL) {
  if (is_null($zid)) list ($zid, $region) = r\unQid($quid); // break it down
  return $zid < 0 ? FALSE : (1 + R_REGION_MAX * (-$region - 1) + $zid);
}

/**
 * Return a "local version" pro se quid without the region but with a leading dot.
 */
function quid2($uid) {return strstr(quid($uid), R_MEMBER_MARK);}

function qxid($xid, $regionId = R_SERVER_ID) {return qid($xid, $regionId = R_SERVER_ID, 4);} // Convert xid to qxid

/**
 * Return an rCard security code for the specified qid.
 */
function cardCode($qid) { 
  $tail = substr(strpbrk($qid, R_MARKS), 1);
  $codeLen = R_CARDCODE_LEN - (strlen($tail) - 3); // shorter code when qid tail len > 3
  return u\randomString($codeLen, 'word'); // create an rCard security code
}

/**
 * Set a cookie
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now)
 * @return: the value
 */
function setCook($key, $value, $expires = NULL) {
  u\setDft($expires, REQUEST_TIME + 365250 * DAY_SECS);
  setcookie($key, $value, $expires, '/'); // make it easy (fails without '/')
  if (t\est()) $_COOKIE[$key] = $value;
  return $value;
}

/**
 * Return the rCredits region id for the given account id
 */
function uidRegion($uid) {
  u\EXPECT($uid > 0, 'non-positive uid in uidRegion');
  return -floor(($uid - 1) / R_REGION_MAX) - 1; // -1 because uids are all bumped up by 1
}

/**
 * Return the rCredits region id for the given geographical location
 */
function region($country = '', $state = '', $zip = '') {
  $where = "s.country_id=:country AND s.id=:state AND (r.zips='' OR :zip RLIKE r.zips)";
  $region = db\lookup('region', 'r_regions r INNER JOIN r_states s ON r.state=s.abbreviation', $where, compact('state', 'zip', 'country'));
  if (!$region) $region = db\lookup("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  return -u\a2n($region ?: 'ZZZ') - 1;
}

/**
 * Determine whether the record can be undone
 * @param assoc $oldRecord: the transaction record+ from be\lastTx or NULL (to make it easy when last_tx found no matching transaction)
 * @param assoc $subs: (RETURNED) necessary substitutions, if any
 * @return string: the error message (FALSE if the current agent CAN undo it on behalf of the current account)
 */
function nonoUndo($oldRecord, &$subs = array()) {
  if (!@$oldRecord) return 'undo no match';
  $mya = acct();
  extract(u\just('amount toMe data payer payee payerAgent payeeAgent payerTid payeeTid taking', $oldRecord));
  $data = unserialize($data);
  if (@$data['undoneBy']) {
    $oldTid = $toMe ? $payeeTid : $payerTid;
    $subs = compact('oldTid');
    return 'already undone';
  }

  $acct = $toMe ? r\acct($payee, $payeeAgent) : r\acct($payer, $payerAgent);
  if ($mya->agentId != $acct->agentId) {
    $aacct = acct($acct->id, $mya->agentId);
    if ($taking xor $amount < 0) { // original transaction was a charge
      if (!$aacct->can(B_BUY)) return 'no buy';
    } elseif (!$aacct->can(B_SELL)) return 'no sell';
  }
  return FALSE;
}

/**
 * Create or update a reverse relation record for the given main and other
 *   if any of the given fields is positive
 * @param int $other: the main (switching to agent)
 * @param int $main: the other (switching to main)
 * @param assoc $data: (byRef) array of field values
 *   Returned with all fields listed in $fields unset (if any of those fields has a positive value)
 * @param string $fields: space-delimited list of fields to check
 * @return (none)
 */
function reverseRelations($other, $main, &$data, $fields) {
  foreach (u\ray($fields) as $field) if (@$data[$field]) $stay = TRUE;
  if (!@$stay and !isset($data['employeeOk'])) return; // this function also UNsets employeeOk
  
  $subs = compact('main', 'other') + u\just($fields, $data);
  if ($reid = relation('reid', $main, $other)) {
    $subs += compact('reid');
    db\update('r_relations', $subs, 'reid');
  } else db\insert('r_relations', $subs);

  foreach (u\ray($fields) as $field) unset($data[$field]); // don't get these fields mixed in with the reverse record
}

/**
 * Say whether one of the accounts owns the other (or can draw from it).
 */
function owning($uid1, $uid2) {
  $where = '(isOwner or draw) AND ((main=:uid1 AND other=:uid2) OR (main=:uid2 AND other=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second
 * @param int $company: defaults to current uid
 */
function isEmployee($uid, $company = '') {
  if (!$company) $company = acct()->id;
  return relation(1, $company, $uid, 'employerOk AND employeeOk');
}

function communityUid($postalCode = '') {
  if (!$postalCode) return serverUid();
  $where = "country=':R_COUNTRY_ID' AND :postalCode RLIKE postalCode AND uid=community";
  return userField('uid', $where, compact(u\ray('postalCode'))) ?: serverUid();
}

/**
 * Say whether the given account id is a foreign region.
 */
function foreignServer($uid) {
  if ($uid == serverUid()) return FALSE;
  return ($uid < 0 and preg_match('/[A-Z]{3}\./', userField('name', $uid)));
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName) {
  $name = $name0 = u\shortName($fullName);
  for ($i = 2; ; $i++) {
    if (!userField(1, 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name or email.
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if (u\isQid($id = trim(strtoupper($id))) and $acct = acct($id)) return $acct->id; // try to interpret as a qid
  return userField('uid', 'name=:id OR mail=:id', compact('id'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM r_relations WHERE main=:myid AND employerOk', compact('myid'))->fetchCol();
}

/**
 * Return the field or fields from the relations table, for the given main and other.
 * Call by:
 *   relation(field, main, other, where, subs) OR
 *   relation(field, where, subs)
 *   relation(field, reid)
 * @param string $field: what field value(s) to return (see varieties at db\lookup())
 * @param int $main: uid of main account
 * @param int $other: uid of other (agent) account
 * @param string $where: SQL criteria
 * @param assoc $subs: criteria parameters
 * @param int $reid: relations record id
 */
function relation($field, $main, $other = array(), $where = 'TRUE', $subs = array()) {
  if (is_numeric($main)) {
    if (empty($other)) {
      $reid = $main;
      $where = "reid=:reid";
    } else {
      $where = "main=:main AND other=:other AND ($where)";
      $subs += compact('main', 'other');
    }
  } else list ($where, $subs) = array($main, $other); // called with alternate form
  return db\lookup($field, 'r_relations', $where, $subs);
}

/**
 * Return our standard encryption password and a salt for the given account.
 */
function passSalt($acct = '') {
  $acct = $acct ?: r\acct();
  $pass = R_SALTY_PASSWORD;
  $salt = $acct->created;
  return compact('pass', 'salt');
}

/**
 * Return the first unused uid for the given base.
 * @param int $base: lowest uid to use (first slot in a region)
 * @return: the uid
 */
function newUid($base) {return u\firstUnusedId('uid', 'users', $base, $base + R_REGION_MAX);}

/*  $where = 'u.uid>=:base and u.uid <:base+:R_REGION_MAX AND u2.uid IS NULL ORDER BY u.uid';
  $previous = db\lookup('u.uid', 'users u LEFT JOIN users u2 ON u2.uid=(u.uid+1)', $where, compact('base'));
  return $previous ? ($previous + 1) : $base;
}*/

/**
 * Record the relation of a device with an account.
 * @param int $uid: record ID of related account
 * @param string $code: phone # or device code
 * @param int $ch: channel (SMS, Web, or App)
 * @return int: the device record ID
 */
function makeDevice($uid, $code = '', $ch = NULL) {
  global $channel;
  $mya = r\acct();
  
  u\setDft($ch, $channel);
  if ($ch == TX_WEB and $mya and $mya->admin()) return 0; // don't create spurious devices as admin
/*  if ($boxName === '0') {
    $where = "t1.boxName REGEXP '^[0-9]+$' AND t1.uid=:uid";
    $boxName = u\firstUnusedId('boxName', 'r_boxes', 1, PHP_INT_MAX, $where, compact('uid'));
  } else */
  //if (!$boxName) $boxName = $code; // code serves as boxName for dumb cell phones
  $created = $access = REQUEST_TIME;
  $boxnum = db\lookup('MAX(boxnum)', 'r_boxes', 'uid=:uid', compact('uid')) + 1;
  $info = u\ray('channel', $ch) + compact(u\ray('uid code boxnum access created'));
  return @db\insert('r_boxes', $info);
}

/**
 * Assign the given account a new id.
 * @return the new uid
 */
function changeUid($oldUid, $newUid = FALSE, $unusable = TRUE) {
  if (!$newUid) $newUid = newUid($base);
  $subs = array( // all tables are listed, to be sure we don't leave any out
  // r_areas
    'r_boxes:uid',
  // r_countries,
    'r_gifts:uid',
  // r_industries
    'r_invites:inviter',
    'r_invites:invitee',
    'r_log:myid',
    'r_log:agent',
  // r_nonces
  // r_nonmembers
    'r_notices:uid',
    'r_proxies:person',
    'r_proxies:proxy',
  // r_regions
    'r_relations:main',
    'r_relations:other',
  // r_states
    'r_txs:payer',
    'r_txs:payee',
    'r_usd:payer',
    'r_usd:payee',
  // r_usderror
    'r_user_industries:uid',
    'users:uid',
  );
  foreach (u\ray(DRUPAL_UID_TABLES) as $one) $subs[] = "$one:uid";

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field) = explode(':', $one);
    db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }
  $oldQuid = r\quid($oldUid);
  list ($uid, $name, $mail) = array($oldUid, "unusable-$oldQuid", "unusable$oldQuid@rcredits.org");
  if ($unusable) db\insert('users', compact(u\ray('uid name mail'))); // mark old uid unusable
  foreach (u\ray('picture proof') as $one) { // this cannot be rolled back, so must follow db ops
    $new = DRUPAL_ROOT . r\acct($newUid)->auxFilename($one, TRUE);
    $old = str_replace("/$newUid-", "/$oldUid-", $new);
    if (file_exists($old)) rename($old, $new);
  }
  unset($DBTX); // commit
  return $newUid;
}

/**
 * Check if an account needs a signup bonus.
 * @param int $uid: account id
 * @return: whether the signup bonus is deserved
 *//*
function checkBona($uid) {
  $acct = r\acct($uid);
  if ($acct->can(B_BONA)) return FALSE;
  if (!db\exists('r_txs', 'payee=:uid and r>0 and payer>0 and state=:TX_DONE', compact('uid'))) return FALSE;
  if (!db\lookup('COUNT(DISTINCT payee)', 'r_txs', 'payer=:uid and goods and state=:TX_DONE', compact('uid')) >= R_BUYS_BEFORE_BONUS) return FALSE;
  return TRUE;
}*/

/**
 * Record an invitation
 * @param string $email: email address to which the invite was sent
 * @param int $inviter: the inviter's account uid
 * @param string $code: proof of the invitation (that invitee must have, in order to sign up) (DEFAULT random)
 *
 * @param string $subject, $message: temporary params during MINI_LAUNCH
 *
 * @return string: the code
 */
function invite($email, $inviter, $code = '', $subject = '', $message = '') {
  $code = $code ?: \user_password(20);
  $invited = REQUEST_TIME;
  $info = compact(u\ray('code invited email inviter subject message'));
  db\insert('r_invites', $info);
  return $info['code'];
}

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out.
 * @param string $page: page to go to
 * @param string $msg: message to output, if any
 * @param string $field: error field, if any
 */
function go($page, $msg = '', $field = '') {
  if (t\est()) {
    global $lastGo; $lastGo = $page; // for testing
    if (strpos($page, 'http://') !== FALSE) $page = '';
    return  t\pageForm($page, $msg, $field);
  }
  
  foreach (u\ray('login signup password reset') as $one) if (strpos($page, "user/$one") !== FALSE) {
    if (\drupal_session_started()) session_destroy();
  }
  if ($msg) r\Web\say($msg, $field);
  \drupal_goto(substr($page, 0, 1) == '/' ? substr($page, 1) : $page);
}

/**
 * Set or return the proxy for the person
 */
function proxy($person, $priority = 1, $proxy = NULL) {
  if (!@$proxy) return db\lookup('proxy', 'r_proxies', 'person=:person AND priority=:priority', compact('person', 'priority'));
  db\q('DELETE FROM r_proxies WHERE person=:person AND priority=:priority', compact('person', 'priority'));
  $info = compact(u\ray('person priority proxy'));
  db\insert('r_proxies', $info);
}

/**
 * Trade some rCredits for USD, from somebody. 
 * @param float $request: how much to try to get
 * @param acct $acct: rCredits account that is trading rCredits to third parties
 * @return how much USD we actually delivered
 */
function getUsd($request, $acct) {
  $need = $request;
  r\usd::beginAtom();
  while ($need > 0) {
    $part = $need;
    if (!$acct3 = nextRBuyer($part, $acct->id)) break; // asks for what we need, returns the part we got in $part
    u\EXPECT($part > 0, 'got nothing');
    if ($txid = r\tradeR($part, $acct->id, $acct3->id)) $need = round($need - $part, 2);
  }
  $got = round($request - $need, 2);
  r\usd::commit($got); // accept partial success
  return $got;
}

/**
 * Return the uid of the next account in line wanting/willing and able to trade enough US Dollars for rCredits.
 * If you trade rCredits for USD or vice versa, you go to the end of the line.
 * @param float $amount: the amount we're looking for. Returned with the amount actually available from the returned account. The amount sought depends on the $amount passed as follows:
 *   $amount < R_CHUNK*1.5 -- seek $amount
 *   otherwise -- seek R_CHUNK (leaving R_CHUNK/2 or more for other buyers
 * return: the best account to handle the trade (there may be none that wants so many rCredits)
 *    if no account wants to buy any, return FALSE (this would be really bad and should never ever happen)
 *    NOTE that CGF is the buyer of last resort.    
 */
function nextRBuyer(&$amount, $except) {
  if ($amount > R_CHUNK) $amount = R_CHUNK; // keep it cheap
  $sql = <<<EOF
    SELECT u.uid, u.usd AS got, u.name,
    (SELECT MAX(completed) FROM r_usd WHERE payer=u.uid and payee<>0) AS last
    FROM users u
    WHERE u.usd>0 AND u.uid<>:except AND :IS_OK AND :IS_DW
    ORDER BY last DESC, u.usd DESC, u.uid LIMIT 1
EOF;

  if (!$row = db\q($sql, compact('except'))->fetchAssoc()) { // find out who (preferably who who wants enough) has waited the longest
    r\tellStaff('no buyers', compact('amount')); // this is a really bad thing (nobody wants any at all)
    return FALSE;
  }
  extract($row); // uid, got, and name
  if ($name =='cgf') r\tellStaff('cgf sole buyer', compact('amount'));
  $amount = min($amount, round($got, 2)); // take what we could get
  return r\acct($uid);
}

/**
 * Return a system-wide or community-wide total.
 * As of the last daily cron, unless testing. For each community, and overall, the totals include:
 *    demand: the total unmet demand for rCredits
 *    r: rCredits in the system
 *    usd: USD in play (approximate)
 *    signup: rCredits issued as signup reward
 *    inflation: rCredits issued to offset inflation
 *    rebates: rCredits issued as rebates (including on USD transactions)
 *    bonuses: rCredits issued as sales bonuses
 * @param int $ctty: the community account id for which to return totals (default ALL)
 */
function totals($ctty = 'ALL', $calculate = FALSE) {
  $totals = unserialize(\variable_get('r_totals'));
  if ($calculate) {
    $totals[$ctty] = getTotals($ctty == 'ALL' ? NULL : $ctty);
    \variable_set('r_totals', serialize($totals));
  }
  foreach ($totals[$ctty] as $key => $value) $fancy[$key] = u\fmtAmt(@$value + 0);
  $demand = $totals[$ctty]['demand'];
  if ($demand >= 1000000) $fancy['demand'] = '$' . number_format(($demand - 9999) / 1000000, 2) . ' million';
  return (object) ($totals[$ctty] + $fancy);

  /*
  $result = db\lookup('SUM(minimum)', 'users', $ctty ? 'community=:ctty' : '1', compact('ctty')); // minima
  $result -= -r\balance($ctty); // less how many rCredits are in circulation
  $result -= r\totalUSD($ctty); // less how much official currency is in play
  */
}

/**
 * Return totals for the given community
 * @param int $ctty: the community account id (NULL means all communities)
 * @return assoc (ctty or system-wide totals of):
 *   r floor rewards usd minimum excess
 *   signup rebate bonus inflation grant loan fine maxRebate
 *   balance (-r) demand (minimum - r)
 */
function getTotals($ctty) {
  $subs = @$ctty ? compact('ctty') : array();
  $where = (@$ctty ? "community=:ctty" : 1) . ' AND uid>0 AND :IS_OK';
  $fields = u\sumAs('r floor rewards usd minimum committed');
  //$max = 'GREATEST(0, maximum)';
  $sql = "SELECT $fields, SUM(:R_DEMAND) AS demand, SUM(:R_CAPACITY) AS capacity FROM users u WHERE $where";
  $result1 = db\q($sql, $subs)->fetchAssoc();

  $where = @$ctty ? '(payer=:ctty OR payee=:ctty)' : 1;
  $sums = '';
  foreach (u\ray('signup rebate bonus inflation grant loan fine') as $one) {
    $ONE = strtoupper($one);
    $sums .= "SUM(IF(type=:TX_$ONE, amount, 0)) AS `$one`, ";
    $sums .= "SUM(IF(type=:TX_$ONE, 1, 0)) AS {$one}Count, ";
  }
  $sql = "SELECT $sums MAX(IF(type=:TX_REBATE, amount, 0)) AS maxRebate FROM r_txs WHERE $where AND state IN(':TX_DONE,:TX_DISPUTED')";
  $result2 = db\q($sql, $subs)->fetchAssoc();
  $balance = -$result1['r'];
  return $result1 + $result2 + compact(u\ray('balance'));
}

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region
 * @param int $when: latest desired statistics (defaults to now)
 *   if $when < 0, calculate the latest
 * @return assoc of named statistics (see r_stats table def)
 */
function stats($ctty = 'ALL', $when = REQUEST_TIME) {
  if ($when < 0) $when = getStats($ctty);

  $sql = 'SELECT * FROM r_stats WHERE created<=:when AND ctty=:ctty ORDER BY id DESC LIMIT 1';
  return db\q($sql, compact('ctty', 'when'))->fetchAssoc();
//  $info['grant'] = $info['grants']; unset($info['grants']); // accommodate drupal bug (see table def)
//  return $info;
}
  
/**
 * Calculate and record statistics for the given community.
 * See the "// get" lines to see what statistics are collected and returned.
 * @param int $ctty: community or region
 * @param int $created: creation date for the statistics record (defaults to current time)
 * @return $created
 * Note that rCredits traded by the community for USD are not counted as issued
 * (they aren't really issued until the USD are spent as a grant/loan/investment/purchase)
 */
function getStats($ctty, $created = NULL) {
  \variable_set('cttyPaidEver', db\lookup('SUM(amount)', 'r_txs', 'payer=:ctty AND type IN (:TX_TRANSFER, :TX_GRANT, :TX_LOAN, :TX_INVEST, :TX_REFUND)', compact('ctty')));
  u\setDft($created, t\est() ? REQUEST_TIME : time());
  $subs = compact('ctty', 'created');
  // get pAccts, bAccts
  $sql = <<<EOF
    SELECT SUM(IF(:IS_PERSON, 1, 0)) AS pAccts, SUM(IF(:IS_PERSON, 0, 1)) AS bAccts
    FROM users WHERE :IS_OK AND uid>1 AND community=:ctty AND created<=:created
EOF;
  $result1 = db\q($sql, $subs)->fetchAssoc() ?: array();
// (calculate instead, so getStats will work for old dates)    SUM(r) AS r, SUM(usd) AS usd
  
  $sums = '';
  foreach (u\ray('transfer signup rebate bonus inflation refund grant loan invest fine') as $one) {
    $type = u\consta('tx', $one);
    if ($type == TX_TRANSFER) $type .= ' AND payer=:ctty AND amount<>0';
    $value = $type == TX_TRANSFER ? 
        "IF(type=$type AND payer=:ctty, amount, 0)" // rCredits issued as ctty purchases
      : "IF(type=$type, amount, 0)";
    $sums .= "SUM($value) AS `{$one}s`, ";
//    $sums .= "SUM(IF(type=$type, 1, 0)) AS {$one}Count, ";
  }
  // get purchases, signups, rebates, bonuss, inflations, refunds, grants, loans, investments, fines, shared
  $sql = <<<EOF
    SELECT $sums SUM(IF(channel=:TX_CRON AND payeeFor='sharing rewards with CGF', amount, 0)) AS shared
    FROM r_txs t LEFT JOIN users u ON t.payer=u.uid LEFT JOIN users u2 ON t.payee=u.uid
    WHERE (u.community=:ctty OR u2.community=:ctty) AND t.state>=:TX_DONE AND completed<=:created
EOF;
  $result2 = db\q($sql, $subs)->fetchAssoc() ?: array();
  // get r
  $result2['r'] = array_sum($result2) - $result2['shared'] - 2 * $result2['fines'];

  $subs += u\ray('monthAgo', strtotime('-1 month', $created));
  // get perMo, txsPerMo, p2b(Txs)PerMo, b2b(Txs)PerMo, b2p(Txs)PerMo, cash(Txs)PerMo
  $sql = <<<EOF
    SELECT 
    SUM(IF(goods, amount, 0)) AS perMo,
    SUM(IF(goods AND u.:IS_PERSON AND NOT u2.:IS_PERSON, amount, 0)) AS p2bPerMo,
    SUM(IF(goods AND NOT u.:IS_PERSON AND NOT u2.:IS_PERSON, amount, 0)) AS b2bPerMo,
    SUM(IF(goods AND NOT u.:IS_PERSON AND u2.:IS_PERSON, amount, 0)) AS b2pPerMo,
    SUM(IF(amount<>0 AND NOT goods, amount, 0)) AS cashPerMo,

    SUM(IF(goods, 1, 0)) AS txsPerMo,
    SUM(IF(goods AND u.:IS_PERSON AND NOT u2.:IS_PERSON, 1, 0)) AS p2bTxsPerMo,
    SUM(IF(goods AND NOT u.:IS_PERSON AND NOT u2.:IS_PERSON, 1, 0)) AS b2bTxsPerMo,
    SUM(IF(goods AND NOT u.:IS_PERSON AND u2.:IS_PERSON, 1, 0)) AS b2pTxsPerMo,
    SUM(IF(amount<>0 AND NOT goods, 1, 0)) AS cashTxsPerMo

    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE (u.community=:ctty OR u2.community=:ctty) AND t.state>=:TX_DONE 
    AND completed>:monthAgo AND completed<=:created
EOF;
  $result3 = db\q($sql, $subs)->fetchAssoc() ?: array();

  // get bankIn(Txs)PerMo, bankOut(Txs)PerMo, xch(Txs)PerMo
  $sql = <<<EOF
    SELECT SUM(IF(amount<0, -amount, 0)) AS bankInPerMo,
    SUM(IF(amount<0, 0, amount)) AS bankOutPerMo,
    SUM(IF(payee>0, amount, 0)) AS xchPerMo,
    
    SUM(IF(amount<0, 1, 0)) AS bankInTxsPerMo,
    SUM(IF(amount<0, 0, 1)) AS bankOutTxsPerMo,
    SUM(IF(payee>0, 1, 0)) AS xchTxsPerMo

    FROM r_usd t LEFT JOIN users u ON t.payer=u.uid
    WHERE u.community=:ctty AND t.completed AND t.payee=0
    AND completed<=:created AND :monthAgo>0
EOF;
  $usdResult = db\q($sql, $subs)->fetchAssoc() ?: array(); // "AND :monthAgo>0" just to match sub count
  $result4 = db\q($sql . ' AND completed>:monthAgo', $subs)->fetchAssoc() ?: array();
  // get usd
  $result4['usd'] = $usdResult['bankInPerMo'] - $usdResult['bankOutPerMo'] - $result2['refunds']; // (actually ever, not perMo)

///debug(compact(u\ray('result1 result2 result3 result4')));
  $info = $result1 + $result2 + $result3 + $result4 + compact('created', 'ctty');
  foreach ($info as $key => $value) {if (is_null($value)) $info[$key] = 0;}
  db\insert('r_stats', $info);
  return $created;
}

/**
 * Return the average balance for the given account and given period.
 * @param int $uid: account id (can be a community or region too)
 * @param int $start: starting date (should be 0 hours) -- first full day to include in the period
 * @param int $end: ending date (should be 0 hours) -- last full day to include in the period
 * @param float $days: number of days from start to end, inclusive
 * @return float: the average balance for that period (possibly with fractional cents)
 */
function averageBalance($uid, $start, $end = REQUEST_TIME, &$days = '') {
  //u\EXPECT($uid > 1, 'bad uid for avg');
  $days = round(($end + 1 - $start) / DAY_SECS); // rounding is for daylight savings time changes
  $bal = be\creditInfo(u\ray('uid asof', $uid, $start))->balance;
  $day = 0;
  $avg = 0;
  $sql = <<<EOF
    SELECT SUM(IF(payee=:uid, amount, -amount)) AS total, FLOOR((completed-$start)/:DAY_SECS) AS day 
    FROM (
      (SELECT payer,payee,amount,completed FROM r_txs WHERE state=:TX_DONE)
      UNION (SELECT payer,payee,amount,completed FROM r_usd WHERE payee=0)
    ) x WHERE :uid IN (payer, payee) AND completed BETWEEN $start AND $end 
    GROUP BY day
EOF;

  $result = db\q($sql, compact('uid'));
  while($row = $result->fetchAssoc()) {
    $dayx = $day;
    extract($row); // total, day
    $avg += $bal * ($day - $dayx) / $days;
///debug(compact('uid','day','dayx','bal','avg','total'));
    $bal += $total;
  }
  $dayx = $day;
  $day = round(($end - $start) / DAY_SECS); // end of period
  $avg += $bal * ($day - $dayx) / $days;
///debug(compact('uid','day','dayx','bal','avg'));

  return $avg; // average end-of-day balance
}

/**
 * @param mixed $which: criteria or record id (defaults to $myid if users)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 */
function tableField($field, $table, $which = '', $subs = array()) {
  if ($table == 'users') {
    if ($which === '') if (!$which = acct()->id) return FALSE;
    if (is_numeric($which) and !strpos($field, '(')) { // handle numbered records efficiently
      if (!$acct = acct($which)) return FALSE;
      if (strpos($field, ',')) {
        $result = array();
        foreach (u\ray($field) as $one) {
          if (strpos($one, ' AS ')) list($one, $key) = explode(' AS ', $one); else $key = $one;
          $result[$key] = $acct->$one;
        }
        return $result;
      } else return $acct->$field;
    }
  } elseif (!$which) return FALSE;
  
  $result = be\getRecords($table, is_numeric($which) ? "uid=$which" : $which, $subs, $field, TRUE);
  if (count($result) < 1) return FALSE;
  u\EXPECT(count($result) == 1, "ambiguous in tableField(): $which");
/**/ u\EXPECT(is_object($result[0]), 'bad object array in tableField(): ' . print_r($result, 1)); // keep this
  return strpos($field, ',') ? ((array) $result[0]) : $result[0]->$field;
}

/**
 * Process a transfer of USD from a bank into the Dwolla/rCredits account or vice versa.
 * @param assoc $tx: the Dwolla transaction
 * @return TRUE if the transfer has been successfully processed.
 */
function usdTransferDone($tx) {
  extract(u\just('Id Amount Date Type DestinationId SourceId ClearingDate Status', $tx));
  if ($Status != 'processed') return FALSE; // pending
  $Amount = $Type == 'deposit' ? -abs($Amount) : abs($Amount); // don't count on Dwolla's sign
  $omit = t\est() ? (\variable_get('cgf_uid') - 1) : NULL; // ignore tester's account
  if (!$payer = r\whoseUsd($Type == 'deposit' ? $DestinationId : $SourceId, $omit)) return FALSE; // tester
  $info = u\ray('txid amount completed', $Id, $Amount, strtotime($ClearingDate));
  if (!$old = db\lookup('completed+1', 'r_usd', 'txid=:txid', u\ray('txid', $Id))) { // system crashed before it got recorded
    $info += u\ray('created payer payee', strtotime($Date), $payer, 0);
  } elseif ($old == $info['completed']) return TRUE; // already correctly marked and dated complete

  $DBTX = \db_transaction();
  db\update('r_usd', $info, 'txid', TRUE); // update or (if crash) insert
  if ($Type == 'deposit') {
    $a = r\acct($payer);
    $a->actualUsd(-$Amount); // adjust cache up (double minus) now that money finally came in
    $hour = strftime('%H', strtotime($date)); // creation hour
    $auto = $hour == R_DAILY_HOUR ? t(' automatic') : ''; // (space is intentional)
    r\notify($a->id, 'transfer complete', u\ray('auto amount', $auto, $Amount));
    r\membershipEvent($a, 'bona', $a->usd > 0); // this works for testing too
  }
  unset($DBTX);
  return TRUE;
}

/**
 * Return the amount available for the given operation.
 * @param string $type: purpose of the draw (buy, cash, or exch -- see acct::shortfall())
 * @param assoc $ray: array of all relevant money fields (possibly including dwolla "fees")
 * @param bool $withUsd: whether to include usd amounts (if not, don't reserve "committed" amounts)
 */
function avail($type, $ray, $withUsd = TRUE) {
  extract($ray);
  if ($type == 'exch' or !$withUsd) $usd = $committed = 0; // no exchanging USD credit for USD credit
  if ($type != 'buy') $floor = max(0, $floor, $rewards); // no exchanging into debt
//  u\setDft($fees, $usd > DW_FEE_THRESHOLD ? DW_FEE : 0);
  u\setDft($fees, 0);
  return $usd + $r - $committed - $floor - $fees;
}

function passHash($pass) {
  u\EXPECT(is_string($pass) and $pass !== '', 'bad pass to hash');
  require_once DRUPAL_ROOT . '/includes/password.inc';
  return \user_hash_password($pass);
}

/**
 * Return the median (untested)
 */
function median($field, $table = 'users', $where = 1, $subs = array()) {
  $middle = db\lookup('COUNT(*)', $table, $where, $subs) / 2;
  $limit = floor($middle) . (is_int($middle) ? ', 1' : ', 2');
  return db\lookup('AVG($field)', 'users', "$where ORDER BY $field LIMIT $limit", $subs);
}

/**
 * Return the next sequential bank transfer number for an account.
 * @param int $payer: the account uid
 * @return int: the next unused positive integer (1 if there are no bank transfers yet)
 */
function getUsdTid($payer) {
  return db\lookup('MAX(tid)+1', 'r_usd', 'payer=:payer', compact('payer')) ?: 1;
}

/**
 * Send the account a notice for the suggested action, with a direct link to do it.
 * The link does not require the user to sign in.
 * FOR NOW: just suggest it, with no direct link. (never a link if channel is SMS)
 */
function suggest($a, $index, $subs, $page = 'Preferences') {
  // $subs['url'] = 
  // $index .= '|click option';
  r\notify($a->id, $index, $subs + compact('page'));
}

/**
 * Return the rebate on the given amount for the given account.
 * Never less than R_MIN_REBATE unless the account is a nonperson (no longer used)
 * @todo? or manager of company with no (other) employees (set bit?)
 */
function rebate($from, $to, $amount = 1) {
  if (relation(1, $to, $from, 'isOwner OR draw')) return 0;
  if (relation(1, $from, $to, 'isOwner OR draw')) return 0;
  $a = r\acct($from);
//  return max($a->person() ? R_MIN_REBATE : 0, round($amount * $a->rebate / 100, 2));
  return round($amount * $a->rebate / 100, 2);
}

//function bonus($to, $from, $amount = 1) {return round($amount * (t\est() ? 2 : R_BONUS_FACTOR) * r\acct($to)->rebate / 100, 2);}
function bonus($to, $from, $amount = 1) {return round($amount * (t\est() ? 2 : R_BONUS_FACTOR) * rebate($to, $from), 2);}

// Single line functions that need no explanation
function userField($field, $who = '', $subs = array()) {return tableField($field, 'users', $who, $subs);}
function txField($field, $which, $subs = array()) {return tableField($field, 'r_txs', $which, $subs);}
function setTxState($newValue, $zserial) {setTxField('state', $newValue, $zserial, TRUE);}
function serverUid($regionId = R_SERVER_ID) {return -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('{BODY}', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\lookup('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\lookup($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function makeQueue($callback, $time = 15) {return array('worker callback' => "rCredits\\Cron\\$callback", 'time' => $time);}
function queue($op, $item = array()) {\DrupalQueue::get('q')->createItem(compact('op') + $item);}
function perm($bit) {return $bit - B_RELATED;}
function up() {return \variable_get('up', TRUE);}