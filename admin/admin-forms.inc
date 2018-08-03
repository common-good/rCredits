<?php
/**
 * @file
 * Admin forms
 */
namespace CG\Web;

use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\DB as db;
use CG\Admin as a;
use CG\Web as w;

include_once __DIR__ . '/admin.inc';
include_once __DIR__ . '/admin-web.inc';
define('T_MSG_FILE', 'test-messages.txt');

/**
 * Display top level Admin options.
 */
function formAdmin($form, &$sta) {return w\subMenu(t('Admin Options'), 'sadmin');}
  
/**
 * Make various administrative operations available to the admin and cAdmin.
'wrong community' => 'SELECT * FROM `users` WHERE uid>0 AND MID(community, 2, 7)<>MID(uid, 1, 7)',
 */
function formAdminPanel($form, &$sta, $args = '') {
  extract(just('qid', $args));
  global $mya;
  if (!$mya->cAdmin) hack('panel');

  $tables = 'users txs usd gifts proxies relations invites'; // tables to show
  $queries = [
    '' => '',
  ];
  
  $title = t('System Administration');
  
  $inviteeStatus = submi(t('Invitee Status'));
  $organizerList = submi(t('Local Requests'));
  $signupCos = submi(t('Connections to Companies'));
  $queries = selectFld(t('Query:'), '', '', $queries);
  $account = textFld(t('Account:'), t('Account to do something to'), dft(@$qid ?: svar('adminingAcct')));
  
  if ($mya->superAdmin) {
    if (r\up()) $stop = submi(t('STOP')); else $start = submi(t('START'));
/*    if (time() < strtotime('6/20/2017')) {
      global $base_url;
      $members = db\col('uid', 'users', ':IS_OK AND NOT :IS_CO AND activated<1486098000 ORDER BY community, uid');
      foreach($members as $k) {
        $list[] = <<<EOF
<a href="$base_url/print-rcard/$k" target="_blank" class="btn btn-xs btn-primary" role="button" onclick="this.style.display='none';">NEXT</a>
EOF;
      }
      $start = item(join('', $list));
    } */
    $do1099btest = submi(t('1099B TEST'));
    $do1099b = submi('1099B');
    $tables .= ' notices boxes do stats usd nonmembers';
    $special = submi(t('Special'));
    $askForAppData = submi(t('Ask for App Data'));
    $seeAppData = submi(t('See App Data'));
    $ssnLink = submi(t('SSN Link'));
    $seeSecure = submi(t('See Secure'));
    $resetSSN = submi(t('Recheck SSN'));
    $make2 = boxFld('make', t('Make:'));
  }

  $printId = submi(t('Print ID'));
//  $connectBank = submi(t('Connect Bank'));
//  $seeHidden = submi(t('See Hidden'));
  $seeData = submi(t('See Data'));
  $seeChanges = submi(t('See Changes'));
  if (!$mya->superAdmin and $ctty = $mya->community) { // not super
    $cttyName = ($cttyA = r\acct($ctty)) ? $cttyA->fullName : NULL;
    if ($mya->cttyUp) $stopCtty = submi(t('STOP ') . $cttyName); else $startCtty = submi(t('START ') . $cttyName);
  }

  foreach (ray($tables) as $one) $do[$one] = submi($one);
  $do += compact(ray('inviteeStatus organizerList signupCos do1099btest do1099b special stop start stopCtty startCtty'));
  $do = fieldSet('do', $do);
  
  $acctSet = fieldSet('acctSet', compact(ray('account make2 printId seeData seeChanges ssnLink seeSecure resetSSN askForAppData seeAppData')));

  return cgform(compact(ray('title do acctSet')));
}

function formAdminPanel_submit($form, &$sta) {
  global $base_url;
  global $mya;
  
  extract(just('account make', $sta['input']));
  $op = op($sta);

  if ($op == 'start' or $op == 'stop') {
    r\up($op == 'start');
    return r\up() ? say(t('System is UP.')) : say(t('System is DOWN.'), 'ERR');
  }
  if ($op == 'special') return a\special();
  if ($up = ($op == 'startCtty') or $op == 'stopCtty') {
    $ctty = r\acct($mya->community);
    $ctty->setBit(B_OK, $up);
    return $up ? say(t('Community is UP.')) : say(t('Community is DOWN.'), 'ERR');
  }

  if (@$account) {
    if (!$a = a($account)) return say('That is not an account.'); // should use whois first
    svar('adminingAcct', $account); // remember what account we're working on
    if (!$mya->admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  } else $a = $mya;
  
  if (@$a and $op == 'printId') {
    if ($a->co and $a->proSe) return say(t('A company cannot have a %PROJECT card (only its agents have %PROJECT cards).'));
    if (@$make) {
      $oldFile = DRUPAL_ROOT . $a->photoFilename(TRUE); // get this before makeCardCode
      $a->makeCardCode(); // this changes photo filename
      if (file_exists($oldFile)) {
        $newFile = DRUPAL_ROOT . $a->photoFilename(TRUE);
        rename($oldFile, $newFile);
      }
    }
    return w\go('print-rcard/' . $a->id . ($a->co ? "/$a->agentId" : ''));
  } elseif ($op == 'askForAppData') {
    $a->update('getAppData appData', '!report', []);
    say(t('Request for app data sent to ') . $a->fullName);
  } elseif ($op == 'seeAppData') {
/**/  debug($a->appData);
  } elseif ($op == 'seeSecure' and @$a) {
      say(u\rayTable(ray('fullName phone dob physicalAddr', $a->fullName, substr(@$a->phone, 2), $admin ? u\fmtDate(@$a->dob) : '', @$a->address . ' ' . $a->zip)));
//      seeHidden($a);
      seeSecure($a);
  } elseif ($op == 'resetSSN' and @$a) {
    $a->setBit(B_MEMBER, FALSE);
    $a->stepDone('ssn', FALSE);
    $a->update('ssnData', '');    
  } elseif ($op == 'seeData') {
    if (@$a and $a->proSe) say(u\rayTable($a->data, FALSE, TRUE));
/*  } elseif ($op == 'connectBank') {
    r\Web\connectBank($a); */
  } elseif ($op == 'seeChanges') {
    w\go("sadmin/changes/$a->mainQid");
  } elseif ($op == 'ssnLink') {
    w\say($a->fullName. t(' SSN Link:<br><br> &nbsp; %PROMO_URL/do/doSsn~%qid~%code &nbsp; ', 'qid code', $a->mainQid, $a->emailCode));
  } elseif ($op == 'do1099b' or $test1099 = ($op == 'do1099btest')) {
    include_once __DIR__ . '/../cg-yearly.inc';
    r\Special\do1099bs(@$test1099);
  } elseif ($op == 'inviteeStatus') {
    return w\go('community/invite/invited-whom');
  } elseif ($op == 'organizerList') {
    return w\go('sadmin/organizer-list');
  } elseif ($op == 'signupCos') {
    global $signupCos; $signupCos = [];
    $crit = $mya->admin ? '1' : 'community=' . $mya->community;
    eachA(function ($a) {
      global $signupCos;
      if ($signupCo = $a->signupCo) {
        extract($signupCo);
        $signupCos[strtolower("$a->community$company$a->id")] = ray('acct fullName relation company', $a->mainQid, $a->fullName, @$isOwner ? t('owner') : ((@$employee or @$employeeOk) ? t('employee') : (@$contractor ? t('contractor') : '')), $company);
      }
    }, $crit . ' ORDER BY community');
    ksort($signupCos);
    w\say($signupCos ? w\showRecords($signupCos) : t('No such companies.'));
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, flags, balance, rewards, savingsAdd AS saveAdd, minimum AS min, floor, city, zip AS zip';
    return say(a\showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,IF(agent=myid,':R_NONE',agent) AS agent, REPLACE(info, ',', ', ') AS info";

/// if ($op == 'log') return say(str_replace("\n", "<br><br>\n", file_get_contents(flog()))); // say(a\showQuery('r_log', 1, 'logid DESC', $logFields, 'myid'));
//  if ($op == 'tells') return say(a\showQuery('r_log', "info LIKE '%tell-staff%' ORDER BY logid DESC LIMIT 50", 'logid DESC', $logFields, 'myid'));

  if ($op == 'txs') {
    $fields = <<<EOF
    xid, t.created, t.flags, t.type AS ty, t.goods AS gd, t.:IS_TAKING AS k,
    IF (LOCATE('"force";s:1:"1"', u1.data)>0, 1, IF(LOCATE('"force";s:2:"-1"', u1.data)>0, -1, IF(LOCATE('"force";s:1:"0"', u1.data)>0, '0', ''))) AS f,
    payer, payee, amount, payerReward, payeeReward, payerFor AS for1, payeeFor AS for2, payerTid AS tid1, payeeTid AS tid2
EOF;
    if ($mya->admin) $fields = str_replace('u1.', '', $fields);
    return say(a\showQuery('r_txs', 1, 'xid DESC', $fields, ['payer', 'payee']));
  }

  if ($op == 'gifts') return say(a\showQuery('r_gifts', "honor<>'share'", 'donid DESC', 'donid,giftDate,t.uid,amount,often,honor,honored,t.share,completed', 'uid'));
  if ($op == 'invites') return say(a\showQuery('r_invites', 1, 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(a\showQuery('r_notices', 1, 'msgid DESC'));
  if ($op == 'proxies') return say(a\showQuery('r_proxies', 1, 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(a\showQuery('r_relations', 1, 'main DESC, reid', 'reid,main,other,otherNum,:IS_DRAW AS draw,permission,:IS_EMPLOYEE AS employee,:IS_OWNER AS owner,:IS_CUSTOMER AS customer,:IS_AUTOPAY AS autopay,code', 'main'));
  if ($op == 'usd') return say(a\showQuery('r_usd', 1, 'created DESC'));
  foreach (ray('boxes do stats nonmembers') as $one) if ($op == $one) return say(a\showQuery("r_$one"));
}

/**
 * Show people interested in rCredits in particular zipcodes.
 */
function formOrganizerList($form, &$sta, $where = '') {
  $title = t('Organizer List');
  if ($where) {
    $where = preg_replace('/[^[0-9 ]/', '', $where); // prevent SQL injection, in case admin is evil
    $where = "zip LIKE '" . str_replace(' ', "%' or zip LIKE '", $where) . "%'";
    $q = db\q("SELECT * FROM r_request WHERE $where");
    while ($row = $q->fetchAssoc()) {
      extract(just('first last phone email zip', $row));
      $phone = u\fmtPhone(u\decry('P', $phone));
      $email = u\decry('P', $email);
      $list[] = "<tr><td>$first $last</td><td>$phone</td><td>$email</td><td>$zip</td></tr>\n";
    }
    $list = join('', $list);
    $list = item(<<<EOF
<table id="organizer-list">
<tr><th>Name</th><th>phone</th><th>email</th><th>zip</th></tr>
$list
</table>
EOF
  );
  } else {
    $subtext = t('Enter a list of zipcodes (or partial zipcodes), separated by spaces.');
    $zips = textFld(t('Zips:'));
    $submit = t('Go');
  }
  return compact(ray('title subtext zips submit list'));
}

function formOrganizerList_submit($form, &$sta) {
  w\go('sadmin/organizer-list/' . $sta['input']['zips']);
}

/**
 * Display dated changes made to an account.
 * @param string $qid: the account QID (default to current account)
 */
function formSeeChanges($form, &$sta, $qid = '') {
  $a = r\acct($qid);
  $title = t('Account Changes for ') . $a->fullName;
  $list[] = '<tr><th>Date</th><th>Field</th><th>Old Value</th><th>New Value</th>';
  $rec = []; // track historical field values

  if ($a->changes) {
    foreach ($a->changes as $dt => $info) {
      $date = u\fmtDate($dt);
      foreach ($info as $k => $v) {
        $newValue = isset($rec[$k]) ? $rec[$k] : a\formatField($k, interpretV($a->$k));
///        if (is_array($v)) $v = print_r($v, 1);
//        while (substr($v, strlen(CRYPT_FLAG)) == CRYPT_FLAG) $v = u\decryptM($v); // u\starts fails here (when encrypted?)
        $rec[$k] = $oldValue = a\formatField($k, interpretV($v));
        $list[] = "<tr><td>$date</td><td>$k</td><td>$oldValue</td><td>$newValue</td></tr>";
      }
    }
  } else $list[] = '<tr><td colspan=4>No changes</td></tr>';
  
  $list = join("\n", $list);
  $list = item("<table>$list</table>");
  
  return cgform(compact(ray('title list')));
}

function interpretV($v) {return u\crypted('V', $v . '') ? (r\allKnowing() ? u\decry('V', $v) : t('(secret)')) : $v;}

/**
 * Log back in to admin automatically.
 */
function formTestRecovery($form, &$sta, $doSay = FALSE) {
//  if (isDEV and !r\acct()) { // called after testing, in case session got destroyed
  if (!isDEV) return;
    //$uid = \user_authenticate('admin', DEV_ADMIN_PASS);
	$mya = r\acct(1);
	global $user; $user = $mya->account('uid pass timezone'); // timezone keeps Drupal happy in bootstrap.inc
	\drupal_session_regenerate();
	w\setAcct($mya->id);
	if ($msgs = @unserialize(@file_get_contents(T_MSG_FILE))) {
		unlink(T_MSG_FILE);
		\drupal_get_messages(); // throw out any existing messages
		$_SESSION['messages'] = $msgs;
	}
	return w\go('sadmin');
}

/**
 * Create a new community or region.
 */
function formMakeCtty($form, &$sta) {
  $title = t('Make a New Community');
  $subtext = t('<p>Typically the name of a %PROJECT community is the place name followed by "%PROJECT"; for example, "<b>Greenfield Area %PROJECT</b>". Legal names should include the state (for disambiguation), spelled out; for example, "Greenfield Massachusetts %PROJECT".</p><p>By contrast, region names begin with "%PROJECT"; for example, "<b>%PROJECT Western Mass</b>".</p><p>Email should reflect the legal name, for example "<b>greenfield@ma.%CG_DOMAIN</b>".</p>');
  $region = textFld(t('Region:'), [t('Region code'), t('3-letter code of community\'s region')], required());
  $zips = textFld(t('Zip Regex:'), [t('Zipcodes to include'), t('|Type a regular expression for all zipcodes to be included (by default) in the community. For example, ^013|01002 would include all zipcodes starting 013 plus zipcode 01002. Individual members can also be moved manually in or out of this community.')]);

  // from w\contactFields
  $phone = textFld(t('Phone:'), [t('Contact phone number')], required(@$phone ? u\fmtPhone($phone): ''));
  $postalAddr = textFld(t('Postal Addr:'), [t('Complete mailing address'), t('Where does the post office send your mail (usually)?')], required(@$postalAddr));

  $submit = submit();
  
  $form = compact('title', 'subtext') + nameAndEmail(t('Community')) + ssnFields(FALSE)
    + compact(ray('phone postalAddr region zips submit'));
//         + ssnFields(FALSE) + contactFields(['zot'=>0], FALSE) // prevent contact fields from defaulting
//  foreach (ray('tenure owns') as $k) unset($form[$k]);
  return cgform($form);
}

function formMakeCtty_validate($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract(u\normalizeCase(just($normals = 'fullName legalName postalAddr', $sta['input'])));
  extract(just('zips region email phone federalId', $sta['input']));

  if (preg_match("~$zips~", null) === false) return say(t('bad Zips REGEX'), 'zips');
  if (!$region or !preg_match('/[A-Z]{3}/i', $region)) return say(t('bad region code'), 'region');

  if (!@$fullName) $fullName = $legalName;
  foreach (ray('fullName legalName') as $field) {
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }
  if (!emailOkay($email, $mya->co, $mya)) return; 
  if (!phoneOk($phone)) return;
//  $name = strtolower(substr($email, 0, strpos($email, '@')));
  $name = u\shortName($fullName);
  if ($otherName = db\get('fullName', 'users', 'name=:name', compact('name'))) return say('shortname taken', compact('otherName'), 'fullName');
//  $name = r\uniqueName($fullName);

  if ($err = u\badSsn($federalId)) return say($err, ['what' => 'federalId'], 'federalId');
  
  list ($address, $city, $state) = u\parseAddr($postalAddr); // zip doesn't apply, since it is used for defining the territory
  if (!$address) return say('Bad postal address', 'postalAddr');
  if (!$state = db\get('id', 'r_states', 'abbreviation=:state', compact('state'))) return say('Bad state', 'postalAddr');

  $region = strtoupper($region);
  if (!db\exists('r_regions', 'region=:region', compact('region'))) return say(t('That region does not exist.'), 'region');

  u\preray(compact(ray($normals . ' name email phone address city state region federalId')), $sta['input']); // fix formatting
}

function formMakeCtty_submit($form, &$sta) {
  $fields = 'name fullName legalName email phone federalId postalAddr address city state';
  extract(just('zips region ' . $fields, $sta['input']));

  $zip = $zips; // store the regex defining the region or community
  $flags = u\bit(B_CO) | u\bit(B_OK) | u\bit(B_UP);
  $rebate = 0;
  $country = R_COUNTRY_ID;
  $region = r\qo("!$region")->id;
  $type = CO_PARTNERSHIP;
  $fields .= ' zip flags rebate region address state country type';

  $DBTX = \db_transaction();
  $info = compact(ray($fields));
  
  if (!$a = new r\Acct($info)) return say('cannot create account', 'zot');
  $a->update('community', $a->id);
  
  unset($DBTX); // commit
  say('info saved');
}

/**
 * Show a list of standard calls to make (to help new members, check-in, ask for donation, etc.)
 * @param int $uid: (optional) account to deal with
 * @param bool $one: (optional) <show just my community>
 * @param bool $recent: (optional) <show latest notes first>
 */
function formFollowup($form, &$sta, $args = '') {
  global $base_url;
  global $mya;
  
	if (!$mya or !$mya->cAdmin) return w\go('');
  extract(just('uid one recent', $args));
  
  if (@$uid) {
    w\setAcct($uid); // switch to that user
    return formSummary($form, $sta);
  }

  $title = t('Admin Action Page');
  $subtitle = item(t('<p>Start with today and work backward. Click an account name.</p><ul><li><b>Live Person.</b> If you get a live person, click "live" for a script. Then, if no followup is needed, make a note about your conversation and click "Auto".</li><li><b>Message #1.</b> If you get a machine, click "msg1" for a Phone Message script, type "PM setup" or "PM welcome" or whatever as a note, and click the "2 days" button.</li><li><b>Message #2.</b> If you already left one message, try 3 more times to catch them live. If you still don\'t catch them click "msg2" for a Phone Message script, then click "email" to send them an Email Message. Type "EM setup" or whatever and click the "7" button if this is for a Setup or Annual check-in or the "Auto" button if it is for a Welcome.</li><li><b>After Setup Help Email.</b> If the email gets no response for a setup help call, keep phoning or emailing at 1-, 2-, or 7-day intervals until you reach them or we close the account.</li><li><b>After Annual Check-in Email.</b> If the email gets no response for an Annual check-in, send a US Mail fund appeal/check-in.</li></ul><p>(comments on this standard procedure are welcome)</p>') . w\btn('/sadmin/followup/one=1', t('Show just my community')) . ' &nbsp; ' . w\btn('/sadmin/followup', t('Show ALL')) . ' &nbsp; ' . w\btn('/sadmin/followup/recent=1', t('Recent notes first')));
  
  $now = time();
  $todayEnd = strtotime('tomorrow') - 1;
  list ($where1, $where2) = (($mya->admin or $mya->isAdmin2) and !@$one) ? [1, 1] : [0, "community=$mya->community"];
  $cttys = r\cttyList();
  $cttys[key($cttys)] = t('Seedpack');
  foreach ($cttys as $k => $v) $cttys[$k] = trim(str_replace(PROJECT, '', $v));
  
  $by = @$recent
  ? "notes IS NULL OR notes='' OR MID(notes,1,6)='knows ', (MID(notes,6,4)*37+LOCATE(MID(notes,3,3),'JanFebMarAprMayJunJulAugSepOctNovDec'))*32+MID(notes,1,2) DESC"
  : "($todayEnd - IF(tickle>0, tickle, u.created)) * IF(tickle>$todayEnd, -5 * 366 * :DAY_SECS, 1)"; // today's calls first, then yesterday, 2 days ago etc., then tomorrow, the next day, etc.
  if ($mya->superAdmin) $by = "(notes LIKE '%ZAP%' AND BINARY notes LIKE '%ZAP%') OR (!activated AND :IS_MEMBER) DESC, " . $by;
  
  $sql = <<<EOF
    SELECT DISTINCT uid, community, tickle, fullName, phone, u.data, u.created, activated, notes, :IS_CO AS co, t.created AS lastTx, :IS_MEMBER AS member, :IS_OK AS ok, :IS_CARDED AS carded
    FROM users u LEFT JOIN (SELECT created,payer,payee FROM r_txs ORDER BY created DESC LIMIT 1) t ON uid IN (payer, payee)
    WHERE uid>:CANONIC_ACCTS AND NOT :IS_NONUDGE AND NOT (activated and NOT :IS_OK) AND $where2 AND (tickle>0 OR NOT :IS_MEMBER
      OR t.created<$now-60*:DAY_SECS OR t.created IS NULL)
    ORDER BY $by
EOF;

  $fields = ray('ctty type call name phone notes');
  if (@$one) array_shift($fields);
  $q = db\q($sql);
  $guts = $heads = '';
  foreach ($fields as $k) {
    $heads .= "<th class=\"hd-$k\">" . ucfirst($k) . '</th>';
    $guts .= "<td>$k</td>";
  }
  $list = "<table id=\"memberlist\"><tr>$heads</tr>\n";
  $emails = '';
  $whities = ray('zap fu'); // use white font for these backgrounds
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    foreach (['tickle', 'activated'] as $k) if ($$k) $$k = strtotime('today', $$k); // round down to nearest day
    $days = round(($tickle - $activated) / DAY_SECS); // tickle: days since activation
    list ($type, $color) = !$activated 
    ? (strpos($notes, 'ZAP') 
      ? [t('DELETE'), 'zap']
      : ($member ? [t('activate'), 'zap'] : (strpos($data, '"signup";b:0') ? [t('pre'), 'pre'] : [t('setup'), 'setup']))
      )
//    : (round(($activated + TICKLE_WELCOME - $tickle) / DAY_SECS) == 0 ? [t('welcome2'), 'welcome']
//    : (round(($activated + TICKLE_DELAYED - $tickle) / DAY_SECS) == 0 ? [t('welcome6'), 'welcome']
    : (strpos($notes, 'ZAP') ? [t('DEACTIVATE'), 'zap']
		: (!$carded ? [t('print card'), 'print-card']
    : (between($days, TICKLE_WELCOME - 1, TICKLE_WELCOME + 14) ? [t('welcome2'), 'welcome']
    : (between($days, TICKLE_DELAYED - 1, TICKLE_DELAYED + 14) ? [t('welcome6'), 'welcome']
    : (($activated < strtotime('9/2/2017') and $tickle < $activated + 90 * DAY_SECS and !$notes) ? [t('welcome'), 'welcome']
    : (($tickle and $days % 365 < 30) ? [t('annual'), 'annual'] // +/-:2/29
//    in_array(($tickle - $activated) % DAY_SECS, [0, 1, DAY_SECS - 1])) ? [t('annual'), 'annual'] // +/-:2/29
    : (($lastTx < $now - 60 * DAY_SECS and $now - $activated > 60 * DAY_SECS) ? [t('dormant'), 'dormant']
    : ($tickle ? [t('followup'), 'fu']
    : ( [t('other'), 'fu'] )))))))));

    if ($mya->superAdmin and $type == t('DELETE')) $type = w\lnk("$base_url/sadmin/delete-account/$uid", $type);
    if (in($color, $whities)) $color .= ' fu-whitey';
    $type = "<span class=\"fu-$color\">$type</span>";
    $call = u\fmtDate($tickle ?: $created);
    $name = w\lnk("$base_url/change-account/acct=$uid&page=summary", $fullName ?: $uid);
    if ($co) $name = "<span class=\"co\">$name</span>";
    $phone = u\fmtPhone(u\decry('P', $phone), '-');
    $notes = strtr($notes, ["\n" => '|| ', date('Y') => '']);
    if ($type == t('pre')) $notes = db\get('code', 'r_relations', 'other=:uid AND :IS_CUSTOMER', compact('uid')) . " $notes";
    $notes = substr($notes, 0, 200) . (strlen($notes) > 200 ? '...' : '');
    $ctty = $cttys[$community];
    $list .= strtr("<tr>$guts</tr>\n", compact($fields));
  }
  if (!isset($tickle)) $list .= t('There are no followup calls to be made.');
  $list .= "</table>\n";
  
  return cgform(compact(ray('title subtitle list')), '', FALSE);
}

/**
 * Show a list of members in the current account's community
 */
function formMemberList($form, &$sta) {
  global $base_url;
  global $mya;

  $cAdmin = $mya->cAdmin;
  $ctty = r\acct($mya->community);

  $title = (($cAdmin and $ctty) ? $ctty->fullName : '') . t(' Member List');
  $sql = 'SELECT uid, login, access, fullName, notes, :IS_CO AS co FROM users WHERE uid>:CANONIC_ACCTS AND community=:cttyId AND NOT :IS_NONUDGE ORDER BY :IS_OK, IF(:IS_OK, fullName, 0-login)'; // -login fails because it gets treated as a string
  $q = db\q($sql, ray('cttyId', $ctty->id));
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $nameClass = $co ? 'name co' : 'name';
    $notes = str_replace("\n", "<br>\n", $notes);
    $notes = str_replace('do:', '<b class="memberlist-notes">do:</b>', $notes);
    $a = r\acct($uid);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone0 = $a->phone));
    $postalAddr = $a->postalAddr;
    $email = $a->email;
    $fullName = "<a href=\"$base_url/change-account/acct=$a->mainQid&page=summary\">$fullName</a>";
    $steps = a\showSteps($a);
    $date = u\fmtDate($a->ok ? $access : $login);

    if ($cAdmin) {
      $bits = []; // zap previous value
      foreach (ray('member ok bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
      $bits = $bits ? ('<span class="memberlist-bits"> ' . join(' ', $bits) . "</span><br>\n") : '';
    } else $bits = '';
    
    $one = "<tr><td colspan=4 class=\"memberlist-line2\">$date: $steps$bits$notes</td></tr>\n";
    $list .= <<<EOF
<tr>
<td class="$nameClass">$fullName</td>
<td><a href="mailto:$email" target="_blank">$email</a></td>
<td>$phone</td>
<td>$postalAddr</td>
</tr>
$one
EOF;
    $emails .= "$email, ";
  }
  $list .= "</table>\n";
  
  $emails = $cAdmin ? item($emails, t('All emails')) : NULL;

  return cgform(compact(ray('title list emails')), '', FALSE);
}

/**
 * Download the member list for the current community
 */

function formDownloadMemberList($form, &$sta, $args = '') {
  global $base_url;
  extract($params = just(EXPORT_PARAMS, $args));
  if (@$do == 'export') return w\export($params);

  if (@$do == 'done') {
    $do = 'export';
    $params = http_build_query(compact(ray(EXPORT_PARAMS)));
    $download = item("<iframe src=\"$base_url/sadmin/export-list/$params\" class=\"hidden\"></iframe>");
  }
  
  global $mya;
  $cttyA = $mya->cttyA;

  $title = t('Download Member List');
  $expMo = r\rdoMonth();
  $subtext = t(<<<EOF
<p>Generate a CSV format spreadsheet for accounts in your community (%cttyName). We recommend you use a service like MailChimp and import this list there before each mailing. Company accounts and members are marked, so there is generally no need to export them separately &mdash; you can do that separation within the mailing program.</p>
<p>Action links for individual accounts take the form %code, where expMonth is the number of an expiration month (the current month is number %expMo). For example, a link from MailChimp to the democracy page, expiring next month, would be:</p>
<blockquote>%example</blockquote>
<p>Other action choices might be listed here someday. :)</p>
EOF
  , 'cttyName code expMo _example', $cttyA->fullName, 'do/action~qid~ecode~monthNum', $expMo, $base_url . '/do/doEvents~*|ID|*~*|ECODE|*~' . ($expMo + 1));
  
  $chimp = boolFld(t('Exporting to MailChimp?'), '', TRUE);
  
  if ($mya->admin) $allCttys2 = boxFld('allCttys', t('All communities:'), '', TRUE); // temporary for CGF oversight
  $requests2 = boxFld('requests', t('Include requests:'), t('Include non-members who have requested an invitation 8/9/2017 or later.'), TRUE);
  $status = w\radiosFld(t('Status:'), '', dft(2), ray('active not-yet-active both'));
  $types = w\radiosfld(t('Types:'), '', dft(2), ray('individuals companies both'));
  $anonymous = boxFld('anonymous', t('Anonymous:'), t('Export only non-identifying information (for data-mapping and analysis)'), FALSE);
  $chimpSet = w\fieldSet('fields', compact(ray('allCttys2 requests2 status types anonymous')));
  
  $submit = t('Download');
  
  w\jsx('chimp');
  return cgform(compact(ray('title subtext chimp chimpSet submit download')));
}

function formDownloadMemberList_submit($form, &$sta) {
  extract(just(EXPORT_PARAMS, $sta['input']));
  $do = 'done';
  return w\go('sadmin/export-list/' . http_build_query(compact(ray(EXPORT_PARAMS))), t('Download initiated.'));
}

/**
 * Generates the execute form.
 */
function formPhp($form, &$sta, $res = '') {
  $res = item($res);
  $code = areaFld('', '', dft(\variable_get('code')));
  $submit = t('Execute Code');
  return cgform(compact('res', 'code', 'submit'), '', FALSE);
}

/**
 * Process PHP execute form submissions.
 */
function formPhp_submit($form, &$sta) {
  extract(just('code', $sta['input']));
  \variable_set('code', $code); // remember for next time
  $code = <<<EOF
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Admin as a;
use CG\Web as w;  
use CG\Cron as cr;
use CG\Smart as s;
use CG\Vote as v;
$code
EOF;
  
  if (!isPRODUCTION) $code = "use CG\Testing as t;\n" . $code;
  ob_start();
  eval($code);
  $res = ob_get_clean();
/**/  \drupal_set_message(print_r($res, 1));
//  w\go('sadmin/php/');
}

//define('DEPOSIT_DATE1', 1411517529); // first deposit date

/**
 * Offer to print checks or a deposit slip for transfers to or from member bank accounts.
 */
function formDeposits($form, &$sta, $deposit = 0) {
  global $mya, $base_url;
//  if ($deposit) return w\go("sadmin/deposit/$deposit");

  $title = t('Bank Transfers');

  $fields = <<<EOF
    SUM(IF(amount<0, amount, 0)) AS amountOut,
    SUM(IF(amount>0, amount, 0)) AS amountIn,
    SUM(IF(amount<0, 1, 0)) AS countOut,
    SUM(IF(amount>0, 1, 0)) AS countIn
EOF;
  $res = db\records($fields, 'r_usd', "deposit=$deposit");
//  if ($res[0]) {
  extract((array) $res[0]);
  foreach (ray('amountIn amountOut') as $k) $$k = u\fmtAmt($$k);
  $subtext = t('<b>New IN:</b> &nbsp;%countIn (%amountIn)<b class="newout">New OUT:</b> %countOut (%amountOut)', compact(ray('countIn amountIn countOut amountOut')));
  if ($mya->superAdmin) {
    $go = w\btn("/sadmin/checks/way=In&date=$deposit&mark=1", t('Print Checks'))
    . ' ' . w\btn("$base_url/sadmin/achs/date=$deposit&mark=1", t('Download ACH file'));
    $go = item($go, ' ');
  }
//  } else $subtext = item(t('There are no new transfer requests.'));
  
/*
  $ways = [t('IN') => '(amount>0 AND txid>0)', t('OUT') => '(amount<0 AND txid>0)']; // , t('BAD') => 'txid<0'];
  if (r\acct()->superAdmin) foreach ($ways as $way => $where) {  
    $count = db\count('r_usd', $where . ' AND deposit=0');
//    $previous = db\get('MAX(deposit)', 'r_usd', $where);
//    if ($previous == 1) $previous = 0; // handle transition from Dwolla
//    $oldCount = $previous ? db\count('r_usd', $where . ' AND deposit=:previous', compact('previous')) : 0;
    $count = item(t('Checks') . " $way: $count");
//    if ($previous) $reprint = boxFld('', t('Include/Reprint ') . $oldCount . t(' checks from ') . strftime('%d-%b', $previous) . '?'); else unset($reprint);
// fails    if ($count or $previous) $submit = submit(t('Print checks ' . $way), 'primary', 'md', w\away());
//    if ($count or $previous) 
    $submit = submit($way == t('IN') ? t('Print checks IN') : t('ACH Batch OUT'));
//    $previous = hidFld($previous);
///    echo "\n\n\nway=$way " . print_r(u\prefixKeys($way, compact(ray('count reprint submit previous'))), 1) . "\n\n\n";
    $$way = w\fieldSet("_$way", u\prefixKeys($way, compact(ray('count submit'))), BARE);
  }
  */
  
  $deposits = item(a\deposits(), ' ');
  w\jsx('deposits');

  return cgform(compact(ray('title subtext go deposits')));
}
/*
function formDeposits_submit($form, &$sta) {
  $op = w\op($sta);
  $way = str_replace('submit', '', $op); // cute. INsubmit or OUTsubmit -> IN or OUT
//  $previous = @$sta['input'][$way . 'previous'];
//  w\go("sadmin/checks/way=$way&date=0&previous=$previous&mark=1");
  $func = ($way == 'IN' or time() < strtotime('1/10/2017')) ? 'checks' : 'achs';
  w\go("/sadmin/$func/way=$way&date=0&mark=1");
//  foreach (['reprint', 'previous'] as $k) $$k = @$sta['input'][$way . $k];
//  w\go("sadmin/checks/way=$way&date=0&previous=$previous&mark=1&reprint=" . @$reprint);
}
*/
function depositDetails($args) {
  extract(just('way date total count only', $args));
  include_once __DIR__ . '/../pdf.class';
  $pdf = new r\Pdf();

  if (!$in = (strtoupper($way) == 'IN')) $total = str_replace('-', '', $total);

  $m = .5; // left/right margin size
  $colW = 1.25; // amount column width
  $pdf->setPageUnit('in'); // measure everything in inches
  $pdf->SetMargins($m, $m, $m);  // left/top/right margin, needed for auto page breaks
  $pdf->SetAutoPageBreak($count > ($in ? 47 : 9), $m+.1); // leave space at bottom (count compensates for TCPDF bug)
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(TRUE);
//  $pdf->setLanguageArray($l); // (eg "page" in footer)

//  list ($activity, $where) = $in ? [t('DEPOSIT'), '(NOT (amount<0 XOR txid<0))'] : [t('CASHOUT'), '(amount<0 XOR txid<0)'];
  $activity = t('Bank Transfer');
  $where = 1;
  $today = $date == 1 ? t('(Historical)') : u\fmtDate($date ?: r\rTime());
  $escrow = $in ? ESCROW_IN : ESCROW_OUT;
  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['........ A5990D1155A '.$escrow.'C', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
//  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['ACHes', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  $pdf->AddPage();

  $left = t(<<<EOF
<h1><b>%activity Details</b><br>
%today</h1><br>
<br>
<br>
<b>%CGF_LEGALNAME<br>
dba %BANK_DBA</b><br>
%CGF_POSTALADDR<br>
%CGF_PHONE<br>
<br>
<br>
<b>%BANK_NAME</b><br>
Account #%escrow<br>
%BANK_ADDR<br>
<br>
<br>
<b>TOTAL: %total</b><br>
%count checks<br>
EOF
  , compact(ray('activity today total count escrow')));

  $pdf->say($left, $m, 0);
//    SELECT ABS(amount) AS amount, txid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payee 

  $sql = <<<EOF
    SELECT amount, txid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payee 
    WHERE $where AND deposit=$date ORDER BY txid
EOF;
  $q = db\q($sql);
  $x = $pdf->pageW / 2 - 1;
  $pdf->setY($m);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
// (needs htmlcell instead of cell)   if (!$in) $name = r\acct($uid)->bankInfoLink();
//    if (!$in) $name = r\acct($uid)->legalNameDpy;
    $pdf->setX($x);
    if (@$only and ($amount > 0 xor $in)) continue;
    if ($amount < 0) $pdf->SetTextColor(255, 0, 0);
    $pdf->Cell(1, 0, u\fmtAmt($amount, ''), '', 0, 'R');
    $pdf->SetTextColor(0);
    $pdf->setX($x + $colW);
    $pdf->Cell($x - $colW, 0, "(# $txid) $name", '', 1);
    if (FALSE) { // !$in) { // for outgoing checks, show bank address and member's legal name
      $pdf->setX($x + $colW);
      if ($bankInfo = r\acct($uid)->bankInfo) {
        extract($bankInfo, EXTR_PREFIX_ALL, 'b');
        $b_phone = u\fmtPhone($b_phone);
        $pdf->Cell($x, 0, $b_name, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, $b_address, '', 1);
        $pdf->setX($x + $colW);
        $pdf->Cell($x, 0, "$b_city, $b_state $b_zip ($b_phone)", '', 1);
      } else $pdf->Cell($x, 0, 'NO BANK ADDRESS', '', 1);
      $pdf->newLine(); // a blank line between
    }
  }
  $pdf->setX($x);
  $pdf->Cell(1, 0, '----------------', '', 1, 'R');
  $pdf->setX($x);
  $pdf->Cell(1, 0, $total, '', 0, 'R');
  $pdf->setX($x + $colW);
  $pdf->Cell(($pdf->pageW - $x) - $colW, 0, t('TOTAL'), '', 1);

  $date = strftime('%Y%m%d', $date);
  if (u\test()) $pdf->Close(); else 
//  $pdf->Output(PROJECT . "-deposit-$way-$date.pdf", 'I'); //Close and output
  $pdf->Output(PROJECT . "-deposit-$date.pdf", 'I'); //Close and output
}

/**
 * Create an ACH Batch file for all relevant transfer requests.
 */
function achBatch($args) {
  include DRUPAL_ROOT . '/rcredits/rcron/rcron-achify.inc';
  extract(just('way date mark', $args));
//  achify("deposit=$date");
  achify("deposit=$date AND amount<0"); // temporarily only money going OUT
  if (u\test()) return;
}
/*
  extract(just('way date mark', $args));
  $ways = ['IN'=>'(amount>0 AND txid>0)', 'OUT'=>'(amount<0 AND txid>0)', 'BAD'=>'txid<0'];
  $where = $ways[strtoupper($way)];
  $q = db\q("SELECT txid,created,payee,amount,deposit,bankAccount FROM r_usd WHERE deposit=:date AND $where ORDER BY created, txid", compact('date'));
  if (!$q->rowCount()) return w\go('sadmin/deposits', t('There are no transactions for that ACH batch.'));

  $date = strftime('%Y%m%d', $date ?: time());
  u\csvStart("CGach$way-$date.csv");
  while ($tx = $q->fetchAssoc()) {
    extract($tx);
    $a = r\acct($payee);
//    $a->fixTxBankAccount($bankAccount, $txid);
    $bankAccount = u\decry('V', $bankAccount); // 2018-03-16
    $routing = substr($bankAccount, 4, 9);
    $account = substr($bankAccount, 4 + 9);
    $fullName = substr($a->fullName, 0, 22); // Citizens Bank limits this field to 22 chars
    $amount = number_format(abs($amount), 2);
    $addenda = substr(PROJECT . " tx#$txid", 0, 79); // Citizens Bank limits this field to 79 chars
    $row = [$payee, $fullName, $routing, $account, $amount, $addenda];
    u\csv($row);
    if (!$deposit and $mark) db\update('r_usd', ray('deposit txid', strtotime('today'), $txid), 'txid');
  }
  
  if (u\test()) return;
  exit();
}
*/

/**
 * Create a PDF of all relevant checks.
 * @param string $way: IN (from member bank account to rCredits) or OUT (from rCredits to member bank account)
 * @param int $date: deposit/check date
 * @param bool $reprint: UNUSED whether to include a reprint of the previous deposit's checks
 * @param int $previous: UNUSED date of previous deposit
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printChecks($args) {
  extract(just('way date mark', $args));
  include_once __DIR__ . '/../pdf.class';
  $ck = new r\Pdf();
  $ck->setPrintHeader(FALSE);
  $ck->setPrintFooter(FALSE);
  $ck->setPageUnit('in'); // measure everything in inches
//  $ck->SetMargins(0, 0, 0); // left/top/right margin
  $ck->SetAutoPageBreak(FALSE);

  $ways = ['IN'=>'(amount>0 AND txid>0)', 'OUT'=>'(amount<0 AND txid>0)', 'BAD'=>'txid<0'];
  $where = $ways[strtoupper($way)];
  $q = db\q("SELECT txid,created,payee,amount,deposit,bankAccount FROM r_usd WHERE deposit=:date AND $where ORDER BY created, txid", compact('date'));
  $pos = 3; // before first position of 3, on 3-up check paper
  $count = 0;
  while ($tx = $q->fetchAssoc()) {
    if ($pos == 3) {$pos = 1; $ck->AddPage();} else $pos++;
    a\printCheck($ck, $pos, $tx, $mark);
    $count++;
  }
  $date = strftime('%Y-%m-%d', $date);
  if (u\test()) $ck->Close(); else $ck->Output(PROJECT . "-deposit-checks-$way-$date.pdf", 'I'); //Close and output 
//  say($count . t(' checks printed'));
}


/**
 * Print an ID card
 * @param mixed $id: qid or uid of account to make a card for
 * @param mixed $agent: qid or uid of agent
 */
function memberId($id, $agent = '') {
  global $mya;
  require_once __DIR__ . '/../pdf.class';
  if (!$a = r\acct($id, $agent) or (!$mya->admin and $a->community != $mya->community)) exit('No such account ia: ' . $id . ':' . $agent);
  
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  list ($fmt, $region, $tail, $agentCode) = $a->qo->qr();
  $tail = $fmt . $tail . $agentCode;
  
  $acctName = $a->bestName;
  $nameFont = strlen($acctName) > 38 ? 'font-stretch:condensed;'
  : (strlen($acctName) > 33 ? 'font-stretch:semi-condensed;' : '');
  
  if (!$proSe = $a->proSe) {
    if (!@$a->agentA->cardCode2) $a->makeCardCode(); // NOT r\cardCode($a->qid);
    $aa = $a->agentA; // hereafter it's all about the agent
  } else $aa = $a;

  list ($cardFld, $role, $ptQid, $yName, $bg) = $proSe 
  ? ['cardCode', t('Member'), 10, 1.70, 'bg'] 
  : ['cardCode2', t('Company'), '9.5;font-stretch:semi-condensed', 1.64, 'bgCo'];

  $server = isPRODUCTION ? 'RC2.ME' : 'RC4.ME';
  $qrUrl = "HTTP://$region.$server/$tail" . $aa->$cardFld;
  $permitted = ($proSe or B_RELATED + r\relation('permission', $a->id, $aa->id) >= B_BUY);
  $photo = ($aa->hasPhoto and $permitted) ? "@$aa->photo" : (DRUPAL_ROOT . R_PICTURE_DIR . R_NOT_VALID); // '@' tells tcpdf it's and image, not a file; $a->canBuy fails because admin has permission

  list ($wCard, $hCard, $m, $xText) = [3.375, 2.125, .24, .06]; // card dimensions, negative page margin, text indent
//  list ($wPhoto, $mPhoto, $xCardNo) = [1.09, .1, 2.4];
  list ($xProj, $yProj) = [1.54, $m+.04];
  list ($xSite, $ySite, $wPhoto, $mxPhoto, $myPhoto, $xCardNo) = [$xProj+.035, $yProj+.29, 1.1, .085, .09, 2.4];
  list ($cCG, $cSite, $cName) = ['#000065', 'black', '#004000'];
  list ($xTest, $xPhoto, $yPhoto, $hPhoto) = [$m+$wCard-.74, $m+$mxPhoto, $m+$myPhoto, $wPhoto*4/3];

  $style = array( // style for barcode
    'border' => 0,
    'vpadding' => '0',
    'hpadding' => '0',
    'fgcolor' => array(0,0,0),
    'bgcolor' => false, //array(255,255,255)
    'module_width' => 1, // width of a single module in points
    'module_height' => 1, // height of a single module in points
  );

  $pdf = new r\Pdf();
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->SetAutoPageBreak(FALSE);

  $pdf->AddPage();
  $pdf->StartTransform();
  $pdf->ScaleXY(100 * 8.5 / $wCard); // widen to page width (for extra resolution)

  $pdf->Image(__DIR__ . "/../images/idcard/$bg.png", $m, $m, $wCard, $hCard, '', '', 'L', true); // file, x, y, w, h, type, link, align, resize
//  $pdf->Image($photo, $m+$mPhoto, $m+$mPhoto, $wPhoto, $wPhoto*4/3, '', '', 'L', true);
  $pdf->Image($photo, $xPhoto, $yPhoto, $wPhoto, $hPhoto, '', '', 'L', true);
  $pdf->Rect($xPhoto, $yPhoto, $wPhoto, $hPhoto, 'D'); // x, y, w, h, style, border, fill
  $pdf->say(PROJECT, $xProj, $yProj, '', '', "18;Arial MT Medium;color:white;", 'L');
  
  $pdf->say('TM', $m+3, $m+.1, 1, '', "3;color:white", 'L');
  $sp = '<span style="font-size:50%;"> </span>';
  $url = str_replace('.', $sp . '<b style="display:inline; font-stretch:expanded; font-size:120%;">.</b>' . $sp, CG_DOMAIN);
  $pdf->say($url,  $xSite, $ySite, '', '', "7;color:white;font-stretch:normal;", 'L');
  $pdf->write2DBarcode($qrUrl, 'QRCODE,Q', $m+1.445, $m+0.645, .8, .8, $style, 'N'); // L,M,Q,H are low-high error-correction

  $pdf->say($acctName, $m+$xText, $m+$yName, '', '', "12;B;$nameFont", 'L');
  if ($proSe) $pdf->say($qid, $m+$xText, $m+$hCard-.19, '', '', "6.5;color:#666666;letter-spacing:.7px;", 'L');
  if (!$proSe) $pdf->say($aa->fullName, $m+$xText, $m+$yName+0.22, '', '', 8, 'L');

  if (NOT_PRODUCTION) $pdf->say(t('TEST'), $xTest, $m+$hCard-.19, .5, '', '6.5;color:darkred', 'C');

  $pdf->StopTransform();
/*  if (time() < strtotime('6/20/2017')) {
    $pdf->Output(strtolower("$qid.cgcard.pd"), 'D'); // Close and output PDF document ('I')
  } else */
  if ($mya->superAdmin) $a->setBit(B_CARDED);
  if (u\test()) $pdf->Close(); else $pdf->Output(strtolower("$qid.cgcard.pd"), 'I'); // Close and output PDF document ('I')
//  if (time() < strtotime('6/20/2017') and $next = nextMember($id)) w\go("print-rcard/$next");
}

function formSMS($form, &$sta) {
  $title = t('Test SMS');
  $myNumber = svar('lastSMS');
  $number = textFld(t('From (number): '), '', required($myNumber));
  $message = textFld(t('Message: '));
  $submit = t('Send it!');

  focusOn('message');
  $form = compact(ray('title number message submit'));
  return cgform($form);  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(just('number message', $sta['input']));
  $number = u\fmtPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

/**
 * Run acceptance tests.
 */
function test($arg = '') {
  \drupal_get_messages(); // clear messages
  $modules = u\prefixValues('rcredits/', ray('admin rweb rcron rsmart'));
  jsx('test');
  include_once DRUPAL_ROOT . '/vendor/gherkin/test.php';
  file_put_contents(T_MSG_FILE, serialize(\drupal_get_messages()));
//  $_SESSION = [];
  if (ini_get("session.use_cookies")) { // from PHP's session_destroy page
    $params = session_get_cookie_params();
    setcookie(session_name(), '', time() - 42000,
        $params["path"], $params["domain"],
        $params["secure"], $params["httponly"]
    );
  }
  if (session_id() and $_SESSION) session_destroy();
	db\q('DELETE FROM sessions');
  \drupal_goto('sadmin/recover'); // has to be a form available when logged out
}

/**
 * Delete an account (superAdmin only)
 * @param int $uid: account to delete
 */
function deleteAccount($uid) {
	r\deleteAccount($uid);
	return w\go('/sadmin/followup');
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
//  if ($arg == 'reset') r\reset();
  if ($arg == 'reset-menu') \menu_rebuild();
  if ($arg == 'rebuild-db') {
    f('i.rebuildDb');
    if (isDEV) {
      f('t.clear'); 
      say(t('All test data has been deleted.'));
    } else a\setupBasicAccounts();
  }
  if ($arg == 'clear') foreach (ray('cache cache_bootstrap cache_form cache_menu sessions') as $table) {
    db\q("TRUNCATE TABLE $table");
  }
  if ($arg == 'php') w\go('devel/php');
  
  say("DONE with \"$arg\".");
  return w\go('sadmin');
}

function handyLinks() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
    'rcredits/util/reset-menu|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Redo MENUS|menu-hamburger',
    'rcredits/util/rebuild-db|Rebuild tables|REBUILD database|fast-backward',
    'rcredits/util/clear|Empty the cache and sessions tables|Clear Cache & Sessions|unchecked',
  );

/*    'devel/settings|Helper functions, pages, and blocks to assist Drupal developers. The devel blocks can be managed via the block administration page.|Devel settings|cog',
    'devel/cache/clear?destination=sadmin|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache|new-window',
    'devel/reference|View a list of currently defined user functions with documentation links.|Functions|education',
    'devel/elements|View the active form/render elements for this site.|Elements|align-center',
    'devel/phpinfo|View your server&#039;s PHP configuration|PHPinfo()|queen',
    'devel/variable?destination=sadmin|Edit and delete site variables.|Variables|copy',
    */
//    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
//    'rcredits/test?menu=1|Test Menu|',
//    'rcredits/util/changeAgent|Change Agent to Bea|',
//    'sms|Simulate an SMS transaction|Simulate SMS',
//    'admin/config/development/testing|Test|Test',
//    'deletetests.php|Delete old tests|Delete old tests',
//    'rcredits/util/reset|Reset rCredits|',
//    'devel/entity/info|View entity information across the whole site.|Entity info',
//    'devel/php|Execute some PHP code|Execute PHP Code',
//    'devel/field/info|View fields information across the whole site.|Field info',
//(empty array)    'devel/menu/item?path=node|Details about a given menu item.|Menu item',
//    'devel/reinstall?destination=sadmin/handy|Run hook_uninstall() and then hook_install() for a given module.|Reinstall modules',
//    'devel/run-cron|Run cron|Run cron',
//    'devel/session|List the contents of SESSION.|Session viewer',
//    'devel/theme/registry|View a list of available theme functions across the whole site.|Theme registry',

  foreach ($links as $link) {
    list ($url, $detail, $label, $icon) = explode('|', $link);
    if (!$label) $label = $detail;
    $handy[$url] = ['', $label, '', ANY, '', $detail, @$icon];
/*    $result[] = <<<EOF
<div><a href="$url" title="$detail">$label</a></div>
EOF; */
  }
  return w\subMenu(t('<h2>Handy Links</h2>'), $handy);
/*  $result = join(' | ', $result);
  return "<h2 id=\"edit-title\"><br>Handy Links</h2><div class='develw'>\n$result</div>";
  */
}

/**
 * Reinstall all of rCredits
 */
function reinstall() {
  global $base_path;
  list ($buildId, $token) = isDEV ? 
    array('8TlWDkqTg6AuTshAyc8pi8iIVuYkB4Wz4naJd9eCu4s', 'Gmsmx1AKlG1Jsr3y78BMwnXX3TSkN-i-8okCgHkl_Bk')
  : array('lSsN1II4kj5gKQQxkm3bn7t0RVW-ZCntoAIhr7H9XbM', 'RwHRLEIACfa2ICgcLTOfRdDvpxADnpooX3JeSF1KXZg');
  $warning = isDEV ? '' : ('WARNING! This is ' . $_SERVER['HTTP_HOST']);

/**/ echo <<<EOF
    $warning<br><br>
<form action="{$base_path}devel/reinstall?destination=handy" method="post" id="devel-reinstall" accept-charset="UTF-8"><div><div class="form-item form-type-checkboxes form-item-list">
 <div id="edit-list" class="form-checkboxes"><div class="form-item form-type-checkbox form-item-list-block">

 <div class="form-item form-type-checkbox form-item-list-rcredits">
 <input type="checkbox" checked="checked" id="edit-list-rcredits" name="list[rcredits]" value="rcredits" class="form-checkbox" />  <label class="option" for="edit-list-rcredits">rcredits </label>
</div>

<div class="form-item form-type-checkbox form-item-list-rsmart">
 <input type="checkbox" checked="checked" id="edit-list-rsmart" name="list[rsmart]" value="rsmart" class="form-checkbox" />  <label class="option" for="edit-list-rsmart">rsmart </label>
</div>

<div class="form-item form-type-checkbox form-item-list-rsms">
 <input type="checkbox" checked="checked" id="edit-list-rsms" name="list[rsms]" value="rsms" class="form-checkbox" />  <label class="option" for="edit-list-rsms">rsms </label>
</div>

<div class="form-item form-type-checkbox form-item-list-rweb">
 <input type="checkbox" checked="checked" id="edit-list-rweb" name="list[rweb]" value="rweb" class="form-checkbox" />  <label class="option" for="edit-list-rweb">rweb </label>
</div>

</div></div>
<input type="submit" id="edit-submit" name="op" value="Reinstall" class="form-submit" />

<input type="hidden" name="form_build_id" value="form-$buildId" />
<input type="hidden" name="form_token" value="$token" />
<input type="hidden" name="form_id" value="devel_reinstall" />

</form>

EOF;

  exit();
}

