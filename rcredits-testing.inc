<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace rCredits\Testing;
use rCredits as r;
use rCredits\Util as u;
use rCredits\API as api;

define('TEST_REAL', FALSE);

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function makeAccount($info = array()) {
  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info, EXTR_PREFIX_ALL, 'my');

  if (!@$my_id) $my_id = r\acct::nextId(); // default to first available number
  $uid = uid($my_id); // change .AAB to 1, etc.
  $rebate_percent = isset($my_rebate) ? $my_rebate: (R_REBATE * 100); 
  $full_name = @$my_full_name ?: ('makeAccount ' . randomString(20, 'A'));
  $name = u\shortName($full_name);
  $mail = @$my_email ?: "$name@example.com";
  $flags = isset($my_flags) ? $my_flags : (BIT_DEFAULTS | (TEST_REAL ? BIT_RTRADER : 0));
  $community_uid = @$my_communityUid ?: r\serverUid();
  u\EXPECT(!empty($community_uid), 'empty community uid');
  
  $info = array_merge($info, compact(u\ray('uid flags rebate_percent name full_name mail community_uid')));
  if (!($acct = new r\acct($info))) return FALSE;
  
  if (@$my_picture) if (!makePicture($my_picture, $acct)) return FALSE;

  return $uid;
}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if called with $infoOnly TRUE, return assoc with the information actually written (called from verifyTx())
 *   otherwise return serial (treated as TRUE in test steps)
 */
function makeTransaction($info, $infoOnly = FALSE, $real = TEST_REAL) {
  global $channel;
  extract($info, EXTR_PREFIX_ALL, 'my');

  if (@$my_tx_id) $xid = u\a2n(substr($my_tx_id, 4));
  $type = isset($my_type) ? $my_type : TX_TRANSFER;
  $state = TX_DONE;
  $created = @$my_created ? strtotime($my_created) : time();
  
  $payer = uid($my_from);
  $payer_agent = uid(@$my_from_agent ?: ($type == TX_TRANSFER ? $my_from : r\communityUid()));
  $payee = uid($my_to); 
  $payee_agent = uid(@$my_to_agent ?: ($type == TX_TRANSFER ? $my_to : r\communityUid()));
  $payer_for = @$my_purpose ?: 'cash';
  $payee_for = @$my_purpose ?: 'cash';
  $goods = @$my_goods ?: (($type != TX_TRANSFER or u\forCash($payer_for)) ? 0 : 1);
  $taking = ($my_from == 'community' or @$my_type == 'payment') ? 0 : 1;
  if ($goods) {
    $rebate = r\rebate($payer, $my_amount);
    $bonus = r\bonus($payee, $my_amount);
    $data = serialize(compact(u\ray('rebate bonus')));
  }
  foreach (u\ray('created from to purpose tx_id') as $one) unset($info[$one]);
  $info += compact(u\ray('xid created payer payer_agent payee payee_agent payer_for payee_for type channel goods taking state data'));
  if ($infoOnly) return $info;
  
  \drupal_write_record($real ? 'r_txs' : 'r_asif', $info);
  if (!@$my_serial) {
    $serial = $xid = $info['xid'];
    r\dbQ('UPDATE {zxs} SET serial=:serial WHERE xid=:xid', compact(u\ray('serial xid real')));
  } else $serial = $my_serial;
  return $serial;
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function makeRelation($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $reid = u\a2n(substr($my_id, -3));
  $main = r\acct(fullQid($my_main));
  $agent = r\acct(fullQid($my_agent));
  u\EXPECT((bool) $main, 'main is not an id');
  u\EXPECT((bool) $agent, 'agent is not an id');
  u\EXPECT(!r\isForeign($main->region) or !r\isForeign($agent->region));
  u\EXPECT($main->proSe() and $agent->proSe());
  list ($main_uid, $agent_uid) = array($main->id, $agent->id);
  if (r\isForeign($main->region)) list ($main_uid, $foreign_uid) = array($main->region, $main->id);
  if (r\isForeign($agent->region)) list ($agent_uid, $foreign_uid) = array($agent->region, $agent->id);
  
  if (@$my_permission) {
    $perms = array_flip($GLOBALS['share permissions']);
    $info['permission'] = $perms[$my_permission]; 
  }
  $info += compact(u\ray('reid main_uid agent_uid foreign_uid'));
  return \drupal_write_record('r_relations', $info);
}

/**
 * Return the credit info for the given uid.
 * To get just the balance, use uidCredit(uid)->balance
 */
function uidCredit($uid) {
  $real = TEST_REAL;
  return api\creditInfo(compact('uid', 'real'));
}

function uid($id) {
  return is_numeric($id) ? 
      $id 
    : ($id == 'community' ? 
        r\serverUid() 
    : u\a2n(substr(str_replace(R_SERVER_ID . '.', '.', $id), 1) 
  ));
}

function findMessage($type, $fields, $message = '', $subject = '', $subs = array()) {
  global $channel;

  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $cuid = r\acct()->id;
  extract($fields, EXTR_PREFIX_ALL, 'my');
  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $fields['message'] = str_replace("\r\n", '', realFix($message));
    if ($subject) $fields['subject'] = realFix($subject);
  } else unset($fields['message']); // empty message in features means NO message
  $agent = r\acct()->agent;
  \drupal_set_message(color($fields, @$subject ? 'email' : 'output')); // keep this
  $info = json_encode2($fields, JSON_UNESCAPED_SLASHES);
  $crit = "channel=:channel AND type=:type AND cuid=:cuid AND agent=:agent ORDER BY logid DESC";
  //debug("info=:info AND $crit"); debug(compact(u\ray('info type cuid agent')));
  return r\dbLookup(1, 'r_log', "info=:info AND $crit", compact(u\ray('info type cuid agent channel')));
}

function findEmail($key, $email, $subs) {
  $subs = $subs[0];
  foreach (array('payer', 'payee') as $role) {
    if (($purpose = @$subs[$role . '_purpose']) and !TEST_REAL) $subs[$role . '_purpose'] = str_replace('#', '#a', $purpose);
  }
  $esubs = array();
  foreach ($subs as $k => $value) $esubs[u\hug(strtoupper($k), '{}')] = $value;
  $message = strtr(r\emailBody($key), $esubs);
  $subject = strtr($GLOBALS['emailSubjects'][$key], $esubs);
  return findMessage('email', compact(u\ray('key email subject')), $message, $subject);
}

function makePicture($picture, $acct) {
  $filename = "$picture.jpg";
  $uri = "public://pictures/$filename";
  $filemime = 'image/jpeg';
  $status = 1;
  $picture = $uid = $fid = $acct->id;
  $info = compact(u\ray('fid uid filename uri filemime status'));
  if (!\drupal_write_record('file_managed', $info)) return FALSE;
  if (!$acct->update(compact('picture'))) return FALSE;
  return TRUE;
}

function getPicture($picture) {
  global $picturePath;
  return file_get_contents("$picturePath/$picture.jpg");
}

function lastMinuteSubs(&$string) {
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if ($string == '%chk') return ($string = '&#10004;');
  if ($string == '%ctty') return ($string = R_REGION_NAME);
}
  
  
function subAgo(&$string, $fmt) {
  if (!strpos($string, '-')) $string .= '-0d';
  $lastC = substr($string, strlen($string) - 1);
  list ($a, $b) = explode('-', $string);
  $bPreC = substr($b, 0, strlen($b) - 1);
  $periods = array('d' => 'days', 'w' => 'weeks', 'm' => 'months', 'y' => 'years');
  $period = $periods[$lastC];
//  debug(compact(u\ray('string fmt lastC a b bPreC period')));
  return ($string = strftime($fmt, strtotime("$bPreC $period ago")));
}

function hitServer($op, $agent, $code = '', $account_id = '', $extra = array()) {
  if (!$code) unset($code);
  if (!$account_id) unset($account_id); else $account_id = trim($account_id);
  if ($op != 'startup') { // startup RETURNS my_id rather than asking for it
    $agent = trim($agent);
    if (strpos($agent, ':') or !@$code) {
      $my_id = $agent;
    } else {
      $acct = r\acct(r\dbLookup('owner', 'r_smarts', 'code=:code', compact('code')), $agent);
      $my_id = $acct ? $acct->qid() : $agent; // accommodate bad agents
    }
  }
  $json = json_encode2(compact(u\ray('op my_id code account_id')) + $extra, JSON_UNESCAPED_SLASHES);
  \drupal_set_message(color($json, 'input')); // keep this
  r\Smart\api(compact('json'));
//  api\send($json); // remember the server's response (this is the real test, once most bugs are gone)
  return TRUE;
}

function logIn($id) {r\acct::setDefault(r\acct($id));}

function verifyTx($info) {
  $info2 = makeTransaction($info, TRUE);
  if (!TEST_REAL) $info2['real'] = 0;
  return verifyRecord('{zxs}', $info2);
}
           
function verifyRecord($table, $info) {
  unset($info['data']);
  unset($info['created']);
  foreach ($info as $key => $value) if (!($table == '{zxs}' and $key == 'real')) $crit[] = "$key=:$key";
  $crit = join(' AND ', $crit);
  return r\dbLookup(1, $table, $crit, $info);
}

function postProcess($form, $formName, $type) {
  $functions = @$form["#$type"];
  if (!@$functions or ((string) $functions) == '<') {
    $function = "rCredits\\Web\\{$formName}_form_$type";
    $functions = function_exists($function) ? array($function) : array();
  }
  return $functions;
}

function strip($form) {
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/', '$1', $form);
  $form = preg_replace('/> *</', '> <', $form); // make sure there's exactly one space between things
  return preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form));
}

function eq($a, $b) {
  $result = (($a - $b) * ($a - $b) < .001);
  if (!$result) print_r(compact('a','b')); // keep for now
  if (!$result) \drupal_set_message(color("unequal: a=$a b=$b", 'error')); // keep for now
  return $result;
}

function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == '.' or $c1 == ':') ? (R_SERVER_ID . $q) : $q;
}

function realFix($msg) {
  $map = array('ransaction #' => 'ransaction #a', 'nvoice #' => 'nvoice #a', '@asif' => R_ASIF_TEXT, '{ASIF}' => R_ASIF_TEXT);
  return TEST_REAL ? $msg : strtr($msg, $map);
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 */
function color($msg, $color) {
  $colors = array(
    'email' => 'lightblue',
    'input' => 'orange',
    'output' => 'thistle',
    'screen' => 'burlywood',
  );
  $color = @$colors[$color] ?: $color;
  $msg = print_r($msg, 1); // keep this
  return "<pre style='background-color:$color;'>$msg</pre>";
}

function testOutput($msg, $color) {if (defined('TESTING')) \drupal_set_message(color($msg, $color));}
