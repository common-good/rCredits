<?php
/**
 * @file
 * Smart phone interface to the rCredits system.
 *
 * Coding conventions:
 *   $cacct is the current account
 *   $acct is some other account
 *
 * @todo: return "stay" param, meaning "stay on current screen" (for some errors, such as missing description). Better might be to include those checks in the app, so the app returns to main menu always after talking to the server, whether successful or not. Or supply a "try again" button.
 * @todo:
 
 http://code.google.com/p/zxing/issues/detail?id=1337
 On my cheap tablet (Allwinner 7401), which does not have a rear-facing camera, the app uses the front-facing camera.  The app displays the camera output fine (albeit with the incorrect aspect ratio).  But is unable to read QR codes at all, even though the yellow dots indicate that the corners are being correctly detected.

However if I first flip/mirror the QR code, then the app has no problems reading the QR code.

I am guessing that the front-facing camera on this device flips the image causing QR codes to be read incorrectly.  I am unsure whether other devices also flip the front-facing camera image.
Comment 1 by project member srowen, Aug 15, 2012
A correctly-behaved Android device will feed image data fro the front camera to the app as it sees it -- as from the perspective of someone facing you. The image it flings onto a SurfaceView during preview is, however, flipped over to make it act like a mirror, which is more intuitive.

From a sample size of 4 devices I've used directly, that's how they all seem to work too.

You are right that a flipped-over QR code can't be decoded, but this particular problem is a problem with the device, yes.

I am guessing this won't be the last device with an issue like this. One I own has a different problem, reporting the front camera orientation incorrectly. I'll keep this open as a placeholder to maybe implement an advanced setting to re-flip the image data for a device like this.
Summary: Add workaround for devices that flip over image data
Labels: Type-Enhancement
Comment 2 by mes...@gmail.com, Aug 15, 2012
Awesome, I look forward to it.
Comment 3 by project member srowen, Oct 24 (5 days ago)
I think this is a niche enough issue that I don't want to implement a whole new setting for it, and I think Daniel would feel even more strongly on that point. However there is in the code base a fairly easy change to flip over the image data. You can hard-code it or add a setting into your own build.

Find where PlanarYUVLuminanceSource is instantiated in CameraManager. Its last parameter is "false". Just change it to "true" and it will flip over the image.

 */

namespace rCredits\Smart;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Backend as be;

define('R_QR_HEADER', 'HTTP://RC2.ME/I/'); // for transient QR IDs
define('R_TRANSIENCE', 60); // number of seconds for device's QR to be valid

$GLOBALS[TX_SMART . ' messages'] = array(
  'first time' => t('You have successfully registered this device for use with rCredits!'),
);

/**
 * Receive and process a REST request from a smart device
 * @param assoc $args: an associative array (defaults $_POST) of a json-encoded assoc that normally includes:
 *   op: what to do (determines what function to call)
 *   my_id: agent qid
 *   code: permanent code that we sent to the device in First Time response (used to set default $cuid)
 *     if $op = 'first_time', expect $password instead
 *   position: (optional) latitude,longitude,elevation (from the device's GPS)
 *   account_id: the qid for the other party, if any 
 *     This gets passed to the specific op functions as $acct
 * @return (normally by calling either be\error or doOp):
 *   $success=1 if doOp is called, otherwise 0 ($op = 'photo' is an exception; see below)
 *   $message: an error message or help message to display on the device; or not set
 */
function api($args = FALSE) {
  global $channel; $channel = TX_SMART;
  if (!$args) $args = $_POST;
  u\log('in', $args);
  if (!(@$args = (array) json_decode($args['json']))) return be\error('no json');
  foreach ($args as $key => $value) $args[$key] = trim($value); // allow some sloppy spaces around each arg
  extract($args, EXTR_PREFIX_ALL, 'q');
  
  // standard error checking and setup, except for first time
  if (@$q_op != 'reset') // DEBUG
  if (@$q_op != 'first_time') {
    if (!@$q_code) return be\error('no code given');
    if (!($smartInfo = r\dbLookup('owner,default_agent,transient_id,accessed', 'r_smarts', 'code=:q_code', compact('q_code')))) return be\error('unknown device');
    $args += $smartInfo; // pass these on to the op function
    extract($smartInfo, EXTR_PREFIX_ALL, 'q');
    r\acct::setDefault(r\acct($q_owner)); // set current account temporarily to owner, so that error log has it right
    if (@$q_op != 'startup') { // no my_id on startup (it gets returned instead)
      if (!u\isQid($q_my_id)) return be\error('bad id format');
      if (!($cacct = r\acct($q_my_id, '', $error))) return be\error($error);
    } else if (!($cacct = r\acct($q_owner))) return be\error('bad code owner'); // this should never happen
    if ($cacct->region != r\serverUid()) return be\error('wrong server');
    if ($cacct->agentRegion != r\serverUid()) return be\error('wrong server for agent');
    if (buttons($cacct, $q_owner) < 0) return be\error('no permission');
    r\acct::setDefault($cacct);
    $args['cacct'] = $cacct;
    $edit = array('code' => $q_code, 'accessed' => time());
    \drupal_write_record('r_smarts', $edit, 'code'); // remember time last accessed so we can drop the deadwood
  }

  // handle scanned QR, if any
  if (@$q_account_id) {
    if (!u\isQid($account_id = basename(@$q_account_id))) return be\error('bad account id');
    if ($account_id != $q_account_id) { // this is a transient id scanned from the other party's smart device
      $transient_id = basename(dirname($q_account_id));
      $recent = time() - R_TRANSIENCE;
      if (dbLookup('transient_id', 'r_smarts', 'code=:code AND accessed>=:recent') != $transient_id) return be\error('bad transient id');
    }
    if (!($acct = r\acct($account_id, '', $error))) return be\error($error);
    $args['acct'] = $acct;
  }
  
  return be\doOp(@$q_op, $args, 'Smart');
}

/*function reset($args) {
  r\dbQ('DELETE FROM r_smarts WHERE 1');
  return array();
}*/

/*
 * Requesting a permanent code from the server.
 * @param array $args:
 *   my_id
 *   password: the user's Drupal account password
 * @return:
 *   code: permanent code for app to prove the identity of the device
 *   owner_id: the device owner's account ID
 *   update_link: URL of updated app; not set if no update is available
 */
function first_time($args) {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  extract(u\just('my_id password', $args));
  if (!u\isQid(@$my_id)) return be\error('bad id');
  if (!($cacct = r\acct(@$my_id))) return be\error('unknown member');
  if (!$cacct->proSe()) $cacct = r\acct($cacct->id); // use the pure account
  r\acct::setDefault($cacct);
  $args['cacct'] = $cacct;
  if (@!$password) return be\error('login error');
  if (!user_check_password($password, $cacct->account())) return be\error('bad login'); // make these two error messages distinct but obscure
  if (!($code = makeDevice(t('Smart Device'), $cacct))) return be\error("can't save device info");
  $args['owner'] = $cacct->id;
  $message = u\tt('first time');
  // also update_link allow_change_account allow_change_agent
  return compact('message', 'code') + startup($args); // testing counts on this arg order
}

/*
 * Respond to the application being run.
 * @param array $args: (none)
 * @return:
 *   update_link (URL of updated app; not set if no update is available)
 */
function startup($args = '') {
  extract(u\just('cacct owner', $args));
//  if (rand() < getrandmax() / 2) $update_link = 'http:/rCredits.org/files/smart.apk';
  $my_id = $cacct->qid();
  $account_name = $cacct->full_name;
  $allow_change_account = 0;
  $allow_change_agent = 1;
  $require_agent = $cacct->can(PERM_COMPANY) ? 1 : 0;
  $show_buttons = buttons($cacct, $owner);
  return compact(u\ray('my_id account_name update_link allow_change_account allow_change_agent require_agent show_buttons')); // standard fields to return on startup);
}

/*
 * Identify the other party.
 * @param array $args:
 *   account_id: the qid for an individual or company and rep
 * @return:
 *   full_name: the individual's name
 *   company_name: the company name (if any)
 *   location: the location of the company if it's a company, otherwise of the individual (typically city, state)
 *   other_balance: the current balance of the company if it's a company, otherwise of the individual
 */
function identify($args) {
  $cacct = r\acct();
  extract(u\just('acct', $args));
  if (!@$acct) return be\error('missing acct');
  $full_name = $acct->full_name;
  if ($acct->id == $cacct->id) return be\error('no self-trading');
  $location = u\location($acct);
  if (!$acct->proSe()) list ($company_name, $full_name) = array($full_name, r\acct($acct->agent)->full_name);
  $uid = $acct->id;
  if ($cacct->can(PERM_CHARGE)) $other_balance = be\creditInfo(compact('uid'))->balance;
  return compact(u\ray('full_name location company_name other_balance'));
}

/*
 * Get an individual's photo.
 * @param array $args:
 *   acct: the qid for an individual or company and rep
 * @return:
 *   just the photo data (no json)
 */
function photo($args) {
  global $base_url;
  extract(u\just('acct', $args));
  $pic_filename = r\Web\profile_picture(@$acct->id);
  $default = "$base_url/sites/default/files/pictures/no-photo-available.jpg";
  $pic = file_get_contents($pic_filename ?: $default);
  if (!$pic) $pic = file_get_contents($default);
  $message = bin2hex(substr($pic, 0, 50));
  u\log('out', compact('message'));

  if (@TESTING == 1) {debug($message); return $message;} else exit($pic);
}

/*
 * Request a transaction.
 * @param array $args:
 *   account_id (converted to uid or company_uid & rep_id): the qid for an individual or company and rep
 *   type: “charge” or “payment”
 *   amount: numeric dollar amount
 *   goods: true unless user checks “cash, loan, etc.”
 *   purpose: description of goods and services
 * @return:
 *   tx_id: transaction ID number, if success, otherwise empty string
 *   my_balance: device owner's new balance
 *   other_balance: the new balance of the company if it's a company, otherwise of the individual
 */
function transact($args) {
  extract($args, EXTR_PREFIX_ALL, 'q');
  if (!@$q_type) return be\error('missing transaction type');
  if ($q_type != 'pay' and $q_type != 'charge') return be\error('bad transaction type');
  if (!@$q_account_id) return be\error('missing account id');
  if (!@$q_amount) return be\error('missing amount');
  if ($error = u\badAmount($q_amount, TRUE)) return be\error($error);
  if (@$q_goods and !@$q_purpose) return be\error('missing purpose');

  if ($q_type == 'pay') $q_type = 'payment'; // this is what be\transfer() expects
  list ($index, $subs, $zot) = be\transfer($q_type, $q_acct, $q_amount, $q_purpose, $q_goods, TRUE);

  $balances = balances($q_acct->id, @$subs['real']); // my_balance, other_balance
  if (!($subs['success'] = (int) @$subs['success'])) return be\error($index, $subs + $balances);
  $tx_id = r\qxid($subs['zxid']); // return the transaction id as a qid (globally unique transaction id) (':' means As-If)
  $message = u\tt($index, $subs);
  return compact(u\ray('message tx_id')) + $balances;
}

/*
 * Undo the specified transaction (perhaps tentatively)
 * @param array $args:
 *   account_id: the qid for an individual or company and rep
 *   tx_id: qid of the transaction to undo
 *   confirmed: TRUE if user has already okayed this undo
 * @return (only if confirmed is TRUE):
 *   tx_id: qid of offsetting transaction, if any (which could in turn be undone). Undefined if failure. 
 *     Otherwise, tx_id not set means transaction was simply deleted, so there is no longer any transaction that can be undone.)
 *   my_balance: device owner's new balance
 *   other_balance: the new balance of the company if it's a company, otherwise of the individual
 */
function undo($params) {
  extract(u\just('tx_id confirmed', $params));
  
  if (!@$tx_id) return be\error('missing transaction id');
  if (!u\isQid(@$tx_id)) return be\error('bad transaction id');
  list ($zxid, $regionUid) = r\unQid($tx_id);
  if ($regionUid != r\serverUid()) return be\error('That is not a transaction on this server.');
  if (@$confirmed !== '1' and @$confirmed !== '0') return be\error('bad confirmation status');
  
  unset($tx_id); // done with the old transaction id
  if ($confirmed) {
    list ($message, $args) = be\undoTx($zxid);
    $message = u\tt($message, $args);
    $balances = balances(@$args['otherUid'], $zxid > 0);
    if (!($success = (int) @$args['success'])) return compact(u\ray('success message')) + (@$balances ?: array());
    if ($new_zxid = @$args['zxid']) $tx_id = r\qxid($new_zxid); // get offsetting transaction id, if any (otherwise state changed)
  } else {
    $oldRecord = be\lastTx('xid=:zxid', compact('zxid'));
    if ($error = r\nonoUndo($oldRecord, $subs)) return be\error($error, $subs); // check nono before confirm AND here
    $message = u\tt('confirm undo', r\txReportArgs($oldRecord));
  }
  return compact(u\ray('message tx_id')) + (@$balances ?: array());
}

function change($params) {
  extract(u\just('cacct acct what', $params));
  if (!@$acct) return be\error('missing account_id');
  if (!in_array($what, u\ray('agent account'))) return be\error('bad what');
  if ($acct->proSe()) $acct = $what == 'agent' ? r\acct($cacct->id, $acct->id) : r\acct($acct->id, $cacct->agent);
  if (($show_buttons = buttons($acct, $params['owner'])) < 0) return be\error('no permission');

  $account_name = $accountName = $acct->full_name;
  $agentName = r\acct($acct->agent)->full_name;
  $message = u\tt('changed ' . $what, compact(u\ray('what agentName accountName')));

  $my_id = $acct->qid();
  $account_name .= ($acct->proSe() ? '' : "\nAgent: $agentName");
  return compact(u\ray('message my_id account_name show_buttons'));
}

function display_id($params) {
  extract(u\just('cacct code', $params));
  $transient_id = R_QR_HEADER . u\n2a(mt_rand(), 20) . '/' . $cacct->qid();
  $edit = compact('code', 'transient_id');
  \drupal_write_record('r_smarts', $edit, 'code'); // remember what id to accept (briefly)
  $message = '';
  return compact('message', 'transient_id');
}

function makeDevice($device, $acct, $code = '') {
  u\EXPECT(compact('device', 'acct'), 'string&notempty acct');
  $owner = $acct->id;
  $code = $code ?: user_hash_password(uniqid($acct->mainQid, TRUE) . time()) . '/'; // add a slash to make sure slashes get handled properly
  $accessed = time();
  $info = compact(u\ray('owner code device accessed'));
  return @\drupal_write_record('r_smarts', $info) ? $code : FALSE; // no way to tell if it's the same device as others
}

function agentButtons($acct) {
  return $acct->can(PERM_BUY) ? 3
      : ($acct->can(PERM_SELL) ? 1 
      : ($acct->can(PERM_READ) ? 0
      : (-1 )));
}

function buttons($acct, $owner) {return min(agentButtons(r\acct($acct->id, $owner)), agentButtons($acct));}
  
function accountName($acct) {
  $agentName = $acct->proSe() ? '' : "\nAgent: " . r\acct($acct->agent)->full_name; 
  return $acct->full_name . $agentName;
}

function balances($uid, $real) {
  if (r\acct()->can(PERM_MANAGE)) $my_balance = be\creditInfo(compact('real'))->fancy['balance'];
  if ($uid) {
    $other_balance = r\otherBalance($uid);
    if (!$other_balance) unset($other_balance); // don't return an empty parameter
  }
  return compact('my_balance', 'other_balance');
}

