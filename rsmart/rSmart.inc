<?php
namespace rCredits\Smart;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Backend as be;
use rCredits\Testing as t;

/**
 * @file
 * Smart phone interface to the rCredits system.
 *
 * Coding conventions:
 *   $mya is the current account
 *   $a is some other account
 *
 * RPC API Definition:
 *
 * Basic request params (always sent):
 * op       the requested operation
 * device   the device ID (supplied once by the server) -- empty if none
 * agent    the already-scanned agent's ID (eg NEW:AAB) or company ID (eg NEW.AAB) -- empty if none yet
 * version  the version number of the running app
 *
 * REQUESTS:
 * identify:	[member, code] 
 *    => [ok, message, logon="1", name, descriptions[], can, device, default, company, update]
 *    => [ok, message, logon="0", name, place, company, balance]
 * photo:	[member] => photo
 * charge:	[member, amount, goods, description] => [ok, message, tx, balance, undo]
 * undo:	[tx] => [ok, message, balance]
 *
 * To create json for app demo, do this from PHP window on STAGE server:
   include __DIR__ . '/../rcredits/rsmart/rsmart.inc';
   $args = rCredits\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', 'NEW.AAB', RPOS_VERSION, 'NEW.ABB', 'ZzhWMCq0zcBowqw'); // cust
   //$args = rCredits\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', '', RPOS_VERSION, 'NEW:AAB', 'WeHlioM5JZv1O9G'); // agent login
   //$args = rCredits\Util\ray('op device agent version member amount goods description', 'charge', '3i2ifsapEjwev3CwBCV7', 'NEW:AAB', RPOS_VERSION, 'NEW.ABB', AMOUNT, 1, 'zot');
   //$args = rCredits\Util\ray('op device agent version tx', 'undo', '3i2ifsapEjwev3CwBCV7', 'NEW:AAB', RPOS_VERSION, '4069');
///debug(rCredits\Smart\pos($args));
 */

define('RPOS_VERSION', 102); // lastest POS app version available for download (this should match the versionCode in the app's manifest. upload and test the app first, before bumping the number up here.)
//define('RPOS_FILENAME', 'http://ws.rcredits.org/rPOS-' . rPOS_VERSION . '.apk');
define('RPOS_FILENAME', 'http://rc4.me/rpos.apk');
define('RPOS_FILENAMEB', 'http://rc4.me/rposb.apk'); // version for buggy front-camera-only machines
define('RPOS_FILENAME_DEBUG', 'http://rc4.me/rPOS-debug-unaligned.apk');
define('R_QR_HEADER', 'HTTP://RC2.ME/I/'); // for transient QR IDs
define('R_TRANSIENCE', 60); // number of seconds for device's transient QR to be valid

$GLOBALS[TX_SMART . ' messages'] = array(
  'bad member' => t('That rCard is not valid.'),
  'bad agent' => t('That is not a valid agent rCard.'),
  'agent cannot scan' => t('That agent is not authorized to charge customers.'),
  'scan in first' => t('You have to scan yourself in before charging a customer.'),
  'already scanned in' => t('You are already scanned in.'),
  'company inactive' => t('That company is not an active participant.'),
  'company inactive' => t('That company is not an active participant.'),
//  'agent cannot sell' => t('That agent is not authorized to charge customers.'),
  'customer balance' => t("Customer: @name\n\nBalance: @balance@spendable\nTradable for cash: @cashable"),
  'report exchange' => t("You @did @otherName @amount in exchange for cash."),
  'just did' => t("\n\nWe just @did @amount@forCash."),
  'unknown device' => t('This device is not recognized. You must scan yourself in (at least once) before charging a customer.'),
);

/**
 * Receive and process a REST request from a smart device
 * @param assoc $args: an associative array (defaults $_POST) of a json-encoded assoc that normally includes:
 *   op: what to do (determines what function to call)
 * @return (normally by calling either be\error or doOp):
 *   ok: 1 if successful, otherwise 0 ($op = 'photo' is an exception; see below)
 *   message: an error message or help message to display on the device; or not set
 */
function pos($args = FALSE) {
  global $channel; $channel = TX_SMART;
  global $scanned; $scanned = true;
  global $mya;
  
  if (!r\up()) return posErr('system down');
  if (!$args) $args = $_POST;
  u\loga('in', $args);
  extract(u\just('op device agent member', $args));
  if (!$op or !in_array($op, u\ray('identify photo charge payment undo'))) return posErr('bad op');

  if (!$mya = getAgent($agent ?: $member, $err)) return posErr($err); // set "current" account for internals
  if (!$mya->cttyUp) return posErr('your community down', u\ray('contact', $mya->cAdminContact()));

  if (@$device) { // device was specified, substitute its record id, herein
    if ($deviceId = getDevice($device, $mya->id)) {
      $edit = u\ray('id access', $deviceId, REQUEST_TIME);
      db\update('r_boxes', $edit, 'id'); // remember time last accessed so we can drop the deadwood
    } elseif ($op != 'identify') return posErr('unknown device');
  } elseif (@$agent or $op != 'identify') return posErr('missing device');
  
  $op = 'rCredits\\Smart\\' . $op;
  return $op($args);
}

/**
 * Return a (normally successful) json result to the remote caller.
 * @param assoc $args: named values to return
 * @param string $ok: "1" for success, "0" for failure
 * @return: does not return (when testing, return the json result)
 */
function ok($args, $ok = "1") {
  if ($msg = @$args['message']) $args['message'] = strip_tags($msg);
  $result = u\jsonEncode(compact('ok') + $args);
  u\loga('out', $args);
  //{t\output($args, 'output'); 
  if (t\est()) return $result;
  exit($result);
}

/**
 * Return an error message to the remote caller.
 * @param string $message: what to say
 * @return: does not return (when testing, return the json result)
 */
function posErr($message, $subs = array()) {
  $message = u\tt($message, $subs);
  return ok(compact('message'), "0");
}

/**
 * Identify the agent or customer.

// [member, code] => [ok, message, logon=1, name, descriptions[], can, device, update]
// [member, code] => [ok, message, logon=0, name, place, company, balance]

 * @params assoc $args:
 *   agent: agent ID of current agent -- empty if none yet
 *   device: a unique device ID
 *   member: account ID of customer or new agent
 * @return json:
 *   logon: 1 if the app's operator is scanning himself or herself in, else 0
 *   device: a unique device identifier (if none passed)
 *   name: the individual's name
 *   place: the location of the company if customer is a company agent, otherwise of the individual
 *   company: the customer's company name (if any)
 *   balance: a message stating the customer's current balance
 *   descriptions: array of the agent's company's most common charge descriptions (most popular first)
 *   can: bit permissions -- what the agent can do
 *   default: the default agent (empty if none allowed)
 *   company: the agent's company account ID
 *   update: URI of updated software package (if current version warrants)
 * device, descriptions, can, and update are returned only if scanning in (logon is true)
 */
function identify($args) {
  global $mya;
  if ($err = missingArg($keys = 'version device member code', $args)) return posErr($err); // no agent on scanin
  extract(u\just($keys . ' agent', $args));

  if (!$a = @$agent ? getCustomer(@$member, $err) : $mya) return posErr($err);

  if (@$agent) {
    if ($agent == $member) return posErr('already scanned in');
    $logon = ($a->id == $mya->id) ? "1" : "0"; // changing to another agent for the same company?
  } else $logon = "1";

  $cardCode = $a->agent->secure[$a->proSe ? 'cardCode' : 'cardCode2'];
  if (isDEV and $code == 'zot') $cardCode = $code; // for debugging
  if (@$device) $deviceId = getDevice($device, $mya->id);

  if (@$logon) { //  identifying agent
    if ($a->proSe and (!$a->co or $a->coCan(CO_REQUIRE_CASHIER))) return posErr('scan in first');
    if ($code != $cardCode) return posErr('bad agent');
    if (!$a->can(B_SCAN)) return posErr('agent cannot scan');
    if (!@$deviceId) {
      if ($device) {
        if ($owner = db\lookup('uid', 'r_boxes', 'code=:device', compact('device')) and !r\acct($owner)->coCan(CO_REQUIRE_CASHIER)) return posErr('restricted');
      } else $device = \user_password(R_CODE_LEN);
      r\makeDevice($a->id, $device, TX_SMART); // card is legit, so use device's code, if any (maybe from STAGE)
    }
/*  $restricted = db\lookup('restricted', 'r_boxes', 'code=:device', compact('device')); // 1 or 0
    if (@$restricted === FALSE) return posErr('unknown device');
    if ($restricted) return posErr('restricted'); */

    $descriptions = explode("\n", $a->selling) ?: array('?');
    $can = $a->can(B_BUY) ? $a->coCan() >> CO_CANS : 0;
    $default = $a->coCan(CO_REQUIRE_CASHIER) ? '' : $a->mainQid;
    $company = $a->fullName;
    
    if ($version + 0 < RPOS_VERSION) $update = substr($version, 0, 1) == '0' ? RPOS_FILENAMEB : RPOS_FILENAME;
  } else { // identifying customer
    if ($code != $cardCode) return posErr('bad member');
    if (!@$deviceId) return posErr('unknown device'); // tried to scan a customer card before very first agent scan-in
    $company = $a->proSe ? "" : $a->fullName;
    $place = r\location($a);
    $balance = getBalance($a);
    unset($device); // don't pass it back
  }

  $name = $a->agent->fullName;
  
  return ok(compact(u\ray('logon device name place company balance descriptions can default update')));
}

/*
 * Get an individual's photo.

// [member] => photo
 
 * @param array $args:
 *   member: the qid for the customer (individual or company rep)
 * @return:
 *   just the photo data (no json) or a short error message (50 characters or less)
 */
function photo($args) {
  if ($err = missingArg($keys = 'member', $args)) return photoReturn($err);
  extract(u\just($keys, $args));

  if (!@$member) return photoReturn('missing member');
  if (!$a = getCustomer($member, $err)) return photoReturn($err);
  $picFilename = @$a ? $a->agent->auxFilename('picture') : R_DFT_PICTURE;
  $pic = file_get_contents(DRUPAL_ROOT . $picFilename);
  return photoReturn($pic, $picFilename);
}

function photoReturn($pic, $picFilename = '') {
  $max = 50;
  $message = strlen($pic) < $max ? $pic : bin2hex(substr($pic, 0, $max)); // identifies pic w/out a big db hit
  u\loga('out', compact('message'));

  if (t\est()) {
    if (strpos($picFilename, 'no-photo')) $message = 'no photo';
    t\output($message); 
    return $message;
  } else exit($pic);
}

/*
 * Request a charge.

// [member, amount, goods, description] => [ok, message, tx, balance, undo]

 * @param array $args:
 *   agent: qid for the seller (company rep)
 *   member: qid for the customer (individual or company rep)
 *   amount: numeric dollar amount
 *   description: description of goods and services sold
 * @param string $type: charge or pay (refund)
 * @return: json (ok, message, tx, balance, undo)
 */
function charge($args, $type = 'charge') {
  global $mya;
  
  if ($err = missingArg($keys = 'agent member amount goods description', $args)) return posErr($err);
  extract(u\just($keys, $args));

  if (!@$member) return posErr('missing member');
  if (@$description === '') return posErr('missing description');
  if (!$a = getCustomer($member, $err)) return posErr($err);
  if ($mya->id == $a->id) return posErr("shoulda been login");
  if ($err = u\badAmount($amount, '')) return posErr($err); // allow negatives! (for refund)

  list ($msg, $subs, $zot) = be\transfer($type, $mya, $a, @$amount, @$description, @$goods, TRUE);
  if (!@$subs['success']) return posErr($msg, $subs);

  $message = u\tt($msg, $subs);
  $undo = u\tt('confirm undo', $subs);
  $forCash = $goods ? '' : t(' for cash');
  if ($balance = getBalance($a)) $balance .= u\tt('just did', $subs + compact('forCash'));
  $tx = $subs['xid'];
  return ok(compact(u\ray('message tx balance undo')));
}

// [member, amount, goods, description] => [ok, message, tx, balance, undo]
function payment($args) {return charge($args, 'payment');}

/*
 * Undo the specified transaction

// [tx] => [ok, message, balance]
 
 * @param array $args:
 *   tx: transaction record number
 * @return: json (ok, message, balance)
 */
function undo($args) {
  global $mya; $myid = $mya->id;
  if ($err = missingArg($keys = 'tx', $args)) return posErr($err);
  extract(u\just($keys, $args));
  
  list ($msg, $subs) = be\undoTx($tx, ':myid IN (payer, payee)', compact('myid'));
  if (!@$subs['success']) return posErr($msg, $subs);
  
  $id = db\lookup('IF(payer=:myid, payee, payer)', 'r_txs', 'xid=:tx', compact('myid', 'tx'));
  $balance = getBalance(r\acct($id)); // get customer balance
  $message = u\tt($msg, $subs);
/// (no undo of an undo)  $undo = u\tt('confirm undo', $subs);
  return ok(compact(u\ray('message balance')));
}

/**
 * Report if an argument is missing.
 */
function missingArg($expected, $args) {
  foreach (u\ray($expected) as $one) if (!isset($args[$one])) return "missing $one";
}

/**
 * Return the agent's acct.
 * @param string $agent: the agent's qid or company's uid (if company is pro se).
 * @param string $err: (returned) error message, if any
 * @return: the agent's acct object (false if error)
 */
function getAgent($agent, &$err = '') {
  if (!$mya = r\acct($agent)) return !$err = 'bad agent'.$agent;
//  if (r\foreignServer(r\uidRegion($mya->agentId))) return !$err = 'wrong server for agent';
  if (!$mya->ok) return !$err = 'company inactive';
//  if (!$mya->can(B_SELL)) return !$err = 'agent cannot sell';
  r\acct::setDefault($mya); // set current account temporarily to owner, so that error log has it right
  return $mya;
}

/**
 * Return the customer's acct.
 * @param string $customer: the customer's qid.
 * @param string $err: (returned) error message, if any
 * @return: the customer's acct object (false if error)
 */
function getCustomer($customer, &$err = '') {
  if (!$a = r\acct($customer)) return !$err = 'bad member';
//  if (r\foreignServer(r\uidRegion($a->agentId))) return !$err = 'wrong server for customer';
  if (!$a->ok) return !$err = 'member inactive';
//  if (!$a->can(B_BUY)) return !$err = 'customer cannot buy';
  return $a;
  
//    if (!$smartInfo = db\lookup('owner,defaultAgent,transientId,access', 'r_boxes', 'code=:q_code', compact('q_code'))) return be\error('unknown device');

/*    if ($account_id != $q_account_id) { // this is a transient id scanned from the other party's smart device
      $transientId = basename(dirname($q_account_id));
      $recent = REQUEST_TIME - R_TRANSIENCE;
      if (db\lookup('transientId', 'r_boxes', 'code=:code AND access>=:recent') != $transientId) return be\error('bad transient id');
    } */
}

/**
 * Describe what the customer can spend on goods and cash.
 * @param acct $a: customer account
 * @return: a descriptive string
 */
function getBalance($a) {
  if ($a->can(B_SECRET_BAL)) return ''; // customer wants it secret
  $name = $a->fullName;
  if (!$a->proSe) $name = $a->agent->fullName . t(", for ") . $name;
  $balance = u\fmtAmt($a->r + $a->usd); // account balance
  $spendable = u\fmtAmt($a->rAvailable('buy', TRUE)); // amount spendable (without drawing on other accounts)
  $spendable = $spendable == $balance ? '' : ("\n" . t('Spendable: ') . $spendable);
  $cashable = u\fmtAmt(max(0, $a->rAvailable('cash', TRUE))); // amount cashoutable
  return u\tt('customer balance', compact(u\ray('name balance spendable cashable')));
}

/**
 * Build a list of the account's most popular charge descriptions, beginning with the default.
 * @params acct $a: the company account
 * @return array of descriptions
 */
function getDescriptions($a) {
  $ray = array();
  //if ($default = $a->description) $ray[] = $default;
  $sql = 'SELECT payeeFor, COUNT( payeeFor )  FROM r_txs WHERE payee=:id GROUP BY payeeFor ORDER BY COUNT(payeeFor) DESC';
  return $ray + (db\q($sql, u\ray('id', $a->id))->fetchCol() ?: array());
}

/**
 * Return the message with adjustments if it contains negative amounts (for refunds or unrefunds)
 * @param string $msg: (returned) the message to adjust, if it has negative amounts
 * @param string $phrase: what to replace
 * @param string $rephrase: what to say instead
 * @return the message, rephrased (if appropriate) without negative amounts
 */
function fixNegative(&$msg, $phrase, $rephrase) {
  if ((strpos($msg, $phrase) === FALSE) or !strpos($msg, '$-')) return $msg;
  $msg = preg_replace('/\$-/', '$', $msg, 1); // just one replacement (leave negative rewards alone)
  return $msg = str_replace($phrase, $rephrase, $msg);
}

/**
 * Return the record id for the record of this company using this device.
 */
function getDevice($code, $uid) {
  return db\lookup('id', 'r_boxes', 'code=:code AND uid=:uid', compact('code', 'uid'));
}