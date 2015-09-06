<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Web as w;

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
  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Mobile number @number is now connected to this account.'),
  'report delete cell' => t('Mobile number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>So far so good. <b class="stopnot">But WAIT!</b> You have a few steps still to go. We sent you an email with your user ID and a link to the next step. If you don\'t see it within 60 seconds, check your spam folder.</p>'),
  'company is ready' => t(<<<EOF
    <p></p><p>After you finish setting up this account, to manage it in the future:</p>
    <ol>
      <li>Sign in to your personal account</li>
      <li>Choose your company account from the account-selection dropdown box in the upper right corner of the screen.</li>
    </ol>
EOF
  ),
  'verify cell' => t('We sent a verification code to your mobile phone (@number).<br>Please type that code in the box below.'),
  'options saved' => t('Your options have been saved'),
  'photo saved' => t('Your photo was successfully uploaded.'),
  'step completed' => t('<span id="completed"> Step completed!</span>'), // space intended
  'after emailing' => t('After you have sent us an email: '),
  'return to membership' => t('<a href="@BASE_URL/status">Click here to return to the checklist</a>.'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('A link has been emailed to you, to choose a new password.'),
  'changed agent' => t('You successfully changed the agent to @agentName.'),
  'changed account' => t('You successfully changed the account to @accountName.'),
  'signed agreement' => t('You signed the rCredits Agreement on @date.'),
  'downloading' => t('Your download will begin shortly...'),
  'changed account' => t('You are now managing the account for @newAcct.'),
  'lost old changes' => t('Any unsaved changes to account @oldAcct have been discarded.'),
  'take a step' => t('Your account is <b class="stopnot">not yet activated.</b> <a href="@BASE_URL/status" class="nextstep">Click here for the next step!</a>.'),
  'got token' => t('Your rCredits and US Dollar accounts have been successfully connected!'),
  'company rcard' => t('Send a <a href="@href">company rCard</a> for @otherName to @manager at @address.'),
  'amount rounded' => t('"@amount" was rounded to the nearest integer value.'),
  'invite sent' => t('Your invitation has been sent.'),
  'repeat invite' => t('Come back any time, to send more invitations (click "Invite Someone" at the bottom of any page).'),
  'gift transfer later' => t('Your funds will be transferred when you have money in your account.'),
  'repeat gift' => t('If you wish to make a donation again sometime, click "Membership" in the upper right corner, then on "Donate".'),
  'setup complete' => t('Your account setup is complete. Expect an email within the next 24 hours, saying your account is activated. Thank you for joining!'),
  //  Meanwhile, click the "<a href="@BASE_URL/community/invite">Invite</a>" tab to invite anyone you know and trust &mdash; you get a $@R_HELPER_BONUSr reward when they make their first purchase.<br><br>
  'ready to charge' => t('<p>You successfully scanned in as "@agentName" to charge customers on behalf of @companyName.</p><p>Now go back to your scanning app, to scan a customer\'s card. Sign out only when your shift ends.</p>'),
  'transaction list truncated' => t('There are too many transactions to show all at once, so only the first @TX_MAX_ROWS are shown (choose a different date range to see the rest).'),
  'address info' => t('<a1>Click here</a> for address information.'),
  'open dwolla' => t('Our partner organization, Dwolla, sent you a link in an email. Click the link, sign in with your phone number and temporary password "@usdPass", then come back here once you have confirmed your phone number. (We are working to eliminate this step, but in the meantime, there is no way around it.)'),
  'agent created' => t('The agent account was successfully created.'),
  'phone verified' => t('Your phone number has been verified.'),
  'id verification pending' => t('Your photo ID has been uploaded successfully. Now we need to wait for our partner organization to verify your identity. Please feel free to email us if you have questions.'),
//  'adjust settings' => t('<b>TIP:</b> You can adjust your settings by clicking the gear in the upper right corner. '),
  'payroll done' => t('Payroll upload was successful. You paid @count employees a total of @total.'),
  'tx edited' => t('Your transaction #@tid was edited by @who.'),
  'new tx goods' => t('It is now for @what.'),
  'new tx amount' => t('The new amount is @amount.'),
  'min zeroed' => t('Your minimum balance has been set to zero, to disable automatic refills of your account (automatic refills are not possible without a connected bank account OR permission to draw on another rCredits account).'),

  // prompts
  'note' => '<b class="loud">' . t('NOTE:') . '</b>',
  'ach warning' => t('If there are insufficient funds in your bank account when you request an exchange for rCredits (either explicitly, or automatically by choosing a "minimum"), your bank will charge you a $20-35 fee, just like for a bounced check. If this happens more than once, the rCredits system will <i>also</i> charge you $@fee, to cover <i>its</i> bank fee. So make sure you have enough in your bank account to cover your transfers.', ['@fee'=>R_DEPOSIT_RETURN_FEE]),
  // If you don\'t want any automation, type zero (then remember to refill your account explicitly when it gets low).'),
//  'prompt max' => t('Keep AT MOST this much in my rCredits account. (When your rCredits balance goes above this amount by $@R_CHUNK or more, the system transfers the extra to your bank account. Blank means no automatic transfers &mdash; recommended in most cases).'),
  'legalname desc' => t(' full legal name, properly capitalized, as on your government-issued ID.'),
  'usename desc' => t('This should be the name you use in formal letters, at work, on your checks and credit cards, etc.'),
  'ssTitle' => t('Soc Sec #:'),
  'einTitle' => t('Federal ID:'),
  'ssDesc' => t('Your social security number (for tax reporting)'),
  'einDesc' => t('The company\'s Employer ID Number (EIN)'),
  'to scan another' => t('To scan another card, run the QR Scanner app again.'),
  'invite links' => t('<p>Here are your links, to include in your own email messages to your friends. Be sure to mention they can visit rCredits.org for more information.</p><p>@links</p>'),

  // confirm
  'yes or no' =>  '@yesNo', // to replace with yesNo(url)

  // errors
  'missing purpose' => t('<p>What purpose? For buying or selling actual goods and services, you must include a description. Rebates and bonuses are intended as rewards for productive economic activity in rCredits.</p>
    <p>For everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field0' => t('@field field is required.'), // Drupal's standard
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no invoices' => t('There are no such invoices in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no devices' => t('Surprisingly, this account has no related devices.'),
  'required field' => t('@field must not be blank.'),
  'duplicate email' => t('"@who" is already using that email address. If you want to use the same email address for this account, add a tag to the local part, for example "@emailTagged".'), // used in user.module
  'forgot password' => t('If you just forgot the password to the other account, <@a>click here</a> to request a new password.'), // used in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad phone' => t('That is not a proper phone number.'),
//  'bad account code' => t('That is not your account code.'),
  'not your account' => t('That is not your account ID.'),
  'bad account number' => t('Your bank account number must be 3-17 digits long.'),
//  'bad pin' => t('Your PIN must be exactly 4 digits'),
  'bad federal id' => t('That is not a correct @what. Try again.'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'short to' => t('You are @short short for that transaction.'),
  'increase min' => t('<@a>Click here</a> to increase your minimum balance!', ['@a'=>'a href=' . BASE_URL . '/account/preferences']),
  'no such company' => t('There is no such company.'),
  'op canceled' => t('Operation Canceled.'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm delete cell' => t('Are you sure you want to remove the selected mobile phones from this account?'),
  'already related' => t('That person is already related to this account. If you want to change the settings, find this person in the list and change the settings on that row.'),
  'already cell' => t('That mobile phone (@number) is already connected to this account.'),
  'cell taken' => t('A mobile phone can be connected to only one account. You are already using that phone (@number) in connection with account "@accountName". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'bad routing number' => t('US bank routing numbers are 9 digits. Use the number on the lower left edge of your checks.'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'unknown member' => t('"@who" has not yet signed up for rCredits.'),
//  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click the one you want or, if you don\'t see them listed, <a href="@draftLink">Click here to add them</a> to the list or <a id="which-cancel">here to cancel</a>.'),
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click the one you want or <a id="which-cancel">here to cancel</a>.'),
//  'similar found' => t('If you mean @otherName (@phone), <a href="@draftLink">click here for YES</a>. Otherwise, <a href="@cancelLink">click here to cancel</a>.'),
  'get app' => t('To scan Account ID cards with a smartphone, you need the app! You can download the Android app from Google\'s Play Store.'),
  'no account permission' => t('You do not have permission to use that account.'),
  'bad transient id' => t('That QR code is out of date. Try again?'),
  'bad email' => t('That email address is not valid.'),
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner, employee, or contractor for the company? (please choose one or more)'),
  'shortr to' => t('You do not have enough rCredits, for that transaction. <a href="@how">Go here</a> to get more rCredits.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("@fullName").'),
  'bad invite' => t('To sign up for rCredits, you must be invited. Ask someone you know to send you an invitation!'),
  'used invite' => t('<p>That invitation has already been used, to open an rCredits account.</p><p>If you already have an rCredits account set up and want to open an additional account for yourself, your business, or a family member, sign in and use the "Open Another Account" button on the Summary page (for cashier accounts, first select your business from the dropdown in the upper right corner).</p><p>Otherwise ask someone you know to send you a new invitation!'),
  'expired invite' => t('<p>That invitation has expired.</p><p><@a>Click here</a> to ask @inviterName for another invitation.</p>'),
  'min sub floor' => t('Your chosen rCredits minimum cannot be less than your <em>allowed</em> minimum (@floor).'),
  'info not saved' => t('We apologize. Your information could not be saved at this time. Please try again in an hour or so.'),
  'no page permission' => t('You do not have permission to visit this page. Return to your <@a>Summary</a> page.', ['@a' => 'a href=' . BASE_URL]),
  'specific nonpositive' => t('The amount (@amount) must be a positive integer.'),
  'insufficient relation' => t('You cannot open an account unless you have a close relationship to it. <@a>Invite someone</a> instead.', ['@a' => 'a href=' . BASE_URL . '/community/invite']),
  'bad spoof check' => t('You must confirm (<i>every time</i>) that you are on the real rCredits site.'),
  'not an image' => t('That is not an acceptable image format. You might try converting it to a JPG or PNG image.'),
  'bad commercial aspect' => t('Your company profile picture must be in landscape orientation, with a width-to-height ratio of about @R_COMMERCIAL_ASPECT.'),
  'bad personal aspect' => t('Your profile picture must be in portrait orientation, with a width-to-height ratio of about @R_PERSONAL_ASPECT.'),
  'need a manager' => t('A company account must have at least one person to manage it.'),
  'self must sign' => t('You cannot sign this agreement on behalf of another person.'),
  'bad zip' => t('US zipcodes must be 5 digits, possibly followed by a hyphen and 4 more digits.'),
  'bad state' => t('You must say what state.'),
  'password required' => t('You must supply your current password, to change your email address or password.'),
  'bad account id' => t('That account does not exist. Check your typing?'),
  'bad login' => t('That account/password combination does not exist. Try again or <!a>ask for a new password</a>.', ['!a' => 'a href="' . BASE_URL . '/account/password"']),
  'wrong pass' => t('That is not your correct password.'),
  'multiple spaces' => t('@field cannot have more than one space in a row.'),
  'illegal char' => t('@field contains an illegal character. Try retyping it'),
  'too long' => t('@field is too long: it must be @USERNAME_MAX_LENGTH characters or less.'),
  'weak pass' => t('You need to choose a password that is harder to guess.'),
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
//  'sign in to scan' => t('You must <a href="/user/login">sign in</a> (or scan yourself in) BEFORE scanning a customer card.'),
  'change min first' => t('That request would leave you with less than your minimum balance. <a href="@BASE_URL/account/preferences">Lower your minimum</a> first.'),
  'bad member id' => t('That is not a valid member ID!'),
  'bad card' => t('That is not a valid member ID.'),
  'bad card code' => t('This rCard is not valid.'),
  'upload required' => t('You must upload a file.'),
  'missing contact info' => t("<h2>Missing Contact Information</h2><p>Please complete your <@a1>Contact Information</a> before continuing.</p>"),
  'correct and retry' => t('Please correct the problem and resubmit this form.'),
//  'duplicate phone' => t('That phone number is already the primary phone for another account.<br><br>If this is your ONLY phone and you can make do without a connected bank account (that is, if you have some other easy way to get money into and out of your rCredits account): <input type="button" value="Continue With No Bank Connection" onclick="jQuery(\'#edit-dupOk\').val(1); jQuery(\'#rcreditswebform@form\').submit();" />'),
  'not a duplicate phone' => t('That phone number is NOT the primary phone for another account.'),
  'bad pin' => t('That is not your correct PIN.'),
  'wrong pin len' => t('Your PIN must be exactly 4 characters long.'),
  'pin required' => t('You must choose a PIN.'),
  'only us banks' => t('Unfortunately, at this time, we can handle only U.S. banks.'),
//  'bad deposit cents' => t('Each deposit is a number less than @DW_VERIFY_ROOF.'),
  'no can scan' => t('You do not have permission to scan an individual\'s rCard.'),
  'must opt' => t('You must not create a limited account for this person without giving him or her the opportunity to open a real rCredits account. <@a>Invite them</a>!', ['@a' => 'a href=' . BASE_URL . '/community/invite']),
  'must agree' => t('Accept Terms checkbox field is required.'), // drupal shows this
  'redo info' => t('We could not verify your personal information. Please try again, carefully.'),
  'bad ssn or dob' => t('There is a problem verifying your personal information.'),
  'bad ssn' => t('That is not a valid social security number.'),
  'bank too little' => t('The amount you asked to transfer is less than the minimum ($@R_ACHMIN).'),
  'members only' => t('You must be a member, to use this feature.'),
  'dwolla terms required' => t("Please accept Dwolla's terms before you click Continue."),
  'no card ordered' => t('No rCard was ordered.'),
  'bad buy info' => t('To pay with rCredits, you must use the same @thing for the purchase as you use for your rCredits account.'),
  'must be signed out' => t('You are already signed in.'),
  'wrong payroll co' => t('The company name in cell A1 must match your company name exactly (@coName)'),
  'not payroll csv' => t('This is not a payroll CSV. It must say "Payroll Details" in cell A2.'),
  'bad payroll csv' => t('Incorrect payroll CSV format at line @line.'),
  'empty payroll' => t('There are no employees listed as having received an rCredits payroll deduction.'),
  'bad payroll employee' => t('@employee is not listed as an employee on your Relations page. Click the Settings gear, then click "Relations".'),
  'illegal amount change' => t('You cannot @action the original amount (@amount). <@a>Send an invoice</a> instead.'),
  'bad achmin' => t('The minimum transfer amount must be at least $@R_ACHMIN.'),
  'say why not' => t('You must say why you are denying the invoice.'),
  'no co pass' => t('Company accounts do not have passwords. Sign in to your personal account, then select @company from the dropdown in the upper right corner.'),
  'account number mismatch' => t('You must have mistyped your account number once. Try again.'),
  'yes or no' => t('You must answer Yes or No.'),
  'no local proxy' => t('No other members nearby. <@a>Invite someone</a>!', ['@a'=>'a href=' . @BASE_URL . '/community/invite']),
  'too many joins' => t('You can join to only one other account.'),
  'short cash help' => t('<@a>Click here</a> for more information about cashing out incentive rewards.', ['@a'=>'a href=' . BASE_URL . '/help/cashing-out']),
  'maybe not cash' => t('If you are actually reimbursing someone or paying them for goods or services (for example, employee labor), choose a different button under "For".'),
  'link expired' => t('That link has expired.'),
  'login failed' => t('Your attempt to sign in failed. Click the "Password problems" link if you need a new password.'),
  'file save error' => t('Your file upload did not succeed.'),
  'forced without perm' => t('<p>An unauthorized transaction was made using a Company Agent rCard.</p><ul><li>Date: @date</li><li>Customer</li><li>@a2 (by @agent2)</li><li>Company: @a1 (by @agent1)</li><li>Amount: @amount</li><li>For: @for</li></ul><p>This transaction was taken offline, so there was no way to know the agent lacked authorization.</p>'),
);
$GLOBALS[TX_WEB . ' messages']['flooded'] = $GLOBALS[TX_WEB . ' messages']['bad login'];


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

/**
 * Return a list of proxy choices for the current account.
 * @param int $dft: the current proxy choice, if any
 */
function proxyChoices($dft) {
  $a = r\acct();
  $uid = $a->id;
  $table = <<<EOF
    (SELECT DISTINCT IF(inviter=:uid, invitee, inviter) AS uid FROM r_invites 
      WHERE :uid IN (inviter, invitee) AND invitee>0
    UNION SELECT DISTINCT IF(person=:uid, proxy, person) AS uid FROM r_proxies 
      WHERE :uid IN (person, proxy)) q
EOF;

  if (!$friends = db\lst('uid', $table, 1, compact('uid'))) $friends = '0'; // list of account's friends
  $sql = <<<EOF
    SELECT u.uid, fullName, city, state, SUM(w) AS weight FROM (
      SELECT uid, :PHP_INT_MAX/2 AS w FROM users 
      WHERE uid IN ($friends)
      UNION
      SELECT uid, :PHP_INT_MAX/4 AS w FROM users 
      WHERE (state=:state AND city=:city) OR postalCode=:zip
      UNION
      SELECT IF(uid1 IN ($friends), uid2, uid1) AS uid, weight AS w FROM r_near
      WHERE uid1 IN ($friends) OR uid2 IN ($friends)
      UNION
      SELECT uid, 0 AS w FROM users
    ) n LEFT JOIN users u ON u.uid=n.uid
    WHERE community=:ctty AND u.uid>1 AND u.uid<>:uid AND NOT :IS_CO
    GROUP BY u.uid, u.state, u.city 
    ORDER BY state<>:state, state, city, fullName
EOF;
//    ORDER BY GREATEST(SUM(w), :PHP_INT_MAX/4) DESC, state<>:state, state, city, SUM(w) DESC, fullName
  $q = db\q($sql, u\ray('uid state city zip ctty', $uid, $a->state, $a->city, $a->postalCode, $a->community));
  $res = $cityX = '';
  while ($row = $q->fetchAssoc()) { // show choices in reverse order, so inviter(s) are at top
    extract($row);
    $state = $state == $a->state ? '' : (', ' . r\realState($state));
/*    if ($weight < PHP_INT_MAX / 2) { // show city and maybe state
      $state = $state == $a->state ? '' : (', ' . r\realState($state));
    } else list ($city, $state) = [t('Top Suggestions'), '']; */
    if ($city <> $cityX) {
      if ($cityX) $res .= "</td></tr>\n"; // finish previous line
      $res .= "<tr><th>$city$state</th><td>";
      $cityX = $city;
    }
    $class = $weight < PHP_INT_MAX / 4 ? 'choice4'
    : ($weight == PHP_INT_MAX / 4 ? 'choice3'
    : ($weight <= PHP_INT_MAX / 2 ? 'choice2'
    : 'choice1' ));
    $checked = $dft == $uid ? 'CHECKED' : '';
///    debug(compact('row','city','cityX','friends') + ['max/2-w'=>PHP_INT_MAX / 2 - $weight]);
    $res .= <<<EOF
<div class="form-type-radio $class">
  <input type="radio" name="proxyChoice" $checked value="$uid" />
  <label>$fullName</label>
</div>
EOF;
  }
  if ($res) $res .= '</td></tr>'; // finish last line
  return "<table>$res</table>";
}

/**
 * Return HTML for a button.
 */
function button($value, $href = '', $title = '', $class = '', $id = '') {
  $title = htmlspecialchars($title);
  $button_type = trim('local form-submit ' . ($class ?: '')); // Drupal prefixes this with 'form-'
  $markup = array($value => formField($id ? 'submit' : 'button', '', '', compact('value', 'button_type') + attrib(compact('id', 'title'))));
  $markup = render($markup);
  if (!$id) $markup = str_replace('"submit"', '"button"', $markup); // compensate for Drupal pigheadedness
  if ($href) { // href might be http or https or /...
    if (substr($href, 0, 4) == 'http' or substr($href, 0, 1) == '/') $href = "document.location.href='$href';";
    $name = 'name="op"';
    $markup = str_replace($name, ($button_type == 'local' ? '' : "$name ") . "onclick=\"$href\"", $markup);
//  if ($href) $markup = "<a href=\"$href\" title=\"$title\">$markup</a>";
  }
  return $markup;
}

function formField($type, $title = '', $description = '', $other = [], $options = NULL) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = $options ? key($options) : ''; // Drupal's select fails without a default
  $title_display = 'before';
//  if (isset($other['value']) and !isset($other['default_value'])) $other['default_value'] = $other['value'];
  $field = u\prefixKeys('#', array_merge(compact(u\ray('type title title_display description options')), $other));
  return $field;
}

function submit($title = '', $other = '') {return formField('submit', '', $other, $title ?: t('Save'));}
function item($markup, $title = '', $desc = '', $other = []) {return formField('item', $title, $desc, compact('markup') + $other);}
function hidField($value = '', $options = []) {return formField('hidden', '', '', compact('value') + $options);}
function textField($title = '', $description = '', $other = [], $options = NULL) {
  return formField('textfield', $title, $description, $other, $options);
}
function boxField($title, $desc = '', $other = NULL) {
  return formField('checkbox', $title, '<div class="box"></div>' . $desc, $other);
}
function fieldSet($id, $flds, $prompt = '') {return formField('fieldset', $prompt, '', compact('id')) + $flds;}

/**
 * Return a form field for sharing percentage.
 */
function shareField() {
  $mya = r\acct();
  $sharePct = round(db\lookup('AVG(share)', 'r_gifts', 'share>=0'), 1);
  return textField(t('Share:'), t('<p>What percentage of your ongoing purchase rewards would you like to donate to Common Good Finance (CGF), to support the rCredits system? This is another way to take responsibility for our community\'s future. You can put anything from 0% to 50%. For example, if you choose 30%, then when you make a $10 purchase and get your $1 reward, your "sharing" contribution is 30 cents (30% of $1). The current average sharing percentage chosen by members is sharePct%.</p><p>Note that this is not "giving back" part of your rewards to the rCredits community &mdash; rather it is an optional extra donation to CGF. CGF is a nonprofit member company, not the system itself. CGF promotes rCredits and handles administration of your rCredits community until the community can take over that responsibility.</p>', compact('sharePct')), required((($mya->share + 0) ?: R_SHARE_DFT) . '%'));
}

/**
 * Return an error message if the chosen sharing amount is invalid.
 * @param string $share: (MODIFIED) the member's chosen sharing percentage (returned normalized)
 * @param bool $required: whether to insist on input
 * @return FALSE if no error, otherwise the error message or index.
 */
function badShare(&$share, $required = FALSE) {
  $share = trim(str_replace('%', '', $share));
  if ($required and $share === '') return tt('missing field', ['field' => 'share']);
  if ($err = u\badAmount($share, '>=0', 3)) return t('AMOUNT:|') . $err;
  if ($share < 1) $share *= 100; // if under 1% assume they meant more
  if ($share > R_SHARE_MAX) return 'share too big';
}

/**
 * Return a boolean (Yes / No) field.
 * @param string $title: field label
 * @param string $description: helpful description of field
 * @param mixed $dft: the default value (TRUE or FALSE) or an assoc of attributes
 * @param array $labels: labels for the two choices (defaults to [No, Yes])
 */
function boolField($title = '', $description = '', $dft = NULL, $labels = NULL) {
  $class = array('yesno');
  return formField('radios', $title, $description, (is_array($dft) ? $dft : required(@$dft ?: 0)) + attrib(compact('class')), @$labels ?: array(t('No'), t('Yes')));
}

/*
 * Create a rendered checkboxes field
 * Drupal is inconsistent about checkboxes formatting, making our scripts fail.
 * Specifically, when the checkbox inputs have a unique id, radiocheck.js fails to set the value successfully
 * (at least it never appears in $_POST). So we do it here by hand.
 * @param string $tag:
 * @param string $title:
 * @param string $desc:
 * @param array $choices: bit name for each checkbox
 * @param array $aliases: bit name to display for each checkbox (defaults to bit name)
 */
function boxesField($tag, $title, $desc, $defaults, $choices, $aliases = NULL) {
  u\setDft($aliases, $choices);
  $boxes = '';
  if (!@$defaults) $defaults = [];
  foreach ($choices as $k => $v) {
    $alias = $aliases[$k];
    $checked = in_array($v, $defaults) ? ' CHECKED' : '';
    $boxes .= <<<EOF
    <div class="form-item form-type-checkbox form-item-$tag-$k">
       <input type="checkbox" name="{$tag}[$k]" value="$k"$checked class="form-checkbox" />
       <label class="option" for="edit-$tag-$k"><div class="box"></div><div>$alias</div></label>
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
 * Return a set of checkboxes representing bits in a bit array.
 * @param string $fieldName: internal name of field   
 * @param string $label: short prompt for the set of boxes
 * @param int $bitRay: the bit array (current value)
 * @param string $prefix: prefix for defined constants for this bit array field (empty means use $bits index)
 * @param string $bits: array (space or comma-delimited) of names of bits to include (without prefixes)
 * @param assoc $aliases: map of bit names to their display aliases (which may include formatting)
 */
function bitsField($fieldName, $label, $bitRay, $prefix, $bits, $aliases = []) {
  $bits = u\ray($bits);
  if (!is_numeric(key($bits))) $bits = array_keys($bits); // ignore values (eg weights for risk arrays)
  $defaults = [];
  foreach ($bits as $bit => $bitName) {
    if ($prefix) $bit = u\consta($prefix, $bitName);
    $bitNames[$bit] = $bitName;
    $bitAliases[$bit] = @$aliases ? strtr($bitName, $aliases) : $bitName;
    if (u\getBit($bitRay, $bit)) $defaults[] = $bitName;
  }
  return boxesField($fieldName, $label, '', $defaults, $bitNames, $bitAliases);
}

/**
 * Return a radio selection for account type.
 */
function acctTypeField() {
  $coAcctTypes = $GLOBALS['account types'];
  unset($coAcctTypes[R_PERSONAL]);
//  $onclick = "attrClick(this);";
//  $acctTypeAttrib = attrib(compact('onclick'));  
  return formField('radios', t('Account type:'), '', required(), $coAcctTypes);
}

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Also sets up $formArray and $formSta, for testing.
 */
function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formArray, $formSta, $rUrl;
  
  setGlobals(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
  if (!@$formSta['confirm'] and !in_array($function, u\ray(R_NOLOG_FORMS))) {
    if ($logs = $_POST) {
      foreach (u\just(R_NOLOG, $logs) as $k => $zot) $logs[$k] = '?';
      u\loga('in', $logs);
    } elseif ($formName = str_replace(base_path(), '', $_SERVER['REQUEST_URI'])) u\loga('form', array($formName));
  }

  try {
    $mya = r\acct();
///    global $zot; $zot = @$zot . ' ' . $function; if (strlen(@$zot) > 50) die($zot);
    if (!$mya and !in_array($function, u\ray(R_ANON_FORMS))) r\go(isGAME ? 'community/game' : '');
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
    $formName = substr($function, 0, 1) == '/' ? substr($function, 1) : "rCredits\\Web\\form$function";
    $specialForm = in_array($function, u\ray('Help Empty footer accounts Admin Login MemberList MemberInfo'));
    $args = func_get_args(); // this and next few lines are from \drupal_get_form()
    array_shift($args); // Remove $function from the arguments.
    $formSta['build_info'] = compact('args');
    if (!r\up() and !$specialForm) {
      if ($mya and $mya->admin) {
        say(t('The system is DOWN.'));
        //if (!$mya->cttyUp) say(t('The community is DOWN.'));
      } else $formName = 'rCredits\\Web\\formSystemDown';
    }
    $form = drupal_build_form($formName, $formSta);
    \rweb_form_alter($form, $formSta, $formName);
    
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any
    js('js/misc.js', 'file', 'footer'); // used to be just for specialForms
    if (!$specialForm) {
      js('jQuery(".messages").dblclick(function() {jQuery(this).hide();});', 'inline', 'header');
      if (@$form['country']) {
        js('js/countries.js', 'file', 'header');
        if ($mya and \form_get_errors()) js("print_country($mya->country, $mya->state);", 'inline', 'footer');
      }
      if (u\isIPhone()) css('iphone.css', -100);
    }

    $form['#attributes']['class'][] = 'rweb';
    $formArray = $form2 = $form; 

    $rent = \render($form2);

    if (strpos($rent, 'form-type-checkbox') or strpos($rent, 'form-type-radio')) js('js/radiocheck.js', 'file', 'footer');
    
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
  } catch(Exception $e) {r\Web\exception_handler($e);}
}

/**
 * Return a unique persistant identifier for the current machine (set randomly, if none yet).
 * @return the box ID string
 */
function box() {
  if (strlen($box = @$_COOKIE['box']) == R_CODE_LEN) return $box; // old codes were shorter
  //\variable_set('r_box', $box = \variable_get('r_box', 0) + 1); // this is prone to timing collisions
  return r\setCook('box', $box = u\code(R_CODE_LEN));
}

/**
 * Return the box record id for the given machine code and account id.
 * @param int $code: machine identifier
 */
function boxUser($code, $uid) {
  if ($id = @$_COOKIE[$cookid = 'boxnum' . $uid] and !u\test()) return $id; // we only get this if last set today
  if ($id = db\lookup('id', 'r_boxes', 'code=:code AND uid=:uid', compact(u\ray('code uid')))) {
    db\update('r_boxes', u\ray('id access', $id, r\rTime()), 'id');
  } else $id = r\makeDevice($uid, $code, TX_WEB);
  return r\setCook($cookid, $id, strtotime('tomorrow'));
}

/**
 * See whether someone is trying too many logins.
 * @param int $uid: account record ID
 * @param assoc $sta: (MODIFIED) the form status, updated with relevant flood info
 * @return mixed: FALSE only if we're not being flooded
 */
function flooding($uid, &$sta) {
  if (u\test()) return !$uid;
  $trigger = 'flood_control_triggered';
  $limit = 50; //variable_get('user_failed_login_ip_limit', 50);
  $window = HOUR_SECS; //variable_get('user_failed_login_ip_window', HOUR_SECS);
  if (!\flood_is_allowed('failed_login_attempt_ip', $limit, $window)) return $sta[$trigger] = 'ip';
  
  if (!$uid) return TRUE;

  $uidOnly = FALSE; //variable_get('user_failed_login_identifier_uid_only', FALSE);
  $limit = 5; //variable_get('user_failed_login_user_limit', 5);
  $window = 6 * HOUR_SECS; //variable_get('user_failed_login_user_window', 6 * HOUR_SECS);
  
  $sta['flood_control_user_identifier'] = $id = $uidOnly ? $uid : $uid . '-' . ip_address();
  if (!\flood_is_allowed('failed_login_attempt_user', $limit, $window, $id)) return $sta[$trigger] = 'user';

  return FALSE;
}

function noFlood(&$sta) {\flood_clear_event('failed_login_attempt_user', @$sta['flood_control_user_identifier']);}

/**
 * Set session variables and other globals.
 * @param int $agent: the account agent, if any (defaults to the account ID)
 * @param bool $force: resetup even if already done
 */
function setGlobals($agent = '', $force = FALSE) {
  global $user, $channel, $boxUser;
  if (@$channel and !$force) return; // already setup
  if (!$agent) $agent = @$user->uid; // register_submit supplies agent (no one else does)
  $myid = svar('myid') ?: $agent;
  $mya = r\acct($myid, $agent);
  r\acct::setDefault($mya); // set default for r\acct()
  $channel = TX_WEB;
  if ($myid) $boxUser = boxUser(box(), $myid); // so far used only when recording transactions
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

  if (!is_array($pop = svar('pop'))) return [];
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($formName != $pop_formName) return [];
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return [];
  
  svar('pop', NULL); // called with no args (#3)
  $onchange = "this.form.elements['$pop_resultField'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $subtext = item($pop_question);
  $choice = formField('select', '', '', attrib(compact('onchange') + array('size' => 12)), $pop_choices);
  $close = popupCloser('which'); // this is needed in mobile layout
  $popup = fieldSet('which', compact(u\ray('subtext close choice')), t('Which One?'));
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
  include_once __DIR__ . '/rweb-help.inc';
//  $subtext = item($question);
  $close = popupCloser('help');
  $text = formField('item', '', '', helpText($page));
  $help = fieldSet('help', compact(u\ray('close subtext text')), t('Help with this page'));
  return compact('help');
}

function memberStep($step, $done, $text, $links = '', $newWindow = FALSE) {
  u\EXPECT('done', 'bool');
  $target = $newWindow ? ' target="_blank"' : '';
  if ($links) {
    if (!is_array($links)) $links = array($links);
    foreach ($links as $href) {
      if (strpos($href, 'http') === FALSE) $href = BASE_URL . "/$href";
      $link = "<a href=\"$href\"$target>";
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

function divButton($href, $title) {
  if (strpos($href, 'http') === FALSE) $href = BASE_URL . "/$href";
  return  <<<EOF
<a href="$href" title="$title"><div></div></a>
EOF;
}

function imageButton($href, $title, $src) {
  global $rUrl;
  if (strpos($href, 'http') === FALSE) $href = BASE_URL . "/$href";
  if (strpos($src, 'http') === FALSE) $src = "$rUrl/images/icons/$src";
  return  <<<EOF
<a href="$href" title="$title"><img src="$src" border="0" /></a>
EOF;
}

/**
 * Return a form element consisting of a button to redirect to a relative or absolute URL.
 * @param string $label: button label
 * @param string $goto: where to go when the button is pressed
 * @param string $title: what to say when hovering over the button
 * @return: the button html
 */
function goButton($label, $goto, $title) {
  $gotoHead = isDEV ? DEV_ROOT : '';
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
  foreach ($perms as $perm) if (!$mya->can($perm)) return [];
  return goButton($label, $goto, $title);
}

/**
 * Undo the last transaction.
 *//*
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
*/

/**
 * Show remaining customer balance. (smartphone)
 *//*
function remainder() {
  if ($uid = svar('lastCustomer')) {
    $a = r\acct($uid);
    $otherName = $a->fullName;
    $balance = be\creditInfo(compact('uid'))->fancy['balance'];
    say('customer balance', compact('otherName', 'balance'));
  } else say('no last customer', 'ERR');
  r\go('empty');
}
*/

function boxOpt($field, $opts, $sta) {
///debug(compact('field','opts','sta'));
  foreach ((@$sta['input'][$field] ?: []) as $key => $value) $result[] = $opts[$key]; // note that key==value
  return @$result ?: [];
}

/**
 * Return the fields used to verify an individual's SSN (also asked for in signup).
 * The dob field is not used for companies (but remains in the form, unseen), so it cannot be required.
 */
function ssnFields($personal = TRUE) {
  $ssTitle = tt($personal ? 'ssTitle' : 'einTitle');
  if ($personal) $dob = textField(t('Birthdate:'), t('Your date of birth (mm/dd/yyyy)'), required());
  $federalId = formField('password', "<span class=\"title\">$ssTitle</span>", tt($personal ? 'ssDesc' : 'einDesc'), required()); // textfield so user doesn't have to retype on error
  return compact(u\ray('dob federalId'));
}

function dwollaTerms() {
  global $rUrl;
  $title = item(t('Welcome to rCredits!'));
  $subtext = item(t(<<<EOF
<p>Your rCredits account will contain both US Dollars and rCredits (incentive rewards issued by your rCredits community).</p>

<p>Our partnership with Dwolla connects you to the mainstream economy. We use the Dwolla network to transfer funds between your rCredits account and your checking account. A few required words about Dwolla:</p>
<div id="graphics"><div><img src="$rUrl/images/dwolla.jpg" /><img id="veridian-logo" src="$rUrl/images/veridian.jpg" /></div></div>
<p>Financial institutions play an important role in the Dwolla network.</p>

<p>Dwolla, Inc. is an agent of Veridian Credit Union and all funds associated with your account in the Dwolla network are held in a pooled account at Veridian Credit Union. These funds are not eligible for individual insurance, and may not be eligible for share insurance by the National Credit Union Share Insurance Fund. Dwolla, Inc. is the operator of a software platform that communicates user instructions for funds transfers to Veridian Credit Union.</p>
EOF
  ));

  $aTerms = 'a href="https://www.dwolla.com/tos?nolayout=true" target="_blank"';
  $aPrivacy = 'a href="https://www.dwolla.com/privacy?nolayout=true" target="_blank"';

  $agreed = boxField(t('Accept Terms checkbox'), tt("I accept Dwolla's <@aTerms>Terms of Service</a> and <@aPrivacy>Privacy Policy</a> and I authorize the rCredits system to open and manage a Dwolla account on my behalf.", compact('aTerms', 'aPrivacy')), suffix('&nbsp;'));
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

/**
 * Return an array of flags and coFlags, appropriate to the given account type.
 */
function acctType($acctType) {  
  $flags = ($acctType == R_PERSONAL ? 0 : u\bit(B_CO));
    
  if ($acctType != R_PERSONAL) { // company
    $type = $GLOBALS['account types'][$acctType];
    $coFlags =
        ($acctType == CO_SOLE_PROPRIETOR ? u\bit(CO_SOLE_PROPRIETOR)
      : ($acctType == CO_PARTNERSHIP ? u\bit(CO_PARTNERSHIP)
      : (strpos($type, 'nonprofit') !== FALSE ? u\bit(CO_NONPROFIT)
      : (u\bit(CO_CORPORATION) | (strpos($type, 'publicly') !== FALSE ? u\bit(CO_PUBLICLY_TRADED) : 0)
    ))));
  }
  return compact(u\ray('flags coFlags'));
}

/**
 * Create relation for company relationship specified in registration form (if any)  
 * @return TRUE if there is a formal relationship
 */
function suCompanyRelation($other, $data) {
  extract(u\just('company companyPhone isOwner employeeOk contractor', $data));
//  $isOwner = (int) @$isOwner; // POSTed as "isOwner" when TRUE
//  $employee = (int) (@$employeeOk or @$contractor);
  if ($main = findCompany($company, @$companyPhone)) { // company account exists
//    return db\insert('r_relations', compact(u\ray('main other employeeOk isOwner')));
    return r\acct($main)->newRelation($other, 0, @$employeeOk, @$isOwner); // no permissions/draw (for security)
  } else return FALSE;
}

/**
 * Set up relationship of this new account with the account that created it (if any).
 */
function suCreatorRelation($helper, $args, $myid) {
  extract($params = u\just('isOwner employeeOk flow', $args));
  if (!@$isOwner and !@$employeeOk and !@$flow) return; // no useful relation (for example by true invitation)

  if (!$error = u\EXPECT($params, 'int int int', TRUE)) { // if not a hack attempt
//    list ($main, $other, $employee, $permission) = array($myid, $helper, $employeeOk, r\perm(B_MANAGE));
    $draw = ($flow & 2) ? 1 : 0;
//    db\insert('r_relations', compact(u\ray('main other employee isOwner draw permission')));
    r\acct($myid)->newRelation($helper, r\perm(B_MANAGE), $employeeOk, $isOwner, $draw);
    if ($flow & 1) r\acct($helper)->newRelation($myid, 0, 0, 0, TRUE);
//      list ($main, $other, $draw) = array($other, $main, TRUE);
//      db\insert('r_relations', compact(u\ray('main other draw')));
//    }
  } else r\tellStaff("bad params in formSignup: $error -- ", $params);
} 

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $email0: the email address to check (also returned with improvements)
 * @param int $uid: account record id (optional), if changing email address rather than creating new account
 * @return: TRUE if email is okay to use, else FALSE
 */
function emailOkay(&$email0, $uid = '') {
  $email = strtolower(str_replace("'", '&#039;', $email0));
  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
  if (emailDup($email, $uid)) return FALSE;
  $email0 = $email;
  return TRUE;
}

/**
 * Say whether the email is a duplicate and complain if it is.
 * @param string $email: the email address to test
 * @param int $uid: the account it is for (empty if the account is not created yet)
 * @return <email is a dup>
 */
function emailDup($email0, $uid) {
  $mail = u\cryptN($email0);
  if (!$who = db\lookup('fullName', 'users', 'mail=:mail and uid<>:uid', compact('mail', 'uid'))) return FALSE;
  $a = "a href=account/password/$email0";
  $emailTagged = str_replace('@', '+whatever@', $email0);
  if (!$uid) $forgot = '|forgot password';
  say('duplicate email' . @$forgot, compact(u\ray('emailTagged who a')), 'email');
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
  $helper = @$helper ? r\acct($helper)->fullName : 'unknown';
  $copts = u\jsonEncode(@$copts);
  $ats = @join(',', array_values($ats));
  $params = compact(u\ray('phone country state acctType helper copts ats')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  r\tellStaff($message, $params);
}

/**
 * Return a record ID for the given company or company phone.
 * @return record ID (FALSE if not found)
 */
function findCompany($company, $companyPhone) {
  if (!$company and !$companyPhone) return FALSE;
  $companyShortName = u\shortName($company);
  $companyUid = db\lookup('uid', 'users', ':IS_CO AND phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = db\lookup('uid', 'users', ':IS_CO AND name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = db\lookup('uid', 'users', ':IS_CO AND name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Return form fields for name and email.
 * @param bool $what: prompt description of what the name is for ("Company", "Community", or "Full name")
 * @param string $fullName: default full name
 * @return the relevant form fields
 */
function nameAndEmail($what, $fullName = '') {
  $who = $what == t('Company') ? t('The company\'s') : ($what == t('Community') ? t('The community\'s') : t('Your'));
  $onchange = "var legal=jQuery('#edit-legalname'); if (legal.val()=='') legal.val(this.value);";
  $fullName = textField("$what:", tt('usename desc'), required(@$fullName) + attrib(compact('onchange')));
  $legalName = textField(t('Legal name:'), t('(if different) ') . $who . tt('legalname desc'));
  $email = textField(t('Email:'), t('All emails from the system will be sent to this address. It will not be made public. <b>Type carefully.</b>'), required());
  return compact(u\ray('fullName legalName email'));
}

/**
 * Return the part of the form that is (mostly) common to formContact and formSignup
 * @param assoc $defaults: initial field values (defaults to current values) - see list at "$fields ="...
 * @param bool $old: <show contact fields for an established account (on Contact form)>
 */
function contactFields($defaults = [], $old) {
  $country = $state = ''; // make sure these are not NULL
  $fields = 'phone faxetc country postalCode address city state tenure owns postalAddr'; // had verifyBy
  if ($old) {
    $mya = r\acct();
    extract($defaults ?: ($mya ? (array) $mya->account($fields) : []));
  }
//  $dupPhone = @$defaults['dupPhone'];

//  js('http://ziplookup.googlecode.com/git/public/ziplookup/zipLookup.min.js', 'file', 'header');
  js('js/zipLookup.js', 'file', 'header');
  js("print_country(\"$country\", \"$state\");", 'inline', 'footer');

  $onchange = "print_state(this.options[this.selectedIndex].value,'');";
  $countryAttrib = attrib(compact('onchange'));
  
  $onchange = $onblur = 'setPostalAddr();';
  $addressAttrib = $cityAttrib = attrib(compact('onchange'));
  $stateAttrib = attrib(compact('onblur'));
  $onchange = "zipChange('010 013');";
  $zipAttrib = attrib(compact('onchange'));
  
  $phone = textField(t('Phone:'), '', required(@$phone ? u\fmtPhone($phone): ''));
//$mya ? '' : t('Your phone number. This should not be a number already used for another rCredits account &mdash; unless you have no other phone number.')
//  $dupOk = hidField(0, attrib(u\ray('id', 'edit-dupOk')));
//  $dupOk = boolField(t('Dup Ok:'), t('If this is already the phone number for another rCredits account, click Yes to use it for this account too. Without a unique phone number, though, you cannot connect a bank account to your rCredits account. We are lobbying our financial partner, Dwolla, to remove this limitation. But, for now, a duplicate phone means no connected bank account.'));
//  $verifyBy = $mya ? NULL : formField('radios', t('Verify by:'), t('We will send a verification code to your phone, by text or voice.'), required(@$mya->verifyBy ? (int) ($mya->verifyBy == 'Voice') : NULL), array(t('Text message'), t('Voice message')));
  $faxetc = @$faxetc ? textField(t('Fax etc:'), t('List other ways to contact you: fax, mobile, IM, etc. This field is NOT public.'), dft(@$faxetc)) : NULL;
  $country = formField('select', t('Country:'), t(''), dft(@$country) + $countryAttrib); // can't use required() here
  $postalCode = textField(t('Postal Code:'), t('Postal code for your <em>physical location</em>'), required(@$postalCode) + $zipAttrib);
  $address = textField(t('Street Address:'), t('Where is your building located (physical address)?'), required(@$address) + $addressAttrib);
  $city = textField(t('City:'), t(''), required(@$city) + $cityAttrib);
  $state = formField('select', t('State:'), t(''), dft(@$state) + $stateAttrib); // can't use required() here
  $tenure = $old ? NULL : textField(t('How long:'), t('MONTHS at this address.'), required(@$tenure));
  $owns = boolField('', '', $old ? dft((int) $mya->owns) : 0, [t('Rent'), t('Own')]);
  $postalAddr = textField(t('Postal Addr:'), t('Where does the post office send your mail (usually)?'), required(@$postalAddr));

  return compact(u\ray($fields));
}  

/**
 * Say whether the phone number is acceptable.
 * @param string $phone: phone number in any format
 * @param int $country: country code
 * @param string $form: the current form name, all lowercase, without "form" (in case of duplicate phone).
 * @return TRUE if the phone number is acceptable
 */
function phoneOk(&$phone, $country, $form = '') {
  $myid = @r\acct()->id ?: 0;
  if (!$phone = u\fmtPhone($phone, '+n')) return say('bad phone', 'phone');
  if ($country == US_COUNTRY_ID and strlen($phone) != 12) return say('bad phone', 'phone');
  //$dup = db\exists('users', 'phone=:phone and uid<>:myid', compact('phone', 'myid'));
  // NO! this makes contact form fail. if (!$dup and $dupOk) return say('not a duplicate phone', 'phone');
  return TRUE;
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function adminField($label) {
  $mya = r\acct();
  return ($mya->cAdmin and $mya->federalId) ? item(R_ON_FILE, $label) : NULL;
}

/**
 * Return a "SHOW" link for the field or a display of the field's value, for community admins only.
 * @param string $field: the field in question
 * @param string $show: the field to display, if any (default none)
 * @return the appropriate markup
 *//*
function show($field, $show = '') {
  global $base_url;
  $mya = r\acct();
  
  if ($field == $show) {
    if ($field == 'idProof') { // display the proof image alone (then user clicks on Back)
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
*/

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
 * Call by:
 *   $email = invitation($code, '', $inviter) returned with $inviter=inviter uid
 *   invitation($code, $invitee) marks the invitation USED by recording the invitee's account id
 * @param string $code: the invitation code
 * @param int $invitee: account id of the invited person
 * @param int $inviter: (returned, if invitee='') account id of the inviter (returned even if error)
 * @param int $err: (returned) error message, if any
 * @return:
 *   (1) encrypted email, if invited specifically with our form
 *   (2) TRUE if invited with an emailable link or marking invitation USED
 *   (3) FALSE if no such invitation or marking invitation USED failed
 */
function invitation($code, $invitee = '', &$inviter = '', &$err = '') {
  if (!$invitee and preg_match('/([^0-9]+)([0-9]+)/', $code, $m) and $a = r\acct($m[1]) and $code == $a->inviteCode) {
    $inviter = $a->id;
    return TRUE;
  }
  $fields = 'id,email,inviter,invitee,invited';
  if (!$result = db\lookup($fields, 'r_invites', 'code=:code', compact('code'))) return !$err = 'bad invite';
  $id = $result['id']; // don't extract yet
  if (@$invitee) return db\update('r_invites', compact('id', 'invitee'), 'id'); // mark it used
  
  extract($result); // looking for email and inviter, so get details
  $lastweek = strtotime((R_INVITE_DAYS + 2) . ' days ago'); // +2 for leniance
  if ($invitee) return !$err = 'used invite';
  if ($invited < $lastweek) return !$err = 'expired invite';
  return u\cryptN($email, FALSE); // $inviter is returned implicitly

}

function passwordOkay($a, $password, $passFieldName = 'pass') {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  if (@$a->oneTimePass) {
    extract(u\just('pass expires', $a->oneTimePass));
    if ($password == @$pass) return ($a->admin or r\rTime() < @$expires) ? TRUE : say('pass expired', $passFieldName);
  }
  $hash = \_password_crypt('sha512', $password, $a->$passFieldName);
  return ($hash and $a->$passFieldName == $hash) ? TRUE : say('bad login', $passFieldName);
}

/**
 * Return a once-use password and record it in the data field.
 * @param string $name: login identifier (email, account ID, or shortname)
 * @return assoc: [pass, expires] where
 *   pass: the once-use password
 *   expires: when it expires
 */
function oneTimePass($name) {
//  $pass = strtolower(u\nonce()); // not a hash, easy to remember and type
  $pass = u\code();
/**/ if (isDEV) \drupal_set_message('Link: ' . "<a href='http://localhost" . DEV_ROOT . "/reset/id=$name&code=$pass'>reset</a>");
  $expires = r\rTime() + HOUR_SECS * R_SIGNIN_HOURS;
//  r\setCook('loginto', $name, $expires);
  return compact('pass', 'expires');
}  

/**
 * Return a formatted, categorized list of businesses
 * @param string $which: word search for business name or industry
 * @param mixed $region: community (the default), postalCode (maybe partial), or country (maybe partial)
 * @param bool $withButtons: include "Pay" and "Chg" buttons
 */
function directoryList($which, $region, $withButtons = FALSE) {
  global $base_url;
  
  if ($withButtons and $mya = r\acct()) {
    $canBuy = $mya->can(B_BUY);
    $canSell = $mya->can(B_SELL);
  } else $canBuy = $canSell = FALSE;

  list ($symbolMember, $symbolRTrader, $symbolNonMember) = array('m', 'R', '-');
  
  if (!$region or (is_numeric($region) and $region < 0)) {
    if (!$region) $region = r\serverUid();
    $regionCrit = 'community=:region';
  } else {
    $region = \db_like($region) . '%';
    $regionCrit = '(postalCode LIKE :region OR country LIKE :region)';
  }
  $regionCrit = "($regionCrit OR name='cgf')"; // always include CGF
  
  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $rows = [];
  $sql = <<<EOF
    SELECT DISTINCT u.uid, IFNULL(i.industry, '~Other') AS industry, u.name AS short_name, u.fullName, u.data
    FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE :IS_CO AND u.uid>1 AND :IS_OK AND $regionCrit AND (u.fullName LIKE :which OR i.industry LIKE :which)
    ORDER BY industry, u.fullName
EOF;
  $result = db\q($sql, compact(u\ray('region which')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]); // uid, industry, short_name, fullName, data
    $data = unserialize($data);
    $website = @$data['website'];
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $headed = TRUE; // always show the category (but keep the option to disable this choice)
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $a = r\acct($uid);
    $pay = $canBuy ? button('pay', "$base_url/pay/who=$short_name") : '';
    $chg = $canSell ? button('chg', "$base_url/charge/who=$short_name") : '';
    $fullName = "<a href='$base_url/member/$short_name'>$fullName</a>";
//    list ($selling) = explode("\n", $a->selling);
    $item = "$pay$chg <div class=\"company\">$fullName</div><div class=\"selling\">$a->shortDesc</div>";
    if (!$headed) $item = "- $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  $key = $list ? '' : t('There are no such businesses listed.');
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return "<ul id=\"key\">$key</ul>$list"; // ul so key gets indented like the rest
}

/**
 * Sign the user in by their scanning their own company card
 *//*
function scanIn($acct) {
  $mya = $acct;
  svar('myid', $mya->id);

  // Tell Drupal who is now logged in
  global $user; $user = $mya->agent->account();
  $user->roles = [];
  $user->uid = $mya->agent->id;
  drupal_session_regenerate();

  setGlobals();
  svar('scanned_in', TRUE);
  $agentName = $mya->agent->fullName;
  $companyName = $mya->fullName;
  $subs = compact('agentName', 'companyName');
  u\loga('login', $subs, $mya->agentId);
  return r\go('empty', 'ready to charge', $subs);
}
*/

function hack($message, $info = []) {
  u\loga('hack', $_SERVER, $message);
  return r\signedIn() ? signout() : r\go('');
}

function signout() {
  @\session_destroy();
  return r\go('signin');
}

/*function changeAccount($quid = '') {
  setGlobals();
  if (!$mya = r\acct()) return r\go('');
  if ((!$acct = r\acct($quid, $mya->id)) or !$acct->can()) return hack('i'); // hacker. so leave.
  svar('myid', $acct->id);
  $newAcct = $acct->fullName;
  say('changed account', compact('newAcct'));
  return r\go('');
}*/

function fundRpt($now, $then, $type = '') {
  //$up = $then ? number_format(abs($now / $then - 1) * 100, 1) : '100';
  $RUS = strpos($type, 'r') === FALSE ? '' : '@R';
  $change = u\fmtAmt(abs($now - $then));
  $way = $now >= $then ? t('up') : t('down');
//  $s = u\fmtAmt($now) . tt("$RUS &mdash; @way @change$RUS from a month ago", compact('way', 'change'));
  $s = u\fmtAmt($now) . tt($RUS, compact('way', 'change'));
  return strpos($type, '$') === FALSE ? str_replace('$', '', $s) : $s;
}

function txsRpt($amt, $count, $accts, $type) {
  $type = $type == 'p' ? t('personal') : ($type == 'b' ? t('company') : '');
//  list ($totaling, $account, $month, $per, $avg) = array(t('totaling'), t('account'), t('mo'), t('per'), t('avg'));
  list ($acct, $month) = array(t('acct'), t('mo'));
  list ($amt, $perAcct) = array(u\fmtAmt($amt), u\fmtAmt($amt / ($accts ?: 1)));
  return "$count ($amt) / $month = $perAcct / $acct";
}

/**
 * Return markup to identify the agent of the given account.
 */
function identifiers($acct) {
  $mya = r\acct();
  list ($agent, $company) = $acct->proSe ? array($acct, '') : array(r\acct($acct->agentId), $acct->fullName);
  $location = r\location($acct);
  $pic = ($mya and $mya->co) ? $acct->agent->photoHtml() : ''; // let companies see the picture
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
 * Present simple and advanced versions of date range selection, for display with optional download.
 * @param assoc $args: parameters passed upon form submission, perhaps including date range selection
 * @param string $url: page to submit to (gets returned, prefixed with base_url and with parameters appended)
 * @param array $choices: choices for selection of states to show
 * @param string $settings: comma-delimited string array of indexes into choices for state settings
 * @param bool $downloadable: <show option to download data>
 * @return assoc [dateRange, url, starting, ending]
 *   dateRange: formatted simple/advanced paired date-selection fieldsets, with submission buttons and states
 *   url: page to submit to
 *   starting: (int) starting date
 *   ending: (int) ending date
 *   states: (array) options to display
 */
function dateRange($args, $url, $choices = [], $settings = '', $downloadable = TRUE) {
  extract(u\just('period starting ending states', $args));
  u\setDft($states, $settings);
  if ($choices) {
    if ($states) foreach (u\ray($states) as $k) $stateDfts[] = $choices[$k];
    $states2 = boxesField('states', t('Show:'), '', @$stateDfts, $choices);
  }
  $showAdvJs = 'jQuery(\'#advanced\').show(); jQuery(\'#simple\').hide();';
  if ($states != $settings or (@$ending and $ending != strtotime('tomorrow') - 1)) js($showAdvJs, 'inline', 'footer'); // showing advanced selection
  u\setDft($period, TX_DEFAULT_PERIOD);
  list ($starting, $ending) = rangeDates(@$period, @$starting, @$ending);
  $url = BASE_URL . "/$url/period=$period&starting=$starting&ending=$ending&states=$states";

  $showAdvanced = item('<a class="nospin" href="javascript:void(0);" onclick="javascript:' . $showAdvJs . '">advanced</a>');
  $period = formField('select', t('For the past:'), '', dft($period), $GLOBALS['TX_DAYS']);
  $submitPeriod = submit(t('Show ')); // label has to be different from submitDates (see below)
  if ($downloadable) $downloadPeriod = submit(t('Download '));
  $simple = fieldSet('simple', compact(u\ray('period submitPeriod downloadPeriod showAdvanced')));
  
  $res = compact(u\ray('url starting ending states')); // the easy parts of the result
  $starting = textField(t('Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textField(t('Ending:'), '', dft(u\fmtDate($ending)));
  $submitDates = submit(t(' Show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $showSimple = item('<a class="nospin" href="javascript:void(0);" onclick="javascript:jQuery(\'#advanced\').hide(); jQuery(\'#simple\').show();">simpler</a>');
  if ($downloadable) $downloadDates = submit(t(' Download'));
  $advanced = fieldSet('advanced', compact(u\ray('states2 starting ending submitDates downloadDates showSimple')));
  return u\ray('dateRange', compact('simple', 'advanced')) + $res;
}

function dateRangeValidate(&$sta) {
  extract(u\just('period starting ending', $sta['input']));
  $op = op($sta);
  list ($starting, $ending, $period) = ($op == 'submitPeriod' or $op == 'downloadPeriod') ? rangeDates(@$period) : rangeDates('', @$starting, @$ending);
  u\preray(compact(u\ray('starting ending period')), $sta['input']);
}

function dateRangeRedirect($page, &$sta) {
  if (confirming_s($sta)) return;

  $sta['input']['states'] = @join(',', @$sta['input']['states'] ?: []);
  $url = "$page/" . http_build_query(u\just('states period starting ending', $sta['input']));

  if (u\abbreviates('download', op($sta))) {
    say('downloading');
    $url .= '&downloaded=1';    
  }
  $sta['redirect'] = $url;
}

/**
 * Return usable starting and ending dates (inclusive).
 * @param int $period: index into TX_DAYS -- for calculating starting and ending if none explicitly specified
 * @param int $starting: explicit starting date (optional)
 * @param int $ending: explicit ending date (optional)
 * @return array($starting, $ending, $period), where $starting is adjusted to start of day and $ending to end of day
 */
function rangeDates($period, $starting = '', $ending = '') {
  if (@$starting and !is_numeric($starting) and $err = u\badDate($starting)) say($err, 'starting');
  if (@$starting > r\rTime()) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and $err = u\badDate($ending)) say($err, 'ending');
  if (!@$ending or $ending > r\rTime()) $ending = r\rTime();

  $period = (int) @$starting ? ($ending - $starting) / DAY_SECS : (@$period ?: TX_DEFAULT_PERIOD);
  $ago = $period == '1' ? '1 day' : (@$GLOBALS['TX_DAYS'][$period] ?: ($period . ' days'));
  if (!@$starting) $starting = strtotime($period == '-1' ? '1jan' : ($period == '-2' ? '1/1/2013' : "$ago ago"));
  return array(strtotime('today', $starting), strtotime('tomorrow', $ending) - 1, $period);
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

function previous_state(&$sta, $message = '', $args = []) {
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
  if (u\isAcct($result = be\identify($who, $selfMsg))) return $result;

  list ($msg, $args, $choices) = $result;
  foreach (u\ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = urlencode(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
  foreach ($choices as $uid => $fullName) $choices[$uid] = r\quid($uid) . ' ' . $fullName;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, tt($msg, $args));
}
  
/**
 * Transfer funds
 * @param string $op: pay, charge, grant, loan, fine, or invest
 * @param array $info: associative array indexed by field names (who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($op, $info, $confirmed) {
  $mya = r\acct();
  $myid = $mya->id;
  extract($params = u\just('who amount goods purpose', $info));
  if (!isset($goods) or $goods === '') return say('required field', ['field'=>'"For"'], 'ERR'); // '' is for test
  u\EXPECT($params, 'string string int|empty string');
///NO!  if (!in_array($op, ['pay', 'charge'])) $goods = R_FOR_USD; // make sure this is set, for blank_field check
  $txType = $op == 'pay' ? 'payment' : strtolower($op);
//  if (blank_field($params)) return FALSE;
  if (!$a = w\whois($who, 'who', $info)) return FALSE;
//  if ($goods == R_FOR_GOODS and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
//  $op = $op == 'pay' ? t('paid') : ($op == 'charge' ? 'charged' : $op . 'ed'); // for possible error message
//  if (u\forCash($purpose)) $goods = R_FOR_USD;
  list ($msg, $args, $confirm) = 
     $txType == 'payment' ? be\transfer($txType, $mya, $a, $amount, $goods, $purpose, NULL, $confirmed)
     : ($txType == 'charge' ? be\invoice($mya, $a, $amount, $goods, $purpose, $confirmed)
     : communityTransfer($txType, $a, $amount, $purpose, $confirmed));

  if ($confirm) return @$identifiers . tt('confirm ' . $msg, $args);
  if (!@$args['success']) return say($msg, $args, 'amount');

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
  $msg = $success ? ($confirmed ? 'report funding' : 'funding') : 'funding error'; // 'funding' not used?!
  $confirmAction = ucwords($type);
  $did = $confirmAction . 'ed';
  $otherName = $acct->fullName;
  $amount = u\fmtAmt($amount);
  return array($msg, compact(u\ray('success did confirmAction amount otherName')), $success and !$confirmed);
}  

/**
 * Output the No-signin result message and terminate.
 * @param string $msg: the message or message index
 */
function doSay($msg, $type = 'error') {
  if (u\test()) file_put_contents(t\dosayFilename($type), $msg); // in case called from a test
  return r\go('empty/1', $msg, $type == 'error');
}

/**

  switch (@$channel) {
    case TX_WEB:
      \drupal_set_message($msg, 'error'); // this fails
      \drupal_goto('error/' . $R_FATAL_ERROR); // this fails -- use shutdown function to show error page (getting message from log)
    case TX_SMS: if (@$rsms_number) \sms_send($rsms_number, R_SYS_ERR); break;
    case TX_POS: be\api_error(R_SYS_ERR);
    default: di e('with no channel');
  }
 */
function error($logid) {
  if ($info0 = db\lookup('info', 'r_log', 'logid=:logid', compact('logid'))) {
    if (!$info = json_decode(utf8_encode($info0))) $info = $info0; // probably too big for json
  }
  r\go('signin/err=' . R_SYS_ERR);
}

/**
 * Add an item to the menu structure (called when reinstalling or rebuilding menus).
 * @param string $title: human-readable label for operation
 * @param int $type: menu item type (see hook_menu)
 * @param string $function: what function to call when item is activated (with or without namespace qualifiers)
 * @param string $args: space-delimited list of arguments to pass to the function
 * @param string $perms: space-delimited list of permission bit names (lowercase without B_ )
 * @return the menu item array as expected by Drupal for hook_menu
 */
function menu($title, $type, $function, $args, $perms = '') {
  global $menuWeight; $menuWeight += !@$menuWeight ?: 1;
  
  if (strpos($function, '\\') === FALSE) $function = "rCredits\\Web\\$function";
  $smart = strpos($function, '\\Smart\\');
  $admin = ($perms == 'dev' or stripos($perms, 'admin') !== FALSE);
  $file = $smart ? '../rsmart/rsmart.inc' 
  : ($admin ? '../admin/admin-forms.inc' 
  : 'rweb.inc');
  
  if ($perms == 'anyone') $perms = FALSE;
  foreach ($args = u\ray($args) as $k => $value) if (is_numeric($value)) $args[$k] = (int) $value;
  
  $item = array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $args,
    'access callback' => $perms ? 'rCredits\\Web\\webAccess' : TRUE,
    'access arguments' => $perms ? array($perms) : NULL,
    'weight' => @$menuWeight - 1,
    'menu_name' => 'main-menu', 
    'module' => $smart ? 'rsmart' : 'rweb',
    'file' => $file,
    'options' => u\ray('class', $title),
  );
  return $item;
}

/**
 * Return a help message for the page.
 */
function helpText($page, $tag = '') {
  include_once __DIR__ . '/rweb-help.inc';
  $page = strtolower($page);
  $help = $page == 'charge' ? strtr(rawHelp('pay', $tag), u\ray('pay', 'charge')) : rawHelp($page, $tag);
  if ($help) return u\SUBS(@$help);
  return tt('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at @regPhone. Or send your question by <a href="mailto:@regEmail?subject=help with rCredits Members Site, @page page" target="_blank">email</a>.', u\ray('regPhone regEmail page', u\fmtPhone(r\regionfield('phone')), r\regionfield('email'), $page));
}

/**
 * Complain about an input error if a required field is blank
 * @param array $fields: associative array of field names, with or without a prefix
 * @param string $prefix: UNUSED? optional field name prefix (add if missing, else remove from message)
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
 *
 * Note: the optional error field prevents form submission only if the field exists (so use "ERR" only when form submission is not an issue)
 *
 * @return FALSE (transfer() depends on this)
 */
function say($index, $args = [], $errField = '') {
  global $fieldErr; // used mostly in tests (and in formTx)
  if (!@$index) return;
///  if (is_array($index) and !isset($index[0])) debug($index);
  if (is_array($index)) list ($index, $args, $errField) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $errField) = array($errField, $args); // allow either order, for easy 2-param calls
  $noTable = (strpos($index, '<table') === FALSE);
  $message = $noTable ? tt($index, $args) : $index;
  $fieldErr = ($errField and $errField != 'ERR');
  if ($fieldErr) \form_set_error($errField, $message); else \drupal_set_message($message, @$errField == 'ERR' ? 'error' : 'status', FALSE);
  if ($noTable) u\loga($fieldErr ? 'sayerr' : 'say', compact('message'));
  return FALSE;
}

/**
 * Display charts representing the condition of the rCredits system in the given community.
 * @param string $whichChart: which chart or set of charts to display
 * @return: html of the relevant charts, in their proper divs
 */
function showCharts($whichChart) {
  global $base_url;
  if ($mya = r\acct()) {
    $myCtty = $mya->community ?: 0;
  } else $myCtty = 0;

  list ($in, $out, $fees, $net, $personal, $companies) = array(t('in'), t('out'), t('fees'), t('net'), t('personal'), t('companies'));
  extract($stats = r\stats($myCtty)); // careful with this (don't overwrite local vars)
  extract(w\fundStats($stats, TRUE)); // get the formatted versions, with originals in {var}0
  foreach (['txRewards0', 'spent0', 'trade0'] as $k) $$k = max(0, $$k); // don't display negative values
 
  $issuedData = array(
    "['for USD',{v:$usd0,f:'$usd'}]",
    "['inflation offset',{v:$inflations0,f:'$inflations'}]",
    "['signup bonuses',{v:$signups0,f:'$signups'}]",
    "['purchase reward',{v:$txRewards0,f:'$txRewards'}]",
    "['inviter/helper',{v:$helpers0,f:'$helpers'}]",
    "['spent (net)',{v:$spent0,f:'$spent'}]",
    "['grants',{v:$grants0,f:'$grants'}]",
    "['loans',{v:$loans0,f:'$loans'}]",
    "['invested',{v:$invests0,f:'$invests'}]",
//   "['fees',{v:$usd0,f:'$usd'}$fines]" think of a way to handle negatives (separate pie?)
    "['trade (net)',{v:$trade0,f:'$trade'}]",
  );
  $superTotal = u\fmtAmt($r0 + $usd0);

  $txTypes = u\ray('p2p p2b b2b b2p', t('Person-to-person'), t('Person-to-business (mostly by rCard)'), t('Business-to-business (paying suppliers)'), t('Business-to-person (including payroll)'));

  $avg = u\fmtAmt($txsPerMo ? $perMo / $txsPerMo : 0);
  $txs = "$txsPerMo @ $avg avg";

  $now = r\rTime();
  $created0 = db\lookup('MIN(created)', 'r_stats'); // show every n days since beginning + daily past n days
  $sql = <<<EOF
    SELECT created, IF(r=0, 0, perMo/r) AS rate, r AS funds, bankIn-bankOut AS usd, savings,
      pAccts, bAccts, pAccts+bAccts AS accts, newbs, top5,
      bankInPerMo, bankOutPerMo,
      perMo, p2bPerMo, b2bPerMo, b2pPerMo,
      txsPerMo, p2bTxsPerMo, b2bTxsPerMo, b2pTxsPerMo
    FROM r_stats WHERE ctty=$myCtty
    AND (DATEDIFF(FROM_UNIXTIME(:now), FROM_UNIXTIME(created))<:STATS_GRAIN OR DATEDIFF(FROM_UNIXTIME(created), FROM_UNIXTIME(:created0))%:STATS_GRAIN=0)
    AND created>:R_LAUNCH ORDER BY created
EOF;
  $q = db\q($sql, compact('created0', 'now'));

  $charts = u\ray('accts funds velocity banking tx issued', 'growth', 'funds', 'velocity', 'demand', 'economic-circles', 'issuing-rcredits');
  if ($whichChart != 'all') $charts = u\just($whichChart, $charts);
  foreach ($charts as $k => $zot) u\setDft(${$k . 'Data'}, []);
  $moneyFields = u\ray('funds savings top5 usd bankInPerMo bankOutPerMo p2pPerMo p2bPerMo b2bPerMo b2pPerMo');

  while ($row = $q->fetchAssoc()) {
    extract($row);
    list ($y, $m, $d) = u\ray(strftime('%Y %m %d', $created));
    list ($m, $d) = array($m - 1, $d - 1); // javascript Date object month, day, etc. are zero-based
    $date = "new Date($y,$m,$d)";
    $p2pPerMo = $perMo - $p2bPerMo - $b2bPerMo - $b2pPerMo;
    $p2pTxsPerMo = $txsPerMo - $p2bTxsPerMo - $b2bTxsPerMo - $b2pTxsPerMo;
//    $actives = abs($pAccts - $newbs); // active members
    foreach ($moneyFields as $k) ${$k . '2'} = u\fmtAmt($$k);
    $rate2 = number_format(100 * $rate, 1) . '%';

    $acctsData[] = "[$date,$pAccts,$newbs,$bAccts]";
    $fundsData[] = "[$date,{v:$funds,f:'$funds2'},{v:$usd,f:'$usd2'},{v:$savings,f:'$savings2'},{v:$top5,f:'$top52'}]";
    $velocityData[] = "[$date,{v:$rate,f:'$rate2'}]";
    $bankingData[] = "[$date,{v:$bankInPerMo,f:'$bankInPerMo2'},{v:$bankOutPerMo,f:'$bankOutPerMo2'}]";

    $x2x = [];
    foreach ($txTypes as $k => $desc) {
      list ($x, $x2, $xTxs) = array(${"{$k}PerMo"}, u\fmtAmt(${"{$k}PerMo"}), ${"{$k}TxsPerMo"});
      $x = max($x, 1); // to make the logarithmic scale look okay
      $avg = $xTxs ? ' @ ' . u\fmtAmt($x / $xTxs) . ' avg' : '';
      $x2x[] = "{v:$x,f:'$x2 ($xTxs$avg) $desc'}"; // eg "$123 (7 @ $6.25 avg)"
    }
    $x2x = join(',', $x2x);
    $txData[] = "[$date,$x2x]";
  }

//  $accts = "{$pAccts} members + {$bAccts} co = " . ($pAccts + $bAccts);
  $accts = "{$pAccts} members + {$bAccts} co";
  $netUsdIn = u\fmtAmt($bankInPerMo - $bankOutPerMo) . t(' (net)');
  $velocity = number_format(($r0+0) ? 100 * $perMo / $r0 : 0, 1) . t('% per mo.');
  
  $js = <<<EOF
  var accts = '$accts';
  var funds = '$superTotal';
  var velocity = '$velocity';
  var netUsdIn = '$netUsdIn';
  var issued = '$r';
  var fees = '(minus $fines in fees)';
  var txs = '$txs';
  google.setOnLoadCallback({$whichChart}Chart);  
EOF;

  foreach ($charts as $k => $zot) {
    ${$k . 'Data'} = join(',', ${$k . 'Data'});
    $js .= "var {$k}Data = [" . ${$k . 'Data'} . "];\n";
  }

  js('https://www.google.com/jsapi', 'external', 'header');
  js('js/charts.js', 'file', 'header', 8);
  js($js, 'inline', 'header', 9);

  foreach($charts as $k => $help) {
    $one = "<div id=\"{$k}Chart\" class=\"onechart\"></div>";
    if (count($charts) > 1) $one = "<a href=\"$base_url/help/$help\">$one</a>";
    $charts[$k] = "<div>$one</div>";
  }
  return u\test() ? $js : join('', $charts);
}

/**
 * Return an array of formatted amounts for the given statics.
 * Used in graphs and table of community funds
 */
function fundStats($stats, $includeRaw = FALSE, $format = 's$') {
  extract($stats);
//  list ($r0, $usd0) = array($r, $usd);
  $trade = $tradeIn + $tradeOut; // tradeIn is negative
  $spent = $cttyBuys + $cttyIncomes; // cttyIncomes is negative
  $txRewards = $rebates + $bonuss + $fines; // $fines is negative
  $usd = $bankIn - $bankOut;

//  foreach (u\ray('bankInPerMo bankOutPerMo r usd total signups inflations helpers txRewards grants loans fines') as $one) {
  $res['ctty'] = $ctty ? trim(strtr(r\acct($ctty)->fullName, [t('Region')=>'', t('rCredits')=>''])) : t('TOTAL');
  foreach (u\ray('r usd trade signups txRewards helpers inflations spent grants loans invests fines') as $one) {
    if ($includeRaw) $res[$one . '0'] = $$one;
    $res[$one] = u\fmtAmt($$one, $format);
  }
  return $res;
}

/**
 * Connect a bank account to the given rCredits account.
 *//*
function connectBank($a) {
  $secure = $a->secure;
  if (!$bankAccount = @$secure['bankAccount']) return say(t('Member has not yet entered bank info.'), 'zot');
  if ($a->country == US_COUNTRY_ID) {
    $routingNumber = substr(@$bankAccount, 4, 9); // chop of USkk
    $bankAccount = substr(@$bankAccount, 4 + 9); // everything after the routing number
  }
  if (!@$secure['auth']) return say(t('You must first set up the Dwolla account for this member.'), 'zot');
  $us = new r\usd($a);
  if (!$us->addBank($bankAccount, @$routingNumber, $secure['bankAccountType'], $err)) return say($err, 'zot');
  say('info saved');
}
*/

/**
 * Return a form showing month choices.
 * @param string $title: page title
 * @param string $things: "statements" or "notices"
 * @param string $subtext: explanatory text
 * @param int $end: how many months from now to end (starts with account creation month)
 * @param string $target: link target ('' or _blank)
 */
function monthChoices($title, $things, $subtext = '', $end = -1, $target = '_blank') {
  $mya = r\acct();
  $start = u\monthDay1($mya->created); // start of account's first month
  $thisMo = u\monthDay1(); // start of this month
  $month = $end ? u\plusMonths($end, $thisMo) : $thisMo;
  
  for (; $month >= $start; $month = u\plusMonths(-1, $month)) {
    $year = date('Y', $month);
    $text = strftime('%b%Y', $month);
    $list[$year] = @$list[$year] . "<a href=\"$things/$text\" target=\"$target\">$text</a><br>\n";
  }
  if (@$list) {
    $headers = $data = '';
    foreach ($list as $year => $v) {
      $headers .= "<th>$year</th>";
      $data .= "<td>$v</td>";
    }
    $list = "<table id=\"$things\">\n<tr>$headers</tr>\n<tr>$data</tr>\n</table>";
  } else $list = t('There are no @THING available yet for this account.', ['@THING' => strtolower($title)]);
  
  $title = item($title);
  if ($subtext) $subtext = item($subtext);
  $list = item($list);
  return compact('title', 'subtext', 'list');
}

/**
 * Create a menu entry (see rweb.module)
 */
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

/**
 * Consolidate or extract hidden fields.
 */
function hidfields($ray) {return is_array($ray) ? hidField(serialize($ray)) : unserialize($ray);}

/**
 * Send an invitation to the email and optionally report it.
 */
function sendInvite($email, $subject, $message, $dupsOk = TRUE) {
  $mya = r\acct();
  if ($admin = $mya->admin) $mya = r\acct(R_ADMIN_QID); // use William for admin invites
  
  if (!$dupsOk) { // applies only to community admins
    $cemail = u\cryptN($email);
    $now = r\rTime();
    if (db\exists('r_invites', 'email=:cemail and :now-invited<:R_INVITE_DAYS*:DAY_SECS', compact('cemail', 'now'))) return say("NOT sent to $email (dup).");
//     if (db\exists('users', 'email=:email', compact('email'))) return say("NOT sent to $email (already a member).");
  }

  $info = array(
    'uid' => $mya->id,
    'noFrame' => TRUE,
    'subject' => $subject,
    'PERSONAL_NOTE' => str_replace("\n", "<br>\n", $message), 
    'MEMBER_NAME' => $mya->fullName, 
    'MEMBER_PHONE' => u\fmtPhone($mya->phone),
    'DATE' => u\fmtDate($mya->signed),
    'CODE' => r\invite($email, $mya->id, '', $subject, $message),
  );
  r\rMail('invite', $email, $info, $mya);
  say(!$dupsOk ? 'Sent to @email' : 'invite sent|repeat invite', compact('email'));
  if (!$admin) r\tellStaff('invited', $info + compact('email'), $mya->id);
}

function focusOn($focusFieldName) {
  u\EXPECT(!empty($focusFieldName), 'focus on null');
  $focusFieldName = str_replace('_', '-', $focusFieldName);
  js("document.getElementById('edit-$focusFieldName').focus();", 'inline', 'footer');
}

/**
 * Return true if the current account has all the accesses listed.
 */
function webAccess($perms = NULL) {
  if ($perms == 'dev' and isDEV) return TRUE;
  setGlobals(); // remember who we are (the main reason for this function)
  if (!@$perms or !$mya = r\acct()) return FALSE;
  if ($perms == 'signedIn') return TRUE;

  foreach (u\ray($perms) as $perm) {
    $not = (substr($perm, 0, 1) == '-');
    $hasPerm = $mya->can(u\consta('b', $not ? substr($perm, 1) : $perm));
    if ($not ? $hasPerm : !$hasPerm) return FALSE;
  }
  return TRUE;
}

function css($sheet, $weight = -99) {
  \drupal_add_css(R_PATH . "/rweb/$sheet", ['group' => CSS_THEME] + weight($weight));
}

/**
 * Insert a javascript file or snippet into the form
 * @param string $what: the filepath or snippet
 * @param string $type: inline, file, or external
 * @param string $scope: header or footer
 * @param int $weight: low means load sooner (-18 and up, defaults to 0)
 */
function js($what, $type = 'inline', $scope = 'footer', $weight = 0) {
  global $rUrl;
  if ($type == 'file') $what = (strpos($what, 'http') !== FALSE or strpos($what, ':')) ? $what : "$rUrl/$what";
  return \drupal_add_js($what, compact('type', 'scope', 'weight'));
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
function tt($message, $subs = []) {return u\tt($message, $subs);}
//function block_def($info, $extra = []) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function blockView($subject, $content) {return compact(u\ray('subject content'));}

function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($default_value) {return compact('default_value');}
function weight($weight) {return compact('weight');}
function attrib($attributes) {return compact('attributes');}
function suffix($suffix = '') {$suffix .= '<div class="clearfix"></div>'; return compact('suffix');}
function prefix($prefix) {return compact('prefix');}
function autocomplete($type = '') {return array('autocomplete_path' => "autocomplete/$type/" . @r\acct()->id);}
function required($dft = NULL) {return ['required' => TRUE] + (isset($dft) ? dft($dft) : []);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {global $testOp; return u\test() ? $testOp : (svar('button') ?: @$sta['clicked_button']['#parents'][0]);}
function verifyBy($byVoice) {return $byVoice ? t('Voice') : t('SMS');}
function blurbLink() {global $rUrl; return "<a href=\"$rUrl/templates/invite.html\" target=\"_blank\">description of rCredits</a>";}
function sayFieldErr($err, $field) {return say(strtoupper($field) . ': ' . tt($err), $field);}
function popHelp($text, $help) {return "<span title=\"$help\">$text</span>";}
