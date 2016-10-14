<?php
/**
 * @file
 * Definitions common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 */
 
// namespace rCredits\Util; // typically abbreviated as "u"

// definitions used in settings file
define('US_COUNTRY_ID', 1228); // United States
define('R_BULLET', '&#8226;'); // bullet in emails (where CSS is difficult)
define('STATS_GRAIN', 10); // granularity of statistics, in days
define('INFLATION_DESC', t('inflation adjustment'));

require_once __DIR__ . '/boot.inc'; // secret settings for this community's rCredits server
require_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server

// Common Good Finance email and other data
define('CGF_LEGALNAME', 'Society to Benefit Everyone, Inc.');
define('CGF_EMAIL', isDEV ? 'cgf@example.com' : 'info@CommonGoodFinance.org');
define('CGF_PHONE', '+14136281723');
define('CGF_ADDRESS', '48 Buckland Road');
define('CGF_CITY', 'Ashfield');
define('CGF_POSTALADDR', 'PO Box 21, Ashfield, MA 013300021');
define('CGF_EIN', '205280780');
define('R_LAUNCH', strtotime('12/15/2013'));
define('SYS_EMAIL', 'info@rCredits.org');
define('PUBLIC_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.publicKey');
define('PRIVATE_KEY', dirname($_SERVER['DOCUMENT_ROOT']) . '/.privateKey');

define('COMPANY_FREEZE', FALSE); // companies cannot spend rewards yet
define('R', '<span class="r">r</span>');
define('US', '<span class="us">us</span>');
define('GAME_COMPANIES', '.,.,.,.,Gift Shop,Religious Institution,For Rent,Empty,Vacant Lot,Land,Cinema,Printing & Office Supply,Car Repair,Health Club/Gym,Restaurant,Hardware & Supply,Grocer,Farm,Food Processing,Department Store,Electric Company,Oil Company,Town Government,Community College,Hospital');
define('R_REGION_FIELDS', 'region,fullName,st,postalCode,federalId,hasServer');
define('R_DATE_FIELDS', 'created completed deposit sent invited giftDate signed login access yesterday'); // date field names for testing
// isGAME ? 'legalName usd toBona autoInvite locus boughtAt lastTx' : 
define('R_DATA_FIELDS', 'legalName shortName newAccount by calling wants website description shortDesc selling coFlags gross employees payrollStart payrollEnd notifyNowBy signupCo stepsDone achMin pinThreshold lastTx lastiCode tenure moves mediaConx jid getAppData appData'); // fields stored in the account's "data" array field (serialized)
define('R_MONEY_FIELDS', 'r rewards floor committed'); // financial amount fields in account record
define('R_VSECURE_FIELDS', 'federalId ssnData dob bankAccount'); // very secure fields (admin-encrypted)
define('R_SECURE_FIELDS', R_VSECURE_FIELDS . ' pw2 address postalAddr faxetc emailCode cardCode cardCode2 pin question answer idProof oneTimePass last4bank'); // non-personal encrypted fields (must be complete list)
define('R_HIDE_CHANGES', 'pw2 ssnData changes init r rewards committed floor login access activated data secure stepsDone lastTx lastiCode pass oneTimePass notes picture photo trust risk risks stats getAppData appData special last4bank'); // do not record changes to these fields: anything often-changed (automatically or not)
define('R_APP_DATA_GETS', '!log !device !members !txs '); // data requests from the rPOS app
//define('R_AVAIL_BUY', '(r-committed-IF(:IS_DEBT, floor, GREATEST(floor, 0)))'); // balance available to spend
define('R_AVAIL_BUY', '(r-IF(:IS_DEBT, floor, GREATEST(floor, 0)))'); // balance available to spend
//define('R_AVAIL_CASH', '(r-committed-GREATEST(0, floor, rewards))'); // balance available to cash out
define('R_AVAIL_CASH', '(r-GREATEST(0, floor, rewards))'); // balance available to cash out
define('R_SHARE_MAX', 50); // minimum sharing percentage
define('R_SHARE_DFT', 25); // default sharing percentage
define('R_SHARING_MSG', t('sharing rewards with CGF'));

define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves
define('R_CRON_PERIOD', 5*60); // how many seconds between cron runs (must divide an hour evenly)
define('R_MAX_TASK_TIME', 1*60); // what's the longest a queued cron task might take
define('DRUPAL_UID_TABLES', 'authmap file_managed history node node_revision sessions users_roles watchdog');
define('R_STEPS', 'contact sign donate proxies prefs photo connect company relations'); // membership steps
define('R_NOLOG', R_SECURE_FIELDS . ' data secure secret oneTimeLoginUrl password pass newPass form_build_id form_token form_id external_input'); // don't log these (external_input may contain passwords)
define('R_NOLOG_FORMS', 'Admin Test Handy footer accounts history settings settings/icon community Summary'); // don't log visits to these forms
define('R_ANON_FORMS', R_NOLOG_FORMS . ' Signup Signin PageNotFound Sinx Pass Empty TestRecovery ChangePassword Help Community Democracy Agreement CttyFunds Reports Flags Game Menu I ResetPassword Profile Accept Buy HandleInvoice FindCo'); // no login required
define('BY_CHECK', '(check)'); // app includes this in description, for a purchase of rCredits by check
define('BY_CARD', '(card)'); // app includes this in description, for a purchase of rCredits by credit card
define('R_ON_FILE', t('(on file)'));

define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_MIN_PHOTO_WIDTH', 240); // how wide to make profile pictures (unless too long)
define('R_MAX_PHOTO_WIDTH', 1024);
define('R_MAX_UPLOAD_SIZE', 3); // maximum upload size in MB
define('R_URL_SLASH', '%_%'); // Drupal gets confused by slashes in a url, even if urlencoded
define('R_EXCHANGE_LIMITS', '1:5000,7:10000,49:20000,365:50000'); // per day, week, 7-week, year
define('R_INVITE_DAYS', 30); // number of days to accept invitation to membership
//define('R_MAXR_DEFAULT', -1); // suggested maximum amount to hold in rCredits (-1 = no limit)
define('R_CARDCODE_LEN', 14); // length of rCard security code (long to deter easy copying, max 14 for small QR)
define('R_CARD_COST', 5); // cost to order a company or replacement rCard
define('ORGANIZER_GRANT_MIN', 10); // minimum number of individuals to recruit, to be eligible for a grant
define('ORGANIZER_GRANT_MAX', '$5,000'); // maximum organizer grant amount

define('ICARD_EXPIRES', 14); // how many days until invitation card expires, after account is activated
define('ICARD_CODELEN_MIN', 10); // minimum length for an invitation code
define('ILEN_REGION', 15); // 3 letters (radix 26-32 -- in other words, 1-6 can be used in region also)
define('ILEN_TAIL', 19); // 3-4 letters (radix 26)
define('ILEN_SECURITY', 16); // 3 alphanumerics (radix 35)
define('ILEN', ILEN_REGION + ILEN_TAIL + ILEN_SECURITY);
define('ILEN_DIV', 25); // this results in two 5-digit codes (otherwise the integer gets too big)

// codes for iCode field (invitation code)
define('IBY0', -2); // lowest IBY number
define('IBY_FORM', -2); // invited by form (including companies) (the ONLY pre-confirmed signup)
define('IBY_LATE', -1); // invited by form, but accepted late (needs to be confirmed)
define('IBY_SELF', 0); // signed up without an invitation/confirmation yet
define('IBY_EMAIL', 1); // invited by email (this invitation # is re-usable, but needs to be confirmed)
define('IBY_ICARD', 2); // >=this is invitation number, also needing confirmation
define('IBYS', 'form late self email icard');

// notification preferences
$GLOBALS['notifyWhens'] = t('N:Never,I:Immediately,D:Daily,W:Weekly,M:Monthly');
$GLOBALS['notifyNowBys'] = t('E:Email,T:Text message,B:Both'); // how to send immediate messages
define('N_DEFAULTS', 'MMNIIMDIDWD');
define('N_EXPLICIT_OUT', 0); // manual payment or transfer out
define('N_AUTO_OUT', 1); // automatic payment made
define('N_AUTO_IN', 2); // auto-refill initiated
define('N_IN_DONE', 3); // transfer from bank account complete
define('N_INCOME', 4); // payment received
define('N_INFLATION', 5); // inflation adjustment received
define('N_REWARD', 6); // other incentive reward received
define('N_OTHER_IN', 7); // fine, grant, loan, or investment received
define('N_CHANGED', 8); // transaction/invoice edited, canceled, or disputed
define('N_NUDGE', 9); // your invitee needs a nudge
define('N_SUGGESTION', 10); // suggestion from the system

define('R_WHYS', t('goods and services,exchange of US Dollars or other currency,loan/reimbursement/etc.')); // transaction purposes
define('FOR_GOODS', 0); // index into R_WHYS for goods
define('FOR_USD', 1); // index into R_WHYS for USD exchange
define('FOR_NONGOODS', 2); // index into R_WHYS for loan, etc.

define('R_CHUNK', 10); // chunk size for buying rCredits (sales amounts are a multiple of this)
define('R_MIN_DEFAULT', 100); // suggested minimum amount to hold in rCredits
define('R_ACHMIN', 10); // minimum granularity of withdrawals from bank account
define('R_ACHMIN_DEFAULT', 50); // suggested achMin
define('R_ACH_TRIES', 3); // how many times to retry an ACH

define('R_SUGGEST_BUMP', 1.25); // suggested minimum multiplier, so as not to unnecessarily sell rCredits
define('R_VPAYMENT_MIN', 0.05); // minimum payment exchange (where bonus rounds to .01)
define('R_COMMERCIAL_ASPECT', '4:3'); // width-to-height ratio for commercial profile picture
define('R_PERSONAL_ASPECT', '3:4'); // width-to-height ratio for personal profile picture
define('R_PHOTO_WIDTH', 300); // rCard photo width, in pixels
define('R_PHOTO_HEIGHT', 400);
define('R_PIXEL_FACTOR', 200); // what to multiply the aspect ratio components by, to get pixel dimensions
define('R_GET_AUTH_URI', 'usd-get-auth'); // where Dwolla calls us back for getting auth code
define('R_INDIVIDUAL_GIFT', 50); // suggested donation for individuals
define('R_COMPANY_GIFT', 250); // suggested donation for companies
define('R_BUYS_BEFORE_BONUS', 1); // number of purchases required before signup bonus (should probably be 3)
define('R_REGION_MAX', isDEV ? 46000 : 3000000000); // maximum uids in a region (46k,3T ~sqrt(PHP_INT_MAX))
define('R_ACCT_REGION', '(-FLOOR((uid-1)/' . R_REGION_MAX . ')-1)'); // SQL region expression
define('R_DW_LINK', 'http://refer.dwolla.com/a/clk/3bf8cy'); // (credits CGF for referral)
define('R_SIGNIN_HOURS', 24); // window for signing in after registering or resetting password
define('R_MEMBER_MARK', '.'); // divides region from specific identifier in member IDs
define('R_AGENT_MARK', ':'); // divides region from specific identifier in relational IDs
define('AGT_MARK', '-'); // divides main QID from agent code
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

define('R_PICTURE_DIR', '/photos/'); // precede this by DRUPAL_ROOT (file) or $base_url (url)
define('R_PROOF_DIR', '/proofs/');
define('R_DFT_PICTURE', 'no-photo-available.jpg'); // default profile picture
define('R_DFT_PICTURE_WIDE', 'no-photo-available-wide.jpg'); // default profile picture
define('R_NOT_VALID', 'not-valid.png'); // says rcard is not valid for purchases

// account type bits
$GLOBALS['account types'] = array(t('personal'), t('sole proprietor'), t('partnership'), t('private corporation or LLC'), t('nonprofit or government'), t('publicly-traded corporation'), t('co-operative, association, or club'));

// Company Account flags (bits)
define('R_PERSONAL', 0);
define('CO_SOLE_PROPRIETOR', 1);
define('CO_PARTNERSHIP', 2);
define('CO_CORPORATION', 3);
define('CO_NONPROFIT', 4);
define('CO_WORKER_COOP', 5);
define('CO_CONSUMER_COOP', 6);
define('CO_GOVERNMENT', 7);
define('CO_PUBLICLY_TRADED', 8);
define('CO_COOP', 9);
define('CO_U10', 10); //
define('CO_U11', 11); //
define('CO_U12', 12); //
define('CO_PRIVATE', 13); // company should not be listed (
define('CO_ATM', 14); // can trade rCredits for cash, even if balance is less than rewards

define('APP_CANS', 15); // company permission bits when signed OUT: bits 15-21
define('CO_CHARGE', 0);
define('CO_UNDO', 1);
define('CO_R4USD', 2);
define('CO_USD4R', 3);
define('CO_REFUND', 4);
define('CO_BUY', 5);
define('CO_u21', 6); // unused
define('CO_MANAGE', 7); // this bit is not actually stored here, but pass B_MANAGE to the app in this position
define('APP_CAN_BITS', 7); // number of bits for permissions (each: signed in and out)
define('APP_AGENT_CANS', APP_CANS + APP_CAN_BITS); // company permission bits when signed IN (limited by agent)
define('APP_CAN_MASK', pow(2, APP_CAN_BITS + 1) - 1); // the CANS bits -- signed-IN permission bits are 22-28

define('CO_FAST', 29); // (deprecated as of app v 2.16) fast checkout -- never ask for photo Id (set by cAdmin)
define('CO_TIP', 30); // ask customer what percent tip to add

define('APP_CAN_BIT_NAMES', 'charge undo r4usd usd4r refund buy');
define('APP_CAN_BIT_DESCS', 'charge customers,undo last transaction,give rCredits for USD,give USD for rCredits,refund any amount,make payments');

// Permissions, options, and flags (flags field in account record)
// If we need more bits, create a flags2 field and make the B_ defs negative or 32-63 for setBit
// for documentation of IS_OK etc. see u\SUBS()
define('B_MEMBER', 0); // fulfilled membership requirements (ready to be approved).
define('B_CONFIRMED', 1); // invitation has been confirmed (in advance by form invite OR explicitly OR implicitly by newb first buying from the inviter in person)
define('B_OK', 2); // can use rCredits for real (set by admin when ready). Set FALSE to freeze ctty funds (NYI)
define('B_BONA', 3); // got signup bonus (good circles - somehow got more r+usd than the initial gift)
define('B_IDED', 4); // a cashier has verified the member's government-issued photo ID
define('B_REFILL', 5); //
define('B_CO', 6); // company account
define('B_u7', 7); //
define('B_u8', 8); // hold incentive rewards in reserve
define('B_NONUDGE', 9); // don't send reminders
define('B_NOSEARCH', 10); // user wants to be not findable by other members, by phone or email
define('B_u11', 11); // 
define('B_PAYROLL', 12); // NYI automatically pay employees their total daily salary - employer chooses pay period
define('B_WEEKLY', 13); // send notices only weekly
define('B_MONTHLY', 14); // send notices only monthly
define('B_u15', 15); //
define('B_BYSMS', 16); // NYI notifications by SMS
define('B_PAPER', 17); // will not accept electronic statements
define('B_SECRET', 18); // don't let merchants tell me my balance on request
define('B_u19', 19); //
define('B_DEBT', 20); // member chose to permit sub-zero balances
define('B_DRAWS', 21); // this account draws on one or more others
define('B_SAVINGS', 22); // the connected bank account is a savings or money market account
define('B_JOINED', 23); // account is a joint account
define('B_u24', 24); //
define('B_u25', 25); // 
define('B_CLOSED', 26); // account is closed (data[newAccount] has new qid)
define('B_REGULATOR', 27); // is a bank partner or government regulator (read-only admin)
define('B_CADMIN2', 28); // is a community sub-administrator
define('B_CADMIN', 29); // is a community administrator
define('B_ADMIN', 30); // is a system administrator -- can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('B_MAX', 30); // maximum bit number on testing machine
define('B_ADMINABLE', 'confirmed co member bona cAdmin cAdmin2 regulator nonudge closed'); // for cAdmin to set
define('B_LIST', 'member confirmed ok bona ided refill co u7 u8 nonudge nosearch u11 payroll weekly monthly u15 bysms paper secret u19 debt draws savings joined u24 u25 closed regulator cAdmin2 cAdmin admin');

// community-specific flags
define('B_CGC', 0); // ctty: is an official CGC (currently has no effect)
define('B_UP', 1); // ctty: is operational (up) (see also B_CTTYUP)
define('B_ADMIN_BITS', 'member:cgc,confirmed:up');

define('R_CTTY_BENE', 'cttyBuys+signups+rebates+bonuss+inflations+givebacks+grants+loans+invests+fines+tradeOut'); // benefit (ever) to the community (fines and givebacks are negative)

// Relationship permissions used in calls to r\Acct::can() -- stored (less B_RELATED) in r_relations
define('B_RELATED', 100); // has a relationship record
define('B_NONE', 100); // no access
define('B_READ', 101); // can view Transaction History
define('B_SCAN', 102); // can accept rCard payments for goods or cash for rCredits (or view Transaction History -- permissions are cummulative)
define('B_REFUND', 103); // can undo transactions and make refunds or trade cash at point of sale
define('B_SELL', 104); // can send invoices (or scan or view)
define('B_BUY', 105); // can make purchase for the account (or sell or scan or view)
define('B_MANAGE', 106); // can manage all aspects of the account
define('B_JOINT', 107); // make this a joint account (subsuming the other, with permission) -- no companies!
$GLOBALS['share permissions'] = array(t('no access'), t('view transactions'), t('accept rCard payments'), t('give refunds & give USD for rCredits'), t('send invoices'), t('make purchases'), t('manage account'), t('joint account'));
define('B_PERMISSIONS', 'related read scan refund sell buy manage joint');

// Other special pseudo-bit values for r\Acct::can() and rweb.module menus
define('B_MANAGING_CTTY', 120); // cadmin is actually managing a qualified account
define('B_PERSON', 121); // personal account (!co)
define('B_DEV', 122); // isDEV or is admin
define('B_u123', 123); //
define('B_CTTYUP', 124); // account's community is operating (see also B_UP)
define('B_BANK', 125); // account has a connected bank account
define('B_SHOW_ALL', 999); // special signal to r\Acct::can() to return all permissions

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
define('TX_BONUS', -2); // sales bonus (negative reward types are invisible on transaction history)
define('TX_REBATE', -1); // sales rebate
define('TX_TRANSFER', 0); // normal fund transfer (usually for actual goods and services) -- not creating rC
define('TX_SIGNUP', 1); // signup incentive reward
define('TX_GIVEBACK', 2); // gift to the community
define('TX_INFLATION', 3); // reward to conpensate for inflation
define('TX_HELPER', 4); // reward for inviting and assisting with account opening
define('TX_FINE', 9); // destroys rCredits (this is essentially a negative reward)
define('TX_REWARDS', '-2,-1,1,2,3,4,9'); // only these count as rewards

define('TX_XFEE', 10); // exchange fee (for buying rCredits with a credit/debit card or check)
define('TX_GRANT', 11);
define('TX_LOAN', 12);
define('TX_INVEST', 13); // equity investment (community purchase are TX_TRANSFER)
define('TX_TYPES', 'payment signup giveback inflation helper u5 u6 u7 u8 fine xfee grant loan investment');

define('TX_DEFAULT_PERIOD', 30); // number of days to show in transaction history, by default
define('TX_MAX_ROWS', 100); // maximum number of transactions to show on one screen
define('TX_DOWNLOAD_MONEY_FIELDS', 'frombank fromyou toyou reward net');
define('TX_DATA_FIELDS', 'rebate bonus undoneBy undoes disputed force changes isGift'); // serialized pseudo-fields in the data field of a transaction record

$GLOBALS['emailSubjects'] = array(
/*  'new-payment' => t('rCredits PAYMENT from {otherName}'),
  'new-charge' => t('rCredits CHARGE from {otherName}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {otherName}'),

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
  'confirm invite' => t('Confirm invitation to @otherName'),
  'not confirmed' => t('Your rCredits membership requires confirmation.'),
  'new invoice' => t('INVOICE from @otherName'),
  'account suspended' => t('Account SUSPENDED'),
  'account funded' => t('Account FUNDED'),
  'approved' => t('Account APPROVED!'),
  'join accounts' => t('Invitation to Create a Joint Account'),
  'force without perm' => t('Unauthorized transaction'),
);

if (@TESTING == 1) {
  global $specialSubs;
  foreach (explode(' ', B_PERMISSIONS) as $k) {
    $perm = constant('B_' . strtoupper($k));
    $specialSubs["%can_$k"] = htmlspecialchars($GLOBALS['share permissions'][$perm - B_RELATED]);
  }
  $specialSubs["%can_none"] = $GLOBALS['share permissions'][0];
  $specialSubs['%chk'] = '&#10004;';
}