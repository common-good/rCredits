<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in rcredits-backend.inc (i)
 *   - utilities, which are in rcredits-util.inc (u)
 */
 
namespace rCredits; // typically abbreviated as "r"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Testing as t;
use rCredits\Util as u;
use rCredits\Backend as be;
use rCredits\Web as w; // use only in rTime (for the game)

require_once __DIR__ . '/rcredits-strings.inc';
require_once __DIR__ . '/rcredits-db.inc';
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
      r\tellAdmin('cannot save regional ' . $e->getMessage(), $record);
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
  if (!$inv = db\lookup($fields, 'r_invoices', 'nvid=:nvid', compact('nvid'))) return 'no matching invoice';
  if ($inv['status'] > 0) return 'already paid';
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
  return db\lookup($max, 'r_txs', $where, compact('uid', 'xid', 'jid')) + 1;
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
  if (!$lastTx = r\acct($uid)->lastTx) return FALSE;
  list ($xid, $dif, $oldAmt, $created) = $lastTx;
///  t\log(compact(ray('uid other amount lastTx')));
  return ($dif == $other - $uid and $amount == $oldAmt and r\rTime() - $created < R_DUPTX_MINS * MIN_SECS);
}
 
/**
 * Say whether the named employee works for the account owner.
 * @param string $name: the employee's name
 * @param int $us: the employer's account ID (defaults to current account ID)
 * @param bool $loose: <the employee need not be formally an employee>
 * @return int: the person's account ID if s/he does work for us, otherwise FALSE
 */
function worksForUs($name, $us = NULL, $loose = TRUE) {
  u\setDft($us, r\acct()->id);
  $words = explode(' ', u\roughName($name)); // abbreviate middle names
  if (strlen($words[0]) > 1) { // unless first name is already abbreviated
    for ($i = 1; $i < count($words) - 1; $i++) $words[$i] = substr($words[$i], 0, 1) . '%';
  }
  $pattern = join(' ', $words);
  $where = 'r.main=:us' . ($loose ? '' : ' AND r.employee');
  
  $sql = <<<EOF
    SELECT u.uid, u.fullName FROM r_relations r
    INNER JOIN users u ON u.uid=r.other
    WHERE $where
    ORDER BY (u.fullName LIKE :pattern) DESC
EOF;
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
 */
function setJoin($reid, $join) {
  extract(r\relation('main,other', $reid));
  list ($a1, $a2) = [r\acct($main), r\acct($other)];

  if ($join) {
    $a1->unjoin(); // zap any join or other join requests
    if (r\relation('permission', $other, $main) == r\perm(B_JOINT)) { // confirming join
      $a1->join($a2);
    } else { // requesting join
      $_atag = $a2->makeDo('join', $main); // seek agreement
      r\message($other, 'join accounts', ray('_atag name', $_atag, $a2->fullName));
    }
  } else $a1->unjoin($a2);
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
    $ray[r\serverUid()] .= t(' (outside any %PROJECT community)');
    return w\selectFld(t('Community:'), '', w\dft($ctty), $ray);
  } else return $ray;
}

/**
 * Wrap all email, so it doesn't go out when testing.
 * @param string $index: index to email template and subject (empty if supplying explicit body and subject)
 * @param mixed $to: where to send the message -- uid, account, or email address
 * @param assoc $subs: substitutions in template. may also include:
 *   noFrame:  don't show salutation and closing and footer
 *   subject:  specific subject
 *   body:     specific message
 * ... the following special substitions are always available:
 *   site, qid, shortName, and every cached field for the account
 * @param mixed $reply: uid, account, or email address to reply to OR 'ctty' (of $to) or NULL (same as from)
 * @return TRUE if the message got sent (or on dev machine pretending it got sent) else FALSE
 * NOTE: from is always SYS_EMAIL
 */
function rMail($index, $to0, $subs, $reply0 = NULL) {
  if (isGAME) return; // no emails from game site
  global $testEmails;

  $subs['site'] = BASE_URL;
  $subs['orgPhone'] = CGF_PHONE;
  extract(u\just('subject body noFrame sms', $subs));
  
  if ($toA = is_object($to0) ? $to0 : r\acct($to0)) {
    $toEmail = $toA->email;
    $to = "\"$toA->fullName\" <$toEmail>";
    $subs += (array) $toA->account(); // this omits any special fields or special formatting
    $subs['qid'] = $toA->mainQid;
    $subs['shortName'] = $toA->name;
  } else $to = $toEmail = $to0;
  
  if (in_array($to, ['jeannine+blossom@wetmeadow.org'])) return FALSE; // invalid or marked us as spam

  $testEmails[] = compact(ray('index toEmail subs'));

  if ($reply0 == 'region') {
    $reply = NULL;
  } else {
    $replyA = is_object($reply0) ? $reply0 : ($reply0 == 'ctty' ? $toA->cttyA : r\acct($reply0));
    $reply = $replyA ? "\"$replyA->fullName\" <$replyA->email>" : $reply0;
    if ($reply == $to) $reply = NULL;
  }

  $nolog = ray(R_NOLOG); // subs not to log (sensitive data)
  foreach ($subs as $k0 => $v) if (!is_object($v) and !is_array($v)) {
    $k = "%$k0"; // '{' . $k0 . '}';
    $resubs[$k] = $v;
    $logSubs[$k] = in_array($k0, $nolog) ? '(?)' : $v; // hide personal info and secrets
  }

  $index = str_replace('_', '-', $index); // correct Drupal template names to rCredits standard
  $subject = u\SUBS(strtr(@$subject ?: $GLOBALS['emailSubjects'][$index], $resubs), '%');
  $body = u\SUBS(strtr(@$body ?: (@$noFrame ? r\emailTemplate($index) : r\emailBody($index)), $resubs), '%');

  // log
  $message = str_replace("\r\n", '', strtr($body, $logSubs)); // easier reading for log and tests
  $erInfo = compact(ray('index toEmail subject message'));
/**/ t\output('Actual (in rMail) ' . print_r($erInfo, 1), 'email');
  u\loga('email', $erInfo + ['toUid' => @$toA->id]);

  if (!isPRODUCTION) if ($toEmail != r\regionField('email') and $toEmail != R_ADMIN_EMAIL) return TRUE;
  if (isDEV) return TRUE; // otherwise cURL error 60: SSL certificate problem: unable to get local issuer certificate
  $from = t('%PROJECT system <%SYS_EMAIL>');
  
  if (@$sms) { // send as text message
    $success = mail($toEmail, '', $body, "From: $from", '-f bounce@' . REGION_DOMAIN); // this is an email-to-sms address
  } else { // if (in_array($toEmail, [R_ADMIN_EMAIL, SYS_EMAIL])) { // send as normal email
    $success = mail($to, $subject, $body, "Content-Type: text/html; charset=ISO-8859-1\nFrom: $from" . ($reply ? "\nReply-To: $reply" : ''), '-f bounce@' . REGION_DOMAIN);
  }
/*
  else { // send as more likely-to-be-delivered email
    require_once DRUPAL_ROOT . '/vendor/autoload.php';
    $client = new \Postmark\PostmarkClient(isDEV ? POSTMARK_API_TEST : POSTMARK_TOKEN);
    try {
      $success = is_object($res = $client->sendEmail($from, $to, $subject, $body, NULL, NULL, NULL, $reply));
/** /  $erInfo += ['error1' => print_r(@$res, 1)];
    } catch(Exception $e) {$erInfo += ['error' => $e->getMessage()];}
  }
  */
  
  return @$success ?: r\tellAdmin('email fail', $erInfo);
}

/**
 * Add a deactivated or phony (and used) card code to the "bad" list.
 * @param string $qid: the account code
 * @param string $code: the bad card code
 * @param string $msg: type of error
 */
function badCard($qid, $code, $msg) {
  $created = time();
  if (!db\exists('r_bad', $info = compact('qid', 'code'))) {
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
  $message = t($index, $subs);
  $topic = $topic ?: (t(@$GLOBALS['topics'][$index], $subs) ?: $index); // index should never be used literally (wrong language) but more importantly never fail
  $testMessages[] = compact(ray('uid index subs topic'));
///  debug(compact('uid','index','subs','message','topic'));
  r\notify($uid, 'message', compact('message', 'topic'), TRUE);
}

/**
 * Notify the specified user, according to their preferences
 * @param integer $uid: account record id
 * @param string $index: message index
 * @param assoc $subs: substitutions for the message
 * @param bool $force: send by email NOW
 */
function notify($uid, $index, $subs = [], $force = FALSE) {
  global $channel;
  if (isGAME) return;
  $a = r\acct($uid);
  u\EXPECT($a, 'no account');
  $subs['uid'] = $uid;
  $subs['otherName'] = @$subs['myName']; // notifying someone else about US, if anyone
  $fullName = $a->fullName;
  $warnIndexes = ray('new invoice,new offer,charge disputed');
  
  if ($force) {
    $mailed = r\rMail($index, $a, $subs, 'ctty');
  } else { //if ($a->can(B_NOTIFY_EMAIL)) {
    list ($message, $created) = array(t($index, $subs), r\rTime());
    t\output("Actual Notice to $fullName: $message");
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
 * If appropriate, update an account's membership status and send the staff an alert.
 * @param acct $a: the account
 * @param string $status: what milestone or event to alert about
 * @param bool $do: prerequisite for the alert
 * @return <the membership event happened and was handled>
 * If there is an appropriate bit to set, it will be set as appropriate.
 */
function membershipEvent($a, $status, $do = TRUE) {
  global $channel;
  
  $bit = @u\consta('b', $status); // get the appropriate bit, if any
  if (!$do or ($bit !== FALSE and $a->can($bit))) return FALSE; // do nothing or bit already set

  $DBTX = \db_transaction();
  if ($bit !== FALSE) $a->setBit($bit); // set the bit, if any
  $fullName = $a->fullName;

  if ($status == 'ok') {
    u\EXPECT(@r\acct()->cAdmin, 'should be ctty admin'); // only ctty admin can approve an account
    $inviterName = @$a->helperA->fullName;
    $subs = compact('inviterName');
    $msg = 'approved';
    if (!$a->confirmed) {
      $msg .= '|must be confirmed';
      if ($channel == TX_WEB) w\say('must be confirmed', $subs);
    }
    if ($a->seq >= IBY_ICARD) $msg .= '|use temporary card';
    r\message($a->id, $msg, $subs);
    $a->update('activated', r\rTime());
  } elseif ($status == 'bona') {
    be\fund($a->id, TX_SIGNUP, R_SIGNUP_BONUS, t('signup bonus'));

    if (!$a->helperA->admin) { // no helper bonus for admin (cAdmin is okay) [don't require $a->confirmed]
      be\fund($hId = $a->helper, TX_HELPER, R_HELPER_BONUS, t('invite reward', compact('fullName')));
      $helped = db\count('users', ':IS_OK AND NOT :IS_CO AND helper=:hId', compact('hId')) + 0;
      if ($helped == ORGANIZER_GRANT_MIN) {
        r\message($a->id, 'organizer grant', ['_a' => w\atag('/community/grant')]);
        r\tellAdmin('eligible for grant', ['name' => $a->helperA->fullName]);
      }
    }
  }
  unset($DBTX);
  
  $qid = $a->mainQid;
  tellCO('event - ' . $status, compact(ray('fullName qid status')), $a->id);
  return TRUE;
}

/**
 * Try to transfer funds from donor to CGF.
 * @param assoc $info: the gift record data
 * @return TRUE if the funds were successfully transferred.
 */
function acceptGift($info) {
  extract(u\just('donid uid giftDate amount often honor', $info));
  $a = r\acct($uid);
  if ($a->shortfall(FOR_GOODS, $amount)) return FALSE; // wait until tomorrow, if not enough
  if ($a->shortfall(FOR_GOODS, $amount, TRUE)) return FALSE; // actually draw from other accounts, as needed
  $cgf = r\acct(r\cgfId());

//  r\usd::beginAtom();
  if ($often != 1) {
    $months = $often == 'M' ? 1 : ($often == 'Q' ? 3 : 12);
    $recurs = ray(t('once monthly quarterly annual'));
    $count = db\count('r_gifts', 'uid=:uid AND often=:often', compact('uid', 'often'));
    $recurring = t(' (%how gift #%count)', 'how count', $recurs[strpos('1MQY', $often)], $count);
  }

  $sharing = (@$honor == 'share');
  list ($goods, $purpose) = $sharing ? [FOR_SHARE, R_SHARING_MSG] : [FOR_GOODS, t('donation') . @$recurring];
  list ($msg, $args, $confirm) = be\transfer('payment', $a, $cgf, $amount, $goods, $purpose);
  if (@$args['success']) {
    $completed = r\rTime();
    db\update('r_gifts', compact('donid', 'completed'), 'donid');
    if ($often != 1) {
      $giftDate = u\plusMonths($months, $giftDate);
      $share = -1; // mark this field N/A (irrelevant)
      db\insert('r_gifts', compact(ray('uid giftDate amount often share')));
    };
//    r\usd::commit();
    r\tellAdmin('gift accepted', $info + $args);
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
  foreach (ray(R_SECURE_FIELDS) as $k) unset($subs[$k]); // secret stuff
  ksort($subs); // put the keys in a predictable order, for testing and easy reading

  if (!$tellCO = isset($uid)) $uid = @$subs['uid'];
  if ($mya = r\acct(@$uid)) $who = "$mya->fullName ($mya->mainQid)"; // (default to current account)
  $message = t($index, $subs) . u\seeAssoc($subs + compact('who'));
    
  if ($tellCO) { // account specified, so tell Community Admin
    $email = $mya->cttyA->email;
  } else list ($email, $uid) = [R_ADMIN_EMAIL, $uid ?: ($mya ? $mya->id : NULL)]; // no account specified so tell SysAdmin

  $noFrame = TRUE;
  $topic = strlen($index) < 30 ? strip_tags($index) : '';
  $subs = compact(ray('who email message topic noFrame uid')) + $subs;
  r\rMail('tell-staff', $email, $subs, $uid);
  if ($email != R_ADMIN_EMAIL) r\rMail('tell-staff', SYS_EMAIL, $subs, $uid); // always tell one or more staff (for now)
  return FALSE;
}

function tellCO($index, $subs = [], $uid = NULL) {return tellAdmin($index, $subs, $uid ?: r\acct()->id);}

/**
 * Return a unique temporary name related to the given uniquely identifying parmeters 
 * in this order of preference: 
 *   phone: "unknown phone dddddddddd"
 *   email: "unknown whatever AT domain DOT ext"
 *   UNUSED (because it's hard to verify): website: "unknown domain DOT ext SLASH andsoforth"
 * If none, just pick a 10-character random string ("unknown 234h2khpsdf987")
 * @param array $info: associative array of fields, each of which uniquely identifies a user.
 * @return string: the temporary name
 */
function tempName($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = @$my_number ?: (@$my_phone ?: '');
  if ($phone) return R_TEMP_NAME_PREFIX . u\fmtPhone($phone, 'n');
  if (isset($my_email)) return R_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return R_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return R_TEMP_NAME_PREFIX . substr(md5('zot' . strval(r\rTime())), 0, 10); // pseudo random
}

/**
 * Return the rough (physical) location of the entity with the given account (city, state, non-US country).
 * @param acct $a: the account
 * @param bool $complete: if TRUE, include the street address
 */
function location($a, $complete = FALSE) {
  $country = realCountry($a->country);
  $state = r\realState($a->state, $a->country);
  $result = [];
  if ($complete) $result[] = (string) $a->address; // (string) in case address is NULL
  $result[] = $a->city;
  $result[] = $state . (($complete and $a->postalCode) ? ' ' . $a->postalCode : '');
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

  if (!$a = r\acct("$region.$tail")) { 
  // temporarily allow '1' for 'I' (until June 2016 or so) -- not forever because we now use Verdana
    return strpos($code, '1') === FALSE ? FALSE : iCardAcct(str_replace('1', 'I', $code0, $iCode));
  } else return ($security == $a->lowSecurity) ? $a : FALSE;
}

/**
 * Set a cookie
 * @param string $key: name of cookie
 * @param string $value: value of cookie
 * @param int $expires: time when cookie expires (defaults to never -- about 1000 years from now) 0=end of session
 * @return: the value
 */
function setCook($key, $value, $expires = NULL) {
  u\setDft($expires, r\rTime() + 365250 * DAY_SECS);
  setcookie($key, $value, $expires, '/'); // make it easy (fails without '/')
  $_COOKIE[$key] = $value; // make it available also before going to a new page
  return $value;
}

/**
 * Return the rCredits region id for the given account id
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
  $where = ":zip RLIKE postalCode OR (postalCode='' AND country_id=:country AND s.id=:state) ORDER BY (postalCode='')";
  $table = 'r_regions r LEFT JOIN r_states s ON s.abbreviation=r.st';
  if (!$region = db\lookup('region', $table, $where, compact('state', 'zip', 'country'))) {
    $region = db\lookup("CONCAT(iso_code, 'A')", 'r_countries', 'id=:country', compact('country'));
  }
  return r\qo('!' . ($raw = $region ?: 'ZZZ') . 'AAA')->id;
}

/**
 * Say whether one of the accounts owns the other (or can draw from it).
 */
function owning($uid1, $uid2) {
  $where = '(isOwner or draw) AND ((main=:uid1 AND other=:uid2) OR (main=:uid2 AND other=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second
 * @param int $company: defaults to current uid
 */
function isEmployee($uid, $company = '') {
  return relation(1, $company ?: r\acct()->id, $uid, 'employee');
}

/**
 * Return the community record ID for the given postal code.
 * @param string $postalCode: postal code to identify community (if none, assume something in server community)
 * @param bool $defaultZero: return 0 if postalCode does not match any community? (else return server uid)
 */
function communityUid($postalCode = '', $defaultZero = FALSE) {
  if (!$postalCode) return serverUid();
  $where = "country=:R_COUNTRY_ID AND :postalCode RLIKE postalCode AND uid<1";
  return db\lookup('uid', 'users', $where, compact('postalCode')) ?: ($defaultZero ? 0 : serverUid());
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($fullName) {
  $name = $name0 = u\shortName($fullName);
  for ($i = 2; ; $i++) {
    if (!db\exists('users', 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name or email.
 * @todo later: when regional prefix is required for signin, this function will ignore it in matching name field
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if (u\isQid($id = trim(strtoupper($id))) and $acct = r\acct($id)) return $acct->id; // try to interpret as a qid
  $mail = u\cryptN(strtolower($id));
  return db\lookup('uid', 'users', 'name=:id OR mail=:mail', compact('id', 'mail'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $myid = r\acct()->id;
  return db\q('SELECT other FROM r_relations WHERE main=:myid AND employee', compact('myid'))->fetchCol();
}

/**
 * Return the field or fields from the relations table, for the given main and other.
 * Call by:
 *   relation(field, main, other, where, subs) OR
 *   relation(field, where, subs)
 *   relation(field, reid)
 *   relation(field, qid)
 * @param string $field: what field value(s) to return (see varieties at db\lookup())
 * @param int $main: uid of main account
 * @param int $other: uid of other (agent) account
 * @param string $where: SQL criteria
 * @param assoc $subs: criteria parameters
 * @param int $reid: relations record id
 * @param string $qid: 
 */
function relation($field, $main, $other = [], $where = 'TRUE', $subs = []) {
  if (is_numeric($main)) {
    if (empty($other)) {
      if (is_numeric($reid = $main)) {
        $where = "reid=:reid";
        $subs += compact('reid');
      } else {
        list ($main, $otherNum) = explode('-', $main);
        list ($main, $otherNum) = [r\qo($main)->id, u\a2n($otherNum)];
        list ($where, $subs) = ['main=:main AND otherNum=:otherNum', compact('main', 'otherNum')];
      }
    } else {
      $where = "main=:main AND other=:other AND ($where)";
      $subs += compact('main', 'other');
    }
  } else list ($where, $subs) = array($main, $other); // called with alternate form
  return db\lookup($field, 'r_relations', $where, $subs);
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
  $previous = db\lookup('u.uid', 'users u LEFT JOIN users u2 ON u2.uid=(u.uid+1)', $where, compact('base'));
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
  $page = urlencode($_SERVER['REQUEST_URI']);
  if ($class) $class = " class=\"$class\"";
  return "<a href=\"$base_url/change-account?acct=$uid&page=$page\"$class>$v</a>";
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
  $mya = r\acct();
  
  u\setDft($ch, $channel);

  if ($ch == TX_WEB and $mya and $mya->admin) return 0; // don't create spurious devices as admin
/*  if ($boxName === '0') {
    $where = "t1.boxName REGEXP '^[0-9]+$' AND t1.uid=:uid";
    $boxName = u\firstUnusedId('boxName', 'r_boxes', 1, PHP_INT_MAX-1, $where, compact('uid'));
  } else */
  //if (!$boxName) $boxName = $code; // code serves as boxName for dumb cell phones
  $created = $access = r\rTime();
  $boxnum = db\lookup('MAX(boxnum)', 'r_boxes', 'uid=:uid', compact('uid')) + 1;
  $info = ray('channel', $ch) + compact(ray('uid code boxnum access created'));
  return @db\insert('r_boxes', $info);
}

/**
 * Assign the given account a new id (or delete it).
 * @param int $oldUid: the account's current record ID
 * @param int $newUid: the new record ID to give the account (defaults to next available)
 * @param bool $unusable: whether to mark the old account record ID as unusuable
 * @param bool $delete: whether to delete the account rather than giving it a new ID (called by deleteAccount)
 * @return the new uid
 */
function deleteAccount($uid) {r\changeUid($uid, 0, FALSE, TRUE);}
function changeUid($oldUid, $newUid = 0, $unusable = FALSE, $delete = FALSE) {
  $a = r\acct($oldUid);
  if ($delete) {
    if (db\exists($table = 'r_txs', 'payer=:oldUid OR payee=:oldUid', compact('oldUid'))
      or db\exists($table = 'r_ballots', ':oldUid IN (voter, proxy)', compact('oldUid'))
      or db\exists($table = 'r_usd', 'payer=:oldUid', compact('oldUid'))) {
      return r\Web\say('Account "%name" has records in table %table, so it cannot be deleted.', ray('name table', $a->fullName, $table), 'ERR');
    }
  } elseif (!$newUid) {
    $region = @$a->postalCode ? r\region(@$a->country, @$a->state, $a->postalCode) : r\communityUid();
    $newUid = r\Acct::nextId($region);
  }
  $subs = array( // all tables are listed, to be sure we don't leave any out
  // r_areas
  // r_bad
    'r_ballots:voter',
    'r_ballots:proxy',
  // r_banks
    'r_boxes:uid:D',
  // r_countries
    'r_do:uid:D',
    'r_gifts:uid:D',
  // r_industries
    'r_invites:inviter',
    'r_invites:invitee:D',
    'r_invoices:payer:D',
    'r_invoices:payee:D',
    'r_log:myid:D',
    'r_log:agent:D',
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
    'r_questions:ctty',
  // r_regions
    'r_relations:main:D',
    'r_relations:other:D',
  // r_request
  // r_states
    'r_stats:ctty:D',
    'r_tous:uid:D',
  // r_transit
    'r_txs:payer',
    'r_txs:payee',
    'r_txs:payerAgent',
    'r_txs:payeeAgent',
    'r_usd:payer',
    'r_user_industries:uid:D',
  // r_votes
    'users:uid:D',
    'users:helper',
    'users:community',
  );
  foreach (ray(DRUPAL_UID_TABLES) as $one) $subs[] = "$one:uid";
  $oldFilename = DRUPAL_ROOT . $a->photoFilename(TRUE); // get old name while account exists
  $newFilename = str_replace("/$oldUid-", "/$newUid-", $oldFilename);
  $files[$oldFilename] = $newFilename;

  $DBTX = \db_transaction();
  foreach ($subs as $one) {
    list ($table, $field, $deletable) = explode(':', $one); // if deleting and not deletable, set to zero
    if ($delete and deletable) {
      db\q("DELETE FROM $table WHERE $field=:oldUid", compact('oldUid'));
    } else db\q("UPDATE $table SET $field=:newUid WHERE $field=:oldUid", compact('newUid', 'oldUid'));
  }
  $oldQid = r\qid($oldUid);
  if ($unusable and !$delete) {
    list ($uid, $name, $email) = array($oldUid, "unusable-$oldQid", "unusable$oldQid@" . CG_DOMAIN);
    db\insert('users', compact(ray('uid name email'))); // mark old uid unusable
  }
  foreach ($files as $old => $new) if (file_exists($old)) {
    if ($delete) unlink($old); else rename($old, $new);
  }
  unset($DBTX); // commit
  return $newUid;
}

/**
 * Package the return arguments and alert staff on certain errors.
 * @param mixed $message: the success or error message OR an array containing all three params
 * @param assoc $info: (optional) message substitutions and other info about transaction
 *   includes success=TRUE if the transaction was successful
 * @param bool $confirm: <to be confirmed> (default FALSE)
 */
function txRet($message, $info = [], $confirm = FALSE) {
  $info0 = $info;
  if (is_array($message)) {list ($message, $info, $confirm) = $message; $info += $info0;}
  if (!@$info['success']) r\tellCO('failed transaction', $info + compact('message', 'confirm'));
  return array($message, $info, $confirm);
}

/**
 * Check if an account needs a signup bonus.
 * @param int $uid: account id
 * @return: whether the signup bonus is deserved
 *//*
function checkBona($uid) {
  $acct = r\acct($uid);
  if ($acct->can(B_BONA)) return FALSE;
  if (!db\exists('r_txs', 'payee=:uid and r>0 and payer>0', compact('uid'))) return FALSE;
  if (!db\lookup('COUNT(DISTINCT payee)', 'r_txs', 'payer=:uid and goods', compact('uid')) >= R_BUYS_BEFORE_BONUS) return FALSE;
  return TRUE;
}*/

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
  $email = u\cryptN($email);
  $info = compact(ray('code invited email inviter subject message'));
  db\insert('r_invites', $info);
  return $code;
}

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out.
 * @param string $page: page to go to
 * @param string $msg: message to output, if any
 * @param string $field: error field, if any
 */
function go($page, $msg = '', $field = '') {
  global $base_url;

  if ($msg) r\Web\say($msg, $field);
//  else $page .= "/field=$field&msg=" . urlencode($msg); // say() fails if signed out

  if (u\test()) {
    global $lastGo; $lastGo = $page; // for testing
    $_POST = []; // make sure it doesn't act like we submitted the target page
    t\output("Going to page $page (page0=$lastGo msg=$msg field=$field)");
    return strpos($page, 'http://') === FALSE ? t\pageForm($page, $msg, $field) : '';
  }

  /*
  foreach (ray('login signup password reset') as $one) if (strpos($page, "user/$one") !== FALSE) {
    if (\drupal_session_started()) session_destroy();
  }
  */
  if (!u\starts($page, 'http')) $page = "$base_url/$page";
  \drupal_session_commit();
  header("Location: $page", TRUE, 302); 
  exit();
//  session_write_close();
//  exit();
//  \drupal_goto(substr($page, 0, 1) == '/' ? substr($page, 1) : $page);
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
      $messages[] = "<tr><td class=\"notice-date\" style=\"vertical-align:top; margin-right:5px;\">$date</td><td>$row->message</td></tr>";
    }
    if (@$messages) $messages = join("\n", $messages); else return FALSE;
  }
  
  $unit = $weekly ? 'week' : ($monthly ? 'month' : 'day');
  $yestertime = u\fmtDate($start = $unit == 'month' ? u\plusMonths(-1, $at) : strtotime("-1 $unit", $at));
  $range = $weekly ? "the week of $yestertime" : ($monthly ? strftime('%B %Y', $start) : $yestertime);
  extract(u\just('return', $a->j_stats()));
  $balance = u\fmtAmt($a->balance);
//  $savings = u\fmtAmt($a->savings);
  $savings = u\fmtAmt($a->rewards);
  list ($rewards, $ourEmail) = array(u\fmtAmt($a->rewards), $a->cttyA->email);
  $companies = \variable_get('companies');
  $companies = $companies[u\test() ? r\serverUid() : $a->community];
  return compact(ray('messages unit range yestertime balance savings return ourEmail companies'));
}

/**
 * Return a list of participating companies for the given community.
 * @param int $ctty: uid of community
 * @param bool $export: <format for use on promo site> (otherwise format for email notices)
 */
function companies($ctty, $export = FALSE) {
  $model = $export ? 
      '<div class="row"><div class="cmpy-name">NAME</div><div class="cmpy-does">DESC</div></div>'
    : '<tr><td width="200">NAME</td><td style="margin-left:10px;">DESC</td></tr>';
  $res = '';
  $q = db\q('SELECT uid FROM users WHERE :IS_CO AND :IS_OK AND uid>1 AND community=:ctty ORDER BY fullName', compact('ctty'));
  while ($row = $q->fetchAssoc()) {
    $a = r\acct($row['uid']);
    if (!$a->coCan(CO_PRIVATE)) $res .= strtr($model, ['NAME' => $a->fullName, 'DESC' => $a->shortDesc]) . "\n";
  }
  return $res;
}

/**
 * Return system-wide or community-wide statistics.
 * @param int $ctty: uid of community or region (0 means ALL)
 * @param int $when: latest desired statistics (defaults to now)
 *   if $when < 0, calculate the latest
 * @return assoc of named statistics (see r_stats table def)
 */
function stats($ctty = 0, $when = NULL) {
  u\setDft($when, r\rTime());
  if ($when < 0) $when = r\getStats($ctty);

  $sql = 'SELECT * FROM r_stats WHERE created<=:when AND ctty=:ctty ORDER BY id DESC LIMIT 1';
  if ($info = db\q($sql, compact('ctty', 'when'))->fetchAssoc()) return $info;

  $keys = db\q("SELECT column_name FROM information_schema.columns WHERE table_name='r_stats'")->fetchCol();
  return array_fill_keys($keys, 0);
}

/**
 * Return the amount the community has ever paid out
 */
function cttyPaidEver($ctty = 0) {
  $dt = db\max('created', 'r_stats', 'ctty=:ctty', compact('ctty')) ?: 0;
  return db\lookup(R_CTTY_BENE, 'r_stats', 'ctty=:ctty AND created=:dt', compact('ctty', 'dt'));
}
  
/**
 * Calculate and record statistics for the given community.
 * See the "// get" lines to see what statistics are collected and returned.
 * @param int $ctty: community or region (0 means ALL)
 * @param int $created: creation date for the statistics record (defaults to current time)
 * @return $created
 * @todo: change r field to bal and include replacements for USD
 */
function getStats($ctty, $created = NULL) {
  u\setDft($created, u\test() ? REQUEST_TIME : r\rTime());
  $subs = compact('created', $ctty ? 'ctty' : '');
  list ($members, $tmembers) = isGAME ? ['members', 't.members'] : [1, 1];
  list ($cttyCrit, $payerCrit, $payeeCrit, $cttysCrit) = $ctty ? 
    ['community=:ctty', 'payer=:ctty', 'payee=:ctty', '(u.community=:ctty OR u2.community=:ctty)']
  : ['1', 'payer<1', 'payee<1', '1'];
  list ($tradeOutSum, $tradeInSum) = $ctty == 0 ? [0, 0]
  : ["SUM($tmembers * IF(u.community<>:ctty AND u2.community=:ctty, amount, 0))",
     "SUM($tmembers * IF(u2.community<>:ctty AND u.community=:ctty, -amount, 0))"];
  
  $runny = 'IF(:HAS_BANK AND :IS_REFILL AND minimum>0, -minimum/2, IF(:IS_CO, 0, tin.rewards))'; // spendable, leading to a run on the bank
  $payins = '(SELECT payee, SUM(amount) AS received, SUM(IF(type IN (:TX_REWARDS), amount, 0)) AS rewards FROM r_txs WHERE created<=:created GROUP BY payee)';
  $payouts = '(SELECT payer, SUM(amount) AS paid FROM r_txs WHERE created<=:created GROUP BY payer)';
  $exchanges = '(SELECT payer, SUM(-amount) AS deposited FROM r_usd WHERE created<=:created GROUP BY payer)';
  
  // get pAccts, bAccts, newbs, savings, savingsIntent
  $sql = <<<EOF
    SELECT SUM($members * IF(activated AND activated<=:created AND NOT :IS_CO, 1, 0)) AS pAccts, 
      SUM(IF(activated AND activated<=:created AND :IS_CO, 1, 0)) AS bAccts,
      SUM($members * IF(created<=:created AND (NOT activated OR activated>:created), 1, 0)) AS newbs,
      SUM($runny) AS runny,
      SUM(IF($runny>0, 1, 0)) AS runnyCount,
      SUM(IF($runny>0, tin.received - tout.paid + tus.deposited, 0)) AS runnyBal,
      SUM(savingsAdd) AS savings
    FROM users u 
    LEFT JOIN $payins tin ON tin.payee=u.uid
    LEFT JOIN $payouts tout ON tout.payer=u.uid
    LEFT JOIN $exchanges tus ON tus.payer=u.uid
    WHERE u.uid>1 AND $cttyCrit AND created<=:created
EOF;
  $res = db\q($sql, $subs)->fetchAssoc() ?: [];
  $topN = max(5, round(.05 * $res['runnyCount']));
  unset($res['runnyCount']);

  /*  SUM(picture)/100 AS savings,
    SELECT SUM(picture)/100 AS top5
    FROM (SELECT picture FROM users WHERE uid>1 AND $cttyCrit AND created<=:created
      ORDER BY picture DESC LIMIT 5) u0
*/

  // get top5
  $sql = <<<EOF
    SELECT SUM(runny) AS top5
    FROM (SELECT $runny AS runny FROM users u LEFT JOIN $payins tin ON tin.payee=u.uid 
      WHERE u.uid>1 AND $cttyCrit AND created<=:created
      ORDER BY $runny DESC LIMIT $topN) u0
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  
  // get usd
  $sql = <<<EOF
    SELECT
    SUM($tmembers * IF(amount<0, -amount, 0)) AS bankIn,
    SUM($tmembers * IF(amount>0, amount, 0)) AS bankOut
    FROM r_usd t LEFT JOIN users u ON u.uid=t.payer
    WHERE $cttyCrit AND t.completed AND t.completed<=:created
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];
  
  // get buyers, sellers
  // (@todo?)


  // get purchases, rewards (of all types), xfees, grants, loans, investments, community trade, shared, forcash
  $sums = ''; // fine and giveback are negative rewards
  foreach (ray('signup rebate bonus giveback inflation helper fine xfee grant loan invest') as $one) {
    $type = u\consta('tx', $one);
    $sums .= "SUM($tmembers * IF(type=$type AND $payerCrit, amount, 0)) AS `{$one}s`, ";
  }

  $sql = <<<EOF
    SELECT $sums 
    SUM($tmembers * IF(type=:TX_TRANSFER AND ($payerCrit OR ($payeeCrit AND amount<0)), ABS(amount), 0)) AS cttyBuys,
    SUM($tmembers * IF(type=:TX_TRANSFER AND $payeeCrit AND amount>0, -amount, 0)) AS cttyIncomes,
    $tradeOutSum AS tradeOut, 
    $tradeInSum AS tradeIn,    
    SUM($tmembers * IF(channel=:TX_CRON AND payeeFor=':R_SHARING_MSG', amount, 0)) AS shared,
    SUM($tmembers * IF(goods=:FOR_GOODS, amount, 0)) AS purchases, 
    SUM($tmembers * IF(payer>1 AND goods=:FOR_USD, amount, 0)) AS forCashs
    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created<=:created
EOF;
  $res2 = db\q($sql, $subs)->fetchAssoc() ?: [];

  // get r
  $res2['r'] = array_sum($res2) - $res2['shared'] - $res2['purchases'] - $res2['forCashs'] - $res2['xfees'];
  $res2['r'] += $res['bankIn'] - $res['bankOut']; // include r provided in exchange for USD
  $res += $res2;

/*
  $cttyInOut = $ctty == 0 ? '' : <<<EOF
    SUM(IF(u.community=:ctty AND t.payee<>0, amount, 0)) AS usd2r,
    SUM(IF(u2.community=:ctty, amount, 0)) AS r2usd,
EOF;
*/

//  $usdNetIn = @$res['r2usd'] - @$res['usd2r'];
//  $res2['usd'] = $res['usd'] + $usdNetIn;
///  debug(compact('ctty','res2','res'));

  // get perMo, txsPerMo, p2b(Txs)PerMo, b2b(Txs)PerMo, b2p(Txs)PerMo, cash(Txs)PerMo
  $subs += ray('monthAgo', u\plusMonths(-1, $created));

  $sql = <<<EOF
    SELECT 
    SUM($tmembers * IF(goods=:FOR_GOODS, amount, 0)) AS perMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND NOT u.:IS_CO AND u2.:IS_CO, amount, 0)) AS p2bPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND u.:IS_CO AND u2.:IS_CO, amount, 0)) AS b2bPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND u.:IS_CO AND NOT u2.:IS_CO, amount, 0)) AS b2pPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS, 0, amount)) AS cashPerMo,
    $tradeOutSum AS tradeOutPerMo,
    $tradeInSum AS tradeInPerMo,

    SUM($tmembers * IF(goods=:FOR_GOODS, 1, 0)) AS txsPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND NOT u.:IS_CO AND u2.:IS_CO, 1, 0)) AS p2bTxsPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND u.:IS_CO AND u2.:IS_CO, 1, 0)) AS b2bTxsPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS AND u.:IS_CO AND NOT u2.:IS_CO, 1, 0)) AS b2pTxsPerMo,
    SUM($tmembers * IF(goods=:FOR_GOODS, 0, 1)) AS cashTxsPerMo

    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
    WHERE $cttysCrit AND t.created>:monthAgo AND t.created<=:created
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  // get bankIn(Txs)PerMo, bankOut(Txs)PerMo
  $sql = <<<EOF
    SELECT SUM($tmembers * IF(amount<0, -amount, 0)) AS bankInPerMo,
    SUM($tmembers * IF(amount<0, 0, amount)) AS bankOutPerMo,
    
    SUM($tmembers * IF(amount<0, 1, 0)) AS bankInTxsPerMo,
    SUM($tmembers * IF(amount<0, 0, 1)) AS bankOutTxsPerMo

    FROM r_usd t LEFT JOIN users u ON t.payer=u.uid
    WHERE $cttyCrit AND t.completed AND completed<=:created AND completed>:monthAgo
EOF;
  $res += db\q($sql, $subs)->fetchAssoc() ?: [];

  foreach ($res as $k => $v) {if (is_null($v)) $res[$k] = 0;}
  db\insert('r_stats', $res + compact('created', 'ctty'));
  return $created;
}

/**
 * Return the average balance, average credit reserve, and minimum balance for the given account (possibly joint) and period.
 * @param int $uid: account id (can be a community or region too)
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
  // use "1" as payer for r_usd, to avoid matching jid=0

  $sql = <<<EOF
    SELECT SUM($signedAmt) AS total,
    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rtotal,
    SUM(IF(t.goods=:FOR_SHARE, $signedAmt, 0)) AS share,
    FLOOR((t.completed-$start)/:DAY_SECS) AS day 
    FROM (
      (SELECT type,goods,payer,payee,amount,created AS completed FROM r_txs)
      UNION (SELECT :TX_BANK,-:FOR_SHARE,payer,1 AS payee,amount,completed FROM r_usd)
    ) t WHERE ($isPayer XOR $isPayee) AND completed BETWEEN $start AND $end 
    GROUP BY day
EOF;

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
  } else return u\test() ? REQUEST_TIME : time();
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
      $why[] = w\popHelp($bitName, $riskHelp[$bitName], "$base_url/help/flags@$bitName", $class);
//      $why[] = "<a href=\"$base_url/help/flags@$bitName\" class=\"$class riskHelp$helpIndex\">$bitName</a>";
    }
  }
  return @$why ? join(', ', $why) : '';
}

/**
 * Check account permissions for a transaction or invoice
 * @param acct $a1: the account paying or requesting payment
 * @param acct $a2: the other account
 * @param bool $taking: the first account is taking money from the second (charging or invoicing)
 * @param bool $neg: the amount is negative (refunding or undoing)
 * @return [message, subs, confirmed] on error, else false
 */
function txPermErr($a1, $a2, $taking = TRUE, $neg = FALSE) {
  global $scanned;
  
  if (!$a2->ok) return array('non member', ['otherName' => $a2->fullName], FALSE);
  u\EXPECT($a1->ok, 'main account inactive tx');
  foreach ([$a1, $a2] as $a) {
    $other = $a == $a1 ? $a2 : $a1; // if neither confirmed nor confirming icard by tx with inviter
//    if (!$a->confirmed and !($a->iCode == IBY_ICARD and $a->helper == $other->id)) { 
    if (!$a->confirmed and $a->helper != $other->id) { 
      return ['not confirmed', ray('youName inviterName', $a->fullName, $a->helperA->fullName), FALSE];
    }
  }
  list ($myAccess, $otherAccess) = u\order($taking xor $neg, B_SELL, B_BUY);
  //list ($myAccessErr, $otherAccessErr) = u\order($taking, 'sell', 'buy');
  list ($myWhat, $otherWhat) = u\order($taking xor $neg, t('sales'), t('purchases'));
  if ($scanned and $taking) list ($myAccess, $myWhat) = $neg ? array(B_REFUND, t('refunds')) : array(B_SCAN, t('sales'));
///  debug(compact(ray('taking neg scanned myAccess otherAccess myWhat otherWhat')));
  if (!$a1->can($myAccess)) return array("no perm", ray('what', $myWhat), FALSE);
  if (!$a2->can($otherAccess)) return array("other no perm", ray('otherName what', $a2->agentA->fullName, $otherWhat), FALSE);
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
  return db\lookup('AVG($field)', 'users', "$where ORDER BY $field LIMIT $limit", $subs);
}

/**
 * Return the next sequential bank transfer number for an account.
 * @param int $payer: the account uid
 * @return int: the next unused positive integer (1 if there are no bank transfers yet)
 */
function getUsdTid($payer) {
  return db\lookup('MAX(tid)+1', 'r_usd', 'payer=:payer', compact('payer')) ?: 1;
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
  if (r\signedIn()) $page .= '?region=' . (isSTAGE ? STAGE_SUBDOMAIN : R_SERVER_ID) . '&tm=' . r\rTime();
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
 * Return the rebate on the given amount for the given account.
 * @todo? or manager of company with no (other) employees (set bit?)
 * NEW: Figure the rebate based on R = the payer's total rewards ever and A = the transaction amount,
 *   such that the rebate percentage p tapers linearly from Rmax to 0
 *   The percentage rebate is based on the total rewards AFTER the transaction. That is:
 *   The new percentage rate p = (R + pA)(Pmax-Pmin)/Rmax + Pmin
 *   So p = (RPmax + (Rmax - R)Pmin) / (Rmax^2 - A(Pmax - Pmin))
 * BETTER: just decrease the reward percentage by 1% for every $500 of rewards received.
 */
function rebate($from, $to, $amount = 1) {
  global $testRewardStep;
  if (relation(1, $to, $from, 'isOwner OR draw')) return 0;
  if (relation(1, $from, $to, 'isOwner OR draw')) return 0;
  $a = r\acct($from);
//  $pct = u\test() ? $a->rebate : max(R_MIN_REWARD_PCT, R_REBATE - floor($a->rewards / R_REWARD_STEP));
  $step = @$testRewardStep ?: R_REWARD_STEP;
  $pct = max(R_MIN_REWARD_PCT, $a->rebate * (1 - floor($a->rewards / $step) / R_REBATE));
  return '' . round(min(R_MAX_REBATE, $pct * $amount / 100), 2); // ''. to avoid serializing a long float
//  return '' . round(min(R_MAX_REBATE, $amount * $a->rebate / 100), 2); // ''. to avoid serializing a long float
}

function bonus($to, $from, $amount = 1) {return rebate($to, $from, $amount * R_BONUS_FACTOR);}
//function bonus($to, $from, $amount = 1) {return round($amount * (u\test() ? 2 : R_BONUS_FACTOR) * r\acct($to)->rebate / 100, 2);}

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
function realCountry($country) {return db\lookup('name', 'r_countries', 'id=:country', compact('country'));}
function realState($state, $country = R_COUNTRY_ID) {return db\lookup($country == US_COUNTRY_ID ? 'abbreviation' : 'name', 'r_states', 'id=:state and country_id=:country', compact('state', 'country'));}
function makeQueue($callback, $time = 15) {return array('worker callback' => "rCredits\\Cron\\$callback", 'time' => $time);}
function perm($bit) {return $bit - B_RELATED;}
function foreignServer($uid) {return (r\uidRegion($uid) != serverUid());}
function doCode($code) {return @unserialize(@u\decrypt(@u\deurlify($code)));}  
function makeDoCode($a) {return u\urlify(u\crypt(serialize($a)));}
function cgfId() {return \variable_get('cgf_uid');}
function disputeTx($xid) {return db\setFlag('r_txs', $xid, B_DISPUTED, TRUE);}
function denyInv($nvid) {return db\setData('r_invoices', $nvid, ['denied' => TRUE]);}
function stepsDone0() {return array_fill_keys(array_keys(ray(R_STEPS)), FALSE);}
function isReward($type) {return in_array($type, ray(TX_REWARDS));}
//function usdin($for) {return u\abbreviates('usd in (', strtolower(is_array($for) ? $for[1] : $for));}
function usdin($for) {return u\abbreviates(S_USD_IN, is_array($for) ? $for[1] : $for, TRUE);}
function regionField($fld) {return @r\acct(serverUid())->$fld;}
function signedIn() {return r\acct() ? TRUE : FALSE;}
function iCodeDesc($iCode) {$iCodeDescs = ray(IBYS); return $iCodeDescs[min($iCode, IBY_ICARD) - IBY0];}
