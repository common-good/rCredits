<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Handle an arbitrary AJAX request.
 * GET parameters:
 * @param string op: what to do
 * @param string sid: session id
 * @todo: maybe all zero uid for some ops (by looking up two fields in sessions?)
 */

function ajax() {
  global $channel; $channel = TX_AJAX; // tell log and tellAdmin our environment
  extract(just('op sid data', $_POST ?: $_GET)); // don't sanitize this, it only comes from us!
  header('Content-Type: application/json');

  lug($_GET);
  if (!@$sid or !$vars = w\sessionVars($sid)) ajaxErr('bad sid');
  if (!$aid = $vars['uid'] or !$myid = $vars['myid']) ajaxErr('not signed in');
//  $mya = r\acct($myid, $aid);
/**/  lug(compact(ray('op data myid aid')));
  $data = json_decode($data); // must be passed encoded or jQuery post() passes null
  $mya = r\acct($myid); // usually relevant
  
  if ($op == 'ssn') { // verify member's SSN
    if ($mya) $mya->ssnCheck();
    exit();
  } elseif ($op == 'typeWho') { // get selections for suggestWho
    exit(json_encode(be\memberRay($myid, $aid, $data))); // data=1 means companies only
  } elseif ($op == 'changeCtty') { // change member's community
    extract(just('newCtty retro', $data));
    if (!$mya or !$mya->changeCtty($newCtty, $retro, $err)) ajaxErr(t('Community change failed.|') . $err);
//    ajaxRet(['msg' => "uid is $uid, newCtty is $newCtty"]);
  } elseif ($op == 'setBit') {
    extract(just('uid bit on', $data));
    if ($a = r\acct($uid) and $b = u\consta('b', $bit) and $b < B_ADMIN2) {
      $a->setBit($b, $on);
    } else ajaxErr(t('nope'));
  } elseif ($op == 'who') {
    extract(just('who question amount', $data));
    $amtDpy = u\fmtAmt($amount);
    if (!is_array($whoA = be\identify($who, $myid, stripos(" $question", t(' pay ')) !== FALSE, 'no self-trading'))) {
      ajaxRet(ray('who confirm', $whoA->mainQid, t($question, 'name amount', $whoA->fullName, $amtDpy)));
    }
    list ($msg, $args, $choices) = $whoA;
    $message = t($msg, $args);
    if (empty($choices)) ajaxErr($message);
    foreach ($choices as $uid => $fullName) {
      $choices[$qid = r\qid($uid)] = "$qid $fullName";
      unset($choices[$uid]);
    }

    // create modal selection form
    $choice = selectFld('', '', attrib(['size' => 12]), $choices);
    $which = render($choice);
    $title = t($question, ray('name amount', t('whom'), $amtDpy));
    ajaxRet(compact('which', 'title', 'message'));
  } elseif ($op == 'whoami') { // return one or more data fields
//    $data = just('fullName', ray($data)); // limit what we can get to what we need
    ajaxRet(ray('whoami', "$mya->fullName ($mya->mainQid)"));
  } else ajaxErr(t('Invalid op'));
  ajaxMsg(t('Done!'));
}

function ajaxErr($msg) {ajaxMsg($msg, 0);}
function ajaxMsg($msg = '', $ok = 1) {ajaxRet(['message' => $msg], $ok);}
function ajaxRet($data, $ok = 1) {
  $data = json_encode($data + compact('ok'));
  u\loga('ajax', $data);
  exit($data);
}
