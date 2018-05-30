<?php
/**
 * @file
 * rCredits admin
 */
namespace CG\Web;

use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;

function seeSecure($a) {
  global $mya;
  $secure = $a->secure;
  foreach (ray(R_SECURE_FIELDS) as $k) if (isset($secure[$k])) {
    $new[$k] = $secure[$k]; // reorder
    $show[$k] = $a->$k; // some need decrypting
  }
  $a->update('secure', @$new ?: []);

	if ($mya->superAdmin) {
	  $vsecure = $a->vsecure;
		foreach (ray(R_VSECURE_FIELDS) as $k) if (isset($vsecure[$k])) {
			$vnew[$k] = $vsecure[$k]; // reorder
			$show[$k] = $a->$k; // some need decrypting
		}
    $a->update('vsecure', @$vnew ?: '');
  }

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
  if (!$rec = db\get('payer,payee,amount,created,goods', 'r_txs', 'xid=:xid', compact('xid'))) return say(t('No such transaction.'), 'ERR');
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
 * Return Summary fields exclusively for community admins (admin, admin2, cAdmin, cAdmin2).
 */
function adminSummary() {
  global $mya, $base_url;
  include_once __DIR__ . '/../admin/admin.inc';  

  $uid = $mya->id;
  $ctty = $mya->community ?: r\serverUid(); // serverUid for admin, if it has no ctty
  if ($mya->proSe and !$mya->superAdmin) return []; // nothing special on admin's own account
  
  if ($others = db\lst('fullName', 'sessions s JOIN users u ON u.uid=s.uid', ':uid=IFNULL(s.acct, s.uid) AND s.uid<>:agentId', ray('uid agentId', $uid, $mya->agentId))) {
    w\say(t('<b class="loud">Other people</b> signed in to this account: %others', compact('others')));
  }
  
  if ($mya->isCtty) { // managing a community itself
    $sql = <<<EOF
      SELECT
      SUM(IF(:IS_OK AND :IS_CO, 1, 0)) AS cos,
      SUM(IF(:IS_OK AND NOT :IS_CO, 1, 0)) AS mems,
      SUM(IF(NOT :IS_OK AND :IS_NONUDGE, 1, 0)) AS closed,
      SUM(IF(NOT :IS_OK AND NOT :IS_NONUDGE, 1, 0)) AS inProcess
      FROM users WHERE community=:ctty AND uid<>:ctty
EOF;
    $res = db\q($sql, compact('ctty'))->fetchAssoc();
    $res['tot'] = array_sum($res);
//    $count = item(db\count('users', 'community=:ctty AND uid<>:ctty', compact('ctty')), t('Members:'));
    $count = item(t('%tot = %mems members + %cos companies + %inProcess in process + %closed closed', $res), t('Accounts:'));
    $cInfo = be\creditInfo();
    foreach ($fields = ray('balance in rewards out') as $k) {
      $v = $cInfo->$k;
//      if ($k == 'out') $v -= $cInfo->rewards;
//      if ($k == 'r') $v += $cInfo->rewards;
      $$k = u\fmtAmt($v + 0);
    }
    
    $balance = item(t('%balance = %in in - %out out (Credit issued: %rewards)', compact($fields)), t('Funds:'));
  } else { // managing some individual account
    if (!$mya->ok) $mya->nextStep();

    if ($mya->admin and !$mya->proSe) {
      $community = r\cttys($mya->region, $ctty);
      jsx('change-ctty', compact('uid'));
    }

    if ($mya->cAdmin and !$mya->proSe) {
      $html = $mya->photoHtml(TRUE, TRUE);
      $photo = item("<a href=\"$base_url/settings/photo\">$html</a>");

      $donations = item($mya->giftsDesc() ?: t('none'), t('Donations:'));
      
//      $creditLimit = textFld(t('Credit limit:'), '', dft(u\fmtAmt(-$mya->floor)));
//      if ($mya->stepsDone['prefs']) { // don't even look at these if newbie hasn't chosen yet
//        $minimum = textFld(t('Minimum:'), '', dft(u\fmtAmt($mya->minimum)));
//        $achMin = textFld(t('ACH Min:'), '', dft(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's')));
//      }
      $minimum = item(u\fmtAmt($mya->minimum), t('Target Bal:'));
      $achMin = item(u\fmtAmt($mya->achMin), t('ACH Min:'));
//      if (!$mya->stepsDone['dw']) { // show info needed to open a Dwolla account
//      extract((array) $mya->secure, EXTR_PREFIX_ALL, 's'); // dunno why (array) is required (indirect mod error)
      if (!$mya->co) $trust = item($mya->trust, t('Trust:'), t('This person\'s trust rating'));
      $invitedBy = $mya->helper ? ($mya->helperA->fullName . "#$mya->iCode (" . r\iCodeDesc($mya->iCode) . ') + ') : '';
      $invitedBy = item($invitedBy . db\lst('fullName', 'r_invites i LEFT JOIN users u ON u.uid=i.inviter', 'invitee=:uid', compact('uid')), t('Invited by:'));
      $proxies = item(db\lst('fullName', 'r_proxies p LEFT JOIN users u ON u.uid=p.proxy', 'p.person=:uid ORDER BY priority', compact('uid')), t('Proxies:'));
      $proxyFor = item(db\count('r_proxies', 'proxy=:uid', compact('uid')), t('Proxy for:'));
      $risk = item(round($mya->risk) . ' ' . button(t('Recalc'), "$base_url/rerisk/acct/$uid/1"), t('Risk Score:'));
      if (!$mya->co) $trust = item(number_format($mya->trust, 1), t('Trust:'));

    if (!$mya->ok) { // don't show helper field if it's moot (too late -- somebody already got credit)
        if ($helper = $mya->helper ?: ($mya->co ? $mya->by : db\get('inviter', 'r_invites', 'invitee=:uid', compact('uid')))) {
          $hA = r\acct($helper);
          $helper = "$hA->mainQid: $hA->fullName";
        }
        $helper = textFld(t('Helped by:'), t('Who gets the "inviter/helper" reward? Must be exactly one account.'), dft($helper)); //  + autocomplete()
      } else $helper = item(@$mya->helperA->fullName ?: t('N/A'), t('Helped by:'));
    }
  
    $activity = item(a\showActivity($mya), t('Activity:'));
    if (!$mya->ok) $steps = item(a\showSteps($mya), t('To do (red):'), t('|sign=agreement | prefs=preferences | connect=connect bank (or not) | company=company info'));
    if ($rels = a\showRelations($mya)) $relations = item($rels, t('Relations:'));  
  }
  
  if ($mya->cAdmin and !$mya->proSe) { // either a community or regular account (but not the cAdmin's own account)
    include __DIR__ . '/../rweb/risk-descs.inc';
    include __DIR__ . '/../rweb/bit-descs.inc';

    $legalName = textFld(t('Legal Name:'), '', required($mya->legalName));    
    $shortName = textFld(t('Short Name:'), t('Shortest usable version of full name.'), dft($mya->shortName));
    
//    if ($mya->superAdmin or ($mya->cAdmin and $mya->activated)) $rTrader = ($mya->isCtty)
    if ($mya->cAdmin) $rTrader = ($mya->isCtty)
    ? boolFld(t('Active?'), '', $mya->ok)
    : boolFld(t('Active?'), t('Is account approved to use %PROJECT?'), $mya->ok);

    if ($mya->member and !$mya->co and $mya->superAdmin) $ssnShow = item(a\ssnShow($mya));
    if ($mya->admin) {
      $dba = ($mya->co and $owner = r\relation('other', 'main=:uid', compact('uid')) and r\acct($owner)->federalId == $mya->federalId);
      $ssnDft = $mya->federalId ? ($dba ? t('SSN') : R_ON_FILE) : '';
      $federalId = $mya->superAdmin ? textFld(t('EIN/SSN:'), '', $ssnDft) : item($ssnDft);
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
    $risks = bitsFld(t('Risks:'), $mya->risks, '', K_ACCT_RISKS, $aliases);
    if ($mya->isCtty) $flagHelp = just(B_CTTY_BITS, $flagHelp);

    $adminable = $mya->adminables();

    foreach (ray($mya->isCtty ? B_CTTY_BITS : B_LIST) as $k) if (!preg_match('/u[0-9]+/', $k)) { // ignore undefined bits
      $blist[] = $k0 = $k;
      $class = in($k, $adminable) ? '' : 'noEdit';
      if ($mya->isCtty) $k = strtr($k, ray(B_ADMIN_BITS));
      $dpy = w\popHelp($k, $flagHelp[$k0], '#');
      $aliases[$k0] = "<div class=\"$class\">$dpy</div>";
    }
    $adminable = bitsFld(t('Bits:'), $mya->flags, 'b', join(' ', $blist), $aliases);
    $calling = item($mya->calling, t('Calling:'));
    $tenure = item($mya->tenure . t(' month(s)'), t('Tenure:'), t('How long had this person or company been at this address before opening a %PROJECT Account?'));

    if (!$mya->co) {
      $moves = item($mya->moves === '' ? '?' : $mya->moves, t('Moves:'), t('(How many times has this person changed counties in the past decade?)'));

//    $who = textFld(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete($admin ? '' : 'ctty')); // keep this for when we need it (just add to returned fields)
  }      

  if ($mya->id == r\cgfId()) {
    $sql = <<<EOF
      SELECT u.community AS ctty, SUM(IF(payee=:uid, amount, 0)) AS `in`,
        SUM(IF(payer=:uid, amount, 0)) AS `out`
      FROM r_txs t LEFT JOIN users u ON u.uid=IF(payer=:uid, payee, payer)
      WHERE ((payer=:uid AND payee<0) OR payee=:uid) AND t.:IS_GIFT GROUP BY community
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
//    $mediaConx = textFld(t('Media Conx:'), t('(How many 1st or 2nd-degree connections do you have with this person on FaceBook or LinkedIn?)'), dft($mya->mediaConx ?: 0));
  }

  if (!$mya->proSe) {
    list ($tickleType, $zot) = $mya->nextTickle();
    $details = item(a\callDetails($tickleType, $mya->tickle));
//    $preNote = item($tip);

    if ($mya->superAdmin) {
      $notes = areaFld(t('Notes:'), '', dft(unplain($mya->notes))); // only needed for default in areaFld
    } else $notes = item(str_replace("\n", '<br>', $mya->notes), t('Notes:'));

    $note = areaFld(t('New Note:'), [t('Type a Note Here'), t('Abbreviations: PM=phone msg EM=email msg. Be sure to say what you left a message <i>about</i> (for example "PM setup")')]);
    $suffix = submitTickle(1, t('Wait a day')) . submitTickle(2, t('2 days')) . submitTickle(7, '7') . submitTickle(30, '30') . submitTickle('', t('Auto')) . ' &nbsp; ' . submitTickle('NONE', t('Save'), 'sm');
    $tickle = textFld(t('Tickle:'), [t('Followup Date'), t('When to follow up ("Auto" sets an automatic Welcome or Annual date)')], dft($mya->tickle ? u\fmtDate($mya->tickle, TRUE) : '') + w\suffix($suffix));
    $source = item($mya->source, t('Source:'));
  }

//  $back = item('<a href="' . $base_url . '/sadmin/member-list">Go to Member List</a>', ' ');

  $uid = hidFld($uid); // must postcede js (which uses uid)
  w\jsx('tickle');
  
  $bigFields = $mya->ok ? 'adminable risks ssnShow' : 'ssnShow adminable risks';
  return compact(ray("photo details notes note tickle donations rTrader activity relations source helper $bigFields community balance cgfDonations legalName shortName calling signupCo tenure steps dwInfo invited invitedBy proxies proxyFor moves floor minimum achMin risk trust mediaConx count back uid submit"));
}

/**
 * Return a button for setting the tickle field and submitting the form.
 * @param mixed $v: what to set tickle to -- empty or number of days hence or "NONE" (meaning don't set the tickle)
 * @param string $text: what the button says
 * @param string $size: size of button (defaults to sm)
 */
function submitTickle($v, $text, $size = 'xs') {
  if ($v) $v = u\fmtDate(strtotime("+$v days"), TRUE);
  return w\rent(w\submi($text, 'primary', $size, ray('class tickle', 'tickle', $v)));
}

/**
 * Download the member list for the current community
 */
define('EXPORT_PARAMS', 'do requests allCttys status types chimp anonymous');

function export($params) {
  extract(just(EXPORT_PARAMS, $params));
  global $mya;
  $cttyA = r\acct($mya->community);

  $filename = ($allCttys ? PROJECT : str_replace('.', '', $cttyA->name))
  . (@$status == 0 ? t('-active') : (@$status == 1 ? t('-notyet') : ''))
  . (@$types == 0 ? t('-members') : (@$types == 1 ? t('-orgs') : t('-accounts')))
  . '.csv';
  u\csvStart($filename);
  $delimiter = @$chimp ? "\t" : ',';
  
  $fields = @$anonymous ?
    'uid city state zip type'
      . ' created activated signed helper iCode balance rewards savingsAdd minimum share risk trust'
      . ' cttyName co gross employees achMin pinThreshold lastiCode tenure moves mediaConx jid'
      . ' owns bankName slave closed notices secret'
  : (@$chimp ? 'id first_name last_name phone email city state zip cttyName type ecode special created'
  : 'id first_name last_name phone email address city state zip cttyName type ecode special modified created');
  u\csv($fields = ray($fields), $delimiter);

  if ($requests) { // handle requests first, in case member status gets updated by later record
    $startDate = strtotime('8/9/2017'); // change this to -1 month?
    $sql = "SELECT * FROM r_request WHERE email<>'' and created>=$startDate";
    if (!$mya->admin) $sql .= " AND ctty=$mya->community";
    $q = db\q($sql);
    while ($row = $q->fetchAssoc()) {
      extract($row);
      if (db\exists('users', 'mail=:email', compact('email'))) continue; // don't duplicate or send to nonudge people
      list ($id, $first_name, $last_name, $modified) = [$listid, $first, $last, time()];
      $address = $city = $state = $ecode = $special = '';
//      $type = $ctty == 0 ? '' : 'member';
      $type = '';
      foreach (ray('phone email') as $k) $$k = u\decry('P', $$k);
      if (strpos($email, '@' . EMAILX)) continue;
//      $cttyName = $ctty ? trim(str_replace(PROJECT, '', r\acct($ctty)->fullName)) : '';
      $cttyName = ($zip and $ctty = r\communityUid($zip)) ? r\cttyName($ctty, TRUE) : '';
      if (!@$chimp) $zip .= "\t"; // prevent leading zeros from getting dropped on import (fails for MailChimp, so don't)
      u\csv(array_values(compact($fields)), $delimiter);
    }
  }  
  
  // add actual accounts
  $where = @$allCttys ? '1' : 'community=:cttyId';
  if (@$status == 0) $where .= ' AND :IS_OK';
  if (@$status == 1) $where .= ' AND NOT :IS_OK';
  if (@$types == 0) $where .= ' AND NOT :IS_CO';
  if (@$types == 1) $where .= ' AND :IS_CO';

  $sql = "SELECT uid AS id, fullName FROM users WHERE uid>:CANONIC_ACCTS AND NOT :IS_NONUDGE AND (NOT activated OR :IS_OK) AND $where ORDER BY :IS_OK, -signed, -created";

  $q = db\q($sql, ray('cttyId', $cttyA->id));
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($id);
    $type = $a->ok ? ($a->co ? 'member company' : 'member') : ($a->co ? 'company' : '');
    if (@$anonymous) {
      foreach ($fields as $k) $$k = $a->$k; // do this first. some will be null.
      $zip = $a->zip;
      foreach (ray('weekly monthly secret') as $k) $$k = $a->can(u\consta('b', $k));
      $notices = $monthly ? t('monthly') : ($weekly ? t('weekly') : t('daily'));
      $state = r\realstate($state);
      if ($routing = substr($a->bankAccount, 4, 9)) {
        $bankName = db\get('name', 'r_banks', 'route=:routing', compact('routing'));
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
      $special = '';
// (BROKEN)      $special = $a->special;
    }
    $cttyName = r\cttyName($a->community, TRUE);
    $created = u\fmtDate($a->created);
    if (is_numeric($zip) and !@$chimp) $zip .= "\t"; // prevent interpretation as a number (fails for MailChimp, so don't)

    u\csv(array_values(compact($fields)), $delimiter);
  }
  
  exit();
}
