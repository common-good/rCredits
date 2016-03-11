<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Db as db;
use rCredits\Txs as x;
use rCredits\Admin as a;
use rCredits\Risk as k;
use rCredits\Web as w;
use rCredits\Vote as v;

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

require_once __DIR__ . '/rweb-subs.inc';
require_once __DIR__ . '/../rcredits-backend.inc';
include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent
// FAILS (WHY?) include_once __DIR__ . (u\test() ? '/../rcredits-testing.inc' : '/../testing-stub.inc'); // else fail in showForm()
include_once __DIR__ . '/../rcredits-testing.inc'; // else fail in showForm()

define('UPLOAD_DIR', dirname($_SERVER['DOCUMENT_ROOT']) . '/public_ftp/incoming');

/**
 * Handle page not found error (404).
 */
function formPageNotFound($form, &$sta, $arg = '') {
  $page = $_SERVER['REQUEST_URI'];
  $page = isDEV ? basename($page) : substr($page, 1); // ignore leading slash
  header_remove(); // prevent 404 circle (doesn't help)
  
  if ($page[0] == '-') return formSignup('', $sta, 'code=' . substr($page, 1)); // invitation!
  return formEmpty(t('The page you requested does not exist.'));
}

/**
 * Handle an arbitrary AJAX request.
 * GET parameters:
 * @param string op: what to do
 * @param string sid: session id
 * @todo: maybe all zero uid for some ops (by looking up two fields in sessions?)
 */
function ajax() {
  extract(u\just('op sid data', $_POST ?: $_GET));
  header('Content-Type: application/json');

  if (!@$sid or !$res = db\lookup('uid,session', 'sessions', 'sid=:sid', compact('sid'))) ajaxErr('bad sid');
  $aid = $res['uid'];
  list ($sess) = @explode(';', strstr($res['session'], 'rcredits_myid|')) ?: ['|'];
  list ($var, $myid) = @explode('|', $sess) ?: ['', ''];
///  flog(compact(u\ray('op sid data res')));
  if ($var != 'rcredits_myid' or !$myid = @unserialize($myid)) ajaxErr('not signed in');
//  $mya = r\acct($uid);
  
  if ($op == 'typeWho') {
    exit(json_encode(be\memberRay($myid)));
  } elseif ($op == 'changeCtty') {
    extract(u\just('uid newCtty retro', $data));
    $a = r\acct($uid);
    if (!$a->changeCtty($newCtty, $retro, $err)) ajaxErr(t('Community change failed.|') . $err);
//    ajaxRet(['msg' => "uid is $uid, newCtty is $newCtty"]);
  } elseif ($op == 'who') {
    extract(u\just('who question amount', $data));
    $amtDpy = u\fmtAmt($amount);
    if (!is_array($whoA = be\identify($who, $myid, 'no self-trading'))) {
      ajaxRet(u\ray('who confirm isNonprofit', $whoA->mainQid, tt($question, u\ray('name amount', $whoA->fullName, $amtDpy)), $whoA->isCtty or $whoA->coCan(CO_NONPROFIT)));
    }
    list ($msg, $args, $choices) = $whoA;
    $message = u\tt($msg, $args);
    if (empty($choices)) ajaxErr($message);
    foreach ($choices as $uid => $fullName) {
      $choices[$qid = r\quid($uid)] = "$qid $fullName";
      unset($choices[$uid]);
    }

    // create modal selection form
    $choice = selectFld('', '', attrib(['size' => 12]), $choices);
    $which = render($choice);
    $title = tt($question, u\ray('name amount', t('whom'), $amtDpy));
    ajaxRet(compact('which', 'title', 'message'));
  } else ajaxErr(t('Invalid op'));
  ajaxMsg(t('Done!'));
}

function ajaxErr($msg) {ajaxMsg($msg, 0);}
function ajaxMsg($msg, $ok = 1) {ajaxRet(['message' => $msg], $ok);}
function ajaxRet($data, $ok = 1) {exit(json_encode($data + compact('ok')));}

/**
 * Complete a "no signin required" request (made by the user clicking a link we sent).
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
 * Alternatively, code can be an error message (recognized by having a space in it).
 * @param string $args: urlencoded array of arguments passed from an email (from whence $_POST is not reliably received by Drupal)
 * @see also r\makeDo()
 */
function rdo($code, $args = '') {
  global $channel;
  if (strpos($code, ' ')) return doSay($code);
  if (strpos($code, '~') < 10) { // mailChimp-type action
    @list ($op, $qid, $ecode, $mo, $v) = explode('~', $code . '~~~~~~');
    if (!@$op or !@$qid or !@$a = r\acct($qid) or @$ecode != $a->emailCode) return doSay(t('That is not a valid link.'));
  } elseif (!$info = r\doCode($code)) return doSay(t('That link is archaic.'));
///  print_r($info);
  if ($once = is_numeric($doid = @$info['op'])) {
    if (!$row = db\lookup('uid,data,expires', 'r_do', 'doid=:doid', compact('doid'))) return doSay(t('That link is no longer valid.')); // already used OR deleted because it expired
    extract($row);
    $info = unserialize($data) + $row; // get new op, etc.
    $a = r\acct($uid);
    extract(u\just('op v mo expires sql subs', $info));
  }

  if (@$mo) $expires = (r\rdoMonth() > $mo + 3) ? 1 : FALSE; // finagle expiration if link is older than 3 months
  if (@$expires and r\rTime() >= $expires) return doSay('link expired');

  $channel = TX_LINK;
  $DBTX = \db_transaction();
  
  if ($op == 'min') {
    if ($jid = $a->jid and $a->slave) $a = r\acct($jid); // for joint accounts, update on the master
    $res = $a->update('minimum', $v);
  }
  if ($op == 'nosecret') $res = $a->setBit(B_SECRET, FALSE);
  if ($op == 'debtok') $res = $a->setBit(B_DEBT, TRUE);
  if ($op == 'rsv') {return r\go("savings/qid=$qid&ecode=$ecode");}
  if ($op == 'sql') $res = db\q($sql, $subs);

  if ($op == 'addr') {
    $a = r\acct($v);
    $phone = $a->co ? t('Phone: ') . u\fmtPhone($a->phone) . "<br>" : '';
    $addr = t('Physical address: ') . r\location($a, TRUE);
    return doSay("$a->fullName<br>$a->postalAddr<br>$addr<br>$phone", 'status');
  } 
  
  if ($op == 'inv') {
    if ($err = r\badUnpaidInv($v)) return doSay($err);
    return r\go("handle-invoice/nvid=$v&toMe=1&code=$code");
  } 
  
  if ($op == 'confirmInvite') {
    $a = r\acct($v);
    $a->setBit(B_CONFIRMED);
    return doSay("Your invitation to $a->fullName is confirmed!", 'status');
  }
  
  if ($op == 'vote' or $op == 'yesno') { // NOTE: about a third of the time, $_POST comes in empty (even from the same computer)
    $issue = $v;
    if ($issue != '20141220') return doSay('link expired');
//    $vote = 0;
//    foreach (u\ray('wl sl wd ed no') as $k => $vv) $vote |= u\setBit($vote, $k, isset($_POST[$vv]));
    extract(u\just('yes no', $args));
    $vote = @$yes ? 1 : 0;
    $info = compact(u\ray('uid issue vote'));
    $vid = db\lookup('vid', 'r_votes', 'uid=:uid AND issue=:issue', compact('uid', 'issue'));
//    $vid = 0;
///  debug(compact(u\ray('op args issue yes no vote info vid')));
    if ($vid) db\update('r_votes', $info + compact('vid'), 'vid'); else db\insert('r_votes', $info);
    $say = t('Thank you for your RSVP! ');
    if (@$no) $say .= t('We will miss you. ');
    r\tellStaff('RSVP!', ['coming'=>@$yes ? 'yes' : 'no'], $uid);
    $res = TRUE;
  }
  
  if ($op == 'join') {
    $otherId = $v;
    $joint = r\perm(B_JOINT);
    if  (r\relation(1, $otherId, $a->id, 'permission=:joint', compact('joint'))) { // inviter still wants it?
      $a->unjoin(); // zap any other joins or join requests
      if ($reid = r\relation('reid', $a->id, $otherId)) {
        $info = ['permission'=>$joint, 'draw'=>0] + compact('reid');
      } else $info = u\ray('main other permission draw', $a->id, $otherId, $joint, 0);
      // (already in a transaction) $DBTX = \db_transaction();
      db\update('r_relations', $info, 'reid', TRUE); // create relation or just set permission
      $a->join(r\acct($otherId)); // remember who is joined
      // unset($DBTX); // commit
      $res = TRUE;
    }
  }

  if (!@$res) return doSay(t('Request failed.'));
  if ($once) db\q('DELETE FROM r_do WHERE doid=:doid', compact('doid'));

  unset($DBTX);

  return doSay(@$say . t('Your request was successfully completed.'), 'status');
}

/**
 * Generate choices for a member autocomplete field
 * @param string $string: what the user has typed so far
 * @param string $role: 'payer', 'payee', 'industry', 'company', or '' (other), depending on the role of the user
 * @param int $myid: the current account (globals and session variables are inappropriate here)
 * @todo: ditch this altogether or make it efficient with separate rCredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the autocomplete URL to the backend? privacy is not crucial here
 * @patched: Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($role = '', $myid = 0, $string = '') {
//  return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string"));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\shortName($string, '%') . '%'; // this also prevents malicious database injection
//  $string = str_replace(' ', '%', \db_like(" $string "));
  if (!is_numeric($myid)) return;
  $params = 'string'; // list of substitution parameters for query (each section below can add additional params)
  
  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
/*  attempt to show recently used members first (add myid to dbQ query subs)
   AND (t.$role IS NULL OR t.$role=:myid) ORDER BY t.created DESC, u.fullName
    $selection = <<<EOF
      u.uid, u.fullName AS display, u.email 
      FROM users u LEFT JOIN r_txs t ON t.$other=u.uid 
      WHERE (u.name LIKE :string OR u.email LIKE :string)
      AND (t.$role IS NULL OR :myid IN (t.payer, t.payee))
      ORDER BY t.created DESC, u.fullName
*/
    $selection = <<<EOF
      u.uid, u.fullName AS display 
      FROM users u WHERE (u.uid>1 AND u.name LIKE :string)
EOF;
//      FROM users u WHERE (u.uid>1 AND (u.name LIKE :string OR u.email LIKE :string))
  } elseif ($role == 'industry') { // the one place we don't need to include the qid (for definiteness)
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry COLLATE utf8_general_ci AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName COLLATE utf8_general_ci AS display FROM users WHERE :IS_CO AND name LIKE :string
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE :IS_CO AND name LIKE :string';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $mya = r\acct($myid);
    $cttyCrit = ($cttyId = $mya->community) ? "AND community=$cttyId" : '';
    $personal = $role == 'ctty' ? '' : ' AND NOT :IS_CO';
    $selection = "uid, fullName AS display FROM users WHERE name LIKE :string $cttyCrit $personal";
  } else $selection = 'uid, fullName AS display FROM users WHERE name LIKE :string';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string sql=$selection"));
///return drupal_json_output(array('zot' => "subs=" . print_r(compact(u\ray('string myid')), 1)));

  $result = db\q($sql, compact(u\ray($params)));
  $matches = [];
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = r\quid($uid) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
//    return drupal_json_output(array('zot' => "display=$display"));
  }
  \drupal_json_output($matches);
}

/**
 * Accept a grant application.
 */
function formGrant($form, &$sta) {
  $mya = r\acct();
  $title = item(t('rCredits Organizer - Grant Application'));
  $subtext = item(t('<p>Are you passionate about democratic economics and the potential of rCredits to create a society to benefit everyone? Do you have the skills to organize a group to begin the process of becoming a Common Good Community? Great! Common Good Finance has a grant for you.</p><p>Open an rCredits account, get one retail business and 10 other people to sign up (you get a @iRewardr reward for each one), then fill out this form to request a grant of up to $5,000. Please also email us your resume, CV, or equivalent.</p><p>Remember the board may not be familiar with your work or your local community, so explain as needed.</p>', ['@iReward' => R_HELPER_BONUS]));
  if (!$mya) say(t('You must sign in, to apply for a grant.'));
  $did10 = boolFld(t('Got 10/1:'), t('Have you successfully recruited 10 rCredits members and at least 1 rCredits business?'));
  $biz = textFld(REQ . t('Biz:'), [t('Business name(s)'), t('Which business(es) did you recruit?')]);
  $amount = textFld(REQ . t('Request:'), [t('Requested grant amount'), t('How much grant funding are you requesting (up to $5,000)?')]);
  
  $class = ['class' => ['inline']];
  foreach ($gets = u\ray('getOrganizers getIndividuals getCompanies') as $k) {
    $$k = w\rendA($k, textFld(BARE, [t('How many')], $class));
  }

  $getPartners = w\rendA('getPartners', textFld(BARE, [t('Organization name(s)')], $class));
  $getOther = w\rendA('getOther', textFld(BARE, [t('Describe')], $class));
  $start = w\rendA('start', textFld(BARE, [t('Start date')], $class));
  $end = w\rendA('end', textFld(BARE, [t('End date')], $class));

  $plan = item(tt(<<<EOF
<p>What do you plan to accomplish with this funding (and over what time period)?<br>
<ul>
<li>Inspire @getOrganizers local organizers to meet regularly, to form a Common Good Community.</li>
<li>Recruit @getIndividuals more individuals.</li>
<li>Recruit @getCompanies more companies.</li>
<li>Develop partnerships with these organizations: @getPartners.</li>
<li>Other: @getOther</li>
<li>Grant period starts @start, ends @end.</li>
</ul>
EOF
  , compact(array_merge(u\ray('getPartners getOther start end'), $gets))), t('Plan:')); // + fails here (PHP bug)

//  foreach ($accountables = u\ray('meet report sayOther') as $k) $$k = w\rendA([$k => boxFld($k, BARE)]);
  foreach ($accountables = u\ray('meet report sayOther') as $k) $$k = "<input type=\"checkbox\" id=\"edit-$k\" name=\"$k\" class=\"inline\" />";
  $sayWhat = w\rendA('sayWhat', textFld(BARE, [t('Describe')], $class));
  
  $accountable = item(tt(<<<EOF
<p>How will you be accountable to Common Good Finance and to other rCredits organizers?<br>
<ul>
<li>@meet Attend most weekly organizer conference calls? (We might ask you to participate in these meetings <i>before</i> we consider your grant request.)</li>
<li>@report Report progress monthly by email?</li>
<li>@sayOther Other (work this out with CGF staff): @sayWhat</li>
</ul>
EOF
  , compact($accountables) + compact('sayWhat')), t('Reporting:'));
  
  $refs = textFld(REQ . t('References:'), [t('Work/Organizing references'), t('Please give at least two work or community organizing references (with contact information). We will also contact the business(es) you have recruited.')]);
  $submit = submit(t('Submit Grant Request'));
  
  return labeled(compact(u\ray('title subtext did10 biz amount plan accountable refs submit')));

// CGF: ask for resume, references, and what do you think it will take to get 50 people and 5 retail businesses and some of their employees and suppliers signed up? what's your strategy?
}

function formGrant_submit($form, &$sta) {
  $mya = r\acct();
  $helperCount = db\Count('users', ':IS_OK AND NOT :IS_CO AND helper=:myid', ['myid' => $mya->id]);
  foreach (u\ray('did10 meet report sayOther') as $k) $sta['input'][$k] = $sta['input'][$k] ? t('YES') : t('NO');
  extract($params = $sta['input'], EXTR_PREFIX_ALL, 'i');

  $th = 'th style="width:50%; text-align:left; padding:5px;"';
  $td = 'td style="padding:5px;"';
  $msg = <<<EOF

<h4>GRANT APPLICATION</h4>

<table id="app" border="1" cellspacing="3" cellpadding="0">
<tr><$th>Name:</th><$td>$mya->fullName</td></tr>
<tr><$th>Amount requested:</th><$td>$i_amount</td></tr>
<tr><$th>At least 10 individuals recruited so far:</th><$td>$i_did10 (actual count: $helperCount)</td></tr>
<tr><$th>Businesses recruited so far:</th><$td>$i_biz</td></tr>
<tr><$th>Grant period:</th><$td>$i_start to $i_end</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$td colspan="2">What do you plan to accomplish with this funding?</td></tr>
<tr><$th>Inspire local organizers to meet regularly, to form a Common Good Community:</th><$td>$i_getOrganizers</td></tr>
<tr><$th>Recruit more individuals:</th><$td>$i_getIndividuals</td></tr>
<tr><$th>Recruit more companies:</th><$td>$i_getCompanies</td></tr>
<tr><$th>Develop partnerships with these organizations:</th><$td>$i_getPartners</td></tr>
<tr><$th>Other:</th><$td>$i_getOther</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$td colspan="2">How will you be accountable to Common Good Finance and to other rCredits organizers?
<tr><$th>Attend most weekly organizer conference calls?</th><$td>$i_meet</td></tr>
<tr><$th>Report progress monthly by email?</th><$td>$i_report</td></tr>
<tr><$th>Other:</th><$td>$i_sayWhat</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$th>References:</th> <$td>$i_refs</td></tr>
</table>
EOF;

  r\tellStaff('organizer grant request', $params + compact('msg'));
  return r\go('', t('Your grant request has been submitted! You will be hearing from Common Good Finance within a week or two.'));
}

/**
 * Choose how often to send each type of notification.
 */
function formNotifications($form, &$sta) {
  global $notifyWhens, $notifyNowBys;
  
  $mya = r\acct();
  $title = item(t('Notifications'));
  $subtext = item(t('How often do you want to hear from us about these various things?'));
  
  $events = [
    t('explicit payment/transfer from your account'),
    t('automatic payment from your account'),
    t('automatic transfer into your account initiated'),
    t('transfer into your account completed'),
    t('payment received'),
    t('inflation adjustment received'),
    t('other incentive reward received'),
    t('fine, grant, loan, or investment received'),
    t('transaction/invoice edited, canceled, or disputed'),
//    t('unpaid invoice notice'),
    t('your invitee needs a nudge'),
    t('suggestion from the system'),
  ];

  $whens = u\ray($notifyWhens);
  $alwaysNow = [
    t('invoice received'),
    t('password reset request'),
    t('request to join with another account'),
    t('invitation/trust confirmation request'),
    t('remember to finish setting up your account'),
    t('account approval'),
    t('unusual activity'),
    t('you connected a bank account'),
  ];
  
  $textColW = 12 - count($whens); // width of event text
  $defaults = $mya->notices ?: N_DEFAULTS;
  
  list ($whenI, $cols) = [0, ''];
  foreach ($whens as $k => $v) {
    $cols .= "<div class=\"col-xs-1\"><div class=\"vertical\">$v</div></div>";
    $whenI++;
  }  
  $table = <<<EOF
<div class="row header"><div class="col-xs-$textColW"></div>$cols</div>
EOF;
  
  for ($eventI = 0; $eventI < count($events); $eventI++) {
    $event = $events[$eventI];
    list ($whenI, $cols) = [0, ''];
    foreach ($whens as $k => $v) {
      $checked = $defaults[$eventI] == $k ? 'checked="checked"' : '';
      $input = <<<EOF
<input type="radio" id="edit-e$eventI-$k" name="e$eventI" value="$whenI" class="form-radio" $checked />
EOF;
      $cols .= "<div class=\"col-xs-1\">$input</div>";
      $whenI++;
    }
    $parity = $eventI % 2 ? 'even' : 'odd';
    $table .= <<<EOF
<div class="row $parity"><div class="col-xs-$textColW">$event</div>$cols</div>
EOF;
  }
  $table = item($table);
  
/*
    t('message from another member'),
    t('rCredits community newsletter'),
*/
  $methods = u\ray($notifyNowBys);
//  $method = radiosFld(t('Immediate by:'), t('Receive immediate messages by text, email, or both?'), 1, $methods);
  return labeled(compact(u\ray('title subtext table')));
}

/**
 * Print a sheet of invitation cards for the member or members.
 * @param array $accts: the accounts for which to print invitation cards (defaults to current account)
 */
function printInvite($accts = '') {
  require_once __DIR__ . '/../pdf.class';
  $pdf = new r\Pdf();
//  $pdf->addTTFfont(__DIR__ . '/../../tcpdf/fonts/verdana.ttf'); // do just once on DEV, then upload font
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  if (is_array($accts)) {
    foreach ($accts as $a) printInvite1($a, $pdf);
  } else printInvite1($a = $accts ?: r\acct(), $pdf); // default to current account
  if ($accts) w\say($pdf->pageNo() . t(' invitation cards have been downloaded to the rCredits temp directory.'));
  $pdf->output(DRUPAL_ROOT . R_PATH . '/temp/rCredits-Invitation-Cards.pdf', $accts ? 'F' : 'I'); //Close and output PDF document
}

/**
 * Print a sheet of invitation cards for one member company.
 * @param acct $a: the account for which to print invitation cards
 * @param object $pdf: the pdf object in process
 */
function printInvite1($a, &$pdf) {
  require_once __DIR__ . '/../pdf.class';

  $co = $a->co;
  $name = $a->shortName ?: $a->fullName;
  $iCode = $a->iCardCode(IBY_EMAIL);
  $iCode = substr($iCode, 0, 4) . ' &nbsp;' . substr($iCode, 4, 4) . ' &nbsp;' . substr($iCode, 8);
  
  list ($mT, $mR, $mB, $mL) = $co ? [0, 0, 0, 0] : [.5, .75, .5, .75]; // margins
  list ($y0, $dy, $wInvite, $wCode, $codeSize) = $co ? [.2, .18, 2.5, 1.9, 10] : [.125, .175, 2, 1.5, 9];
  list ($dxInvite, $dyLine, $dxLine, $dyGoodFor) = $co ? [.25, .84, .25, .95] : [.08, .69, .125, .74];
  if (strlen($name) < 30) $wInvite = 2; // center near the left if possible
  list ($dxPic, $dyPic, $hPic, $dySite, $siteSize) = $co ? [.5, 1.2, 1.1, 2.35, 12] : [.62, .95, .8, 1.75, 10];
  list ($wCard, $hCard) = $co ? [4.25, 2.75] : [3.5, 2]; // card dimensions
  list ($goodFor) = $co ? [t('This card good for')] : [t('Get')];
  list ($wPage, $hPage) = [8.5, 11]; // page dimensions
//  $gLen = .1; // cutting guide length from card corner (for pluses in each corner of each card)
  $gStyle = ['color' => [192,192,192]];

  $pdf->AddPage();
  
  for ($y = $mT; $y < $hPage - $mB; $y += $hCard) for ($x = $mL; $x < $wPage - $mR; $x += $wCard) { // each card
/*  // cutting guides
    foreach ([0, $hCard] as $dy) foreach ([0, $wCard] as $dx) { // for each corner
      $pdf->Line($x+$dx-$gLen, $y+$dy, $x+$dx+$gLen, $y+$dy, $gStyle); // horizontal
      $pdf->Line($x+$dx, $y+$dy-$gLen, $x+$dx, $y+$dy+$gLen, $gStyle); // vertical
    } */
    
    $pdf->say($name, $x+$dxInvite, $y+$y0, $wInvite, '', '10', 'C');
    $pdf->say(t('invites you to') . ' <span style="font-weight:bold; color:darkblue">rCredits<sup>&reg;</sup></span>', $x+$dxInvite, $y+$y0+$dy, $wInvite, '', '10', 'C');
    $pdf->say(t('the currency of a better future'), $x+$dxInvite, $y+$y0+2*$dy, $wInvite, '', '9;I;color:darkgreen;', 'C');
    $pdf->Line($x+$dxLine, $y+$dyLine, $x+$wCard-$dxLine, $y+$dyLine, $gStyle); // x1, y1, x2, y2, style
    $pdf->say(" $goodFor" . ' <span style="font-size:90%;"><sup>$</sup></span><span style="color:black;">' . R_SIGNUP_BONUS . '</span><span style="color:darkgreen;">r</span> ' . t('and 10% rewards at all member businesses.'), $x, $y+$dyGoodFor, $wCard, '', '9;color:darkgray', 'C');
//    $pdf->say('<sup style="color:silver; font-size:50%; font-weight:normal;">$</sup>' . R_HELPER_BONUS . ' &nbsp;', $x, $y+.78, 2, '', '44;B;color:brown', 'C');
    $pdf->Image(__DIR__ . '/../images/r=heart+$.png', $x+$dxPic, $y+$dyPic, 2.83*$hPic, $hPic, '', '', '', true); // file, x, y, w, h, type, link, align, resize
    $pdf->say('Activate your membership at <span style="font-weight:bold; color:darkgreen;">rCredits.org</span>. It\'s free!', $x, $y+$dySite, $wCard, '', $siteSize, 'C');

    $pdf->say('Invitation #', $x+$wCard-$wCode, $y+$y0+$dy-.02, $wCode, '', $codeSize, 'C'); // text, x, y, w, h, format, align, borders
    $pdf->say($iCode, $x+$wCard-$wCode, $y+$y0+2*$dy-.02, $wCode, '', $codeSize . ';color:darkred;Verdana', 'C'); // text, x, y, w, h, fmt, align, borders
  }
}

/**
 * Sections with submenus.
 */
function formCommunity($form, &$sta) {return w\subMenu(t('Community'), 'community');}
function formHistory($form, &$sta) {return w\subMenu(t('Account History'), 'history');}
function formSettings($form, &$sta) {return w\subMenu(t('Settings'), 'settings');}

/**
 * List the months for which notices are available for the current account, with links.
 */
function formNotices($form, $sta, $month = '') {
  return $month ? notice($month) : monthChoices(t('Notices'), 'notices', '', 0, '');
}

/**
 * List notices for a given month.
 * @param string $month: mmmyyyy
 */
function notice($month = '') {
  if (!$start = strtotime("1$month")) return r\go('empty', t('That is not a recognizable month.' . $month), 'ERR');
  $start = u\monthDay1($start);
  $end = strtotime('+1 month', $start) - 1;

  $mya = r\acct();
  if ($noticeArgs = r\noticeArgs($mya->id, "created BETWEEN $start AND $end", FALSE, TRUE)) {
    extract(u\just('messages', $noticeArgs));
  } else $messages = '';
  $title = item(t('Notices for ') . strftime('%B %Y', $start));
  $list = item("<table>$messages</table>");
  return compact('title', 'list');
}

/**
 * Generate an account statement for the given month.
 * @param string $month: mmmyyyy
 */
function statement($month) {
  include_once __DIR__ . '/../rcredits-txs.inc';
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$start = strtotime("1$month")) return r\go('empty', t('That is not a recognizable month.'), 'ERR');
  $start = u\monthDay1($start);
  $end = u\plusMonths(1, $start) - 1;
  if (!$mya = r\acct()) return r\go('', t('You must first sign in.'), 'ERR');

  include_once __DIR__ . '/../statement.class';
  
  $pdf = new r\Statement($mya, $start);

  $size = $pdf->getFontSizePt();
  $m = 5; // left/right margin size
  $pdf->SetMargins($m, PDF_MARGIN_TOP, $m);
  $pdf->SetAutoPageBreak(TRUE, 15); // leave space at bottom
  $pdf->setLanguageArray($l); // (eg "page" in footer)
  $pdf->AddPage();
  $pageWidth = $pdf->getPageWidth();

/*  $pdf->SetLineStyle(array('width' => 0.5, 'cap' => 'butt', 'join' => 'miter', 'dash' => 0, 'color' => array(0, 0, 0)));
  $pdf->RoundedRect(91, 30, 60, 60, 3.50, '1111', 'DF', NULL, array(255, 255, 255)); // x, y, w, h, ?, fill?
  */
  //$pdf->Cell(0, 0, 'Page ', 0, false, 'L', 0, '', 0, false, 'T', 'M');
  //$pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">rCredits.org</div>');
  list ($address, $city, $state, $zip) = u\parseAddr($mya->postalAddr);
  $pdf->setX($m + 10);
  $pdf->Cell(0, 0, $mya->j_fullName, '', 2);
  $pdf->Cell(0, 0, $address, '', 2);
  $pdf->Cell(0, 0, "$city, $state $zip", '', 2);
  
  //$pdf->Line($m, $y = $pdf->getY(), $pageWidth - $m, $y);
  
  $pdf->setX($m);
  $pdf->setY($pdf->getY() + 10);

  list ($header, $classes, $rows, $tot) = x\getTxRows($start, $end); // get the transaction data
  extract(u\just('frombank fromyou toyou reward', $tot));
  $info = be\creditInfo(u\ray('asof', $end));

  $info0 = be\creditInfo(u\ray('asof', $start - 1));
  $bal0 = $info0->balance; // balance BEFORE start of this period
  list ($savings0, $savings9) = [$info0->savings, $info->savings];
  $dSavings = round($savings9 - $reward - $savings0, 2);
  list ($toSavings, $fromSavings) = u\order($dSavings >= 0, abs($dSavings), 0);
  $fromyou += $toSavings;
  $toyou += $fromSavings;
  $savingsTitle = t('Savings:');

  list ($bal9, $rewardsEver) = array($info->balance, $info->rewards);
//  u\EXPECT($bal9 == round($bal0 + @$frombank - @$fromyou + @$toyou + @$reward, 2), 'balance mismatch');
///**/  if (round($bal9, 2) != round($bal0 + @$frombank - @$fromyou + @$toyou, 2)) return r\go('empty', 'balance mismatch: ' . print_r(compact('tot', 'bal0', 'bal9'), 1));
  
  $pdf->barHead('SUMMARY');
  $fields = explode('; ', '//15/; Starting Bal/bal0/28/R; /plus/7/R; From Bank/frombank/23/C; /minus/7/C; FROM You/fromyou/23/R; /plus/7/R; TO You/toyou/23/R; /plus/7/R/; Rewards//23/C; /equals/7/R; Ending Bal/bal9/28/R');
  $pdf->colHeads($fields);
  $fields2 = explode('; ', '/savingsTitle/15/; /savings0/28/R; //7/R; //23/C; /minus/7/C; /fromSavings/23/R; /plus/7/R; /toSavings/23/R; /plus/7/R/; /reward/23/C; //7/R; /savings9/28/R');
  
  list ($plus, $minus, $equals, $space) = array('+', '-', '=', ' ');
  $committed = $mya->j_committed;
  $avg = r\averageBalance($mya->id, $mya->created, $end, TRUE);
  if ($jid = $mya->jid and $j = r\acct($jid)) $avg += r\averageBalance($j->id, $j->created, $end, TRUE);
  
  $yield = number_format($avg ? 100 * $rewardsEver / $avg : 0, 1) . '%';
  $numFields = 'bal0 frombank fromyou toyou reward bal9 rewardsEver committed savings0 savings9 fromSavings toSavings';
  foreach (u\ray($numFields) as $one) $$one = number_format(@$$one, 2);
  u\prefix('+ ', $savings0);
  u\prefix('+ ', $savings9);
  
  $pdf->setFontSize(.9 * $size);
  foreach (['fields', 'fields2'] as $f) {
    foreach ($$f as $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      $pdf->Cell($width, 0, $fldName ? $$fldName : '', '', @$fldName[strlen($fldName) - 1] == '9' ? 1 : 0, $align);
    }
  }
  
  list ($labelWidth, $numWidth) = array(45, 25);
  $pdf->newLine();
  $pdf->Cell($labelWidth, 0, 'Incentive Rewards Ever:');  
  $pdf->Cell($numWidth, 0, $rewardsEver, '', 0, 'R');
  $pdf->Cell(0, 0, " (Effective yield $yield APR)", '', 1, 'L');
  if ($mya->id == CGF_ID) {
    $shared = db\lookup('SUM(amount)', 'r_txs', "payee=:CGF_ID AND payeeFor=':R_SHARING_MSG' AND created<=:end", compact('end'));
    $pdf->Cell($labelWidth, 0, 'Sharing Donations Ever:');  
    $pdf->Cell($numWidth, 0, number_format($shared, 2), '', 1, 'R');
  } elseif ($start == u\monthDay1()) { // committed amount is not available retroactively
    $pdf->Cell($labelWidth, 0, 'Reserved to donate to CGF:');  
    $pdf->Cell($numWidth, 0, $committed, '', 1, 'R');
  }
  $pdf->newLine();

  $pdf->setFontSize($size);
  $pdf->barHead('DETAILS');
  $fields = u\ray('Tx#/tid/15/C /space/1/ Date//17/ Name//55/ /space/1/ Purpose//70/ /space/1/ Amount//20/R *Extra/extra/20/R');
  $pdf->colHeads($fields);

  $pdf->setFontSize(.9 * $size);
  foreach ($rows as $row) {
    foreach ($row as $k => $v) $row[$k] = str_replace(',', '', strip_tags($v));
    extract(u\just('tid date name fromyou toyou purpose reward', array_combine($classes, $row)));
    $amount = number_format($toyou - $fromyou, 2);
    if (strlen($purpose) > $purposeLenMax + 3) $purpose = substr($purpose, 0, $purposeLenMax) . '...';
    if ($reward == '--') $reward = 0;
    $extra = number_format($reward, 2);
    foreach ($fields as $one) {
      list ($head, $fldName, $width, $align) = explode('/', $one);
      if (!$fldName) $fldName = strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = str_replace(',', '', $$fldName)) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      $pdf->Cell($width, 0, $$fldName, '', $head == '*Extra' ? 1 : 0, $align, '', '', 1);
    }
    if ($toyou != $fromyou) x\addCat($cats, $purpose, $toyou - $fromyou, $dups);
    $catDesc = r\usdin($purpose) ? t('exchange fees') 
    : ($purpose == INFLATION_DESC ? $purpose : t('rewards, not including inflation adjustment'));
    if ($reward) x\addCat($cats, $catDesc, $reward, $zot);
  }
  
  $pdf->newLine();
  $pdf->Cell($pageWidth, 0, t('* The Extras column includes incentive rewards and any fees paid or received.'), '', 1 , 'L');
  
  if (@$dups) { // don't show categorization unless it's worthwhile
    $pdf->setFontSize($size);
    $pdf->newLine();
    $pdf->barHead('CATEGORY TOTALS');
    $fields = u\ray('Category/cat/75/ Total/total/25/R Count/count/17/R');
    $pdf->colHeads($fields);
    $pdf->setFontSize(.9 * $size);

    x\finishCats($cats);
    
    foreach ($cats as $cat => $ray) {
      list ($count, $amount) = $ray;
      $total = number_format($amount, 2);
      foreach ($fields as $one) {
        list ($head, $fldName, $width, $align) = explode('/', $one);
        $pdf->Cell($width, 0, $$fldName, '', $head == 'Count' ? 1 : 0, $align, '', '', 1);
      }
    }
  }
  
  if (u\test()) $pdf->Close(); else $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

define('R_GIFT_SHOWHIDE', "var other = jQuery('.form-item-amount'); if(jQuery('#edit-gift').val() == 0) {other.show();} else other.hide();");

function formDonate($form, &$sta) {
  $mya = r\acct();
  $donated = $mya->share ? t('<p>You have already made a donation. Thank you! Please feel free to donate again.</p>') : '';
  //  Whatever "share" percentage you choose this time will replace your previous choice') . " ($mya->share%).
  $giftLevels = array(
  // 10000 => t('Marble .. $10,000'),
  // 5000 => t('Granite ... 5,000'),
    2500 => t('Slate .... $2,500'),
    1000 => t('Glass .... $1,000'),
     500 => t('Iron ....... $500'),
     250 => t('Oak ........ $250'),
     100 => t('Two Bricks . $100'),
      50 => t('One Brick ... $50'),
      25 => t('Half Brick .. $25'),
      10 => t('Tile ........ $10'),
       5 => t('Cloth ........ $5'),
       0 => t('Water ... (other)'),
  );
  $oftens = array(0 => 'when?', 'M' => 'Monthly', 'Q' => 'Quarterly', 1 => 'Just Now (maybe more later)');
  $honors = array('-' => '(optional)', 'honor' => 'in honor of', 'memory' => 'in memory of', 'other' => 'other');
  $onchange = str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_GIFT_SHOWHIDE);
  $giftAttrib = attrib(compact('onchange'));
   
  $inSetup = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p><p>Also choose a percentage of your ongoing incentive rewards to share with Common Good Finance.');
  $form = array(
    'title' => item(t('Donate to Common Good Finance')),
    'subtext' => item($donated . '<p>' . t('Make a gift of any size, to help support the rCredits system. Buy a BRICK or some other piece of the Common Good Economy, as a tax-deductible donation (one-time, monthly, or quarterly). <b class="loud">NOTE: This is a donation, not a deposit.</b>') . $inSetup . t('<p>Thank you for your support!</p>')),
    'gift' => selectFld(t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT) + $giftAttrib, $giftLevels),
    'amount' => textFld(t('Other amount $'), ''),
    'often' => selectFld(t('When:'), '', required(), $oftens),
    'honor' => selectFld(t('Honoring:'), '', '', $honors),
    'honored' => areaFld('', [t('Honoring whom or what')]),
    'share' => $mya->ok ? '' : w\shareFld(),
    'submit' => submit('Donate'),
  );

  js(R_GIFT_SHOWHIDE, 'inline', 'footer');
  return labeled($form);
}

function formDonate_validate($form, &$sta) {
  $mya = r\acct();
  js(R_GIFT_SHOWHIDE, 'inline', 'footer');
  extract(u\just('gift amount share often', $sta['input']));
  if (!$amount = $gift ?: $amount) return say('missing field', array('field' => 'amount'), 'amount');
  if (!$often) return say('missing field', array('field' => 'when'), 'often');
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  if (!$mya->ok and $err = w\badShare($share)) return say($err, 'share');
//  $share = min($share, 999.999); // any bigger kills PDO
  $sta['input'] = compact(u\ray('amount share')) + $sta['input'];
}

function formDonate_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('amount often honor honored share', $sta['input']);
  if ($info['honor'] == '-') $info['honor'] = 'honor';
  if (!$info['honored']) $info['honor'] = '';
  $uid = $mya->id;
  $giftDate = r\rTime();
  $info += compact('uid', 'giftDate');
  $donid = db\insert('r_gifts', $info);
  if (!$mya->ok) $mya->update(u\just('share', $info));
  $msg = 'gift successful';
  if (!r\acceptGift($info + compact('donid'))) $msg .= '|gift transfer later';

  $amount = u\fmtAmt($info['amount']);
  if (!$mya->ok) {
    $mya->stepDone('donate', $info);
    say("$msg|step completed", compact('amount'));
    r\go('status');
  } else {
    r\tellStaff('gift', $info);
    say($msg, compact('amount'));
  }
}

/**
 * Show general or specific help. For members seeking general help, provide a secure communications channel.
 */
function formHelp($form, &$sta, $what = '') {
  include_once __DIR__ . '/rweb-help.inc';

  $signedIn = ($mya = r\acct()) ? TRUE : FALSE;
  if ($signedIn and $mya->cAdmin and $what and strpos($what2 = @u\deurlify($what), '&')) {
    extract(u\just('filename type', $what2));
    $s = file_get_contents($filename);
    list ($zot, $type) = explode('/', "$type/");
    $ext = $type ?: 'bin';
    u\beginDownload("from $mya->id.$ext");
/**/ echo $s;
    exit();
  }
  
  list ($what, $tag) = (strpos($what, '@')) ? explode('@', $what) : array($what == 'other' ? '' : $what, '');
  $title = item($what ? str_replace('Rcredits', 'rCredits', ucwords(str_replace('-', ' ', $what))) : t('General Help'));
  $stranger = t('Interested in rCredits? Visit <@a>rCredits.org</a> for more information or to request an invitation to participate.', ['@a' => 'a href=http://rCredits.org']);
  $text = fld('item', '', '', ($signedIn or $what)? helpText($what ?: 'general', $tag) : $stranger);
  if ($signedIn and !$what) {
    if ($mya->cAdmin) {
      $q = db\q('SELECT id, message AS msg FROM r_tous WHERE uid=:uid', ['uid' => $mya->id]);
      while ($row = $q->fetchAssoc()) {
        extract($row);
        $message[] = "#$id: " . u\decrypt($msg);
      }
      $message = $message ? join("<br><br>", $message) : '';
      $message = item($message, t('All messages:'), t('Be sure to delete resolved messages.'));
    } else $message = areaFld(t('Send a secure message to an rCredits Administrator:'), '', required());
    $file = fld('file', t('Attachment:'), t('(optionally) attach a file to your message. The file will also be encrypted.'));
    $submit = submit(t('Send'));
  }

  return labeled(compact(u\ray('title text message file submit')));
}

function formHelp_validate($form, &$sta) {
  if ($err = $_FILES['files']['error']['file'] and $err != 4) return say('file save error', 'file');
}

function formHelp_submit($form, &$sta) {
  $mya = r\acct(); $uid = $mya->id;
  extract(u\just('message file', $sta['input']));
  $time = r\rTime();

  foreach ($_FILES['files'] as $k => $v) $file[$k] = $v['file'];
  if ($tmp = $file['tmp_name']) { // got an attachment
    $filename = UPLOAD_DIR . "/$uid-$time";
    extract(u\just('name type size', $file));
    $link = BASE_URL . '/help/' . u\urlify("type=$type&filename=$filename");
    $extra = " [<a href=\"$link\">attachment</a>: $name type=$type size=$size bytes]";
    $message .= $extra;
    if (!rename($tmp, $filename)) return say('file save error', 'file');
  }  
  $message = u\crypt($message);
  db\insert('r_tous', compact('uid', 'time', 'message'));
  r\tellStaff(t('secure message from @who ', ['@who' => $mya->fullName]) . @$extra);
  say(t('Your secure message has been sent. An administrator will respond as soon as possible.'));
}

/**
 * Show the account's current membership status and steps to get to the next milestone.
 * @param acct $a: the account to test if just updating membership status bits -- otherwise empty
 * @param string $code: numeric means show specific screen even if not appropriate (for in-person demos)
 * @return TRUE if the member has done everything possible toward opening the account.
 */
function formMembership($a, &$sta = '', $code = '') {
  global $base_url;

  if (!($updating = (bool) $a) and !$a = r\acct()) return NULL; // no account available
  if ($updating and $a->ok) return TRUE;

  $myid = $a->id;  
  $co = $a->co;
  $ok = $a->ok;
  if ($code == 'gotPhoto') say('got photo');
  $mempage = is_numeric($code) ? $code : FALSE;
  extract($a->stepsDone ?: r\stepsDone0(), EXTR_PREFIX_ALL, 'ok');

  if (!$ok or !$a->can(B_MEMBER)) {
    $ready = (@$ok_sign and $ok_contact and $ok_donate and $ok_photo and $ok_prefs and $ok_connect); // @ for tests
    $ready = ($ready and ($co? ($ok_company and $ok_relations) : $ok_proxies));
  } else $ready = TRUE;

  if ($updating) return $ready;
  
  $message1 = $message2 = '';

  if ($ready and !$ok) {
    $nextMsg = $co ? t('you can begin accepting rCredits payments') : t('you can expect to receive your rCard in the mail within a day or two');
    $message1 = t("<h2>Your Account Setup Is Complete</h2><p>You have done everything you need to do. Once a staff member has approved your account, $nextMsg.</p><p>Thank you for joining us!</p>");
  } elseif (!$ok) { // not a member yet (no rCard, no vote)
    $message1 = ($ok_photo or $ok_sign or $ok_donate or ($ok_proxies and !$co) or $ok_contact or ($ok_connect and !$co) or $ok_prefs) ? 
          t("<h2>You're getting there!</h2><p>")
        : t('<h2>Welcome to rCredits</h2><p>Thanks for joining us. ');
    $message1 .= $co ? t('Before you can begin accepting rCredits payments, you must complete the following steps (in any order).') 
      : t('Before we can send your rCard, and before you can vote on community funding, you must complete the following steps (in any order). This usually takes about 20 minutes.');
    $message1 .= tt(' If you need help, please <@a1>send us an email</a>.</p>', ['a1' => 'a href="mailto:' . r\regionField('email') . '" target="_blank"']);

    $stepContact = t('Complete your <a>Contact Information</a>.');
    $stepAgree = t('Sign the <a>rCredits Agreement</a>.');
    $stepGift = t('Make a <a>Donation</a> of <i>any size</i>, to support the rCredits system.');
    $stepProxies = t('<a>Choose two people</a> to represent your views whenever you are not there, to discuss and vote on community funding issues. (A list of members in your area will be provided, to choose from.)');
    $stepPrefs = t('Set your <a>Account Preferences</a>.');
    $stepPhoto = t('Upload a <a>Photo</a> for your ') . ($co ? t('company profile.') : t('Member ID Card.'));
/*    $stepSecurity = t('<a>Provide your security information.</a> Upload ')
      . ($co ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotSecurity ? t(' <span class="pending">(DONE, pending approval)</span>') : ''); */
    $stepBank = t('<a>Connect</a> to the mainstream economy.');
/*
    $stepProof = t('<a>Prove your identity.</a> Upload ')
      . ($co ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotIdProof ? t(' <span class="pending">(DONE, pending approval)</span>') : '');*/
    $stepCompany = t('Tweak your <a>Company Information</a>.');
    $stepRelations = t('Set up <a>Relations</a> to individuals who will use or manage this account.');

    $stepNum = 1;
    $message2 = $ok_contact ? '' : memberStep(0, $ok_contact, $stepContact, 'settings/contact');
    $message2 .= memberStep($stepNum++, $ok_sign, $stepAgree, 'community/agreement')
              . memberStep($stepNum++, $ok_donate, $stepGift, 'community/donate');
              
    if (!$co) 
    $message2 .= memberStep($stepNum++, $ok_proxies, $stepProxies, 'settings/proxies');

//    $message2 .= memberStep($stepNum++, $gotSecurity, $stepSecurity, 'settings/security');
    $message2 .= memberStep($stepNum++, $ok_prefs, $stepPrefs, 'settings/preferences')
               . memberStep($stepNum++, $ok_photo, $stepPhoto, 'settings/security/photo?' . rand());

    $message2 .= memberStep($stepNum++, $ok_connect, $stepBank, 'settings/bank');
//    if (!$co) $message2 .= memberStep($stepNum++, $gotIdProof, $stepProof, 'settings/id-proof');
    if ($co)
    $message2 .= memberStep($stepNum++, $ok_company, $stepCompany, 'settings/company')
               . memberStep($stepNum++, $ok_relations, $stepRelations, 'settings/relations');

// (Suggest it after 1mo) if (!$co) $message2 .= memberStep(4, $invited, $step4, 'invite');
  } elseif (FALSE) {
    $message1 = t('<h2>You have completed all membership steps.</h2><p>Within 24 hours, you should receive an email saying your account is approved and you can expect your rCard in the mail within a day or two. Hurray!</p>'); // UNUSED
  } else {
    $bonusMsg = ($a->can(B_BONA) or ($a->hasBank and $a->j_minimum > 0)) ? '' : t('<p>To receive your $@BONUS signup bonus, you must have a way to get money into your account. Ask someone to pay you with rCredits, or <@a>transfer funds from your bank account</a>, or trade US Dollars for rCredits at a participating business. As soon as you get some money in, your signup bonus will arrive.</p>', ['@BONUS'=>R_SIGNUP_BONUS, '@a'=>"a href=$base_url/get"]);
    $personMsg = !$a->co ? t(' You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>') . $bonusMsg : '</p>';
    $message1 = t('<h2>Congratulations! Your account is Activated.</h2><p>You are authorized to use rCredits to buy and sell with other members.') . $personMsg;
  }
  $message = $message1 . "<table width=\"100%\">$message2</table>";
  
  $title = item(t('Membership Steps'));
//  $subtext = item(t('for ') . $a->fullName);

  $steps = item($message);
  $form = compact(u\ray('title steps addendum'));
  return labeled($form);
}

/**
 * Give member a chance to sign the rCredits Agreement. (agreement revised 4/17/2014)
 */
function formAgreement($form, &$sta) {
  $mya = r\acct();
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
//  $id = "edit-$name";
//  $check = boxFld('checkNUMBER', '', '', attrib(compact('id')));
  $model = @\render(boxFld('checkNUMBER'));
  $instructions = '';

  if (!$mya) {
    $instructions = '';
  } elseif ($date = $mya->signed) {
    $instructions = '';
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif (!$mya->co and !$mya->proSe and !$mya->admin) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $aacct = $mya->cAdmin ? ($mya->co ? r\acct($mya->helper) : $mya) : $mya->agent;
    $signedBy = textFld(t('Signed:'), t('Type your full name here ') . "($aacct->legalName)", required($mya->cAdmin ? $aacct->legalName : ''));
    $submit = submit(t('Sign'));
    $addendum = t('This means, for example, paying attention to your experience and making comments to improve the system.');
    if ($mya->cAdmin) $addendum .= t('</p><p>ADMINISTRATORS need not put checkmarks in the boxes.');
    
    $instructions = tt(<<<EOF
      <div id="instructions" class="well">
      <p><b>Instructions:</b> Put a check mark in each checkbox to indicate you have read that section and agree to it. Really DO read the agreement and make sure you understand it. This agreement, that we make with each other, is the foundation for the rCredits system and the Common Good Economy.</p>

      <p><b>Summary.</b> In the first part of the agreement (A), you acknowledge the intent and purpose of the rCredits system. In the second part (B), you agree to use the rCredits system the way it is designed to work. Notice you can stop participating at any time, simply by giving back the rCredits you got for free (B4).</p>
      
      <p><b>Responsibility.</b> Since the purpose of the rCredits system is to give us control of our local economy, as a community, the agreement requires that each member be responsible. You agree to take on that responsibility "as [your] life circumstances allow".@addendum</p>
      </div>
EOF
    , compact('addendum'));
  }

  $personOnly = ($mya and $mya->co) ? '' : tt(<<<EOF
  <li>[6] I agree to participate responsibly in managing the Common Good Economy to the best of my ability and as my life circumstances allow, including:
    <ol style="list-style-type:lower-alpha;">
      <li><a href="@BASE_URL/help/paying-attention">paying attention</a> and understanding how the Common Good Economy works</li>
      <li>participating in the <a href="@BASE_URL/help/decision-principles">decision process</a> to set my community's funding priorities:
        <ol style="list-style-type:lower-roman;">
          <li>to foster healthy, fulfilling lives for all community members and</li>
          <li>to assist communities elsewhere</li>
        </ol></li>
      <li>participating in <a href="@BASE_URL/help/other-decisions">other decisions</a> from time to time, as needed.</li>
    </ol>
  </li>
EOF
  );

  $agreement = tt(<<<EOF
@instructions
<div id="agreement">
<h4>A. I recognize that:</h4>
<ul>
  <li>[-2] rCredits are intended as the basis for the Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1] Together as a community we have the power to <a href="@BASE_URL/help/issuing-rcredits">issue credit</a> for our own use as a medium of exchange. It is our promise to accept rCredits that gives them value.</li>
  <li>[0] Governance by the people is essential for a just society.</li>
</ul>

<h4>B. As a responsible participant in the Common Good Economy:</h4>
<ol>
  <li>[1] As long as I can spend my rCredits easily or exchange them easily for US Dollars, I will accept them without limit as payment for my goods and services. I will exchange rCredits for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will accept rCredits from members of any <a href="@BASE_URL/help/common-good-community">Common Good Community</a> in good standing. Once my geographic area becomes a Common Good Community, in the event of a <a href="@BASE_URL/help/cashflow-crunch">cash flow crunch</a> I will <a href="@BASE_URL/help/backing-rcredits">back</a> rCredits up to my average balance over the past six months.</li>
  <li>[3] Whenever I exchange rCredits for US Dollars or vice versa, I will exchange them one for one, and I will price my goods and services the same in rCredits and in US Dollars. </li>
  <li>[4] I understand I will receive some rCredits from the community as Incentive Rewards that I can spend but cannot <a href="@BASE_URL/help/cashing-out">cash out</a>. If I stop participating (or <a href="@BASE_URL/help/graceful-failure">if the system ends</a>) within 6 months of receiving my first rCredits <i>or before the community has <a href="@BASE_URL/help/enough-revenue">enough revenue</a> to cover the Incentive Rewards</i>, I will <a href="@BASE_URL/help/graceful-failure">immediately give back</a> whatever amount of Incentive Rewards I have received (while keeping any rCredits I received from other participants).</li>
  <li>[5] I promise to maintain a high level of honesty, integrity, and <a href="@BASE_URL/help/ethics">ethics</a> in my dealings with other rCredits participants and with the community (including the rCredits system itself). When there is a dispute, I will follow the <a href="@BASE_URL/help/dispute-resolution-process">Dispute Resolution Process</a> and will honor its outcome.</li>
  @personOnly
</ol>
</div>
EOF
  , compact('instructions', 'personOnly'));
//  <li>[5] I will move my deposits and investments to the Common Good Economy when that is sensible, in my judgment.</li>

  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", (!$mya or !$mya->can(B_MANAGE) or @$date) ? '' : $box, $agreement);
  }
  $agreement = fld('item', '', '', $agreement);
  
  return labeled(compact(u\ray('title agreement signedBy submit')));
}

function formAgreement_validate($form, &$sta) {
  $mya = r\acct();
  $i9 = !$mya->co ? R_AGREE_9 : R_AGREE_9 - 1;
  if (!$mya->cAdmin) for ($i = R_AGREE_0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  $legalName = r\agent()->legalName;
  if (!$mya->cAdmin and strcasecmp(trim($sta['input']['signedBy']), $legalName) != 0) return say('bad signature', compact('legalName'), 'signedBy');
}

function formAgreement_submit($form, &$sta) {
  $mya = r\acct();
  $signed = r\rTime();
  extract(u\just('signedBy', $sta['input']));
  r\acct()->update($info = compact('signed', 'signedBy'));
  $mya->stepDone('sign', $info);
  say('info saved|step completed');
  r\go('status');
}

/**
 * Choose one proxy.
 * @param int $priority: 1 for proxy, 2 for alternate
 * uid is not passed, for security.
 */
function formProxy($form, &$sta, $priority = '') {
//  if ($priority != 1 and $priority != 2) w\hack('bad proxy priority');
  if ($priority != 1 and $priority != 2) ;
  $dft = r\acct()->proxy($priority);
  if (!$choices = w\proxyChoices($dft)) r\go('settings/proxies', 'no local proxy');

  $title = $priority == 1 ? t('Your #1 Proxy') : t('Your Alternate Proxy');
  $title = item($title . t(' (choose one)'));
  $subtext = item(t('Choose and click someone you know and trust. Top suggestions for you are highlighted in green.'));
  $list = item($choices);
  $priority = hidFld($priority);
  return compact(u\ray('title subtext list priority'));
}

function formProxy_validate($form, &$sta) {
  extract(u\just('priority proxyChoice', $sta['input']));
  $choice = key($sta['input']);
  u\EXPECT(substr($choice, 0, 2) == 'i-', 'missing proxy choice');
  $proxyChoice = substr($choice, 2);
  if (@$proxyChoice == r\acct()->proxy(1+2 - $priority)) r\go('settings/proxies', 'doubled proxy', 'err'); // can't just return say() because js gets deactivated (drupal bug?)
  $sta['input'] += compact('proxyChoice');
  return formProxy_submit($form, $sta); // oddly, _submit never gets called otherwise
}

function formProxy_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('priority proxyChoice', $sta['input']));

  $mya->proxy($priority, $proxyChoice);
  if ($mya->proxy(1+2 - $priority)) { // do we already have the other proxy?
    $mya->stepDone('proxies');
    r\go($mya->ok ? 'settings/proxies' : 'status', 'info saved');
  } else say('proxy to go');

  r\go('settings/proxies', 'info saved');
}

/**
 * Choose default proxies.
 */
function formProxies($form, &$sta) {
  $mya = r\acct();
  $title = item(t('Proxies'));
  $afterOk = $mya->ok ? '' : ', once your account is approved,';
  $subtext = item(tt(<<<EOF
    <p><b>Direct democracy</b> is at the heart of the rCredits design. In each community, members meet to discuss and decide on funding priorities for incentives, grants, loans, and investments of rCredits for the greater good.</p>
    <p>You assign a person in your community as your ongoing Representative (Proxy). This should be a member you know and trust and generally agree with. Whenever you don't vote, your Representative votes for you. The Representative's vote simply counts double (one vote for them and one vote for you). If your Representative also fails to vote, then your Representative's Proxy votes for all three of you.<br><br>Everyone's voice is counted, whether or not they vote directly &mdash; one person one vote.</p>
    <p>Our community's most trusted Representatives (chosen by many people to be their proxy) will meet in person to research and debate specific issues. These trusted Representatives ("Trustees") do not decide broad policy issues &mdash; all members decide the policy issues. Instead, Trustees oversee public discussions, choose wording for questions to be voted on, take action as directed by the members, and assure that the system is running smoothly.</p>
    <p>If you don't see listed here the people you want, choose someone else temporarily, then$afterOk use the <a href="@BASE_URL/community/invite">Invite</a> tab to invite who you most want to represent you.</p>
EOF
  ));

  $mya = r\acct();
  foreach ([1, 2] as $priority) {
    if ($p = $mya->proxy($priority)) $p = r\acct($p)->fullName; else $p = t('(none chosen)');
    $proxy[$priority] = $p . t(' [<@a>show choices</a>]', ['@a'=>"a href=proxy/$priority"]);
  }
  $proxy1 = item($proxy[1], t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'));
  $proxy2 = item($proxy[2], t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'));
  $form = compact(u\ray('title subtext proxy1 proxy2'));
  return labeled($form);
}

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 * NOTE: We allow agents to send invitations on behalf of a company, but be aware that companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever it invites -- something usually reserved for humans.
 */
function formInvite($form, &$sta) {
  global $base_url;
//  if ($agent = r\agent()) {
  if ($mya0 = $mya = r\acct()) {
    $cAdmin = $mya->cAdmin;
    $admin = $mya->admin;
    if ($mya->id == 1) return say(t('Sys Admin cannot invite.'), 'err');
//    if ($admin = $mya->admin) $mya = r\acct(R_ADMIN_QID); // use admin's personal info
    $stepsDone = (object) $mya->stepsDone;
    $nearly = ($admin or $mya->can(B_MEMBER)) ?: (@$stepsDone->sign and @$stepsDone->donate);
  } else $cAdmin = FALSE;
  
  $title = item(t('Invite Someone to rCredits'));
  $blurb = blurbLink();
  $a1 = "a href=$base_url/community/invite/waiting class=\"btn btn-xs btn-primary\" role=button";
  $a2 = "a href=$base_url/community/invite/invited-whom class=\"btn btn-xs btn-primary\" role=button";
  $where = r\location($mya0->cttyA); // currently UNUSED
  $inviteCode = $mya0->iCardCode(IBY_EMAIL);
  $inviteLink = "$base_url/signup/code=$inviteCode";
  $warning = $mya->co ? t('<p>Note that you are sending an invitation on behalf of a company. Companies are not people and there is a danger in giving companies permission to act like people. The company is "trusting" whoever you invite -- a sentiment usually reserved for humans.</p>') : '';
  //Plus $@R_COUNTED_BONUSr for each employee, if they open a company account. 
  $subtext1 = tt("We're sorry, you cannot invite someone to sign up until you have signed the <a href=\"@BASE_URL/community/agreement\">Agreement</a> and made a <a href=\"@BASE_URL/community/donate\">donation</a>.<br><br>Return to the <a href=\"@BASE_URL/status\">Membership Steps</a> page to see where you stand.");
  //      <li>Order @orderCount sheets of 10 invitation cards by mail (free!). @orderSubmit</li>
//      <li><b>OR</b> Print @printCount sheets of 10 invitation cards. @printFront @printBack</li>

  $aSheet = "a href=$base_url/community/invite/print target=_blank class=.nospin";
//  $aBack = 'a href=http://commongoodfinance.org/files/promo/invitation-back.pdf target=_blank';
  list ($region, $zot) = explode(R_MEMBER_MARK, $mya0->mainQid);
  foreach (u\ray('NEW MIW') as $k) if ($region == $k) {
    list ($nm, $zot) = explode('@', $mya0->cttyA->email);
//    $aBack = str_replace('-back', '-back-' . $nm, $aBack);
  }
  $friend = $mya->co ? t('customer, employee, or supplier') : t('friend');
  $size = $mya->co ? '4.25" &times; 2.75"' : '3.5" &times; 2"';
  
  $subtext2 = tt(<<<EOF
<p>Your friends and business associates can join rCredits <b>only if you invite them</b>. Get a $@R_HELPER_BONUSr reward for each $friend you invite, who signs up! You may need to help or remind the people you invite.</p><p>Invite <b>only people you trust</b> who live, shop, or work in your community. <@a2>Who you have invited</a> <@a1>People waiting to be invited</a></p>
  <br>
  <p>Here are three different ways to invite someone:</p>
    <ul>
      <li><b>BEST: <@aSheet>Print</a> a sheet</b> of invitation cards ($size, on card stock).<br>(or <a href="mailto:info@rCredits.org?subject=rCredits Invitation Card request&body=Please send me some more rCredits invitation cards. Thanks! -- $mya->fullName (account #$mya->mainQid)" target="_blank">ask to have some mailed to you</a> at no charge)</li>
      <li><b>Tell your $friend</b> to sign in at rCredits.org with invitation #&nbsp;@inviteCode or send them this link: <div id=inviteLink>@inviteLink</div></li>
      <li><b>Complete this form</b> to send an invitation NOW:</li>
    </ul>
EOF
  , compact(u\ray('a1 a2 where aSheet inviteLink inviteCode')));
//  (Later, you will need to confirm you know and trust them.)
  $subtext = item(!@$nearly ? $subtext1 : ($subtext2 . $warning));
//  $contact = textFld(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());

  $email = textFld(t('Email:'), [t('Their email address'), t('An email address for the person you want to invite')]);
//  $emails = areaFld(t('Email(s):'), t('A comma-separated list of email addresses to invite'));
  if ($cAdmin) $dupsOk = boolFld(t('Dups Okay?'), '', FALSE);
  $trusted = boolFld(t('Trusted?'), t('If you lent this person (or people) $250, would you trust them to pay it back?'), @$cAdmin ?: NULL);
  $subject = textFld(t('Subject:'), t('The subject of your message'), required((@$admin ? ADMIN_NAME : $mya->fullName) . t(' invites you to rCredits')));

  $default = @$admin ? 
      tt("The rCredits network is growing. Check it out and sign up!\n\nWilliam")
    : (@$mya->data['inviteBody'] ?: tt("rCredits are great. I signed up. Check it out!\n\n@name", u\ray('name', $mya->fullName)));
  $messageHelp = tt('Invite them to sign up, and say why. A @blurb will follow your message.', compact('blurb'));
  $message = areaFld(t('Message:'), $messageHelp, required($default));
  $submit = submit(t('Send'));
  $js = <<<EOF
jQuery('#inviteLink').click(function () {SelectText(this.id);}); // document.execCommand("copy");
function SelectText(element) { // from http://stackoverflow.com/questions/985272
    var doc = document
        , text = doc.getElementById(element)
        , range, selection
    ;    
    if (doc.body.createTextRange) {
        range = doc.body.createTextRange();
        range.moveToElementText(text);
        range.select();
    } else if (window.getSelection) {
        selection = window.getSelection();        
        range = doc.createRange();
        range.selectNodeContents(text);
        selection.removeAllRanges();
        selection.addRange(range);
    }
}
EOF;
  js($js);
  
  $form = $nearly ? 
      labeled(compact(u\ray('title subtext email dupsOk trusted subject message submit')))
    : compact(u\ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(u\just('email emails trusted', $sta['input']));
  if (op($sta) == 'submitN') {
    if (!$emails) return say('bad email', compact('emails'), 'emails');
    foreach (u\ray($emails) as $one) if (!\valid_email_address(trim($one))) return say("bad email: $one", 'emails');
  } elseif (!\valid_email_address($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  global $base_url;
  extract(u\just('email emails subject message dupsOk', $sta['input']));
  $mya = r\acct();

  if (op($sta) == 'submitN') {
//    foreach (u\ray($email) as $one) sendInvite($one, $subject, $message, @$dupsOk);
    $code = ''; // get a random code just once
    foreach (u\ray($emails) as $email) {
      $code = r\invite($email, $mya->id, $code, $subject, $message);
      $links[] = "$base_url/signup/code=$code/e=$email";
    }
    $links = join('<br>', $links);
    say('invite links', compact('links'));
  } else {
    $email = strtolower($email); // otherwise u\SUBS breaks on cap after @
    sendInvite($email, $subject, $message, @$dupsOk);
    $data = u\ray('inviteBody', substr($message, 0, 10000)) + $mya->data;
    $mya->update(compact('data'));
  }
}

/**
 * Send another member a message.
 */
function formMessage($form, &$sta) {
  $title = item(t('Send a Message'));
  $subtext = item(t('Complete this form and click Send. The other member will receive an email from your email address. When they reply, you will also have their email address, so you can communicate by email after that (you won\'t need to use this form again).'));
  $to = w\textFld(t('To:'), [t('Recipient'),t('The account, name, or partial name of the member you want to send a message to.')], required());
  $subject = w\textFld(t('Subject:'), [t('Subject')], required(t('a message from rCredits member ') . r\acct()->fullName));
  $message = areaFld(t('Message:'), [t('Your message goes here.')], required());
  $submit = submit(t('Send'));
  $onSubmit = 'return ' . w\whoFldSubmit('to', t('Send to @name?'));

  return labeled(compact(u\ray('title subtext to subject message submit')), $onSubmit);
}

function formMessage_validate($form, &$sta) {
  extract($info = u\just('to subject message', $sta['input']));
  if (strpos($to, '@')) return say(t('If you know the member\'s email address, don\'t use this form -- just send them an email.'), 'to');
  if (!$a = w\whois($to, 'to', $info)) return FALSE;
  u\preray(u\ray('to fullName', $a->email, $a->fullName), $sta['input']);
}

function formMessage_submit($form, &$sta) {
  extract(u\just('to subject message fullName', $sta['input']));
  $mya = r\acct();
  r\rMail('', $to, u\ray('noFrame subject body', TRUE, $subject, $message), "\"$mya->fullName\" <$mya->email>");
  say('sent message', compact('fullName'));
}

function formVideos($form, &$sta) {
  $title = item(t('rCredits Member Videos'));
  $subtitle = fld('item', '', '', t('The videos may not be available until mid-January. Try again in a few days!'));
  $form = compact(u\ray('title subtitle'));
  return labeled($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from r\go() in Summary

/**
 * Temporary savings amount-setter until 12/15/2015
 */
function formChooseSavings($form, &$sta, $args = '') {
  extract(u\just('qid ecode', $args));
  if (!@$qid or !@$mya = r\acct($qid) or @$ecode != $mya->emailCode) w\hack('choosing savings', $args);
  $title = item(t('Choose Your Savings Reserve Amount'));
//  $subtitle = item(t(''));
  $savings = textFld(t('Savings Reserve:'), [t('Savings amount'), t('How much to hold as a savings reserve, for later or as needed. Your Savings Reserve works like a traditional reserve account that kicks in whenever you would otherwise overdraw your primary account -- except there are no fees, you don\'t have to open a second account, and you don\'t have to put any money in it because your incentive rewards go in it automatically. This amount is not counted as part of your account balance.')], required(u\fmtAmt($mya->savings ?: round($mya->rewards + 4.999, -1))));
  if (u\starts($qid, 'NEW.')) {
    $btw = item(t('<big class="loud">By the way</big>, this new feature means we are ready to begin discussion in Greater Greenfield on how to use our rCredits Escrow funds for incentives, grants, loans, and local investments. We hope you will be part of that discussion, beginning with a meeting in January.'));
    $discuss = boxFld('discuss', t('Participate?'), t('Yes! Count me in.'));
  }
  $qid = hidFld($qid);
  $submit = submit();
  return labeled(compact(u\ray('title subtitle savings btw discuss qid submit')));
}

function formChooseSavings_validate($form, &$sta) {
  extract(u\just('qid savings', $sta['input']));
  $mya = r\acct($qid);
  if ($err = amtErr('savings', $sta)) return say($err, 'savings');
  $savings = $sta['input']['savings'];
  if ($savings < $mya->rewards) return say('savings too low', ['rewards' => u\fmtAmt($mya->rewards)], 'savings');
}

function formChooseSavings_submit($form, &$sta) {
  extract($info = u\just('qid savings discuss', $sta['input']));
  $mya = r\acct($qid);
  $mya->update(compact('savings'));
  r\tellStaff("$mya->fullName chose savings=$savings (rewards=$mya->rewards)" . ($mya->community == r\serverUid() ? (@$discuss ? ' Participant!' : '') : ''), $info);
  doSay(t('Success! Thank you for setting your Savings Reserve amount.'), '');
}
   
/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 * @param string $qid: (optional) qid of the account to summarize and manage (for cAdmin)
 */
function formSummary($form, &$sta, $qid = '') {
  global $base_url;
  if (!$mya = r\acct()) return r\go(isGAME ? 'community/game' : 'signin'); // this cannot be '' (infinite redirect)
  if ($mya->closed) say(t('Your account is closed, but you can still see your account history.'));
  $myid = $mya->id;
  
  if ($mya->can(B_REGULATOR)) { // some kind of admin (regulator, cAdmin2, cAdmin, or admin
    include_once __DIR__ . '/../admin/admin-web.inc';  
    if (!$a = r\acct($qid)) return r\go('/sadmin/member-list', t('Unrecognized Account Code: ') . $qid, 'ERR');
    $form = w\adminSummary($a, $mya->cAdmin and $a->id != $mya->agentId); // ctty admin not administering own acct
    foreach (u\ray('photo community') as $k) if (@$form[$k]) $$k = $form[$k]; // move these up (see return line)

    $extras = '<br>' . u\fmtPhone($a->phone) . " ($a->faxetc)<br>$a->email";
    $idLink = 'href="' . $base_url . '/print-rcard/' . $a->mainQid . '" target="_blank"';
    if (!$a->co and $a->isNormal and $mya->cAdmin) $extras .= " <a $idLink>" . t('Print ID Card') . '</a>';
  } else { // normal
    $a = $mya; // non-cAdmin (or cAdmin managing own account)
//    $a->refreshCan(); // update any permissions that may have been set elsewhere (superfluous?)

    if ($mem = formMembership($a)) { // all steps complete
      if (u\test() and $mem !== TRUE) return $mem; // redirected to another page
      if (!$a->ok and !$a->closed) say('setup complete' . ($a->confirmed ? '' : '|must be confirmed'));
    } else say('take a step');
    
    if ($a->legalName != $a->fullName) $legalName = item($a->legalName, t('Legal Name:'));    
    //extract(u\just('r usd pAccts', r\stats($a->community)));
    extract(u\just('returnMo return cttyBeneMo cttyBene', $a->j_stats()));
    $returnMo = item("$returnMo% APR " . t('over the past month'), t('Your return:'));
    $return = item("$return% APR " . t('overall (ever)'), ' ', t('Your effective rate of return on your average balance [<@a>details</a>]', u\ray('@a', 'a href=help/your-return')));
    $cttyBeneMo = item("$cttyBeneMo " . t('over the past month'), t('Social return:'));
    $cttyBene = item("$cttyBene " . t('overall (ever)'), ' ', t('How much your participation has benefited the community financially [<@a>details</a>]', u\ray('@a', 'a href=help/social-return')));  
    
    $form = compact(u\ray('legalName returnMo return cttyBeneMo cttyBene'));
  }
  
  if (!$a->co) {
    $hasMultipleAccounts = db\exists('r_relations', 'other=:id AND permission>0', ['id' => $a->id]);
    $another = $hasMultipleAccounts ? t('Another') : t('a');
    $openCompany = submit(t('Open @another Company Account', ['@another' => $another]));
    if ($hasMultipleAccounts) $clickPhoto = item(t('Click your photo to manage your other account(s).'));
  }
  $title = item(t('Account Summary'));
  if (isDEV and !$a->proSe) $agentName = ' &nbsp; &nbsp; ' . t('<b>Agent: </b>') . $a->agent->fullName;
  $accountName = item("$a->j_fullName ($a->j_name)" . @$agentName, t('Name:'), $a->postalAddr . @$extras);
  if ($a->isNormal) { // unless this is a community or region account
    $acctType = $a->co ? t('company account') : ($a->jid ? t('joint account') : t('personal account'));
    $balance = u\fmtAmt($a->j_balance);
    $totalDesc = u\fmtAmt($a->j_r);
    $balanceDesc = $a->savings ? t(' plus your savings, for a total of @total', ['@total' => $totalDesc]) : '';
    if ($a->j_rewards) $balanceDesc .= t(' (see <@a>History</a>)', ['@a'=>"a href=$base_url/history"]);
    $balance = item($balance . "<small>$balanceDesc</small>", t('Balance:'));
    $savings = item(u\fmtAmt($a->j_savings) . t('<small> (including @rewards in rewards to date)</small>', ['@rewards' => u\fmtAmt($a->j_rewards)]), t('Savings:'), t('funds set aside for later') . ($a->co ? '' : t(' or as needed')));
    $committed = item(u\fmtAmt(max(0, $a->j_committed)), t('Committed:'), t('reserved for donation to Common Good Finance'));

    $invites = db\count('r_invites', 'inviter=:myid', compact('myid'));
    $successes = db\count('users', 'helper=:myid AND (:IS_OK OR :IS_CLOSED)', compact('myid'));
    $invites = item(t('You have invited @invites people (@successes are now members).', ['@invites' => $invites, '@successes' => $successes]), t('Invitations:'));    
/* disable for now    $floorDesc = t('If you choose "Debt Okay", your combined balance (account balance plus savings reserve) can go this far below zero.')
    . ($a->j_floor < 0 ? t(' for up to 30 days with no charge') : '')
    . '. ' . t('See "Debt Okay" in') . " <a href=\"$base_url/settings/preferences\">Preferences</a>.";
  $creditLimit = item(u\fmtAmt(-$a->j_floor), t('Credit limit:'), @$floorDesc);
  */
  } else $acctType = t('community account');

  if ($mya->cAdmin) {
    $extra = " id#$a->id";
    if ($bankInfo = $a->bankInfo) {
      extract($bankInfo, EXTR_PREFIX_ALL, 'b');
      $bankInfo = "$b_name (routing #$b_route)\n$b_address, $b_city, $b_state $b_zip";
      $extra .= ' ' . w\popHelp(t('bank info'), $bankInfo, '#'); //strtr(t(' <@a>bank info</a>'), ['@a'=>$bankInfo]); // strtr circumvents Drupal crap
    }
  } else $extra = '';
  $accountId = fld('item', t('ID:'), '', ($a->mainQid ?: 'N/A') . ' (' . $acctType . ')' . $extra);

  $fields = 'title clickPhoto openCompany photo accountName accountId community balance savings creditLimit committed invites';
  return labeled(compact(u\ray($fields)) + $form);
}

function formSummary_validate($form, &$sta) {
//  if (!r\acct()->cAdmin2) return hack('non-admin submitted summary form');
  if (op($sta) == 'openCompany') return;

  extract(u\just('uid who legalName federalId creditLimit rTrader helper mediaConx moves adminable', $sta['input']));
  $a = r\acct(@$uid);
  
  if ($federalId == R_ON_FILE) {
    unset($sta['input']['federalId']);
  } elseif (@$federalId and $err = u\badSsn($federalId, $a->dob)) return say($err, ['what' => 'federalId'], 'federalId');

  if (@$helper) {
    if (!$h = whois($helper, 'helper', $sta['input'])) return say(t('I don\'t know what helper account you mean.'), 'helper');
    $helper = $h->id;
  } else {unset($helper); unset($sta['input']['helper']);}

  if ($rTrader and !$a->ok and $a->isNormal) {
    if (!@$helper or $helper == 1) return say(t('You must specify who gets the helper reward.'), 'helper');
    if (is_null(@$adminable[B_MEMBER])) return say(t('That account is not READY (not a member).'), 'adminable'); // test "is_null" because 0 is the value when set
    if (!$a->confirmed) say(t('NOTE: Member @helperName has not yet confirmed this invitation.'), ['helperName' => $h->fullName]);
  }

  foreach (u\ray('creditLimit') as $k) {
    if (isset($$k)) {
      if ($err = u\badAmount($$k)) return sayFieldErr($err, $k); 
      $$k = u\cleanAmount($$k);
    }
  }
  if ($rTrader and isset($mediaConx) and $err = u\badAmount($mediaConx, '>=0', 0)) return sayFieldErr($err, 'mediaConx');
  if (isset($moves) and $err = u\badAmount($moves, '>=0', 0)) return sayFieldErr($err, 'moves');
  if (@$who and !$who = whois($who, 'who', $sta['input'])) return say(t('I don\'t know what account you mean to go to.'), 'who');
  u\preray(compact(u\ray('helper who creditLimit')), $sta['input']);
}

function formSummary_submit($form, &$sta) {
  include_once __DIR__ . '/../admin/admin.inc';
  
  $mya = r\acct();
  if (op($sta) == 'openCompany') return r\go('signup-company');
  if (!$mya->cAdmin) return; // hack attempt

  extract(u\just('uid who legalName federalId creditLimit notes helper mediaConx moves rTrader risks adminable', $sta['input']));
  $a = r\acct(@$uid);

  $a->stampNotes($notes);
  
  if ($mya->admin) { // most of this is for regional admin only
    $aa = $a->agent;
    if (@$who and ($who->community == $aa->community or $aa->id == 1)) {
      svar('myid', $who->id); // make it the new current account
      list ($newAcct, $oldAcct) = array($who->fullName, $a->fullName);
      return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
    } 
    
    $DBTX = \db_transaction();

    if ($a->isNormal) {
      $fullName = $a->fullName;
      if (!$a->confirmed and @$helper and @$helper != $a->helper) { // helper has not confirmed yet (must be self-signup)
        $a1 = r\acct($helper)->makeDo('confirmInvite', $a->id); // link to confirm invitedness
        r\message($helper, 'confirm invite', compact('a1', 'fullName')); // ask inviter to confirm
      }

      if ($rTrader and !$a->ok) { // activating the account
        $DBTX2 = \db_transaction();
        u\EXPECT($helper, 'no helper!');
        $a->update(compact('helper')); // this must happen before activating account (on next line)
        r\membershipEvent($a, 'ok', $rTrader);
        a\secureVery($a);
        if (!$a->co) {
  //        if (!a\shrinkPhoto($a, $err)) return say($err, 'ERR');
          r\membershipEvent($a, 'bona');
          $adminable[B_BONA] = B_BONA; // otherwise gets unset
        }
        unset($DBTX2); 
      } elseif (@$federalId and $federalId != $a->federalId) $a->update('ssnData', '');
    }

    $a->setBit(B_OK, $rTrader, TRUE);
    $risks0 = $a->risks;
    $a->setRisks(@$risks);
    if ($a->risks != $risks0) {
      include_once __DIR__ . '/../rcredits-risk.inc';
      k\acctRisk1($uid);
    }

    $a->setBits(@$adminable);
    if ($a->helper and ($a->ok or !@$helper)) unset($helper); // don't overwrite if too late or empty
  //  $floor = -$creditLimit;
    $a->update(compact(u\ray('legalName federalId floor notes helper mediaConx moves')));
    
    unset($DBTX);
  } else $a->update(compact('notes')); // community admin
  
  say('info saved');
}

/**
 * Recalculate risk.
 * @param string $type: type of entity: acct or tx
 * @param int $id: record ID for the entity
 * @param bool $show: <show the risk calculation>
 */
function formRerisk($form, &$sta, $type = '', $id = '', $show = FALSE) {
  include_once __DIR__ . '/../rcredits-risk.inc';
  global $showRiskCalc; 
  
  $showRiskCalc = '';

  if ($type == 'acct') {
    k\acctRisk1($id, $risk, $calc);
    $w = ''; $m = K_RED;
  } elseif ($type == 'tx') {
    $x = r\x($id);
    k\txRisk1($x->ray, $risk, $calc);
    $w = u\fmtAmt($x->amount);
    $m = tt('@K_RED / @K_THRESHOLD');
  }

  $risk = round($risk);
  
  $showRiskCalc = <<< EOF
  <table id="riskCalc">
  <tr><th>Risk Name</th><th>Weight</th><th>Multiplier</th><th>Risk</th><th>Set Flag?</th></tr>
  $showRiskCalc
  <tr><td><b>TOTAL</b></td><td>$w</td><td>$m</td><td>$calc</td><td>= $risk</td></tr>
  </table>
EOF;
  say($showRiskCalc);
  r\go("sadmin/$type-info/$id", t('info saved'));
}

/**
 * Show information about a specific transaction and its risks.
 */
function formTxInfo($form, &$sta, $xid = '') {
  include_once __DIR__ . '/../admin/admin-web.inc';  
  include __DIR__ . '/risk-descs.inc';
  return txInfo($xid);
}

/**
 * Output an account photo if the caller is authorized.
 * @param string $acct: qid for the account
 * @param string $cardCode: rCard security code
 * @return the photo if the account is a company or the cardCode is valid. Otherwise a generic picture.
 */
function memberPhoto($acct = '', $cardCode = '') {
  header("Content-type: image/jpeg");
  if ($a = r\acct($acct) and ($a->co ?: ($a->cardCode == $cardCode))) $photo = $a->photo();
/**/  echo @$photo ?: \file_get_contents(DRUPAL_ROOT . R_PATH . '/images/icons/account.png');
  exit();
}

/**
 * Output a small version of the current account's photo.
 */
function formIcon() {
  list ($acct, $code) = ($a = r\acct()) ? [$a->mainQid, $a->cardCode] : ['', ''];
  memberPhoto($acct, $code);
  exit();
}

/**
 * Handle the uploaded photo: crop, resize, rotate, and save.
 * @param assoc $args: parameters passed from croppic
 * @return json success or error message            
 */
function photoUpload($uid = '', $sid = '') {
  global $base_url;
  $f0 = u\ray('imgUrl imgInitW imgInitH imgW imgH imgY1 imgX1 cropH cropW rotation uid sid');
  $f = u\ray('url w0 h0 w h y1 x1 cropH cropW angle uid sid');
  foreach (array_combine($f0, $f) as $k0 => $k) $$k = $_POST[$k0];

  if (!@$uid or !@$sid or !db\exists('sessions', 'sid=:sid', compact('sid'))) return w\photoRet(t('bad sid'));
  $mya = r\acct($uid);
  $quality = $mya->co ? 100 : 50; // jpeg compression/quality (0=worst quality, 100=best quality)

  if (!$what = @getimagesize($url)) return w\photoRet(t('That is not an image file.'));
  list ($zotW, $zotH, $type, $zotAttr) = $what;
  if (!(imagetypes() & $type)) return w\photoRet(t('That image format is not recognized.'));
  $ext = image_type_to_extension($type, FALSE);
  $func = 'imagecreatefrom' . ($ext == 'bmp' ? 'wbmp' : $ext);
  if (!$img0 = $func($url)) return w\photoRet(t('file save error'));
  /* preliminary rotate and/or mirror if necessary (never necessary -- jscript apparently does it)
  $exif = exif_read_data($url);
  if ($orientation = @$exif['Orientation']) {
    $orients = u\ray('0/0 0/0 0/1 180/0 180/1 270/1 270/0 90/1 90/0');
    list ($angle0, $mirror) = explode('/', $orients[$orientation]);
    if ($angle0) $img0 = imagerotate($img0, $angle0, 0); 
    if ($mirror) $img0 = _mirrorImage($img0);
  }
  */

  $img = u\alterImg($img0, $w, $h, $x1, $y1, $cropW, $cropH, $angle);

  if ($mya->co) {
	  imagejpeg($img, DRUPAL_ROOT . $mya->photoFilename(TRUE), $quality);
  } else {
    ob_start();
    imagejpeg($img, NULL, $quality);
    $photo = ob_get_clean();
    $mya->update(compact('photo')); // do not combine this with line above
  }
  
  $mya->stepDone('photo'); // "if ($mya->photo)" fails above (dunno why)

  return w\photoRet(str_replace(DRUPAL_ROOT, $base_url, $filename), 'success');
}

/**
 * Upload a photo.
 * @later: maybe drag&drop (see http://hayageek.com/drag-and-drop-file-upload-jquery/)
 */
function formPhoto($form, &$sta, $arg = '') {
  global $base_url;
  $mya = r\acct();

  if ($arg) say($arg, 'err');
  $title = item(t('Photo ID Picture'));

  $desc = $mya->co
  ? tt('<p>1. Choose a picture to represent your company in rCredits promotions.</p>')
  : tt('<p>1. Choose a recent color picture of yourself for your photo-ID rCard.</p><p>It must show your full face clearly (no sunglasses), filling most of the frame (no shoulders).</p>');
  $body = t('I am attaching a photo of me for my rCard. &nbsp; - @fullName', ['@fullName' => $mya->fullName]);
  $desc .= t('<p>2. Adjust the position, size, and rotation, then click the <@b>green</b> crop-button to upload the image. Please feel free to <!a>email it to us</a>, if that is easier for you.', ['@b' => 'b style=color:lightgreen; background-color:darkgray;', '!a' => "a href=\"mailto:new@rCredits.org?subject=photo&body=$body\" target=\"_blank\""]);
  
  if ($mya->hasPhoto) {
    $oldPhotoFilename = BASE_URL . $mya->photoFilename();
    //$oldPhoto = 'data:image/jpeg;base64,' . base64_encode(file_get_contents($oldPhotoFilename));
    $oldPhoto = $mya->photo ? 'data:image/jpeg;base64,' . base64_encode($mya->photo) : $oldPhotoFilename;
    list ($background, $choose) = ["background-image:url($oldPhoto);", t('Choose a New Photo')];
    if (!$mya->co and $mya->ok) $desc .= '<p>' . tt('triggers new card', ['field' => t('photo')]) . '</p>';
    if ($mya->admin) $del = submit(t('Delete'));
  } else list ($background, $choose) = ['', t('Choose a Photo File')];

  $class = $mya->co ? ' class="co"' : '';
  $nextPage = $mya->ok ? ($mya->co ? 'settings/company' : 'settings/security') : 'status';

  $desc = item($desc);
  $newPhoto = item(<<<EOF
<style>#photoUp { $background }</style>
<div id="choosePhotoWrap"><div class="btn btn-md btn-primary" id="choosePhoto">$choose</div></div>
<div class="cropHeaderWrapper">
  <div id="photoUp"$class></div>
</div>

EOF
  );
  
  $sid = db\lookup('sid', 'sessions', 'uid=:uid', ['uid' => $mya->agent->id]);
  $failure = urlencode(t('That is not a usable image file. Try again.'));
  $maxSize = 4*1024*1024;
  $maxSizeDpy = '4MB';

  $js = <<<EOF
  var err = false;
  var croppicOptions = {
//      uploadUrl:'$base_url/settings/security/photo',
    cropUrl:'$base_url/settings/security/photo/upload',
    customUploadButtonId:'choosePhoto',
    cropData:{
      'uid':$mya->id,
      'sid':'$sid'
    },
    modal:false,
    processInline:true,
    loaderHtml:'<div class="loader bubblingG"><span id="bubblingG_1"></span><span id="bubblingG_2"></span><span id="bubblingG_3"></span></div> ',
    onBeforeImgUpload: function() {
      var photoUp = jQuery('#photoUp_imgUploadField').get(0);
      if (photoUp.files[0].size > $maxSize) {
        croppic.err = 'That photo file is too big. The maximum file size is $maxSizeDpy. Reduce the file size and try again (or choose a smaller file).';
      }    
    },
    onAfterImgUpload: function() { console.log('onAfterImgUpload') }, 
//    onImgDrag: function(){ console.log('onImgDrag') },
//    onImgZoom: function(){ console.log('onImgZoom') },
//    onBeforeImgCrop: function() { console.log('onBeforeImgCrop') },
    onAfterImgCrop:function(){
      jQuery('#photoUp').hide();
      window.location.replace('$base_url/$nextPage/gotPhoto');
    },
//    onReset:function(){ console.log('onReset') },
    onError:function(errormessage){
      jQuery.alert(errormessage, 'Error');
    }
  }	
  var croppic = new Croppic('photoUp', croppicOptions);
EOF;
  js('js/alert.js', 'file', 'footer');
  js('js/croppic.js', 'file', 'footer');
  js($js);
  css('css/croppic.css');

  $form = compact(u\ray('title desc newPhoto del'));
  return labeled($form);
}

function formPhoto_submit($form, &$sta) {
  global $base_url;

  $mya = r\acct();
  
  if ($mya->admin and op($sta) == 'del' and $mya->hasPhoto) {
    $pictureFilename = DRUPAL_ROOT . $mya->photoFilename(TRUE);
    $newFilename = str_replace('.jpg', '.X', $pictureFilename);
    if (file_exists($pictureFilename)) {
      rename($pictureFilename, $newFilename);
    } else imagejpeg(imagecreatefromstring($mya->photo), $newFilename, 100);
    $stepsDone = $mya->stepsDone;
    $stepsDone['photo'] = FALSE;
    $mya->update('photo stepsDone', '', $stepsDone);
    $mya->setBit(B_MEMBER, FALSE);
    $mya->setBit(B_OK, FALSE);
    return say(t('photo removed, step undone, person set to non-member (photo renamed to .X)'));
  }
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  global $base_url, $rUrl;
  if (!$mya = r\acct()) return ''; //['signin' => item('<a href="' . "$base_url/signin" . '">Sign in</a>')];

  $myid = @$mya->id; // may be a non-existent account if tests are run while looking at .ZZA, for example
  if (!$accounts = be\accountChoices()) return FALSE;
  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or cadmin
  $choices = $ltrs = [];
  foreach ($accounts as $uid) {
    $a = r\acct($uid);
    if (!$a and isDEV) continue; // ignore this if testing -- "current" account has been undermined is all
    $choice = $a->fullName;
    if ($mya->cAdmin) {
      if ($a->isCtty) { // show communities in a separate category
//        $class = $a->id < 0 ? 'cgc' : '';
        $ltrs['_'][$uid] = trim(str_replace('rCredits', '', "<small>$a->mainQid</small> $a->fullName"));
        continue; // omit regions from dropdown
      }
      $pos = substr($choice, 1, 2) == '. ' ? 3 : (strtoupper(substr($choice, 0, 4)) == 'THE ' ? 4 : 0);
      $ltr = strtoupper(substr($choice, $pos, 1));
      $ltrs[$ltr][$uid] = "<small>$a->mainQid</small> $choice";
      $choice = ($mya->admin ? $a->mainQid : substr($a->mainQid, -3, 2)) . ' ' . $choice;
    }
    $choices[$uid] = $choice;
  }

  if ($mya->cAdmin) {
    ksort($ltrs);
    $index = $details = '';
    foreach ($ltrs as $ltr => $accts) {
      $index .= "<a id=\"index-$ltr\">$ltr</a>";
      $detail = $detailDivs = '';
      $i = 0;
      foreach ($accts as $uid => $v) {
        $i++;
        $page = urlencode($_SERVER['REQUEST_URI']);
        $detail .= "<a href=\"$base_url/change-account?acct=$uid&page=$page\">$v</a>";
        if ($i == count($accts) or $i % 12 == 0) {
          $detailDivs .= "<div>$detail</div>\n";
          $detail = '';
        }
      }
      $details .= "<div id=\"index-$ltr-detail\" class=\"index-detail\">$detailDivs</div>\n";
    }
    $account = item("<div id=\"index\">$index</div>$details");
  } else {
    
//    if ($mya->can(B_MANAGE)) $settings = item(divButton('settings', 'Settings'));

    $page = $_SERVER['REQUEST_URI'];
  //  $onchange = "location.href='$base_url/change-account?acct=' + this.options[this.selectedIndex].value + '&page=$page'; jQuery('#saywait').show();";
  //  $account = selectFld('', '', dft($myid) + attrib(compact('onchange')), $choices);
    $links = '';
    foreach ($choices as $k => $v) {
      $one = $k == $mya->id ? $v : "<a href=\"change-account?acct=$k&page=$page\">$v</a>";
      $links .= "<li>$one</li>\n";
    }
    $account = item("<ul>$links</ul>");
  }
  
  $form = u\prefixKeys('acct_', compact(u\ray('account')));
  
  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op acct_account', $sta['input']));
  $op = op($sta);
  if ($op == 'Menu') return r\go('menu'); 
}

/**
 * Provide infrastructure for members to discuss and vote.
 */
function formDemocracy($form, &$sta, $args = '') {
  global $base_url;
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  
  extract(u\just('question do', $args));

  $mya = r\acct();
  $cttyA = $mya->cttyA;
  $title = w\item(t('Community: ') . $cttyA->fullName);

  if (!@$do or $do == 'options' or $do == 'results') {  
    list($questionList, $voting) = v\questionList($cttyA, @$question, @$do);

    $votebutton = !$voting ? '' : <<<EOF
    <div id='votenow'>
    <div><input type='button' onclick="$base_url/community/democracy/do=ballot" value='Vote Now!'></div>
    <div class='halfdown'>Polls close at midnight on $voting.</div>
    </div votenow>
EOF;
    $middle = $votebutton . $questionList;
  } elseif ($do == 'ballot') {
    $middle = v\ballot($extraFields, @$question);
//    debug(compact('middle', 'extraFields')); $middle = $extraFields = '';
  } elseif ($do == 'done') {
    $middle = v\done();
  } else $middle = t('Invalid democracy parameter(s)');
  
  $middle = w\item(@$middle);

  js('js/rvote/ballot.js', 'file', 'footer');
  css('css/rvote.css');
  
  return labeled(compact(u\ray('title middle')) + (@$extraFields ?: []));
}

function formDemocracy_submit($form, &$sta) {
  extract(u\just('question', $sta['input']));
  $mya = r\acct();
  
  list($quids, $qutypes) = v\getQuestions($mya->community); 
  v\processvote($sta['input'], $qutypes[$question]); // any errors are handled by javaScript (only the user suffers if no JS)
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
  if (w\op($sta) == 'submit') $qnum++; else $qnum--;

  if ($qnum < 0) return say(t('Previous question has been deleted.')); // stay on current question
  $args = $qnum < count($quids) ? 'do=ballot&question=' . $quids[$qnum] : 'do=done';
  return r\go('community/democracy/' . $args); 
}

/**
 * Change the current account.
 */
function changeWho() {
  extract(u\just('acct page', $_GET));

  if (@$acct == 'signout') return signout();
  setGlobals();
  $mya = r\acct(); // get old current account
  if (!@$mya->admin and !r\acct($acct, $mya->agentId)->can()) hack('accts');
  $mya = r\acct($acct); // get new current account

  svar('myid', $mya->id); // this gets checked ONLY in setGlobals()
  if (isDEV) $page = str_replace(DEV_ROOT, '', @$page);
  r\go(@$page);
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company = '') {
  $uid = db\lookup('uid', 'users', 'name=:company', compact('company'));
  $a = r\acct($uid);
  if (!$uid or !$a->co) return say('no such company', 'ERR');

  $pic = item($a->photoHtml());
  $state = r\realState($a->state);
  $country = r\realCountry($a->country);
  $siteList = '';
  if ($website = $a->website) foreach (u\ray($website) as $one) $siteList .= "<a href='http://$one' target='_blank'>$one</a><br>";
    
  $title = item($a->fullName);
  $phone = 'Phone: ' . u\fmtPhone($a->phone, '+');
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = db\q($sql, compact('uid'))->fetchAllKeyed();
  $cats = item(empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats)));

  $contact = item("$a->address<br>
    $a->city, $state $a->postalCode $country<br><br>
    $phone<br>
    $siteList");
  $desc = item($a->description);

  return labeled(compact(u\ray('pic title contact desc cats')));
}

/**
 * Say the site is down
 */
function formSystemDown($form, &$sta) {return array('zot' => item(tt('system down')));}

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 */
function formTx($form, &$sta, $args = '') {
//  if ($confirm = sureForm($sta)) return $confirm;
  extract(u\just('who amount goods purpose', $args));
  $mya = r\acct();

  if (!$mya->can(B_SELL)) return r\go('summary');

  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path())); // pay, charge, etc.
  $role = in_array($type, u\ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, u\ray('fine grant loan invest'));
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($type == 'charge' ? @$selling[0] : ($type == 'pay' ? '' : $type));
  
  focusOn(@$who ? 'amount' : 'who');

  // set up form fields
  $title = item(ucwords($type));
  if ($mya->co and $type == 'pay') $subtext = item('<small>[<a href="' . BASE_URL . '/pay/payroll">Upload Payroll</a>]</small>');
  $whoLabel = ($type == 'loan' ? t('Lend to') : ucwords($type)) . t(' whom:');
  $question = ($type == 'loan' ? t('Lend') : ucwords($type)) . t(' @amount to @name?'); 
  $who = textFld($whoLabel, [t('name, id, email, or phone')], dft(@$who));

  $amount = textFld('Amount:', [t('amount to ') . $type], dft(@$amount));
  $goods = $incentive ? hidFld(FALSE) : radiosFld(t('For:'), '', dft(@$goods), u\ray(R_WHYS)); // required() fails here
  $purpose = textFld(' ', [t('purpose or description of goods and services')], required(@$purpose));
// (keep this for a while)  $calc = item(u\calculator('amount'));
  $scanClass = 'not-scanned';
  $isGift = hidfld(FALSE);
  $$type = submit(ucwords($type));

  $onSubmit = 'return ' . w\whoFldSubmit('who', $question, 'this.amount.value', $type == 'pay');
  
  $form = labeled(compact(u\ray("title subtext who amount goods purpose isGift calc $type")), $onSubmit);
  $form['#attributes']['class'][] = $scanClass;
  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
//  if (confirming_v($sta)) return;
//  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // not needed
//  if ($confirm = transfer(op($sta), $sta['input'], FALSE)) confirm($confirm, $sta); // op who amount goods what
  transfer(op($sta), $sta['input'], FALSE);
}

function formTx_submit($form, &$sta) {
//  global $fieldErr;
//  if (confirming_s($sta)) return;
  w\transfer($op = op($sta), $sta['input'], TRUE); // op who amount goods what
//  if (!$fieldErr) return r\go($op); // this is especially important when skipping the confirmation (scanning)
}

/**
 * Upload payroll spreadsheet to create partial payments in rCredits.
 */
function formPayroll($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $title = item(t('Payroll Upload'));
  $subtext = item(t(<<<EOF
  <ol>
  <li><b>Ask your participating employees</b> how many rCredits they can receive each pay period, without having to cash them out (how many can they easily spend in two weeks, for example). Put those amounts in a two-column spreadsheet with name* in the first column and amount in the second. Save the spreadsheet as a CSV file.</li>
  <li><b>In your accounting software</b>, create an after-tax payroll deduction called "rCredits" (you need do this only once). Set that deduction for each employee to the amount of rCredits they asked to be paid. Run payroll, to pay the US Dollar part of their wages.</li>
  <li><b>Upload here</b> the CSV file you created in step one, to automatically pay the amount of rCredits each employee requested.</li>
  </ol>
  <p><b>* NOTE:</b> Names in your spreadsheet must match names on the <@a>Relations page</a>. And make sure the amounts on the spreadsheet match the deduction amounts in your accounting software!!!</p>
EOF
  , ['@a' => "a href=$base_url/settings/relations"]));
  $payroll = fld('file', '', '', attrib(u\ray('enctype', "multipart/form-data")));
  if ($end = $mya->payrollEnd) { // get date(s) from previous payroll
    $start = strtotime('tomorrow', $end); // start the day after previous period ends
    $end = $start + ($end - $mya->payrollStart); // same period length as last time
  } else {
    $end = $fri = strtotime('next Friday', strtotime('-3 days'));
    $start = $fri - 13 * DAY_SECS + HOUR_SECS; // + HOUR_SECS is in case of daylight time
  }
  $startDate = textFld(t('Starting:'), [t('Pay period start')], required(u\fmtDate($start, TRUE)));
  $endDate = textFld(t('Ending:'), [t('Pay period end')], required(u\fmtDate($end, TRUE)));
  $submit = submit('Upload and Pay');

  return labeled(compact(u\ray('title subtext startDate endDate payroll submit')));
}

function formPayroll_validate($form, &$sta) {
  extract(u\just('startDate endDate', $sta['input']));
  list ($min, $max) = [strtotime('-2 months'), strtotime('+7 days')];
  foreach (['startDate', 'endDate'] as $k) if ($err = u\badDate($$k, $min, $max)) return sayFieldErr($err, $k);
  if (!@$_FILES['files']['name']['payroll']) return say('upload required', 'payroll');
  $validators = array(
    'file_validate_extensions' => array('csv'),
    'file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000),
  );
  if (!$file = \file_save_upload('payroll', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'payroll');
  if ($file->filemime != 'text/csv') return say('bad file type', 'payroll');
  $sta['input']['filename'] = $filePath = file_directory_temp() . '/' . $file->filename;
}

define('R_NOREWARD', '<noreward>'); // "no rewards" flag in description

function formPayroll_submit($form, &$sta) {
  extract(u\just('startDate endDate filename', $sta['input']));
  $mya = r\acct();
  file_put_contents($filename, strtr(file_get_contents($filename), ["\r\n" => "\n", "\r" => "\n"]));
  $fp = fopen($filename, "r"); 
  while ($row = fgetcsv($fp)) $rows[] = $row; // get all rows, so we can look ahead
  fclose($fp);
  unlink($filename);
  $desc0 = t('labor: ') . $startDate . t(' to ') . $endDate;
  $map = ['<startdate>' => $startDate, '<enddate>' => $endDate, R_NOREWARD => ''];
  $total = 0;

  foreach ($rows as $i => $row) {
    $subs = ['line' => $i + 1]; // for error messages
    if (count($row) < 2) say('empty row');
    @list ($employee, $amt, $desc) = $row; // desc is optional
    if (!$employee or u\starts(strtoupper($employee), t('TOTAL'))) break; // should never happen unless user has a total line
    if ($err = u\badAmount($amt, '>0')) return say('bad payroll csv|' . $err, $subs, 'payroll');
    if (!@$employee) return say('bad payroll csv|' . t('Missing employee name.'), $subs, 'payroll');
    if (!$uid = r\worksForUs($employee, NULL, TRUE)) return say('bad payroll employee', $subs + compact('employee'), 'payroll');
    $goods = strpos($desc, R_NOREWARD) !== FALSE ? FOR_NONGOODS : FOR_GOODS;
    $toPay[] = [$uid, $amt, $desc ? strtr($desc, $map) : $desc0, $goods];
    $total += $amt;
    $employee = $amt = '';
  }      

  if (!@$toPay) return say('empty payroll', 'payroll');
  if ($shortfall = $mya->shortfall(FOR_GOODS, $total)) return say('short to', u\ray('short', u\fmtAmt($shortfall)), 'payroll');
  
  foreach ($toPay as $one) {
    list ($uid, $amt, $desc, $goods) = $one;
    list ($msg, $subs) = be\transfer('payment', $mya, r\acct($uid), $amt, $goods, $desc);
    say($msg, $subs);
    if (!@$subs['success']) $err = TRUE;
  }
  
  if (!@$err) {
    $mya->update('payrollStart payrollEnd', u\s2t($startDate), u\s2t($endDate));
    return r\go('history', tt('payroll done', u\ray('total count', u\fmtAmt($total), count($toPay))));
  }
}

function formMenu($form, &$sta) {return array('title' => item('Menu'));}

/**
 * Present an empty slate for status and error messages.
 * @param mixed $form: usually unused (but required by Drupal); if string, output it.
 * @param bool $doSay: TRUE if called from the doSay function.
 */
function formEmpty($form, &$sta = '', $doSay = FALSE) {
  if ($form and !is_array($form)) w\say($form);
  $title = item(t('Miscellaneous')); // this is suppressed with CSS
  if ($doSay) $subtext = item(t('To protect your account, <span class="loud">no sign-in was required</span> for this action. To sign in safely, always go to rCredits.org.'));
/// FAILS :(  if ($doSay == 'test') $testMessages = item(print_r(\drupal_get_messages(), 1));
  return labeled(compact(u\ray('title testMessages subtext')));
}

/**
 * Pay with rCredits, arriving from some other site or from rDo().
 * @param string $doCode: the encrypted code from rDo, if any
 */
function formBuy($form, &$sta, $doCode = '') {
  $fields = 'item amount company code verb return_to cust_name ship_zip cust_email';
  extract(u\just($fields, $doCode ? r\doCode($doCode) : (@$sta['input'] ?: $_GET)));
  
  foreach (u\ray($fields) as $field) {
    if (!@$$field) return r\go('', 'missing field', compact('field'), 'zot'); // handle hackers gracefully
  }
  if ($err = u\badAmount($amount, '>0')) return r\go('', $err, 'zot');
  if (!u\isQid(@$company) or !($co = r\acct($company)) or !$co->co) return r\go('', 'bad account id', 'zot');
//  if ($code != @$co->data['cardCode']) return r\go('', 'bad account code', 'zot');
  $cust_email = strtolower($cust_email);
  if (($a = r\acct()) ? !u\similarEmail($a->email, $cust_email) : (!$a = r\acct($cust_email))) return r\go('', u\tt('bad buy info', u\ray('thing', t('email'))), 'zot');
//  if (!in_array(strtolower($cust_name), array(strtolower($a->fullName), strtolower($a->legalName)))) return r\go('', u\tt('bad buy info', u\ray('thing', t('name'))), 'zot');

  if (!\valid_url($return_to, TRUE)) return r\go('', 'bad url', 'zot');

  $verbs = u\ray(t('Donate, Donate, Pay'));
  $verb = in_array(ucwords(@$verb), $verbs) ? ucwords($verb) : t('Pay');
  $disabled = array('disabled' => 1);
  
  $title = item(t('Confirm Payment'));
  $subtext = item(t('To confirm payment, enter your account ID and PIN below.'));
  $amountx = textFld("$verb $", '', disabled(number_format($amount, 2)));
  $companyx = textFld(t('To:'), '', disabled($co->fullName));
  $forx = textFld(t('For:'), '', disabled($item));
  $account = textFld(t('Account ID:'), '', required());
  $pin = textFld(t('PIN:'), '', required());
  if (substr($ship_zip, 0, 5) != substr($a->postalCode, 0, 3)) $answer = textFld(t('Answer:'), $a->question, required());

  $for = $item; // note that disabled fields do not get passed
  $hid = hidFld(serialize(compact(u\ray('amount for return_to co a'))));
  $submit = submit($verb);

  return labeled(compact(u\ray('title subtext amountx companyx forx account pin answer hid submit')));
}

function formBuy_validate($form, &$sta) {
  extract(u\just('account pin answer hid', $sta['input']));
  extract(u\just('a', unserialize($hid)));

  if (strtoupper($account) != $a->mainQid) return r\go('', 'not your account', 'zot');
  //if ($pin != $a->pin) return r\go('', 'wrong pin', 'zot');
  //if ($answer != $a->answer) return r\go('', 'wrong answer', 'zot');
}

function formBuy_submit($form, &$sta) {
  extract(u\just('hid', $sta['input']));
  extract(u\just('amount for co a return_to', unserialize($hid)));
  
  list ($index, $subs) = be\transfer('payment', $a, $co, $amount, FOR_GOODS, $for);
  if (!@$subs['success']) return r\go('', tt($index, $subs), 'zot');

  return r\go('', "Success! <a href='$my_return_to'>Click here</a> to return to the $company_name website.");
}

/**
 * Pay or deny an invoice, possibly by clicking a link without signing in (from rDo).
 * If no user is signed in, $args must include the do code (to prevent mischief)
 * @param mixed $args: array of paramaters (from rDo) or invoice number (if signed in):
 *   int nvid: invoice number
 *   bool toMe: is the invoice for me to pay (or deny)? (FALSE if it is FROM me)
 *   string ret: URL to return to
 */
function formHandleInvoice($form, &$sta, $args = '') {
  $mya = r\acct();
  extract(u\just('nvid ret code toMe', $args));
  $ret = @$ret ? @u\deurlify($ret) : 'empty/1';
  
  if ($err = r\badUnpaidInv(@$nvid, $inv)) return r\go($ret, $err, 'ERR');
  extract(u\just('nvid amount payer payee purpose created data', $inv));
  if ($mya) {
    if ($payer != $mya->id and $payee != $mya->id) return hack(t('bad person in invoice payment'));
  } elseif (!$info = r\doCode($code)) return hack(t('fake doCode in handleInvoice'));

  $subs = u\ray('amount what', u\fmtAmt($amount), $purpose);
  
  if (@$toMe) {
    $title = item(t('Confirm Payment'));
    $subs += u\ray('who', r\acct($payee)->fullName);
    $subtext = item(tt('Pay @amount to @who for @what?', $subs));
    $pay = submit(t('Pay'));

    $whyNot = textFld(t('Why not:'));
    $deny = submit(t('Deny'));
    $denySet = fieldSet('denySet', compact('whyNot', 'deny'), BARE);
    $toMe = hidFld(TRUE);
  } else { // confirming delete by originator
    $title = item(t('Confirm Deletion'));
    $subs += u\ray('who', r\acct($payer)->fullName);
    $subtext = item(tt('Delete invoice charging @who @amount for @what?', $subs));
    $delete = submit(t('Delete'));
    $cancel = submit(t('Cancel'));
    $delSet = fieldSet('delSet', compact('delete', 'cancel'), BARE); // separate it from subtext
  }
  
  $hid = w\hidFlds(compact('ret') + $inv);
  $form['#validate'] = array('rCredits\\Web\\formHandleInvoice_validate');
  $form['#submit'] = array('rCredits\\Web\\formHandleInvoice_submit');
  return labeled(compact(u\ray('title subtext pay denySet delSet toMe hid')) + $form);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(u\just('whyNot toMe', $sta['input']));
  if (@$toMe and op($sta) == 'deny' and !$whyNot) return say('say why not', 'whyNot');
}

function formHandleInvoice_submit($form, &$sta) {
  extract(u\just('hid whyNot toMe', $sta['input']));
  extract(u\just('ret nvid amount payer payee purpose created data', w\hidFlds($hid)));

  $subs = u\ray('amount payerName payeeName created purpose reason', u\fmtAmt($amount), r\acct($payer)->fullName, r\acct($payee)->fullName, u\fmtDate($created), $purpose, @$whyNot);

  $op = op($sta);
  $ok = TRUE;
  if ($op == 'pay' and !$whyNot) {
    $ok = be\payInvoice($nvid, $msg);
  } elseif ($op == 'delete') {
    db\q('DELETE FROM r_invoices WHERE nvid=:nvid', compact('nvid'));
    r\notify($payer, 'invoice withdrawn', $subs);
    $msg = 'invoice delete done';
  } elseif ($op == 'deny' or @$whyNot) {
    $data = serialize(compact('whyNot') + (@$data ? unserialize($data) : []));
    db\q('UPDATE r_invoices SET status=:TX_DENIED, data=:data WHERE nvid=:nvid', compact('nvid', 'data'));
    r\notify($payee, 'invoice denied', $subs);
    $msg = 'denial done';
  } else $msg = ''; // default: canceled

//  if ($toMe and !r\acct()) return r\go('do/code=' . ($msg ?: 'nothing done', $ok ? 'status' : 'error');
  return r\go($ret, $msg ?: 'nothing done', $ok ? '' : 'ERR');
}

/**
 * Exchange USD for rCredits or vice-versa (the Bank tab)
 */
function formGet($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $jid = $mya->jid ?: 0;
  extract(u\just('cancel', $args));
  if ($txid = @$cancel) {
    db\q('DELETE FROM r_usd WHERE txid=:txid', compact('txid'));
    say('transfer canceled', compact('txid'));
    r\tellStaff("canceled transfer #$txid");
  }
  $title = item(t('Bank Transfer'));
  $subtext = item(t('<p>Transfer funds to or from your connected bank account. Transfers may take 3-5 business days.</p><p>To refill your rCredits account automatically (or not), as needed, adjust your "minimum" setting on the <@a>Preferences</a> page.</p>', ['@a' => "a href=$base_url/settings/preferences"]) . tt('note|ach warning'));

//  $minimum = textFld(t('Minimum:'), tt('prompt min'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->minimum : R_MINR_DEFAULT, 's')));
  $toSpend = u\fmtAmt($mya->avail(FOR_GOODS));
  $waiting0 = $mya->waitingToBank();

  if (!db\exists('r_usd', 'payer IN (:myid,:jid) AND amount>0 AND created>:dayAgo', u\ray('myid jid dayAgo', $myid, $jid, strtotime('-1 day')))) { // limit to one withdrawal request per 24 hours
    $avail = $mya->avail(FOR_USD); // drawing from connected accounts not allowed
    foreach (u\ray(R_EXCHANGE_LIMITS) as $days => $limit) {
      $sofar = $days == 1 ? 0 : db\lookup('SUM(amount)', 'r_usd', 'payer=:myid AND amount>0 AND created>:date', u\ray('myid date', $myid, strtotime("-$days days")));
      $avail = min($avail, $limit - $sofar);
    }
//    $avail = db\lookup('SUM(LEAST(usd, :DW_FEE_THRESHOLD))', 'users', 'community=:ctty and uid<>:myid', u\ray('ctty myid', $mya->community, $myid)); // remove this one-day limit if people complain?
//    $avail = min($myAvail, $mya->usd + $avail); // what i have or what USD is available, whichever is less
  } else $avail = 0;

  $toCash = u\fmtAmt(max(0, $avail));
  if ($waiting = u\fmtAmt(abs($waiting0))) {
    $where = 'payer IN (:myid, :jid) AND completed=0 AND deposit=0 ORDER BY txid DESC';
    if ($rec = db\lookup('amount, txid', 'r_usd', $where, compact('myid', 'jid'))) {
      extract($rec);
      $text = t('Cancel') . (($amount == $waiting0) ? '' : (' ' . u\fmtAmt(abs($amount)) . t(' request')));
      $link = " <a class=\"buttino\" href=\"get/cancel=$txid\">$text</a>";
    }
  }
  $way = $waiting0 < 0 ? 'from' : 'to';
  $waiting = item($waiting == '$0' ? t('You have no pending transfer requests.') : (tt('You have total pending transfer requests of @waiting @way your bank account.', compact('waiting', 'way')) . @$link), t('Pending:')); // amount already requested (if any)
  $avail = hidFld($avail);
  $balance = item(tt('balances', compact('toSpend', 'toCash')), t('Available:'));
  $amount = textFld(t('Amount $'), t('Transfer how much?') . ($mya->admin ? t(' (no limit for admin)') : ''));
  $get = submit(BARE . t('From Bank'));
//  $fromBank = fld('checkbox', t('Automate:'), t('(recommended) Refill my rCredits account from my bank account automatically, as needed.'), dft(!$mya->prefsSet ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = textFld(t('Maximum:'), tt('prompt max'), dft(u\fmtAmt(!$mya->prefsSet ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = textFld(t('Excess'), '', dft(u\fmtAmt($mya->overMax(), 's')));
//  $toBank = fld('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft(!$mya->prefsSet ? $mya->can(B_TO_BANK) : TRUE));
  $put = submit(BARE . t('To Bank'));
  $actions = fieldSet('actions', compact('get', 'put'));

  return labeled(compact(u\ray('title subtext waiting balance amount actions avail')));
}
  
function formGet_validate($form, &$sta) {
  $mya = r\acct();
  if ($err = amtErr('amount', $sta)) return say($err, 'amount');
  extract(u\just('amount avail', $sta['input']));
  if (!$mya->cAdmin and $amount < R_ACHMIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > ($mya->cAdmin ? round($mya->r - $mya->rewards, 2) : $avail)) {
    $max = u\fmtAmt($avail);
    return say('short put|short cash help', compact('max'), 'amount');
  }
  if (round($mya->j_r - $amount, 2) < $mya->j_minimum) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  $mya = r\acct();
//  $us = new r\usd($mya = r\acct());
  extract(u\just('amount', $sta['input']));

  if (op($sta) == 'put') {
    $action = 'deposit to';
//    if (!$mya->cacheOk()) return r\go('', 'cannot bank', u\ray('action error', $action, t('data integrity issue')));
  } else list ($action, $amount) = array('draw from', -$amount);
  
//  if ($us->bank($amount, $error) or isDEV) { // ignore lack of verified bank connection, if testing
  $checkNum = $mya->bank($amount, $completed);
  if ($amount < 0) {
    $extra = '|bank tx number' . ($completed ? '|available now' : '');
  } else $extra = '';
  $amount = u\fmtAmt(abs(@$amount));
  say('banked' . $extra, $subs = compact(u\ray('action amount checkNum')));
  r\tellStaff('banked' . $extra, $subs);
//  } else return say('cannot bank', compact('action', 'error'), 'amount'); // delete this when we ditch Dwolla
}

/**
 * Prepare to open a company account.
 * This form creates an invite and redirects to signup, using that code.
 * @see also: formAgent
 */
function formSignupCo($form, &$sta) {
  $a = r\agent();
  if (!$a->cAdmin and !$a->can(B_MEMBER)) return r\go('', 'members only', 'ERR');
  
  //t('an additional personal account that I will manage'), 
  $title = item('Open a Company Account');
  $relations = array(t('a company and I own at least 10% of it'), t('a company or organization whose accounts I manage,<br>but I am not a 10%-or-more owner'), t('neither of the above') . ($a->cAdmin ? '' : tt(' (send an <a href="@BASE_URL/community/invite">invitation</a> instead!)')));
  $relationPrompt = item(t('What is the new account for?'));
  $relation = radiosFld(t(''), '', dft(2), $relations);
//  $flows = array(t('neither way (recommended in most cases)'), t('Your personal account can cover the new account\'s debts and overdrafts.'), t('The new account can cover your personal account\'s debts and overdrafts.'), t('both ways'));
//  $flowPrompt = item(t('Which way can the credit flow, when you need it to?'));
//  $flow = radiosFld(t(''), '', required(0), $flows);
  $flow = hidFld(0);
  $submit = submit(t('Open Account'));
  return labeled(compact(u\ray('title relationPrompt relation flowPrompt flow submit')));
}

function formSignupCo_validate($form, &$sta) { 
  extract(u\just('relation flow', $sta['input']));
  if ($relation == 2) say('insufficient relation', 'relation');
}

function formSignupCo_submit($form, &$sta) {
  global $signupArgs; // for testing
  $a = r\agent();
//  if ($a->id == 1) $a = r\acct(R_ADMIN_QID); // use admin's personal account

  extract(u\just('relation flow', $sta['input']));

  $employeeOk = $relation < 2; // let signupCo info override this
  $personal = ($relation > 1);
  if (!$personal) {
    if ($a->cAdmin) $a = r\acct(); // let admins open company accounts on behalf of others
    extract(u\just('company companyPhone isOwner employeeOk', $a->signupCo)); // predict co name
    if (!findCompany(@$company, @$companyPhone)) list ($fullName, $phone) = [@$company, @$companyPhone];
  }
  $isOwner = $relation == 0; // don't let signupCo info influence this

  $helper = $a->mainQid;
  $code = r\invite($a->email, $a->id);
  $flow += 0;

  $signupArgs = compact(u\ray('code personal isOwner employeeOk flow helper fullName phone'));
  $args = http_build_query($signupArgs);
///  debug($args); return;
  if ($personal) \session_destroy();
  r\go('signup/' . $args);
}

/**
 * Modify the user registration form.
 * @param string $args: if not empty, a urlencoded assoc (from an email invitation or the "Create Another Account" button):
 *   code: the invitation code 
 *   personal: opening a personal account (default TRUE)
 *   fullName: the likely name of the account to create
 *   phone: the account's likely phone number
 *   isOwner: is the inviter an owner (these last four params are processed by suCreatorRelation())
 *   employeeOk: is the inviter an employee
 *   flow: which way credit can flow (be drawn)
 *   helper: uid of account creating this one -- non null (same as $inviter) means created with formSignupCo
 */
function formSignup($form, &$sta, $args = '') {
  if (!$args) $args = basename(t\SERVER('REQUEST_URI')); // because menu_execute_active_handler fails
  extract(u\just('code personal fullName phone helper', $args));
  extract(u\just('hidCountry hidState', $sta['input']));

  if (@$personal and r\signedIn()) return formEmpty('must be signed out');
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost
  u\setDft($personal, TRUE);

  $title = item($personal ? t('Open a Personal rCredits Account') : t('Open a Company Account'));
  
  if (!@$helper) { // no need to clarify if user clicked the "Open a Company Account" button
    $clarify = $personal ? t('To open a company account, use the "Open a Company Account" button after your personal account is approved.') : t('To open another personal account, send an Invitation (click the Invite tab).');
    $onclick = $onmouseover = 
    $forother = $personal ? t('open a company account instead') : t('open a personal account instead');
    $forother = item("[<a class=\"nospin\">$forother</a>]");
    $js = <<<EOF
  var clarify = function() {alert('$clarify');};
  jQuery('#edit-forother a').click(clarify);
  //jQuery('#edit-forother a').mouseover(clarify);
EOF;
    js($js, 'inline', 'footer');
  }
  
  if ($i = strpos(@$code, '(') or @$code == 'w') { // self-invitation with friend's name and zipcode
    $inviterName = str_replace(')', '', substr($code, $i + 1));
    $notes = hidFld("knows: $code");
    $code = r\acct(1)->iCardCode(IBY_SELF); // inviter is admin for now; invitation() will set $iCode=IBY_SELF
  }
    
  $inviteOk = ((isDEV and @$code == 'w') or w\invitation(@$code, '', $inviter, $iCode, $err));
  if (@$inviter and $iCode != IBY_SELF) {
    $ia = r\acct($inviter);
    $inviterEmail = htmlspecialchars($ia->email);
    $inviterName = htmlspecialchars($ia->fullName);
//      $a = "a target=\"_blank\" href=\"mailto:$inviterEmail?subject=invite me again?&body=Hi $inviterName, the rCredits invitation you sent me expired. :( Please send me another one. :)\"";
  }
  if (!$inviteOk) {
    w\say($err, compact('inviterName'), 'err');
    $subtext = item(t('To sign up for rCredits, you must be invited or approved by someone who is already a member. Ask someone you know to invite you OR mention your friend when you sign up at <@a>rCredits.org</a>!', ['@a' => 'a href=http://rcredits.org/signup.html']));
    return labeled(compact(u\ray('title subtext')));
  } 

//  If you want to use this address <b>for more than one account</b>, include a tag on the local part starting with "+" (for example, myemail<b>+whatever</b>@example.com).');

  if ($iCode != IBY_FORM) say(tt('NOTE: @inviterName will need to affirm trust in you OR your first transaction will need to be a purchase from @inviterName with your rCard.', compact('inviterName')));
  $subtext = item(t(<<<EOF
Opening an rCredits account is very much like opening an online bank account. It takes about that much time. After this signup page there are <b class="loud">@count more short steps</b> we will guide you through. Opening your account is the hard part &mdash; after that it gets much easier. If you need help, please don't hesitate to ask (<a href="mailto:info@rCredits.org?subject=signup help">click here to send us an email</a>).
EOF
  , ['@count' => $personal ? t('six') : t('seven')]));

  if ($personal) {
    $acctType = hidFld(R_PERSONAL);
    $pin = passFld(t('PIN:'), [t('Choose a PIN (any 4 digits)'), t('Your security code for purchases.')]); // first time logging in, get PIN
    $calling = textFld(t('Life Calling:'), [t('Your calling in life'), t('One of the main purposes of rCredits is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do, to earn rCredits (that you\'re not doing yet)? What might you have to offer?')]);
    $companyOptions = array(
      'isOwner' => t('owner'),
      'employeeOk' => t('employee'),
      'contractor' => t('other worker (contractor, supplier, etc.)'),
    );
    $copts = boxOpt('companyOptions', $companyOptions, $sta);

    $companyInfo = array(
      'company_text' => item(t('Are you an owner and/or employee of a local company, or of any rCredits member company? Otherwise leave this part blank.')),
      'company' => textFld(t('Company: '), [t('Your company')], autocomplete('company')),
      'companyPhon' => textFld(t('Company Ph<span>&nbsp;</span>one:'), [t('Company phone number')]), // Google Chrome bug require obfuscation of "phone"
      'companyOptions' => boxesFld('', '', @$copts, $companyOptions), 
    );
    $companyInfo = fieldSet('companyInfo', $companyInfo, BARE);
  } else { // company
    $acctType = w\acctTypeField();
  }

//  $buysAts = db\q('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
//  $ats = boxOpt('ats', $buysAts, $sta);
//  $buysAt = boxesFld(t('<br>Over 60 Greenfield businesses have already signed up. Which of these <b>Not-Yet-Member Businesses</b> do you frequent?'), '', @$ats, $buysAts);

  $submit = submit(t('Sign up'));
  $args = hidFld(@$args);
  $confirmed = hidFld(@$iCode == IBY_FORM); // iff inviter used form (or opening a company account), invitation is already confirmed
  $iCode = hidFld(@$iCode + 0);
  $helper = hidFld(@$helper ? r\acct($helper)->id : @$inviter);
  
  $form = compact(u\ray('title forother subtext acctType'))
        + w\nameAndEmail($personal ? t('Full name') : t('Company'), @$fullName)
        + w\contactFields(u\ray('phone country state', @$phone, @$hidCountry, @$hidState), FALSE) + w\ssnFields($personal)
        + compact(u\ray('pin calling companyInfo buysAt submit args confirmed helper iCode notes'));
//  keep_values($form);
  return labeled($form);
}

function formSignup_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = []; // for testing (keep Drupal from griping)
  extract(u\just('confirmed acctType email phone country tenure pin company companyPhon companyOptions fullName legalName postalCode address city state postalAddr', $sta['input']));
//  if (!$inviteOk) return r\go('signup'); // in case user resubmits
  //and @$code != bin2hex(R_WORD)

  \variable_set('signupData', u\ray('time phone state companyOptions', time(), @$phone, @$state, @$companyOptions));
  if (!emailOkay($email, $acctType != R_PERSONAL)) return;
  if (!phoneOk($phone, $country)) return;
  if ($err = u\badZip($postalCode)) say($err, 'postalCode');
  if (!@$state) return say('bad state', 'state');
  if (!formBasic_validate($form, $sta, $acctType == R_PERSONAL)) return;
  if ($err = u\badAmount($tenure, '>=0')) return say(t('How-long: ') . tt($err), 'tenure');
  if (@$pin === '') return say('pin required', 'pin');
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  
  if (@$companyOptions) extract(u\just('isOwner employeeOk contractor', $companyOptions));
  $gotrelation = (@$employeeOk or @$isOwner or @$contractor);
  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (u\ray('company companyPhon') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $signupCo = compact(u\ray('company companyPhone employeeOk isOwner contractor'));
  }
  
  if (!@$legalName) $legalName = $fullName;
  foreach (u\ray('fullName legalName address city postalAddr') as $field) {
    if ($err = u\badName(@$$field)) return say($err, compact('field'), $field); // @ for testing
    $$field = u\normalizeCase($$field);
  }
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) return say('po in location', 'address');

  $flags = $acctType == R_PERSONAL ? 0 : u\bit(B_CO);
  if (@$confirmed) $flags |= u\bit(B_CONFIRMED); // companies are always pre-confirmed, because the company account is opened by a member
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  u\preray(compact(u\ray('fullName legalName address city postalAddr email flags signupCo administer_users status')), $sta['input']);
}

function formSignup_submit($form, &$sta) {
  $dbFields = 'fullName legalName email flags phone federalId dob pin country postalCode address city state postalAddr oneTimePass helper iCode notes wants calling signupCo tenure';
  extract(u\just('confirmed acctType args company ats owns ' . $dbFields, $sta['input']));
//  if (op($sta) == 'continue') return r\go('signup/' . urldecode(u\urlArgs()) . '&dwok=1'); // got agreement, so continue
//  if (!emailOkay($email)) return; // I don't know how people create duplicate accounts, but maybe this will prevent it. Nope, apparently not. (ws 1/27/2014)

  if (u\test() and !@$args) $args = basename(t\SERVER('REQUEST_URI'));

  u\EXPECT($args, 'no args');
  extract(u\just('code', $args));
  if (isDEV) $helper = $helper + 0; // don't let lack of helper stop us

  $DBTX = \db_transaction();
  $wants = wants(@$ats);
//  $verifyBy = verifyBy(@$verifyBy);
/*  if (u\test()) {
    $where = u\parseZip($postalCode) ?: u\ray('state', 'MA'); // get $state/$city (dft to MA)
    $postalAddr = "$address, $city, $state $postalCode";
  } else */
  $where = [];

  // @todo: try this 2 or 3 times (in case someone else was trying to register the same name
  $name = r\uniqueName($fullName);
  if ($acctType == R_PERSONAL) {
    $oneTimePass = oneTimePass($name);
    $pw1 = $oneTimePass['pass'];
  }
  
  $info = compact(u\ray('name ' . $dbFields)) + $where + acctType($acctType);
  if (!$mya = new r\acct($info)) return say('cannot create account', 'zot');
  
  $myid = $mya->id;
  u\EXPECT($myid > 0, 'registration failure');
  setGlobals($myid); // some rcredits functions need to know who we are
  
  if (@$iCode < 0) w\invitation($code, $myid); // mark the invitation used
  $email = u\cryptN($email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!@$owns) $mya->setRisk('rents');
  
  if (@$signupCo) {
    suCompanyRelation($myid, $signupCo);
    $mya->update(compact('signupCo'));
  }

  if ($h = $mya->helperA and @$args and !$h->cAdmin) suCreatorRelation($helper, $args, $myid);
  
  // report to staff and user
  $quid = $mya->mainQid;
  tellStaffNewMember($info + compact('acctType', 'quid'));

  if ($acctType == R_PERSONAL) {
    say('your account is ready');
    $region = strtolower(R_SERVER_ID);
    r\notify($myid, 'welcome', u\ray('name quid code', $name, $quid, $pw1), TRUE); 
    if (!@$confirmed) {
      u\EXPECT($h, 'unconfirmed with no helper');
      if ($helper != 1) { // sysadmin helper means the potential inviter is named in the notes field
        $a1 = $h->makeDo('confirmInvite', $myid); // link to confirm invitedness
        r\message($helper, 'confirm invite', compact('a1', 'fullName')); // ask inviter to confirm
      }
      say('must be confirmed', ['inviterName' => $h->fullName]);
    }
    $goto = 'empty';
  } else { // non-personal company (no independent login)
    say('company is ready', compact('quid'));
    //if (@$helper) $quid = r\quid($helper); // "if" for dev faked code HUH?
    if (!@$h) hack('company no helper');
    svar('myid', $mya->id); // this is necessary for company creation, to keep the company selected
    $goto = 'status';
  }
  $mya->stepDone('contact');
  
  unset($DBTX); // commit
  return r\go($goto);
}

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  $mya = r\acct();
  focusOn('fullname');
  $nameDesc = tt('usename desc');
  if ($mya->ok) $nameDesc .= '<br>' . ($mya->co ? t('Your username and rCredits webpage address') : tt('triggers new card', u\ray('field', t('name'))) . t('Your username')) . t(' will also change.');
  $title = item(t('Contact Information'));
  $fullName = textFld($mya->co ? t('Company Name:') : t('Your Name:'), $nameDesc, required(@$mya->fullName));
  $onchange = "jQuery('.form-item-pin').show();";
  $email = textFld(t('Email:'), '', attrib(compact('onchange')) + required(@$mya->email));
  if (!$mya->admin and $mya->pin !== '') $pin = passFld(t('PIN:'), t('Required for changing your email address'));

  $submit = submit();

  $form = compact(u\ray('title fullName email pin')) 
        + contactFields(NULL, $mya)
        + compact('submit');
  
  return labeled($form);
}

function formContact_validate($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\normalizeCase(u\just($normals = 'fullName city address postalAddr', $sta['input'])));
  extract(u\just('email pin phone country state', $sta['input']));

  // what's this doing in validate?!
//  js('js/countries.js', 'file', 'header'); // country and state dropdowns
//  js("print_country(\"$country\", \"$state\");", 'inline', 'footer'); // initialize dropdowns

  if ($fullName != $mya->fullName) {
    if ($err = u\badName($fullName)) return say($err, array('field' => 'fullName'), 'fullName');
    $name = u\shortName($fullName);
    $otherName = db\lookup('fullName', 'users', 'name=:name AND uid<>:myid', compact('name', 'myid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
  }

  if (!emailOkay($email, $mya->co, $mya)) return; // say('bad email', u\ray('email', $email), 'email');
  if (!$mya->admin and $email != $mya->email and $pin != $mya->agent->pin) return say('bad pin', 'pin');
  if (!phoneOk($phone, $country)) return;
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) return say('po in location', 'address');
  
  u\preray(compact(u\ray($normals . ' email phone name')), $sta['input']); // fix formatting and add name
}

function formContact_submit($form, &$sta) {
  extract($info = u\just('fullName name email phone faxetc country postalCode state city address postalAddr owns', $sta['input']));
  unset($info['owns']);
  
  $mya = r\acct();
  if ($mya->id == 1) unset($info['name']); // don't change 'admin'
  
  $info = u\normalizeCase(u\just('fullName city address postalAddr', $info)) + $info;

  $mya->setRisk('rents', !@$owns); // @ for tests
  if ($mya->update($info)) {
// (problematic if addPhone fails)  if (!$mya->ok) say('step completed');
    $mya->stepDone('contact', $info);
    return r\go($mya->ok ? 'settings' : 'status', 'info saved');
  } else say('not saved');
}

/**
 * Display a list of people nearby, waiting for an invitation.
 */
function formWaiting($form, &$sta) {
  $mya = r\acct();
  
  $title = item(t('People You Might Know, Waiting to Be Invited'));
//  $subtext = item(t('<p>If the status of someone you invited is "<span style="color:orange;">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span style="color:red;">expired</span>", you may want to call them and send another invitation.</p>'));
  $table = "(SELECT CONCAT_WS(' ', first, last) AS name, MID(zip,1,5) AS zip FROM r_request WHERE NOT done AND ctty=:ctty ORDER BY created DESC,listid DESC LIMIT 50) r";
  $q = db\q("SELECT name, zip FROM $table ORDER BY zip,name", ['ctty' => $mya->community]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $lines[$zip] = (@$lines[$zip] ? $lines[$zip] . ', ' : '') . $name;
  }
  if ($mya->community == r\serverUid()) $lines['01301'] = (@$lines['01301'] ? $lines['01301'] . ', ' : '') . 'Caitlin Sullivan';
  if (@$lines) {
    foreach ($lines as $zip => $zipList) {
      if ($city = db\lookup('city', 'users', 'MID(postalCode, 1, 5)=:zip', compact('zip'))) $zip = $city;
      $list[] = "<tr><th>$zip</th><td>$zipList</td></tr>\n";
    }
  } else $list = [t('No one in your area is currently waiting to be invited.')];
  $list = item('<table>' . join('', $list) . '</table>');
  return compact('title', 'list');
}

/**
 * Display a list of the member's invitees.
 */
function formInvitedWhom($form, &$sta) {
  $mya = r\acct();
  $uid = $mya->cAdmin ? $mya->id : $mya->agentId;
  
  $title = item(t('Your Invitees'));
  $subtext = item(t('<p>If the status of someone you invited is "<span style="color:orange;">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span style="color:red;">expired</span>", you may want to call them and send another invitation.</p>'));
  $old = strtotime((R_INVITE_DAYS + 2) . ' days ago');
  $sql = <<<EOF
    SELECT DISTINCT IFNULL(u1.fullName, i.email) AS who, invited, invited<:old AS expired,
      (u1.flags&(1<<:B_MEMBER)) AS member
    FROM r_invites i LEFT JOIN users u1 ON u1.uid=i.invitee
    WHERE i.inviter=:uid ORDER BY invited DESC
EOF;
  $q = db\q($sql, compact('uid', 'old')); // IFNULL fails for u1.flags (not sure why -- it comes out 0)

  $list = "<table id='invitelist'>\n<tr><th>Invited</th><th>Whom</th><th>Status</th></tr>\n";
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $invited = u\fmtDate($invited);
    if (!$started = strpos($who, ' ')) $who = u\cryptN($who0 = $who, FALSE) ?: $who0; // decrypt email
    $status = ($member or $ok) ? t('is a Member!') : ($started ? t('account Not Complete') : ($expired ? t('expired') : '(no response yet)'));
    $color = $member ? 'green' : ($started ? 'orange' : ($expired ? 'red' : 'gray'));
    $nameClass = $co ? 'name co' : 'name';
    $list .= "<tr><td>$invited</td><td>$who</td><td style='color:$color;'>$status</td></tr>\n";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  return compact(u\ray('title subtext list'));
}

define('R_BASIC_FIELDS', 'first last org federalId dob');

/**
 * Validate the data that affects social security number verification.
 * called from signup validation, as well as from formBasic()
 * @param string/bool $personal: is this a personal account ('mya' means use the current account's type)
 *   'mya' also means we are retrying basic information (the AccountInfo step) during signup
 */
function formBasic_validate($form, &$sta, $personal = 'mya') {
  extract(u\just(R_BASIC_FIELDS, $sta['input']));
  if ((string) $personal == 'mya') {
//    $us = new r\usd($mya = r\acct());
    $personal = !$mya->co;
//    $dw = $mya->can(B_DW);
  }
  $what = $personal ? 'social security number' : 'federal ID';
  if ($personal) {
    list ($min, $max) = array(r\rTime() - 150 * 365.25 * DAY_SECS, r\rTime() + 9 * 31 * DAY_SECS);
    if ($err = u\badDate($dob, $min, $max)) return say($err, 'org'); // badDate makes dob internal
  }
  if ($err = u\badSsn($federalId, @$dob)) return say($err, compact('what'), 'org');

  if (@$mya) { // retrying account info
    if ($personal) { // personal
      if ($err = u\badName($first)) return say($err, 'first');
      if ($err = u\badName($last)) return say($err, 'last');
//      $emptyErr = t('There was a problem submitting your data. Try again?'); // (this happened early on)
//      if ($dw and !$us->ssn($federalId, $err)) return say(@$err ?: $emptyErr, 'org');
/*      if ($dw and $us->step() == 'AccountInfo' and !$us->accountInfo($dob, '', $first, $last, '', '', $err)) {
        return say(@$err ?: $emptyErr, 'first');
      } */
    } else { // org
      if ($err = u\badName($org)) return say($err, 'org');
/*      if ($dw and !$us->accountInfo('', $federalId, '', '', $org, $mya->businessStructure(), $err)) {
        if ($err) return say($err ?: $emptyErr, 'first');
      }
      */
    }
  }
  u\preray(compact(u\ray(R_BASIC_FIELDS)), $sta['input']);
  return TRUE;
}

/**
 * Connect a bank account.
 */
function formBank($form, &$sta, $args = '') {
  global $rUrl;
  $mya = r\acct();
  $usa = ($mya->country == US_COUNTRY_ID);

  $title = item(t('Bank Information'));
  
  if ($hasBank = $mya->risk('hasBank') and $bankAccount = $mya->bankAccount) {
    if ($usa) list ($route, $bankAccount) = u\parseBankAccount(@$bankAccount);
    $bankAccount = str_repeat('x', strlen($bankAccount) - 4) . substr($bankAccount, -4, 4);
    $bankAccount = item($bankAccount, t('Account:'));
    $remove = submit(t('Remove connection'));
    return labeled(compact(u\ray('title bankAccount remove')));
  } else { // no account yet or EFT not yet authorized
    $doConnect = (int) !@$mya->stepsDone['connect'];
    $securityLink = r\promoLink('security.html');
    $reassurance = $mya->minimum ? '' : t('<p><b>NOTE:</b> Funds will be taken from your bank account ONLY when you explicitly request a transfer or when you choose an rCredits minimum balance greater than zero.</p>');
    $subtext = item(t('<p>Choose how you want to get money in and out of your rCredits account. Most people find it convenient to connect a checking account. Connecting lets you use your rCard like a debit card. Otherwise you can trade US Dollars for rCredits at a participating business.</p><p>Complete this form to connect your checking account and authorize funds to be taken from your checking account in exchange for any rCredits you request. Funds may be transferred either by Remotely-Created Check (RCC) or by Automated Clearing House (ACH) direct debit. See our <@a>Privacy and Security Policy</a> for details on how your information is protected.</p>', ['@a' => "a href=$securityLink target=_blank"]) . $reassurance . t('<p>Do you want to connect your checking account at this time?</p>'));
    $connect = boolFld(t('Connect:'), t('Connecting lets you use your rCard like a debit card. Otherwise you can trade US Dollars for rCredits at a participating business.'), $doConnect);

    if ($usa) {
//      $checkHint = item("<img src=\"$rUrl/images/check-hint.png\">");
      $routingNumber = textFld(t('Routing:'), t('Type carefully your bank\'s routing number.'));
    }
    //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
    $bankAccount = passFld(t('Account:'), t('Type carefully your <b>checking</b> account number. <span><br>Savings accounts will not work, at this time.</span>'));
    
    $bankAccount2 = passFld(t('Again:'), t('Type your account number again.<br>(we need to be REALLY SURE we don\'t connect to the wrong account)'));
    //$bankAccountType = radiosFld(t('Type:'), '', required($mya->can(B_SAVINGS)), $typeOptions);
    $connectFields = fieldSet('connectFields2', compact(u\ray('checkHint routingNumber bankAccount bankAccount2')), BARE);
    $submit = submit($doConnect ? t('Connect') : '');    
    
    $js = <<<EOF
function showBankFields(show) {var f=jQuery("#connectFields2"); if (show) f.show(); else f.hide();}
showBankFields(jQuery("#edit-connect-1").attr("checked"));
jQuery("#edit-connect-0, #edit-connect-1").click(function() {
  var yes=jQuery("#connectFields2").is(':hidden');
  showBankFields(yes); jQuery("#edit-submit").val(yes ? "Connect" : "Save");
});
EOF;
    js($js, 'inline', 'footer');
    return labeled(compact(u\ray('title subtext connect connectFields submit')));
  }

}

function formBank_validate($form, &$sta) {
  if (op($sta) == 'remove') return;
  extract(u\just('connect routingNumber bankAccount bankAccount2 bankAccountType', $sta['input']));
  if (is_null($connect)) return say('yes or no', 'connect');
  if (!$connect) return; // nothing more to do

  foreach (u\ray($accts = 'routingNumber bankAccount bankAccount2') as $one) $$one = preg_replace('/[\.\- ]/', '', @$$one);
  u\preray(compact(u\ray($accts)), $sta['input']);
  if (!preg_match('/^(|[0-9]{9})$/', $routingNumber)) say('bad routing number', 'routingNumber');
  if (!db\exists('r_banks', 'route=:routingNumber', compact('routingNumber'))) return say('bad routing number', 'routingNumber');
  if (!preg_match('/^[0-9]{3,17}$/', $bankAccount)) say('bad account number', 'bankAccount'); // {3,20} ? Dwolla says 3-17
  if ($bankAccount != $bankAccount2) say('mismatch', ['thing' => t('account number')], 'bankAccount2');
}

function formBank_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('connect routingNumber bankAccount', $sta['input']));
  $op = op($sta);
  if ($op == 'submit') {
    if ($connect) {
      if ($mya->country == US_COUNTRY_ID) $bankAccount = "USkk$routingNumber$bankAccount";
      $mya->update(compact('bankAccount'));
    }
    $mya->setRisk('hasBank', $connect);
    $mya->stepDone('connect', compact('connect'));
    say('info saved|return to membership');
  } elseif ($op == 'remove') {
    $mya->setRisk('hasBank', $connect = FALSE);
    $mya->update('bankAccount', '');
  }
  
  if ($mya->minimum and !$connect and !r\relation('draw', 'other=:id', ['id' => $mya->id])) {
    $mya->update('minimum', 0);
    say('min zeroed');
  }
}

/**
 * Get the member's security info.
 * @param string $arg: "gotPhoto" if just sucessfully uploaded a photo
 */
function formSecurity($form, &$sta, $arg = '') {
  global $base_url;
  $mya = r\acct();

  if ($arg == 'gotPhoto') say('got photo');
  $person = !$mya->co;
    
/*
Dwolla says: 
We're having trouble confirming your information. We want to be absolutely sure that you are creating this account, not someone else creating an account under your name. Protecting your identity is our #1 concern.

Personal Accounts or Sole Proprietors:
Please upload a government-issued photo ID such as a U.S. driver's license or passport. 

Businesses or Non-Profits:
Please upload a copy of your EIN documentation or Non-Profit Status documentation. 

2.5MB max
*/
//  $has = $mya->hasPhoto('proof');
//  $proofType = $person ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t("EIN documentation from the IRS (or the equivalent) -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  
//    'subtext = item(t('Just like for a bank account, we need to verify your identity. The photo ID file you upload can be either an image or a PDF. Please be patient while the file is encrypted &mdash; it may take several seconds.'));

  $title = item(t('Security Information'));
  $change = item(
    button(t('password'), "$base_url/settings/security/change-password", '', 'warning')
  . button(t('PIN'), "$base_url/settings/security/change-pin", '', 'warning')
  . button(t('photo'), "$base_url/settings/security/photo?" . rand(), '', 'warning')
  , t('Change:'), ['', $mya->ok ? tt('triggers new card', u\ray('field', 'photo')) : ' ']);
  $question = $person ? textFld(t('Question:'), [t('Choose a security question'), t('Type a question no one but you could guess the answer to.')], required($mya->question)) : NULL;
  $answer = $person ? textFld(t('Answer:'), [t('The answer to that question')], required(@$mya->answer)) : NULL;
  if ($mya->cAdmin) $answer = item(@$mya->answer, t('Answer:'));
/* KEEP   'idProof = $has ? 
        fld('item', t('Photo ID:'), '', R_ON_FILE . ($mya->ok ? '' : t(' (pending approval)')))
      : fld('file', t('Photo ID:'), tt('Upload an image of your @proofType. (maximum @R_MAX_PHOTOID_SIZE MB)', compact('proofType')), attrib(array('enctype = "multipart/form-data")));
      */
//    $submit = submit($has ? t('Save') : t('Upload and Save'));  );
  $submit = submit();
  $form = compact(u\ray('title change question answer submit'));

  return labeled($form);
}

function formSecurity_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pin federalId dob', $sta['input']));
}

function formSecurity_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('federalId dob pin question answer idProof', $sta['input']);

  foreach ($info as $k => $v) if ($v == R_ON_FILE or $v == @$form[$k]['#default_value']) unset($info[$k]); // don't change if on file or unchanged

  if (!empty($info)) {
    if (!$mya->update($info)) return say('info not saved', 'ERR');
  }

  say('info saved');
}

//define('R_REL_EMPLOYER_COL', 2); // number of amount/employer column
define('R_REL_OWNER_COL', 3); // number of family/owner column

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function formRelations($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
//  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers = be\accountName($myid);
  $header = u\ray('The other account,Draw,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person draw employee owner permission link'));
  if ($co = $mya->co) {
    $header[R_REL_OWNER_COL] = 'Owns 10%+';
    if (!$mya->ok) $mya->stepDone('relations');
  }

  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, IF(:myid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.employee, s.isOwner, s.draw
    FROM ((r_relations s RIGHT JOIN users u ON u.uid = s.main)
      RIGHT JOIN users v ON v.uid = s.other)
    WHERE s.main=:myid
    ORDER BY fullName, IF(:myid=s.main, s.other, s.main), (:myid=s.other)
EOF;
  $result = db\q($sql, compact('myid'));
  $rows = $raws0 = $raws = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original ("0") raw values from table
    $a = r\acct($other);
    $otherCo = $a->co;
    $otherPerson = !$otherCo;

    $idLink = R_NONE; // default to no link
    if ($mya->co and !$otherCo and $permission >= r\perm(B_SCAN) and $permission != r\perm(B_JOINT)) {
      $text = str_replace(' ', '&nbsp;', $permission >= r\perm(B_BUY) ? t(' rCard') : t(' Cashier Card'));
      $idLink = "<a href=\"$base_url/request-employee-rcard/$other\">request$text</a>";
    }

    $ii = count($rows) . "-$reid"; // "row-reid"
    $drawn = toggle('draw', $draw, $ii);
//    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employee2);
    $isEmployee = !$otherCo ? toggle('employee', $employee, $ii) : R_NONE;
    $Owner = toggle('isOwner', $isOwner, $ii);
    $permissions = $GLOBALS['share permissions'];
// NO    if (!$mya->can(B_JOINED)) $permissions[$permission = B_JOINT] .= '?'; // pending agreement
    if ($co or $otherCo or !$mya->ok or !$a->ok) unset($permissions[r\perm(B_JOINT)]); // no joint accounts with a company or nonmember
    $perm = $otherPerson ? dropdown('permission', $permission, $permissions, $ii) : R_NONE;
    $amountValue = ''; // was amountField('amount', number_format($amount,0), $ii);

    $thisRow = array($fullName, $drawn, $isEmployee, $Owner, $perm, $idLink);
    $thisRaw = compact(u\ray('draw employee isOwner permission'));
//      if ($co) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    
    $rows[] = $thisRow; // what to show on screen
    $raws[] = $thisRaw; // original conceptual ("raw") values for what's show on screen
  }
  $sta['rawValues'] = compact('raws0', 'raws');
  
  $aExact = "a href=\"$base_url/help/payment-exchanges\"";
  $aInvite = "a href=\"$base_url/community/invite\"";
  $aAgent = "a href=\"$base_url/agent\"";
  $help = tt('Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. If they don\'t have an account yet, <@aInvite>invite them</a>! Each permission includes all the permissions above it.', compact(u\ray('aExact aInvite aAgent')));
  //If they wish NOT to open a real account, create an <@aAgent>rPOS Sign-in Account</a> for them.

  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$acctIdentifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no relations');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => item(t('Relations')),
    'subtext' => item($subtext),
    'list' => fld('item', '', '', array('markup' => $list)),
//    'newPerson' => textFld(t('Add Someone:'), t('name, id, email, or phone')), // autocomplete causes problems
    'newPerson' => textFld(t('Add Someone:'), [t('Name, id, email, or phone')], autocomplete('relation')),
    'go' => submit(t('Save Changes')),
  );

  return labeled($form);
}

function formRelations_validate($form, &$sta) {
//  if (confirming_v($sta)) return;
  $mya = r\acct();
  
  if ($newPerson = @$sta['input']['newPerson']) {
    $a = whois($newPerson, 'newPerson', $sta['input'], 'no self-relation');
    $id = $a ? $a->id : '';
    if ($id and r\relation(1, $mya->id, $id)) return say('already related', 'newPerson');
    $sta['input']['newPerson'] = $id; // save for submission handling
  }
  $managers = 0; // for company accounts there needs to be at least one manager
  foreach ($sta['input'] as $key => $value) {
    if (!strpos($key, '-')) continue;
    list ($keya, $rawnum, $reid) = explode('-', $key);
/*    if ($keya == 'amount') {
      $amount = $value;
      if ($err = u\badAmount($amount)) return say($err, compact('amount'), 'amount');
      if (($sta['input'][$key] = round($amount, 0)) != $amount) say('amount rounded', compact('amount'));
    } else
    */
    if ($keya == 'permission') {
      if ($value == r\perm(B_MANAGE)) $managers++;
    }
  }
  if ($mya->co and !$managers and !$mya->admin) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
//  if (confirming_s($sta)) return;
  $mya = r\acct();

  if ($other = @$sta['input']['newPerson']) { // adding a new relation
    $mya->newRelation($other);
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }
  $changes = [];
  extract(u\just('raws0 raws', $sta['rawValues']));
///  debug($sta['rawValues']);
  $joinPerm = r\perm(B_JOINT);
  
  $DBTX = \db_transaction();
  foreach ($sta['input'] as $k => $v) { // ... be\updateRelations must check permissions and data carefully
    if (!strpos($k, '-')) continue;
    list ($k, $rawnum, $reid) = explode('-', $k); // eg split permission-2-37
/**/ if (!in_array($k, $ks = array_keys($raws))) return say("bad field in relations: $k " . print_r($ks, 1), 'err'); // hack attempt?

    $v = 0 + strtr($v, array('true' => 1, 'false' => 0));
    $oldv = 0 + $raws[$rawnum][$k];
/// debug(compact('v','rawnum','k'));
/// debug(0 + $raws[$rawnum][$k]);
    if ($v != $oldv) { // changed
      if ($k == 'permission' and ($v == $joinPerm or $oldv == $joinPerm)) {
        if ($v == $joinPerm) {
          if (@$gotJoin) {say('too many joins', 'err'); continue;} else $gotJoin = TRUE;
        }
        r\setJoin($reid, @$gotJoin); // handle joining or unjoining accounts
      }
      $changes[$reid][$k] = $v;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
///  debug($changes);
  if (!empty($changes)) say(be\updateRelations($changes));
  unset($DBTX);
  
  say($mya->ok ? 'step completed|return to membership' : 'info saved');
  if (u\test()) return r\go('settings/relations');
}

/**
 * @todo: focus on code input
 */
function formBoxes($form, &$sta) {
  $myid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only deletion
  if (form_step($sta, $info) == 'prove') {
    $new = u\fmtPhone($sta['storage']['values']['new']);
    $form = array(
      'title' => item(t('Verify')),
      'subtext' => item(tt('verify cell', u\ray('number', $new))),
      'code' => textFld(t('Code:')),
      'new' => hidFld($new),
      'verify' => submit(t('Verify')),
    );
    return labeled($form);
  }

  $headers = array('#', 'Type', 'Device name', 'Last used');
  $types = u\ray(TX_CHANNELS);

  $sql = "SELECT channel, boxnum, code, boxName, access FROM r_boxes WHERE uid=:myid ORDER BY boxnum";
  $result = db\q($sql, compact('myid'));
  $list = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $type = $types[$channel += 0]; // make sure channel is int, not string
    $boxName = 
        @$boxName ? htmlspecialchars($boxName)
      : ($channel == TX_POS ? (t('POS Device #') . $boxnum)
      : ($channel == TX_WEB ? (t('Computer/Browser #') . $boxnum)
      : ($channel == TX_SMS ? u\fmtPhone($code)
      : 'ERROR')));
//    $list["x$boxid"] = array($boxid, $boxName, $restricted ? 'Yes' : 'No', u\fmtDate($access));
    $list["x$boxnum"] = array($boxnum, $type, $boxName, u\fmtDate($access));
  }

  unset($sta['input']['new']); // don't show number just added
  $form = array(
    'title' => item(t('Devices (computers, phones, tablets, etc.)')),
    'subtitle' => item(t('These are the devices used with this account.')),
    'boxes' => fld('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no devices')), $list),
    'new' => textFld(t('Add a cell phone*: ')),
    'save' => submit(t('Save changes')),
//    'delete' => submit(t('Remove Selected')),
//    'postscript' => item(t('* You may have a cell phone listed on the <a href="settings/contact">Contact Information</a> page, or even as a POS device on this page, but to use it for text message transactions you must add its phone number here.')),
    'postscript' => item(t('* A device may appear on this list more than once if you use it in different ways (for texting, as a Point of Sale device, and/or as a web browser).')),
  );
  return labeled($form);
}

function formBoxes_validate($form, &$sta) {
  $myid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract(u\just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op == 'delete') {
    if (no_selection($boxes)) {
      if ($new) $op = 'add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $sta);
  }

  if ($op == 'save') {
    if (blank_field(compact('new'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($new, 'VALIDATE'), 'new');
  }
}

function formBoxes_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract(u\just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'save') {
    if ($info = previous_state($sta)) return say($info, 'new'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($new, tt('verification code', compact('nonce')));
/**/ if (isDEV) debug("nonce is $nonce"); // (keep this) tell developer what code to type
    $info = compact('new');
    return form_step($sta, $info, 'prove');
  }

  if ($op == 'verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($new));
    $sta['redirect'] = 'settings/boxes';
  }
    
  if ($op == 'delete') {
// needs work    foreach ($boxes as $key => $one) if ($one[1]) say(be\deleteCell(substr($one[1], 1))); // ignore the leading 'x'
  }
}

/**
 * Reset password by choosing a new one from an official email link.
 */
function formResetPassword($form, &$sta, $args = '') {
  extract(u\just('id code', $args)); // linked from an email

  if (flooding($uid = r\loginString(@$id), $sta)) return r\go('', 'flooded', 'ERR'); // getting hammered
  $mya = r\acct($uid);
  if (!passwordOkay($mya, @$code)) return r\go('', 'bad login', 'ERR');
  noFlood($sta);
  $name = hidFld($id); // used in formSignin_submit
  $uid = hidFld($uid);
  $submit = w\submit(t('Save'));
  return labeled(w\pickPassword() + compact(u\ray('submit name uid')));
}

function formResetPassword_validate($form, &$sta) {return formChangePassword_validate($form, $sta);}

function formResetPassword_submit($form, &$sta) {
  extract(u\just('pass1 uid', $sta['input']));
  $mya = r\acct($uid);
  $mya->update('pass oneTimePass', $pass1, '');
  say(t('Your new password has been saved.'));
  $sta += compact('uid'); // _submit requires call by reference
  formSignin_submit('', $sta); // lead user to next step
}

function formChangePassword($form, &$sta) {
  $mya = r\acct();
  if (!$mya->admin) $pass = passFld(t('Current (old) Password:'), '', required());
  $submit = submit(t('Save'));
  return labeled(w\pickPassword() + compact('pass', 'submit'));
}

/**
 * Check new password strength, match with second typing, and validity of existing password, if any.
 * ("if any" because this function is called by formResetPassword_validate())
 */
function formChangePassword_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass1 pass2 strong pass', $sta['input']));

  if (!$mya or !$mya->admin) {
    if (!$strong) return say('weak pass', 'pass1');
    if (isset($pass) and !w\passwordOkay($mya, $pass)) return FALSE;
  }
  if ($pass1 != $pass2) return say('mismatch', ['thing' => t('password')], 'pass1');
}

function formChangePassword_submit($form, &$sta) {
  extract($info = u\just('pass1', $sta['input']));
  $mya = r\acct();
  $mya->update('pass', $pass1);
  say(t('Your new password has been saved.'));
  return r\go('settings/security');
}

function formChangePin($form, &$sta) {
  $mya = r\acct();

  $title = item(t('Change PIN'));
  if (!$mya->admin) {
    $pass = passFld(t('Current Password:'));
  }
  $pin = passFld(t('New PIN:'));
  $submit = submit();
  
  return labeled(compact(u\ray('title pin pass submit')));
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formChangePin_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass pin', $sta['input']));
  if (!$mya->admin) {
    if (empty($pass)) return say('password required', 'pass');
    if (!passwordOkay($mya, $pass)) return FALSE;
  }
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  if ($err = u\badAmount($pin, '>=0', 0)) return say($err, 'pin');
}

function formChangePin_submit($form, &$sta) {
  extract($info = u\just('pin', $sta['values'])); // values NOT input
  $mya = r\acct();
  
  if (empty($pin) or !$mya->update(compact('pin'))) return r\go('security', t('Your PIN remains unchanged.'));
//  if (r\acct()->update(u\ray('pass', r\passHash($pass)))) {
  say(t('Your new pin has been saved.'));
  return r\go('settings/security');
}

/**
 * Login from elsewhere.
 *//*
function formSinx($form, &$sta) {
  extract(u\just('id pw pw2', t\POST())); // posted from rCredits.org or admin computer

  if (flooding($uid = r\loginString(@$id), $sta)) return r\go('', 'flooded', 'ERR'); // getting hammered
///  if (!\user_authenticate($name = r\acct($uid)->name, @$pw)) return r\go('', 'bad login', 'ERR');
  $a = r\acct($uid);
  // if admin, check pw2 against $a->pw2 (a datafield hash)
  $sta['uid'] = ((isDEV and $uid == 1) or passwordOkay($a, @$pw)) ? $uid : FALSE;
  if (!$sta['uid']) return r\go('', 'bad login', 'ERR');
  noFlood($sta);
  $input = ['name' => $a->name];
  formSignin_submit('', compact('uid', 'input'));
}*/

/**
 * Completely redo the login form (see user_login() in user.module).
 */
function formSignin($form, &$sta) {
  global $base_url;

  extract(u\just('name pw2', @$sta['input']));
  //$name = 'L5MA LLDC 3MBG';
  if (strlen(str_replace(' ', '', @$name)) >= ICARD_CODELEN_MIN and !strpos($name, '.') and !db\exists('users', 'name=:name', compact('name'))) {
    \drupal_get_messages(); // drop previous errors
    if (r\iCardAcct($name)) return r\go("signup/code=$name"); // invitation card manual signup!
    return r\go('accept', 'bad invite num', 'err');
  }
  unset($form['actions']);
  unset($form['pass']);
  focusOn((@$name == 'admin' or strlen(@$name) > 4) ? 'pass' : 'name');

  $title = item(t('Welcome to rCredits!'));
  $signup = isDEV ? "<a href=\"$base_url/signup/code=w\">Signup</a>" : '';
  $subtext = item(t('<p>Not yet a member? <@a>Check it out</a>!</p>', u\ray('@a', 'a href=' . r\promoLink(''))) . $signup . tt('<p><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right website (<b><em>@BASE_URL/</em>...</b>). And be sure to sign OUT when you\'re done!</p>'));

  $a = "a tabindex=3 href=$base_url/settings/password/" . @$name; // uses $name, so must precede $name = textField
  $passDesc = tt('<@a>Password problems?</a>', compact('a'));
  $name = textFld(t('Account ID:'), t('Your account ID, email, or username (the letters of your full name, all lowercase)'), required(@$name) + w\autofill('section-rSignin rAccount'));
  $pass = passFld(t('Password:'), $passDesc, required() + w\autofill('section-rSignin rPass'));
  $pw2 = hidFld(@$pw2 ?: ''); // extra password for admins
  
  $submit = submit(t('Sign in'));
  $form = compact(u\ray('title name pass pw2 submit subtext'));

  $form['external_input'] = hidFld(serialize(@$sta['input'])); // pass this to rweb_user_login

  $form['#attributes']['class'][] = 'rweb labeled';
//  if (TRUE) js('e:\\rCredits-admin.js', 'external', 'footer'); // get extra admin password
//  js('js/radiocheck.js', 'file', 'footer'); // this normally gets called in showForm()
  return labeled($form);
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account id, with password.
 * Sets $sta['uid'] to signify successful authentication.
 */
function formSignin_validate($form, &$sta) {
  extract(u\just('name pass pw2', $sta['input']));
  if (w\flooding($uid = r\loginString($name), $sta)) return say('bad login', 'pass'); // getting hammered
  $a = r\acct($uid);
  if ($uid == 1) {
    $pw2 = @$pw2 ? hex2bin($pw2) : (isDEV ? DEV_PW2 : '');
    if ($pw2 and !$a->pw2) a(1)->update('pw2', r\passHash($pw2)); // first ever admin signin sets pw2
    if (!isGAME and !w\passwordOkay($a, $pw2, 'pw2')) return say(t('Admin requires second pass.'), 'pass');
    r\setCook('pw2', $pw2, r\rTime() + SESSION_LIFE); // store as cookie, so pw2 is never on server
  }
  $sta['uid'] = ((isDEV and $uid == 1) or passwordOkay($a, $pass)) ? $uid : FALSE; // ignore pass on dev machine
  if ($sta['uid']) noFlood($sta); else return say('login failed', 'name');

  if (empty($sta['uid'])) {
    // Always register an IP-based failed login event.
    flood_register_event('failed_login_attempt_ip', variable_get('user_failed_login_ip_window', 3600));
    // Register a per-user failed login event.
    if (isset($sta['flood_control_user_identifier'])) {
      flood_register_event('failed_login_attempt_user', variable_get('user_failed_login_user_window', 21600), $sta['flood_control_user_identifier']);
    }

    if (isset($sta['flood_control_triggered'])) {
      if ($sta['flood_control_triggered'] == 'user') {
        form_set_error('name', format_plural(variable_get('user_failed_login_user_limit', 5), 'Sorry, there has been more than one failed login attempt for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', 'Sorry, there have been more than @count failed login attempts for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
      }
      else {
        // We did not find a uid, so the limit is IP-based.
        form_set_error('name', t('Sorry, too many failed login attempts from your IP address. This IP address is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
      }
    }
    else patch('user_login_final_validate2207', $sta['values']['name']); /* CGF begin
      form_set_error('name', t('Sorry, unrecognized username or password. <a href="@password">Have you forgotten your password?</a>', array('@password' => url('user/password', array('query' => array('name' => $sta['values']['name']))))));
      watchdog('user', 'Login attempt failed for %user.', array('%user' => $sta['values']['name']));
    } CGF end */
  }
  elseif (isset($sta['flood_control_user_identifier'])) {
    // Clear past failures for this user so as not to block a user who might
    // log in and out more than once in an hour.
    flood_clear_event('failed_login_attempt_user', $sta['flood_control_user_identifier']);
  }  
//  $sta['submitted'] = TRUE; // dunno why this doesn't get set otherwise (required, to trigger call to _submit)
}

/**
 * Finish signing in.
 * @param assoc $sta: intermal input or external input (from a member merchant website), including
 *   $uid: record ID of account signing in
 * @param string $goto: what page to start on for non-admin
 */
function formSignin_submit($form, &$sta, $goto = 'summary') {
  extract(u\just('uid input external_input', $sta));
  if (!@$uid) hack('sign-in is not authorized');
  $mya = r\acct($uid);
  global $user; $user = $mya->account('uid pass timezone'); // timezone keeps Drupal happy in bootstrap.inc
  drupal_session_regenerate(); // this may need to be disabled for testing?

  extract(u\just('name', $sta['input']));
  
  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($sta['external_input'])); // save params from external online merchant

  svar('myid', $uid);
  svar('scanned_in', FALSE);
  setGlobals($uid, TRUE);

//  r\setCook('loginto', '', r\rTime()-1); // no more default login
  u\loga('login', compact('name'), $uid);
  if ($mya->admin) {
    include_once __DIR__ . '/../admin/admin.inc';
    a\adminSignin();
  }
  return r\go($uid == 1 ? 'sadmin' : $goto);
}

function formPass($form, $sta, $id = '') {
  $id = urldecode(u\urlArgs());
  $title = item(t('Request New Password'));
  $name = textFld(t('Account:'), t('Username (the letters of your full name, all lowercase), Email, or Account ID'), required(u\neq($id, 'password', '')));
  $submit = submit(t('Request'));

  $form = compact(u\ray('title name submit'));
  focusOn('name');
  return labeled($form);
}

function formPass_validate($form, &$sta) {
  extract(u\just('name', $sta['input']));
  if (!@$name) return say('missing field', array('field' => 'account'), 'name');
  if (!@$uid = r\loginString($name)) return say('bad account id', 'name');
  $a = r\acct($uid);
  if ($a->co) return say('no co pass', u\ray('company', $a->fullName), 'name');
  $sta['input']['uid'] = $uid;
}

function formPass_submit($form, &$sta) {
  extract(u\just('uid', $sta['input']));
  u\EXPECT($uid, 'missing uid in pass_submit');
  $a = r\acct($uid);
  $name = $a->name;
  $oneTimePass = oneTimePass($name);
  list ($region, $code) = array(strtolower(R_SERVER_ID), $oneTimePass['pass']);
  $a->update(compact('oneTimePass'));
  r\notify($uid, 'password-reset', compact(u\ray('name code')), TRUE);
  say('password reset');
  r\tellStaff('password reset', compact(u\ray('name uid')), $uid);
  return r\go('empty');
  //$sta['redirect'] = 'user';
}

function formFindCo($form, &$sta) {
  $mya = r\acct();
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: '';

  $list = w\directoryList($which, $region ?: @$mya->community);
  
  $form = array(
    'title' => item(t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => item(t('for a country name or postal code...')),
    'which' => textFld(t('Search for:'), [t('Whatever'), t('Type part of company name or industry category')], dft($which) + autocomplete('industry')),
    'region' => textFld(t('Where:'), [t('Start of postal code'), t('Type the first few characters of your postal code<br>For example, use 013 for Franklin County, Massachusetts; 05 for Vermont, etc.')], dft($region)), // or country name
    'submit' => submit(t('Find')),
    'item' => item($list),
  );

  return labeled($form);
}

function formFindCo_validate($form, &$sta) {
}

function formFindCo_submit($form, &$sta) {
  extract($sta['input']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

function formCompany($form, &$sta, $arg = '') {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  if ($arg == 'gotPhoto') say('got photo');

  $sql = <<<EOF
    SELECT iid, IF(parent=iid, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY parent, IF(parent=iid,0,iid)
EOF;
  $cats = db\q($sql)->fetchAllKeyed();
  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$mya->name" target="_blank">See your public rCredits web page</a> (in a new window)
EOF;

  $canDescs = u\ray(APP_CAN_BIT_DESCS);
  $permissions = '';
  
  foreach ($canDescs as $i => $desc) {
    $outbox = <<<EOF
<div class="form-item form-type-checkbox">
  <input type="checkbox" name="can[$i]" CHECKED class="form-checkbox" />
  <div class="description"><div class="box"></div></div>
</div>
EOF;
    $j = $i + APP_CAN_BITS; // convert signed out to signed in index
    $inbox = str_replace("can[$i]", "can[$j]", $outbox);
    if (!$mya->coCan(APP_CANS + $i)) $outbox = str_replace('CHECKED ', '', $outbox); // uncheck if not set
    if (!$mya->coCan(APP_CANS + $i + APP_CAN_BITS)) $inbox = str_replace('CHECKED ', '', $inbox);
    $permissions .= "<tr><th>$desc</th><td>$inbox</td><td>$outbox</td></tr>\n";
  }

  $permissions = <<<EOF
<div id="permissions"><table>
<tr id="perm-head"><td></td><th>Signed IN</th><th>Signed OUT</th></tr>
$permissions
</table></div>
EOF;

  $title = item(t('Company Information'));
  $links = item($links, ' ');
  $name = item($mya->fullName, t('Company name:'));
  $photo = item(t('<@a>Change your company photo</a>', ['@a' => "a href=$base_url/settings/security/photo?" . rand()]), t('Photo:'));
  $private = boxFld('private', t('Private:'), t('Hide this company from the public (omit from public listings)'), $mya->coCan(CO_PRIVATE) ?: 0);
  $categories = selectFld(t('Categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE] + dft($myCats), $cats);
  $selling = areaFld(t('Selling:'), [t('Products'),t('Enter one or more transaction descriptions for when you make a sale &mdash; one line for each. Put the most common one first.')], required($mya->selling));
  $can2 = item($permissions, t('Permissions:'), t('Download the rPOS app from Google Play Store. Limit app permissions here, or for each agent on the Relations page.'));
//    $shareRpos = boolFld(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), $mya->coCan(CO_SHARE_RPOS) ?: 0);
//    $requireCashier = boolFld(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), $mya->coCan(CO_REQUIRE_CASHIER) ?: 0);
  $website = textFld(t('Website:'), [t('Website address'), t('Especially if you want to accept payments on your website, be sure to list it here.')], dft($mya->website));
  $descPreface = item(t('<br>What does the company do? This description will appear on your rCredits web page.')); //, '', ['class'=>'help-block']);
  $description = fld('text_format', t('Description:'), [t('Arbitrarily complex description')], ['format' => 'filtered_html'] + dft(strtr($mya->description, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])));
  $shortDesc = textFld(t('Short Desc:'), [t('Short description'), t('Short description of your company\'s products and services. Maximum 35 characters.')], attrib(['maxlength'=>35]) + required($mya->shortDesc));
  $employees = textFld(t('Employees:'), [t('Employee count'), t('How many employees do you have?')], required($mya->employees));
  $gross = textFld(t('Annual Gross:'), [t('Approximate annual gross income')], required($mya->gross));
  $tips = boolFld(t('Tips:'), t('After each rCard sale, ask customer to choose a tip percentage (or no tip)'), $mya->coCan(CO_TIP) ?: 0);
  if ($mya->cAdmin) {
    $fast = boolFld(t('No ID Chk:'), t('Never ask customer for photo ID (set by admin)'), $mya->coCan(CO_FAST) ?: 0);
    $atm = boolFld(t('ATM service:'), t('Allow company to trade rCredits for cash even if their balance is less than their rewards (in-person only).'), $mya->coCan(CO_ATM) ?: 0);
  }
  $submit = submit();
  return labeled(compact(u\ray('title links photo name private categories selling shortDesc employees gross website descPreface description can2 tips fast atm submit')));
}

function formCompany_validate($form, &$sta) {
  extract(u\just($fields = 'selling employees gross', $sta['input']));
  $selling = explode("\n", $selling);
  foreach ($selling as $one) {
    $one = trim($one); // don't allow empty or reserved values
    if (!u\nonish($one) and !in_array($one, array('', S_REFUND, S_USD_OUT)) and !r\usdin($one)) $new[] = strtolower($one);
  }
  if (!@$new) return say('missing field', u\ray('field', 'selling'), 'selling');
  $selling = join("\n", $new);
  if ($err = u\badAmount($employees, '>=0')) return sayFieldErr($err, 'employees');
  if ($err = u\badAmount($gross, '', 2, '')) return sayFieldErr($err, 'gross');
  u\preray(compact(u\ray($fields)), $sta['input']);
}

function formCompany_submit($form, &$sta) {
  $mya = r\acct();
  $id = $mya->id;
  extract($info = u\just('private description shortDesc website selling categories can requireCashier employees gross tips fast atm', $sta['input']));
//  $description = \check_markup($description['value']);
  if (is_array($description)) $description = @$description['value']; // when u\test(), description is scalar
  $description = str_ireplace([PHP_EOL, "\r", "\n", "\0", '</li><br>', '<ol><br>', '<ul><br>'], ['<br>', '', '', '', '</li>', '<ol>', '<ul>'], $description);
  $website = str_ireplace('http://', '', @$website);

  $mya->setCoCans(@$can);
  //$mya->setBits(APP_CAN_BITS, $can, 'co');
//  $mya->setBitx(CO_REQUIRE_CASHIER, @$requireCashier, 'coFlags');
  $mya->setCoBit(CO_PRIVATE, $private);
  $mya->setCoBit(CO_TIP, $tips);
  if ($mya->cAdmin) $mya->setCoBit(CO_FAST, $fast);
  if ($mya->cAdmin) $mya->setCoBit(CO_ATM, $atm);
  $mya->update(compact(u\ray('description shortDesc website selling employees gross')));
  
  $DBTX = db_transaction();
  db\q('DELETE FROM r_user_industries WHERE uid=:id', compact('id')); // out with the old
  if (@$categories) foreach ($categories as $iid) {
    db\q("INSERT INTO r_user_industries (uid, iid) VALUES (:id, :iid)", compact('id', 'iid')); // in with the new
  }
  unset($DBTX); // commit

  $mya->stepDone('company', $info);
  return $mya->ok ? say('options saved') : r\go('status', 'options saved|step completed');
}

/**
 * Show a page when someone scans an account ID QR, for example with QRDroid.
 * Cases:
 *   Curious about someone's card (not signed in, scanned person) also give a link to the promo site
 *   Agent looking at a business (signed in, scanned company) - show Site button (to company's rCredits page)
 *   Person looking at a business (signed in, scanned company) - show Site button on the Pay page (NYI)
 
 UNUSED:
 *   Scanning in (not signed in, scanned agent)
 *   Signing in (not signed in, scanned person) - show login page with account id filled in
 *   Agent charging a customer (signed in, scanned any) - show photo and Charge button
 *   Agent paying someone (signed in, scanned any) - show Pay button
 *   Person paying someone (signed in, scanned any) - go to Pay page

 * @param string $who: the part of a scanned rCard's QR code after the domain and '/I/':
 *   the account's qid tail, then "." (pro se) or "-" (agent), then an rCard security code:
 *   must match the cardCode (for pro se) or cardCode2 (for agent) stored in the account's secure field.
 */
function formI($form, &$sta, $who = '') {
  $parts = preg_split('/(' . R_MARKS . ')/', $who, 0, PREG_SPLIT_DELIM_CAPTURE);
  $count = count($parts);
  $promo = r\promoLink('');

  if ($count == 5) { // old fashioned format: ABC.DEF-whatever or ABC-DEF-whatever (agent)
    list ($region, $mark, $tail, $zot, $code) = $parts;
  } elseif ($count == 3) { // new format: DEF.whatever or DEF-whatever (agent)
    list ($tail, $mark, $code) = $parts;
    $region = R_SERVER_ID;
  } else return r\go($promo); // say('bad member id', 'ERR'); // not an ID QR
  
  if ($mark == R_AGENT_URL_MARK) {
    list ($mark, $field) = array(R_AGENT_MARK, 'cardCode2');
  } else $field = 'cardCode';
  
  $qid = "$region$mark$tail";
  if (!$a = r\acct($qid)) return r\go($promo); //say('bad card', 'ERR'); // not a real code
  
  $mya = r\acct();
  if (!$gotCode = @$a->agent->$field) return r\go($promo); // say('bad card code', 'ERR');
  if ($gotCode != $code) return r\go($promo); // say('bad card code', 'ERR');
  list ($otherId, $otherAgent) = array($a->id, $a->agentId);

  //  if (!$mya) return $a->proSe ? r\go('user/login?name=' . $a->name) : scanIn($a);
  return r\go($a->co ? "member/$a->name" : $promo);
}

function formPreferences($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $title = item(t('Account Preferences'));
//  $virtualMaxMessage = $mya->co ? t(' If you choose to enable payment exchanges, this setting is moot (irrelevant and unused).') : '';
//  $subtext = item(t('The suggested settings are best for most people. You can change your choices at any time.'));
  $minDesc = t('How much to keep in your rCredits account, for purchases. When your primary balance goes below this amount, the system automatically refills it for you from your bank account. Refilling may take 3-5 business days. Choose zero (no automatic transfers) if you tend to bounce checks, live close to the wire, or prefer not to use automatic refills.');
  $minDescNegative = $mya->j_balance < 0 ? t(' <b>If you choose a negative amount</b>, it will be increased by @bump a week until it is greater than zero.', ['@bump' => u\fmtAmt(MIN_WEEKLY_BUMP)]) : '';
  $minWarning = tt('note|ach warning');
  $minDefault = u\fmtAmt($mya->prefsSet ? $mya->j_minimum : 0, 's$');
  $minimum = textFld(t('Minimum:'), [htmlspecialchars(t('Your preferred "normal" primary balance')), "<p>$minDesc$minDescNegative</p><p>$minWarning</p>"], required($minDefault));
  $savings = textFld(t('Savings Reserve:'), [t('Savings amount'), t('How much to hold as a savings reserve, for later or as needed. Your Savings Reserve works like a traditional reserve savings account that kicks in whenever you would otherwise overdraw your primary account -- except there are no fees. This also helps the community by keeping more funds in the system. Your incentive rewards go into savings automatically. You get a monthly inflation adjustment on any funds you hold in the rCredits system (at @inflate% APR).', ['@inflate' => R_INFLATION_RATE * 100])], required($mya->prefsSet ? u\fmtAmt($mya->j_savings) : ''));
  $saveWeekly = textFld(t('Save Weekly:'), [t('Additional amount to add weekly to savings'), t('Putting money away as savings can be a challenge. This may help: choose a small additional amount to be added automatically to your @what every week.', ['@what' => $mya->j_balance < 0 ? t('Minimum') : t('Savings Reserve')])], dft($mya->prefsSet ? u\fmtAmt($mya->j_saveWeekly) : ''));
  $showAdvancet = item(t('<a>Show advanced options</a>'), ' '); // different name from showAdvanced (for CSS)

  // Advanced settings
  $achMin = textFld(t('Min Transfer:'), [t('Minimum bank transfer amount'), t('Choose a multiple of $@dft. This much (or more, if necessary) will be transferred from your bank account to your rCredits account when your balance goes below the minumum you chose, above.', ['@dft'=>R_ACHMIN])], required(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's$')));
  if ($mya->ok) $share = w\shareFld($mya); // don't show this during signup (it's on the donation page)

  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));
  $dft = $mya->can(B_WEEKLY) ? 1 : ($mya->can(B_MONTHLY) ? 2 : 0);
  $notices = radiosFld(t('Email Notices:'), t('How often, at most, do you want an email digest of your rCredits messages? (No email if no messages.)'), dft($dft), $noticeRadios);
//  $smsNotices2 = boxFld('smsNotices', t('SMS notices:'), tt('Send me a text message when I receive a payment, charge, or invoice (requires a <a href="' . $base_url . '/settings/boxes">connected cell phone</a>).'), $mya->can(B_BYSMS)); // (currently disabled)
  $mediumRadios = array(
    t('I will accept electronic statements**.'), 
    t('I choose to receive printed statements by mail.') . u\SUBS(t('<div>I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>'))
  );
  $statements = radiosFld(t('Statements:'), '', dft((int) $mya->can(B_PAPER)), $mediumRadios);

  $secretBal2 = boxFld('secretBal', t('Secret Balance:'), t("Don't let merchants see my balance, even if I ask."), $mya->can(B_SECRET));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $nosearch2 = boxFld('nosearch', t('No Search:'), tt("Require other members to identify my account by name or account number only &mdash; not by phone or email."), $mya->can(B_NOSEARCH));
//  $debtOk2 = boxFld('debtOk', t('Debt Okay:'), tt('Let me use my rCard as a <i>credit</i> card, allowing my account balance to go negative, limited by my current credit limit (currently @limit &mdash; based on my account activity). I understand there will be a fee (currently $@R_DEBT_FEE plus @R_DEBT_PERCENT% per month) if I fail to bring my balance back up to zero within @R_DEBT_DAYS days.', ['limit' => u\fmtAmt(-$mya->j_floor)]), $mya->can(B_DEBT));
  $posttext = item(tt(<<<EOF
<h4>** Disclaimers required by the IRS:</h4>
<p>Your choice of how to receive statements applies to all statements you receive from now on. You can change your preference at any time by returning to this Preferences page (using the Settings button) or by notifying the regional administrator by mail: @regName, @regAddr. Any change will be confirmed in writing (electronically if you accept electronic statements, otherwise by US Mail).</p>
<p>To view your electronic statements, you need an typical computer or mobile device connected to the internet, including an email program and a web browser. To print the statements, you need a printer.</p>
<p>Your annual tax statements will be accessible online from January through December.</p>
EOF
  , u\ray('regName regAddr', r\regionfield('legalName'), r\regionfield('postalAddr'))));

  $advancedFields = 'achMin share smsNotices2 notices statements secretBal2 nosearch2 debtOk2 posttext';
  
  $submit = submit(t('Update Preferences'));
  
  $js = <<<EOF
toggleFields('$advancedFields', false);
$('#edit-showAdvancet').click(function() { $(this).hide(); toggleFields('$advancedFields', true); });
EOF;
  js($js, 'inline', 'footer');
  $form = compact(u\ray('title subtext minimum savings saveWeekly showAdvancet ' . $advancedFields . ' submit'));
  return labeled($form);
}

function formPreferences_validate($form, &$sta) {
  $mya = r\acct();
  if (!$sta['input']['saveWeekly']) $sta['input']['saveWeekly'] = 0;
  foreach (u\ray($fields = 'minimum achMin savings saveWeekly') as $k) {
    if ($err = amtErr($k, $sta)) return say($err, $k);
    $$k = $sta['input'][$k];
  }

  extract(u\just($fields, $sta['input'])); // after amtErr checks

  $floor = min($mya->j_balance, max(0, $mya->j_floor)); // can't go deeper in debt
  if ($minimum < $floor) return say('min sub floor', ['floor' => u\fmtAmt($floor)], 'minimum');

  if ($savings < $mya->j_rewards) return say('savings too low', ['rewards' => u\fmtAmt($mya->j_rewards)], 'savings');
  $savingsMax = max($mya->j_savings, $mya->j_rewards, $mya->j_r);
  if ($savings > $savingsMax) return say('savings too high', ['max' => u\fmtAmt($savingsMax)], 'savings');
  if ($minimum < 0 and $saveWeekly < MIN_WEEKLY_BUMP) return say('saveWeekly too low', ['min' => u\fmtAmt(MIN_WEEKLY_BUMP)], 'saveWeekly');
  if ($saveWeekly < 0 and $mya->j_savingsAdd == 0) return say('negative saveWeekly', 'saveWeekly');
  if ($mya->ok and $err = w\badShare($sta['input']['share'], TRUE)) return say($err, 'share');
  if ($achMin < R_ACHMIN) return say('bad achmin', 'achMin');
}

function formPreferences_submit($form, $sta) {
  include_once __DIR__ . '/../admin/admin.inc';
  $mya = r\acct();
  $fields = 'minimum achMin savings saveWeekly share flags';
  extract($info = u\just($fields . ' smsNotices notices statements nosearch debtOk secretBal', $sta['input']));
//  $mya->setBit(B_NOTIFY_EMAIL, $notifyBy & 1, FALSE);
  $flags = $mya->flags;
  $old = ['flags' => a\formatField('flags', $flags)] + (array) $mya->account($fields);
//  u\setBit($flags, B_BYSMS, @$smsNotices);
  u\setBit($flags, B_WEEKLY, @$notices == 1);
  u\setBit($flags, B_MONTHLY, @$notices == 2);
  u\setBit($flags, B_PAPER, @$statements);
//  u\setBit($flags, B_DEBT, @$debtOk);
  u\setBit($flags, B_SECRET, @$secretBal);
  u\setBit($flags, B_NOSEARCH, @$nosearch);
  
  if (@$statements) r\tellStaff($mya->mainQid . ' wants paper statements');
  if ($saveWeekly < 0) say(t('@amount will be moved every week from your Savings Reserve to your Primary Account, until your Savings Reserve reaches its minimum.', ['@amount' => u\fmtAmt($saveWeekly)]));
  
  if ($jid = $mya->jid and $mya->slave) { // slave setting minimum etc.
    $savings -= $mya->rewards; // reserve minimal savings on slave
    r\acct($jid)->update(compact(u\ray('minimum achMin savings saveWeekly'))); // set these only on master
    $minimum = $saveWeekly = 0; // let achMin be set on slave also
    $savings = $mya->rewards;
  }
  $mya->update(compact(u\ray($fields)));
  $mya->stepDone('prefs', compact('old') + ['flags' => a\formatField('flags', $flags)] + $info);
  return $mya->ok ? say('options saved') : r\go('status', 'options saved|step completed');
}

function formGraphs($form, &$sta) {
  if ($mya = r\acct()) {
    $admin = $mya->admin;
// (use Admin tab instead)    $cadmin2 = $mya->can(B_CADMIN2);
    if ($myCtty = $mya->community and $ctty = r\acct($myCtty)) $cttyName = $ctty->fullName;
  }

  $title = item(t('Community Statistics'));
  $subheader = item(@$cttyName ? t('for ') . $cttyName : ' ');
  $subtext = item(t('This page helps us keep an eye on our economy so we can manage it effectively together. <b class="loud">Click</b> any chart for an explanation, or <b class="loud">hover</b> for the numbers.'));
  $charts = item(w\showCharts('all'));
//  $shared = @$admin ? item(fundRpt($shared, @$lm_shared, '$'), t('Shared w CGF:')) : NULL;

  $form = compact(u\ray('title subheader subtext charts'));

  return $form;
}

function formGraphs_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formGraphs_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'community');
}

/**
 * Show accounts and recent transactions and ACHs that carry risk of fraud.
 */
function formFlags($form, &$sta, $args = '') {
  $mya = r\acct();
  $MAXLINES = 20; // max number of lines to show in each section
  include __DIR__ . '/risk-descs.inc';

// fails  u\setDft($args['period'], 1);
  extract(u\just('period starting ending download downloaded', $args)); 
  if (!@$period) $args['period'] = $period = 7;
  extract(u\just('dateRange url starting ending', dateRange($args, 'community/flags', [], '', FALSE)));
  $url = hidFld($url);
  
  $title = item(t('Suspicious Activity Report')); //  . strftime('%A, %m/%d/%y')
  $subtext = item(tt(t('<p>Any potentially suspicious accounts, transactions, and bank transfers (ACHs) are listed here, so all members can keep an eye on things. Only the top @MAXLINES are listed in each category. System administrators will investigate any account or transaction with a suspiciousness score of @K_RED or more (shown in red).</p><p>Green risks are "good" (they mitigate other risk factors). Hover over any risk for a brief description or click it for details.</p>'), compact('MAXLINES')));

  $achHeader = $txHeader = $acctHeader = '';
  $accts = $txs = $achs = '';
  $headers = u\ray(t('Score Account Type Created Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $acctHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $acctHeader = "<tr>$acctHeader</tr>";
  list ($cttyCrit, $cttysCrit) = (!$mya or $mya->admin)? [1, 1] 
    : ["community=$mya->community", "$mya->community IN (u1.community, u2.community)"];
  $sql = <<<EOF
    SELECT risk AS score,uid,IF(:IS_CO,'Co','Indiv') as type,created,risks FROM users 
    WHERE uid>1 AND :IS_OK AND $cttyCrit
    ORDER BY risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql);
  for ($i = 0; $line = riskLine($q, K_ACCT_RISKS, $red) and ($i < K_LINES or $red); $i++) $accts .= $line;
  
  $headers = u\ray(t('Score Tx# Date Amount From To Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $txHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $txHeader = "<tr>$txHeader</tr>";
  $subs = compact('starting', 'ending');
  $sql = <<<EOF
    SELECT t.risk AS score,xid,t.created,amount,payer,payee,payeeFor AS description,t.risks 
    FROM r_txs t LEFT JOIN users u1 ON u1.uid=payer LEFT JOIN users u2 ON u2.uid=payee 
    WHERE (payer>0 OR ABS(amount)>:K_CTTY_THRESHOLD) 
    AND $cttysCrit AND t.created BETWEEN :starting AND :ending 
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $txs .= $line;

  $headers = u\ray(t('Score Tx# Date Amount Account Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $achHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $achHeader = "<tr>$achHeader</tr>";
  $sql = <<<EOF
    SELECT t.risk AS score,txid,t.created,ABS(t.amount) AS amount,payer,IF(t.amount<0,'in','out') AS way,t.risks 
    FROM r_usd t LEFT JOIN users u ON u.uid=payer
    WHERE t.created BETWEEN :starting AND :ending AND $cttyCrit
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $achs .= $line;
  
  $table = item(<<<EOF
  <center>
  <h3>ACCOUNTS</h3>
  <table id="accts">
  $acctHeader
  $accts
  </table>
  <h3>TRANSACTIONS</h3>
  <table id="txs">
  $txHeader
  $txs
  </table>
  <h3>ACHs</h3>
  <table id="achs">
  $achHeader
  $achs
  </table>
  </center>
EOF
  );

  if (@$downloaded) js("window.open(\"$url&download=1\", \"download\");", 'inline', 'footer');
  return labeled(compact('title') + $dateRange + compact('subtext', 'table'));
}

function formFlags_validate($form, &$sta) {w\dateRangeValidate($sta);}
function formFlags_submit($form, &$sta) {dateRangeRedirect('community/flags', $sta);}

function formCttyFunds($form, &$sta) {
  $title = item(t('Funds in Each rCredits Community'));
  $subtext = item(t('How rCredits were issued in each community. Hover over headings for more information.'));
  $headers = u\ray(t('Community r USD Trade-IN Trade-OUT Signup+ Purchase+ Invite+ Inflate+ Spent Grant Loan Invest Fees'));
  $headHelps = u\ray(t('Community name (abbreviated)|Total rCredits issued to date|In exchange for US Dollars|Value of goods and services imported from other communities (negated because credit leaves the community)|Value of goods and services exported to other communities|Signup bonuses (incentive rewards for opening an rCredits account)|Purchase rewards (incentive rewards for buying or selling using rCredits)|Incentive rewards for inviting (and helping) a new member to open an rCredits account|Monthly inflation adjustments|rCredits community spending (net)|rCredits community grants|Zero-interest loans from the rCredits community|Investments by the rCredits community|Fees collected by the rCredits community'));
  foreach ($headers as $k => $v) $headers[$k] = w\popHelp($v, $headHelps[$k]);
  $headers = '<tr><th>' . join('</th><th>', $headers) . '</th></tr>';
  
  $q = db\q("SELECT DISTINCT ctty FROM r_stats s LEFT JOIN users u ON u.uid=ctty ORDER BY ctty=0, u.name NOT LIKE '%.', u.fullName");
  while ($row = $q->fetchAssoc()) { // not fetchField because one value is 0
    extract($row);
    $stats = w\fundStats(r\stats($ctty), FALSE, '1');
    $list[] = '<tr><td>' . join('</td><td>', $stats) . '</td></tr>';
  }
  $list = join("\n", @$list ?: [t('No statistics are available')]); // @ is for tests
  
  $list = item(<<<EOF
<table id="cttyFunds">
$headers
$list
</table>
EOF
  );
  return compact(u\ray('title subtext list'));
}

/**
 * Return a formatted line for the next record, describing its risks.
 * @param SQLresult $q: the query result
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @param bool $red: (RETURNED) the
 * @return an HTML table row to show the record
 */
function riskLine($q, $descs, &$red = '') {
  global $base_url; 
  if (!$row = $q->fetchAssoc()) return FALSE;
  
  if (@$row['amount'] + 0 < 0) list ($row['payer'], $row['payee'], $row['amount']) = array($row['payee'], $row['payer'], -$row['amount']); // don't show user negative amounts
  extract(u\just('uid xid created description score amount risks', $row));

  $regulator = (($mya = r\acct()) and $mya->can(B_REGULATOR));
  global $riskQ, $riskCount; 
  if ($q == @$riskQ) {
    $riskCount++;
    if ($riskCount > K_LINES) return FALSE;
  } else {$riskQ = $q; $riskCount = 1;}
  
  u\setDft($score, '');
  $score = round($score) + 0; // +0 to avoid "-0"
  $red = ($score >= K_RED);
  if ($red) $score = "<span class=\"risky\">$score</span>";
  if ($score <= K_GREEN) $score = "<span class=\"safe\">$score</span>";
  $row['score'] = $score;
  
  foreach (u\just('uid payer payee', $row) as $k => $v) {
    list ($region, $local) = explode('.', $qid = r\quid($v));
//    $row[$k] = "<span>$region.</span>$local";
    $a = r\acct($v);
    if ($v < 0) {
      $v = t('ctty');
    } elseif ($regulator) {
//      $v = "<a href=\"$base_url/sadmin/acct-info/$qid\" title=\"$a->fullName\">$local</a>";
      $v = w\popHelp($local, $a->fullName, "$base_url/sadmin/acct-info/$qid");
    } elseif ($a->co) {
      $v = w\popHelp($local, $a->fullName); //"<a href=\"#\" title=\"$a->fullName\">$local</a>";
    } else $v = t('...');
    $row[$k] = $v;
  }
  
  if (isset($amount)) $row['amount'] = number_format($amount, 2);
  if ($regulator and isset($xid)) $row['xid'] = "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";
  if (isset($created)) $row['created'] = strftime(@$uid ? '%d-%b-%Y' : '%d-%b', $created);
  if (isset($description)) foreach ([t('labor'), t('automatic transfer'), t('donation')] as $k) {
    if (u\abbreviates($k, $description)) $row['description'] = $k;
  }
  $row['risks'] = r\riskList($risks, $descs);
  
  $line = '';
  foreach ($row as $k => $v) $line .= "<td class=\"k-$k\">$v</td>";
  return "<tr>$line</tr>\n";
}

/**
 * Accept an invitation by supplying the invitee email address.
 */
function formAccept($form, &$sta, $args = '') {
  if (@$args == 'zip') {
    $title = item(t('Who do you know?'));
    $friend = textFld(t('Friend\'s name:'), '', required());
    $postalCode = textFld(t('Their zipcode:'), '', required());
  } else {
    $title = item(t('Accept Your Invitation'));
    $invitation = textFld(t('Invitation #:'), t('Carefully type your invitation number.'), required());
  }
  
  $submit = submit(t('Continue'));
  return labeled(compact(u\ray('title subtext invitation friend postalCode submit')));

  //$subtext = item(
//  $subtext = item(strtr(t('A short @BLURB was sent with the invitation (click the link to see it).'), ['@BLURB' => blurbLink()]));
//  $email = textFld(t('Your email:'), t('Type the email address to which the invitation was sent.'));
}

function formAccept_validate($form, &$sta) {
  extract(u\just('invitation postalCode', $sta['input']));
  \drupal_get_messages(); // drop previous errors
  
  if (@$invitation) {
    if (!r\iCardAcct($invitation)) return say('bad invite num', 'invitation');
  } elseif ($err = u\badZip($postalCode)) say($err, 'postalCode');

//  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
}

function formAccept_submit($form, &$sta) {
  extract(u\just('invitation friend postalCode', $sta['input']));
//  $email = u\cryptN($email);
//  if (!$code = db\lookup('code', 'r_invites', 'email=:email ORDER BY (invitee=0) DESC, invited DESC', compact('email'))) return say('bad invite', 'email');
  say(t('Hurray! You qualify to open an rCredits account.'));
  $code = @$invitation ?: "$friend ($postalCode)";
  return r\go("signup/code=$code");
}

/**
 * Show forms 1099-B for the previous calendar year.
 */
function formTaxInfo($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $year = date('Y') - 1;
  $title = item(t('Your rCredits "Barter" Income for @year (Forms 1099-B)', ['@year' => $year]));

  $list = "<table id='f1099b'>\n";
  $list .= "<tr><th>Date</th><th class=\"num\">Income</th><th>From</th><th>Description</th></tr>\n";
  $total = 0;
  $negatives = 0; // total negative transactions (refunds that do not specifically reverse another tx)
  $cats = [];
  $map = [t('rebate on') => t('rebates'), t('bonus on') => t('bonuses')];
  
  while ($row = be\get1099b($mya->id)) {
    extract(u\just('created amount uid for', $row));
    if ($amount <= 0) { // do not issue 1099-B for negative amounts (per IRC 1.6041-...(c)?)
      $negatives += $amount;
      continue;
    }
    x\addCat($cats, strtr($for, $map), $amount, $dups);
    $date = strftime('%d-%b', $created);
    $total += $amount;
    $amount = str_replace('-', '- ', number_format($amount, 2));
    $a = r\acct($uid);
    $addr = str_replace(', Massachusetts', ', MA', $a->postalAddr);
    $phone = str_replace('+1 ', '', u\fmtPhone($a->phone));
    $text = htmlspecialchars("$a->fullName, $addr ($phone)");
    $who = $a->fullName;
    $list .= <<<EOF
<tr>
<td>$date</td>
<td class="num">$amount</td>
<td>$who</td>
<td>$for</td>
</tr>
EOF;
  }

  x\finishCats($cats);
  $total = str_replace('-', '- ', number_format($total, 2));
  $list .= "<tr><td><b>TOTAL</b></td><td class=\"num\"><b>$total</b></td><td></td><td></td></tr>\n</table>\n";
  $list = item($list, t('Detail:'));

  $summary = '';
  foreach ($cats as $cat => $ray) { // for each category
    list ($count, $amount) = $ray;
    $amount = number_format($amount, 2);
    $summary .= <<<EOF
    <tr><td class="category">$cat</td><td class="count">$count</td><td class="amount">$amount</td></tr>\n
EOF;
  }
  $summary = <<<EOF
<table id="summary">
<tr><th>$year</th><th>Count</th><th class="num">Total</th></tr>
$summary
<tr><td><b>TOTAL</b></td><td class="count"></td><td class="num"><b>$total</b></td></tr>
</table>
EOF;
  
  $summary = item($summary, t('Summary:'));
  $regName = r\regionfield('legalName');
  $regPhone = str_replace('+1 ', '', u\fmtPhone(r\regionfield('phone')));
  $negatives = $negatives ? t('<p>You also had negative (refund) transactions totaling @NEG. These are not reported to the IRS, so you might include this negative transaction total on line 2 of 1040 Form C.</p>', ['@NEG' => u\fmtAmt($negatives)]) : '';
 
  $subtext = item(t(<<<EOF
  <p>The rCredits system is required by law to report (to the IRS and to state government) all rCredits payments made to you in the past calendar year (on Forms 1099-B). This page shows a list of those transactions. Each individual income transaction is reported, including your incentive rewards and inflation adjustments.</p>
   <p>YOU are not required to file Forms 1099-B. So the information on this page is for just for your information.</p>
  <p>The purpose of 1099-B is to report transactions, not total income. So don't worry if some of the payments listed here aren't income (such as reimbursements). Also, you may receive a Form W2 or 1099-MISC from your employer or customers for some of these payments. Take care not to over-report your income by counting things twice.</p>
  <p>For more information on the tax implications of rCredits, <@aTaxHelp>click here</a> or download <@aTips>these rCredits Accounting Tips</a>.</p>
EOF
  , ['@aTaxHelp' => "a href=$base_url/help/taxes", '@aTips' => 'a href=http://commongoodfinance.org/files/rCreditsAccountingTips.pdf target=_blank']) . $negatives);
  
  $posttext = item(tt('<p>This report is provided by @regName (EIN #@R_REGION_EIN). Address: @regAddr (@regPhone, @regEmail).</p><p>You can see this report here until January 1 of next year (when the report for calendar year @thisYear replaces it).</p><p>If you would rather receive reports by US Mail, you can change that setting in your <a href="settings/preferences">Preferences</a>.', u\ray('regName regAddr regPhone regEmail thisYear', $regName, r\regionfield('postalAddr'), $regPhone, r\regionfield('email'), date('Y'))));
  return labeled(compact(u\ray('title subtext table summary list posttext')));
}

/**
 * Request an employee ID card (for buying on behalf of the company).
 */
function formRequestRCard($form, &$sta, $other = '') {
  $mya = r\acct();
  if (!$acct = r\acct($mya->id, @$other)) return hack('request rCard');
//  if (!$mya->co or !$mya->can(B_REFUND)) return say('no account permission');
  if (!$mya->can(B_SCAN)) return say('no account permission', 'err');
  $fullName = $acct->fullName;
  
  if ($mya->can(B_BUY)) {
    $title = item(t('Request Employee rCard'));
    $subtext = tt('A Company Agent rCard will be US Mailed to @fullName, to be used for making in-person purchases on behalf of the company.', compact('fullName'));
  } else { // requesting cashier card -- not for purchases
    $title = item(t('Request Cashier Card'));
    $subtext = tt('An rCredits Cashier Card will be US Mailed to @fullName, to be used for charging customers on behalf of the company.', compact('fullName'));
  }
  
  $subtext = item($subtext . tt(' Your company account will be charged $@R_CARD_COST.'));
  $other = hidFld($other);
  $request = submit(t('Request Card'));
  $cancel = submit(t('Cancel'));

  return labeled(compact(u\ray('title subtext other request cancel')));
}

function formRequestRCard_submit($form, &$sta) {
  global $base_url;
  extract(u\just('other', $sta['input']));
  if (op($sta) == 'request') {
    $mya = r\acct();
    $reid = r\relation('reid', $main = $mya->id, @$other);
    $quid = r\quid($main, $other);
    $company = $mya->fullName;
    $otherName = r\acct($other)->fullName;
    $address = $mya->postalAddr; // was r\location($mya, TRUE);
    $href = "$base_url/print-rcard/$quid";
    r\tellStaff($mya->can(B_BUY) ? 'company rcard' : 'cashier card', compact(u\ray('main other reid quid href manager otherName address')));
    say('card ordered', compact('otherName', 'company'));
  } else say('no card ordered');
  return r\go('settings/relations');
}

function formFooter() {
  global $base_url, $rUrl;
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $mya = r\acct();
  $extraLinks = ($mya and ($mya->can(B_MANAGE) or $mya->admin)) ? <<<EOF
    <a href="$base_url/community/donate">Donate</a> |
    <a href="$base_url/community/invite">Invite Someone</a> |
EOF
  : ($mya ? '' : '<a href="' . $base_url . '/signin">Sign in</a> |');

  $helpType = $mya ? '' : 'other';
  $siteLink = r\promoLink('');
  $securityLink = r\promoLink('security.html');
  $sysEmail = SYS_EMAIL;
  
  $markup =<<<EOF
<hr>
$extraLinks
    <a href="$siteLink">About rCredits</a> |
    <a href="$base_url/community/agreement">The Agreement</a> |
    <a href="$base_url/community">Communities</a> |
    <a href="$securityLink">Privacy & Security</a> |
    <a href="$base_url/help/$helpType">Help</a>
<br><br>
copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance<sup>&reg;</sup></a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:$sysEmail" target="_blank">$sysEmail</a><br>
EOF;

  $footerText = item($markup); // unique name for this page
  $sid = db\lookup('sid', 'sessions', 'uid=:uid', ['uid' => $mya ? $mya->agent->id : 0]);
  $life = ($mya and !isDEV) ? SESSION_LIFE : 0;

  $js = <<< EOF
//$(function() { // the DOM is ready enough (this being in the footer)
var signoutUrl = '$base_url/signout/timedout';
var ajaxUrl = '$base_url/ajax';
var ajaxSid = '$sid';
var sessionLife = 1000 * $life; // convert to seconds
var signoutWarningAdvance = Math.min(sessionLife / 2, 5 * 60 * 1000); // give the user a few minutes to refresh
var sTimeout = $life ? sessionTimeout() : 0; // Warn the user before automatically signing out.
EOF;
  
  js('js/jquery-ui.min.js', 'file', 'footer', -999);
  js('js/bootstrap.min.js', 'file', 'footer', 0);
  js('js/spin.min.js', 'file', 'footer', 0); // used by ladda
  js('js/ladda.min.js', 'file', 'footer', 0);
  js('js/ie10-viewport-bug-workaround.js', 'file', 'footer', 0); // IE10 viewport hack for Surface/desktop 
  js('js/misc.js', 'file', 'footer', 0);
  js('js/jquery.confirm.min.js', 'file', 'footer', 0);
  js('js/alert.js', 'file', 'footer', 0);
  js($js, 'inline', 'footer', 1);
  return compact('footerText');
}

/**
 * Return a display of participating companies (for promo site)
 */
function whosin($ctty = '') {
//  if (!$uid = db\lookup('uid', 'users', 'name=:ctty', ['ctty' => "$ctty."])) return '';
  if (!$uid = r\unQuid("!$ctty") or !$list = r\companies($uid, TRUE)) return '';
  $map = [
    '<tr><td width="200">' => '<div class="row"><div class="cmpy-name">',
    '</td><td style="margin-left:10px;">' => '</div><div class="cmpy-does">',
    '</td></tr>' => '</div></div>',
  ];
  $list = strtr($list, $map); // temporary fudge
/**/  echo $list;
  exit();
}