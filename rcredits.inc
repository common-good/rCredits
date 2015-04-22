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
define('WEEK7_SECS', 52 * DAY_SECS);
define('MONTH_DAYS', 30.4368);
define('YEAR_SECS', round(12 * MONTH_DAYS * DAY_SECS));
define('US_COUNTRY_ID', 1228); // United States
define('R_BULLET', '&#8226;'); // bullet in emails (where CSS is difficult)
define('STATS_GRAIN', 10); // granularity of statistics, in days
define('INFLATION_DESC', t('inflation adjustment'));

require_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server
require_once __DIR__ . '/rcredits-db.inc';
require_once __DIR__ . '/acct.class';
require_once __DIR__ . '/x.class';

define('CGF_LEGALNAME', 'Society to Benefit Everyone, Inc.');
define('CGF_EMAIL', isDEV ? 'wspademan+cgf@gmail.com' : 'info@CommonGoodFinance.org'); // Common Good Finance email and other data
define('CGF_PHONE', '+14136281723');
define('CGF_ADDRESS', '48 Buckland Road');
define('CGF_CITY', 'Ashfield');
define('CGF_POSTALADDR', 'PO Box 21, Ashfield, MA 01330');
define('CGF_EIN', '205280780');

define('R', '<span class="r">r</span>');
define('US', '<span class="us">us</span>');
define('R_VERSION', '2.1'); // software version
define('R_ROOT', DRUPAL_ROOT . '/sites/all/modules/rcredits');
define('GAME_COMPANIES', '.,.,.,.,Gift Shop,Religious Institution,For Rent,Empty,Vacant Lot,Land,Cinema,Printing & Office Supply,Car Repair,Health Club/Gym,Restaurant,Hardware & Supply,Grocer,Farm,Food Processing,Department Store,Electric Company,Oil Company,Town Government,Community College,Hospital');
define('R_DATE_FIELDS', 'created completed deposit sent invited giftDate signed login access yesterday'); // date field names for testing
define('R_DATA_FIELDS', isGAME ? 'legalName usd members toBona locus boughtAt lastTx' : 'legalName newAccount by calling wants website description shortDesc selling coFlags gross employees verifyBy signupCo stepsDone achMin pinThreshold lastTx tenure moves mediaConx jid getAppData appData'); // fields stored in the account's "data" array field (serialized)
define('R_MONEY_FIELDS', 'r rewards floor committed'); // financial amount fields in account record
define('R_SECURE_FIELDS', 'pw2 address postalAddr faxetc cardCode cardCode2 pin federalId ssnData dob bankAccount photoId question answer idProof oneTimePass'); // non-personal encrypted fields (must be complete list)
define('R_VSECURE_FIELDS', 'federalId ssnData dob bankAccount photoId'); // very secure fields (admin-encrypted)
//define('R_SECURE_FIELDS', 'cardCode cardCode2 question'); // non-personal encrypted fields (must be complete list)
//define('R_SECRET_FIELDS', 'federalId dob idProof answer ssnData photoId pin bankAccount'); // personal identifying information
define('R_HIDE_CHANGES', R_SECURE_FIELDS . ' changes init r rewards committed floor login access activated data secure stepsDone lastTx pass notes picture photo trust risk risks stats getAppData appData'); // do not record changes to these fields (generally anything secret, or often-changed -- perhaps automatically)
define('R_APP_DATA_GETS', '!log !device !members !txs '); // data requests from the rPOS app
define('R_AVAIL_BUY', '(r-committed-IF(:IS_DEBT, floor, GREATEST(floor, 0)))'); // balance available to spend
define('R_AVAIL_CASH', '(r-committed-GREATEST(0, floor, rewards))'); // balance available to cash out
define('R_SHARE_MAX', 50); // minimum sharing percentage
define('R_SHARE_DFT', 50); // default sharing percentage

define('R_SECRET_URL', isPRODUCTION ? 'http://rcredits.org' : (isDEV ? 'http://localhost/rcredits-promo' : 'http://' . STAGE)); // where to store secrets
define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves
define('R_CRON_PERIOD', 5*60); // how many seconds between cron runs (must divide an hour evenly)
define('R_MAX_TASK_TIME', 1*60); // what's the longest a queued cron task might take
define('DRUPAL_UID_TABLES', 'authmap file_managed history node node_revision sessions users_roles watchdog');
define('R_STEPS', 'contact sign donate proxies prefs photo connect company relations'); // membership steps
define('R_NOLOG', R_SECURE_FIELDS . ' data secure secret code oneTimeLoginUrl password pass newPass form_build_id form_token form_id'); // don't log these
define('R_NOLOG_FORMS', 'Admin Test Handy footer accounts Summary'); // don't log visits to these forms
define('R_ANON_FORMS', R_NOLOG_FORMS . ' Signup Signin Sinx Pass Empty TestRecovery ChangePassword Help Agreement CttyFunds Reports Flags Game Menu I ResetPassword Profile Accept Buy HandleInvoice Directory'); // no login required

define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_MIN_PHOTO_WIDTH', 240); // how wide to make profile pictures (unless too long)
define('R_MAX_PHOTO_WIDTH', 1024);
define('R_MAX_UPLOAD_SIZE', 3); // maximum upload size in MB
define('R_MAX_PHOTOID_SIZE', 2.5); // Dwolla's limit (MB)
define('R_URL_SLASH', '%_%'); // Drupal gets confused by slashes in a url, even if urlencoded
define('R_MAX_DAILY_EXCHANGE', 1000); // per Common Good Finance policy and to avoid formal FinCEN regulation
define('R_INVITE_DAYS', 30); // number of days to accept invitation to membership
define('R_MINR_DEFAULT', 200); // suggested minimum amount to hold in rCredits
//define('R_MAXR_DEFAULT', -1); // suggested maximum amount to hold in rCredits (-1 = no limit)
define('R_CARDCODE_LEN', 14); // length of rCard security code (long to deter easy copying, max 14 for small QR)
define('R_CARD_COST', 5); // cost to order a company or replacement rCard
define('R_FREEZE_TIME', FALSE); // how long to freeze newly bought rCredits (was '2 weeks')

define('R_WHYS', t('other money,loan/reimbursement/etc.,goods and services')); // transaction purposes
define('R_FOR_USD', 0); // index into R_WHYS for USD exchange
define('R_FOR_NONGOODS', 1); // index into R_WHYS for loan, etc.
define('R_FOR_GOODS', 2); // index into R_WHYS for goods

define('R_CHUNK', 10); // chunk size for buying rCredits (sales amounts are a multiple of this)
define('R_ACHMIN', 10); // minimum granularity of withdrawals from bank account
define('R_ACHMIN_DEFAULT', 50); // suggested achMin
define('R_ACH_TRIES', 3); // how many times to retry an ACH

define('R_SUGGEST_BUMP', 1.25); // suggested minimum multiplier, so as not to unnecessarily sell rCredits
define('R_VPAYMENT_MIN', 0.05); // minimum payment exchange (where bonus rounds to .01)
define('R_COMMERCIAL_ASPECT', '4:3'); // width-to-height ratio for commercial profile picture
define('R_PERSONAL_ASPECT', '3:4'); // width-to-height ratio for personal profile picture
define('R_PIXEL_FACTOR', 200); // what to multiply the aspect ratio components by, to get pixel dimensions
define('R_GET_AUTH_URI', 'usd-get-auth'); // where Dwolla calls us back for getting auth code
define('R_INDIVIDUAL_GIFT', 50); // suggested donation for individuals
define('R_COMPANY_GIFT', 250); // suggested donation for companies
define('R_BUYS_BEFORE_BONUS', R_PILOT ? 1 : 3); // number of purchases required before signup bonus
define('R_REGION_MAX', isDEV ? 46000 : 3000000000); // maximum uids in a region (46k,3T ~sqrt(PHP_INT_MAX))
define('R_ACCT_REGION', '(-FLOOR((uid-1)/' . R_REGION_MAX . ')-1)'); // SQL region expression
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
define('HAS_XFEE', "data LIKE '%s:4:\"xfee\";%'"); // sql to recognize a transaction that has an exchange fee
define('R_DEPOSIT_RETURN_FEE', 20); // what we charge members for a second bounced transfer

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
define('R_NOT_VALID', 'not-valid.png'); // says rcard is not valid for purchases

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

define('CO_CANS', 15); // company permission bits when signed OUT: bits 15-21
define('CO_CHARGE', 0);
define('CO_UNDO', 1);
define('CO_R4USD', 2);
define('CO_USD4R', 3);
define('CO_REFUND', 4);
define('CO_BUY', 5);
define('CO_u21', 6); // unused
define('CO_MANAGE', 7); // this bit is not actually stored here, but pass B_MANAGE to the app in this position
define('CO_CANS_BITS', 7); // number of bits for permissions (each: signed in and out)
define('CO_AGENT_CANS', CO_CANS + CO_CANS_BITS); // company permission bits when signed IN (limited by agent)
define('CO_CANS_MASK', pow(2, CO_CANS_BITS + 1) - 1); // the CANS bits -- signed-IN permission bits are 22-28

define('CO_FAST', 29); // fast checkout -- never ask for photoId (set by cAdmin)
define('CO_TIP', 30); // ask customer what percent tip to add
define('CO_U31', 31); //

define('CO_CAN_BITS', 'charge undo r4usd usd4r refund buy');
define('CO_CAN_BIT_DESCS', 'charge customers,undo last transaction,give rCredits for USD,give USD for rCredits,refund any amount,make payments');

// Permissions, options, and flags (flags field in account record)
// If we need more bits, create a flags2 field and make the B_ defs negative or 32-63 for setBit
// for documentation of IS_OK etc. see u\SUBS()
define('B_MEMBER', 0); // fulfilled membership requirements (ready to be approved). For ctty: is an official CGC
define('B_u1', 1); // UNUSED
define('B_OK', 2); // approved to use rCredits for real (set by admin when ready). For ctty: is operational
define('B_BONA', 3); // got signup bonus (good circles - somehow got more r+usd than the initial gift)
define('B_FROZEN', 3); // ctty: is frozen (no member can cash out)
define('B_COUNTED', 4); // someone got a COUNTED reward for this employee signing up
define('B_u5', 5); //
define('B_CO', 6); // company account
define('B_u7', 7); //
define('B_u8', 8); //
define('B_NONUDGE', 9); // don't send reminders
define('B_SEARCHY', 10); // user wants to be findable by other members, by phone or email
define('B_u11', 11); // 
define('B_PAYROLL', 12); // NYI automatically pay employees their total daily salary - employer chooses pay period
define('B_WEEKLY', 13); // send notices only weekly
define('B_MONTHLY', 14); // send notices only monthly
define('B_u15', 15); //
define('B_BYSMS', 16); // notifications by SMS
define('B_PAPER', 17); // will not accept electronic statements
define('B_SECRET', 18); // don't let merchants tell me my balance on request
define('B_u19', 19); //
define('B_DEBT', 20); // member chose to permit sub-zero balances
define('B_DRAWS', 21); // this account draws on one or more others
define('B_SAVINGS', 22); // the connected bank account is a savings or money market account
define('B_JOINED', 23); // account is a joint account
define('B_u24', 24); // account is the slave in a virtual joint account
define('B_u25', 25); // 
define('B_u26', 26); // 
define('B_REGULATOR', 27); // is a bank partner or government regulator (read-only admin)
define('B_CADMIN2', 28); // is a community sub-administrator
define('B_CADMIN', 29); // is a community administrator
define('B_ADMIN', 30); // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('B_CLOSED', 31); // account is closed (data[newAccount] has new qid)
define('B_MAX', 31); // maximum bit number
define('B_ADMINABLE', 'co member bona cadmin cadmin2 regulator nonudge closed'); // for cAdmin to set
define('B_LIST', 'member u1 ok bona counted u5 co u7 u8 nonudge searchy u11 payroll weekly monthly u15 bysms paper secret u19 debt draws savings joint u24 u25 u26 regulator cadmin2 cadmin admin closed');

define('R_CTTY_BENE', 'cttyBuys+signups+rebates+bonuss+inflations+refunds+grants+loans+invests+fines+tradeOut'); // benefit (ever) to the community)

// Relationship permissions used in calls to acct::can() -- stored (less B_RELATED) in r_relations
define('B_RELATED', 100); // has a relationship record
define('B_NONE', 100); // no access
define('B_READ', 101); // can view Transaction History
define('B_SCAN', 102); // can scan rCard (or view -- permissions are cummulative)
define('B_REFUND', 103); // can undo transactions and make refunds at point of sale
define('B_SELL', 104); // can send invoices (or scan or view)
define('B_BUY', 105); // can make purchase for the account (or sell or scan or view)
define('B_MANAGE', 106); // can manage all aspects of the account
define('B_JOINT', 107); // make this a joint account (subsuming the other, with permission) -- no companies!
$GLOBALS['share permissions'] = array(t('no access'), t('view transactions'), t('accept rCard payments'), t('give refunds & give USD for rCredits'), t('send invoices'), t('make purchases'), t('manage account'), t('joint account'));

// Other special pseudo-bit values for acct::can() and rweb.module menus
define('B_MANAGING_CTTY', 120); // cadmin is actually managing a qualified account
define('B_PERSON', 121); // personal account (!co)
define('B_DEV', 122); // isDEV or is admin
define('B_CTTYACTIVE', 123); // account's community is approved to operate
define('B_CTTYUP', 124); // account's community is operating
define('B_BANK', 125); // account has a connected bank account
define('B_SHOW_ALL', 999); // special signal to acct::can() to return all permissions
$GLOBALS['permissions'] = 'related read sell buy manage - company charge - member partner steward - rtrader - - managing_ctty cadmin - admin';

// Canonic strings
define('S_REFUND', t('refund'));
define('S_USD_OUT', t('USD out'));
define('S_USD_IN', t('USD in'));

// Risk definitions (for accounts and transactions) -- suspiciousness for possible reporting to FinCEN
define('K_THRESHOLD', 200); // how big an amount in a single transaction is suspicious
define('K_DAY_THRESHOLD', 300); // how big an amount spent or received by an individual in one day is suspicious
define('K_WEEK_THRESHOLD', 600); // how large an amount is suspicious in one week
define('K_7WEEK_THRESHOLD', 1200); // how large an amount is suspicious in one 52-day period
define('K_YEAR_THRESHOLD', 2400); // how large an amount is suspicious in one year
define('K_LIMIT_FACTOR', 3); // how many times the various thresholds is a prohibited amount
define('K_UNUSUAL_BUMP', 20); // what percentage 7-week increase (in or out) is suspicious
define('K_BIT_MIN', .05); // lowest risk factor that sets a bit
define('K_RED', 100); // lowest score to flag for investigation
define('K_GREEN', -500); // threshold for an account or transaction to be unquestionably beyond reproach
define('K_LINES', 20); // number of lines to show for each type of each potentially risky record (acct or tx)
define('K_MISER_DONATION', 5); // minimum donation to not be a miser
define('K_MISER_SHARE', 25); // minimum sharing percentage, to not be a miser
define('K_CTTY_THRESHOLD', 100); // minimum size of community transactions to consider for risk analysis

// names and weights of account and transaction risks (and, for negative weights, assurance)
// weight is the number of such risks that together would make the account or transaction suspicious
// a weight of "X" means the risk is named but does not contribute to the total risk score
define('K_ACCT_RISKS', 'adminOk:-0.3,trusted:-4,socialConx:-3,hasBank:-3,geography:4,cashCo:2,new:5,moves:3,rents:10,badConx:2,homeCo:20,shady:3,miser:5,photoOff:1,addrOff:5,ssnOff:1,poBox:5,fishy:2,moreIn:2,moreOut:2,bigDay:1,bigWeek:1,big7Week:1,bigYear:2');
define('K_ACCT_RISKS_EDITABLE', 'adminOk cashCo rents shady photoOff addrOff ssnOff fishy');
define('K_TX_RISKS', 'txAdminOk:-0.3,redo:2,exchange:2,cashIn:5,cashOut:2,fromBank:5,toBank:2,b2p:2,p2p:2,inhouse:2,fromSuspect:3,toSuspect:3,absent:4,invoiceless:4,bigFrom:3,biggestFrom:3,oftenFrom:3,bigTo:3,biggestTo:3,oftenTo:3,offline:10,firstOffline:3,origins:3,suspectOut:2'); // names of transaction risks
define('K_ADMINOK', 0); // community admin blessed the account or transaction
define('K_HASBANK', 3); // account has a connected bank account
define('HAS_BANK', '(risks&(1<<' . K_HASBANK . '))'); // mysql criterion

define('FLOW_NONE', 0); // which way can credit flow, with another account
define('FLOW_TO', 1);
define('FLOW_FROM', 2);
define('FLOW_BOTH', 3);

// Invoice status (use data['disputed'] for txs)
define('TX_DENIED', -1); // invoice rejected by the non-originator
define('TX_PENDING', 0); // charged, waiting for confirmation (in all other states, transaction is complete)

// Transaction channels (roughly in order of simplicity and generality of messages)
define('TX_SYS', 0);
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_POS', 3); // smart phone or other computer-like device
define('TX_TONE', 4); // touch tone phone
define('TX_CRON', 5);
define('TX_LINK', 6); // user clicked a no-signin link
define('TX_CHANNELS', t('System SMS Web App Tone Cron Link'));

// Transaction types (all but type TX_TRANSFER and TX_BANK create rCredits) -- negative types generally not shown
define('TX_BANK', -9); // used only internally, to mark bank transfers
define('TX_XFEE', -4); // exchange fee (for buying rCredits with a credit/debit card or check)
define('TX_REFUND', -3); // UNUSED refund of fees (Dwolla)
define('TX_BONUS', -2); // sales bonus (negative reward types are invisible on transaction history)
define('TX_REBATE', -1); // sales rebate
define('TX_TRANSFER', 0); // normal fund transfer (usually for actual goods and services) -- not creating rC
define('TX_SIGNUP', 1); // signup incentive reward
define('TX_u2', 2); // UNUSED pioneer reward (special early-stage rewards) -- might change later to TX_OTHER
define('TX_INFLATION', 3); // reward to conpensate for inflation
define('TX_HELPER', 4); // reward for inviting and assisting with account opening
define('TX_REWARDS', '-3,-2,-1,1,3,4'); // only these count as rewards

define('TX_GRANT', 5);
define('TX_LOAN', 6);
define('TX_INVEST', 7); // equity investment (community purchase are TX_TRANSFER)
define('TX_u8', 8);
define('TX_FINE', 9);
define('TX_TYPES', 'payment signup u2 inflation helper grant loan investment u8 fine'); // this might need tweaks

define('TX_DEFAULT_PERIOD', 30); // number of days to show in transaction history, by default
define('TX_MAX_ROWS', 100); // maximum number of transactions to show on one screen
define('TX_DOWNLOAD_MONEY_FIELDS', 'frombank fromyou toyou reward net');
define('TX_DATA_FIELDS', 'rebate bonus undoneBy undoes disputed force changes'); // serialized pseudo-fields in the data field of a transaction record

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
  'join accounts' => t('Invitation to Create a Joint Account'),
  'force without perm' => t('Unauthorized transaction'),
);

/**
 * Save a record in the given table, in the appropriate regional section of the id field's number space.
 * @param string $table: table to save in (users or r_relations)
 * @param string $idField: name of record ID field (uid or reid)
 * @param assoc $record: the record to save (possibly with record ID already chosen)
 * @param int $region: record ID of the region in which to create a new record (defaults to server region)
 * @return record ID of the inserted record
 */
function regionalInsert($table, $idField, $record, $region = NULL) {
  if (@$record[$idField]) return db\insert($table, $record); // predestined id (probably for testing)
  if (!@$region) $region = r\serverUid();
  
  for ($try = 0; $try < 20; $try++) { // limit the retries
    $id = $record[$idField] = u\firstUnusedId($idField, $table, -$region, -$region + R_REGION_MAX);
    try {
      if (db\insert($table, $record)) return $id;
    } catch (Exception $e) { // BIG MYSTERY: why does this happen sometimes with no collision?
      r\tellStaff('cannot save regional ' . $e->getMessage(), $record);
    }
  }
  u\EXPECT(FALSE, "failed to save $table record in region $region after $try tries.");
}

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
  $confirmAction = $toMe ? t('charge') : ($channel == TX_POS ? t('credit') : t('pay'));
  $why = u\ray(R_WHYS)[$goods];

  If ($amount < 0) { // for POS
    $amount = -$amount; // rephrase as positive
    $tofrom = t('to');
    $confirmAction = $toMe ? t('credit') : t('charge');
    u\both($tx, 'did', $toMe, $goods == R_FOR_GOODS ?  t('refunded') : t('credited'), t('charged'));
  } else u\both($tx, 'did', $toMe, $reverses ? t('re-charged') : t('charged'), ($channel == TX_POS ? t('credited') : t('paid')));

///  debug(compact('tx','toMe','amount','reverses','goods'));

  $amount = u\fmtAmt($amount);
  $short = u\fmtAmt($shortfall);
  if ($getMyBalance) {
    if ($acct->can(B_BUY)) $balance = u\fmtAmt($acct->avail(R_FOR_GOODS));
  }
  
  if ($goods == R_FOR_GOODS) {
//    u\both($tx, 'rewardType', $toMe, t('bonus'), t('rebate'));
    u\both($tx, 'rewardType', $toMe, t('reward'), t('reward'));
    u\both($tx, 'rewardAmount', $toMe, u\fmtAmt(@$bonus + 0), u\fmtAmt(@$rebate + 0));
  }
  $myName = $acct->fullName;
  $otherName = r\acct($otherUid)->fullName;
  $created = u\fmtDate($created0 = $created);

  $success = TRUE;
  foreach (u\ray('r payer payee') as $one) unset($tx[$one]); // else preempts all subs that start thus
  return compact(u\ray('created created0 xid role why payerPurpose payeePurpose toMe confirmAction amount tofrom balance otherUid myName otherName short success')) + $tx;
}

/**
 * Return an error message if the given invoice is already paid or doesn't exist.
 * @param int $nvid: record ID of invoice
 * @param assoc $inv: (RETURNED) the invoice record
 * @return the error message (FALSE if no error)
 */
function badUnpaidInv($nvid, &$inv = '') {
  $fields = 'nvid,status,amount,payer,payee,goods,purpose,data,created';
  if (!$inv = db\lookup($fields, 'r_invoices', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['status'] > 0) return 'already paid';
}

/**
 * Return the user's sequential transaction number
 * @param int $xid: the transaction record number
 * @param int $uid: the user's uid
 * @return string: the user's transaction number
 */
function xid2tid($xid, $uid) {
  $jid = r\acct($uid)->jid ?: 0;
  $isIt = ' IN (:uid, :jid)';
  $where = "(payer$isIt OR payee$isIt) AND xid<:xid AND type>=0";
  $max = "MAX(GREATEST(IF(payee$isIt, payeeTid, 0), IF(payer$isIt, payerTid, 0)))";
  return db\lookup($max, 'r_txs', $where, compact('uid', 'xid', 'jid')) + 1;
}

/**
 * Trade r in one account for USD in another
 * @param float $amount: the amount to trade (limited to R_MAX_DAILY_EXCHANGE)
 * @param int $uid1: uid of account with r to trade
 * @param int $uid2: uid of account with USD to trade
 * @param string $err: (RETURNED) an error message on failure
 * @return string $txid: USD transaction ID (from Dwolla) or FALSE on failure
 * Caller is responsible for determining whether the accounts have enough r and USD for the trade.
 * NOTE: This function can go away once all Dwolla funds have been pooled into the community account.
 *//*
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
*/

/**
 * Say whether an account's proposed transaction duplicates the last recent one.
 * @param int $uid: the account's record ID
 * @param int $other: the other account record ID
 * @param float $amount: the amount proposed to transfer from the account to the other account (can be negative)
 * @return TRUE if the proposed transaction duplicates the last one.
 * @see also x::cacheTotals(), where the transaction data is recorded
 */
function dupTx($uid, $other, $amount) {
  if (!$lastTx = r\acct($uid)->lastTx) return FALSE;
  list ($xid, $dif, $oldAmt, $created) = $lastTx;
///  t\log(compact(u\ray('uid other amount lastTx')));
  return ($dif == $other - $uid and $amount == $oldAmt and REQUEST_TIME - $created < R_DUPTX_MINS * MIN_SECS);
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
    WHERE r.main=:us AND r.employee
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
 * Join or unjoin this account to another.
 * @param int $reid: relations record ID
 * @param bool $join: whether to join the accounts (otherwise unjoin)
 */
function setJoin($reid, $join) {
  extract(r\relation('main,other', $reid));
  list ($a1, $a2) = [r\acct($main), r\acct($other)];

  if ($join) {
    $a1->unjoin(); // zap any join or other join requests
    if (r\relation('permission', $other, $main) == r\perm(B_JOINT)) { // confirming join
      $a1->join($a2);
    } else { // requesting join
      $atag = $a2->makeDo('join', $main); // seek agreement
      r\message($other, 'join accounts', u\ray('atag name', $atag, $a2->fullName));
    }
  } else $a1->unjoin($a2);
}

/**
 * Return the number of months since the first rdo of this type (for conciseness).
 */
function rdoMonth($time = REQUEST_TIME) {return round(($time - strtotime('12/1/2014')) / (MONTH_DAYS * DAY_SECS));}

/**
 * Wrap all email, so it doesn't go out when testing.
 */
function rMail($index, $email, $subs, $lingo = '') {
  if (isGAME) return; // no emails from game site
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
function message($uid, $index, $subs = []) {
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
function notify($uid, $index, $subs = [], $force = FALSE) {
  global $channel;
  if (isGAME) return;
  $a = r\acct($uid);
  u\EXPECT($a, 'no account');
  $subs['uid'] = $uid;
  $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone
  $fullName = $a->fullName;
  $warnIndexes = u\ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $a->email, $subs); // no need to pass $a->language
  } else { //if ($a->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(u\tt($index, $subs), REQUEST_TIME);
    t\output("Notice to $fullName: $message");
    db\insert('r_notices', compact(u\ray('uid message created')));
  }
  
  if ($a->can(B_BYSMS)) {
    if ($num = $a->smsNumber) $smsed = SMS\send($num, u\tt($index, $subs, TX_SMS));
  }
  if (FALSE and !@$mailed and !@$smsed and array_search($index, $warnIndexes) !== FALSE) {
    $notification = str_replace('-', ' ', $index);
    $warning = u\tt('cannot contact', compact('fullName', 'notification'));
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) SMS\send($a->smsNumber, $warning);
    if ($channel == TX_POS) { } // dunno how to handle this
  }
}

/**
 * Suggest a higher minimum (because of a failed purchase).
 * @param acct $a: the account that needs a higher minimum
 */
function suggestMin($a) {
  $suggested = u\fmtAmt($suggested0 = max(R_ACHMIN, $a->j_rewards, (R_SUGGEST_BUMP * $a->j_minimum) ?: R_MINR_DEFAULT));
  $a1 = $a->makeDo('min', $suggested0);
  $msg = 'you were short';
  
  if ($a->can(B_SECRET)) {
    $a2 = $a->makeDo('nosecret');
    $msg .= '|be not secretive';
  }
  if (!$a->can(B_DEBT) and $a->floor < 0) {
    $a3 = $a->makeDo('debtok');
    $msg .= '|try debt';
  }
  
  r\notify($a->id, $msg, compact(u\ray('suggested a1 a2 a3')));
}

/**
 * Create an rCard security code for the given account (either for an individual or a company agent).
 * @param acct $a: the account
 */
function makeCardCode($a) {
  $field = $a->proSe ? 'cardCode' : 'cardCode2';
  $a->agent->update($field, r\cardCode($a->qid)); // resave the secure fields
}

/**
 * If appropriate, update an account's membership status and send the staff an alert.
 * @param acct $a: the account
 * @param string $status: what milestone or event to alert about
 * @param bool $do: prerequisite for the alert
 * @return <the membership event happened and was handled>
 * If there is an appropriate bit to set, it will be set as appropriate.
 */
function membershipEvent($a, $status, $do = TRUE) {
  $bit = @u\consta('b', $status); // get the appropriate bit, if any
  if (!$do or ($bit !== FALSE and $a->can($bit))) return FALSE; // do nothing or bit already set

  if ($bit !== FALSE) $a->setBit($bit); // set the bit, if any
  if ($status == 'ok') {
    u\EXPECT(@r\acct()->cAdmin, 'should be ctty admin'); // only ctty admin can approve an account
    r\message($a->id, 'approved');
    $a->update('activated', time());
  }
  
  $fullName = $a->fullName;
  $quid = $a->mainQid;
  tellStaff('event', compact(u\ray('fullName quid status')), $a->id);
  return TRUE;
}

/**
 * Try to transfer funds from donor to CGF.
 * @param assoc $info: the gift record data
 * @return TRUE if the funds were successfully transferred.
 */
function acceptGift($info) {
  extract(u\just('donid uid giftDate amount often honor', $info));
  $a = r\acct($uid);
  if ($a->shortfall(R_FOR_GOODS, $amount)) return FALSE; // wait until tomorrow, if not enough
  if ($a->shortfall(R_FOR_GOODS, $amount, TRUE)) return FALSE; // actually draw from other accounts, as needed
  $cgf = r\acct(r\cgfId());

//  r\usd::beginAtom();
  if ($often != 1) {
    $months = $often == 'M' ? 1 : ($often == 'Q' ? 3 : 12);
    $recurs = u\ray(t('once monthly quarterly annual'));
    $count = db\count('r_gifts', 'uid=:uid AND often=:often', compact('uid', 'often'));
    $recurring = t(' (@how gift #@count)', u\ray('@how @count', $recurs[strpos('1MQY', $often)], $count));
  }

  $purpose = @$honor == 'share' ? t('sharing rewards with CGF') : (t('donation') . @$recurring);
  list ($msg, $args, $confirm) = be\transfer('payment', $a, $cgf, $amount, R_FOR_GOODS, $purpose);
  if (@$args['success']) {
    $completed = REQUEST_TIME;
    db\update('r_gifts', compact('donid', 'completed'), 'donid');
    if ($often != 1) {
      $giftDate = u\plusMonths($months, $giftDate);
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
 * @param assoc $subs: substitutions for the message and/or data to show (show both)
 * @param bool/int $uid: related account record id (defaults to current account), none if FALSE
 */
function tellStaff($index, $subs = [], $uid = TRUE) {
  if (empty($subs)) $subs = []; // accept ''
  foreach (u\ray('federalId dob answer secure pass') as $k) unset($subs[$k]); // secret stuff
  ksort($subs); // put the keys in a predictable order, for testing and easy reading
  $message = u\tt($index, $subs) . u\seeAssoc($subs);

  $email = R_REGION_EMAIL; // default (tell sysAdmin)
  if ($uid) {
    if ($mya = r\acct($uid > 1 ? $uid : '')) {
      $message = "$mya->fullName (#$mya->id): $message";
      if ($ca = $mya->cAdminUid()) $email = r\acct($ca)->email; // tell Community Admin
    } else $uid = 1;
  } else $uid = 1; // default to sysadmin

  $noFrame = TRUE;
  $topic = strlen($index) < 20 ? strip_tags($index) : '';
  r\rMail('tell-staff', $email, compact(u\ray('email message topic noFrame uid')));
  if ($email != R_REGION_EMAIL and $email != R_TESTER_EMAIL) { // temporary until ctty admins get established
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
  $update = [];
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
  $result = [];
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
 * @return [zid, regionUid, $isRel] (FALSE on failure) where
 *   zid is the record ID
 *   regionUid is the region account ID
 *   isRel is <this qid represents a relation between two accounts>
 */
function unQid($qid) {
  if (!u\isQid($qid)) {
    $qid = R_SERVER_ID . $qid; // if local, complete it
    if (!u\isQid($qid)) return FALSE;
  }
  list ($regionId, $type, $iid) = preg_split('/(' . R_MARKS . ')/U', $qid, 0, PREG_SPLIT_DELIM_CAPTURE);
  $id = u\a2n($iid);
  return array($id, serverUid($regionId), $type != R_MEMBER_MARK);
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
    $region = uidRegion($uid);
    $regionNum = (-$region - 1) / R_REGION_MAX;
    return u\n2a($regionNum, -3) . R_MEMBER_MARK . u\n2a($uid - -$region, -3);
  } else {
    $reid = r\relation('reid', $uid, $agentId); // compound quid
    return str_replace(R_MEMBER_MARK, R_AGENT_MARK, quid($reid));
  }
}

/**
 * Return the record ID for a quid (local or complete, pro se or not).
 * call by:
 *   unQuid(quid) OR
 *   unQuid('', zid, region, isRel)
 * @param string quid: an account record ID such as NEW.AAA, .AAA, NEW:AAA, :AAA, !NEW, or !NEW.AAA
 * @param int $zid: offset within a region
 * @param int $region: region ID
 * @param bool isRel: <this qid represents a relation between two accounts>
 */
function unQuid($quid, $zid = NULL, $region = NULL, $isRel = FALSE) {
  if (substr($quid, 0, 1) == '!') {
    if (!strpos($quid, '.')) $quid .= '.AAA';
    return -unQuid(substr($quid, 1));
  }
  if (is_null($zid)) list ($zid, $region, $isRel) = r\unQid($quid); // break it down
  return $isRel ? NULL : (-$region + $zid);
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
 * @param string $key: name of cookie
 * @param string $value: value of cookie
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now) 0=end of session
 * @return: the value
 */
function setCook($key, $value, $expires = NULL) {
  u\setDft($expires, REQUEST_TIME + 365250 * DAY_SECS);
  setcookie($key, $value, $expires, '/'); // make it easy (fails without '/')
  if (u\test()) $_COOKIE[$key] = $value;
  return $value;
}

/**
 * Return the rCredits region id for the given account id
 */
function uidRegion($uid) {
  $uid = abs($uid) - 1; // -1 because uids are all bumped up by 1
  return -1 - ($uid - ($uid % R_REGION_MAX));
}

/**
 * Return the rCredits region id for the given geographical location
 * @param int $country: country ID
 * @param int $state: state or province ID (defaults to MA)
 * @param int $zip: postal code
 * @param string $raw: (RETURNED) the alphabetic identifier for the region (usually 3 characters)
 * @return int: the region uid
 */
function region($country = '', $state = '', $zip = '', &$raw = '') {
  if (!$state) $state =  R_STATE_ID;
  $where = ":zip RLIKE r.zips OR (r.zips='' AND s.country_id=:country AND s.id=:state) ORDER BY (r.zips='')";
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
function nonoUndo($oldRecord, &$subs = []) {
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
 *//*
function reverseRelations($other, $main, &$data, $fields) {
  foreach (u\ray($fields) as $field) if (@$data[$field]) $stay = TRUE;
  if (!@$stay and !isset($data['employeeOk'])) return; // this function also UNsets employeeOk
  
  $subs = compact('main', 'other') + u\just($fields, $data);
  if ($reid = relation('reid', $main, $other)) {
    $subs += compact('reid');
    db\update('r_relations', $subs, 'reid');
  } else db\insert('r_relations', $subs);

  foreach (u\ray($fields) as $field) unset($data[$field]); // don't get these fields mixed in with the reverse record
}*/

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
  return relation(1, $company ?: acct()->id, $uid, 'employee');
}

/**
 * Return the community record ID for the given postal code.
 * @param string $postalCode: postal code to identify community (if none, assume something in server community)
 * @param bool $defaultZero: return 0 if postalCode does not match any community? (else return server uid)
 */
function communityUid($postalCode = '', $defaultZero = FALSE) {
  if (!$postalCode) return serverUid();
  $where = "country=':R_COUNTRY_ID' AND :postalCode RLIKE postalCode AND uid=community";
  return db\lookup('uid', 'users', $where, compact(u\ray('postalCode'))) ?: ($defaultZero ? 0 : serverUid());
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
  $mail = u\cryptN(strtolower($id));
  return db\lookup('uid', 'users', 'name=:id OR mail=:mail', compact('id', 'mail'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM r_relations WHERE main=:myid AND employee', compact('myid'))->fetchCol();
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
function relation($field, $main, $other = [], $where = 'TRUE', $subs = []) {
  if (is_numeric($main)) {
    if (empty($other)) {
      $reid = $main;
      $where = "reid=:reid";
      $subs += compact('reid');
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
function changeUid($oldUid, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  $a = r\acct($oldUid);
  if ($delete) {
    if (db\exists($table = 'r_txs', 'payer=:oldUid OR payee=:oldUid', compact('oldUid'))
      or db\exists($table = 'r_usd', 'payer=:oldUid', compact('oldUid'))) {
      return r\Web\say('Account "@name" has transactions in @table, so it cannot be deleted.', u\ray('name table', $a->fullName, $table), 'ERR');
    }
  } elseif (!$newUid) {
    $region = @$a->postalCode ? r\region(@$a->country, @$a->state, $a->postalCode) : r\communityUid();
    $newUid = r\acct::nextId($region);
  }
  $subs = array( // all tables are listed, to be sure we don't leave any out
    'r_ach:uid',
  // r_areas
  // r_banks
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
    'r_near:uid1',
    'r_near:uid2',
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
    'users:helper',
    'users:community',
  );
  foreach (u\ray(DRUPAL_UID_TABLES) as $one) $subs[] = "$one:uid";
  foreach (u\ray('picture picture2 proof') as $one) { // file ops can't be rolled back, so must follow db ops
    $oldFilename = DRUPAL_ROOT . $a->auxFilename($one, TRUE); // get old name while account exists
    $newFilename = str_replace("/$oldUid-", "/$newUid-", $oldFilename);
    $files[$oldFilename] = $newFilename;
  }

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field) = explode(':', $one);
    if ($delete and in_array($one, u\ray('r_boxes:uid r_do:uid r_gifts:uid r_invites:inviter r_invoices:payee r_notices:uid r_proxies:person r_relations:main r_relations:other r_txs:payerAgent r_txs:payeeAgent r_user_industries:uid users:uid'))) {
      db\q("DELETE FROM $table WHERE $field=:oldUid", compact('oldUid'));
    } else db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }
  $oldQuid = r\quid($oldUid);
  if ($unusable and !$delete) {
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQuid", "unusable$oldQuid@rcredits.org");
    db\insert('users', compact(u\ray('uid name email'))); // mark old uid unusable
  }
  foreach ($files as $old => $new) if (file_exists($old)) {
    if ($delete) unlink($old); else rename($old, $new);
  }
  unset($DBTX); // commit
  return $newUid;
}

/**
 * Package the return arguments and alert staff on certain errors.
 * @param string $message: the success or error message
 * @param assoc $info: (optional) message substitutions and other info about transaction
 *   includes success=TRUE if the transaction was successful
 * @param bool $confirm: <to be confirmed> (default FALSE)
 */
function txRet($message, $info = [], $confirm = FALSE) {
  if (!@$info['success']) r\tellStaff('failed transaction', $info + compact('message', 'confirm'));
  return array($message, $info, $confirm);
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
  $email = u\cryptN($email);
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
  if (u\test()) {
    global $lastGo; $lastGo = $page; // for testing
    $_POST = []; // make sure it doesn't act like we submitted the target page
    t\output("Going to page $page (page0=$lastGo msg=$msg field=$field)");
    return strpos($page, 'http://') === FALSE ? t\pageForm($page, $msg, $field) : '';
  }

  foreach (u\ray('login signup password reset') as $one) if (strpos($page, "user/$one") !== FALSE) {
    if (\drupal_session_started()) session_destroy();
  }
  if ($msg) r\Web\say($msg, $field);
  \drupal_goto(substr($page, 0, 1) == '/' ? substr($page, 1) : $page);
}

/**
 * Return parameters for notices for the account.
 * @param int $uid: the account record ID
 * @param string $where: criteria to filter notices for this account
 * @param bool $weekly: digest notices for the week
 * @param bool $monthly: digest notices for the month
 * @param int $at: time of reporting
 * @return assoc of all parameters in the notices template (FALSE if none)
 */
function noticeArgs($uid, $where, $weekly, $monthly, $at = REQUEST_TIME) {
  $a = r\acct($uid);
  $q = db\q("SELECT message, created FROM r_notices WHERE uid=:uid AND ($where)", compact('uid'));

  while ($row = $q->fetch()) {
    $date = ($weekly or $monthly) ? (strftime('%b&nbsp;%d', $row->created)) : R_BULLET;
    $messages[] = "<tr><td class=\"notice-date\" style=\"vertical-align:top; margin-right:5px;\">$date</td><td>$row->message</td></tr>";
  }

  if (@$messages) $messages = join("\n", $messages); else return FALSE;
  $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
  $yestertime = u\fmtDate($start = $unit == 'month' ? u\plusMonths(-1, $at) : strtotime("-1 $unit", $at));
  $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
  extract(u\just('return', $a->j_stats()));
  list ($balance, $rewards, $ourEmail) = array(u\fmtAmt($a->j_r), u\fmtAmt($a->j_rewards), $a->cttyA->email);
  $companies = \variable_get('companies');
  $companies = $companies[u\test() ? r\serverUid() : $a->community];
  return compact(u\ray('messages unit range yestertime balance rewards return ourEmail companies'));
}

/**
 * Trade some rCredits for USD, from somebody. 
 * @param float $request: how much to try to get
 * @param acct $acct: rCredits account that is trading rCredits to third parties
 * @return how much USD we actually delivered
 *//*
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
*/

/**
 * Return the uid of the next account in line wanting/willing and able to trade enough US Dollars for rCredits.
 * If you trade rCredits for USD or vice versa, you go to the end of the line.
 * @param float $amount: the amount we're looking for. Returned with the amount actually available from the returned account. The amount sought depends on the $amount passed as follows:
 *   $amount < R_CHUNK*1.5 -- seek $amount
 *   otherwise -- seek R_CHUNK (leaving R_CHUNK/2 or more for other buyers
 * return: the best account to handle the trade (there may be none that wants so many rCredits)
 *    if no account wants to buy any, return FALSE (this would be really bad and should never ever happen)
 *    NOTE that CGF is the buyer of last resort.    
 *//*
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
*/

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
 *//*
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
  *//*
}
*/

/**
 * Return totals for the given community
 * @param int $ctty: the community account id (NULL means all communities)
 * @return assoc (ctty or system-wide totals of):
 *   r floor rewards usd minimum excess
 *   signup rebate bonus inflation grant loan fine maxRebate
 *   balance (-r) demand (minimum - r)
 *//*
function getTotals($ctty) {
  $subs = @$ctty ? compact('ctty') : [];
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
*/

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region (0 means ALL)
 * @param int $when: latest desired statistics (defaults to now)
 *   if $when < 0, calculate the latest
 * @return assoc of named statistics (see r_stats table def)
 */
function stats($ctty = 0, $when = REQUEST_TIME) {
  if ($when < 0) $when = r\getStats($ctty);

  $sql = 'SELECT * FROM r_stats WHERE created<=:when AND ctty=:ctty ORDER BY id DESC LIMIT 1';
  if ($info = db\q($sql, compact('ctty', 'when'))->fetchAssoc()) return $info;

  $keys = db\q("SELECT column_name FROM information_schema.columns WHERE table_name='r_stats'")->fetchCol();
  return array_fill_keys($keys, 0);
}

/**
 * Return the amount the community has ever paid out
 */
function cttyPaidEver($ctty = 0) {
  return db\lookup(R_CTTY_BENE, 'r_stats', 'created=' . strtotime('today'));
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
  u\setDft($created, u\test() ? REQUEST_TIME : time());
  $subs = compact('created', $ctty ? 'ctty' : '');
  list ($cttyCrit, $payerCrit, $payeeCrit, $cttysCrit) = $ctty ? 
    array('community=:ctty', 'payer=:ctty', 'payee=:ctty', '(u.community=:ctty OR u2.community=:ctty)')
  : array('1', 'payer<1', 'payee<1', '1');
    
/*
  $sql = <<<EOF
    SELECT SUM(IF(firstTx<:created AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(firstTx<:created AND :IS_CO, 1, 0)) AS bAccts,
      SUM(IF(firstTx<:created, 0, 1)) AS newbs,
      SUM(picture)/100 AS savings
    FROM users u LEFT JOIN (
      SELECT payee, MIN(created) AS firstTx
      FROM r_txs GROUP BY payee
    ) t ON t.payee=u.uid
    WHERE uid NOT IN (0,1) AND $cttyCrit AND u.created<=:created 
EOF;
*/

  // get pAccts, bAccts, newbs, savings
  $sql = <<<EOF
    SELECT SUM(IF(activated AND activated<=:created AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(activated AND activated<=:created AND :IS_CO, 1, 0)) AS bAccts,
      SUM(IF(created<=:created AND (NOT activated OR activated>:created), 1, 0)) AS newbs,
      SUM(picture)/100 AS savings
    FROM users
    WHERE uid>1 AND $cttyCrit AND created<=:created 
EOF;
  $res = db\q($sql, $subs)->fetchAssoc() ?: [];

  // get top5
  $sql = <<<EOF
    SELECT SUM(picture)/100 AS top5
    FROM (SELECT picture FROM users WHERE uid>1 AND $cttyCrit AND created<=:created
      ORDER BY picture DESC LIMIT 5) u0
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  
  // get usd
  $sql = <<<EOF
    SELECT
    SUM(IF(amount<0, -amount, 0)) AS bankIn,
    SUM(IF(amount>0, amount, 0)) AS bankOut
    FROM r_usd t LEFT JOIN users u ON u.uid=t.payer
    WHERE $cttyCrit AND t.completed AND t.completed<=:created
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  
  // get buyers, sellers
  // (@todo?)
  $sums = '';
  foreach (u\ray('signup xfee rebate bonus inflation helper refund grant loan invest fine') as $one) { // fine<=0
//    $type = $one == 'cttyBuy' ? TX_TRANSFER : u\consta('tx', $one);
    $type = u\consta('tx', $one);
/*    if ($type == TX_TRANSFER) $type .= " AND $payerCrit AND amount<>0";
    $value = $type == TX_TRANSFER ? 
        "IF(type=$type AND $payerCrit, amount, 0)" // rCredits issued as ctty purchases
      : "IF(type=$type, amount, 0)";
*/
    $sums .= "SUM(IF(type=$type AND $payerCrit, amount, 0)) AS `{$one}s`, ";
  }

  // get cttyBuys, cttyIncomes  
  $sums .= <<<EOF
    SUM(IF(type=:TX_TRANSFER AND ($payerCrit OR ($payeeCrit AND amount<0)), ABS(amount), 0)) AS cttyBuys,
    SUM(IF(type=:TX_TRANSFER AND $payeeCrit AND amount>0, -amount, 0)) AS cttyIncomes,
EOF;

  // get purchases, signups, xfees, rebates, bonuss, inflations, helpers, refunds, grants, loans, investments, fines, shared
  $sums .= $ctty == 0 ? '' : <<<EOF
    SUM(IF(u.community<>:ctty AND u2.community=:ctty, -amount, 0)) AS tradeIn,
    SUM(IF(u2.community<>:ctty AND u.community=:ctty, amount, 0)) AS tradeOut,
EOF;
    
  $sql = <<<EOF
    SELECT $sums SUM(IF(channel=:TX_CRON AND payeeFor='sharing rewards with CGF', amount, 0)) AS shared,
    SUM(IF(goods=:R_FOR_GOODS, amount, 0)) AS purchases, 
    SUM(IF(payer>1 AND goods<>:R_FOR_GOODS, amount, 0)) AS forCashs
    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created<=:created
EOF;
  $res2 = db\q($sql, $subs)->fetchAssoc() ?: [];

  // get r
  $res2['r'] = array_sum($res2) - $res2['shared'] - $res2['purchases'] - $res2['forCashs'];
  $res2['r'] += $res['bankIn'] - $res['bankOut']; // include r provided in exchange for USD
  $res += $res2;

/*
  $cttyInOut = $ctty == 0 ? '' : <<<EOF
    SUM(IF(u.community=:ctty AND t.payee<>0, amount, 0)) AS usd2r,
    SUM(IF(u2.community=:ctty, amount, 0)) AS r2usd,
EOF;
*/

//  $usdNetIn = @$res['r2usd'] - @$res['usd2r'];
//  $res2['usd'] = $res['usd'] + $usdNetIn;
///  debug(compact('ctty','res2','res'));

  // get perMo, txsPerMo, p2b(Txs)PerMo, b2b(Txs)PerMo, b2p(Txs)PerMo, cash(Txs)PerMo
  $subs += u\ray('monthAgo', u\plusMonths(-1, $created));

  $sql = <<<EOF
    SELECT 
    SUM(IF(goods=:R_FOR_GOODS, amount, 0)) AS perMo,
    SUM(IF(goods=:R_FOR_GOODS AND NOT u.:IS_CO AND u2.:IS_CO, amount, 0)) AS p2bPerMo,
    SUM(IF(goods=:R_FOR_GOODS AND u.:IS_CO AND u2.:IS_CO, amount, 0)) AS b2bPerMo,
    SUM(IF(goods=:R_FOR_GOODS AND u.:IS_CO AND NOT u2.:IS_CO, amount, 0)) AS b2pPerMo,
    SUM(IF(goods=:R_FOR_GOODS, 0, amount)) AS cashPerMo,

    SUM(IF(goods=:R_FOR_GOODS, 1, 0)) AS txsPerMo,
    SUM(IF(goods=:R_FOR_GOODS AND NOT u.:IS_CO AND u2.:IS_CO, 1, 0)) AS p2bTxsPerMo,
    SUM(IF(goods=:R_FOR_GOODS AND u.:IS_CO AND u2.:IS_CO, 1, 0)) AS b2bTxsPerMo,
    SUM(IF(goods=:R_FOR_GOODS AND u.:IS_CO AND NOT u2.:IS_CO, 1, 0)) AS b2pTxsPerMo,
    SUM(IF(goods=:R_FOR_GOODS, 0, 1)) AS cashTxsPerMo

    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created>:monthAgo AND t.created<=:created
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  // get bankIn(Txs)PerMo, bankOut(Txs)PerMo
  $sql = <<<EOF
    SELECT SUM(IF(amount<0, -amount, 0)) AS bankInPerMo,
    SUM(IF(amount<0, 0, amount)) AS bankOutPerMo,
    
    SUM(IF(amount<0, 1, 0)) AS bankInTxsPerMo,
    SUM(IF(amount<0, 0, 1)) AS bankOutTxsPerMo

    FROM r_usd t LEFT JOIN users u ON t.payer=u.uid
    WHERE $cttyCrit AND t.completed AND completed<=:created AND completed>:monthAgo
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  foreach ($res as $k => $v) {if (is_null($v)) $res[$k] = 0;}
///  debug($res + compact('created', 'ctty'));
  db\insert('r_stats', $res + compact('created', 'ctty'));
  return $created;
}

/**
 * Return the average balance for the given account (possibly joint) and given period.
 * @param int $uid: account id (can be a community or region too)
 * @param int $start: starting date (should be 0 hours) -- first full day to include in the period
 * @param int $end: ending date (should be 0 hours) -- last full day to include in the period
 * @param bool $noj: ignore any joined account
 * @param float $min: minimum end-of-day balance in the period
 * @param float $days: (RETURNED) (UNUSED) number of days from start to end, inclusive
 * @return float: the average balance for that period (possibly with fractional cents)
 */
function averageBalance($uid, $start, $end = REQUEST_TIME, $noj = FALSE, &$min = '', &$days = '') {
  //u\EXPECT($uid > 1, 'bad uid for avg');
  if (!$days = round(($end + 1 - $start) / DAY_SECS)) return 0; // rounding is for daylight savings time changes
  $min = $bal = be\creditInfo(u\ray('uid asof', $uid, $start))->balance;
  $jid = @$noj ? 0 : (r\acct($uid)->jid ?: 0);
  $isPayer = 't.payer IN (:uid, :jid)';
  $isPayee = 't.payee IN (:uid, :jid)'; // account can be both payer and payee, if joint account

  $day = 0;
  $avg = 0;
  // use "1" as payer for r_usd, to avoid matching jid=0
  $sql = <<<EOF
    SELECT SUM(IF($isPayee, t.amount, -t.amount)) AS total, 
    FLOOR((t.completed-$start)/:DAY_SECS) AS day 
    FROM (
      (SELECT payer,payee,amount,created AS completed FROM r_txs)
      UNION (SELECT payer,1 AS payee,amount,completed FROM r_usd)
    ) t WHERE ($isPayer XOR $isPayee) AND completed BETWEEN $start AND $end 
    GROUP BY day
EOF;

  $result = db\q($sql, compact('uid', 'jid'));
  while($row = $result->fetchAssoc()) {
    $dayx = $day;
    extract($row); // total, day
    $avg += $bal * ($day - $dayx) / $days;
/// debug(compact('uid','day','dayx','bal','avg','total'));
    $bal += $total;
    $min = min($min, $bal);
  }
  $dayx = $day;
  $day = round(($end - $start) / DAY_SECS); // end of period
  $avg += $bal * ($day - $dayx) / $days;
  $min = round($min, 2);
/// debug(compact('uid','day','dayx','bal','avg'));

  return $avg; // average end-of-day balance
}

/**
 * Interpret an integer bit array of risks.
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @return formatted list of risk names, with HTML titles
 */
function riskList($risks, $descs) {  
  global $riskHelp, $base_url;

  $bitNames = array_keys($descs = u\ray($descs));
  foreach ($bitNames AS $bit => $bitName) {
    if (u\getBit($risks, $bit)) {
      $class = $descs[$bitName] < 0 ? 'goodRisk' : 'badRisk';
      $helpIndex = @$riskHelp ? array_search($bitName, array_keys($riskHelp)) : -1;
      $why[] = "<a href=\"$base_url/help/flags@$bitName\" class=\"$class riskHelp$helpIndex\">$bitName</a>";
    }
  }
  return @$why ? join(', ', $why) : '';
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
//  if (!$a1->can($myAccess)) return array("no $myAccessErr", [], FALSE);
//  if (!$a2->can($otherAccess)) return array("other no $otherAccessErr", u\ray('otherName', $a2->agent->fullName), FALSE);

  if ($a1->id > 0 and $a2->id != r\cgfId()) { // allow rewards and donations in not-yet-active communities
    if (!$a1->cttyActive) return array('community not active', u\ray('whose', t('Your')), FALSE);
    if (!$a2->cttyActive) return array('community not active', u\ray('whose', t('That account\'s')), FALSE);
  }
  if (!$a1->cttyUp) return array('your community down', u\ray('contact', $a1->cAdminContact()), FALSE);
  if (!$a2->cttyUp) return array('other community down', [], FALSE);
  if ($a1->id == $a2->id) return array('no self-trading', [], FALSE);
  return FALSE;
}

/**
 * Process a transfer of USD from a bank into the Dwolla/rCredits account or vice versa.
 * @param assoc $tx: the Dwolla transaction
 * @return TRUE if the transfer has been successfully processed.
 *//*
function usdTransferDone($tx) {
  extract(u\just('Id Amount Date Type DestinationId SourceId ClearingDate Status', $tx));
  if ($Status != 'processed') return FALSE; // pending
  $Amount = $Type == 'deposit' ? -abs($Amount) : abs($Amount); // don't count on Dwolla's sign
  $omit = u\test() ? (r\cgfId() - 1) : NULL; // ignore tester's account
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
*/

function passHash($pass) {
  u\EXPECT(is_string($pass) and $pass !== '', 'bad pass to hash');
  require_once DRUPAL_ROOT . '/includes/password.inc';
  return \user_hash_password($pass);
}

/**
 * Return the median (untested)
 */
function median($field, $table = 'users', $where = 1, $subs = []) {
  $middle = db\count($table, $where, $subs) / 2;
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
 * @todo? or manager of company with no (other) employees (set bit?)
 */
function rebate($from, $to, $amount = 1) {
  if (relation(1, $to, $from, 'isOwner OR draw')) return 0;
  if (relation(1, $from, $to, 'isOwner OR draw')) return 0;
  $a = r\acct($from);
  return '' . round(min(R_MAX_REBATE, $amount * $a->rebate / 100), 2); // ''. to avoid serializing a long float
}

function bonus($to, $from, $amount = 1) {return rebate($to, $from, $amount * R_BONUS_FACTOR);}
//function bonus($to, $from, $amount = 1) {return round($amount * (u\test() ? 2 : R_BONUS_FACTOR) * r\acct($to)->rebate / 100, 2);}

/**
 * Return the appropriate exchange fee for the given transaction description and amount.
 */
function xfee($for, $amount) {
  if (is_array($for)) $for = $for[1];
  if (!r\usdin($for)) return '';
  if (strpos($for, '(cash)')) return 0;
  if (!strpos($for, '(card)') and !strpos($for, '(check)')) return 0; // for FCCoop until they update their app
  return '' . round(strpos($for, '(card)') ? -R_XFEE_CARD * $amount : R_XFEE_CHECK, 2);
}
  
// Single line functions that need no explanation
function serverUid($regionId = R_SERVER_ID) {return -1 - R_REGION_MAX * u\a2n($regionId);} // -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('{BODY}', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\lookup('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\lookup($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function makeQueue($callback, $time = 15) {return array('worker callback' => "rCredits\\Cron\\$callback", 'time' => $time);}
function queue($op, $item = []) {\DrupalQueue::get('q')->createItem(compact('op') + $item);}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
function doCode($code) {return @unserialize(@u\decrypt(@u\deurlify($code)));}  
function makeDoCode($a) {return u\urlify(u\crypt(serialize($a)));}
function cgfId() {return \variable_get('cgf_uid');}
function disputeTx($xid) {return db\setData('r_txs', $xid, ['disputed' => TRUE]);}
function denyInv($nvid) {return db\setData('r_invoices', $nvid, ['denied' => TRUE]);}
function stepsDone0() {return array_fill_keys(u\ray(R_STEPS), FALSE);}
function isReward($type) {return in_array($type, u\ray(TX_REWARDS));}
//function usdin($for) {return u\abbreviates('usd in (', strtolower(is_array($for) ? $for[1] : $for));}
function usdin($for) {return u\abbreviates(S_USD_IN, is_array($for) ? $for[1] : $for, TRUE);}
