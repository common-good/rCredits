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
  $acct = new r\acct($info);
  return $acct ? $uid : FALSE;
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
  u\expect(!r\is_foreign($main->region) or !r\is_foreign($agent->region));
  u\expect($main->proSe() and $agent->proSe());
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
  
function uidHasR($uid) {
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

function findMessage($type, $fields, $message, $subject = '') {
  global $cuid, $channel;
  if (!TEST_REAL) {
    $map = array('ransaction #' => 'ransaction #a', 'nvoice #' => 'nvoice #a', '@asif' => R_ASIF_TEXT, '{ASIF}' => R_ASIF_TEXT);
    $message = strtr($message, $map);
    $subject = strtr($subject, $map);
  }
  $agent = r\acct()->agent;
  $crit = "channel=:channel AND type=:type AND cuid=:cuid AND agent=:agent ORDER BY logid DESC";
//  if (strpos($message, '@nonce')) $message = str_replace('@nonce', r\dbLookup('special', 'r_log', $crit, compact('type')), $message);
  $message = str_replace("\n", '~', $message);
  debug($message ?: (@$fields['message'] ?: $fields)); // keep this
  $extras = ($subject ? 'subject ' : '') . ($message ? 'message' : '');
  $info = json_encode2($fields + compact(u\ray($extras)), JSON_UNESCAPED_SLASHES);
  //debug("info=:info AND $crit"); debug(compact(u\ray('info type cuid agent')));
  return r\dbLookup(1, 'r_log', "info=:info AND $crit", compact(u\ray('info type cuid agent channel')));
}

function hitServer($op, $my_id, $code = '', $account_id = '', $extra = array()) {
  if (!$code) unset($code);
  if (!$account_id) unset($account_id);
  $json = json_encode2(compact(u\ray('op my_id code account_id') + $extra), JSON_UNESCAPED_SLASHES);
  r\Smart\api(compact('json'));
//  api\send($json); // remember the server's response (this is the real test, once most bugs are gone)
  return TRUE;
}

function eq($a, $b) {
  $result = (($a - $b) * ($a - $b) < .001);
  if (!$result) debug("unequal: a=$a b=$b");
  return $result;
}

function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == '.' or $c1 == ':') ? (R_SERVER_ID . $q) : $q;
}