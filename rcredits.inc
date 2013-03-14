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
use rCredits\Util as u;
use rCredits\Backend as be;

define('isDEV', @$_SERVER['WINDIR']); // developing on Windows (unlike production server)

define('R_VERSION', '1.0b'); // software version
define('R_SEPARATE_BACKEND', FALSE);
define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_MIN_PHOTO_WIDTH', 240); // how wide to make profile pictures (unless too long)
define('R_MAX_PHOTO_WIDTH', 1024);
define('R_FIRST_CTTY_UID', -1000001); // community uids are less than or equal to this (farther from zero)
define('R_DATA_FIELDS', ''); // fields stored in the account's "data" array field (serialized)
define('R_SECURE_FIELDS', 'type number pin auth card_code'); // non-personal encrypted fields
define('R_SECRET_FIELDS', 'federal_id dob id_proof answer'); // personal identifying information
define('R_SECRET_URL', isDEV ? 'http://localhost/rcredits' : 'http://rcredits.org'); // where to store secrets
define('R_URL_SLASH', '%_%'); // Drupal gets confused by slashes in a url, even if urlencoded
define('R_MAX_DAILY_EXCHANGE', 1000); // per Common Good Finance policy and to avoid formal regulation by FinCEN
define('R_INVITE_DAYS', 7); // number of days to accept invitation to membership
define('R_MINR_DEFAULT', 200); // suggested minimum amount to hold in rCredits
define('R_MAXR_DEFAULT', 0); // suggested maximum amount to hold in rCredits
define('R_MINUSD_DEFAULT', 200); // suggested minimum amount to hold in Dwolla
define('R_MAXUSD_DEFAULT', 0); // suggested maximum amount to hold in Dwolla
define('R_CARDCODE_LEN', 20); // length of rCard security code (long to deter easy copying)

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');
define('R_PICTURE_DIR', '/sites/default/files/pictures'); // precede this by DRUPAL_ROOT (file) or $base_url (url)
define('R_PROOF_DIR', '/sites/default/files/proofs');

// Account types
define('R_PERSONAL', 0);
define('R_SELF_EMPLOYED', 1);
define('R_COMMERCIAL', 2);
define('R_NONPROFIT', 3);
$GLOBALS['account types'] = array(t('personal'), t('personal AND commercial'), t('commercial'), t('non-profit or government'));

// Relationship permissions used in calls to acct::can() -- other than the first 5, these are not stored
define('PERM_RELATED', 0); // has a relationship record
define('PERM_READ', 1);
define('PERM_SELL', 2);
define('PERM_BUY', 3);
define('PERM_MANAGE', 4);
$GLOBALS['share permissions'] = array('no access', 'read transactions', 'sell', 'buy and sell', 'manage account');

define('PERM_RESERVED6', 6);
define('PERM_COMPANY', 7); // can act as a company (not a purely personal account)
define('PERM_CHARGE', 8); // can charge unilaterally
define('PERM_MEMBER', 10); // completed the rCredits requirements
//define('PERM_PARTNER', 11); // signed the membership agreement and donated
//define('PERM_STEWARD', 12); // is allowed to vote and is responsible for stewarding the Common Good Community
define('PERM_RTRADER', 14); // is allowed to use rCredits for real
$GLOBALS['permissions'] = 'related read sell buy manage - company charge - member partner steward - rtrader - - managing_ctty ctty_admin - admin';

define('PERM_MANAGING_CTTY', 17); // ctty_admin is actually managing a qualified account
define('PERM_CTTY_ADMIN', 18); // is authorized to manage own community
define('PERM_MANAGING', 19); // UNUSED admin or ctty_admin is actually managing a qualified account
define('PERM_ADMIN', 20); // can manage server, including assigning ctty admins, but cannot manage communities or individual accounts (except temporarily)
define('R_ADMIN_CAN_MANAGE_CTTYS', TRUE); // should be false once communities can manage themselves

// Permissions, options, and flags (flags field in account record) - max 2,147,483,647 = 2^31 - 1
// If we need more bits, create a flags2 field and make the BIT_ definitions negative for acct::hasBit and setBit (et -0x200)
define('BIT_MEMBER', 0x1); // signed the membership agreement, etc.
define('BIT_RTRADER', 0x2); // is allowed to use rCredits for real
define('BIT_UNUSED4', 0x4); // 
define('BIT_CHARGE', 0x8); // can charge customers without confirmation (treat this like a permission)

define('BIT_VIRTUAL_EMPLOYEES', 0x10); // automatically pay employees virtually --16
define('BIT_VIRTUAL_ALL', 0x20); // automatically pay everyone virtually
define('BIT_PAYROLL', 0x40); // automatically pay employees their total daily salary - employer chooses pay period)
define('BIT_NOTIFY_EMAIL', 0x80); // notifications by email

define('BIT_NOTIFY_SMS', 0x100); // notifications by SMS --256
define('BIT_ELECTRONIC', 0x200); // will accept electronic statements
//define('BIT_PROVED', 0x400); // proof of ID has been approved
//define('BIT_GOT_PROOF', 0x800); // proof of ID has been submitted

//define('BIT_SAW_VID2', 0x1000); // saw the pre-Steward vid --4096
define('BIT_SECRET_BAL', 0x2000); // don't let merchants tell me my balance on request
define('BIT_TO_BANK', 0x4000); // automatically transfer excess US Dollars from Dwolla to bank account
//define('BIT_MET', 0x8000); // has participated in a community meeting

//define('BIT_RQUALIFIED', 0x10000); // makes good economic circles --65,536 = 256 * 256

define('BIT_CTTY_ADMIN', 0x1000000); // is a community administrator --16,777,216 = 256 * 256 * 256
define('BIT_ADMIN', 0x2000000); // is a system administrator
define('BIT_CLOSED', 0x4000000); // account is closed (data[new_account] has new qid)
define('BIT_DEFAULTS', BIT_NOTIFY_EMAIL | BIT_TO_BANK); // what bits to set when creating a new account

// Transaction states
define('TX_STATES', 'deleted done pending disputed denied');

define('TX_DENIED', -2); // invoice rejected by the non-originator
define('TX_DELETED', -1);
define('TX_PENDING', 0); // charged, waiting for confirmation (in all other states, transaction is complete)
define('TX_DONE', 1); // transactions with a positive state count as done
define('TX_TENTATIVE', 2); // done in rCredits, but waiting for USD funds instead (amount minus r is the expected amount)
define('TX_DISPUTED', 3); // unilaterally charged, but disputed by the payer (the charge stands, until settled)

// Transaction channels (roughly in order of simplicity and generality of messages)
define('TX_SYSTEM', 0); // cron
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_SMART', 3); // smart phone or other computer-like device
define('TX_TOUCH', 4); // touch tone phone
define('TX_CHANNELS', 'System SMS Web App Touch');

// Transaction types
define('TX_SIGNUP', 0); // signup incentive reward
define('TX_TRANSFER', 1); // normal fund transfer (usually for actual goods and services)
define('TX_REBATE', 2);
define('TX_BONUS', 3);
define('TX_RESERVED4', 4); // not currently used
define('TX_AUTOMATIC', 4); // highest numbered automatic

define('TX_INFLATION', 5); // reward to conpensate for inflation
define('TX_GRANT', 6);
define('TX_LOAN', 7);
define('TX_RESERVED8', 8); // not currently used
define('TX_FINE', 9);
$GLOBALS['txTypes'] = array(
  t('signup'), t('transfer'), t('rebate'), t('bonus'), '', t('inflation'), t('grant'), t('loan'), '', t('fine'),
);
define('TX_DEFAULT_PERIOD', 30); // number of days to show in transaction history, by default

$GLOBALS['emailSubjects'] = array(
  'new-payment' => t('rCredits PAYMENT from {otherName}'),
  'new-charge' => t('rCredits CHARGE from {otherName}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {otherName}'),

  'new-invoice' => t('rCredits INVOICE from {otherName}'),
  'invoice-denied' => t('rCredits Invoice DENIED by {otherName}'),
  'invoice-canceled' => t('rCredits Invoice CANCELED by {otherName}'),

  'new-offer' => t('rCredits PAYMENT OFFER from {otherName}'),
  'offer-refused' => t('rCredits Payment REJECTED by {otherName}'),
  'offer-canceled' => t('rCredits Invoice CANCELED by {otherName}'),
  
  'welcome' => t('Welcome to rCredits!'),
  'password_reset' => t('New rCredits PASSWORD'),
  'tell_staff' => t('rCredits STAFF ALERT'),
);

include_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server
include_once __DIR__ . '/acct.class';
include_once __DIR__ . '/rcredits-circles.inc'; // treat this as an extension of this file, for testing

/**
 * Return the account's current stage in its progression toward rTrader
 *//*
function stage($acct = '', $numeric = FALSE) {  
  if (!$acct = ($acct ?: acct())) return FALSE;
  foreach (u\ray(R_STAGES) as $key => $one) if ($acct->can(constant('PERM_' . strtoupper($one)))) $stage = $numeric ? $key : $one;
  return @$stage;
}
*/

/**
 * Format and return args for reporting a transaction (old or new) to the current account.
 * Not all returned args are used for any particular type of transaction.
 * Note that templates and tests depend on the names of some of these fields
 * @param array $tx: the transaction array (including xid)
 * @param int $getBalance: 0=don't get any balances, 1=get my balance and tid, 2=get other party's balance and tid also
 * @param float $shortfall: how much short my balance is, for a transaction (if any)
 * @return assoc array of args
 */
function txReportArgs($tx, $getMyBalance = FALSE, $shortfall = 0) {
  extract($tx); // any or all standard transaction fields
  if ($data) if ($data = unserialize($data)) extract($data, EXTR_PREFIX_ALL, 'data');
  global $lastXid; $lastXid = $xid; // remember last transaction for current user, for potential undo (here and on undo)

  $cacct = acct();
  $toMe = ($payee == $cacct->id);
  $acct = acct($toMe ? $payer : $payee);
  
  $role = $toMe ? 'payee' : 'payer';
  list ($purpose, $otherPurpose) = u\order($toMe, $payee_for, $payer_for);
  list ($payerPurpose, $payeePurpose) = array($payer_for, $payee_for); // clearer names, for substitutions
  $tofrom = $toMe ? t('from') : t('to');
  $uid = $otherUid = ($toMe ? $payer : $payee);
  $action = $taking ? t('charged') : ($goods ? t('paid') : t('gave'));
  $confirmAction = $taking ? t('charge') : ($goods ? t('pay') : t('give'));
  $summary_word = $taking ? t('Charge') : t('Payment');
  $amount = u\formatAmount($amount);
  $short = u\formatAmount($shortfall);
  if (@$data_bonus) $rewardType = $taking ? t('bonus') : t('rebate');
  $rewardAmount = u\formatAmount($toMe ? @$data_bonus : @$data_rebate);
  $otherName = userField('full_name', @$data_foreign_uid ?: $otherUid);
  $created = u\formatDate($created);

  if ($getMyBalance) {
    $tid = be\xid2tid($xid);
    if ($cacct->can(PERM_BUY)) $balance = be\creditInfo()->fancy['balance'];
  }
//  $otherTid = be\xid2tid($xid, $otherUid);
  $summary = "$summary_word of $amount $tofrom $otherName for \"$purpose\" on $created";
  $success = TRUE;
  return array_merge($tx, compact(u\ray('created xid action role payerPurpose payeePurpose purpose otherPurpose toMe confirmAction amount tofrom rewardType rewardAmount balance tid otherUid otherName short summary success')));
}

/**
 * Return the percentage tithe chosen by given user (as identified by their email).
 * Consider the tithe valid only if it was specified as part of a COMPLETED contribution.
 */
function tithe($acct) {
  if (!$tithe = $acct->tithe + 0) {
    $tithe = file_get_contents("http://rcredits.org/inc/tithe.php/$acct->mail");
    if (is_numeric($tithe)) $acct->update(compact('tithe'));
  }
  return $tithe + 0;
}

function otherBalance($uid) {
  $cacct = acct();
  $acct = acct($uid);
  if ($acct and $cacct->can(PERM_CHARGE) and !$acct->hasBit(BIT_SECRET_BAL)) {
    return be\creditInfo(compact('uid'))->fancy['balance'];
  } else return FALSE;
}

function exchange($amount, $payer, $payee, $for) {
  $taking = FALSE;
  r\transact(compact(u\ray('amount payer payee for')));
}

/**
 * Record a transaction between two participants
 * @see also transactionSetup(), r\transactionRewards(), and be\transfer()
 */
function transact($info) {
  $txs = array(transactionSetup($info));
  transactionRewards($txs);
  return txSave($txs);
}

/**
 * Save the transaction set
 */
function txSave($txs) {
  extract(u\just('payer payee data', $txs[0]));
  $DBTX = db_transaction();
//  $table = zxs($payer, $payee);
  $data = @unserialize($data) ?: array();
  foreach ($txs as $key => $one) {
    if (empty($one)) continue; // skipping rebate or bonus
    $txs[$key]['serial'] = @$serial; // set serial for dependent transactions
    \drupal_write_record('r_txs', $txs[$key]); // pass actual array element, to get xid
    if ($key == 0) {
      $txs[$key]['serial'] = $serial = $txs[0]['xid']; // set serial for main transaction
      dbQ("UPDATE r_txs SET serial=:serial WHERE xid=:serial", compact('serial'));
    }
  }

  cacheTotals($payer, $txs);
  cacheTotals($payee, $txs);

  unset($DBTX); // commit (commit)
  return $txs;
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 */
function transactionRewards(&$txs) {
  extract(u\just('type goods payer payee data state', $txs[0]));
  
  if ($type != TX_TRANSFER) return;
  if (!$goods) return; // quit unless rebates and bonuses apply
  if (owning($payer, $payee)) return; // no rewards for transactions between owner and owned
  $rewards = unserialize($data);
  
  $txs[] = txRewardSetup(TX_REBATE, $payer, $state, $rewards);
  $txs[] = txRewardSetup(TX_BONUS, $payee, $state, $rewards);
}

function txRewardSetup($type, $payee, $state, $rewards) {
  $payer = $payer_agent = $payee_agent = userField('community', $payee);
  if ($payer == $payee) return NULL; // payee is a community, so no rewards

  $taking = FALSE;
  $goods = FALSE;
  $r = $amount = $rewards[$type == TX_REBATE ? 'rebate' : 'bonus'];
  $for = $type == TX_REBATE ? t('rebate') : t('bonus');
  return transactionSetup(compact(u\ray('type amount r goods payer payee payer_agent payee_agent for taking state')));
}

/**
 * Create a new transaction and set up all the important fields.
 * @param array $info: an associative array of transaction info. The following elements are required:
 *   amount: how much to transfer
 *   payer, payee: uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   taking (bool): did the payee initiate the transaction
 * @return the array, with some extra field information added
 */
function transactionSetup($info) {
  global $channel; // through what medium was the transaction requested (web, SMS, etc.).
  extract($info, EXTR_PREFIX_ALL, 'my');
  $required_params = compact(u\prefixValues('my_', u\ray('amount payer payee for taking')));
  u\EXPECT($required_params, 'float zid zid notempty bool');

  setDft($info['type'], TX_TRANSFER);
  setDft($info['r'], $my_amount);
  setDft($info['state'], TX_DONE);
  setDft($info['goods'], FALSE);
  setDft($info['payer_agent'], $my_payer);
  setDft($info['payee_agent'], $my_payee);

  $created = REQUEST_TIME;
  if (@$my_state != TX_PENDING) $completed = $created;
  $serial = NULL; // always set equal to xid of main transaction after saving
  
  list ($payer_for, $payee_for) = is_array($my_for) ? $my_for : array($my_for, $my_for);
  unset($info['for']);
  if ($info['type'] == TX_TRANSFER) {
    $payer_tid = xid2tid($xid, $info['my_payer']);
    $payee_tid = xid2tid($xid, $info['my_payee']);
  }

  return array_merge($info, compact(u\ray('created completed serial payer_for payee_for channel')));
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 * @param integer $who: uid of the user
 * @param numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($uid, $demand = FALSE) {
  $acct = acct($uid);
  if ($demand === FALSE) return $acct->demand;
  $demand = min(R_MAX_AMOUNT, $demand);
  $acct->update(compact('demand')); 
  return $demand;
}

/**
 * Return the amount the given account has available to pay.
 * @param acct $acct: the account
 * @param bool $goods: is the request for real goods and services (or cash)
 */
function available($acct, $goods) {
  $data = $acct->data;
  return $data['available'] - ($goods ? 0 : $data['rewards']);
}

/**
 * Keep track of balance, available balance, and other figures in data field of account record:
 *    balance: cached current rCredits account balance (sum of transactions is authoritative)
 *    available: the amount available to spend
 *    rewards: incentive rewards received to date (cannot be cashed out)
 *    frozen: how much of the balance is unavailable to spend (array of amounts keyed by dates)
 *    noBonus: how many rCredits must the account accept as payment for goods and services, before receiving more sales bonuses
 */
function cacheTotals($uid, $txs) {
  if ($uid < 0) return; // don't cache community and region totals
  if ($txs[0]['state'] <= TX_PENDING) return; // don't change totals if transaction doesn't count (yet)
  $acct = acct($uid);
  $data = $acct->data;
//  extract(u\just('balance rewards frozen', $data));
  $balance = @$data['balance'] ?: 0;
  $rewards = @$data['rewards'] ?: 0;
  $frozen = @$data['frozen'] ?: array();
  
  foreach ($txs as $tx) {
    extract(u\just('payer payee r type goods', $tx));
    $amount = $payer == $uid ? -$r : ($payee == $uid ? $r : 0);
    $balance += $amount;
    if ($type != TX_TRANSFER) {
      $rewards += $amount;
    } elseif (!$goods) $frozen[$wait] = @$frozen[$wait] + $amount;
  }
  $available = $balance - array_sum($frozen);
  $data = compact(u\ray('balance rewards frozen available')) + $data;
  $acct->update(compact('data'));
}

/**
 * Return how much of a transaction will be in USD.
 * @param float $shortfall (RETURNED) how much too little the payer has
 * @return float: how much of the transaction will be in USD (NULL if no USD account)
 */
function usdPart($requestAmount, $acct, $preapproved, $goods, &$shortfall) {
  $shortfall = 0; // no shortfall if preapproved
  if (!$preapproved) return 0; // if just an invoice, funds don't have to be there yet (no shortfall, no usd)
  
  $uid = $acct->id;
//  $available = be\creditInfo(compact('uid'))->available;
  $r = min($requestAmount, r\available($acct, $goods));
  if (!$shortfall = max(0, $requestAmount - $r)) return 0;

  $usdAcct = new usd($acct); // try to do some in USD
  $usdBalance = $usdAcct->bal();
  $shortfall = max(0, round($shortfall - $usdBalance, 2));
//    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
//    $shortfall = ceil($shortfall); // shortfall should be a round figure (no cents)
  return round($requestAmount - $r - $shortfall, 2);
}

/**
 * Pay each employee the amount
 */
 /*
function payEmployees($amount0, $for, $confirmed) {
  $employees = employees();
  if (!$employeeCount = count($employees)) return array('no employees', array(), array());
  if ($shortfall = shortfall(acct()->id, $amount0 * $employeeCount, TRUE, isRTrader())) {
     $short = u\formatAmount($shortfall);
     return array('short payment', compact('short'), FALSE); // no partial payment of employees
  }
  $total = u\formatAmount($amount0 * $employeeCount);
  $amount = u\formatAmount($amount0);
  if (!$confirmed) return array('pay employees', compact(u\ray('employeeCount amount total')), TRUE);
  $pay_all = TRUE;
  $results = array();
  $successes = 0;
  foreach ($employees as $acct) {
    $results[] = list ($message, $args, $confirm) = be\transfer('payment', $acct, $amount0, $for, !u\forCash($for), TRUE, compact('pay_all'));
    if ($args['success']) $successes++;
  }
  $success = ($successes > 0);
  return array('paid employees', compact(u\ray('total successes employeeCount results success')), FALSE);
}
*/

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $info: substitutions for the message
 */
function notify($uid, $index, $info) {
  global $channel;
  $cacct = acct(); // careful: current account is empty if called from formRegister_submit
  $acct = acct($uid);
  $info['uid'] = $uid;
  $fullName = $acct->full_name;
  $warnIndexes = u\ray('new-invoice new-offer charge-disputed');
  
  if ($uid != @$cacct->id) $info['otherName'] = @$cacct->full_name; // this comes in backwards if notifying elseone

  if (!strlen($acct->pass)) {
    $warning = u\tt('no asif notify', compact('fullName'));
  } else {
    if ($acct->hasBit(BIT_NOTIFY_EMAIL) and @$acct->mail) {
      $mailed = \drupal_mail('rCredits', $index, $acct->mail, '', $info); // no need to pass $acct->language
    }
    if ($acct->hasBit(BIT_NOTIFY_SMS)) {
      $number = dbLookup('number', 'r_sms', 'uid=:uid AND status=:SMS_PRIMARY', compact('uid'));
      if ($number) $smsed = SMS\send($number, u\tt($index, $info, TX_SMS));
    }
    if (!@$mailed and !@$smsed and array_search($index, $warnIndexes) !== FALSE) {
      $notification = str_replace('-', ' ', $index);
      $warning = u\tt('cannot contact', compact('fullName', 'notification'));
    }
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) { } // dunno how to handle this (send a second message?)
    if ($channel == TX_SMART) { } // dunno how to handle this, but easier than SMS
  }
}

/**
 * If appropriate, update an account's membership status and send the staff an alert.
 * @param acct $acct: the account
 * @param string $status: what milestone or event to alert about
 * @param bool $do: prerequisite for the alert
 * If there is an appropriate bit to set, it will be set as appropriate.
 */
function membershipEvent($acct, $status, $do = TRUE) {
  $bit = @constant('BIT_' . strtoupper($status));
  if ($do and (!$bit or !$acct->hasBit($bit))) { // met the prerequisite and hasn't previously
    if ($bit) $acct->setBit($bit);
    if ($status == 'member') be\fund($acct->id, TX_SIGNUP);
    $fullName = $acct->full_name;
    $quid = $acct->mainQid;
    notifyStaff('Account @fullName (@quid) is now @status.', compact(u\ray('fullName quid status')));
  }
}

/**
 * Notify rCredits staff
 * @param string $message: the message or message index
 * @param assoc $subs: substitutions for the message
 * @param int $uid: account id (only needed when called from register, defaults to current account id)
 */
function notifyStaff($message, $subs = array(), $uid = '') {
  $noFrame = TRUE;
  $message = u\tt($message, $subs);
  if (!$uid) $uid = acct()->id ?: 1; // use Site Admin account if no other
  $mailed = \drupal_mail('rCredits', 'tell_staff', R_ORG_EMAIL, '', compact(u\ray('message noFrame uid')));
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
  if ($phone) return R_TEMP_NAME_PREFIX . u\formatPhone($phone, 'n');
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
 *     (prefixes added to the payer_for or payee_for fields when changing state will not be included in changes)
 */
function setTxField($fieldName, $newValue, $xid, $allRelated = FALSE) {
  $cuid = acct()->id;
  u\EXPECT(compact(u\ray('fieldName xid allRelated')), 'name zid bool');
  
  $where = "xid=:xid and $cuid in (payer, payee)"; // "$cuid in" is a security measure
  list ($oldValue, $oldState, $data) = array_values(dbLookup("$fieldName AS f1,state,data", 'r_txs', $where, compact(u\ray('xid')))); // f1 prevent Drupal from combining state results when $fieldName is 'state'
  $data = unserialize($data);
  $data['changes'][] = array(REQUEST_TIME, acct()->agentId, $fieldName, $oldValue);
  $data = serialize($data);
  $newState = $fieldName == 'state' ? $newValue : $oldState;
  $completion = (@$newState != TX_PENDING and !@$oldState) ? ',completed=:REQUEST_TIME' : ''; // update completion time
  $sql = "UPDATE r_txs SET $fieldName=:newValue,data=:data$completion WHERE $where";
  $sql2 = "UPDATE r_txs SET $fieldName=:newValue$completion WHERE serial=:xid AND type<>:TX_TRANSFER";
//  $sql2 = str_replace(', data=:data', '', str_replace('xid=:xid', 'serial=:xid', $sql));
  
  $DBTX = \db_transaction();
  dbQ($sql, compact(u\ray('newValue data xid')));
  if ($allRelated) dbQ($sql2, compact(u\ray('newValue xid'))); // gotta do the auxiliary records separately so as not to affect their data field (separate query, to avoid "unbuffered query" error -- known Drupal bug)
  unset($DBTX); // commit
}

/**
 * Merge a temporary ("old") account into the current account (a permanent ("new") account).
 * @return 1 (for counting how many were merged)
 */
function mergeAccounts($oldUid) {
  $newUid = acct()->id;
  if (!be\isTempAccount($oldUid)) return; // don't merge non-temp account
//  u\EXPECT(be\access('manage account'), "unauthorized mergeAccounts: $newUid into $oldUid");
//  if (!isTempName(userField('name', $oldUid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = acct($newUid);
  $old = acct($oldUid);
  $update = array();
  foreach (u\ray('phone fax website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  acct()->update($update);
  newMemberId($oldUid, $newUid, TRUE);
  dbQ('DELETE FROM users WHERE uid=:oldUid', compact('oldUid')); // must be last, for referential integrity
  return 1;
//  dbQ("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:oldUid", compact('oldUid'));
}

/**
 * Return the rough location of the entity with the given account (city, state, non-US country).
 */
function location($acct, $complete = FALSE) {
  $country = realCountry($acct->country);
  $state = realState($acct->state, $acct->country);
  $result = array();
  if ($complete) $result[] = (string) $acct->address; // (string) in case address is NULL
  $result[] = $acct->city;
  $result[] = $state . (($complete and $acct->postal_code) ? ' ' . $acct->postal_code : '');
  if ($acct->country != R_COUNTRY_ID) $result[] = strtoupper($country);
  foreach ($result as $key => $value) if ($value === '') unset($result[$key]);
  return join(', ', $result);
}

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
//  $ctty_uid = serverUid();
  $sqls = array(
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE r_sms',
//    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
  \rebuild_menus();
  drupal_goto('devel/cache/clear?destination=node');
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function dbLookup($fieldName, $table, $where = '1', $subs = array()) {
  u\EXPECT(compact(u\ray('fieldName table where subs')), 'notempty notempty notempty assoc');
  if (!strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $sql = "SELECT $fieldName FROM $table WHERE $where";
  $result = dbQ($sql, $subs);
  $fieldCount = substr_count($fieldName, ',') + 1;
  $result = $fieldCount > 1 ? $result->fetchAssoc() : $result->fetchField();
  u\EXPECT($fieldCount == 1 or !$result or count($result) == $fieldCount); // count is off, eg, if $fieldName == 'a,a'
  return $result;
}

/**
 * Return a qid for the zid
 * @param $zid: a zid
 * @param string $regionId: the 3-character regional server ID (defaults to this server)
 * @param int $min_len: minimum length of the part of the qid after the "." or ":"
 * @return the qid, FALSE if wrong format
 */
function qid($zid = '', $regionId = R_SERVER_ID, $min_len = 1) {
  if (!u\isZid($zid)) return FALSE;
  return $regionId . ($zid < 0 ? (':' . u\n2a(-$zid, -$min_len)) : ('.' . u\n2a($zid, -$min_len)));
}

/**
 * Return a qid without the 3-char header
 */
function qid2($zid, $regionId = R_SERVER_ID) {return substr(qid($zid, $regionId, 3), strlen($regionId));}

/**
 * Return the short qid and full name of the specified account.
 * For example, return ".AAC Abe One".
 */
function definiteName($uid) {
  if ($uid <= 0) return '';
  $acct = acct($uid);
  return qid2($uid) . ' ' . $acct->full_name;
}

/**
 * Return a zid and region for the qid
 * @return [zid, regionUid]
 */
function unQid($qid) {
  if (!u\isQid($qid)) return FALSE;
  list ($regionId, $type, $iid) = preg_split('/(\.+|=|:)/', $qid, 0, PREG_SPLIT_DELIM_CAPTURE); // allow a horizontal colon
  
  $id = u\a2n($iid);
  return array($type == '.' ? $id : -$id, serverUid($regionId));
}
  
/**
 * Convert uids to quid
 * Call by:
 *   quid()
 *   quid(uid) == quid(uid, uid)
 *   quid(uid, agentUid)
 *   quid(uid, agentUid, regionUid)
 * @param int uid: record id of the account (defaults to current account)
 * @param int agentUid: uid of agent (defaults to uid)
 * @param int $regionUid: uid of the region (always negative, defaults to current region)
 * @return the corresponding quid
 */
function quid($uid = '', $agentUid = '', $regionUid = '') {
  $cacct = acct();
  if ($agentUid) {
    if (!$regionUid) $regionUid = serverUid();
  } else list ($uid, $agentUid, $regionUid) = 
    $uid ? array($uid, $uid, serverUid()) 
         : array($cacct->id, $cacct->agentId, $cacct->regionId);
  return qid($uid == $agentUid ? $uid : -relation('reid', $uid, $agentUid), u\n2a(-$regionUid, 3), 3);
}

function qxid($xid, $regionId = R_SERVER_ID) {return qid($xid, $regionId = R_SERVER_ID, 4);} // Convert xid to qxid

/**
 * Interpret transaction record fields as an acct object
 * @param int/array $id: the account id or [id, agent, data]
 * @param int $agent: account id of the transaction agent
 * @param assoc $data: extra info from the transaction, including info about foreign parties to the transaction
 * @return acct
 */
function tx2acct($id, $agent = '', $data = '') {
  if (is_array($id)) list ($id, $agent, $data) = $id;
  if (is_array($data)) extract($data); // foreign_uid and/or foreign_agent (or neither)
  return acct(@$foreign_uid ? qid($foreign_uid, $id) : $id, @$foreign_agent ? qid($foreign_agent, $agent) : $agent);
}

/**
 * Determine whether the record can be undone
 * @param assoc $oldRecord: the transaction record or NULL (to make it easy when last_tx found no matching transaction)
 * @param assoc $subs: (RETURNED) necessary substitutions, if any
 * @return string: the error message (FALSE if the current agent CAN undo it on behalf of the current account)
 */
function nonoUndo($oldRecord, &$subs = array()) {
  if (!@$oldRecord) return 'undo no match';
  $cacct = acct();
  extract($oldRecord, EXTR_PREFIX_ALL, 'old');
  $data = unserialize($old_data);
  if (@$data['undoneBy']) {
    $subs = array($old_tid => xid2tid($old_xid));
    return 'already undone';
  }

  $acct = tx2acct($old_toMe ? array($old_payee, $old_payee_agent, $data) : array($old_payer, $old_payer_agent, $data));
  if ($cacct->agentId != $acct->agentId) {
    $perm = $old_taking ? PERM_BUY : PERM_SELL;
    if (!acct($acct->id, $cacct->agentId)->can($perm)) return $old_taking ? 'no buy' : 'no sell';
  }
  return FALSE;
}

/**
 * Create or update a reverse relation record for the given main and other
 * if any of the given fields is positive
 * @param int $other: the main (switching to agent)
 * @param int $main: the other (switching to main)
 * @param assoc $data: (byRef) array of field values
 *   Returned with all fields listed in $fields unset (if any of those fields has a positive value)
 * @param string $fields: space-delimited list of fields to check
 * @return (none)
 */
function reverseRelations($other, $main, &$data, $fields) {
  foreach (u\ray($fields) as $field) if (@$data[$field]) $stay = TRUE;
  if (!@$stay) return;
  $subs = compact('main', 'other') + u\just($fields, $data);
  if ($reid = relation('reid', $main, $other)) $subs += compact('reid');
  \drupal_write_record('r_relations', $subs, $reid ? 'reid' : array());
  foreach (u\ray($fields) as $field) unset($data[$field]); // don't get these fields mixed in with the reverse record
}

/**
 * Say whether one of the accounts owns the other.
 */
function owning($uid1, $uid2) {
  $where = 'is_owner AND ((main=:uid1 AND other=:uid2) OR (main=:uid2 AND other=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second
 * @param int $company: defaults to current uid
 */
function isEmployee($uid, $company = '') {
  if (!$company) $company = acct()->id;
  return relation(1, $company, $uid, 'employer_ok AND employee_ok');
}

function communityUid($postal_code = '') {
  if (!$postal_code) return serverUid();
  $where = "country=':R_COUNTRY_ID' AND :postal_code RLIKE postal_code AND uid=community";
  return userField('uid', $where, compact(u\ray('postal_code'))) ?: serverUid();
}

function isForeign($uid) {
  $name = userField('name', $uid);
  return $name == (R_SERVER_ID . '.') ? FALSE : ($uid < 0 and preg_match('/[A-Z]{3}\./', $name));
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($full_name) {
  $name = $name0 = u\shortName($full_name);
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
  if ($acct = acct(strtoupper($id))) return $acct->id; // try to interpret it as a qid
  return userField('uid', 'name=:id OR mail=:id', compact('id'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $cuid = acct()->id;
  $result = dbQ('SELECT other, foreign_uid FROM r_relations WHERE main=:cuid AND employer_ok AND employee_ok', compact('cuid'));
  $accts = array();
  foreach ($result as $key => $one) {
    extract((array) $one); // other, foreign_uid
    $accts[] = $other > 0 ? acct($other) : acct(qid($foreign_uid, $other));
  }
  return $accts;
}

/**
 * Return the field or fields from the relations table, for the given id and agent.
 * Call by:
 *   relation(field, id, agent, where, subs) OR
 *   relation(field, where, subs)
 */
function relation($field, $id, $agent = array(), $where = '1', $subs = array()) {
  if (is_numeric($id)) {
     $where = "main=:id AND other=:agent AND ($where)";
     $subs += compact('id', 'agent');
  } else list ($where, $subs) = array($id, $agent); // called with alternate form
  return dbLookup($field, 'r_relations', $where, $subs);
}

/**
 * Return our standard encryption password and a salt for the given account.
 */
function passSalt($acct = '') {
  $acct = $acct ?: r\acct();
  require(DRUPAL_ROOT . '/sites/default/settings.php');
  $pass = $databases['default']['default']['password'];
  $salt = $acct->created;
  return compact('pass', 'salt');
}

/**
 * Assign the given account a new id.
 * This is safe only if run in Maintenance mode or if there is a gap in the used uids.
 * For an ugly QUID, a replacement record will need to be created for the old uid, and marked unusable
 * @param bool $skipUsers: don't change the actual users table record 
 *   called from mergeAccounts, where the old record is to be deleted and the new already exists
 * @return the new uid
 */
function newMemberId($oldUid, $newUid = FALSE, $skipUsers = FALSE) {
  $where = 'u.uid>0 AND u2.uid IS NULL ORDER BY u.uid';
  if (!$newUid) $newUid = dbLookup('u.uid', 'users u LEFT JOIN users u2 ON u2.uid=(u.uid+1)', $where) + 1;
  $subs = array( // all tables are listed, to be sure we don't leave any out
  // r_areas
    'r_candidates:uid', // in case we're in the middle of figuring who's next
  // r_countries,
    'r_demand:who',
  // r_industries
    'r_invites:inviter',
    'r_invites:invitee',
    'r_log:cuid',
    'r_log:agent',
  // r_nonces
    'r_proxies:person',
    'r_proxies:proxy',
  // r_regions
    'r_relations:main',
    'r_relations:other',
  // r_scores
    'r_smarts:owner',
    'r_smarts:default_agent',
    'r_sms:uid',
  // r_states
    'r_txs:payer',
    'r_txs:payee',
    'r_user_industries:uid',
    'users:uid',
  );

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field) = explode(':', $one);
    if ($skipUsers and $table == 'users') {
/*      $cols = dbQ("SELECT column_name FROM information_schema.columns WHERE table_name='users'")->fetchCol();
      $map = array(',uid=DEFAULT,' => ',', ',mail=DEFAULT' => ',mail=uid', ',name=DEFAULT' => ",name='UNUSABLE$oldUid'");
      $updates = strtr(',' . join('=DEFAULT,', $cols) . '=DEFAULT,', $map);
      $updates = str_replace(',name=DEFAULT', ",name='UNUSABLE$oldUid'", $updates); // mystery why the strtr skips this (???!)
      $updates = substr($updates, 1, strlen($updates) - 2); // remove surrounding commas
      dbQ("UPDATE $table SET $updates WHERE uid=:oldUid", compact('oldUid'));
*/
    } else dbQ("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }
  unset($DBTX); // commit
  return $newUid;
}

/**
 * Record an invitation
 * @param string $email: email address to which the invite was sent
 * @param int $inviter: the inviter's account uid
 * @param string $code: proof of the invitation (that invitee must have, in order to sign up) (DEFAULT random)
 * @return string: the code
 */
function invite($email, $inviter, $code = '') {
  $info = array(
    'code' => $code ?: \user_password(20),
    'email' => $email,
    'inviter' => $inviter,
    'invited' => REQUEST_TIME,
  );
  \drupal_write_record('r_invites', $info);
  return $info['code'];
}

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out. (can't use Header() here)
 */
function go($page) {
  if (\testing()) {
    list ($form, $arg1) = ($page == 'pay' or $page == 'charge') ? array('tx', $page) : array($page, '');
    $form = function_exists("\\rCredits\\Web\\form$form") ? $form 
      : ($form = 'user/register' ? 'register'
      : "\\$form");
    return Web\showForm($form, $arg1);
  }
  \drupal_goto($page);
}

/**
 * Set or return the proxy for the person
 */
function proxy($person, $priority = 1, $proxy = NULL) {
  if (!@$proxy) return dbLookup('proxy', 'r_proxies', 'person=:person AND priority=:priority', compact('person', 'priority'));
  dbQ('DELETE FROM r_proxies WHERE person=:person AND priority=:priority', compact('person', 'priority'));
  $info = compact(u\ray('person priority proxy'));
  \drupal_write_record('r_proxies', $info);
}

/**
 * @param mixed $which: criteria or record id (defaults to $cuid if users)
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
  u\EXPECT(is_object($result[0]), 'bad object array in tableField(): ' . print_r($result, 1)); // keep this
  return strpos($field, ',') ? ((array) $result[0]) : $result[0]->$field;
}

/**
 * Do a database query with easy substitutions.
 */
function dbQ($sql, $subs = array(), $die = FALSE) {
  u\EXPECT(compact(u\ray('sql subs')), 'notempty assoc');
//  debug(u\getConstants($sql));
//  debug($subs);
//  if ($die)
 //if (@$subs['info']) {print_r(u\getConstants($sql) . ' ' . print_r(u\prefixKeys(':', $subs), 1)); die();}
//if (strpos($sql, 'PDATE') and !@$subs['flags']) {print_r(compact('sql','subs','key'));} 
  //if (!strpos($sql, 'PDATE ')) 
  return \db_query(u\getConstants($sql), u\prefixKeys(':', $subs));
/*
  global $databases, $db;
  extract($databases['default']['default']);
  $db = new \PDO("$driver:host=$host;port=$port;dbname=$database", $username, $password);
  return query(u\getConstants($sql), u\prefixKeys(':', $subs));
  */
}

function query($sql, $subs = array()) {
  global $db;
  $q = $db->prepare($sql);
  $q->execute($subs);
  return $q;
//  return substr($sql, 0, 7) == 'SELECT ' ? $q : $q->rowCount();
}

/**
 * Update a database record 
 * (replaces half the functionality of \drupal_write_record, which does not update bigint fields properly)
 * @param string $table: table name
 * @param assoc $info: data to update
 * @param string $key: record key name
 * @return: number of rows affected
 */
function dbUpdate($table, $info, $key) {
  $sql = '';
  foreach ($info as $field => $value) if ($field != $key) $sql .= ",$field=:$field";
  $sql = "UPDATE $table SET " . substr($sql, 1) . " WHERE $key=:$key";
  $result = dbQ($sql, $info);
  return $result->rowCount();
}
 
function error($logid) {
  $info = dbLookup('info', 'r_log', 'logid=:logid', compact('logid'));
  if ($info) $info = json_decode(utf8_encode($info)); else $info = 'unknown error';
  print_r($info); // keep this for now
//  drupal_set_message(print_r($info, 1), 'error');
//  drupal_goto('');
}

// Single line functions that need no explanation
function setDft(&$param, $value) {if (!isset($param)) $param = $value;}
function userField($field, $who = '', $subs = array()) {return tableField($field, 'users', $who, $subs);}
function rebate($uid, $amount = 1) {return round($amount * userField('rebate_percent', $uid) / 100, 2);}
function bonus($uid, $amount = 1) {return rebate($uid, 2 * $amount);}
function txField($field, $which, $subs = array()) {return tableField($field, 'r_txs', $which, $subs);}
function setTxState($newValue, $zserial) {setTxField('state', $newValue, $zserial, TRUE);}
function word() {return file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . "/.shibboleth");}
function totalDemand($ctty = '') {return dbLookup('SUM(demand)', 'users', $ctty ? 'community=:ctty' : '1', compact('ctty'));}
function balance($uid = '') {return be\creditInfo(compact('uid'))->balance;}
//function serverUid($regionId = '') {return -u\a2n(u\neq($regionId, CT_SERVER_ID) ?: R_SERVER_ID);}
function serverUid($regionId = R_SERVER_ID) {return -u\a2n($regionId);}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('{BODY}', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function isRTrader($uid = '') {return ($acct = acct($uid)) ? $acct->hasBit(BIT_RTRADER) : FALSE;}
function realCountry($country) {return dbLookup('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return dbLookup($country == R_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function makeQueue($callback, $time = 15) {return array('worker callback' => "rCredits\\$callback", 'time' => $time);}
function enqueue($queue, $item) {\DrupalQueue::get($queue)->createItem($item);}
function dbExists($table, $where = '1', $subs = array()) {return dbLookup(1, $table, $where, $subs);}