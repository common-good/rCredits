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
  global $mya;
  if (!$mya->cAdmin) hack('panel');
  
  $tables = 'users txs usd log gifts proxies relations invites'; // tables to show
  $queries = [
    '' => '',
  ];
  
  $title = item(t('System Administration'));
  
  $inviteeStatus = submi(t('Invitee Status'));
  $organizerList = submi(t('Local Requests'));
  $queries = selectFld(t('Query:'), '', '', $queries);
  $account = textFld(t('Account:'), t('Account to do something to'), (auto() ?: []) + dft(@$qid ?: svar('adminingAcct')));
  
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
      $start = w\item(join('', $list));
    } */
    $do1099btest = submi(t('1099B TEST'));
    $do1099b = submi('1099B');
    $tables .= ' notices boxes do stats usd nonmembers';
    $special = submi(t('Special'));
    $askForAppData = submi(t('Ask for App Data'));
    $seeAppData = submi(t('See App Data'));
    $ssnLink = submi(t('SSN Link'));
    $seeSecure = submi(t('See Secure'));
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
  $do += compact(ray('inviteeStatus organizerList do1099btest do1099b special stop start stopCtty startCtty'));
  $do = fieldSet('do', $do);
  
  $acctSet = fieldSet('acctSet', compact(ray('account make2 printId seeData seeChanges ssnLink seeSecure askForAppData seeAppData')));

  return labeled(compact(ray('title do acctSet')));
}

function formAdminPanel_submit($form, &$sta) {
  global $base_url;
  global $mya;
  
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
    if (!$mya->admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  }
  
  if (@$a and $op == 'printId') {
    if ($a->co and $a->proSe) return say(t('A company cannot have a %PROJECT Card (only its agents have %PROJECT Cards).'));
    if (@$make) {
      $oldFile = DRUPAL_ROOT . $a->photoFilename(TRUE); // get this before makeCardCode
      $a->makeCardCode(); // this changes photo filename
      if (file_exists($oldFile)) {
        $newFile = DRUPAL_ROOT . $a->photoFilename(TRUE);
        rename($oldFile, $newFile);
      }
    }
    return r\go('print-rcard/' . $a->id . ($a->co ? "/$a->agentId" : ''));
  } elseif ($op == 'askForAppData') {
    $a->update('getAppData appData', '!report', []);
    say(t('Request for app data sent to ') . $a->fullName);
  } elseif ($op == 'seeAppData') {
/**/  debug($a->appData);
  } elseif ($op == 'seeSecure') {
    if (@$a) {
      say(u\rayTable(ray('fullName phone dob physicalAddr', $a->fullName, substr(@$a->phone, 2), $admin ? u\fmtDate(@$a->dob) : '', @$a->address . ' ' . $a->zip)));
//      seeHidden($a);
      seeSecure($a);
    }
  } elseif ($op == 'seeData') {
    if (@$a and $a->proSe) say(u\rayTable($a->data, FALSE, TRUE));
/*  } elseif ($op == 'connectBank') {
    r\Web\connectBank($a); */
  } elseif ($op == 'seeChanges') {
    r\go("sadmin/changes/$a->mainQid");
  } elseif ($op == 'ssnLink') {
    w\say($a->fullName. t(' SSN Link:<br><br> &nbsp; %PROMO_URL/do/doSsn~%qid~%code &nbsp; ', 'qid code', $a->mainQid, $a->emailCode));
  } elseif ($op == 'do1099b' or $test1099 = ($op == 'do1099btest')) {
    include_once __DIR__ . '/../rcredits-yearly.inc';
    r\Special\do1099bs(@$test1099);
  } elseif ($op == 'inviteeStatus') {
    return r\go('community/invite/invited-whom');
  } elseif ($op == 'organizerList') {
    return r\go('sadmin/organizer-list');
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, flags, balance, rewards, savingsAdd AS saveAdd, minimum AS min, floor, city, zip AS zip';
    return say(a\showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,IF(agent=myid,':R_NONE',agent) AS agent, REPLACE(info, ',', ', ') AS info";

/**/ if ($op == 'log') return say(file_get_contents(flog())); // say(a\showQuery('r_log', 1, 'logid DESC', $logFields, 'myid'));
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
  if ($op == 'relations') return say(a\showQuery('r_relations', 1, 'main DESC, reid', 'reid,main,other,otherNum,draw,permission,employee,isOwner,code', 'main'));
  if ($op == 'usd') return say(a\showQuery('r_usd', 1, 'created DESC'));
  foreach (ray('boxes do stats nonmembers') as $one) if ($op == $one) return say(a\showQuery("r_$one"));
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
  return compact(ray('title subtext zips submit list'));
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
        while (substr($v, strlen(CRYPT_FLAG)) == CRYPT_FLAG) $v = u\decrypt($v); // u\starts failes here (when encrypted?)
        $rec[$k] = $oldValue = a\formatField($k, $v);
        $list[] = "<tr><td>$date</td><td>$k</td><td>$oldValue</td><td>$newValue</td></tr>";
      }
    }
  } else $list[] = '<tr><td colspan=4>No changes</td></tr>';
  
  $list = join("\n", $list);
  $list = item("<table>$list</table>");
  
  return compact(ray('title list'));
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
    w\setAcct($mya->id);
    if ($msgs = @unserialize(@file_get_contents(T_MSG_FILE))) {
      unlink(T_MSG_FILE);
      \drupal_get_messages(); // throw out any existing messages
      $_SESSION['messages'] = $msgs;
    }
    r\go('sadmin');
  }
  
  $title = item(t('Recover Admin'));
  return compact(ray('title'));
}

/**
 * Create a new community or region.
 */
function formMakeCtty($form, &$sta) {
  $title = item(t('Make a New Community'));
  $subtext = item(t('<p>Typically the name of a %PROJECT community is the place name followed by "%PROJECT"; for example, "<b>Greenfield Area %PROJECT</b>". Legal names should include the state (for disambiguation), spelled out; for example, "Greenfield Massachusetts %PROJECT".</p><p>By contrast, region names begin with "%PROJECT"; for example, "<b>%PROJECT Western Mass</b>".</p><p>Email should reflect the legal name, for example "<b>greenfield@ma.%CG_DOMAIN</b>".</p>'));
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
  return labeled($form);
}

function formMakeCtty_validate($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract(u\normalizeCase(u\just($normals = 'fullName legalName postalAddr', $sta['input'])));
  extract(u\just('zips region email phone federalId', $sta['input']));

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
  if ($otherName = db\lookup('fullName', 'users', 'name=:name', compact('name'))) return say('shortname taken', compact('otherName'), 'fullName');
//  $name = r\uniqueName($fullName);

  if ($err = u\badSsn($federalId)) return say($err, ['what' => 'federalId'], 'federalId');
  
  list ($address, $city, $state) = u\parseAddr($postalAddr); // zip doesn't apply, since it is used for defining the territory
  if (!$address) return say('Bad postal address', 'postalAddr');
  if (!$state = db\lookup('id', 'r_states', 'abbreviation=:state', compact('state'))) return say('Bad state', 'postalAddr');

  $region = strtoupper($region);
  if (!db\exists('r_regions', 'region=:region', compact('region'))) return say(t('That region does not exist.'), 'region');

  u\preray(compact(ray($normals . ' name email phone address city state region federalId')), $sta['input']); // fix formatting
}

function formMakeCtty_submit($form, &$sta) {
  $fields = 'name fullName legalName email phone federalId postalAddr address city state';
  extract(u\just('zips region ' . $fields, $sta['input']));

  $zip = $zips; // store the regex defining the region or community
  $flags = u\bit(B_CO) | u\bit(B_OK) | u\bit(B_UP);
  $rebate = 0;
  $country = R_COUNTRY_ID;
  $region = r\qo("!$region")->id;
  $fields .= ' zip flags rebate region address state country';

  $DBTX = \db_transaction();
  $info = compact(ray($fields)) + acctType(CO_PARTNERSHIP);
  
  if (!$a = new r\Acct($info)) return say('cannot create account', 'zot');
  $a->update('community', $a->id);
  
  unset($DBTX); // commit
  say('info saved');
}

/**
 * Show a list of members in the current account's community
 */
function formMemberList($form, &$sta) {
  global $base_url;
  global $mya;

  $cAdmin = $mya->cAdmin;
  $ctty = r\acct($mya->community);

  $title = item((($cAdmin and $ctty) ? $ctty->fullName : '') . t(' Member List'));
  $sql = 'SELECT uid, login, access, fullName, notes, :IS_CO AS co FROM users WHERE uid>1 AND community=:cttyId AND NOT :IS_NONUDGE ORDER BY :IS_OK, IF(:IS_OK, fullName, 0-login)'; // -login fails because it gets treated as a string
  $q = db\q($sql, ray('cttyId', $ctty->id));
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
    $fullName = "<a href=\"$base_url/summary/$a->mainQid\">$fullName</a>";
    $steps = a\showSteps($a);
    $date = u\fmtDate($a->ok ? $access : $login);

    if ($cAdmin) {
      $bits = []; // zap previous value
      foreach (ray('member ok bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
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
  return compact(ray('title list emails'));
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
    $params = http_build_query(compact(ray(EXPORT_PARAMS)));
    $download = item("<iframe class=\"hidden\" src=\"$base_url/sadmin/export-list/$params\"></iframe>");
  }
  
  global $mya;
  $cttyA = $mya->cttyA;

  $title = item(t('Download Member List'));
  $expMo = r\rdoMonth();
  $subtext = w\item(t(<<<EOF
<p>Generate a CSV format spreadsheet for accounts in your community (%cttyName). We recommend you use a service like MailChimp and import this list there before each mailing. Company accounts and members are marked, so there is generally no need to export them separately &mdash; you can do that separation within the mailing program.</p>
<p>Action links for individual accounts take the form %code, where expMonth is the number of an expiration month (the current month is number %expMo). For example, a link from MailChimp to the democracy page, expiring next month, would be:</p>
<blockquote>%example</blockquote>
<p>Other action choices might be listed here someday. :)</p>
EOF
  , 'cttyName code expMo _example', $cttyA->fullName, 'do/action~qid~ecode~monthNum', $expMo, $base_url . '/do/doEvents~*|ID|*~*|ECODE|*~' . ($expMo + 1)));
  
  $chimp = boolFld(t('Exporting to MailChimp?'), '', TRUE);
  
  if ($mya->admin) $allCttys2 = boxFld('allCttys', t('All communities:'), '', TRUE); // temporary for CGF oversight
  $requests2 = boxFld('requests', t('Include requests:'), t('Include non-members who have requested an invitation 8/9/2017 or later.'), TRUE);
  $status = w\radiosFld(t('Status:'), '', dft(2), ray('active not-yet-active both'));
  $types = w\radiosfld(t('Types:'), '', dft(2), ray('individuals companies both'));
  $anonymous = boxFld('anonymous', t('Anonymous:'), t('Export only non-identifying information (for data-mapping and analysis)'), FALSE);
  $chimpSet = w\fieldSet('fields', compact(ray('allCttys2 requests2 status types anonymous')));
  
  $submit = submit(t('Download'));
  
  $js = "$('#edit-chimp-1').click(function() { $('.form-item-chimpSet').hide();});
         $('#edit-chimp-0').click(function() { $('.form-item-chimpSet').show();});";
  w\js($js, 'inline', 'footer');
  return labeled(compact(ray('title subtext chimp chimpSet submit download')));
}

function formDownloadMemberList_submit($form, &$sta) {
  extract(u\just(EXPORT_PARAMS, $sta['input']));
  $do = 'done';
  return r\go('sadmin/export-list/' . http_build_query(compact(ray(EXPORT_PARAMS))), t('Download initiated.'));
}

/**
 * Generates the execute form.
 */
function formPhp($form, &$sta, $res = '') {
  $res = item($res);
  $code = areaFld('', '', dft(\variable_get('code')));
  $submit = submit(t('Execute Code'));
  return compact('res', 'code', 'submit');
}

/**
 * Process PHP execute form submissions.
 */
function formPhp_submit($form, &$sta) {
  extract(u\just('code', $sta['input']));
  \variable_set('code', $code); // remember for next time
  ob_start();
  eval($code);
  $res = ob_get_clean();
/**/  \drupal_set_message(print_r($res, 1));
  r\go('/sadmin/php/');
}

//define('DEPOSIT_DATE1', 1411517529); // first deposit date

/**
 * Offer to print checks or a deposit slip for transfers to or from member bank accounts.
 */
function formDeposits($form, &$sta, $deposit = '') {
//  if ($deposit) return r\go("sadmin/deposit/$deposit");
  $title = item(t('Deposits'));

  $ways = [t('IN') => '(amount>0 AND txid>0)', t('OUT') => '(amount<0 AND txid>0)']; // , t('BAD') => 'txid<0'];
  if (r\acct()->superAdmin) foreach ($ways as $way => $where) {  
    $count = db\count('r_usd', $where . ' AND deposit=0');
//    $previous = db\lookup('MAX(deposit)', 'r_usd', $where);
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
  
  $deposits = item(a\deposits(), ' ');
  return compact('title', array_keys($ways), 'deposits');
}

function formDeposits_submit($form, &$sta) {
  $op = w\op($sta);
  $way = str_replace('submit', '', $op); // cute. INsubmit or OUTsubmit -> IN or OUT
//  $previous = @$sta['input'][$way . 'previous'];
//  r\go("sadmin/checks/way=$way&date=0&previous=$previous&mark=1");
  $func = ($way == 'IN' or time() < strtotime('1/10/2017')) ? 'checks' : 'achs';
  r\go("sadmin/$func/way=$way&date=0&mark=1");
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
  $pdf->SetAutoPageBreak($count > ($in ? 47 : 9), $m+.1); // leave space at bottom (count compensates for TCPDF bug)
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(TRUE);
//  $pdf->setLanguageArray($l); // (eg "page" in footer)

  list ($activity, $where) = $in ? [t('DEPOSIT'), '(NOT (amount<0 XOR txid<0))'] : [t('CASHOUT'), '(amount<0 XOR txid<0)'];
  $today = $date == 1 ? t('(Historical)') : u\fmtDate($date ?: r\rTime());
  $escrow = $in ? ESCROW_IN : ESCROW_OUT;
  $pdf->setupFooter('', BANK_DBA . " $activity $today", ['........ A5990D1155A '.$escrow.'C', 'GnuMICR;12'], 'Page @PAGE of @PAGES');
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

  $sql = <<<EOF
    SELECT ABS(amount) AS amount, txid, uid, u.fullName AS name FROM r_usd t LEFT JOIN users u ON u.uid=t.payee 
    WHERE $where AND deposit=$date ORDER BY txid
EOF;
  $q = db\q($sql);
  $x = $pdf->pageW / 2 - 1;
  $pdf->setY($m);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
// (needs htmlcell instead of cell)   if (!$in) $name = r\acct($uid)->bankInfoLink();
    if (!$in) $name = r\acct($uid)->legalNameDpy;
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

  $date = strftime('%Y%m%d', $date);
  if (u\test()) $pdf->Close(); else 
  $pdf->Output(PROJECT . "-deposit-$way-$date.pdf", 'I'); //Close and output
}

/**
 * Create an ACH Batch file for all relevant checks.
 */
function achBatch($args) {
  extract(u\just('way date mark', $args));
  $ways = ['IN'=>'(amount>0 AND txid>0)', 'OUT'=>'(amount<0 AND txid>0)', 'BAD'=>'txid<0'];
  $where = $ways[strtoupper($way)];
  $q = db\q("SELECT txid,created,payee,amount,deposit,bankAccount FROM r_usd WHERE deposit=:date AND $where ORDER BY created, txid", compact('date'));
  if (!$q->rowCount()) return r\go('sadmin/deposits', t('There are no transactions for that ACH batch.'));

  $date = strftime('%Y%m%d', $date);
  u\csvStart("CGach$way-$date.csv");
  while ($tx = $q->fetchAssoc()) {
    extract($tx);
    $a = r\acct($payee);
    $a->fixTxBankAccount($bankAccount, $txid);
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

/**
 * Create a PDF of all relevant checks.
 * @param string $way: IN (from member bank account to rCredits) or OUT (from rCredits to member bank account)
 * @param int $date: deposit/check date
 * @param bool $reprint: UNUSED whether to include a reprint of the previous deposit's checks
 * @param int $previous: UNUSED date of previous deposit
 * @param bool $mark: whether to set the deposit date for each relevant transfer, in r_usd
 */
function printChecks($args) {
  extract(u\just('way date mark', $args));
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
 *//*
function memberIDX($id, $agent = '') {
  require_once __DIR__ . '/../pdf.class';

  if (!$a = r\acct($id, $agent)) exit('No such account ia: ' . $id . ':' . $agent);
  
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  list ($fmt, $region, $tail, $agentCode) = $a->qo->qr();
  $tail = $fmt . $tail . $agentCode;
  
  $acctName = $a->shortName ?: $a->fullName;
  $nameFont = strlen($acctName) > 38 ? 'font-stretch:condensed;'
  : (strlen($acctName) > 33 ? 'font-stretch:semi-condensed;' : '');
  
  if (!$proSe = $a->proSe) {
    if (!@$a->agentA->cardCode2) $a->makeCardCode(); // NOT r\cardCode($a->qid);
    $aa = $a->agentA; // hereafter it's all about the agent
  } else $aa = $a;

  list ($cardFld, $role, $ptQid, $yName, $bg) = $proSe 
  ? ['cardCode', t('Member'), 10, 1.73, 'background'] 
  : ['cardCode2', t('Company'), '9.5;font-stretch:semi-condensed', 1.68, 'backgroundCo'];

  $server = isPRODUCTION ? 'RC2.ME' : 'RC4.ME';
  $qrUrl = "HTTP://$region.$server/$tail" . $aa->$cardFld;
  $photo = $aa->photo ? "@$aa->photo" : str_replace(R_DFT_PICTURE, R_NOT_VALID, DRUPAL_ROOT . $aa->photoFilename());

  list ($wCard, $hCard, $m, $xText) = [3.375, 2.125, .24, .06]; // card width, height, negative page margin, text indent
  list ($xSite, $ySite, $wPhoto, $mxPhoto, $myPhoto, $xCardNo) = [$m+.1, $m+.38, 1.09, .09, .13, 2.4];
  list ($cCG, $cSite, $cName) = ['#000065', 'black', '#004000'];
  list ($xPhoto, $yPhoto, $wPhoto, $hPhoto) = [$m+$wCard-$mxPhoto-$wPhoto, $m+$myPhoto, $wPhoto, $wPhoto*4/3];
  
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
//  $pdf->say(isPRODUCTION ? $a->cttyUrl : CG_DOMAIN,  $xSite, $ySite, '', '', "7;color:$cSite;font-stretch:normal;", 'L');
  $pdf->say(CG_DOMAIN,  $xSite, $ySite, '', '', "7;color:$cSite;font-stretch:normal;", 'L');
  $pdf->say('TM', $m+2, $m+.1, 1, '', "3;color:$cCG", 'L');
//  $pdf->Image(__DIR__ . "/../images/idcard/logo250.png", $m+$xLogo, $m+$yLogo, $wLogo, '', '', '', 'C', true); // file, x, y, w, h, type, link, align, resize
  $pdf->Image($photo, $xPhoto, $yPhoto, $wPhoto, $hPhoto, '', '', 'L', true);
  $pdf->Rect($xPhoto, $yPhoto, $wPhoto, $hPhoto, 'D'); // x, y, w, h, style, border, fill
  $pdf->write2DBarcode($qrUrl, 'QRCODE,Q', $m+1.12, $m+0.68, .8, .8, $style, 'N'); // L,M,Q,H are low-high error-correction

//  $pdf->say("$role<br>" . t('Card #'),  $m+$xCardNo, $m+.52, 1, '', '8;color:white;font-stretch:semi-expanded;', 'C');
//  $pdf->say($qid, $m+$xCardNo, $m+.81, 1, '', "$ptQid;color:yellow;letter-spacing:0.5px", 'C');

  $pdf->say($acctName, $m+$xText, $m+$yName, $wCard, '', "12;B;$nameFont;color:$cName", 'C');
  if (!$proSe) $pdf->say($aa->fullName, $m+$xText, $m+$yName+0.22, $wCard, '', "8;color:$cName", 'C');
  $pdf->say($qid, $m+$wCard-.5, $m+$hCard-.15, .5, '', "5;color:black;font-stretch:normal;", 'C');

  if (!isPRODUCTION) $pdf->say(t('TEST'), $m+$wCard-.5, $m+$hCard-.25, .5, '', '6;color:darkred', 'C');

  $pdf->StopTransform();
  if (u\test()) $pdf->Close(); else $pdf->Output('t.pdf', 'I'); //Close and output PDF document
}
*/

function memberId($id, $agent = '') {
  require_once __DIR__ . '/../pdf.class';
  if (!$a = r\acct($id, $agent)) exit('No such account ia: ' . $id . ':' . $agent);
  
  $a->setAgentNum(); // assign an agentCode, if appropriate (and not yet done)
  $qid = $a->qid; // get this before setting $a to agent (below)
  list ($fmt, $region, $tail, $agentCode) = $a->qo->qr();
  $tail = $fmt . $tail . $agentCode;
  
  $acctName = $a->shortName ?: $a->fullName;
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

  if (!isPRODUCTION) $pdf->say(t('TEST'), $xTest, $m+$hCard-.19, .5, '', '6.5;color:darkred', 'C');

  $pdf->StopTransform();
/*  if (time() < strtotime('6/20/2017')) {
    $pdf->Output(strtolower("$qid.cgcard.pd"), 'D'); // Close and output PDF document ('I')
  } else */
  if (u\test()) $pdf->Close(); else $pdf->Output(strtolower("$qid.cgcard.pd"), 'I'); // Close and output PDF document ('I')
//  if (time() < strtotime('6/20/2017') and $next = nextMember($id)) r\go("$base_url/print-rcard/$next");
}

function formSMS($form, &$sta) {
  $title = item(t('Test SMS'));
  $myNumber = svar('lastSMS');
  $number = textFld(t('From (number): '), '', required($myNumber));
  $message = textFld(t('Message: '));
  $submit = submit(t('Send it!'));

  focusOn('message');
  $form = compact(ray('title number message submit'));
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
  $modules = u\prefixValues('rcredits/', ray('admin rweb rcron rsmart'));
  include_once DRUPAL_ROOT . '/vendor/gherkin/test.php';
  file_put_contents(T_MSG_FILE, serialize(\drupal_get_messages()));
  \session_destroy();
  \drupal_goto('sadmin/recover'); // has to be a form available when logged out
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'reset') r\reset();
  if ($arg == 'rebuild') {
    require_once __DIR__ . '/../rcredits-install.inc';
    r\rebuildDb();
    if (isDEV) t\clear(); else a\setupBasicAccounts();
  /*    $modules = ray('rcredits rsms rsmart rweb');
    \module_disable($modules, FALSE);
    foreach ($modules as $module) {
      \drupal_install_schema($module);
    }
    \drupal_uninstall_modules($modules, FALSE);
    \module_enable($modules, FALSE);
    */
//    w\say(t('Uninstalled and installed: %names.', 'names', implode(', ', $modules)));
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
    'rcredits/util/rebuild|Rebuild all project tables|REINSTALL cg|fast-backward',
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

