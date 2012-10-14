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

function makeAccount($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $uid = uid(@$my_id, 1); unset($info['id']); // change .AAB to 1, etc.
  $rebate_percent = $my_rebate; unset($info['rebate']);
  $name = u\short_name($my_full_name);
  $flags = isset($my_flags) ? $my_flags : (TEST_REAL ? BIT_RTRADER : 0);
  $info += compact(u\ray('uid rebate_percent name'));
  \drupal_write_record('users', $info);
  if (@$my_phone and !@$my_cell) {
    list ($uid, $number, $status) = array($uid, $my_phone, SMS_OK);
    drupal_write_record('r_sms', compact(u\ray('uid number status')));
  }
}

function makeTransaction($info, $real = TEST_REAL) {
  global $channel;
  extract($info, EXTR_PREFIX_ALL, 'my');

  $type = TX_TRANSFER; // the default
  $state = TX_DONE;
  $created = strtotime(@$my_date); unset($info['date']);
  $payer = uid($my_from); unset($info['from']);
  $payer_agent = uid(@$my_from_agent ?: $my_from);
  $payee = uid($my_to); unset($info['to']);
  $payee_agent = uid(@$my_to_agent ?: $my_to);
  $payer_for = @$my_for ?: 'cash';
  $payee_for = @$my_for ?: 'cash';
  unset($info['for']);
  $goods = $payer_for == 'cash' ? 0 : 1;
  $taking = ($my_from == 'community' or @$my_type == 'payment') ? 0 : 1;
  if ($goods) {
    $rebate = r\rebate($payer, $my_amount);
    $bonus = r\bonus($payee, $my_amount);
    $data = compact(u\ray('rebate bonus'));
  }
  $info += compact(u\ray('created payer payer_agent payee payee_agent payer_for payee_for type channel goods taking state data'));
  \drupal_write_record($real ? 'r_txs' : 'r_asif', $info);
  $serial = $xid = $info['xid'];
  r\db_q('UPDATE {zxs} SET serial=:serial WHERE xid=:xid', compact(u\ray('serial xid real')));
}

function uidHasR($uid, $amount) {
  $real = TEST_REAL;
  return (api\credit_info(compact('uid', 'real'))->balance == $amount);
}

function uid($id) {return $id == 'community' ? r\server_uid() : u\a2n(substr($id, 1));}
