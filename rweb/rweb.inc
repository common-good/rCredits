<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\db as db;

require_once __DIR__ . '/rweb-subs.inc';
require_once __DIR__ . '/../rcredits-backend.inc';
require_once __DIR__ . '/../rcredits-testing.inc'; // else fails in showForm()
include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

function test($arg = '') {
  global $user;
  include __DIR__ . '/../../gherkin/test.php';
/*  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate(); */
  \drupal_goto('handy');
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
  \drupal_goto('handy');
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
    'devel/cache/clear?destination=handy|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache',
    'devel/entity/info|View entity information across the whole site.|Entity info',
//    'devel/php|Execute some PHP code|Execute PHP Code',
    'devel/field/info|View fields information across the whole site.|Field info',
    'devel/reference|View a list of currently defined user functions with documentation links.|Function reference',
    'devel/elements|View the active form/render elements for this site.|Hook_elements()',
    'devel/menu/item?path=node|Details about a given menu item.|Menu item',
    'devel/phpinfo|View your server&#039;s PHP configuration|PHPinfo()',
    'devel/menu/reset?destination=handy|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Rebuild menus',
    'devel/reinstall?destination=handy|Run hook_uninstall() and then hook_install() for a given module.|Reinstall modules',
//    'devel/run-cron|Run cron|Run cron',
    'devel/session|List the contents of SESSION.|Session viewer',
    'devel/theme/registry|View a list of available theme functions across the whole site.|Theme registry',
    'devel/variable?destination=handy|Edit and delete site variables.|Variable editor',
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

function usdCallback($type, $uid) {
  include_once __DIR__ . '/../usd.class';  
  return r\usd::$type($uid);
}

/**
 * Complete a "no signin required" request (made by the user clicking on a link we sent).
 * @param assoc $args: the request parameters:
 *   id: the r_do record id
 *   code: a security code that must match the code in the record (or the request fails)
 * The action field of the r_do record, referenced by the id, includes some of these parameters:
 *   op: what to do
 *   uid: the related account record id
 *   sql: a database request, if any
 *   subs: substitutions for the database request
 *   amount: an amount
 *   
 * Possible values for op:
 *   min: change minimum to $amount
 *   sql: do the database operation
 *   addr: show an address (for shipping or thank-you card)
 * @see also r\makeDo()
 */
function rdo($args) {
  extract(u\just('id code', $args));
  if (!$info = db\lookup('action', 'r_do', 'doid=:id AND code=:code AND (completed=0 OR :REQUEST_TIME<-completed)', compact('id', 'code'))) return doSay(t('That link has expired.'));
  if (!$info = unserialize($info)) return doSay(t());
  
  $DBTX = \db_transaction();
  extract(u\just('op uid sql subs amount', $info));

  if ($op == 'min') {
    $result = r\acct($uid)->update(u\ray('minimum', $amount));
  } elseif ($op == 'sql') {
    $result = db\q($sql, $subs);
  } elseif ($op == 'addr') {
    $a = r\acct($uid);
    $phone = $a->co() ? t('Phone: ') . u\fmtPhone($a->phone) . "<br>" : '';
    $addr = t('Physical address: ') . r\location($a, TRUE);
    return doSay("$a->fullName<br>$a->postalAddr<br>$addr<br>$phone", 'status');
  } else return doSay(t('bad rdo'));

  if (!$result) return doSay(t('Request failed.'));
  db\update('r_do', u\ray('doid box completed', $id, box(), REQUEST_TIME), 'doid');
  unset($DBTX);

  return doSay(t('Your request was successfully completed.'), 'status');
}

/**
 * Automatically deactivate an account (called when the user changes the Dwolla email or password).
 */
function coup($usdEmail) {
  list ($email) = explode('@', $usdEmail);
  list ($code, $region, $id) = explode('.', $email);
/**/  if (!$a = r\acct(strtoupper("$region.$id"))) die('no1');
/**/  if (@$a->secure['usdEmail'] != $usdEmail) die('no2');
  if ($a->usd > 0) { // store its USD in ctty fund (single transfer for speed? many for hard-to-contest?)
    if (!r\tradeR(min(R_MAX_DAILY_EXCHANGE, $a->usd), $a->community, $a->id, $err) or $a->usd > R_MAX_DAILY_EXCHANGE) {
      r\tellStaff("Immediate coup threat: $a-fullName $$a->usd ($err)");
    }
  }
  $a->suspend(t('your Dwolla PIN, email, and/or password was changed.'));
  exit('ok');
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
      u.uid, u.fullName AS display, u.mail AS email 
      FROM users u LEFT JOIN r_txs t ON t.$other=u.uid 
      WHERE (u.name LIKE :string OR u.mail LIKE :string)
      AND (t.$role IS NULL OR :myid IN (t.payer, t.payee))
      ORDER BY t.created DESC, u.fullName
*/
    $selection = <<<EOF
      u.uid, u.fullName AS display, u.mail AS email 
      FROM users u WHERE (u.uid>1 AND (u.name LIKE :string OR u.mail LIKE :string))
EOF;
  } elseif ($role == 'industry') { // the one place we don't need to include the qid (for definiteness)
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry COLLATE utf8_general_ci AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName COLLATE utf8_general_ci AS display FROM users WHERE :IS_COMPANY AND name LIKE :string
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE :IS_COMPANY AND name LIKE :string)';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $mya = r\acct($myid);
    $cttyId = $mya->community;
    $personal = $role == 'ctty' ? '' : ' AND :IS_PERSON';
    $selection = "uid, fullName AS display FROM users WHERE (name LIKE :string AND community=:cttyId$personal)";
    $params .= ' cttyId';
  } else $selection = 'uid, fullName AS display FROM users WHERE (name LIKE :string)';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string sql=$selection"));
///return drupal_json_output(array('zot' => "subs=" . print_r(compact(u\ray('string myid')), 1)));

  $result = db\q($sql, compact(u\ray($params)));
  $matches = array();
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = r\quid($uid) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
//    return drupal_json_output(array('zot' => "display=$display"));
  }
  drupal_json_output($matches);
}

define('R_CONTRIB_SHOWHIDE', "var other = jQuery('.form-item-amount'); if(jQuery('#edit-gift').val() == 0) {other.show();} else other.hide();");

function formContribute($form, &$sta) {
  $mya = r\acct();
  $donated = $mya->share ? (t('<p>You have already made a contribution. Thank you! Please feel free to contribute again. Whatever "share" percentage you choose this time will replace your previous choice') . " ($mya->share%).</p>") : '';
  $giftLevels = array(
  //	10000 => 'Marble .. $10,000',
  //	 5000 => 'Granite ... 5,000',
    2500 => 'Slate .... $2,500',
    1000 => 'Glass ..... 1,000',
     500 => 'Iron ........ 500',
     250 => 'Oak ......... 250',
     100 => 'Two Bricks .. 100',
      50 => 'One Brick .... 50',
      25 => 'Half Brick ... 25',
       0 => 'Water ... (other)',
  );
  $oftens = array(0 => 'when?', 'M' => 'Monthly', 'Q' => 'Quarterly', 1 => 'Just Now (maybe more later)');
  $honors = array('-' => '(optional)', 'honor' => 'in honor of', 'memory' => 'in memory of', 'other' => 'other');
  $onchange = str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_CONTRIB_SHOWHIDE);
  $giftAttrib = attrib(compact('onchange'));
   
  $sharePct = round(db\lookup('AVG(share)', 'r_gifts', 'share>=0'), 1);
  $form = array(
    'title' => item(t('Contribute to Common Good Finance')),
    'subtext' => item($donated . t('<p>Make a gift of any size, to help support the rCredits system. Buy a BRICK or some other piece of the Common Good Economy, as a tax-deductible contribution (one-time, monthly, or quarterly). Also choose a percentage of your ongoing incentive rewards to share with Common Good Finance.') . ($mya->ok() ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>')) . t('</p><p>Thank you for your support!</p>')),
    'gift' => formField('select', t('Contribution:'), '', required($mya->co() ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT) + $giftAttrib, $giftLevels),
    'amount' => textField(t('Other amount $'), ''),
    'often' => formField('select', t('When:'), '', required(), $oftens),
    'honor' => formField('select', t('Honoring:'), '', '', $honors),
    'honored' => formField('textarea'),
    'share' => textField(t('Share:'), t('What percentage of your ongoing rebates and bonuses would you like to contribute to Common Good Finance, to support the rCredits system? This is another way to take responsibility for our community\'s future. You can put anything from 0% to 100%. The current average is sharePct%.', compact('sharePct')), required($mya->share)),
    'submit' => submit('Contribute'),
  );

  js(R_CONTRIB_SHOWHIDE, 'inline', 'footer');
  return labeled($form);
}

function formContribute_validate($form, &$sta) {
  js(R_CONTRIB_SHOWHIDE, 'inline', 'footer');
  extract(u\just('gift amount share often', $sta['input']));
  if (!$amount = $gift ?: $amount) return say('missing field', array('field' => 'amount'), 'amount');
  if (!$often) return say('missing field', array('field' => 'when'), 'often');
  $share = trim(str_replace('%', '', $share));
  if ($share === '') return say('missing field', array('field' => 'share'), 'share');
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  if ($err = u\badAmount($share, '>=0', 3)) return say($err, 'share');
  $share = min($share, 999.999); // any bigger kills PDO
  $sta['input'] = compact(u\ray('amount share')) + $sta['input'];
}

function formContribute_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('amount often honor honored share', $sta['input']);
  if ($info['honor'] == '-') $info['honor'] = 'honor';
  if (!$info['honored']) $info['honor'] = '';
  $uid = $mya->id;
  $giftDate = REQUEST_TIME;
  $info += compact('uid', 'giftDate');
  $donid = db\insert('r_gifts', $info);
  $mya->update(u\just('share', $info));
  $msg = 'gift successful';
  if (!r\acceptGift($info + compact('donid'))) {
    r\tellStaff('gift', $info);
    $msg .= '|gift transfer later';
  }
  $amount = u\fmtAmt($info['amount']);
  if (!$mya->ok()) {
    say("$msg|step completed", compact('amount'));
    r\go('status');
  } else say($msg, compact('amount'));
}

function formHelp($form, &$sta, $what = '') {
  require_once __DIR__ . '/rweb-help.inc';
  $title = item($what ? ('"' . ucwords(str_replace('-', ' ', $what)) . '"') : t('General Help'));
  $text = formField('item', '', '', helpText($what ?: 'general'));
  return compact(u\ray('title text'));
}

/**
 * Show the account's current membership status and steps to get to the next milestone.
 * @param acct $a: the account to test if just updating membership status bits -- otherwise empty
 * @param string $code: numeric means show specific screen even if not appropriate (for in-person demos)
 * @return TRUE if the member has done everything possible toward opening the account.
 */
function formMembership($a, &$sta = '', $code = '') {
  global $base_url;
  ;
  if (!($updating = (bool) $a) and !$a = r\acct()) return NULL; // no account available
  if ($updating and $a->ok()) return TRUE;

  $myid = $a->id;  
  $nonperson = !$a->person();
  $mempage = is_numeric($code) ? $code : FALSE;
  $dw = $a->can(B_DW);

  if ($mempage or !$a->can(B_MEMBER) or !$a->ok()) {
    if ($dw and !$a->can(B_PHOTOID) and ($result = setupUs($step)) and t\est()) return $result;
    $nearly = $a->nearlyMember($steps);
    extract(u\just('signed donated', $steps));
    $photoOk = $a->hasAux('picture');
    $gotProxies = $nonperson ?: (db\lookup('COUNT(*)', 'r_proxies', 'person=:myid', compact('myid')) >= 2);
//    r\membershipEvent($a, 'member', $nearly and $gotProxies);
//    $gotIdProof = (@$a->dob and @$a->federalId and ($nonperson ?: $a->hasAux('proof')));
//    $gotSecurity = (($secure = @$a->secure) and @$secure['auth'] and @$a->dob and @$a->federalId and ($nonperson ?: $a->hasAux('proof')));
//    $contactOk = ($step and $a->postalAddr and !in_array($step, u\ray('Email Phone Address')));
    $contactOk = ($a->postalAddr and (@$step == 'Finished' or !$dw));
    $bankOk = (!$dw or @$a->secure['bankAccount']);
    $prefsOk = !is_null($a->minimum);
    $ready = ($nearly and $gotProxies and $contactOk and $photoOk and ($nonperson or $bankOk) and $prefsOk);
    if ($nonperson) {
      $companyOk = (bool) @$a->selling;
      $relationsOk = @$a->data['relationsOk'];
      $ready = ($ready and $companyOk and $relationsOk);
    }
    r\membershipEvent($a, 'member', $ready);
  } else $ready = TRUE;

  if ($updating) return $ready;
  
  $message1 = $message2 = $message3 = '';

  /*if ($mempage == .5 or (!$contactOk and !$mempage)) { // did all the initial steps, but somehow didn't complete contact info
    $a1 = "a href=\"$base_url/account/contact\"";
    $message1 = u\tt('missing contact info', compact('a1'));
  } else */
  if ($mempage == '4' or ($ready and !$a->ok() and !$mempage)) {
    $nextMsg = $nonperson ? t('you can begin accepting rCredits payments') : t('you can expect to receive your rCard in the mail within a day or two');
    $message1 = t("<h2>Your Account Setup Is Complete</h2><p>You have done everything you need to do. Once a staff member has approved your account, $nextMsg.</p><p>Thank you for joining us!</p>");
  } elseif ($mempage == '1' or (!$a->ok() and !$mempage)) { // not a member yet (no rCard, no vote)
    $message1 = ($photoOk or $signed or $donated or ($gotProxies and !$nonperson) or $contactOk or ($bankOk and !$nonperson) or $prefsOk) ? 
          t("<h2>You're getting there!</h2><p>")
        : t('<h2>Welcome to rCredits</h2><p>Thanks for joining us. ');
    $message1 .= $nonperson ? t('Before you can begin accepting rCredits payments, you must complete the following steps.') 
      : t('Before we can send your rCard, and before you can vote on community funding, you must complete the following steps. This usually takes at least 20 minutes of your time, with a 3-day wait for the bank connection.');
    $message1 .= tt(' If you need help, please <a href="mailto:@R_ORG_EMAIL" target="_blank">send us an email</a>.</p>');

//    $step1 = t('Complete your <a>Contact Information</a>.'); // keep this in case
    $stepAgree = t('Sign the <a>rCredits Agreement</a>.');
    $stepContact = t('Complete your <a>Contact Information</a>.');
    $stepGift = t('Make a <a>Contribution</a> of <i>any size</i>, to support the rCredits system.');
    $stepProxies = t('<a>Choose two people</a> to represent your views whenever you are not there, to discuss and vote on community funding issues. (A list of members in your area will be provided, to choose from.)');
    $stepPrefs = t('Set your <a>Account Preferences</a>.');
    $stepPhoto = t('Upload a <a>Photo</a> for your ') . ($nonperson ? t('company profile.') : t('Member ID Card.'));
/*    $stepSecurity = t('<a>Provide your security information.</a> Upload ')
      . ($nonperson ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotSecurity ? t(' <span class="pending">(DONE, pending approval)</span>') : ''); */
    $stepBank = t('Connect your <a>Bank Account</a> (your doorway to the mainstream economy).') . ($nonperson ? t(' <em>(optional for companies)</em>') : '');
/*    $stepDwolla = t(<<<EOF
    <p>Open a <a href="$base_url/help/dwolla-account-setup">Dwolla</a> account and <a href="$base_url/account/usd">Connect</a> it to your rCredits account (click "Dwolla" and "Connect").</p>
EOF
    );
    $stepProof = t('<a>Prove your identity.</a> Upload ')
      . ($nonperson ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotIdProof ? t(' <span class="pending">(DONE, pending approval)</span>') : '');*/
    $stepCompany = t('Tweak your <a>Company Information</a>.');
    $stepRelations = t('Set up <a>Relations</a> to employees and suppliers.');

    $stepNum = 1;
    $message2 = memberStep($stepNum++, $signed, $stepAgree, 'agreement')
              . memberStep($stepNum++, $contactOk, $stepContact, 'account/contact')
              . memberStep($stepNum++, $donated, $stepGift, 'contribute');
              
    if (!$nonperson) 
    $message2 .= memberStep($stepNum++, $gotProxies, $stepProxies, array('account/proxies', 'invite'));

//    $message2 .= memberStep($stepNum++, $gotSecurity, $stepSecurity, 'account/security');
//    $message2 .= memberStep($stepNum++, $usdOk, $stepDwolla, R_DW_LINK, TRUE);
    $message2 .= memberStep($stepNum++, $prefsOk, $stepPrefs, 'account/preferences')
               . memberStep($stepNum++, $photoOk, $stepPhoto, 'account/photo');

    if ($dw) $message2 .= memberStep($stepNum++, $bankOk, $stepBank, 'account/bank');
//    if (!$nonperson) $message2 .= memberStep($stepNum++, $gotIdProof, $stepProof, 'account/id-proof');
    if ($nonperson)
    $message2 .= memberStep($stepNum++, $companyOk, $stepCompany, 'account/company')
               . memberStep($stepNum++, $relationsOk, $stepRelations, 'account/relations');

// (Suggest it after 1mo) if (!$nonperson) $message2 .= memberStep(4, $invited, $step4, 'invite');
  } elseif ($mempage == 2 or (!$a->ok() and !$mempage)) {
    $message1 = t('<h2>You have completed all membership steps.</h2><p>Within 24 hours, you should receive an email saying your account is approved and you can expect your rCard in the mail within a day or two. Hurray!</p>'); // UNUSED
  } else {
    $bonusMsg = ($a->can(B_BONA) or ($a->can(B_BANK) and $a->minimum > 0)) ? '' : t("<p>To receive your $" . R_SIGNUP_BONUS . " signup bonus, you must have a way to get money into your account. Ask someone to pay you with rCredits OR <a href=\"$base_url/get\">transfer funds from you bank account</a>. As soon as you get some money in, your signup bonus will arrive.</p>");
    $personMsg = $a->person() ? t(' You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>') . $bonusMsg : '</p>';
    $message1 = t('<h2>Congratulations! Your account is Activated.</h2><p>You are authorized to use rCredits to buy and sell with other members.') . $personMsg;
  }
  $message = $message1 . "<table width=\"100%\">$message2</table>" . @$message3;
  
  $title = item(t('Membership Steps'));
//  $subtext = item(t('for ') . $a->fullName);

  $steps = formField('item', '', '', $message);
  $form = compact(u\ray('title steps addendum'));
  return labeled($form);
}

/**
 * Give member a chance to sign the rCredits Agreement.
 */
function formAgreement($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
  $id = "edit-$name";
  $check = boxField('', '', attrib(compact('id', 'name')));
  $model = \render($check);
  $instructions = '';

  if ($date = $mya->signed) {
    $instructions = '';
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif ($mya->person() and !$mya->proSe() and !$mya->admin()) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $aacct = $mya->admin() ? $mya : $mya->agent;
    $signedBy = textField(t('Signed:'), t('Type your full name here ') . "($aacct->fullName)");
    $submit = submit(t('Sign'));
    $addendum = MINI_LAUNCH ? t('During the Pilot Phase, this means paying attention to your experience and making comments to improve the system before it goes public. The amount of rCredits circulating during this Pilot Phase will intentionally be kept small.') : '';
    $instructions = t(<<<EOF
      <div id='instructions'>
      <p><b>Instructions:</b> Put a check mark in each checkbox to indicate that you have read that section and agree to it. Really DO read the agreement and make sure you understand it. This agreement, that we make with each other, is the foundation for the rCredits system and the Common Good Economy.</p>

      <p><b>Summary.</b> In the first part of the agreement (A), you acknowledge the intent and purpose of the rCredits system. In the second part (B), you agree to use the rCredits system the way it is designed to work. Notice that you can stop participating at any time, simply by giving back the rCredits you got for free (B4). Unlikely that you will <i>want</i> to stop, but you can.</p>
      
      <p><b>Responsibility.</b> Since the purpose of the rCredits system is to give us control of our local economy, as a community, the agreement requires that each member be responsible. You agree to take on that responsibility "as [your] life circumstances allow".$addendum</p>
      </div>
EOF
    );
  }

  $personOnly = !$mya->person() ? '' : <<<EOF
  <li>[6] I agree to participate responsibly in managing the Common Good Economy to the best of my ability and as my life circumstances allow, including:
    <ol style="list-style-type:lower-alpha;">
      <li><a href="$base_url/help/paying-attention">paying attention</a> and understanding how the Common Good Economy works</li>
      <li>participating in the <a href="$base_url/help/decision-process">decision process</a> to set my community's funding priorities:
        <ol style="list-style-type:lower-roman;">
          <li>to foster healthy, fulfilling lives for all community members and</li>
          <li>to assist communities elsewhere</li>
        </ol></li>
      <li>participating in <a href="$base_url/help/other-decisions">other decisions</a> from time to time, as needed.</li>
    </ol>
  </li>
EOF;

  $agreement = <<<EOF
$instructions
<div id="agreement">
<h2>A. I recognize that:</h2>
<ul>
  <li>[-2] rCredits are intended as the basis for the Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1] Together as a community we have the power to <a href="$base_url/help/issue-credit">issue credit</a> for our own use as a medium of exchange. It is our promise to accept rCredits that gives them value.</li>
  <li>[0] Governance by the people is essential, for a just society.</li>
</ul>

<h2>B. As a responsible participant in the Common Good Economy:</h2>
<ol>
  <li>[1] As long as I can spend my rCredits easily or exchange them easily for US Dollars, I will accept them without limit as payment for my goods and services. I will exchange rCredits for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will accept rCredits from members of any <a href="$base_url/help/common-good-community">Common Good Community</a> in good standing.</li>
  <li>[3] Whenever I exchange rCredits for US Dollars or vice versa, I will exchange them one for one, and I will price my goods and services the same in rCredits and in US Dollars. </li>
  <li>[4] I understand that I will receive some rCredits from the community as Incentive Rewards. If I stop participating (or <a href="$base_url/help/graceful-failure">if the system ends</a>) within 6 months of receiving my first rCredits or before the community has enough revenue to cover the Incentive Rewards, I will <a href="$base_url/help/graceful-failure">immediately give back</a> whatever amount of Incentive Rewards I have received (while keeping any rCredits I received from other participants).</li>
  <li>[5] I promise to maintain a high level of honesty, integrity, and <a href="$base_url/help/ethics">ethics</a> in my dealings with other rCredits participants and with the community (including the rCredits system itself). When there is a dispute, I will follow the <a href="$base_url/help/dispute-resolution-process">Dispute Resolution Process</a> and will honor its outcome.</li>
  $personOnly
</ol>
</div>
EOF;
//  <li>[5] I will move my deposits and investments to the Common Good Economy when that is sensible, in my judgment.</li>

  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", $date ? '' : $box, $agreement);
  }
  $agreement = formField('item', '', '', $agreement);
  $form = compact(u\ray('title agreement signedBy submit'));
  return $form;
}

function formAgreement_validate($form, &$sta) {
  $mya = r\acct();
  $i9 = $mya->person() ? R_AGREE_9 : R_AGREE_9 - 1;
  for ($i = R_AGREE_0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  $fullName = r\agent()->fullName;
  if (!$mya->admin() and strcasecmp(trim($sta['input']['signedBy']), $fullName) != 0) return say('bad signature', compact('fullName'), 'signedBy');
}

function formAgreement_submit($form, &$sta) {
  $signed = REQUEST_TIME;
  extract(u\just('signedBy', $sta['input']));
  r\acct()->update($info = compact('signed', 'signedBy'));
  say('info saved|step completed');
  r\tellStaff('agreement signed', $info);
  r\go('status');
}

/**
 * Choose default proxies.
 */
function formProxies($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $id = $mya->id;
  
  $title = item(t('Proxies'));
  $subtext = item(t(<<<EOF
    <p><b>Direct democracy</b> is at the heart of the rCredits design. In each community, members meet to discuss and decide on funding priorities for grants, loans, and investments of rCredits for the greater good. (Although we won't be doing that for several months yet.)</p>
    <p>You assign a person in your community as your ongoing Representative (Proxy). This should be a member you know and trust and generally agree with. Whenever you don't vote, your Representative votes for you. The Representative's vote simply counts double (one vote for you and one vote for the Representative). If your Representative also fails to vote, then your Representative's Proxy votes for all three of you.<br><br>Everyone's voice is counted, whether or not they vote directly &mdash; one person one vote.</p>
    <p>Our community's most trusted Representatives (chosen by many people to be their proxy) will meet in person to research and debate specific issues. These trusted Representatives ("Trustees") do not decide broad policy issues &mdash; all members decide the policy issues. The Trustees oversee the public discussions, choose wording for questions to be voted on, take action as directed by the members, and assure that the system is running smoothly.</p>
    <p>If you don't see the people you want listed here, <a href="$base_url/invite">invite them</a>!</p>
EOF
  ));
//<p>You can identify your representatives (in the boxes below) by name, email, or member ID. Click "Help with this page" for a list of choices during the Pilot Phase.</p>

  /*
  $proxy1 = textField(t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'), required(r\definiteName(r\proxy($id, 1))) + autocomplete('cttyPerson'));
  $proxy2 = textField(t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'), dft(r\definiteName(r\proxy($id, 2))) + autocomplete('cttyPerson'));
  */

  $ctty = $mya->community;
  $myid = $mya->id;
  $region = r\uidRegion($myid);
  $choices = array();
  $result = db\q('SELECT uid, fullName, city FROM users WHERE uid>1 AND uid<>:myid AND :IS_PERSON AND community=:ctty AND :R_ACCT_REGION=:region ORDER BY postalCode, city, fullName', compact(u\ray('myid ctty region')));
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $choices[$uid] = "$fullName ($city)";
  }
  $choices = array(0 => empty($choices) ? t('--No other members nearby. Invite someone!--') : t('--Select a proxy--')) + $choices; // tricky to keep PHP from re-indexing

  list ($p1, $p2) = array(r\proxy($id, 1), r\proxy($id, 2));
  $proxy1 = formField('select', t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'), required($p1), $choices);
  $proxy2 = formField('select', t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'), required($p2), $choices);
  $submit = submit(t('Save'));
  $form = compact(u\ray('title subtext proxy1 proxy2 submit'));
  return labeled($form);
}

function formProxies_validate($form, &$sta) {
  extract($fields = u\just('proxy1 proxy2', $sta['input']));
  foreach ($fields as $field => $value) if (!$value) return say('required field', compact('field'), $field);
/*    if (!$proxy = $sta['input'][$proxyName]) continue; // ignore blanks
    if (!$proxy = whois($proxy, $proxyName, $sta['input'])) return FALSE;
    $name = $proxy->fullName;
    if (!$proxy->person()) return say('inhuman proxy', compact('name'), $proxyName);
    $sta['input'][$proxyName] = $proxy->id; */
  if ($proxy1 == $proxy2) return say('doubled proxy', 'proxy2');
}

function formProxies_submit($form, &$sta) {
  $myid = r\acct()->id;
  extract($info = u\just('proxy1 proxy2', $sta['input']));
  if ($proxy1) r\proxy($myid, 1, $proxy1);
  if ($proxy2) r\proxy($myid, 2, $proxy2);
  if ($proxy1 and $proxy2) {
    r\go('status');
    r\tellStaff('chose proxies', $info);
    say('info saved');
  } else say('proxy to go');
}

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 */
function formInvite($form, &$sta) {
  global $base_url, $rUrl;
  $agent = r\agent();
  if ($admin = $agent->admin()) $agent = r\acct('NEW.AAA'); // use William's info for admin invites
  
  $title = item(t('Invite Someone to rCredits'));
  $nearly = $admin ?: $agent->nearlyMember();
  $subtext = item(!$nearly ? t("We're sorry, you cannot invite someone to sign up until you have signed the <a href=\"$base_url/agreement\">Agreement</a> and made a <a href=\"$base_url/contribute\">contribution</a>.<br><br>Return to the <a href=\"$base_url/status\">Membership Steps</a> page to see where you stand.") : (t('Adjust the subject and message, then click "Send". For now, please invite only Greenfield-area companies and people who live, shop, or work in the area of Greenfield, Massachusetts. The invitation will include your contact info.')));
//  $subtext = item(strtr($subtext, array('<a>' => "<a href=\"$base_url/status\">")));
//  $contact = textField(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());
  $email = formField($admin ? 'textarea' : 'textfield', t('Email:'), t('An email address for the person you want to invite'), required());
  if ($admin) $dupsOk = boolField(t('Dups Okay?'), '', FALSE);
  $trusted = boolField(t('Trusted?'), t('If you lent this person $250, would you trust them to pay it back?'), dft(@$admin ?: NULL));
  $subject = textField(t('Subject:'), t('The subject of your message'), required($agent->fullName . t(' invites you to rCredits')));

  $default = $admin ? 
      tt("rCredits is up and running. Check it out and sign up!\n\nWilliam")
    : tt("rCredits looks amazing. I signed up. Check it out!\n\n@name", u\ray('name', $agent->fullName));
  $blurbLink = array('<a>' => "<a href=\"$rUrl/templates/invite.html\" target=\"_blank\">");
  $messageHelp = strtr(t('Invite them to sign up, and say why. A <a>blurb about rCredits</a> will follow your message, with a signup link.'), $blurbLink);
  $message = formField('textarea', t('Message:'), $messageHelp, required($default));
  $submit = submit(t('Send'));
  $form = $nearly ? 
      labeled(compact(u\ray('title subtext email dupsOk trusted subject message submit')))
    : compact(u\ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(u\just('email trusted', $sta['input']));
  if (r\acct()->admin()) {
    foreach (u\ray($email) as $one) if (!\valid_email_address(trim($one))) return say("bad email: $one", 'email');
  } elseif (!\valid_email_address($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  extract(u\just('email subject message dupsOk', $sta['input']));
  $email = strtolower($email); // otherwise u\SUBS breaks on cap after @
  if (r\acct()->admin()) {
    foreach (u\ray($email) as $one) sendInvite($one, $subject, $message, $dupsOk, 0);
  } else sendInvite($email, $subject, $message);
}

function formVideos($form, &$sta) {
  $title = item(t('rCredits Member Videos'));
  $subtitle = formField('item', '', '', t('The videos may not be available until mid-January. Try again in a few days!'));
  $form = compact(u\ray('title subtitle'));
  return labeled($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from r\go() in Summary

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 */
function formSummary($form, &$sta) {
  global $base_url;
  if (!$mya = r\acct()) return r\go('/user'); // this cannot be '' (infinite redirect)
///  debug($mya->can(B_ALL)); // keep
  $agent = $mya->agent;
  $mya->refreshCan(); // update any permissions that may have been set elsewhere (superfluous?)

  if ($mya->admin() ? !$mya->can(B_MEMBER) : !formMembership($mya)) {
      say('take a step');
  } else {
    if (!$mya->ok()) say('setup complete');
    say('adjust settings');
    if ($mya->can(B_DW) and !$mya->can(B_BANK)) say('verify bank');
  }

  $name = $mya->name;
  $floorDesc = t('Your balance can go this low') . ($mya->floor < 0 ? t(' for up to 30 days') : '')
    . '. ' . t('See "Debt Okay" in') . " <a href=\"$base_url/account/preferences\">Preferences</a>.";
  
  if ($mya->can(B_CTTY_ADMIN) and !$mya->proSe()) {
    $extras = '<br>' . u\fmtPhone($mya->phone) . " ($mya->faxetc)";
    $idLink = 'href="print-rcard/' . $mya->mainQid . '" target="_blank"';
    if ($mya->person()) $extras .= " <a $idLink>" . t('Print ID Card') . '</a>';
  } 
  
  extract(u\just('r usd pAccts', r\stats($mya->community)));
  
  extract(u\just('benePastMo beneEver extraPastMo extraEver avgBalPastMo avgBalEver giftsEver', $mya->data));
  $years = round((strtotime('today') - strtotime('today', $mya->created)) / DAY_SECS) * DAY_SECS / YEAR_SECS;
  // rounding is to assure consistency for tests, despite daylight savings time changes
  $returnMo = @$avgBalPastMo > 0 ? 
      number_format(100 * 12 * (@$benePastMo + @$extraPastMo) / $avgBalPastMo, 1)
    : (@$benePastMo + @$extraPastMo > 0 ? '&infin;' : number_format(100 * R_INFLATION_RATE, 1));
  $return = ($years > 0 and @$avgBalEver > 0) ? 
      number_format(100 * (@$beneEver + @$extraEver) / $avgBalEver / $years, 1)
    : (@$beneEver + @$extraEver > 0 ? '&infin;' : number_format(100 * R_INFLATION_RATE, 1));
  $cttyBeneMo = u\fmtAmt(@$giftsPastMo + max(0, @$bankedPastMo) + @$benePastMo * (1 + R_CC_RATE / $mya->rebate));
  $cttyBene = u\fmtAmt(@$giftsEver + max(0, @$bankedEver) + @$beneEver * (1 + R_CC_RATE / R_REBATE) + \variable_get('cttyPaidEver', 0) / (@$pAccts ?: 1));

  $title = item(t('Account Summary'));
//  'subtext = basic_account_info();
  $accountName = item($mya->fullName . ' ' . u\hug($name), t('Name:'), $mya->postalAddr . @$extras);
  $balance = u\fmtAmt($mya->r);
  $usdBalance = u\fmtAmt($mya->usd);
  $total = u\fmtAmt($mya->r + $mya->usd);
  $balance = item($total . ($mya->rewards ? " (see <a href=\"$base_url/transactions\">" . t('Transaction History') . '</a>)' : ''), t('Balance:'));
//  $balance = item(tt("$balance@R + $usdBalance@US = $total") . ($mya->rewards ? " [<a href=\"$base_url/transactions\">" . t('see Transaction History') . '</a>]' : ''), t('Balance:'));
  
  if ($mya->id > 0) {
    $personal = $mya->person();
    $acctType = ($personal ? t('personal') : t('company')) . t(' account');
    if ($agent->can(B_MEMBER)) $another = submit(t('Open Another Account'));

    $extra = $agent->admin() ? " id#$mya->id" : '';
    $accountId = formField('item', t('ID:'), '', ($mya->mainQid ?: 'N/A') . ' (' . $acctType . ")$extra");
    $floor = formField('item', t('Credit floor:'), $floorDesc, u\fmtAmt($mya->floor));
    
    $rewards = formField('item', t('Rewards:'), t('to date'), u\fmtAmt($mya->rewards));
    $committed = formField('item', t('Committed:'), t('rewards reserved for contribution to Common Good Finance'), u\fmtAmt(max(0, $mya->committed)));
    // return and bene
    $returnMo = item("$returnMo% APR " . t('over the past month'), t('Your return:'));
    $return = item("$return% APR " . t('overall (ever)'), ' ', t('Your effective rate of return on your average balance [<@a>details</a>]', u\ray('@a', 'a href=help/your-return')));
    $cttyBeneMo = item("$cttyBeneMo " . t('over the past month'), t('Social return:'));
    $cttyBene = item("$cttyBene " . t('overall (ever)'), ' ', t('How much your participation has benefited the community financially [<@a>details</a>]', u\ray('@a', 'a href=help/social-return')));
  }

  if ($agent->can(B_CTTY_ADMIN) and (!$mya->proSe() or $mya->id == 1)) { // ctty admin not managing own personal account
    $ctty = $mya->community;
    if ($mya->id < 0) { // managing the community itself
      $count = formField('item', t('Members:'), '', db\lookup('COUNT(*)', 'users', 'community=:ctty AND uid<>:ctty', compact('ctty')));
    } else { // managing some individual community member account
//      $acctType = formField('select', t('Account type:'), '', $mya->acctType+0, $GLOBALS['account types']); // +0 for admin
//      if ($mya->co()) $canCharge = boolField(t('Can charge'), t('This account can charge others unilaterally (in person)'), $mya->can(B_CHARGE) + 0);
      $floor = textField(t('Credit floor:'), $floorDesc, dft(u\fmtAmt($mya->floor)));
      $rTrader = boolField(t('rTrader?'), t('Is this account approved to use rCredits?'), $mya->ok() + 0);

      $bits = u\ray(B_ADMINABLE . ($mya->admin() ? ' admin' : ''));
      $adminableDefaults = array();
      foreach ($bits as $bitName) {
        $bit = u\consta('b', $bitName);
        $adminables[$bit] = $bitName;
        if ($mya->hasBit($bit)) $adminableDefaults[] = $bitName;
      }
      $adminable2 = boxesField('adminable', t('Other bits:'), '', $adminableDefaults, $adminables);

      $notes = formField('textarea', t('Notes'), '', dft($mya->notes));
    }
    $who = textField(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete($mya->admin() ? '' : 'ctty'));

    $submit1 = $submit2 = submit(t('Go'));
  }

  $form = $mya->can(B_MANAGE) ? compact(u\ray('title another who submit1 accountName accountId balance rewards committed returnMo return cttyBeneMo cttyBene floor canCharge rTrader adminable2 notes count submit2')) : compact(u\ray('title accountName accountId submit2'));
  return labeled($form);
}

function formSummary_validate($form, &$sta) {
  $mya = r\acct();
  if (op($sta) == 'another') return;
  extract(u\just('who floor rTrader adminable', $sta['input']));

  if ($rTrader and is_null(@$adminable[B_MEMBER])) return say(t('That account is not READY (not a member).'), 'zot'); // isnull because 0
  if (isset($floor)) {
    if ($error = u\badAmount($floor)) return say($error, 'floor'); 
    $sta['input']['floor'] = u\cleanAmount($floor);
  }
  if (@$who) {
    if (!$acct = whois($who, 'who', $sta['input'])) return FALSE;
    $sta['input']['who'] = $acct;
  }
}

function formSummary_submit($form, &$sta) {
  $mya = r\acct();
  $agent = $mya->agent;
//  if (op($sta) == 'another') return r\go($mya->co() ? 'agent' : 'another');
  if (op($sta) == 'another') return r\go('another');

  if (!$agent->can(B_CTTY_ADMIN)) return; // hack attempt
  extract(u\just('who floor notes canCharge rTrader adminable', $sta['input']));

  if (@$who and ($who->community == $agent->community or $agent->id == 1)) {
    svar('myid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $mya->fullName);
    return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
  } 
  
  if ($mya->id > 0) {
//    $mya->setBit(B_CHARGE, @$canCharge, FALSE);
    r\membershipEvent($mya, 'ok', $rTrader); // activate the account
    $mya->setBit(B_OK, $rTrader, FALSE);

    $mya->setBits(B_ADMINABLE, $adminable, 'b');
    $mya->update(compact(u\ray('floor notes')));
    say('info saved');
  }
}

/**
 * Upload a photo.
 */
function formPhoto($form, &$sta) {
  global $base_url;
  $mya = r\acct();

  $form0['#user'] = $mya->account();
  user_account_form($form0, $sta);
  extract(u\just('picture_upload picture_current', $form0['picture']));
  $title = item(t('Profile Photo'));
  
  $has = $mya->hasAux('picture');
  $picture_current['#markup'] = $mya->picture(TRUE);

  list ($width, $height) = explode(':', $mya->person() ? R_PERSONAL_ASPECT : R_COMMERCIAL_ASPECT);
  $width *= R_PIXEL_FACTOR;
  $height *= R_PIXEL_FACTOR;
  $picture_upload['#description'] = 
    ($mya->person() ? 
        tt('Upload a recent picture of yourself for your photo-ID rCard. It must be taller than wide (@R_PERSONAL_ASPECT ratio is ideal) and must show your full face clearly, filling most of the frame.')
      : tt('Upload a picture to represent your company in rCredits promotions. The picture must be wider than tall (@R_COMMERCIAL_ASPECT ratio is ideal).')) 
    . tt(' File size must be under 3MB. The picture will be cropped and resized to @width x @height pixels.', compact('width', 'height'));
  
  if ($has and $mya->person() and $mya->ok()) {
    $picture_upload['#description'] .= tt('<p><b>NOTE:</b> If you change your photo, a new rCard will be mailed to you and your account will be charged $@R_CARD_COST.</p>');
    $picture_upload = item(R_ON_FILE);
  }
  $submit = submit(t('Upload and Save'));

  $form = compact(u\ray('title picture_current picture_upload submit'));
  return $form;
}

function formPhoto_validate(&$form, &$sta) {
  $validators = array(
    'file_validate_is_image' => array(),
    'file_validate_size' => array(R_MAX_UPLOAD_SIZE * 1024000),
  );

  $file = \file_save_upload('picture_upload', $validators); // Save the file as a temporary file.
  if ($file === FALSE) {
    say('photo upload failed', 'picture_upload');
  } elseif ($file !== NULL) $sta['values']['picture_upload'] = $file;
}

function formPhoto_submit($form, &$sta) {
  global $base_url;
  if (!$picture = @$sta['values']['picture_upload']) {
    say('nothing done');
    r\go('status');
  }  

  $mya = r\acct();
  $pictureFilename = DRUPAL_ROOT . $mya->auxFilename('picture', TRUE);

  db\q('DELETE FROM file_managed');
  $tempName = str_replace('temporary://', \file_directory_temp() . '/', $picture->uri);
  $aspect = $mya->person() ? R_PERSONAL_ASPECT : R_COMMERCIAL_ASPECT;
  if (!u\fixPicture($tempName, $aspect, R_PIXEL_FACTOR, $err)) {
    if ($err == 'aspect') $err = $mya->person() ? 'bad personal aspect' : 'bad commercial aspect';
    return say($err, 'picture');
  }
  if (!rename($tempName, $pictureFilename)) return say('file save error', 'picture'); // (rename fails)
  $msg = 'photo saved' . ($mya->ok() ? '' : '|step completed|return to membership');
  $link = "$base_url/status";
  say($msg, compact('link'));
  if ($mya->ok()) {
    $return = $mya->person() ? 'account/security' : 'account/company';
    return r\go($return);
  }
}

function formChangePassword($form, &$sta) {
  $mya = r\acct();
  $form['#user'] = $mya->account();
  user_account_form($form, $sta);
// Each region has its own timezone.  system_user_timezone($form, $sta);

  $form['title'] = formField('item', t('Change Password'), '', weight(-99));
  $form['account']['pass']['#description'] = NULL;
  //$form['account']['pass']['#process'] = array('form_process_password_confirm', 'user_form_process_password_confirm'); //'rCredits\\Web\\password_confirm');
  if (!$mya->admin()) {
    $form['account']['current_pass'] = formField('password', t('Current Password:'));
  } else unset($form['account']['current_pass']);
  $form['strong'] = hidField(0, attrib(array('id' => 'edit-strong')));

  foreach (u\ray('mail name timezone status roles') as $one) unset($form['account'][$one]);
  unset($form['picture']);
  $form['submit'] = formField('submit', '', '', array('value' => t('Save')) + weight(99));
  $form['#user_category'] = 'account'; // ?
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'] = array('rCredits\\Web\\formChangePassword_validate'); // use ours instead
  return $form;
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formChangePassword_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass current_pass', $sta['values'])); // values NOT input
  extract(u\just('strong', $sta['input'])); // input NOT values
  if (!$mya->admin() and empty($current_pass)) return say('password required', 'current_pass');
//    and ($mail != $form['account']['mail']['#default_value'] or !empty($pass))
  if (!$strong) return say('weak pass', 'pass');
//  if (!emailOkay($sta['values']['mail'] = $mail = trim($mail), $mya->id)) return say('bad email', 'mail');
  if (!passwordOkay($mya, $current_pass)) return FALSE;
}

function formChangePassword_submit($form, &$sta) {
  extract($info = u\just('pass', $sta['values'])); // values NOT input

  if (empty($pass) or !r\acct()->update(compact('pass'))) return r\go('security', t('Your password remains unchanged.'));
//  if (r\acct()->update(u\ray('pass', r\passHash($pass)))) {
  say(t('Your new password has been saved.'));
  return r\go('account/security');
}

function formChangePin($form, &$sta) {
  $mya = r\acct();

  $title = item(t('Change PIN'));
  if (!$mya->admin()) {
    $currentPass = formField('password', t('Current Password:'));
  }
  $pin = formField('password', t('New PIN:'));
  $submit = submit();
  
  return labeled(compact(u\ray('title currentPass pin submit')));
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formChangePin_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass currentPass pin', $sta['input']));
  if (!$mya->admin()) {
    if (empty($currentPass)) return say('password required', 'currentPass');
    if (!passwordOkay($mya, $currentPass, 'currentPass')) return FALSE;
  }
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
}

function formChangePin_submit($form, &$sta) {
  extract($info = u\just('pin', $sta['values'])); // values NOT input
  $mya = r\acct();
  
  $secure = compact('pin') + $mya->secure;
  if (empty($pin) or !$mya->update(compact('secure'))) return r\go('security', t('Your PIN remains unchanged.'));
//  if (r\acct()->update(u\ray('pass', r\passHash($pass)))) {
  say(t('Your new pin has been saved.'));
  return r\go('account/security');
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  global $base_url, $rUrl;
  if (!$mya = r\acct()) return NULL; // no account selector before login
  $myid = @$mya->id;
  if (!$accounts = be\accountChoices()) return FALSE;
  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or ctty_admin
  $choices = array();
  foreach ($accounts as $uid) {
    $acct = r\acct($uid);
//    $balance = u\fmtAmt($acct->balance());
//    $choices[$uid] = "$acct->fullName: $balance";
    $choices[$uid] = substr($acct->fullName, 0, 25); // keep it short
    if ($mya->admin()) $choices[$uid] = substr($acct->mainQid, -2, 2) . ' ' . $choices[$uid];
  }

  $src = $base_url . $mya->agent->auxFilename('picture');
  $signOut = t('Sign out');
  $choices['signout'] = $signOut;
//  $icon = item(imageButton('user/logout', t('Sign out'), $src)); // (should be a dropdown menu)
  $icon = item('<img id="my_icon" src="' . $src . '"/>');

//  $qid = r\quid($mya->id, $mya->agentId, $mya->regionId);
//  $settings = item(imageButton('account/settings', 'Settings', 'gear.png'));
  if ($mya->can(B_MANAGE)) $settings = item(divButton('account/settings', 'Settings'));

  //  $oneText = $choices[$myid];
//  $account = count($choices) == 1 ? 
//      textField('', '', dft($oneText) + attrib(array('disabled' => TRUE, 'size' => strlen($oneText)+2)))
//  $onchange = "this.form.submit();";
  $onchange = "location.href='$base_url/change-account?acct=' + this.options[this.selectedIndex].value + '&page=' + encodeURIComponent(document.URL); jQuery('#saywait').show();";
  $account = formField('select', '', '', dft($myid) + attrib(compact('onchange')), $choices);
//  $link = item('<a href="http://rCredits.org">Main Site</a
  $submit = submit(); // default submission

  $chunks = explode('/', $_SERVER['REQUEST_URI']);
  if (isDEV) array_shift($chunks); // site is in a subfolder on the dev server
  $function = @$chunks[1]; // normally just one part of URI is the page (but sometimes two)
  foreach (u\ray('account user') as $one) {
    if (in_array($one, $chunks) and @$chunks[2]) $function .= '/' . $chunks[2];
  }

  $functionName = ucwords($function);
  $help = imageButton("help/$function", "Help for $functionName", 'qmark-white.png');
  $menu = imageButton('menu', 'Show Menu', 'menu.png');
  
//  $showMenu = formField('item', '', '', button('Menu', '', 'Show the menu'));
//  $showHelp = formField('item', '', '', button('Help', '', 'Help for ' . ucwords($function)));
  $showMenu = item($menu);
  $showHelp = item($help);
//  $version = item(t('v 0.1d'));
  $form = u\prefixKeys('acct_', compact(u\ray('settings account icon showMenu showHelp submit')));

//  $menu = formMenu($function);

  $help = help($function);
  $form += $help; // + $menu;
  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op acct_account', $sta['input']));
  $op = op($sta);
  if ($op == 'Menu') return r\go('menu'); 
}

/**
 * Change the current account.
 */
function changeWho($args = '') {
  extract(u\just('acct page', $_GET));
  if ($acct == 'signout') r\go('/user/logout');
  
  setupGlobals();
  $mya = r\acct();
  if (!@$mya->admin() and !r\acct($acct, $mya->agentId)->can()) hack('accts');

  $mya = r\acct($acct);
  svar('myid', $mya->id); // this gets checked ONLY in setupGlobals()
  $mya->setBank();
  r\go($page);
}

/**
 * Settings form for mobile.
 */
function formSettings2($form, &$sta) {
  $menu = array()
//    + goButtonIf(B_MANAGE, t('Basic'), 'account/basic', t('Email and password'))
    + goButtonIf(B_MANAGE, t('Contact Info'), 'account/contact', t('Tell us where to find you'))
    + goButtonIf(B_MANAGE, t('Preferences'), 'account/preferences', t('Set your account preferences'))
    + goButtonIf(array(B_PERSONISH, B_MANAGE), t('Security'), 'account/security', t('Adjust your security settings or passwords'))
//    + goButtonIf(B_MANAGE, t('Photo'), 'account/photo', t('Upload a photo for your rCard'))
//    + goButtonIf(B_MANAGE, t('Bank'), 'account/bank', t('Connect to your US Dollar bank account'))
    + goButtonIf(array(B_COMPANY, B_MANAGE), t('Company Info'), 'account/company', t('Describe this company'))
    + goButtonIf(B_MANAGE, t('Relations'), 'account/relations', t('Connect other members to this account'))
    + goButtonIf(B_MANAGE, t('Devices'), 'account/boxes', t('Authorize mobile devices for this account'))
    + goButtonIf(array(B_PERSON, B_MANAGE), t('Proxies'), 'account/proxies', t('Choose two people as your representatives'))
//    + goButtonIf(array(B_PERSON, B_MANAGE), t('ID Proof'), 'account/id-proof', t('Provide proof of your identity'))
  ;
  $title = item(t('Settings'));
  $menu = formField('fieldset', '', '', array('id' => 'settings')) + compact('menu');
  return compact('title', 'menu');
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company) {
  $uid = r\userField('uid', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or !$acct->co()) return say('no such company', 'ERR');

  $pic = $acct->picture();
  $state = r\realState($acct->state);
  $country = r\realCountry($acct->country);
  if ($info = db\lookup('website,description', 'r_companies', 'uid=:uid', compact('uid'))) extract($info);
  $siteList = '';
  if (@$website) foreach (u\ray($website) as $one) $siteList .= "<a href='http://$one' target='_blank'>$one</a><br>";
  $description = @$description ?: '';
    
  $phone = 'Phone: ' . u\fmtPhone($acct->phone, '+');
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = db\q($sql, compact('uid'))->fetchAllKeyed();
  $cats = empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats));

  $markup = <<<EOF
  $pic
  <div id='member-details'>
    <div id='member-contact'>
    <h2>$acct->fullName</h2>
    $acct->address<br>
    $acct->city, $state $acct->postalCode $country<br><br>
    $phone<br>
    $siteList<br>
    </div>
    <div id='member-description'>$description</div>
    <div id='member-categories'>$cats</div>
  </div>
EOF;

  $form = array(
    'all' => formField('item', $markup),
  );
  return $form;
}

define('R_SHOWABLE_TABLES', 'users txs usd gifts invites notices proxies relations sms log');

/**
 * Say the site is down
 */
function formSystemDown($form, &$sta) {return array('zot' => item(tt('system down')));}

/**
 * Make various administrative operations available to the admin.
 */
function formAdmin($form, &$sta, $args = '') {
//  $form = u\insertBefore(compact('communities'), formCommunity(array(), &$sta, $args), 'count');

/* (This dropdown might be better here than at the top of every page)
   (top of page dropdown would then include only the CURRENTLY selected community (and the admin))
  $rows = db\q('SELECT uid, fullName FROM users WHERE uid<0 AND uid<:R_CTTY_UID0 ORDER BY fullName')->fetchAllKeyed();
  $cttyChoices = array_merge(array('ctty-0' => 'Select a community to manage'), u\prefixKeys('ctty', $rows));
  $community = formField('select', t('Manage:'), t('Select a community to manage'), '', $cttyChoices);
*/
  $title = item(t('System Administration'));
  if (r\up()) $stop = submit(t('STOP')); else $start = submit(t('START'));
  
  $account = textField(t('Account:'), t('Account to do something to'),auto());
  $make = boxfield(t('Make:'));
  $printId = submit(t('Print ID'));
  $seeSecure = submit(t('See Secure'));
  $newUsdEmail = submit(t('New USD Email'));
  $form = compact(u\ray('title stop start'));
  foreach (u\ray('USD_Accounts Gfld tells ' . R_SHOWABLE_TABLES) as $one) $form[$one] = submit(str_replace('_', ' ', $one));
  $form += compact(u\ray('account make printId seeSecure newUsdEmail'));
  return $form;
}

function formAdmin_submit($form, &$sta) {
  include_once __DIR__ . '/../admin.inc';
  adminForm($sta);
}

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 */
function formTx($form, &$sta, $args = '') {
  if ($confirm = sureForm($sta)) return $confirm;
  extract(u\just('who amount goods purpose scanned', $args));
  $mya = r\acct();
  if (!@$scanned and !$mya->can(B_SELL)) return r\go('summary');

  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path())); // pay, charge, etc.
  $role = in_array($type, u\ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, u\ray('fine grant loan invest'));
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($type == 'charge' ? @$selling[0] : ($type == 'pay' ? '' : $type));
  
  focusOn(@$who ? 'amount' : 'who');

  //$onchange = "nixwhat();";
  $radios = array(t('cash/loan/etc.'), t('goods & services:'));
  
  // set up form fields
  $title = item(ucwords($type));
  $whoLabel = ($type == 'loan' ? 'Lend to' : ucwords($type)) . ' whom:';

//  $onchange = $incentive ? '' : "if(this.value.toUpperCase() == 'ALL') jQuery('#edit-purpose').val('payroll');";
  $who = textField($whoLabel, t('Type a name, id, email, or phone'), dft(@$who) + autocomplete($role));

  $amount = textField('Amount: $', t('Amount to ') . $type, dft(@$amount));
  $dollarSign = item('$'); // for mobile
  $goods = $incentive ? hidField(FALSE) : formField('radios', '&nbsp;', '', dft(isset($goods) ? $goods : 1), $radios); //  + attrib(compact('onchange'))
  $purpose = textField('For:', t('Description of goods and services or purpose'), dft(@$purpose));
  $calc = item(u\calculator('amount'));
  $scanClass = @$scanned ? 'scanned' : 'not-scanned';
  $scanned = hidField(@$scanned); // scanned means user already saw a name and photo
  $$type = submit(ucwords($type));

  $form = labeled(compact(u\ray("title who amount dollarSign goods purpose calc $type scanned")));
  $form['#attributes']['class'][] = $scanClass;

  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  if ($scanned = @$sta['input']['scanned']) return; // skip confirmation if scanning
  if (confirming_v($sta)) return;
//  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // not needed
  if ($confirm = transfer(op($sta), $sta['input'], FALSE)) confirm($confirm, $sta); // op who amount goods what
}

function formTx_submit($form, &$sta) {
  global $fieldErr;
  $scanned = @$sta['input']['scanned'];
  if (confirming_s($sta)) return;
  transfer($op = op($sta), $sta['input'], TRUE); // op who amount goods what
  if (!$fieldErr) { // this is especially important when skipping the confirmation (scanning)
    if ($scanned) say('to scan another');
    return r\go($scanned ? 'empty' : $op); // shun tx page after scan regardless of perms
  }
}

function formMenu($form, &$sta) {return array('title' => item('Menu'));}
function formEmpty($form, &$sta) {return formMenu($form, $sta);}

function formSettings($form, &$sta) {
  $mya = r\acct();
  $tabs = u\ray('contact preferences security bank company relations boxes proxies');
  $captions = u\ray(t('Contact,Preferences,Security,Bank Info,Company,Relations,Devices,Proxies'));
  $title = item('Settings');
  foreach(array_combine($tabs, $captions) as $tab => $caption) {
    $img = imageButton("account/$tab", $caption, "$tab.png");
    $icons[$tab] = item("<div class=\"img\">$img</div><div class=\"caption\">$caption</div>");
  }
//  $icons['photo'] = str_replace('.png', rand(0, 1) ? '-f.png' : '-m.png', $icons['photo']);
  if (!$mya->person()) {
    unset($icons['security']);
    unset($icons['proxies']);
  }
  if (!$mya->co()) unset($icons['company']);
  $icons = item(\drupal_render($icons));
  return compact(u\ray('title icons'));
}

/**
 * Pay with rCredits, arriving from some other site.
 * @todo: handle <user logged in> better
 */
function formBuy($form, &$sta) {
  $in = @$sta['input'] ?: (svar('external_input') ?: $_GET); // $_GET is always an array here
  extract($in, EXTR_PREFIX_ALL, 'my'); // get input from outside site or (stored) from login
  if (@$my_external_input) extract(unserialize($my_external_input), EXTR_PREFIX_ALL, 'my');
  $confirmed = @$my_confirmed ?: (!@$sta['input'] and user_is_logged_in());
  
  if (!$confirmed) {
    foreach (u\ray('amount company item code') as $field) {
      $my_field = "my_$field";
      if (!@$$my_field) return say('missing field', compact('field'), $field); // handle hackers gracefully
    }
    if (!is_numeric($my_amount)) return say('bad amount', 'amount');
    if (!u\isQid(@$my_company) or !($acct = r\acct($my_company))) return say('bad account id', 'company'); // qid?
    
    $company_name = $acct->fullName; 

    $verbs = u\ray('Contribute, Donate, Pay');
    $verb = in_array(ucwords(@$my_verb), $verbs) ? ucwords($my_verb) : 'Pay';

    $button = user_is_logged_in() ? submit($verb)
      : item(t('By typing your username and password, you agree to pay with rCredits, as detailed above.'));

    $my_amount = @number_format($my_amount, 2);
    $disabled = array('disabled' => 1);
    $verblower = strtolower($verb);
  }
  
  list ($amount, $goods) = array($my_amount, TRUE);
  list ($who, $what) = @$my_confirmed ? array($my_who, $my_what) : array($my_company, $my_item);

  if (@$confirmed) {
    transfer('pay', compact(u\ray('who amount goods what')), TRUE);
    if (u\valid_url(@$my_return_to)) return array(item("<a href='$my_return_to'>Click here</a> to return to the $company_name website."));
    r\go('');
  }

  $form = array(
    'title' => item(t('Confirm Payment')),
    'xwho' => textField('to:', t('Pay whom?'), disabled($company_name)),
    'who' => hidField($who),
    'xamount' => textField('$', t("Amount to $verblower"), disabled($amount)),
    'amount' => hidField($amount),
    'xwhat' => textField('for:', t('Description of goods and services'), disabled($what)),
    'what' => hidField($what),
    'goods' => hidField($goods),
    'return_to' => hidField(@$my_return_to),
    'confirmed' => hidField(TRUE),
    'submit' => $button,
  );

  return labeled($form);
}

function formBuy_submit($form, &$sta) {
//  formTx_submit($form, $sta);
//  drupal_goto('charge'); // member page when done
}

/**
 * Exchange USD for rCredits or vice-versa (the Bank tab)
 */
function formGet($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $isSetUp = !is_null($mya->minimum); // user has already saved some options
  $title = item(t('Bank Transfer'));
  $subtext = item(t('Transfer funds to or from your connected bank account. For automatic transfers from your bank account, as needed, adjust your "minimum" setting on the <a href="' . $base_url . '/account/preferences">Preferences</a> page. Transfers may take 3-5 days.') . ($mya->dwStatus() == DW_VSAVINGS ? t(' Savings and Money Market accounts (like yours) are limited by law to 6 transfers per month.') : ''));

//  $minimum = textField(t('Minimum:'), tt('prompt min'), dft(u\fmtAmt($isSetUp ? $mya->minimum : R_MINR_DEFAULT, 's')));
  $ray = (array) $mya->account(R_MONEY_FIELDS);
  $toSpend = u\fmtAmt(r\avail('buy', $ray));
  $waiting0 = $mya->waitingToBank();
  if (!db\exists('r_usd', 'payer=:myid AND payee=0 AND amount>0 AND created>:dayAgo', u\ray('myid dayAgo', $myid, strtotime('-1 day')))) { // limit to one withdrawal request per day
    $myAvail = r\avail('cash', $ray); // drawing from connected accounts not allowed here
    $avail = db\lookup('SUM(LEAST(usd, :DW_FEE_THRESHOLD))', 'users', 'community=:ctty and uid<>:myid', u\ray('ctty myid', $mya->community, $myid)); // remove this one-day limit if people complain (cost = 25 cents)
    $avail = min($myAvail, $mya->usd + $avail); // what i have or what USD is available, whichever is less
  } else $avail = 0;
  $toCash = u\fmtAmt(max(0, $avail));
  $waiting = u\fmtAmt(abs($waiting0));
  $way = $waiting0 < 0 ? 'from' : 'to';
  $waiting = item($waiting == '$0' ? t('You have no pending transfer requests.') : tt('You have total pending transfer requests of @waiting @way your bank account.', compact('waiting', 'way')), t('Pending:')); // amount already requested (if any)
  $avail = hidField($avail);
  $balance = item(tt('balances', compact('toSpend', 'toCash')), t('Balance:'));
  $amount = textField(t('Amount:'), t('Transfer how much?'));
  $get = submit('FROM bank');
//  $fromBank = formField('checkbox', t('Automate:'), t('(recommended) Refill my rCredits account from my bank account automatically, as needed.'), dft($isSetUp ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = textField(t('Maximum:'), tt('prompt max'), dft(u\fmtAmt($isSetUp ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = textField(t('Excess'), '', dft(u\fmtAmt($mya->overMax(), 's')));
//  $toBank = formField('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  $put = submit('TO bank');

  return labeled(compact(u\ray('title subtext waiting balance amount get put avail')));
}
  
function formGet_validate($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('amount avail', $sta['input']));
  if ($err = amtErr('amount', $sta)) return say($err, 'amount');
  if ($amount < R_BANK_MIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > $avail) {
    $max = u\fmtAmt($avail);
    return say('short put', compact('max'), 'amount');
  }
//  $a1 = 'a href="' . $base_url . '/account/preferences"';
  if ($mya->r + $mya->usd - $amount < $mya->minimum) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  $us = new r\usd($mya = r\acct());
  extract(u\just('amount', $sta['input']));

  if (op($sta) == 'put') {
    $action = 'deposit to';
    if (!$mya->cacheOk()) return r\go('', 'cannot bank', u\ray('action error', $action, t('data integrity issue')));
    if ($need = max(0, $amount - $mya->usd)) {
      if (!$got = r\getUsd($need, $mya) or $got < $need) return say('cannot offload', 'amount');
    }
  } else list ($action, $amount) = array('draw from', -$amount);
  
  if ($us->bank($amount, $error) or isDEV) { // ignore lack of verified bank connection, if testing
    $amount = u\fmtAmt(abs(@$amount));
    say('banked', $subs = compact(u\ray('action amount')));
    r\tellStaff('banked', $subs);
  } else return say('cannot bank', compact('action', 'error'), 'amount');
}

/**
 * Create a pseudo-account (with no personal data and no money)
 */ /*
function formAgent($form, &$sta) {
  $mya = r\acct();
  if (!@$mya or $mya->person()) r\go(''); // no error message, in case user changed accounts
  $title = item(t('Open an rPOS Sign-in Account'));
  $subtext = item(t('Use this form only for cashiers who do not wish to participate in the rCredits system as individuals.'));
  $optedOut = boolField(t('Opted out:'), t('Was this agent invited to open a personal rCredits account? And did this agent decide not to?'));
  $fullName = textField(t('Name:'), t('The person\'s full name'), required());
  //$permissions = formField('radios', t('Permissions:'), '', required(), u\ray('read only,charge customers'));
//  $employee = boolField(t('Employee?'), t('Is this person an employee of the company?'));
//  $pay = textField(t('Pay:'), t('How much is this agent paid by the company each month (on average)?'), required());
  $submit = submit(t('Create Account'));
  return labeled(compact(u\ray('title subtext optedOut fullName submit')));
}

function formAgent_validate($form, &$sta) {
  extract(u\just('optedOut fullName', $sta['input']));
  if (!$optedOut) return say('must opt', 'optedOut');
  if (u\badName($fullName)) return say('bad name', 'fullName');
//  if (u\badAmount($pay)) return say('bad amount', 'pay');
}

function formAgent_submit($form, &$sta) {
  extract(u\just('fullName permissions', $sta['input']));
  $a = new r\acct(compact('fullName'));
//  $perms = ($permissions ? B_SCAN : B_READ) - B_RELATED;
  $info = u\ray('main other permission', r\acct()->id, $a->id, B_SCAN - B_RELATED);
  if (db\insert('r_relations', $info)) return r\go('', 'agent created');
}
*/

/**
 * Prepare to open another account.
 * This form creates an invite and redirects to signup, using that code.
 * @see also: formAgent
 */
function formAnother($form, &$sta) {
  if (!r\agent()->can(B_MEMBER)) return r\go('', 'members only', 'ERR');
  
  $title = item('Open Another Account');
  $relations = array(t('a company and I own at least 10% of it'), t('a company or organization that I work for, but I am not a 10%-or-more owner'), t('a family member or friend, with whom I have a primarily personal rather than financial relationship'), t('myself individually or jointly with others -- not for business'), t('none of the above'));
  $relationPrompt = item(t('What is the new account for?'));
  $relation = formField('radios', t(''), '', '', $relations);
  $flows = array(t('neither way'), t('current account can cover new account\'s debts and overdrafts'), t
('new account can cover current account\'s debts and overdrafts'), t('both ways'));
  $flowPrompt = item(t('Which way can the credit flow, when you need it to?'));
  $flow = formField('radios', t(''), '', '', $flows);
  $submit = submit(t('Open Account'));
  return labeled(compact(u\ray('title relationPrompt relation flowPrompt flow submit')));
}

function formAnother_validate($form, &$sta) { 
  foreach (u\ray('relation flow') as $field) {
    $$field = $sta['input'][$field]; // values fails here (?!!)
    if (!is_numeric($$field)) say('required field', compact('field'), $field);
  }
//  if ($relation == 4 and $flow == 3) say('insufficient relation', 'relation');
  if ($relation == 4) say('insufficient relation', 'relation');
}

function formAnother_submit($form, &$sta) {
  $a = r\agent();
  if (!$a->can(B_MEMBER)) return r\go('', 'members only', 'ERR');

  extract(u\just('company companyPhone isOwner employeeOk', $a->data));
  extract(u\just('relation flow', $sta['input']));
  $by = $a->id;
  $code = r\invite($a->mail, $a->id);
  $flow += 0;
  $isOwner = (@$isOwner or $relation == 0);
  $employeeOk = (@$employeeOk or $relation == 1);
  $personal = ($relation > 1);
  if (!$personal and (@$company or @$companyPhone)) {
    if (findCompany(@$company, @$companyPhone)) list ($fullName, $phone) = array(@$company, @$companyPhone);
  }
  \session_destroy();
  r\go('signup/' . http_build_query(compact(u\ray('code personal fullName phone isOwner employeeOk flow by'))));
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
 *   by: uid of account creating this one -- non null (same as $inviter) means created with formAnother
 */
function formSignup($form, &$sta, $args = '') {
  if (!$args) $args = basename(t\SERVER('REQUEST_URI')); // because menu_execute_active_handler fails
  extract(u\just('code personal fullName phone dwok', $args));

  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost
  u\setDft($personal, TRUE);

  $title = item(t('Sign up for rCredits'));

  if (@$code != 'w' and !$email = invitation(@$code, '', $inviter, $err)) {
    if (@$inviter) {
      $a = r\acct($inviter);
      $inviterEmail = htmlspecialchars($a->mail);
      $inviterName = htmlspecialchars($a->fullName);
      $a = "a target=\"_blank\" href=\"mailto:$inviterEmail?subject=invite me again?&body=Hi $inviterName, the rCredits invitation you sent me expired. :( Please send me another one. :)\"";
    }
    $subtext = item(tt($err, compact('a', 'inviterName')));
    return compact(u\ray('title subtext'));
  } elseif (!@$dwok) {
    r\setCook('dupPhone', FALSE);
    return dwollaTerms(); // Dwolla's terms not yet approved
  }

//  If you want to use this address <b>for more than one account</b>, include a tag on the local part starting with "+" (for example, myemail<b>+whatever</b>@example.com).');

  $subtext = item(t(<<<EOF
Opening an rCredits account is very much like opening an online bank account. It takes about that much time. After this signup page there are seven more short steps we will guide you through. Opening your account is the hard part &mdash; after that it gets much easier. If you need help, please don't hesitate to ask (<a href="mailto:info@rCredits.org?subject=signup help">click here to send us an email</a>).
EOF
  ));

  if ($personal) {
    $acctType = hidField(R_PERSONAL);
    $calling = textField(t('Life Calling:'), t('One of the main purposes of rCredits is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do, to earn rCredits (that you\'re not doing yet)? What might you have to offer?'));
    $companyOptions = array(
      'isOwner' => t('owner'),
      'employeeOk' => t('employee'),
      'contractor' => t('other worker (contractor, supplier, etc.)'),
    );
    $copts = boxOpt('copts', $companyOptions, $sta);

    $companyInfo = array(
      'company_text' => item(t('Are you an owner and/or employee of a local company, or of any rCredits member company? Otherwise leave this part blank.')),
      'company' => textField(t('Company: '), t(''), autocomplete('company')),
      'companyPhon' => textField(t('<small>Company</small> Ph<span>&nbsp;</span>one:'), t('What is the company\'s phone number?')), // Google Chrome bug require obfuscation of "phone"
      'companyOptions' => boxesField('copts', '', '', @$copts, $companyOptions), 
    );
    $companyInfo = formField('fieldset', '', '', array('id' => 'companyInfo')) + $companyInfo;
  } else { // company
    $acctType = acctTypeField();
  }

  $fullName = textField(t('Full name:'), tt('fullName desc'), required(@$fullName));
  $mail = textField(t('Email:'), t('All emails from the system will be sent to this address. It will not be made public.'), required());

  $buysAts = db\q('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
  $ats = boxOpt('ats', $buysAts, $sta);
  $buysAt = boxesField('ats', t('<br>Over 60 Greenfield businesses have already signed up. Which of these <b>Not-Yet-Member Businesses</b> do you frequent?'), '', @$ats, $buysAts);

  $submit = submit(t('Sign up'));
  $args = hidField(@$args);
  $inviter = hidField(@$inviter);
  
  $form = compact(u\ray('title subtext acctType fullName mail')) 
         + ssnFields($personal) + contactFields(compact('phone'))
         + compact(u\ray('calling companyInfo buysAt submit args inviter'));
//  keep_values($form);
  return labeled($form);
}

function formSignup_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = array(); // for testing (keep Drupal from griping)
  extract(u\just('agreed args inviter acctType mail phone dupOk country company companyPhon copts fullName postalCode', u\trimAll($sta['input'])));

  if (op($sta) == 'continue') { // don't require Dwolla checkbox, so we can give a good message
    return @$agreed ? '' : say('dwolla terms required', 'agreed');
  }
 
  if (@$args) extract(u\just('code', $args));
  if (!invitation(@$code) and @$code != 'w') return r\go('signup'); // in case user resubmits
  //and @$code != bin2hex(R_WORD)

  if (!emailOkay($mail)) return;
  if (!phoneOk($phone, $country, @$dupOk ? $inviter : FALSE)) return;
  if ($country == US_COUNTRY_ID) {
    if (!preg_match('/^\d{5}(\-\d{4})?$/', $postalCode)) return say('bad zip', 'ERR');
  }
  if (!formBasic_validate($form, $sta, $acctType == R_PERSONAL)) return;

  if (@$copts) extract(u\just('isOwner employeeOk contractor', $copts));
  $gotrelation = (@$employeeOk or @$isOwner or @$contractor);
  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (u\ray('company companyPhon') as $one) {
      //if (!@$$one) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $data = compact(u\ray('company companyPhone employeeOk isOwner contractor'));
  } else $data = array();
  
  if ($err = u\badName($fullName)) return say($err, array('field' => 'fullName'), 'fullName');

  $community = r\communityUid($postalCode);
  $flags = B_DFT | u\bit($acctType == R_PERSONAL ? B_PERSON : B_COMPANY) | (@$dupOk ? 0 : u\bit(B_DW));
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  u\preray(compact(u\ray('fullName mail community data flags administer_users status')), $sta['input']);
}

function formSignup_submit($form, &$sta) {
  $dbFields = 'fullName mail community data flags phone federalId dob country postalCode';
  extract(u\just('calling acctType verifyBy args company ats inviter ' . $dbFields, $sta['input']));
  if (op($sta) == 'continue') return r\go('signup/' . urldecode(basename($_SERVER['REQUEST_URI'])) . '&dwok=1'); // got agreement, so continue

  if (t\est()) { // testing
    if (!@$args) $args = basename(t\SERVER('REQUEST_URI'));
    if (!@$data) $data = array();
  }
  u\EXPECT($args, 'no args');
  extract(u\just('code by', $args));
  
  if (!emailOkay($mail)) return; // I don't know how people create duplicate accounts, but maybe this will prevent it. (ws 1/19/2014)

  $DBTX = \db_transaction();
  $wants = wants(@$ats);

  $data += u\ray('by wants calling verifyBy', @$inviter, $wants, @$calling, verifyBy(@$verifyBy));
  $where = u\parseZip($postalCode) ?: array(); // gets $state and $city
  $fullName = u\normalizeCase($fullName);

  // @todo: try this 2 or 3 times (in case someone else was trying to register the same name
  $name = r\uniqueName($fullName);
  if ($acctType == R_PERSONAL) $pass = oneTimePass($name, $data); // record one time pass in data
  $info = compact(u\ray('name ' . $dbFields)) + $where + acctType($acctType);
  if (!$mya = new r\acct($info)) return say('cannot create account', 'zot');
  
  $myid = $mya->id;
  u\EXPECT($myid > 0, 'registration failure');
  setupGlobals($myid); // some rcredits functions need to know who we are
  
  invitation($code, $myid); // mark the invitation used
  
  if (@$company) suCompanyRelation($company, $myid, $data);
  if (@$by and !r\acct($by)->can(B_CTTY_ADMIN)) suCreatorRelation($args, $myid);
  
  // report to staff and user
  $quid = $mya->mainQid;
  tellStaffNewMember($info + $data + compact('acctType', 'quid'));

  if ($acctType == R_PERSONAL) {
    say('your account is ready', compact('quid'));
    $region = strtolower(R_SERVER_ID);
    r\notify($myid, 'welcome', compact(u\ray('name quid pass region')), TRUE); 
  } else { // non-personal company (no independent login)
    say('company is ready', compact('quid'));
    if (@$by) $quid = r\quid($by); // "if" for dev faked code HUH?
    if (!@$by and !$mya->admin()) hack('company no by');
  }
  unset($DBTX); // commit
  //$sta['redirect'] = 'user/login/name=' . strtolower($quid);
  //$us = new r\usd($mya);
  //$us->getAuth();
  return r\go('/user/login/');
}

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  $mya = r\acct();
  extract((array) $mya->account('fullName mail postalAddr'));
  focusOn('fullName');
  $fullNameDesc = tt('fullName desc') . ($mya->co() ? '<br>' . t('(If you change this, your username and rCredits webpage address will also change.)') : '');

  $title = item(t('Contact Information'));
  $fullName = textField(t('Full name:'), $fullNameDesc, required(@$fullName));
  $onchange = "jQuery('.form-item-pin').show();";
  $mail = textField(t('Email:'), '', attrib(compact('onchange')) + required(@$mail));
  if (!$mya->admin() and isset($mya->secure['pin'])) $pin = formField('password', t('PIN:'), t('Required for changing your email address'));

  $postalAddr = textField(t('Postal Address:'), t('Where does the post office send your mail (usually)?'), required(@$postalAddr));
  $submit = submit();

  $form = compact(u\ray('title fullName mail pin')) 
        + contactFields()
        + compact(u\ray('postalAddr submit'));
  
  return labeled($form);
}

function formContact_validate($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  $info = u\trimAll($sta['input']); // trim ALL values, for later
  extract(u\normalizeCase(u\just($normals = 'fullName city address postalAddr', $info)));
  extract(u\just('mail pin phone country state', $info));

  // what's this doing in validate?!
//  js('inc/countries.js', 'file', 'header'); // country and state dropdowns
//  js("print_country(\"$country\", \"$state\");", 'inline', 'footer'); // initialize dropdowns

  if ($fullName != r\acct()->fullName) {
    if ($err = u\badName($fullName)) return say($err, array('field' => 'fullName'), 'fullName');
    $name = u\shortName($fullName);
    $otherName = r\userField('fullName', 'name=:name AND uid<>:myid', compact('name', 'myid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
  }

  if (!emailOkay($mail, $mya->id)) return; // say('bad email', u\ray('email', $mail), 'mail');
  if (!$mya->admin() and $mail != $mya->mail and $pin != $mya->agent->secure['pin']) return say('bad pin', 'pin');
  if (!phoneOk($phone, $country)) return;
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) say('po in location', 'address');
  
  $sta['input'] = compact(u\ray($normals . ' mail phone name')) + $info; // fix formatting and add name
}

function formContact_submit($form, &$sta) {
  extract($info = u\just('fullName name phone verifyBy fax country postalCode state city address postalAddr', $sta['input']));
  $mya = r\acct();
  if ($mya->id == 1) unset($info['name']); // don't change 'admin'

  unset($info['verifyBy']);
  if (isset($verifyBy)) $data = u\ray('verifyBy', verifyBy(@$verifyBy)) + $mya->data; // @ for testing
  $info = u\normalizeCase(u\just('fullName city address postalAddr', $info)) + $info + compact('data');

  if ($mya->update($info)) {
    say('info saved');
// (problematic if addPhone fails)  if (!$mya->ok()) say('step completed');
    return r\go($mya->ok() ? 'account/settings' : 'status');
  } else say('not saved');
}

/**
 * Ask for the 5-digit phone verification code that Dwolla sent.
 */
function formVerifyPhone($form, &$sta) {
  $mya = r\acct();
  $title = item(t('Verify Phone'));
  $subtext = item(t('<p>A verification code has been sent to your phone (@phone). Please enter it here.</p><p>Or, if you need to use a different telephone, <@a>click here to return to the Contact Information page</a>.</p>', u\ray('@phone @a', u\fmtPhone($mya->phone), 'a href=contact')));
  $code = textfield(t('Code:'), t('<@a>Send it again?</a>', u\ray('@a', 'a href=readd-phone')));
  $submit = submit(t('Continue'));
  return labeled(compact(u\ray('title subtext code submit')));
}

function formVerifyPhone_validate($form, &$sta) {
  if (strlen($code = u\digits($sta['input']['code'])) != 5) return say('bad phone code', 'code');
  $us = new r\usd($mya = r\acct());
  if (!$us->verifyPhone(svar('phoneId'), $code, $err) and $us->step() == 'Phone') return say($err, 'code');
}

function formVerifyPhone_submit($form, &$sta) {
  return r\go('status', 'phone verified');
}

/**
 * Offer to revise data that affects social security number verification.
 */
function formBasic($form, &$sta) {
  $mya = r\acct();
  $nameDesc = t(', properly capitalized, as on your most recent government-issued ID');
  $title = item(t('Retry Verification'));
  if ($personal = $mya->person()) {
    list ($first, $last) = u\parseName($mya->fullName);
    $first = textField(t('First name:'), t('Your first name') . $nameDesc, required(@$first));
    $last = textField(t('Last name:'), t('Your last name') . $nameDesc, required(@$last));
  } else {
    $org = textField(t('Full name:'), tt('fullName desc'), required($mya->fullName));
    $acctType = acctTypeField();
  }
  $submit = submit();

  return labeled(compact(u\ray('title first last org acctType')) + ssnFields($personal) + compact('submit'));
}

define('R_BASIC_FIELDS', 'first last org federalId dob');

/**
 * Validate the data that affects social security number verification.
 * called from signup validation, as well as from formBasic()
 * @param string/bool $personal: is this a personal account ('mya' means use the current account's type)
 *   'mya' also means we are retrying basic information (the AccountInfo step) during signup
 */
function formBasic_validate($form, &$sta, $personal = 'mya') {
  extract(u\just(R_BASIC_FIELDS, u\trimAll($sta['input'])));
  if ((string) $personal == 'mya') {
    $us = new r\usd($mya = r\acct());
    $personal = $mya->person();
    $dw = $mya->can(B_DW);
  }
  $what = $personal ? 'social security number' : 'federal ID';
  if ($personal) {
    list ($min, $max) = array(REQUEST_TIME - 150 * 365.25 * DAY_SECS, REQUEST_TIME + 9 * 31 * DAY_SECS);
    if ($err = u\badDate($dob, $min, $max)) return say($err, 'dob'); // badDate makes dob internal
  }
  if ($err = u\badSsn($federalId, @$dob)) return say($err, compact('what'), 'federalId');

  if (@$mya) { // retrying account info
    if ($personal) { // personal
      if ($err = u\badName($first)) return say($err, 'first');
      if ($err = u\badName($last)) return say($err, 'last');
      $emptyErr = t('There was a problem submitting your data. Try again?'); // (this happened early on)
      if ($dw and !$us->ssn($federalId, $err)) return say(@$err ?: $emptyErr, 'ERR');
      if ($dw and $us->step() == 'AccountInfo' and !$us->accountInfo($dob, '', $first, $last, '', '', $err)) {
        return say(@$err ?: $emptyErr, 'ERR');
      }
    } else { // org
      if ($err = u\badName($org)) return say($err, 'org');
      if ($dw and !$us->accountInfo('', $federalId, '', '', $org, $mya->businessStructure(), $err)) {
        if ($err) return say($err ?: $emptyErr, 'ERR');
      }
    }
  }
  u\preray(compact(u\ray(R_BASIC_FIELDS)), $sta['input']);
  return TRUE;
}

function formBasic_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just(R_BASIC_FIELDS, $sta['input']));
  $fullName = $mya->person() ? trim("$first $last") : $org;
  if ($mya->update(compact(u\ray('fullName federalId dob')))) say('info saved');
  return r\go('status'); // on to next step (or total failure)
}

function getqs() {
  return json_decode ('[
  {
    "Id": "36848018",
    "Text": "According to your DRIVER\'S LICENSE, what COLOR is your HAIR?",
    "Choices": {
      "181649905": "AUBURN",
      "181649906": "BLOND",
      "181649907": "GREY",
      "181649908": "BROWN",
      "181649909": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848019",
    "Text": "Which of the following colleges have you PREVIOUSLY OR CURRENTLY attended?",
    "Choices": {
      "181649910": "TOCCOA FALLS COLLEGE",
      "181649911": "ARKANSAS TECH UNIVERSITY",
      "181649912": "CITY COLLEGES OF CHICAGO",
      "181649913": "HUSSIAN SCHOOL OF ART",
      "181649914": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848020",
    "Text": "According to your DRIVER\'S LICENSE, what COLOR are your EYES?",
    "Choices": {
      "181649915": "BROWN",
      "181649916": "GREEN",
      "181649917": "BLACK",
      "181649918": "BLUE",
      "181649919": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848021",
    "Text": "In which of the following CITIES have you NEVER lived or used in your address?",
    "Choices": {
      "181649920": "MOUNT VERNON",
      "181649921": "WOODSIDE",
      "181649922": "FLUSHING",
      "181649923": "EAST ELMHURST",
      "181649924": "ALL OF THE ABOVE"
    }
  }
  ]');
}

/**
 * Ask a set of personal questions to confirm the person's identity.
 * If the answers fail 3 times (or even if the user refreshes the screen 3 times), the PhotoId step is next.
 */
function formKba($form, &$sta) {
  $us = new r\usd($mya = r\acct());
  $form['title'] = item(t('Confirm Your Identity'));
//  if ($questions = getqs() and $transactionId = 123) { // debug
  if ($transactionId = $us->kba($questions, $err)) { // get the set of questions
    foreach ($questions as $qi => $question) {
      extract(u\just('Id Text Choices', $question));
      // Mysteriously, $Choices[$key] (or previously $Choices->$key) cannot be referenced
      // For example, if $key is '181649905', "echo $Choices[$key];" gives "Undefined offset: 181649905..."
      foreach ($Choices as &$value) $value = strlen($value) == 2 ? $value : u\titleCase($value);
      unset($value);
      $form["q-$Id"] = formField('radios', $Text, '', required(), (array) $Choices);
    }
    $form['transactionId'] = hidField($transactionId);
    $form['submit'] = submit(t('Submit answers'));
  } elseif ($us->step() != 'Kba') {
    return r\go('status'); // let it fall through, for appropriate message on status page
  } else say(t('Confirmation questions are not available. Please try again later.') . " ($err)");

  return $form;
}

function formKba_submit($form, &$sta) {
  $us = new r\usd($mya = r\acct());
  foreach ($sta['input'] as $key => $value) {
    if (substr($key, 0, 2) == 'q-' and is_numeric($qid = substr($key, 2))) $answers[] = u\ray('answerid, questionid', $value, $qid);
  }
///debug($answers); return;
  
  if (!$us->verifyKba($sta['input']['transactionId'], $answers, $err)) say($err, 'ERR');
  return r\go('status');
}

/**
 * Connect a bank account.
 */
function formBank($form, &$sta, $args = '') {
  $mya = r\acct();
  $typeOptions = array('checking', 'savings or money market');
  extract(u\just('bankAccount', $mya->secure));
  $usa = ($mya->country == US_COUNTRY_ID);
  if (!$usa) r\go('empty', t('only us banks'));

  $title = item(t('Bank Information'));
  
  if (@$bankAccount) {
    if ($usa) $bankAccount = substr(@$bankAccount, 4 + 9); // everything after the routing number
//      $routingNumber = substr(@$bankAccount, 4, 9); // chop of USkk
    $bankAccount = str_repeat('x', strlen($bankAccount) - 4) . substr($bankAccount, -4, 4);

    $us = new r\usd($mya);
    $source = $us->source($err);
    $verified = @$source['Verified'];
    $bankAccount = item($bankAccount . ($verified ? t(' (verified)') : ''), t('Account:'));

    if ($verified) { // account is all set (later this will want to be a form to change/add/remove)
    } else { // got an account, but not yet verified
      $subtext = item(tt('verify connection'));
      $deposit1 = textField(t('Deposit #1:'), '', suffix('&cent;'));
      $deposit2 = textField(t('Deposit #2:'), '', suffix('&cent;'));
      $verify = submit(t('Verify'));
    }
  } else { // no account yet, so get one
    $subtext = item(t('Complete this form to connect your rCredits account to your bank and the mainstream economy. Checking accounts work best because of legal restrictions on other types.'));
    if ($usa) $routingNumber = textField(t('Routing:'), t('What is the 9-digit routing number (type carefully)'), required());
    $bankAccount = textField(t('Account:'), t('What is the account number? (be sure to get this right)'), required());
    $accountType = formField('radios', t('Type:'), '', required($mya->can(B_SAVINGS)), $typeOptions);
    $submit = submit(t('Connect'));
  }

  return labeled(compact(u\ray('title subtext routingNumber bankAccount accountType deposit1 deposit2 submit verify remove')));
}

function formBank_validate($form, &$sta) {
  $values = u\trimAll($sta['input']);
  $op = op($sta);
  if ($op == 'submit') {
    extract(u\just('routingNumber bankAccount accountType', $values));
    if (!preg_match('/^(|[0-9]{9})$/', $routingNumber)) say('bad routing number', 'routingNumber');
    if (!preg_match('/^[0-9]{3,17}$/', $bankAccount)) say('bad account number', 'bankAccount'); // {3,20} ? Dwolla says 3-17
    if (!isset($accountType)) say('missing field', u\ray('field', 'Account Type'), 'accountType');
  } elseif ($op == 'verify') {
    extract(u\just('deposit1 deposit2', $values));
    for ($i = 1; $i <= 2; $i++) {
      $v = "deposit$i";
      $sta['input'][$v] = u\digits($$v);
      if ($$v < 1 or $$v >= DW_VERIFY_ROOF) say('bad deposit cents', $v);
    }
  }
}

function formBank_submit($form, &$sta) {
  $mya = r\acct();
  $us = new r\usd($mya);
  extract(u\just('routingNumber bankAccount accountType deposit1 deposit2', $sta['input']));
  $op = op($sta);
  if ($op == 'submit') {
    if (!$us->addBank($bankAccount, @$routingNumber, $accountType ? 'Savings' : 'Checking', $err)) return say($err, 'bankAccount');
    $mya->setBit('B_SAVINGS', (bool) $accountType, FALSE);
    if ($mya->country == US_COUNTRY_ID) $bankAccount = "USkk$routingNumber$bankAccount";
    $secure = compact(u\ray('bankAccount')) + $mya->secure;
    $mya->update(compact('secure'));
    say('info saved|verify connection|return to membership');
  } elseif ($op == 'remove') {
  } elseif ($op == 'verify') {
    if (!$source = $us->source($err)) return say($err, 'ERR');
    if (!$us->verifyBank($source['Id'], $deposit1/100, $deposit2/100, $err)) return say($err, 'deposit1');
    $mya->setBit(B_BANK);
    r\go('status', 'step completed');
  }
}

/**
 * Get the member's photo ID, as part of Dwolla's registration process.
 */
function formPhotoId($form, &$sta) {
  $mya = r\acct();
  $person = $mya->person();

  say('after emailing|return to membership');
  $mya->setBit(B_PHOTOID, TRUE); // allow the member to continue completing steps
  
  $proofType = $person ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t('EIN documentation from the IRS or proof of registration with state or local government');

  //  -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  
  $title = item(t('Photo ID Verification'));
  $subtext = item(tt('<p>We are having trouble confirming your information. We want to be absolutely sure that YOU are creating this account, not someone else using your name.</p><p>Please upload either a PDF or an image file of your @proofType.', compact('proofType')));
  $name = urlencode($mya->fullName);
  $subtext = item(tt('<p>We are having trouble confirming your information. We want to be absolutely sure that YOU are creating this account, not someone else using your name.</p><p>Please <a href="mailto:new@rCredits.org?subject=Photo ID from @name&body=Dear rCredits Team, My photo ID file is attached. - @name">email us (click here)</a> a PDF or image file of your @proofType.', compact('proofType', 'name')));
//  $idProof = formField('file', t('Photo ID:'), t('maximum') . tt(' @R_MAX_PHOTOID_SIZE MB'), attrib(u\ray('enctype', "multipart/form-data")));
//  $submit = submit('Upload');

  $form = compact(u\ray('title subtext idProof submit'));
  return $form;
  return labeled($form);
}

function formPhotoId_validate($form, &$sta) {
  if (@$_FILES['files']['name']['idProof']) {
    $validators = array('file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000));
    if (!$file = \file_save_upload('idProof', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime) and @$file->filemime != 'application/pdf') return say('bad file type', 'idProof');
    $sta['input']['filename'] = $filePath = file_directory_temp() . '/' . $file->filename;
    $mya = r\acct();
    if (FALSE and $mya->can(B_DW)) { // Dwolla API bug (skip it)
      $us = new r\usd($mya);
      if (!$us->sendPhotoId($filePath, $err)) return say($err, 'idProof');
    } else {
      $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof', TRUE);
      if (!$key = $sta['input']['idProof'] = bin2hex(u\encryptFile($filePath, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    }
  } return say('upload required', 'idProof');
}

function formPhotoId_submit($form, &$sta) {
  unlink($sta['input']['filename']);
  say('id verification pending');
  return r\go('status');
}

/**
 * Get the member's security info and proof of ID, open a Dwolla account.
 * @param string $args: params or report from Dwolla (via usd::callback()) on success of getting auth token
 */
function formSecurity($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();
/*  if (!$mya->admin()) {
    if (!$mya->gotContact()) {
      $a1 = "a href=\"$base_url/account/contact\"";
      $missing = item(u\tt('missing contact info', compact('a1')));
      return compact('missing');
    }
  } */
  //links to change password and PIN!!!
  extract(u\just('show', $args));
  /*
  if (@$connected == 'yes') {
    $link = "$base_url/status";
    $completed = r\acct()->ok()? '' : '|step completed';
    say("got token$completed|return to membership", compact('link'));
    r\tellStaff('connected to Dwolla');
  } elseif (@$connected) say("There was an error. Dwolla said: $connected", 'ERR');
  */
  $cttyAdmin = $mya->can(B_CTTY_ADMIN);
  $person = $mya->person();
  
  if ($mya->offsite) { // already got all this stuff
/*    if ($cttyAdmin and !@$mya->usdAccount) {
      say('There was a temporary technical problem. Please try again (click the Save button).');
       // !@$secure['auth']) { // no Dwolla account yet
//      exit($usd->getAuth()); // get (new) auth token (does not return until after a visit to Dwolla)
    } */
    $federalId = $dob = $pin = $answer = R_ON_FILE;
  }
  
/*
Dwolla says: 
We're having trouble confirming your information. We want to be absolutely sure that you are creating this account, not someone else creating an account under your name. Protecting your identity is our #1 concern.

Personal Accounts or Sole Proprietors:
Please upload a government-issued photo ID such as a U.S. driver's license or passport. 

Businesses or Non-Profits:
Please upload a copy of your EIN documentation or Non-Profit Status documentation. 

2.5MB max
*/
//  $has = $mya->hasAux('proof');
//  $proofType = $person ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t("EIN documentation from the IRS (or the equivalent) -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  
//    'subtext = item(t('Have you set up your <a href="' . "$base_url/help/dwolla-account-setup" . '">Dwolla</a> account? Great! Now connect it to your rCredits account. Enter your PIN and security information here. When you click "Save", you will be sent again to the Dwolla site, to sign in and "Allow" the needed permissions.'));
//    'subtext = item(t('Just like for a bank account, we need to verify your identity. The photo ID file you upload can be either an image or a PDF. Please be patient while the file is encrypted &mdash; it may take several seconds.'));

  $title = item(t('Security Information'));
  $federalId = adminField(t('Federal ID:'));
  $dob = $person ? adminField(t('Birthdate:')) : NULL;
  $usdAccount = adminField(t('Dwolla Account:'));
  $usdEmail = adminField(t('Dwolla Email:'));
  $usdPass = adminField(t('Dwolla Pass:'));
  $usdPin = adminField(t('Dwolla PIN:'));
  
  $pw = item(t('<@a>Change your password</a>', u\ray('@a', 'a href=/account/change-password')), ' ');
  $pin = item(t('<@a>Change your PIN</a>', u\ray('@a', 'a href=/account/change-pin')), ' ');
  $photo = item(t('<@a>Change your photo</a>', u\ray('@a', 'a href=/account/photo')), ' ');
  $question = $person ? textField(t('Question:'), t('Type a question that no one but you could guess the answer to.'), required($mya->question)) : NULL;
  $answer = $person ? textField(t('Answer:'), t('The answer to that question'), required(@$answer)) : NULL;
/* KEEP   'idProof = $has ? 
        formField('item', t('Photo ID:'), '', R_ON_FILE . ($mya->ok() ? '' : t(' (pending approval)')))
      : formField('file', t('Photo ID:'), tt('Upload an image of your @proofType. (maximum @R_MAX_PHOTOID_SIZE MB)', compact('proofType')), attrib(array('enctype = "multipart/form-data")));
      */
//    'posttext = item(t('We partner with Dwolla to handle the US Dollars you spend and receive. When you click "Save" here, you will be sent to the Dwolla site, to "Allow" the needed permissions.'));
//    $submit = submit($has ? t('Save') : t('Upload and Save'));  );
  $submit = submit();
  $form = compact(u\ray('title federalId dob usdAccount usdEmail usdPass usdPin pw pin photo question answer submit'));
  
  if ($cttyAdmin) {
    foreach (u\ray('pin answer') as $field) { // and idProof
      u\prefix(show($field, @$show), $form[$field]['#description']);
    }
//    if (!$mya->admin()) unset($form['subtext']);
  }

//  if (!$person) unset($form['idProof']); // for now, do not require company proof
  
  return labeled($form);
}

function formSecurity_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pin federalId dob', $sta['input']));
/*  if (!onFile('usdAccount', $sta)) {
    $usdAccount = $sta['input']['usdAccount'] = u\digits($usdAccount);
    if (!preg_match('/^[0-9]{10}$/', $usdAccount)) return say('bad account number', 'usdAccount');
  } */
/*  if (!onFile('pin', $sta)) {
    $pin = $sta['input']['pin'] = u\digits($pin);
    if (!preg_match('/^[0-9]{4}$/', $pin)) return say('bad pin', 'pin');
  } */
/*  if (!onFile('dob', $sta)) { // 12/14/1901 is the earliest
    if (!$dob = $sta['input']['dob'] = u\s2t($dob) or $dob > REQUEST_TIME) return say('bad date', 'dob');
  }
  if (!onFile('federalId', $sta)) {
    $federalId = $sta['input']['federalId'] = u\digits($federalId);
    $what = r\acct()->person() ? 'social security number' : 'federal ID';
    if (!preg_match('/^[0-9]{9}$/', $federalId)) return say('bad federal id', compact('what'), 'federalId');
  } */
  
/*  $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof', TRUE);

  if (@$_FILES['files']['name']['idProof']) {
    $validators = array('file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000));
    if (!$file = \file_save_upload('idProof', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime) and @$file->filemime != 'application/pdf') return say('bad file type', 'idProof');
    $tempfile = file_directory_temp() . '/' . $file->filename;
    if (!$key = $sta['input']['idProof'] = bin2hex(u\encryptFile($tempfile, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    \file_delete($file);
  } elseif ($mya->hasAux('proof')) {
    $sta['input']['idProof'] = R_ON_FILE; // flag for no change
  } // not required! elseif ($mya->person()) return say('upload required', 'idProof'); */
}

function formSecurity_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('federalId dob pin question answer idProof', $sta['input']);

  foreach ($info as $key => $value) if ($value == R_ON_FILE or $value == @$form[$key]['#default_value']) unset($info[$key]); // don't change if on file or unchanged
  extract($info); // extract just the ones that changed

  $info = u\just(R_SECRET_FIELDS . ' question', $info);
  if (!empty($info) and !$mya->update($info)) return say('info not saved', 'ERR');

  say('info saved');
}

define('R_REL_EMPLOYER_COL', 2); // number of amount/employer column
define('R_REL_OWNER_COL', 4); // number of family/owner column

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function formRelations($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $co = $mya->co();
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers = be\accountName($myid);
  $header = u\ray('Other account,Draw,My employer?,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person draw employer employee owner permission link'));
  if ($co) {
    $header[R_REL_EMPLOYER_COL] = ''; // was '$/mo';
    if (!$mya->person()) $header[R_REL_OWNER_COL] = 'Owner?';
    if (!$mya->ok()) $mya->update(u\ray('data', u\ray('relationsOk', TRUE) + $mya->data));
  }
  
  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, s.amount, IF(:myid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.employerOk, s.employeeOk, s.isOwner, s.draw,
      s2.reid AS reid2, s2.employerOk AS employerOk2, s2.employeeOk AS employeeOk2
    FROM ((r_relations s RIGHT JOIN users u ON u.uid = s.main)
      RIGHT JOIN users v ON v.uid = s.other)
      LEFT JOIN r_relations s2 ON (s2.other=s.main AND s2.main=s.other)
    WHERE s.main=:myid OR (s.other=:myid AND s2.reid IS NULL)
    ORDER BY fullName, IF(:myid=s.main, s.other, s.main), (:myid=s.other)
EOF;
  $result = db\q($sql, compact('myid'));
  $rows = $raws0 = $raws = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original ("0") raw values from table
    if ($other == $myid) list ($main, $other, $amount, $permission, $employerOk, $employeeOk, $isOwner, $draw, $reid2, $employerOk2, $employeeOk2)
      = array($other, $main, 0, 0, 0, 0, 0, 0, $reid, $employerOk, $employeeOk);
    
    $otherCo = r\acct($other)->co();
    $otherPerson = r\acct($other)->person();

    $idLink = R_NONE; // default to no link
    if (!$otherCo) {
      if ($employeeOk and $permission >= r\perm(B_BUY)) {
        $idLink = "<br><a href=\"$base_url/request-employee-rcard/$other\">request&nbsp;rCard</a>";
      }
    }

    $ii = count($rows) . "-$reid"; // "row-reid"
    $drawn = toggle('draw', $draw, $ii);
    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employerOk2);
    $employee = !$otherCo ? toggle('employerOk', $employerOk, $ii, $employerOk xor $employeeOk) : R_NONE;
    $Owner = toggle('isOwner', $isOwner, $ii);
    $perm = $otherPerson ? dropdown('permission', $permission, $GLOBALS['share permissions'], $ii) : R_NONE;
    $amountValue = ''; // was amountField('amount', number_format($amount,0), $ii);

    $thisRow = array($fullName, $drawn, !$co ? $employer : $amountValue, $employee, $Owner, $perm, $idLink);
    $employeeOk = @$employeeOk2;
    $thisRaw = compact(u\ray((!$co ? 'employeeOk' : 'amount') . ' draw employerOk isOwner permission'));
//      if ($co) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    
    $rows[] = $thisRow; // what to show on screen
    $raws[] = $thisRaw; // original conceptual ("raw") values for what's show on screen
  }
  $sta['original_settings'] = compact('raws0', 'raws');
  
  $aExact = "a href=\"$base_url/help/payment-exchanges\"";
  $aInvite = "a href=\"$base_url/invite\"";
  $aAgent = "a href=\"$base_url/agent\"";
  $help = tt('Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. If they don\'t have an account yet, <@aInvite>invite them</a>!', compact(u\ray('aExact aInvite aAgent')));
  //If they wish NOT to open a real account, create an <@aAgent>rPOS Sign-in Account</a> for them.

  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$acctIdentifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no relations');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => item(t('Relations')),
    'subtext' => item($subtext),
    'newPerson' => textField(t('Add Someone:'), t("name, id, email, or phone"), autocomplete('relation')),
    'list' => formField('item', '', '', array('markup' => $list)),
//    'new_permission' => formField('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'go' => submit(t('Save Changes')),
  );

  return $form;
}

function formRelations_validate($form, &$sta) {
if (confirming_v($sta)) return;
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\just('newPerson', $sta['input']));
  
  if (@$newPerson) {
    $acct = whois($newPerson, 'newPerson', $sta['input'], 'no self-relation');
    if (($id = @$acct->id) and r\relation(1, $myid, $id)) return say('already related', 'newPerson');
    $sta['input']['other'] = $id; // save for submission handling
  }
  $managers = 0; // for company accounts there needs to be at least one manager
  foreach ($sta['input'] as $key => $value) {
    if (!strpos($key, '-')) continue;
    list ($keya, $rawnum, $reid) = explode('-', $key);
    if ($keya == 'amount') {
      $amount = $value;
      if ($err = u\badAmount($amount)) return say($err, compact('amount'), 'amount');
      if (($sta['input'][$key] = round($amount, 0)) != $amount) say('amount rounded', compact('amount'));
    } elseif ($keya == 'permission') {
      if ($value == r\perm(B_MANAGE)) $managers++;
    }
  }
  if (!$mya->person() and !$managers and !$mya->admin()) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
  if (confirming_s($sta)) return;

  $mya = r\acct();

  if ($other = @$sta['input']['other']) { // adding a new relation
    $main = $mya->id;
    $permission = 0; // default to no permissions
    $info = compact(u\ray('main other permission'));
    db\insert('r_relations', $info);
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }

  $changes = array();
  extract(u\just('raws0 raws', $sta['original_settings']));
///  debug($sta['original_settings']);
  foreach ($sta['input'] as $key => $value) { // ... be\updateRelations will have to check permissions and data carefully
    if (!strpos($key, '-')) continue;
    list ($key, $rawnum, $reid) = explode('-', $key); // eg split permission-2-37
    if (!in_array($key, array_keys($raws))) return 'bad field in relations'; // hack attempt?

    $value = 0 + strtr($value, array('true' => 1, 'false' => 0));
/// if ($key == 'employeeOk') debug($value);
/// debug(compact('value','rawnum','key'));
/// debug(0 + $raws[$rawnum][$key]);
    if ($value != 0 + $raws[$rawnum][$key]) { // changed?
      $changes[$reid][$key] = $value;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
///  debug($changes);
  if (!empty($changes)) say(be\updateRelations($changes));
  if (!$mya->ok()) return r\go('status', 'step completed');
  if (t\est()) return r\go('account/relations');
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
      'code' => textField(t('Code:')),
      'new' => hidField($new),
      'verify' => submit(t('Verify')),
    );
    return $form;
  }

  $headers = array('#', 'Type', 'Device name', 'Last used');
  $types = u\ray(TX_CHANNELS);

  $sql = "SELECT channel, boxnum, code, boxName, access FROM r_boxes WHERE uid=:myid ORDER BY boxnum";
  $result = db\q($sql, compact('myid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $type = $types[$channel += 0]; // make sure channel is int, not string
    $boxName = 
        @$boxName ? htmlspecialchars($boxName)
      : ($channel == TX_SMART ? (t('POS Device #') . $boxnum)
      : ($channel == TX_WEB ? (t('Browser #') . $boxnum)
      : ($channel == TX_SMS ? u\fmtPhone($code)
      : 'ERROR')));
//    $list["x$boxid"] = array($boxid, $boxName, $restricted ? 'Yes' : 'No', u\fmtDate($access));
    $list["x$boxnum"] = array($boxnum, $type, $boxName, u\fmtDate($access));
  }

  unset($sta['input']['new']); // don't show number just added
  $form = array(
    'title' => item(t('Devices ("boxes")')),
    'subtitle' => item(t('These are the devices used with this account.')),
    'boxes' => formField('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no devices')), $list),
    'new' => textField(t('Add a cell phone*: ')),
    'save' => submit(t('Save changes')),
//    'delete' => submit(t('Remove Selected')),
//    'postscript' => item(t('* You may have a cell phone listed on the <a href="account/contact">Contact Information</a> page, or even as a POS device on this page, but to use it for text message transactions you must add its phone number here.')),
    'postscript' => item(t('* A device may appear on this list more than once if you use it in different ways (for texting, as a POS device, and/or as a web browser).')),
  );
  return $form;
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
    $sta['redirect'] = 'account/boxes';
  }
    
  if ($op == 'delete') {
// needs work    foreach ($boxes as $key => $one) if ($one[1]) say(be\deleteCell(substr($one[1], 1))); // ignore the leading 'x'
  }
}

/**
 * Completely redo the login form (see user_login() in user.module).
 * @see also loginFollowup, below (called from rweb_user_login()).
 */
function formLogin(&$form, &$sta) {
  global $base_url;
  $args = basename(t\est() ? t\SERVER('REQUEST_URI') : $_SERVER['REQUEST_URI']);
  extract(u\just('err', $args));
  if (@$err) say($err, 'ERR');

  unset($form['actions']);
  unset($form['pass']);

  $title = item(t('Welcome to rCredits!'));
  $signup = isDEV ? "<a href=\"$base_url/signup/code=w\">Signup</a>" : '';
  $subtext = item(t('<p>Not yet a member? <@a>Check it out</a>!</p>', u\ray('@a', 'a href=http://rCredits.org')) . $signup . t('<p><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right site (<b><em>new.rcredits.org/</em>something</b>). And be sure to sign OUT when you\'re done!</p>'));
  if (($name = @$_COOKIE['loginto']) and ($uid = r\loginString($name))) {
    //$process = array('form_process_password_confirm', 'user_form_process_password_confirm'); // 'rCredits\\Web\\password_confirm0');
    $newPass = formField('password_confirm', '', '', compact('process')); // not required!!
    if (!isset(r\acct($uid)->secure['pin'])) $pin = formField('password', t('PIN:'), t('Choose a 4-character security code (to verify your identity quickly, when appropriate).')); // first time logging in, get PIN
  } else $name = @$sta['input']['name']; // this fails (not sure why)
  
  $passDesc = @$newPass ? t('Your current or one-time password. ') : '';
  $passDesc .= t('(<@aForgot>Forgot it?</a>)', u\ray('@aForgot', 'a tabindex=3 href=account/password/' . $name)); // uses $name, so must precede $name = textField
  $name = textField(t('Username/ID:'), t('Your account ID, email, or username (the letters of your legal name, all lowercase)'), required(@$name));
  $pass = formField('password', @$newPass ? t('Tmp password:') : t('Password:'), $passDesc, required());
  $strong = hidField(0, attrib(array('id' => 'edit-strong')));

  $submit = submit(t('Sign in'));
  $form = compact(u\ray('title name pass newPass pin strong submit subtext')) + $form;

  $form['#validate'] = array('rCredits\\Web\\login_extra_validate', 'user_login_final_validate'); // add replacement function
  
  
  $form['external_input'] = hidField(serialize(@$sta['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $form['#attributes']['class'][] = 'rweb labeled';
  focusOn('name');
  js('inc/radiocheck.js', 'file', 'footer'); // this normally gets called in showForm()
  t\snapShot($form);
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account id, with password.
 * Sets $sta['uid'] to signify successful authentication.
 */
function login_extra_validate($form, &$sta) {
  extract(u\just('name pass newPass pin strong', u\trimAll($sta['input'])));
  if (!$name or empty($pass)) return FALSE;

  $ipLimit = variable_get('user_failed_login_ip_limit', 50);
  $ipWindow = variable_get('user_failed_login_ip_window', HOUR_SECS);
  if (!flood_is_allowed('failed_login_attempt_ip', $ipLimit, $ipWindow)) {
    $sta['flood_control_triggered'] = 'ip';
    return FALSE;
  }
  if (!$uid = r\loginString($name)) return FALSE; // not a user

//  if ($name == 'abeone' and @$newPass['pass1'] == 123) {
//    $sta['flood_control_user_identifier'] = $sta['uid'] = $uid; // for testing
//  } else {
  $uidOnly = variable_get('user_failed_login_identifier_uid_only', FALSE);
  $userLimit = variable_get('user_failed_login_user_limit', 5);
  $userWindow = variable_get('user_failed_login_user_window', 21600);
  
  $identifier = $uidOnly ? $uid : $uid . '-' . ip_address();
  $sta['flood_control_user_identifier'] = $identifier;

  if(0) if (!t\est() and !flood_is_allowed('failed_login_attempt_user', $userLimit, $userWindow, $identifier)) {
    $sta['flood_control_triggered'] = 'user';
    return FALSE;
  }

  $mya = r\acct($uid);
  if (!isset($mya->secure['pin']) and @$pin === '') return say('pin required', 'pin');
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  if (@$newPass['pass1'] and !$strong and !(isDEV and @$newPass['pass1'] == '123')) return say('weak pass', 'pass');
  $sta['uid'] = (passwordOkay($mya, $pass) ? $uid : FALSE);
//  }
  if ($sta['uid']) flood_clear_event('failed_login_attempt_user', $sta['flood_control_user_identifier']);
}

function loginFollowup($edit, $account, $goto = 'summary') {
  extract(u\just('newPass pin name', $edit['values'])); // values not input
  $resetPass = svar('reset_pass'); // svar gets unset by unset($_SESSION[$key]) just below
  if (!$pass = $resetPass ?: @$newPass) unset($pass);
  
  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($edit['external_input'])); // save params from external online merchant

  $mya = r\acct($myid = $account->uid);
  svar('myid', $myid);
  svar('scanned_in', FALSE);
  setupGlobals();

  r\setCook('loginto', '', REQUEST_TIME-1); // no more default login
  $data = $mya->data ?: array();
  unset($data['oneTimePass']);
  if (@$pin) $secure = compact('pin') + ($mya->secure ?: array());
  $mya->setBank();
  $mya->update(compact(u\ray('pass data secure')));

  u\loga('login', compact('name'), $myid);
  return r\go($goto);
}

function formPass($form, $sta, $id = '') {
  $id = urldecode(basename($_SERVER['REQUEST_URI']));
  $title = item(t('Request New Password'));
  $name = textField(t('Account:'), t('Username, Email, or Account ID'), required(u\neq($id, 'password', '')));
  $submit = submit(t('Request'));
  /*$form['name']['#title'] = t('Account:');
  $form['name']['#description'] = t('Username, Email, or Account ID');
  $form['name']['#default_value'] = u\neq($id, 'password', '');*/
//  $form['#validate'] = array('rCredits\\Web\\formPass_validate');
//  $form['#submit'] = array('rCredits\\Web\\formPass_submit');
//  $form['#attributes']['class'] = array('rweb', 'labeled');
  focusOn('name');
//  t\snapShot($form);
  $form = compact(u\ray('title name submit'));
  return labeled($form);
}

function formPass_validate($form, &$sta) {
  extract(u\just('name', $sta['input']));
  if (!@$name) return say('missing field', array('field' => 'account'), 'name');
  if (!@$sta['input']['uid'] = r\loginString($name)) return say('bad login', 'name');
}

function formPass_submit($form, &$sta) {
  extract(u\just('uid', $sta['input']));
  u\EXPECT($uid, 'missing uid in pass_submit');
  $a = r\acct($uid);
  list ($name, $data) = array($a->name, $a->data);
  list ($region, $quid, $pass) = array(strtolower(R_SERVER_ID), $a->mainQid, oneTimePass($name, $data));
  $a->update(compact('data'));
  r\notify($uid, 'password-reset', compact(u\ray('region name quid pass')), TRUE);
  say('password reset');
  r\tellStaff('password reset', compact(u\ray('region name quid uid')), $uid);
  return r\go('');
  //$sta['redirect'] = 'user';
}

function formDirectory($form, &$sta) {
  $myid = r\acct()->id;
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: substr(r\userField('postalCode', $myid), 0, 3);
  
  $list = ($region !== FALSE or $which) ? directoryList($which, $region) : '';
  
  $form = array(
    'title' => item(t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => item(t('for a country name or postal code...')),
    'which' => textField(t('Search for:'), t('Type part of company name or industry category'), dft($which) + autocomplete('industry')),
    'region' => textField(t('Where:'), t('Type the first few characters of your postal code<br>Try 013, to see the list for Greenfield, MA.'), dft($region)), // or country name
    'submit' => submit(t('Find')),
    'item' => formField('item', '', '', $list),
  );

  return labeled($form);
}

function formDirectory_validate($form, &$sta) {
}

function formDirectory_submit($form, &$sta) {
  extract($sta['input']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

function formCompany($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;

  $sql = <<<EOF
    SELECT iid, IF(parent=iid, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY parent, IF(parent=iid,0,iid)
EOF;
  $cats = db\q($sql)->fetchAllKeyed();
  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$mya->name" target="_blank">Show my public rCredits web page</a> (in a new window)
EOF;
	$bits = u\ray(CO_CAN_BIT_DESCS);
	$coCanDfts = array();
	foreach ($bits as $bitName) {
		if (substr($bitName, 0, 1) == 'u') continue; // undefined
		$bit = u\consta('co', $bitName);
		$cans[$bit] = $bitName;
		if ($mya->coCan($bit)) $coCanDfts[] = $bitName;
	}

  $form = array(
    'title' => item(t('Company Information')),
    'links' => item($links),
    'name' => formField('item', t('Company name:'), '', $mya->fullName),
    'photo' => item(t('<@a>Change your company photo</a>', u\ray('@a', 'a href=photo')), t('Photo')),
    'categories' => formField('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($myCats), $cats),
    'selling' => formField('textarea', t('Selling:'), t('Enter one or more transaction descriptions for when you make a sale &mdash; one line for each. Put the most common one first.'), required($mya->selling)),
    'can2' => boxesField('can', t('Manager can:'), t('What can company agents do at the POS, when they have "buy" permission?'), $coCanDfts, $cans),
//    'shareRpos' => boolField(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), dft($mya->coCan(CO_SHARE_RPOS) ?: 0)),
    'requireCashier' => boolField(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), dft($mya->coCan(CO_REQUIRE_CASHIER) ?: 0)),
    'website' => textField(t('Website(s):'), t('Especially if you want to accept payments on your website, be sure to list it here.'), dft($mya->website)),
    'descPreface' => item(t('<br>What does this company do? The description you enter below will appear on your rCredits web page.')),
    'description' => formField('text_format', t('Description:'), '', $filtered_html + dft($mya->description)),
    'submit' => submit(),
  );

  return labeled($form);
}

function formCompany_validate($form, &$sta) {
  extract(u\just('selling', $sta['input']));
  $selling = explode("\n", $selling);
  foreach ($selling as $one) {
    $one = trim(strtolower($one));
    if (!in_array($one, array('', 'refund', 'cash in', 'cash out'))) $new[] = $one;
  }
  if (!@$new) return say('missing field', u\ray('field', 'selling'), 'selling');
  $sta['input']['selling'] = join("\n", $new);
}

function formCompany_submit($form, &$sta) {
  $mya = r\acct();
  $id = $mya->id;
  extract(u\just('description website selling categories can requireCashier', u\trimAll($sta['input'])));
//  $description = \check_markup($description['value']);
  $description = @$description['value'];
  $website = str_ireplace('http://', '', @$website);

  $mya->setBits(CO_CAN_BIT_DESCS, $can, 'co');
  $mya->setCoBit(CO_REQUIRE_CASHIER, @$requireCashier);
  $mya->update(compact(u\ray('description website selling')));
  
  $DBTX = db_transaction();
  db\q('DELETE FROM r_user_industries WHERE uid=:id', compact('id')); // out with the old
  if (@$categories) foreach ($categories as $iid) {
    db\q("INSERT INTO r_user_industries (uid, iid) VALUES (:id, :iid)", compact('id', 'iid')); // in with the new
  }
  unset($DBTX); // commit
  return $mya->ok() ? say('options saved') : r\go('status', 'options saved|step completed');
}

/**
 * Show a page when someone scans an account ID QR, for example with QRDroid.
 * Cases:
 *   Scanning in (not signed in, scanned agent)
 *   Signing in (not signed in, scanned person) - show login page with account id filled in
 *   Curious about someone's card (not signed in, scanned person) also give a link to the promo site
 *   Agent charging a customer (signed in, scanned any) - show photo and Charge button
 *   Agent paying someone (signed in, scanned any) - show Pay button
 *   Person paying someone (signed in, scanned any) - go to Pay page
 *   Agent looking at a business (signed in, scanned company) - show Site button (to company's rCredits page)
 *   Person looking at a business (signed in, scanned company) - show Site button on the Pay page (NYI)
 * @param string $who: the part of a scanned rCard's QR code after the domain and '/I/':
 *   the account's qid tail, then "." (pro se) or "-" (agent), then an rCard security code:
 *   must match the cardCode (for pro se) or cardCode2 (for agent) stored in the account's secure field.
 */
function formI($form, &$sta, $who = '') {
  $parts = preg_split('/(' . R_MARKS . ')/', $who, 0, PREG_SPLIT_DELIM_CAPTURE);
  $count = count($parts);

  if ($count == 5) { // old fashioned format: ABC.DEF-whatever or ABC-DEF-whatever (agent)
    list ($region, $mark, $tail, $zot, $code) = $parts;
  } elseif ($count == 3) { // new format: DEF.whatever or DEF-whatever (agent)
    list ($tail, $mark, $code) = $parts;
    $region = R_SERVER_ID;
  } else return say('bad member id', 'ERR'); // not an ID QR
  
  if ($mark == R_AGENT_URL_MARK) {
    list ($mark, $field) = array(R_AGENT_MARK, 'cardCode2');
  } else $field = 'cardCode';
  
  $qid = "$region$mark$tail";
  if (!$a = r\acct($qid)) return say('bad card', 'ERR'); // not a real code -- just go to home page
  $co = $a->co();
  
  $mya = r\acct();
  if (!$gotCode = @$a->agent->secure[$field]) return say('bad card code', 'ERR');
  if ($gotCode != $code) return say('bad card code', 'ERR');
  list ($otherId, $otherAgent) = array($a->id, $a->agentId);

  //  if (!$mya) return $a->proSe() ? r\go('user/login?name=' . $a->name) : scanIn($a);
  return r\go($a->proSe() ? 'http://rCredits.org' : "member/$a->name");
  
  /*
  $shortName = $a->name;
  $txArgs = "/who=$shortName&scanned=1";
  if ($mya->proSe() and !$co) return r\go("pay$txArgs");
  
  list ($myid, $myAgent) = array($mya->id, $mya->agentId);
  if ($otherId == @$myid) return r\go('', 'no self-trading');

  if ($co) $site = goButton(t('Show Webpage'), $goSite = "member/$shortName", t("See this company's rCredits webpage"));
  if ($mya->can(B_SCAN)) $charge = goButton(t('Charge'), "charge$txArgs", t('Charge someone'));
  if ($mya->can(B_BUY)) $pay = goButton(t('Pay'), "pay$txArgs", t('Pay someone'));
//    if (r\acct($myid, $otherAgent)->can()) $changeAgent = goButton(t('Change Agent'), "user/logout/who=$shortName", t('Use this button when your shift begins'));
//  if (r\acct($otherId, $myAgent)->can()) $changeAccount = goButton(t('Change Acct'), "change-account/who=" . $a->mainQid, t('Manage a different account'));
  $theId = hidField($otherId);
  $theAgent = hidField($otherAgent);

  if (!@$charge) return $co ? r\go($goSite) : r\go('empty', 'no can scan');

  $zot = compact(u\ray('charge pay site'));
  $all = identifiers($a) . render($zot);
  svar('lastCustomer', $a->id);
  $all = formField('item', '', '', $all);
  return compact('all'); */
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

function formOptions($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $isSetUp = !is_null($mya->minimum); // user has already saved some options
  $title = item(t('Account Preferences'));
//  $virtualMaxMessage = $mya->co() ? t(' If you choose to enable payment exchanges, this setting is moot (irrelevant and unused).') : '';
  $subtext = item(t('(The defaults here are appropriate for most people.)'));
  $minimum = textField(t('Minimum:'), tt('prompt min'), required(u\fmtAmt($isSetUp ? $mya->minimum : R_MINR_DEFAULT, 's')));
  //$maxDft = $isSetUp ? $mya->maximum : R_MAXR_DEFAULT;
  //$maxDft = $maxDft < 0 ? '' : u\fmtAmt($maxDft, 's');
//  $maximum = textField(t('Maximum:'), tt('prompt max') . $virtualMaxMessage, dft($maxDft));
//  $toBank = formField('checkbox', t('To bank:'), t('(recommended) Automatically transfer the excess to your bank account. [<a href="../help/to-bank">why ever not?</a>]'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));
  $dft = $mya->can(B_NOTICE_WEEKLY) ? 1 : ($mya->can(B_NOTICE_MONTHLY) ? 2 : 0);
  $notices = formField('radios', t('Email notices:'), t('How often do you want to receive an email digest of rCredits messages?'), required($dft), $noticeRadios);
  $smsNotices = boxField(t('SMS notices:'), tt('Send me a text message when I receive a payment, charge, or invoice (requires a <a href="' . $base_url . '/account/boxes">connected cell phone</a>).'), $mya->can(B_NOTIFY_SMS));
  $statements = $mya->can(B_ELECTRONIC) + 0;
  $statementRadios = array(
    t('I will accept electronic statements.'), 
    t('I choose to receive printed statements by mail.') . u\SUBS(t('<div>I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>'))
  );
  $statements = formField('radios', t('Statements:'), '', required($statements), $statementRadios);

  /*
  if ($mya->co()) {
    $a1 = "a href=\"$base_url/help/payment-exchanges\"";
    $a3 = "a href=\"$base_url/account/relations\"";
    // , minus 25 cents for each payment larger than $@DW_FEE_THRESHOLD. For example, a $100 payment exchange gives you a net rebate of $5.00 - 0.25 = $4.75. 
    $desc = tt(<<<EOF
    Saying "<b>Yes</b>" to <@a1>payment exchanges</a> (<b>highly recommended</b>) means you trade your rCredits to your employees and vendors automatically every day, in exchange for US Dollars (your total account balance does not change). You get a 5% bonus for these exchanges. To make the payment exchanges automatic, you need to set up <@a1>average</a> monthly payment amounts on the <@a3>Relations</a> page. As a result, US Dollars (rather than rCredits) will accumulate in your account until you spend them or move them to your bank account.
EOF
    , compact('a1', 'a3'));
    //$dft = $isSetUp ? $mya->can(B_PAYEX) : TRUE;
    //$payVirtual = boolField(t('Pay virtually:'), $desc, $dft);
  } */
  $secretBal = boxField(t('Secret Balance:'), t("Don't let qualified merchants tell me my balance, when I ask."), $mya->can(B_SECRET_BAL));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $floor = u\fmtAmt($mya->floor);
  $debtOk = boxField(t('Debt Okay:'), tt('Allow my account balance to go negative, limited by my current "credit floor" (which starts at zero, then gets lower and lower as time goes by &mdash; currently @floor). I understand there will be a substantial fee (currently $@R_DEBT_FEE plus @R_DEBT_PERCENT% per month) if I fail to bring my balance back up to zero within @R_DEBT_DAYS days.', compact('floor')), $mya->can(B_DEBT_OK));
  $submit = submit(t('Update Preferences'));
  
  $form = compact(u\ray('title subtext minimum smsNotices notices statements secretBal debtOk submit'));
  js('showMax();', 'inline', 'footer');
  return labeled($form);
}

function formOptions_validate($form, &$sta) {
  $mya = r\acct();
  $floor = u\fmtAmt($mya->floor);
  if ($err = amtErr('minimum', $sta)) return say($err, 'minimum');
  extract(u\just('minimum payVirtual', $sta['input'])); // after amtErr checks
  if ($minimum < $mya->floor) return say('min sub floor', compact('floor'), 'minimum');
//  if ($minimum > 0 and $minimum < R_BANK_MIN) return say('min too small', compact('floor'), 'minimum');
}

function formOptions_submit($form, $sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('minimum smsNotices notices statements payVirtual debtOk secretBal', $sta['input']));

//  $mya->setBit(B_TO_BANK, @$toBank, FALSE); 
//  $mya->setBit(B_PAYEX, @$payVirtual, FALSE);
//  $mya->setBit(B_NOTIFY_EMAIL, $notifyBy & 1, FALSE);
  $mya->setBit(B_NOTIFY_SMS, @$smsNotices, FALSE);
  $mya->setBit(B_NOTICE_WEEKLY, $notices == 1, FALSE);
  $mya->setBit(B_NOTICE_MONTHLY, $notices == 2, FALSE);
  $mya->setBit(B_ELECTRONIC, @$statements, FALSE);
  $mya->setBit(B_DEBT_OK, @$debtOk, FALSE);
  $mya->setBit(B_SECRET_BAL, @$secretBal, FALSE);
  
//  if ($payVirtual) $maximum = $minimum;
  $mya->update(compact(u\ray('minimum'))); // flag bits get saved automatically
  return $mya->ok() ? say('options saved') : r\go('status', 'options saved|step completed');
}

function formReports($form, &$sta, $args = '') {
  if (!$mya = r\acct()) return NULL;
  $admin = $mya->admin();
  if (!$ctty = $mya->community) $ctty = r\serverUid();

  list ($in, $out, $fees, $net, $personal, $companies) = array(t('in'), t('out'), t('fees'), t('net'), t('personal'), t('companies'));
  extract(r\stats($ctty));
  extract(u\just('r usd refunds shared pAccts bAccts', r\stats($ctty, strtotime('-1 month', REQUEST_TIME))), EXTR_PREFIX_ALL, 'lm');
  list ($r0, $usd0) = array($r, $usd);
  $refundsPerMo = $refunds - @$lm_refunds;

  list ($bankNet, $txRewards, $total) = array(abs($bankInPerMo - $bankOutPerMo - $refundsPerMo), $rebates + $bonuss + $refunds - $fines, $r + $usd);
  $bankSign = ($bankInPerMo >= $bankOutPerMo + $refundsPerMo) ? '+' : '-';
  $savings = u\fmtAmt($total - $perMo);
  $savingsPct = ($usd0 + 0) ? number_format(100 * ($total - $perMo) / $usd0, 1) . '%' : t('n/a');
  $velocity = number_format(100 * $perMo / $total, 1) . t('% per month');
  foreach (u\ray('bankInPerMo bankOutPerMo refundsPerMo bankNet r usd total signups inflations txRewards grants loans fines') as $one) $$one = u\fmtAmt($$one);
  $p2pPerMo = $perMo - $p2bPerMo - $b2bPerMo - $b2pPerMo;
  $p2pTxsPerMo = $txsPerMo - $p2bTxsPerMo - $b2bTxsPerMo - $b2pTxsPerMo;
  $issuedAs = tt("signup: @signups@R, inflation adjustments: @inflations@R, rebates/bonuses: @txRewards@R, grants: @grants@R, loans: @loans@R, fees: -@fines@R", compact(u\ray('signups inflations txRewards grants loans fines')));
//  $issuedAs = str_replace('$', '', $issuedAs);
  $accounts = fundRpt($pAccts + $bAccts, @$lm_pAccts + @$lm_bAccts);
  $accounts = str_replace(' &mdash;', " ($pAccts $personal, $bAccts $companies) &mdash;", $accounts);
  $membership = ($mya->admin() or $mya->id == \variable_get('cgf_uid')) ? ' <a href="member-list">member list</a>' : '';
  
  $title = item(t('Community Statistics'));
  $subheader = item(t('for ') . r\acct($ctty)->fullName . $membership);
  $subtext = item(t('This page is designed to give us the information we need to keep an eye on our economy and manage it effectively. Hover over any label, for a definition. <em>Key:</em> <b>mo</b>=month <b>acct</b>=account <b>/</b>=per'));
  $accounts = item($accounts, popHelp(t('Accounts:'), t('The number of accounts approved to buy and sell through the rCredits system, as of today.')));
  $issued = item(fundRpt($r0, @$lm_r, '$r'), popHelp(t('rCredits issued:'), t('Total number of rCredits in circulation, as of today (the breakdown shows how they were issued).')), $issuedAs);
  $shared = $admin ? item(fundRpt($shared, @$lm_shared, '$'), t('Shared w CGF:')) : NULL;
  $demand = item(fundRpt($usd0, @$lm_usd, '$'), popHelp(t('Demand:'), t('Total amount of US Dollars that people have put into the system, available for the community to exchange for rCredits and possibly spend.')), t('community demand for additional rCredits (<@a>important</a>)', u\ray('@a', 'a href=help/demand')));
  $total = item(tt("$r@R + $usd@US = $total"), popHelp(t('Total funds:'), t('Total amount of money (rCredits and US Dollars) in the system, as of today')), t('including about @savings in savings = @savingsPct of demand (<@a>important</a>)', u\ray('@savings @savingsPct @a', $savings, $savingsPct, 'a href=help/savings')));
  $banking = item(tt("$bankInPerMo@US ($in) - $bankOutPerMo@US ($out) - $refundsPerMo ($fees) = $bankSign$bankNet@US ($net)"), popHelp(t('Banking / mo:'), t('How much money did members transfer in and out of the system, since this time last month')), t('transferring funds to or from a bank account (<@a>important</a>)', u\ray('@a', 'a href=help/to-bank')));
  $txs = item(txsRpt($perMo, $txsPerMo, $pAccts + $bAccts, ''), t('Purchases / mo:'));
  $p2p = item(txsRpt($p2pPerMo, $p2pTxsPerMo, $bAccts, 'p'), popHelp(t('p2p:'), t('Person-to-person transactions (individual members buying from other individual members)')));
  $p2b = item(txsRpt($p2bPerMo, $p2bTxsPerMo, $pAccts, 'p'), popHelp(t('p2b:'), t('Person-to-business transactions (customers buying from businesses, mostly using their rCard)')));
  $b2b = item(txsRpt($b2bPerMo, $b2bTxsPerMo, $bAccts, 'b'), popHelp(t('b2b:'), t('Business-to-business transactions (businesses paying their suppliers)')));
  $b2p = item(txsRpt($b2pPerMo, $b2pTxsPerMo, $bAccts, 'b'), popHelp(t('b2p:'), t('Business-to-person transactions (payroll)')));
//  $exchanges = item(txsRpt($xchPerMo + $cashPerMo, $xchTxsPerMo + $cashTxsPerMo, $pAccts + $bAccts, ''), t('Trades / mo:'), t('exchanges of rCredits for US Dollars (<@a>important</a>)', u\ray('@a', 'a href=help/trades')));
  $velocity = item($velocity, popHelp(t('Velocity:'), t('How fast the money is circulating.')), t('how much of the total funds in the system (rCredits plus US Dollars) turns over every month'));
  
  $form = labeled(compact(u\ray('title subheader subtext accounts issued shared demand total banking txs p2p p2b b2b b2p exchanges velocity')));
  if (strpos($args, 'variety=') === FALSE) $args .= '&variety=stats';
  if ($mya->can(B_CTTY_ADMIN)) $form += formTxs('', $sta, $args);
  return $form;
}

function formReports_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formReports_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'community');
}

/**
 * Accept an invitation by supplying the invitee email address.
 */
function formAccept($form, &$sta) {
  $title = item(t('Accept Your Invitation'));
  $email = textField(t('Your email:'), t('Type the email address to which the invitation was sent.'));
  $submit = submit(t('Continue'));
  return labeled(compact(u\ray('title email submit')));
}

function formAccept_validate($form, &$sta) {
  extract(u\just('email', $sta['input']));
  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
}

function formAccept_submit($form, &$sta) {
  extract(u\just('email', $sta['input']));
  if (!$code = db\lookup('code', 'r_invites', 'email=:email ORDER BY (invitee IS NULL) DESC, invited DESC', compact('email'))) return say('bad invite', 'email');
  return r\go("signup/code=$code");
}

/**
 * Show a list of members in the member's community
 */
function formMemberList($form, &$sta) {
  $mya = r\acct();
  if (!$mya->admin() and $mya->id != \variable_get('cgf_uid')) return hack('member list');
  
  $title = item(R_REGION_NAME . t(' Member List'));
  $sql = 'SELECT uid, fullName, notes, phone, mail AS email, postalAddr, :IS_COMPANY AS co FROM users WHERE uid>1 AND :IS_OK ORDER BY fullName';
  if ($mya->admin()) $sql = str_replace('AND :IS_OK', '', $sql); // include everybody, for admin
  $q = db\q($sql);
  $list = "<table id='memberlist'>\n";
  $emails = '';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $postalAddr = str_replace(', Massachusetts', ', MA', $postalAddr);
    $phone = str_replace('+1 ', '', u\fmtPhone($phone));
    $nameClass = $co ? 'name co' : 'name';
    $list .= <<<EOF
<tr>
<td class="$nameClass">$fullName</td>
<td>&lt;$email&gt;</td>
<td>$phone</td>
<td>$postalAddr</td>
</tr>
EOF;
    if ($mya->admin()) {
      $us = new r\usd($a = r\acct($uid));
      formMembership($a); // make sure membership bits are up to date
      $bits = array(); // zap previous value
      foreach (u\ray('member ok dw bank') as $one) if ($a->can(u\consta('b', $one))) $bits[] = $one;
      $bits = $bits ? ('<span style="color:darkgreen;">' . join(' ', $bits) . "</span><br>\n") : '';
      if (!$a->ok() and $a->can(B_DW)) {
        if (($step = $us->step($a))) $step = "<span style=\"color:orange;\">$step</span> ";
      } else $step = '';
      $notes = str_replace("\n", "<br>\n", $notes);
      $notes = str_replace('do:', '<b style="color:blue; font-size:130%;">do:</b>', $notes);
      if ($step or $bits or $notes) $list .= "<tr><td colspan=4 style='font-size:70%; font-weight:normal; padding-left:20px;'> $step$bits$notes</td></tr>\n";
    }
    $emails .= "$email, ";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  $emails = $mya->admin() ? item($emails, t('All emails')) : NULL;
  return compact(u\ray('title list emails'));
}

/**
 * Request an employee ID card (for buying on behalf of the company).
 */
function formRequestRCard($form, &$sta, $other = '') {
  $mya = r\acct();
  if (!$acct = r\acct($mya->id, @$other)) return hack('request rCard');
  if (!$mya->co() or !$mya->can(B_MANAGE)) return say('no account permission');
  $fullName = $acct->agent->fullName;
  
  $form = array(
    'title' => item(t('Request Employee rCard')),
    'subtext' => item(tt('A Company Agent rCard will be US Mailed to @fullName, to be used for purchases on behalf of the company. Your company account will be charged $@R_CARD_COST.', compact('fullName'))),
    'other' => hidField($other),
    'request' => submit(t('Request Card')),
    'cancel' => submit(t('Cancel')),
  );
  return labeled($form);
}

function formRequestRCard_submit($form, &$sta) {
  global $base_url;
  extract(u\just('other', $sta['input']));
  if (op($sta) == 'request') {
    $mya = r\acct();
    $reid = r\relation('reid', $main = $mya->id, @$other);
    $quid = r\quid($main, $other);
    $name = $mya->agent->fullName;
    $address = r\location($mya, TRUE);
    $href = "$base_url/print-rcard/$quid";
    r\tellStaff('company rcard', compact(u\ray('main other reid quid href name address')));
    say('card ordered', compact('name'));
  } else say('no card ordered');
  return r\go('account/relations');
}

/**
 * Resubmit the phone number to Dwolla, to get a verification code.
 * Called directly from /account/readd-phone, as well as from setupUs()
 */
function addPhone() {
  $us = new r\usd($mya = r\acct());
  if (!$phoneId = $us->addPhone(substr($mya->phone, 2), @$mya->data['verifyBy'], $err)) {
    return r\go('account/contact', $err ?: t('Unknown error'), 'phone');
  }
  $mya->upSecure('usdPhone', $mya->phone);

  svar('phoneId', $phoneId);
  return r\go('account/verify-phone');
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
    if ($acct->agent->co()) exit('You may print scan-in cards only for employees.');
  } elseif (!$acct = r\acct(strtoupper($id))) exit('No such account.');
  
  $fullName = $acct->fullName;
  $regionName = r\acct($acct->community)->fullName;
  $qid = $acct->qid();
  
  if (!$proSe = $acct->proSe()) {
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

function formFooter() {
  global $base_url, $rUrl;
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $mya = r\acct();
  $loggedInLinks = ($mya and ($mya->can(B_MANAGE) or $mya->admin())) ? <<<EOF
    <a href="$base_url/contribute">Contribute</a> |
    <a href="$base_url/invite">Invite Someone</a> |
    <a href="$base_url/agreement">The Agreement</a> |
EOF
  : (basename($_SERVER['REQUEST_URI']) == 'user' ? '' : <<<EOF
    <a href="$base_url/user">Sign in</a> |
EOF
  );

  $markup =<<<EOF
<hr>
$loggedInLinks
<a href="http://rCredits.org">About rCredits</a> |
<a href="$base_url/help">Help</a>
<br><br>
website copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance&trade;</a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:info@rCredits.org" target="_blank">info@rCredits.org</a><br>
<div id="saywait"><br><img src="$rUrl/images/saywait.gif" /><br>processing...</div>
EOF;

  $footerText = item($markup); // unique name for this page

  $js = basename($_SERVER['REQUEST_URI']) == 'handy' ? '' : <<<EOF
jQuery('a').click(function() { // #messages a, .links>li>a, .tabs>li>a, #edit-footertext>a
  jQuery('#saywait').show();
});
EOF;
  
  $js .= <<<EOF
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
})();
EOF;

  js($js, 'inline', 'footer');
  return compact('footerText');
}
