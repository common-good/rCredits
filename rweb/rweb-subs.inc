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
define('R_AGREE_0', -2); // number of the first point in the rCredits Agreement
define('R_AGREE_9', 6); // number of the last point in the rCredits Agreement
define('REQ', '*'); // marks required fields (use this as start of field's label)
define('BARE', '-'); // marks bare (unwrapped) fields -- no div wrapper around them

$GLOBALS[TX_WEB . ' messages'] = array(
  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Mobile number @number is now connected to this account.'),
  'report delete cell' => t('Mobile number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>So far so good. <b class="stopnot">But WAIT!</b> You have a few steps still to go. We sent you an email with your user ID and a link to the next step. If you don\'t see it within 60 seconds, check your spam folder.</p>'),
  'company is ready' => t(<<<EOF
    <p></p><p>After you finish setting up this account, to manage it in the future:</p>
    <ol>
      <li>Sign in to your personal account at rCredits.org</li>
      <li>Click on your photo in the upper right corner. Choose your company account from dropdown there.</li>
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
  'got photo' => t('Your photo was successfully uploaded.'),
  'company rcard' => t('Send a <a href="@href">company rCard</a> for @otherName to @manager at @address.'),
  'amount rounded' => t('"@amount" was rounded to the nearest integer value.'),
  'invite sent' => t('Your invitation has been sent.'),
  'repeat invite' => t('Come back any time, to send more invitations (click "Invite Someone" at the bottom of any page).'),
  'gift transfer later' => t('<br><br>Funds will be transferred when you have money in your account.'),
  'repeat gift' => t('If you wish to make a donation again sometime, click "Membership" in the upper right corner, then on "Donate".'),
  'setup complete' => t('<p>Your account setup is complete. Expect an email within the next 24 hours, saying your account is activated. Your rCard will arrive by US Mail within a few days. Thank you for joining!</p>'),
  'use temporary card' => t('<p>While you wait for your rCard to arrive, you can use your invitation card to make purchases at any member business (beginning with your inviter).</p>'),
  'must be confirmed' => t('An email has been sent to @inviterName. They will need to confirm the invitation before you can use your rCredits account. Or you can make your first rCard purchase from them (or sale to them), as confirmation.'),
  //  Meanwhile, click the "<a href="@BASE_URL/community/invite">Invite</a>" tab to invite anyone you know and trust &mdash; you get a $@R_HELPER_BONUSr reward when they make their first purchase.<br><br>
  'ready to charge' => t('<p>You successfully scanned in as "@agentName" to charge customers on behalf of @companyName.</p><p>Now go back to your scanning app, to scan a customer\'s card. Sign out only when your shift ends.</p>'),
  'transaction list truncated' => t('There are too many transactions to show all at once, so only the first @TX_MAX_ROWS are shown (choose a different date range to see the rest).'),
  'address info' => t('<a1>Click here</a> for address information.'),
  'agent created' => t('The agent account was successfully created.'),
  'phone verified' => t('Your phone number has been verified.'),
  'payroll done' => t('<br>Payroll upload was successful. You paid @count employees a total of @total.'),
  'tx edited' => t('Your transaction #@tid was edited by @who.'),
  'new tx goods' => t('It is now for @what.'),
  'new tx amount' => t('The new amount is @amount.'),
  'expired invite converted' => t('Your invitation has expired, so @inviterName will be asked to re-confirm.'),
  'new savings balance' => t('<p>IMPORTANT! Your rCredits balance has been split to include a Credit Reserve containing your incentive rewards. Read <@a>last week\'s rCredits newsletter</a> for details.</p>', ['@a' => 'a href=http://us9.campaign-archive1.com/?u=b1d89f860859e1d9b906fa0b0&id=f1f54edd80&e=']),
  'minimum zeroed' => t('<p>Your rCredits "automatic refills" have been turned off, to prevent any unintended transfer from your bank account. To turn them back on or transfer funds manually, sign in to your account at rCredits.org OR <@a1>click here</a>.</p>'),
  'sent message' => t('Your message has been sent to @fullName.'),
  
  // prompts
  'note' => '<b class="loud">' . t('NOTE:') . '</b>',
  'ach warning' => t('If there are insufficient funds in your bank account when you request a transfer to your rCredits account (either explicitly, or by automatic refills), your bank will charge you a $20-35 fee, just like for a bounced check. If this happens more than once, the rCredits system will <i>also</i> charge you $@fee, to cover <i>its</i> bank fee. So make sure you have enough in your bank account to cover your transfers.', ['@fee'=>R_DEPOSIT_RETURN_FEE]),
  // If you don\'t want any automation, type zero (then remember to refill your account explicitly when it gets low).'),
//  'prompt max' => t('Keep AT MOST this much in my rCredits account. (When your rCredits balance goes above this amount by $@R_CHUNK or more, the system transfers the extra to your bank account. Blank means no automatic transfers &mdash; recommended in most cases).'),
  'legalname desc' => t(' full legal name, properly capitalized (for tax reporting).'),
  'usename desc' => t('This should be the name you use in formal letters, at work, on your checks and credit cards, etc.'),
  'ssTitle' => t('Soc Sec #:'),
  'einTitle' => t('Federal ID:'),
  'ssDesc' => t('Your social security number (for tax reporting)'),
  'einDesc' => t('The company\'s Employer ID Number (EIN)'),
  'to scan another' => t('To scan another card, run the QR Scanner app again.'),
  'invite links' => t('<p>Here are your links, to include in your own email messages to your friends. Be sure to mention they can visit rCredits.org for more information.</p><p>@links</p>'),
  'timed out' => t('You were automatically signed out, to protect your account(s).'),

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
  'duplicate email' => t('"@who" is already using that email address.'),
  'email plus' => t('If you want to use the same email address for this account, add a plus sign (+) and an arbitrary tag to the local part, for example "@emailTagged".'), // used in user.module
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
  'increase min' => t('<@a>Click here</a> to increase your minimum balance!', ['@a'=>'a href=' . BASE_URL . '/settings/preferences']),
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
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click the one you want.'),
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
  'bad invite' => t('That is not a valid Invitation Number or username.'),
  'used invite' => t('<p>That invitation has already been used, to open an rCredits account.</p><p>If you already have an rCredits account set up and want to open an additional account for yourself, your business, or a family member, sign in and use the "Open a Company Account" button on the Summary page (for cashier accounts, first click your photo in the upper right corner and select your business from the dropdown).</p><p>Otherwise ask someone you know to send you a new invitation!'),
  'expired invite' => t('<p>That invitation has expired.</p><p><@a>Click here</a> to ask @inviterName for another invitation.</p>'),
  'min sub floor' => t('You cannot choose a Target Balance less than @floor.'),
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
  'bad login' => t('That account/password combination does not exist. Try again or <!a>ask for a new password</a>.', ['!a' => 'a href="' . BASE_URL . '/settings/password"']),
  'wrong pass' => t('That is not your correct password.'),
  'multiple spaces' => t('@field cannot have more than one space in a row.'),
  'illegal char' => t('@field contains an illegal character. Try retyping it'),
  'too long' => t('@field is too long: it must be @USERNAME_MAX_LENGTH characters or less.'),
  'weak pass' => t('You need to choose a password that is harder to guess.'),
  'mismatch' => t('You must have mistyped your @thing once. Try again.'),
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
  'change min first' => t('That request would leave you with less than your minimum balance. <a href="@BASE_URL/settings/preferences">Lower your minimum</a> first.'),
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
  'bad payroll employee' => t('@employee is mispelled, does not have an active rCredits account, or is not listed as an employee on your Relations page. Click Settings, then Relations.'),
  'illegal amount change' => t('You cannot @action the original amount (@amount). <@a>Send an invoice</a> instead.'),
  'bad achmin' => t('The minimum transfer amount must be at least $@R_ACHMIN.'),
  'say why not' => t('You must say why you are denying the invoice.'),
  'no co pass' => t('Company accounts do not have passwords. Sign in to your personal account, then click your photo in the upper right corner and select @company from the dropdown.'),
  'account number mismatch' => t('You must have mistyped your account number once. Try again.'),
  'yes or no' => t('You must answer Yes or No.'),
  'no local proxy' => t('No other members nearby. <@a>Invite someone</a>!', ['@a'=>'a href=' . @BASE_URL . '/community/invite']),
  'too many joins' => t('You can join to only one other account.'),
  'short cash help' => t('<@a>Click here</a> for more information about cashing out incentive rewards.', ['@a'=>'a href=' . BASE_URL . '/help/cashing-out']),
  'maybe not cash' => t('If you are actually reimbursing someone or paying them for goods or services (for example, employee labor), choose a different button under "For".'),
  'link expired' => t('That link has expired.'),
  'login failed' => t('Your attempt to sign in failed. Click the "Password problems" link if you need a new password.'),
  'file save error' => t('Your file upload did not succeed.'),
  'forced without perm' => t('<p>An unauthorized transaction was made using a Company Agent rCard.</p><ul><li>Date: @date</li><li>Customer: @a2 (by @agent2)</li><li>Company: @a1 (by @agent1)</li><li>Amount: @amount</li><li>For: @for</li></ul><p>This transaction was taken offline, so there was no way to know the agent lacked authorization.</p>'),
  'bad invite num' => t('That is not a valid username or invitation number.'),
  'negative saveWeekly' => t('Your discretionary savings are already at their minimum, so your weekly savings addition cannot be negative.'),
  'requires signin' => t('You must be signed in, to use that page.'),
//  'saveWeekly too low' => t('With a negative "minimum", your weekly savings addition must be at least @min.'),
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
function proxyChoices($a, $dft) {
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
    $style = $weight < PHP_INT_MAX / 4 ? 'default'
    : ($weight == PHP_INT_MAX / 4 ? 'slight'
    : ($weight <= PHP_INT_MAX / 2 ? 'primary'
    : 'success' ));
    if ($dft == $uid) $style = 'warning';
///    debug(compact('row','city','cityX','friends') + ['max/2-w'=>PHP_INT_MAX / 2 - $weight]);
    $res .= <<<EOF
<button type="submit" name="i-$uid" class="btn btn-xs btn-$style ladda-button" data-style="zoom-out">
    <span class="ladda-label">$fullName</span></button>    
EOF;
  }
  if ($res) $res .= '</td></tr>'; // finish last line
  return "<div class=\"well\"><table>$res</table></div>";
}

function htmlItem($name, $id, $value, $class, $other = []) {return $value;}

function htmlText($name, $id, $value, $class, $other = []) {
  extract(u\just('tribs invisible id prefix suffix inline class box disabled autocomplete_path', $other));

  $class = [@$class];
  $class[] = 'form-control input-md';
  if ($extra = rcAuto(@$autocomplete_path, $id)) $class[] = 'form-autocomplete';

  $tribs = u\tribs(compact('type', 'class') + (@$tribs ?: []));
  return "<input $tribs />" . $extra;
}
function htmlPassword($name, $id, $value, $class, $other = []) {
  return htmlText($name, $id, $value, $class, ['type' => 'password'] + $other);
}
function htmlHidden($name, $id, $value) {
  return htmlText($name, $id, $value, '', ['type' => 'hidden']);
}

function htmlSubmit($name, $id, $value, $class, $other = []) {
  extract(u\just('tribs size style', $other));
  return <<<EOF
  <button type="submit" id="$id" name="op" class="btn btn-$style btn-$size ladda-button" data-style="expand-right">
    <span class="ladda-label">$value</span>
  </button>
EOF;
}

function htmlButton($name, $id, $value, $class, $other = []) {
  extract(u\just('tribs href class', $other));
  
  if (strpos($href, 'http') === FALSE) $onclick = $href;
  if ($href) list ($tag, $role) = ['a', 'button']; else $tag = 'button';
  return u\tag($tag, $value, (@$tribs ?: []) + compact(@$onclick ? 'onclick' : 'href', 'class', 'role'));
}

/**
 * Create HTML for a form or page element (field).
 * @param string $type: what type of field
 * @param string $label: how to label the field
 *    if label begins with '*' the field is required
 *    if the first or second character of label is '-', don't wrap the field in a div wrapper
 * @param array $desc: how to describe the field: [hint, help text, hover text]
 *    hover text defaults to same as hint
 * @param mixed $other: assoc of additional settings
 * @return the HTML
 */
//  if (!is_array($other)) $other = ['markup' => $other]; // move this to item
// * @param mixed $choices: array or assoc of choices // move this to htmlSelect
//    if (!isset($x_default)) $x_default = $choices ? key($choices) : ''; // Drupal's select fails without this

function NEWfld($type, $label, $desc = '', $other = []) {
///  debug(compact(u\ray('type label desc other')));
  if (@$label[0] == REQ) list ($required, $label) = ['yes', substr($label, 1)];
  if ($bare = (@$label[0] == BARE)) $label = substr($label, 1); // bare? (no div wrapper)

  @list ($placeholder, $help, $title) = is_array($desc) ? $desc : ['', $desc];
  if (!@$title) $title = $placeholder; // hover text defaults to same as hint
  foreach (['placeholder', 'title'] as $k) if ($$k) $tribs[$k] = $$k;

  return compact(u\ray('type label required bare help tribs other')); // see rend()
}

// only barely begun
function rendForm($form) {
  $guts = rend($form);
  return str_replace('rcreditswebform', 'r-', u\tagN('form', $guts, $form['#attributes']));
} 

function NEWlabeled($form, $onSubmit = '', $labeled = TRUE) {
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
//    $form[$k]['#tabled'] = TRUE;
//    if (@$form[$k]['#type'] == 'fieldset') $form[$k] = labeled($form[$k], $onSubmit, $level + 1);
    $all[$k] = $v;
    unset($form[$k]);
  }
  $form['all'] = oldItem(rend($all));
  return $form;
}

function labeled($form, $onSubmit = '', $labeled = TRUE, $level = 0) {
  if ($onSubmit) {
    list ($zot1, $zot2, $caller) = explode('\\', u\caller());
    $formId = 'rcreditsweb' . strtolower($caller);
    w\js("$('#$formId').submit(function() { $onSubmit });", 'inline'); // spaces are required around $onSubmit
    // (Drupal kills the quotes) $form['#attributes']['onsubmit'] = $onSubmit; // eg ' return func();'; 
  }
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
    $type = @$v['#type'];
    if ($type == 'hidden') t\POST([$k => $v['#value']]); // fake post for tests
    if ($type == 'fieldset') $form[$k] = labeled($form[$k], $labeled, $level + 1);
    $form[$k]['#tabled'] = TRUE; // remember that form is laid out in a table (labels in left column)
  }
  return $form;
}

/**
 * Drupal render the given field, with the given name.
 * @param string $name: field name
 * @param assoc $field: a Drupal field description array
 * @return Drupal rendering, with repairs.
 */
function rendA($name, $field) {
//    $rent .= preg_replace('/name="(.*)"/', "name=\"$k\"", rent($field));
  return preg_replace('/<(input|textarea|select) /', "<$1 id=\"edit-$name\" name=\"$name\" ", rent($field));
}
    
function rend($fields) {
  $html = '';
  foreach ($fields as $name => $ray) {
    extract(u\just('type bare other', $ray)); // see fld()
    extract(u\just('id value class', $other));
    u\setDft($id, "edit-$name");
    $func = 'rCredits\\Web\\html' . ucfirst($type);
    $fldHtml = $func($name, $id, @$value, @$class, @$other);
    $html .= $bare ? $fldHtml : wrap($name, $fldHtml, $ray);
  }
  return $html;
}

function wrap($name, $html, $ray) {
  extract(u\just('type label required bare help other', $ray)); // see fld()
  extract(u\just('invisible id prefix suffix inline class box disabled', $other));
  $for = strtr(@$name, ['_'=>'-', '['=>'-', ']'=>'']);
  if ($for == 'title') $html = "<h3>$html</h3>";
  $html = @$field_prefix . $html . @$field_suffix;

  $boxy = in_array($type, ['radio', 'checkbox']) ? $type : '';

  $class = [@$class];
  $class[] = $boxy ? 'option' : 'control-label col-sm-offset-1 col-xs-2';
  if (@$invisible) $class[] = 'sr-only';
  $label = u\tag('label', $boxy ? $html . @$label : @$label, compact('for', 'class'));
    
  if (@!$bare) {
    $help = @$help ? "\n      <div class=\"help-block\">$help</div>" : '';
    $lineup = $type == 'item' ? ' lineup' : '';
    $core = <<<EOF
    $label
    <div class="control-data col-xs-10 col-sm-6$lineup">
      $html$help
    </div>
EOF;
  } else $core = $boxy ? $label : $label . $html;

  $id = @$lineup ? " id=\"edit-$for\"" : '';
  $outerClass = "form-group wrap-$for";
  $outerClass .= ' ' . ($boxy ?: "type-$type");
  if (@$box) $outerClass .= ' box1';
  if (@$disabled) $outerClass .= ' disabled';
  
  return <<<EOF
  <div$id class="$outerClass">
$core
  </div>
EOF;
}
  
// old
function fld($type, $title = '', $help = '', $other = [], $options = NULL) {
  $label = $title;
  if (@$label[0] == REQ) list ($required, $label) = ['yes', substr($label, 1)];
  if ($bare = (@$label[0] == BARE)) $label = substr($label, 1); // bare? (no div wrapper)
  if (@$other['wrap']) $bare = FALSE;
  $title = $label;

  if (!is_array($other)) $other = [$type == 'item' ? 'markup' : 'value' => $other];
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = $options ? key($options) : ''; // Drupal's select fails without a default
  $title_display = 'before';
//  if (isset($other['value']) and !isset($other['default_value'])) $other['default_value'] = $other['value'];
  $field = u\prefixKeys('#', $other + compact(u\ray('type title title_display required bare help options')));
  return $field;
}

// Plan: move all theme functions here. Output each page as a single item(). Each field type function should have a nogroup option (start label with -) and an easier required option (start label with star)
function submit($title = '', $style = '', $size = '', $other = []) {
  if (!$style) $style = 'primary';
  if (!$size) $size = 'md';
  $other += [
    'input' => TRUE,
    'name' => 'op',
    'button_type' => 'submit',
    'executes_submit_callback' => TRUE,
    'limit_validation_errors' => FALSE,
    'process' => array('ajax_process_form'),
    'theme_wrappers' => array('button'),
  ];
  if (@$title[0] == BARE) list ($bare, $title) = [BARE, substr($title, 1)];
  return fld('submit', @$bare, '', $other + ['value' => $title ?: t('Save')] + compact('style', 'size'));
}
function submi($title = '', $style = '', $size = 'xs', $other = []) {
  return submit(BARE . $title, $style, $size, $other);
}

function buttonFld($value, $href = '', $title = NULL, $style = 'success', $size = 'xs') {
  $title = htmlspecialchars($title);
  $class = "btn btn-$style btn-$size";
  return NEWfld('button', BARE, ['', '', $title], compact(u\ray('href value class')));
}
function button($value, $href = '', $title = NULL, $style = 'success', $size = 'xs') {
  return rend([buttonFld($value, $href, $title, $style, $size)]);
}

function NEWitem($value, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('value') + $other);}
function item($markup, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('markup') + $other);}
function hidFld($value = '', $other = []) {return fld('hidden', '', '', compact('value') + $other);}
function NEWtextFld($label = '', $desc = '', $other = []) {return fld('text', $label, $desc, $other);}
function textFld($label = '', $desc = '', $other = []) {return fld('textfield', $label, $desc, $other);}
function passFld($label = '', $desc = '', $other = []) {return fld('password', $label, $desc, $other);}
function areaFld($label = '', $help = '', $other = []) {return fld('textarea', $label, $help, $other);}
function radiosFld($label, $help, $other, $options) {return fld('radios', $label, $help, $other, $options);}
function selectFld($label, $help, $other, $options = []) {return fld('select', $label, $help, $other, $options);}
function htmlFld($label = '', $help = '', $dft = '', $other = []) {return fld('text_format', $label, $help, ['format' => 'filtered_html'] + dft(strtr($dft, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])) + $other);}

function boxFld($name, $label = '', $desc = '', $dft = NULL, $other = []) {
  $fld = fld('checkbox', $desc, '', compact('name') + dft(@$dft) + $other);
  return $label ? item(@\render($fld), $label, '', ['box' => TRUE]) : $fld;
}

/**
 *
 * @see also boxesFld()
 */ 
function boxFlds($group, $label, $question, $options, $other = []) {
  $question = $question ? "<p>$question</p>\n" : '';
  $list = '';
  foreach ($options as $k => $v) {
    $list .= <<<EOF
<div class="row">
<div><input type="checkbox" id="edit-$group-$k" name="{$group}[$k]" /></div>
<div>$v</div>
</div>
EOF;
  }
  return item($question . $list, $label, '', ['class' => ['boxes']] + $other);
}

function fieldSet($id, $flds, $label = '', $other = []) {return fld('fieldset', $label, '', compact('id') + $other) + $flds;}

/**
 * Return a form field for sharing percentage.
 */
function shareFld() {
  $mya = r\acct();
  $sharePct = round(db\lookup('AVG(share)', 'r_gifts', 'share>=0'), 1);
  $shareDft = $mya->prefsSet ? (($mya->share + 0) ?: 0) : R_SHARE_DFT;
  return textFld(t('Share:'), t('<p>What percentage of your ongoing purchase rewards would you like to donate to Common Good Finance (CGF), to support the rCredits system? This is another way to take responsibility for our community\'s future. You can put anything from 0% to 50%. For example, if you choose 30%, then when you make a $10 purchase and get your $1 reward, your "sharing" contribution is 30 cents (30% of $1). The current average sharing percentage chosen by members is sharePct%.</p><p>Note that this is not "giving back" part of your rewards to the rCredits community &mdash; rather it is an optional extra donation to CGF. CGF is a nonprofit member company, not the system itself. CGF promotes rCredits and handles administration of your rCredits community until the community can take over that responsibility.</p>', compact('sharePct')), required($shareDft . '%'));
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
 * @param string $help: helpful description of field
 * @param mixed $dft: the default value (TRUE or FALSE) or an assoc of attributes
 * @param array $labels: labels for the two choices (defaults to [No, Yes])
 */
function boolFld($title = '', $help = '', $dft = NULL, $labels = NULL) {
  $class = ['yesno'];
  $inline = TRUE;
  $dft = is_null($dft) ? [] : (is_array($dft) ? $dft : dft((@$dft + 0) ?: 0));
  return radiosFld($title, $help, $dft + compact('class', 'inline'), @$labels ?: array(t('No'), t('Yes')));
}

/*
 * Create a rendered checkboxes field
 * @param string $title:
 * @param string $desc:
 * @param array $defaults: simple array of names of already-checked boxes
 * @param array $choices: name for each checkbox
 * @param array $aliases: name to display for each checkbox (defaults to choice name)
 * @see also boxFlds()
 */
function boxesFld($title, $desc, $defaults, $choices, $aliases = NULL) {
  u\setDft($aliases, $choices);
  if (!@$defaults) $defaults = [];
  return fld('checkboxes', $title, $desc, compact('choices', 'defaults', 'aliases'), []);
}

/**
 * Return a set of checkboxes representing bits in a bit array.
 * @param string $label: short prompt for the set of boxes
 * @param int $bitRay: the bit array (current value)
 * @param string $prefix: prefix for defined constants for this bit array field (empty means use $bits index)
 * @param string $bits: array (space or comma-delimited) of names of bits to include (without prefixes)
 * @param assoc $aliases: map of bit names to their display aliases (which may include formatting)
 */
function bitsFld($label, $bitRay, $prefix, $bits, $aliases = []) {
  $bits = u\ray($bits);
  if (!is_numeric(key($bits))) $bits = array_keys($bits); // ignore values (eg weights for risk arrays)
  $defaults = [];
  foreach ($bits as $bit => $bitName) {
    if ($prefix) $bit = u\consta($prefix, $bitName);
    $bitNames[$bit] = $bitName;
    $bitAliases[$bit] = @$aliases ? strtr($bitName, $aliases) : $bitName;
    if (u\getBit($bitRay, $bit)) $defaults[] = $bitName;
  }
  return boxesFld($label, '', $defaults, $bitNames, $bitAliases);
}

/**
 * Return a radio selection for account type.
 */
function acctTypeField() {
  $coAcctTypes = $GLOBALS['account types'];
  unset($coAcctTypes[R_PERSONAL]);
//  $onclick = "attrClick(this);";
//  $acctTypeAttrib = attrib(compact('onclick'));  
  return radiosFld(t('Account type:'), '', required(), $coAcctTypes);
}

/**
 * Return onSubmit script for a typeahead member account field (and include needed script files).
 * @param string $field: the field name
 * @param string $question: what to ask for confirmation or disambiguation (with params @amount and @name)
 * @param string $amount: optional amount substitution for question (include quotes or formula, if not a number)
 * @param bool $askGift: <ask if this form submission is a payment that is a gift> (defaults to no, don't ask)
 * @return complete script without leading "return ".
 */
function whoFldSubmit($field, $question, $amount = '0', $askGift = '0') {
  $id = '#edit-' . strtolower($field);
  $question = addslashes($question);

  js('js/typeahead.bundle.min.js', 'file');
  js("suggestWho('$id');", 'inline', 'footer');
  return "$('$id').val() == '' ? true : who(this, '$id', '$question', $amount, $askGift);"; // amount might be 'this.amount.value'
}

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Also sets up $formArray and $formSta, for testing.
 */
function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formArray, $formSta, $rUrl;
  
  setGlobals(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
  if (!@$formSta['confirm'] and !in_array($function, u\ray(R_NOLOG_FORMS))) {
    if ($logs = t\POST()) {
      foreach (u\just(R_NOLOG, $logs) as $k => $zot) $logs[$k] = '?';
      u\loga('in', $logs);
    } else {
      $formName = str_replace(base_path(), '', $_SERVER['REQUEST_URI']);
      u\loga('form', [$formName]);
    }
  }

  try {
    $mya = r\acct();
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
    $formName = substr($function, 0, 1) == '/' ? substr($function, 1) : "rCredits\\Web\\form$function";
    $specialForm = in_array($function, u\ray('Help Empty footer accounts Login'));
    $adminForm = in_array($function, u\ray('Admin MemberList MemberInfo'));
    $args = func_get_args(); // this and next few lines are from \drupal_get_form()
    array_shift($args); // Remove $function from the arguments.
    $formSta['build_info'] = compact('args');
    if (!r\up() and !$specialForm and !$adminForm) {
      if ($mya and $mya->admin) {
        say(t('The system is DOWN.'));
        //if (!$mya->cttyUp) say(t('The community is DOWN.'));
      } else $formName = 'rCredits\\Web\\formSystemDown';
    }
    
    $form = \drupal_build_form($formName, $formSta);
    \rweb_form_alter($form, $formSta, $formName);
    
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any
    if (!$specialForm) {
      js("jQuery('.messages').dblclick(function() {jQuery(this).hide();});", 'inline', 'header');
/*      if (@$form['country']) {
        js('js/countries.js', 'file', 'header');
//        if ($mya and \form_get_errors()) js("print_country($mya->country, $mya->state);", 'inline', 'footer');
      }*/
      $tree = explode('/', 'home' . str_replace(DEV_ROOT, '', $_SERVER['REQUEST_URI']));
      foreach ($tree as $k) if (preg_match('/^[a-z\-]+(\?|$)/', $k)) $crumbs[ucwords($k)] = $kk = isset($kk) ? "$kk/$k" : '';
      \drupal_set_breadcrumb($crumbs);
    }

//    if (!in_array('form-horizontal', @$form['#attributes']['class'])) $form['#attributes']['class'][] = 'form-vertical';
    $formArray = $form2 = $form; 

    $rent = \render($form2);

//    if (strpos($rent, 'form-type-checkbox') or strpos($rent, 'form-type-radio')) js('js/radiocheck.js', 'file', 'footer');
    
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
 * Output a photo upload success or error message and exit.
 * @param string $msg: if error, what to say. if success, url and type of file uploaded.
 * @param string $status: 'success' or 'error' (the default)
 */
function photoRet($msg, $status = 'error') {
  if ($status == 'success') $url = $msg; else $message = $msg;
  u\loga('photoRet', compact('status', 'message', 'url'));
/**/	echo json_encode(compact('status', 'message', 'url'), JSON_UNESCAPED_SLASHES);
  exit();
}

/**
 * Provide the crucial fields and scripts for resetting or changing a password.
 */
function pickPassword() {
  $mya = r\acct();
  $title = item(t('Choose a New Password'));
  $pass1 = passFld(REQ . t('New password:'), '', clas('password-field') + autofill('off'));
  $pass2 = passFld(REQ . t('Confirm new password:'), '', clas('password-confirm') + autofill('off'));
  $strong = hidFld(0, attrib(['id' => 'edit-strong']));
  focusOn('pass1');
  js(BASE_URL . '/modules/user/user.js', 'file', 'footer');
  
  $password = [
    'strengthTitle' => 'Strength:',
    'makeLonger' => 'Make it longer',
    'weak' => 'Weak (not strong enough)',
    'fair' => 'Fair (but not strong enough)',
    'good' => 'Good (but not strong enough)',
    'strong' => 'Strong (but not strong enough)',
    'verystrong' => 'Very strong (strong enough)',
    'mighty' => 'Mighty (strong enough)',
    'paranoid' => 'Paranoid? (plenty strong)',
    'hasWeaknesses' => 'To make your password stronger:',
    'tooShort' => 'Make it at least 6 characters',
    'addLowerCase' => 'Add lowercase letters',
    'addUpperCase' => 'Add uppercase letters',
    'addNumbers' => 'Add numbers',
    'addPunctuation' => 'Add punctuation',
    'sameAsUsername' => 'Make it different from your username',
    'confirmSuccess' => 'yes',
    'confirmFailure' => 'no',
    'confirmTitle' => 'Passwords match:',
    'username' => $mya ? strtolower($mya->mainQid) : '',
  ];

  $settings = json_encode(compact('password'));
  js(BASE_URL . '/misc/drupal.js', 'file', 'footer', -999); // for password strength meter
  js("jQuery.extend(Drupal.settings, {$settings});");

  return compact(u\ray('title pass1 pass2 strong'));
}
  
/**
 * Return a unique persistant identifier for the current machine (set randomly, if none yet).
 * @return the box ID string
 */
function box() {
  if ($box = @$_COOKIE['box']) return $box;
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
  return r\setCook($cookid, $id, r\rTime() + SESSION_LIFE);
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
  r\Acct::setDefault($mya); // set default for r\acct()
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
  $tribs['onchange'] = "this.form.elements['$pop_resultField'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $tribs['size'] = 12;
  $choice = selectFld('', '', w\attrib($tribs), $pop_choices);
  $choice = render($choice);
  $legend = t('Which one?');
  $popup = item(w\modal('which', $legend, $pop_question, $choice, t('Cancel')));
  js('$("#which").modal("show");', 'inline', 'footer', 9999);
  return compact('popup');
}

/**
 * Create a modal dialog
 * @param string $id: the dialog id
 * @param string $legend: header
 * @param string $prompt: what to say under the header
 * @param string $body: what to put in the body of the dialog
 * @param string $closer: what to show in the upper right corner, to close the dialog (default empty)
 * call with <button type="button" class="btn btn-info btn-lg" data-toggle="modal" data-target="#$id">Go</button>
 * @return HTML for the modal dialog
 */
function modal($id, $legend, $prompt, $body, $closer = '') {
  if ($closer == 'X') $closer = '&times;'; // a better X (normally with class "close" alone)
  return <<<EOF
<div id="$id" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="pull-right btn btn-xs btn-default" data-dismiss="modal">$closer</button>
        <h4 class="modal-title">$legend</h4>
        <p>$prompt</p>
      </div>
      <div class="modal-body">
        $body
      </div>
      <!--div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div-->
    </div>

  </div>
</div>
EOF;
}

/**
 * Return a submenu page (list) for a given menu or named menu section.
 * @param string $title: header for the menu
 * @param mixed $menu: menu section name or assoc of menu items
 * @return the menu as a Drupal form (not yet rendered)
 */
function subMenu($title, $menu) {
  $title = item($title, '', '', clas('submenu'));
  $list = item(subMenuHtml($menu), BARE);
  return compact('title', 'list');
}

function subMenuHtml($menu) {
  global $base_url;
  if (!is_array($menu)) $menu = \rweb_menu($menu);
  $list = '';
  foreach ($menu as $k => $v) {
    @list ($type, $head, $args, $perms, $func, $detail, $icon) = $v;
    if ($type == 'call' or $head == 'zot' or !w\webAccess($perms)) continue;
    if (@$detail) $detail = "<p class=\"list-group-item-text\">$detail</p>";
    $text = <<<EOF
    <h4 class="list-group-item-heading">
      <span class="glyphicon glyphicon-$icon"></span> &nbsp;
      $head
    </h4>
    $detail
EOF;
    $onclick = "jQuery(this).find('.glyphicon').css('color', 'darkblue');"; // glyph looks funny when ladda-spinning
    $list .= spinLink("$base_url/$k", $text, '', 'list-group-item', '', compact('onclick'));
  }
  return $list;
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

// NOT USED
function divButton($href, $title) {
  if (strpos($href, 'http') === FALSE) $href = BASE_URL . "/$href";
  return w\popHelp('', $title, $href);
  /*
  return  <<<EOF
<a href="$href" title="$title"><div></div></a>
EOF;
*/
}

// NOT USED
function imageButton($href, $title, $src) {
  global $rUrl;
  if (strpos($href, 'http') === FALSE) $href = BASE_URL . "/$href";
  if (strpos($src, 'http') === FALSE) $src = "$rUrl/images/icons/$src";
  return w\popHelp("<img src=\"$src\" />", $title, $href);
/*  return  <<<EOF
<a href="$href" title="$title"><img src="$src" border="0" /></a>
EOF; */
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
  return array($name => fld('item', '', '', button($label, "$gotoHead/$goto", $title)));
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

function boxOpt($field, $opts, $sta) {
///   debug(compact('field','opts','sta'));
  foreach ((@$sta['input'][$field] ?: []) as $key => $value) $result[] = $opts[$key]; // note that key==value
  return @$result ?: [];
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
  if ($main = findCompany($company, @$companyPhone)) { // company account exists
    return r\acct($main)->newRelation(u\ray('other employee isOwner', $other, @$employeeOk, @$isOwner)); // no permissions/draw (for security)
  } else return FALSE;
}

/**
 * Set up relationship of this new account with the account that created it (if any).
 */
function suCreatorRelation($helper, $args, $myid) {
  extract($params = u\just('isOwner employeeOk flow', $args));
  if (!@$isOwner and !@$employeeOk and !@$flow) return; // no useful relation (for example by true invitation)

  $draw = ($flow & 2) ? 1 : 0;
  r\acct($myid)->newRelation(u\ray('other permission employee isOwner draw', $helper, r\perm(B_MANAGE), $employeeOk, $isOwner, $draw));
  if ($flow & 1) r\acct($helper)->newRelation(u\ray('other draw', $myid, TRUE));
//  } else r\tellStaff("bad params in formSignup: $error -- ", $params);
} 

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $email0: the email address to check (also returned with improvements)
 * @param bool $dupsOk: <allow duplication of another account's email address>
 * @param acct $a: current account if any
 * @return: TRUE if email is okay to use, else FALSE
 */
function emailOkay(&$email0, $dupsOk, $a = FALSE) {
  $email = strtolower(str_replace("'", '&#039;', $email0));
  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
  if (!$dupsOk and emailDup($email, $a ? $a->id : 0)) return FALSE;
  $email0 = $email;
  return TRUE;
}

/**
 * Say whether the email is a duplicate and complain if it is.
 * NOTE: Companies with duplicate email addresses are okay, so don't call this for companies.
 * @param string $email: the email address to test
 * @param int $uid: the account it is for (0 if the account is not created yet)
 * @return <email is a dup>
 */
function emailDup($email0, $uid) {
  global $base_url;
  $mail = u\cryptN($email0);
  if (!$who = db\lookup('fullName', 'users', 'mail=:mail and uid<>:uid AND NOT :IS_CO', compact('mail', 'uid'))) return FALSE;
  $a = "a href=$base_url/settings/password/$email0";
  $emailTagged = str_replace('@', '+whatever@', $email0);
  $dupMsg = 'duplicate email';
  if (stripos($email0, '@gmail.com')) $dupMsg .= '|email plus';
  if (!$uid) $forgot = '|forgot password';
  say($dupMsg . @$forgot, compact(u\ray('emailTagged who a')), 'email');
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
  $onchange = w\onchange("var legal=jQuery('#edit-legalname'); if (legal.val()=='') legal.val(this.value);");
  $fullName = textFld("$what:", ["$who " . t('full name'), tt('usename desc')], required(@$fullName) + $onchange);
  $legalName = textFld(t('Legal name:'), ["$who " . t('full legal name'), t('(if different) ') . $who . tt('legalname desc')]);
  $email = textFld(t('Email:'), [t('Email'), t('All emails from the system will be sent to this address. It will not be made public. <b>Type carefully.</b>')], required());
  return compact(u\ray('fullName legalName email'));
}

/**
 * Return the part of the form that is (mostly) common to formContact and formSignup
 * @param assoc $defaults: initial field values (defaults to current values) - see list at "$fields ="...
 * @param acct $a: account for which to show existing contact field values (empty if new account)
 */
function contactFields($defaults = [], $a = '') {
//  $country = $state = ''; // make sure these are not NULL
  $fields = 'phone faxetc country postalCode address city state tenure owns postalAddr';
  extract($defaults ?: ($a ? (array) $a->account($fields) : []));
  $country = @$country + 0;
  $state = @$state + 0;
  //  $dupPhone = @$defaults['dupPhone'];

//  js('http://ziplookup.googlecode.com/git/public/ziplookup/zipLookup.min.js', 'file', 'header');
  js('js/countries.js', 'file', 'footer');
  js('js/zipLookup.js', 'file', 'footer');
  $js = <<<EOF
  print_country($country, $state);
  $('#rcreditswebformsignup').submit(function() {
    $('#edit-hidcountry').val($('#edit-country').val());
    $('#edit-hidstate').val($('#edit-state').val());
  });
EOF;
//  print_country("$country", "$state");
//  if ("$state" == '') zipChange('010 013');
//EOF;
  js($js, 'inline', 'footer');

  $countryAttrib = w\onchange("print_state(this.options[this.selectedIndex].value,'');");
  $onblur = 'setPostalAddr();';
  $stateAttrib = attrib(compact('onblur'));
  $addressAttrib = $cityAttrib = w\onchange($onblur);
  $zipAttrib = w\onchange("zipChange('010 013');");
  $hidCountry = hidFld($country); // remember this when Drupal forgets it on error
  $hidState = hidFld($state); // remember this when Drupal forgets it on error
  
  $phone = textFld(t('Phone:'), [t('Your phone number')], required(@$phone ? u\fmtPhone($phone): ''));
//$a ? '' : t('Your phone number. This should not be a number already used for another rCredits account &mdash; unless you have no other phone number.')
//  $dupOk = hidFld(0, attrib(u\ray('id', 'edit-dupOk')));
//  $dupOk = boolFld(t('Dup Ok:'), t('If this is already the phone number for another rCredits account, click Yes to use it for this account too. Without a unique phone number, though, you cannot connect a bank account to your rCredits account. We are lobbying our financial partner, Dwolla, to remove this limitation. But, for now, a duplicate phone means no connected bank account.'));

  $faxetc = @$faxetc ? textFld(t('Fax etc:'), [t('Other contact info'),t('List other ways to contact you: fax, mobile, IM, etc. This field is NOT public.')], dft(@$faxetc)) : NULL;
  $country = selectFld(t('Country:'), t(''), dft(@$country) + $countryAttrib); // can't use required() here
  $postalCode = textFld(t('Postal Code:'), [t('Physical location postal code'), t('Postal code for your <em>physical location</em>')], required(@$postalCode) + $zipAttrib);
  $address = textFld(t('Street Address:'), [t('Physical address'), t('Where is your building located (physical address)?')], required(@$address) + $addressAttrib);
  $city = textFld(t('City:'), [t('City where you live')], required(@$city) + $cityAttrib);
  $state = selectFld(t('State:'), t(''), dft(@$state) + $stateAttrib); // can't use required() here
  $tenure = $a ? NULL : textFld(t('How long:'), [t('MONTHS at this address.')], required(@$tenure));
  $owns = boolFld('', '', $a ? $a->owns : 0, [t('Rent'), t('Own')]);
  $postalAddr = textFld(t('Postal Addr:'), [t('Complete mailing address'), t('Where does the post office send your mail (usually)?')], required(@$postalAddr));

  return compact(u\ray($fields . ' hidCountry hidState'));
}  

/**
 * Return the fields used to verify an individual's SSN (also asked for in signup).
 * The dob field is not used for companies (but remains in the form, unseen), so it cannot be required.
 */
function ssnFields($personal = TRUE) {
  $ssTitle = tt($personal ? 'ssTitle' : 'einTitle');
  if ($personal) $dob = textFld(t('Birthdate:'), [t('Your date of birth (mm/dd/yyyy)')], required() + autofill('off'));
  $federalId = passFld($ssTitle, [tt($personal ? 'ssDesc' : 'einDesc')], required() + autofill('off')); // textfield so user doesn't have to retype on error
  return compact(u\ray('dob federalId'));
}

/**
 * Say whether the phone number is acceptable.
 * @param string $phone: phone number in any format
 * @param int $country: country code
 * @return TRUE if the phone number is acceptable
 */
function phoneOk(&$phone, $country = US_COUNTRY_ID) {
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

function onFile($field, $sta) {
  return (@$sta['complete form'][$field]['#value'] == R_ON_FILE and @$sta['input'][$field] == R_ON_FILE);
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  $zot = array('zot' => selectFld('', '', compact('value') + attrib(compact('name')), $options));
  return render($zot);
}

function amountField($name, $value, $id) {
  $name = "$name-$id";
  $class = 'amount'; $class = compact('class');
  $zot = array('zot' => textFld('', '', compact('value') + attrib(compact('name', 'class'))));
  return render($zot);
}

function toggle($field, $value, $id, $highlight = 'never') {
  $path = current_path();
  $name = $id = "$field-$id";
  $zot = array($id => hidFld($value, attrib(compact(u\ray('id name')))));
  $input = \render($zot);
  $b1 = button(' - ', "toggle('$id');", t('Change to NO'), 'danger');
  $b2 = button(' + ', "toggle('$id');", t('Change to YES'), 'success');

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
 *   $email = invitation($code, '', $inviter, $iCode, $err) returned with $inviter=inviter uid
 *   invitation($code, $invitee) marks the invitation USED by recording the invitee's account id
 * @param string $code: the invitation code
 * @param int $invitee: account id of the invited person
 * @param int $inviter: (RETURNED, if invitee='') account id of the inviter (returned even if error)
 * @param int $iCode: (RETURNED) the invitation sequence number
 * @param int $err: (RETURNED) error message, if any
 * @return:
 *   (1) encrypted email, if invited specifically with our form
 *   (2) TRUE if invited with an emailable link or marking invitation USED
 *   (3) FALSE if no such invitation or marking invitation USED failed
 */
function invitation($code, $invitee = '', &$inviter = '', &$iCode = -1, &$err = '') {
  if (!$invitee and $a = r\iCardAcct($code, $iCode)) {
    if ($iCode >= IBY_ICARD and db\exists('users', 'helper=:id AND iCode=:iCode', u\ray('id iCode', $a->id, $iCode))) return !$err = 'used invite';
    $inviter = $a->id;
    return TRUE;
  } else {
    $fields = 'id,email,inviter,invitee,invited';
    if (!$result = db\lookup($fields, 'r_invites', 'code=:code', compact('code'))) return !$err = 'bad invite';
    $id = $result['id']; // don't extract yet
    if (@$invitee) return db\update('r_invites', compact('id', 'invitee'), 'id'); // mark it used
    
    $iCode = IBY_FORM;
    extract($result); // looking for email and inviter, so get details
    if ($invitee) return !$err = 'used invite';
    $lastweek = strtotime((R_INVITE_DAYS + 2) . ' days ago'); // +2 for leniance
//    if ($invited < $lastweek) return !$err = 'expired invite';
    if ($invited < $lastweek) {
      $iCode = IBY_LATE; // convert to "unconfirmed" email invitation
      $inviterName = r\acct($inviter)->fullName;
      w\say('expired invite converted', compact('inviterName'));
    }
    return u\cryptN($email, FALSE); // $inviter is returned implicitly
  }
}

function passwordOkay($a, $password, $passFieldName = 'pass') {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  if (@$a->oneTimePass) {
    extract(u\just('pass expires', $a->oneTimePass));
    if ($password == @$pass) return ($a->admin or r\rTime() < @$expires) ? TRUE : say('pass expired', $passFieldName);
  }
  $hash = \_password_crypt('sha512', $password, $a->$passFieldName);
  return ($hash and $a->$passFieldName == $hash) ?: say('bad login', $passFieldName);
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
/// oops! don't let people reset passwords on test server. if (!isPRODUCTION and !in_array($name, u\ray('cornerstore bobbossman cathycashier helgas curtcustomer susanshopper'))) \drupal_set_message('Link: ' . "<a href='http://localhost" . DEV_ROOT . "/reset/id=$name&code=$pass'>reset</a>");
/**/ if (isDEV or (!isPRODUCTION and time() - db\lookup('created', 'users', 'name=:name', compact('name')) < HOUR_SECS)) \drupal_set_message('Link: ' . "<a href='http://localhost" . DEV_ROOT . "/reset/id=$name&code=$pass'>reset</a>");
/// if (isDEV) \drupal_set_message('Link: ' . "<a href='http://localhost" . DEV_ROOT . "/reset/id=$name&code=$pass'>reset</a>");
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
    $a = r\acct($uid);
    if ($a->coCan(CO_PRIVATE)) continue;

    $data = unserialize($data);
    $website = @$data['website'];
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $headed = TRUE; // always show the category (but keep the option to disable this choice)
      $list .= $headed ? ($list ? '</ul>' : '') . "<h4>$cat</h4>\n<ul>\n" : '<li>&nbsp;</li>';
    }
    $pay = $canBuy ? button('pay', "$base_url/pay/who=$short_name") : '';
    $chg = $canSell ? button('chg', "$base_url/charge/who=$short_name") : '';
    $fullName = "<a href='$base_url/member/$short_name'>$fullName</a>";
//    list ($selling) = explode("\n", $a->selling);
    $item = "$pay$chg <div class=\"company\">$fullName</div><div class=\"selling\">$a->shortDesc</div>";
    if (!$headed) $item = "- $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  return $list ? "<ul>$list</ul>" : t('There are no such rCredits member businesses.');
}

/**
 * Check if form was called with a QID (typically from an email link). Return the appropriate account.
 * @param assoc $sta: the form's status parameter, possibly with an eLink parameter (for verify and submit)
 *   (RETURNED) with $sta['eLink'] equal to just the qid and ecode part of eLink, plus the allow param
 * @param string $eLink: parameters from URL
 *   (RETURNED) same as $sta['eLink']
 * @param string $allow: what not-signed-in accounts to allow: individual (the default), 'acct', or 'any'
 * @return the account (defaults to current account)
 */
function eLinkAcct(&$sta, &$eLink = '', $allow = 'individual') {
  if (!$eLink) extract(u\just('eLink', $sta)); // verifying or submitting; get stored eLink args
  extract(u\just('qid ecode allow', $eLink));
  $a = r\acct($qid = @$qid); // current or specified account
  if (!$a and $allow != 'any') return r\go('', 'requires signin');
  if ($allow == 'individual' and $a->co) return r\go('', t('That page is for individual accounts only.'));
  if ($qid and @$ecode != $a->emailCode) return w\hack('bad ecode');
  $eLink = $sta['eLink'] = "allow=$allow&qid=$qid&ecode=" . @$ecode; // remember it (cttys don't use ecode)
  return $a;
}

function hack($message, $info = []) {
  u\loga('hack', $_SERVER, $message);
  r\tellStaff($message, $info);
  return r\signedIn() ? signout() : r\go('');
}

function signout($arg = '') {
  $msg = $arg == 'timedout' ? 'timed out' : '';
  @\session_destroy();
//  return r\go(isDEV ? 'http://rPromo/_sites?region=x' : 'http://rCredits.org/control.html?region=x');
  if (isDEV) r\go('signin', $msg);
  r\go('http://rCredits.org/control.html?region=x' . ($arg ? "&a=$arg" : ''));
}

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
function identifiers($a) {
  $mya = r\acct();
  list ($agentA, $company) = $a->proSe ? array($a, '') : array(r\acct($a->agentId), $a->fullName);
  $location = r\location($a);
  $pic = ($mya and $mya->co) ? $a->agentA->photoHtml() : ''; // let companies see the picture
  return <<<EOF
    $pic
    <div id="id-other">
      <div id="id-name">$agentA->fullName</div>
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
///  debug($states);
  if ($choices) {
    if ($states) foreach (u\ray($states) as $k) $stateDfts[] = $choices[$k];
    $states2 = boxesFld(t('Show:'), '', @$stateDfts, $choices);
  }
  $showAdvJs = 'jQuery(\'#advanced\').show(); jQuery(\'#simple\').hide();';
  if ($states != $settings or (@$ending and $ending != strtotime('tomorrow') - 1)) js($showAdvJs, 'inline', 'footer'); // showing advanced selection
  u\setDft($period, TX_DEFAULT_PERIOD);
  list ($starting, $ending) = rangeDates(@$period, @$starting, @$ending);
  $url = BASE_URL . "/$url/period=$period&starting=$starting&ending=$ending&states=$states";

  $showAdvanced2 = '<a id="showAdvanced" href="javascript:void(0);" onclick="' . $showAdvJs . '">advanced</a>';
  $submitPeriod = submi(t('Show')); // label has to be different from submitDates (see below)
  if ($downloadable) $downloadPeriod = submi(t('Download'));
///  debug(render($submitPeriod));
///  debug(rendA(compact('submitPeriod')));
///  $actions = render($submitPeriod) . render(@$downloadPeriod) . render($showAdvanced);
  $actions = fieldSet('actionsSimple', compact('submitPeriod', 'downloadPeriod'), '', suffix($showAdvanced2));
///  $period = selectFld(t('For the past:'), '', dft($period), $GLOBALS['TX_DAYS']);
  $period = selectFld(t('For the past:'), '', dft($period), $GLOBALS['TX_DAYS']);
  $simple = fieldSet('simple', compact('period', 'actions'), BARE);
  
  $res = compact(u\ray('url starting ending states')); // the easy parts of the result
  $submitDates = submi(t('show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
  $showSimple2 = '<a id="showSimple" href="javascript:void(0);" onclick="jQuery(\'#advanced\').hide(); jQuery(\'#simple\').show();">simpler</a>';
  if ($downloadable) $downloadDates = submi(t('download')); // must be lowercase
  $actions = fieldSet('actionsAdvanced', compact('submitDates', 'downloadDates'), '', suffix($showSimple2));
  $starting = textFld(t('Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textFld(t('Ending:'), '', dft(u\fmtDate($ending)));
  $advanced = fieldSet('advanced', compact('starting', 'ending', 'states2', 'actions'), BARE);
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

  if (u\starts(op($sta), 'download')) {
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
  if (!@$starting) $starting = strpos($ago, 'month') ?
    u\plusMonths(-preg_replace('/ ?months?/', '', $ago))
  : strtotime($period == '-1' ? '1jan' : ($period == '-2' ? '1/1/2013' : "$ago ago"));
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
  
  $title = item($title ?: t('Confirm'));
  $question = item($sta['confirm']);
  $confirm = submi(t('Okay'));
  $cancel = submi(t('Cancel'));
  $actions = w\fieldSet('actions', compact('confirm', 'cancel'));
  $form = compact('title', 'question', 'actions');
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => u\ray('confirmation'));
//  $form['#validate'] = 'rCredits\\Web\\sureForm_validate';
//  $form['#submit'] = 'rCredits\\Web\\sureForm_submit';
//  $sta['rebuild'] = TRUE;

  focusOn('confirm');

  return labeled($form);
}

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
  if (u\isAcct($result = be\identify($who, r\acct()->id, $selfMsg))) return $result;
  list ($msg, $args, $choices) = $result;
  foreach (u\ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = urlencode(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
  foreach ($choices as $uid => $fullName) $choices[$uid] = r\qid($uid) . ' ' . $fullName;
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
  extract($params = u\just('who amount goods purpose isGift', $info));
  if (!isset($goods) or $goods === '') return say('required field', ['field'=>'"For"'], 'ERR'); // '' is for test
  u\EXPECT($params, 'string string int|empty string bool');
///NO!  if (!in_array($op, ['pay', 'charge'])) $goods = FOR_USD; // make sure this is set, for blank_field check
  $txType = $op == 'pay' ? 'payment' : strtolower($op);
//  if (blank_field($params)) return FALSE;
  if (!$a = w\whois($who, 'who', $info)) return FALSE;
//  if ($goods == FOR_GOODS and trim($purpose) == '') return say('missing purpose', 'purpose');
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
//  $op = $op == 'pay' ? t('paid') : ($op == 'charge' ? 'charged' : $op . 'ed'); // for possible error message
//  if (u\forCash($purpose)) $goods = FOR_USD;

  if (@$isGift) { // tag gifts to nonprofits and handle special case of "giving back" rewards to the community
    $data = compact('isGift');
    if (r\isGiveback($myid, $a->id, $data)) list ($mya, $a, $amount, $goods) = [$a,$mya,-$amount,FOR_NONGOODS];
  } else $data = [];

  list ($msg, $args, $confirm) = 
     $txType == 'payment' ? be\transfer($txType, $mya, $a, $amount, $goods, $purpose, NULL, $confirmed, $data)
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
function doSay($msg, $ok = FALSE) {
  $type = $ok ? 'status' : 'error';
  @file_put_contents(t\dosayFilename($type), $msg); // in case called from a test (no way to know)
  return r\go('empty/1', $msg, !$ok);
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
  foreach (u\ray('Democracy Proposal') as $k) if (strpos($args, $k) !== FALSE) $dem = TRUE;
  $file = $smart ? '../rsmart/rsmart.inc' 
  : ($admin ? '../admin/admin-forms.inc' 
  : (@$dem ? '../rvote/rvote-web.inc'
  : 'rweb.inc'));
  
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
function helpText($page, $tag = '', $args = '') {
  include_once __DIR__ . '/rweb-help.inc';
  $page = strtolower($page);
  $help = $page == 'charge' ? strtr(rawHelp('pay', $tag, $args), u\ray('pay', 'charge')) : rawHelp($page, $tag, $args);
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
 * @param int $ctty: account ID of community to show graphs for (defaults to all communities)
 * @return: html of the relevant charts, in their proper divs
 */
function showCharts($whichChart, $ctty = 0) {
  global $base_url;

  list ($in, $out, $fees, $net, $personal, $companies) = array(t('in'), t('out'), t('fees'), t('net'), t('personal'), t('companies'));
  extract($stats = r\stats($ctty)); // careful with this (don't overwrite local vars)
  extract(w\fundStats($stats, TRUE)); // get the formatted versions, with originals in {var}0
  $trade = u\fmtAmt($trade0 = $tradeOut0 + $tradeIn0); // tradeIn is negative
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
  $totalR = u\fmtAmt($r0); // was u\fmtAmt($r0 + $usd0);
  $totalUsd = u\fmtAmt($usd0);

  $txTypes = u\ray('p2p p2b b2b b2p', t('Person-to-person'), t('Person-to-business (mostly by rCard)'), t('Business-to-business (paying suppliers)'), t('Business-to-person (including payroll)'));

  $avg = u\fmtAmt($txsPerMo ? $perMo / $txsPerMo : 0);
  $txs = "$txsPerMo @ $avg avg";

  $now = r\rTime();
  $created0 = db\lookup('MIN(created)', 'r_stats'); // show every n days since beginning + daily past n days
  $sql = <<<EOF
    SELECT created, IF(r=0, 0, perMo/r) AS rate, r AS funds, bankIn-bankOut AS usd, savings, runny, runnyBal,
      pAccts, bAccts, pAccts+bAccts AS accts, newbs, top5,
      bankInPerMo, bankOutPerMo,
      perMo, p2bPerMo, b2bPerMo, b2pPerMo,
      txsPerMo, p2bTxsPerMo, b2bTxsPerMo, b2pTxsPerMo, tradeIn, tradeOut, tradeInPerMo, tradeOutPerMo
    FROM r_stats WHERE ctty=:ctty
    AND (DATEDIFF(FROM_UNIXTIME(:now), FROM_UNIXTIME(created))<:STATS_GRAIN OR DATEDIFF(FROM_UNIXTIME(created), FROM_UNIXTIME(:created0))%:STATS_GRAIN=0)
    AND created>:R_LAUNCH ORDER BY created
EOF;
  $q = db\q($sql, compact('created0', 'now', 'ctty'));

  $charts = u\ray('accts funds velocity banking tx issued', 'growth', 'funds', 'velocity', 'demand', 'economic-circles', 'issuing-rcredits');
  if ($whichChart != 'all') $charts = u\just($whichChart, $charts);
  foreach ($charts as $k => $zot) u\setDft(${$k . 'Data'}, []);
  $moneyFields = u\ray('funds savings runny runnyBal top5 usd bankInPerMo bankOutPerMo perMo p2pPerMo p2bPerMo b2bPerMo b2pPerMo tradeInPerMo tradeOutPerMo');

  while ($row = $q->fetchAssoc()) {
    extract($row);
    list ($y, $m, $d) = u\ray(strftime('%Y %m %d', $created));
    list ($m, $d) = array($m - 1, $d - 1); // javascript Date object month, day, etc. are zero-based
    $date = "new Date($y,$m,$d)";
    $p2pPerMo = $perMo - $p2bPerMo - $b2bPerMo - $b2pPerMo;
    $p2pTxsPerMo = $txsPerMo - $p2bTxsPerMo - $b2bTxsPerMo - $b2pTxsPerMo;
    $funds -= $usd;
    $usd += $tradeOut + $tradeIn; // tradeIn is negative
    $bankOutPerMo = - $bankOutPerMo; // show as negative
//    $runnyBal = -$runnyBal; // for now, show runny DEBT instead of runny BAL
//    $actives = abs($pAccts - $newbs); // active members
    foreach ($moneyFields as $k) {
      if (!in_array($k, u\ray('bankOutPerMo tradeIn tradeInPerMo'))) $$k = max(0, $$k); // most can't be negative
      ${$k . '2'} = u\fmtAmt($$k);
    }
    $rate2 = number_format(100 * $rate, 1) . '%';

    $acctsData[] = "[$date,$pAccts,$newbs,$bAccts]";
    $fundsData[] = "[$date,{v:$funds,f:'$funds2'},{v:$usd,f:'$usd2'},{v:$runny,f:'$runny2'},{v:$top5,f:'$top52'},{v:$runnyBal,f:'$runnyBal2'},{v:$perMo,f:'$perMo2'}]";
    $velocityData[] = "[$date,{v:$rate,f:'$rate2'}]";
    $bankingData[] = "[$date,{v:$bankInPerMo,f:'$bankInPerMo2'},{v:$bankOutPerMo,f:'$bankOutPerMo2'},{v:$tradeOutPerMo,f:'$tradeOutPerMo2'},{v:$tradeInPerMo,f:'$tradeInPerMo2'}]";

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
  $accts = "{$pAccts} members + {$bAccts} companies";
  $netUsdIn = u\fmtAmt($bankInPerMo + $bankOutPerMo + $tradeOutPerMo + $tradeInPerMo) . t(' (net)');
  $velocity = number_format(($r0+0) ? 100 * $perMo / $r0 : 0, 1) . t('% per mo.');
  
  $js = <<<EOF
  var accts = '$accts';
  var r = '$totalR';
  var usd = '$totalUsd';
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
  js('js/charts.js', 'file', 'footer', 8);
  js($js, 'inline', 'footer', 9);

  foreach($charts as $k => $help) {
    $one = "<div id=\"{$k}Chart\" class=\"onechart\"></div>";
    if (count($charts) > 1) $one = "<a href=\"$base_url/help/$help/qid=$ctty\">$one</a>";
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
  $txRewards = $rebates + $bonuss;
  $usd = $bankIn - $bankOut;

//  foreach (u\ray('bankInPerMo bankOutPerMo r usd total signups inflations helpers txRewards grants loans fines') as $one) {
  if ($ctty) { // a community
    $cttyA = r\acct($ctty);
    $res['cttyName'] = $ctty == r\serverUid() ? t('Seedpack (Outliers)') : trim(strtr($cttyA->fullName, [t('Region')=>'', t('rCredits')=>'']));
  } else $res['cttyName'] = t('TOTAL');
  foreach (u\ray('r usd tradeIn tradeOut signups txRewards helpers inflations spent grants loans invests fines') as $one) {
    if ($includeRaw) $res[$one . '0'] = $$one;
    $res[$one] = u\fmtAmt($$one, $format);
  }
  return $res;
}

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
function hidFlds($ray) {return is_array($ray) ? hidFld(urlencode(serialize($ray))) : unserialize(urldecode($ray));}

/**
 * Send an invitation to the email and optionally report it.
 */
function sendInvite($email, $subject, $message, $dupsOk = TRUE) {
  $mya = r\acct();
  $admin = $mya->admin;
//  if ($admin = $mya->admin) $mya = r\acct(R_ADMIN_QID); // use William for admin invites
  
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
  if ($perms == 'anyone') return TRUE; // needed for submenus (see w\subMenu())
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
  global $rUrl;
  $options = ['group' => CSS_THEME, 'preprocess' => FALSE, 'external' => TRUE] + weight($weight);
  \drupal_add_css("$rUrl/$sheet", $options);
}

/**
 * Insert a javascript file or snippet into the form
 * @param string $what: the filepath or snippet
 * @param string $type: inline, file, or external
 * @param string $scope: header or footer
 * @param int $weight: low means load sooner (-18 and up, defaults to 0)
 */
function js($what, $type = 'inline', $scope = 'footer', $weight = 99) {
  global $rUrl;
  if ($type == 'file') $what = (strpos($what, 'http') !== FALSE or strpos($what, ':')) ? $what : "$rUrl/$what";
  return \drupal_add_js($what, compact('type', 'scope', 'weight'));
}

/**
 * Return the text with a help popover and optional link.
 */
function popHelp($text, $help, $href = '', $class = '') {
  list ($href, $tag) = $href ? [" href=\"$href\"", 'a'] : ['', 'span'];
  if ($class) $class = " class=\"$class\"";
  $text = ucfirst($text);
  return "<$tag$href$class data-toggle=\"popover\" data-trigger=\"hover\" title=\"$text\" data-content=\"$help\">$text</$tag>";
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

function disabled($value = NULL, $disabled = TRUE) {return attrib(compact('disabled')) + (is_null($value) ? compact('value') : []);}
function onchange($onchange) {return w\attrib(compact('onchange'));}
function dft($default_value) {return compact('default_value');}
function weight($weight) {return compact('weight');}
function clas($class) {return ['class' => [$class]];}
function attrib($attributes) {return compact('attributes');}
function suffix($field_suffix = '') {return compact('field_suffix');}
function prefix($prefix) {return compact('prefix');}
function autocomplete($type = '') {return array('autocomplete_path' => "autocomplete/$type/" . @r\acct()->id);}
function autofill($settings = 'on') {return attrib(['autocomplete' => $settings]);}
function required($dft = NULL) {return ['required' => TRUE] + (isset($dft) ? dft($dft) : []);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {global $testOp; return u\test() ? $testOp : str_replace('edit-', '', @$sta['input']['opid']);}
function verifyBy($byVoice) {return $byVoice ? t('Voice') : t('SMS');}
function blurbLink() {global $rUrl; return "<a href=\"$rUrl/templates/invite.html\" target=\"_blank\">description of rCredits</a>";}
function sayFieldErr($err, $field) {return say(strtoupper($field) . ': ' . tt($err), $field);}
function rent($element) {return \render($element);} // Drupal makes render hard, requires pass by reference