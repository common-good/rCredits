<?php
namespace CG\Smart;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;

/**
 * @file
 * Smart phone interface to the %PROJECT system.
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
 * (see https://docs.google.com/document/d/1fZE_wmEBRTBO9DAZcY4ewQTA70gqwWqeqkUnJSfWMV0/edit)
 * time:  [] => [ok=”1”, time, message]
 * identify:  [member, code] 
 *      => [ok, message, logon="1", name, descriptions[], can, device, default, company, time]
 *   OR  => [ok, message, logon, name, place, company, balance] where logon is 0 or -1
 * photo:  [member, code] => photo
 * charge:  [member, amount, goods, description, created, force] 
 *   => [ok, message, txid, created, balance, undo]
 * undo:  [txid, force] => [ok, message, balance]
 *
 * To create json for app demo, do this from PHP window on STAGE server:
   include_once __DIR__ . '/../rcredits/rsmart/rsmart.inc';
   $args = CG\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', 'NEW.AAB', RPOS_VERSION, 'NEW.ABB', 'ZzhWMCq0zcBowqw'); // cust
   //$args = CG\Util\ray('op device agent version member code', 'identify', '3i2ifsapEjwev3CwBCV7', '', RPOS_VERSION, 'NEW:AAB', 'WeHlioM5JZv1O9G'); // agent login
   //$args = CG\Util\ray('op device agent version member amount goods description', 'charge', '3i2ifsapEjwev3CwBCV7', 'NEW:AAB', RPOS_VERSION, 'NEW.ABB', AMOUNT, 1, 'zot');
   //$args = CG\Util\ray('op device agent version txid', 'undo', '3i2ifsapEjwev3CwBCV7', 'NEW:AAB', RPOS_VERSION, '4069');
///     debug(CG\Smart\pos($args));
 */

define('RPOS_VERSION', 221); // lastest POS app version available for download (this should match the versionCode in the app's manifest. upload and test the app first (and make sure it is available in Google Play Store), before bumping the number up here.)
define('R_QR_HEADER', 'HTTP://RC2.ME/I/'); // for transient QR IDs
define('R_TRANSIENCE', 60); // number of seconds for device's transient QR to be valid

require_once __DIR__ . '/../cg-strings.inc';

$GLOBALS['tStrings'] += array(
  'bad proof' => t('That transaction is not valid.'),
  'bad member' => t('That is not a valid account. The customer used a lost, stolen, or fraudulent %PROJECT card.'),
//  'card deactivated' => t('The %PROJECT card has been de-activated.'),
  'bad agent' => t('That is not a valid %PROJECT company card.'),
  'agent cannot scan' => t('That agent is not authorized to charge customers.'),
  'scan in first' => t('You have to scan yourself in before charging a customer.'),
  'already scanned in' => t('You are already scanned in.'),
  'wrong company' => t('That %PROJECT card is for a different company.'),
  'company inactive' => t('That company is not an active participant.'),
//  'customer balance' => t("Customer: %name\n\nBalance: %balance%spendable\nTradable for cash: %cashable"),
  'customer balance' => "Customer: %name\n\nBalance: %balance\nRewards to date: %rewards\n", // (parseable)
  'report exchange' => t("You %did %otherName %amount in exchange for cash."),
  'just did' => t("\n\nWe just %did %amount %forCash."), // "We" not "You" here, in case viewed by manager
  'unknown device' => t('This device is not recognized. You must scan yourself in (at least once) before charging a customer.'),
  'confirm undo' => t('Undo transfer of %amount %tofrom %otherName? (Press OK for Yes, Cancel for No.)'), // no creation date, unlike web and SMS
  'member inactive' => t('That account is not yet activated.'),
  'bad icard' => t('The customer has not yet activated his or her %PROJECT Account.'),
  'expired icard' => t('That invitation card has expired. The customer can use their %PROJECT card instead.'),
  'bad pin' => t('That is not your correct PIN.'), // self-service error
  'no such test' => t('That test function does not exist.'),
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
  global $channel; $channel = TX_POS;
  global $scanned; $scanned = true;
  global $mya;
  
  if (!r\up()) exit('system down'); // app does not (and should not) distinguish this from system failure
  if (is_object($args)) $args = (array) $args; // this helps with debugging when passing json_decoded args
  if (is_string($args)) $args = urlq2ray($args); // convert from query string to array
//  if (!$args) $args = t\POST(); else t\POST($args);
  if ($args) f('t.POST', $args); else $args = $_POST;
  u\loga('in', $args);
//  $args = u\sanitizeInput($args0 = $args);
  $args = $args0 = $args;
  extract(just('op device agent member version', $args));
  $args['version'] = (int) @$version;
  
  if (!@$op or !in($op, 'time identify photo charge test')) return posErr('bad op');
  
  if (isDEV and $op == 'test') return test($args0);
  
  $mya = getAgent(@$agent ?: @$member, $agentErr);
///     print_r(compact(ray('mya agent member agentErr')));

  if (@$device) { // device was specified, substitute its record id, herein
    if (!$mya) return posErr($agentErr);
    if ($deviceId = getDevice($device, $mya->id)) {
      $edit = ray('id access', $deviceId, r\rTime());
      db\update('r_boxes', $edit, 'id'); // remember time last accessed so we can drop the deadwood
    } elseif ($op != 'identify') return posErr('unknown device');
  } elseif (@$agent or $op != 'identify') return posErr('missing device');
  
  if ($op == 'time') return getTime($args); // no agent required

  if (!$mya) return posErr($agentErr); // set "current" account for internals
  
  if (!$mya->cttyUp) return posErr('your community down', ray('contact', $mya->cAdminContact()));

  $op = 'CG\\Smart\\' . $op;
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
  if (@$args['op'] != 'time') u\loga('out', $args); // avoid filling the log with time requests
  //{t\output($args, 'output'); 
  if (u\test()) return $result;
  header('Access-Control-Allow-Origin: *'); // allow access from web browsers
  exit($result);
}

/**
 * Return an error message to the remote caller.
 * @param string $message: what to say
 * @param assoc $subs: substitions in message (if any)
 * @param assoc $extra: extra parameters to return (if any)
 * @return: does not return (when testing, return the json result)
 */
function posErr($message, $subs = [], $extra = []) {
  $message = t($message, $subs);
  return ok(compact('message') + $extra, "0");
}

/**
 * Return information from a test function.
 */
function test($args) {
  global $testOnly;

  if ($err = missingArg($keys = 'function testOnly args', $args)) return posErr($err); 
  extract(just($keys, $args));
  $args = [json_decode($args)];

  require_once __DIR__ . '/rsmart.steps';
  $func = "\\$function";
  if (function_exists($func)) {
    $res = call_user_func_array($func, $args);
    //$reflFunc = new \ReflectionFunction($func);
    //print "\n" . $reflFunc->getFileName() . ':' . $reflFunc->getStartLine();
  } else {
    require_once __DIR__ . '/../cg-testing.inc';
    $func = "\\CG\\Testing\\$function";
    if (function_exists($func)) $res = call_user_func_array($func, $args);
  } 
  
  if (!isset($res)) return posErr('no such test');

  return $res ? ok([], '1') : posErr('step failed', [], \drupal_get_messages());
}

/**
 * Get the current server time
 * time: [data] => [ok=”1”, time, message]
 * @param string $data: optional message string or json-encoded data (perhaps data we requested from the app)
 * @return json:
 *   time: the current unixtime
 *   message: something for the app to say or do (if it begins with "!")
 */
function getTime($args) {
  global $mya;
  if ($err = missingArg($keys = 'version', $args)) return posErr($err); 
  extract(just($keys . ' data', $args));
  $time = r\rTime();
  
  if ($version < RPOS_VERSION) $message = $version < 218 
  ? t("IMPORTANT: Your app needs updating.\n\nSearch for \"CG Pay\" in the Google Play Store, to download the latest version.")
  : '!update';
  if (@$data or !@$message) $message = handleAppMsg(@$data);

  return ok(compact(ray('time message')));
}
 
/**
 * Identify the agent or customer.
 * identify: [member, code] 
 *      => [ok, message, logon=”1”, name, device, default, company, time, descriptions[], bad[], can]
 *   OR => [ok, message, logon, name, place, company, balance, rewards, time, descriptions[], bad[], can]
 *          where logon is 0 or -1
 * @params assoc $args:
 *   agent: agent code of current agent -- empty if none yet
 *   device: a unique device ID
 *   member: account ID of customer or new agent
 *   code: rCard security code (possibly encrypted with itself as the key)
 * @return json:
 *   logon: 1 if the app's operator is scanning himself or herself in, else 0 (normal) or -1 (first-time)
 *   device: a unique device identifier (if none passed)
 *   name: the individual's name
 *   place: the location of the company if customer is a company agent, otherwise of the individual
 *   company: the member's (agent's or customer's) company name (if any)
 *   balance: a message stating the customer's current balance
 *   descriptions: array of the agent's company's most common charge descriptions (most popular first)
 *   can: bit permissions -- what the agent can do
 *   default: the default agent (empty if none allowed)
 * device, descriptions, and can are returned only if scanning in (logon is true)
 */
function identify($args) {
  global $mya;

  $companyInfo = getCompanyInfo();
  if ($err = missingArg($keys = 'device member code version', $args)) return posErr($err, [], $companyInfo);
  extract(just($keys . ' agent pin signin', $args)); // no agent on scanin

  if (@$agent) { // get other party
    if ($info = machine(@$member, $code)) return ok($info + $companyInfo); // purchase from fuel pump or other machine
    if (!$a = getMember(@$member, $code, $err, $version > 217)) return posErr($err, [], $companyInfo);
  } else $a = $mya; // get agent

  if ($version > 217 and !u\test()) {
    if (@$signin) {
      if (@$agent) {
        if ($a == $mya) return posErr('already scanned in', [], $companyInfo);
        if ($a->id != $mya->id) return posErr('wrong company', [], $companyInfo);
      }
    } elseif (!@$agent) return posErr('scan in first', [], $companyInfo);
    $first = $a->agentA->ided ? '0' : '1'; // regular or first-time transaction 
  } else {
    $logon = '1'; // assume logging in
    if (@$agent) {
      if ($agent == $member) return posErr('already scanned in', [], $companyInfo);
      if ($a->id != $mya->id) $logon = $a->agentA->ided ? '0' : '-1'; // regular or first-time transaction for customer
  //    if ($logon == '-1' and $mya->coCan(CO_FAST)) $logon = '0'; // this company exempt from photoId checks
      $first = $logon == '-1' ? '1' : '0';
    }
    $signin = ($logon == 1); // ditto
    if (@$signin and $a->proSe and !$a->co) return posErr('scan in first', [], $companyInfo);
  }
  
  if (@$device) $deviceId = getDevice($device, $mya->id);

  if (@$signin) { //  identifying agent
//    if ($a->proSe and (!$a->co or $a->coCan(CO_REQUIRE_CASHIER))) return posErr('scan in first');
    if ($code != $a->cardCode() and $code != $a->codeHash()) {
      if (u\test() or $version >= 300) return posErr('bad agent', [], $companyInfo); // . substr(bin2hex($code), 0, 20) . ' <> ' . substr(bin2hex($a->codeHash()), 0, 20) . ' <> ' . substr($code, 0, 20)); // . "=$agent (cardCode) id=$a->id myid=$mya->id agentId=$a->agentId qid=$a->qid code=$code acode=" . $a->cardCode());
      r\tellAdmin('bad code in identify op: ' . $err, compact(ray('member code')));
    }
    if (!$a->can(B_SCAN)) return posErr('agent cannot scan', [], $companyInfo);
    if (!@$deviceId) {
/*      if ($device) {
        if ($owner = db\get('uid', 'r_boxes', 'code=:device', compact('device')) and !r\acct($owner)->coCan(CO_REQUIRE_CASHIER)) return posErr('restricted');
      } else $device = u\code(); */
      if (!$device) $device = u\code();
      r\makeDevice($a->id, $device, TX_POS); // card is legit, so use device's code, if any
    }
    $default = $mya->mainQid;
    $company = $mya->fullName;
    $time = r\rTime();
    
  } else { // identifying customer
    if (!@$deviceId) return posErr('unknown device'); // tried to scan a customer card before very first agent scan-in
    if (isset($pin) and $pin != $a->agentA->pin) return posErr('bad pin', [], $companyInfo); // maybe never use this
    $agentCans = 0;
    $company = $a->proSe ? "" : $a->fullName;
    $place = r\location($a);
    unset($device); // don't pass it back
    extract(balrew($a));
  }

  $name = $a->agentA->fullName;
  
  return ok(compact(ray('logon first device name place company balance rewards default time')) + $companyInfo);
}

/**
 * Get an individual's photo.
 * photo:  [member, code] => photo
 * @param array $args:
 *   member: the qid for the customer (individual or company rep)
 *   code: card code
 * @return:
 *   just the photo data (no json) or a short error message (50 characters or less)
 */
function photo($args) {
  if ($err = missingArg($keys = 'member', $args)) return photoReturn($err);
  extract(just($keys . ' code version', $args));

  if (!@$member) return photoReturn('missing member');
// FAILS in background in version 217  if (!@$code) return posErr(missingArg('code', []));
// FAILS in background in version 217    if (!$a = getMember($member, $code, $err)) return photoReturn($err);
  if (!@$code) r\tellAdmin('missing code in photo op', compact(ray('member code')));
  if (!$a = getMember($member, $code, $err, u\test() or $version > 217)) return photoReturn($err);

	$signingIn = (bool) strpos($member, '-'); // allow agents with no photo by faking a photo (>= 100 chars)
  return photoReturn($a->agentA->photo ?: ($signingIn ? file_get_contents(R_ROOT . '/images/idcard/no-photo.jpg') : ''));
}

/**
 * Return the customer's account photo.
 */
function photoReturn($pic) {
  $max = 50; // arbitrary abbreviation length for logging and testing
  $message = strlen($pic) <= $max ? $pic : bin2hex(substr($pic, 0, $max)); // identifies pic in log w/out a big storage hit
  u\loga('out', compact('message'));

  if (u\test()) {
    $filename = DRUPAL_ROOT . R_PICTURE_DIR . R_DFT_PICTURE; // never a company photo
///		echo "flnm=$filename";
    $picNone = file_get_contents($filename);
    if (u\starts(bin2hex(substr($picNone, 0, $max)), $message)) $message = 'no photo';
//    if ($pic === '') $message = 'no photo';
    f('t.output', $message); 
    return $message;
  }

  if (strlen($pic) > $max) header("Content-type: image/jpeg");
  exit($pic);
}

/*
 * Request a charge.
 * charge:  [member, amount, code, goods, description, created, force]
 *   => [ok, message, txid, created, balance, undo]
 * @param array $args:
 *   agent: qid for the seller (company rep)
 *   amount: numeric dollar amount
 *   member: qid for the customer (individual or company rep)
 *   code: card code (phase out in new app 4/1/2016)
 *   goods: <transaction is for real goods and services>
 *   description: description of goods and services sold
 *   created: transaction creation date
 *   proof: hash of agent + amount + member + code + created (proof of card-present tx on or before created date)
 *   force: 0=nothing special, 1=do transaction even if insufficient funds, -1=force reversal of transaction
 * @param string $type: charge or pay (refund)
 * @return: json (ok, message, txid, created, balance, undo)
 */
function charge($args, $type = 'charge') {
  global $mya;
  
  $keys = 'agent member amount goods description created';
  if ($err = missingArg($keys, $args)) return posErr($err);
  extract(just($keys . ' proof force version', $args));

  if (!@$member) return posErr('missing member'); // handle empty string (versus missing param)
  if ($version > 216 and !@$proof) return posErr(missingArg('proof', []));
  if (@$description === '') return posErr('missing description');
  if (strpos($description, ' ') !== FALSE) {
    r\tellAdmin(t('POS tx description not urlencoded'), $args);
  } else $description = urldecode($description);
  if ($err = u\badAmount($amount, '')) return posErr($err); // allow negatives! (for refund)

  if (!$a = getMember($member, '', $err, FALSE)) { // proof is proof enough (no cardCode needed)
    return badTx($args, "Account $member", $err);
  } else $name = $a->fullName; // for use in error messages
  if ($mya->id == $a->id) return posErr('shoulda been login'); // should never happen

  $selfhelp = ($goods == '2');
  $goods = $goods == "3" ? FOR_NONGOODS : ($goods == "0" ? FOR_USD : FOR_GOODS); // selfhelp is FOR_GOODS
  if (@$force) {
    $subs = ray('created payer payee amount goods payeeFor', $created, $a->id, $mya->id, $amount, $goods, $description);
    if ($q = db\get('xid,data', 'r_txs', $subs)) extract($q);
  }
  
  $amount = str_replace(',', '', number_format($amount, 2)); // make sure both cents digits are included
  $code = $a->cardCode();
  $hashOfTx = u\hash($wanted = "$mya->mainQid$amount$member$code$created");
  if ($version > 216) if ($proof != $hashOfTx) if (!$force or $a->badOldProof($proof, $wanted, $created)) {
    r\tellAdmin('bad proof', compact(ray('version proof hashOfTx agent amount member code created force')));
    if (u\test() or $version > 218 ) {
      if (u\test()) f('t.output', "Should be proof of $wanted");
      $err = u\test() ? t('bad proof') : t('bad proof (wanted %hashOfTx as hash of %wanted', compact('hashOfTx', 'wanted'));
      return badTx($args, $name, $err);
    }
  }

  if (@$force == "1" and @$xid) { // does exist and should (always background)
    $message = $did = $undo = '';
    $txid = $xid;
  } elseif (@$force == "-1" and @$xid) { // does exists and shouldn't
    $did = '';
    $undo = $xid;
    $data = unserialize($data);
    if ($txid = @$data['undoneBy']) { // already got reversed
      $created = db\get('created', 'r_txs', 'xid=:txid', compact('txid'));
      $message = "Transaction has been reversed.";
    } else {
      list ($msg, $subs) = be\undoTx($xid, (@$amount < 0 and !$mya->co) ? 'payer=:myid' : 'payee=:myid', ['myid' => $mya->id], $force);
      if (is_array($msg)) list ($msg, $subs) = [$msg[0], $msg[1] + $subs];
      if (!@$subs['success']) return badTx($args, $name, $msg, $subs);
      $txid = $subs['xid'];
      $created = $subs['created0'];
      $message = t($msg, $subs);
      r\tellAdmin('reversing tx', ray('xid uid created message subs', $xid, $mya->id, u\fmtDate($created), $message, $subs));
    }
  } elseif (@$force == "-1" and !@$xid) { // doesn't exist and shouldn't
    $did = $undo = $created = '';
    $txid = 0;
    $message = "Transaction has been canceled.";
  } else { // doesn't exist and should (forcing or not)
    if (@$amount < 0 and $a->co and $a->proSe) return badTx($args, $name, $msg, $subs);
    if (@$amount < 0 and !$mya->co) list ($type, $amount) = ['payment', -$amount];
    list ($msg, $subs) = be\transfer($type, $mya, $a, @$amount, @$description, compact(ray('goods created force')));
    if (!@$subs['success']) return badTx($args, $name, $msg, $subs);

    if (!@$force and $aa = $a->agentA and $mya->co and !$aa->ided) $aa->setBit(B_IDED);
    $txid = $subs['xid'];
    if ($selfhelp) {
      $msg = 'selfhelp tx';
      if ($mya->cttyRewardy) $msg .= '|for why|reward other';
    } else {
      $forCash = $goods == FOR_USD ? t('for USD') : '';
      $did = t('just did', $subs + compact('forCash'));
    }
    $message = t($msg, $subs);
    $undo = t('confirm undo', $subs);
  }

  extract(balrew($a));

  return ok(compact(ray('message txid created balance rewards did undo')));
}

/**
 * Handle request for purchase from fuel pump or other machine.
 * @param string $member: account qid of machine (including agent number, which is machine number)
 * @param string $code: M<n>, where <n> is a 1-6 digit radix-36 machine number, from QR code in place of security code
 * @return assoc of parameters to return or FALSE if this is not a machine purchase
 */
function machine($member, $code){
  global $mya;
  if ($mya->co or !$mya->proSe) return FALSE;
  if (strlen($code) > 7 or substr($code, 0, 1) != 'M') return FALSE;
  
}

/**
 * Tell the device owner about a failed background transaction
 */
function badTx($args, $account, $msg, $subs = []) {
  extract(just('force amount created', $args));
  
  if (@$force) {
    global $mya;
    $by = $mya->proSe ? '' : t(' (agent %agent)', 'agent', $mya->agentA->fullName);
    $subs += ray('account amount created by', $account, u\fmtAmt($amount), u\fmtDate($created), $by);
    r\message($mya->id, "bad forced tx|$msg", $subs);
  }
  
  return posErr($msg, $subs);
}

/**
 * Return info about current company
 */
function getCompanyInfo() {
  global $mya; 
  
  $descriptions = explode("\n", $mya->selling) ?: ['?'];
  $agentCan = ($mya->can(B_SCAN) ? u\bit(CO_CHARGE) | u\bit(CO_R4USD) : 0)
       | ($mya->can(B_REFUND) ? u\bit(CO_UNDO) | u\bit(CO_REFUND) | u\bit(CO_USD4R) : 0)
       | ($mya->can(B_BUY) ? u\bit(CO_BUY) : 0)
       | ($mya->can(B_MANAGE) ? u\bit(CO_MANAGE) : 0);
  $agentCans = ($mya->coFlags >> APP_AGENT_CANS) & APP_CAN_MASK & $agentCan; // limit standard perms by agent
  $can = ($mya->coFlags >> APP_CANS) & APP_CAN_MASK;
  $can |= $agentCans << (APP_CAN_BITS + 1); // extra bit is for "manage" permission
  $bad = db\q("SELECT CONCAT(qid, ',', code) FROM r_bad")->fetchCol() ?: [];
  $balance = $mya->balance;
  return compact(ray('descriptions bad can balance'));
}

/**
 * Report if an argument is missing.
 */
function missingArg($expected, $args = []) {
  foreach (ray($expected) as $one) if (!isset($args[$one])) return "missing $one";
}

/**
 * Return the agent's company acct.
 * @param string $agent: the agent's company qid or company's uid (if company is pro se).
 * @param string $err: (returned) error message, if any
 * @return: the agent's company acct object (false if error)
 */
function getAgent($agent, &$err = '') {
//  $agent = preg_match('/[\\.:]/', $agent0) ? $agent0 : r\qo::qr2($agent0);
  if (!$mya = r\acct($agent)) return !$err = 'bad agent'; // . ": $agent";
//  if (r\foreignServer(r\uidRegion($mya->agentId))) return !$err = 'wrong server for agent';
  if (!$mya->ok) return !$err = 'company inactive';
//  if (!$mya->can(B_SELL)) return !$err = 'agent cannot sell';
  r\Acct::setDefault($mya);
  return $mya;
}

/**
 * Return the member's acct (the agent's account if signing in).
 * @param string $qid: the customer's qid.
 * @param string $code: hash of the customer's card code (formerly the card code itself)
 * @param string $err: (returned) error message, if any
 * @param bool $requireCode: require card code (FALSE for transactions)
 * @return: the customer's acct object (false if error)
 */
function getMember($qid, $code, &$err = '', $requireCode = TRUE) {
/*  if (substr($customer, -1, 1) == ':') { // invitation card -- get real account
    if (!$inviter = r\iCardAcct($code, $seq)) return !$err = 'bad member';
    if (!$uid = db\get('uid', 'users', 'helper=:inviter AND seq=:seq', compact('inviter', 'seq'))) return !$err = 'bad icard';
    $a = r\acct($uid);
    if (time() - $a->activated > ICARD_EXPIRES * DAY_SECS) return !$err = 'expired icard';
    $code = $a->cardCode; // no need for function here
  } else { */
//    $customer = preg_match('/[\\.:]/', $customer0) ? $customer0 : r\qo::qr2($customer0);

  if (!$a = r\acct($qid) or ($requireCode and $code != $a->cardCode() and $code != $a->codeHash())) {
    r\badCard($qid, $code, $err = 'bad member'); 
    return FALSE;
  }
//  }
//  if (r\foreignServer(r\uidRegion($a->agentId))) return !$err = 'wrong server for customer';
  if (!$a->ok) {r\badCard($qid, $code, $err = 'member inactive'); return FALSE;}
//  if (!$a->can(B_BUY)) return !$err = 'customer cannot buy';
  return $a;
  
//    if (!$smartInfo = db\get('owner,defaultAgent,transientId,access', 'r_boxes', 'code=:q_code', compact('q_code'))) return be\error('unknown device');

/*    if ($account_id != $q_account_id) { // this is a transient id scanned from the other party's smart device
      $transientId = basename(dirname($q_account_id));
      $recent = r\rTime() - R_TRANSIENCE;
      if (db\get('transientId', 'r_boxes', 'code=:code AND access>=:recent') != $transientId) return be\error('bad transient id');
    } */
}

/**
 * Return the balance and rewards.
 * @param acct $a: the customer account
 * @return: assoc [balance, rewards]
 */
function balrew($a) {
  global $mya;
  if (!$mya->co) $a = $mya;
  $balance = (($a->can(B_SECRET) and $mya->co) ? '*' : '') . round($a->balance - $a->partRounds, 2);
//  $rewards = $a->rewards;
  $rewards = -$a->floor;
  return compact('balance', 'rewards');
}

/**
 * Build a list of the account's most popular charge descriptions, beginning with the default.
 * @params acct $a: the company account
 * @return array of descriptions
 */
function getDescriptions($a) {
  $ray = [];
  //if ($default = $a->description) $ray[] = $default;
  $sql = 'SELECT payeeFor, COUNT( payeeFor )  FROM r_txs WHERE payee=:id GROUP BY payeeFor ORDER BY COUNT(payeeFor) DESC';
  return $ray + (db\q($sql, ray('id', $a->id))->fetchCol() ?: []);
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
  return db\get('id', 'r_boxes', 'code=:code AND uid=:uid', compact('code', 'uid'));
}

/**
 * Handle the data requested and sent from the rPOS app on an account's device.
 * @param string $data: optional json-encoded data (perhaps data we requested from the app)
 * @return string $message: what extra to respond to the app's "time" request: another request for data or ''
 */
function handleAppMsg($data0) {
  global $mya;
  if (!$mya) return '';
  
  $get = $mya->getAppData ?: '';
// NO! this would preclude app-initiated reporting  if (!$get = $mya->getAppData) return '';
  $appData = $mya->appData ?: [];
  if ($data0) {
    if ($getRay = !u\starts($get, '!delete:')) {
      if (is_object($obj = json_decode($data0)) and $obj) {
        $data = (array) $obj;
        ksort($data);
/*        if (is_array(array_shift($data))) {
          foreach ($data as $k => $v) if (u\starts($v, '{') and $ray = json_decode($v)) $data[$v] = $ray;
        } */
      } else $data = $data0;
/**/    $dpy = print_r($data, 1);
    } else list ($data, $dpy) = [$data0, str_replace("'", "\\'", str_replace('\\"', '"', $data0))];
    r\tellAdmin(t('got App Data'), ray('who get zdata', $mya->fullName, $get, $dpy));
    $appData[$get ?: '!report'] = $data;
//    if ($get) $get = $getRay ? (u\nextAfter($get, ray(R_APP_DATA_GETS)) ?: '') : ''; // '' if none (eg for !report)
    $get = ''; // don't request anything automatically
    $mya->update('getAppData appData', $get, $appData);
  }
  return $get;
}
