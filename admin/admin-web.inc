<?php
/**
 * @file
 * rCredits admin
 */
namespace rCredits\Web;

use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Testing as t;
use rCredits\Util as u;
use rCredits\Admin as a;

function seeSecure($a) {
  $secure = $a->secure;
  u\setDft($secure['usdType'], 'Dwolla');
  foreach (u\ray(R_SECURE_FIELDS) as $one) if (isset($secure[$one])) $new[$one] = $secure[$one]; // reorder
  $a->update(u\ray('secure', @$new ?: array()));
  say(u\rayTable($a->secure, FALSE));
}

/**
 * Change a transaction amount (only if the transaction happened today and only for a non-rewardable type).
 * @param int $xid: tx record ID
 * @param float $oldAmt: the old amount (for a consistency check)
 * @param float $newAmt: what the amount should be
 */
function changeTxAmt($xid, $oldAmt, $newAmt) {
  if (!$rec = db\lookup('payer,payee,amount,created,goods', 'r_txs', 'xid=:xid', compact('xid'))) return say(t('No such transaction.'), 'err');
  extract($rec);
  if ($amount != $oldAmt) return say(t('The existing amount of that transaction is different.'), 'err');
  if (u\fmtDate($created) != u\fmtDate(REQUEST_TIME)) return say(t('That transaction is not from today.'), 'err');
  if ($goods) return say(t('That transaction has rewards.'), 'err');
  $DBTX = \db_transaction();
  if (!$amount = round($newAmt, 2)) return say(t('Bad new amount.'));
  db\update('r_txs', compact('xid', 'amount'), 'xid');
  $change = $newAmt - $oldAmt;
  if ($payee > 0) db\q('UPDATE users SET r=r+:change WHERE uid=:payee', compact('change', 'payee'));
  if ($payer > 0) db\q('UPDATE users SET r=r-:change WHERE uid=:payer', compact('change', 'payer'));
  say(t('done'));
  unset($DBTX);
}

/**
 * Return Summary fields exclusively for community admins (admin, cAdmin, or cAdmin2).
 */
function adminSummary($a, $cAdmin = FALSE) {
  global $base_url;
  include_once __DIR__ . '/../admin/admin.inc';  
  $mya = r\acct();
  $uid = $a->id;
  $admin = $mya->admin;
  $self = ($uid == $mya->agent->id);
  
  if ($a->ctty) { // managing the community itself
    $ctty = $a->community;
    $count = item(db\recCount('users', 'community=:ctty AND uid<>:ctty', compact('ctty')), t('Members:'));
  } else { // managing some individual community member account
    $a->nextStep(); // update Dwolla status
    if ($cAdmin) {
      $donations = db\lookup($a->ok ? 'SUM(amount)' : 'amount', 'r_gifts', 'uid=:uid', compact('uid')) ?: 0;
      if ($donationInfo = $a->ok ? '' : db\lookup('honor,honored,share', 'r_gifts', 'uid=:uid', compact('uid'))) {
        extract($donationInfo);
        $donationInfo = " ($honor) $honored $share%";
      }
      $donations = item(u\fmtAmt($donations) . @$donationInfo, t('Donations:'));

      $floor = textField(t('Credit floor:'), '', dft(u\fmtAmt($a->floor)));
      if (!$a->stepsDone['dw']) { // show info needed to open a Dwolla account
        extract((array) $a->secure, EXTR_PREFIX_ALL, 's'); // dunno why (array) is required (indirect mod error)
        $dwInfo = item(u\rayTable(u\ray('usdEmail usdPass phone SSN address zip usdPIN', @$s_usdEmail, @$s_usdPass, @substr(@$s_usdPhone, 2), $a->federalId, $a->address, $a->postalCode, @$s_usdPin)));
      }
    }
  
    $steps = $a->ok
      ? item(a\showActivity($a), t('Activity:'))
      : item(a\showSteps($a), t('To do (red):'), t('sign=agreement | contact=contact info | dw:for admin to do | connect=connect bank | verify=verify bank deposits'));
    $date = $a->ok ? item(u\fmtDate($a->access), t('Last use:')) : item(u\fmtDate($a->login), t('Last signin'));
    if ($rels = a\showRelations($a)) $relations = item($rels, t('Relations:'));
  }

  if ($cAdmin) {
    $legalName = textField(t('Legal Name:'), '', required($a->legalName));    
    $rTrader = boolField(t('rTrader?'), t('Is this account approved to use rCredits? (member, <a href="' . $base_url . '/account/photo">good photo</a>, ss# checks out)'), $a->ok + 0);
    if (!$a->ok and !$a->co) {
      if ($a->member) $ssnCheck = item(a\ssnCheck($a));
      if ($helper = $a->helper ?: (db\lookup('inviter', 'r_invites', 'invitee=:myid', u\ray('myid', $uid)) ?: '')) {
        $hA = r\acct($helper);
        $helper = "$hA->mainQid   $hA->fullName";
      }
      $helper = textField(t('Helped by:'), t('Who gets the "inviter/helper" reward? Must be exactly one account.'), dft($helper) + autocomplete());
    }

    $bits = u\ray(B_ADMINABLE . ($admin ? ' admin' : ''));
    $adminableDefaults = array();
    foreach ($bits as $bitName) {
      $bit = u\consta('b', $bitName);
      $adminables[$bit] = $bitName;
      if ($a->hasBit($bit)) $adminableDefaults[] = $bitName;
    }
    $adminable2 = boxesField('adminable', t('Other bits:'), '', $adminableDefaults, $adminables);
    $who = textField(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete($admin ? '' : 'ctty')); // keep this for when we need it (just add to returned fields)
  }      
  if (!$self) {
    $notes = formField('textarea', t('Notes:'), '', dft($a->notes));
    $submit = submit(t('Go'));    
    $qid = hidField($a->mainQid);
  }
  $back = item('<a href="' . $base_url . '/sadmin/member-list">Go to Member List</a>', ' ');
  return compact(u\ray('legalName donations date steps dwInfo relations floor ssnCheck adminable2 rTrader helper notes count submit back qid'));
}

function showCardCode($a) {
  $len = strlen(@$a->secure['cardCode']);
  if ($len == 14 or $a->co) return;
/**/ if ($a->ok) debug("$a->fullName\t" . $a->secure['cardCode'] . " (" . $len . ')');
}
