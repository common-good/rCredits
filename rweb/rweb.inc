<?php
namespace rCredits\Web;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits as r;
use rCredits\Testing as t;

include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent
require_once __DIR__ . '/../rcredits-backend.inc';
require_once __DIR__ . '/../rcredits-testing.inc'; // else fails in showForm()

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

define('R_NONE', '<span class="none">--</span>'); // let the tests see null values
define('R_ON_FILE', t('(on file)'));
define('R_AGREE_0', -2); // number of the first point in the rCredits Agreement
define('R_AGREE_9', 6); // number of the last point in the rCredits Agreement

$GLOBALS[TX_WEB . ' messages'] = array(
//  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"

  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
//  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account ID is @quid. Your starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in. Pretend to buy and sell just as you would in a typical month. We need that information, to make sure everyone has plenty of places to receive and spend their rCredits.</p><p>Experiment! Have fun!'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! We sent you an email with your user ID and password. Come back here to log in and finish your account setup. Then if you have another account to create, use the "Create Another Account" button on the Summary page.</p>'),
  'company is ready' => t(<<<EOF
    <p></p><p>To finish setting up this company account:</p>
    <ol>
      <li>Log in to your personal account</li>
      <li>Choose this company account from the account-selection dropdown box in the upper right corner of the screen.</li>
    </ol>
EOF
  ),
  'info saved' => t('Your information has been saved.'),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  'options saved' => t('Your options have been saved'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('Your new password has been emailed to you.'),
  'changed agent' => t('You successfully changed the agent to @agentName.'),
  'changed account' => t('You successfully changed the account to @accountName.'),
  'nothing undone' => t('Nothing undone.'),
  'signed agreement' => t('You signed the rCredits Agreement on @date.'),
  'downloading' => t('Your download will begin shortly...'),
  'changed account' => t('You are now managing the account for @newAcct. Any unsaved changes to account @oldAcct have been discarded.'),
  'take a step' => t('Your account is not yet activated. <a href="membership">Take the next step! Click here</a> to see where you are in your membership progress and what to do next.'),
  'invite sent' => t('Your invitation has been sent to @email.'),
  'got token' => t('Your rCredits and US Dollar accounts have been successfully connected!'),

  // confirm
  'yes or no' =>  '@yesNo', // to replace with yesNo(url)
  'confirm undo' => t('Undo @created transfer of @amount @tofrom @otherName?'),

  // errors
  'missing purpose' => t('<p>For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc.". Rebates and bonuses are intended as rewards for productive economic activity in rCredits.</p>
    <p>For everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no cells' => t('This account has no related cell phones.'),
  'required field' => t('@field must not be blank.'),
  'duplicate email' => t('"@duplicateAccount" is already using that email address. If you want to use the same email address for this account, add a tag to the local part, for example "@emailTagged". '), // used in user.module
//  'duplicate email' => t('That email address is already taken by @duplicateAccount.'),
  'forgot password' => t('If you just forgot the password to the other account, <a href="@passwordLink">click here</a> to request a new password.'), // used in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad login' => t('That account does not exist. Check your typing?'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad account id' => t('That is not a proper account ID.'),
  'bad account number' => t('Your Dwolla account number must be exactly 10 digits'),
  'bad pin' => t('Your PIN must be exactly 4 digits'),
  'bad federal id' => t('Your @what must be exactly 9 digits'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'short to' => t('You are @short short for that transaction. <a href="account/preferences">Increase your minimum balance</a>!'),
  'no such company' => t('There is no such company.'),
  'op canceled' => t('Operation Canceled.'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm delete cell' => t('Are you sure you want to remove the selected cell phones from this account?'),
  'already related' => t('That person is already related to this account. If you want to change the settings, find this person in the list and change the settings on that row.'),
  'already cell' => t('That cell phone (@number) is already connected to this account.'),
  'cell taken' => t('A cell phone can be connected to only one account. You are already using that cell phone (@number) in connection with account "@accountName". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'bad routing number' => t('US bank routing numbers are 9 digits. Use the number on the lower left edge of your checks.'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'unknown member' => t('"@who" has not yet signed up for rCredits. <a href="@draftLink">Click here to add them</a> to the list or <a href="" id="which-cancel">click here to cancel</a>.'),
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click on the one you want or, if you don\'t see them listed, <a href="@draftLink">Click here to add them</a> to the list or <a id="which-cancel">here to cancel</a>.'),
  'similar found' => t('If you mean @otherName (@phone), <a href="@draftLink">click here for YES</a>. Otherwise, <a href="@cancelLink">click here to cancel</a>.'),
  'get app' => t('To scan Account ID cards with a smartphone, you need the app! You can download the Android app <a href="http://rCredits.org/rc.apk">here</a>. The iphone version is in the works (check again in January).'),
  'no help' => t('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at @R_ORG_PHONE. Or send your question by <a href="mailto:@R_ORG_EMAIL?subject=help with rCredits Members Site, @page page" target="_blank">email</a>.'),
  'no permission' => t('You do not have permission to use that account.'),
  'bad transient id' => t('That QR code is out of date. Try again?'),
  'bad email' => t('The e-mail address <em class="placeholder">@email</em> is not valid.'), // must match Drupal, for tests
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner or an employee of the company? (please choose one or both)'),
  'shortr to' => t('You do not have enough rCredits, for that transaction. <a href="@how">Go here</a> to get more rCredits.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("@fullName").'),
  'bad invite' => t('To sign up for rCredits, you must be invited and you must use the invitation link within @R_INVITE_DAYS days. If you already have an rCredits account and want to open an additional account for yourself, your business, or a family member, sign in to your personal account and use the "Open Another Account" on the Summary page.'),
  'bad minr' => t('Your chosen rCredits minimum cannot be less than your assigned minimum balance (@floor).'),
  'bad maxr' => t('Your chosen rCredits maximum cannot be less than your rCredits minimum. Nor can it be less than your total amount of incentive rewards to date.'),
//  'bad maxusd' => t('Your chosen USD maximum cannot be less than your rCredits minimum.'),
  'info not saved' => t('We apologize. Your information could not be saved at this time. Please try again in an hour or so.'),
  'no permission' => t('You do not have permission to visit this page.'),
  'specific bad amount' => t('The amount (@amount) must be a number.'),
  'insufficient relation' => t('You cannot open an account unless you have a closer relationship to it. Use the <a href="membership/invite">invitation page</a> instead.'),
//  'humans sign' => t('You must be logged in to a personal account, to sign the agreement on behalf of a company.'),
  'bad spoof check' => t('You must confirm (<i>every time</i>) that you are on the real rCredits site.'),
  'no auth yet' => t('You have not yet authorized a connection with your Dwolla account. Please resubmit.'),
  'not an image' => t('That is not an acceptable image format.'),
  'bad commercial aspect' => t('Your company profile picture must be in landscape orientation, with a width-to-height ratio of about @R_COMMERCIAL_ASPECT.'),
  'bad personal aspect' => t('Your profile picture must be in portrait orientation, with a width-to-height ratio of about @R_PERSONAL_ASPECT.'),
  'need a manager' => t('A company account must have at least one person to manage it.'),
);

/**
 * Return yes and no buttons.
 * Do NOT use button() here (it kills the site)
 */
function yesNo($yesURL, $noURL = '', $yesNext = 'empty', $noNext = 'empty') {
  svar('yesNext', $yesNext);
  svar('noNext', $noNext);
  return <<<EOF
<br>
<input type="button" name="op" onclick="document.location.href='$yesURL'" value="Yes" class="form-local form-submit">
&nbsp;
<input type="button" name="op" onclick="document.location.href='$noURL'" value="No" class="form-local form-submit">
EOF;
}

function button($value, $href = '', $title = '', $type = '') {
  $title = htmlspecialchars($title);
  $button_type = 'local form-submit' . ($type ? " $type" : ''); // Drupal prefixes this with 'form-'
  $markup = array($value => formField('button', '', '', compact('value', 'button_type') + attrib(compact('title'))));
  $markup = render($markup);
  $markup = str_replace('"submit"', '"button"', $markup); // compensate for Drupal pigheadedness
  if ($href) {
    if (strpos($href, 'http://') !== FALSE or substr($href, 0, 1) == '/') $href = "document.location.href='$href';";
    $name = 'name="op"';
    $markup = str_replace($name, ($button_type == 'local' ? '' : "$name ") . "onclick=\"$href\"", $markup);
//  if ($href) $markup = "<a href=\"$href\" title=\"$title\">$markup</a>";
  }
  return $markup;
}

function formField($type, $title = '', $description = '', $other = array(), $options = NULL) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  if ($title == 'Sign up for rCredits') debug($other);
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = key($options); // Drupal's select fails without a default
//  if (isset($other['value']) and !isset($other['default_value'])) $other['default_value'] = $other['value'];
  $field = u\prefixKeys('#', array_merge(compact(u\ray('type title description options')), $other));
  return $field;
}

function submit($title = '') {return formField('submit', '', '', $title ?: t('Submit'));}
function item($value, $title = '', $desc = '') {return formField('item', $title, $desc, $value);}
function hidField($value) {return formField('hidden', '', '', $value);}
function textField($title = '', $description = '', $other = array(), $options = NULL) {
  return formField('textfield', $title, $description, $other, $options);
}
function boolField($title = '', $description = '', $dft = NULL) {
  $class = array('yesno');
  return formField('radios', $title, $description, required($dft) + attrib(compact('class')), array(t('No'), t('Yes')));
}

function test($arg) {
  global $user;
//  cache_set('t_resume', FALSE); // cleanup after potential previous failures
//  cache_set('t_messages', FALSE);
  include __DIR__ . '/../../gherkin/test.php';
  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate();
  \drupal_goto('handy');
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'reset') {
    r\reset();
/*    $uid = variable_get('rcredits_communityUid') + 1;
    $uid2 = $uid + 1; $uid3 = $uid2 + 1;
    $pass = '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae';
    $sql = <<<EOF
      INSERT INTO users (uid, name, pass, mail, fullName, status, postalCode) VALUES
      ($uid, 'uten', '$pass', 'uten@ex.com', 'U Ten', 1, '01301'),
      ($uid2, 'btwo', '$pass', 'btwo@ex.com', 'B Two', 1, '01301'),
      ($uid3, 'cornerstore', '$pass', 'cstore@ex.com', 'Corner Store', 1, '01301');
      
      INSERT INTO r_sms (number, uid, status) VALUES
      (10, $uid, 2),
      (2, $uid2, 2);
EOF;
    r\dbQ($sql);
    be\fund($uid, TX_SIGNUP);
    be\fund($uid2, TX_SIGNUP);
*/
  }
  
  if ($arg == 'clear_sessions') r\dbQ('TRUNCATE table SESSIONS');
  
  \drupal_set_message("Doing rCredits Util '$arg'...");
  \drupal_goto('handy');
}

function handy_links() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
    'rcredits/test?menu=1|Test Menu|',
//    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
    'sms|Simulate an SMS transaction|Simulate SMS',
//    'admin/config/development/testing|Test|Test',
//    'deletetests.php|Delete old tests|Delete old tests',
//    'rcredits/util/reset|Reset rCredits|',
    'rcredits/util/clear_sessions|Clear Sessions|',
    
    'devel/settings|Helper functions, pages, and blocks to assist Drupal developers. The devel blocks can be managed via the block administration page.|Devel settings',
    'devel/cache/clear?destination=handy|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache',
    'devel/entity/info|View entity information across the whole site.|Entity info',
    'devel/php|Execute some PHP code|Execute PHP Code',
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

function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formOutput; // for testing
  try {
    setupGlobals(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
//    $layout = @$_GET['layout'] ?: svar('layout');
//    if ($function == 'I') $layout = 'Mobile';
//    $newLayout = ($layout != svar('layout'));
    
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
    if ($function != 'accounts' and $function != 'summary') u\log("show $function", compact(u\ray($args)));
//    if ($layout == 'Mobile') css('mobile.css', 100);
    $formName = substr($function, 0, 1) == '\\' ? substr($function, 1) : "rCredits\\Web\\form$function";
//    if ($newLayout) svar('layout', $layout); // do this before drupal_get_form, so forms can know the current layout
    $form = \drupal_get_form($formName, $arg1, $arg2, $arg3, $arg4);
    $sta = array(); \rweb_form_alter($form, $sta, $formName);
    $form['#attributes']['class'][] = 'rweb';
//    if ($newLayout) $form['#cache'] = FALSE;
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any


    if ($function != 'help') if (($i = array_search('title', array_keys($form))) !== FALSE or $function == 'Txs') {
      if ($function == 'Txs') $i = -1; // no title on Transactions page, so insert at beginning
      $i = -1; // always at beginning
      $extras = array('#id' => 'help-link', '#weight' => @$form['title']['#weight']);
      $help_link = item(t('<a><div></div>Help with this page</a>')) + $extras; // add a help link at top of page
      $form = array_slice($form, 0, $i+1) + compact('help_link') + array_slice($form, $i+1);
      js('inc/misc.js', 'file', 'footer');
    }

    $form2 = $form; $rent = render($form2);
    if ($function == 'Usd') $rent = str_replace('Dwolla', '<a href="http://refer.dwolla.com/a/clk/3KkS2w" target="_blank">Dwolla</a>', $rent);
    t\testOutput($formOutput = t\strip($rent), 'screen');
    if (strpos($rent, 'form-type-checkbox') or strpos($rent, 'form-type-radio')) js('inc/radiocheck.js', 'file', 'footer');
    
    if ($colgroup = @$form['#colgroup']) {
      unset($form['#colgroup']);
      $markup = '';
      foreach ($colgroup as $col) {
        $guts = '';
        foreach ($col as $attrib => $value) $guts .= " $attrib=\"$value\"";
        $markup .= "<col $guts>\n";
      }
      $markup = "<colgroup>\n$markup</colgroup>\n";
      return str_replace('<thead>', $markup . '<thead>', $rent); // shore up Drupal lack of colgroups in tableselect
    } else return $rent;
  } catch(u\fit $e) {r\Web\exception_handler($e);}
}

function setupGlobals($agent = '') {
  global $user, $channel;
  if (@$channel) return; // already setup
  if (!$agent) $agent = @$user->uid; // register_submit supplies agent (no one else does)
  $cuid = svar('cuid') ?: $agent;
  r\acct::setDefault(r\acct($cuid, $agent)); // set default for r\acct()
  $channel = TX_WEB;
}

/**
 * Set or retrieve an rCredits session variable.
 */
function svar($name) {
  $name = 'rcredits_' . $name;
  $args = func_get_args();
  if (count($args) == 1) return @$_SESSION[$name]; else $_SESSION[$name] = $args[1];
}

/**
 * Offer choices for user to choose from
 * 1. If called with all arguments, the arguments are stored in svar('pop') and the form is reloaded
 * 2. If called as which('info'), this function returns $info from svar('pop')
 * 3. If called with NO arguments (from showForm()), this function returns a fieldset to insert as a popup layover div into
 * the form awaiting the choice. Clicking on a choice fills in the missing value and submits the form.
 * For #3, we expect svar('pop') to contain: $formName, $choices, $resultField, and $question
 *   $formName: the form name
 *   $choices: an array of choices
 * @param array $choices: associative array of things to choose from
 * @param string $resultField: name of field that gets the choice
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $question: the question to ask
 */
function which($choices = '', $resultField = '', $info = '', $question = 'Which one?') {
  $formName = current_path();
  
  if (is_array($choices)) { // called with all args (#1)
    svar('pop', compact(u\ray('formName choices resultField info question')));
    r\go($formName); // reload page
  }

  if (!is_array($pop = svar('pop'))) return array();
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($formName != $pop_formName) return array();
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return array();
  
  svar('pop', NULL); // called with no args (#3)
  $onchange = "this.form.elements['$pop_resultField'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $subtext = item($pop_question);
  $choice = formField('select', '', '', attrib(compact('onchange') + array('size' => 12)), $pop_choices);
  $close = popupCloser('which'); // this is needed in mobile layout
  $popup = formField('fieldset', t('Which One?'), '', array('id' => 'which')) + compact(u\ray('subtext close choice'));
  return compact('popup');
}

function keep_values(&$form) {
  if (!$info = which('info')) return $form;
  foreach ($form as $key => $value) {
    if (isset($info[$key])) {
      $form[$key]['#default_value'] = $info[$key];
    } elseif (substr($key, 0, 1) != '#') foreach ($form[$key] as $key2 => $value2) { // fieldset
      if (isset($info[$key2])) $form[$key][$key2]['#default_value'] = $info[$key2];
    }
  }
  return $form; // for convenience we both modify the form and return it
}

/**
 * Modal window closer
 *//*
function close_form() {
  $js = 'window.opener = bottom; window.close();';
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  return '';
}
*/

/**
 * Generate choices for a member autocomplete field
 * @param string $string: what the user has typed so far
 * @param string $role: 'payer', 'payee', 'industry', 'company', or '' (other), depending on the role of the user
 * @param int $cuid: the current account (globals and session variables are inappropriate here)
 * @todo: ditch this altogether or make it efficient with separate rCredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the autocomplete URL to the backend? privacy is not crucial here
 * @patched: Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($role = '', $cuid = 0, $string = '') {
//  return drupal_json_output(array('zot' => "role=$role cuid=$cuid string=$string"));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\shortName($string, '%') . '%'; // this also prevents malicious database injection
//  $string = str_replace(' ', '%', \db_like(" $string "));
  if (!is_numeric($cuid)) return;
  $params = 'string'; // list of substitution parameters for query (each section below can add additional params)
  
  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
/*  attempt to show recently used members first (add cuid to dbQ query subs)
   AND (t.$role IS NULL OR t.$role=:cuid) ORDER BY t.created DESC, u.fullName
    $selection = <<<EOF
      u.uid, u.fullName AS display, u.mail AS email 
      FROM users u LEFT JOIN r_txs t ON t.$other=u.uid 
      WHERE (u.name LIKE :string OR u.mail LIKE :string)
      AND (t.$role IS NULL OR :cuid IN (t.payer, t.payee))
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
      industry AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName AS display FROM users WHERE ((flags&(1<<B_COMPANY)) AND name LIKE :string)
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE (flags&(1<<B_COMPANY)) AND name LIKE :string)';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $cacct = r\acct($cuid);
    $cttyId = $cacct->community;
    $personal = $role == 'ctty' ? '' : ' AND (flags&(1<<B_PERSONAL))';
    $selection = "uid, fullName AS display FROM users WHERE (name LIKE :string AND community=:cttyId$personal)";
    $params .= ' cttyId';
  } else $selection = 'uid, fullName AS display FROM users WHERE (name LIKE :string)';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role cuid=$cuid string=$string sql=$selection"));
//return drupal_json_output(array('zot' => "subs=" . print_r(compact(u\ray('string cuid')), 1)));

  $result = r\dbQ($sql, compact(u\ray($params)));
  $matches = array();
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = substr(r\quid($uid), 3) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
//    return drupal_json_output(array('zot' => "display=$display"));
  }
  drupal_json_output($matches);
}

define('R_CONTRIB_SHOWHIDE', "var other = jQuery('.form-item-amount'); if(jQuery('#edit-gift').val() == 0) {other.show();} else other.hide();");

function formContribute($form, &$sta) {
  $cacct = r\acct();
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
  $oftens = array(0 => 'when?', 'M' => 'Monthly', 'Q' => 'Quarterly', 1 => 'Now (maybe again later)');
  $honors = array('honor' => 'in honor of', 'memory' => 'in memory of', 'other' => '(other)');
  $onchange = str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_CONTRIB_SHOWHIDE);
  $giftAttrib = attrib(compact('onchange'));
   
  $form = array(
    'title' => item(t('Membership Contribution to Common Good Finance')),
    'subtext' => item(t('<p>Make a contribution of any size, to help support the rCredits system. Buy a BRICK or some other piece of the Common Good Economy, as a tax-deductible contribution to Common Good Finance. We especially love monthly giving commitments.') . ($cacct->can(B_OK) ? '' : t(' The amount you choose will come from your combined rCredits/US Dollar account after your account is activated.</p>')) . t('</p><p>Thank you for your support!</p>')),
    'gift' => formField('select', t('Contribution:'), '', required($cacct->can(B_COMPANY) ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT) + $giftAttrib, $giftLevels),
    'amount' => textField(t('Other amount $'), ''),
    'often' => formField('select', t('When:'), '', required(), $oftens),
    'honor' => formField('select', t('Honoring:'), '', '', $honors),
    'honored' => textField(),
    'share' => textField(t('Share:'), t('What percentage of your ongoing rebates and bonuses would you like to contribute back to Common Good Finance, to support the rCredits system?'), required('50%')),
    'submit' => submit(),
  );

  js(R_CONTRIB_SHOWHIDE, 'inline', 'footer');
  return labeled($form);
}

function formContribute_validate($form, &$sta) {
  js(R_CONTRIB_SHOWHIDE, 'inline', 'footer');
  extract(u\just('gift amount share', $sta['values']));
  extract(u\just('often', $sta['input']));
  if (!$amount = $gift ?: $amount) return say('missing field', array('field' => 'amount'), 'amount');
  if (!$often) return say('missing field', array('field' => 'when'), 'often');
  if (!$share = str_replace('%', '', $share)) return say('missing field', array('field' => 'share'), 'share');
  if (u\badAmount($amount, '>0')) return say('nonpositive amount', 'amount');
  if (u\badAmount($share)) return say('nonpositive amount', 'share');
  $sta['values'] = compact(u\ray('amount share')) + $sta['values'];
}

function formContribute_submit($form, &$sta) {
  $cacct = r\acct();
  $info = u\just('amount often honor honored share', $sta['values']) + u\just('often', $sta['input']);
  $uid = $cacct->id;
  $created = REQUEST_TIME;
  r\dbInsert('r_gifts', $info + compact('uid', 'created'));
  say('gift successful');
}

/*
function contribute() {
  setupGlobals(); // required because not called from showForm
  if ($cacct = r\acct()) {
    $memberid = $cacct->mainQid;
    if ($cacct->can(B_COMPANY)) $organization = $cacct->fullName; else $yourname = $cacct->fullName;
    $phone = u\formatPhone($cacct->phone, '');
    $email = $cacct->mail;
    $country = $cacct->country;
    $postalCode = $cacct->postalCode;
    $state = $cacct->state;
    $city = $cacct->city;
    $address = $cacct->address;
    $donation = @$organization ? 250 : 50;
    $info = compact(u\ray('memberid yourname organization phone email country postalCode state city address donation'));
  } else $info = array();
  r\go('http://rCredits.org/donate?rparams=' . bin2hex(gzdeflate(http_build_query($info))));
}
*/

function formHelp($form, &$sta, $what = '') {
  require_once __DIR__ . '/rweb-help.inc';
  $title = item($what ? t('Help for ') . ucwords(str_replace('-', ' ', $what)) : t('General Help'));
  $text = formField('item', '', '', helpText($what ?: 'general'));
  return compact(u\ray('title text'));
}

function popupCloser($idHead) {
  global $base_url;
  return item("<img src='$base_url/sites/all/modules/rcredits/images/icons/close.png' border='0'>") + array('#id' => "$idHead-close");
}

/**
 * Return a help overlay
 * @todo: make it move right when "labeled"
 */
function help($page, $class = '') {
  require_once __DIR__ . '/rweb-help.inc';
//  $subtext = item($question);
  $close = popupCloser('help');
  $text = formField('item', '', '', helpText($page));
  $help = formField('fieldset', t('Help with this page'), '', array('id' => 'help')) + compact(u\ray('close subtext text'));
  return compact('help');
}

/**
 * Show the account's current membership status and steps to get to the next milestone.
 * @$updating bool: TRUE if just updating membership status bits.
 * @param string $code: w means show second screen even if not appropriate (for in-person demos)
 */
function formMembership($updating, &$sta = '', $code = '') {
  if (!$acct = r\acct()) return NULL;
  $cuid = $acct->id;  
  $nonperson = !$acct->can(B_PERSONAL);
//  $acct->setBit(B_MEMBER, 0); // for testing
//  $acct->setBit(B_OK, 0);
  if (!$acct->can(B_MEMBER)) {
    extract(u\just('gotPhoto signed donated nearly', r\nearlyMember($acct)));
    $gotProxies = $nonperson ?: (r\dbLookup('COUNT(*)', 'r_proxies', 'person=:cuid', compact('cuid')) >= 2);
    r\membershipEvent($acct, 'member', $nearly and $gotProxies);
  }

  $gotContact = ($acct->phone and $acct->mail and $acct->address and $acct->city and $acct->country);

  if (@$code == 'w' or ($acct->can(B_MEMBER) and !$acct->can(B_OK))) {
    $gotUsdInfo = (($secure = @$acct->secure) and @$secure['auth']);
    $gotIdProof = (file_exists(DRUPAL_ROOT . $acct->auxFilename('proof')) and @$acct->dob and @$acct->federalId);
    $gotPreferences = !is_null($acct->minimum);
    $email = $acct->mail;
    $invited = (r\dbLookup('COUNT(*)', 'r_invites', 'inviter=:cuid AND email<>:email', compact('cuid', 'email')) >= 1);
    r\membershipEvent($acct, 'eligible', $gotUsdInfo and $gotIdProof and $gotPreferences and $invited);
  }

  if ($updating) return $acct->can(B_OK);
  
  $message1 = $message2 = $message3 = '';
  
  if (!$acct->can(B_MEMBER) and !@$code) { // not a member yet (no rCard, no vote)
    $message1 = t('<h2>Congratulations on signing up for rCredits!</h2><p>Thanks for joining us. ') 
    . ($nonperson ? t('Before you can begin accepting rCredits payments, you must complete a few simple tasks (this is page one of two):</p>') 
      : t('Before we can send your rCard, and before you can vote on community funding, you must complete a few simple tasks:</p>'));

//    $step1 = t('Complete your <a>contact information</a>.'); // keep this in case
    $step1 = t('<a>Upload a photo</a> for your ') . ($nonperson ? t('company profile.') : t('Member ID Card.'));
    $step2 = t('Sign the <a>rCredits Agreement</a>.');
    $step3 = t('Make a <a>contribution</a> of <i>any size</i>, to support the rCredits system.');
    $step4 = t('<a>Choose two people</a> to represent your views whenever you are not there, to discuss and vote on community funding issues. They will need to sign up, before you can choose them (<a>invite them</a>!).'); // These should be people you know and trust and generally agree with. (need email or phone if not yet a member, to identify them when they do sign up)
//    .= memberStep(1, $gotContact, $step1, 'account/contact')
    $message2 = memberStep(1, $gotPhoto, $step1, 'account')
              . memberStep(2, $signed, $step2, 'membership/agreement')
              . memberStep(3, $donated, $step3, 'membership/contribute');
    if (!$nonperson) $message2 .= memberStep(4, $gotProxies, $step4, array('membership/proxies', 'membership/invite'));
              
  } elseif (!$gotContact) { // did all the initial steps, but somehow didn't complete contact info
    $message1 = t('<h2>Missing Contact Information</h2><p>Please complete your <a href="../account/contact">Contact Information</a>, ') . ($nonperson ? t('before continuing.</p>') : t('so we can mail your rCard.</p>'));
  } elseif (!$acct->can(B_OK)) { // got rCard, but not activated yet
    $message1 = $nonperson ? t('<h2>To activate your account, complete these final steps:</h2>') 
    : (t('<h2>You have completed all the rCredits membership requirements!</h2>')
      . t('<p>You will receive your rCard in the mail within a few days. To activate your account and begin using your rCard for purchases from participating businesses, there are a few final tasks:</p>'));
    $step1 = t('Open a <a>Dwolla</a> account and <a href="account/usd">connect it</a> to your rCredits account. In partnership with Veridian Federal Credit Union, Dwolla communicates with your US Dollar bank account. Whenever you buy something, your rCredits (if any) are used first and the remainder is paid in US Dollars &mdash; just like a typical debit card. Tip: You will probably want to set Dwolla transaction notifications "off".');
    $step2 = t('<a>Upload a photo</a> of your ')
      . ($nonperson ? t('Charter, Articles of Organization, or similar document.')
        : t('driver\'s license, birth certificate, passport, or other official ID.'))
      . ($gotIdProof ? t(' <span class="pending">(pending approval)</span>') : '');
    $step3 = t('Set your <a>Account Preferences</a>.');
    $step4 = t('<a>Invite someone</a> to sign up for rCredits! No one can join without an invitation. You can help by sending one. You might particularly want to invite companies and individuals you buy from or sell to (for example your employer or employees).');
    $message2 .= memberStep(1, $gotUsdInfo, $step1, 'http://refer.dwolla.com/a/clk/3KkS2w', TRUE)
              . memberStep(2, $gotIdProof, $step2, 'account/usd')
              . memberStep(3, $gotPreferences, $step3, 'account/preferences')
              . memberStep(4, $invited, $step4, 'membership/invite');
  } else {
    $message1 = t('<h2>Congratulations! Your account is Activated.</h2><p>You are authorized to use rCredits to buy and sell with other members.')
      . ($acct->can(B_PERSONAL) ? t(' You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>') : '</p>')
      . ($acct->can(B_BONA) ? '' : t("<p>To receive your signup bonus, you must get paid at least once in rCredits (or exchange some US Dollars for rCredits by <a href=\"$baseurl/account/preferences\">setting a minimum balance</a>) AND you must buy from at least three other members."))
      . (MINI_LAUNCH ? t(' During the "mini-launch", you only need to buy from ONE other member, to get your signup bonus.</p>') : '</p>');
  }
  $message = $message1 . "<table width=\"100%\">$message2</table>" . @$message3;
  
//  $title = item(t('Membership Status'));
//  $subtext = item(t('for ') . $acct->fullName);

  $steps = formField('item', '', '', $message);
  $form = compact(u\ray('title steps'));
  return labeled($form);
}

function memberStep($step, $done, $text, $links = '', $newWindow = FALSE) {
  u\EXPECT('done', 'bool');
  global $base_url;
  $target = $newWindow ? ' target="_blank"' : '';
  if ($links) {
    if (!is_array($links)) $links = array($links);
    foreach ($links as $link) {
      if (strpos($link, 'http://') === FALSE) $link = "$base_url/$link";
      $link = "<a href=\"$link\"$target>";
      $linkx = '</a>';
      $text = preg_replace('/<a>/', $link, $text, 1);
    }
  } else $link = $linkx = '';
  return <<<EOF
<tr>
  <td class="done state$done">$link<div>&nbsp;</div>$linkx</td>
  <td class="step step$step">$link<div>$step</div>$linkx</td>
  <td class="text"><div>$text</div></td>
</tr>
EOF;
}

/**
 * Give member a chance to sign the rCredits Agreement.
 */
function formAgreement($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
/* NEVER HAPPENS if ($cacct->proSe() and $cacct->acctType >= R_COMMERCIAL) {
    say('humans sign', 'signedBy');
    r\go('membership');
  } */
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
  $id = "edit-$name";
  $check = formField('checkbox', '', '', attrib(compact('id', 'name')));
  $model = \render($check);
  if ($date = $cacct->signed) {
    $instructions = '';
    $date = u\formatDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$cacct->signedBy</b>, $date");
  } else {
    $signedBy = textField(t('Signed:'), t('Type your full name here ') . "($cacct->fullName)");
    $submit = submit();
    $instructions = <<<EOF
<div id='instructions'><b>Instructions:</b> Put a check mark in each checkbox to indicate that you have read that section and agree to it. Really DO read the agreement and make sure you understand it! This agreement, that we make with each other, is the foundation for the rCredits system and the Common Good Economy.</div>
EOF;
  }

//  as long as its rCredits trade debt does not exceed its readiness to redeem that debt with official currency or the equivalent
//  as written in the ever-evolving Common Good Economy governance documents
// (including amending the Common Good Economy governance documents)
/*
Common Good Economy:
issue money:
Common Good Community: (links to explanation of both “CGC” and “in good standing”, including something like “as long as its rCredits trade debt does not exceed its readiness to redeem that debt with official currency or the equivalent”)
if the system ends:  (links to explanation of our plans for how to end gracefully, if necessary -- listing which participants you can still spend at for a while, etc.)
immediately pay back:
ethics:
Dispute Resolution Process: linked to the actual process description “as written in the Common Good Economy governance documents”
paying attention:
decision process:
other decisions:  including amending the Common Good Economy governance documents
*/

  $agreement = <<<EOF
$instructions
<div id="agreement">
<h2>A. I recognize that:</h2>
<ul>
  <li>[-2] rCredits are intended as the basis for the <a href="$base_url/help/common-good-economy">Common Good Economy</a> -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1] Together as a community we have the power to <a href="$base_url/help/common-good-economy">issue money</a> for our own use as a medium of exchange. It is our promise to accept rCredits that gives them value.</li>
  <li>[0] Governance by the people is essential, for a just society.</li>
</ul>

<h2>B. As a responsible participant in the Common Good Economy:</h2>
<ol>
  <li>[1] As long as I can spend my rCredits easily or exchange them easily for US Dollars, I will accept them without limit as payment for my goods and services. I will exchange rCredits for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will accept rCredits from members of any <a href="$base_url/help/common-good-community">Common Good Community</a> in good standing.</li>
  <li>[3] Whenever I exchange rCredits for US Dollars or vice versa, I will exchange them one for one, and I will price my goods and services the same in rCredits and in US Dollars. </li>
  <li>[4] I understand that I will receive some rCredits from the community as Incentive Rewards. If I stop participating (or <a href="$base_url/help/if-the-system-ends">if the system ends</a>) within 6 months of receiving my first rCredits or before the community has enough revenue to cover the Incentive Rewards, I will <a href="$base_url/help/immediately-pay-back">immediately pay back</a> whatever amount of Incentive Rewards I have received (while keeping any rCredits I received from other participants).</li>
  <li>[5] I promise to maintain a high level of honesty, integrity, and <a href="$base_url/help/ethics">ethics</a> in my dealings with other rCredits participants and with the community (including the rCredits system itself). When there is a dispute, I will follow the <a href="$base_url/help/dispute-resolution-process">Dispute Resolution Process</a> and will honor its outcome.</li>
  <li>[6] I agree to participate responsibly in managing the Common Good Economy to the best of my ability and as my life circumstances allow, including:
    <ol style="list-style-type:lower-alpha;">
      <li><a href="$base_url/help/paying-attention">paying attention</a> and understanding how the Common Good Economy works</li>
      <li>participating in the <a href="$base_url/help/decision-process">decision process</a> to set my community's funding priorities:
        <ol style="list-style-type:lower-roman;">
          <li>to foster healthy, fulfilling lives for all community members and</li>
          <li>to assist communities elsewhere</li>
        </ol></li>
      <li>participating in <a href="$base_url/help/other-decisions">other decisions</a> from time to time, as needed.</li>
    </ol></li>
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
  return labeled($form);
}

function formAgreement_validate($form, &$sta) {
  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) if (!@$_POST["check$i"]) return say('incomplete agreement', "check$i"); // $_POST not $sta
  $fullName = r\agent()->fullName;
  if (strcasecmp(trim($sta['values']['signedBy']), $fullName) != 0) return say('bad signature', compact('fullName'), 'signedBy');
}

function formAgreement_submit($form, &$sta) {
  $signed = REQUEST_TIME;
  extract(u\just('signedBy', $sta['values']));
  r\acct()->update(compact('signed', 'signedBy'));
}

/**
 * Choose default proxies.
 */
function formProxies($form, &$sta) {
  $cacct = r\acct();
  $id = $cacct->id;
  
  $title = item(t('Proxies'));
  $subtext = item(t(<<<EOF
<p><b>Direct democracy</b> is at the heart of the rCredits design. In each community, members meet to discuss and decide on funding priorities for grants, loans, and investments of rCredits for the greater good.</p>
<p>You assign another person as your ongoing Representative (Proxy). Whenever you don't vote, your Representative votes for you. The Representative's vote simply counts double (one vote for you and one vote for the Representative). If your Representative also fails to vote, then your Representative's Proxy votes for all three of you.<br><br>Everyone's voice is counted, whether or not they vote directly &mdash; one person one vote.</p>
<p>Our community's most trusted Representatives (who represent a lot of people) will meet in person to research and debate specific issues. These trusted Representatives do not decide policy issues directly. They oversee the public discussions, choose the wording for questions to be voted on, take action as directed by the members, and assure that the system is running smoothly.</p>
EOF
  ));

  $proxy1 = textField(t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly?'), required(r\definiteName(r\proxy($id, 1))) + autocomplete('cttyPerson'));
  $proxy2 = textField(t('Alternate:'), t('Who will vote on your behalf, if your Proxty doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'), required(r\definiteName(r\proxy($id, 2))) + autocomplete('cttyPerson'));
  $submit = submit();
  $form = compact(u\ray('title subtext proxy1 proxy2 submit'));
  return labeled($form);
}

function formProxies_validate($form, &$sta) {
  for ($i = 1; $i <= 2; $i++) {
    $proxyName = "proxy$i";
    $proxy = $sta['values'][$proxyName];
    if (@$proxy) if (!$proxy = whois($proxy, $proxyName, $sta['values'])) return FALSE;
    $sta['values'][$proxyName] = $proxy->id;
  }
}

function formProxies_submit($form, &$sta) {
  extract(u\just('proxy1 proxy2', $sta['values']));
  r\proxy(r\acct()->id, 1, $proxy1);
  r\proxy(r\acct()->id, 2, $proxy2);
  say('account updated');
}

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 */
function formInvite($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $title = item(t('Invite Someone to Sign Up for rCredits'));
  extract(u\just('nearly', r\nearlyMember($cacct)));
  $subtext = t('We\'re sorry, you cannot invite someone to sign up until you have completed your other membership steps.<br><br><a>Click here</a> to see where you stand.'); // don't require membership (conceptual loop)
  $subtext = item(strtr($subtext, array('<a>' => "<a href=\"$base_url/membership\">")));
//  $contact = textField(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());
  $email = textField(t('Email:'), t('An email address for the person you want to invite'), required());
  $trusted = boolField(t('Trusty?'), t('If you lent this person $250, would you trust them to pay it back?'));
  $subject = textField(t('Subject:'), t('The subject of your message'), required(t('Invitation to rCredits')));

  $default = "Dude,\n\nrCredits looks amazing. I signed up. Check it out!\n\n$cacct->fullName";
  $blurbLink = array('<a>' => "<a href=\"$base_url/sites/all/modules/rcredits/templates/invite.html\" target=\"_blank\">");
  $messageHelp = strtr(t('Invite them to sign up, and say why. A <a>blurb about rCredits</a> will follow your message, with a signup link.'), $blurbLink);
  $message = formField('textarea', t('Message:'), $messageHelp, required($default));
  $submit = submit();
  $form = $nearly ? 
      labeled(compact(u\ray('title email trusted subject message submit')))
    : compact(u\ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(u\just('email trusted', $sta['values']));
  if (!\valid_email_address($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  $cacct = r\acct();
  extract(u\just('email subject message', $sta['values']));
  $info = array(
    'uid' => $cacct->id,
    'noFrame' => TRUE,
    'subject' => $subject,
    'PERSONAL_NOTE' => $message, 
    'MEMBER_NAME' => $cacct->fullName, 
    'DATE' => u\formatDate($cacct->signed),
    'CODE' => r\invite($email, $cacct->id),
  );
  \drupal_mail('rCredits', 'invite', $email, '', $info);

  say('invite sent', compact('email'));
}

function formVideos($form, &$sta) {
  $title = item(t('rCredits Member Videos'));
  $subtitle = formField('item', '', '', t('The videos may not be available until mid-January. Try again in a few days!'));
  $form = compact(u\ray('title subtitle'));
  return labeled($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from r\go() in formSummary

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 */
function formSummary($form, &$sta) {
  global $base_url;

  $cacct = r\acct();
//  debug($cacct->can(-1)); // keep
  if (!$cacct) r\go('user');

  extract(be\creditInfo()->fancy, EXTR_PREFIX_ALL, 'my');

  $name = $cacct->name;
  $address = r\location($cacct, TRUE);
  $regionName = r\acct($cacct->community)->fullName;
  $floorDesc = t('Your balance can go this low for up to 30 days');
  
  if (!$cacct->can(B_COMPANY) or !$cacct->proSe()) {
// FAILS on mobile    $idLink = 'id="idcard"'; // use jQuery to create popup (see inc/misc.js)
    $idLink = 'href="Print-ID-Card" target="_blank"';
    $card = $cacct->can(B_CTTY_ADMIN) ? ("<a $idLink>" . t('Print ID Card') . '</a>') : ''; // no card for community accounts
    if ($cacct->agentId == 1) $card .= ' [<a href="http://localhost/devcore/rcredits/test?module=rcredits/rweb&feature=Signup&scene=&restart=1">run current test</a>]'; // DEBUG (keep)
  } else $card = t("To request employee ID cards, visit the <a href=\"$base_url/account/relations\">Account Relations</a> page.");

  $title = item(t('Account Summary'));
//  'subtext = basic_account_info();
  $accountName = formField('item', t('Name:'), $address, $cacct->fullName . ' ' . u\hug($name));
  $balance = u\formatAmount($bal0 = $cacct->available + array_sum($cacct->frozen));
  $usdBalance = u\formatAmount($cacct->usd);
  $total = u\formatAmount($bal0 + $cacct->usd);
  $balance = formField('item', t('Balance:'), '', "r$balance + us$usdBalance = $total");
  
  if ($cacct->id > 0) {
    $extra = r\agent()->can(B_ADMIN) ? " id#$cacct->id" : '';
    $accountId = formField('item', t('ID:'), $card, ($cacct->mainQid ?: 'N/A') . ' (' . $regionName . ")$extra");
    if ($cacct->can(B_MEMBER) and $cacct->can(B_PERSONAL)) $another = submit(t('Open Another Account'));
    $acctType = formField('item', t('Account type:'), '', $GLOBALS['account types'][$cacct->acctType+0]); // +0 for admin
    $floor = formField('item', t('Credit floor:'), $floorDesc, u\formatAmount($cacct->floor));
    $rewards = formField('item', t('Rewards:'), t('(to date)'), $my_rewards);
  }

  if (r\agent()->can(B_CTTY_ADMIN) and (!$cacct->proSe() or $cacct->id == 1)) { // ctty admin not managing own personal account
    $ctty = $cacct->community;
    if ($cacct->id < 0) { // managing the community itself
      $count = formField('item', t('Members:'), '', r\dbLookup('COUNT(*)', 'users', 'community=:ctty AND uid<>:ctty', compact('ctty')));
    } else { // managing some individual community member account
      $acctType = formField('select', t('Account type:'), '', $cacct->acctType+0, $GLOBALS['account types']); // +0 for admin
      if ($cacct->can(B_COMPANY)) $canCharge = boolField(t('Can charge'), t('This account can charge others unilaterally (in person)'), $cacct->hasBit(B_CHARGE) + 0);
      $floor = textField(t('Credit floor:'), $floorDesc, dft(u\formatAmount($cacct->floor)));

      $rTrader = boolField(t('rTrader?'), t('Does the submitted proof of ID pass muster? Is this account approved to use rCredits?'), $cacct->hasBit(B_OK) + 0);
      $notes = formField('textarea', t('Notes'), '', dft($cacct->notes));
    }
    $who = textField(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete('ctty'));
    $submit1 = $submit2 = submit();
  }

  if ($cacct->proSe() and !$cacct->can(B_COMPANY) and !formMembership(TRUE)) say('take a step');
  $form = compact(u\ray('title who submit1 accountName accountId another acctType canCharge balance floor rewards rTrader idApproved notes count submit2'));
  return labeled($form);
}

function formSummary_validate($form, &$sta) {
  if (op($sta) == 'another') r\go('another');
  extract(u\just('who floor', $sta['values']));
  if (isset($floor)) {
    if ($error = u\badAmount($floor)) return say($error, 'floor'); 
    $sta['values']['floor'] = -u\cleanAmount($floor);
  }
  if (@$who) {
    if (!$acct = whois($who, 'who', $sta['values'])) return FALSE;
    $sta['values']['who'] = $acct;
  }
}

function formSummary_submit($form, &$sta) {
  if (op($sta) == 'another') return r\go('another');

  extract(u\just('who floor notes canCharge rTrader idApproved', $sta['values']));
  if (!r\agent()->can(B_CTTY_ADMIN)) return; // hack attempt
  $cacct = r\acct();

  if (@$who and ($who->community == r\agent()->community or r\agent()->id == 1)) {
    svar('cuid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $cacct->fullName);
    return say('changed account', compact('newAcct', 'oldAcct'));
  } 
  
  if (($cuid = $cacct->id) > 0) {
//    $wasRTrader = $cacct->hasBit(B_OK);
    $cacct->setBit(B_OK, $rTrader);
    if (isset($canCharge)) $cacct->setBit(B_CHARGE, $canCharge);
    $cacct->update(compact(u\ray('floor notes')));

//    if ($rTrader and !$wasRTrader) { // signup incentive if promoted to rTrader
    say('account updated');
  }
}

function formAccount($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $pictureFilename = $cacct->auxFilename('picture');
  $pictureURL = $base_url . (file_exists(DRUPAL_ROOT . $pictureFilename) ? $pictureFilename : ('/' . \variable_get('user_picture_default')));
  $form['#user'] = $cacct->account();
  user_account_form($form, $sta);
// Each region has its own timezone.  system_user_timezone($form, $sta);

  $form['title'] = formField('item', t('Account Information'), '', weight(-99));
  $form['account']['name'] = $form['account']['timezone'] = NULL;
  $form['account']['mail']['#description'] = t('All emails from the system will be sent to this address. The email address is not made public.');
  $form['account']['pass']['#description'] = t('Enter your new password in both fields above.<br>To change your email address and/or password, enter your current password below.');
  $form['account']['pass']['#process'] = array('form_process_password_confirm', 'rCredits\\Web\\password_confirm');
  $form['account']['current_pass']['#description'] = '';
  $form['account']['current_pass']['#weight'] = 4;
  $form['account']['status']['#weight'] = 5;
  $form['account']['roles']['#weight'] = 6;
  $form['picture']['picture_upload']['#description'] .= t(' File size must be under 3MB.')
    . $cacct->can(B_PERSONAL) ? 
      t(' Your picture must be taller than it is wide (3:4 ratio is ideal) and must show your full face, filling most of the frame.')
    : t(' A picture in landscape orientation will look best, in rCredits promotions.');
  $form['picture']['picture_current']['#markup'] = <<<EOF
  <div class="user-picture">
  <img src="$pictureURL" />
  </div>
EOF;
  $form['submit'] = formField('submit', '', '', array('value' => t('Save')) + weight(99));
  $form['#user_category'] = 'account';
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'][0] = 'rCredits\\Web\\formAccount_validate'; // use ours instead
// (already has this)  $form['#validate'][] = 'user_validate_picture';
  return $form;
}

/**
 * Implements expand_password_confirm.
 */
function password_confirm($element) {
  $element['pass1']['#title'] = t("New Password (if any)");
  $element['pass2']['#title'] = t("Confirm New Password");
  return $element;
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formAccount_validate($form, &$sta) {
  $sta['values']['mail'] = $mail = trim($sta['values']['mail']);
  if (!emailOkay($mail, r\acct()->id)) return;
}

/**
 * @todo: delete picture if delete box checked.
 */
function formAccount_submit($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $id = $cacct->id;
  $pictureFilename = DRUPAL_ROOT . $cacct->auxFilename('picture');
  $values = $sta['values'];

  if ($picture = @$values['picture_upload']) {
    r\dbQ('DELETE FROM file_managed');
    $tempName = str_replace('temporary://', \file_directory_temp() . '/', $picture->uri);
    if ($cacct->can(B_PERSONAL)) {
      if (!fixPicture($tempName, R_PERSONAL_ASPECT, 'bad personal aspect')) return;
    } elseif (!fixPicture($tempName, R_COMMERCIAL_ASPECT, 'bad commercial aspect')) return;
    if (!rename($tempName, $pictureFilename)) return say('file save error', 'picture');
  }
  
  if (@$values['picture_delete'] and file_exists($pictureFilename)) unlink($pictureFilename);

  if (empty($values['pass'])) {
    unset($values['pass']); // don't overwrite old password except with a new one
  } else $values['pass'] = user_hash_password($values['pass']);
  
  if (r\acct()->update($values, 'mail pass status')) say('account updated');
}

function fixPicture($filename, $aspect, $errMsg) {
  if (!$info = @getimagesize($filename)) return say('not an image', 'picture');
  list ($w, $h) = $info;
  list ($w0, $h0) = explode(':', $aspect);
  $off = $w/$h - $w0/$h0;
  if (abs($off) > .25) return say($errMsg, 'picture');

  $img = new \Imagick($filename);
  list ($wFactor, $hFactor) = u\order($off > 0, 1 - abs($off), 1);
  if ($off > 0) $img->cropImage($w*$wFactor, $h*$hFactor, ($w-$w*$wFactor)/2, ($h-$h*$hFactor)/2); // narrow or shorten
  $img->resizeImage($w0 * R_PIXEL_FACTOR, $h0 * R_PIXEL_FACTOR, \Imagick::FILTER_CATROM, 1);
  if (!$img->writeImage($filename)) return say('file save error', 'picture');
  return TRUE;
}

/*
$thumb->destroy(); 
      $img = imagecreatetruecolor($w0 * R_PIXEL_FACTOR, $h0 * R_PIXEL_FACTOR);
      $sourceImg = imagecreatefromjpeg($tempName);
      $ims = getimagesize($image);
imagecopy($img,$org_img, 0, 0, 20, 20, 200, 150);
*/

/**
 * Account Selector form
 * appears above the whole Members Section
 * @todo: make help form say what it is help FOR
 */
function formAccounts($form, &$sta) {
  if (!$cacct = r\acct()) return NULL; // no account selector before login
  $cuid = @$cacct->id;
  if (!$accounts = be\accountChoices()) return FALSE;
  debug($accounts);
  if (!in_array($cuid, @$accounts) and $cacct->can()) array_splice($accounts, 1, 0, $cuid); // admin or ctty_admin
  debug($accounts);
  $choices = array();
  foreach ($accounts as $uid) {
    $acct = r\acct($uid);
    $balance = u\formatAmount($acct->balance());
    $choices[$uid] = "$acct->fullName: $balance";
  }

  $qid = r\quid(); // $credit_id = r\userField('credit_id', $cuid);
  $onchange = "this.form.submit();";
  $oneText = $choices[$cuid];
  
  $account = count($choices) == 1 ? 
      textField('', '', dft($oneText) + attrib(array('disabled' => TRUE, 'size' => strlen($oneText)+2)))
    : formField('select', '', '', dft($cuid) + attrib(compact('onchange')), $choices);
//  $link = item('<a href="http://rCredits.org">Main Site</a>');
  $submit = submit(); // default submission

  $chunks = explode('/', $_SERVER['REQUEST_URI']);
  if (isDEV) array_shift($chunks); // site is in a subfolder on the dev server
  $chunkIndex = (($chunks[1] == 'account' and @$chunks[2]) or $chunks[0] == 'user') ? 2 : 1;
  $function = $chunks[$chunkIndex];

  $showMenu = formField('item', '', '', button('Menu', '', 'Show the menu'));
  $showHelp = formField('item', '', '', button('Help', '', 'Help for ' . ucwords($function)));
//  $version = item(t('v 0.1d'));
  $form = u\prefixKeys('account_', compact(u\ray('account showMenu showHelp submit'))); // always give a dropdown

  $menu = formMenu($function);
  $help = help($function);
  $form += $help + $menu;

  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op account_account', $sta['values']));
  $cacct = r\acct();
  if (!$cacct->can(B_ADMIN) and !r\acct($account_account, $cacct->agentId)->can()) r\go('user/logout'); // hack attempt?
  if ($op == 'Menu') {
    r\go('menu'); 
  } else svar('cuid', $account_account); // this gets checked ONLY in setupGlobals()
}

function formMenu($page) {
  $menu = array()
        + goButtonIf(B_SELL, t('Charge'), 'charge', t('Charge someone'))
        + goButtonIf(B_BUY, t('Pay'), 'pay', t('Pay someone'))
  ;
  if ($lastXid = svar('lastXid')) $menu += goButton(t('Undo'), "undo", t('Undo the last transaction'));
  if (r\otherBalance(svar('lastOther'))) $menu += goButton(t('Customer Bal'), 'remainder', t("Display the customer's current balance."));
//  if (count($menu) == 1) $menu += goButton(t(''), '', ''); // filler
  $menu += array()
        + goButtonIf(B_MANAGE, t('Summary'), 'summary', t('Account summary'))
        + goButtonIf(B_MANAGE, t('Transactions'), 'transactions', t('Show transaction history'))
        + goButtonIf(B_MANAGE, t('Get r/USD'), 'exchange', t('Exchange rCredits for USD or vice-versa'))
        + goButton(t('Find Companies'), 'directory', t('Find an rCredits member company'))
//        + goButton(t('Help'), "help/$page", t('Help with this page'))
        + goButton(t('Reports'), 'reports', t('rCredits syste-wide or community-wide activity reports'))
        + goButtonIf(B_MANAGE, t('Account'), 'account', t('Basic account information'))
        + goButtonIf(B_MANAGE, t('Mobile'), 'account/cells', t('Authorize mobile devices for this account'))
        + goButtonIf(B_MANAGE, t('Relations'), 'account/relations', t('Connect other members to this account'))
        + goButtonIf(B_MANAGE, t('Preferences'), 'account/preferences', t('Set your account preferences'))
        + goButtonIf(B_MANAGE, t('Contact Info'), 'account/contact', t('Tell us where to find you'))
        + goButtonIf(B_MANAGE, t('Membership'), 'membership', t('Your rCredits membership progress'))
        + goButtonIf(array(B_COMPANY, B_MANAGE), t('Company Info'), 'account/company', t('Describe this company'))
        + goButton(t('Logout'), 'user/logout', t('End your rCredits session'))
  ;
  $menu = formField('fieldset', '', '', array('id' => 'menu')) + compact('menu');
  return compact('menu');
}

/**
 * Go to the button's destination if the permission requirement is met.
 * @param string $label: button label
 * @param string $goto: where to go when the button is pressed
 * @param string $title: what to say when hovering over the button
 * @return: the button html
 */
function goButton($label, $goto, $title) {
  $gotoHead = isDEV ? '/devcore' : '';
  $name = str_replace('/', '_', $goto);
  return array($name => formField('item', '', '', button($label, "$gotoHead/$goto", $title)));
}

/**
 * Go to the button's destination if the permission requirement is met.
 * @param mixed $perms: single permission number or array of permission numbers
 * Remaining params are as for goButton().
 */
function goButtonIf($perms, $label, $goto, $title) {
  $cacct = r\acct();
  if (!is_array($perms)) $perms = array($perms);
  foreach ($perms as $perm) if (!$cacct->can($perm)) return array();
  return goButton($label, $goto, $title);
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company) {
  $uid = r\userField('uid', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or !$acct->can(B_COMPANY)) return say('no such company', 'zot');
  extract((array) $acct->account(), EXTR_PREFIX_ALL, 'my');

  $pic = profile_picture($uid, TRUE);
  $country = u\countryAbbrev($my_country);
  $website = $my_website ? "<a href='http://$my_website' target='_blank'>$my_website</a>" : '';
  $phone = $my_phone ? 'Phone: ' . u\formatPhone($my_phone, '+') : '';
  $fax = $my_fax ? (($phone ? '&nbsp; * &nbsp;' : '') . 'Fax: ' . u\formatPhone($my_fax, '+')) : '';
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = r\dbQ($sql, compact('uid'))->fetchAllKeyed();
  $cats = empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats));

  $markup = <<<EOF
  $pic
  <div id='member-details'>
    <div id='member-contact'>
    <h2>$my_fullName</h2>
    $my_address<br>
    $my_city, $my_state $my_postalCode $country<br><br>
    $phone$fax<br>
    $website<br><br>
    </div>
    <div id='member-description'>$my_description</div>
    <div id='member-categories'>$cats</div>
  </div>
EOF;

  $form = array(
    'all' => formField('item', $markup),
  );
  return $form;
}

/**
 * Return specified account's profile picture, with or without markup
 * @return FALSE if none
 */
function profile_picture($uid, $with_markup = FALSE) {
  $acct = r\acct($uid);
  $data = @unserialize($acct->data);
  $filename = DRUPAL_ROOT . $acct->auxFilename('picture');
  $url = $base_url . $acct->auxFilename('picture');
  if (!$with_markup or !file_exists($filename)) return $url;

  $info = @getimagesize($filename);
  list ($width, $height) = is_array($info) ? $info : array(1, 0);
  $new_width = min(max($width, R_MIN_PHOTO_WIDTH), R_MAX_PHOTO_WIDTH);
  $new_height = round($new_width * $height / $width, 0);
  $subs = compact(u\ray('new_width new_height url'));
  return strtr('<div class="user-picture"><img src="url" width="new_width" height="new_height" alt="profile picture" /></div>', $subs);
}

function formSystem($form, &$sta, $args = '') {
//  $form = u\insertBefore(compact('communities'), formCommunity(array(), &$sta, $args), 'count');

/* (This dropdown might be better here than at the top of every page)
   (top of page dropdown would then include only the CURRENTLY selected community (and the admin))
  $rows = r\dbQ('SELECT uid, fullName FROM users WHERE uid<0 AND uid<=:R_FIRST_CTTY_UID ORDER BY fullName')->fetchAllKeyed();
  $cttyChoices = array_merge(array('ctty-0' => 'Select a community to manage'), u\prefixKeys('ctty', $rows));
  $community = formField('select', t('Manage:'), t('Select a community to manage'), '', $cttyChoices);
*/
  $title = item(t('System Administration'));
  $submit = submit();
  $form = compact(u\ray('title community submit'));
  return labeled($form);
}

function formSystem_submit($form, &$sta) {
  extract(u\just('', $sta['values']));
//  list ($community) = explode('-', $community, 2); // get the absolute value of community
//  if ($community) svar('cuid', -$community); // make it the new current account
}

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 */
function formTx($form, &$sta, $args = '') {
  if ($confirm = sureForm($sta)) return $confirm;
  
  extract(u\just('who amount goods purpose scanned', $args));
  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path())); // pay, charge, etc.
  $role = in_array($type, u\ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, u\ray('fine grant loan'));
  $purpose = @$purpose ?: ($type == 'charge' ? r\acct()->selling : ($type == 'pay' ? '' : $type));
  
  focusOn(@$who ? 'amount' : 'who');

  //$onchange = "nixwhat();";
  $radios = array(t('cash/loan/etc.'), t('goods & services'));
  
  // set up form fields
  $title = item(ucwords($type));
  $whoLabel = ($type == 'loan' ? 'Lend to' : ucwords($type)) . ' whom:';

  $onchange = $incentive ? '' : "if(this.value.toUpperCase() == 'ALL') jQuery('#edit-purpose').val('payroll');";
  $who = textField($whoLabel, t('Type a name, id, email, or phone'), dft(@$who) + autocomplete($role) + attrib(compact('onchange')));

  $amount = textField('Amount: $', t('Amount to ') . $type, required(@$amount));
  $goods = $incentive ? hidField(FALSE) : formField('radios', '&nbsp;', '', required(isset($goods) ? $goods : 1), $radios); //  + attrib(compact('onchange'))
  $purpose = textField('Purpose:', t('Description of goods and services or purpose'), dft(@$purpose));
  $scanned = hidField(@$scanned); // scanned means user already saw a name and photo
  $$type = submit(ucwords($type));

  $form = labeled(compact(u\ray("title who amount goods purpose $type scanned")));
  
  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  if (confirming_v($sta)) return;
  if ($confirm = transfer($sta['values'], FALSE)) confirm($confirm, $sta); // op who amount goods what
}

function formTx_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  transfer($sta['values'], TRUE); // op who amount goods what
  $path = current_path(); // clean out the fields
  if ($i = strpos($path, '/who=')) $path = substr($path, 0, $i);
  $sta['redirect'] = $path;
//  if (!t\EST()) r\go(current_path());
}

/**
 * Undo the last transaction.
 */
function undo($confirmed = FALSE) {
  if (!$xid = svar('lastXid')) r\go('');
  if ($confirmed) {
    list ($message, $args) = be\undoTx($xid);
    say($message, $args);
    if (@$args['success'] and svar('lastXid') == $xid) svar('lastXid', FALSE);
  } elseif ($confirmed === '0') {
    say('nothing undone');
  } else {
    $args = be\lastTx('xid=:xid', compact('xid'));
    $args['yesNo'] = yesNo('undo/1', 'undo/0');
    if ($args) say('confirm undo|yes or no', r\txReportArgs($args));
  }
  r\go('empty');
}

/**
 * Show remaining customer balance.
 */
function remainder() {
  $acct = svar('lastCustomer');
  if ($acct) {
    $otherName = $acct->fullName;
    $uid = $acct->id;
    $balance = be\creditInfo(compact('uid'))->fancy['balance'];
    say('customer balance', compact('otherName', 'balance'));
  } else say('no last customer', 'zot');
  r\go('empty');
}

function formEmpty($form, &$sta) {
  return array('title' => item('this'));
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
  
  list ($op, $amount, $goods) = array('Pay', $my_amount, TRUE);
  list ($who, $what) = @$my_confirmed ? array($my_who, $my_what) : array($my_company, $my_item);

  if (@$confirmed) {
    transfer(compact(u\ray('op who amount goods what')), TRUE);
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
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 *//*
function formGetR($form, &$sta) {
  $cacct = r\acct();
  $cuid = $cacct->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $request = u\formatAmount($cacct->demand);
  $demand = be\creditInfo()->fancy['totalDemand'];
  $minR = u\formatAmount($cacct->minimum);
  $subtext = tt(<<<EOF
<div class="r-explain">The amounts you request here are one-time requests for rCredits beyond the minimum level you set in your <a href="account/preferences">Account Preferences</a>. You will probably get at least part of your request within a week or so and can spend your new rCredits two weeks later. [<a href="help/why-wait">why the wait?</a>]</div><br>
<div class="r-amounts">
Your minimum rCredits level is set to <b>@minR</b><br>
Your current request for extra rCredits is <b>@request</b><br>
Total demand for rCredits overall is <b>@demand</b></div><br>
<div></div>
EOF
, compact(u\ray('request demand minR')));

  $form = array(
    'title' => item(t('Get Extra rCredits')),
    'subtext' => item($subtext),
    'amount' => textField(t('Amount:'), t('Amount of US Dollars to exchange for rCredits'), required()),
//    'way' => formField('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
//    'who' => textField('to/from:', t('Exchange with whom?')),
    'getr' => submit(t('Get rCredits')),
//    'getusd' => submit(t('Get USD')),
  );

  return labeled($form);
}

function formGetR_validate($form, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return;

  extract($sta['values']);
//  if (blank_field(compact('amount'))) return;
//  list ($message, $args, $confirm) = be\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cuid, FALSE);
  list ($message, $args, $confirm) = be\get('r', $amount, $cuid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $sta);
  if (!@$args['success']) say($message, $args, 'amount');
}

function formGetR_submit($form, &$sta) {
  $cacct = r\acct();
  if (confirming_s($sta)) return;

  extract($sta['values']);
  list ($message, $args, $confirm) = be\get('r', $amount, $cacct->id, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($amount != 0) {
    if ($amount < R_MIN_DEMAND) say('demand too small');
    if (!$cacct->can(B_PARTNER)) say('demand not yet');
  }
}
*/

/**
- ** (implement by creating an invite and redirecting to registration, using that code). Choices for the new account (similar choices should be in the invitation form and should be recorded in the invite):
  ONLY individuals can accept invitations (you have to create a personal account before you create a company account)
 */
function formAnother($form, &$sta) {
  $cacct = r\acct();
  if (!$cacct->can(B_PERSONAL)) r\go('', 'no permission');
  $title = item('Open Another Account');
  $relations = array(t('a company and I own at least 10% of it'), t('a company or organization that I work for, but I am not a 10%-or-more owner'), t('a family member or friend, with whom I have a primarily personal rather than financial relationship'), t('myself individually or jointly with others -- not for business'), t('none of the above'));
  $relationPrompt = item(t('What is the new account for?'));
  $relation = formField('radios', t(''), '', '', $relations);
  $flows = array(t('neither way'), t('current account can cover new account\'s debts and overdrafts'), t
('new account can cover current account\'s debts and overdrafts'), t('both ways'));
  $flowPrompt = item(t('Which way can the credit flow, when you want it to? (no funds will be moved automatically)'));
  $flow = formField('radios', t(''), '', '', $flows);
  $submit = submit();
  return labeled(compact(u\ray('title relationPrompt relation flowPrompt flow submit')));
}

function formAnother_validate($form, &$sta) { 
  foreach (u\ray('relation flow') as $field) {
    $$field = $sta['input'][$field]; // values fails here (?!!)
    if (!is_numeric($$field)) say('required field', compact('field'), $field);
  }
  if ($relation == 4 and $flow == 3) say('insufficient relation', 'relation');
}

function formAnother_submit($form, &$sta) {
  $cacct = r\acct();
  extract(u\just('company companyPhone isOwner employeeOk', $cacct->data));
  extract(u\just('relation flow', $sta['input']));
  $by = $cacct->id;
  $code = r\invite($cacct->mail, $cacct->id);
  $flow += 0;
  $isOwner |= ($relation == 0);
  $employeeOk |= ($relation == 1);
  if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = array($company, $companyPhone);
  r\go('user/register/' . http_build_query(compact(u\ray('code fullName phone isOwner employeeOk flow by'))));
}

/**
 * Modify the user registration form.
 * @param string $args: if not empty, a urlencoded assoc (from an email invitation or the "Create Another Account" button):
 *   code: the invitation code 
 *   company: the likely name of the account to create
 *   companyPhone: the account's likely phone number
 *   isOwner: is the inviter an owner
 *   employeeOk: is the inviter an employee
 */
function formRegister(&$form, &$sta, $args = '') {
  if (!$args) $args = basename(t\EST() ? t\SERVER('REQUEST_URI') : $_SERVER['REQUEST_URI']);
  extract(u\just('code fullName phone', $args));
  
  // zap standard Drupal fields
  $form['account']['name'] = NULL; // name is calculated from fullName
  $form['account']['mail'] = NULL; // (e)mail is redefined here below

  $form['#attributes']['class'][] = 'rweb';
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  css('rweb.css', -9); // needed because this function doesn't go through show()
  
  $title = item(t('Sign up for rCredits'));
  list ($email, $inviter) = invitation(@$code);
  if (!$email and @$code != bin2hex(R_WORD) and @$code != 'w') {
    $form['actions']['submit'] = NULL; // nothing to submit
    $subtext = item(tt('bad invite'));
    $form['account']['mail'] = hidField('unused@example.com'); // keep Drupal processing happy
    $form += compact(u\ray('title subtext mail'));
    return $form;
  }
  $selfInvite = ($email == @r\acct($inviter)->mail);
  
  $form['#attributes']['class'][] = 'labeled'; // "invitation required" message is not "labeled", so this comes after
  js('inc/radiocheck.js', 'file', 'footer'); // this function is not called by showForm, so we have to do this here
  $form['#validate'] = array('rCredits\\Web\\formRegister_validate');
  $form['#submit'] = array('rCredits\\Web\\formRegister_submit');

  $acctTypes = $GLOBALS['account types'];
  if (!$selfInvite) $acctTypes = u\just('0 1', $acctTypes);
  $acctType_help = t('For all but Personal accounts, contact information will be public (except your email address, which is never made public). If you are self-employed, we recommend that you open a separate account for the business. Choose "personal AND commercial" only if you use your own social security number and photograph for the business and you don\'t want a separate account.');
  $companyOptions = array('employeeOk' => t('employee'), 'isOwner' => t('owner'));
  $onchange = "var info = jQuery('#edit-companyInfo'); var calling = jQuery('.form-item-calling'); if (jQuery('#edit-acctType').val() == 0) info.show(); else info.hide(); if (jQuery('#edit-acctType').val() <= 1) calling.show(); else calling.hide();";
  $acctType_attrib = attrib(compact('onchange'));
  
  $companyInfo = array(
    'company_text' => item(t('Are you an owner and/or employee of a local company, or of any participating rCredits company? Otherwise leave this section blank.')),
    'company' => textField(t('Company: '), t(''), autocomplete('company')),
    'companyPhone' => textField(t('<small>Company</small> Phone:'), t('What is the company\'s primary phone number?')),
    'companyOptions' => formField('checkboxes', '', '', '', $companyOptions),
  );
  $emailHelp = t('All emails from the system will be sent to this address. It will not be made public. If you want to use this address <b>for more than one account</b>, include a tag on the local part starting with "+" (for example, myemail<b>+whatever</b>@example.com).');

  $mail = textField(t('Email:'), $emailHelp, required());
  $acctType = formField('select', t('Account type:'), $acctType_help, dft('Personal') + $acctType_attrib, $acctTypes);
  $companyInfo = formField('fieldset', '', '', array('id' => 'companyInfo')) + $companyInfo;
  
  $calling = textField(t('Calling:'), t('One of the main purposes of rCredits is to provide ourselves the opportunity to do what we feel called to do in life. What would bring you joy to do, to earn rCredits (that you\'re not doing yet)? What might you have to offer?'));
  $buysAts = r\dbQ('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
  $buysAt = formField('checkboxes', t('<br>Over 60 Greenfield businesses have already signed up. Which of these <b>not-yet-member</b> businesses do you frequent?'), '', '', $buysAts);
  
  $args = hidField($args);
  
  $defaults = compact(u\ray('country state fullName phone'));
  $form += compact('title') + contactFields($form, $defaults) 
    + compact(u\ray('acctType companyInfo calling buysAt args'));
  $form = u\insertBefore(compact('mail'), $form, 'phone');

  keep_values($form);
  t\snapShot($form);
}

function formRegister_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = array(); // for testing (keep Drupal from griping)
  extract(u\just('args mail company companyPhone companyOptions fullName postalCode code', $values = u\trimAll($sta['values'])));
  if (@$companyOptions) extract(u\just('isOwner employeeOk', $companyOptions));
  if (@$args) extract(u\just('code', $args));
  $sta['values'] = array(); // make sure Drupal doesn't save the record when WE find an error (no longer needed?)
  list ($email) = invitation(@$code);
  if (!$email and $code != bin2hex(R_WORD) and $code != 'w') return r\go('user/register'); // in case user resubmits
    
  if (!emailOkay($mail)) return;
  
  if (@$company) {
    if (!$companyPhone = u\formatPhone($companyPhone, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    $sta['companyPhone'] = $companyPhone;
    if (@$companyOptions) extract(u\just('employeeOk isOwner', $companyOptions));
    if (!(@$employeeOk or @$isOwner)) return say('what relation', 'company');
    $company = $company;
    $data = compact(u\ray('company employeeOk isOwner'));
  }
  
  if ($error = \user_validate_name($fullName)) return say(str_replace('The username', 'That name', $error), 'fullName');
  $name = $name0 = r\uniqueName($fullName);
//  $name = $name0 = u\shortName($fullName);
//  if (r\userField(1, 'name=:name', compact('name'))) return say('duplicate name'); // require unique fullName

  $params = u\ray('fullName city address'); // prepare for pass-by-reference
  list ($fullName, $city, $address) = u\normalizeCase($params);
  $community = r\communityUid($postalCode);
  $flags = B_DEFAULTS;
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  $sta['values'] = array_merge($values, compact(u\ray('name data community flags administer_users status')));
}

function formRegister_submit($form, &$sta) {
  $fields1 = 'args company companyOptions companyPhone';
  $fields2 = 'name fullName mail phone fax country postalCode state city address';
  debug($sta['input']);

  $values = $sta['values'];
  extract(u\just("name acctType $fields1", $values));
  extract(u\just('code', $args));
  $pass = \user_password();
  $flags = B_DEFAULTS
    | ($acctType <= R_SELF_EMPLOYED ? u\bit(B_PERSONAL) : 0)
    | ($acctType >= R_SELF_EMPLOYED ? u\bit(B_COMPANY) : 0);

  if ($acctType >= R_SELF_EMPLOYED) { // company
    $type = $GLOBALS['account types'][$acctType];
    $structure =
        ($acctType == R_SELF_EMPLOYED ? u\bit(R_SELF_EMPLOYED)
      : (strpos($type, 'nonprofit') ? u\bit(R_NONPROFIT)
      : (strpos($type, 'corporation') ? (u\bit(R_CORPORATION) | u\bit(R_PUBLICLY_TRADED)) : 0)));
  }
  $cacct = new r\acct(u\just($fields2, $values) + compact(u\ray('pass flags structure')));
  u\EXPECT(($cuid = $cacct->id) > 0, 'registration failure');
  setupGlobals($cuid); // some rcredits functions need to know who we are
  
  if (@$company) {
    extract(u\just('isOwner employeeOk', $companyOptions));
    $isOwner = (bool) $isOwner; // POSTed as "isOwner" when TRUE
    $employeeOk = (bool) $employeeOk;
    // (can't use whois to create record because user is logged out) $companyAcct = whois($company, 'company', $values);
    if ($main = findCompany($company, $companyPhone)) { // company account exists
      $other = $cuid;
      r\dbInsert('r_relations', compact(u\ray('main other employeeOk isOwner')));
    } else {
      $data = compact(u\ray('company companyPhone isOwner employeeOk'));
      $cacct->update(compact('data'));
    }
  } 
  
  $buysAt = 0; // store a bit for each bought-at company
  foreach ($sta['input']['buysAt'] as $one) if (@$one) {
	r\dbQ('UPDATE r_nonmembers SET potential = potential+1 WHERE id=:one', compact('one'));
	$buysAt |= u\bit($one);
  }
  $data = $cacct->data + compact('buysAt');
  $cacct->update(compact('data'));
  
  extract($params = u\just('isOwner employeeOk flow by', $args));
  if (@$by) { // created by a related account? set it up
    if (!$error = u\EXPECT($params, 'bool bool int id', TRUE)) { // if not a hack attempt
      list ($main, $other, $employerOk, $permission) = array($cuid, $by, $employeeOk, B_MANAGE - B_RELATED);
      r\dbInsert('r_relations', compact(u\ray('main other employerOk employeeOk isOwner flow permission')));
    } else r\notifyStaff('bad params in formRegister');
  }
  
  $quid = r\quid($cuid, $cuid);
  tellStaffNewMember(u\just("$fields1 $fields2", $values) + compact('by', 'quid'));

//  if (@$_SESSION['messages']['status'][0] == t('A welcome message with further instructions has been sent to your email address.')) unset($_SESSION['messages']['status']);

  if ($acctType <= R_SELF_EMPLOYED) { // personal
    say('your account is ready', compact(u\ray('quid')));
    r\notify($cuid, 'welcome', compact(u\ray('name quid pass'))); 
  } else { // non-personal (no independent login)
    say('company is ready', compact(u\ray('quid')));
    $quie = r\quid($by, $by);
  }

  r\go('user/login/name=' . strtolower($quid));
}

function emailOkay($mail, $uid = '') {
  if ($error = \user_validate_mail($mail)) return say($error, 'mail');
  if ($duplicateAccount = r\dbLookup('fullName', 'users', 'mail=:mail and uid<>:uid', compact('mail', 'uid'))) {
    $passwordLink = \url("user/password/$mail");
    $emailTagged = str_replace('@', '+whatever@', $mail);
    if (!$uid) $forgot = '|forgot password';
    return say('duplicate email' . @$forgot, compact(u\ray('emailTagged duplicateAccount passwordLink')), 'mail');
  }
  return TRUE;
}

/**
 * Tell the staff about a new member.
 */
function tellStaffNewMember($params) {
  extract($params);
  $phone = u\formatPhone($phone);
  $fax = u\formatPhone($fax);
  $country = r\realCountry($country);
  $state = r\realState($state);
  $acctType = $GLOBALS['account types'][$acctType];
  $by = r\acct(@$by)->fullName;

  $companyOptions = json_encode($companyOptions);
  $params = compact(u\ray('phone fax country state acctType by')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  foreach ($params as $key => $value) $message .= "\n$key: $value<br>";
  r\notifyStaff($message);
}

function findCompany($company, $companyPhone) {
  $companyShortName = u\shortName($company);
  $where = '(flags&(1<<B_COMPANY)) AND ';
  $companyUid = r\dbLookup('uid', 'users', $where . 'phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = r\dbLookup('uid', 'users', $where . 'name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = r\dbLookup('uid', 'users', $where . 'name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Return the part of the form that is common go formContact and formRegister
 * (the contact fields)
 */
function contactFields($form, $defaults = array()) {
  $country = $state = ''; // make sure these are not NULL
  extract(u\just('fullName phone fax website country postalCode state city address', $defaults));

  $fullnameDesc = t('Your full legal name, properly capitalized');
  $onchange = "print_state(this.options[this.selectedIndex].value,'');";
  $countryAttrib = attrib(compact('onchange'));
  js('inc/countries.js', 'file', 'header');
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');
  focusOn('fullName');
  $onchange = "if (jQuery('#edit-postalCode').val().substr(0, 3) == '013') jQuery('.form-item-buysAt').show();";
  $zipAttrib = attrib(compact('onchange'));

  $form += array(
    'fullName' => textField(t('Full name:'), $fullnameDesc, required(@$fullName)),
    'phone' => textField(t('Phone:'), t('Your primary phone (landline or cell)'), required(@$phone ? u\formatPhone($phone): '')),
    'fax' => textField(t('Fax:'), t(''), dft(@$fax ? u\formatPhone($fax) : '')),
    'country' => formField('select', t('Country:'), t(''), dft(@$country) + $countryAttrib), // can't use required() here
    'postalCode' => textField(t('Postal code:'), t(''), required(@$postalCode) + $zipAttrib),
    'state' => formField('select', t('State:'), t(''), dft(@$state)), // can't use required() here
    'city' => textField(t('City:'), t(''), required(@$city)),
    'address' => textField(t('Address:'), t('Mailing address'), required(@$address)),
  );

  return $form;
}  

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  be\mergeCheck(compact(u\ray('name phone fax'))); // check before and after
  $cacct = r\acct();
//  extract((array) $cacct->account(), EXTR_PREFIX_ALL, 'my');

  $title = item(t('Contact Information'));
  $submit = submit();
  $defaults = u\just('fullName phone fax country postalCode state city address', $cacct->account());
  $form = compact('title') + contactFields($form, $defaults) + compact('submit');
  
  if ($cacct->can(B_COMPANY)) $form['fullName']['#description'] .= '<br>' . t('(If you change this, your username and rCredits webpage address will also change.)');

  return labeled($form);
}

function formContact_validate($form, &$sta) {
  $cuid = r\acct()->id;
  extract(u\just('phone fax city address fullName country state', u\trimAll($sta['values']))); // trim ALL values, for later
//  if (blank_field(compact(u\ray('phone city address')))) return;

  // add dropdowns, just in case there is an error
  js('inc/countries.js', 'file', 'header');
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');

  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['values']['phone'] = $phone;
  if ($fax and !($fax = u\formatPhone($fax, '+n'))) say('bad phone', 'fax'); else $sta['values']['fax'] = $fax;
  if ($fullName != r\acct()->fullName) {
    $shortName = u\shortName($fullName);
    $otherName = r\userField('fullName', 'name=:shortName AND uid<>:cuid', compact('shortName', 'cuid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
    $sta['values']['name'] = $shortName;
  }
}

function formContact_submit($form, &$sta) {
  extract($info = u\just('fullName name phone fax country postalCode state city address', $sta['values']));
  if (r\acct()->id == 1) unset($info['name']); // don't change 'admin'
  r\acct()->update($info);
  be\mergeCheck(compact(u\ray('name phone fax'))); // check before and after
  say('info saved');
}

/**
 * Draft an individual or company into the database.
 * Called when a member tries to pay or charge a non-member.
 */
function formDraft($form, &$sta, $args = '') {
  extract($info = u\just('who return amount goods purpose', $args));
  $fullnameDesc = t('Their full legal name, properly capitalized');
  unset($info['return']); // be neat
  if (!$phone = u\formatPhone($who)) $fullName = $who;
  
  $form = array(
    'title' => item(t('Draft a Potential New rCredits Member')),
    'subtext' => item(t('Complete both fields and click on "Submit" OR "Cancel".')),
//    'subtext' => basic_account_info(),
    'fullName' => textField(t('Full name:'), $fullnameDesc, required(@$fullName)),
    'phone' => textField(t('Phone:'), t('Their primary phone (landline or cell)'), required($phone)),
    'argString' => hidField(http_build_query($info)),
    'return' => hidField(str_replace(R_URL_SLASH, '/', urldecode($return))), // undo original Drupal-anomaly workaround
    'submit' => submit(),
    'cancel' => submit(t('Cancel')),
  );

  return labeled($form);
}

function formDraft_validate($form, &$sta) {
  global $base_url;
  extract(u\just('op fullName phone argString return', $sta['values'])); // (trim ALL values for later)
  if (op($sta) == 'cancel') {
    drupal_get_messages('error'); // don't complain about blank fields
    return r\go("$return/$argString");
  }
  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['values']['phone'] = $phone;
  if (!u\validName($fullName, FALSE, FALSE)) say('bad name', 'name');
  $sta['values']['fullName'] = u\normalizeCase($fullName);
  $shortName = u\shortName($fullName);
  if (($otherName = r\userField('fullName', 'name=:shortName or phone=:phone', compact('shortName', 'phone')))) {
    $draftLink = "$base_url/$return/who=$otherName&$argString";
    say('similar found', compact(u\ray('otherName phone draftLink')), u\shortName($otherName) == $shortName ? 'fullName' : 'phone');
  } else $sta['values']['name'] = $shortName;
}

function formDraft_submit($form, &$sta) {
  extract(u\just('argString return name', $info = $sta['values']));
  $acct = new r\acct($info);
  say('info saved|ok to continue');
  $sta['redirect'] = "$return/who=$name&$argString";
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}


/**
 * Get the Dwolla account info
 * @param string $args: params or report from Dwolla (via usd::callback()) on success of getting auth token
 */
function formUsd($form, &$sta, $args = '') {
  extract(u\just('show connected', $args));
  if (@$connected == 'yes') {
    say('got token'); 
  } elseif (@$connected) say("There was an error. Dwolla said: $connected", 'zot');
  $cacct = r\acct();
  
  if ($cacct->offsite) $federalId = $dob = $answer = R_ON_FILE;
  if ($cacct->usdAccount) {
    $usdAccount = $pin = R_ON_FILE;
    if (!$cacct->secure['auth']) say('no auth yet', 'usdAccount');
  }

  $form = array(
    'title' => item(t('USD Account and Security Info')),
    'subtext' => item(t('If you need help completing this section, visit the rCredits office in Greenfield, MA or call @phone.', array('@phone' => R_ORG_PHONE))),
    'usdAccount' => textField(t('Account:'), t('Your 10-digit Dwolla account number'), required(@$usdAccount)),
    'pin' => textField(t('PIN'), t('Your 4-digit Dwolla PIN'), required(@$pin)),
    'federalId' => $cacct->can(B_PERSONAL) ?
        textField(t('Soc Sec #:'), t('Your social security number (for required annual reporting)'), required(@$federalId))
      : textField(t('Federal ID:'), t('Your official employer ID number (EIN)'), required(@$federalId)),
    'dob' => textField(t('Birthdate:'), t('Your date of birth'), required(@$dob)),
    'question' => textField(t('Question:'), t('Type a question that no one but you could ever guess the answer to.'), required($cacct->question)),
    'answer' => textField(t('Answer:'), t('The answer to that question'), required(@$answer)),
    'idProof' => file_exists(DRUPAL_ROOT . $cacct->auxFilename('proof')) ? 
        formField('item', t('Photo ID:'), '', R_ON_FILE . ($cacct->hasBit(B_OK) ? '' : t(' (pending approval)')))
      : formField('file', t('Photo ID:'), t('Upload an image of your driver\'s license, passport, or other official photo ID.'), attrib(array('enctype' => "multipart/form-data"))),
    'submit' => submit(),
  );

  if ($cacct->can(B_CTTY_ADMIN)) {
    foreach (u\ray('usdAccount pin federalId dob answer idProof') as $field) {
      u\prefix($form[$field]['#description'], show($field, @$show));
    }
    unset($form['subtext']);
  }
  
  return labeled($form);
}

/**
 * Return a "SHOW" link for the field or a display of the field's value, for community admins only.
 * @param string $field: the field in question
 * @param string $show: the field to display, if any (default none)
 * @return the appropriate markup
 */
function show($field, $show = '') {
  global $base_url;
  $cacct = r\acct();
  
  if ($field == $show) {
    if (strpos(R_SECURE_FIELDS, $field) !== FALSE) {
      $secure = $cacct->secure;
      $result = $secure[$field];
    } elseif ($field == 'idProof') { // display the proof image alone (then user clicks on Back)
      $proofFilename = DRUPAL_ROOT . $cacct->auxFilename('proof');
      $imgString = u\decode(file_get_contents($proofFilename), hex2bin($cacct->idProof));
      header("Content-Type: image/jpeg");
      imagejpeg(imagecreatefromstring($imgString));
      exit();
    } elseif ($field == 'dob') {
      $result = u\formatDate($cacct->$field);
    } elseif (strpos(R_SECRET_FIELDS . ' usdAccount', $field) !== FALSE) {
      $result = $cacct->$field;
    }
  } else $result = "<a href=\"$base_url/account/usd/show=$field\">SHOW</a>";
  return "[$result] ";
}

function onFile($field, $sta) {
  return ($sta['complete form'][$field]['#value'] == R_ON_FILE and $sta['values'][$field] == R_ON_FILE);
}

function formUsd_validate($form, &$sta) {
  $cacct = r\acct();
  $proofFilename = DRUPAL_ROOT . $cacct->auxFilename('proof');
  extract(u\just('usdAccount pin federalId dob', $sta['values']));
  if (!onFile('usdAccount', $sta)) {
    $usdAccount = $sta['values']['usdAccount'] = u\digits($usdAccount);
    if (!preg_match('/^[0-9]{10}$/', $usdAccount)) return say('bad account number', 'usdAccount');
  }
  if (!onFile('pin', $sta)) {
    $pin = $sta['values']['pin'] = u\digits($pin);
    if (!preg_match('/^[0-9]{4}$/', $pin)) return say('bad pin', 'pin');
  }
  if (!onFile('federalId', $sta)) {
    $federalId = $sta['values']['federalId'] = u\digits($federalId);
    $what = r\acct()->can(B_PERSONAL) ? 'social security number' : 'federal ID';
    if (!preg_match('/^[0-9]{9}$/', $federalId)) return say('bad federal id', compact('what'), 'federalId');
  }
  if (!onFile('dob', $sta)) { // 12/14/1901 is the earliest
    if (!$dob = $sta['values']['dob'] = strtotime($dob) or $dob > REQUEST_TIME) return say('bad date', 'dob');
  }
  if (@$_FILES['files']['name']['idProof']) { // fine to let this be uploaded separately
    if (!$file = file_save_upload('idProof', array(), '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime)) return say('bad file type', 'idProof');
    $tempfile = file_directory_temp() . '/' . $file->filename;
    if (!$key = $sta['values']['idProof'] = bin2hex(u\encryptFile($tempfile, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    \file_delete($file);
  } elseif(file_exists($proofFilename)) {
    $sta['values']['idProof'] = R_ON_FILE; // flag for no change
  } else unset($sta['values']['idProof']); // no attempt yet to provide this
 }

function formUsd_submit($form, &$sta) {
  include_once __DIR__ . '/../usd.class'; // treat this as an extension of this file, just to make menu ['file'] consistent

  $usd = new r\usd(r\acct());
//  debug($usd->bank(-.12, $error));debug($error); return;
  /*r\usd::beginAtom();
  debug($usd->send(.01, new r\usd(r\acct(2)), 'my note',FALSE, TRUE, $error));debug($error); return;
  r\usd::commit();*/
  
  $usdFields = '';
  $cacct = r\acct();
  $info = u\just('type usdAccount pin auth federalId dob question answer idProof', $sta['values']);
  $secure = @$cacct->secure ?: array();
  $retry = (!@$secure['auth'] and $info['usdAccount'] == R_ON_FILE and $info['pin'] == R_ON_FILE);

  foreach ($info as $key => $value) if ($value == R_ON_FILE or $value == @$form[$key]['#default_value']) unset($info[$key]); // don't change if on file or unchanged
  extract($info); // extract just the ones that changed

  $info = u\just(R_SECRET_FIELDS . ' question', $info);
  $saved = (empty($info) or $cacct->update($info));

  if (isset($usdAccount) or isset($pin) or $retry) { // this must be last, because it redirects to Dwolla
    $type = 'Dwolla';
    $auth = ''; // no valid auth token until Dwolla calls us back
    $secure = compact(u\ray('type pin auth')) + $secure; // secure includes more than these
    $cacct->update(compact('secure', 'usdAccount'));
    $usd = new r\usd($cacct);
    exit($usd->getAuth()); // get (new) auth token (does not return until after a visit to Dwolla)
  }
  
  if ($saved) say('info saved'); else say('info not saved', 'zot'); // don't say anything if getting auth
}

/**
 * Get the bank account info
 * @todo: store two verification EFT deposit amounts in bank_account_status as 1000*one+two
 * (if status > 100, it's waiting for verification)
 */
 /*
function formBank($form, &$sta) {
  extract((array) r\acct()->account(), EXTR_PREFIX_ALL, 'my');

  if (!$my_bank_account_name) $my_bank_account_name = $my_fullName;
  
  $form = array();
  if ($my_country == R_COUNTRY_ID) {
    $my_routing_number = substr($my_bank_account_number, 4, 9); // chop of USkk
    $my_bank_account_number = substr($my_bank_account_number, 4 + 9); // everything after the routing number
    $routing_number = textField(t('Routing number:'), t('What is the 9-digit routing number (type carefully)'), required($my_routing_number));
  } else $routing_number = NULL;
  
  $form = array(
    'title' => item(t('Bank Information')),
//    'subtext' => ,
    'bank_account_name' => textField(t('Account name:'), t('What is the name on this bank account?'), required($my_bank_account_name)),
    'routing_number' => $routing_number,
    'bank_account_number' => textField(t('Account number:'), t('What is the account number? (be sure to get this right)'), required($my_bank_account_number)),
    'submit' => submit(),
  );

  return labeled($form);
}

function formBank_validate($form, &$sta) {
  extract(u\trimAll($sta['values']));
  if (!u\validName($bank_account_name, FALSE, FALSE)) say('bad name', 'bank_account_name');
  if (!preg_match('/^(|[0-9]{9})$/', $routing_number)) say('bad routing number', 'routing_number');
  if (!preg_match('/^[0-9]{3,20}$/', $bank_account_number)) say('bad account number', 'bank_account_number');
}

function formBank_submit($form, &$sta) {
  extract($sta['values']);
  if (r\acct()->country == R_COUNTRY_ID) $bank_account_number = "USkk$routing_number$bank_account_number";
  r\acct()->update(compact(u\ray('bank_account_name bank_account_number')));
  say('info saved');
}
*/

/**
 * Give someone access to the current account or mark them as an employee or owner.
 * This function uses jQuery code defined in misc.js
 */
function formRelations($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $cuid = $cacct->id;
  $notCo = !$cacct->can(B_COMPANY);
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $account_identifiers = be\accountName($cuid);
  $header = u\ray('Other account,My employer?,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person employer employee owner permission link'));
  if (!$notCo) $header[1] = '$/mo';
  if (!$cacct->can(B_PERSONAL)) $header[3] = 'Owner?';
  
  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, s.amount, IF(:cuid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.employerOk, s.employeeOk, s.isOwner
    FROM (r_relations s RIGHT JOIN users u ON u.uid = s.main)
      RIGHT JOIN users v ON v.uid = s.other
    WHERE s.main=:cuid OR (s.other=:cuid AND (s.employeeOk OR s.employerOk))
    ORDER BY fullName, IF(:cuid=s.main, s.other, s.main), (:cuid=s.other)
EOF;
  $result = r\dbQ($sql, compact('cuid'));
  $rows = $raws0 = $raws = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original data from table
    $isAgent = ($cuid != $main);
    $combo = ($isAgent and @$previousAgent == $main); // related both ways
    $otherNotCo = !r\acct($isAgent ? $main : $other)->can(B_COMPANY);
    $otherNotPerson = !r\acct($isAgent ? $main : $other)->can(B_PERSONAL);
    $idLink = (!$isAgent and $otherNotCo and $employeeOk and $permission >= B_SELL) ? "<a target=\"_blank\" href=\"$base_url/Print-ID-Card/$other\">print&nbsp;ID&nbsp;Card</a>" : R_NONE;

    $ii = (count($rows) - ($combo ? 1 : 0)) . "-$reid"; // "row-reid"
    $discrepant = ($employerOk xor $employeeOk); // exactly one has claimed the relation
    list ($employeeOk, $employerOk) = array($employeeOk and $isAgent, $employerOk and !$isAgent);
    $isEmployer = toggle('employeeOk', $employeeOk, $ii, $isAgent and $discrepant);
    $isEmployee = $otherNotCo ? toggle('employerOk', $employerOk, $ii, !$isAgent and $discrepant) : R_NONE;
    $isOwner = ($isOwner and !$isAgent);
    $isOwner = toggle('isOwner', $isOwner, $ii);
    $permission = $isAgent ? 0 : $permission;
    $perm = $otherNotPerson ? R_NONE : dropdown('permission', $permission, $GLOBALS['share permissions'], $ii);
    $amountValue = amountField('amount', number_format($amount,0), $ii);

    if ($combo) { // employee record preceded by other record for same entity
      $thisRow = array_pop($rows); // throw out everything (everything but $isEmployer), use previous record data
      if ($notCo) $thisRow[1] = $isEmployer;
      $thisRaw = array_pop($raws);
      $thisRaw['employeeOk'] = $employeeOk;
    } else {
      $thisRow = array($fullName, $notCo ? $isEmployer : $amountValue, $isEmployee, $isOwner, $perm, $idLink);
      $thisRaw = compact(u\ray(($notCo ? 'employeeOk' : 'amount') . ' employerOk permission isOwner'));
//      if (!$notCo) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    }
    
    $rows[] = $thisRow;
    $raws[] = $thisRaw;
    $previousAgent = $other;
  }
  $sta['original_settings'] = compact('raws0', 'raws');
  
  $help = t("Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. For employees and suppliers, say how much you typically pay them in 4 weeks.");
  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$account_identifiers</span><br><br>$help";

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
    'new_person' => textField(t('Add Someone:'), t("name, id, email, or phone"), autocomplete('relation')),
    'list' => formField('item', '', '', array('markup' => $list)),
//    'new_permission' => formField('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'go' => submit(t('Save Changes')),
  );

  return $form;
}

function formRelations_validate($form, &$sta) {
  if (confirming_v($sta)) return;
  $cacct = r\acct();
  $cuid = $cacct->id;
  extract(u\just('new_person', $sta['values']));
  
  if (@$new_person) {
    $acct = whois($new_person, 'new_person', $sta['values'], FALSE, 'no self-relation');
//    if ($acct->can(B_COMPANY)) return say('no company relations', 'new_person');
    if (r\relation(1, $cuid, $acct->id)) return say('already related', 'new_person');
    $sta['values']['other'] = $acct->id; // save for submission handling
  }
  $managers = 0; // for company accounts there needs to be at least one manager
  foreach ($_POST as $key => $value) { // change this (and the _POST below) to $sta['input'] ?
    if (!strpos($key, '-')) continue;
    list ($keya, $rawnum, $reid) = explode('-', $key);
    if ($keya == 'amount') {
      if (u\badAmount($value)) return say('specific bad amount', array('amount' => $value), 'amount');
      $_POST[$key] = $value;
    } elseif ($keya == 'permission') {
      if ($value == B_MANAGE - B_RELATED) $managers++;
    }
  }
  if (!$cacct->can(B_PERSONAL) and !$managers) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
  if (confirming_s($sta)) return;

  $cuid = r\acct()->id;

  if ($other = @$sta['values']['other']) { // adding a new relation
    $main = $cuid;
    $permission = 0; // default to no permissions
    $info = compact(u\ray('main other permission'));
    r\dbInsert('r_relations', $info);
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }

  $changes = array();
  extract(u\just('raws0 raws', $sta['original_settings']));
//  debug($sta['original_settings']);
  // Drupal doesn't pass along our hidden inputs, so get them directly from $_POST ...
  foreach ($_POST as $key => $value) { // ... be\updateRelations will have to check permissions and data carefully
    if (!strpos($key, '-')) continue;
    list ($key, $rawnum, $reid) = explode('-', $key); // eg split permission-2-37
    if (!in_array($key, array_keys($raws))) return 'bad field in relations'; // hack attempt?

    $value = 0 + strtr($value, array('true' => 1, 'false' => 0));
//    debug(compact('value','rawnum','key'));
//    debug(0 + $raws[$rawnum][$key]);
    if ($value != 0 + $raws[$rawnum][$key]) { // changed?
      $changes[$reid][$key] = $value;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
//  debug($changes);
  if (!empty($changes)) say(be\updateRelations($changes));
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  $zot = array('zot' => formField('select', '', '', compact('value') + attrib(compact('name')), $options));
  return render($zot);
}

function amountField($name, $value, $id) {
  $name = "$name-$id";
  $class = 'amount'; $class = compact('class');
  $zot = array('zot' => textField('', '', compact('value') + attrib(compact('name', 'class'))));
  return render($zot);
}

function toggle($field, $value, $id, $highlight = 'never') {
  $path = current_path();
  $name = $id = "$field-$id";
  $zot = array($id => hidField(attrib(compact(u\ray('id name value')))));
  $input = \render($zot);
  $b1 = button(' - ', "toggle('$id');", t('Change to NO'), 'form-no');
  $b2 = button(' + ', "toggle('$id');", t('Change to YES'), 'form-yes');

  if ('never' != (string) $highlight) {
    list ($visHigh, $invisHigh) = u\order($highlight, ' highlight', '');
  } else $highlight = $visHigh = $invisHigh = '';

  list ($yesvis, $novis) = u\order($value, "visible$visHigh", "invisible$invisHigh");
  return <<<EOF
    <div class="$field-buttons$highlight">
    <div id="$id-YES" class="$yesvis form-yes">Yes $b1</div>
    <div id="$id-NO" class="$novis form-no">No &nbsp;$b2</div>
    $input
    </div>
EOF;
}

/**
 * @todo: focus on code input
 */
function formCells($form, &$sta) {
  $cuid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only deletion
  if (form_step($sta, $info) == 'prove') {
    $number = u\formatPhone($sta['storage']['values']['number']);
    $form = array(
      'title' => item(t('Verify')),
      'subtext' => item(tt('verify cell', compact('number'))),
      'code' => textField(t('Code:')),
      'number' => hidField($number),
      'verify' => submit(t('Verify')),
    );
    return $form;
  }

  $headers = array('Cell Number');

  $sql = "SELECT number FROM r_sms WHERE uid=:cuid ORDER BY number";
  $result = r\dbQ($sql, compact('cuid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(u\formatPhone($number));
  }

  $form = array(
    'title' => item(t('Cell Phones')),
    'subtitle' => item(t('Authorized devices for this account')),
    'cells' => formField('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no cells')), $list),
    'delete' => submit(t('Remove Selected Numbers')),

    'number' => textField(t('Add another cell phone:')),
    'add' => submit(t('Add')),
  );
  return $form;
}

function formCells_validate($form, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract($sta['values']);

  if ($op == 'Verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op != 'Add') { // $op == remove
    if (no_selection($cells)) {
      if ($number) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $sta);
  }

  if ($op == 'Add') {
    if (blank_field(compact('number'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($number, 'VALIDATE'), 'number');
  }
}

function formCells_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract($sta['values']);
  if ($op == 'Add') {
    if ($info = previous_state($sta)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($number, tt('verification code', compact('nonce')));
    if (isDEV) debug("nonce is $nonce"); // tell developer what code to type
    return form_step($sta, compact('number'), 'prove');
  }

  if ($op == 'Verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($number));
    $sta['redirect'] = 'account/cells';
  }
    
  if ($op == 'Remove') {
    foreach ($cells as $key => $number) if ($number) say(be\deleteCell(substr($number, 1))); // ignore the leading 'x'
  }
}

/**
 * Return the recipient of a recent invitation OR mark an invitation USED
 * @param string $code: the invitation code
 * @param int $invitee: the invitee's uid
 * Call by:
 *   [$email, $inviter] = invitation($code) 
 *     returns the email address to which the invitation was sent and the inviter, if sent recently (else NULL)
 *   $inviter = invitation($code, $invitee)
 *     marks the invitation USED by recording the invitee's account id
 */
function invitation($code, $invitee = '') {  
  if ($invitee) r\dbUpdate('r_invites', compact('code', 'invitee'), 'code');
  $lastweek = strtotime(R_INVITE_DAYS . ' days ago');
  $sql = 'code=:code AND invited>=:lastweek AND invitee IS NULL';
  return r\dbLookup($invitee ? 'inviter' : 'email,inviter', 'r_invites', $sql, compact('code', 'lastweek'));
}

/**
 * Modify the password reset form.
 */
function resetAlter(&$form, &$sta) {
  $form['title'] = formField('item', t('First Time Login'), '', weight(-99));
  $form['message']['#markup'] = str_replace('</p><p>' . t('Click on this button to log in to the site and change your password.'), t(' Choose a new password, then click on the button to log in.'), $form['message']['#markup']);
  $form['account']['pass'] = formField('password_confirm', '', t('Enter your new password in both fields.'));
  $form['actions']['submit'] = submit(t('Login')); // + weight(99));
  $form['#submit'][] = 'rCredits\\Web\\resetSubmit';
  $form['#attributes']['class'][] = 'rweb';
  focusOn('pass-pass1'); // note that password field gets expanded to two fields, of which pass1 is the first
  t\snapShot($form);
}

function resetSubmit($form, &$sta) {
  svar('resetPass', $sta['values']['pass']);
//  $uid = $sta['buid_info']['args'][0];
//  r\acct($uid)->update(compact('pass'));
}

/**
 * Modify the login form.
 * @see also loginFollowup, below (called from rweb_user_login()).
 * @param bool $regular: is this the full-page login form (default TRUE) -- otherwise the narrower login block
 */
function loginAlter(&$form, &$sta, $regular = TRUE) {
  $args = basename(t\EST() ? t\SERVER('REQUEST_URI') : $_SERVER['REQUEST_URI']);
  extract(u\just('name', $args));
  if ($regular) {
    $title = item(t('rCredits Login'));
    $subtext = item(t('Welcome! If you have not yet signed up for rCredits, click on "New account" above.'));
//    $isRCredits = rand(0, 1) ? formField('checkbox', t('Spoof check:'), u\SUBS(t('Does the address in your URL bar start with ":R_SITE_URL" (as usual)?')), required()) : hidField(TRUE);

    array_unshift($form, compact(u\ray('title subtext isRCredits')));
//    $form['tip'] = item(t('<b>NOTE: </b>If you created an account using cell texting request a "New password".'));
    $form['name']['#description'] = t('Enter your username (the letters of your legal name, all lowercase) OR your rCredits account ID.');
    $form['pass']['#description'] = t('Enter your account password.');
  }
  
  $form['external_input'] = hidField(serialize(@$sta['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $form['#validate'] = array('rCredits\\Web\\login_extra_validate', 'user_login_final_validate'); // add replacement function
  
  $form['name']['#title'] = t('Username/ID:');
  if (@$name) $form['name']['#default_value'] = $name;
  $form['pass']['#title'] = t('Password:');
  
  $form['#attributes']['class'][] = $regular ? 'rweb labeled' : 'rweb';
  focusOn('name');
  css('rweb.css', -9); // this normally gets called in showForm()
  js('inc/radiocheck.js', 'file', 'footer'); // this normally gets called in showForm()
  t\snapShot($form);
  loginCheck(); // must follow snapShot()
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account id, with password.
 * Sets $sta['uid'] to signify successful authentication.
 */
function login_extra_validate($form, &$sta) {
  extract(u\just('name pass isRCredits', $sta['values']));
  if (!$name or empty($pass)) return;

  $failed_login_ip_limit = variable_get('user_failed_login_ip_limit', 50);
  $failed_login_ip_window = variable_get('user_failed_login_ip_window', 3600);
  if (!flood_is_allowed('failed_login_attempt_ip', $failed_login_ip_limit, $failed_login_ip_window)) {
    $sta['flood_control_triggered'] = 'ip';
    return;
  }
  if (!$uid = r\loginString($name)) return; // not a user
  
  $failed_login_identifier_uid_only = variable_get('user_failed_login_identifier_uid_only', FALSE);
  $failed_login_user_limit = variable_get('user_failed_login_user_limit', 5);
  $failed_login_user_window = variable_get('user_failed_login_user_window', 21600);
  
  $identifier = $failed_login_identifier_uid_only ? $uid : $uid . '-' . ip_address();
  $sta['flood_control_user_identifier'] = $identifier;

  if (!flood_is_allowed('failed_login_attempt_user', $failed_login_user_limit, $failed_login_user_window, $identifier)) {
    $sta['flood_control_triggered'] = 'user';
    return;
  }

  // if (!$isRCredits) return say('bad spoof check', 'isRCredits');
  $cacct = r\acct($uid);
  $sta['uid'] = $cacct->passwordOkay($pass) ? $uid : FALSE;
}

function loginFollowup($edit, $account) {
  $resetPass = svar('resetPass'); // this gets unset by unset($_SESSION[$key]) below

  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($edit['external_input'])); // save params from external online merchant

  $name = @$edit['values']['name'];
  svar('cuid', $uid = $account->uid);
  if ($uid and $resetPass) {
    require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
    $pass = \user_hash_password(trim($resetPass));
    r\acct($uid)->update(compact('pass'));
  }
  setupGlobals();
  u\log('login', compact('name'), $uid);
  /*
  if ($resetting) { // 
    $token = drupal_hash_base64(drupal_random_bytes(55));
    $_SESSION['pass_reset_' . $user->uid] = $token;
    drupal_goto('account', array('query' => array('pass-reset-token' => $token)));
  } else 
  */
  r\go(''); // go to default page unless resetting password (then let 
}

function passAlter(&$form, $sta, $id = '') {
  $id = urldecode(basename($_SERVER['REQUEST_URI']));
  $title = item(t('Request New Password'));
  array_unshift($form, compact('title'));
  $form['name']['#title'] = t('Account:');
  $form['name']['#description'] = t('Username, Email, or Account ID');
  $form['name']['#default_value'] = u\neq($id, 'password', '');
  $form['#validate'] = array('rCredits\\Web\\pass_validate');
  $form['#attributes']['class'] = array('rweb', 'labeled');
  focusOn('name');
  t\snapShot($form);
}

function pass_validate($form, &$sta) {
  if (!$uid = r\loginString($sta['values']['name'])) return say('bad login', 'name');
  form_set_value(array('#parents' => array('account')), r\acct($uid)->account(), $sta);
  say('password reset');
}

/**
 * Automatically log back in and continue testing, if testing (offline only)
 */
function loginCheck() {
  global $channel;
u\log('top of loginCheck');
  if (t\EST()) return;
u\log('loginCheck not testing yet');
  if (!@cache_get('t_resume')->data) return;
u\log('loginCheck redirecting');
/*  global $user;
  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate();
  $user = (object) array(); // we need this empty for registration and login tests
  setupGlobals(1);
*/
  \drupal_goto('http://localhost/devcore/rcredits/test'); // must be drupal_goto not r\go
}

function formDirectory($form, &$sta) {
  $cuid = r\acct()->id;
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: substr(r\userField('postalCode', $cuid), 0, 3);
  
  $list = ($region !== FALSE or $which) ? directoryList($which, $region) : '';
  
  $form = array(
    'title' => item(t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => item(t('for a country name or postal code...')),
    'which' => textField(t('Search for:'), t('Type part of company name or industry category'), dft($which) + autocomplete('industry')),
    'region' => textField(t('Where:'), t('Type the first few characters of your postal code<br>Try 013, to see the list for Greenfield, MA.'), dft($region)), // or country name
    'submit' => submit(t('Go')),
    'item' => formField('item', '', '', $list),
  );

  return labeled($form);
}

function formDirectory_validate($form, &$sta) {
}

function formDirectory_submit($form, &$sta) {
  extract($sta['values']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

/**
 * Return a formatted, categorized list of businesses
 */
function directoryList($which, $region) {
  global $base_url;
  list ($symbolMember, $symbolRTrader, $symbolNonMember) = array('m', 'R', '-');
  $field = is_numeric($region) ? 'postalCode' : 'country';
  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $region = \db_like($region) . '%';
  $rows = array();
  $sql = <<<EOF
    SELECT DISTINCT u.uid, IFNULL(i.industry, '~Other') AS industry, u.name AS short_name, u.fullName, u.website 
    FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE (flags&(1<<B_COMPANY)) AND u.$field LIKE :region AND (u.fullName LIKE :which OR i.industry LIKE :which)
    ORDER BY industry, u.fullName
EOF;
  $result = r\dbQ($sql, compact(u\ray('region which')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]); // uid, industry, short_name, fullName, website
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $headed = TRUE; // always show the category (but keep the option to disable this choice)
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $cacct = r\acct();
    $acct = r\acct($uid);
    $pay = $cacct->can(B_BUY) ? button('pay', "$base_url/pay/who=$short_name") : '';
    $chg = $cacct->can(B_SELL) ? button('chg', "$base_url/charge/who=$short_name") : '';
    $fullName = "<a href='$base_url/member/$short_name'>$fullName</a>";
    $item = "$pay$chg $fullName";
    if (!$headed) $item = "- $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  $key = $list ? '' : t('There are no such businesses listed.');
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return "<ul id=\"key\">$key</ul>$list"; // ul so key gets indented like the rest
}

function formCompany($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $cuid = $cacct->id;
  $my_description = $cacct->description;
  $my_selling = $cacct->selling;
  
  $cats = r\dbQ('SELECT iid, industry FROM r_industries ORDER BY industry')->fetchAllKeyed();
  $my_cats = r\dbQ('SELECT iid FROM r_user_industries WHERE uid=:cuid', compact('cuid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$cacct->name" target="_blank">Show my public rCredits web page</a> (in a new window)
EOF;
  
  $form = array(
    'title' => item(t('Company Information')),
    'links' => item($links),
//    'subtext' => basic_account_info(),
    'name' => formField('item', t('Company name:'), '', $cacct->fullName),
    'categories' => formField('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($my_cats), $cats),
    'selling' => textField(t('Selling:'), t('What is the typical description of what you sell? (This will appear as the default "purpose" when you charge someone).'), dft($my_selling)),
    'website' => textField(t('Website(s):'), t('If you want to accept payments on your website, be sure to list it here.'), dft(@$website)),
    'description' => formField('text_format', t('Description:'), t('What does this company do?'), $filtered_html + dft($my_description)),
    'submit' => submit(),
  );

  return labeled($form);
}

function formCompany_validate($form, &$sta) {
//  extract($sta['values']);
}

function formCompany_submit($form, &$sta) {
  $cacct = r\acct();
  $id = $cacct->id;
  extract(u\just('description website selling categories', u\trimAll($sta['values'])));
//  $description = \check_markup($description['value']);
  $description = $description['value'];
  $info['website'] = str_replace('http://', '', @$website);
  $cacct->update(compact(u\ray('description website selling')));
  
  $DBTX = db_transaction();
  r\dbQ('DELETE FROM r_user_industries WHERE uid=:id', compact('id')); // out with the old
  foreach ($categories as $iid) {
    r\dbQ("INSERT INTO r_user_industries (uid, iid) VALUES (:id, :iid)", compact('id', 'iid')); // in with the new
  }
  unset($DBTX); // commit
  say('info saved');
}

/**
 * Show a page when someone scans an account ID QR, for example with QRDroid.
 * Cases (ideally):
 *   Using Account ID card as a business card -- show the company's rCredits webpage
 *   Curious or trying to use the app -- say a bit about rCredits and how to get the app and how to sign up / sign in.
 *   Charging / Paying with Webcam (signed in) -- show pic and name, and ask whether to charge or pay
 * @param string $who: some account's short qid (without the R_SERVER_ID prefix) REVERSED, followed by bin2hex of a R_CARDCODE_LEN-long rCard security code that must match the one stored in the secure field of the account record.
 */
function formI($form, &$sta, $who) {
  $m = strcspn($who, '.=');
  $code = hex2bin(substr($who, $m + 1));
  $who = R_SERVER_ID . strrev(substr($who, 0, $m + 1));
  if (!$acct = r\acct($who) or $acct->secure['card_code'] != $code) return r\go(''); // not a real code -- just go to home page

  list ($myId, $myAgent) = ($cacct = r\acct()) ? array($cacct->id, $cacct->agentId) : array(FALSE, FALSE);
  list ($otherId, $otherAgent) = array($acct->id, $acct->agentId);
  if ($otherId == $myId) {
    say('no self-trading', array(), 'error');
    return r\go('');
  }
  
  $shortName = $acct->name;
  if ($acct->can(B_COMPANY)) $site = goButton(t('Show Webpage'), "member/$shortName", t("See this company's rCredits webpage"));

  if ($cacct) { // can charge, pay, or change account or agent only if signed in
    $theId = hidField($otherId);
    $theAgent = hidField($otherAgent);
    if ($cacct->can(B_SELL)) $charge = goButton(t('Charge'), "charge/who=$shortName&scanned=1", t('Charge someone'));
    if ($cacct->can(B_BUY)) $pay = goButton(t('Pay'), "pay/who=$shortName&scanned=1", t('Pay someone'));
    if (r\acct($myId, $otherAgent)->can()) $changeAgent = goButton(t('Change Agent'), "user/logout/who=$shortName", t('Change Shift?'));
    if (r\acct($otherId, $myAgent)->can()) $changeAccount = submit(t('Change Account'));
    $zot = compact(u\ray('charge pay changeAgent changeAccount site'));
    $all = identifiers($acct) . render($zot);
    svar('lastCustomer', $acct);
  } else {
    $login = item(t('Please login, to Pay or Charge this account.'));
    $zot = compact('login', 'site');
    $all = render($zot);
  }

  $all = formField('item', '', '', $all);
  return compact('all');
}

function formI_submit($form, &$sta) {
  extract(u\just('theId theAgent', $sta['values']));
  $acct = r\acct($theId, $theAgent);
  $op = op($sta);
  if ($op == 'changeAccount') {
    $cacct = r\acct();
    if (!$cacct or !r\acct($theId, $cacct->id)->can()) return r\go('user/logout'); // hacker. so leave.
    svar('cuid', $theId);
    $accountName = $acct->fullName;
    say('changed account', compact('accountName'));
  }
}

function formSMS($form, &$sta) {
  $title = item(t('Test SMS'));
  $my_number = svar('lastSMS');
  $number = textField(t('From (number): '), '', required($my_number));
  $message = textField(t('Message: '));
  $submit = submit(t('Send it!'));

  focusOn('message');
  $form = compact(u\ray('title number message submit'));
  return labeled($form);  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(u\just('number message', $sta['values']));
  $number = u\formatPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

function formOptions($form, &$sta) {
  $cacct = r\acct();
  $title = item(t('Account Preferences'));
  $virtualMaxMessage = $cacct->can(B_COMPANY) ? t(' If you make automatic "virtual" payments, your maximum is the same as your minimum, regardless of this setting.') : '';
  $minimum = textField(t('Minimum:'), t('Keep AT LEAST this much in your rCredits account, for purchases or inflation-proof savings. When your balance goes below this amount, the system refills it for you automatically from your bank account (which may take 3 days), then trades US Dollars for rCredits whenever possible, up to this amount.'), required(u\formatAmount(u\nn($cacct->minimum, R_MINR_DEFAULT), 's')));
  $maximum = textField(t('Maximum:'), u\SUBS(t('Keep AT MOST this much in your rCredits account. When your balance goes above this amount by $:R_RBUY_QUANTUM or more, the extra rCredits are automatically exchanged for US Dollars. 0 means no limit (recommended in most cases).')) . $virtualMaxMessage, required(u\formatAmount(u\nn($cacct->maximum, R_MAXR_DEFAULT), 's')));
  $toBank = formField('checkbox', t('To bank:'), t('Automatically transfer the excess to your bank account (recommended). [<a href="../help/to-bank">why ever not?</a>]'), $cacct->hasBit(B_TO_BANK));
//  $max_usd = textField(t('USD Max:'), u\SUBS(t('Keep at most this much in your Dwolla account. When your Dwolla balance goes above this amount by $:R_RBUY_QUANTUM or more, the system will automatically initiate a transfer to your bank account. 0 means no limit.'), required(u\formatAmount(u\nn($cacct->max_usd, R_MAXUSD_DEFAULT), 's'))));
  $notifyBys = array(t('no notifications'), t('by email'), t('by SMS'), t('by both email and SMS'));
  $dft = $cacct->hasBit(B_NOTIFY_EMAIL) + 2 * $cacct->hasBit(B_NOTIFY_SMS);
  $notifyBy = formField('radios', t('Notifications:'), t('How do you want to be notified about new charges to your account, short-term opportunities, and the like?'), required($dft), $notifyBys);
  $statements = $cacct->hasBit(B_ELECTRONIC) + 0;
  $statementRadios = array(
    t('I will accept electronic statements.'), 
    t('I choose to receive printed statements by mail.') . u\SUBS(t('<div>I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>'))
  );
  $statements = formField('radios', t('Statements:'), '', required($statements), $statementRadios);

  if ($cacct->can(B_COMPANY)) $payVirtual = formField('checkbox', t('Pay virtually:'), t('Do you want to pay people directly in rCredits or do you want to pay in US Dollars, letting them automatically <i>buy</i> their share of rCredits from you? The second way lets you use your current payroll system without any change. You still get the 5% rebate, minus 25 cents for each payment. For example, a $100 payment gives you a net rebate of $5.00 - 0.25 = $4.75. For automatic "virtual" payments, you need to set up average monthly payment amounts on the <a href="account/relations">Relations</a> page.'), $cacct->hasBit(B_SECRET_BAL)isset($cacct->minimum));
  $secretBal = formField('checkbox', t('Secret Balance:'), t("Don't let qualified merchants tell me my balance, when I ask."), $cacct->hasBit(B_SECRET_BAL));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $submit = submit(t('Update options'));
  
  $form = compact(u\ray('title minimum maximum toBank payVirtual notifyBy statements secretBal submit'));
  return labeled($form);
}

function formOptions_validate($form, $sta) {
  $cacct = r\acct();
  $floor = u\formatAmount($cacct->floor);
  extract(u\just('minimum maximum', $sta['values']));
  if ($minimum < $cacct->floor) say('bad minr', compact('floor'), 'minimum');
  if ($maximum != 0 and ($maximum < min($minimum, $cacct->rewards))) say('bad maxr', 'maximum');
//  if ($max_usd != 0 and $max_usd < $minimum) say('bad maxusd', 'max_usd');
}

function formOptions_submit($form, $sta) {
  $cacct = r\acct();
  extract(u\just('minimum maximum notifyBy', $sta['values']));
  extract(u\just('toBank payVirtual secretBal', $sta['input'])); // damn Drupal sets these (=1) iff true, but not in values.

  $cacct->setBit(B_TO_BANK, @$toBank); 
  $cacct->setBit(B_VIRTUAL, @$payVirtual);
  $cacct->setBit(B_NOTIFY_EMAIL, $notifyBy & 1);
  $cacct->setBit(B_NOTIFY_SMS, $notifyBy & 2);
  $cacct->setBit(B_ELECTRONIC, @$statements);
  $cacct->setBit(B_SECRET_BAL, @$secretBal);
  $cacct->update(compact(u\ray('minimum maximum')));
  say('options saved');
}

function formReports($form, &$sta, $args = '') {
  $cacct = r\acct();
  $ctty = r\acct($cacct->community);
  $totals = r\totals($ctty->id);
  $issued = -$totals->balance;
  $demand = $totals->demand;
  $demandPct = $issued ? (' (' . number_format(100 * $demand / $issued, 1) . '% beyond amount issued)') : '';
   
  $title = item(t('Reports'));
  $subtext = item(t('for ') . $ctty->fullName);
  $issued = item(u\formatAmount($issued), t('rCredits issued:'), t('How many rCredits have been issued')); // ($xxx worldwide)
  $demand = item(u\formatAmount($demand) . $demandPct, t('Margin:'), t('Community demand for additional rCredits')); // ($xxx worldwide)
//  $moreLater = item('', t('After rCredits is launched, this page will also report progress statistics.'));
  if (strpos($args, 'variety=') === FALSE) $args .= '&variety=report';
  $form = compact(u\ray('title subtext issued demand')) + formTxs('', $sta, $args);
  return labeled($form);
}

function formReports_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formReports_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'reports');
}

/**
 * Print an ID card
 */
function memberID($uid = '', $big = FALSE) {
  require_once(__DIR__ . "/../../tcpdf/config/lang/eng.php");
  require_once(__DIR__ . "/../../tcpdf/tcpdf.php");
  define('PHOTO_MAX', 40); // maximum height and width of photo

  $cacct = r\acct();
  $acct = $uid ? r\acct($cacct->id, $uid) 
    : (($cacct->can(B_CTTY_ADMIN) and $cacct->can(B_MANAGE)) ? r\acct($cacct->id) : $cacct);
  if (!$acct) return 'No such account. Your hack attempt has been logged.';
  
  $fullName = $acct->fullName;
  $regionName = r\acct($acct->community)->fullName;
  if (!$acct->proSe()) {
    $agent_name = r\acct($acct->agentId)->fullName;
    $fullName = "$agent_name<br>$fullName";
    $nameSize = '36px';
  } else $nameSize = '60px';
  $qid = str_replace(':', '=', $acct->qid()); // "=" to avoid GoDaddy's 'Bad Request' death redirecting RC2.ME with a ":" qid
  $qid = 'NEW.BHO';
  $qid2 = strpbrk($qid, '.=');
  $qrURL = 'http://' . R_SERVER_ID . '.rC2.me/I/' . strrev($qid2) . bin2hex($acct->secure['card_code']);
  debug($qrURL);
  if(!$photo = profile_picture($acct->agentId)) $photo = __DIR__ . '/../images/no-photo-available.gif';

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
    'module_height' => 1 // height of a single module in points
  );

  // Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=array())
  $pdf->Image(__DIR__ . '/../images/idcard/frame.jpg', 5, 5, 101, 64, '', '', '', true, 150, '', false, false, 0, false, false, false);
  $pdf->Image($photo, 10, 10, $photoW, $photoH, '', './account', '', true, 150, '', false, false, 0, false, false, false);
  $pdf->write2DBarcode("HTTP://RC2.ME/I/$qid", 'QRCODE,Q', 53, 25, 25, 25, $style, 'N'); // L,M,Q,H are low-high error-correction
  $pdf->Image(__DIR__ . '/../images/rlogo150.png', 85, 25, 14.93, 20, '', '', '', true, 150, '', false, false, 0, false, false, false);

  $html = strtr('<span style="font-size:22px;">Member<br><span style="font-size:38px; color:darkred;">@CODE</span><br>@REGION</span>', array('@CODE' => $qid, '@REGION' => $regionName));
  $pdf->writeHTMLCell(50, 15, 52, 10, $html); // w, h, x, y
  $pdf->writeHTMLCell(20, 5, 85, 47, '<div style="font-size:22px;">rCredits.org</div>');
  $pdf->writeHTMLCell(91, 20, 10, 55, "<div style=\"font-size:$nameSize; text-align:center; color:midnightblue; font-weight:bold;\">$fullName</div>");
  
$instructions = <<<EOF
<style>
  big {color:darkgreen; font-size:2em;}
  p {margin-top:.5em;}
</style>
Here is your<br><b style="color:darkgreen; font-size:1.3em;">Advance ID Card</b>
<p>You may print this card and use it temporarily, along with a second photo ID. To print, right-click here, then click on "Print" OR use your browser's print icon.</p>
<p>To upload or change your photo, visit the "Settings" section.</p>
EOF;
 
  $pdf->writeHTMLCell(90, 64, 110, 5, $instructions);
  // Add temporary symbol over image (maybe remove lower right corner and add "temp"), to discourage unauthorized use
//  $pdf->Image(__DIR__ . '/../images/icons/print.png', 120, 30, 32, 32, '', '', '', true, 150, '', false, false, 0, false, false, false);
  
  $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

/**
 * Return markup to identify the agent of the given account.
 */
function identifiers($acct) {
  $cacct = r\acct();
  list ($agent, $company) = $acct->proSe() ? array($acct, '') : array(r\acct($acct->agentId), $acct->fullName);
  $location = r\location($acct);
  $pic = ($cacct and $cacct->can(B_CHARGE)) ? profile_picture($agent->id, TRUE) : ''; // let qualified companies see the picture
  return <<<EOF
    $pic
    <div id="id-other">
      <div id="id-name">$agent->fullName</div>
      <div id="id-company">$company</div>
      <div id="id-location">$location</div>
    </div id-other>
EOF;
}

/**
 * Set or report next step in form's workflow
 * Syntax:
 *   form_step($sta, $info, 'id of next step') -- sets the next step
 *   form_step($sta, $zot, NULL) -- sets the next step to none
 *   form_step($sta, $info) -- gets the name of the next step and (in $info) any parameters
 * @param string $next_step: a step id ('' means ignore argument, NULL means no next step)
 * @param array $info: associative array of parameters to next step (passed or returned)
 * @return string: the next step ('' if none)
 * @see also step_one() and previous_state()
 */
function form_step(&$sta, &$info = NULL, $next_step = '') {
  if (is_null($next_step)) { // set the next step (and args) to none
    $sta['storage'] = NULL;
    $sta['rebuild'] = TRUE;
  } elseif ($next_step) { // set the next step (and args)
    $sta['storage']['previous'][$next_step] = $sta;
    $sta['storage']['step'] = $next_step;
    $sta['storage']['values'] = $info;
    $sta['rebuild'] = TRUE;
  } else $info = @$sta['storage']['values']; // return args
  
  return @$sta['storage']['step']; // return the next step
}

function step_one(&$sta) {form_step($sta, $zot, NULL);}

function previous_state(&$sta, $message = '', $args = array()) {
  if ($message) {
    $sta = $sta['storage']['previous'][$sta['storage']['step']];
    $sta['storage']['say'] = array($message, $args);
  } else return @$sta['storage']['say'] ?: '';
}

/**
 * Asks the user to confirm, calls the caller with the answer.

function ok_form($form, &$sta) {
  //extract($sta['input'], EXTR_PREFIX_ALL, 'in');
  extract(form_args('ok'), EXTR_PREFIX_ALL, 'in');
  $form = array(
    'title' => item(@$in_title),
    'question' => item(@$in_question),
    'confirm' => formField('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => formField('submit', NULL, t("Don't do it"), 'Cancel'),

    'detour_to' => hidField(@$in_detour_to),
    'return_with' => hidField(@$in_return_with),

    '#skip_duplicate_check' => TRUE, // Confirm form fails duplication check, as the form values rarely change -- so skip it.
    '#attributes' => array('class' => u\ray('rweb confirmation')),
  );

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

function ok_form_submit($form, &$sta) {
  extract($sta['values']);
  $ok = ($op == 'Okay');
  return_from_detour($detour_to, compact('ok', 'return_with'));
}

function get_ok($question = '', $title = '', $args = FALSE) {detour('ok', compact(u\ray('question title args')));}

function got_ok() {}
*/
/*
function detour($detour_via, $args = array(), $detour_to = '') {
  if (!$detour_to) $detour_to = current_path();
  if ($detour_to != 'none') $args += compact('detour_to');
  form_args($detour_via, $args);
  drupal_goto($detour_via);
}

function return_from_detour($detour_to, $args = '') {if ($detour_to) detour($detour_to, $args, 'none');}

function form_args($function, $args = '') {
  if (!$args) return svar('form_args']['function'] == $function ? svar('form_args'] : array();
  svar('form_args'] = $args + compact('function');
}
*/

/**
 * Replacement for \confirm_form
 */
function sureForm(&$sta, $title = '') {
  if (!isset($sta['confirm'])) return FALSE;
  
  $form = array(
    'title' => $title ? item($title) : NULL,
    'question' => item($sta['confirm']),
    'confirm' => submit(t('Okay')),
    'cancel' => submit(t('Cancel')),
  );
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => u\ray('rweb confirmation'));
//  $form['#validate'] = 'rCredits\\Web\\sureForm_validate';
//  $form['#submit'] = 'rCredits\\Web\\sureForm_submit';
//  $sta['rebuild'] = TRUE;

  focusOn('confirm');

  return $form;
}

/*
function detour($detour_via, $detour_args, $whence = '', $modal = TRUE) {
  svar('detour_via', $detour_via);
  svar('detour_args', $args);
  svar('whence', $whence ?: current_path());
//  drupal_goto($modal ? $whence : $detour_via); // modal forms pop up over the reloaded current form
  drupal_goto($detour_via); // no modal forms yet
}
*/

/**
 * Figure out who the user means, offering choices to disambiguate
 * If the intended person or company does not exist, 
 * @param string $who: what the user typed
 * @param string $field_name: name of the field the user typed in
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param boolean $allow_ALL: all $who to be "all" (meaning all employees) (default FALSE)
 * @param string $selfMsg: index to error message to give if user self-refers
 * UNUSED @param boolean $create: whether to create the person/company if it doesn't exist
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $allow_ALL = FALSE, $selfMsg = 'no self-trading') {
  global $base_url;
  $result = be\identify($who, $allow_ALL, $selfMsg);
  if ($result == 'ALL' or u\isAcct($result)) return $result;

  list ($msg, $args, $choices) = $result;
  foreach (u\ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = urlencode(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
  $args['draftLink'] = "$base_url/draft/" . http_build_query($info);
  foreach ($choices as $uid => $fullName) $choices[$uid] = substr(r\quid($uid), 3) . '   ' . $fullName;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, tt($msg, $args));
}
  
/**
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($info, $confirmed) {
  global $lastXid, $scanned;
  $cuid = r\acct()->id;
  extract(u\just('op who amount goods purpose scanned', $info)); // $scanned is global, for be\transfer
  $txType = $op == 'Pay' ? 'payment' : strtolower($op);
  if (blank_field(compact(u\ray('who amount')))) return FALSE;
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if (!$goods) $purpose = 'cash'; // tell be\transfer it's not for goods (should be the only place this string occurs)
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
  if (!$acct = whois($who, 'who', $info, $txType == 'payment')) return FALSE; // (allow "ALL" for payment)
  svar('lastCustomer', $acct);
  list ($msg, $args, $confirm) = in_array($txType, array('payment', 'charge')) ?
       be\transfer($txType, $acct, $amount, $purpose, !u\forCash($purpose), $confirmed)
     : communityTransfer($txType, $acct, $amount, $purpose, $confirmed);

  if (!@$scanned and strtoupper($who) != 'ALL') { // not scanned -- We need to show identifying photo, name, etc. for the target account
    $identifiers = identifiers($acct);
    $identifiers = "<div id='identifiers' class='clearfix'>$identifiers</div identifiers>";
  }
  if ($confirm) return @$identifiers . tt('confirm ' . $msg, $args);
  if (!@$args['success']) return say($msg, $args, 'amount');

  svar('lastXid', $lastXid); // remember identity of most recent transaction
  return say($msg, $args);
}

/**
 * Record a transfer to or from the community.
 * @param int $type: what type of transfer
 * @param acct $acct: the account to credit or debit
 * @param int $amount: how much to transfer
 * @param string @purpose: what's the transfer for
 * @param bool $confirmed: has this transaction been confirmed by the user?
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 */
function communityTransfer($type, $acct, $amount, $purpose, $confirmed) {
  $success = (bool) be\fund($acct->id, constant(strtoupper("TX_$type")), $amount, $purpose, $confirmed);
  $msg = $success ? ($confirmed ? 'report funding' : 'funding') : 'funding error';
  $confirmAction = ucwords($type);
  $action = $confirmAction . 'ed';
  $otherName = r\userField('fullName', $acct->id);
  $amount = u\formatAmount($amount);
  return array($msg, compact(u\ray('success action confirmAction amount otherName')), $success and !$confirmed);
}  

function usdCallback($type, $uid) {
  include_once __DIR__ . '/../usd.class';  
  return r\usd::$type($uid);
}
  
function menu($title, $type, $function = NULL, $args = array(), $accesses = 0, $other = array()) {
  global $menuWeight;
  $menuWeight += !@$menuWeight ?: 1;
  $access = 'a'; // 'a' because Drupal doesn't like numbers
  if (!is_array($accesses)) $accesses = array($accesses);
  foreach ($accesses as $one) $access .= " $one";
   
  if (function_exists($localFunction = "rCredits\\Web\\$function")) $function = $localFunction;
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $args,
//    'access callback' => TRUE,
    'access callback' => 'rCredits\\Web\\webAccess', // don't use be\access directly (rweb needs to remember $cuid)
    'access arguments' => array($access),
    'weight' => @$menuWeight - 1,
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  ), $other); // $other overwrites, if there is a conflict
}

/**
 * Say whether user has submitted the confirmation form (as opposed to the primary form)
 * The "v" verion is called from _validate, the "s" version from _submit.
 */
function confirming_s(&$sta) {return ($sta['rebuild'] = isset($sta['confirm']));}
function confirming_v(&$sta) {
  if(@$sta['confirm']) {
    extract($sta['values']);
    $sta = $sta['submitted_state'];
    if ($op == 'Cancel') {
      $sta['rebuild'] = TRUE;
      say('op canceled'); // not an error message, else confirmation form persists
    }
    return TRUE;
  }
  $sta['submitted_state'] = $sta;
  return FALSE;
}

function confirm($msg, &$sta) {
  t\testOutput(preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($msg)), 'screen');
  return ($sta['confirm'] = $msg);
}

/**
 * Complain about an input error if a required field is blank
 * @param array $fields: associative array of field names, with or without a prefix
 * @param string $prefix: option field name prefix (add if missing, else remove from message)
 */
function blank_field($fields, $prefix = '') {
  foreach ($fields as $key => $value) {
    $prefixed = u\abbreviates($prefix, $key);
    $field = strtoupper($prefixed ? substr($key, strlen($prefix)) : $key);
    $actual_name = $prefixed ? $key : ($prefix . $key);
    if (trim($value) == '') {say('required field', compact('field'), $actual_name); return TRUE;}
  }
  return FALSE;
}

/**
 * Display a drupal message (error or not)
 * Possible syntaxes:
 *   say(NULL) [do nothing in this case]
 *   say('index', array(optional args), 'optional error field')
 *   say('index', 'error field')
 *   say(array(index, args), 'optional error field')
 * @return FALSE (transfer() depends on this)
 */
function say($index, $args = array(), $error_field = '') {
  if (!@$index) return;
//  if (is_array($index) and !isset($index[0])) debug($index);
  if (is_array($index)) list ($index, $args, $error_field) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $error_field) = array($error_field, $args); // allow either order, for easy 2-param calls
  $message = tt($index, $args);
  if ($error_field) \form_set_error($error_field, $message); else \drupal_set_message($message);
  u\log($error_field ? 'say error' : 'say', compact('message'));
  return FALSE;
}

function userMenuItem($router_path, $link_title) {
  $menu_name = 'user-menu';
  $link_path = $router_path;
  if (r\dbExists('{menu_links}', 'menu_name=:menu_name AND link_path=:link_path', compact('menu_name', 'link_path'))) return; // done
  $module = 'menu';
  $customized = TRUE;
  $depth = 1;
  $mlid = 0;
  $item = compact(u\ray('mlid menu_name router_path link_path link_title module customized depth'));
  menu_link_save($item);
}

function focusOn($focusFieldName) {
  u\EXPECT(!empty($focusFieldName), 'focus on null');
  $focusFieldName = str_replace('_', '-', $focusFieldName);
  js("document.getElementById('edit-$focusFieldName').focus();", 'inline', 'footer');
}

function formFooter() {
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $markup =<<<EOF
<hr>
<a href="http://rCredits.org">About rCredits</a> |
<a href="help">Help</a>
<br><br>
website copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance&trade;</a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:info@rCredits.org">info@rCredits.org</a><br>
EOF;
  $footerText = item($markup); // unique name for this page
$js =<<<EOF
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
EOF;
  js($js, 'inline', 'footer');
  return compact('footerText');
}

function webAccess($accesses) {
  setupGlobals(); // remember who we are (the main reason for this function)
  if (!$cacct = r\acct()) return FALSE;
  $accesses = u\ray(substr($accesses, 2)); // turn this from a string back into an array
  foreach ($accesses as $access) if (!$cacct->can($access)) return FALSE;
  return TRUE;
}

function css($sheet, $weight = -99) {
  drupal_add_css(\drupal_get_path('module', 'rweb') . "/$sheet", array('group' => CSS_THEME) + weight($weight));
}

function js($what, $type, $scope) {
  global $base_url;
  if ($type == 'file') $what = "$base_url/sites/all/modules/rcredits/$what";
  return drupal_add_js($what, compact('type', 'scope'));
}

function labeled($form) {
  $form['#attributes']['class'][] = 'labeled';
  foreach ($form as $key => $value) if (substr($key, 0, 1) != '#') {
//    if (!isset($value['#id'])) $form[$key]['#id'] = "edit-$key"; // is this necessary?
  }
  return $form;
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs);}
//function block_def($info, $extra = array()) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function blockView($subject, $content) {return compact(u\ray('subject content'));}

function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($default_value) {return compact('default_value');}
function weight($weight) {return compact('weight');}
function attrib($attribs) {return array('attributes' => $attribs);}
function autocomplete($type = '') {return array('autocomplete_path' => "autocomplete/$type/" . @r\acct()->id);}
function required($dft = NULL) {return array('required' => TRUE) + (isset($dft) ? dft($dft) : array());}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {return $sta['clicked_button']['#parents'][0];}
