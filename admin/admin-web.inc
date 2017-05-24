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
use rCredits\Web as w;

function seeSecure($a) {
  $mya = r\acct();
  $secure = $a->secure;
  foreach (ray(R_SECURE_FIELDS) as $k) if (isset($secure[$k])) {
    $new[$k] = $secure[$k]; // reorder
    if ($mya->superAdmin or ($k != 'pw2' and !u\inList($k, R_VSECURE_FIELDS))) $show[$k] = $a->$k; // some need decrypting
  }
  $a->update('secure', @$new ?: []);
  say(u\rayTable($show, FALSE));
}

/*
function seeHidden($a) {
  foreach (ray(R_SECRET_FIELDS) as $k) if ($k != 'ssnData') $ray[$k] = $a->$k;
  say(u\rayTable($ray, FALSE));
}
*/

/**
 * Change a transaction amount (only if the transaction happened today and only for a non-rewardable type).
 * @param int $xid: tx record ID
 * @param float $oldAmt: the old amount (for a consistency check)
 * @param float $newAmt: what the amount should be
 */ /*
function changeTxAmt($xid, $oldAmt, $newAmt) {
  if (!$rec = db\lookup('payer,payee,amount,created,goods', 'r_txs', 'xid=:xid', compact('xid'))) return say(t('No such transaction.'), 'ERR');
  extract($rec);
  if ($amount != $oldAmt) return say(t('The existing amount of that transaction is different.'), 'ERR');
  if (u\fmtDate($created) != u\fmtDate(r\rTime())) return say(t('That transaction is not from today.'), 'ERR');
  if ($goods) return say(t('That transaction has rewards.'), 'ERR');
  if (!$amount = round($newAmt, 2)) return say(t('Bad new amount.'), 'ERR');

  $DBTX = \db_transaction();
  db\update('r_txs', compact('xid', 'amount'), 'xid');
  $change = $newAmt - $oldAmt;
  if ($payee > 0) db\q('UPDATE users SET r=r+:change WHERE uid=:payee', compact('change', 'payee'));
  if ($payer > 0) db\q('UPDATE users SET r=r-:change WHERE uid=:payer', compact('change', 'payer'));
  say(t('done'));
  unset($DBTX);
}*/

/**
 * Return Summary fields exclusively for community admins (admin, cAdmin, cAdmin2, or regulator).
 */
function adminSummary($a, $cAdmin = FALSE) {
  global $base_url;
  include_once __DIR__ . '/../admin/admin.inc';  

  $mya = r\acct();
  $uid = $a->id;
  $self = ($uid == $mya->agentId);
  $ctty = $a->community ?: r\serverUid(); // serverUid for admin, if it has no ctty
  
  if ($a->isNormal) { // managing some individual community member account
    if (!$a->ok) $a->nextStep();
    if ($cAdmin) {
      if (showSecureMessages()) $secureMsgs = submit(t('Show secure messages'));
      $html = $a->photoHtml(TRUE, TRUE);
      $photo = item("<a href=$base_url/settings/security/photo>$html</a>");
      if ($mya->admin) $community = r\cttys($a->region, $ctty);
      $donations = db\lookup($a->ok ? 'SUM(amount)' : 'amount', 'r_gifts', 'uid=:uid', compact('uid')) ?: 0;
      if ($donationInfo = db\lookup('often,honor,honored', 'r_gifts', 'uid=:uid', compact('uid'))) {
        extract($donationInfo);
        $donationInfo = "howoften:$often ($honor) $honored";
      }
      $donations = item(u\fmtAmt($donations) . ", $donationInfo @$a->share%", t('Donations:'));

//      $creditLimit = textFld(t('Credit limit:'), '', dft(u\fmtAmt(-$a->floor)));
//      if ($a->stepsDone['prefs']) { // don't even look at these if newbie hasn't chosen yet
//        $minimum = textFld(t('Minimum:'), '', dft(u\fmtAmt($a->minimum)));
//        $achMin = textFld(t('ACH Min:'), '', dft(u\fmtAmt($a->achMin ?: R_ACHMIN_DEFAULT, 's')));
//      }
        $minimum = item(u\fmtAmt($a->minimum), t('Minimum:'));
        $achMin = item(u\fmtAmt($a->achMin), t('ACH Min:'));
//      if (!$a->stepsDone['dw']) { // show info needed to open a Dwolla account
//      extract((array) $a->secure, EXTR_PREFIX_ALL, 's'); // dunno why (array) is required (indirect mod error)
      if (!$a->co) $trust = item($a->trust, t('Trust:'), t('This person\'s trust rating'));
    }
  
    $activity = item(a\showActivity($a), t('Activity:'));
    if (!$a->ok) $steps = item(a\showSteps($a), t('To do (red):'), t('|sign=agreement | prefs=preferences | connect=connect bank (or not) | company=company info'));
//    $created = item(u\fmtDate($a->created), t('Created:'));
    if ($rels = a\showRelations($a)) $relations = item($rels, t('Relations:'));
    if ($a->signupCo) $signupCo = item($a->signupCo['company'] . ' (' . $a->signupCo['companyPhone'] . ') ' . (@$a->signupCo['isOwner'] ?: '') . (@$a->signupCo['isEmployee'] ?: ''), t('Signup Co:'));
    
//    $invited = item(db\count('r_invites', 'inviter=:uid', compact('uid')), t('Invited:'));
    $invitedBy = $a->helper ? ($a->helperA->fullName . "#$a->iCode (" . r\iCodeDesc($a->iCode) . ') + ') : '';
    $invitedBy = item($invitedBy . db\lst('fullName', 'r_invites i LEFT JOIN users u ON u.uid=i.inviter', 'invitee=:uid', compact('uid')), t('Invited by:'));
    $proxies = item(db\lst('fullName', 'r_proxies p LEFT JOIN users u ON u.uid=p.proxy', 'p.person=:uid ORDER BY priority', compact('uid')), t('Proxies:'));
    $proxyFor = item(db\count('r_proxies', 'proxy=:uid', compact('uid')), t('Proxy for:'));
  } else { // managing a community itself
    $count = item(db\count('users', 'community=:ctty AND uid<>:ctty', compact('ctty')), t('Members:'));
    $cInfo = be\creditInfo();
    foreach ($fields = ray('balance in rewards out') as $k) {
      $v = $cInfo->$k;
      if ($k == 'out') $v -= $cInfo->rewards;
//      if ($k == 'r') $v += $cInfo->rewards;
      $$k = u\fmtAmt($v + 0);
    }
    
    $balance = item(t('%balance = %in in - %out out (Rewards issued: %rewards)', compact($fields)), t('Funds:'));
  }
  
  if ($cAdmin and !$self) {
    include __DIR__ . '/../rweb/risk-descs.inc';
    include __DIR__ . '/../rweb/bit-descs.inc';

    $legalName = textFld(t('Legal Name:'), '', required($a->legalName));    
    if ($mya->superAdmin) $rTrader = ($a->isCtty)
    ? boolFld(t('rTrader?'), '', $a->ok)
    : boolFld(t('rTrader?'), t('Is account approved to use %PROJECT? -- member, <%a>good photo</a>, ss# checks out, set risks and mediaConx', '_a', w\atag('/settings/security/photo?' . rand())), $a->ok);
    if (!$a->ok) { // don't show helper field if it's moot (too late -- somebody already got credit)
      if ($helper = $a->helper ?: ($a->co ? $a->by : db\lookup('inviter', 'r_invites', 'invitee=:uid', compact('uid')))) {
        $hA = r\acct($helper);
        $helper = "$hA->mainQid: $hA->fullName";
      }
      $helper = textFld(t('Helped by:'), t('Who gets the "inviter/helper" reward? Must be exactly one account.'), dft($helper)); //  + autocomplete()
    } else $helper = item(@$a->helperA->fullName ?: t('N/A'), t('Helped by:'));

    if ($a->member and !$a->co and $mya->superAdmin) $ssnCheck = item(a\ssnCheck($a));
    if ($mya->admin) {
      $dba = ($a->co and $owner = r\relation('other', 'main=:uid', compact('uid')) and r\acct($owner)->federalId == $a->federalId);
      $federalId = textFld(t('EIN/SSN:'), '', $a->federalId ? ($dba ? t('SSN') : R_ON_FILE) : '');
    }
  
    $acctRisks = ray(K_ACCT_RISKS);
    $riskBits = array_keys($acctRisks);
    $risksEditable = array_flip(ray(K_ACCT_RISKS_EDITABLE));
    foreach ($riskBits as $k) {
      $class = $acctRisks[$k] < 0 ? 'goodRisk' : 'badRisk';
      if (!isset($risksEditable[$k])) $class .= ' noEdit';
      $dpy = w\popHelp($k, $riskHelp[$k], '#');
      $aliases[$k] = "<div class=\"$class\">$dpy</div>";
    }
    $risks = bitsFld(t('Risks:'), $a->risks, '', K_ACCT_RISKS, $aliases);
    if ($a->isCtty) $flagHelp = u\just(B_CTTY_BITS, $flagHelp);

    $adminable = ray(B_ADMINABLE . ($mya->superAdmin ? ' admin' : ''));
    foreach (ray($a->isCtty ? B_CTTY_BITS : B_LIST) as $k) if (!preg_match('/u[0-9]+/', $k)) { // ignore undefined bits
      $blist[] = $k0 = $k;
      $class = in_array($k, $adminable) ? '' : 'noEdit';
      if ($a->isCtty) $k = strtr($k, ray(B_ADMIN_BITS));
      $dpy = w\popHelp($k, $flagHelp[$k0], '#');
      $aliases[$k0] = "<div class=\"$class\">$dpy</div>";
    }
    $adminable = bitsFld(t('Bits:'), $a->flags, 'b', join(' ', $blist), $aliases);

//    $who = textFld(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete($admin ? '' : 'ctty')); // keep this for when we need it (just add to returned fields)
  }      

  if ($a->id == r\cgfId()) {
    $sql = <<<EOF
      SELECT u.community AS ctty, SUM(IF(payee=:uid, amount, 0)) AS `in`,
        SUM(IF(payer=:uid, amount, 0)) AS `out`
      FROM r_txs t LEFT JOIN users u ON u.uid=IF(payer=:uid, payee, payer)
      WHERE (payer=:uid AND payee<0) OR payee=:uid GROUP BY community
EOF;
    $q = db\q($sql, compact('uid'));
    $lines = '<tr><th>Community</th><th>Donations</th><th>Paid</th><th>Due</th></tr>';
    while ($row = $q->fetchAssoc()) {
      extract($row);
      $row['due'] = round($in / 2 - $out, 2);
      foreach (['in', 'out', 'due'] as $k) $$k = number_format($row[$k], 2);
      $ctty = r\acct($ctty)->fullName;
      $lines .= "<tr><td>$ctty</td><td>$in</td><td>$out</td><td>$due</td></tr>\n";
    }
    $cgfDonations = item("<table id=\"cgf-donations\">$lines</table>");
  }
  $calling = item($a->calling, t('Calling:'));
  $tenure = item($a->tenure . t(' month(s)'), t('Tenure:'), t('How long had this person or company been at this address before opening a %PROJECT Account?'));

  if (!$a->co) {
    $moves = item($a->moves === '' ? '?' : $a->moves, t('Moves:'), t('(How many times has this person changed counties in the past decade?)'));
    $mediaConx = textFld(t('Media Conx:'), t('(How many 1st or 2nd-degree connections do you have with this person on FaceBook or LinkedIn?)'), dft($a->mediaConx ?: 0));
  }

  $notes = areaFld(t('Notes:'), '', dft($a->notes));
  $risk = item(round($a->risk) . ' ' . button(t('Recalc'), "$base_url/rerisk/acct/$uid/1"), t('Risk Score:'));
  if (!$a->co) $trust = item(number_format($a->trust, 1), t('Trust:'));
  if ($mya->can(B_CADMIN2)) $submit = submit(t('Go'));    

  $back = item('<a href="' . $base_url . '/sadmin/member-list">Go to Member List</a>', ' ');

  $js = <<<EOF
$('#edit-community').on('change', function() {
  var newCtty = this.value;
  yesno('Change rewards transactions retroactively?', 
    function() {changeCtty(newCtty, true);},
    function() {changeCtty(newCtty, false);}
  );
});

function changeCtty(newCtty, retro) {
  post('changeCtty', {uid:$uid, newCtty:newCtty, retro:retro}, function(j) {
//    var jo = JSON.parse(j);
    if (!j.ok) $.alert(j.message, 'Error');
    //JSON.stringify(j, null, 2)
  });
}
EOF;
  js($js);

  $uid = hidFld($uid); // must postcede js (which uses uid)
  
  return compact(ray('community donations secureMsgs photo balance cgfDonations federalId legalName calling signupCo tenure activity steps dwInfo relations invited invitedBy proxies proxyFor ssnCheck moves floor minimum achMin risk trust risks adminable mediaConx helper rTrader notes count submit back uid'));
}

/**
 * Download the member list for the current community
 */
define('EXPORT_PARAMS', 'do requests allCttys status types anonymous');

function export($params) {
  extract(u\just(EXPORT_PARAMS, $params));
  $mya = r\acct();
  $ctty = r\acct($mya->community);

  $filename = ($allCttys ? PROJECT : str_replace('.', '', $ctty->name))
  . (@$status == 0 ? t('-active') : (@$status == 1 ? t('-notyet') : ''))
  . (@$types == 0 ? t('-members') : (@$types == 1 ? t('-orgs') : t('-accounts')))
  . '.csv';
  u\csvStart($filename);
  $fields = @$anonymous ?
    'uid city state zip type'
      . ' created activated signed helper iCode balance rewards savingsAdd minimum share risk trust'
      . ' cttyName co gross employees achMin pinThreshold lastiCode tenure moves mediaConx jid'
      . ' owns bankName slave closed notices secret'
  : 'id first_name last_name phone email address city state zip cttyName type ecode modified created special';
  u\csv($fields = ray($fields));

  if ($requests) { // handle requests first, in case member status gets updated by later record
    $startDate = strtotime('1/1/2016');
    $sql = 'SELECT * FROM r_request WHERE created>=' . $startDate;
    if (!$mya->admin) $sql .= ' AND ctty=' . $mya->community;
    $q = db\q($sql);
    while ($row = $q->fetchAssoc()) {
      extract($row);
      if (db\exists('users', 'mail=:email', compact('email'))) continue; // don't duplicate or send to nonudge people
      list ($id, $first_name, $last_name, $modified) = [$listid, $first, $last, time()];
      $address = $city = $state = $ecode = $special = '';
      $type = $ctty == 0 ? '' : 'member';
      foreach (ray('phone email') as $k) $$k = u\decryptN($$k);
      if (strpos($email, '@example.com')) continue;
      $cttyName = $ctty ? trim(str_replace(PROJECT, '', r\acct($ctty)->fullName)) : '';
      u\csv(array_values(compact($fields)));
    }
  }  
  
  $where = @$allCttys ? '1' : 'community=:cttyId';
  if (@$status == 0) $where .= ' AND :IS_OK';
  if (@$status == 1) $where .= ' AND NOT :IS_OK';
  if (@$types == 0) $where .= ' AND NOT :IS_CO';
  if (@$types == 1) $where .= ' AND :IS_CO';
  $sql = "SELECT uid AS id, fullName FROM users WHERE uid>1 AND NOT :IS_NONUDGE AND NOT :IS_CLOSED AND $where ORDER BY :IS_OK, -signed, -created";

  $q = db\q($sql, ray('cttyId', $ctty->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($id);
    $type = $a->ok ? ($a->co ? 'member company' : 'member') : ($a->co ? 'company' : '');
    if (@$anonymous) {
      foreach ($fields as $k) $$k = $a->$k; // do this first. some will be null.
      $zip = $a->postalCode;
      foreach (ray('weekly monthly secret') as $k) $$k = $a->can(u\consta('b', $k));
      $notices = $monthly ? t('monthly') : ($weekly ? t('weekly') : t('daily'));
      $state = r\realstate($state);
      if ($routing = substr($a->bankAccount, 4, 9)) {
        $bankName = db\lookup('name', 'r_banks', 'route=:routing', compact('routing'));
      } else $bankName = '';
    } else {
      list ($first_name, $last_name) = u\parseName($fullName);
      $email = $a->email;
      $phone = $a->phone;
      list ($address, $city, $state, $zip) = u\parseAddr($a->postalAddr);
      $phone = u\fmtPhone($phone);
      $id = $a->mainQid;
      $ecode = "$a->emailCode";
      $modified = u\fmtDate(time());
// (BROKEN)      $special = $a->special;
    }
    $cttyName = trim(str_replace(PROJECT, '', $a->cttyA->fullName));
    $created = u\fmtDate($a->created);
    if (is_numeric($zip)) $zip = ' ' . $zip; // prevent interpretation as a number (but Excel will strip any leading zeroes)

    u\csv(array_values(compact($fields)));
  }
  
  exit();
}

/*
function showCardCode($a) {
  $len = strlen(@$a->cardCode);
  if ($len == 14 or $a->co) return;
/// if ($a->ok) debug("$a->fullName\t" . $a->cardCode . " (" . $len . ')');
}*/

/**
 * Show information about a specific transaction and its risks.
 */
function txInfo($xid) {
  global $base_url;
  $x = r\x($xid);
  extract($x->ray);
  extract(u\just('xfee force undoes undoneBy', $x->data));
  
  $title = item(t('Transaction #') . $xid);
  $when = item(u\fmtDate($created) . t(' at ') . u\fmtTime($created) . (@$force ? t(' (created offline)') : ''), t('Date/Time:'));
  $amountDesc = item(u\fmtAmt($amount), t('Amount:'));
  $from = item(txInfoParty($payer, $payerAgent, $payerTid), t('From:'));
  $to = item(txInfoParty($payee, $payeeAgent, $payeeTid), t('To:'));
  $same = ($payerFor == $payeeFor);
  $goodsDesc = ' (' . ray(R_WHYS)[$goods] . ')';
  $for1 = item($payerFor . $goodsDesc, $same ? t('For:') : t('Payer Desc:'));
  if (!$same) $for2 = item($payeeFor, t('Payee Desc:'));
  $by = item($x->taking ? t('by payee') : t('by payer'), t('Initiated:'));
  $channels = ray(TX_CHANNELS);
  $where = item($channels[$channel] . t(' (Device #box)', compact('box')), t('Method:'));
  // related transactions (xfer/undoes/undoneBy
  if (@$undoes) $rel[] = t('undoes tx#') . txLink($undoes);
  if (@$undoneBy) $rel[] = t('undone by tx#') . txLink($undoneBy);
  if (@$xfee) $rel[] = u\fmtAmt($xfee) . ' xfee';

  if (@$rel) $related = item(join(', ', $rel), t('Related Txs:'));
  $riskDesc = $risk < K_RED ? t('low') : ($risk < 2 * K_RED ? t('moderate') : t('high'));
  $risks = r\riskList($risks0 = $risks, K_TX_RISKS); 
  $risk = item(round($risk) . button(t('Recalculate'), "$base_url/rerisk/tx/$xid/1") . " ($riskDesc): $risks", t('Risk:'));
  $adminOk = boolFld(t('Risk Ok?'), '', $risks0 & 1);
  $submit = submit();
  return labeled(compact(ray('title when amountDesc from to for1 for2 by where risk $adminOk related submit')));
}

function txInfoParty($uid, $agent, $tid) {
  $desc = r\acct($uid)->linkedDesc();
  $by = $agent == $uid ? t('pro se') : (t('by ') . r\acct($agent)->linkedDesc());
  return "$desc $by (#$tid)";
}
  
function txLink($xid) {global $base_url; return "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";}