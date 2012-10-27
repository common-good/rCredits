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
  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); }
  extract($info, EXTR_PREFIX_ALL, 'my');

  if (!@$my_id) $my_id = r\acct::nextId(); // default to first available number
  $uid = uid($my_id); // change .AAB to 1, etc.
  $rebate_percent = isset($my_rebate) ? $my_rebate: (R_REBATE * 100); 
  $full_name = @$my_full_name ?: ('makeAccount ' . randomString(20, 'A'));
  $name = u\short_name($full_name);
  $mail = @$my_email ?: "$name@example.com";
  $flags = isset($my_flags) ? $my_flags : (TEST_REAL ? BIT_RTRADER : 0);
  $community_uid = @$my_community_uid ?: r\server_uid();
  
  $info = array_merge($info, compact(u\ray('uid rebate_percent name full_name mail community_uid')));
  if (!($acct = new r\acct($info))) return FALSE;
  
  if (@$my_picture) if (!makePicture($my_picture, $acct)) return FALSE;

  return $uid;
}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 */
function makeTransaction($info, $real = TEST_REAL) {
  global $channel;
  extract($info, EXTR_PREFIX_ALL, 'my');

  $type = TX_TRANSFER; // the default
  $state = TX_DONE;
  $created = @$my_created ? strtotime($my_created) : time();  unset($info['created']);
  
  $payer = uid($my_from);
  $payer_agent = uid(@$my_from_agent ?: $my_from);
  $payee = uid($my_to); 
  $payee_agent = uid(@$my_to_agent ?: $my_to);
  $payer_for = @$my_purpose ?: 'cash';
  $payee_for = @$my_purpose ?: 'cash';
  $goods = u\for_cash($payer_for) ? 0 : 1;
  $taking = ($my_from == 'community' or @$my_type == 'payment') ? 0 : 1;
  if ($goods) {
    $rebate = r\rebate($payer, $my_amount);
    $bonus = r\bonus($payee, $my_amount);
    $data = serialize(compact(u\ray('rebate bonus')));
  }
  $info += compact(u\ray('created payer payer_agent payee payee_agent payer_for payee_for type channel goods taking state data'));
  \drupal_write_record($real ? 'r_txs' : 'r_asif', $info);
  $serial = $xid = $info['xid'];
  r\db_q('UPDATE {zxs} SET serial=:serial WHERE xid=:xid', compact(u\ray('serial xid real')));
  return TRUE;
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
  u\EXPECT(!r\is_foreign($main->region) or !r\is_foreign($agent->region));
  u\EXPECT($main->proSe() and $agent->proSe());
  list ($main_uid, $agent_uid) = array($main->id, $agent->id);
  if (r\is_foreign($main->region)) list ($main_uid, $foreign_uid) = array($main->region, $main->id);
  if (r\is_foreign($agent->region)) list ($agent_uid, $foreign_uid) = array($agent->region, $agent->id);
  
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
  return api\credit_info(compact('uid', 'real'));
}

function uid($id) {
  return is_numeric($id) ? 
      $id 
    : ($id == 'community' ? 
        r\server_uid() 
    : u\a2n(substr(str_replace(R_SERVER_ID . '.', '.', $id), 1) 
  ));
}

function findMessage($type, $fields, $message = '', $subject = '', $subs = array()) {
  global $channel;

  $subs = @$subs[0] ?: @$subs;
  $cuid = r\acct()->id;
  extract($fields, EXTR_PREFIX_ALL, 'my');
  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $message = str_replace(PHP_EOL, '~', $message);
    if (!TEST_REAL) {
      $map = array('ransaction #' => 'ransaction #a', 'nvoice #' => 'nvoice #a', '@asif' => R_ASIF_TEXT, '{ASIF}' => R_ASIF_TEXT);
      $message = strtr($message, $map);
      $subject = strtr($subject, $map);
    }
    if ($subject) $fields['subject'] = $subject;
    $fields['message'] = $message;
  } else unset($fields['message']); // empty message in features means NO message
  $agent = r\acct()->agent;
  $crit = "channel=:channel AND type=:type AND cuid=:cuid AND agent=:agent ORDER BY logid DESC";
  
  $info = json_encode2($fields, JSON_UNESCAPED_SLASHES);
  debug($info); // keep this
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
  $message = strtr(r\email_body($key), $esubs);
  $subject = strtr($GLOBALS['email_subjects'][$key], $esubs);
  return findMessage('email', compact(u\ray('key email')), $message, $subject);
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

function hitServer($op, $my_id, $code = '', $account_id = '', $extra = array()) {
  if (!$code) unset($code);
  if (!$account_id) unset($account_id);
  $json = json_encode2(compact(u\ray('op my_id code account_id')) + $extra, JSON_UNESCAPED_SLASHES);
  debug($json); // keep this
  r\Smart\api(compact('json'));
//  api\send($json); // remember the server's response (this is the real test, once most bugs are gone)
  return TRUE;
}

function eq($a, $b) {
print_r(compact('a','b'));
  $result = (($a - $b) * ($a - $b) < .001);
  if (!$result) debug("unequal: a=$a b=$b");
  return $result;
}

function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == '.' or $c1 == ':') ? (R_SERVER_ID . $q) : $q;
}