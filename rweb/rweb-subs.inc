<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;

/**
 * @file
 * rWeb subroutine file
 * Tools for rWeb forms.
 */

define('R_NONE', '<span class="none">--</span>'); // let the tests see null values
define('R_ON_FILE', t('(on file)'));
define('R_AGREE_0', -2); // number of the first point in the rCredits Agreement
define('R_AGREE_9', 6); // number of the last point in the rCredits Agreement

$GLOBALS[TX_WEB . ' messages'] = array(
//  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"
// Then if you have another account to create, use the "Create Another Account" button on the Summary page.

  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! We sent you an email with your user ID and password (if you don\'t see it within 60 seconds, check your spam folder). Come back here <b style="color:red;">within the next @R_SIGNIN_HOURS hours</b> to sign in and finish your account setup.</p>'),
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
  'step completed' => t('<span id="completed"> Step completed!</span>'), // space intended
  'return to membership' => t('<a href="@link">Click here to return to the checklist</a>.'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('A temporary password has been sent to your email address.'),
  'changed agent' => t('You successfully changed the agent to @agentName.'),
  'changed account' => t('You successfully changed the account to @accountName.'),
  'signed agreement' => t('You signed the rCredits Agreement on @date.'),
  'downloading' => t('Your download will begin shortly...'),
  'changed account' => t('You are now managing the account for @newAcct.'),
  'lost old changes' => t('Any unsaved changes to account @oldAcct have been discarded.'),
  'take a step' => t('Your account is <b style="color:red;">not yet activated</b>. <a href="status"><span id="nextstep">Take the next step!</span> Click here</a> to see where you are in your new account progress and what to do next.'),
  'got token' => t('Your rCredits and US Dollar accounts have been successfully connected!'),
  'company rcard' => t('Send a <a href="@href">company rCard</a> to @name at @address.'),
  'amount rounded' => t('"@amount" was rounded to the nearest integer value.'),
  'invite sent' => t('Your invitation has been sent.'),
  'repeat invite' => t('Come back any time, to send more invitations (click on "Invite Someone" at the bottom of any page).'),
  'gift transfer later' => t('Your funds will be transferred when you have money in your account.'),
  'repeat gift' => t('If you wish to make a contribution again sometime, click on "Membership" in the upper right corner, then on "Contribute".'),
  'setup complete' => t('Your account setup is complete. Expect an email within the next 24 hours, saying your account is activated. Thank you for joining!'),
  'ready to charge' => t('<p>You successfully scanned in as "@agentName" to charge customers on behalf of @companyName.</p><p>Now go back to your scanning app, to scan a customer\'s card. Sign out only when your shift ends.</p>'),
  'transaction list truncated' => t('There are too many transactions to show all at once, so only the first @TX_MAX_ROWS are shown (choose a different date range to see the rest).'),
  'address info' => t('<a1>Click here</a> for address information.'),
  'open dwolla' => t('Our partner organization, Dwolla, sent you a link in an email. Click on the link, sign in with your phone number and temporary password "@usdPass", then come back here once you have confirmed your phone number. (We are working to eliminate this step, but in the meantime, there is no way around it.)'),
  'agent created' => t('The agent account was successfully created.'),
  'phone verified' => t('Your phone number has been verified.'),

  // prompts
  'prompt min' => t('Keep this much in my rCredits account, for purchases or inflation-proof savings. (When your balance goes below this amount, the system refills it for you from your bank account. Refilling may take 3-5 days.)'),
//  'prompt max' => t('Keep AT MOST this much in my rCredits account. (When your rCredits balance goes above this amount by $@R_CHUNK or more, the system transfers the extra to your bank account. Blank means no automatic transfers &mdash; recommended in most cases).'),
  'fullName desc' => t('Your full legal name, properly capitalized.'),
  'verify connection' => t('Your next step is to verify the connection.'),
  'ssTitle' => t('Soc Sec #:'),
  'einTitle' => t('Federal ID:'),
  'ssDesc' => t('Your social security number (for required annual reporting)'),
  'einDesc' => t('The company\'s Employer ID Number (EIN)'),
  'to scan another' => t('To scan another card, run the QR Scanner app again.'),

  // confirm
  'yes or no' =>  '@yesNo', // to replace with yesNo(url)
  'confirm undo' => t('Undo @created transfer of @amount @tofrom @otherName?'),

  // errors
  'missing purpose' => t('<p>What purpose? For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc". Rebates and bonuses are intended as rewards for productive economic activity in rCredits.</p>
    <p>For everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field0' => t('@field field is required.'), // Drupal's standard
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no devices' => t('Surprisingly, this account has no related devices.'),
  'required field' => t('@field must not be blank.'),
  'duplicate email' => t('"@duplicateAccount" is already using that email address. If you want to use the same email address for this account, add a tag to the local part, for example "@emailTagged".'), // used in user.module
//  'duplicate email' => t('That email address is already taken by @duplicateAccount.'),
  'forgot password' => t('If you just forgot the password to the other account, <a href="@passwordLink">click here</a> to request a new password.'), // used in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad login' => t('That account does not exist. Check your typing?'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad account id' => t('That is not a proper account ID.'),
  'bad account number' => t('Your bank account number must be 3-17 digits long.'),
  'bad pin' => t('Your PIN must be exactly 4 digits'),
  'bad federal id' => t('That is not a correct @what. Try again.'),
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
//  'bad email' => t('The e-mail address <em class="placeholder">@email</em> is not valid.'), // must match Drupal, for tests
  'bad email' => t('That email address is not valid.'),
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner, employee, or contractor for the company? (please choose one or more)'),
  'shortr to' => t('You do not have enough rCredits, for that transaction. <a href="@how">Go here</a> to get more rCredits.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("@fullName").'),
  'bad invite' => t('To sign up for rCredits, you must be invited and you must use the invitation link (just once) within @R_INVITE_DAYS days.<br><br>If you already have an rCredits account set up and want to open an additional account for yourself, your business, or a family member, sign in to your personal account and use the "Open Another Account" on the Summary page.'),
  'min sub floor' => t('Your chosen rCredits minimum cannot be less than your <em>allowed</em> minimum (@floor).'),
//  'min too small' => t('You minimum (if above zero) cannot be less than $@R_BANK_MIN.'),
//  'bad maxr' => t('Your chosen rCredits maximum cannot be less than your rCredits minimum. Nor can it be less than your total amount of incentive rewards to date.'),
//  'bad maxusd' => t('Your chosen USD maximum cannot be less than your rCredits minimum.'),
  'info not saved' => t('We apologize. Your information could not be saved at this time. Please try again in an hour or so.'),
  'no page permission' => t('You do not have permission to visit this page. Return to your <@a>Summary</a> page.', array('@a' => 'a href="/"')),
  'specific nonpositive' => t('The amount (@amount) must be a positive integer.'),
  'insufficient relation' => t('You cannot open an account unless you have a closer relationship to it. Use the <a href="invite">invitation page</a> instead.'),
//  'humans sign' => t('You must be logged in to a personal account, to sign the agreement on behalf of a company.'),
  'bad spoof check' => t('You must confirm (<i>every time</i>) that you are on the real rCredits site.'),
//  'no auth yet' => t('You have not yet authorized a connection with your Dwolla account. Please resubmit.'),
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
//  'dw mismatch' => t('WARNING: The "@field" of your Dwolla account and your rCredits account are conflicting. Your account permissions will be restricted until you correct this mismatch.'),
  'proxy to go' => t('You have not yet completed your selection of proxies.'),
  'no stop draw' => t('The account for @fullName has overspent its rCredits, based on your balance. So you cannot discontinue "draw" permission without first bringing that account back up to even (for example by giving it some funds.'),
  'doubled proxy' => t('Your alternate proxy cannot be the same person as your first proxy.'),
  'po in location' => t('Your physical location address cannot be a Post Office box.'),
  'missing currency' => t('You must choose rCredits, USD, or both.'),
  'download always both' => t('Note: Downloads always include both rCredits and USD amounts.'),
  'sign in to scan' => t('You must <a href="/user/login">sign in</a> (or scan yourself in) BEFORE scanning a customer card.'),
  'change min first' => t('That request would leave you with less than your minimum balance. <a href="account/preferences">Lower your minimum</a> first.'),
  'bad member id' => t('That is not a valid member ID!'),
  'bad card' => t('That is not a valid member ID.'),
  'bad card code' => t('This rCard is not valid.'),
  'upload required' => t('You must upload a file.'),
  'missing contact info' => t("<h2>Missing Contact Information</h2><p>Please complete your <@a1>Contact Information</a> before continuing.</p>"),
  'correct and retry' => t('Please correct the problem and resubmit this form.'),
  'duplicate transaction' => t('You already just @op that member that much.'),
  'duplicate phone' => t('That phone number is already the primary phone for another account.'),
  'bad pin' => t('That is not your correct PIN.'),
  'wrong pin len' => t('Your PIN must be exactly 4 characters long.'),
  'pin required' => t('You must choose a PIN.'),
  'only us banks' => t('Unfortunately, at this time, we can handle only U.S. banks.'),
  'bad deposit cents' => t('Each deposit is a number less than @DW_VERIFY_ROOF.'),
  'no can scan' => t('You do not have permission to scan an individual\'s rCard.'),
  'must opt' => t('You must not create a limited account for this person without giving him or her the opportunity to open a real rCredits account. <@a>Invite them</a>!', array('@a' => 'a href=invite')),
  'must agree' => t('Accept Terms checkbox field is required.'), // drupal shows this
  'redo info' => t('We could not verify your personal information. Please try again, carefully.'),
  'bad ssn or dob' => t('There is a problem verifying your personal information.'),
  'bank too little' => t('The amount you asked to transfer is less than the minimum ($@R_BANK_MIN).'),
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
  if ($href) { // href might be http or https or /...
    if (substr($href, 0, 4) == 'http' or substr($href, 0, 1) == '/') $href = "document.location.href='$href';";
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
function item($markup, $title = '', $desc = '', $other = array()) {return formField('item', $title, $desc, compact('markup') + $other);}
function hidField($value = '', $options = array()) {return formField('hidden', '', '', compact('value') + $options);}
function textField($title = '', $description = '', $other = array(), $options = NULL) {
  return formField('textfield', $title, $description, $other, $options);
}
function boxField($title, $desc = '', $other = NULL) {
  return formField('checkbox', $title, '<div class="box"></div>' . $desc, $other);
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
function boxesField($tag, $title, $desc, $defaults, $choices) {
  $boxes = '';
  if (!$defaults) $defaults = array();
  foreach ($choices as $key => $value) {
    $checked = in_array($value, $defaults) ? ' CHECKED' : '';
    $boxes .= <<<EOF
    <div class="form-item form-type-checkbox form-item-$tag-$key">
       <input type="checkbox" name="{$tag}[$key]" value="$key"$checked class="form-checkbox" />
       <label class="option" for="edit-$tag-$key"><div class="box"></div><div>$value</div></label>
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

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Also sets up $formArray and $formSta, for testing.
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

    $args = func_get_args(); // this and next few lines are from \drupal_get_form()
    array_shift($args); // Remove $function from the arguments.
    $formSta['build_info'] = compact('args');
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
      if (@$form['country']) {
        js('inc/countries.js', 'file', 'header');
        js("print_country(0, 0);", 'inline', 'footer'); // I don't know why, but this IS NEEDED on input errors
      }
      if (u\isIPhone()) css('iphone.css', -100);
    }

    $formArray = $form2 = $form; $rent = \render($form2);
//    if ($function == 'Usd') $rent = str_replace('Dwolla', '<a href="' . R_DW_LINK . '" target="_blank">Dwolla</a>', $rent);
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

function box() {
  if ($box = @$_COOKIE['box']) return $box;
  \variable_set('r_box', $box = \variable_get('r_box', 0) + 1); // this is prone to timing collisions
  return r\setCook('box', $box);
}

function boxUser($boxid, $uid) {
  if ($boxUser = @$_COOKIE[$cookid = 'box' . $uid]) return $boxUser;
  $boxName = db\lookup('COUNT(*)', 'r_boxes', 'uid=:uid', compact('uid'));
  $id = db\insert('r_boxes', u\ray('boxid uid boxName created', $boxid, $uid, $boxName + 1, REQUEST_TIME));
  r\setCook($cookid, $id);
}

function setupGlobals($agent = '') {
  global $user, $channel, $boxUser;
  if (@$channel) return; // already setup
  if (!$agent) $agent = @$user->uid; // register_submit supplies agent (no one else does)
  $myid = svar('myid') ?: $agent;
  r\acct::setDefault(r\acct($myid, $agent)); // set default for r\acct()
  if ($myid) $boxUser = boxUser(box(), $myid);
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
 * Go to the next USD setup page (return the page if testing) or return FALSE.
 * @param string $step: (returned) the next Dwolla setup step
 * @return (if testing) a page to display as appropriate, else FALSE
 */
function setupUs(&$step) {
  $us = new r\usd($mya = r\acct());
  if (!@$mya->secure['auth']) {
    if (!$mya->createUsdAccount($err)) return say($err, 'zot'); // set auth
//    if ($mya->secure['usdEmail'] == $mya->mail) r\Web\say('Dwolla has sent you an email with a verification link. Please click on that link to confirm your email address, then come back here to continue setting up your account.'); // temporary, for Dwolla compliance testing
  }
  if (!$mya->usdAccount) if (!$us->me($err)) return say($err, 'zot'); // set usdAccount
  if (!$step = $us->step($err) or $step == 'PhotoId') {
    r\tellStaff('null step', compact('err', 'step') + (array) $mya);
    if (u\abbreviates('Invalid account status', $err)) { // sometimes "...for user of this access token"
      //$mya->update(u\ray('federalId', '')); // prevent resumission of same ssn (temporary Dwolla bug)
      return say('bad ssn or dob|Please contact rCredits customer support at +1 413 628 1723.', 'zot');
    } else return say($err, 'zot');
  }
  if ($step == 'Email') return FALSE; // await automatic or manual confirmation of email (meanwhile user can sign Agreement)
  if ($step == 'Finished') return FALSE;
  if ($mya->phone and $step == 'Phone') return addPhone();
  if ($step == 'AccountInfo') return r\go('account/basic', 'redo info', 'zot'); // here on ssn() failure
  if ($step == 'Kba') return r\go('account/kba'); // here on accountInfo() failure

  if ($mya->address and $step == 'Address') {
    if (!$us->address($mya->address, $mya->city, r\realState($mya->state, $mya->country), $mya->postalCode, $err)) return r\go('account/contact', $err);
  } elseif ($step == 'Ssn') {
    if (!$us->ssn($mya->federalId, $err)) {
      if ($err) return say($err, 'zot');
    } else u\EXPECT($us->step() != $step, 'weird ssn return');
  } elseif ($step == 'AuthorizedRepresentative') {
    u\EXPECT(!$mya->person(), 'verify rep for person?!');
    $by = r\acct($mya->data['by']);
    u\EXPECT($by, 'no by for non-person');
    list ($first, $last) = u\parseName($by->fullName);
    $state = r\realState($by->state, $by->country);
    if (!$us->authorizedRep($first, $last, $by->address, $by->city, $state, $by->postalCode, substr($by->phone, 2), $by->dob, $by->federalId, $err)) return say($err, 'zot');
  }
  if ($us->step() != $step) return setupUs($step); // step may have progressed
}


function divButton($href, $title) {
  global $base_url, $rUrl;
  if (strpos($href, 'http') === FALSE) $href = "$base_url/$href";
  return  <<<EOF
<a href="$href" title="$title"><div></div></a>
EOF;
}

function imageButton($href, $title, $src) {
  global $base_url, $rUrl;
  if (strpos($href, 'http') === FALSE) $href = "$base_url/$href";
  if (strpos($src, 'http') === FALSE) $src = "$rUrl/images/icons/$src";
  return  <<<EOF
<a href="$href" title="$title"><img src="$src" border="0" /></a>
EOF;
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
 * Undo the last transaction.
 */
function undo($confirmed = FALSE) {
  if (!$xid = svar('lastXid')) u\EXPECT(!$confirmed, 'missing last xid');
  
  if ($confirmed) {
    list ($message, $args) = be\undoTx($xid);
    say($message, $args);
    if (@$args['success'] and svar('lastXid') == $xid) svar('lastXid', FALSE);
  } elseif ($confirmed === '0') {
    say('nothing undone');
  } else {
    $tx = $xid ? be\lastTx('xid=:xid', compact('xid')) : be\lastTx('box=:box', u\ray('box', box()));
    $tx['yesNo'] = yesNo('undo/1', 'undo/0');
    if ($tx) say('confirm undo|yes or no', r\txReportArgs(r\acct(), $tx));
  }
  r\go('empty'); // blank backdrop for result message
}

/**
 * Show remaining customer balance.
 */
function remainder() {
  if ($uid = svar('lastCustomer')) {
    $a = r\acct($uid);
    $otherName = $a->fullName;
    $balance = be\creditInfo(compact('uid'))->fancy['balance'];
    say('customer balance', compact('otherName', 'balance'));
  } else say('no last customer', 'zot');
  r\go('empty');
}

function boxOpt($field, $opts, $sta) {
//debug(compact('field','opts','sta'));
  foreach ((@$sta['input'][$field] ?: array()) as $key => $value) $result[] = $opts[$key]; // note that key==value
  return @$result ?: array();
}

/**
 * Return the fields used to verify an individual's SSN (also asked for in signup).
 * The dob field is not used for companies (but remains in the form, unseen), so it cannot be required.
 */
function ssnFields() {
  $ssTitle = tt('ssTitle');
  $federalId = textField("<span class=\"title\">$ssTitle</span>", tt('ssDesc'), required()); // textfield so user doesn't have to retype on error
  $dob = textField(t('Birthdate:') . R_REQUIRED, t('Your date of birth (mm/dd/yyyy)')); // don't use "required()" here
  return compact(u\ray('federalId dob'));
}

function dwollaTerms() {
  global $rUrl;
  $title = item(t('Welcome to rCredits!'));
  $subtext = item(t(<<<EOF
<p>Your rCredits account will contain both official currency and rCredits (incentive rewards we issue by agreement among ourselves, as members of the rCredits system).</p>

<p>Our partnership with Dwolla connects you to the mainstream economy. We use the Dwolla network to handle transfers of official currency between rCredits members. A few required words about Dwolla:</p>
<div id="graphics"><div><img src="$rUrl/images/dwolla.jpg" /><img id="veridian-logo" src="$rUrl/images/veridian.jpg" /></div></div>
<p>Financial institutions play an important role in the Dwolla network.</p>

<p>Dwolla, Inc. is an agent of Veridian Credit Union and all funds associated with your account in the Dwolla network are held in a pooled account at Veridian Credit Union. These funds are not eligible for individual insurance, and may not be eligible for share insurance by the National Credit Union Share Insurance Fund. Dwolla, Inc. is the operator of a software platform that communicates user instructions for funds transfers to Veridian Credit Union.</p>
EOF
  ));

  $aTerms = 'a href="https://www.dwolla.com/tos?nolayout=true" target="_blank"';
  $aPrivacy = 'a href="https://www.dwolla.com/privacy?nolayout=true" target="_blank"';

  $agreed = boxField(t('Accept Terms checkbox'), tt("I accept Dwolla's <@aTerms>Terms of Service</a> and <@aPrivacy>Privacy Policy</a> and I authorize the rCredits system to open and manage a Dwolla account on my behalf.", compact('aTerms', 'aPrivacy')), suffix('&nbsp;') + required());
  $continue = submit(t('Continue'));
  return compact(u\ray('title subtext agreed continue'));
}

function wants($ats) {  
  $wants = 0; // store a bit for each bought-at company
  if ($ats) foreach ($ats as $one) if (@$one) {
    db\q('UPDATE r_nonmembers SET potential = potential+1 WHERE id=:one', compact('one'));
    $wants |= u\bit($one);
  }
  return $wants;
}

function acctType($acctType) {  
  $flags = B_DFT
    | ($acctType <= R_SELF_EMPLOYED ? u\bit(B_PERSON) : 0)
    | ($acctType >= R_SELF_EMPLOYED ? u\bit(B_COMPANY) : 0);
    
  if ($acctType >= R_SELF_EMPLOYED) { // company
    $type = $GLOBALS['account types'][$acctType];
    $coFlags =
        ($acctType == R_SELF_EMPLOYED ? u\bit(R_SELF_EMPLOYED)
      : (strpos($type, 'nonprofit') !== FALSE ? u\bit(CO_NONPROFIT)
      : (strpos($type, 'corporation') !== FALSE ? (u\bit(CO_CORPORATION) | u\bit(CO_PUBLICLY_TRADED)) : 0)));
  }
  return compact(u\ray('flags coFlags'));
}

/**
 * Create relation for company relationship specified in registration form (if any)  
 */
function suCompanyRelation($company, $other, $data) {
  extract(u\just('companyPhone isOwner employeeOk contractor', $data));
  $isOwner = (int) @$isOwner; // POSTed as "isOwner" when TRUE
  $employeeOk = (int) (@$employeeOk or @$contractor);
  if ($main = findCompany($company, @$companyPhone)) { // company account exists
    db\insert('r_relations', compact(u\ray('main other employeeOk isOwner')));
  }
}

/**
 * Set up relationship of this new account with the account that created it (if any).
 */
function suCreatorRelation($args, $myid) {
  extract($params = u\just('isOwner employeeOk flow by', $args));

  if (!$error = u\EXPECT($params, 'int int int id', TRUE)) { // if not a hack attempt
    list ($main, $other, $employerOk, $permission) = array($myid, $by, $employeeOk, r\perm(B_MANAGE));
    $draw = ($flow & 2) ? 1 : 0;
    db\insert('r_relations', compact(u\ray('main other employerOk employeeOk isOwner draw permission')));
    if ($flow & 1) {
      list ($main, $other, $draw) = array($other, $main, TRUE);
      db\insert('r_relations', compact(u\ray('main other draw')));
    }
  } else r\tellStaff("bad params in formSignup: $error -- ", $params);
} 

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $mail: the email address to check (also returned with improvements)
 * @param int $uid: account record id (optional), if changing email address rather than creating new account
 * @return: TRUE if email is okay to use, else FALSE
 */
function emailOkay(&$mail, $uid = '') {
  $email = str_replace("'", '&#039;', $mail);
  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'mail');
  if ($duplicateAccount = db\lookup('fullName', 'users', 'mail=:email and uid<>:uid', compact('email', 'uid'))) {
    $passwordLink = \url("password/$email");
    $emailTagged = str_replace('@', '+whatever@', $mail);
    if (!$uid) $forgot = '|forgot password';
    return say('duplicate email' . @$forgot, compact(u\ray('emailTagged duplicateAccount passwordLink')), 'mail');
  }
  $mail = $email;
  return TRUE;
}

/**
 * Tell the staff about a new member.
 */
function tellStaffNewMember($params) {
  extract($params);
  $phone = u\fmtPhone(@$phone);
  $country = r\realCountry($country);
  $state = @$state ? r\realState($state) : 'state lookup failed';
  $acctType = $GLOBALS['account types'][$acctType];
  $by = @$by ? r\acct($by)->fullName : 'unknown';
  $copts = u\jsonEncode(@$copts);
  $ats = @join(',', array_values($ats));
  $params = compact(u\ray('phone country state acctType by copts ats')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  r\tellStaff($message, $params);
}

function findCompany($company, $companyPhone) {
  $companyShortName = u\shortName($company);
  $companyUid = db\lookup('uid', 'users', ':IS_COMPANY AND phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = db\lookup('uid', 'users', ':IS_COMPANY AND name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = db\lookup('uid', 'users', ':IS_COMPANY AND name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Return the part of the form that is common to formContact and formSignup
 * @param assoc $defaults: initial field values (defaults to current values)
 * include all fields (for Contact form) only if r\acct() is set
 */
function contactFields($defaults = array()) {
  $country = $state = ''; // make sure these are not NULL
  $fields = 'phone verifyBy faxetc country address city state postalCode';
  $mya = r\acct();
  extract($mya ? (array) $mya->account($fields) : $defaults);

  $onchange = "print_state(this.options[this.selectedIndex].value,'');";
  $countryAttrib = attrib(compact('onchange'));
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');

  $onchange = <<<EOF
var country = jQuery('#edit-country').val();
var state = jQuery('#edit-state').val();
var city = jQuery('#edit-city').val();
var address = jQuery('#edit-address').val();
var postalCode = jQuery('#edit-postalcode').val();
var postalAddr = jQuery('#edit-postaladdr');
if (postalAddr.val() == '') {
  postalAddr.val(address + ', ' + city + ', ' + ss[country][state] + ' ' + postalCode);
} // postalAddr.attr('selectionEnd',0); fails
EOF;
  $addressAttrib = attrib(compact('onchange'));

  $onchange = "var z3=jQuery('#edit-postalcode').val().substr(0, 3); if (z3=='013' || z3=='010') jQuery('#edit-buysat').show();";
  $zipAttrib = attrib(compact('onchange'));
  
  $phone = textField(t('Phone:'), t('Your primary phone (landline or cell). Cannot be the primary phone for another account.'), required(@$phone ? u\fmtPhone($phone): ''));
  $dft = ($mya and isset($mya->data['verifyBy'])) ? dft(@$mya->data['verifyBy']) : NULL;
  $verifyBy = formField('radios', t('Verify by:'), t('We will send a verification code to your phone, by text or voice.'), $dft, array(t('Text message'), t('Voice')));
  if ($mya) $faxetc = textField(t('Fax etc:'), t('List other ways to contact you: fax, cell, IM, etc. This field is NOT public.'), dft(@$faxetc));
  $country = formField('select', t('Country:'), t(''), dft(@$country) + $countryAttrib); // can't use required() here
  if ($mya) $address = textField(t('Street Address:'), t('Where is your building located (physical address)?'), required(@$address) + $addressAttrib);
  if ($mya) $city = textField(t('City:'), t(''), required(@$city));
  if ($mya) $state = formField('select', t('State:'), t(''), dft(@$state)); // can't use required() here
  $postalCode = textField(t('Postal code:'), t('Postal code for your <em>physical location</em>'), required(@$postalCode) + $zipAttrib);

  return compact(u\ray($fields));
}  

function phoneOk(&$phone, $country) {
  $myid = @r\acct()->id ?: 0;
  if (!$phone = u\fmtPhone($phone, '+n')) return say('bad phone', 'phone');
  if ($country == US_COUNTRY_ID and strlen($phone) != 12) return say('bad phone', 'phone');
  if (db\exists('users', 'phone=:phone and uid<>:myid', compact('phone', 'myid'))) return say('duplicate phone', 'phone');
  return TRUE;
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function adminField($label) {
  $mya = r\acct();
  return ($mya->can(B_CTTY_ADMIN) and $mya->usdAccount) ? item(R_ON_FILE, $label) : NULL;
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
      $result = u\fmtDate($mya->$field);
    } elseif (strpos(R_SECRET_FIELDS . ' usdAccount', $field) !== FALSE) {
      $result = $mya->$field;
    }
  } else $result = "<a href=\"$base_url/account/security/show=$field\">show</a>";
  return "[$result] ";
}

function onFile($field, $sta) {
  return (@$sta['complete form'][$field]['#value'] == R_ON_FILE and @$sta['input'][$field] == R_ON_FILE);
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
  if (!$result = db\lookup('email,inviter', 'r_invites', $sql, compact('code', 'lastweek'))) return FALSE;
  extract($result);
  if (@$invitee) {
    db\update('r_invites', compact('code', 'invitee'), 'code');
  } else {
    $invitee = $inviter;
    return $email;
  }
}

function passwordOkay($a, $password, $passFieldName = 'current_pass') {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  $stored_hash = $a->pass;
  $hash = \_password_crypt('sha512', $password, $stored_hash);
  if (@$a->data['oneTimePass']) extract(u\just('pass expires', $a->data['oneTimePass']));
  if ($password == @$pass) return REQUEST_TIME < @$expires ? TRUE : say('pass expired', $passFieldName);
  return ($hash and $stored_hash == $hash) ? TRUE : say('wrong pass', $passFieldName);
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
    INNER JOIN r_companies c ON c.uid=u.uid
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE :IS_COMPANY AND u.$field LIKE :region AND (u.fullName LIKE :which OR i.industry LIKE :which OR c.description LIKE :which)
    ORDER BY industry, u.fullName
EOF;
  $result = db\q($sql, compact(u\ray('region which')));
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

/**
 * Sign the user in by their scanning their own company card
 */
function scanIn($acct) {
  $mya = $acct;
  svar('myid', $mya->id);

  // Tell Drupal who is now logged in
  global $user; $user = $mya->agent->account();
  $user->roles = array();
  $user->uid = $mya->agent->id;
  drupal_session_regenerate();

  setupGlobals();
  svar('scanned_in', TRUE);
  $agentName = $mya->agent->fullName;
  $companyName = $mya->fullName;
  $subs = compact('agentName', 'companyName');
  u\loga('login', $subs, $mya->agentId);
  return r\go('empty', 'ready to charge', $subs);
}

function hack($message, $info = array()) {
  u\loga('hack', $_SERVER, $message);
  return r\go(r\acct() ? '/user/logout' : '');
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

function fundRpt($now, $then, $type = '') {
  //$up = $then ? number_format(abs($now / $then - 1) * 100, 1) : '100';
  $RUS = strpos($type, 'r') === FALSE ? '' : '@R';
  $change = u\fmtAmt(abs($now - $then));
  $way = $now >= $then ? t('up') : t('down');
  $s = u\fmtAmt($now) . tt("$RUS &mdash; @way @change$RUS from a month ago", compact('way', 'change'));
  return strpos($type, '$') === FALSE ? str_replace('$', '', $s) : $s;
}

function txsRpt($amt, $count, $accts, $type) {
  $type = $type == 'p' ? t('personal') : ($type == 'b' ? t('company') : '');
//  list ($totaling, $account, $month, $per, $avg) = array(t('totaling'), t('account'), t('mo'), t('per'), t('avg'));
  list ($acct, $month) = array(t('acct'), t('mo'));
  list ($amt, $perAcct) = array(u\fmtAmt($amt), u\fmtAmt($amt / $accts));
  return "$count ($amt) / $month = $perAcct / $acct";
}

/**
 * Return markup to identify the agent of the given account.
 */
function identifiers($acct) {
  $mya = r\acct();
  list ($agent, $company) = $acct->proSe() ? array($acct, '') : array(r\acct($acct->agentId), $acct->fullName);
  $location = r\location($acct);
  $pic = ($mya and $mya->co()) ? $acct->agent->picture() : ''; // let companies see the picture
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
  global $testConfirmation; $testConfirmation = $msg;
//  t\output(preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($msg)), 'screen');
  return ($sta['confirm'] = $msg);
}

/**
 * Replacement for \confirm_form: ask a confirmation question, with choices Okay or Cancel
 * @param assoc $sta: the form's status array
 *   'confirm': the question to 
 * @param string $title: header for the question (normally none)
 * @return a confirmation form (FALSE if there is no question to ask)
 */
function sureForm(&$sta, $title = '') {
  if (!@$sta['confirm']) return FALSE;
//  if (!isset($sta['confirm'])) return FALSE;
  
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
 * @param string $op: pay, charge, grant, loan, fine, or invest
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($op, $info, $confirmed) {
  global $lastXid, $scanned, $lastOp;
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\just('who amount goods purpose scanned', $info)); // $scanned is global, for be\transfer
  $txType = $op == 'pay' ? 'payment' : strtolower($op);
  if (blank_field(compact(u\ray('who amount')))) return FALSE;
  if (!$acct = whois($who, 'who', $info)) return FALSE;
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
  svar('lastCustomer', $acct->id);
  $lastOp = array($txType, $acct->id, $amount); 
  $op = $op == 'pay' ? t('paid') : ($op == 'charge' ? 'charged' : $op . 'ed'); // for possible error message
  if ($lastOp == svar('lastOp')) return say('duplicate transaction', compact('op'), 'who');
  list ($msg, $args, $confirm) = in_array($txType, array('payment', 'charge')) ?
       be\transfer($txType, $mya, $acct, $amount, $purpose, $goods and !u\forCash($purpose), $confirmed)
     : communityTransfer($txType, $acct, $amount, $purpose, $confirmed);

  if (FALSE and !@$scanned) { // not scanned -- We need to show identifying photo, name, etc. for the target account
    $identifiers = identifiers($acct);
    $identifiers = "<div id='identifiers' class='clearfix'>$identifiers</div identifiers>";
  }
  if ($confirm) return @$identifiers . tt('confirm ' . $msg, $args);
  if (!@$args['success']) return say($msg, $args, 'amount');

  svar('lastXid', $lastXid); // remember identity of most recent transaction
  svar('lastOp', $lastOp); // remember nature of last op
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
  $success = (bool) be\fund($acct->id, u\consta('tx', $type), $amount, $purpose, $confirmed);
  $msg = $success ? ($confirmed ? 'report funding' : 'funding') : 'funding error';
  $confirmAction = ucwords($type);
  $action = $confirmAction . 'ed';
  $otherName = r\userField('fullName', $acct->id);
  $amount = u\fmtAmt($amount);
  return array($msg, compact(u\ray('success action confirmAction amount otherName')), $success and !$confirmed);
}  

/**
 * Complete a "no signin required" request (made by the user clicking on a link we sent).
 * @see also r\makeDo()
 */
function rdo($args) {
  extract(u\just('id code', $args));
  if (!$info = db\lookup('action', 'r_do', 'doid=:id AND code=:code AND (completed=0 OR :REQUEST_TIME<-completed)', compact('id', 'code'))) doSay(t('That link has expired.'));
  if (!$info = unserialize($info)) doSay(t());
  
  $DBTX = \db_transaction();
  extract(u\just('type uid sql subs amount', $info));

  if ($type == 'min') {
    $result = r\acct($uid)->update(u\ray('minimum', $amount));
  } elseif ($type == 'sql') {
    $result = db\q($sql, $subs);
  } else doSay(t('bad rdo'));

  if (!$result) doSay(t('Request failed.'));
  db\update('r_do', u\ray('doid box completed', $id, box(), REQUEST_TIME), 'doid');
  unset($DBTX);

  doSay(t('Your request was successfully completed.'), 'status');
}

/**
 * Output the No-signin result message and terminate.
 */
function doSay($msg, $type = 'error') {
  $template = file_get_contents(DRUPAL_ROOT . R_PATH . '/templates/do.html');
  $map = u\ray('TITLE BASE_URL VERSION MSGTYPE MESSAGE SUBTEXT',
    t('No-signin Action ') . ($type == 'error' ? t('ERROR') : t('Success!')),
    BASE_URL, R_VERSION, $type, $msg,
    t('No sign-in was required for this action (rCredits will never ask you to click on a link then sign in).')
  );
  exit(strtr($template, $map));
}

/**

  switch (@$channel) {
    case TX_WEB:
      \drupal_set_message($msg, 'error'); // this fails
      \drupal_goto('error/' . $R_FATAL_ERROR); // this fails -- use shutdown function to show error page (getting message from log)
    case TX_SMS: if (@$rsms_number) \sms_send($rsms_number, R_SYS_ERR); break;
    case TX_SMART: be\api_error(R_SYS_ERR);
    default: die('with no channel'); // (keep) definitely do the print_r first
  }
 */
function error($logid) {
  if ($info0 = db\lookup('info', 'r_log', 'logid=:logid', compact('logid'))) {
    if (!$info = json_decode(utf8_encode($info0))) $info = $info0; // probably too big for json
  }
  r\go('/user/login/err=' . R_SYS_ERR);
}

function usdCallback($type, $uid) {
  include_once __DIR__ . '/../usd.class';  
  return r\usd::$type($uid);
}
  
function menu($title, $type, $function = NULL, $args = array(), $accesses = '', $other = array()) {
  global $menuWeight;
  $menuWeight += !@$menuWeight ?: 1;
   
  if (function_exists($localFunction = "rCredits\\Web\\$function")) $function = $localFunction;
  $smart = strpos($function, '\\Smart\\');
  
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
    'module' => $smart ? 'rsmart' : 'rweb',
    'file' => $smart ? '../rsmart/rsmart.inc' : 'rweb.inc',
    'options' => u\ray('class', $title),
  ), $other); // $other overwrites, if there is a conflict
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
function say($index, $args = array(), $errField = '') {
  global $fieldErr; // used mostly in tests (and in formTx)
  if (!@$index) return;
///  if (is_array($index) and !isset($index[0])) debug($index);
  if (is_array($index)) list ($index, $args, $errField) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $errField) = array($errField, $args); // allow either order, for easy 2-param calls
  $message = tt($index, $args);
  $fieldErr = (bool) $errField;
  if ($fieldErr) \form_set_error($errField, $message); else \drupal_set_message($message);
  u\loga($fieldErr ? 'say error' : 'say', compact('message'));
  return FALSE;
}

function userMenuItem($router_path, $link_title) {
  $menu_name = 'user-menu';
  $link_path = $router_path;
  if (db\exists('{menu_links}', 'menu_name=:menu_name AND link_path=:link_path', compact('menu_name', 'link_path'))) return; // done
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

/**
 * Automatically deactivate an account (called when the user changes the Dwolla email or password).
 */
function coup($usdEmail) {
  list ($email) = explode('@', $usdEmail);
  list ($code, $region, $id) = explode('.', $email);
  if (!$a = r\acct($quid = strtoupper("$region.$id"))) die('no1');
  if (@$a->secure['usdEmail'] != $usdEmail) die('no2');
  if ($a->usd > 0) { // store its USD in ctty fund
    if (!r\tradeR(min(R_MAX_DAILY_EXCHANGE, $a->usd), $a->community, $a->id, $err) or $a->usd > R_MAX_DAILY_EXCHANGE) {
      r\tellStaff("Immediate coup threat: $a-fullName $$a->usd ($err)");
    }
  }
  $a->setBit(B_OK, FALSE);
  $notes = u\fmtDate() . ': account suspended due to changed Dwolla PIN, email, and/or password.' . @$a->notes . "\n";
  $a->update(compact('notes'));
  r\tellStaff('account couped', compact('quid'));
  exit('ok');
}

/**
 * Return true if the current account has all the accesses listed.
 */
function webAccess($accesses) {
  setupGlobals(); // remember who we are (the main reason for this function)
  if (!$mya = r\acct()) return FALSE;
  foreach (u\ray($accesses) as $one) if (!$mya->can(u\consta('b', $one))) return FALSE;
  return TRUE;
}

function css($sheet, $weight = -99) {
  global $rUrl, $base_url;
  $path = str_replace("$base_url/", '', $rUrl);
  \drupal_add_css("$path/rweb/$sheet", array('group' => CSS_THEME) + weight($weight));
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
function attrib($attributes) {return compact('attributes');}
function suffix($suffix = '') {$suffix .= '<div class="clearfix"></div>'; return compact('suffix');}
function prefix($prefix) {return compact('prefix');}
function autocomplete($type = '') {return array('autocomplete_path' => "autocomplete/$type/" . @r\acct()->id);}
function required($dft = NULL) {return array('required' => TRUE) + (isset($dft) ? dft($dft) : array());}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {global $testOp; return t\est() ? $testOp : $sta['clicked_button']['#parents'][0];}
