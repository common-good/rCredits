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
define('MIN_SECS', 60);
define('HOUR_SECS', 60 * MIN_SECS);
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
define('CGF_EIN', '205280780');

define('R', '<span class="r">r</span>');
define('US', '<span class="us">us</span>');
define('R_VERSION', '1.1'); // software version
define('R_ROOT', DRUPAL_ROOT . '/sites/all/modules/rcredits');
define('R_DATE_FIELDS', 'created completed sent invited giftDate signed login access yesterday'); // date field names for testing
define('R_DATA_FIELDS', 'newAccount by calling wants legalName website description selling coFlags stats verifyBy oneTimePass signupCo stepsDone achMin pinThreshold lastTx helper'); // fields stored in the account's "data" array field (serialized)
define('R_MONEY_FIELDS', 'usd r rewards floor committed'); // financial amount fields in account record
define('R_SECURE_FIELDS', 'usdType usdPhone usdEmail usdPass usdPin auth bankAccount bankAccountType pin cardCode cardCode2'); // non-personal encrypted fields (must be complete list)
define('R_SECRET_FIELDS', 'federalId dob idProof answer ssnData'); // personal identifying information
define('R_SECRET_URL', isPRODUCTION ? 'http://rcredits.org' : (isDEV ? 'http://localhost/rcredits-promo' : 'http://' . STAGE)); // where to store secrets
define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves
define('R_CRON_PERIOD', 5*60); // how many seconds between cron runs (must divide an hour evenly)
define('R_MAX_TASK_TIME', 1*60); // what's the longest a queued cron task might take
define('DRUPAL_UID_TABLES', 'authmap file_managed history node node_revision sessions users_roles watchdog');
define('R_STEPS', 'contact sign dw donate proxies prefs photo connect verify company relations'); // membership steps
define('R_NOLOG', R_SECURE_FIELDS . ' code oneTimeLoginUrl password pass newPass form_build_id form_token form_id ' . R_SECRET_FIELDS); // don't log these

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
define('R_DEFAULT_PURPOSE', t('rCredits/USD exchange')); // default transaction purpose
define('R_FOR_GOODS', t('goods and services')); // default purpose for goods
define('R_FOR_CASH', t('cash/loan/etc.')); // default purpose for non-goods
define('R_CHUNK', 10); // chunk size for buying rCredits (sales amounts are a multiple of this)
define('R_ACHMIN', 10); // minimum granularity of withdrawals from bank account
define('R_ACHMIN_DEFAULT', 100); // suggested achMin
define('R_ACH_TRIES', 3); // how many times to retry an ACH

define('R_SUGGEST_BUMP', 1.25); // suggested minimum multiplier, so as not to unnecessarily sell rCredits
define('R_VPAYMENT_MIN', 0.05); // minimum payment exchange (where bonus rounds to .01)
define('R_COMMERCIAL_ASPECT', '4:3'); // width-to-height ratio for commercial profile picture
define('R_PERSONAL_ASPECT', '3:4'); // width-to-height ratio for personal profile picture
define('R_PIXEL_FACTOR', 200); // what to multiply the aspect ratio components by, to get pixel dimensions
define('R_GET_AUTH_URI', 'usd-get-auth'); // where Dwolla calls us back for getting auth code
define('R_INDIVIDUAL_GIFT', 50); // suggested donation for individuals
define('R_COMPANY_GIFT', 250); // suggested donation for companies
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
define('R_CTTY_USD_SHARE', 1); // fraction of a community's total USD to hold in the community account
define('R_CC_RATE', 2.5); // typical credit card percentage rate
define('R_CODE_LEN', 20); // length of most random identifiers
define('R_DUPTX_MINS', 10); // number of minutes before a duplicate transaction is allowed

define('DW_SITE', isPRODUCTION ? 'https://www.dwolla.com' : 'https://uat.dwolla.com');
//                               : 'http://new.rcredits.org/stage.php?stageWord=' . R_STAGE_WORD . '&request=');
//define('DW_TEST_ACCT', '8127139234'); // "reflector" account on www (totally reimburses anything under $10)
define('DW_TEST_ACCT', '8127419900'); // "reflector" account on www (totally reimburses anything under $10)
define('DW_TESTER_ACCT', '8127413762'); // william's account on Dwolla sandbox
define('DW_TESTER_AUTH', 'lBtIKmAL/b9PzZhrPX0m5SwIGMtAIHcxXiEq2gTMYwtZ+LYK7x');
/*  'usdPhone' => '+14136281021',
  'usdEmail' => 'h8hg.new.aaa@rc4.me',
  'usdPass' => 'a4P33bu8sXhy7mL27ebE*B2',
  */
define('DW_FEE_THRESHOLD', 10); // over this amount, Dwolla charges a fee
define('DW_FEE', .25); // the fee
define('DW_VERIFY_ROOF', 20); // bank verification deposits are less than this (actually less than 15 cents)
define('DW_STEPS', 'Email Phone Address Ssn Kba AuthorizedRepresentative Finished'); // registration steps

define('R_USPS_WEB_TOOLS_ID', '921COMMO5331'); // USPS API credentials

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');

define('R_PICTURE_DIR_OLD', '/sites/default/files/pictures');
define('R_PROOF_DIR_OLD', '/sites/default/files/proofs');

define('R_PICTURE_DIR', '/photos'); // precede this by DRUPAL_ROOT (file) or $base_url (url)
define('R_PROOF_DIR', '/proofs');
define('R_DFT_PICTURE', 'no-photo-available.jpg'); // default profile picture
define('R_DFT_PICTURE_WIDE', 'no-photo-available-wide.jpg'); // default profile picture

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
define('B_MEMBER', 0); // fulfilled membership requirements (ready to be approved). For ctty: is an official CGC
define('B_DW', 1); // has a connected Dwolla account
define('B_OK', 2); // approved to use rCredits for real (set by admin when ready). For ctty: is operational
define('B_BONA', 3); // got signup bonus (good circles - somehow got more r+usd than the initial gift)
define('B_FROZEN', 3); // ctty: is frozen (no member can cash out)
define('B_BANK', 4); // has an effective connected bank account
define('B_u5', 5); //
define('B_CO', 6); // company account (could be personal also, for independent contractor)
define('B_PHOTOID', 7); // photoid required and not yet approved
define('B_CHARGE', 8); // UNUSED (any company can do this) can charge customers without confirmation
define('B_NONUDGE', 9); // don't send reminders
define('B_u10', 10); //
define('B_u11', 11); // 
define('B_PAYROLL', 12); // automatically pay employees their total daily salary - employer chooses pay period)
define('B_NOTICE_WEEKLY', 13); // send notices only weekly
define('B_NOTICE_MONTHLY', 14); // send notices only monthly
//define('B_NOTIFY_EMAIL', 15); // notifications by email
define('B_u15', 15); //
define('B_NOTIFY_SMS', 16); // notifications by SMS
define('B_PAPER', 17); // will not accept electronic statements
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
define('B_CADMIN2', 28); // is a community sub-administrator
define('B_CADMIN', 29); // is a community administrator
define('B_ADMIN', 30); // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('B_CLOSED', 31); // account is closed (data[newAccount] has new qid)
define('B_MAX', 31); // maximum bit number
define('B_ADMINABLE', 'co member dw bona cadmin cadmin2 nonudge closed'); // for cAdmin to set
define('B_LIST', 'member dw ok bona bank u5 co photoid charge nonudge u10 u11 payroll weekly monthly u15 sms paper secret u19 debt draws savings u23 u24 u25 u26 u27 cadmin2 cadmin admin closed');

define('R_SHORTFALL', 'GREATEST(0, minimum-(r+usd))'); // sql for shortfall in account (to draw in)
define('R_DEMAND', 'u.usd'); // definitely WANTS this many rCredits (was LEAST(u.usd, IFNULL(u.minimum,0)-u.r))
define('R_CAPACITY', 'u.usd'); // how much rCredits is each account WILLING and able to buy

// for documentation of IS_OK etc. see u\SUBS()

// Relationship permissions used in calls to acct::can() -- stored (less B_RELATED) in r_relations
define('B_RELATED', 100); // has a relationship record
define('B_READ', 101); // can view Transaction History
define('B_SCAN', 102); // can scan rCard (or view -- permissions are cummulative)
define('B_REFUND', 103); // can undo transactions and make refunds at point of sale
define('B_SELL', 104); // can send invoices (or scan or view)
define('B_BUY', 105); // can make purchase for the account (or sell or scan or view)
define('B_MANAGE', 106); // can manage all aspects of the account
$GLOBALS['share permissions'] = array(t('no access'), t('read transactions'), t('scan rCards'), t('give refunds'), t('send invoices'), t('buy and sell'), t('manage account'));

// Other special pseudo-bit values for acct::can()
define('B_MANAGING_CTTY', 120); // cadmin is actually managing a qualified account
define('B_PERSONISH', 121); // personal account or is admin
define('B_DEV', 122); // isDEV or is admin
define('B_CTTYACTIVE', 123); // account's community is approved to operate
define('B_CTTYUP', 124); // account's community is operating
define('B_SHOW_ALL', 999); // special signal to acct::can() to return all permissions
$GLOBALS['permissions'] = 'related read sell buy manage - company charge - member partner steward - rtrader - - managing_ctty cadmin - admin';

define('FLOW_NONE', 0); // which way can credit flow, with another account
define('FLOW_TO', 1);
define('FLOW_FROM', 2);
define('FLOW_BOTH', 3);

// Invoice status (use data['disputed'] for txs)
define('TX_DENIED', -1); // invoice rejected by the non-originator
define('TX_PENDING', 0); // charged, waiting for confirmation (in all other states, transaction is complete)

// Transaction channels (roughly in order of simplicity and generality of messages)
define('TX_SYSTEM', 0);
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_SMART', 3); // smart phone or other computer-like device
define('TX_TONE', 4); // touch tone phone
define('TX_CRON', 5);
define('TX_LINK', 6); // user clicked a no-signin link
define('TX_CHANNELS', t('System SMS Web App Tone Cron Link'));

// Transaction types (all but type TX_TRANSFER and TX_BANK create rCredits)
define('TX_BANK', -9); // used only internally, to mark bank transfers
define('TX_REFUND', -3); // refund of fees (Dwolla)
define('TX_BONUS', -2); // sales bonus (negative reward types are invisible on transaction history)
define('TX_REBATE', -1); // sales rebate
define('TX_TRANSFER', 0); // normal fund transfer (usually for actual goods and services) -- not creating rC
define('TX_SIGNUP', 1); // signup incentive reward
define('TX_u2', 2); // UNUSED pioneer reward (special early-stage rewards) -- might change later to TX_OTHER
define('TX_INFLATION', 3); // reward to conpensate for inflation
define('TX_HELPER', 4); // reward for inviting and assisting with account opening
define('TX_REWARDS', 4); // above this, it's not "rewards"

define('TX_GRANT', 5);
define('TX_LOAN', 6);
define('TX_INVEST', 7); // equity investment (community purchase are TX_TRANSFER)
define('TX_u8', 8);
define('TX_FINE', 9);
define('TX_TYPES', 'payment signup u2 inflation helper grant loan investment u8 fine'); // this might need tweaks

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
  'nonmember' => t('rCredits invitation followup'),
  'welcome' => t('Welcome to rCredits!'),
  'password-reset' => t('New rCredits PASSWORD'),
  'tell-staff' => t('rCredits STAFF ALERT: {topic}'),
  'notices' => t('rCredits Notices for {range}'),
  'message' => t('rCredits {topic}'),
);
$GLOBALS['topics'] = array( // topics for 'message' email
  'account suspended' => t('Account SUSPENDED'),
  'account funded' => t('Account FUNDED'),
  'approved' => t('Account APPROVED!'),
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

  $toMe = ($payee == $acct->id);
  $reverses = isset($data['undoes']);
  $role = $toMe ? 'payee' : 'payer';
  u\both($tx, 'purpose', $toMe, $payeeFor, $payerFor);
  u\both($tx, 'tid', $toMe, @$payeeTid, @$payerTid); // no tid if unconfirmed
  list ($payerPurpose, $payeePurpose) = array($payerFor, $payeeFor); // clearer names, for substitutions
  $tofrom = $toMe ? t('from') : t('to');
  $uid = $otherUid = ($toMe ? $payer : $payee);
  $confirmAction = $toMe ? t('charge') : ($channel == TX_SMART ? t('credit') : t('pay'));

  If ($amount < 0) { // for POS
    $amount = -$amount; // rephrase as positive
    $tofrom = t('to');
    $confirmAction = $toMe ? t('credit') : t('charge');
    u\both($tx, 'did', $toMe, $goods ?  t('refunded') : t('credited'), t('charged'));
  } else u\both($tx, 'did', $toMe, $reverses ? t('re-charged') : t('charged'), ($channel == TX_SMART ? t('credited') : t('paid')));

///  debug(compact('tx','toMe','amount','reverses','goods'));

  $amount = u\fmtAmt($amount);
  $short = u\fmtAmt($shortfall);
  if ($getMyBalance) {
    if ($acct->can(B_BUY)) $balance = u\fmtAmt($acct->rAvailable('buy', TRUE));
  }
  
  if ($goods) {
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
 * Return an error message if the given invoice is already paid or doesn't exist.
 * @param int $nvid: record ID of invoice
 * @param assoc $inv: (RETURNED) the invoice record
 * @return the error message (FALSE if no error)
 */
function badUnpaidInv($nvid, &$inv = '') {
  $fields = 'nvid,status,amount,payer,payee,purpose,data,created';
  if (!$inv = db\lookup($fields, 'r_invoices', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['status'] > 0) return 'already paid';
}

/**
 * Return the rCredits account id for the given USD account id.
 * @param string $usdAccountId: the Dwolla ID (with no hyphens)
 * @param int $omit: an account id to ignore (optional)
 */
function whoseUsd($usdAccountId, $omit = NULL) {
  $usdAccount = bin2hex(u\ezencrypt(str_replace('-', '', $usdAccountId), R_WORD));
  $where = 'usdAccount=:usdAccount' . (@$omit ? ' AND uid<>:omit' : '');
  if (t\est()) $where .= " AND mail<>':R_TESTER_EMAIL'";
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
  $where = ':uid IN (payer, payee) AND xid<:xid AND type>=0';
  return db\lookup('MAX(IF(payee=:uid, payeeTid, payerTid))', 'r_txs', $where, compact('uid', 'xid')) + 1;
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
 * @param bool $done: whether this is a transaction (TRUE) or invoice (FALSE)
 * @return array: the list of transactions created (main, rebate, bonus)
 */
function transact($info, $done = TRUE) {
  $txs = array();

  $DBTX = db_transaction();
  $txs[] = txSave(txSetup($info));
  r\txRewards($txs);
  r\cacheBothTotals($txs);
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
function txRewards(&$txs) {
  extract(u\just('type serial goods payer payee data payerTid payeeTid created', $txs[0]));
  
  if ($type != TX_TRANSFER) return;
  if (!$goods) return; // quit unless rebates and bonuses apply
  if (owning($payer, $payee)) return; // no rewards for transactions between owner and owned
  $rewards = unserialize($data);
  
  foreach (array('rebate', 'bonus') as $type) {
    $amount = $rewards[$type];
    $to = $type == 'rebate' ? $payer : $payee;
    $tid = $type == 'rebate' ? $payerTid : $payeeTid;
    $type = u\consta('tx', $type);
    if ($amount and $to > 0) $txs[] = txSave(txRewardSetup($serial, $type, $to, $amount, $tid, $created));
  }
}

function txRewardSetup($serial, $type, $payee, $amount, $tid, $created) {
  $payer = $payerAgent = r\acct($payee)->community;
  $payeeAgent = $payee;
  $taking = FALSE;
  $goods = FALSE;
  $for = ($type == TX_REBATE ? t('rebate') : t('bonus')) . t( ' on #') . $tid;

  return txSetup(compact(u\ray('serial type amount goods payer payee payerAgent payeeAgent for taking created'))); // created is here mostly for testing
}

/**
 * Create a new transaction and set up all the important fields.
 * @param assoc $info: transaction info.
 *   amount: (required) how much to transfer
 *   payer, payee: (required) uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   etc.
 * @return the array, with some extra field information added
 * Give full rewards for donations to CGF, whether in r or USD
 */
function txSetup($info) {
  global $channel, $boxUser; // through what medium was the transaction requested (web, SMS, etc.), what machine
  $fields = 'type data taking goods amount payer payee payerAgent payeeAgent created';
  extract(u\just($fields . ' for', $info));

  $specs = " -- payer=$payer payee=$payee";
  u\EXPECT(compact(u\ray('amount payer payee')), 'float zid zid');
  u\EXPECT($payer and $payee and $payer != $payee, 'null or self trading' . $specs);
  list ($a1, $a2) = array(r\acct($payer), r\acct($payee));
  u\EXPECT($a1->ok and $a2->ok, 'non-member in tx' . $specs);
  
  u\setDft($type, TX_TRANSFER);
  u\setDft($taking, FALSE);
  u\setDft($goods, FALSE);
  u\setDft($for, $goods ? R_FOR_GOODS : R_FOR_CASH);
  u\setDft($payerAgent, $payer);
  u\setDft($payeeAgent, $payee);
  u\setDft($created, REQUEST_TIME);
//  $rewardBasis = ($payee == r\cgfId() and $for == 'donation') ? $amount : $r;
  $data = serialize(dataSetup($payer, $payee, $amount, $goods, @$data ?: array()));
  $taking += 0;
  $goods += 0;
  $box = @$boxUser ?: 0;

  list ($payerFor, $payeeFor) = is_array($for) ? $for : array($for, $for);
  unset($info['for']);

  return compact(u\ray($fields . ' payerFor payeeFor channel box')) + $info;
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
  return $data;
}

function cacheBothTotals($txs) {foreach(u\ray('payer payee') as $one) cacheTotals($txs[0][$one], $txs);}

/**
 * Keep track of r balance, rewards, committed amount in the account record
 * None of these figures is authoritative except 'committed'. 
 * The r balance and rewards are (definitively) the sum of the relevant fields in transactions.
 * The USD balance is (definitively) what is actually in the Dwolla account.
 */
function cacheTotals($uid, $txs) {
  extract(u\just('amount type xid payer payee created', $txs[0]), EXTR_PREFIX_ALL, 't');
//  if ($t_state < TX_DONE) return; // don't change totals if transaction doesn't count (yet)
  if ($uid <= 1) return; // cache only usd for community and region

  $a = r\acct($uid);
  $cache = $a->account('r rewards committed lastTx access');
  $cache->access = REQUEST_TIME; // track last transaction or login (just the major access)

  $toMe = $uid == $t_payee ? -1 : 1;
  if ($t_payer > 0) $cache->lastTx = array($t_xid, $toMe * ($t_payee - $t_payer), round($toMe * $t_amount, 2), $t_created);

  foreach ($txs as $tx) {
    extract(u\just('payer payee amount type', $tx));
    if ($payer == $uid or $payee == $uid) $a->recache3($payee == $uid ? $amount : -$amount, $type, $cache);
  }

  $a->update((array) $cache);
  if ($t_type == TX_TRANSFER and $t_amount > 0) r\membershipEvent($a, 'bona', TRUE);
}

/**
 * Say whether an account's proposed transaction duplicates the last recent one.
 * @param int $uid: the account's record ID
 * @param int $other: the other account record ID
 * @param float $amount: the amount proposed to transfer from the account to the other account (can be negative)
 * @return TRUE if the proposed transaction duplicates the last one.
 * @see also r\cacheTotals(), where the transaction data is recorded
 */
function dupTx($uid, $other, $amount) {
  if (!$lastTx = r\acct($uid)->lastTx) return FALSE;
  list ($xid, $dif, $oldAmt, $created) = $lastTx;
  t\log(compact(u\ray('uid other amount lastTx')));
  return ($dif == $other - $uid and $amount == $oldAmt and REQUEST_TIME - $created < R_DUPTX_MINS * MIN_SECS);
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
  if (!$preapproved or $a->id < 0) return 0; // just an invoice or ctty, funds don't have to be there (yet)
  return $a->shortfall($goods ? 'buy' : 'cash', $request, $confirmed);
}

/**
 * Say whether the named employee works for the account owner.
 * @param string $name: the employee's name
 * @param int $us: the employer's account ID (defaults to current account ID)
 * @return int: the person's account ID if s/he does work for us, otherwise FALSE
 */
function worksForUs($name, $us = NULL) {
  u\setDft($us, r\acct()->id);
  $words = explode(' ', u\roughName($name)); // abbreviate middle names
  if (strlen($words[0]) > 1) { // unless first name is already abbreviated
    for ($i = 1; $i < count($words) - 1; $i++) $words[$i] = substr($words[$i], 0, 1) . '%';
  }
  $pattern = join(' ', $words);
  
  $sql = <<<EOF
    SELECT u.uid, u.data FROM r_relations r
    INNER JOIN users u ON u.uid=r.other
    WHERE r.main=:us AND r.employerOk
    ORDER BY (u.fullName LIKE :pattern) DESC
EOF;
  $q = db\q($sql, compact('us', 'pattern'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $data = unserialize($data);
    $legalWords = explode(' ', u\roughName($data['legalName']));
    if (count($words) != count($legalWords)) continue;
    for ($i = 0, $match = TRUE; $i < count($words) and $match; $i++) {
      if (!u\abbreviates(str_replace('%', '', $words[$i]), $legalWords[$i])) $match = FALSE;
    }
    if ($match) return $uid;
  }
  return FALSE;
}

/**
 * Return a link to a "no signin required" action.
 * @param assoc $data:
 *   op, once, expires, id, amount, sql, subs, etc.
 * @return a clickable link that will do something without requiring the member to sign in
 * @see also r\Web\rdo()
 */
function makeDo($data) {
  global $base_url;
  extract($data = u\rayy(func_get_args()) ?: $data);
  
  if (@$expires) $data['expires'] = ($expires += REQUEST_TIME);
  if (@$once) $data = u\ray('op', db\insert('r_do', u\ray('uid expires data', @$id, @$expires, serialize($data))));

  return "$base_url/do/" . r\makeDoCode($data);
}

/**
 * Wrap all email, so it doesn't go out when testing.
 */
function rMail($index, $email, $subs, $lingo = '') {
  global $testEmails;
  $subs['email'] = $email;
  $testEmails[] = compact(u\ray('index email subs'));
  if (!isPRODUCTION) if ($email != R_REGION_EMAIL and $email != R_TESTER_EMAIL) $email = 'none@example.com'; // black hole
  \drupal_mail('rCredits', $index, $email, '', $subs);
}

/**
 * Email an arbitrary message to the member, NOW (otherwise use notify).
 * @param int $uid: account record ID
 * @param string $index: index to arbitrary HTML message body (between salutation & closing) or the HTML itself
 * @param assoc $subs: message parameters
 */
function message($uid, $index, $subs = array()) {
  $message = u\tt($index, $subs);
  $topic = @$GLOBALS['topics'][$index] ?: $index; // index should never be used (wrong language) but don't fail
  r\notify($uid, 'message', compact('message', 'topic'), TRUE);
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $subs: substitutions for the message
 * @param bool $force: send by email NOW
 */
function notify($uid, $index, $subs = array(), $force = FALSE) {
  global $channel;
  $acct = acct($uid);
  u\EXPECT($acct, 'no account');
  $subs['uid'] = $uid;
  $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone
  $fullName = $acct->fullName;
  $warnIndexes = u\ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $acct->mail, $subs); // no need to pass $acct->language
  } else { //if ($acct->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(u\tt($index, $subs), REQUEST_TIME);
    if (t\est()) t\output("Notice to $fullName: $message");
    db\insert('r_notices', compact(u\ray('uid message created')));
  }
  
  if ($acct->can(B_NOTIFY_SMS)) {
    if ($num = $acct->smsNumber) $smsed = SMS\send($num, u\tt($index, $subs, TX_SMS));
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
 * Suggest a higher minimum (because of a failed purchase).
 * @param acct $a: the account that needs a higher minimum
 */
function suggestMin($a) {
  $suggested = u\fmtAmt($suggested0 = max(R_ACHMIN, $a->rewards, (R_SUGGEST_BUMP * $a->minimum) ?: R_MINR_DEFAULT));
  $a1 = 'a href="' . r\makeDo('op id amount once', 'min', $a->id, $suggested0, TRUE) . '"';
  r\notify($a->id, 'you were short', compact(u\ray('suggested a1')));
}
  
/**
 * Create an rCard security code for the given account (either for an individual or a company agent).
 * @param acct $a: the account
 */
function makeCardCode($a) {
  $secure = u\ray($a->proSe ? 'cardCode' : 'cardCode2', r\cardCode($a->qid)) + $a->agent->secure;
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
    if ($status == 'bona' and !$a->co) {
      be\fund($a->id, TX_SIGNUP, R_SIGNUP_BONUS, t('signup bonus'));
      //if (@$a->helper) be\fund($a->helper, TX_HELPER, R_HELPER_BONUS, t('reward for inviting and assisting'));
      r\message($a->id, 'account funded');
      $helper = t(' invited by: ') . r\acct($a->helper)->fullName . " ($a->mainQid)";
    }
    if ($status == 'ok') {
      u\EXPECT(@r\acct()->cAdmin, 'should be ctty admin'); // only ctty admin can approve an account
      r\message($a->id, 'approved');
    }
    unset($DBTX);
    
    $fullName = $a->fullName;
    $quid = $a->mainQid;
    tellStaff('event', compact(u\ray('fullName quid status helper')), $a->id);
	  return TRUE;
  } else return FALSE;
}

/**
 * Try to transfer funds from donor to CGF.
 * @param assoc $info: the gift record data
 * @return TRUE if the funds were successfully transferred.
 */
function acceptGift($info) {
  extract(u\just('donid uid giftDate amount often honor', $info));
  $a = r\acct($uid);
  if ($a->shortfall('buy', $amount)) return FALSE; // wait until tomorrow, if not enough
  if ($a->shortfall('buy', $amount, TRUE)) return FALSE; // actually draw from other accounts, as needed
  $cgf = r\acct(r\cgfId());

//  r\usd::beginAtom();
  if ($often != 1) {
    $months = $often == 'M' ? 1 : ($often == 'Q' ? 3 : 12);
    $recurs = u\ray(t('once monthly quarterly annual'));
    $count = db\recCount('r_gifts', 'uid=:uid AND often=:often', compact('uid', 'often'));
    $recurring = t(' (@how gift #@count)', u\ray('@how @count', $recurs[strpos('1MQY', $often)], $count));
  }

  $purpose = @$honor == 'share' ? t('sharing rewards with CGF') : (t('donation') . @$recurring);
  list ($msg, $args, $confirm) = be\transfer('payment', $a, $cgf, $amount, TRUE, $purpose);
///  debug(compact('msg','args','confirm'));
  if (@$args['success']) {
    $completed = REQUEST_TIME;
    db\update('r_gifts', compact('donid', 'completed'), 'donid');
    if ($often != 1) {
      $giftDate = strtotime($months . ' months', $giftDate);
      $share = -1; // mark this field N/A (irrelevant)
      db\insert('r_gifts', compact(u\ray('uid giftDate amount often share')));
    };
//    r\usd::commit();
    r\tellStaff('gift accepted', $info + $args);
    r\notify($uid, 'gift sent', $args);
    return TRUE;
  } //else r\usd::rollback();
  return FALSE;
}

/**
 * Notify rCredits staff (Community Admin, if uid is in their area)
 * @param string $index: the message or message index
 * @param assoc $subs: substitutions for the message OR (if no subs in message) data to show
 * @param bool/int $uid: related account record id (defaults to current account), none if FALSE
 */
function tellStaff($index, $subs = array(), $uid = TRUE) {
  if (empty($subs)) $subs = array(); // accept ''
  foreach (u\ray('federalId dob answer secure pass') as $one) unset($subs[$one]); // secret stuff
  ksort($subs); // put the keys in a predictable order, for testing and easy reading
  $message = u\tt($index, $subs) . u\seeAssoc($subs);

  $email = R_REGION_EMAIL; // default (tell sysAdmin)
  if ($uid) {
    if ($mya = r\acct($uid > 1 ? $uid : '')) {
      $message = "$mya->fullName (#$mya->id): $message";
      if ($ca = $mya->cAdminUid()) $email = r\acct($ca)->mail; // tell Community Admin
    } else $uid = 1;
  } else $uid = 1; // default to sysadmin

  $noFrame = TRUE;
  $topic = strlen($index) < 20 ? strip_tags($index) : '';
  r\rMail('tell-staff', $email, compact(u\ray('email message topic noFrame uid')));
  if ($email != R_REGION_EMAIL) { // temporary until ctty admins get established
    $email = R_REGION_EMAIL;
    r\rMail('tell-staff', $email, compact(u\ray('email message topic noFrame uid')));
  }
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
 * @param bool $allRelated: set all related transactions too -- ie ones with the same serial (default FALSE)
 * NOTE: tx data changes(date, agent, field, oldvalue) (this gives a complete history of the transaction)
 *     (prefixes added to the payerFor or payeeFor fields when changing state will not be included in changes)
 */
function setTxField($fieldName, $newValue, $xid, $allRelated = FALSE) {
  $myid = acct()->id;
  u\EXPECT(compact(u\ray('fieldName xid allRelated')), 'name zid bool');
  
  $where = "xid=:xid and $myid in (payer, payee)"; // "$myid in" is a security measure
  $txs = db\records('r_txs', $where, compact('xid'));
  $tx = (array) $txs[0];
  extract(u\just("$fieldName data payer payee amount goods", $tx));
  
  $data = unserialize($data);
  $data['changes'][] = array(REQUEST_TIME, acct()->agentId, $fieldName, $$fieldName);

  $DBTX = \db_transaction();
  
  $setGoods = ($fieldName == 'goods');
  if ($setGoods or $fieldName == 'amount') {
    extract(u\just('rebate bonus', $data));
    if (@$rebate or @$bonus) {
      db\q('DELETE FROM r_txs WHERE serial=:xid AND xid<>:xid', compact('xid')); // remove any rewards
      $txs = array(array('amount' => -$amount) + $tx); // negate the amount
      $txs[] = u\ray('type payer payee amount', -1, TX_REBATE, $payer, -$rebate);
      $txs[] = u\ray('type payer payee amount', -1, TX_BONUS, $payee, -$bonus);
      r\cacheBothTotals($txs); // subtract values from cache for the old transaction
      unset($data['rebate']);
      unset($data['bonus']);
    }
    $tx[$fieldName] = $newValue;
    if ($tx['goods']) {
      $tx['data'] = serialize($data = dataSetup($payer, $payee, $tx['amount'], TRUE, $data));
      $txs = array($tx);
      r\txRewards($txs); // create new rewards transactions
    } else $txs = array($tx);
    r\cacheBothTotals($txs); // cache values for the revised transaction
  }
    
  $data = serialize($data);
  $sql = "UPDATE r_txs SET $fieldName=:newValue,data=:data WHERE $where";
  db\q($sql, compact(u\ray('newValue data xid')));

  if ($allRelated) {
    $sql2 = "UPDATE r_txs SET $fieldName=:newValue WHERE serial=:xid AND type<>:TX_TRANSFER";
    db\q($sql2, compact(u\ray('newValue xid'))); // gotta do the auxiliary records separately so as not to affect their data field (separate query, to avoid "unbuffered query" error -- known Drupal bug)
  }
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
    if ($uid < 0) return '!' . quid(-$uid);
    //if ($uid < 0) return u\n2a(-$uid - 1, -3); // -1 because region and ctty uids are bumped down by 1
    $region = uidRegion($uid);
    $regionNum = (-$region - 1) / R_REGION_MAX;
    $i = $uid - -$region;
    //$region = -uidRegion($uid) - 1;
    //$i = ($uid - 1) - $region * R_REGION_MAX; // -1 because uids are all bumped up by 1
    return u\n2a($regionNum, -3) . R_MEMBER_MARK . u\n2a($i, -3);
  } else {
    $reid = relation('reid', $uid, $agentId); // compound quid
    return R_SERVER_ID . R_AGENT_MARK . u\n2a($reid, -3);
  }
}

/**
 * Return the uid for a pro se quid (local or complete).
 */
function unQuid($quid, $zid = NULL, $region = NULL) {
  if (is_null($zid)) list ($zid, $region) = r\unQid($quid); // break it down
  return $zid < 0 ? FALSE : (-$region + $zid);
}

/**
 * Return a "local version" pro se quid without the region but with a leading dot.
 */
function quid2($uid) {return strstr(quid($uid), R_MEMBER_MARK);}

/**
 * Accept abbreviations of alphabetic ids for accounts, transactions, and relations
 * @param string $q: the possibly-abbreviated id
 * @return: the full id (if $q is numeric or an already-full alphabetic id, return it unchanged)
 */
function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return strtoupper(($c1 == R_MEMBER_MARK or $c1 == R_AGENT_MARK) ? (R_SERVER_ID . $q) : $q);
}

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
  $uid = abs($uid) - 1; // -1 because uids are all bumped up by 1
  return -1 - ($uid - ($uid % R_REGION_MAX));
  //return -floor(($uid - 1) / R_REGION_MAX) - 1; // -1 because uids are all bumped up by 1
}

/**
 * Return the rCredits region id for the given geographical location
 * @param int $country: country ID
 * @param int $state: state or province ID
 * @param int $zip: postal code
 * @param string $raw: (RETURNED) the alphabetic identifier for the region (usually 3 characters)
 * @return int: the region uid
 */
function region($country = '', $state = '', $zip = '', &$raw = '') {
  $where = "s.country_id=:country AND s.id=:state AND (r.zips='' OR :zip RLIKE r.zips) ORDER BY (r.zips='')";
  $region = db\lookup('region', 'r_regions r INNER JOIN r_states s ON r.state=s.abbreviation', $where, compact('state', 'zip', 'country'));
  if (!$region) $region = db\lookup("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  return -r\unQuid(($raw = $region ?: 'ZZZ') . '.AAA');
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
  extract(u\just('amount toMe data payer payee payerTid payeeTid taking', $oldRecord));
  $data = unserialize($data);
  if (@$data['undoneBy']) {
    $oldTid = $toMe ? $payeeTid : $payerTid;
    $subs = compact('oldTid');
    return 'already undone';
  }

  $a1 = r\acct($toMe ? $payee : $payer, $mya->agentId);
  $a2 = r\acct($toMe ? $payer : $payee); // second account's permissions are irrelevant
  if ($err = r\txPermErr($a1, $a2, $toMe, $toMe xor $amount < 0)) return $err;

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
  return db\lookup('uid', 'users', $where, compact(u\ray('postalCode'))) ?: serverUid();
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName) {
  $name = $name0 = u\shortName($fullName);
  for ($i = 2; ; $i++) {
    if (!db\exists('users', 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name or email.
 * @todo later: when regional prefix is required for signin, this function will ignore it in matching name field
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if (u\isQid($id = trim(strtoupper($id))) and $acct = acct($id)) return $acct->id; // try to interpret as a qid
  return db\lookup('uid', 'users', 'name=:id OR mail=:id', compact('id'));
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

  if ($ch == TX_WEB and $mya and $mya->admin) return 0; // don't create spurious devices as admin
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
 * Assign the given account a new id (or delete it).
 * @param int $oldUid: the account's current record ID
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by deleteAccount)
 * @return the new uid
 */
function deleteAccount($uid) {r\changeUid($uid, FALSE, FALSE, TRUE);}
function changeUid($oldUid, $newUid = FALSE, $unusable = TRUE, $delete = FALSE) {
  $a = r\acct($oldUid);
  if ($delete) {
    foreach (u\ray('r_usd r_txs') as $table) {
      if (db\exists($table, 'payer=:oldUid OR payee=:oldUid', compact('oldUid'))) return r\Web\say('Account "@name" has transactions in @table, so it cannot be deleted.', u\ray('name table', $a->fullName, $table), 'err');
    }
  } elseif (!$newUid) {
    $region = @$a->postalCode ? r\region(@$a->country, @$a->state, $a->postalCode) : communityUid();
    $newUid = r\acct::nextId($region);
  }
  $subs = array( // all tables are listed, to be sure we don't leave any out
    'r_ach:uid',
  // r_areas
    'r_boxes:uid',
  // r_countries
    'r_do:uid',
    'r_gifts:uid',
  // r_industries
    'r_invites:inviter',
    'r_invites:invitee',
    'r_invoices:payer',
    'r_invoices:payee',
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
    'r_stats:ctty',
    'r_txs:payer',
    'r_txs:payee',
    'r_txs:payerAgent',
    'r_txs:payeeAgent',
    'r_usd:payer',
    'r_usd:payee',
  // r_usderror
    'r_user_industries:uid',
    'users:uid',
    'users:community',
  );
  foreach (u\ray(DRUPAL_UID_TABLES) as $one) $subs[] = "$one:uid";
  foreach (u\ray('picture proof') as $one) { // file ops cannot be rolled back, so they must follow db ops
    $oldFilename = DRUPAL_ROOT . $a->auxFilename($one, TRUE); // get old name while account exists
    $newFilename = str_replace("/$oldUid-", "/$newUid-", $oldFilename);
    $files[$oldFilename] = $newFilename;
  }

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field) = explode(':', $one);
    if ($delete) {
      db\q("DELETE FROM $table WHERE $field=:oldUid", compact('oldUid'));
    } else db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }
  $oldQuid = r\quid($oldUid);
  if ($unusable and !$delete) {
    list ($uid, $name, $mail) = array($oldUid, "unusable-$oldQuid", "unusable$oldQuid@rcredits.org");
    db\insert('users', compact(u\ray('uid name mail'))); // mark old uid unusable
  }
  foreach ($files as $old => $new) if (file_exists($old)) {
    if ($delete) unlink($old); else rename($old, $new);
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
  if (!db\exists('r_txs', 'payee=:uid and r>0 and payer>0', compact('uid'))) return FALSE;
  if (!db\lookup('COUNT(DISTINCT payee)', 'r_txs', 'payer=:uid and goods', compact('uid')) >= R_BUYS_BEFORE_BONUS) return FALSE;
  return TRUE;
}*/

/**
 * Record an invitation
 * @param string $email: email address to which the invite was sent
 * @param int $inviter: the inviter's account uid
 * @param string $code: proof of the invitation (that invitee must have, in order to sign up) (DEFAULT random)
 * @param string $subject, $message: temporary params (can be eliminated from the table at some point)
 *
 * @return string: the code
 * @see also r\Web\invitation()
 */
function invite($email, $inviter, $code = '', $subject = '', $message = '') {
  $code = $code ?: u\code();
  $invited = REQUEST_TIME;
  $info = compact(u\ray('code invited email inviter subject message'));
  db\insert('r_invites', $info);
  return $code;
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
    $_POST = array(); // make sure it doesn't act like we submitted the target page
    if (strpos($page, 'http://') !== FALSE) $page = '';
    t\output("Going to page $page (page0=$lastGo msg=$msg field=$field)");
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
  $totals = \variable_get('r_totals');
  if ($calculate or !@$totals or !@$totals[$ctty]) {
    $totals[$ctty] = getTotals($ctty == 'ALL' ? NULL : $ctty);
    \variable_set('r_totals', $totals);
  }
  foreach ($totals[$ctty] as $key => $value) {
    $value = @$value + 0;
    $fancy[$key] = $value >= 1000000 ? ('$' . number_format(($value - 9999) / 1000000, 2) . ' million') : u\fmtAmt($value);
  }
  
  return (object) ($totals[$ctty] + compact('fancy'));

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
  $sql = "SELECT $sums MAX(IF(type=:TX_REBATE, amount, 0)) AS maxRebate FROM r_txs WHERE $where";
  $result2 = db\q($sql, $subs)->fetchAssoc();
  $balance = -$result1['r'];
  return $result1 + $result2 + compact(u\ray('balance'));
}

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region (0 means ALL)
 * @param int $when: latest desired statistics (defaults to now)
 *   if $when < 0, calculate the latest
 * @return assoc of named statistics (see r_stats table def)
 */
function stats($ctty = 0, $when = REQUEST_TIME) {
  if ($when < 0) $when = getStats($ctty);

  $sql = 'SELECT * FROM r_stats WHERE created<=:when AND ctty=:ctty ORDER BY id DESC LIMIT 1';
  if ($info = db\q($sql, compact('ctty', 'when'))->fetchAssoc()) return $info;

  $keys = db\q("SELECT column_name FROM information_schema.columns WHERE table_name='r_stats'")->fetchCol();
  return array_fill_keys($keys, 0);
}

/**
 * Return the amount the community has ever paid out
 */
function cttyPaidEver($ctty = 0) {
  $sum = 'cttyBuys+signups+rebates+bonuss+inflations+refunds+grants+loans+invests+fines+cttyOut';
  return db\lookup($sum, 'r_stats', 'created=' . strtotime('today'));
}
  
/**
 * Calculate and record statistics for the given community.
 * See the "// get" lines to see what statistics are collected and returned.
 * @param int $ctty: community or region (0 means ALL)
 * @param int $created: creation date for the statistics record (defaults to current time)
 * @return $created
 * @todo: change r field to bal and include replacements for USD
 */
function getStats($ctty, $created = NULL) {
  u\setDft($created, t\est() ? REQUEST_TIME : time());
  $subs = compact('created', $ctty ? 'ctty' : '');
  list ($cttyCrit, $payerCrit, $cttysCrit) = $ctty ? 
    array('community=:ctty', 'payer=:ctty', '(u.community=:ctty OR u2.community=:ctty)')
  : array('1', 'payer<1', '1');
    
  // get pAccts, bAccts, newbs, savings
//       SUM(r) AS r, SUM(usd) AS usd, 
  $sql = <<<EOF
    SELECT SUM(IF(firstTx<:created AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(firstTx<:created AND :IS_CO, 1, 0)) AS bAccts,
      SUM(IF(firstTx<:created, 0, 1)) AS newbs,
      SUM(picture)/100 AS savings
    FROM users u LEFT JOIN (
      SELECT payee, MIN(created) AS firstTx
      FROM r_txs GROUP BY payee
    ) t ON t.payee=u.uid
    WHERE uid NOT IN (0,1) AND $cttyCrit AND u.created<:created 
EOF;
  $result1 = db\q($sql, $subs)->fetchAssoc() ?: array();

  // get buyers, sellers
  // (@todo?)
    
  $sums = '';
  foreach (u\ray('cttyBuy signup rebate bonus inflation helper refund grant loan invest fine') as $one) { // fine<=0
    $type = $one == 'cttyBuy' ? TX_TRANSFER : u\consta('tx', $one);
/*    if ($type == TX_TRANSFER) $type .= " AND $payerCrit AND amount<>0";
    $value = $type == TX_TRANSFER ? 
        "IF(type=$type AND $payerCrit, amount, 0)" // rCredits issued as ctty purchases
      : "IF(type=$type, amount, 0)";
*/
    $sums .= "SUM(IF(type=$type AND $payerCrit, amount, 0)) AS `{$one}s`, ";
  }
  // get purchases, signups, rebates, bonuss, inflations, helpers, refunds, grants, loans, investments, fines, shared
  $cttyInOut = $ctty == 0 ? '' : <<<EOF
    SUM(IF(u2.community=:ctty AND u.community<>:ctty, amount, 0)) AS cttyIn,
    SUM(IF(u.community=:ctty AND u2.community<>:ctty, amount, 0)) AS cttyOut,
EOF;
    
  $sql = <<<EOF
    SELECT $sums SUM(IF(channel=:TX_CRON AND payeeFor='sharing rewards with CGF', amount, 0)) AS shared,
    $cttyInOut
    SUM(IF(goods, amount, 0)) AS purchases, SUM(IF(payer>1 AND NOT goods, amount, 0)) AS forCashs
    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created<:created
EOF;
  $result2 = db\q($sql, $subs)->fetchAssoc() ?: array();

  // get r
  $result2['r'] = array_sum($result2) - $result2['shared'] - $result2['purchases'] - $result2['forCashs'];
  $result2['r'] -= 2 * @$result2['cttyOut'];

  // get usd, modify r
  
  $cttyInOut = $ctty == 0 ? '' : <<<EOF
    SUM(IF(u.community=:ctty AND t.payee NOT IN (-1,0), amount, 0)) AS usd2r,
    SUM(IF(u2.community=:ctty, amount, 0)) AS r2usd,
EOF;

  $sql = <<<EOF
    SELECT
    $cttyInOut
    SUM(IF(t.payee=0, -amount, 0)) AS usd,
    SUM(IF(t.payee=-1, amount, 0)) AS fees
    FROM r_usd t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.completed AND t.completed<:created
EOF;
  $usdResult = db\q($sql, $subs)->fetchAssoc() ?: array();
  $usdNetIn = @$usdResult['r2usd'] - @$usdResult['usd2r'];
  $result2['usd'] = $usdResult['usd'] + $usdNetIn - $usdResult['fees'];
  $result2['r'] -= $usdNetIn;
///  debug(compact('ctty','result2','usdResult'));

  // get perMo, txsPerMo, p2b(Txs)PerMo, b2b(Txs)PerMo, b2p(Txs)PerMo, cash(Txs)PerMo
  $subs += u\ray('monthAgo', strtotime('-1 month', $created));

  $sql = <<<EOF
    SELECT 
    SUM(IF(goods, amount, 0)) AS perMo,
    SUM(IF(goods AND NOT u.:IS_CO AND u2.:IS_CO, amount, 0)) AS p2bPerMo,
    SUM(IF(goods AND u.:IS_CO AND u2.:IS_CO, amount, 0)) AS b2bPerMo,
    SUM(IF(goods AND u.:IS_CO AND NOT u2.:IS_CO, amount, 0)) AS b2pPerMo,
    SUM(IF(goods, 0, amount)) AS cashPerMo,

    SUM(IF(goods, 1, 0)) AS txsPerMo,
    SUM(IF(goods AND NOT u.:IS_CO AND u2.:IS_CO, 1, 0)) AS p2bTxsPerMo,
    SUM(IF(goods AND u.:IS_CO AND u2.:IS_CO, 1, 0)) AS b2bTxsPerMo,
    SUM(IF(goods AND u.:IS_CO AND NOT u2.:IS_CO, 1, 0)) AS b2pTxsPerMo,
    SUM(IF(NOT goods, 1, 0)) AS cashTxsPerMo

    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created>:monthAgo AND t.created<:created
EOF;
  $result3 = db\q($sql, $subs)->fetchAssoc() ?: array();

  // get bankIn(Txs)PerMo, bankOut(Txs)PerMo
  $sql = <<<EOF
    SELECT SUM(IF(amount<0, -amount, 0)) AS bankInPerMo,
    SUM(IF(amount<0, 0, amount)) AS bankOutPerMo,
    
    SUM(IF(amount<0, 1, 0)) AS bankInTxsPerMo,
    SUM(IF(amount<0, 0, 1)) AS bankOutTxsPerMo

    FROM r_usd t LEFT JOIN users u ON t.payer=u.uid
    WHERE $cttyCrit AND t.completed AND t.payee=0
    AND completed<:created AND :monthAgo>0
EOF;
  $usdResult = db\q($sql, $subs)->fetchAssoc() ?: array(); // "AND :monthAgo>0" just to match sub count
  $result4 = db\q($sql . ' AND completed>:monthAgo', $subs)->fetchAssoc() ?: array();

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
 * @param float $min: minimum end-of-day balance in the period
 * @param float $days: (RETURNED) (UNUSED) number of days from start to end, inclusive
 * @return float: the average balance for that period (possibly with fractional cents)
 */
function averageBalance($uid, $start, $end = REQUEST_TIME, &$min = '', &$days = '') {
  //u\EXPECT($uid > 1, 'bad uid for avg');
  $days = round(($end + 1 - $start) / DAY_SECS); // rounding is for daylight savings time changes
  $min = $bal = be\creditInfo(u\ray('uid asof', $uid, $start))->balance;
  $day = 0;
  $avg = 0;
  $sql = <<<EOF
    SELECT SUM(IF(payee=:uid, amount, -amount)) AS total, FLOOR((completed-$start)/:DAY_SECS) AS day 
    FROM (
      (SELECT payer,payee,amount,created AS completed FROM r_txs)
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
    $min = min($min, $bal);
  }
  $dayx = $day;
  $day = round(($end - $start) / DAY_SECS); // end of period
  $avg += $bal * ($day - $dayx) / $days;
  $min = round($min, 2);
///debug(compact('uid','day','dayx','bal','avg'));

  return $avg; // average end-of-day balance
}

/**
 * Check account permissions for a transaction or invoice
 * @param acct $a1: the account paying or requesting payment
 * @param acct $a2: the other account
 * @param bool $taking: the first account is taking money from the second (charging or invoicing)
 * @param bool $neg: the amount is negative (refunding or undoing)
 * @return [message, subs, confirmed] on error, else false
 */
function txPermErr($a1, $a2, $taking = TRUE, $neg = FALSE) {
  global $scanned;
  if (!$a2->ok) return array('non member', u\ray('otherName', $a2->fullName), FALSE);
  list ($myAccess, $otherAccess) = u\order($taking xor $neg, B_SELL, B_BUY);
  //list ($myAccessErr, $otherAccessErr) = u\order($taking, 'sell', 'buy');
  list ($myWhat, $otherWhat) = u\order($taking xor $neg, t('sales'), t('purchases'));
  if ($scanned and $taking) list ($myAccess, $myWhat) = $neg ? array(B_REFUND, t('refunds')) : array(B_SCAN, t('sales'));
///  debug(compact(u\ray('taking neg scanned myAccess otherAccess myWhat otherWhat')));
  if (!$a1->can($myAccess)) return array("no perm", u\ray('what', $myWhat), FALSE);
  if (!$a2->can($otherAccess)) return array("other no perm", u\ray('otherName what', $a2->agent->fullName, $otherWhat), FALSE);
//  if (!$a1->can($myAccess)) return array("no $myAccessErr", array(), FALSE);
//  if (!$a2->can($otherAccess)) return array("other no $otherAccessErr", u\ray('otherName', $a2->agent->fullName), FALSE);

  if (!$a1->cttyActive) return array('community not active', u\ray('whose', t('Your')), FALSE);
  if (!$a2->cttyActive) return array('community not active', u\ray('whose', t('That account\'s')), FALSE);
  if (!$a1->cttyUp) return array('your community down', u\ray('contact', $a1->cAdminContact()), FALSE);
  if (!$a2->cttyUp) return array('other community down', array(), FALSE);
  if ($a1->id == $a2->id) return array('no self-trading', array(), FALSE);
  return FALSE;
}

/**
 * @param mixed $which: criteria or record id (defaults to $myid if users)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 *//*
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
  
  $result = db\records($table, is_numeric($which) ? "uid=$which" : $which, $subs, $field, TRUE);
  if (count($result) < 1) return FALSE;
  u\EXPECT(count($result) == 1, "ambiguous in tableField(): $which");
///  u\EXPECT(is_object($result[0]), 'bad object array in tableField(): ' . print_r($result, 1)); // keep this
  return strpos($field, ',') ? ((array) $result[0]) : $result[0]->$field;
}
*/

/**
 * Process a transfer of USD from a bank into the Dwolla/rCredits account or vice versa.
 * @param assoc $tx: the Dwolla transaction
 * @return TRUE if the transfer has been successfully processed.
 */
function usdTransferDone($tx) {
  extract(u\just('Id Amount Date Type DestinationId SourceId ClearingDate Status', $tx));
  if ($Status != 'processed') return FALSE; // pending
  $Amount = $Type == 'deposit' ? -abs($Amount) : abs($Amount); // don't count on Dwolla's sign
  $omit = t\est() ? (r\cgfId() - 1) : NULL; // ignore tester's account
  if (!$payer = r\whoseUsd($Type == 'deposit' ? $DestinationId : $SourceId, $omit)) return FALSE; // tester
  $info = u\ray('txid amount completed', $Id, $Amount, strtotime($ClearingDate));

  $old = db\lookup('completed', 'r_usd', 'txid=:txid', u\ray('txid', $Id)); // existing completion date
  if (is_null($old)) { // no record: system crashed before it got recorded
    $info += u\ray('created payer payee', strtotime($Date), $payer, 0); // create whole record
  } elseif ($old == $info['completed']) return TRUE; // already recorded and correctly dated complete

  $DBTX = \db_transaction();
  db\update('r_usd', $info, 'txid', TRUE); // update or (if crash) insert
  
  if ($Type == 'deposit') {
    $a = r\acct($payer);
    if (!$old) $a->actualUsd(-$Amount); // adjust cache up (double minus) now that money finally came in
    $hour = strftime('%H', strtotime($date)); // creation hour
    $auto = $hour == R_DAILY_HOUR ? t(' automatic') : ''; // (space is intentional)
    $amount = u\fmtAmt(abs($Amount));
    r\notify($a->id, 'transfer complete', compact('auto', 'amount'));
    r\membershipEvent($a, 'bona', $a->usd > 0); // this works for testing too
  }
  unset($DBTX); // if system crashes in the middle of that, user might get duplicate emails -- no biggie
  return TRUE;
}

/**
 * Return the amount available for the given operation.
 * @param string $type: purpose of the draw (buy, cash, or exch -- see acct::shortfall())
 * @param assoc $ray: array of all relevant money fields
 */
function avail($type, $ray) {
  extract($ray);
  if ($type == 'exch') $usd = $committed = 0; // no exchanging USD credit for USD credit
  if ($type != 'buy') $floor = max(0, $floor, $rewards); // no exchanging into debt
  return $usd + $r - $committed - $floor;
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
  $middle = db\recCount($table, $where, $subs) / 2;
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
 * Start/Stop the system OR return the "up" status.
 * @param bool $up: whether to start (TRUE), stop (FALSE), or query (NULL, the default)
 * @return TRUE if the system is up
 */
function up($up = NULL) {
  if (is_null($up)) return \variable_get('up', TRUE);
  u\log('System is ' . ($up ? 'UP' : 'DOWN'));
  return \variable_set('up', $up);
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
//  return max(!$a->co ? R_MIN_REBATE : 0, round($amount * $a->rebate / 100, 2));
  return round($amount * $a->rebate / 100, 2);
}

//function bonus($to, $from, $amount = 1) {return round($amount * (t\est() ? 2 : R_BONUS_FACTOR) * r\acct($to)->rebate / 100, 2);}
function bonus($to, $from, $amount = 1) {return round($amount * (t\est() ? 2 : R_BONUS_FACTOR) * rebate($to, $from), 2);}

// Single line functions that need no explanation
//function userField($field, $who = '', $subs = array()) {return tableField($field, 'users', $who, $subs);}
//function txField($field, $which, $subs = array()) {return tableField($field, 'r_txs', $which, $subs);}
function serverUid($regionId = R_SERVER_ID) {return -1 - R_REGION_MAX * u\a2n($regionId);} // -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('{BODY}', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\lookup('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\lookup($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function makeQueue($callback, $time = 15) {return array('worker callback' => "rCredits\\Cron\\$callback", 'time' => $time);}
function queue($op, $item = array()) {\DrupalQueue::get('q')->createItem(compact('op') + $item);}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
function doCode($code) {return @unserialize(@u\ezdecrypt(@u\deurlify($code)));}  
function makeDoCode($a) {return u\urlify(u\ezencrypt(serialize($a)));}
function cgfId() {return \variable_get('cgf_uid');}
function disputeTx($xid) {return db\setData('r_txs', $xid, u\ray('disputed', TRUE));}
function denyInv($nvid) {return db\setData('r_invoices', $nvid, u\ray('denied', TRUE));}
function stepsDone0() {return array_fill_keys(u\ray(R_STEPS), FALSE);}
