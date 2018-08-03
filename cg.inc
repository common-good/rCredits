<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in cg-backend.inc (i)
 *   - utilities, which are in cg-util.inc (u)
 */
 
namespace CG; // typically abbreviated as "r"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w; // use only in rTime (for the game)

require_once __DIR__ . '/cg-strings.inc';
require_once __DIR__ . '/cg-db.inc';
require_once __DIR__ . '/acct.class';
require_once __DIR__ . '/x.class';
require_once __DIR__ . '/qo.class';

define('CGF_ID', r\cgfId()); // Common Good Finance record ID

/**
 * Save a record in the given table, in the appropriate regional section of the id field's number space.
 * @param string $table: table to save in (users or r_relations)
 * @param string $idField: name of record ID field (uid or reid)
 * @param assoc $record: the record to save (possibly with record ID already chosen)
 * @param int $region: uid of the region in which to create a new record (defaults to server region, negated)
 *   if the region is negated (>0), an individual or company account will be inserted -- otherwise a community
 * @return record ID of the inserted record
 */
function regionalInsert($table, $idField, $record, $region = NULL) {
  if (@$record[$idField]) return db\insert($table, $record); // predestined id (probably for testing)
  if (!@$region) $region = -r\serverUid();
  $end = $region + ($region < 0 ? -(R_REGION_MAX - 1) : (R_REGION_MAX - 1));
  $start = $region < 0 ? $region - 1 : $region; // -1: do not use the uid of the region itself

  for ($try = 0; $try < 20; $try++) { // limit the retries
    if (!$id = $record[$idField] = u\firstUnusedId($idField, $table, $start, $end)) break;
    try {
      if (db\insert($table, $record)) return $id;
    } catch (Exception $e) { // BIG MYSTERY: why does this happen sometimes with no collision?
      r\tellAdmin(t('cannot save regional: ') . $e->getMessage(), $record + ['topic' => t('cannot save')]);
    }
  }
  u\EXPECT(FALSE, "failed to save $table record in region $region after $try tries.");
}

/**
 * Say whether a given payment is giving back rewards to the community.
 * @param int $payer: record id of who is maybe giving back
 * @param int $payee: record id of payee
 * @param assoc $data: data record from transaction
 * @return <yes, giving back rewards>
 *//*
function isGiveback($payer, $payee, $data) {
  return ($payee == r\acct($payer)->community and @$data['isGift']); // may be wrong if member changes community!
}
*/

/**
 * Return an error message if the given invoice is already paid or doesn't exist.
 * @param int $nvid: record ID of invoice
 * @param assoc $inv: (RETURNED) the invoice record
 * @return the error message (FALSE if no error)
 */
function badUnpaidInv($nvid, &$inv = '') {
  $fields = 'nvid,status,amount,payer,payee,goods,purpose,data,created';
  if (!$inv = db\get($fields, 'r_invoices', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['status'] > TX_APPROVED) return 'already paid';
}

/**
 * Return the user's sequential transaction number
 * @param int $xid: the transaction record number
 * @param int $uid: the user's uid
 * @return string: the user's transaction number
 */
function xid2tid($xid, $uid) {
  $jid = r\acct($uid)->jid ?: 0;
  $isIt = ' IN (:uid, :jid)';
  $where = "(payer$isIt OR payee$isIt) AND xid<:xid AND type>=0";
  $max = "MAX(GREATEST(IF(payee$isIt, payeeTid, 0), IF(payer$isIt, payerTid, 0)))";
  return db\get($max, 'r_txs', $where, compact('uid', 'xid', 'jid')) + 1;
}

/**
 * Say whether an account's proposed transaction duplicates the last recent one.
 * @param int $uid: the account's record ID
 * @param int $other: the other account record ID
 * @param float $amount: the amount proposed to transfer from the account to the other account (can be negative)
 * @return TRUE if the proposed transaction duplicates the last one.
 * @see also X::cacheTotals(), where the transaction data is recorded
 */
function dupTx($uid, $other, $amount) {
  global $channel;
  if ($channel == TX_WEB) return FALSE; // allow dups on web interface
  if (!$lastTx = r\acct($uid)->lastTx) return FALSE;
  list ($xid, $dif, $oldAmt, $created) = $lastTx;
///  t\log(compact(ray('uid other amount lastTx')));
  return ($dif == $other - $uid and $amount == $oldAmt and r\rTime() - $created < R_DUPTX_MINS * MIN_SECS);
}
 
/**
 * Say whether the named employee works for the account owner.
 * @param string $name: the employee's name
 * @param int $us: the employer's account record ID (defaults to current account record ID)
 * @param bool $loose: <the employee need not be formally an employee>
 * @return int: the person's account record ID if s/he does work for us, otherwise FALSE
 */
function worksForUs($name, $us = NULL, $loose = TRUE) {
  u\setDft($us, r\acct()->id);
  $words = explode(' ', u\roughName($name)); // abbreviate middle names
  if (strlen($words[0]) > 1) { // unless first name is already abbreviated
    for ($i = 1; $i < count($words) - 1; $i++) $words[$i] = substr($words[$i], 0, 1) . '%';
  }
  $pattern = join(' ', $words);
  $where = 'r.main=:us' . ($loose ? '' : ' AND r.:IS_EMPLOYEE');
  
  $sql = <<< X
    SELECT u.uid, u.fullName FROM r_relations r
    INNER JOIN users u ON u.uid=r.other
    WHERE $where
    ORDER BY (u.fullName LIKE :pattern) DESC
X;
  $q = db\q($sql, compact('us', 'pattern'));
  while ($row = $q->fetchAssoc()) {
    extract($row);
//    $data = unserialize($data);
    $legalWords = explode(' ', u\roughName($fullName)); // not $data['legalName']
    if (count($words) != count($legalWords)) continue;
    for ($i = 0, $match = TRUE; $i < count($words) and $match; $i++) {
      if (!u\abbreviates(str_replace('%', '', $words[$i]), $legalWords[$i])) $match = FALSE;
    }
    if ($match) return $uid;
  }
  return FALSE;
}

/**
 * Join or unjoin this account to another.
 * @param int $reid: relations record ID
 * @param bool $join: whether to join the accounts (otherwise unjoin)
 * @return a message saying the join was successful
 */
function setJoin($reid, $join) {
  extract(r\relation('main,other', $reid));
  list ($a1, $a2) = [r\acct($main), r\acct($other)];

  if ($join) {
    $a1->unjoin(); // zap any join or other join requests
    if (r\relation('permission', $other, $main) == r\perm(B_JOINT)) { // confirming join
      $a1->join($a2);
      $msg = t('Success! Your accounts have been successfully joined.');
    } else { // requesting join
      $_atag = $a2->makeDo('join', $main); // seek agreement
      r\message($other, 'join accounts', ray('_atag name', $_atag, $a2->fullName));
      $msg = t('Success! An email has been sent to the other account owner, to confirm joining with yours.');
    }
  } else {
    $a1->unjoin($a2);
    $msg = t('Your accounts have been successfully separated.');
  }
  return $msg;
}

/**
 * Return the number of months since the first rdo of this type (for conciseness).
 */
function rdoMonth($time = NULL) {
  u\setDft($time, r\rTime());
  return round(($time - strtotime('12/1/2014')) / (MONTH_DAYS * DAY_SECS));
}

/**
 * Return an array or dropdown of communities (just in the given region?).
 * @param int $region: current region uid
 * @param int $ctty: current community (if any, return a dropdown input field)
 * @return array of communities if $ctty is FALSE, otherwise a dropdown
 */
function cttys($region, $ctty = FALSE) {
  $where = 'uid<0'; // uid BETWEEN :region-(:R_REGION_MAX-1) AND :region-1
  $sql = "SELECT uid,fullName FROM users WHERE $where ORDER BY fullName";
  $ray = db\q($sql, compact('region'))->fetchAllKeyed();
  if ($ctty) {
//    if (!@$cttys[$ctty]) u\preray([$ctty => $a->cttyA->fullName . t(' (outside the region)')], $cttys);
    $ray[r\serverUid()] = t('Seedpack (no community yet)');
    return w\selectFld(t('Community:'), '', w\dft($ctty), $ray);
  } else return $ray;
}

/**
 * Record the current IP as approved (for whitelisting in WHM)
 * @param int $uid: account record ID
 * @param string $device: one device code for this account (not very important, but interesting, especially for POS)
 */
function ipok($uid, $device) {
  $ip = $_SERVER['REMOTE_ADDR'];
  if (!db\exists('r_ips', 'ip=:ip', compact('ip'))) db\insert('r_ips', compact(ray('ip uid device'))); 
}

/**
 * Wrap all email, so it doesn't go out when testing.
 * @param string $index: index to email template and subject (empty if supplying explicit body and subject)
 * @param mixed $to: where to send the message -- uid, account, email address, or [$email => $name]
 * @param assoc $subs: substitutions in template. may also include:
 *   noFrame:  don't show salutation and closing and footer
 *   subject:  specific subject
 *   body:     specific message
 * ... the following special substitions are always available:
 *   site, qid, zip3, shortName, and every cached field for the account
 * @param mixed $reply: uid, account, or email address to reply to OR 'ctty' (of $to) or NULL (CGF_EMAIL) OR [email => name]
 * @return TRUE if the message got sent (or on dev machine pretending it got sent) else FALSE
 * NOTE: from is always SYS_EMAIL
 */
function rMail($index, $to0, $subs, $reply0 = NULL) {
//  require_once __DIR__ . '/dkim.php';
//  if (isGAME) return; // no emails from game site
  global $testEmails;

  foreach (ray('secure vsecure') as $k) unset($subs[$k]); // show not even in error messages
  $subs['site'] = BASE_URL;
  $subs['orgPhone'] = CGF_PHONE;
  extract(just('subject body noFrame sms', $subs));
  
  if ($toA = is_object($to0) ? $to0 : r\acct($to0)) {
    $toEmail = $toA->email;
//    $to = "\"$toA->fullName\" <$toEmail>";
    $to = [$toEmail => $toA->fullName];
    $subs += (array) $toA->account(); // this omits any special fields or special formatting
    $subs['qid'] = $toA->mainQid;
    $subs['shortName'] = $toA->name;
    $subs['zip3'] = substr($toA->zip, 0, 2); // temporarily show a wider region than zip3
    if ($toA->closed) return r\tellAdmin(t('attempt to email closed account'), compact(ray('index subs reply0')));
  } else $to = $toEmail = $to0;
  
  $testEmails[] = compact(ray('index toEmail subs'));

  if (!is_array($reply = $reply0) and !is_object($reply0)) {
    $reply = (!@$reply0 or $reply0 == 'region') ? [CGF_EMAIL => PROJECT] // standard
    : ($reply0 == 'ctty' ? $toA->cttyA : r\acct($reply0)); // get account object
  }
  
  if (is_object($replyA = $reply)) $reply = [$replyA->email => $replyA->fullName];

//  $nolog = ray(R_NOLOG); // subs not to log (sensitive data)
  foreach ($subs as $k0 => $v) if (!is_object($v) and !is_array($v)) {
    $k = "%$k0"; // '{' . $k0 . '}';
    $resubs[$k] = $v;
//    $logSubs[$k] = in($k0, R_NOLOG) ? '(?)' : $v; // hide personal info and secrets
  }

//  $index = str_replace('_', '-', $index); // correct Drupal template names to rCredits standard
  $subject = u\SUBS(strtr(@$subject ?: $GLOBALS['emailSubjects'][$index], $resubs), '%');
  $body = u\SUBS(strtr(@$body ?: (@$noFrame ? r\emailTemplate($index) : r\emailBody($index)), $resubs), '%');

  // log
  $logSubs = u\noSecrets($resubs) + ["\r\n" => '', ' style=' => ' stylish=']; // easier reading for log and tests & evade CSP
  $message = strtr($body, $logSubs);
  $erInfo = compact(ray('index toEmail subject message'));
/**/ f('t.output', 'Actual (in rMail) ' . print_r($erInfo, 1), 'email');
  u\loga('email', $erInfo + ['toUid' => @$toA->id]);

  if (isPRODUCTION !== TRUE) { // make sure we never send rogue emails from test server even if constant isn't set
/**/  if (!isDEV) debug(compact(ray('subject to body')));
    return; // no emails unless real!!
    if ($toEmail != r\regionField('email') and $toEmail != R_ADMIN_EMAIL) return TRUE;
    if (isDEV) return TRUE; // otherwise cURL error 60: SSL certificate problem: unable to get local issuer certificate
    $subject = '(test) ' . $subject;
  }
  
  require_once DRUPAL_ROOT . '/vendor/swiftmailer/lib/swift_required.php';
  $transport = \Swift_SendmailTransport::newInstance('/usr/sbin/sendmail -bs');
  $mailer = \Swift_Mailer::newInstance($transport);

  $privateKey = "-----BEGIN RSA PRIVATE KEY-----\n" . str_replace(' ', "\n", DKIM_PRIVATE) . "\n-----END RSA PRIVATE KEY-----";
  $signer = new \Swift_Signers_DKIMSigner($privateKey, EMAIL_DOMAIN, 'system');

  $m = \Swift_SignedMessage::newInstance()
  ->attachSigner($signer)
  ->setSubject($subject)
  ->setFrom([SYS_EMAIL => t('%PROJECT system')])
  ->setTo($to)
  ->setReturnPath(BOUNCE_EMAIL)
  ->setBody($body, 'text/html')
  ->addPart(strip_tags($body), 'text/plain'); // don't bother making the plain text pretty

  if ($reply) $m->setReplyTo($reply);
  if (@$sms) $m->setSubject('')->setTo($toEmail); // send as text message
  try {
    $success = $mailer->send($m);
  } catch (Exception $e) {
    u\EXPECT($toEmail != R_ADMIN_EMAIL, 'cannot send admin email');
    r\tellAdmin(t('cannot send email'), $erInfo);
  }
  return TRUE;
//  ->attach(Swift_Attachment::fromPath('my-document.pdf'))  
}

/**
 * Add a deactivated or phony (and used) card code to the "bad" list.
 * @param string $qid: the account ID
 * @param string $code: the bad card code
 * @param string $msg: type of error
 */
function badCard($qid, $code, $msg) {
  $created = time();
  $info = compact('qid', 'code');
  if (!db\exists('r_bad', $info)) {
    db\insert('r_bad', $info += compact('created'), 'created', TRUE);
    $index = 'card marked bad';
  } else $index = 'attempted card already marked bad';
  r\tellCO($index, $info + compact('msg'));
}

/**
 * Email an arbitrary message to the member, NOW (otherwise use notify).
 * @param int $uid: account record ID
 * @param string $index: index to arbitrary HTML message body (between salutation & closing) or the HTML itself
 * @param assoc $subs: message parameters
 * @param string $topic: optional specific topic (otherwise look in $GLOBALS['topics'])
 */
function message($uid, $index, $subs = [], $topic = '') {
  global $testMessages;

  $subs['otherName'] = @$subs['myName']; // messaging someone else about US, if anyone
  $reply = @$subs['reply'];
  $message = t($index, $subs);
  $topic = $topic ?: (t(@$GLOBALS['topics'][$index], $subs) ?: $index); // index should never be used literally (wrong language) but more importantly never fail
  $testMessages[] = compact(ray('uid index subs topic'));
///  debug(compact('uid','index','subs','message','topic'));
  r\notify($uid, 'message', compact(ray('message topic reply')), TRUE);
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $subs: substitutions/parameters for the message
 * @param bool $force: send by email NOW
 */
function notify($uid, $index, $subs = [], $force = FALSE) {
  global $channel;
  if (isGAME) return;
  $a = r\acct($uid);
  u\EXPECT($a, 'no account');
  $subs['uid'] = $uid;
  $subs['qid'] = $a->qid;
  $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone
  $fullName = $a->fullName;
  $warnIndexes = ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $a, $subs, @$subs['reply'] ?: 'ctty');
  } else { //if ($a->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(t($index, $subs), r\rTime());
    f('t.output', "Actual Notice to $fullName: $message");
    db\insert('r_notices', compact(ray('uid message created')));
  }
  
  if ($a->can(B_BYSMS)) {
    if ($num = $a->smsNumber) $smsed = SMS\send($num, t($index, $subs));
  }
  if (FALSE and !@$mailed and !@$smsed and array_search($index, $warnIndexes) !== FALSE) {
    $notification = str_replace('-', ' ', $index);
    $warning = t('cannot contact', compact('fullName', 'notification'));
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) SMS\send($a->smsNumber, $warning);
    if ($channel == TX_POS) { } // dunno how to handle this
  }
}

/**
 * Suggest a higher minimum (because of a failed purchase).
 * @param acct $a: the account that needs a higher minimum
 * @param int $default: the default minimum, if special
 * @param string $dftMst: default message, if any
 * @param bool $get: <just return the message>
 */
function suggestMin($a, $default = '', $dftMsg = '', $get = FALSE) {
  $suggested0 = $default ?: max(R_ACHMIN, (R_SUGGEST_BUMP * $a->minimum) ?: 100);
  $suggested = u\fmtAmt($suggested0);

  $_a1 = $a->makeDo('min', $suggested0);
  $msg = $dftMsg ?: 'you were short';
  
  if ($a->can(B_SECRET)) {
    $_a2 = $a->makeDo('nosecret');
    $msg .= '|be not secretive';
  }
/* DISABLE FOR NOW  if (!$a->can(B_DEBT) and $a->floor < 0) {
    $_a3 = $a->makeDo('debtok');
    $msg .= '|try debt';
  } */
  
  $args = compact(ray('suggested _a1 _a2 _a3'));
  if ($get) return t($msg, $args); else r\notify($a->id, $msg, $args);
}

/**
 * Try to transfer funds from donor to CGF.
 * @param assoc $info: the gift record data
 * @return TRUE if the funds were successfully transferred.
 */
function acceptGift($info) {
  extract(just('donid uid giftDate amount often honor', $info));
  $a = r\acct($uid);
  if ($a->shortfall(FOR_GOODS, $amount)) return FALSE; // wait until tomorrow, if not enough
  if ($a->shortfall(FOR_GOODS, $amount, TRUE)) return FALSE; // actually draw from other accounts, as needed
  $cgf = r\acct(r\cgfId());
  $data['gift'] = 1;

//  r\usd::beginAtom();
  if ($often != 1) {
    $months = $often == 'M' ? 1 : ($often == 'Q' ? 3 : 12);
    $recurs = ray(t('once monthly quarterly yearly'));
    $count = db\count('r_gifts', 'uid=:uid AND often=:often', compact('uid', 'often'));
    $purpose = t('regular donation (%how gift #%count)', 'how count', $recurs[strpos('1MQY', $often)], $count);
    $data['patronage'] = 1;
  } else $purpose = t('donation');

//  $sharing = (@$honor == 'share');
//  list ($goods, $purpose) = $sharing ? [FOR_SHARE, R_SHARING_MSG] : [FOR_GOODS, t('donation') . @$recurring];
  list ($msg, $args) = $a->pay($cgf, $amount, $purpose, $data);
  if (@$args['success']) {
    $completed = r\rTime();
    db\update('r_gifts', compact('donid', 'completed'), 'donid');
    if ($often != 1) {
      $giftDate = u\plusMonths($months, $giftDate);
      $share = -1; // mark this field N/A (irrelevant)
      db\insert('r_gifts', compact(ray('uid giftDate amount often share')));
    };
//    r\usd::commit();
//    r\tellAdmin('gift accepted', $info + $args);
    r\notify($uid, 'gift sent', $args);
    return TRUE;
  } //else r\usd::rollback();
  return FALSE;
}

/**
 * Notify rCredits staff (Community Admin, if uid is specified and in their area)
 * @param string $index: the message or message index
 * @param assoc $subs: optional substitutions for the message and/or data to show (show both)
 * @param bool/int $uid: related account record id (TRUE means current account), defaults to none
 * @return FALSE;
 */
function tellAdmin($index, $subs = [], $uid = NULL) {
/**/  if (isDEV and !u\test()) return debug(['tellingStaff' => $index] + $subs + compact('uid'));
  foreach (ray(R_NOLOG) as $k) unset($subs[$k]); // secret stuff
  ksort($subs); // put the keys in a predictable order, for testing and easy reading

  if (!$tellCO = isset($uid)) $uid = @$subs['uid'];
  if ($mya = r\acct(@$uid)) $who = "$mya->fullName ($mya->mainQid)"; // (default to current account)
  $message = t($index, $subs) . u\seeAssoc($subs + compact('who'));
    
  if ($tellCO) { // account specified, so tell Community Admin
    $email = $mya->cttyA->email;
  } else list ($email, $uid) = [R_ADMIN_EMAIL, $uid ?: ($mya ? $mya->id : NULL)]; // no account specified so tell SysAdmin

  $noFrame = TRUE;
  $topic = strlen($index) < 50 ? strip_tags($index) : '';
  $subs = compact(ray('who email message topic noFrame uid')) + $subs;
  $replyTo = (@$uid and $a = r\acct($uid) and $a->email) ? [$a->email => $a->fullName] : NULL;
  r\rMail('tell-staff', $email, $subs, $replyTo);
  if ($email != R_ADMIN_EMAIL) r\rMail('tell-staff', SYS_EMAIL, $subs, $replyTo); // always tell one or more staff (for now)
  return FALSE;
}

/**
 * Report something of interest to a community admin.
 * @param string $index: message index or text
 * @param assoc $subs: substitutions/parameter, if any, to include in the report
 * @param int $uid: record ID of an account in the community to report about.
 *    If none, choose something reasonable, reporting to superAdmin as a last resort.
 */
function tellCO($index, $subs = [], $uid = NULL) {
  if (!$uid) $uid = ($mya = r\acct()) ? $mya->id : @$subs['loserId'];
  return tellAdmin($index, $subs, $uid);
}

/**
 * Return the rough (physical) location of the entity with the given account (city, state, non-US country).
 * @param acct $a: the account
 * @param bool $complete: if TRUE, include the street address
 */
function location($a, $complete = FALSE) {
  $country = realCountry($a->country);
  $result = [];
  if ($complete) $result[] = (string) $a->address; // (string) in case address is NULL
  $result[] = $a->city;
  $result[] = $a->st . (($complete and $a->zip) ? ' ' . $a->zip : '');
  if ($a->country != R_COUNTRY_ID) $result[] = strtoupper($country);
  foreach ($result as $key => $value) if ($value === '') unset($result[$key]);
  return join(', ', $result);
}

/**
 * Return the short qid and full name of the specified account.
 * For example, return ".ZZA Abe One".
 */
function definiteName($uid) {
  if ($uid <= 0) return '';
  $acct = r\acct($uid);
  return r\qid($uid) . ' ' . $acct->fullName;
}

/**
 * Return an rCard security code for the specified qid.
 */
function cardCode($qid) { 
  $codeLen = R_CARDCODE_LEN - (strlen($qid) - 6); // shorter code when qid len > 6
  return u\randomString($codeLen, 'word'); // create an rCard security code
}

/**
 * Return the account for the given invitation card code.
 * @param string $code: the code to interpret
 * @param int $iCode: (RETURNED) the code sequence number
 * @return: the account or FALSE if error
 */
function iCardAcct($code0, &$iCode = NULL) {
  $code = str_replace(' ', '', $code0); // ignore spaces
  if (!preg_match('/[A-Z0-9]{' . ICARD_CODELEN_MIN . ',}/i', $code)) return FALSE;
  $iCode = u\ai2n(substr($code, ICARD_CODELEN_MIN)) + 0;
///  debug("code=$code iCode=$iCode");
  $code = substr($code, 0, ICARD_CODELEN_MIN); // chop off sequence number
  $b = u\lpad(decbin(u\ai2n(substr($code, 0, 5))), ILEN_DIV). u\lpad(decbin(u\ai2n(substr($code, 5))), ILEN - ILEN_DIV);
///  debug("code=$code iCode=$iCode b=$b");
  $b = u\xorBits($b, substr(R_INVITE_KEY, -strlen($b)), TRUE);
///  debug("code=$code iCode=$iCode b=$b");
  $b = u\rotateBits($b, -($iCode + 1) * (bindec(substr(R_INVITE_KEY, 0, 31)) % strlen($b)));
///  debug("code=$code iCode=$iCode b=$b");
  $region = u\n2a(bindec(substr($b, 0, ILEN_REGION)), 3);
  $tail = u\n2a(bindec(substr($b, ILEN_REGION, ILEN_TAIL)), -3);
  $security = u\n2ai(bindec(substr($b, -ILEN_SECURITY)), 3);
///  debug("code=$code iCode=$iCode b=$b region=$region tail=$tail security=$security");
///  debug(r\acct("$region$tail")->lowSecurity);

  return ($a = r\acct("$region$tail") and $security == $a->lowSecurity) ? $a : FALSE;
}

/**
 * Set a cookie
 * @param string $key: name of cookie
 * @param string $value: value of cookie
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now) 0=end of session
 * @param bool $raw: <suppress URL-encoding>
 * @return: the value
 */
function setCook($key, $value, $expires = NULL, $raw = FALSE) {
	u\EXPECT(($len = strlen(rawurlencode("$key=$value"))) <= 4096, "cookie too long len=$len"); // 4094 not 4096 (browser bug?)
  u\setDft($expires, r\rTime() + 365250 * DAY_SECS);
	$func = $raw ? 'setcookie' : 'setrawcookie';
  $res = $func($key, $value, (int) $expires, '/'); // make it easy (fails without '/')
	u\EXPECT($res, 'cookie failed');
  $_COOKIE[$key] = $value; // make it available also before going to a new page
  return $value;
}

function setCryptCook($k, $v) {return setCook($k, substr(u\cry('C', $v), 3), 0, TRUE);}
function cryptCook($k) {return ($v = @$_COOKIE[$k]) ? u\decry('C', CRYPT_FLAG . 'C' . $v) : '';}

/**
 * Return the unformatted private key for highly sensitive data (use u\fmtKey() to get the formatted key).
 */
function vKey() {
	global $mya, $channel;
  u\EXPECT($channel == TX_WEB, 'wrong channel for vKey: ' . $channel);
  if (!@$mya) w\setAcct(); // make sure we got mya
	return ($vKeyE = r\cryptCook('vKeyE') and $pw = $mya->agentA->vKeyPw) ? u\decry('R', $vKeyE, $pw) : '';
}

/**
 * Return the rCredits region id for the given account record ID
 */
function uidRegion($uid) {
  $uid = abs($uid) - 1; // -1 because uids are all bumped up by 1
  return -1 - ($uid - ($uid % R_REGION_MAX));
}

/**
 * Return the rCredits region id for the given geographical location
 * @param int $country: country ID
 * @param int $state: state or province ID (defaults to MA)
 * @param int $zip: postal code
 * @param string $raw: (RETURNED) the alphabetic identifier for the region (usually 3 characters)
 * @return int: the region uid
 */
function region($country = '', $state = '', $zip = '', &$raw = '') {
  if (!$state) $state =  R_STATE_ID;
  $where = "IF(zip='', country_id=:country AND s.id=:state, :zip RLIKE zip) ORDER BY (zip='') LIMIT 1";
  $table = 'r_regions r LEFT JOIN r_states s ON s.abbreviation=r.st';
  if (!$region = db\get('region', $table, $where, compact(ray('state zip country')))) {
    $region = db\get("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  }
  return r\qo('!' . ($raw = $region ?: 'ZZZ') . 'AAA')->id;
}

/**
 * Say whether one of the accounts owns the other (or can draw from it).
 */
function owning($uid1, $uid2) {
  $where = '(:IS_OWNER or :IS_DRAW) AND ((main=:uid1 AND other=:uid2) OR (main=:uid2 AND other=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second (or of the current account)
 * @param int $company: defaults to current account ID
 */
function isEmployee($uid, $company = '') {
  return relation(1, $company ?: @r\acct()->id, $uid, ':IS_EMPLOYEE');
}

/**
 * Return the community record ID for the given postal code.
 * @param string $zip: postal code to identify community (if none, assume something in server community)
 * @param bool $defaultZero: return 0 if zip does not match any community? (else return server uid)
 */
function communityUid($zip = '', $defaultZero = FALSE) {
  if (!$zip) return serverUid();
  $where = "country=:R_COUNTRY_ID AND :zip RLIKE zip AND uid<1";
  return db\get('uid', 'users', $where, compact('zip')) ?: ($defaultZero ? 0 : serverUid());
}

/**
 * Return the name of the given community (optionally without the project name)
 */
function cttyName($uid, $omit = FALSE) {
  if (!$cttyA = r\acct($uid)) return '';
  $name = $cttyA->isRegion ? t('%PROJECT Seedpack') : $cttyA->fullName;
  return $omit ? trim(str_replace(PROJECT, '', $name)) : $name;
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName, $info = []) {
  if (!$fullName) {
    extract(just('phone email', $info));
    $fullName = R_TEMP_NAME_PREFIX .
      (@$phone ?  u\fmtPhone($phone, 'n')
      : (@$email ? R_TEMP_NAME_PREFIX . strtr($email, ['@'=>'AT', '.'=>'DOT'])
      : substr(md5('zot' . strval(r\rTime())), 0, 10))); // pseudo random
  }
  $name = $name0 = u\shortName($fullName);

  for ($i = 2; ; $i++) {
    if (!db\exists('users', 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name or email.
 * If $id is an email and more than one account has that same email address, the first non-company account is assumed.
 * @todo later: when regional prefix is required for signin, this function will ignore it in matching name field
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if (u\isQid($id = trim(strtoupper($id))) and $acct = r\acct($id)) return $acct->id; // try to interpret as a qid
  $mail = u\cry('P', strtolower($id));
  return db\get('uid', 'users', 'name=:id OR mail=:mail ORDER BY :IS_CO', compact('id', 'mail'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM r_relations WHERE main=:myid AND :IS_EMPLOYEE', compact('myid'))->fetchCol();
}

/**
 * Return the field or fields from the relations table, for the given main and other.
 * Call by:
 *   relation(field, main, other, where, subs) OR
 *   relation(field, where, subs)
 *   relation(field, reid)
 *   relation(field, qid)
 * @param string $field: what field value(s) to return (see varieties at db\get())
 * @param int $main: uid of main account
 * @param int $other: uid of other (agent) account (changed to zero if same as main)
 * @param string $where: SQL criteria
 * @param assoc $subs: criteria parameters
 * @param int $reid: relations record id
 * @param string $qid: QID for a compound account
 */
function relation($field, $main, $other = [], $where = 'TRUE', $subs = []) {
  if (is_numeric($main)) {
    if (empty($other)) { // reid
      list ($where, $subs) = [ray('reid', $main), []];
/*      if (is_numeric($reid = $main)) {
        $where = "reid=:reid";
        $subs += compact('reid');
      } else { // 
        list ($main, $otherNum) = explode('-', $main);
        list ($main, $otherNum) = [r\qo($main)->id, u\a2n($otherNum)];
        list ($where, $subs) = ['main=:main AND otherNum=:otherNum', compact('main', 'otherNum')];
      } */
    } else { // field, main, other, where, subs
      if ($main == $other) $other = 0; // handle tentative proSe relation with future customer
      $where = "main=:main AND other=:other AND ($where)";
      $subs += compact('main', 'other');
    }
  } elseif ($other) { // where, subs
    list ($where, $subs) = [$main, $other];
  } else { // qid
    $a = r\acct($qid = $main);
    list ($where, $subs) = [ray('main other', $a->id, $a->agentId), []];
  }
  return db\get($field, 'r_relations', $where, $subs);
}

/**
 * Return our standard encryption password and a salt for the given account.
 */
function passSalt($acct = '') {
  $acct = $acct ?: r\acct();
  $pass = R_SALTY_PASSWORD;
  $salt = $acct->created;
  return compact('pass', 'salt');
}

/**
 * Return the first unused uid for the given base.
 * @param int $base: lowest uid to use (first slot in a region)
 * @return: the uid
 */
function newUid($base) {return u\firstUnusedId('uid', 'users', $base, $base + R_REGION_MAX - 1);}

/*  $where = 'u.uid>=:base and u.uid <:base+:R_REGION_MAX AND u2.uid IS NULL ORDER BY u.uid';
  $previous = db\get('u.uid', 'users u LEFT JOIN users u2 ON u2.uid=(u.uid+1)', $where, compact('base'));
  return $previous ? ($previous + 1) : $base;
}*/

/**
 * Return a link to change the current account 
 * (presumably to one the current account is allowed to change to)
 * @param int $uid: record ID of account to change to
 * @param string $v: link text to display
 * @param string $class: CSS class, if any
 * @return the link
 */
function changeAcctLink($uid, $v, $class = '') {        
  global $base_url;
  $page = str_replace('/', ',', $_SERVER['REQUEST_URI']); // not urlencode because Drupal sees slashes anyway
  if ($class) $class = " class=\"$class\"";
  return "<a href=\"$base_url/change-account/acct=$uid&page=$page\"$class>$v</a>";
}

/**
 * Record the relation of a device with an account.
 * @param int $uid: record ID of related account
 * @param string $code: phone # or device code
 * @param int $ch: channel (SMS, Web, or App)
 * @return int: the device record ID
 */
function makeDevice($uid, $code = '', $ch = NULL) {
  global $channel;
  global $mya;
  
  u\setDft($ch, $channel);

//  if ($ch == TX_WEB and $mya and $mya->superAdmin) return 0; // don't create spurious devices as admin
/*  if ($boxName === '0') {
    $where = "t1.boxName REGEXP '^[0-9]+$' AND t1.uid=:uid";
    $boxName = u\firstUnusedId('boxName', 'r_boxes', 1, PHP_INT_MAX-1, $where, compact('uid'));
  } else */
  //if (!$boxName) $boxName = $code; // code serves as boxName for dumb cell phones
  $created = $access = r\rTime();
  $boxnum = db\get('MAX(boxnum)', 'r_boxes', 'uid=:uid', compact('uid')) + 1;
  $info = ray('channel', $ch) + compact(ray('uid code boxnum access created'));
  return @db\insert('r_boxes', $info);
}

/**
 * Assign the given account a new id (or delete it).
 * @param int $a: the account to delete (qid or uid are also acceptable)
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by deleteAccount)
 */
function deleteAccount($a) {r\changeUid($a, 0, FALSE, TRUE);}
function changeUid($a, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  if (!$a or !$a = is_object($a) ? $a : r\acct($a)) return w\say(t('nope'), 'err');
  $oldUid = $a->id;
  $nm = $a->fullName;

  if ($delCtty = ($delete and $a->isCtty)) { // deleting a community (merge into Seedpack)
    db\del('users', $oldUid);
    $unusable = $delete = FALSE; // don't actually delete anything except the account record
    $newUid = r\serverUid(); // Seedpack uid
  }
  
  if ($delete) {
    if (db\exists($table = 'r_txs', 'payer=:oldUid OR payee=:oldUid', compact('oldUid'))
      or db\exists($table = 'r_ballots', ':oldUid IN (voter, proxy)', compact('oldUid'))
      or db\exists($table = 'r_usd', 'payee=:oldUid', compact('oldUid'))) {
      return w\say(t('Account "%name" has records in table %table, so it cannot be deleted.', 'name table', $a->fullName, $table), 'err');
    }
  } elseif (!$newUid) {
    $region = @$a->zip ? r\region(@$a->country, @$a->state, $a->zip) : r\communityUid();
    $newUid = r\Acct::nextId($region);
  }
  $subs = array( // all cg tables are listed, to be sure we don't leave any out (D=deletable)
  // r_areas
  // r_bad
    'r_ballots:voter',
    'r_ballots:proxy',
  // r_banks
    'r_boxes:uid:D',
		'r_company:uid:D',
  // r_countries
    'r_coupated:uid',
    'r_coupons:fromId',
    'r_do:uid:D',
    'r_events:ctty',
    'r_gifts:uid:D',
  // r_industries
    'r_investments:coid',
    'r_investments:clubid',
    'r_invites:inviter',
    'r_invites:invitee:D',
    'r_invoices:payer:D',
    'r_invoices:payee:D',
    'r_ips:uid:D',
    'r_near:uid1:D',
    'r_near:uid2:D',
  // r_nonces
  // r_nonmembers
    'r_notices:uid:D',
  // r_options
  // r_pairs
    'r_proposals:ctty',
    'r_proxies:person:D',
    'r_proxies:proxy:D',
  // r_questions
    'r_ratings:uid',
  // r_regions
    'r_relations:main:D',
    'r_relations:other:D',
  // r_request
  // r_states
  // r_shares',
    'r_stakes:uid',
    'r_stakes:clubid',
    'r_stats:ctty:D',
    'r_tous:uid:D',
  // r_transit
    'r_txs:payer',
    'r_txs:payee',
    'r_txs:payerAgent',
    'r_txs:payeeAgent',
    'r_usd:payee',
    'r_user_industries:uid:D',
  // r_votes
    'sessions:uid:D',
    'users:jid',
    'users:uid:D',
    'users:helper',
    'users:community',
  );

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field, $deletable) = explode(':', $one . '::'); // if deleting and not deletable, set to zero
    if ($delCtty and $table == 'users') continue; // for ctty deletion, skip users (already done)
    
    if ($delete) {
      if (!$deletable) {
        if (db\exists($table, "$field=:oldUid", compact('oldUid'))) {
          $DBTX->rollback(); 
          return w\say(t('Record in table %table cannot be deleted.', compact('table')), 'err');
        }
      } else db\del($table, "$field=:oldUid", compact('oldUid'));
    } else db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }

  if ($unusable and !$delete) {
    $oldQid = r\qid($oldUid);
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQid", "unusable$oldQid@" . CG_DOMAIN);
    db\insert('users', compact(ray('uid name email'))); // mark old uid unusable
  }

  unset($DBTX); // commit
  w\say(t('Success. Account %nm has been %what.', 'nm what', $nm, $newUid ? t('changed to ') . $newUid : t('deleted')));
}

/**
 * Package the return arguments and alert staff on certain errors.
 * @param mixed $message: the success or error message OR an array containing all three params
 * @param assoc $info: (optional) message substitutions and other info about transaction
 *   includes success=TRUE if the transaction was successful
 * @param bool $confirm: UNUSED <to be confirmed> (default FALSE)
 */
function txRet($message, $info = []) {
  $info0 = $info;
  if (is_array($message)) {list ($message, $info, $confirm) = $message; $info += $info0;}
  if (!@$info['success']) r\tellCO('failed transaction', $info + compact('message', 'confirm'));
  return [$message, $info];
}

/**
 * Record an invitation
 * @param string $email: email address to which the invite was sent
 * @param int $inviter: the inviter's account uid
 * @param string $code: proof of the invitation (that invitee must have, in order to sign up) (DEFAULT random)
 * @param string $subject, $message: temporary params (can be eliminated from the table at some point)
 *
 * @return string: the code
 * @see also r\Web\invitation()
 */
function invite($email, $inviter, $code = '', $subject = '', $message = '') {
  $code = $code ?: u\code();
  $invited = r\rTime();
  $email = u\cry('P', $email);
  $info = compact(ray('code invited email inviter subject message'));
  db\insert('r_invites', $info);
  return $code;
}

/**
 * Return parameters for notices for the account.
 * @param int $uid: the account record ID
 * @param string $where: criteria to filter notices for this account
 * @param bool $weekly: digest notices for the week
 * @param bool $monthly: digest notices for the month
 * @param int $at: time of reporting
 * @return assoc of all parameters in the notices template (FALSE if none)
 */
function noticeArgs($uid, $where = '0', $weekly = FALSE, $monthly = FALSE, $at = NULL) {
  u\setDft($at, r\rTime());
  $a = r\acct($uid);
  
  if ($where) {
    $q = db\q("SELECT message, created FROM r_notices WHERE uid=:uid AND ($where)", compact('uid'));

    while ($row = $q->fetch()) {
      $date = ($weekly or $monthly) ? (strftime('%b&nbsp;%d', $row->created)) : R_BULLET;
      $messages[] = "<tr><td class=\"notice-date\" stule=\"vertical-align:top; margin-right:5px;\">$date</td><td>$row->message</td></tr>"; // "stule" because CSP forbids inline online (rcron reverts it before emailing)
    }
    if (@$messages) $messages = join("\n", $messages); else return FALSE;
  }
  
  $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
  $yestertime = u\fmtDate($start = $unit == 'month' ? u\plusMonths(-1, $at) : strtotime("-1 $unit", $at));
  $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
  extract(just('return', $a->j_stats()));
  $balance = u\fmtAmt($a->balance);
//  $savings = u\fmtAmt($a->rewards);
  $creditLine = u\fmtAmt(-$a->floor);
  list ($rewards, $ourEmail) = array(u\fmtAmt($a->rewards), $a->cttyA->email);
  $companies = \variable_get('companies');
  $companies = $companies[u\test() ? r\serverUid() : (int) $a->community];
  return compact(ray('messages unit range yestertime balance creditLine return ourEmail companies'));
}

/**
 * Return a list of participating companies for the given community.
 * @param mixed $ctty: uid of community OR zipcode OR zipcode pattern or array of business uids
 * @param bool $email: <format for use in emails and show just new companies> (else format for promo site or Find Co page)
 */
function companies($ctty, $email = FALSE) {
  $model = $email 
  ? '<tr><td width="200">%name</td><td style="margin-left:10px;">%desc</td></tr>'
  : '<div class="row"><div class="cmpy-name">%name</div><div class="cmpy-does">%desc</div></div>';
  $where = (is_numeric($ctty) and strlen($ctty) > 9) ? 'community=:ctty' : 'zip REGEXP :ctty';
  if ($email) $where .= ' AND activated>:REQUEST_TIME-9*:WEEK_SECS'; // show only new companies in emails
  
  $uids = is_array($ctty) 
  ? $ctty
  : db\q("SELECT DISTINCT uid FROM users WHERE :IS_CO AND :IS_OK AND uid>:CANONIC_ACCTS AND $where ORDER BY fullName", compact('ctty'))->fetchCol();
  if (!$uids) return $email ? t('no new companies this month') : t('No companies there yet. Invite them!');
  
  $res = '';
  $_newClass = $email ? 'style="color:red; font-weight:bold;"' : 'class="company-listing"';
  $NEW = t(' <small %newClass> NEW!</small>', compact('_newClass'));
  
  foreach ($uids as $uid) {
    $a = r\acct($uid);
    if (!$a->coCan(CO_PRIVATE)) {
      $name = $a->website ? w\lnk("http://$a->website", $a->bestName) : $a->bestName;
      $new = ($a->activated > REQUEST_TIME - 9 * WEEK_SECS) ? $NEW : '';
      $res .= tr($model, '_name desc', "$name$new", $a->shortDesc) . "\n";
    }
  }
  return $res;
}

/**
 * Return the amount the community has ever paid out
 */
function cttyPaidEver($ctty = 0) {
  u\EXPECT(u\myInt($ctty), 'bad ctty');
  return max(0, -db\sum('balance', 'users', $ctty ? "uid=$ctty" : 'uid<0'));
  $dt = db\max('created', 'r_stats', 'ctty=:ctty', compact('ctty')) ?: 0;
  return db\get(R_CTTY_BENE, 'r_stats', 'ctty=:ctty AND created=:dt', compact('ctty', 'dt'));
}

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region (0 means ALL)
 * @param int $when: latest statistics date to return
 *   if NULL, calculate statics for today
 * @return assoc of named statistics for one community on one date (see r_stats table def)
 */
function stats($ctty = 0, $when = NULL) {
  u\setDft($when, strtotime('today'));
  $sql = 'SELECT * FROM r_stats WHERE created>=:when AND ctty=:ctty ORDER BY created LIMIT 1';

  $info = db\q($sql, compact('ctty', 'when'))->fetchAssoc() ?: r\getStats($ctty, $when);

  extract($info);
  $info['bals'] = $bals = $balsPos + $balsNeg;
  foreach (ray('cg usd') as $z) {
    $info[$z] = ${$z . 'In'} + ${$z . 'Out'};
    $info["{$z}Vol"] = ${$z . 'In'} - ${$z . 'Out'};
    $info["{$z}VolCount"] = ${$z . 'InCount'} - ${$z . 'OutCount'};
  }
  foreach (['', 'In', 'Out'] as $z) {
    if ($ctty) $info["usd$z"] += $info["cg$z"]; // settle trade imbalance with USD
  }
  foreach (['', 'Count'] as $z) {
    $info["txsVol$z"] = ${"p2p$z"} + ${"b2b$z"} + ${"p2b$z"} + ${"b2p$z"} + ${"cashs$z"};
    $info["localVol$z"] = $info["txsVol$z"] - $info[$ctty == 0 ? 'cgOut' : "cgVol$z"];
  }
  
  return $info;
}

/**
 * Calculate and record statistics for the given community.
 * See the "// get" lines to see what statistics are collected and returned.
 * @param int $ctty: community or region (0 means ALL)
 * @param int $created: creation date for the statistics record (defaults to current date)
 * @return the created record
 * @todo: change r field to bal and include replacements for USD
 */
function getStats($ctty, $created = NULL) {
  $created0 = $created;
  u\setDft($created, time());
  $created = strtotime('today', $created); // use start of day
  $subs = compact('created', $ctty ? 'ctty' : '');
  $thisCtty = 'community=:ctty';
  list ($cttyCrit, $payerCrit, $payeeCrit, $cttysCrit) = $ctty ? 
    [$thisCtty, 'payer=:ctty', 'payee=:ctty', "(u.$thisCtty OR u2.$thisCtty)"]
  : ['community<>0', 'payer<1', 'payee<1', '1'];
  $creCrit = 'created<=:created';
  $nonCtty = 'uid>:CANONIC_ACCTS';
  $accounts = "(SELECT uid,jid,community,flags FROM users WHERE $nonCtty)";
  $isMaster = '(jid=0 OR uid<jid)';
  $masterBal = '(balance+IFNULL(balance2, 0))';
  $slave = "(SELECT uid AS uid2, balance AS balance2 FROM users WHERE $nonCtty) u2 ON u2.uid2=u.jid";
  
  $res = [];
  // get account counts (can't be combined with financial data because of joint accounts
  $sql = <<< X
	  SELECT SUM(IF(:IS_OK AND activated<=:created AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(:IS_OK AND activated<=:created AND :IS_CO, 1, 0)) AS bAccts,
      SUM(IF(NOT activated OR activated>:created, 1, 0)) AS newbs
    FROM users u
    WHERE $nonCtty AND $cttyCrit AND $creCrit	
X;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  // get users table financial data (must happen before topN/botN)
  $sql = <<< X
    SELECT 
      SUM(IF($masterBal>0, $masterBal, 0)) AS balsPos,
      SUM(IF($masterBal<0, $masterBal, 0)) AS balsNeg,
      SUM(IF($masterBal>0, 1, 0)) AS balsPosCount,
      SUM(IF($masterBal<0, 1, 0)) AS balsNegCount,
      SUM(IF(:IS_OK, floor, 0)) AS floors
    FROM users u LEFT JOIN $slave
    WHERE $nonCtty AND $isMaster AND $cttyCrit AND $creCrit
X;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  // get bank transfer data
  $in = 'amount>0 XOR txid<0';
  
  $sql = <<< X
    SELECT
    SUM(IF($in, amount, 0)) AS usdIn,
    SUM(IF($in, 0, amount)) AS usdOut,
    SUM(IF($in, 1, 0)) AS usdInCount,
    SUM(IF($in, 0, 1)) AS usdOutCount
    FROM r_usd t LEFT JOIN $accounts u ON u.uid=t.payee
    WHERE $cttyCrit AND t.completed AND t.completed<=:created
X;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

// get transaction table data
  $sql = '';
  list ($cgIn, $cgOut) = $ctty == 0 // make cgIn and cgOut selectors
  ? ['SUM(IF(u.community=u2.community, amount, 0))',
     'SUM(IF(u.community=u2.community, 0, amount))']
  : ["SUM(IF(NOT u.$thisCtty AND u2.$thisCtty, amount, 0))\n",
     "SUM(IF(u.$thisCtty AND NOT (u2.$thisCtty), -amount, 0))\n"];
  foreach (ray('cgIn cgOut') as $k) $sql .= "${$k} AS $k,\n";
  
  foreach (ray('p2b:01,b2b:11,b2p:10,p2p:00') as $k => $v) { // make x2x selectors
    foreach ([0, 1] as $i) ${'not' . $i} = $v[$i] ? '' : 'NOT';
    $sql .= "SUM(IF(goods<>:FOR_USD AND $not0 u.:IS_CO AND $not1 u2.:IS_CO, amount, 0)) AS $k,\n";
  }
  $sql .= "SUM(IF(payer>1 AND goods=:FOR_USD, ABS(amount), 0)) AS cashs,\n";
  $sql .= strtr($sql, ['-amount' => '1', 'amount' => '1', ",\n" => "Count,\n"]); // add counts
  $sql = str_replace(",\n!", '', $sql . '!'); // remove final comma in field list
  
  $sql = <<< X
    SELECT $sql
    FROM r_txs t 
    LEFT JOIN $accounts u ON u.uid=t.payer 
    LEFT JOIN $accounts u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND $creCrit
X;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  
  // get aAccts
  $recent = $created - 60 * DAY_SECS; // not n months (keep length of time consistent)

  $sql = <<< X
    SELECT COUNT(DISTINCT payer) as aAccts FROM r_txs  t
    JOIN $accounts u ON u.uid=t.payer
    WHERE NOT u.:IS_CO AND $cttyCrit AND $creCrit AND created>$recent
X;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  // get conx, conxLocal
  foreach (ray('conx conxLocal', 1, "u2.$thisCtty") as $k => $crit) {
    if (!$ctty) $crit = 1; // avoid local query when doing stats for ALL cttys
    $sql = <<< X
      SELECT AVG(conx) as $k FROM (
        SELECT me, COUNT(DISTINCT other) AS conx, SUM(isRecentPayer) AS recentPays, u2.community
        FROM (
          SELECT u0.uid AS me, 
            IF(u0.uid=payer AND t0.created>$recent, 1, 0) as isRecentPayer,
            IF(u0.uid=payer, payee, payer) AS other 
          FROM users u0
          JOIN r_txs t0 ON u0.uid in (payer, payee) 
          WHERE t0.$creCrit AND u0.$nonCtty and NOT u0.:IS_CO AND u0.$cttyCrit
        ) t LEFT JOIN users u2 ON u2.uid=other WHERE $crit AND u2.$nonCtty
        GROUP BY t.me
      ) u3 WHERE recentPays>0        
X;
    $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  }
  
  if (@$created0 and $created < strtotime('today')) { // redoing stats for the past, so figure bals, floors, and topN the hard way
    $recent = strtotime(-FLOOR_DAYS . ' days', $created); // six months ago
    $isPayer = '(payer IN (uid, jid) AND payer<>0)';
    $isPayee = '(payee IN (uid, jid))';

    // NOTE: xid and txid are needed here to prevent merging
    $txs = <<< X
      ((SELECT txid AS xid, 0 AS payer, payee, amount, completed AS created FROM r_usd)
       UNION (SELECT xid, payer, payee, amount, created FROM r_txs))
X;

    $pastTxs = <<< X
      (SELECT DISTINCT uid,
        SUM(IF($isPayer, -amount, amount)) AS amts,
        SUM(IF($isPayee AND created>$recent, amount, 0)) AS amtsIn,
        SUM(IF($isPayer AND created>$recent, amount, 0)) AS amtsOut
        FROM $accounts u0 LEFT JOIN $txs t0 ON ($isPayer OR $isPayee)
        WHERE amount IS NOT NULL AND $isMaster AND $cttyCrit AND $creCrit GROUP BY uid)
X;

    // get OLD balsPos, balsNeg, floors
    $sql = <<< X
      SELECT 
      SUM(IF(amts>0, amts, 0)) AS balsPos,
      SUM(IF(amts<0, amts, 0)) AS balsNeg,
      SUM(IF(amts>0, 1, 0)) AS balsPosCount,
      SUM(IF(amts<0, 1, 0)) AS balsNegCount,
      -SUM(GREATEST(amtsOut,amtsIn))/6 AS floors
      FROM $pastTxs t
X;
    $res = (db\q($sql, $subs)->fetchAssoc() ?: []) + $res; // put $res 2nd so OLD takes precedence
   
    // get OLD topN
    $topN = topN($res); // NOTE: this line must be duplicated below because $res is different
    $sql = <<< X
      SELECT SUM(amts) AS topN FROM 
        (SELECT DISTINCT uid, amts FROM $pastTxs u1 
         WHERE $nonCtty AND amts IS NOT NULL ORDER BY amts DESC LIMIT $topN) u
X;
    $res += db\q($sql, $subs)->fetchAssoc() ?: [];

    // get OLD botN
    $sql = strtr($sql, ray('topN DESC', 'botN', ''));
    $res += db\q($sql, $subs)->fetchAssoc() ?: [];

    $txs = <<< X
      ((SELECT txid AS xid, 0 AS payer, payee, amount, completed AS created FROM r_usd)
       UNION (SELECT xid, payer, payee, amount, created FROM r_txs))
X;

  } else {
    // get topN
    $topN = topN($res); // NOTE: this line must be duplicated above because $res is different
    $sql = <<< X
      SELECT SUM(bal) AS topN FROM 
        (SELECT $masterBal AS bal FROM users u LEFT JOIN $slave
         WHERE $nonCtty AND $isMaster AND $cttyCrit AND $creCrit GROUP BY uid
         ORDER BY $masterBal DESC LIMIT $topN) u
X;
    $res += db\q($sql, $subs)->fetchAssoc() ?: [];
    
    // get botN
    $sql = strtr($sql, ray('topN DESC', 'botN', ''));
    $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  }
  
  foreach ($res as $k => $v) {if (is_null($v)) $res[$k] = 0;}
  $info = compact('created', 'ctty') + $res;
  if ($created >= R_LAUNCH) db\insert('r_stats', $info);
  return $info;
}

/**
 * Return N or N% the number of active (or business) accounts, whichever is greater.
 * Do not change the default N without recreating all statistics (with a\fixStats)
 */
function topN($res, $n = 3) {return max($n, round($n * array_sum(just('bAccts aAccts', $res)) / 100));}


/**
 * Return the average balance, average credit reserve, and minimum balance for the given account (possibly joint) and period.
 * @param int $uid: account record ID (can be a community or region too)
 * @param int $start: starting date (should be 0 hours) -- first full day to include in the period
 * @param int $end: ending date (should be 0 hours) -- last full day to include in the period
 * @param bool $noj: ignore any joined account
 * @return array: [balance, min, rewards, days]
 *    balance: the average end-of-day balance for that period (possibly with fractional cents)
 *    min:     minimum end-of-day balance in the period
 *    rewards: the average end-of-day credit reserve amount for that period (possibly with fractional cents)
 *    days:    (unused) number of days from start to end, inclusive
 */
function averageBalance($uid, $start, $end = NULL, $noj = FALSE) {
  //u\EXPECT($uid > 1, 'bad uid for avg');
  u\setDft($end, r\rTime());
  if (!$days = round(($end + 1 - $start) / DAY_SECS)) return [0, 0, 0, 0]; // rounding is for daylight savings time changes
  $info = be\creditInfo(ray('uid asof', $uid, $start - 1));
  $min = $bal = $info->balance;
  $rbal = $info->rewards;
  $jid = @$noj ? 0 : (r\acct($uid)->jid ?: 0);
  $isPayer = 't.payer IN (:uid, :jid)';
  $isPayee = 't.payee IN (:uid, :jid)'; // account can be both payer and payee, if joint account
  $signedAmt = "IF($isPayer, -amount, amount)";

  $day = 0;
  $avg = $ravg = 0;

  // use "1" as payee for r_usd, to avoid matching jid=0
  $sql = <<< X
    SELECT SUM($signedAmt) AS total,
    SUM(IF($isPayer, payerReward, payeeReward)) AS rtotal,
    SUM(IF(t.goods=:FOR_SHARE, $signedAmt, 0)) AS share,
    FLOOR((t.completed-$start)/:DAY_SECS) AS day 
    FROM (
      (SELECT type,goods,payer,payee,amount,payerReward,payeeReward,created AS completed FROM r_txs)
      UNION (SELECT :TX_BANK,-:FOR_SHARE,1 AS payer,payee,amount,0,0,completed FROM r_usd)
    ) t WHERE ($isPayer XOR $isPayee) AND completed BETWEEN $start AND $end 
    GROUP BY day
X;

  $q = db\q($sql, compact('uid', 'jid'));

  while($row = $q->fetchAssoc()) {
    $dayx = $day;
    extract($row); // total, rewards, share, day

    $rtotal += $share;
    $total -= $rtotal; // don't include rewards in balance
    $avg += $bal * ($day - $dayx) / $days;
    $ravg += $rbal * ($day - $dayx) / $days;
/// debug(compact('uid','day','dayx','days','bal','avg','total', 'start','end'));
    $bal += $total;
    $rbal += $rtotal;
    $min = min($min, $bal);
  }
  $dayx = $day;
  $day = round(($end - $start) / DAY_SECS); // end of period
  $avg += $bal * ($day - $dayx) / $days;
  $ravg += $rbal * ($day - $dayx) / $days;
  $min = round($min, 2);
/// debug(compact('uid','day','dayx','bal','avg'));

  return [$avg, $min, $ravg, $days];
}

/**
 * Return the current time (manipulated for the rCredits Game and possibly for tests)
 */
function rTime() {
  if (isGAME) {
    extract(w\gameVars());
    return mktime(0, 0, 0, $week, 4 * $day, date('Y'));
  } else return u\test() ? strtotime('today') : time(); // so times in tests are reliable
}

/**
 * Interpret an integer bit array of risks.
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @return formatted list of risk names, with HTML titles
 */
function riskList($risks, $descs) {  
  global $riskHelp, $base_url;

  $bitNames = array_keys($descs = ray($descs));
  foreach ($bitNames AS $bit => $bitName) {
    if (u\getBit($risks, $bit)) {
      $class = $descs[$bitName] < 0 ? 'goodRisk' : 'badRisk';
      $helpIndex = @$riskHelp ? array_search($bitName, array_keys($riskHelp)) : -1;
      $why[] = w\popHelp($bitName, $riskHelp[$bitName], "$base_url/help/risks@$bitName", $class);
//      $why[] = "<a href=\"$base_url/help/flags@$bitName\" class=\"$class riskHelp$helpIndex\">$bitName</a>";
    }
  }
  return @$why ? join(', ', $why) : '';
}

/**
 * Check account permissions for a transaction or invoice. Note that invoicing a not-yet-complete account is permitted.
 * @param acct $a1: the account paying or requesting payment
 * @param acct $a2: the other account
 * @param bool $taking: the first account is taking money from the second (charging or invoicing)
 * @param bool $neg: the amount is negative (refunding or undoing)
 * @return [message, subs, confirmed] on error, else false
 */
function txPermErr($a1, $a2, $taking = TRUE, $neg = FALSE) {
  global $scanned, $channel;
  
  $invoicing = ($taking and $channel == TX_WEB);
  if ($invoicing and !$a2->confirmed) $a2->setBit(B_CONFIRMED);

  if (!$a2->ok) if (!$invoicing or $a2->closed) return array('non member', ['otherName' => $a2->fullName], FALSE); // okay to invoice (and maybe someday pay) a not-yet-active, but not a no-longer-active account
  u\EXPECT($a1->ok, 'main account inactive tx');
  foreach ([$a1, $a2] as $a) {
    $other = $a == $a1 ? $a2 : $a1; // if neither confirmed nor confirming icard by tx with inviter
//    if (!$a->confirmed and !($a->iCode == IBY_ICARD and $a->helper == $other->id)) { 
//    if (!$a->confirmed and $a->helper != $other->id) { 
    if (!$a->confirmed and $other->id > 0 and $other->id != r\cgfId()) { // communities and CG can transact with anyone
      if ($a1->community != $a2->community or $channel != TX_POS) { // first tx must be in-person at home
        return FALSE ? ['not confirmed', ray('youName inviterName', $a->fullName, $a->helperA->fullName), FALSE]
        : ['first at home', ray('whose', $a == $a1 ? t('Your') : "$a->fullName's"), FALSE]; // "FALSE ?" should be $a1->cttyA->invites ?
      }
    }
  }
  list ($myAccess, $otherAccess) = u\order($taking xor $neg, B_SELL, B_BUY);
  //list ($myAccessErr, $otherAccessErr) = u\order($taking, 'sell', 'buy');
  list ($myWhat, $otherWhat) = u\order($taking xor $neg, t('sales'), t('purchases'));
  if ($scanned and $taking) list ($myAccess, $myWhat) = $neg ? array(B_REFUND, t('refunds')) : array(B_SCAN, t('sales'));
///  debug(compact(ray('taking neg scanned myAccess otherAccess myWhat otherWhat')));
  if (!$a1->can($myAccess)) return array("no perm", ray('what', $myWhat), FALSE);
  if (!$a2->can($otherAccess)) if (!($invoicing and $a2->proSe)) return array("other no perm", ray('otherName what', $a2->agentA->fullName, $otherWhat), FALSE);
//  if (!$a1->can($myAccess)) return array("no $myAccessErr", [], FALSE);
//  if (!$a2->can($otherAccess)) return array("other no $otherAccessErr", ray('otherName', $a2->agentA->fullName), FALSE);

/*
  if ($a1->id > 0 and $a2->id != r\cgfId()) { // allow rewards and donations in not-yet-active communities
    if (!$a1->cttyActive) return array('community not active', ray('whose', t('Your')), FALSE);
    if (!$a2->cttyActive) return array('community not active', ray('whose', t('That account\'s')), FALSE);
  }
  */
  if (!$a1->cttyUp) return array('your community down', ray('contact', $a1->cAdminContact()), FALSE);
  if (!$a2->cttyUp) return array('other community down', [], FALSE);
  if ($a1->id == $a2->id) return array('no self-trading', [], FALSE);
  return FALSE;
}

function passHash($pass) {
  u\EXPECT(is_string($pass) and $pass !== '', 'bad pass to hash');
  require_once DRUPAL_ROOT . '/includes/password.inc';
  return \user_hash_password($pass);
}

/**
 * Return the median (untested)
 */
function median($field, $table = 'users', $where = 1, $subs = []) {
  $middle = db\count($table, $where, $subs) / 2;
  $limit = floor($middle) . (is_int($middle) ? ', 1' : ', 2');
  return db\get('AVG($field)', 'users', "$where ORDER BY $field LIMIT $limit", $subs);
}

/**
 * Return the next sequential bank transfer number for an account.
 * @param int $payee: the account uid
 * @return int: the next unused positive integer (1 if there are no bank transfers yet)
 */
function getUsdTid($payee) {
  return db\get('MAX(tid)+1', 'r_usd', 'payee=:payee', compact('payee')) ?: 1;
}

/**
 * Send the account a notice for the suggested action, with a direct link to do it.
 * The link does not require the user to sign in.
 * FOR NOW: just suggest it, with no direct link. (never a link if channel is SMS)
 */
function suggest($a, $index, $subs, $page = 'Preferences') {
  // $subs['url'] = 
  // $index .= '|click option';
  r\notify($a->id, $index, $subs + compact('page'));
}

/**
 * Return the url for a page on the promo site, with a way to come back still signed in.
 * @param string $page: what page on the promo site to visit
 * @return the full url
 */
function promoLink($page = '') {
  if (r\signedIn()) $page .= '?region=' . R_SERVER_ID . '&tm=' . r\rTime();
  return PROMO_URL . "/$page";
}

/**
 * Start/Stop the system OR return the "up" status.
 * @param bool $up: whether to start (TRUE), stop (FALSE), or query (NULL, the default)
 * @return TRUE if the system is up
 */
function up($up = NULL) {
  if (is_null($up)) return \variable_get('up', TRUE);
  u\log('System is ' . ($up ? 'UP' : 'DOWN'));
  return \variable_set('up', $up);
}

/**
 * Return the reward on the given amount for the given account.
 * @param int $from: payer account record ID
 * @param int $to: payee account record ID
 * @param numeric $amount: transaction amount
 * @param bool $isPayer: <return reward for payer> (else payee)
 * @todo? or manager of company with no (other) employees (set bit?)
 * (not used): Figure the rebate based on R = the payer's total rewards ever and A = the transaction amount,
 *   such that the rebate percentage p tapers linearly from Rmax to 0
 *   The percentage rebate is based on the total rewards AFTER the transaction. That is:
 *   The new percentage rate p = (R + pA)(Pmax-Pmin)/Rmax + Pmin
 *   So p = (RPmax + (Rmax - R)Pmin) / (Rmax^2 - A(Pmax - Pmin))
 * Used here: If the member's balance after the transaction is negative, their reward is negative the maximum reward (-10%).
 *   Otherwise the reward percentage starts at the maximum (10%), then decreases by 1% for every chunk ($500) of rewards
 *   they have received, down to the minimum (1%).
 *   
 */
function reward($from, $to, $amount = 1, $isPayer = TRUE) {
  global $testRewardStep;
  if (relation(1, $to, $from, ':IS_OWNER OR :IS_DRAW')) return 0;
  if (relation(1, $from, $to, ':IS_OWNER OR :IS_DRAW')) return 0;
  $a = r\acct($isPayer ? $from : $to);
//  $pct = u\test() ? $a->rebate : max(R_MIN_REWARD_PCT, R_REBATE - floor($a->rewards / R_REWARD_STEP));
  $step = @$testRewardStep ?: R_REWARD_STEP;

  $over = (($isPayer xor $amount < 0) and $a->balance + $a->rewards - abs($amount) < max(0, $a->floor)); // ignore floor<0
  if ($over) { // member is spending in (or into) debt
//    $pct = $a->cttyNoreduce ? 0 : -max($a->rebate, R_REBATE); // reduce rewards
    $pct = -max($a->rebate, R_REBATE); // reduce rewards
  } else $pct = max(R_MIN_REWARD_PCT, $a->rebate * (1 - floor($a->o_rewards / $step) / R_REBATE)); // increase rewards
  if (!$isPayer) $pct *= R_BONUS_FACTOR;
///   debug(compact(ray('from to amount pct step over res')) + ['new bal' => $a->balance + $a->rewards + ($isPayer ? -$amount : $amount)]);
  return round(min(R_MAX_REBATE, $pct * $amount / 100), 2);
//  return '' . round(min(R_MAX_REBATE, $amount * $a->rebate / 100), 2); // ''. to avoid serializing a long float
}

/**
 * Handle the investment request for one account in one club.
 * @param int $stakeid: record ID of the account's stake
 * @param string $msg: (RETURNED) a message, if any, resulting from the attempt
 * @return <successful>
 */
function handleInvestmentRequest($stakeid, &$msg) {
  if (!$res = db\get('stake,request,uid,clubid', 'r_stakes', compact('stakeid'))) return !$msg = t('bad stakeid');
  extract($res);
  
  if ($request <= 0) return (bool) $msg = t('No additional investment is requested.');
  $a = r\acct($uid);
  $clubA = r\acct($clubid);
  $stake += $request;
  
  $DBTX = \db_transaction();
  list ($index, $subs) = $a->pay($clubA, $request, t('investment'), ray('stake goods', 1, FOR_USD));
  $msg = t($index, $subs);
  if ($ok = $subs['success']) {
    db\update('r_stakes', ray('stakeid stake request', $stakeid, $stake, 0), 'stakeid');
    $msg .= t('Your investment in %clubName has been successfully increased by %request.', 'clubName request', $clubA->fullName, u\fmtAmt($request));
  }
  unset($DBTX);
  return $ok;
}

/**
 * Return the appropriate exchange fee for the given transaction description and amount.
 */
function xfee($for, $amount) {
  if (is_array($for)) $for = $for[1];
  if (!r\usdin($for)) return '';
  if (strpos($for, BY_CASH)) return 0;
  return '' . round(strpos($for, BY_CARD) ? -R_XFEE_CARD * $amount : R_XFEE_CHECK, 2);
}
  
// Single line functions that need no explanation
function serverUid($regionId = R_SERVER_ID) {return -1 - R_REGION_MAX * u\a2n($regionId);} // -u\a2n($regionId) - 1;}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('%body', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
function realCountry($country) {return db\get('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\get($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
//function doCode($code) {return @unserialize(@u\decry('P', CRYPT_FLAG . $code));} // this is how it should be
function doCode($code) { // accommodate temporary weirdness because email clients bollix CRYPT_FLAG in an href
  foreach ([0, 1] as $i) {if ($code[0] == CRYPT_FLAG[$i]) $code = substr($code, 1);} // delete this line after 7/1/2018
  if (substr(u\b64decode($code), 0, 2) == 'c~') return FALSE; // signal archaic link
  return @unserialize(@u\decry('P', CRYPT_FLAG . $code));
} 
function makeDoCode($ray) {return substr(u\cry('P', serialize($ray)), strlen(CRYPT_FLAG));}
function cgfId() {return \variable_get('cgf_uid');}
function disputeTx($xid) {return db\setFlag('r_txs', $xid, B_DISPUTED, TRUE);}
function denyInv($nvid) {return db\setData('r_invoices', $nvid, ['denied' => TRUE]);}
function stepsDone0() {return array_fill_keys(array_keys(ray(R_STEPS)), FALSE);}
function isReward($type) {return in($type, TX_REWARDS);}
//function usdin($for) {return u\abbreviates('usd in (', strtolower(is_array($for) ? $for[1] : $for));}
function usdin($for) {return u\abbreviates(S_USD_IN, is_array($for) ? $for[1] : $for, TRUE);}
function regionField($fld) {return ($mya = r\acct(serverUid())) ? $mya->$fld : NULL;}
function signedIn() {return r\acct() ? TRUE : FALSE;}
function iCodeDesc($iCode) {$iCodeDescs = ray(IBYS); return $iCodeDescs[min($iCode, IBY_ICARD) - IBY0];}
function cttyList() {return db\q("SELECT uid,fullname FROM users WHERE uid<0 ORDER BY created,fullName")->fetchAllKeyed();}
function allKnowing() {global $mya; return (bool) $mya->agentA->vKeyPw;}
function clubid() {return r\serverUid();} // temporarily use the region as the club