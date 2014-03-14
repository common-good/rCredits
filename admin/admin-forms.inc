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

include_once __DIR__ . '/admin.inc';

/**
 * Make various administrative operations available to the admin and cAdmin.
 */
function formAdmin($form, &$sta, $args = '') {
//  $form = u\insertBefore(compact('communities'), formCommunity(array(), &$sta, $args), 'count');

/* (This dropdown might be better here than at the top of every page)
   (top of page dropdown would then include only the CURRENTLY selected community (and the admin))
  $rows = db\q('SELECT uid, fullName FROM users WHERE uid<0 AND uid<:R_CTTY_UID0 ORDER BY fullName')->fetchAllKeyed();
  $cttyChoices = array_merge(array('ctty-0' => 'Select a community to manage'), u\prefixKeys('ctty', $rows));
  $community = formField('select', t('Manage:'), t('Select a community to manage'), '', $cttyChoices);
*/
  $mya = r\acct();
  $admin = $mya->admin;
  $tables = 'users txs gifts proxies relations invites'; // tables to show
  
  $title = item(t('System Administration'));
  if ($admin) {
    if (r\up()) $stop = submit(t('STOP')); else $start = submit(t('START'));
    $do1099b = submit('1099B');
    $tables .= ' notices boxes do log stats usd';
  }
  if ($ctty = $mya->community) {
    $cttyName = r\acct($ctty)->name;
    if ($mya->cttyUp) $stopCtty = submit(t('STOP ') . $cttyName); else $startCtty = submit(t('START ') . $cttyName);
  }
  
  $inviteeStatus = submit(t('Invitee Status'));
  $account = textField(t('Account:'), t('Account to do something to'),auto());
  $make = boxfield(t('Make:'));
  $printId = submit(t('Print ID'));
  $connectBank = submit(t('Connect Bank'));
  $seeSecure = submit(t('See Secure'));
  $newUsdEmail = submit(t('New USD Email'));
  $form = compact(u\ray('title stop start stopCtty startCtty'));
  foreach (u\ray($tables) as $one) $form[$one] = submit($one);
  $form += compact(u\ray('memberList inviteeStatus makeCtty account make printId seeSecure connectBank do1099b'));
  return $form;
}

function formAdmin_submit($form, &$sta) {
  $mya = r\acct();
  $admin = $mya->admin; // as distinct from merely cadmin
  
  extract(u\just('account make', $sta['input']));
  $op = op($sta);

  if ($op == 'start' or $op == 'stop') {
    r\up($op == 'start');
    return r\up() ? say(t('System is UP.')) : say(t('System is DOWN.'), 'ERR');
  }
  if ($up = ($op == 'startCtty') or $op == 'stopCtty') {
    $ctty = r\acct($mya->community);
    $ctty->setBit(B_OK, $up);
    return $up ? say(t('Community is UP.')) : say(t('Community is DOWN.'), 'ERR');
  }

  if (@$account) {
    $a = a($account); // should use whois first
    if (!$admin and $a->community != $mya->community) return say('That account is not in your community.', 'ERR');
  }
  
  if ($op == 'printId') {
    if (@$make) {
      seeSecure($a);
      r\makeCardCode($a);
      seeSecure($a);
    }
    return r\go('print-rcard/' . $a->qid);
  } elseif ($op == 'seeSecure') {
    if (@$a) {
/**/  debug(u\ray('legalName dwAccount phone federalId physicalAddr', @$a->legalName, @$a->usdAccount, substr(@$a->phone, 2), @$a->federalId, @$a->address . ' ' . $a->postalCode));
      seeSecure($a);
    }
  } elseif ($op == 'connectBank') {
    $secure = $a->secure;
    if (!$bankAccount = @$secure['bankAccount']) return say(t('Member has not yet entered bank info.'), 'zot');
    if ($a->country == US_COUNTRY_ID) {
      $routingNumber = substr(@$bankAccount, 4, 9); // chop of USkk
      $bankAccount = substr(@$bankAccount, 4 + 9); // everything after the routing number
    }
    if (!@$secure['auth']) return say(t('You must first set up the Dwolla account for this member.'), 'zot');
    $us = new r\usd($a);
    if (!$us->addBank($bankAccount, @$routingNumber, $secure['bankAccountType'], $err)) return say($err, 'zot');
    if (!$a->stepsDone['dw']) $a->stepDone('dw');
    say('info saved');
  } elseif ($op == 'do1099b') {
    include __DIR__ . '/../rcredits/rcredits-yearly.inc';
    r\Special\do1099bs();
  } elseif ($op == 'inviteeStatus') {
    return r\go('invited-whom');
  }
  
  // handle all other buttons with showQuery()
  if ($op == 'users') {
    $fields = 'uid, MID(phone,3) AS phone, flags, r, usd, rewards, minimum AS min, committed AS comit, share, floor, address, city, postalCode AS zip';
    return say(showQuery('users', 1, 'name', $fields));
  }
  
  $logFields = "channel,type,myid,IF(agent=myid,'--',agent) AS agent, REPLACE(special, ',', ', ') AS special, REPLACE(info, ',', ', ') AS info";

  if ($op == 'log') return say(showQuery('r_log', 1, 'logid DESC', $logFields));
  if ($op == 'tells') return say(showQuery('r_log', "info LIKE '%tell-staff%' ORDER BY logid DESC LIMIT 50", 'logid DESC', $logFields));

  if ($op == 'txs') return say(showQuery('r_txs', isDEV ? 1 : 'type NOT IN (:TX_REBATE,:TX_BONUS)', 'xid DESC', 'xid, t.created, type AS ty, t.state AS st, goods AS gd, payer, payee, amount, payerFor AS for1, payeeFor AS for2, payerTid AS tid1, payeeTid AS tid2', array('payer', 'payee')));

  if ($op == 'gifts') return say(showQuery('r_gifts', "honor<>'share'", 'donid DESC', 'donid,giftDate,t.uid,amount,often,honor,honored,t.share,completed', 'uid'));
  if ($op == 'invites') return say(showQuery('r_invites', 1, 'invited DESC', 'invited, inviter, invitee, email', 'inviter'));
  if ($op == 'notices') return say(showQuery('r_notices', 1, 'msgid DESC'));
  if ($op == 'proxies') return say(showQuery('r_proxies', 1, 'id DESC', 'person, proxy, priority', 'person'));
  if ($op == 'relations') return say(showQuery('r_relations', 1, 'main, reid', 'reid,main,other,draw,permission,employerOk,employeeOk,isOwner', 'main'));
  foreach (u\ray('boxes do stats usd') as $one) if ($op == $one) return say(showQuery("r_$one"));
}


/**
 * Create a new community or region.
 */
function formMakeCtty($form, &$sta) {
  $title = item(t('Make a New Community or Region'));
  $region = textfield(t('Region Code:'), t('3-letter region code (for communities within a region, leave this blank)'));
  $zips = textField(t('Zip Regex:'), t('Type a regular expression for all zipcodes to be included in the community or region. For example, ^013|01002 would include all zipcodes starting 013 plus zipcode 01002.'));
  $postalAddr = textField(t('Postal Address:'), t('Where does the post office send your mail (usually)?'), required(@$postalAddr));
  $submit = submit();
  
  $form = compact(u\ray('title subtext zips region')) + nameAndEmail()
         + ssnFields(FALSE) + contactFields(u\ray('zot', 0), TRUE) // prevent contact fields from defaulting
         + compact('postalAddr', 'submit');
  return labeled($form);
}

function formMakeCtty_validate($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\normalizeCase(u\just($normals = 'fullName legalName city address postalAddr', $sta['input'])));
  extract(u\just('zips region mail phone country state postalCode', $sta['input']));

  if (preg_match("~$zips~", null) === false) return say(t('bad Zips REGEX'), 'zips');
  if ($region and !preg_match('/[A-Z]{3}/i', $region)) return say(t('bad region code'), 'region');

  if (!@$fullName) $fullName = $legalName;
  foreach (u\ray('fullName legalName') as $field) {
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }

  if (!emailOkay($mail, $mya->id)) return; 
  if (!phoneOk($phone, $country, FALSE, 'makectty')) return;
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) say('po in location', 'address');

  if ($region) {
    $region = strtoupper($region);
    $stateAbbrev = r\realState($state, $country);
    if (!u\abbreviates($stateAbbrev, $region)) return say(t('Region code should start with state abbreviation (@state).', u\ray('@state', $stateAbbrev)), 'region');
    if (db\exists('r_regions', 'region=:region', compact('region'))) return say(t('That region already exists.'), 'region');
    $uid = r\region($country, $state, $postalCode);
    $name = $region . '.';
  } else {
    $uid = db\lookup('MIN(uid)-1', 'users');
    $name = u\shortName($fullName);
  }
  if (db\exists('users', 'name=:name', compact('name'))) return say('shortname taken', u\ray('otherName', $name), 'fullName');
  
  u\preray(compact(u\ray($normals . ' mail phone uid name region')), $sta['input']); // fix formatting
}

function formMakeCtty_submit($form, &$sta) {
  $dbFields = 'uid name fullName legalName mail flags phone faxetc federalId address country state city postalCode postalAddr verifyBy';
  extract(u\just('zips region ' . $dbFields, $sta['input']));

  $flags = u\bit(B_CO) | u\bit(B_DW) | u\bit(B_OK);
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;

  $DBTX = \db_transaction();

  $stateAbbrev = r\realState($state, $country);
  if ($region) db\insert('r_regions', u\ray('region state zips', $region, $stateAbbrev, $zips));

  $verifyBy = verifyBy(@$verifyBy);
  $postalCode = $zips; // store the regex defining the region or community
  
  $info = compact(u\ray($dbFields)) + acctType(CO_PARTNERSHIP) + u\ray('community rebate', $uid, 0);
  $info = u\normalizeCase(u\just('fullName city address', $info)) + $info;

  if (!$a = new r\acct($info)) return say('cannot create account', 'zot');
  
  $a->createUsdAccount($err); // initialize Dwolla account
  if ($err) say($err . t(' (You will have to initialize the Dwolla account manually, using a($uid)->createUsdAccount().)'));

  unset($DBTX); // commit
  say('info saved');
}

/**
 * Show customer support info about one member.
 * @param string $qid: what account to show (qid or uid)
 */
function formMemberInfo($form, &$sta, $qid = '') {
  global $base_url;
  $a = r\acct($qid);
  $title = item($a->legalName);
  if ($showUsd = (r\acct()->cAdmin and !$a->stepsDone['dw'])) {
    extract((array) $a->secure, EXTR_PREFIX_ALL, 's'); // dunno why (array) is required (indirect mod error)
    debug(u\ray('usdEmail usdPass phone SSN address zip usdPIN', @$s_usdEmail, @$s_usdPass, @substr(@$s_usdPhone, 2), $a->federalId, $a->address, $a->postalCode, @$s_usdPin), 1);
  }
  $account = item($a->qid, t('Account code:'));
  $username = item($a->name, t('User name:'));
  $steps = $a->ok
    ? item(showActivity($a), t('Activity:'))
    : item(showSteps($a), t('To do (red):'), t('sign=agreement | contact=contact info | dw:for admin to do | connect=connect bank | verify=verify bank deposits'));
  $relations = item(showRelations($a), t('Relations:'));
  $notes = formField('textarea', t('Notes:'), '', dft($a->notes));
  $submit = submit();
  $back = item('<a href="' . $base_url . '/sadmin/member-list">Back to Member List</a>', ' ');
  $qid = hidField($qid);
  return labeled(compact(u\ray('qid title account username steps relations notes submit back')));
}

function formMemberInfo_submit($form, &$sta) {
  extract(u\just('qid notes', $sta['input']));
  $a = r\acct($qid);
  $a->update(compact('notes'));
  say('info saved');
//  return r\go('member-list');
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
  $sql = 'SELECT uid, fullName, notes, phone, mail AS email, postalAddr, :IS_CO AS co FROM users WHERE uid>1 AND community=:cttyId ORDER BY :IS_OK, fullName';
  $q = db\q($sql, u\ray('cttyId', $ctty->id));
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone0 = $phone));
    $nameClass = $co ? 'name co' : 'name';
    $notes = str_replace("\n", "<br>\n", $notes);
    $notes = str_replace('do:', '<b style="color:blue; font-size:130%;">do:</b>', $notes);
    $a = r\acct($uid);
    $fullName = "<a href=\"$base_url/sadmin/member-info/$a->mainQid\">$fullName</a>";
    $steps = showSteps($a);

    if ($cAdmin) {
      $bits = array(); // zap previous value
      foreach (u\ray('member ok dw bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
      $bits = $bits ? ('<span style="color:darkgreen;">' . join(' ', $bits) . "</span><br>\n") : '';
    } else $bits = '';
    
    $one = ($steps or $bits or $notes) ? "<tr><td colspan=4 style='font-size:70%; font-weight:normal; padding-left:20px;'> $steps$bits$notes</td></tr>\n" : '';
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
 * Print an ID card
 * @param mixed $id: quid or uid of account to make a card for
 * @param bool $scan: create a scan-in card (for the currently managed company, with no photo)
 */
function memberID($id, $scan) {
  require_once(__DIR__ . "/../../tcpdf/config/lang/eng.php");
  require_once(__DIR__ . "/../../tcpdf/tcpdf.php");
  define('PHOTO_MAX', 90); // maximum height and width of photo

  if (!$mya = r\acct()) exit('You must sign in first.');
  if ($scan) {
    $perm = r\relation('permission', $mya->id, $id);
    if ($perm < r\perm(B_SCAN)) exit('That is not an employee with scan permission.');
    $acct = r\acct($mya->id, $id);
    if ($acct->agent->co) exit('You may print scan-in cards only for employees.');
  } elseif (!$acct = r\acct(strtoupper($id))) exit('No such account.');
  
  $fullName = $acct->fullName;
  $regionName = r\acct($acct->community)->fullName;
  $qid = $acct->qid;
  
  if (!$proSe = $acct->proSe) {
    if (!@$acct->agent->secure['cardCode2']) r\makeCardCode($acct);
    $acct = $acct->agent; // hereafter it's all about the agent
    $fullName = "$acct->fullName<br>$fullName"; // agent and company
    $nameSize = '72px';
    $fields = array('cardCode2', 5, 'agent', 'Company Agent');
  } else {
    $nameSize = strlen($fullName) > 22 ? '100px' : '120px';
    $fields = array('cardCode', 0, 'member', 'Member');
  }
  list ($cardField, $lower, $role, $roleName) = $fields;
  $mark = u\consta("r_$role", 'mark');
  list ($region, $tail) = explode($mark, $qid);
  $urlMark = $proSe ? $mark : R_AGENT_URL_MARK;
  $server = isPRODUCTION ? 'RC2.ME' : 'RC4.ME';
  $qrUrl = "HTTP://$region.$server/$tail$urlMark" . $acct->secure[$cardField]; // NOT r\cardCode($qid);
//  $qrServer = strtoupper('http://' . R_SERVER_ID . '.rC2.me/I/');
//  $qrUrl = $qrServer . str_replace(R_AGENT_MARK, R_AGENT_URL_MARK, $qid . $mark) . $acct->secure[$cardField];
  $photo = DRUPAL_ROOT . $acct->auxFilename('picture');

  $info = @getimagesize($photo);
  list ($photoW, $photoH) = is_array($info) ? $info : array(0, 0);
  list ($photoW, $photoH) = $photoW > $photoH ? array(PHOTO_MAX, '') : array('', PHOTO_MAX);
  $pdf = new \TCPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
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

  if ($scan) { // scale it down for member-printed cards
    $pdf->StartTransform();
    $scale = 43.75; // make it card-size
    $margin = .25; // print margin, in inches
    $offset = 2500 * $margin / (100 - $scale);
    $pdf->ScaleXY($scale, $offset, $offset);
    $pdf->Rect(0, 0, 200, 128.57, 'D');
    $pdf->writeHTMLCell(80, 50, 13, 35 + $lower, "<div style=\"font-size:180px; text-align:left; color:darkgreen; font-weight:bold;\">r<span style=\"color:darkblue;\">POS</span><br><span style=\"font-size:140px;\">SIGN-IN</span></div>");
  }
  // Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=array())

  if ($proSe) {
    $pdf->Image(__DIR__ . '/../images/idcard/background.jpg', 0, 0, 202, 128, '', '', '', true, 150, '', false, false, 0, false, false, false); // white background for company cards
  } else $pdf->Image(__DIR__ . '/../images/idcard/rlogo.png', 162, 40.5, 28, 42, '', '', '', true, 150, '', false, false, 0, false, false, false);
  
  $pdf->SetLineStyle(array('width' => 0.5, 'cap' => 'butt', 'join' => 'miter', 'dash' => 0, 'color' => array(0, 0, 0)));
  if ($proSe) $pdf->RoundedRect(91, 30, 60, 60, 3.50, '1111', 'DF', NULL, array(255, 255, 255)); // x, y, w, h, ?, fill?
  
  if (!$scan) $pdf->Image($photo, 0, 0, $photoW, $photoH, '', './account', '', true, 300, '', false, false, 1, false, false, false);
  $pdf->write2DBarcode($qrUrl, 'QRCODE,Q', 96, 35 + $lower, 50, 50, $style, 'N'); // L,M,Q,H are low-high error-correction

  $html = strtr('<span style="font-size:44px;">@ROLE<br><span style="font-size:76px; color:darkred; font-weight:bold;">@CODE</span><br>@REGION</span>', array('@ROLE' => $roleName, '@CODE' => $qid, '@REGION' => $regionName));
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
  
  if ($scan) $pdf->StopTransform();
  $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

function formSMS($form, &$sta) {
  $title = item(t('Test SMS'));
  $myNumber = svar('lastSMS');
  $number = textField(t('From (number): '), '', required($myNumber));
  $message = textField(t('Message: '));
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
  include __DIR__ . '/../../gherkin/test.php';
/*  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate(); */
  \drupal_goto('sadmin/handy');
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'reset') r\reset();
  if ($arg == 'changeAgent') {
    //session_destroy();
    $mya = r\acct('NEW.ZZB');
    svar('myid', $mya->id);
    global $user; $user = $mya->account();
    $user->roles = array();
    setupGlobals();
    r\go('');
  }
  if ($arg == 'clear_sessions') db\q('TRUNCATE table SESSIONS');
  //if ($arg == 'tests') r\go('http://localhost/devcore/rcredits/test?menu=1');
  if ($arg == 'php') r\go('devel/php');
  
  \drupal_set_message("Doing rCredits Util '$arg'...");
  \drupal_goto('sadmin/handy');
}

function handyLinks() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
//    'rcredits/test?menu=1|Test Menu|',
    'rcredits/util/changeAgent|Change Agent to Bea|',
    'reinstall|Reinstall rCredits|',//    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
    'sms|Simulate an SMS transaction|Simulate SMS',
//    'admin/config/development/testing|Test|Test',
//    'deletetests.php|Delete old tests|Delete old tests',
//    'rcredits/util/reset|Reset rCredits|',
    'rcredits/util/clear_sessions|Clear Sessions|',
    
    'devel/settings|Helper functions, pages, and blocks to assist Drupal developers. The devel blocks can be managed via the block administration page.|Devel settings',
    'devel/cache/clear?destination=sadmin/handy|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache',
    'devel/entity/info|View entity information across the whole site.|Entity info',
//    'devel/php|Execute some PHP code|Execute PHP Code',
    'devel/field/info|View fields information across the whole site.|Field info',
    'devel/reference|View a list of currently defined user functions with documentation links.|Function reference',
    'devel/elements|View the active form/render elements for this site.|Hook_elements()',
    'devel/menu/item?path=node|Details about a given menu item.|Menu item',
    'devel/phpinfo|View your server&#039;s PHP configuration|PHPinfo()',
    'devel/menu/reset?destination=sadmin/handy|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Rebuild menus',
    'devel/reinstall?destination=sadmin/handy|Run hook_uninstall() and then hook_install() for a given module.|Reinstall modules',
//    'devel/run-cron|Run cron|Run cron',
    'devel/session|List the contents of SESSION.|Session viewer',
    'devel/theme/registry|View a list of available theme functions across the whole site.|Theme registry',
    'devel/variable?destination=sadmin/handy|Edit and delete site variables.|Variable editor',
  );

  $result = array();
  foreach ($links as $link) {
    list ($url, $title, $label) = explode('|', $link);
    if (!$label) $label = $title;
    if (!u\abbreviates('http://', $url)) $url = $base_path . $url;
    $result[] = <<<EOF
<div><a href="$url" title="$title">$label</a></div>
EOF;
  }
  $result = join(' | ', $result);
  return "<div class='develw'>\n$result</div>";
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

  echo <<<EOF
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
