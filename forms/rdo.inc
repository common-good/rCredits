<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Complete a "no signin required" request
 *   called when the user clicks an email link: new.CommonGood.earth/do/do<form>~<qid>~<emailCode>~mo~v
 *   like new.CommonGood.earth/do/doProxies~NEWAAA~23lkjlk6j~2~$16.
 * @param string code: an encoded serialized array of parameters, possibly including:
 *   op: what to do (REQUIRED)
 *   v: an amount or identifier
 *   mo: month link was created, relative to first rdo (see rdoMonth())
 *   sql: a database request, if any
 *   subs: substitutions for the database request
 *   
 * Possible values for op:
 *   (numeric): record id in r_do -- other parameters for one-time op are serialized in that record's data field
 *      including the true op. The record also includes uid and expiration time.
 *   min: change minimum to $v
 *   sql: do the database operation in $sql
 *   addr: show an address for account QID $v (for shipping or thank-you card)
 *   inv: pay invoice number $v
 *   confirmInvite: confirm an invitation
 *   do<form>: allow member to visit one specific page
 * Alternatively, code can be an error message (recognized by having a space in it).
 * @param string $args: urlencoded array of arguments passed from an email (from whence $_POST is not reliably received by Drupal)
 * @see also r\makeDo()
 */
function rdo($code, $args = '') {
/**/	u\log(print_r(compact(ray('code args')), 1));
  global $channel;
  if (isDEV) foreach (['error', 'status'] as $k) unlink(f('t.dosayFilename', $type)); // clear tests (no way to see if testing)
  if (time() < strtotime('6/1/2017')) u\loga('rdo', compact('code', 'args'));
  if (strpos($code, ' ')) return doSay(plain($code));
  if ($i = strpos($code, '~') and $i < 15) { // mailChimp-type action
    @list ($op, $qid, $ecode, $mo, $v) = explode('~', $code . '~~~~~~');
/**/		u\log(print_r(compact(ray('op qid ecode mo v code')), 1));
    if (!@$op or !@$qid or !@$a = r\acct($qid) or @$ecode != $a->emailCode) return doSay(t('That is not a valid link.'));
  } else {
    if (!$info = r\doCode($code)) return doSay(t('That link is archaic.'));
///  print_r($info);
    if ($once = is_numeric($doid = @$info['op'])) {
      if (!$row = db\get('uid,data,expires', 'r_do', 'doid=:doid', compact('doid'))) return doSay(t('That link is no longer valid.')); // already used OR deleted because it expired
      extract($row);
      $info = unserialize($data) + $row; // get new op, etc.
      $a = r\acct($uid);
    }
    extract(just('op v mo expires sql subs', $info));
  }
  
  if (@$mo) $expires = (r\rdoMonth() > $mo + 3) ? 1 : FALSE; // finagle expiration if link is older than 3 months
  if (@$expires and r\rTime() >= $expires) return doSay('link expired');

  $channel = TX_LINK;
  
  if (u\starts($op, 'do') and $formName = substr($op, 2)) {
    $file = strtolower(R_ROOT . "/forms/$formName.inc");
    if (file_exists($file)) include_once $file;		
    return w\showForm($formName, "qid=$a->mainQid&ecode=$ecode&$v");
  }
//  if ($op == 'min') $res = $a->update('minimum', $v); // not used (rewrite as a confirmation form)
  if ($op == 'nosecret') $res = $a->setBit(B_SECRET, FALSE);
  if ($op == 'debtok') $res = $a->setBit(B_DEBT, TRUE);
  if ($op == 'rsv') {return w\go("savings/qid=$qid&ecode=$ecode");}
  if ($op == 'sql') $res = db\q($sql, $subs);

  if ($op == 'addr') {
    $a = r\acct($v);
    $phone = $a->co ? t('Phone: ') . u\fmtPhone($a->phone) : '';
    $addr = t('Physical address: ') . r\location($a, TRUE);
    return doSay(tr('%nm<br>%postal<br>%addr<br>%phone<br>', 'nm postal addr phone', $a->fullName, $a->postalAddr, $addr, $phone), 'ok');
  } 
  
  if ($op == 'inv') {
    if ($err = r\badUnpaidInv($v)) return doSay($err);
    return w\go("/handle-invoice/nvid=$v&toMe=1&code=$code");
  } 
  
  if ($op == 'confirmInvite') {
    $a = r\acct($v);
    $a->setBit(B_CONFIRMED);
    return doSay(t('Your invitation to %nm is confirmed!', 'nm', $a->fullName), 'ok');
  }
  
  if ($op == 'vote' or $op == 'yesno') { // NOTE: about a third of the time, $_POST comes in empty (even from the same computer)
    $issue = $v;
    if ($issue != '20141220') return doSay('link expired');
//    $vote = 0;
//    foreach (ray('wl sl wd ed no') as $k => $vv) $vote |= u\setBit($vote, $k, isset($_POST[$vv]));
    extract(just('yes no', $args));
    $vote = @$yes ? 1 : 0;
    $info = compact(ray('uid issue vote'));
    $vid = db\get('vid', 'r_votes', 'uid=:uid AND issue=:issue', compact('uid', 'issue'));
//    $vid = 0;
///  debug(compact(ray('op args issue yes no vote info vid')));
    if ($vid) db\update('r_votes', $info + compact('vid'), 'vid'); else db\insert('r_votes', $info);
    $say = t('Thank you for your RSVP! ');
    if (@$no) $say .= t('We will miss you. ');
    r\tellAdmin('RSVP!', ['coming'=>@$yes ? 'yes' : 'no'], $uid);
    $res = TRUE;
  }
  
  if ($op == 'join') {
    $otherId = $v;
    $joint = r\perm(B_JOINT);
    if  (r\relation(1, $otherId, $a->id, 'permission=:joint', compact('joint'))) { // inviter still wants it?
      $a->unjoin(); // zap any other joins or join requests
      r\acct($a->id, $otherId)->setRel('permission draw', $joint, 0);
      $a->join(r\acct($otherId)); // remember who is joined
      $res = TRUE;
    }
  }

  if (!@$res) return doSay(t('Request failed.'));
  if ($once) db\q('DELETE FROM r_do WHERE doid=:doid', compact('doid'));

  return doSay(@$say . t('Your request was successfully completed.'), 'ok');
}

