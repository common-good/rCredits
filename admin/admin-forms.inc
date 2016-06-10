<?php
/**
 * @file
 * Admin forms
 */
namespace rCredits\Web;

use rCredits as r;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\db as db;
use rCredits\Admin as a;
use rCredits\Web as w;

include_once __DIR__ . '/admin.inc';
include_once __DIR__ . '/admin-web.inc';

/**
 * Display top level Admin options.
 */
function formAdmin($form, &$sta) {return w\subMenu(t('Admin Options'), 'sadmin');}
  
/**
 * Make various administrative operations available to the admin and cAdmin.
'wrong community' => 'SELECT * FROM `users` WHERE uid>0 AND MID(community, 2, 7)<>MID(uid, 1, 7)',
 */
function formAdminPanel($form, &$sta, $args = '') {
  extract(u\just('qid', $args));
  $mya = r\acct();
  $admin = $mya->admin;
  $tables = 'users txs gifts proxies relations invites'; // tables to show
  
  $title = item(t('System Administration'));
  
  if ($admin) {
    if (r\up()) $stop = submi(t('STOP')); else $start = submi(t('START'));
    $do1099btest = submi(t('1099B TEST'));
    $do1099b = submi('1099B');
    $tables .= ' notices boxes do log stats usd nonmembers';
    $special = submi(t('Special'));
    $askForAppData = submi(t('Ask for App Data'));
    $seeAppData = submi(t('See App Data'));
  }
  if (!$admin and $ctty = $mya->community) {
    $cttyName = @r\acct($ctty)->fullName; // @ for testing
    if ($mya->cttyUp) $stopCtty = submi(t('STOP ') . $cttyName); else $startCtty = submi(t('START ') . $cttyName);
  }
  
  $inviteeStatus = submi(t('Invitee Status'));
  $organizerList = submi(t('Organizer List'));
  $account = textFld(t('Account:'), t('Account to do something to'), (auto() ?: []) + dft(@$qid ?: svar('adminingAcct')));
  if ($admin) $make2 = boxFld('make', t('Make:'));
  $printId = submi(t('Print ID'));
//  $connectBank = submi(t('Connect Bank'));
  $seeSecure = submi(t('See Secure'));
//  $seeHidden = submi(t('See Hidden'));
  $seeData = submi(t('See Data'));
  $seeChanges = submi(t('See Changes'));
  $newUsdEmail = submi(t('New USD Email'));
  $acctSet = fieldSet('acctSet', compact(u\ray('account make2 printId seeData seeChanges seeSecure askForAppData seeAppData')));
  
  $form = compact('title');
  if ($mya->cAdmin) {
    foreach (u\ray($tables) as $one) $do[$one] = submi($one);
    $do += compact(u\ray('inviteeStatus organizerList do1099btest do1099b special stop start stopCtty startCtty'));
    $do = fieldSet('do', $do);
  }
  return labeled(compact(u\ray('title do acctSet')));
}

function formAdminPanel_submit($form, &$sta) {
  $mya = r\acct();
  $admin = $mya->admin; // as distinct from merely cadmin
  
  extract(u\just('account make', $sta['input']));
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
    if (!$admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  }
  
  if (@$a and $op == 'printId') {
    if ($a->co and $a->proSe) return say(t('A company cannot have an rCard (only its agents have rCards).'));
    if (@$make) {
      $oldFile = DRUPAL_ROOT . $a->photoFilename(TRUE);
      r\makeCardCode($a); // this changes photo filename
      if (file_exists($oldFile)) {
        $newFile = DRUPAL_ROOT . $a->photoFilename(TRUE);
        rename($oldFile, $newFile);
      }
    }
    return r\go('/print-rcard/' . $a->id);
  } elseif ($op == 'askForAppData') {
    $gets = u\ray(R_APP_DATA_GETS);
    $a->update('getAppData appData', $gets[0], []);
    say(t('Request for app data sent to ') . $a->fullName);
  } elseif ($op == 'seeAppData') {
/**/  debug($a->appData);
  } elseif ($op == 'seeSecure') {
    if (@$a) {
      say(u\rayTable(u\ray('fullName phone dob physicalAddr', $a->fullName, substr(@$a->phone, 2), $admin ? u\fmtDate(@$a->dob) : '', @$a->address . ' ' . $a->postalCode)));
//      seeHidden($a);
      seeSecure($a);
    }
  } elseif ($op == 'seeData') {
    if (@$a and $a->proSe) say(u\rayTable($a->data, FALSE, TRUE));
/*  } elseif ($op == 'connectBank') {
    r\Web\connectBank($a); */
  } elseif ($op == 'seeChanges') {
    r\go("sadmin/changes/$a->mainQid");
  } elseif ($op == 'do1099b' or $test1099 = ($op == 'do1099btest')) {
    include_once __DIR__ . '/../rcredits-yearly.inc';
    r\Special\do1099bs($test1099);
  } elseif ($op == 'inviteeStatus') {
    return r\go('community/invite/invited-whom');
  } elseif ($op == 'organizerList') {
    return r\go('sadmin/organizer-list');
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, flags, r, rewards, savingsAdd AS saveAdd, minimum AS min, committed AS comit, share, floor, city, postalCode AS zip';
    return say(a\showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,IF(agent=myid,'--',agent) AS agent, REPLACE(special, ',', ', ') AS special, REPLACE(info, ',', ', ') AS info";

  if ($op == 'log') return say(a\showQuery('r_log', 1, 'logid DESC', $logFields));
  if ($op == 'tells') return say(a\showQuery('r_log', "info LIKE '%tell-staff%' ORDER BY logid DESC LIMIT 50", 'logid DESC', $logFields));

  if ($op == 'txs') {
    $fields = <<<EOF
    xid, t.created, type AS ty, goods AS gd, taking AS k,
    IF (LOCATE('"force";s:1:"1"', u1.data)>0, 1, IF(LOCATE('"force";s:2:"-1"', u1.data)>0, -1, IF(LOCATE('"force";s:1:"0"', u1.data)>0, '0', ''))) AS f,
    payer, payee, amount, payerFor AS for1, payeeFor AS for2, payerTid AS tid1, payeeTid AS tid2
EOF;
    if ($mya->admin) $fields = str_replace('u1.', '', $fields);
    return say(a\showQuery('r_txs', isDEV ? 1 : 'type NOT IN (:TX_REBATE,:TX_BONUS)', 'xid DESC', $fields, array('payer', 'payee')));
  }

  if ($op == 'gifts') return say(a\showQuery('r_gifts', "honor<>'share'", 'donid DESC', 'donid,giftDate,t.uid,amount,often,honor,honored,t.share,completed', 'uid'));
  if ($op == 'invites') return say(a\showQuery('r_invites', 1, 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(a\showQuery('r_notices', 1, 'msgid DESC'));
  if ($op == 'proxies') return say(a\showQuery('r_proxies', 1, 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(a\showQuery('r_relations', 1, 'main, reid', 'reid,main,other,draw,permission,employee,isOwner', 'main'));
  if ($op == 'usd') return say(a\showQuery('r_usd', 1, 'created DESC'));
  foreach (u\ray('boxes do stats nonmembers') as $one) if ($op == $one) return say(a\showQuery("r_$one"));
}

/**
 * Show people interested in rCredits in particular zipcodes.
 */
function formOrganizerList($form, &$sta, $where = '') {
  $title = item(t('Organizer List'));
  if ($where) {
    $where = preg_replace('/[^[0-9 ]/', '', $where); // prevent SQL injection, in case admin is evil
    $where = "zip LIKE '" . str_replace(' ', "%' or zip LIKE '", $where) . "%'";
    $q = db\q("SELECT * FROM r_request WHERE $where");
    while ($row = $q->fetchAssoc()) {
      extract(u\just('first last phone email zip', $row));
      $phone = u\fmtPhone(u\decryptN($phone));
      $email = u\decryptN($email);
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
    $subtext = item(t('Enter a list of zipcodes (or partial zipcodes), separated by spaces.'));
    $zips = textFld(t('Zips:'));
    $submit = submit(t('Go'));
  }
  return compact(u\ray('title subtext zips submit list'));
}

function formOrganizerList_submit($form, &$sta) {
  r\go('sadmin/organizer-list/' . $sta['input']['zips']);
}

/**
 * Display dated changes made to an account.
 * @param string $qid: the account QID (default to current account)
 */
function formSeeChanges($form, &$sta, $qid = '') {
  $a = r\acct($qid);
  $title = item(t('Account Changes for ') . $a->fullName);
  $list[] = '<tr><th>Date</th><th>Field</th><th>Old Value</th><th>New Value</th>';
  $rec = []; // track historical field values

  if ($a->changes) {
    foreach ($a->changes as $dt => $info) {
      $date = u\fmtDate($dt);
      foreach ($info as $k => $v) {
        $newValue = isset($rec[$k]) ? $rec[$k] : a\formatField($k, $a->$k);
///        if (is_array($v)) $v = print_r($v, 1);
//        while (u\starts($v, CRYPT_FLAG)) $v = u\decrypt($v);
        $rec[$k] = $oldValue = a\formatField($k, $v);
        $list[] = "<tr><td>$date</td><td>$k</td><td>$oldValue</td><td>$newValue</td></tr>";
      }
    }
  } else $list[] = '<tr><td colspan=4>No changes</td></tr>';
  
  $list = join("\n", $list);
  $list = item("<table>$list</table>");
  
  return compact(u\ray('title list'));
}

/**
 * Log back in to admin automatically.
 */
function formTestRecovery($form, &$sta, $doSay = FALSE) {
  if (isDEV and !r\acct()) { // called after testing, in case session got destroyed
    //$uid = \user_authenticate('admin', DEV_ADMIN_PASS);
    $mya = r\acct(1);
    global $user; $user = $mya->account('uid pass timezone'); // timezone keeps Drupal happy in bootstrap.inc
    \drupal_session_regenerate();
    if ($msgs = @unserialize(@file_get_contents(T_MSG_FILE))) {
      unlink(T_MSG_FILE);
      \drupal_get_messages(); // throw out any existing messages
      $_SESSION['messages'] = $msgs;
    }
    r\go('sadmin');
  }
  
  $title = item(t('Recover Admin'));
  return compact(u\ray('title'));
}

/**
 * Create a new community or region.
 */
function formMakeCtty($form, &$sta) {
  $title = item(t('Make a New Community'));
  $subtext = item(t('<p>Typically the name of an rCredits community is the place name followed by "rCredits"; for example, "<b>Greenfield Area rCredits</b>". Legal names should include the state (for disambiguation), spelled out; for example, "Greenfield Massachusetts rCredits".</p><p>By contrast, region names begin with "rCredits"; for example, "<b>rCredits Western Mass</b>".</p><p>Email should reflect the legal name, for example "<b>greenfield@westernmass.rcredits.org</b>".</p>'));
  $region = textFld(t('Region:'), [t('Region code'), t('3-letter code of community\'s region')]);
  $zips = textFld(t('Zip Regex:'), [t('Zipcodes to include'), t('Type a regular expression for all zipcodes to be included (by default) in the community. For example, ^013|01002 would include all zipcodes starting 013 plus zipcode 01002. Individual members can also be moved manually in or out of this community.')]);

  // from w\contactFields
  $phone = textFld(t('Phone:'), [t('Contact phone number')], required(@$phone ? u\fmtPhone($phone): ''));
  $postalAddr = textFld(t('Postal Addr:'), [t('Complete mailing address'), t('Where does the post office send your mail (usually)?')], required(@$postalAddr));

  $submit = submit();
  
  $form = compact('title', 'subtext') + nameAndEmail(t('Community')) + ssnFields(FALSE)
    + compact(u\ray('phone postalAddr region zips submit'));
//         + ssnFields(FALSE) + contactFields(['zot'=>0], FALSE) // prevent contact fields from defaulting
//  foreach (u\ray('tenure owns') as $k) unset($form[$k]);
  return labeled($form);
}

function formMakeCtty_validate($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\normalizeCase(u\just($normals = 'fullName legalName postalAddr', $sta['input'])));
  extract(u\just('zips region email phone federalId', $sta['input']));

  if (preg_match("~$zips~", null) === false) return say(t('bad Zips REGEX'), 'zips');
  if ($region and !preg_match('/[A-Z]{3}/i', $region)) return say(t('bad region code'), 'region');

  if (!@$fullName) $fullName = $legalName;
  foreach (u\ray('fullName legalName') as $field) {
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }
  if (!emailOkay($email, $mya->co, $mya)) return; 
  if (!phoneOk($phone)) return;
//  $name = strtolower(substr($email, 0, strpos($email, '@')));
  $name = u\shortName($fullName);
  if ($otherName = db\lookup('fullName', 'users', 'name=:name', compact('name'))) return say('shortname taken', compact('otherName'), 'fullName');
//  $name = r\uniqueName($fullName);

  if ($err = u\badSsn($federalId)) return say($err, ['what' => 'federalId'], 'federalId');
  
  $region = strtoupper($region);
  if (!db\exists('r_regions', 'region=:region', compact('region'))) return say(t('That region does not exist.'), 'region');

  u\preray(compact(u\ray($normals . ' name email phone region federalId')), $sta['input']); // fix formatting
}

function formMakeCtty_submit($form, &$sta) {
  $fields = 'name fullName legalName email phone federalId postalAddr';
  extract(u\just('zips region ' . $fields, $sta['input']));

  $postalCode = $zips; // store the regex defining the region or community
  $flags = u\bit(B_CO) | u\bit(B_OK) | u\bit(B_UP);
  $rebate = 0;
  $country = R_COUNTRY_ID;
  $region = r\qo("!$region")->id;
  $fields .= ' postalCode flags rebate region country';

  $DBTX = \db_transaction();
  $info = compact(u\ray($fields)) + acctType(CO_PARTNERSHIP);
  
  if (!$a = new r\Acct($info)) return say('cannot create account', 'zot');
  $a->update('community', $a->id);
  
  unset($DBTX); // commit
  say('info saved');
}

/**
 * Show a list of members in the member's community (all members, for admin)
 */
function formMemberList($form, &$sta) {
  global $base_url;
  $mya = r\acct();

  $cAdmin = $mya->cAdmin;
  $ctty = r\acct($mya->community);

  $title = item((($cAdmin and $ctty) ? $ctty->fullName : '') . t(' Member List'));
  $sql = 'SELECT uid, login, access, fullName, notes, :IS_CO AS co FROM users WHERE uid>1 AND community=:cttyId ORDER BY :IS_OK, IF(:IS_OK, fullName, 2000000000-login)'; // -login fails because it gets treated as a string
  $q = db\q($sql, u\ray('cttyId', $ctty->id));
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $nameClass = $co ? 'name co' : 'name';
    $notes = str_replace("\n", "<br>\n", $notes);
    $notes = str_replace('do:', '<b style="color:blue; font-size:130%;">do:</b>', $notes);
    $a = r\acct($uid);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone0 = $a->phone));
    $postalAddr = $a->postalAddr;
    $email = $a->email;
    $fullName = "<a href=\"$base_url/sadmin/acct-info/$a->mainQid\">$fullName</a>";
    $steps = a\showSteps($a);
    $date = u\fmtDate($a->ok ? $access : $login);

    if ($cAdmin) {
      $bits = []; // zap previous value
      foreach (u\ray('member ok bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
      $bits = $bits ? ('<span style="color:darkgreen;"> ' . join(' ', $bits) . "</span><br>\n") : '';
    } else $bits = '';
    
    $one = "<tr><td colspan=4 style='font-size:70%; font-weight:normal; padding-left:20px;'>$date: $steps$bits$notes</td></tr>\n";
    $list .= <<<EOF
<tr>
<td class="$nameClass">$fullName</td>
<td><a href="mailto:$email" target="_blank">$email</a></td>
<td><a href="tel:$phone0">$phone</a></td>
<td>$postalAddr</td>
</tr>
$one
EOF;
    $emails .= "$email, ";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  $emails = $cAdmin ? item($emails, t('All emails')) : NULL;
  return compact(u\ray('title list emails'));
}

/**
 * Download the member list for the current community
 */

function formDownloadMemberList($form, &$sta, $args = '') {
  global $base_url;
  extract($params = u\just(EXPORT_PARAMS, $args));
  if (@$do == 'export') return w\export($params);

  if (@$do == 'done') {
    $do = 'export';
    $params = http_build_query(compact(u\ray(EXPORT_PARAMS)));
    $download = item("<iframe class=\"hidden\" src=\"$base_url/sadmin/export-list/$params\"></iframe>");
  }
  
  $mya = r\acct();
  $cttyA = $mya->cttyA;

  $title = item(t('Download Member List'));
  $subtext = item(t('for ') . $cttyA->fullName . t('<br>Link is do/op~qid~ecode~expMo. Current Month #') . r\rdoMonth());
  if ($cttyA->name == 'greenfieldarearcredits') $allCttys2 = boxFld('allCttys', t('All communities:')); // temporary for CGF oversight
  $onlyActive2 = boxFld('onlyActive', t('Active only:'), tt('Export only active accounts'));
  $onlyPeople2 = boxFld('onlyPeople', t('People only:'), tt('Export only individual accounts (especially when emailing)'), TRUE);
  $anonymous = boxFld('anonymous', t('Anonymous:'), tt('Export only non-identifying information (for data-mapping and analysis)'), FALSE);
  //$onlyCos = boxFld(t('Companies only:'), tt('Export only company accounts'), FALSE);
  $submit = submit(t('Download'));
  return labeled(compact(u\ray('title subtext allCttys2 onlyActive2 onlyPeople2 anonymous submit download')));
}

function formDownloadMemberList_submit($form, &$sta) {
  extract(u\just(EXPORT_PARAMS, $sta['input']));
  $do = 'done';
  return r\go('sadmin/export-list/' . http_build_query(compact(u\ray(EXPORT_PARAMS))), t('Download initiated.'));
}

/**
 * Generates the execute form.
 */
function formPhp() {
  $code = areaFld('', '', dft(\variable_get('code')));
  $submit = submit(t('Execute Code'));
  return compact('code', 'submit');
}

/**
 * Process PHP execute form submissions.
 */
function formPhp_submit($form, &$sta) {
  extract(u\just('code', $sta['input']));
  ob_start();
  print eval($code);
  \variable_set('code', $code);
  dsm(ob_get_clean());
}

//define('DEPOSIT_DATE1', 1411517529); // first deposit date

/**
 * Offer to print checks or a deposit slip for transfers to or from member bank accounts.
 */
function formDeposits($form, &$sta, $deposit = '') {
//  if ($deposit) return r\go("sadmin/deposit/$deposit");
  $title = item(t('Deposits'));

  foreach ($ways = [t('IN'), t('OUT')] as $way) {  
    $where = $way == t('IN') ? 'amount<0' : 'amount>0';
    $count = db\count('r_usd', $where . ' AND deposit=0');
    $previous = db\lookup('MAX(deposit)', 'r_usd', $where);
    if ($previous == 1) $previous = 0; // handle transition from Dwolla
    $oldCount = $previous ? db\count('r_usd', $where . ' AND deposit=:previous', compact('previous')) : 0;
    $count = item(t('Checks') . " $way: $count");
//    if ($previous) $reprint = boxFld('', t('Include/Reprint ') . $oldCount . t(' checks from ') . strftime('%d-%b', $previous) . '?'); else unset($reprint);
// fails    if ($count or $previous) $submit = submit(t('Print checks ' . $way), 'primary', 'md', ['target' => '_blank']);
    if ($count or $previous) $submit = submit(t('Print checks ') . $way);
    $previous = hidFld($previous);
///    echo "\n\n\nway=$way " . print_r(u\prefixKeys($way, compact(u\ray('count reprint submit previous'))), 1) . "\n\n\n";
    $$way = w\fieldSet("_$way", u\prefixKeys($way, compact(u\ray('count submit previous'))), BARE);
  }
  
  $deposits = item(a\deposits(), ' ');
  return compact('title', $ways, 'deposits');
}

function formDeposits_submit($form, &$sta) {
  $op = w\op($sta);
  $way = str_replace('submit', '', $op); // cute. INsubmit or OUTsubmit -> IN or OUT
  $previous = @$sta['input'][$way . 'previous'];
  r\go("sadmin/checks/way=$way&date=0&previous=$previous&mark=1");
//  foreach (['reprint', 'previous'] as $k) $$k = @$sta['input'][$way . $k];
//  r\go("sadmin/checks/way=$way&date=0&previous=$previous&mark=1&reprint=" . @$reprint);
}

function depositDetails($args) {
  extract(u\just('way date total count', $args));
  include_once __DIR__ . '/../pdf.class';
  $pdf = new r\Pdf();

  if (!$in = (strtoupper($way) == 'IN')) $total = str_replace('-', '', $total);

  $m = .5; // left/right margin size
  $colW = 1.25; // amount column width
  $pdf->setPageUnit('in'); // measure everything in inches
  $pdf->SetMargins($m, $m, $m);  // left/top/right margin, needed for auto page breaks
  $pdf->SetAutoPageBreak($count > ($in ? 47 : 9), $m); // leave space at bottom (count compensates for TCPDF bug)
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(TRUE);
//  $pdf->setLanguageArray($l); // (eg "page" in footer)

  list ($activity, $where) = $in ? [t('DEPOSIT'), 'amount<0'] : [t('CASHOUT'), 'amount>0'];
  $today = $date == 1 ? t('(Historical)') : u\fmtDate($date ?: r\rTime());
  $pdf->setupFooter('', "rCredits $activity $today", ['A5990D1155A '.ESCROW_ACCOUNT.'C', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
  $pdf->AddPage();

  $phone = u\fmtPhone(CGF_PHONE);
  $left = tt(<<<EOF
<h1><b>$activity Details</b><br>
$today</h1><br>
<br>
<br>
<b>@CGF_LEGALNAME<br>
dba rCredits</b><br>
PO Box 21<br>
Ashfield, MA 01330<br>
$phone<br>
<br>
<br>
<b>Citizens Bank</b><br>
Account #@ESCROW_ACCOUNT<br>
Greenfield Branch<br>
89 French King Highway<br>
Greenfield, MA 01301<br>
+1 413 774 2135<br>
<br>
<br>
<b>TOTAL: $total</b><br>
$count checks<br>
EOF
  );
  
  $pdf->say($left, $m, 0);

  $sql = <<<EOF
    SELECT ABS(amount) AS amount, txid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payer 
    WHERE $where AND deposit=$date ORDER BY txid
EOF;
  $q = db\q($sql);
  $x = $pdf->pageW / 2 - 1;
  $pdf->setY($m);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
// (needs htmlcell instead of cell)   if (!$in) $name = r\acct($uid)->bankInfoLink();
    if (!$in) $name = r\acct($uid)->legalName;
    $pdf->setX($x);
    $pdf->Cell(1, 0, u\fmtAmt($amount, ''), '', 0, 'R');
    $pdf->setX($x + $colW);
    $pdf->Cell($x - $colW, 0, "(# $txid) $name", '', 1);
    if (!$in) { // for outgoing checks, show bank address and member's legal name
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

  if (u\test()) $pdf->Close(); else $pdf->Output('rCredits-deposit-$date.pdf', 'I'); //Close and output
}

/**
 * Create a PDF of all relevant checks.
 * @param string $way: IN (from member bank account to rCredits) or OUT (from rCredits to member bank account)
 * @param int $date: deposit/check date
 * @param bool $reprint: whether to include a reprint of the previous deposit's checks
 * @param int $previous: date of previous deposit
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printChecks($args) {
  extract(u\just('way date reprint previous mark', $args));
  include_once __DIR__ . '/../pdf.class';
  $ck = new r\Pdf();
  $ck->setPrintHeader(FALSE);
  $ck->setPrintFooter(FALSE);
  $ck->setPageUnit('in'); // measure everything in inches
//  $ck->SetMargins(0, 0, 0); // left/top/right margin
  $ck->SetAutoPageBreak(FALSE);

  $where = @$reprint ? "(deposit=0 OR deposit>=$previous)" : "deposit=$date";
  $where .= strtoupper($way) == 'IN' ? ' AND amount<0' : ' AND amount>0';
  $q = db\q("SELECT txid,created,payer,amount,deposit,bankAccount FROM r_usd WHERE $where ORDER BY created, txid");
  $pos = 3; // before first position of 3, on 3-up check paper
  $count = 0;
  while ($tx = $q->fetchAssoc()) {
    if ($pos == 3) {$pos = 1; $ck->AddPage();} else $pos++;
    a\printCheck($ck, $pos, $tx, $mark);
    $count++;
  }
  $date = strftime('%Y-%m-%d', @$previous ?: r\rTime());
  if (u\test()) $ck->Close(); else $ck->Output("rCredits-deposit-checks-$way-$date.pdf", 'I'); //Close and output 
//  say($count . t(' checks printed'));
}

define('PHOTO_MAX', 90); // maximum height and width of photo

/**
 * Print an ID card
 * @param mixed $id: qid or uid of account to make a card for
 * @param mixed $agent: qid or uid of agent
 */
function memberID($id, $agent = '') {
  require_once __DIR__ . '/../statement.class';

// causes test to fail  if (!$mya = r\acct()) exit('You must first sign in.');
/*  if ($scan) {
    $perm = r\relation('permission', $mya->id, $id);
    if ($perm < r\perm(B_SCAN)) exit('That is not an employee with scan permission.');
    $acct = r\acct($mya->id, $id);
    if ($acct->agent->co) exit('You may print scan-in cards only for employees.');
  } else */

  if (!$a = r\acct($id, $agent)) exit('No such account ia: ' . $id . ':' . $agent);
  
  $fullName = $a->shortName ?: $a->fullName;
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  list ($fmt, $region, $tail, $agentCode) = $a->qo->qr();
  $tail = $fmt . $tail . $agentCode;
  if (TRUE) { // old style
    list ($region, $tail, $agentCode) = $a->qo->parse();
    if ($a->proSe) $tail .= '.'; else $tail = substr(r\qid(r\relation('reid', $a->id, $a->agentId)), 3) . '-';
  }
  
  if ($proSe = $a->proSe) {
    $nameSize = strlen($fullName) > 21 ? '100px' : '120px';
    $fields = array('cardCode', 0, 'Personal rCard #:');
  } else {
    if (!@$a->agent->cardCode2) r\makeCardCode($a); // NOT r\cardCode($a->qid);
    $a = $a->agent; // hereafter it's all about the agent
    $fullName = "$a->fullName<br>$fullName"; // agent and company
    $nameSize = '72px';
    
    $fields = array('cardCode2', 5, 'Company rCard #:');
  }
  list ($cardField, $lower, $roleName) = $fields;
  $server = isPRODUCTION ? 'RC2.ME' : 'RC4.ME';
  $qrUrl = "HTTP://$region.$server/$tail" . $a->$cardField;
  $photo = $a->photo ? "@$a->photo" : str_replace(R_DFT_PICTURE, R_NOT_VALID, DRUPAL_ROOT . $a->photoFilename());

//  $info = @getimagesize($photo);
//  list ($photoW, $photoH) = is_array($info) ? $info : array(0, 0);
//  list ($photoW, $photoH) = $photoW > $photoH ? array(PHOTO_MAX, '') : array('', PHOTO_MAX);
  list ($photoW, $photoH) = ['', PHOTO_MAX];
  $pdf = new r\Statement($a); // this function should be rewritten to use r\check class instead maybe?
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->AddPage();

  $style = array( // set style for barcode
    'border' => 0,
    'vpadding' => '0',
    'hpadding' => '0',
    'fgcolor' => array(0,0,0),
    'bgcolor' => false, //array(255,255,255)
    'module_width' => 1, // width of a single module in points
    'module_height' => 1, // height of a single module in points
  );

/*  if ($scan) { // scale it down for member-printed cards
    $pdf->StartTransform();
    $scale = 43.75; // make it card-size
    $margin = .25; // print margin, in inches
    $offset = 2500 * $margin / (100 - $scale);
    $pdf->ScaleXY($scale, $offset, $offset);
    $pdf->Rect(0, 0, 200, 128.57, 'D');
    $pdf->writeHTMLCell(80, 50, 13, 35 + $lower, "<div style=\"font-size:180px; text-align:left; color:darkgreen; font-weight:bold;\">r<span style=\"color:darkblue;\">POS</span><br><span style=\"font-size:140px;\">SIGN-IN</span></div>");
  } */
  // Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=[])

  if ($proSe) {
    $pdf->Image(__DIR__ . '/../images/idcard/background.jpg', 0, 0, 202, 128, '', '', '', true, 150, '', false, false, 0, false, false, false); // white background for company cards
  } else $pdf->Image(__DIR__ . '/../images/idcard/rlogo.png', 162, 40.5, 28, 42, '', '', '', true, 150, '', false, false, 0, false, false, false);

  $pdf->SetLineStyle(array('width' => 0.5, 'cap' => 'butt', 'join' => 'miter', 'dash' => 0, 'color' => array(0, 0, 0)));
  if ($proSe) $pdf->RoundedRect(91, 30, 60, 60, 3.50, '1111', 'DF', NULL, array(255, 255, 255)); // x, y, w, h, ?, fill?
  
  //if (!$scan) 
  $pdf->Image($photo, 0, 0, $photoW, $photoH, '', './account', '', true, 300, '', false, false, 1, false, false, false);
  $pdf->write2DBarcode($qrUrl, 'QRCODE,Q', 96, 35 + $lower, 50, 50, $style, 'N'); // L,M,Q,H are low-high error-correction

  $html = strtr('<span style="font-size:44px;">@ROLE<br><span style="font-size:76px; color:darkred; font-weight:bold;">@CODE</span><br>@REGION</span>', ['@ROLE' => $roleName, '@CODE' => $qid, '@REGION' => $a->regionName]);
  $pdf->writeHTMLCell(100, 30, 94, 3 + $lower, $html); // w, h, x, y
  $pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">rCredits.org</div>');
  $pdf->writeHTMLCell(182, 40, 10, 100, "<div style=\"font-size:$nameSize; text-align:center; color:midnightblue; font-weight:bold;\">$fullName</div>");

    /*
  $instructions = <<<EOF
<style>
  big {color:darkgreen; font-size:2em;}
  p {margin-top:.5em;}
</style>
Here is your<br><b style="color:darkgreen; font-size:1.3em;">Advance ID Card</b>
<p>You may print this card and use it temporarily, along with a second photo ID. To print, right-click here, then click "Print" OR use your browser's print icon.</p>
<p>To upload or change your photo, visit the "Settings" section.</p>
EOF;
*/
//  $pdf->writeHTMLCell(90, 64, 110, 5, $instructions);
  // Add temporary symbol over image (maybe remove lower right corner and add "temp"), to discourage unauthorized use
//  $pdf->Image(__DIR__ . '/../images/icons/print.png', 120, 30, 32, 32, '', '', '', true, 150, '', false, false, 0, false, false, false);
  
//  if ($scan) $pdf->StopTransform();
  if (u\test()) $pdf->Close(); else $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

function formSMS($form, &$sta) {
  $title = item(t('Test SMS'));
  $myNumber = svar('lastSMS');
  $number = textFld(t('From (number): '), '', required($myNumber));
  $message = textFld(t('Message: '));
  $submit = submit(t('Send it!'));

  focusOn('message');
  $form = compact(u\ray('title number message submit'));
  return labeled($form);  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(u\just('number message', $sta['input']));
  $number = u\fmtPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

function test($arg = '') {
  global $user;
  \drupal_get_messages(); // clear messages
  $modules = u\prefixValues('rcredits/', u\ray('admin rweb rcron rsmart'));
  include_once __DIR__ . '/../../gherkin/test.php';
  if ($user->uid != 1) { // test signed us out
    file_put_contents(T_MSG_FILE, serialize(\drupal_get_messages()));
    \session_destroy();
    \drupal_goto('sadmin/recover'); // has to be a form available when logged out
  } else \drupal_goto('sadmin');
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'reset') r\reset();
/*  if ($arg == 'changeAgent') {
    //session_destroy();
    $mya = r\acct('NEW.ZZB');
    svar('myid', $mya->id);
    global $user; $user = $mya->account();
    $user->roles = [];
    setGlobals();
    r\go('');
  } */
  if ($arg == 'reinstall') { // from devel module
    $modules = ['rcredits','rsms','rsmart','rweb'];
    \module_disable($modules, FALSE);
    \drupal_uninstall_modules($modules, FALSE);
    \module_enable($modules, FALSE);
    \drupal_set_message(t('Uninstalled and installed: %names.', array('%names' => implode(', ', $modules))));
  }
  if ($arg == 'clear_sessions') db\q('TRUNCATE table SESSIONS');
  if ($arg == 'php') r\go('devel/php');
  
  \drupal_set_message("DONE with \"$arg\".");
  \drupal_goto('sadmin');
}

function handyLinks() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
    'devel/menu/reset?destination=sadmin|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Redo MENUS|menu-hamburger',
    'rcredits/util/reinstall|Remove and reinstall all the rCredits modules|REINSTALL r|fast-backward',
    'rcredits/util/clear_sessions|Empty the sessions table|Clear Sessions|unchecked',
    
    'devel/settings|Helper functions, pages, and blocks to assist Drupal developers. The devel blocks can be managed via the block administration page.|Devel settings|cog',
    'devel/cache/clear?destination=sadmin|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache|new-window',
    'devel/reference|View a list of currently defined user functions with documentation links.|Functions|education',
    'devel/elements|View the active form/render elements for this site.|Elements|align-center',
    'devel/phpinfo|View your server&#039;s PHP configuration|PHPinfo()|queen',
    'devel/variable?destination=sadmin|Edit and delete site variables.|Variables|copy',
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
  );

  foreach ($links as $link) {
    list ($url, $detail, $label, $icon) = explode('|', $link);
    if (!$label) $label = $detail;
    $handy[$url] = ['', $label, '', 'anyone', '', $detail, @$icon];
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

