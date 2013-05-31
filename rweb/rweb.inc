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
  'your account is ready' => t('<p></p><p>Welcome to rCredits! We sent you an email with your user ID and password (if you don\'t see it within 60 seconds, check your spam folder). Come back here <b style="color:red;">within the next @R_SIGNIN_HOURS hours</b> to sign in and finish your account setup. Then if you have another account to create, use the "Create Another Account" button on the Summary page.</p>'),
  'company is ready' => t(<<<EOF
    <p></p><p>To finish setting up this company account:</p>
    <ol>
      <li>Sign in to your personal account</li>
      <li>Choose your company account from the account-selection dropdown box in the upper right corner of the screen.</li>
    </ol>
EOF
  ),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  'options saved' => t('Your options have been saved'),
  'photo saved' => t('Your photo was successfully uploaded.'),
  'step completed' => t('<span id="completed">Step completed!</span>'),
  'return to membership' => t('<a href="@link">Click here to return to the checklist</a>.'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('A temporary password has been sent to your email address.'),
  'changed agent' => t('You successfully changed the agent to @agentName.'),
  'changed account' => t('You successfully changed the account to @accountName.'),
  'signed agreement' => t('You signed the rCredits Agreement on @date.'),
  'downloading' => t('Your download will begin shortly...'),
  'changed account' => t('You are now managing the account for @newAcct.'),
  'lost old changes' => t('Any unsaved changes to account @oldAcct have been discarded.'),
  'take a step' => t('Your account is <b style="color:red;">not yet activated</b>. <a href="membership"><span id="nextstep">Take the next step!</span> Click here</a> to see where you are in your membership progress and what to do next.'),
  'got token' => t('Your rCredits and US Dollar accounts have been successfully connected!'),
  'company rcard' => t('Send a <a href="@href">company rCard</a> to @name at @address.'),
  'amount rounded' => t('"@amount" was rounded to the nearest integer value.'),
  'invite sent' => t('Your invitation has been sent.'),
  'repeat invite' => t('Come back any time, to send more invitations (click on "Invite Someone" at the bottom of any page).'),
  'gift transfer later' => t('Your funds will be transferred when you have money in your account.'),
  'repeat gift' => t('If you wish to make a contribution again sometime, click on "Membership" in the upper right corner, then on "Contribute".'),
  'setup complete' => t('Your account setup is complete. Expect an email within the next 24 hours, saying your account is activated. Thank you for joining!'),

  // prompts
  'prompt min' => t('Keep this much in my rCredits account, for purchases or inflation-proof savings. (When your balance goes below this amount, the system refills it for you from your bank account. Refilling may take 3-4 days &mdash; 10 days for Savings and Money Market accounts. Choose 0 if you prefer to refill it yourself.)'),
  'prompt max' => t('Keep AT MOST this much in my rCredits account. (When your rCredits balance goes above this amount by $@R_CHUNK or more, the system transfers the extra to your bank account. 0 means no automatic transfers &mdash; recommended in most cases).'),
  
  // confirm
  'yes or no' =>  '@yesNo', // to replace with yesNo(url)
  'confirm undo' => t('Undo @created transfer of @amount @tofrom @otherName?'),

  // errors
  'missing purpose' => t('<p>What purpose? For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc". Rebates and bonuses are intended as rewards for productive economic activity in rCredits.</p>
    <p>For everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no cells' => t('This account has no related cell phones.'),
  'required field' => t('@field must not be blank.'),
  'duplicate email' => t('"@duplicateAccount" is already using that email address. If you want to use the same email address for this account, add a tag to the local part, for example "@emailTagged".'), // used in user.module
//  'duplicate email' => t('That email address is already taken by @duplicateAccount.'),
  'forgot password' => t('If you just forgot the password to the other account, <a href="@passwordLink">click here</a> to request a new password.'), // used in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad login' => t('That account does not exist. Check your typing?'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad account id' => t('That is not a proper account ID.'),
  'bad account number' => t('Your Dwolla id must be exactly 10 digits'),
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
//  'unknown member' => t('"@who" has not yet signed up for rCredits. <a href="@draftLink">Click here to add them</a> to the list or <a href="" id="which-cancel">click here to cancel</a>.'),
  'unknown member' => t('"@who" has not yet signed up for rCredits.'),
//  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click on the one you want or, if you don\'t see them listed, <a href="@draftLink">Click here to add them</a> to the list or <a id="which-cancel">here to cancel</a>.'),
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click on the one you want or <a id="which-cancel">here to cancel</a>.'),
//  'similar found' => t('If you mean @otherName (@phone), <a href="@draftLink">click here for YES</a>. Otherwise, <a href="@cancelLink">click here to cancel</a>.'),
  'get app' => t('To scan Account ID cards with a smartphone, you need the app! You can download the Android app <a href="http://rCredits.org/rc.apk">here</a>. The iphone version is in the works (check again in January).'),
  'no help' => t('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at @R_ORG_PHONE. Or send your question by <a href="mailto:@R_ORG_EMAIL?subject=help with rCredits Members Site, @page page" target="_blank">email</a>.'),
  'no account permission' => t('You do not have permission to use that account.'),
  'bad transient id' => t('That QR code is out of date. Try again?'),
  'bad email' => t('The e-mail address <em class="placeholder">@email</em> is not valid.'), // must match Drupal, for tests
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner, employee, or contractor for the company? (please choose one or more)'),
  'shortr to' => t('You do not have enough rCredits, for that transaction. <a href="@how">Go here</a> to get more rCredits.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("@fullName").'),
  'bad invite' => t('To sign up for rCredits, you must be invited and you must use the invitation link (just once) within @R_INVITE_DAYS days.<br><br>If you already have an rCredits account set up and want to open an additional account for yourself, your business, or a family member, sign in to your personal account and use the "Open Another Account" on the Summary page.'),
  'bad minr' => t('Your chosen rCredits minimum cannot be less than your assigned minimum balance (@floor).'),
  'bad maxr' => t('Your chosen rCredits maximum cannot be less than your rCredits minimum. Nor can it be less than your total amount of incentive rewards to date.'),
//  'bad maxusd' => t('Your chosen USD maximum cannot be less than your rCredits minimum.'),
  'info not saved' => t('We apologize. Your information could not be saved at this time. Please try again in an hour or so.'),
  'no page permission' => t('You do not have permission to visit this page.'),
  'specific nonpositive' => t('The amount (@amount) must be a positive integer.'),
  'insufficient relation' => t('You cannot open an account unless you have a closer relationship to it. Use the <a href="invite">invitation page</a> instead.'),
//  'humans sign' => t('You must be logged in to a personal account, to sign the agreement on behalf of a company.'),
  'bad spoof check' => t('You must confirm (<i>every time</i>) that you are on the real rCredits site.'),
  'no auth yet' => t('You have not yet authorized a connection with your Dwolla account. Please resubmit.'),
  'not an image' => t('That is not an acceptable image format.'),
  'bad commercial aspect' => t('Your company profile picture must be in landscape orientation, with a width-to-height ratio of about @R_COMMERCIAL_ASPECT.'),
  'bad personal aspect' => t('Your profile picture must be in portrait orientation, with a width-to-height ratio of about @R_PERSONAL_ASPECT.'),
  'need a manager' => t('A company account must have at least one person to manage it.'),
  'self must sign' => t('You cannot sign this agreement on behalf of another person.'),
  'bad zip' => t('US zipcodes must be 5 digits, possibly followed by a hyphen and 4 more digits.'),
  'password required' => t('You must supply your current password, to change your email address or password.'),
  'wrong pass' => t('That is not your correct password.'),
  'multiple spaces' => t('Field @field cannot have more than one space in a row.'),
  'illegal char' => t('Field @field contains an illegal character. Try retyping it'),
  'too long' => t('Field @field is too long: it must be @USERNAME_MAX_LENGTH characters or less.'),
  'weak pass' => t('Your password must be a "Strong" one (or better).'),
  'photo upload failed' => t('Your photo upload was not successful.'),
  'pass expired' => t('Your temporary password has expired.'),
  'inhuman proxy' => t('@name is a company. Your proxies must be people.'),
  'dw mismatch' => t('WARNING: The "@field" of your Dwolla account and your rCredits account are conflicting. Your account permissions will be restricted until you correct this mismatch.'),
  'proxy to go' => t('You have not yet completed your selection of proxies.'),
  'no stop draw' => t('The account for @fullName has overspent its rCredits, based on your balance. So you cannot discontinue "draw" permission without first bringing that account back up to even (for example by giving it some funds.'),
  'doubled proxy' => t('Your alternate proxy cannot be the same person as your first proxy.'),
  'po in location' => t('Your physical location address cannot be a Post Office box.'),
  'missing currency' => t('You must choose rCredits, USD, or both.'),
  'download always both' => t('Note: Downloads always include both rCredits and USD amounts.'),
  'signin to scan' => t('You must sign in BEFORE scanning a card.'),
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
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = key($options); // Drupal's select fails without a default
  $title_display = 'before';
//  if (isset($other['value']) and !isset($other['default_value'])) $other['default_value'] = $other['value'];
  $field = u\prefixKeys('#', array_merge(compact(u\ray('type title title_display description options')), $other));
  return $field;
}

function submit($title = '') {return formField('submit', '', '', $title ?: t('Save'));}
function item($value, $title = '', $desc = '') {return formField('item', $title, $desc, $value);}
function hidField($value = '', $options = array()) {return formField('hidden', '', '', compact('value') + $options);}
function textField($title = '', $description = '', $other = array(), $options = NULL) {
  return formField('textfield', $title, $description, $other, $options);
}

/**
 * Return a boolean (Yes / No) field.
 * @param string $title: field label
 * @param string $description: helpful description of field
 * @param mixed $dft: the default value (TRUE or FALSE) or an assoc of attributes
 */
function boolField($title = '', $description = '', $dft = NULL) {
  $class = array('yesno');
  return formField('radios', $title, $description, (is_array($dft) ? $dft : required(@$dft ?: 0)) + attrib(compact('class')), array(t('No'), t('Yes')));
}

/*
 * Create a rendered checkboxes field
 * Drupal is inconsistent about checkboxes formatting, making our scripts fail.
 * Specifically, when the checkbox inputs have a unique id, radiocheck.js fails to set the value successfully
 * (at least it never appears in $_POST). So we do it here by hand.
 */
function boxField($tag, $title, $desc, $defaults, $choices) {
  $boxes = '';
  foreach ($choices as $key => $value) {
    $checked = in_array($value, $defaults) ? ' CHECKED' : '';
    $boxes .= <<<EOF
    <div class="form-item form-type-checkbox form-item-$tag-$key">
       <input type="checkbox" name="{$tag}[$key]" value="$key"$checked class="form-checkbox" />
       <label class="option" for="edit-$tag-$key">$value </label>
    </div>
EOF;
  }
  
  return item(<<<EOF
<div class="form-item form-type-checkboxes form-item-$tag">
  <label for="edit-$tag">$title</label>
  <div id="edit-$tag" class="form-checkboxes">
$boxes
</div></div>
<div class="description">$desc</div>
EOF
  );
}

function test($arg) {
  global $user;
//  cache_set('t_resume', FALSE); // cleanup after potential previous failures
//  cache_set('t_messages', FALSE);
  include __DIR__ . '/../../gherkin/test.php';
/*  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate(); */
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
    'reinstall|Reinstall rCredits|',//    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
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

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Also sets $formArray and $formSta, for testing.
 */
function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formArray, $formSta;
 
  try {
    setupGlobals(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
//    $layout = @$_GET['layout'] ?: svar('layout');
//    if ($function == 'I') $layout = 'Mobile';
//    $newLayout = ($layout != svar('layout'));
    
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
//    if (!strpos(' accounts summary footer ', $function)) u\loga("show $function", compact(u\ray($args)));
//    if ($layout == 'Mobile') css('mobile.css', 100);
    $formName = substr($function, 0, 1) == '/' ? substr($function, 1) : "rCredits\\Web\\form$function";
//    if ($newLayout) svar('layout', $layout); // do this before drupal_get_form, so forms can know the current layout

    $formSta = array(); // next few lines are from \drupal_get_form()
    $args = func_get_args();
    array_shift($args); // Remove $function from the arguments.
    $formSta['build_info']['args'] = $args;
    $form = drupal_build_form($formName, $formSta);

//    $form = \drupal_get_form($formName, $arg1, $arg2, $arg3, $arg4);
//    if (!is_array($form)) return $form; // form probably did return r\go() during testing
    \rweb_form_alter($form, $formSta, $formName);
    $form['#attributes']['class'][] = 'rweb';
//    if ($newLayout) $form['#cache'] = FALSE;
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any
    if (!in_array($function, u\ray('Help footer accounts'))) {
      $extras = array('#id' => 'help-link', '#weight' => @$form['title']['#weight']);
      $help_link = item(t('<a><div></div>Help with this page</a>')) + $extras; // add a help link at top of page
      $form = compact('help_link') + $form;
      js('inc/misc.js', 'file', 'footer');
    }

    $formArray = $form2 = $form; $rent = \render($form2);
//    if ($function == 'Usd') $rent = str_replace('Dwolla', '<a href="' . R_DWOLLA_LINK . '" target="_blank">Dwolla</a>', $rent);
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
  $myid = svar('myid') ?: $agent;
  r\acct::setDefault(r\acct($myid, $agent)); // set default for r\acct()
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
      industry AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName AS display FROM users WHERE :IS_COMPANY AND name LIKE :string
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE :IS_COMPANY AND name LIKE :string)';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $mya = r\acct($myid);
    $cttyId = $mya->community;
    $personal = $role == 'ctty' ? '' : ' AND :IS_PERSONAL';
    $selection = "uid, fullName AS display FROM users WHERE (name LIKE :string AND community=:cttyId$personal)";
    $params .= ' cttyId';
  } else $selection = 'uid, fullName AS display FROM users WHERE (name LIKE :string)';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string sql=$selection"));
//return drupal_json_output(array('zot' => "subs=" . print_r(compact(u\ray('string myid')), 1)));

  $result = r\dbQ($sql, compact(u\ray($params)));
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
  $oftens = array(0 => 'when?', 'M' => 'Monthly', 'Q' => 'Quarterly', 1 => 'Now (maybe again later)');
  $honors = array('-' => '(optional)', 'honor' => 'in honor of', 'memory' => 'in memory of', 'other' => 'other');
  $onchange = str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_CONTRIB_SHOWHIDE);
  $giftAttrib = attrib(compact('onchange'));
   
  $sharePct = round(r\dbLookup('AVG(share)', 'r_gifts'), 1);
  $form = array(
    'title' => item(t('Membership Contribution to Common Good Finance')),
    'subtext' => item($donated . t('<p>Make a contribution of any size, to help support the rCredits system. Buy a BRICK or some other piece of the Common Good Economy, as a tax-deductible contribution (one-time, monthly, or quarterly). Also choose a percentage of your ongoing incentive rewards to share with Common Good Finance.') . ($mya->can(B_OK) ? '' : t(' The amount you choose will come from your account once it is activated.</p>')) . t('</p><p>Thank you for your support!</p>')),
    'gift' => formField('select', t('Contribution:'), '', required($mya->can(B_COMPANY) ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT) + $giftAttrib, $giftLevels),
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
  $donid = r\dbInsert('r_gifts', $info);
  $mya->update(u\just('share', $info));
  $msg = 'gift successful';
  if (!r\acceptGift($info + compact('donid'))) {
    r\tellStaff('gift', $info);
    $msg .= '|gift transfer later';
  }
  $amount = u\formatAmount($info['amount']);
  if (!$mya->can(B_OK)) {
    say("$msg|step completed", compact('amount'));
    r\go('membership');
  } else say($msg, compact('amount'));
}

function formHelp($form, &$sta, $what = '') {
  require_once __DIR__ . '/rweb-help.inc';
  $title = item($what ? ('"' . ucwords(str_replace('-', ' ', $what)) . '"') : t('General Help'));
  $text = formField('item', '', '', helpText($what ?: 'general'));
  return compact(u\ray('title text'));
}

function popupCloser($idHead) {
  global $rUrl;
  return item('<img src="' . $rUrl . '/images/icons/close.png" border="0">') + array('#id' => "$idHead-close");
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
 * @param string $code: numeric means show specific screen even if not appropriate (for in-person demos)
 * @return TRUE if the member has done everything possible toward opening the account.
 */
function formMembership($updating, &$sta = '', $code = '') {
  global $base_url;
  if (!$mya = r\acct()) return NULL;
  if ($updating and $mya->can(B_OK)) return TRUE;
  $myid = $mya->id;  
  $nonperson = !$mya->can(B_PERSONAL);
  $mempage = is_numeric($code) ? $code : FALSE;
  $gotContact = ($mya->phone and $mya->address and $mya->physical and $mya->city and $mya->country);

  if ($mempage or !$mya->can(B_MEMBER) or !$mya->can(B_OK)) {
    extract(u\just('gotPhoto signed donated nearly', r\nearlyMember($mya)));
    $gotProxies = $nonperson ?: (r\dbLookup('COUNT(*)', 'r_proxies', 'person=:myid', compact('myid')) >= 2);
    r\membershipEvent($mya, 'member', $nearly and $gotProxies);
    $gotUsdInfo = (($secure = @$mya->secure) and @$secure['auth']);
    $gotIdProof = (($nonperson ?: file_exists(DRUPAL_ROOT . $mya->auxFilename('proof'))) and @$mya->dob and @$mya->federalId);
    $gotPreferences = !is_null($mya->minimum);
    $ready = ($nearly and $gotProxies and $gotUsdInfo and $gotIdProof and $gotPreferences);
    r\membershipEvent($mya, 'ready', $ready);
  } else $ready = TRUE;

  if ($updating) return $ready;
  
  $message1 = $message2 = $message3 = '';

  if ($mempage == .5 or (!$gotContact and !$mempage)) { // did all the initial steps, but somehow didn't complete contact info
    $message1 = t("<h2>Missing Contact Information</h2><p>Please complete your <a href=\"$base_url/account/contact\">Contact Information</a> before continuing.</p>");
  } elseif ($mempage == '4' or ($ready and !$mya->can(B_OK) and !$mempage)) {
    $message1 = t('<h2>Your Account Setup Is Complete</h2><p>You have done everything you need to do. You can expect your account to be activated within the next 24 hours.</p><p>Thank you for joining us!</p>');
  } elseif ($mempage == '1' or (!$mya->can(B_OK) and !$mempage)) { // not a member yet (no rCard, no vote)
    $message1 = ($gotPhoto or $signed or $donated or ($gotProxies and !$nonperson) or $gotUsdInfo or $gotIdProof or $gotPreferences) ? 
          t("<h2>You're getting there!</h2><p>")
        : t('<h2>Congratulations on signing up for rCredits!</h2><p>Thanks for joining us. ');
    $message1 .= $nonperson ? t('Before you can begin accepting rCredits payments, you must complete the following steps:</p>') 
      : t('Before we can send your rCard, and before you can vote on community funding, you must complete the following steps:</p>');

//    $step1 = t('Complete your <a>contact information</a>.'); // keep this in case
    $stepPhoto = t('<a>Upload a photo</a> for your ') . ($nonperson ? t('company profile.') : t('Member ID Card.'));
    $stepAgree = t('Sign the <a>rCredits Agreement</a>.');
    $stepGift = t('Make a <a>contribution</a> of <i>any size</i>, to support the rCredits system.');
    $stepProxies = t('<a>Choose two people</a> to represent your views whenever you are not there, to discuss and vote on community funding issues. (A list of members in your area will be provided, to choose from.)');
    $stepDwolla = t(<<<EOF
    <p>Open a <a href="$base_url/help/dwolla-account-setup">Dwolla</a> account and <a href="$base_url/account/usd">Connect</a> it to your rCredits account (click on "Dwolla" and "Connect").</p>
EOF
    );
    $stepProof = t('<a>Prove your identity.</a> Upload ')
      . ($nonperson ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotIdProof ? t(' <span class="pending">(DONE, pending approval)</span>') : '');
    $stepPrefs = t('Set your <a>Account Preferences</a>.');

    $stepNum = 1;
    $message2 = memberStep($stepNum++, $gotPhoto, $stepPhoto, 'account/photo')
              . memberStep($stepNum++, $signed, $stepAgree, 'membership/agreement')
              . memberStep($stepNum++, $donated, $stepGift, 'membership/contribute');

    if (!$nonperson) 
    $message2 .= memberStep($stepNum++, $gotProxies, $stepProxies, array('membership/proxies', 'invite'));
          
    $message2 .= memberStep($stepNum++, $gotUsdInfo, $stepDwolla, R_DWOLLA_LINK, TRUE);
    
    if (!$nonperson)
    $message2 .= memberStep($stepNum++, $gotIdProof, $stepProof, 'account/id-proof');

    $message2 .= memberStep($stepNum++, $gotPreferences, $stepPrefs, 'account/preferences');

    $addendum = item(tt('<p>Opening an rCredits account is almost exactly like opening two online bank accounts and connecting them together. We will guide you through the process.</p><p>Please feel free to <a href="mailto:@R_ORG_EMAIL" target="_blank">send us an email</a> if you need help.</p>'));
// (Suggest it after 1mo) if (!$nonperson) $message2 .= memberStep(4, $invited, $step4, 'invite');
  } elseif ($mempage == 2 or (!$mya->can(B_OK) and !$mempage)) {
    $message1 = t('<h2>You have completed all membership steps.</h2><p>Within 24 hours, you should receive an email saying your account is approved and you can expect your rCard in the mail within a day or two. Hurray!</p>'); // UNUSED
  } else {
    $message1 = t('<h2>Congratulations! Your account is Activated.</h2><p>You are authorized to use rCredits to buy and sell with other members.')
      . ($mya->can(B_PERSONAL) ? t(' You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>') : '</p>')
      . (($mya->can(B_BONA) or $mya->verified()) ? '' : t("<p>To receive your $" . R_SIGNUP_BONUS . " signup bonus, you must have a way to get money into your account. Ask someone to pay you with rCredits OR exchange some US Dollars for rCredits by <a href=\"$base_url/account/preferences\">setting a minimum balance</a> and connecting your bank account to Dwolla. As soon as you get some money in, your signup bonus will arrive."));
  }
  $message = $message1 . "<table width=\"100%\">$message2</table>" . @$message3;
  
//  $title = item(t('Membership Status'));
//  $subtext = item(t('for ') . $mya->fullName);

  $steps = formField('item', '', '', $message);
  $form = compact(u\ray('title steps addendum'));
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
  <td class="done done-$step state$done">$link<div>&nbsp;</div>$linkx</td>
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
  $mya = r\acct();
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
  $id = "edit-$name";
  $check = formField('checkbox', '', '', attrib(compact('id', 'name')));
  $model = \render($check);
  $instructions = '';

  if ($date = $mya->signed) {
    $instructions = '';
    $date = u\formatDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif ($mya->hasBit(B_PERSONAL) and !$mya->proSe()) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $aacct = $mya->agent;
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

  $personOnly = !$mya->hasBit(B_PERSONAL) ? '' : <<<EOF
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
  $i9 = $mya->hasBit(B_PERSONAL) ? R_AGREE_9 : R_AGREE_9 - 1;
  for ($i = R_AGREE_0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  $fullName = r\agent()->fullName;
  if (strcasecmp(trim($sta['input']['signedBy']), $fullName) != 0) return say('bad signature', compact('fullName'), 'signedBy');
}

function formAgreement_submit($form, &$sta) {
  $signed = REQUEST_TIME;
  extract(u\just('signedBy', $sta['input']));
  r\acct()->update($info = compact('signed', 'signedBy'));
  say('info saved|step completed');
  r\tellStaff('agreement signed', $info);
  r\go('membership');
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
//<p>You can identify your representatives (in the boxes below) by name, email, or member ID. Click on "Help with this page" for a list of choices during the Pilot Phase.</p>

  /*
  $proxy1 = textField(t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'), required(r\definiteName(r\proxy($id, 1))) + autocomplete('cttyPerson'));
  $proxy2 = textField(t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'), dft(r\definiteName(r\proxy($id, 2))) + autocomplete('cttyPerson'));
  */

  $ctty = $mya->community;
  $region = r\uidRegion($mya->id);
  $choices = array();
  $result = r\dbQ('SELECT uid, fullName, city FROM users WHERE uid>1 AND :IS_PERSONAL AND community=:ctty AND :R_ACCT_REGION=:region ORDER BY postalCode, city, fullName', compact('ctty', 'region'));
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
    if (!$proxy->can(B_PERSONAL)) return say('inhuman proxy', compact('name'), $proxyName);
    $sta['input'][$proxyName] = $proxy->id; */
  if ($proxy1 == $proxy2) return say('doubled proxy', 'proxy2');
}

function formProxies_submit($form, &$sta) {
  $myid = r\acct()->id;
  extract($info = u\just('proxy1 proxy2', $sta['input']));
  if ($proxy1) r\proxy($myid, 1, $proxy1);
  if ($proxy2) r\proxy($myid, 2, $proxy2);
  if ($proxy1 and $proxy2) {
    r\go('membership');
    r\tellStaff('chose proxies', $info);
    say('info saved');
  } else say('proxy to go');
}

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 */
function formInvite($form, &$sta) {
  global $base_url, $rUrl;
  $mya = r\acct();
  $title = item(t('Invite Someone to Sign Up for rCredits'));
  extract(u\just('nearly', r\nearlyMember($mya)));
  $subtext = item(!$nearly ? t("We're sorry, you cannot invite someone to sign up until you have completed your other membership steps.<br><br>Return to the <a href=\"$base_url/membership\">Membership</a> page to see where you stand.") : (t('Adjust the subject and message, then click on "Send".') . (MINI_LAUNCH ?  t(' NOTE: Your invitation will be saved and sent <span style="color:red;">after this Pilot Phase</span> is completed.') : '')));
//  $subtext = item(strtr($subtext, array('<a>' => "<a href=\"$base_url/membership\">")));
//  $contact = textField(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());
  $email = textField(t('Email:'), t('An email address for the person you want to invite'), required());
  $trusted = boolField(t('Trusted?'), t('If you lent this person $250, would you trust them to pay it back?'));
  $subject = textField(t('Subject:'), t('The subject of your message'), required($mya->fullName . t(' invites you to rCredits')));

  $default = "rCredits looks amazing. I signed up. Check it out!\n\n$mya->fullName";
  $blurbLink = array('<a>' => "<a href=\"$rUrl/templates/invite.html\" target=\"_blank\">");
  $messageHelp = strtr(t('Invite them to sign up, and say why. A <a>blurb about rCredits</a> will follow your message, with a signup link.'), $blurbLink);
  $message = formField('textarea', t('Message:'), $messageHelp, required($default));
  $submit = submit(t('Send'));
  $form = $nearly ? 
      labeled(compact(u\ray('title subtext email trusted subject message submit')))
    : compact(u\ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(u\just('email trusted', $sta['input']));
  if (!\valid_email_address($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('email subject message', $sta['input']));
  $email = strtolower($email); // otherwise u\SUBS breaks on cap after @
  $info = array(
    'uid' => $mya->id,
    'noFrame' => TRUE,
    'subject' => $subject,
    'PERSONAL_NOTE' => $message, 
    'MEMBER_NAME' => $mya->fullName, 
    'DATE' => u\formatDate($mya->signed),
    'CODE' => r\invite($email, $mya->id, '', $subject, $message),
  );
  if ($mya->id == 26742000000001 or !MINI_LAUNCH) {
    \drupal_mail('rCredits', 'invite', $email, '', $info);
    say('invite sent|repeat invite', compact('email'));
  } else say('Your invitation has been saved and will be sent after this Pilot Phase is completed and membership is open to the public.|repeat invite');
  r\tellStaff('invited', $info + compact('email'));
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
  $mya = r\acct();
//  debug($mya->can(B_ALL)); // keep
  if (!$mya) return r\go('/user'); // this cannot be '' (infinite redirect)
  $mya->refreshCan(); // update any permissions that may have been set elsewhere
//  if (!formMembership(TRUE) and $mya->proSe() and !$mya->can(B_COMPANY)) say('take a step');
  if (!formMembership(TRUE)) {
    say('take a step');
  } elseif (!$mya->can(B_OK)) say('setup complete');

  $name = $mya->name;
  $address = r\location($mya, TRUE);
//  $regionName = $mya->id == 1 ? '' : r\acct($mya->community)->fullName;
  $floorDesc = t('Your balance can go this low') . ($mya->floor < 0 ? t(' for up to 30 days') : '')
    . '. ' . t('See "Debt Okay" in') . " <a href=\"$base_url/account/preferences\">Preferences</a>.";
  
  if ($mya->can(B_CTTY_ADMIN) and !$mya->proSe()) {
// FAILS on mobile    $idLink = 'id="idcard"'; // use jQuery to create popup (see inc/misc.js)
    $address .= '<br>' . u\formatPhone($mya->phone) . " ($mya->faxetc)";
    $idLink = 'href="print-rcard" target="_blank"';
    $card = $mya->can(B_PERSONAL) ? "<a $idLink>" . t('Print ID Card') . '</a>' : '';
    if ($mya->can(B_ADMIN)) $card .= ' [<a href="http://localhost/devcore/rcredits/test?module=rcredits/rweb&feature=Signup&scene=&restart=1">run current test</a>]'; // DEBOG (keep)
  } else $card = '';

  $title = item(t('Account Summary'));
//  'subtext = basic_account_info();
  $accountName = formField('item', t('Name:'), $address, $mya->fullName . ' ' . u\hug($name));
  $balance = u\formatAmount($mya->r);
  $usdBalance = u\formatAmount($mya->usd);
  $total = u\formatAmount($mya->r + $mya->usd);
  $balance = formField('item', t('Balance:'), '', "$balance<span class='r'>r</span> + $usdBalance<span class='us'>us</span> = $total" . ($mya->rewards ? " [<a href=\"$base_url/transactions\">" . t('see Transaction History') . '</a>]' : ''));
  
  if ($mya->id > 0) {
    list ($personal, $commercial) = array($mya->can(B_PERSONAL), $mya->can(B_COMPANY));
    $acctType = ($personal and $commercial) ? 'personal & company' : ($personal ? 'personal' : 'company');
//    $acctType = formField('item', t('Account type:'), '', $acctType);

    $extra = r\agent()->can(B_ADMIN) ? " id#$mya->id" : '';
    $accountId = formField('item', t('ID:'), $card, ($mya->mainQid ?: 'N/A') . ' (' . $acctType . ")$extra");
    if ($mya->can(B_MEMBER) and $mya->can(B_PERSONAL)) $another = submit(t('Open Another Account'));
    $floor = formField('item', t('Credit floor:'), $floorDesc, u\formatAmount($mya->floor));
    $rewards = formField('item', t('Rewards:'), t('(to date)'), u\formatAmount($mya->rewards));
  }

  if (r\agent()->can(B_CTTY_ADMIN) and (!$mya->proSe() or $mya->id == 1)) { // ctty admin not managing own personal account
    $ctty = $mya->community;
    if ($mya->id < 0) { // managing the community itself
      $count = formField('item', t('Members:'), '', r\dbLookup('COUNT(*)', 'users', 'community=:ctty AND uid<>:ctty', compact('ctty')));
    } else { // managing some individual community member account
//      $acctType = formField('select', t('Account type:'), '', $mya->acctType+0, $GLOBALS['account types']); // +0 for admin
      if ($mya->can(B_COMPANY)) $canCharge = boolField(t('Can charge'), t('This account can charge others unilaterally (in person)'), $mya->can(B_CHARGE) + 0);
      $floor = textField(t('Credit floor:'), $floorDesc, dft(u\formatAmount($mya->floor)));
      $rTrader = boolField(t('rTrader?'), t('Does the submitted proof of ID pass muster? Is this account approved to use rCredits?'), $mya->can(B_OK) + 0);

      $bits = u\ray(B_ADMINABLE . ($mya->can(B_ADMIN) ? ' admin' : ''));
      $adminableDefaults = array();
      foreach ($bits as $bitName) {
        $bit = constant('B_' . strtoupper($bitName));
        $adminables[$bit] = $bitName;
        if ($mya->hasBit($bit)) $adminableDefaults[] = $bitName;
      }
      $adminable2 = boxField('adminable', t('Other bits:'), '', $adminableDefaults, $adminables);

      $notes = formField('textarea', t('Notes'), '', dft($mya->notes));
    }
    $who = textField(t('Be who:'), t('Type a name, id, email, or phone of a (different) member account to manage.'), autocomplete($mya->can(B_ADMIN) ? '' : 'ctty'));

    $submit1 = $submit2 = submit(t('Go'));
  }

  $form = compact(u\ray('title who submit1 accountName accountId another balance rewards floor canCharge rTrader adminable2 notes count submit2'));
  return labeled($form);
}

function formSummary_validate($form, &$sta) {
  if (op($sta) == 'another') r\go('another');
  extract(u\just('who floor', $sta['input']));
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
  if (op($sta) == 'another') return r\go('another');

  if (!r\agent()->can(B_CTTY_ADMIN)) return; // hack attempt
  $mya = r\acct();
  extract(u\just('who floor notes canCharge rTrader adminable', $sta['input']));

  if (@$who and ($who->community == r\agent()->community or r\agent()->id == 1)) {
    svar('myid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $mya->fullName);
    return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
  } 
  
  if ($mya->id > 0) {
    $mya->setBit(B_CHARGE, $canCharge, FALSE);
    r\membershipEvent($mya, 'ok', $rTrader); // activate the account
    $mya->setBit(B_OK, $rTrader, FALSE);

    foreach (u\ray(B_ADMINABLE) as $bitName) {
      $bit = constant('B_' . strtoupper($bitName));
      $mya->setBit($bit, isset($adminable[$bit]), FALSE);
    }
    $mya->update(compact(u\ray('floor notes')));
    say('info saved');
  }
}

function formPhoto($form, &$sta) {
  global $base_url;
  $mya = r\acct();

  $form0['#user'] = $mya->account();
  user_account_form($form0, $sta);
  extract(u\just('picture_upload picture_current', $form0['picture']));
  $title = item(t('Profile Photo'));
  
  $got = file_exists(DRUPAL_ROOT . $mya->auxFilename('picture'));
  $picture_current['#markup'] = $mya->picture(TRUE);

  list ($width, $height) = explode(':', $mya->can(B_PERSONAL) ? R_PERSONAL_ASPECT : R_COMMERCIAL_ASPECT);
  $width *= R_PIXEL_FACTOR;
  $height *= R_PIXEL_FACTOR;
  $picture_upload['#description'] = 
    ($mya->can(B_PERSONAL) ? 
        tt('Upload a recent picture of yourself for your photo-ID rCard. It must be taller than wide (@R_PERSONAL_ASPECT ratio is ideal) and must show your full face clearly, filling most of the frame.')
      : tt('Upload a picture to represent your company in rCredits promotions. The picture must be wider than tall (@R_COMMERCIAL_ASPECT ratio is ideal).')) 
    . tt(' File size must be under 3MB. The picture will be cropped and resized to @width x @height pixels.', compact('width', 'height'));
  
  if ($got and $mya->can(B_MEMBER)) {
    $picture_upload = item(R_ON_FILE);
  } else $submit = submit(t('Upload and Save'));

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
    r\go('membership');
  }  

  $mya = r\acct();
  $pictureFilename = DRUPAL_ROOT . $mya->auxFilename('picture');

  r\dbQ('DELETE FROM file_managed');
  $tempName = str_replace('temporary://', \file_directory_temp() . '/', $picture->uri);
  $aspect = $mya->can(B_PERSONAL) ? R_PERSONAL_ASPECT : R_COMMERCIAL_ASPECT;
  if (!u\fixPicture($tempName, $aspect, R_PIXEL_FACTOR, $err)) {
    if ($err == 'aspect') $err = $mya->can(B_PERSONAL) ? 'bad personal aspect' : 'bad commercial aspect';
    return say($err, 'picture');
  }
  if (!rename($tempName, $pictureFilename)) return say('file save error', 'picture'); // (rename fails)
//  if (!unlink($tempName)) say('cannot delete tmp file');
//  if (!unlink($pictureFilename)) say('cannot delete old picture'); return;

  //  if (@$values['picture_delete'] and file_exists($pictureFilename)) unlink($pictureFilename);
  $link = "$base_url/membership";
  $completed = $mya->can(B_OK)? '' : '|step completed';
  say("photo saved$completed|return to membership", compact('link'));
}

function formAccount($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $form['#user'] = $mya->account();
  user_account_form($form, $sta);
// Each region has its own timezone.  system_user_timezone($form, $sta);

  $form['title'] = formField('item', t('Account Information'), '', weight(-99));
  $form['account']['name'] = $form['account']['timezone'] = NULL;
  $form['account']['mail']['#description'] = t('All emails from the system will be sent to this address. The email address is not made public.');
  $form['account']['pass']['#description'] = t('To change your email address and/or password, enter your current password below.');
  //$form['account']['pass']['#process'] = array('form_process_password_confirm', 'user_form_process_password_confirm'); //'rCredits\\Web\\password_confirm');
  $form['account']['current_pass']['#description'] = '';
  $form['account']['current_pass']['#weight'] = 4;
  $form['account']['status']['#weight'] = 5;
  $form['account']['roles']['#weight'] = 6;
  $form['strong'] = hidField(0, attrib(array('id' => 'edit-strong')));

  unset($form['picture']);
  $form['submit'] = formField('submit', '', '', array('value' => t('Save')) + weight(99));
  $form['#user_category'] = 'account';
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'] = array('rCredits\\Web\\formAccount_validate'); // use ours instead
  return $form;
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formAccount_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('mail pass current_pass', $sta['values'])); // values NOT input
  extract(u\just('strong', $sta['input'])); // input NOT values
  if (!$mya->can(B_ADMIN) and empty($current_pass) 
    and ($mail != $form['account']['mail']['#default_value'] or !empty($pass))
  ) return say('password required', 'current_pass');
  // Drupal checks that pass1 == pass2
  if ($pass and !$strong) return say('weak pass', 'pass');
  if (!emailOkay($sta['values']['mail'] = $mail = trim($mail), $mya->id)) return say('bad email', 'mail');
  if (!passwordOkay($mya, $pass)) return FALSE;
}

function formAccount_submit($form, &$sta) {
  extract($info = u\just('mail pass status', $sta['values'])); // values NOT input

  if (empty($pass)) {
    unset($info['pass']); // don't overwrite old password except with a new one
  } else $info['pass'] = r\passHash($pass);
  
  if (r\acct()->update($info)) say('info saved');
}

/**
 * Account Selector form
 * appears above the whole Members Section
 * @todo: make help form say what it is help FOR
 */
function formAccounts($form, &$sta) {
  global $base_url;
  if (!$mya = r\acct()) return NULL; // no account selector before login
  $myid = @$mya->id;
  if (!$accounts = be\accountChoices()) return FALSE;
  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or ctty_admin
  $choices = array();
  foreach ($accounts as $uid) {
    $acct = r\acct($uid);
//    $balance = u\formatAmount($acct->balance());
//    $choices[$uid] = "$acct->fullName: $balance";
    $choices[$uid] = substr($acct->fullName, 0, 20); // keep it short
  }

//  $qid = r\quid($mya->id, $mya->agentId, $mya->regionId);
  
  $onchange = "this.form.submit();";
  $oneText = $choices[$myid];
  
  if (!$mya->proSe() or $mya->hasBit(B_PERSONAL)) {
    $src = $mya->agent->auxFilename('picture');
    $thumb = item("<img src=\"$base_url/$src\"/>");
  }
  $account = count($choices) == 1 ? 
      textField('', '', dft($oneText) + attrib(array('disabled' => TRUE, 'size' => strlen($oneText)+2)))
    : formField('select', '', '', dft($myid) + attrib(compact('onchange')), $choices);
//  $link = item('<a href="http://rCredits.org">Main Site</a
  $submit = submit('Submit'); // default submission

  $chunks = explode('/', $_SERVER['REQUEST_URI']);
  if (isDEV) array_shift($chunks); // site is in a subfolder on the dev server
  $function = $chunks[1]; // normally just one part of URI is the page (but sometimes two)
  foreach (u\ray('account user membership') as $one) {
    if (in_array($one, $chunks) and @$chunks[2]) $function .= '/' . $chunks[2];
  }

  $showMenu = formField('item', '', '', button('Menu', '', 'Show the menu'));
  $showHelp = formField('item', '', '', button('Help', '', 'Help for ' . ucwords($function)));
//  $version = item(t('v 0.1d'));
  $form = u\prefixKeys('acct_', compact(u\ray('thumb account showMenu showHelp submit'))); // always give a dropdown

  $menu = formMenu($function);

  $help = help($function);
  $form += $help + $menu;

  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op acct_account', $sta['input']));
  $op = op($sta);
  $mya = r\acct();
  if (!$mya->can(B_ADMIN) and !r\acct($acct_account, $mya->agentId)->can()) hack('accts');
  \drupal_get_messages(); // clear out messages from previously managed account
  if ($op == 'Menu') {
    r\go('menu'); 
  } else svar('myid', $acct_account); // this gets checked ONLY in setupGlobals()
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
//        + goButtonIf(B_MANAGE, t('Get r/USD'), 'exchange', t('Exchange rCredits for USD or vice-versa'))
        + goButton(t('Find Company'), 'directory', t('Find an rCredits member company'))
//        + goButton(t('Help'), "help/$page", t('Help with this page'))
        + goButton(t('Reports'), 'reports', t('rCredits system-wide or community-wide activity reports'))
        + goButtonIf(B_MANAGE, t('Membership'), 'membership', t('Your rCredits membership progress'))
        + goButtonIf(B_MANAGE, t('Settings'), 'settings', t('Account settings'))
        + goButton(t('Logout'), 'user/logout', t('End your rCredits session'))
  ;
  $menu = formField('fieldset', '', '', array('id' => 'menu')) + compact('menu');
  return compact('menu');
}

/**
 * Settings form for mobile.
 */
function formSettings($form, &$sta) {
  $menu = array()
    + goButtonIf(B_MANAGE, t('Basic'), 'account/basic', t('Email and password'))
    + goButtonIf(B_MANAGE, t('Contact Info'), 'account/contact', t('Tell us where to find you'))
    + goButtonIf(B_MANAGE, t('Photo'), 'account/photo', t('Upload a photo for your rCard'))
    + goButtonIf(array(B_PERSONAL, B_MANAGE), t('Proxies'), 'membership/proxies', t('Choose two people as your representatives'))
    + goButtonIf(B_MANAGE, t('Cells'), 'account/cells', t('Authorize mobile devices for this account'))
    + goButtonIf(B_MANAGE, t('Relations'), 'account/relations', t('Connect other members to this account'))
    + goButtonIf(array(B_COMPANY, B_MANAGE), t('Company Info'), 'account/company', t('Describe this company'))
    + goButtonIf(B_MANAGE, t('Preferences'), 'account/preferences', t('Set your account preferences'))
    + goButtonIf(B_MANAGE, t('USD Account'), 'account/usd', t('Set up the US Dollar part of your account'))
    + goButtonIf(array(B_PERSONAL, B_MANAGE), t('ID Proof'), 'account/id-proof', t('Provide proof of your identity'))
  ;
  $title = item(t('Settings'));
  $menu = formField('fieldset', '', '', array('id' => 'settings')) + compact('menu');
  return compact('title', 'menu');
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
  $mya = r\acct();
  if (!is_array($perms)) $perms = array($perms);
  foreach ($perms as $perm) if (!$mya->can($perm)) return array();
  return goButton($label, $goto, $title);
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company) {
  $uid = r\userField('uid', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or !$acct->hasBit(B_COMPANY)) return say('no such company', 'zot');

  $pic = $acct->picture();
  $state = r\realState($acct->state);
  $country = r\realCountry($acct->country);
  if ($info = r\dbLookup('website,description', 'r_companies', 'id=:uid', compact('uid'))) extract($info);
  $siteList = '';
  if (@$website) foreach (u\ray($website) as $one) $siteList .= "<a href='http://$one' target='_blank'>$one</a><br>";
  $description = @$description ?: '';
    
  $phone = 'Phone: ' . u\formatPhone($acct->phone, '+');
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
 * Make various administrative operations available to the admin.
 */
function formAdmin($form, &$sta, $args = '') {
//  $form = u\insertBefore(compact('communities'), formCommunity(array(), &$sta, $args), 'count');

/* (This dropdown might be better here than at the top of every page)
   (top of page dropdown would then include only the CURRENTLY selected community (and the admin))
  $rows = r\dbQ('SELECT uid, fullName FROM users WHERE uid<0 AND uid<:R_CTTY_UID0 ORDER BY fullName')->fetchAllKeyed();
  $cttyChoices = array_merge(array('ctty-0' => 'Select a community to manage'), u\prefixKeys('ctty', $rows));
  $community = formField('select', t('Manage:'), t('Select a community to manage'), '', $cttyChoices);
*/
  $title = item(t('System Administration'));
  $account = textField(t('Account:'), t('Account to do something to'),auto());
  $printId = submit(t('Print ID'));
  $form = compact(u\ray('title account printId'));
  foreach (u\ray('USD_Accounts Gfld ' . R_SHOWABLE_TABLES) as $one) $form[$one] = submit(str_replace('_', ' ', $one));
  return $form;
}

function formAdmin_submit($form, &$sta) {
  include_once __DIR__ . '/../admin.inc';
  extract(u\just('account', $sta['input']));
  $op = op($sta);
  if ($op == 'USD_Accounts') return say(usdAccts());
  if (@$account) $acct = r\acct(strtoupper($account)); // should use whois first
  
  if ($op == 'printId') {
    $qid = $acct->qid();
    $agent = $acct->agent;
    $field = strpos($qid, '=') ? 'cardCode2' : 'cardCode';
    if (!$code = $agent->$field) {
      u\EXPECT($field == 'cardCode2', 'missing cardCode'); // only cardCode2 gets created here
      $secure = $agent->secure;
      $code = $$field = u\randomString(R_CARDCODE_LEN, 'word'); // create an rCard security code
      $secure += compact($field); // add an rcard code for this agent's relationships
      $agent->update(compact('secure')); // resave the secure fields
    }
    return r\go('print-rcard/' . $qid);
  }
  
  // handle all other buttons with showTableQ()
  $where = "uid>1";
  $userSql = "SELECT uid, MID(phone,3) AS phone, flags, r, usd, rewards, minimum AS min, maximum AS max, committed AS comit, share, floor, address, city, postalCode AS zip FROM users WHERE $where ORDER BY name";
  if ($op == 'users') return say(showTableQ($userSql));
  $userSql = str_replace($where, "postalCode LIKE '013%'", $userSql);

  if ($op == 'log') return say(showTableQ("SELECT channel,type,myid,agent, REPLACE(special, ',', ', ') AS special, REPLACE(info, ',', ', ') AS info FROM r_log"));
  if ($op == 'Gfld') return say(showTableQ($userSql));
  if ($op == 'txs') return say(showTableQ('SELECT xid, type, state, payer, payee, amount, r, payerFor, payeeFor, payerTid, payeeTid FROM r_txs'));

  foreach (u\ray(R_SHOWABLE_TABLES) as $one) if ($op == $one) return say(showTableQ("r_$one"));
//  list ($community) = explode('-', $community, 2); // get the absolute value of community
//  if ($community) svar('myid', -$community); // make it the new current account
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

  $amount = textField('Amount: $', t('Amount to ') . $type, dft(@$amount));
  $goods = $incentive ? hidField(FALSE) : formField('radios', '&nbsp;', '', dft(isset($goods) ? $goods : 1), $radios); //  + attrib(compact('onchange'))
  $purpose = textField('Purpose:', t('Description of goods and services or purpose'), dft(@$purpose));
  $scanned = hidField(@$scanned); // scanned means user already saw a name and photo
  $$type = submit(ucwords($type));

  $form = labeled(compact(u\ray("title who amount goods purpose $type scanned")));
  
  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  if (confirming_v($sta)) return;
//  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // not needed
  if ($confirm = transfer(op($sta), $sta['input'], FALSE)) confirm($confirm, $sta); // op who amount goods what
}

function formTx_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  transfer(op($sta), $sta['input'], TRUE); // op who amount goods what
  $path = current_path(); // clean out the fields
  if ($i = strpos($path, '/who=')) $path = substr($path, 0, $i);
  $sta['redirect'] = $path;
//  $sta['redirect'] = 'http://www.youtube.com/v/pOCxegQupMU&autoplay=1';
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
    if ($args) say('confirm undo|yes or no', r\txReportArgs(r\acct(), $args));
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
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 */
function formGet($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $isSetUp = !is_null($mya->minimum); // user has already saved some options
  $title = item(t('Get rCredits or US Dollars'));
  $subtext = item(t('Transfer funds to or from your connected bank account. Normally these transfers are controlled automatically by your "minimum" and "maximum" settings on the <a href="' . $base_url . '/account/preferences">Preferences</a> page. But you can do it yourself here. Transfers to or from your checking account may take 3-4 days (10 days for savings and money market accounts).'));

//  $minimum = textField(t('Minimum:'), tt('prompt min'), dft(u\formatAmount($isSetUp ? $mya->minimum : R_MINR_DEFAULT, 's')));
  $ray = (array) $mya->account(R_MONEY_FIELDS);
  $toSpend = u\formatAmount(r\avail('buy', $ray));
  $toCash = u\formatAmount($avail = r\avail('cash', $ray));
  $waiting = u\formatAmount(abs($way = $mya->waitingToBank()));
  $way = $way < 0 ? 'from' : 'to';
  $waiting = item($waiting == '$0' ? t('You have no pending transfer requests.') : tt('You have total pending transfer requests of @waiting @way your bank account.', compact('waiting', 'way')), t('Pending:')); // amount already requested (if any)
  $avail = hidField($avail);
  $balance = item(tt('balances', compact('toSpend', 'toCash')), t('Balance:'));
  $amount = textField(t('Amount:'), t('Transfer how much?'), dft(u\formatAmount(max(0, $mya->overMax()), 's')));
  $getNow = submit('FROM bank');
//  $fromBank = formField('checkbox', t('Automate:'), t('(recommended) Refill my rCredits account from my bank account automatically, as needed.'), dft($isSetUp ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = textField(t('Maximum:'), tt('prompt max'), dft(u\formatAmount($isSetUp ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = textField(t('Excess'), '', dft(u\formatAmount($mya->overMax(), 's')));
//  $toBank = formField('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  $putNow = submit('TO bank');

  return labeled(compact(u\ray('title subtext waiting balance amount getNow putNow avail')));
}
  
function formGet_validate($form, &$sta) {
  extract(u\just('amount avail', $sta['input']));
  if ($err = amtErr('amount', $sta)) return say($err, 'amount');
  if ($amount > $avail) {
    $max = u\formatAmount($avail, '');
    return say('short deposit', compact('max'), 'amount');
  }
  if (abs($amount) < R_BANK_MIN) return say('The amount you asked to transfer is less than the minimum ($@R_BANK_MIN).', 'amount');
}
  
function formGet_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('amount', $sta['input']));

  $action = $amount > 0 ? 'deposit to' : 'draw from';
  $usAcct = new r\usd($mya);
  if ($usAcct->bank($amount, $error)) {
    $amount = u\formatAmount(abs(@$amount));
    say('banked', compact(u\ray('action amount')));
  } else return say('cannot bank', compact('action', 'error'), 'amount');
}

/**
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 *//*
function formGetR($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $request = u\formatAmount($mya->demand);
  $demand = be\creditInfo()->fancy['totalDemand'];
  $minR = u\formatAmount($mya->minimum);
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
  $myid = r\acct()->id;
  if (confirming_v($sta)) return;

  extract($sta['input']);
//  if (blank_field(compact('amount'))) return;
//  list ($message, $args, $confirm) = be\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $myid, FALSE);
  list ($message, $args, $confirm) = be\get('r', $amount, $myid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $sta);
  if (!@$args['success']) say($message, $args, 'amount');
}

function formGetR_submit($form, &$sta) {
  $mya = r\acct();
  if (confirming_s($sta)) return;

  extract($sta['input']);
  list ($message, $args, $confirm) = be\get('r', $amount, $mya->id, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($amount != 0) {
    if ($amount < R_MIN_DEMAND) say('demand too small');
    if (!$mya->can(B_PARTNER)) say('demand not yet');
  }
}
*/

/**
- ** (implement by creating an invite and redirecting to registration, using that code). Choices for the new account (similar choices should be in the invitation form and should be recorded in the invite):
  ONLY individuals can accept invitations (you have to create a personal account before you create a company account)
 */
function formAnother($form, &$sta) {
  $mya = r\acct();
  if (!$mya->can(B_PERSONAL)) r\go('', 'no page permission');
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
  if ($relation == 4 and $flow == 3) say('insufficient relation', 'relation');
}

function formAnother_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('company companyPhone isOwner employeeOk', $mya->data));
  extract(u\just('relation flow', $sta['input']));
  $by = $mya->id;
  $code = r\invite($mya->mail, $mya->id);
  $flow += 0;
  $isOwner |= ($relation == 0);
  $employeeOk |= ($relation == 1);
  if (!findCompany($company, $companyPhone)) list ($fullName, $phone) = array($company, $companyPhone);
  r\go('/user/register/' . http_build_query(compact(u\ray('code fullName phone isOwner employeeOk flow by'))));
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
  if (!$args) $args = basename(t\SERVER('REQUEST_URI')); // necessary as of 20130407
  extract(u\just('code fullName phone by', $args));
  $selfInvite = isset($by);

  // zap standard Drupal fields
  $form['account']['name'] = NULL; // name is calculated from fullName
  $form['account']['mail'] = NULL; // (e)mail is redefined here below
  $form['account']['pass'] = NULL;
  $form['account']['status'] = NULL;

  $form['#attributes']['class'][] = 'rweb';
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  css('rweb.css', -9); // needed because this function doesn't go through show()
  $form['#validate'] = array('rCredits\\Web\\formRegister_validate');
  $form['#submit'] = array('rCredits\\Web\\formRegister_submit'); // must come before any return
  
  $email = invitation(@$code, $inviter);

  $title = item(t('Sign up for rCredits'));
  if (!$email and @$code != bin2hex(R_WORD) and @$code != 'w') {
    $form['actions']['submit'] = NULL; // nothing to submit
    $subtext = item(tt('bad invite'));
    $form['account']['mail'] = hidField('unused@example.com'); // keep Drupal processing happy
    $form += compact(u\ray('title subtext mail'));
  //if (r\dbExists('users', "name='abeone'")) die('1 ' . print_r(debug_backtrace(), 1));
    return $form;
  }

  $subtext = item(t('Please complete this form, to open your rCredits account. For personal accounts, all contact information is private. For other accounts, your rCredits web page displays most contact information (but not your email address).'));
  
  $form['#attributes']['class'][] = 'labeled'; // "invitation required" message is not "labeled", so this comes after
  js('inc/radiocheck.js', 'file', 'footer'); // this function is not called by showForm, so we have to do this here
  js("jQuery('#edit-accttype').change();", 'inline', 'footer');

  $acctTypes = $GLOBALS['account types'];
  if (!$selfInvite) $acctTypes = u\just('0 1', $acctTypes);
  $acctTypeHelp = t('If you are self-employed, we recommend you open a separate account for the business. Choose "personal AND commercial" only if you use your own social security number and photograph for the business and you don\'t want a separate account.');
  $companyOptions = array(
    'isOwner' => t('owner'),
    'employeeOk' => t('employee'),
    'contractor' => t('other worker (contractor, supplier, etc.)'),
  );
  $onchange = "var type = jQuery('#edit-accttype').val(); var info = jQuery('#edit-companyinfo'); var calling = jQuery('.form-item-calling'); if (type == 0) info.show(); else info.hide(); if (type <= 1) calling.show(); else calling.hide();";
  $acctTypeAttrib = attrib(compact('onchange'));
  
  $companyInfo = array(
    'company_text' => item(t('Are you an owner and/or employee of a local company, or of any participating rCredits company? Otherwise leave this section blank.')),
    'company' => textField(t('Company: '), t(''), autocomplete('company')),
    'companyPhone' => textField(t('<small>Company</small> Phone:'), t('What is the company\'s primary phone number?')),
    'companyOptions' => formField('checkboxes', '', '', '', $companyOptions),
  );
  $emailHelp = t('All emails from the system will be sent to this address. It will not be made public.');
//  If you want to use this address <b>for more than one account</b>, include a tag on the local part starting with "+" (for example, myemail<b>+whatever</b>@example.com).');

  $mail = textField(t('Email:'), $emailHelp, required());
  $acctType = formField('radios', t('Account type:'), $acctTypeHelp, required() + $acctTypeAttrib, $acctTypes);
  $companyInfo = formField('fieldset', '', '', array('id' => 'companyInfo')) + $companyInfo;
  
  $calling = textField(t('Life Calling:'), t('One of the main purposes of rCredits is to provide ourselves the opportunity to do what we feel called to do in life. What would bring you joy to do, to earn rCredits (that you\'re not doing yet)? What might you have to offer?'));
  $buysAts = r\dbQ('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
  $buysAt = formField('checkboxes', t('<br>Over 60 Greenfield businesses have already signed up.<br>Which of these <b>Not-Yet-Member Businesses</b> do you frequent?'), '', '', $buysAts);
  
  $args = hidField(@$args);
  
  $defaults = compact(u\ray('country state fullName phone'));
  $form += compact(u\ray('title subtext acctType')) + contactFields($form, $defaults) 
    + compact(u\ray('companyInfo calling buysAt args'));
  $form = u\insertBefore(compact('mail'), $form, 'phone');

  keep_values($form);
  t\snapShot($form);
}

function formRegister_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = array(); // for testing (keep Drupal from griping)
  extract(u\just('args phone mail country city address company companyPhone companyOptions fullName postalCode physical code', $values = u\trimAll($sta['input'])));
  if (@$args) extract(u\just('code', $args));
  $sta['input'] = array(); // make sure Drupal doesn't save the record when WE find an error (no longer needed?)
  //if (r\dbExists('users', "name='abeone'")) die('1 ' . print_r(debug_backtrace(), 1));
  if (!invitation(@$code) and @$code != bin2hex(R_WORD) and @$code != 'w') return r\go('/user/register'); // in case user resubmits
  //if (r\dbExists('users', "name='abeone'")) die('1 ' . print_r(debug_backtrace(), 1));
  if (!$phone = u\formatPhone($phone, '+n')) return say('bad phone', 'phone');
  
  if (!emailOkay($mail)) return;
  if ($country == R_COUNTRY_ID) {
    if (!preg_match('/^\d{5}(\-\d{4})?$/', $postalCode)) return say('bad zip', 'zot');
    if (strlen($phone) != 12) return say('bad phone', 'phone');
  }
  
  if (@$companyOptions) extract(u\just('isOwner employeeOk contractor', $companyOptions));
  $gotrelation = (@$employeeOk or @$isOwner or @$contractor);
  if (@$company or @$companyPhone or $gotrelation) {
    foreach (u\ray('company companyPhone') as $one) {
      if (!@$$one) return say('missing field', array('field' => $one), $one);
    }
    if ($err = badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (!$companyPhone = u\formatPhone($companyPhone, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $data = compact(u\ray('company companyPhone employeeOk isOwner contractor'));
  } else $data = array();
  
  if ($err = badName($fullName)) return say($err, array('field' => 'fullName'), 'fullName');
  $name = $name0 = r\uniqueName($fullName);
//  $name = $name0 = u\shortName($fullName);
//  if (r\userField(1, 'name=:name', compact('name'))) return say('duplicate name'); // require unique fullName
  if (stripos(@$physical, ' box ') or stripos(@$physical, 'pob ') !== FALSE) return say('po in location', 'physical');

  extract(u\normalizeCase(compact(u\ray($normals = 'fullName city address physical'))));
  $community = r\communityUid($postalCode);
  $flags = B_DFT;
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  $sta['input'] = compact(u\ray("name data $normals community flags administer_users status phone")) + $values;
  //if (r\dbExists('users', "name='abeone'")) die('1 ' . print_r(debug_backtrace(), 1));
}

function formRegister_submit($form, &$sta) {
  $fields1 = 'args data company buysAt';
  $fields2 = 'name fullName mail phone faxetc country postalCode state city address physical';
  //if (r\dbExists('users', "name='abeone'")) die('s1 ' . print_r(debug_backtrace(), 1));

  $values = $sta['input'];
  extract(u\just("name acctType $fields1", $values));
  u\EXPECT($args, 'no args');
  extract(u\just('code', $args));

  $flags = B_DFT
    | ($acctType <= R_SELF_EMPLOYED ? u\bit(B_PERSONAL) : 0)
    | ($acctType >= R_SELF_EMPLOYED ? u\bit(B_COMPANY) : 0);
    
  if ($acctType >= R_SELF_EMPLOYED) { // company
    $type = $GLOBALS['account types'][$acctType];
    $structure =
        ($acctType == R_SELF_EMPLOYED ? u\bit(R_SELF_EMPLOYED)
      : (strpos($type, 'nonprofit') !== FALSE ? u\bit(R_NONPROFIT)
      : (strpos($type, 'corporation') !== FALSE ? (u\bit(R_CORPORATION) | u\bit(R_PUBLICLY_TRADED)) : 0)));
  }
  
  $DBTX = \db_transaction();

  $wants = 0; // store a bit for each bought-at company
  if (@$buysAt) foreach ($buysAt as $one) if (@$one) {
    r\dbQ('UPDATE r_nonmembers SET potential = potential+1 WHERE id=:one', compact('one'));
    $wants |= u\bit($one);
  }
  $data += compact(u\ray('wants'));
  if ($acctType <= R_SELF_EMPLOYED) $pass = oneTimePass($name, $data); // must precede new r\acct()

  $mya = new r\acct(u\just($fields2, $values) + compact(u\ray('flags structure data')));
  $myid = $mya->id;
  u\EXPECT($myid > 0, 'registration failure');
  setupGlobals($myid); // some rcredits functions need to know who we are
  invitation($code, $myid); // mark the invitation used

  // create relation for company relationship specified in registration form (if any)  
  if (@$company) {
    extract(u\just('companyPhone isOwner employeeOk contractor', $data));
    $isOwner = (int) @$isOwner; // POSTed as "isOwner" when TRUE
    $employeeOk = (int) (@$employeeOk or @$contractor);
    if ($main = findCompany($company, $companyPhone)) { // company account exists
      $other = $myid;
      r\dbInsert('r_relations', compact(u\ray('main other employeeOk isOwner')));
    }
  } 

  // set up relationship of this new account with the account that created it (if any)  
  extract($params = u\just('isOwner employeeOk flow by', $args));

  if (@$by) { // created by a related account? set it up
    if (!$error = u\EXPECT($params, 'int int int id', TRUE)) { // if not a hack attempt
      list ($main, $other, $employerOk, $permission) = array($myid, $by, $employeeOk, PERM_MANAGE);
      $draw = ($flow & 2) ? 1 : 0;
      r\dbInsert('r_relations', compact(u\ray('main other employerOk employeeOk isOwner draw permission')));
      if ($flow & 1) {
        list ($main, $other, $draw) = array($other, $main, TRUE);
        r\dbInsert('r_relations', compact(u\ray('main other draw')));
      }
    } else r\tellStaff("bad params in formRegister: $error -- ", $params);
  }
  
  // report to staff and user
  $quid = $mya->mainQid;
  tellStaffNewMember(u\just("name acctType by $fields1 $fields2", $values) + compact('by', 'quid'));

  if ($acctType <= R_SELF_EMPLOYED) { // personal (possibly self-employed)
    say('your account is ready', compact(u\ray('quid')));
    $region = strtolower(R_SERVER_ID);
    r\notify($myid, 'welcome', compact(u\ray('name quid pass region')), TRUE); 
  } else { // non-personal company (no independent login)
    say('company is ready', compact(u\ray('quid')));
    if (@$by) $quid = r\quid($by); // "if" for dev faked code
    if (!@$by and !$mya->can(B_ADMIN)) hack('company no by');
  }
  unset($DBTX); // commit

  //$sta['redirect'] = 'user/login/name=' . strtolower($quid);
  return r\go('/user/login/');
}

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $mail: the email address to check
 * @param int $uid: account record id (optional), if changing email address rather than creating new account
 * @return: TRUE if email is okay to use, else FALSE
 */
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
 * Verify the syntax of the given name. (replacement for \user_validate_name() )
 * Accept any printable character.
 * @param string $name: the field to test
 * @return: the error message (FALSE if $name is okay)
 */
function badName($name) {
  if (strpos($name, '  ') !== FALSE) return 'multiple spaces';
  if (!ctype_print($name)) return 'illegal char';
  if (drupal_strlen($name) > USERNAME_MAX_LENGTH) return 'too long';
  return FALSE;
}

/**
 * Tell the staff about a new member.
 */
function tellStaffNewMember($params) {
  extract($params);
  $phone = u\formatPhone(@$phone);
  $country = r\realCountry($country);
  $state = r\realState($state);
  $acctType = $GLOBALS['account types'][$acctType];
  $by = @$by ? r\acct($by)->fullName : 'unknown';
  $companyOptions = json_encode(@$companyOptions);
  $buysAt = @join(',', array_values($buysAt));
  $params = compact(u\ray('phone country state acctType by companyOptions buysAt')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  r\tellStaff($message, $params);
}

function findCompany($company, $companyPhone) {
  $companyShortName = u\shortName($company);
  $companyUid = r\dbLookup('uid', 'users', ':IS_COMPANY AND phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = r\dbLookup('uid', 'users', ':IS_COMPANY AND name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = r\dbLookup('uid', 'users', ':IS_COMPANY AND name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Return the part of the form that is common to formContact and formRegister
 * (the contact fields)
 */
function contactFields($form, $defaults = array()) {
  $country = $state = ''; // make sure these are not NULL
  extract(u\just('fullName phone faxetc website country postalCode state city address physical', $defaults));

  $fullnameDesc = t('Your full legal name, properly capitalized');
  $onchange = "print_state(this.options[this.selectedIndex].value,'');";
  $countryAttrib = attrib(compact('onchange'));
  js('inc/countries.js', 'file', 'header');
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');
  focusOn('fullName');
  $onchange = "if (jQuery('#edit-postalCode').val().substr(0, 3) == '013') jQuery('.form-item-buysAt').show();";
  $zipAttrib = attrib(compact('onchange'));
//  $onfocus = "var physical = jQuery('#edit-physical'); var country = jQuery('#edit-country').val(); var state = jQuery('#edit-state').val(); if (physical.val() == '') physical.val(jQuery('#edit-address').val() + ', ' + jQuery('#edit-city').val() + ', ' + ss[country][state] + ' ' + jQuery('#edit-postalCode').val() + ' ' + cs[country]);";
  $onfocus = "var physical = jQuery('#edit-physical'); var country = jQuery('#edit-country').val(); var state = jQuery('#edit-state').val(); if (physical.val() == '') physical.val(jQuery('#edit-address').val() + ', ' + jQuery('#edit-city').val() + ', ' + ss[country][state]);"; // physical.attr('selectionEnd',0); fails
  $physicalAttrib = attrib(compact('onfocus'));
  
  $form += array(
    'fullName' => textField(t('Full name:'), $fullnameDesc, required(@$fullName)),
    'phone' => textField(t('Phone:'), t('Your primary phone (landline or cell)'), required(@$phone ? u\formatPhone($phone): '')),
    'faxetc' => textField(t('Fax etc:'), t('List other ways to contact you: fax, cell, IM, etc. This field is NOT public.'), dft(@$faxetc)),
    'country' => formField('select', t('Country:'), t(''), dft(@$country) + $countryAttrib), // can't use required() here
    'postalCode' => textField(t('Postal code:'), t(''), required(@$postalCode) + $zipAttrib),
    'state' => formField('select', t('State:'), t(''), dft(@$state)), // can't use required() here
    'city' => textField(t('City:'), t(''), required(@$city)),
    'address' => textField(t('Address:'), t('Where does the post office send your mail (usually)?'), required(@$address)),
    'physical' => textField(t('Location:'), t('Where is your building located?'), required(@$physical) + $physicalAttrib),
  );

  return $form;
}  

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
//  be\mergeCheck(compact(u\ray('name phone faxetc'))); // check before and after
  $mya = r\acct();
//  extract((array) $mya->account(), EXTR_PREFIX_ALL, 'my');

  $title = item(t('Contact Information'));
  $submit = submit();
  $defaults = u\just('fullName phone faxetc country postalCode state city address physical', $mya->account());
  $form = compact('title') + contactFields($form, $defaults) + compact('submit');
  
  if ($mya->can(B_COMPANY)) $form['fullName']['#description'] .= '<br>' . t('(If you change this, your username and rCredits webpage address will also change.)');

  return labeled($form);
}

function formContact_validate($form, &$sta) {
  $myid = r\acct()->id;
  extract(u\just('phone fullName country state physical', u\trimAll($sta['input']))); // trim ALL values, for later
//  if (blank_field(compact(u\ray('phone city address')))) return;

  // add dropdowns, just in case there is an error
  js('inc/countries.js', 'file', 'header');
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');

  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['input']['phone'] = $phone;
//  if ($fax and !($fax = u\formatPhone($fax, '+n'))) say('bad phone', 'fax'); else $sta['input']['fax'] = $fax;
  if ($fullName != r\acct()->fullName) {
    $shortName = u\shortName($fullName);
    $otherName = r\userField('fullName', 'name=:shortName AND uid<>:myid', compact('shortName', 'myid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
    $sta['input']['name'] = $shortName;
  }
  if (stripos($physical, ' box ') or stripos($physical, 'pob ')) say('po in location', 'physical');
}

function formContact_submit($form, &$sta) {
  extract($info = u\just('fullName name phone fax country postalCode state city address physical', $sta['input']));
  if (r\acct()->id == 1) unset($info['name']); // don't change 'admin'
  $info = u\normalizeCase(u\just('fullName city address physical', $info)) + $info;

  r\acct()->update($info);
//  be\mergeCheck(compact(u\ray('name phone fax'))); // check before and after
  say('info saved');
}

/**
 * Draft an individual or company into the database.
 * Called when a member tries to pay or charge a non-member.
 */
 /*
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
  extract(u\just('op fullName phone argString return', $sta['input'])); // (trim ALL values for later)
  if (op($sta) == 'cancel') {
    drupal_get_messages('error'); // don't complain about blank fields
    return r\go("$return/$argString");
  }
  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['input']['phone'] = $phone;
  if (!u\validName($fullName, FALSE, FALSE)) say('bad name', 'name');
  $sta['input']['fullName'] = u\normalizeCase($fullName);
  $shortName = u\shortName($fullName);
  if (($otherName = r\userField('fullName', 'name=:shortName or phone=:phone', compact('shortName', 'phone')))) {
    $draftLink = "$base_url/$return/who=$otherName&$argString";
    say('similar found', compact(u\ray('otherName phone draftLink')), u\shortName($otherName) == $shortName ? 'fullName' : 'phone');
  } else $sta['input']['name'] = $shortName;
}

function formDraft_submit($form, &$sta) {
  extract(u\just('argString return name', $info = $sta['input']));
  $acct = new r\acct($info);
  say('info saved|ok to continue');
  $sta['redirect'] = "$return/who=$name&$argString";
}
*/
function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

/**
 * Upload a photo ID
 */
function formProof($form, &$sta, $args = '') {
  extract(u\just('show', $args));
  $mya = r\acct();
  $got = file_exists(DRUPAL_ROOT . $mya->auxFilename('proof'));
  $proofType = $mya->can(B_PERSONAL) ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t("EIN documentation from the IRS (or the equivalent) -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  $form = array(
    'title' => item(t('Proof of Identity')),
    'subtext' => item(t('Just like for a bank account, we need to verify your identity. The file you upload can be either an image or a PDF. Please be patient while the file is encrypted &mdash; it may take several seconds.')),
    'idProof' => $got ? 
        formField('item', t('Photo ID:'), '', R_ON_FILE . ($mya->hasBit(B_OK) ? '' : t(' (pending approval)')))
      : formField('file', t('Photo ID:'), tt('Upload an image of your @proofType. (maximum @R_MAX_UPLOAD_SIZE MB)', compact('proofType')), attrib(array('enctype' => "multipart/form-data"))),
    'submit' => $got ? NULL : submit(t('Upload and Save')),
  );

  if ($mya->can(B_CTTY_ADMIN)) {
    $field = 'idProof';
    u\prefix($form[$field]['#description'], show($field, @$show));
  }
  return labeled($form);
}

function formProof_validate($form, &$sta) {
  $mya = r\acct();
  $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof');

  if (@$_FILES['files']['name']['idProof']) { // fine to let this be uploaded separately
    $validators = array('file_validate_size' => array(R_MAX_UPLOAD_SIZE * 1024000));
    if (!$file = file_save_upload('idProof', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime) and @$file->filemime != 'application/pdf') return say('bad file type', 'idProof');
    $tempfile = file_directory_temp() . '/' . $file->filename;
    if (!$key = $sta['input']['idProof'] = bin2hex(u\encryptFile($tempfile, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    \file_delete($file);
  } elseif(file_exists($proofFilename)) {
    $sta['input']['idProof'] = R_ON_FILE; // flag for no change
  } else return say('nothing done', 'idProof');
}

function formProof_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('idProof', $sta['input']));
  if (@$idProof != R_ON_FILE) {
    if (!$mya->update(compact('idProof'))) return say('info not saved', 'idProof');
  }
  say('info saved'); // don't say anything if getting auth
  r\go('membership');
}

/**
 * Get the Dwolla account info
 * @param string $args: params or report from Dwolla (via usd::callback()) on success of getting auth token
 */
function formUsd($form, &$sta, $args = '') {
  global $base_url;
  extract(u\just('show connected', $args));
  if (@$connected == 'yes') {
    $link = "$base_url/membership";
    $completed = r\acct()->can(B_OK)? '' : '|step completed';
    say("got token$completed|return to membership", compact('link'));
    r\tellStaff('connected to Dwolla');
  } elseif (@$connected) say("There was an error. Dwolla said: $connected", 'zot');
  $mya = r\acct();
  
  if ($mya->offsite) $federalId = $dob = $answer = R_ON_FILE;
  if ($mya->usdAccount) $usdAccount = R_ON_FILE;
  if (@$mya->secure['pin']) $pin = R_ON_FILE;
  if (!@$mya->secure['auth']) say('no auth yet', 'pin');

  $form = array(
    'title' => item(t('USD Account and Security Info')),
    'subtext' => item(t('Have you set up your <a href="' . "$base_url/help/dwolla-account-setup" . '">Dwolla</a> account? Great! Now connect it to your rCredits account. Enter your PIN and security information here. When you click "Save", you will be sent again to the Dwolla site, to sign in and "Allow" the needed permissions.')),
    'usdAccount' => $mya->can(B_ADMIN) ? textField(t('Account:'), t('(This field shows up for administrators only)')) : NULL,
    'pin' => formField(@$pin == R_ON_FILE ? 'textfield' : 'password', t('PIN'), t('Your 4-digit Dwolla PIN'), required(@$pin)),
    'federalId' => $mya->can(B_PERSONAL) ?
        formField(@$federalId == R_ON_FILE ? 'textfield' : 'password', t('Soc Sec #:'), t('Your social security number (for required annual reporting)'), required(@$federalId))
      : textField(t('Federal ID:'), t('Your official employer ID number (EIN)'), required(@$federalId)),
    'dob' => textField(t('Birthdate:'), $mya->can(B_PERSONAL) ? t('Your date of birth (mm/dd/yyyy)') : t('Your date of incorporation or founding (mm/dd/yyyy)'), required(@$dob)),
    'question' => $mya->can(B_PERSONAL) ? textField(t('Question:'), t('Type a question that no one but you could ever guess the answer to.'), required($mya->question)) : NULL,
    'answer' => $mya->can(B_PERSONAL) ? textField(t('Answer:'), t('The answer to that question'), required(@$answer)) : NULL,
    'submit' => submit(),
  );

  if ($mya->can(B_CTTY_ADMIN)) {
    foreach (u\ray('usdAccount pin federalId dob answer') as $field) {
      u\prefix($form[$field]['#description'], show($field, @$show));
    }
    if (!$mya->can(B_ADMIN)) unset($form['subtext']);
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
  $mya = r\acct();
  $page = $field == 'idProof' ? 'id-proof' : 'usd'; 
  
  if ($field == $show) {
    if (strpos(R_SECURE_FIELDS, $field) !== FALSE) {
      $secure = $mya->secure;
      $result = $secure[$field];
    } elseif ($field == 'idProof') { // display the proof image alone (then user clicks on Back)
      $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof');
      $imgString = u\decode(file_get_contents($proofFilename), hex2bin($mya->idProof));
      header("Content-Type: image/jpeg");
      imagejpeg(imagecreatefromstring($imgString));
      exit();
    } elseif ($field == 'dob') {
      $result = u\formatDate($mya->$field);
    } elseif (strpos(R_SECRET_FIELDS . ' usdAccount', $field) !== FALSE) {
      $result = $mya->$field;
    }
  } else $result = "<a href=\"$base_url/account/$page/show=$field\">SHOW</a>";
  return "[$result] ";
}

function onFile($field, $sta) {
  return (@$sta['complete form'][$field]['#value'] == R_ON_FILE and @$sta['input'][$field] == R_ON_FILE);
}

function formUsd_validate($form, &$sta) {
  extract(u\just('pin federalId dob', $sta['input']));
/*  if (!onFile('usdAccount', $sta)) {
    $usdAccount = $sta['input']['usdAccount'] = u\digits($usdAccount);
    if (!preg_match('/^[0-9]{10}$/', $usdAccount)) return say('bad account number', 'usdAccount');
  } */
  if (!onFile('pin', $sta)) {
    $pin = $sta['input']['pin'] = u\digits($pin);
    if (!preg_match('/^[0-9]{4}$/', $pin)) return say('bad pin', 'pin');
  }
  if (!onFile('federalId', $sta)) {
    $federalId = $sta['input']['federalId'] = u\digits($federalId);
    $what = r\acct()->can(B_PERSONAL) ? 'social security number' : 'federal ID';
    if (!preg_match('/^[0-9]{9}$/', $federalId)) return say('bad federal id', compact('what'), 'federalId');
  }
  if (!onFile('dob', $sta)) { // 12/14/1901 is the earliest
    if (!$dob = $sta['input']['dob'] = strtotime($dob) or $dob > REQUEST_TIME) return say('bad date', 'dob');
  }
}

function formUsd_submit($form, &$sta) {
  include_once __DIR__ . '/../usd.class'; // treat this as an extension of this file, just to make menu ['file'] consistent

  $usd = new r\usd(r\acct());
  $usdFields = '';
  $mya = r\acct();
  $info = u\just('type pin auth federalId dob question answer', $sta['input']);
  $secure = @$mya->secure ?: array();
  $retry = ((!@$secure['auth'] or !@$secure['usdAccount']) and $info['pin'] == R_ON_FILE);
//  $retry = FALSE; // DEBUG

  foreach ($info as $key => $value) if ($value == R_ON_FILE or $value == @$form[$key]['#default_value']) unset($info[$key]); // don't change if on file or unchanged
  extract($info); // extract just the ones that changed

  $info = u\just(R_SECRET_FIELDS . ' question', $info);
  if (!empty($info) and !$mya->update($info)) return say('info not saved', 'zot');
  
  if (isset($pin) or $retry) { // this must be last, because it redirects to Dwolla
    $type = 'Dwolla';
    $auth = ''; // no valid auth token until Dwolla calls us back
    $secure = compact(u\ray('type pin auth')) + $secure; // secure includes more than these
    $mya->update(compact('secure'));
    $usd = new r\usd($mya);
    exit($usd->getAuth()); // get (new) auth token (does not return until after a visit to Dwolla)
  }
  
  say('info saved'); // don't say anything if getting auth
  r\go('membership');
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
  extract(u\trimAll($sta['input']));
  if (!u\validName($bank_account_name, FALSE, FALSE)) say('bad name', 'bank_account_name');
  if (!preg_match('/^(|[0-9]{9})$/', $routing_number)) say('bad routing number', 'routing_number');
  if (!preg_match('/^[0-9]{3,20}$/', $bank_account_number)) say('bad account number', 'bank_account_number');
}

function formBank_submit($form, &$sta) {
  extract($sta['input']);
  if (r\acct()->country == R_COUNTRY_ID) $bank_account_number = "USkk$routing_number$bank_account_number";
  r\acct()->update(compact(u\ray('bank_account_name bank_account_number')));
  say('info saved');
}
*/

define('R_REL_EMPLOYER_COL', 2); // number of amount/employer column
define('R_REL_OWNER_COL', 4); // number of family/owner column

/**
 * Give someone access to the current account or mark them as an employee or owner.
 * @todo: get "OR (s.other=:myid AND (s.employeeOk OR s.employerOk))" out of here (look it up instead)
 */
function formRelations($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $notCo = !$mya->can(B_COMPANY);
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers = be\accountName($myid);
  $header = u\ray('Other account,Draw,My employer?,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person draw employer employee owner permission link'));
  if (!$notCo) $header[R_REL_EMPLOYER_COL] = '$/mo';
  if (!$mya->can(B_PERSONAL)) $header[R_REL_OWNER_COL] = 'Owner?';
  
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
  $result = r\dbQ($sql, compact('myid'));
  $rows = $raws0 = $raws = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original data from table
    if ($other == $myid) list ($main, $other, $amount, $permission, $employerOk, $employeeOk, $isOwner, $draw, $reid2, $employerOk2, $employeeOk2)
      = array($other, $main, 0, 0, 0, 0, 0, 0, $reid, $employerOk, $employeeOk);
    
    $otherNotCo = !r\acct($other)->can(B_COMPANY);
    $otherNotPerson = !r\acct($other)->can(B_PERSONAL);
    $idLink = ($otherNotCo and $employeeOk and ($permission >= PERM_SELL)) ? "<a target=\"_blank\" href=\"$base_url/request-employee-rcard/$other\">request&nbsp;rCard</a>" : R_NONE;

    $ii = count($rows) . "-$reid"; // "row-reid"
    $drawn = toggle('draw', $draw, $ii);
    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employerOk2);
    $employee = $otherNotCo ? toggle('employerOk', $employerOk, $ii, $employerOk xor $employeeOk) : R_NONE;
    $Owner = toggle('isOwner', $isOwner, $ii);
    $perm = $otherNotPerson ? R_NONE : dropdown('permission', $permission, $GLOBALS['share permissions'], $ii);
    $amountValue = amountField('amount', number_format($amount,0), $ii);

    $thisRow = array($fullName, $drawn, $notCo ? $employer : $amountValue, $employee, $Owner, $perm, $idLink);
    $employeeOk = @$employeeOk2;
    $thisRaw = compact(u\ray(($notCo ? 'employeeOk' : 'amount') . ' draw employerOk isOwner permission'));
//      if (!$notCo) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    
    $rows[] = $thisRow;
    $raws[] = $thisRaw; // original conceptual values for what's show on screen
  }
  $sta['original_settings'] = compact('raws0', 'raws');
  
  $help = t('Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. For employees and suppliers, say how much you typically pay them in 4 weeks. You do not have to be <a>exact</a>.', array('<a>' => "<a href=\"$base_url/help/virtual-payments\">"));
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
//    if ($acct->can(B_COMPANY)) return say('no company relations', 'newPerson');
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
      if ($value == PERM_MANAGE) $managers++;
/*    } elseif ($keya == 'draw') {
      if (!$value and $raws[$rawnum][$keya]) { // turning off the draw
        $other = r\acct(r\relation('other', $reid));
        $shortfall = -$other->available('exch', FALSE, FALSE);
        $fullName = $other->fullName;
        if ($shortfall > 0) say('no stop draw', compact('shortfall', 'fullName'), 'draw');
      } */
    }
  }
  if (!$mya->can(B_PERSONAL) and !$managers) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
  if (confirming_s($sta)) return;

  $myid = r\acct()->id;

  if ($other = @$sta['input']['other']) { // adding a new relation
    $main = $myid;
    $permission = 0; // default to no permissions
    $info = compact(u\ray('main other permission'));
    r\dbInsert('r_relations', $info);
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }

  $changes = array();
  extract(u\just('raws0 raws', $sta['original_settings']));
//  debug($sta['original_settings']);
  foreach ($sta['input'] as $key => $value) { // ... be\updateRelations will have to check permissions and data carefully
    if (!strpos($key, '-')) continue;
    list ($key, $rawnum, $reid) = explode('-', $key); // eg split permission-2-37
    if (!in_array($key, array_keys($raws))) return 'bad field in relations'; // hack attempt?

    $value = 0 + strtr($value, array('true' => 1, 'false' => 0));
// if ($key == 'employeeOk') debug($value);
// debug(compact('value','rawnum','key'));
// debug(0 + $raws[$rawnum][$key]);
    if ($value != 0 + $raws[$rawnum][$key]) { // changed?
      $changes[$reid][$key] = $value;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
//  debug($changes);
  if (!empty($changes)) say(be\updateRelations($changes));
  if (t\EST()) return r\go('account/relations');
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
  $zot = array($id => hidField($value, attrib(compact(u\ray('id name')))));
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
  $myid = r\acct()->id;
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

  $sql = "SELECT number FROM r_sms WHERE uid=:myid ORDER BY number";
  $result = r\dbQ($sql, compact('myid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(u\formatPhone($number));
  }

  $form = array(
    'title' => item(t('Cell Phones')),
    'subtitle' => item(t('List here the authorized devices for this account. You may also have a cell phone listed on the <a href="account/contact">Contact Information</a> page, but you cannot use it for text message transactions until you list it here.')),
    'cells' => formField('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no cells')), $list),
    'delete' => submit(t('Remove Selected Numbers')),

    'number' => textField(t('Add another cell phone:')),
    'add' => submit(t('Add')),
  );
  return $form;
}

function formCells_validate($form, &$sta) {
  $myid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract(u\just('op code cells number', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op == 'delete') {
    if (no_selection($cells)) {
      if ($number) $op = 'add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $sta);
  }

  if ($op == 'add') {
    if (blank_field(compact('number'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($number, 'VALIDATE'), 'number');
  }
}

function formCells_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract(u\just('op code cells number', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'add') {
    if ($info = previous_state($sta)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($number, tt('verification code', compact('nonce')));
/**/ if (isDEV) debug("nonce is $nonce"); // (keep this) tell developer what code to type
    $info = compact('number');
    return form_step($sta, $info, 'prove');
  }

  if ($op == 'verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($number));
    $sta['redirect'] = 'account/cells';
  }
    
  if ($op == 'delete') {
    foreach ($cells as $key => $number) if ($number) say(be\deleteCell(substr($number, 1))); // ignore the leading 'x'
  }
}

/**
 * Return the recipient of a recent invitation OR mark an invitation USED
 * @param string $code: the invitation code
 * @param int $invitee: the invitee's uid
 * Call by:
 *   $email = invitation($code, $inviter) where $inviter is passed empty, returned with inviter uid
 *     returns the email address to which the invitation was sent and the inviter, if sent recently (else NULL)
 *   invitation($code, $invitee)
 *     marks the invitation USED by recording the invitee's account id
 * @param string $code: the invitation code
 * @param int $inviter: (returned, if empty) account id of the inviter
 * @param int $invitee: account id of the invited person
 * @return: email or nothing, as above (FALSE if no such invitation)
 */
function invitation($code, &$invitee = '') {  
  $lastweek = strtotime(R_INVITE_DAYS . ' days ago');
  $sql = 'code=:code AND invited>=:lastweek AND invitee IS NULL';
  if (!$result = r\dbLookup('email,inviter', 'r_invites', $sql, compact('code', 'lastweek'))) return FALSE;
  extract($result);
  if (@$invitee) {
    r\dbUpdate('r_invites', compact('code', 'invitee'), 'code');
  } else {
    $invitee = $inviter;
    return $email;
  }
}

/**
 * Modify the password reset form.
 */
 /*
function resetAlter(&$form, &$sta) {
  $form['title'] = formField('item', t('First Time Sign In'), '', weight(-99));
  $form['message']['#markup'] = str_replace('</p><p>' . t('Click on this button to log in to the site and change your password.'), t(' Choose a new password, then click on the button to sign in.'), $form['message']['#markup']);
  $form['account']['pass'] = formField('password_confirm', '', t('Enter your new password in both fields.'));
  $form['actions']['submit'] = submit(t('Sign In')); // + weight(99));
  $form['#submit'][] = 'rCredits\\Web\\resetSubmit';
  $form['#attributes']['class'][] = 'rweb';
  focusOn('pass-pass1'); // note that password field gets expanded to two fields, of which pass1 is the first
  t\snapShot($form);
}

function resetSubmit($form, &$sta) {
  svar('reset_pass', $sta['input']['pass']);
//  $uid = $sta['buid_info']['args'][0];
//  r\acct($uid)->update(compact('pass'));
}
*/

/**
 * Implements expand_password_confirm.
 */
 /*
function password_confirm($element, $ifAny = TRUE) {
  $element['pass1']['#title'] = t('New Password') . ($ifAny ? t(' (if any)') : '') . ':';
  $element['pass2']['#title'] = t('Confirm:');
  $element['pass2']['#description'] = t('Type your new password again.');
  return $element;
}

function password_confirm0($element) {return password_confirm($element, FALSE);}
*/

/**
 * Completely redo the login form (see user_login() in user.module).
 * @see also loginFollowup, below (called from rweb_user_login()).
 * @param bool $regular: is this the full-page login form (default TRUE) -- otherwise the narrower login block
 */
function formLogin(&$form, &$sta, $regular = TRUE) {
  $args = basename(t\EST() ? t\SERVER('REQUEST_URI') : $_SERVER['REQUEST_URI']);
  extract(u\just('err', $args));
  if (@$err) say($err, 'zot');

  unset($form['actions']);
  unset($form['pass']);

  if ($regular) {
    $title = item(t('Sign In To Your rCredits Account'));
    $subtext = item(t('Welcome! If you have not yet signed up for rCredits, click on "New account" above.<br><br><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right site. And be sure to sign OUT when you\'re done!'));
  }
  if (($name = @$_COOKIE['loginto']) and ($uid = r\loginString($name))) {
    //$process = array('form_process_password_confirm', 'user_form_process_password_confirm'); // 'rCredits\\Web\\password_confirm0');
    $newPass = formField('password_confirm', '', '', required() + compact('process'));
  }
  $name = textField(t('Username/ID:'), $regular ? t('Your username (the letters of your legal name, all lowercase) OR your rCredits account ID.') : '', required(@$name));
  $pass = formField('password', @$newPass ? t('Tmp password:') : t('Password:'), @$newPass ? t('Your temporary one-time password (the one just emailed to you).') : '', required());
  $strong = hidField(0, attrib(array('id' => 'edit-strong')));

  $submit = submit(t('Sign in'));
  $form = compact(u\ray('title subtext name pass newPass strong submit')) + $form;

  $form['#validate'] = array('rCredits\\Web\\login_extra_validate', 'user_login_final_validate'); // add replacement function
  
  
  $form['external_input'] = hidField(serialize(@$sta['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $form['#attributes']['class'][] = $regular ? 'rweb labeled' : 'rweb';
  focusOn('name');
  css('rweb.css', -9); // this normally gets called in showForm()
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
  extract(u\just('name pass newPass strong', $sta['input']));
  if (!$name or empty($pass)) return FALSE;

  $ipLimit = variable_get('user_failed_login_ip_limit', 50);
  $ipWindow = variable_get('user_failed_login_ip_window', HOUR_SECS);
  if (!flood_is_allowed('failed_login_attempt_ip', $ipLimit, $ipWindow)) {
    $sta['flood_control_triggered'] = 'ip';
    return FALSE;
  }
  if (!$uid = r\loginString($name)) return FALSE; // not a user

  if ($name == 'abeone' and @$newPass['pass1'] == 123) {
    $sta['uid'] = $uid; // for testing
  } else {
    $uidOnly = variable_get('user_failed_login_identifier_uid_only', FALSE);
    $userLimit = variable_get('user_failed_login_user_limit', 5);
    $userWindow = variable_get('user_failed_login_user_window', 21600);
    
    $identifier = $uidOnly ? $uid : $uid . '-' . ip_address();
    $sta['flood_control_user_identifier'] = $identifier;

    if(0) if (!t\EST() and !flood_is_allowed('failed_login_attempt_user', $userLimit, $userWindow, $identifier)) {
      $sta['flood_control_triggered'] = 'user';
      return FALSE;
    }
    if (@$newPass['pass1'] and !$strong) return say('weak pass', 'pass');
    $mya = r\acct($uid);
    $sta['uid'] = (passwordOkay($mya, $pass) ? $uid : FALSE);
  }
  if ($sta['uid']) flood_clear_event('failed_login_attempt_user', $sta['flood_control_user_identifier']);
}

function passwordOkay($a, $password) {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  $stored_hash = $a->pass;
  $hash = \_password_crypt('sha512', $password, $stored_hash);
  if (@$a->data['oneTimePass']) extract(u\just('pass expires', $a->data['oneTimePass']));
  if ($password == @$pass) return REQUEST_TIME < @$expires ? TRUE : say('pass expired', 'current_pass');
  return ($hash and $stored_hash == $hash) ? TRUE : say('wrong pass', 'current_pass');
}

function loginFollowup($edit, $account) {
  extract(u\just('newPass name', $edit['values'])); // values not input
  $resetPass = svar('reset_pass'); // svar gets unset by unset($_SESSION[$key]) just below
  if (!$pass = $resetPass ?: @$newPass) unset($pass);
  
  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($edit['external_input'])); // save params from external online merchant

  $mya = r\acct($myid = $account->uid);
  svar('myid', $myid);
  setupGlobals();

  r\setCook('loginto', '', REQUEST_TIME-1); // no more default login
  $data = $mya->data ?: array();
  unset($data['oneTimePass']);
  $mya->update(compact(u\ray('pass data')));

  u\loga('login', compact('name'), $myid);
  return r\go('summary');
}

function passAlter(&$form, $sta, $id = '') {
  $id = urldecode(basename($_SERVER['REQUEST_URI']));
  $title = item(t('Request New Password'));
  array_unshift($form, compact('title'));
  $form['name'] = textField(t('Account:'), t('Username, Email, or Account ID'), required(u\neq($id, 'password', '')));
  /*$form['name']['#title'] = t('Account:');
  $form['name']['#description'] = t('Username, Email, or Account ID');
  $form['name']['#default_value'] = u\neq($id, 'password', '');*/
  $form['#validate'] = array('rCredits\\Web\\formPass_validate');
  $form['#submit'] = array('rCredits\\Web\\formPass_submit');
  $form['#attributes']['class'] = array('rweb', 'labeled');
  focusOn('name');
  t\snapShot($form);
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

/**
 * Return a once-use password and record it in the data field.
 * @param string $name: login identifier (email, account ID, or shortname)
 * @param assoc $data: data field from account record, returned with "oneTimePass" set to [pass, expires]
 * @return string: the raw password.
 */
function oneTimePass($name, &$data) {
  $pass = strtolower(u\nonce()); // not a hash, easy to remember and type
  if (isDEV) say('Password: ' . $pass);
  $expires = REQUEST_TIME + HOUR_SECS * R_SIGNIN_HOURS;
  r\setCook('loginto', $name, $expires);
  $data['oneTimePass'] = compact('pass', 'expires');
  return $pass;
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
    SELECT DISTINCT u.uid, IFNULL(i.industry, '~Other') AS industry, u.name AS short_name, u.fullName, c.website 
    FROM users u 
    INNER JOIN r_companies c ON c.id=u.uid
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE :IS_COMPANY AND u.$field LIKE :region AND (u.fullName LIKE :which OR i.industry LIKE :which OR c.description LIKE :which)
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
    $mya = r\acct();
    $acct = r\acct($uid);
    $pay = $mya->can(B_BUY) ? button('pay', "$base_url/pay/who=$short_name") : '';
    $chg = $mya->can(B_SELL) ? button('chg', "$base_url/charge/who=$short_name") : '';
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
  $mya = r\acct();
  $myid = $mya->id;

  $sql = <<<EOF
    SELECT iid, IF(parent=iid, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY parent, IF(parent=iid,0,iid)
EOF;
  $cats = r\dbQ($sql)->fetchAllKeyed();
  $myCats = r\dbQ('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$mya->name" target="_blank">Show my public rCredits web page</a> (in a new window)
EOF;

  $form = array(
    'title' => item(t('Company Information')),
    'links' => item($links),
    'name' => formField('item', t('Company name:'), '', $mya->fullName),
    'categories' => formField('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($myCats), $cats),
    'selling' => textField(t('Selling:'), t('What is the typical description of what you sell? (This will appear as the default "purpose" when you charge someone).'), dft($mya->selling)),
    'website' => textField(t('Website(s):'), t('Especially if you want to accept payments on your website, be sure to list it here.'), dft($mya->website)),
    'descPreface' => item(t('<br>What does this company do? The description you enter below will appear on your rCredits web page.')),
    'description' => formField('text_format', t('Description:'), '', $filtered_html + dft($mya->description)),
    'submit' => submit(),
  );

  return labeled($form);
}

function formCompany_validate($form, &$sta) {
//  extract($sta['input']);
}

function formCompany_submit($form, &$sta) {
  $mya = r\acct();
  $id = $mya->id;
  extract(u\just('description website selling categories', u\trimAll($sta['input'])));
//  $description = \check_markup($description['value']);
  $description = @$description['value'];
  $website = str_ireplace('http://', '', @$website);
  u\setDft($selling, '');
  $mya->update(compact(u\ray('description website selling')));
//  r\dbUpdate('r_companies', compact(u\ray('id description website selling')), 'id');
  
  $DBTX = db_transaction();
  r\dbQ('DELETE FROM r_user_industries WHERE uid=:id', compact('id')); // out with the old
  if (@$categories) foreach ($categories as $iid) {
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
  if (!strpos($who, '-')) return r\go('');
  list ($who, $code) = explode('-', $who);
  if (!$acct = r\acct($who)) return hack('i code'); // not a real code -- just go to home page
  if (!$mya = r\acct()) {
    say('signin to scan', 'zot');
    return r\go('');
  }
  $field = strpos($who, '=') ? 'cardCode2' : 'cardCode';
  if ($acct->secure[$field] != $code and !$mya->can(B_ADMIN)) return hack('i code2');
  
  list ($myid, $myAgent) = array($mya->id, $mya->agentId);
  list ($otherId, $otherAgent) = array($acct->id, $acct->agentId);
  if ($otherId == $myid) {
    say('no self-trading', array(), 'error');
    return r\go('');
  }
  
  $shortName = $acct->name;
  if ($acct->can(B_COMPANY)) $site = goButton(t('Show Webpage'), "member/$shortName", t("See this company's rCredits webpage"));

  if ($mya) { // can charge, pay, or change account or agent only if signed in
    $theId = hidField($otherId);
    $theAgent = hidField($otherAgent);
    if ($mya->can(B_SELL)) $charge = goButton(t('Charge'), "charge/who=$shortName&scanned=1", t('Charge someone'));
    if ($mya->can(B_BUY)) $pay = goButton(t('Pay'), "pay/who=$shortName&scanned=1", t('Pay someone'));
    if (r\acct($myid, $otherAgent)->can()) $changeAgent = goButton(t('Change Agent'), "user/logout/who=$shortName", t('Use this button when your shift begins'));
    if (r\acct($otherId, $myAgent)->can()) $changeAccount = goButton(t('Change Acct'), "change-account/who=" . $acct->mainQid, t('Manage a different account'));
    $zot = compact(u\ray('charge pay changeAgent changeAccount site'));
    $all = identifiers($acct) . render($zot);
    svar('lastCustomer', $acct);
  } else {
    $login = item(t('Please sign in, to Pay or Charge this account.'));
    $zot = compact('login', 'site');
    $all = render($zot);
  }
  $all = formField('item', '', '', $all);
  return compact('all');
}

function hack($message, $info = array()) {
  u\loga('hack', $_SERVER, $message);
  return r\go('/user/logout');
}

function changeAccount($quid = '') {
  setupGlobals();
  if (!$mya = r\acct()) return r\go('');
  if ((!$acct = r\acct($quid, $mya->id)) or !$acct->can()) return hack('i'); // hacker. so leave.
  svar('myid', $acct->id);
  $newAcct = $acct->fullName;
  say('changed account', compact('newAcct'));
  return r\go('');
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
  $number = u\formatPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

function formOptions($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $isSetUp = !is_null($mya->minimum); // user has already saved some options
  $title = item(t('Account Preferences'));
  $virtualMaxMessage = $mya->can(B_COMPANY) ? t(' If you choose to make automatic "virtual" payments, this setting is moot (irrelevant and unused).') : '';

  $minimum = textField(t('Minimum:'), tt('prompt min'), required(u\formatAmount($isSetUp ? $mya->minimum : R_MINR_DEFAULT, 's')));
  $maxDft = $isSetUp ? $mya->maximum : R_MAXR_DEFAULT;
  $maxDft = $maxDft < 0 ? '' : u\formatAmount($maxDft, 's');
  $maximum = textField(t('Maximum:'), tt('prompt max') . $virtualMaxMessage, dft($maxDft));
//  $toBank = formField('checkbox', t('To bank:'), t('(recommended) Automatically transfer the excess to your bank account. [<a href="../help/to-bank">why ever not?</a>]'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));
  $dft = $mya->can(B_NOTICE_WEEKLY) ? 1 : ($mya->can(B_NOTICE_MONTHLY) ? 2 : 0);
  $notices = formField('radios', t('Email notices:'), t('How often do you want to receive an email digest of rCredits messages?'), required($dft), $noticeRadios);
  $smsNotices = formField('checkbox', t('SMS notices:'), tt('Send me a text message when I receive a payment, charge, or invoice (requires a <a href="' . $base_url . '/account/cells">connected cell phone</a>).'), $mya->can(B_NOTIFY_SMS));
  $statements = $mya->can(B_ELECTRONIC) + 0;
  $statementRadios = array(
    t('I will accept electronic statements.'), 
    t('I choose to receive printed statements by mail.') . u\SUBS(t('<div>I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>'))
  );
  $statements = formField('radios', t('Statements:'), '', required($statements), $statementRadios);

  if ($mya->can(B_COMPANY)) {
    $desc = t(<<<EOF
    Saying "yes" to <a href="$base_url/help/virtual-payments">virtual payments</a> (<b>highly recommended</b>) means you trade your rCredits to your employees and vendors automatically every day, in exchange for US Dollars, without any change to your normal US Dollar payments! You still get the 5% rebate, minus 25 cents for each payment. For example, a $100 virtual payment gives you a net rebate of $5.00 - 0.25 = $4.75. To make the virtual payments automatic, you need to set up <a href="$base_url/help/virtual-payments">average</a> monthly payment amounts on the <a href="$base_url/account/relations">Relations</a> page. "No" here means you pay everyone directly in mixed rCredits/USD transactions.
EOF
    );
    $dft = $isSetUp ? $mya->hasBit(B_VIRTUAL) : TRUE;
    $payVirtual = boolField(t('Pay virtually:'), $desc, $dft);
  }
  $secretBal = formField('checkbox', t('Secret Balance:'), t("Don't let qualified merchants tell me my balance, when I ask."), $mya->hasBit(B_SECRET_BAL));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $floor = u\formatAmount($mya->floor);
  $debtOk = formField('checkbox', t('Debt Okay:'), tt('Allow my account balance to go negative, limited by my current "credit floor" (which starts at zero, then gets lower and lower as time goes by &mdash; currently @floor). I understand there will be a substantial fee (currently $@R_DEBT_FEE plus @R_DEBT_PERCENT% per month) if I fail to bring my balance back up to zero within @R_DEBT_DAYS days.', compact('floor')), $mya->hasBit(B_DEBT_OK));
  $submit = submit(t('Update options'));
  
  $form = compact(u\ray('title payVirtual minimum maximum smsNotices notices statements secretBal debtOk submit'));
  js('showMax();', 'inline', 'footer');
  return labeled($form);
}

function formOptions_validate($form, &$sta) {
  $mya = r\acct();
  $floor = u\formatAmount($mya->floor);
  if ($err = amtErr('minimum', $sta)) return say($err, 'minimum');
  if ($err = amtErr('maximum', $sta)) return say($err, 'maximum');
  extract(u\just('minimum maximum payVirtual', $sta['input'])); // after amtErr checks
  if ($minimum < $mya->floor) say('bad minr', compact('floor'), 'minimum');
  if (!$payVirtual and $maximum >= 0 and ($maximum < min($minimum, $mya->rewards))) say('bad maxr', 'maximum');
}

function formOptions_submit($form, $sta) {
  global $base_url;
  $mya = r\acct();
  extract(u\just('minimum maximum smsNotices notices statements payVirtual debtOk secretBal', $sta['input']));

//  $mya->setBit(B_TO_BANK, @$toBank, FALSE); 
  $mya->setBit(B_VIRTUAL, @$payVirtual, FALSE);
//  $mya->setBit(B_NOTIFY_EMAIL, $notifyBy & 1, FALSE);
  $mya->setBit(B_NOTIFY_SMS, @$smsNotices, FALSE);
  $mya->setBit(B_NOTICE_WEEKLY, $notices == 1, FALSE);
  $mya->setBit(B_NOTICE_MONTHLY, $notices == 2, FALSE);
  $mya->setBit(B_ELECTRONIC, @$statements, FALSE);
  $mya->setBit(B_DEBT_OK, @$debtOk, FALSE);
  $mya->setBit(B_SECRET_BAL, @$secretBal, FALSE);
  
  if ($payVirtual) $maximum = $minimum;
  $mya->update(compact(u\ray('minimum maximum'))); // flag bits get saved automatically
  if ($mya->can(B_OK)) return say('options saved');
  
  say('options saved|step completed');
  r\go('membership');
}

function formReports($form, &$sta, $args = '') {
  if (!$mya = r\acct()) return NULL;
  if (!$ctty = r\acct($mya->community)) return NULL;
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
  $form = labeled(compact(u\ray('title subtext issued demand')));
  if ($mya->can(B_CTTY_ADMIN)) $form += formTxs('', $sta, $args);
  return $form;
}

function formReports_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formReports_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'reports');
}

/**
 * Request an employee ID card (for buying on behalf of the company).
 */
function formRequestRCard($form, &$sta, $other = '') {
  if (!$acct = r\acct(r\acct()->id, @$other)) return hack('request rCard');
  if (!$acct->hasBit(B_COMPANY) or !$acct->can(B_MANAGE)) return say('no account permission');
  $fullName = $acct->agent->fullName;
  
  $form = array(
    'title' => item(t('Request Employee rCard')),
    'subtext' => item(tt('An rCard will be US Mailed to @fullName, to be used for purchases on behalf of the company. Your company account will be charged $@R_CARD_COST.', compact('fullName'))),
    'other' => hidField($other),
    'okay' => boolField(t('Okay?')),
    'submit' => submit(t('Request Card')),
  );
  return labeled($form);
}

function formRequestRCard_submit($form, &$sta) {
  extract(u\just('okay other', $sta['input']));
  $mya = r\acct();
  if ($okay and $reid = r\relation('reid', $main = $mya->id, @$other)) {
    $quid = r\quid($main, $other);
    $name = $mya->agent->fullName;
    $address = r\location($mya, TRUE);
    $href = "$base_url/print-rcard/$quid";
    r\tellStaff('company rcard', compact(u\ray('main other reid quid href name address')));
    say('card ordered', compact('name'));
  } else say('nothing done', 'zot');
  return r\go('account/relations');
}

/**
 * Print an ID card
 */
function memberID($quid = '', $big = FALSE) {
  require_once(__DIR__ . "/../../tcpdf/config/lang/eng.php");
  require_once(__DIR__ . "/../../tcpdf/tcpdf.php");
  define('PHOTO_MAX', 90); // maximum height and width of photo

  $mya = r\acct();
  $acct = strpos($quid, '=') ? r\acct($quid)
    : ($quid ? r\acct($mya->id, $quid) 
    : (($mya->can(B_CTTY_ADMIN) and $mya->can(B_MANAGE)) ? r\acct($mya->id) : $mya));
  if (!$acct) return 'No such account. Your hack attempt has been logged.';
  
  $fullName = $acct->fullName;
  $regionName = r\acct($acct->community)->fullName;
  $qid = $acct->qid();
  
  if (!$acct->proSe()) {
    $acct = $acct->agent; // hereafter it's all about the agent
    $fullName = "$acct->fullName<br>$fullName"; // agent and company
    $nameSize = '72px';
    $qid = str_replace(':', '=', $qid); // "=" to avoid GoDaddy's 'Bad Request' death redirecting RC2.ME with a ":" qid
    $cardField = 'cardCode2';
  } else {
    $nameSize = strlen($fullName) > 22 ? '100px' : '120px';
    $cardfield = 'cardCode';
  }
  $qrServer = 'http://' . R_SERVER_ID . '.rC2.me/I/';
  $qrUrl = $qrServer . $qid . '-' . $acct->secure[$cardField];
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

  // Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=array())
  $pdf->Image(__DIR__ . '/../images/idcard/background.jpg', 0, 0, 202, 128, '', '', '', true, 150, '', false, false, 0, false, false, false);
  $pdf->Image($photo, 0, 0, $photoW, $photoH, '', './account', '', true, 300, '', false, false, 1, false, false, false);
  $pdf->write2DBarcode($qrUrl, 'QRCODE,Q', 96, 40, 50, 50, $style, 'N'); // L,M,Q,H are low-high error-correction
//  $pdf->Image(__DIR__ . '/../images/idcard/rlogo-sky.jpg', 162, 40.5, 28, 42, '', '', '', true, 150, '', false, false, 0, false, false, false);

  $html = strtr('<span style="font-size:44px;">Member<br><span style="font-size:76px; color:darkred;">@CODE</span><br>@REGION</span>', array('@CODE' => $qid, '@REGION' => $regionName));
  $pdf->writeHTMLCell(100, 30, 94, 10, $html); // w, h, x, y
  $pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">rCredits.org</div>');
  $pdf->writeHTMLCell(182, 40, 10, 100, "<div style=\"font-size:$nameSize; text-align:center; color:midnightblue; font-weight:bold;\">$fullName</div>");
  /*
  $instructions = <<<EOF
<style>
  big {color:darkgreen; font-size:2em;}
  p {margin-top:.5em;}
</style>
Here is your<br><b style="color:darkgreen; font-size:1.3em;">Advance ID Card</b>
<p>You may print this card and use it temporarily, along with a second photo ID. To print, right-click here, then click on "Print" OR use your browser's print icon.</p>
<p>To upload or change your photo, visit the "Settings" section.</p>
EOF;
*/
//  $pdf->writeHTMLCell(90, 64, 110, 5, $instructions);
  // Add temporary symbol over image (maybe remove lower right corner and add "temp"), to discourage unauthorized use
//  $pdf->Image(__DIR__ . '/../images/icons/print.png', 120, 30, 32, 32, '', '', '', true, 150, '', false, false, 0, false, false, false);
  
  $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

/**
 * Return markup to identify the agent of the given account.
 */
function identifiers($acct) {
  $mya = r\acct();
  list ($agent, $company) = $acct->proSe() ? array($acct, '') : array(r\acct($acct->agentId), $acct->fullName);
  $location = r\location($acct);
  $pic = ($mya and $mya->can(B_CHARGE)) ? $acct->picture() : ''; // let qualified companies see the picture
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
  extract($sta['input']);
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
 * @param string $selfMsg: index to error message to give if user self-refers
 * UNUSED @param boolean $create: whether to create the person/company if it doesn't exist
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $selfMsg = 'no self-trading') {
  global $base_url;
  $result = be\identify($who, $selfMsg);
  if ($result == 'ALL' or u\isAcct($result)) return $result;

  list ($msg, $args, $choices) = $result;
  foreach (u\ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = urlencode(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
//  $args['draftLink'] = "$base_url/draft/" . http_build_query($info);
  foreach ($choices as $uid => $fullName) $choices[$uid] = r\quid($uid) . ' ' . $fullName;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, tt($msg, $args));
}
  
/**
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($op, $info, $confirmed) {
  global $lastXid, $scanned;
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\just('who amount goods purpose scanned', $info)); // $scanned is global, for be\transfer
  $txType = $op == 'pay' ? 'payment' : strtolower($op);
  if (blank_field(compact(u\ray('who amount')))) return FALSE;
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if (!$goods) $purpose = 'cash'; // tell be\transfer it's not for goods (should be the only place this string occurs)
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
  if (!$acct = whois($who, 'who', $info)) return FALSE;
  svar('lastCustomer', $acct);
  list ($msg, $args, $confirm) = in_array($txType, array('payment', 'charge')) ?
       be\transfer($txType, $mya, $acct, $amount, $purpose, !u\forCash($purpose), $confirmed)
     : communityTransfer($txType, $acct, $amount, $purpose, $confirmed);

  if (FALSE and !@$scanned) { // not scanned -- We need to show identifying photo, name, etc. for the target account
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
  
function menu($title, $type, $function = NULL, $args = array(), $accesses = '', $other = array()) {
  global $menuWeight;
  $menuWeight += !@$menuWeight ?: 1;
   
  if (function_exists($localFunction = "rCredits\\Web\\$function")) $function = $localFunction;
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $args,
//    'access callback' => TRUE,
    'access callback' => 'rCredits\\Web\\webAccess', // don't use be\access directly (rweb needs to remember $myid)
    'access arguments' => array($accesses),
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
    extract($sta['input']);
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
  u\loga($error_field ? 'say error' : 'say', compact('message'));
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
  global $base_url;
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $loggedInLinks = !r\acct() ? '' : <<<EOF
    <a href="$base_url/membership/contribute">Contribute</a> |
    <a href="$base_url/invite">Invite Someone</a> |
EOF;

  $markup =<<<EOF
<hr>
$loggedInLinks
<a href="http://rCredits.org">About rCredits</a> |
<a href="$base_url/membership/agreement">The Agreement</a> |
<a href="$base_url/help">Help</a>
<br><br>
website copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance&trade;</a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:info@rCredits.org" target="_blank">info@rCredits.org</a><br>
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

/**
 * Return true if the current account has all the accesses listed.
 */
function webAccess($accesses) {
  setupGlobals(); // remember who we are (the main reason for this function)
  if (!$mya = r\acct()) return FALSE;
  foreach (u\ray($accesses) as $one) if (!$mya->can(constant('B_' . strtoupper($one)))) return FALSE;
  return TRUE;
}

function css($sheet, $weight = -99) {
  global $rUrl;
  drupal_add_css("$rUrl/rweb/$sheet", array('group' => CSS_THEME) + weight($weight));
}

function js($what, $type, $scope) {
  global $rUrl;
  if ($type == 'file') $what = "$rUrl/$what";
  return drupal_add_js($what, compact('type', 'scope'));
}

function labeled($form) {
  $form['#attributes']['class'][] = 'labeled';
  foreach ($form as $key => $value) if (substr($key, 0, 1) != '#') {
//    if (!isset($value['#id'])) $form[$key]['#id'] = "edit-$key"; // is this necessary?
  }
  return $form;
}

/**
 * Check the amount, fix it in the input array, return any error.
 */
function amtErr($field, &$sta, $restriction = '', $fractionLimit = 2) {
  return u\badAmount($sta['input'][$field], $restriction, $fractionLimit);
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
function op($sta) {global $testOp; return t\EST() ? $testOp : $sta['clicked_button']['#parents'][0];}
