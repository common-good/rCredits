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
 * agent    the already-scanned-in agent's agent ID (for example NEW:AAB) -- ignored for "update" op
 *
 * REQUESTS:
 * update:	[version] => uri
 * identify:	[member, code] => [ok, message, logon="1", name, descriptions[], can, device]
 * identify:	[member, code] => [ok, message, logon="0", name, place, company]
 * photo:	[member] => photo
 * charge:	[member, amount, description] => [ok, message, tx, balance, undo]
 * refund:	[member, amount, description="refund"] => [ok, message, tx, balance, undo]
 * undo:	[tx] => [ok, message, balance]
 */

define('R_QR_HEADER', 'HTTP://RC2.ME/I/'); // for transient QR IDs
define('R_TRANSIENCE', 60); // number of seconds for device's QR to be valid

$GLOBALS[TX_SMART . ' messages'] = array(
  'bad member' => t('That rCard is not valid.'),
  'bad agent' => t('That is not a valid agent rCard.'),
  'agent cannot scan' => t('That agent is not authorized to charge customers.'),
  'scan in first' => t('You have to scan yourself in before charging a customer.'),
  'already scanned in' => t('You are already scanned in.'),
  'company inactive' => t('That company is not an active participant.'),
//  'agent cannot sell' => t('That agent is not authorized to charge customers.'),
  'customer balance' => t("The customer's balance is @spendable\n(@cashable can be exchanged for cash)."),
  'report exchange' => t("You @did @otherName @amount in exchange for cash."),
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
  global $mya;
  
  if (!$args) $args = $_POST;
  u\loga('in', $args);
  extract(u\just('op device agent member', $args));
  if (!in_array($op, u\ray('update identify photo charge undo refund'))) return posErr('bad op');

  if (!$mya = getAgent($agent ?: $member)) return posErr('bad agent'); // set "current" account for internals

  if (@$device) {
    if (!$args['device'] = getDevice($device, $mya->id)) return posErr('unknown device');
  } elseif ($op != 'identify') return posErr('missing device');
  
  $op = 'rCredits\\Smart\\' . $op;
  return $op($args);
}

function ok($args, $ok = "1") {
  $result = u\jsonEncode(compact('ok') + $args);
  u\loga('out', $args);
  if (t\est()) {t\output($args, 'output'); return $result;}
  exit($result);
}

function posErr($message) {
  $message = u\tt($message);
  ok(compact('message'), "0");
}

/**
 * Update the Android app if appropriate.
 
// [version] => uri
 
 * @params assoc $args:
 *   version: the app's current version number
 * @return string: the download URI for the update -- empty if none needed
 */
function update($args) {
  if ($err = badArg($keys = 'device version', $args)) return posErr($err);
  extract(u\just($keys, $args));
  exit(''); // no update for now
}

/**
 * Identify the agent or customer.

// [member, code] => [ok, message, logon=1, name, descriptions[], can, device]
// [member, code] => [ok, message, logon=0, name, place, company]

 * @params assoc $args:
 *   agent: agent ID of current agent -- empty if none yet
 *   device: r_smarts record id (if any) -- originally the device ID, but changed in pos() to record id
 *   member: account ID of customer or new agent
 * @return json:
 *   logon: 1 if the app's operator is scanning himself or herself in, else 0
 *   device: a unique device identifier (if none passed)
 *   name: the individual's name
 *   company: the customer's company name (if any)
 *   place: the location of the company if customer is a company agent, otherwise of the individual
 *   descriptions: array of the agent's company's most common charge descriptions (most popular first)
 *   manager: true if the agent is allowed to make refunds and undo transactions
 * device, descriptions and manager are returned only if scanning in (logon is true)
 */
function identify($args) {
  global $mya;
  if ($err = badArg($keys = 'agent device member code', $args)) return posErr($err);
  extract(u\just($keys, $args));

  if (!$a = @$agent ? getCustomer(@$member, $err) : $mya) return posErr(@$err ?: 'missing mya');
  $name = $a->agent->fullName;
  $name .= " ($member)"; // debug

  if (@$agent) {
    if ($agent == $member) return posErr('already scanned in');
    $logon = ($a->id == $mya->id) ? "1" : "0"; // changing to another agent for the same company?
    //if ($logon == "0") die(print_r(compact(u\ray('args agent member a mya')), 1));
  } else $logon = "1";

  $cardCode = $a->agent->secure[$a->proSe() ? 'cardCode' : 'cardCode2'];
  if (isDEV and $code == 'zot') $cardCode = $code; // for debugging

  if (@$logon) { //  identifying agent
    if ($a->proSe()) return posErr('scan in first');
    if ($code != $cardCode) return posErr('bad agent');
    if (!$a->can(B_SCAN)) return posErr('agent cannot scan');
    $descriptions = getDescriptions($a);
    $can = $a->can(B_BUY) ? 0b11111 : 0; // pass all permission for now
    $manager = $a->can(B_BUY) ? "1" : "0"; // should probably be B_REFUND (between B_SELL and B_BUY)
    if (@$device) {
      $edit = u\ray('id access', $device, REQUEST_TIME);
      db\update('r_smarts', $edit, 'id'); // remember time last accessed so we can drop the deadwood
    } else $device = makeDevice($a->id);
  } else { // identifying customer
    if ($code != $cardCode) return posErr('bad member');
    $company = $a->proSe() ? "" : $a->fullName;
    $place = r\location($a);
  }
  
  return ok(compact(u\ray('logon device name company place descriptions can')));
}

/*
 * Get an individual's photo.

// [member] => photo
 
 * @param array $args:
 *   member: the qid for the customer (individual or company rep)
 * @return:
 *   just the photo data (no json)
 */
function photo($args) {
  if ($err = badArg($keys = 'member', $args)) return posErr($err);
  extract(u\just($keys, $args));

  if (!$a = r\acct($member)) return posErr('bad member');
  $picFilename = @$a ? $a->auxFilename('picture') : R_DFT_PICTURE;
  $pic = file_get_contents(DRUPAL_ROOT . $picFilename);
  $message = bin2hex(substr($pic, 0, 50)); // this allows identifying the picture without a big db hit
  u\loga('out', compact('message'));

/**/ if (t\est()) {debug($message); return $message;} else exit($pic); // keep
}

/*
 * Request a charge.

// [member, amount, description] => [ok, message, tx, balance, undo]

 * @param array $args:
 *   agent: qid for the seller (company rep)
 *   member: qid for the customer (individual or company rep)
 *   amount: numeric dollar amount
 *   description: description of goods and services sold (empty if exchanging cash for rCredits)
 * @param string $type: charge or pay (refund)
 * @return: json (ok, message, tx, balance, undo)
 */
function charge($args, $type = 'charge') {
  global $scanned; $scanned = true;
  global $mya;
  
  if ($err = badArg($keys = 'agent member amount description', $args)) return posErr($err);
  extract(u\just($keys, $args));

  if (!$a = r\acct(@$member, $err)) return posErr($err);
  if ($mya->id == $a->id) return posErr("shoulda been login. agent=$agent member=$member."); // debug
  u\EXPECT($mya->id != $a->id, 'should have been a login');
  if ($err = u\badAmount($amount, '>0')) return posErr($err);
  $goods = (@$description === '');

  list ($msg, $subs, $zot) = be\transfer($type, $mya, $a, $amount, $description, $goods, TRUE);
  if (!$subs['success']) return posErr($msg, $subs);

  $balance = getBalance($a);
  $tx = $subs['xid'];
  $undo = u\tt('confirm undo', $subs);
  $message = u\tt($msg, $subs);
  return ok(compact(u\ray('message tx balance undo')));
}

// [member, amount, description="refund"] => [ok, message, tx, balance, undo]
function refund($args) {return charge($args, 'payment');}

/*
 * Undo the specified transaction

// [tx] => [ok, message, balance]
 
 * @param array $args:
 *   tx: transaction record number
 * @return: json (ok, message, balance)
 */
function undo($args) {
  global $scanned; $scanned = true;
  if ($err = badArg($keys = 'tx', $args)) return posErr($err);
  extract(u\just($keys, $args));
  
  list ($msg, $subs) = be\undoTx($tx);
  if (!$subs['success']) return posErr($msg, $subs);
  $balance = getBalance(r\acct());
  $message = u\tt($msg, $subs);
  $undo = u\tt('confirm undo', $subs);
  return ok(compact(u\ray('message balance')));
}

/**
 * Report if an argument is missing.
 */
function badArg($expected, $args) {
  foreach ($expected as $one) if (!isset($args[$one])) return "Missing $one";
}

/**
 * Return the agent's acct.
 * @param string $agent: the agent's qid.
 * @param string $err: (returned) error message, if any
 * @return: the agent's acct object (false if error)
 */
function getAgent($agent, $err = '') {
  if (!$mya = r\acct($agent)) return !$err = 'bad agent';
//  if (r\foreignServer(r\uidRegion($mya->agentId))) return !$err = 'wrong server for agent';
  if (!$mya->ok()) return !$err = 'company inactive';
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
  if (!$a->ok()) return !$err = 'member inactive';
//  if (!$a->can(B_BUY)) return !$err = 'customer cannot buy';
  return $a;
  
//    if (!$smartInfo = db\lookup('owner,defaultAgent,transientId,access', 'r_smarts', 'code=:q_code', compact('q_code'))) return be\error('unknown device');

/*    if ($account_id != $q_account_id) { // this is a transient id scanned from the other party's smart device
      $transientId = basename(dirname($q_account_id));
      $recent = REQUEST_TIME - R_TRANSIENCE;
      if (db\lookup('transientId', 'r_smarts', 'code=:code AND access>=:recent') != $transientId) return be\error('bad transient id');
    } */
}

function getBalance($a) {
  if ($a->can(B_SECRET_BAL)) return ''; // customer wants it secret
  $spendable = u\fmtAmt($a->rAvailable('buy', TRUE)); // amount spendable (without drawing on other accounts)
  $cashable = u\fmtAmt($a->rAvailable('cash', TRUE)); // amount cashoutable
  return u\tt('customer balance', compact('spendable', 'cashable'));
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

/*
 * Get a permanent device code from the server.
 */
function makeDevice($uid) {
  $code = \user_password(20);
  $device = t('Smart Device');
  $access = REQUEST_TIME;
  $info = compact(u\ray('uid code device access'));
  return @db\insert('r_smarts', $info) ? $code : '';
}

/**
 * Return the record id for the record of this company using this device.
 */
function getDevice($code, $uid) {
  return db\lookup('id', 'r_smarts', 'code=:code AND uid=:uid', compact('code', 'uid'));
}