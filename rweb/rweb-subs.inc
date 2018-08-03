<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;

/**
 * @file
 * rWeb subroutine file
 * Tools for rWeb forms.
 */

define('R_NONE', '<span class="none">--</span>'); // how to show an empty field let the tests see null values
define('R_AGREE_0', -2); // number of the first point in the rCredits Agreement
define('R_AGREE_9', 7); // number of the last point in the rCredits Agreement
define('REQ', '*'); // marks required fields (use this as start of field's label)
define('BARE', '-'); // marks bare (unwrapped) fields -- no div wrapper around them

require_once __DIR__ . '/../cg-util.inc';
require_once __DIR__ . '/../cg-strings.inc';

global $txDays; $txDays = array(1 => 'Day', 7 => '7 days', 30 => '30 days', 90 => '90 days', 180 => '6 months', 365 => '12 months', -1 => 'Year to date', -2 => 'Ever');

$GLOBALS['tStrings'] += array(
  // reports
  'report new relation' => t('%who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Mobile number %number is now connected to this account.'),
  'report delete cell' => t('Mobile number %number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>So far so good. <b class="stopnot">But WAIT!</b> You have a few steps still to go. We sent you an email with your user ID and a link to the next step. If you don\'t see it within 60 seconds, check your spam folder.</p>'),
  'company is ready' => t(<<<EOF
    <p></p><p>After you finish setting up this account, to manage it in the future:</p>
    <ol>
      <li>Sign in to your personal account at %CG_DOMAIN</li>
      <li>Click your photo in the upper right corner. Choose your company account from the dropdown there.</li>
    </ol>
EOF
  ),
  'verify cell' => t('We sent a verification code to your mobile phone (%number).<br>Please type that code in the box below.'),
  'options saved' => t('Your options have been saved'),
  'step completed' => ' ', // (confusing) t('<span id="completed"> Step completed!</span>'), // space intended
  'after emailing' => t('After you have sent us an email: '),
//  'return to membership' => t('<a href="%BASE_URL/status">Click here to return to the checklist</a>.'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('A link has been emailed to you, to choose a new password.'),
  'changed agent' => t('You successfully changed the agent to %agentName.'),
  'changed account' => t('You successfully changed the account to %accountName.'),
  'signed agreement' => t('You signed the %PROJECT Agreement on %date.'),
  'downloading' => t('Your download has begun!'),
  'changed account' => t('You are now managing the account for %newAcct.'),
  'lost old changes' => t('Any unsaved changes to account %oldAcct have been discarded.'),
//  'take a step' => t('Your account is <b class="stopnot">not yet activated.</b> <a href="%BASE_URL/status" class="nextstep">Click here for the next step!</a>.'),
  'got token' => t('Your %RCREDITS and US Dollar accounts have been successfully connected!'),
  'got photo' => t('Your photo was successfully uploaded.'),
  'company rcard' => t('Send a <%a>company %PROJECT card</a> for %otherName to %manager at %address.'),
  'amount rounded' => t('"%amount" was rounded to the nearest integer value.'),
  'invite sent' => t('Your invitation has been sent.'),
  'repeat invite' => t('Come back any time, to send more invitations (click "Invite Someone" at the bottom of any page).'),
  'gift transfer later' => t('<br><br>Funds will be transferred when you have money in your account.'),
  'repeat gift' => t('If you wish to make a donation again sometime, click "Membership" in the upper right corner, then on "Donate".'),
//  'setup complete' => t('<p>Your account setup is complete. Expect an email within the next 24 hours, saying your account is activated. Your %PROJECT card will arrive by US Mail within a few days. Thank you for joining!</p>'),
  'setup complete' => t('<h2>Your Account Setup Is Complete</h2>You have done everything you need to do.'),
  'individual approval' => t('Once a staff member has approved your account, you can expect to receive your %PROJECT card in the mail soon.'),
  'company approval' => t('Once a staff member has approved your account, you can begin accepting %PROJECT payments.'),
  'join thanks' => t('Thank you for joining us!'),
  'use temporary card' => t('<p>While you wait for your %PROJECT card to arrive, you can use your invitation card to make purchases at any member business (beginning with your inviter).</p>'),
  'must be confirmed' => t('An email has been sent to %inviterName, who will need to confirm the invitation before you can use your %PROJECT Account. Or you can do your first rCard transaction with %inviterName as confirmation.'),
  //  Meanwhile, click the "<a href="%BASE_URL/community/invite">Invite</a>" tab to invite anyone you know and trust &mdash; you get a $%R_HELPER_BONUSr reward when they make their first purchase.<br><br>
  'ready to charge' => t('<p>You successfully scanned in as "%agentName" to charge customers on behalf of %companyName.</p><p>Now go back to your scanning app, to scan a customer\'s card. Sign out only when your shift ends.</p>'),
  'transaction list truncated' => t('There are too many transactions to show all at once, so only the first %TX_MAX_ROWS are shown (choose a different date range to see the rest).'),
  'address info' => t('<a1>Click here</a> for address information.'),
  'agent created' => t('The agent account was successfully created.'),
  'phone verified' => t('Your phone number has been verified.'),
  'txs upload done' => t('<br>Transaction upload was successful. You %did %count accounts a total of %total.'),
  'tx edited' => t('Your transaction #%tid was edited by %who.'),
  'new tx goods' => t('It is now for %what.'),
  'new tx amount' => t('The new amount is %amount.'),
  'expired invite converted' => t('Your invitation has expired, so %inviterName will be asked to re-confirm.'),
//  'new savings balance' => t('<p>IMPORTANT! Your %RCREDITS balance has been split to include a Credit Reserve containing your incentive rewards. Read <%a>last week\'s %PROJECT newsletter</a> for details.</p>', '_a', atag('http://us9.campaign-archive1.com/?u=b1d89f860859e1d9b906fa0b0&id=f1f54edd80&e=')),
//  'minimum zeroed' => t('<p>Your "automatic refills" have been turned off, to prevent any unintended transfer from your bank account. To turn them back on or transfer funds manually, sign in to your account at %CG_DOMAIN OR <%a1>click here</a>.</p>'),
  'sent message' => t('Your message has been sent to %fullName.'),
  'pass saved' => t('Your new password has been saved.'),
    
  // prompts
  'note' => '<b class="loud">' . t('NOTE:') . '</b>',
  'ach warning' => t('If there are insufficient funds in your bank account when you request an exchange of US Dollars for %RCREDITS (either explicitly, or by automatic refills), your bank will charge you a $20-35 fee, just like for a bounced check. If this happens more than once, the %PROJECT system will <i>also</i> charge you $%R_DEPOSIT_RETURN_FEE, to cover <i>its</i> bank fee. So make sure you have enough in your bank account to cover your exchanges.'),
  // If you don\'t want any automation, type zero (then remember to refill your account explicitly when it gets low).'),
//  'prompt max' => t('Keep AT MOST this much in my %PROJECT Account. (When your %RCREDITS balance goes above this amount by $%R_CHUNK or more, the system transfers the extra to your bank account. Blank means no automatic transfers &mdash; recommended in most cases).'),
  'legalname desc' => t(' full legal name, properly capitalized, for tax reporting and banking.'),
  'usename desc' => t('Your full "doing-business-as" name, properly capitalized! &mdash; the full name you use most of the time.'),
  'ssTitle' => t('Soc Sec #:'),
  'einTitle' => t('Federal ID:'),
  'ssDesc' => t('Your social security number (for tax reporting)'),
  'einDesc' => t('The company\'s Employer ID Number (EIN)'),
  'to scan another' => t('To scan another card, run the QR Scanner app again.'),
  'invite links' => t('<p>Here are your links, to include in your own email messages to your friends. Be sure to mention they can visit %CG_DOMAIN for more information.</p><p>%links</p>'),
  'timed out' => t('You were automatically signed out, to protect your account(s).'),
  'msg to admin' => t('%who sent you a message.'),
  'secure attach' => t('[<_%aAttach>Attachment</a>: %name type=%type size=%size bytes]'),
  'new customer welcome' => t('<h2>Congratulations!</h2><p>You have successfully completed your sign-up with %partnerName.</p><p><b class="loud">But wait!</b> You still need to sign up with %PROJECT, in order to be able to PAY %partnerName for their services.</p><p>%PROJECT is also worthwhile on its own merits. Fill out this form to get started.</p>'),
  
  // confirm
  'yes or no' =>  '%yesNo', // to replace with yesNo(url)
  'confirm accept' => '%who %did you %amount for %for on %date. Okay to %do', // used in EditTx test
  
  // errors
  'missing purpose' => t('<p>What purpose? For buying or selling actual goods and services, you must include a description. Rebates and bonuses are intended as rewards for productive economic activity in %RCREDITS.</p>
    <p>For everyone\'s protection, the %PROJECT software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field0' => t('%field field is required.'), // Drupal's standard
  'missing field' => t('Missing a required field: %field'),
  'nothing to do' => t('There is nothing to process.'),
  'in csv' => t('in CSV file line %line: %row'),
  'no txs' => t('There are no transactions in that period.'),
  'no invoices' => t('There are no such invoices in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no devices' => t('Surprisingly, this account has no related devices.'),
  'required field' => t('%field must not be blank.'),
  'duplicate email' => t('"%who" is already using that email address.'),
  'email plus' => t('If you want to use the same email address for this account, add a plus sign (+) and an arbitrary tag to the local part, for example "%emailTagged".'), // used in user.module
  'forgot password' => t('If you just forgot the password to the other account, <%a>click here</a> to request a new password.'), // used in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad phone' => t('That is not a proper phone number.'),
//  'bad account ID' => t('That is not your account ID.'),
  'not your account' => t('That is not your account ID.'),
  'bad account number' => t('Your bank account number must be 3-17 digits long.'),
//  'bad pin' => t('Your PIN must be exactly 4 digits'),
  'bad federal id' => t('That is not a correct %what. Try again.'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'short to' => t('You are %short short for that transaction.'),
  'increase min' => t('<%a>Click here</a> to increase your target balance!', '_a', atag('/settings/preferences')),
  'no such company' => t('There is no such company.'),
  'op canceled' => t('Operation Canceled.'),
  'demand not yet' => t('You will be notified when there are some %RCREDITS for you to buy with US Dollars.'),
  'confirm delete cell' => t('Are you sure you want to remove the selected mobile phones from this account?'),
  'already related' => t('That person is already related to this account. If you want to change the settings, find this person in the list and change the settings on that row.'),
  'already cell' => t('That mobile phone (%number) is already connected to this account.'),
  'cell taken' => t('A mobile phone can be connected to only one account. You are already using that phone (%number) in connection with account "%accountName". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'bad routing number' => t('US bank routing numbers are 9 digits. Use the number on the lower left edge of your checks.'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'unknown member' => t('"%who" has not yet signed up for %PROJECT.'),
  'unknown ctty member' => t('"%who" does not have an active %PROJECT account in your community.'),
//  'ambiguous other' => t('"%who" might mean any of these %PROJECT Accounts. Click the one you want or, if you don\'t see them listed, <a href="%draftLink">Click here to add them</a> to the list or <a id="which-cancel">here to cancel</a>.'),
//  'similar found' => t('If you mean %otherName (%phone), <a href="%draftLink">click here for YES</a>. Otherwise, <a href="%cancelLink">click here to cancel</a>.'),
  'get app' => t('To scan Account ID cards with a smartphone, you need the app! You can download the Android app from Google\'s Play Store.'),
  'no account permission' => t('You do not have permission to use that account.'),
  'bad transient id' => t('That QR code is out of date. Try again?'),
  'bad email' => t('That email address is not valid.'),
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner, employee, or contractor for the company? (please choose one or more)'),
  'shortr to' => t('You do not have enough %RCREDITS, for that transaction. <a href="%how">Go here</a> to get more %RCREDITS.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("%legalName").'),
  'bad invite' => t('That is not a valid Invitation Number or username.'),
  'used invite' => t('<p>That invitation has already been used, to open a %PROJECT Account.</p><p>If you already have a %PROJECT Account set up and want to open an additional account for yourself, your business, or a family member, sign in and use the "Open a Company Account" button on the Summary page (for cashier accounts, first click your photo in the upper right corner and select your business from the dropdown).</p><p>Otherwise ask someone you know to send you a new invitation!'),
//  'expired invite' => t('<p>That invitation has expired.</p><p><%a>Click here</a> to ask %inviterName for another invitation.</p>'),
  'min sub floor' => t('You cannot choose a Target Balance less than %floor.'),
  'info not saved' => t('We apologize. Your information could not be saved at this time. Please try again in an hour or so.'),
  'no page permission' => t('You do not have permission to visit this page. Return to your <%a>Summary</a> page.', '_a', atag('/summary')),
  'specific nonpositive' => t('The amount (%amount) must be a positive integer.'),
  'insufficient relation' => t('You cannot open an account unless you have a close relationship to it. <%a>Invite someone</a> instead.', '_a', atag('/community/invite')),
  'bad spoof check' => t('You must confirm (<i>every time</i>) that you are on the real %PROJECT site.'),
  'not an image' => t('That is not an acceptable image format. You might try converting it to a JPG or PNG image.'),
  'bad commercial aspect' => t('Your company profile picture must be in landscape orientation, with a width-to-height ratio of about %R_COMMERCIAL_ASPECT.'),
  'bad personal aspect' => t('Your profile picture must be in portrait orientation, with a width-to-height ratio of about %R_PERSONAL_ASPECT.'),
  'need a manager' => t('A company account must have at least one person to manage it.'),
  'self must sign' => t('You cannot sign this agreement on behalf of another person.'),
  'bad zip' => t('US zipcodes must be 5 digits, possibly followed by a hyphen and 4 more digits.'),
  'bad state' => t('You must say what state.'),
  'password required' => t('You must supply your current password, to change your email address or password.'),
  'bad account id' => t('That account does not exist. Check your typing?'),
  'bad login' => t('That account/password combination does not exist. Try again or <%a>ask for a new password</a>.', '_a', atag('/settings/password')),
  'wrong pass' => t('That is not your correct password.'),
  'multiple spaces' => t('%field cannot have more than one space in a row.'),
  'illegal char' => t('%field contains an illegal character. Try retyping it'),
  'too long' => t('%field is too long: it must be %USERNAME_MAX_LENGTH characters or less.'),
  'weak pass' => t('You need to choose a password that is harder to guess.'),
  'mismatch' => t('You must have mistyped your %thing once. Try again.'),
  'photo upload failed' => t('Your photo upload was not successful.'),
  'pass expired' => t('Your temporary password has expired.'),
  'inhuman proxy' => t('%name is a company. Your proxies must be people.'),
//  'dw mismatch' => t('WARNING: The "%field" of your Dwolla account and your %PROJECT Account are conflicting. Your account permissions will be restricted until you correct this mismatch.'),
  'proxy to go' => t('You have not yet completed your selection of proxies.'),
  'no stop draw' => t('The account for %fullName has overspent its %RCREDITS, based on your balance. So you cannot discontinue "draw" permission without first bringing that account back up to even (for example by giving it some funds.'),
  'doubled proxy' => t('Your alternate proxy cannot be the same person as your first proxy.'),
  'po in location' => t('Your physical location address cannot be a Post Office box.'),
  'missing currency' => t('You must choose %RCREDITS, USD, or both.'),
  'download always both' => t('Note: Downloads always include both %RCREDITS and USD amounts.'),
//  'sign in to scan' => t('You must <a href="/user/login">sign in</a> (or scan yourself in) BEFORE scanning a customer card.'),
  'change min first' => t('That request would leave you with less than your target balance. <%a>Lower your target balance</a> first.','_a', atag('/settings/connect')),
  'bad member id' => t('That is not a valid member ID!'),
  'bad card' => t('That is not a valid member ID.'),
  'bad card code' => t('This %PROJECT card is not valid.'),
  'upload required' => t('You must upload a file.'),
//  'missing contact info' => t("<h2>Missing Contact Information</h2><p>Please complete your <%a1>Contact Information</a> before continuing.</p>"),
  'correct and retry' => t('Please correct the problem and resubmit this form.'),
//  'duplicate phone' => t('That phone number is already the primary phone for another account.<br><br>If this is your ONLY phone and you can make do without a connected bank account (that is, if you have some other easy way to get money into and out of your %PROJECT Account): <input type="button" value="Continue With No Bank Connection" onclick="jQuery(\'#edit-dupOk\').val(1); jQuery(\'#frm-%form\').submit();" />'),
  'not a duplicate phone' => t('That phone number is NOT the primary phone for another account.'),
  'bad pin' => t('That is not your correct PIN.'),
  'wrong pin len' => t('Your PIN must be exactly 4 characters long.'),
  'pin required' => t('You must choose a PIN.'),
  'only us banks' => t('Unfortunately, at this time, we can handle only U.S. banks.'),
//  'bad deposit cents' => t('Each deposit is a number less than %DW_VERIFY_ROOF.'),
  'no can scan' => t('You do not have permission to scan an individual\'s %PROJECT card.'),
  'must opt' => t('You must not create a limited account for this person without giving them the opportunity to open a real %PROJECT Account. <%a>Invite them</a>!', '_a', atag('/community/invite')),
  'must agree' => t('Accept Terms checkbox field is required.'), // drupal shows this
  'redo info' => t('We could not verify your personal information. Please try again, carefully.'),
  'bad ssn or dob' => t('There is a problem verifying your personal information.'),
  'bad ssn' => t('That is not a valid social security number.'),
  'bank too little' => t('The amount you asked to transfer is less than the minimum ($%R_ACHMIN).'),
  'members only' => t('You must be a member, to use this feature.'),
  'dwolla terms required' => t("Please accept Dwolla's terms before you click Continue."),
  'no card ordered' => t('No %PROJECT card was ordered.'),
  'bad buy info' => t('To pay with %RCREDITS, you must use the same %thing for the purchase as you use for your %PROJECT Account.'),
  'must be signed out' => t('You are already signed in.'),
  'wrong payroll co' => t('The company name in cell A1 must match your company name exactly (%coName)'),
  'not payroll csv' => t('This is not a payroll CSV. It must say "Payroll Details" in cell A2.'),
  'bad csv' => t('Incorrect CSV format'),
  'empty payroll' => t('There are no employees listed as having received a %PROJECT payroll deduction.'),
  'bad payroll employee' => t('%employee is mispelled, does not have an active %PROJECT Account, or is not listed as an employee on your Relations page. Click Settings, then Relations.'),
  'illegal amount change' => t('You cannot %action the original amount (%amount). <%a>Send an invoice</a> instead.'),
  'bad achmin' => t('The minimum transfer amount must be at least $%R_ACHMIN.'),
  'say why not' => t('You must say why you are disputing the invoice.'),
  'no co pass' => t('Company accounts do not have passwords. Sign in to your personal account, then click your photo in the upper right corner and select %company from the dropdown.'),
  'account number mismatch' => t('You must have mistyped your account number once. Try again.'),
  'yes or no' => t('You must answer Yes or No.'),
  'no local proxy' => t('There are no other members nearby. <%a>Invite someone</a>!', '_a', atag('/community/invite')),
  'too many joins' => t('You can join to only one other account.'),
  'short cash help' => t('<%a>Click here</a> for more information about cashing out incentive rewards.', '_a', atag('/help/cashing-out')),
  'maybe not cash' => t('If you are actually reimbursing someone or paying them for goods or services (for example, employee labor), choose a different button under "For".'),
  'link expired' => t('That link has expired.'),
  'login failed' => t('Your attempt to sign in failed. Click the "Password problems" link if you need a new password.'),
  'bad file type' => t('Bad file type: %filetype'),
  'file save error' => t('Your file upload did not succeed.'),
  'forced without perm' => t('<p>An unauthorized transaction was made using a Company %PROJECT card.</p><ul><li>Date: %date</li><li>Customer: %a2 (by %agent2)</li><li>Company: %a1 (by %agent1)</li><li>Amount: %amount</li><li>For: %for</li></ul><p>This transaction was taken offline, so there was no way to know the agent lacked authorization.</p>'),
  'bad invite num' => t('That is not a valid username or invitation number.'),
  'negative saveWeekly' => t('Your discretionary savings are already at their minimum, so your weekly savings addition cannot be negative.'),
  'requires signin' => t('You must be signed in, to use that page.'),
  'too many days' => t('You cannot view more than 18 months at a time on screen (but you can download any period).'),
  'co code timeout' => t('That form has expired. Refresh your page and try again.'),
  
//  'saveWeekly too low' => t('With a negative "target balance", your weekly savings addition must be at least %min.'),
);
$GLOBALS['tStrings']['flooded'] = $GLOBALS['tStrings']['bad login'];

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out.
 * @param string $page: page to go to
 * @param string $msg: message to output, if any
 * @param string $field: error field, if any ('err' is typical, triggering error message rather than status message)
 */
function go($page, $msg = '', $field = '') {
  global $base_url;
  global $lastGo; $lastGo = $page; // for testing

  if ($msg) {
/*    if (u\starts($page, 'empty')) {
      list ($page, $msg) = strpos($page, '/') ? ["$page&msg=$msg", ''] : ["$page/msg=$msg", '']; 
    } else */
    r\Web\say(strip_tags($msg), $field);
  }
//  else $page .= "/field=$field&msg=" . urlencode($msg); // say() fails if signed out
  if (substr($page, 0, 1) == '/') $page = substr($page, 1); // supplying the slash is preferred, to agree with w\atag()
    
  if (u\test()) {
    $_POST = []; // make sure it doesn't act like we submitted the target page
    f('t.output', "Going to page $page (page0=$lastGo msg=$msg field=$field)");
    return strpos($page, 'http://') === FALSE ? f('t.pageForm', $page, $msg, $field) : '';
  }

  if (!u\starts($page, 'http')) $page = "$base_url/$page";

  /*
  foreach (ray('login signup password reset') as $one) if (strpos($page, "user/$one") !== FALSE) {
    if (\drupal_session_started()) session_destroy();
  }
  */
  \drupal_session_commit();

  header("Location: $page", TRUE, 302); 
  exit();
//  session_write_close();
//  exit();
//  \drupal_goto(substr($page, 0, 1) == '/' ? substr($page, 1) : $page);
}

/**
 * Redirect to a new page AND open a new tab with a different URL, with focus on the first new page.
 */
function go2($url1, $url2) {
  global $base_url;
	
  global $lastGo; $lastGo = $url1; // for testing

  foreach (ray('url1 url2') as $k) {
    if (substr($$k, 0, 1) == '/') $$k = substr($$k, 1); // supplying the slash is preferred, to agree with w\atag()
		if (!u\starts($$k, 'http')) $$k = "$base_url/" . $$k;
	}
	
	header_remove('Content-Security-Policy'); // allow the script
	echo <<< X
<html><script>
  window.open('$url2', '_blank');
  location.href = '$url1';
</script></html>
X;
  exit();
}

/**
 * Sometimes when redirecting offsite, we just hang. This works.
 * @param string $url: URL to go to
 */
function goFar($url, $params = '') {
  $mark = strpos($url, '?') ? '&' : '?';
  $url = urlencode("$url$mark$params");
  return w\go(BASE_URL . '/rcredits/relay.php?goto=' . $url);
}

function htmlItem($name, $id, $value, $class, $other = []) {return $value;}

function htmlText($name, $id, $value, $class, $other = []) {
  extract(just('tribs invisible id prefix suffix inline class box disabled', $other));

  $class = [@$class];
  $class[] = 'form-control input-md';
//  if ($extra = rcAuto(@$autocomplete_path, $id)) $class[] = 'form-autocomplete';

  $tribs = u\tribs(compact('type', 'class') + (@$tribs ?: []));
  return "<input $tribs />";
}
function htmlPassword($name, $id, $value, $class, $other = []) {
  return htmlText($name, $id, $value, $class, ['type' => 'password'] + $other);
}
function htmlHidden($name, $id, $value) {
  return htmlText($name, $id, $value, '', ['type' => 'hidden']);
}

function htmlSubmit($name, $id, $value, $class, $other = []) {
  extract(just('tribs size style', $other));
  return <<<EOF
  <button type="submit" id="$id" name="op" class="btn btn-$style btn-$size ladda-button" data-style="expand-right">
    <span class="ladda-label">$value</span>
  </button>
EOF;
}

function htmlButton($name, $id, $value, $class, $other = []) {
  extract(just('tribs href class', $other));
  
  u\EXPECT(!@$href or strpos($href, 'http') !== FALSE, 'no http in html button'); // $onclick = $href;
  if (@$href) list ($tag, $role) = ['a', 'button']; else $tag = 'button';
  return w\tag($tag, $value, (@$tribs ?: []) + compact(@$onclick ? 'onclick' : 'href', 'class', 'role'));
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
///  debug(compact(ray('type label desc other')));
  if (@$label[0] == REQ) list ($required, $label) = ['yes', substr($label, 1)];
  if ($bare = (@$label[0] == BARE)) $label = substr($label, 1); // bare? (no div wrapper)

  @list ($placeholder, $help, $title) = is_array($desc) ? $desc : ['', $desc];
  if (!@$title) $title = $placeholder; // hover text defaults to same as hint
  foreach (['placeholder', 'title'] as $k) if ($$k) $tribs[$k] = $$k;

  return compact(ray('type label required bare help tribs other')); // see rend()
}

// only barely begun
function rendForm($form) {
  $guts = rend($form);
  return str_replace('frm-', 'r-', w\tagN('form', $guts, $form['#attributes']));
} 

function NEWcgform($form, $onSubmit = '', $labeled = TRUE) {
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
//    $form[$k]['#tabled'] = TRUE;
//    if (@$form[$k]['#type'] == 'fieldset') $form[$k] = cgform($form[$k], $onSubmit, $level + 1);
    $all[$k] = $v;
    unset($form[$k]);
  }
  $form['all'] = oldItem(rend($all));
  return $form;
}

function cgform($form, $onSubmit = '', $labeled = TRUE, $level = 0) {
  if ($onSubmit) {
    list ($zot1, $zot2, $caller) = explode('\\', u\caller());
    //$formId = 'rcreditsweb' . strtolower($caller);
    //w\js('on-submit', "jQuery('#$formId').submit(function() { $onSubmit });"); 
    w\jsx('on-submit', 'caller', strtolower($caller)); // spaces are required around $onSubmit
    // (Drupal kills the quotes) $form['#attributes']['onsubmit'] = $onSubmit; // eg ' return func();'; 
  }
  $form['#attributes']['class'][] = $labeled ? 'form-horizontal' : 'form-vertical';
 
  foreach (ray('title subtext buttons list back submit') as $k) if ($v = @$form[$k] and !is_array($v)) $form[$k] = $k == 'submit' ? submit($v) : item($v); // let these common items be created simply
  
  foreach ($form as $k => $v) if (substr($k, 0, 1) != '#') {
    $type = @$v['#type'];
    $params = [$k => @$v['#value']];
    if ($type == 'hidden') f('t.POST', $params); // fake post for tests
    if ($type == 'fieldset') $form[$k] = cgform($form[$k], '', $labeled, $level + 1);
    if ($labeled and is_array($form[$k])) $form[$k]['#tabled'] = TRUE; // remember that form is laid out in a table (labels in left column)
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
  return preg_replace('/<(input|textarea|select|button) /', "<$1 id=\"edit-$name\" name=\"$name\" ", rent($field));
}
    
function rend($fields) {
  $html = '';
  foreach ($fields as $name => $ray) {
    extract(just('type bare other', $ray)); // see fld()
    extract(just('id value class', $other));
    u\setDft($id, "edit-$name");
    $func = 'CG\\Web\\html' . ucfirst($type);
    $fldHtml = $func($name, $id, @$value, @$class, @$other);
    $html .= $bare ? $fldHtml : wrap($name, $fldHtml, $ray);
  }
  return $html;
}

function wrap($name, $html, $ray) {
  extract(just('type label required bare help other', $ray)); // see fld()
  extract(just('invisible id prefix suffix inline class box disabled', $other));
  $for = strtr(@$name, ['_'=>'-', '['=>'-', ']'=>'']);
  if ($for == 'title') $html = "<h3>$html</h3>";
  $html = @$field_prefix . $html . @$field_suffix;

  $boxy = in($type, 'radio checkbox') ? $type : '';

  $class = [@$class];
  $class[] = $boxy ? 'option' : 'control-label col-sm-offset-1 col-xs-2';
  if (@$invisible) $class[] = 'sr-only';
  $label = w\tag('label', $boxy ? $html . @$label : @$label, compact('for', 'class'));
    
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
  
/**
 * Say whether the member has permission to run the named query (all queries are for members only).
 * @param string $qName: name of the query -- if first character is a digit, restrict to admins/companies
 *   0=super 1=admin 2=cAdmin2 B_CO(6)=company
 * @param string $sql: the query, optionally followed by subtext, after a tilda (~)
 *                     (RETURNED) the query modified to apply to the current community, if appropriate
 * @param string $subtext: (RETURNED) the explanatory text, if any
 * @return FALSE if not permitted, else the query name without any leading digit
 */
function queryPermitted($qName, &$sql = '', &$subtext = '') {
  global $mya; if (!$mya) return FALSE; // queries are for signed-in members only

  $cttyName = $mya->community ? $mya->cttyA->fullName : t('All Communities');
  list ($sql, $text2) = explode('~', "$sql~");
  $map[':myid'] = $mya->id;
  $map[':ctty'] = $mya->community ?: 'community'; // alternative is for superAdmin
  $sql = strtr($sql0 = $sql, $map);
  $subtext = strpos($sql0, ':ctty') ? "<h4>$cttyName</h4><br>$text2" : $text2;
  
  if (!preg_match('/^([0-9])(.*)/', $qName, $m)) {
    return $qName; // no restrictions
  } else {
    list ($zot, $d, $qName) = $m;
    if (strpos($sql0, ':myid')) $subtext = "<div><h4>for $mya->fullName</h4>$text2</div>";
    return $d >= ($mya->superAdmin ? 0 : ($mya->admin ? 1 : ($mya->cAdmin2 ? 2 : ($mya->co ? B_CO : 9))))
    ? $qName
    : FALSE;
  }
}
  
/**
 * Return a table of formatted records, given an array of rows.
 */
define('ALIGN_RIGHT', 'proxiedFor trust');
define('ALIGN_CENTER', '');
define('FMT_INT', 'trust count');
//define('FMT_2D', 'negativeBalanceTotal positiveBalanceTotal');
 
function showRecords($ray, $formatFunc = 'CG\\Web\\formatField', $flagsField = B_LIST) {
  if (!$ray or empty($ray)) return 'data set is empty';
  $lines[] = '<tr><th>' . join('</th><th>', array_keys((array) $ray[0])) . "</th></tr>\n";
  foreach ($ray as $row) {
    $line = '';
    foreach ($row as $k => $v) {
      $v = $formatFunc($k, plain($v), $flagsField);
      if ($k == 'for2' and $v == $row->for1) $v = 'same'; // admin uses this for conciseness
      $align = in($k, ALIGN_RIGHT) ? 'R' : (in($k, ALIGN_CENTER) ? 'C' : 'L');
      foreach (ray('balance credit trust count total $') as $w) if (stripos($k, $w) !== FALSE) {
        $align = 'R';
        if ($v !== '' and is_numeric($v)) $v = number_format($v, in($w, FMT_INT) ? 0 : 2);
      }
//      if (in_array($k, ray(FMT_INT))) $v = round($v);
//      if (in_array($k, ray(FMT_2D)) or strpos($k, '$') !== FALSE) if ($v != '') $v = number_format($v, 2);
      $line .= "  <td class=\"align$align\">$v</td>\n";
    }
    $lines[] = $line;
  }
  return "<table>\n<tr>" . join("</tr>\n<tr>\n", $lines) . "</tr>\n</table>";
}

/**
 * Return the given field, formatted for viewing.
 * Unlike a\formatField, the third argument ($flagsField) is ignored.
 */
function formatField($k, $v) {
  $v = $v . ''; // convert to string
//  if ($v and $k == 'channel') $v = u\rayv(ray(TX_CHANNELS), $v);
//  if ($v and in_array($k, ray(R_DATE_FIELDS))) $v = '<div style="line-height:65%;">' . strftime('%d%b', $v) . '<span style="font-size:50%;">\'' . strftime('%y', $v) . '<br>' . strftime(isDEV ? '%I:%M %p' : '%l:%M %P', $v) . '</span></div>';

  if (u\crypted('P', $v)) $v = u\decry('P', $v); // decrypt phone and email
  if ($k == 'phone') $v = u\fmtPhone($v);
  if ($k == 'account') $v = $v ? r\qid($v) : '';
  return $v;
}

// old
function fld($type, $title = '', $help = '', $other = [], $options = NULL) {
  $label = $title;
  if (@$label[0] == REQ) list ($required, $label) = ['yes', substr($label, 1)];
  if ($bare = (@$label[0] == BARE)) $label = substr($label, 1); // bare? (no div wrapper)
  if (@$other['wrap']) $bare = FALSE;
  $title = @$other['title'] ?: $label;

  if (!is_array($other)) $other = [$type == 'item' ? 'markup' : 'value' => $other];
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = $options ? key($options) : ''; // Drupal's select fails without a default
  $title_display = 'before';
//  if (isset($other['value']) and !isset($other['default_value'])) $other['default_value'] = $other['value'];
  $field = u\prefixKeys('#', $other + compact(ray('type title title_display required bare help options')));
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

/**
 * Return a button field array.
 * @param string $value: what the button says
 * @param string $href: where the button goes (or does)
 * @param string $title: hover text
 * @param string $style: Bootstrap style
 * @param string $size: Bootstrap size
 * @param assoc $extra: other attributes (id, class, etc)
 */
function buttonFld($value, $href = '', $title = NULL, $style = 'success', $size = 'xs', $extra = []) {
  $class = trim(@$extra['class'] . " btn btn-$style btn-$size");
  return NEWfld('button', BARE, ['', '', $title], compact(ray('href value title class id')) + $extra);
}
function button($value, $href = '', $title = NULL, $style = 'success', $size = 'xs', $extra = []) {
  return rend([buttonFld($value, $href, $title, $style, $size, $extra)]);
}
function btnSubmit($value, $submit = NULL) {
  $class = 'submit' . (@$submit ?: t('Submit'));
  return button($value, '', NULL, 'success', 'xs', compact('class'));
}
function btnShowDiv($divId, $title, $style='success', $size = 'xs') {
  return "<a href=\"#$divId\" class=\"btn btn-$size btn-$style\" data-toggle=\"collapse\">$title</a>";
}

function NEWitem($value, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('value') + $other);}
function item($markup, $title = '', $desc = '', $other = []) {return fld('item', $title, $desc, compact('markup') + $other);}
function hidFld($value = '', $other = []) {return fld('hidden', '', '', compact('value') + $other);}
function NEWtextFld($label = '', $desc = '', $other = []) {return fld('text', $label, $desc, $other);}
function textFld($label = '', $desc = '', $other = []) {return fld('textfield', $label, $desc, $other);}
function passFld($label = '', $desc = '', $other = []) {return fld('password', $label, $desc, $other);}
function areaFld($label = '', $desc = '', $other = []) {return fld('textarea', $label, $desc, $other);}
function radiosFld($label, $desc, $other, $options) {return fld('radios', $label, $desc, $other, $options);}
function selectFld($label, $desc, $other, $options = []) {return fld('select', $label, $desc, $other, $options);}
function fileFld($label = '', $desc = '', $extra = []) {return fld('file', $label, $desc, $extra + attrib(ray('enctype', 'multipart/form-data')));}
//function htmlFld($label = '', $desc = '', $dft = '', $other = []) {return fld('text_format', $label, $desc, ['format' => 'filtered_html'] + dft(strtr($dft, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])) + $other);}
//function htmlFld($label = '', $desc = '', $other = []) {return areaFld($label, $desc, $other);}
function dataFld($id, $data) {return item(w\tags('div', "<!--$data-->", compact('id')));}
function boxFld($name, $label = '', $desc = '', $dft = NULL, $other = []) {
  $fld = fld('checkbox', $desc, '', compact('name') + dft(@$dft) + $other);
  return $label ? item(@\render($fld), $label, '', ['box' => TRUE]) : $fld;
}
function togFld($name, $label = '', $desc = '', $dft = NULL, $other = []) {
  js('x/bootstrap-toggle.min');
  css('x/bootstrap-toggle.min.css');
  extract(just($ks = 'on off onstyle offstyle size', $other));
  foreach (ray($ks, t('Yes'), t('No'), 'success', 'default', 'mini') as $k => $v) u\setDft($$k, $v);
  
  return boxFld($name, $label, $desc, $dft, $other + w\attrib(ray("data-toggle:togglel,data-on:$on,data-off:$off,data-onstyle:$onstyle,data-offstyle:$offstyle,data-size:$size")));
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
 *//*
function shareFld($a) {
  $sharePct = round(db\get('AVG(share)', 'r_gifts', 'share>=0'), 1);
  $shareDft = $a->prefsSet ? (($a->share + 0) ?: 0) : R_SHARE_DFT;
  return textFld(t('Share:'), t('<p>What percentage of your ongoing purchase rewards would you like to donate to Common Good Finance (CGF), to support the %PROJECT system? This is another way to take responsibility for our community\'s future. You can put anything from 0% to 50%. For example, if you choose 30%, then when you make a $10 purchase and get your $1 reward, your "sharing" contribution is 30 cents (30% of $1). The current average sharing percentage chosen by members is sharePct%.</p><p>Note that this is not "giving back" part of your rewards to the %PROJECT community &mdash; rather it is an optional extra donation to CGF. CGF is a nonprofit member company, not the system itself. CGF promotes %PROJECT and handles administration of your %PROJECT community until the community can take over that responsibility.</p>', compact('sharePct')), required($shareDft . '%'));
}
*/

/**
 * Return a form field for donating fractional dollars.
 */
function roundupFld($a) {
  return w\boolFld(t('Round Up?'), t('Fund the common good! Round all your payments up to a whole dollar amount and donate the cents to local nonprofits, community investments, and the common good.'), $a ? $a->roundup : FALSE);
}

/**
 * Return an error message if the chosen sharing amount is invalid.
 * @param string $share: (MODIFIED) the member's chosen sharing percentage (returned normalized)
 * @param bool $required: whether to insist on input
 * @return FALSE if no error, otherwise the error message or index.
 */
function badShare(&$share, $required = FALSE) {
  $share = trim(str_replace('%', '', $share));
  if ($required and $share === '') return t('missing field', 'field', 'share');
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
function boolFld($title = '', $help = '', $dft = NULL, $labels = NULL, $extra = []) {
  $class = ['yesno'];
  $inline = TRUE;
  $dft = is_null($dft) ? [] : (is_array($dft) ? $dft : dft((@$dft + 0) ?: 0));
  return radiosFld($title, $help, $extra + $dft + compact('class', 'inline'), @$labels ?: array(t('No'), t('Yes')));
}

/*
 * Create a rendered checkboxes field
 * @param string $title:
 * @param string $desc:
 * @param array $defaults: simple array of indexes of already-checked boxes
 * @param assoc $choices: name for each checkbox (indexed)
 * @param assoc $aliases: name to display for each checkbox (defaults to choice name)
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
  $bits = ray($bits);
  if (!is_numeric(key($bits))) $bits = array_keys($bits); // ignore values (eg weights for risk arrays)
  $defaults = [];
  foreach ($bits as $bit => $bitName) {
    if ($prefix) $bit = u\consta($prefix, $bitName);
    $bitNames[$bit] = $bitName;
    $bitAliases[$bit] = @$aliases ? strtr($bitName, $aliases) : $bitName;
    if (u\getBit($bitRay, $bit)) $defaults[] = $bit;
  }
  return boxesFld($label, '', $defaults, $bitNames, $bitAliases);
}

/**
 * Return a "signed by" field.
 */
function signedBy() {
  global $mya;
  $behalf = $mya->co ? t('(on behalf of %co) ', 'co', $mya->fullName) : '';
  $signerA = $mya->cAdmin ? ($mya->co ? $mya->helperA : $mya) : $mya->agentA;
  return textFld(t('Signed:'), $behalf . t('Type your full name here ') . "($signerA->legalName)", required($mya->cAdmin ? $signerA->legalNameDpy : ''));
}

/**
 * Complain if the signature parameter is bad.
 */
function badSignature(&$sta) {
  global $mya;
  $legalName = r\agent()->legalNameDpy;
  if (!$mya->cAdmin and strcasecmp(trim($sta['input']['signedBy']), $legalName) != 0) return say('bad signature', compact('legalName'), 'signedBy');
}  
  
/**
 * Return onSubmit script for a typeahead member account field (and include needed script files).
 * @param string $field: the field name
 * @param string $question: what to ask, for confirmation or disambiguation (with params %amount and %name)
 * @param assoc $extra: less common parameters:
 *   amount: UNUSED amount substitution for question (defaults to form.amount.value)
 *   allowNonmember: <allow a non-member email address>
 *   coOnly: <restrict results to companies>
 */
function whoFldSubmit($field, $question, $extra = []) {
  extract(just('amount allowNonmember coOnly', $extra));
  js('x/typeahead.bundle.min');
  jsx('suggest-who', compact(ray('field question amount allowNonmember coOnly')));
}

/**
 * Retrieve the specified form, called with the args, and return it rendered.
 * Also sets up $formArray and $formSta, for testing.
 */
function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formArray, $formSta, $rUrl;
  setAcct(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
  if (!@$formSta['confirm'] and !in($function, R_NOLOG_FORMS)) {
    $formName = str_replace(base_path(), '', $_SERVER['REQUEST_URI']);
    if ($logs = $_POST) {
/**/  u\loga('in', $formName . substr(print_r(u\noSecrets($logs), 1), 5)); // without "Array"
    } else {
      if (strpos($formName, 'sadmin/') === FALSE and strpos($formName, 'account-') === FALSE) u\loga('form', [$formName]);
    }
  }

  try {
    global $mya;
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
    $formName = substr($function, 0, 1) == '/' ? substr($function, 1) : "CG\\Web\\form$function";
    $specialForm = in($function, 'Help Empty footer accounts Login');
    $adminForm = in($function, 'Admin MemberList MemberInfo');
    $args = func_get_args(); // this and next few lines are from \drupal_get_form()
    array_shift($args); // Remove $function from the arguments.
    $formSta['build_info'] = compact('args');
    $formSta['cache'] = TRUE;
//    $formSta['method'] = 'post';
    if (!r\up() and !$specialForm and !$adminForm) {
      if ($mya and $mya->superAdmin) {
        say(t('The system is DOWN.'));
        //if (!$mya->cttyUp) say(t('The community is DOWN.'));
      } else $formName = 'CG\\Web\\formSystemDown';
    }
    
//    if (strpos($formName, 'Event') !== FALSE) require_once DRUPAL_ROOT . R_PATH . '/rvote/rvote-web.inc';                       
    
    $form = \drupal_build_form($formName, $formSta);

    $req = basename($_SERVER['REQUEST_URI']);
    if (!$mya and in($req, 'login logout')) return w\go(PROMO_URL); // nothing else works
    if ($req == 'user' and $mya) return w\go('summary'); // summary not ''
    foreach ($form as $k => $v) if (@$v['#type'] == 'submit') {$submit1 = @$v['#id'] ?: "edit-$k"; break;}
    $form['opid'] = hidFld(@$submit1 ?: 'none', ['name' => 'opid']); // default to first, in case submitted programmatically
  
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any
    if (!$specialForm) {
      $tree = explode('/', 'home/' . str_replace(DEV_ROOT, '', $_SERVER['REQUEST_URI']));
      foreach ($tree as $k) if (preg_match('/^[a-z\-]+(\?|$)/', $k)) $crumbs[ucwords($k)] = $kk = isset($kk) ? "$kk/$k" : '';
      \drupal_set_breadcrumb($crumbs);
    }

    $formArray = $form2 = $form; 
    return \render($form2);
    
  } catch(Exception $e) {r\Web\exception_handler($e);}
}

define('FORBID_TAGS', '/.{0,40}<(script|style|frame|object|embed|applet|audio|video).{0,40}/i'); // forbidden html tags
define('FORBID_ATTRS', '/(.{0,40}\\s(href|src|style|on[a-z]+)\\s*=.{0,40})/i'); // forbidden attributes

/**
 * Sanitize the page before displaying it. 
 * Actually, since the page should NEVER have illegal stuff, we don't display a failed page, just detect it.
 * @param string $page: the body of the page, not including our selected scripts
 */
function sanitizePage($page) {
  $except = join('|', [
    '<img src="' . BASE_URL, 
    '<iframe src="(' . str_replace(' ', '|', FRAME_SOURCES) . ')',
    'href="(mailto:|http|#|/|"|sadmin)',
    '(href|src|style)=&quot',
  ]);
  
  $page = preg_replace("~$except~", '', $page);
//  return;
  if ($tags = preg_match(FORBID_TAGS, $page, $matches) or preg_match(FORBID_ATTRS, $page, $matches)) {
/**/ $msg = t('Page has a forbidden HTML %thing: %what', 'thing what', $tags ? t('tag') : t('attribute'), print_r($matches, 1));
/**/ if (isDEV) die($msg);
     if (isPRODUCTION) w\hack($msg);
/**/ die($msg);
  }
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
 * Provide the crucial fields and scripts for choosing or changing a password.
 * @param bool $reset: <changing, rather than choosing>
 */
function pickPassword($reset = TRUE) {
  global $mya;
  $pass1 = passFld(REQ . ($reset ? t('New password:') : t('Choose a Password:')), '', clas('password-field') + autofill('off'));
  $pass2 = passFld(REQ . ($reset ? t('Confirm new password:') : t('Confirm password:')), '', clas('password-confirm') + autofill('off'));
  $strong = hidFld(0, attrib(['id' => 'edit-strong']));
  focusOn('pass1');
  
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
  js('drupal'); // for password strength meter
  js('user');
  $settings = item("<!--//--><![CDATA[//><!--\njQuery.extend(Drupal.settings, {$settings});\n//--><!]]>");

  return compact(ray('pass1 pass2 strong settings'));
}
  
/**
 * Return a unique persistant identifier for the current machine (set randomly, if none yet).
 * @return the box ID string
 */
function box() {
  global $box;
  if ($box = @$_COOKIE['box']) return $box;
  return r\setCook('box', $box = u\code());
}

/**
 * Return the box record id for the given machine code and account record ID.
 * @param int $code: machine identifier
 */
function boxUser($code, $uid) {
  if ($id = @$_COOKIE[$cookid = 'boxnum' . $uid] and !u\test()) return $id; // we only get this if last set today
  if ($id = db\get('id', 'r_boxes', 'code=:code AND uid=:uid', compact(ray('code uid')))) {
    db\update('r_boxes', ray('id access', $id, r\rTime()), 'id');
  } else $id = r\makeDevice($uid, $code, TX_WEB);
  return r\setCook($cookid, $id, r\rTime() + SESSION_LIFE);
}

/**
 * Return rows from an uploaded file.
 */
function uploadrows($filename) {
  file_put_contents($filename, $str = strtr($get = file_get_contents($filename), ["\r\n" => "\n", "\r" => "\n"]));
  $fp = fopen($filename, "r");  
  while ($row = fgetcsv($fp)) $rows[] = $row; // get all rows, so we can look ahead
  fclose($fp);
  unlink($filename);
  return $rows;
}

/**
 * See whether someone is trying too many logins.
 * @param int $uid: account record ID
 * @param assoc $sta: (MODIFIED) the form status, updated with relevant flood info
 * @param
 * @return mixed: FALSE only if we're not being flooded
 */
define('MAX_PW_FAILS', 10);
define('MAX_IP_FAILS', 50);

function flooding($uid, &$sta, &$err) {
  if (u\test()) return !$uid;
  $trigger = 'flood_control_triggered';
  $window = HOUR_SECS; //variable_get('user_failed_login_ip_window', HOUR_SECS);
  if (!\flood_is_allowed('failed_login_attempt_ip', MAX_IP_FAILS, $window)) {
    $sta[$trigger] = 'ip';
    $err = t('There have been too many failed sign-in attempts from your IP Address. Try again later.');
    r\tellAdmin($err, ['ip' => $_SERVER['REMOTE_ADDR']]);
    return TRUE;
  }
  
  $err = t('bad login');
  if (!$uid) return TRUE;

  $uidOnly = FALSE; //variable_get('user_failed_login_identifier_uid_only', FALSE);
  $window = 6 * HOUR_SECS; //variable_get('user_failed_login_user_window', 6 * HOUR_SECS);
  $sta['flood_control_user_identifier'] = $id = $uidOnly ? $uid : $uid . '-' . ip_address();
  if (!\flood_is_allowed('failed_login_attempt_user', MAX_PW_FAILS, $window, $id)) {
    $sta[$trigger] = 'user';
    $err = t('There have been too many failed sign-in attempts to your account (%MAX_PW_FAILS). Try again later.');
    return TRUE;
  }

  return FALSE;
}

function noFlood($sta) {\flood_clear_event('failed_login_attempt_user', @$sta['flood_control_user_identifier']);}

function updateFlood($sta, $success) {
  if ($success) return noFlood($sta); // Clear failures (don't block a user who might log in and out more than once in an hour)
  
  \flood_register_event('failed_login_attempt_ip', 3600); // Always register an IP-based failed login event.
  if ($id = @$sta['flood_control_user_identifier']) \flood_register_event('failed_login_attempt_user', 21600, $id); // Register a per-user failed login event.

  if ($trigger = @$sta['flood_control_triggered']) {
    $msg = $trigger == 'user' ? t('Sorry, there have been more than :MAX_PW_FAILS failed sign-in attempts for this account. It is temporarily blocked.') : t('Sorry, too many failed login attempts from your IP address. This IP address is temporarily blocked.');
    w\say($msg . t(' Try again later or <%a>request a new password</a>.', '_a', atag('/user/password')), 'name');
  } else return w\say(t('bad login', '_a', atag('/settings/password')), 'name');
}


/**
 * Set session variables and other globals.
 * Call: $a = setAcct($myid) to set the current account (and agent if none yet)
 *    OR $a = setAcct() to remember what account and agent we are signed in to
 *    OR setAcct(NULL) to simulate signing out (for testing)
 * @param int $myid: the new account, if any
 * @return the current or new account
 */
function setAcct($myid = '') {
  global $channel, $boxUser, $box;

  if (is_null($myid)) {
    svar('myid', NULL); svar('agentId', NULL);
    return r\Acct::setDefault(NULL);
  }

  $agentId = svar('agentId');
  
  if ($myid) {
    if (!$agentId) $agentId = svar('agentId', $myid);
    svar('myid', $myid);
    db\update('sessions', ray('uid acct', $agentId, $myid), 'uid');
  } else $myid = svar('myid');

  if (!@$channel) { // only once per HTTP request
    $channel = TX_WEB;
    if (@$myid) $boxUser = boxUser($box = box(), $myid); // so far used only when recording transactions
  }
  
  $mya = r\acct($myid, $agentId);

  return r\Acct::setDefault($mya);
}

/**
 * Return all the session variables
 * @param string $sid: the session ID
 */
function sessionVars($sid) {
  if (!$both = db\get('uid,session', 'sessions', 'sid=:sid', compact('sid'))) return [];
  extract($both);
  $res['uid'] = $uid;
  
  $vars = explode(SVAR_HEADER, $session);
  array_shift($vars); // first is empty of course
  foreach ($vars as $one) {
    list ($k, $v) = explode('|', $one);
    $res[$k] = unserialize($v);
  }
  return $res;
}

/**
 * Set (if second parameter is given) or retrieve an rCredits session variable.
 * @param string $name: name of variable
 * @return: current value of the named session variable
 */
function svar($name) {
  $name = SVAR_HEADER . $name;
  $args = func_get_args();
  $res = @$_SESSION[$name]; // current value
  if (count($args) > 1) $_SESSION[$name] = $args[1];
  return $res;
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
    svar('pop', compact(ray('formName choices resultField info question')));
    w\go($formName); // reload page
  }

  if (!is_array($pop = svar('pop'))) return [];
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($formName != $pop_formName) return [];
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return [];
  
  svar('pop', NULL); // called with no args (#3)
//  $tribs['onchange'] = "this.form.elements['$pop_resultField'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $tribs['size'] = 12;
  $choice = selectFld('', '', w\attrib($tribs), $pop_choices);
  $choice = render($choice);
  $legend = t('Which one?');
  $popup = item(w\modal('which', $legend, $pop_question, $choice, t('Cancel')));
  jsx('which', 'field', $pop_resultField);
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
  if ($prompt !== '') $prompt = "<p>$prompt</p>";
  return <<<EOF
<div id="$id" class="modal fade" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="pull-right btn btn-xs btn-default" data-dismiss="modal">$closer</button>
        <h4 class="modal-title">$legend</h4>
        $prompt
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
//    $onclick = "jQuery(this).find('.glyphicon').css('color', 'darkblue');"; // glyph looks funny when ladda-spinning
//    $list .= spinLink("/$k", $text, '', 'list-group-item', '', compact('onclick'));
    $list .= spinLink("/$k", $text, '', 'list-group-item', '');
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
  global $mya;
  if (!is_array($perms)) $perms = array($perms);
  foreach ($perms as $perm) if (!$mya->can($perm)) return [];
  return goButton($label, $goto, $title);
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
 UNUSED, but keep for migrating data to r_company
 */
function acctType($acctType) {  
  $flags = ($acctType == CO_PERSONAL ? 0 : u\bit(B_CO));
    
  if ($acctType != CO_PERSONAL) { // company
    $type = $GLOBALS['account types'][$acctType];
    $coFlags =
        ($acctType == CO_SOLE_PROPRIETOR ? u\bit(CO_SOLE_PROPRIETOR)
      : ($acctType == CO_PARTNERSHIP ? u\bit(CO_PARTNERSHIP)
      : (strpos($type, 'nonprofit') !== FALSE ? u\bit(CO_NONPROFIT)
      : (u\bit(CO_CORPORATION) | (strpos($type, 'publicly') !== FALSE ? u\bit(CO_PUBLICLY_TRADED) : 0)
    ))));
  }
  return compact(ray('flags coFlags'));
}

/**
 * Create relation for company relationship specified in registration form (if any)  
 * @return TRUE if there is a formal relationship
 */
function suCompanyRelation($other, $data) {
  extract(just('company companyPhone owner employee contractor', $data));
  if ($main = findCompany($company, @$companyPhone)) { // company account exists
    return r\acct($main)->newRelation(compact(ray('other employee owner'))); // no permissions/draw (for security)
  } else return FALSE;
}

/**
 * Set up relationship of this new account with the account that created it (if any).
 */
function suCreatorRelation($helper, $args, $myid) {
  extract($params = just('owner employee flow', $args));
  if (!@$owner and !@$employee and !@$flow) return; // no useful relation (for example by true invitation)

  $draw = ($flow & 2) ? 1 : 0;
  r\acct($myid)->newRelation(ray('other permission employee owner draw', $helper, r\perm(B_MANAGE), $employee, $owner, $draw));
  if ($flow & 1) r\acct($helper)->newRelation(ray('other draw', $myid, TRUE));
//  } else r\tellAdmin("bad params in formSignup: $error -- ", $params);
} 

/**
 * Check email format and duplication and complain if appropriate.
 * @param string $email0: the email address to check (also RETURNED with improvements)
 * @param bool $dupsOk: <allow duplication of another account's email address>
 * @param acct $a: current account if any
 * @return: TRUE if email is okay to use, else FALSE
 */
function emailOkay(&$email0, $dupsOk, $a = FALSE, $suppressErr = FALSE) {
// NO  $email = strtolower(str_replace("'", '&#039;', $email0));
  if (!u\validEmail($email = $email0)) return $suppressErr ? FALSE : say('bad email', compact('email'), 'email');
  if (!$dupsOk and emailDup($email, $a ? $a->id : 0, $suppressErr)) return FALSE;
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
function emailDup($email0, $uid, $suppressErr = FALSE) {
  global $base_url;
  $mail = u\cry('P', $email0);
  if (!$who = db\get('fullName', 'users', 'mail=:mail and uid<>:uid AND NOT :IS_CO', compact('mail', 'uid'))) return FALSE;
  if (!$suppressErr) {
    $_a = atag("/settings/password/$email0");
    $emailTagged = str_replace('@', '+whatever@', $email0);
    $dupMsg = 'duplicate email';
    if (stripos($email0, '@gmail.com')) $dupMsg .= '|email plus';
    if (!$uid) $forgot = '|forgot password';
    say($dupMsg . @$forgot, compact(ray('emailTagged who _a')), 'email');
  }
  return TRUE;
}

/**
 * Tell the staff about a new member.
 */
function tellStaffNewMember($params) {
  extract($params);
  $phone = u\fmtPhone(@$phone);
  $state = @$state ? r\realState($state, $country) : 'state lookup failed';
  $country = r\realCountry($country);
  $coTypes = ray(CO_TYPES);
  $acctType = $coTypes[$acctType];
  $helper = @$helper ? r\acct($helper)->fullName : 'unknown';
  $copts = u\jsonEncode(@$copts);
  $ats = @join(',', array_values($ats));
  $params = compact(ray('phone country state acctType helper copts ats')) + $params;
  unset($params['code']); // don't need to see this
  $message = '<h2>NEW SIGNUP</h2>';
  r\tellCO($message, $params);
}

/**
 * Return a record ID for the given company or company phone.
 * @return record ID (FALSE if not found)
 */
function findCompany($company, $companyPhone) {
  if (!$company and !$companyPhone) return FALSE;
  $companyShortName = u\shortName($company);
  $companyUid = db\get('uid', 'users', ':IS_CO AND phone=:companyPhone', compact('companyPhone'));
  if (!$companyUid) $companyUid = db\get('uid', 'users', ':IS_CO AND name=:companyShortName', compact('companyShortName'));
  if (!$companyUid and strlen($companyShortName) > 8 ) {
    $companyShortName .= '%';
    $companyUid = db\get('uid', 'users', ':IS_CO AND name LIKE :companyShortName', compact('companyShortName'));
  }
  return $companyUid;
}

/**
 * Return form fields for name and email.
 * @param bool $what: prompt description of what the name is for ("Company", "Community", or "Full name")
 * @param string $fullName: default full name
 * @param string $email: default email address
 * @param bool $hideEmail: <don't ask for email address>
 * @param int $myid: current account record ID, if any
 * @return the relevant form fields
 */
function nameAndEmail($what, $fullName = '', $email = '', $hideEmail = FALSE, $myid = 0) {
  $who = $what == t('Company') ? t('The company\'s') : ($what == t('Community') ? t('The community\'s') : t('Your'));
//  $onchange = w\onchange("var legal=jQuery('#edit-legalname'); if (legal.val()=='') legal.val(this.value);");
//  $fullName = textFld("$what:", ["$who " . t('full name'), t('usename desc')], required(@$fullName) + $onchange);
  $fullName = textFld("$what:", ["$who " . t('full name'), t('usename desc')], required(@$fullName));
  $legalName = textFld(t('Legal name:'), ["$who " . t('full legal name (if different)'), $who . t('legalname desc')]);
  if ($email and !emailOkay($email, FALSE, $myid ? r\acct($myid) : '')) $hideEmail = FALSE; // make sure they can fix it
  if (strpos($email, '@' . EMAILX) and !u\test()) $hideEmail = FALSE; // handle placeholder for making joint accounts
  $email = $hideEmail ? hidFld(@$email) : textFld(t('Email:'), [t('Email'), t('All emails from the system will be sent to this address. It will not be made public. <b>Type carefully.</b>')], required(@$email));
  jsx('legal-name');
  return compact(ray('fullName legalName email'));
}

/**
 * Return the part of the form that is (mostly) common to formContact and formSignup
 * @param assoc $defaults: initial field values (defaults to current values) - see list at "$fields ="...
 * @param bool $step0: <called from signup form>
 *   plus {partner, partnerCode, customer} for signup from partner site
 * @param int $myid: account record ID of account for which to show existing contact field values, if any
 */
function contactFields($defaults = [], $step0, $myid = 0) {
  global $mya;
//  $country = $state = ''; // make sure these are not NULL
  $partnerExtras = ' partner customer partnerCode'; // add-on fields passed through only for partner signup (start with space) (even partnerCode is necessary, in case form is redrawn on error)
  $fields = 'phone country physical address city state zip tenure owns sameAddr2 address2 city2 state2 zip2 postalAddr hidCountry hidState hidState2 source source2';
  $a = $myid ? r\acct($myid) : FALSE;
  foreach ($defaults as $k => $v) if (is_null($v)) unset($defaults[$k]); // needed for joint account setup
  extract($defaults + ($a ? (array) $a->account($fields) : []));

  if (@$postalAddr) list ($address2, $city2, $state2, $zip2) = u\parseAddr(@$postalAddr);
  if (!@$state2) $state2 = 0; // otherwise default fails in countrys.js
  if (!@$country) $country = US_COUNTRY_ID;

  foreach (['state', 'state2'] as $k) {
    if (@$$k) $$k = db\get('id', 'r_states', ":$k IN (id, name, abbreviation) and country_id=:country", compact($k, 'country'));
  }
  js('countries'); // required for setPostalAddr, whether or not the fields are shown

  if ($step0 and @$address and @$city and @$state and !u\badZip($zip) and !u\poAddress($address)
    and @$address2 and @$city2 and @$state2 and !u\badZip($zip2)) { // already got a bunch of data

    $postalAddr = ''; // get it from the separate fields
    $hidCountry = $country;
    $hidState = $state + 0;
    $hidState2 = $state2 + 0;
    if (@$partner and @$partnerCode and @$customer // customer signup 
      and ($partnerA = r\acct($partner) and $partnerA->emailCode == $partnerCode)) { // using valid data from partner org
      $fields .= $partnerExtras;
      $hideNot = 'tenure owns'; // DO ask about these
    } else $hideNot = u\badAmount($tenure, '>=0') ? 'tenure' : ''; // otherwise this is a joint Account setup or error, so DON'T ask about tenure and owns (unless tenure error)

    $fieldsToHide = justNOT($hideNot, ray($fields));
    foreach ($fieldsToHide as $k) $$k = hidFld(@$$k);
    
  } else {
    $fieldsToHide = [];
    $country = @$country + 0;
    $state = @$state + 0;
    $hidCountry = hidFld($country + 0); // remember this when Drupal forgets it on error
    $hidState = hidFld($state + 0); // remember this when Drupal forgets it on error
    $hidState2 = hidFld(@$state2 + 0);

  //  js('http://ziplookup.googlecode.com/git/public/ziplookup/zipLookup.min.js', 'file', 'header');
//    js('zipLookup.js', 'file', 'footer');
    jsx('addr', compact(ray('country state state2')));

/*    $countryAttrib = w\onchange("
      print_state(this.options[this.selectedIndex].value,'','state');
      print_state(this.options[this.selectedIndex].value,'','state2');
    ");
*/
    $stateAttrib = $addressAttrib = $cityAttrib = $zipAttrib = [];
    $phone = textFld(t('Phone:'), [t('Your phone number')], required(@$phone ? u\fmtPhone($phone): ''));
    $faxetc = @$faxetc ? textFld(t('Fax etc:'), [t('Other contact info'),t('List other ways to contact you: fax, mobile, IM, etc. This field is NOT public.')], dft(@$faxetc)) : NULL;
//    $country = selectFld(t('Country:'), t(''), dft(@$country) + $countryAttrib); // can't use required() here
    $country = selectFld(t('Country:'), t(''), dft(@$country)); // can't use required() here
    $physical = item('', t('Physical Address'));
    $address = textFld(t('Street Address:'), [t('Physical address'), t('Where is your building located (physical address)?')], required(@$address) + $addressAttrib);
    $city = textFld(t('City:'), [t('City where you live')], required(@$city) + $cityAttrib);
    $state = selectFld(t('State:'), t(''), dft(@$state) + $stateAttrib); // can't use required() here
    $zip = textFld(t('Postal Code:'), [t('Physical location postal code'), t('Postal code for your <em>physical location</em>')], required(@$zip) + $zipAttrib);

    $sameAddr2 = ($step0 ? @$state2 : !(@$mya and $mya->cAdmin)) // show box on signup (until mailing addr filled) & to admin
    ? item('', t('Mailing Address')) 
    : w\boxFld('sameAddr', t('Mailing Address'), t('same as above'), 0);
//    : w\boxFld('sameAddr', t('Mailing Address'), t('same as above'), 0, w\onchange('setPostalAddr(true)'));
    
    $postalAddr = hidFld();
    $address2 = textFld(t('Street Address:'), [t('Postal street address')], required(@$address2) + $addressAttrib);
    $city2 = textFld(t('City:'), [t('Postal city')], required(@$city2) + $cityAttrib);
    $state2 = selectFld(t('State:'), t(''), dft(@$state2) + $stateAttrib); // can't use required() here
    $zip2 = textFld(t('Postal Code:'), [t('Postal code'), ''], required(@$zip2) + $zipAttrib);
    
    if (!@$mya or !$mya->co) {
      $source = $step0 ? textFld(t('Referred by:'), [t('How you heard about %PROJECT'), t('Where did you hear about %PROJECT? (name of company or person, web address, publication name, radio station name, or whatever)')]) : NULL;
      $source2 = $step0 ? textFld(t('Code:'), [t('Invitation or card code'), t('If you have an invitation or card code, type it here ("B" for brochure). Otherwise leave blank.')]) : NULL;
    } else $source = hidFld(@$source);
  }

  if (!in('tenure', $fieldsToHide)) $tenure = $step0 ? textFld(t('How long:'), [t('MONTHS at current address.'), t('How long have you been at your current address, in months (roughly)')], required(@$tenure)) : NULL;
  if (!in('owns', $fieldsToHide)) $owns = boolFld('', '', $step0 ? @$owns : $a->owns, [t('Rent'), t('Own')]); // default fails

  return compact(ray($fields));
}  

/**
 * Return a list of company categories.
 */
function coCats() {
  $sql = <<<EOF
    SELECT iid, IF(parent=iid, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY parent, IF(parent=iid,0,iid)
EOF;
  return db\q($sql)->fetchAllKeyed();
}
  
/**
 * Return the fields used to verify an individual's SSN (also asked for in signup).
 */
function ssnFields($personal = TRUE) {
  $ssTitle = t($personal ? 'ssTitle' : 'einTitle');
  if ($personal) {
    $dob = textFld(t('Birthdate:'), [t('Birth date'), t('Your date of birth (mm/dd/yyyy)')], required() + autofill('off'));
// NOT YET    $ssReason = t('As a registered Money Services Business, we are required by law to ask for your social security number, even though it makes some people nervous. Fortunately, we have <%aSecure>top notch security</a>. If you do not have a social security number <%aUndoc>click here</a>.', '_aSecure _aUndoc', atag(r\promoLink('security.html')), atag("$$('.form-item-sponsor').show(); $('.form-item-federalId').hide();"));
    $ssReason = t('As a registered Money Services Business, we are required by law to ask for your social security number, even though it makes some people nervous. Fortunately, we have <%aSecure>top notch security</a>.', '_aSecure', atag(r\promoLink('security.html')));
// NOT YET    $sponsor = textFld(t('Sponsor:'), [t('Sponsor code'), t('Your sponsoring organization\'s Common Good account ID (required if you have no social security number).')]);
  } else {
    $dob = textFld(t('Founded:'), [t('Founding date'), t('When did this company begin, approximately (mm/dd/yyyy)')], required());
  }
  
  $federalId = passFld($ssTitle, [t($personal ? 'ssDesc' : 'einDesc'), @$ssReason], required() + autofill('off')); // textfield so user doesn't have to retype on error
  
  return compact(ray('dob federalId sponsor'));
}

/**
 * Say whether the phone number is acceptable.
 * @param string $phone: phone number in any format
 * @param int $country: country code
 * @return TRUE if the phone number is acceptable
 */
function phoneOk(&$phone, $country = US_COUNTRY_ID) {
  if (!$phone = u\fmtPhone($phone, '+n')) return say('bad phone', 'phone');
  if ($country == US_COUNTRY_ID and strlen($phone) != 12) return say('bad phone', 'phone');
//  global $mya;
//  $myid = $mya ? $mya->id : 0;
  //$dup = db\exists('users', 'phone=:phone and uid<>:myid', compact('phone', 'myid'));
  // NO! this makes contact form fail. if (!$dup and $dupOk) return say('not a duplicate phone', 'phone');
  return TRUE;
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function adminField($label) {
  global $mya;
  return ($mya->cAdmin and $mya->federalId) ? item(R_ON_FILE, $label) : NULL;
}

function onFile($field, $sta) {
  return (@$sta['complete form'][$field]['#value'] == R_ON_FILE and @$sta['input'][$field] == R_ON_FILE);
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  $zot = array('zot' => selectFld('', '', compact('value') + attrib(compact('name')), $options));
  return \render($zot);
}

function amountField($name, $value, $id) {
  $name = "$name-$id";
  $class = 'amount'; $class = compact('class');
  $zot = array('zot' => textFld('', '', compact('value') + attrib(compact('name', 'class'))));
  return \render($zot);
}

function toggle($field, $value, $id, $highlight = 'never') {
  $name = $id = "$field-$id";
  $zot = w\togFld($id, '', '', $value);
  return \render($zot);
}
  /*
function toggle($field, $value, $id, $highlight = 'never') {
  $path = current_path();
  $name = $id = "$field-$id";
  $zot = array($id => hidFld($value, attrib(compact(ray('id name')))));
  $input = \render($zot);
  $b1 = button(' - ', "toggle('$id');", t('Change to NO'), 'danger');
  $b2 = button(' + ', "toggle('$id');", t('Change to YES'), 'success');

  if ('never' != (string) $highlight) {
    list ($visHigh, $invisHigh) = u\order($highlight, ' highlight', '');
  } else $highlight = $visHigh = $invisHigh = '';

  list ($yesvis, $novis) = u\order($value, "visible$visHigh", "gone$invisHigh");
  return <<<EOF
    <div class="$field-buttons$highlight">
    <div id="$id-YES" class="$yesvis form-yes">Yes $b1</div>
    <div id="$id-NO" class="$novis form-no">No &nbsp;$b2</div>
    $input
    </div>
EOF;
}
*/

/**
 * Return the recipient of a recent invitation OR mark an invitation USED
 * Call by:
 *   $email = invitation($code, '', $inviter, $iCode, $err) returned with $inviter=inviter uid
 *   invitation($code, $invitee) marks the invitation USED by recording the invitee's account record ID
 * @param string $code: the invitation code
 * @param int $invitee: account record ID of the invited person
 * @param int $inviter: (RETURNED, if invitee='') account record ID of the inviter (returned even if error)
 * @param int $iCode: (RETURNED) the invitation sequence number
 * @param int $err: (RETURNED) error message, if any
 * @return:
 *   (1) encrypted email, if invited specifically with our form
 *   (2) TRUE if invited with an emailable link or marking invitation USED
 *   (3) FALSE if no such invitation or marking invitation USED failed
 */
function invitation($code, $invitee = '', &$inviter = '', &$iCode = -1, &$err = '') {
  if (!$invitee and $a = r\iCardAcct($code, $iCode)) {
    if ($iCode >= IBY_ICARD and db\exists('users', 'helper=:id AND iCode=:iCode', ray('id iCode', $a->id, $iCode))) return !$err = 'used invite';
    $inviter = $a->id;
    return TRUE;
  } else {
    $fields = 'id,email,inviter,invitee,invited';
    if (!$result = db\get($fields, 'r_invites', 'code=:code', compact('code'))) return !$err = 'bad invite';
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
    return u\decry('P', $email); // $inviter is returned implicitly
  }
}

/**
 * Return a formatted, categorized list of businesses
 * @param string $which: word search for business name or industry
 * @param mixed $region: community (the default), zip (maybe partial), or country (maybe partial)
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
    $regionCrit = '(zip LIKE :region OR country LIKE :region)';
  }

  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $rows = [];

  $sql = <<<EOF
    SELECT DISTINCT u.uid
    FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE :IS_CO AND u.uid>:CANONIC_ACCTS AND :IS_OK AND $regionCrit AND (u.fullName LIKE :which OR i.industry LIKE :which)
    ORDER BY u.fullName
EOF;
///   debug(compact(ray('region which sql')));
  $result = db\q($sql, compact(ray('region which')));
  return r\companies($result->fetchCol(), FALSE);
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
  if (!$eLink) extract(just('eLink', $sta)); // verifying or submitting; get stored eLink args
  extract(just('qid ecode allow', $eLink));
  $a = r\acct($qid = @$qid); // current or specified account
  if ($allow != 'any') {
    if (!$a) return w\go('', 'requires signin');
    if ($allow == 'individual' and $a->co) return w\go('', t('That page is for individual accounts only.'));
    if ($qid and @$ecode != $a->emailCode) return w\hack('bad ecode');
  }
  $eLink = $sta['eLink'] = "allow=$allow&qid=$qid&ecode=" . @$ecode; // remember it (cttys don't use ecode)
  return $a;
}

function hack($message, $info = []) {
  u\loga('hack', $_SERVER + compact('message'));
  r\tellAdmin($message, $info);
/**/	if (isDEV) debug("$message: " . print_r($info, 1));
  return r\signedIn() ? signout() : w\go('');
}

/**
 * Let there be no current user.
 * @param bool $goPromo: <go to Promo site after signout>
 */
function signout($goPromo = TRUE) {
  @\session_destroy();
  if (isDEV and !u\test()) w\go('signin');
  if ($goPromo) return w\go(PROMO_URL . '?region=x');
}

function fundRpt($now, $then, $type = '') {
  //$up = $then ? number_format(abs($now / $then - 1) * 100, 1) : '100';
  $RUS = strpos($type, 'r') === FALSE ? '' : '@R';
  $change = u\fmtAmt(abs($now - $then));
  $way = $now >= $then ? t('up') : t('down');
//  $s = u\fmtAmt($now) . t("$RUS &mdash; @way @change$RUS from a month ago", compact('way', 'change'));
  $s = u\fmtAmt($now) . t($RUS, compact('way', 'change'));
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
  global $txDays;
  extract(just('period starting ending states', $args));
  u\setDft($states, $settings);
///  debug($states);
  if ($choices) {
    if ($states) $stateDfts[] = ray($states);
    $states2 = boxesFld(t('Show:'), '', @$stateDfts, $choices);
  }
  $showingAdv = ($states != $settings or (@$ending and $ending != strtotime('tomorrow') - 1));
  jsx('advanced-dates', compact('showingAdv')); // showing advanced selection
  u\setDft($period, strpos($url, 'invoice') ? TX_DEFAULT_INV_PERIOD : TX_DEFAULT_PERIOD);
  list ($starting, $ending) = rangeDates(@$period, @$starting, @$ending);
  $url = BASE_URL . "/$url/period=$period&starting=$starting&ending=$ending&states=$states";

  $showAdvanced2 = '<a id="showAdvanced">advanced</a>';
  $submitPeriod = submi(t('Show')); // label has to be different from submitDates (see below)
  if ($downloadable) $downloadPeriod = submi(t('Download'));
///  $actions = render($submitPeriod) . render(@$downloadPeriod) . render($showAdvanced);
  $actions = fieldSet('actionsSimple', compact('submitPeriod', 'downloadPeriod'), '', suffix($showAdvanced2));
///  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
//  $onchange = "$('#edit-starting, #edit-ending').val('');";
//  $onchange = "var id='#edit-submitPeriod'; if (!$(id).is(':visible')) id='#edit-downloadPeriod'; $(id).click();";
//  $period = selectFld(t('For the past:'), '', dft($period) + w\onchange($onchange), $txDays);
  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
//  $period = selectFld(t('For the past:'), '', dft($period), $txDays);
  $simple = fieldSet('simple', compact('period', 'actions'), BARE);
  
  $res = compact(ray('url starting ending states')); // the easy parts of the result
  $submitDates = submi(t('show')); // label has to be different (Drupal bug -- else can't tell which is clicked)
//  $showSimple2 = '<a id="showSimple" href="javascript:void(0);" onclick="jQuery(\'#advanced\').hide(); jQuery(\'#simple\').show();">simpler</a>';
  $showSimple2 = '<a id="showSimple">simpler</a>';
  if ($downloadable) $downloadDates = submi(t('download')); // must be lowercase
  $actions = fieldSet('actionsAdvanced', compact('submitDates', 'downloadDates'), '', suffix($showSimple2));
  $starting = textFld(t('Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textFld(t('Ending:'), '', dft(u\fmtDate($ending)));
  if ($downloadable) $downloadMsg = item(t('<b class="loud">TIP</b>: Tell your browser to allow popups for this site.'));
  $advanced = fieldSet('advanced', compact(ray('starting ending states2 actions')), BARE);
  $dateRange = compact(ray('simple advanced downloadMsg'));
  return compact('dateRange') + $res;
}

/*
function dateRange2($args, $url, $download = TRUE) {
  extract(just('starting ending', $args));
  if (!@$starting) { // default to previous month
    $ending = u\monthDay1() - 1;
    $starting = u\monthDay1($ending);
  }
  $url = hidFld(BASE_URL . "/$url/starting=$starting&ending=$ending");

  if ($download) $download = submit(t('Download')); else $show = submit(t('Show'));
///  debug(render($submitPeriod));
///  debug(rendA(compact('submitPeriod')));
///  $actions = render($submitPeriod) . render(@$downloadPeriod) . render($showAdvanced);
  
  $starting = textFld(t('Starting:'), '', dft(u\fmtDate($starting)));
  $ending = textFld(t('Ending:'), '', dft(u\fmtDate($ending)));
  $advanced = fieldSet('advanced', compact(ray('starting ending download show url')), BARE);
}
*/

/**
 * Validate the chosen date range.
 * @param bool $limit: <limit the range to 180 days or less>
 */
function dateRangeValidate(&$sta, $limit = TRUE) {
  extract(just('period starting ending download', $sta['input']));
  $op = op($sta);
  $downloadOp = ($op == 'submitPeriod' or $op == 'downloadPeriod');
  list ($starting, $ending, $period) = $downloadOp ? rangeDates(@$period) : rangeDates('', @$starting, @$ending);
  if ($limit and $ending - $starting > TX_MAX_DAYS * DAY_SECS and !$downloadOp) return w\say('too many days', 'err');
  u\preray(compact(ray('starting ending period')), $sta['input']);
}

/**
 * Initiate a download, if appropriate.
 */
function downloadCheck($page) {
  global $base_url;
  if ($query = w\svar("$page-downloadQuery", '')) { // get value and empty it
    jsx('download', 'url', "$base_url/$page/$query");
  }
}

/**
 * Handle the results of the date range dialog.
 * @param string $page: what page we're coming from and going to
 * @param assoc $sta: form status array, for finding out whether user pressed Download and building redirect url
 * @param assoc $sta: (MODIFIED) with $sta['redirect'] set unless downloading
 */
function dateRangeRedirect($page, &$sta) {
  if (confirming_s($sta)) return;

  $sta['input']['states'] = @join(',', @$sta['input']['states'] ?: []);
  $query = http_build_query(just('states period starting ending', $sta['input']));
  if (u\starts(op($sta), 'download')) {
    w\svar("$page-downloadQuery", $query);
    say('downloading');
  } else return w\go("$page/$query");
//  } else $sta['redirect'] = "$page/$query";
}

/**
 * Return usable starting and ending dates (inclusive).
 * @param int $period: index into TX_DAYS -- for calculating starting and ending if none explicitly specified
 * @param int $starting: explicit starting date (optional)
 * @param int $ending: explicit ending date (optional)
 * @return array($starting, $ending, $period), where $starting is adjusted to start of day and $ending to end of day
 */
function rangeDates($period, $starting = '', $ending = '') {
  global $txDays;
  if (@$starting and !is_numeric($starting) and $err = u\badDate($starting)) say($err, 'starting');
  if (@$starting > r\rTime()) say('beyond today', 'starting');
  if (@$ending and !is_numeric($ending) and $err = u\badDate($ending)) say($err, 'ending');
  if (!@$ending or $ending > r\rTime()) $ending = r\rTime();

  $period = (int) @$starting ? ($ending - $starting) / DAY_SECS : (@$period ?: TX_DEFAULT_PERIOD);
  $ago = $period == '1' ? '1 day' : (@$txDays[$period] ?: ($period . ' days'));
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
  $form['#attributes'] = array('class' => ray('confirmation'));
//  $form['#validate'] = 'CG\\Web\\sureForm_validate';
//  $form['#submit'] = 'CG\\Web\\sureForm_submit';
//  $sta['rebuild'] = TRUE;

  focusOn('confirm');

  return cgform($form);
}

/**
 * Figure out who the user means, offering choices to disambiguate
 * If the intended person or company does not exist, 
 * @param string $who: what the user typed
 * @param string $field_name: name of the field the user typed in
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $selfMsg: index to error message to give if user self-refers
 * @param bool $paying: <we're preparing to pay this person> (in which case the persn has to have an active account)
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $selfMsg = 'no self-trading', $paying = FALSE) {
  global $mya;
  if (u\isAcct($result = be\identify($who, @$mya->id, $paying, $selfMsg))) return $result;
  list ($msg, $args, $choices) = $result;
//  foreach (ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = plain(str_replace('/', R_URL_SLASH, $return)); // even urlencoded slashes confuse Drupal
  foreach ($choices as $uid => $fullName) $choices[$uid] = r\qid($uid) . ': ' . $fullName;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, t($msg, $args));
}
  
/**
 * Output the No-signin result message and terminate.
 * @param string $msg: the message or message index
 */
function doSay($msg, $ok = FALSE) {
  if (isDEV) @file_put_contents(f('t.dosayFilename', $ok ? 'status' : 'error'), $msg); // no way to know if actually testing
  return w\go('empty/say=1', $msg, !$ok);
}

/**
 * Add an item to the menu structure (called when reinstalling or rebuilding menus).
 * @param string $path: URL path to this item (without the domain)
 * @param string $title: human-readable label for operation
 * @param int $type: menu item type (see hook_menu)
 * @param string $function: what function to call when item is activated (with or without namespace qualifiers)
 * @param string $args: space-delimited list of arguments to pass to the function
 * @param string $perms: space-delimited list of permission bit names (lowercase without B_ )
 * @return the menu item array as expected by Drupal for hook_menu
 */
function menu($path, $title, $type, $function, $args, $perms = '') {
  global $menuWeight; $menuWeight += !@$menuWeight ?: 1;

  foreach ($args = ray($args) as $k => $v) if (is_numeric($v)) $args[$k] = (int) $v;
  
  $adminPerms = (stripos($perms, 'admin') !== FALSE and stripos($perms, 'icadmin') === FALSE);
  $admin = (u\starts($path, 'sadmin') or $perms == 'dev' or $adminPerms);
  $smart = ($path == 'pos');

  $formName = ($args and !is_numeric($args[0])) ? $args[0] : $function;
  $alt = $smart ? 'rsmart/rsmart.inc' : ($admin ? 'admin/admin-forms.inc' : 'rweb/rweb.inc');
  $file = strtolower("../forms/$formName.inc");
  if (!file_exists(R_ROOT . "/rweb/$file")) $file = "../$alt";
  
  $item = array(
    'title' => $title,
    'type' => $type,
    'page callback' => ($smart ? 'CG\\Smart\\' : 'CG\\Web\\') . $function,
    'page arguments' => $args,
    'access callback' => $perms ? 'CG\\Web\\webAccess' : TRUE,
    'access arguments' => $perms ? array($perms) : NULL,
    'weight' => @$menuWeight - 1,
    'menu_name' => 'main-menu', 
    'module' => $smart ? 'rsmart' : 'rweb',
    'file' => $file,
    'options' => ray('class', $title),
  );

  return $item;
}

/**
 * Return a help message for the page.
 * @param string $args: parameters to pass to rawHelp
 * @param assoc $args: (RETURNED) returned parameters (passed from rawHelp back to caller)
 */
function helpText($page, $tag = '', &$args = '') {
  include_once __DIR__ . '/rweb-help.inc';
  $page = strtolower($page);
  $help = $page == 'charge' ? strtr(rawHelp('pay', $tag, $args), ray('pay', 'charge')) : rawHelp($page, $tag, $args);
  if ($help) return u\SUBS(@$help, '%');
  return t('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at %regPhone. Or send your question by <%a>email</a>.', 'regPhone _a', u\fmtPhone(r\regionfield('phone')), w\emailtag(NULL, t('help with %PROJECT Members Site, %page page', 'page', $page)));
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
  global $mya, $fieldErr; // used mostly in tests (and in formTx)

  if (!@$index) return;
///  if (is_array($index) and !isset($index[0])) debug($index);
  if (is_array($index)) list ($index, $args, $errField) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $errField) = array($errField, $args); // allow either order, for easy 2-param calls
  $noTable = (strpos($index, '<table') === FALSE);
  $message = $noTable ? t($index, $args) : $index; 
  $fieldErr = ($errField and $errField != 'ERR');
  if ($fieldErr) \form_set_error($errField, $message); else \drupal_set_message($message, @$errField == 'ERR' ? 'error' : 'status', FALSE);
  $adminReport = ($mya and $mya->admin2 and strlen($message) > 200);
  if ($noTable and !$adminReport) u\loga($fieldErr ? 'sayerr' : 'say', compact('message'));
  return FALSE;
}

/**
 * Return a data series value, with formatting.
 * @param string $f: the formatted value
 * @param numeric $add: an amount to "stack" under the returned value (but not the formatted value)
 * @return an assoc [v, f], where v is the value and f is what to display when hovering over it
 */
function vf($f, $add = 0) {
  $v = strtr($f, ['$' => '', ',' => '', '%' => '']);
  if (strpos($f, '%')) $v *= .01;
  $v += $add; // assume add is a percentage, if v is
  return compact('v', 'f');
}
 
define('DATA_POINTS', 50); // how many x-axis data points to show in each dataset

/**
 * Return an iframe containing a chart
 * @param string $chart: the chart name
 * @param int $ctty: community account ID
 * @param bool $selectable: <show the account and community selectors> (default FALSE)
 */
function chartFrame($chart = 'growth', $selectable = FALSE, $ctty = NULL) {
  global $base_url, $mya;
  u\setDft($ctty, $mya ? $mya->community : 0);
  list ($w, $h) = $selectable ? [600, 600]: [480, 320];
  $url = isDEV ? "$base_url/rcredits/misc/chart.php?site=dev" : ("https://cg4.us/chart.php?site=" . str_replace('https://', '', $base_url));
  return w\tag('iframe', '', ray('src width height', "$url&ctty=0&chart=$chart&selectable=$selectable&t=" . time(), $w, $h)); // src must be first
}

function firstGraphDt($ctty) {  
  $dt = db\min('created', 'r_stats', 'ctty=:ctty', compact('ctty'));
  if ($ctty and $cttyA = r\acct($ctty)) $dt = max($dt, $cttyA->created);
  return max(R_LAUNCH, strtotime('today', $dt));
}

/**
 * Display charts representing the condition of the %PROJECT system in the given community.
 * @param string $whichChart: which chart or set of charts to display
 * @param int $ctty: account record ID of community to show graphs for (defaults to all communities)
 * @param assoc $chartData: (RETURNED) a data field containing the chart data
 * @return: html of the relevant charts, in their proper divs
 * FOR DEV: http://localhost/cgMembers/rcredits/misc/chart.php?ctty=0&chart=growth&site=dev
 */
function showCharts($whichChart, $ctty = 0, &$chartData = '') {
  global $base_url;

  list ($in, $out, $net, $personal, $companies) = array(t('in'), t('out'), t('net'), t('personal'), t('companies'));
  $txTypes = ray('p2p p2b b2b b2p', t('Person-to-person'), t('Person-to-business (mostly by %PROJECT card)'), t('Business-to-business (paying suppliers)'), t('Business-to-person (including payroll)'));
  $today = strtotime('today');
  $dt1 = w\firstGraphDt($ctty);
    
  $delta = max(1, floor(($today - $dt1) / DAY_SECS / DATA_POINTS));
  $dt = strtotime((-DATA_POINTS * $delta) . ' days', $today);
  $charts = ray('growth funds velocity banking volume issued', 'growth', 'dollar-pool', 'circulation-velocity', 'exchanges-for-dollars', 'transaction-volume', 'issuing-credits');
  if ($whichChart != 'all') $charts = just($whichChart, $charts);
  foreach ($charts as $k => $zot) u\setDft(${$k . 'Data'}, []);

  while ($dt <= $today) {
    $stats = r\stats($ctty, $dt);
    extract(just('ctty created', $stats)); // careful with this (don't overwrite local vars)
    if (!@$dt1) $dt1 = $created; // remember first date for this community
    $stats30 = r\stats($ctty, strtotime('-30 days', $created)); 
    extract(w\fundStats($stats, FALSE, $stats30)); // format, originals in {var}0, difs in d_{var}
    $ray = ray('v f', $pAccts0 + $bAccts0, $bAccts);
//    $growthData[] = [$created, vf($bAccts, $pAccts0), $pAccts0, $newbs0, $aAccts0, $conx0, $conxLocal0];  
    $growthData[] = [$created, vf($bAccts, $pAccts0), $pAccts0, $newbs0, $aAccts0];  
    $fundsData[] = [$created, vf($balsPos), vf($usd), vf(u\fmtAmt($balsPos0 - $d_txsVol0)), vf($topN), vf($botN), vf($floors), vf($balsNeg)]; // $bals === $usd, so not shown
    $velocityData[] = [$created, vf($cgRate, $localRate0), vf($localRate), vf($usdRate)];
    $bankingData[] = [$created, vf($d_usdIn), vf($d_usdOut), vf($d_cgIn), vf($d_cgOut)];

    $x2x = [];
    foreach ($txTypes as $k => $desc) {
      $v = max(1, ${"d_{$k}0"}); // to make the logarithmic scale look okay
      $cnt = ${"d_{$k}Count"};
      $cnt .= $cnt ? ' @ ' . u\fmtAmt($v / $cnt) . ' avg' : '';
      $f = ${"d_{$k}"} . " ($cnt) $desc"; // eg "$123 (7 @ $6.25 avg)"
      $x2x[] = compact('v', 'f');
    }
    array_unshift($x2x, $created);
    $volumeData[] = $x2x;
    
    // add a pie chart for p2p,... cash purchase (one for volume, one for count?)
    // also pie for circling/explicit savings/biz required nut/other (runny)
    // pie for admin showing channel? member activity (0 txs/mo, 1-5, etc.), where's the money, etc.

    $dt = strtotime("+$delta days", $dtX = $dt);
    if ($dt > $today and $dtX < $today) $dt = $today; // always show the latest
  }

  
//    $rate = number_format(100 * $d_txs, 1) . '%';

  $accts = "$pAccts members + $bAccts companies";
//  $funds = "$usd + {$balsPos}cg";
  $funds = $usd;
//  $velocity = ($bals0 ? $txsRate : 0) . t(' per mo.'); // FAILS (wrong amount)
  $velocity = ($bals0 ? u\fmtAmt($localRate0 + $cgRate0, '%') : 0) . t(' per mo.');
  $usd = $d_usd . t(' (net)');

  $avg = u\fmtAmt($d_txsVolCount0 ? $d_txsVol0 / $d_txsVolCount0 : 0);
  $txs = "$d_txsVolCount @ $avg avg";
  $topPct = $aAccts0 >= 100 ? '%' : '';

  $vs = compact(ray('accts funds velocity usd txs topPct dt1'));

  foreach($charts as $k => $help) {
    $one = "<div id=\"{$k}Chart\" class=\"onechart ct-chart ct-golden-section\"></div>";
    if (count($charts) > 1) $one = "<a href=\"$base_url/help/$help/qid=$ctty\">$one</a>";
    $charts[$k] = "<div>$one</div>";
    $dataNames[] = $k . 'Data';
  }

  $chartData = json_encode($data = compact('vs', $dataNames));

  return u\test() ? $chartData : join('', $charts);
}


/**
 * Return an array of formatted amounts for the given statistics.
 * Used in graphs and table of community funds
 */
function fundStats($stats, $round = FALSE, $stats30 = NULL) {
  extract($stats);
  if ($stats30) extract($stats30, EXTR_PREFIX_ALL, 'o');
//  list ($r0, $usd0) = array($r, $usd);

  if ($ctty) { // a community
    $cttyA = r\acct($ctty);
    $res['cttyName'] = $ctty == r\serverUid() ? t('Seedpack (Outliers)') : trim(strtr($cttyA->fullName, [t('Region')=>'', PROJECT=>'']));
  } else $res['cttyName'] = t('TOTAL');

  $scheme = f('i.scheme', 'r_stats');
  $fields = $scheme['fields'];
  
  // calculate monthly velocity with respect to total credit in circulation (balsPos not bals)
  foreach ($rateFlds = ray('cg usd txs local') as $z) {
    $stats["{$z}Rate"] = ($stats["{$z}Vol"] - $stats30["{$z}Vol"]) / (($balsPos + 0) ?: 1);
    $stats30["{$z}Rate"] = 0; // just to make d_ not die
  }
    
  foreach (array_keys($stats) as $k) {
    if (!in($k, 'id ctty created')) {
      if ($stats30) $stats["d_$k"] = $stats[$k] - $stats30[$k];
      $fmt = ($round or strpos($k, 'Count') or @$fields[$k]['type'] == 'int') ? '1' : 's$';
      if (strpos($k, 'Rate')) $fmt = '%';
//      if (strpos($k, 'Per')) $fmt = '';
      foreach ($stats30 ? [$k, "d_$k"] : [$k] as $fnm) {
        $res[$fnm . '0'] = $stats[$fnm] + 0; // not $$fnm (because that wouldn't catch rates)
        $res[$fnm] = u\fmtAmt($stats[$fnm], $fmt);
      }
    }
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
  global $mya, $base_url;
  $start = u\monthDay1($mya->created); // start of account's first month
  $thisMo = u\monthDay1(); // start of this month
  $end = $end ? u\plusMonths($end, $thisMo) : $thisMo;
  $endYear = date('Y', $end);
  $startYear = date('Y', $start);

  for ($year = $endYear; $year >= $startYear; $year--) {  
//    for ($month = $start; $month <= $; $month = u\plusMonths(-1, $month)) {
    $list[$year] = '';
    for ($month = strtotime('1/1/' . $year); 
         $month <= $end and date('Y', $month) == $year; 
         $month = u\plusMonths(1, $month)) {
      $mon = strftime('%b', $month);
      $list[$year] .= $month < $start ? '.<br>' : "<a href=\"$base_url/history/$things/$mon$year\" target=\"$target\">$mon</a><br>\n";
    }
  }
  
  if ($end >= $start) {
    $headers = $data = '';
    foreach ($list as $year => $v) {
      $headers .= "<th>$year</th>";
      $data .= "<td>$v</td>";
    }
    $list = "<table id=\"$things\">\n<tr>$headers</tr>\n<tr>$data</tr>\n</table>";
  } else $list = t('There are no %things available yet for this account.', 'things', strtolower($title));
  
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
  $item = compact(ray('mlid menu_name router_path link_path link_title module customized depth'));
  menu_link_save($item);
}

/**
 * Consolidate or extract hidden fields.
 */
function hidFlds($ray) {return is_array($ray) ? hidFld(plain(serialize($ray))) : unserialize(unplain($ray));}

/**
 * Send an invitation to the email and optionally report it.
 */
function sendInvite($email, $subject, $message, $dupsOk = TRUE) {
  global $mya;
  
  if (!$dupsOk) { // applies only to community admins
    $cemail = u\cry('P', $email);
    $now = r\rTime();
    if (db\exists('r_invites', 'email=:cemail and :now-invited<:R_INVITE_DAYS*:DAY_SECS', compact('cemail', 'now'))) return say(t('NOT sent to %email (dup).', compact('email')));
//     if (db\exists('users', 'email=:email', compact('email'))) return say("NOT sent to $email (already a member).");
  }

  $info = array(
    'uid' => $mya->id,
    'noFrame' => TRUE,
    'subject' => $subject,
    'personalNote' => str_replace("\n", "<br>\n", $message), 
    'fullName' => $mya->fullName, 
    'phone' => u\fmtPhone($mya->phone),
    'signed' => u\fmtDate($mya->signed),
    'CODE' => r\invite($email, $mya->id, '', $subject, $message),
  );
  r\rMail('invite', $email, $info, $mya);
  say(!$dupsOk ? 'Sent to %email' : 'invite sent|repeat invite', compact('email'));
  if (!$mya->admin) r\tellAdmin('invited', $info + compact('email'), $mya->id);
}

function focusOn($focusFieldName) {
  u\EXPECT(!empty($focusFieldName), 'focus on null');
  $focusFieldName = str_replace('_', '-', $focusFieldName);
  jsx('focus-on', 'field', $focusFieldName);
}

/**
 * Return TRUE if the current account has all the required permissions.
 */
function webAccess($perms = NULL) {
  global $mya;
  setAcct(); // recall who we are (DO THIS FIRST!)
  
  foreach (ray($perms) as $perm) {
    if ($not = (substr($perm, 0, 1) == '-')) $perm = substr($perm, 1);

    $pass =
      $perm == ANY ? TRUE // needed for submenus (see w\subMenu())
    : ($perm == 'dev' ? isDEV
    : ($perm == 'signedIn' ? (bool) $mya
    : ($perm == 'superadmin' ? ($mya and $mya->superAdmin)
    : ($perm == 'superadminORdev' ? (($mya and $mya->superAdmin) or isDEV)
    : ($perm == 'prose' ? ($mya and $mya->proSe)
    : ($perm == 'investor' ? ($mya and db\exists('r_stakes', 'uid=:id', ray('id', $mya->id)))
    : ($mya and $mya->can(u\consta('b', $perm))) ))))));

    if ($not ? $pass : !$pass) return FALSE;
  }
  return TRUE;
}

function css($sheet, $weight = -99) {
  global $rUrl;
  $options = ['group' => CSS_THEME, 'preprocess' => FALSE, 'external' => TRUE] + weight($weight);
  $version = isPRODUCTION ? R_VERSION : time();
  \drupal_add_css("$rUrl/css/$sheet?$version", $options);
}

/**
 * Return standard fields for the bottom of any form used during account setup.
 * 
 */
function setupFoot($label = '', $extra = []) {
  global $mya, $base_url;
  if (!$mya->member) {
    if ($remains = $mya->stepsRemaining() - 1) { // don't count current step
      $skip = button(t('Skip for now'), "$base_url/settings/skip", t('Leave this step for last'), 'warning', 'xs', ['class' => 'skip']);
      $nextStep = submit(t('Next ') . '&raquo;', '', '', w\suffix($skip));
      $steps = $remains == 1 ? t('step') : t('steps');
      $progress = item(t('Just <b>%n</b> more %steps to go', 'n steps', $remains, $steps));
    } else $nextStep = submit(t('Finish'));
  } else $submit = submit($label, '', '', $extra);
  return compact(ray('submit nextStep progress'));
}

/**
 * Return the text with a help popover and optional link.
 * @param string $text: text to hover over
 * @param string $help: help verbage
 * @param string $href: where to go on click (defaults to nowhere)
 * @param string $class: html class(es)
 */
function popHelp($text, $help, $href = '', $class = '') {
  $trigger = $href == '#' ? 'hover' : 'hover'; // click doesn't work :(
  list ($href, $tag) = ($href and $href != '#') ? [" href=\"$href\"", 'a'] : ['', 'span'];
  if ($class) $class = " class=\"$class\"";
  $text = ucfirst($text);
  return "<$tag$href$class type=\"button\" data-toggle=\"popover\" data-placement=\"bottom\" data-trigger=\"$trigger\" data-content=\"$help\" title=\"$text\">$text</$tag>";
}

/**
 * Check the amount, fix it in the input array, return any error.
 */
function amtErr($field, &$sta, $restriction = '', $fractionLimit = 2) {
  return u\badAmount($sta['input'][$field], $restriction, $fractionLimit);
}

function tags($tag, $content = '', $tribs = []) {
	$tribs = $tribs ? ' ' . u\tribs($tribs) : '';
	return "<$tag$tribs>$content</$tag>";
}

/**
 * Return an HTML tag with optional attributes, surrounding optional HTML.
 * @param string $tag: the tag
 * @param string $html: (optional) inner HTML
 * @param assoc $tribs: (optional) keyed-array of attributes
 * @return the resultant HTML
 */
function tag($tag, $html = '', $tribs = []) {
  $tribs = $tribs ? ' ' . u\tribs($tribs) : '';
  return "<$tag$tribs>$html</$tag>";
}
function tagN($tag, $html = '', $tribs = []) {return str_replace('#', '', tag($tag, $html, $tribs));}

/**
 * Return an a tag for a non-spinning button to a new page.
 */
function atagB($href, $style = 'primary', $size = 'xs', $extras = []) {
  $class = @$extras['class'];
  $extras['class'] =  ($class ? "$class " : '') . "btn btn-$size btn-$style";
  return atag($href, $extras + ray('role', 'button'));
}

/**
 * Return an a tag.
 */
function atag($href, $extras = []) {
  if (@$href[0] == '/') $href = BASE_URL . $href;
// NO  if ($href[0] == '$') $href = 'javascript:' . substr($href, 1);
  if ($href and !u\starts($href, BASE_URL)) {
    if (substr($href, 0, 6) == 'mailto' or substr($href, 0, 4) == 'http') $extras += w\away();
  } elseif (!$href) unset($href); // don't show empty href
  return 'a ' . u\tribs(compact('href') + $extras);
}
function emailtag($email = NULL, $subject = '', $body = '') {
  u\setDft($email, r\regionfield('email'));
  foreach (['subject', 'body'] as $k) if ($$k) $$k = "$k=" . urlencode($$k) . '&';
  return "a href=\"mailto:$email?$subject$body\" target=\"_blank\"";
}
function btn($href, $text, $style = 'primary', $size = 'xs', $extras = []) {
  return '<' . atagB($href, $style, $size, $extras) . ">$text</a>";
}
function lnk($href, $text, $extras = []) {return '<' . atag($href, $extras) . ">$text</a>";}
function doLnk($class, $text, $data = NULL, $extras = []) {return w\lnk('', $text, compact('class', 'data') + $extras);}
function doBtn($class, $text, $data = '') {return w\btn('', $text, 'primary', 'xs', compact('class', 'data'));}

/**
 * Skip a setup step and save it for last. Has no effect if step is already done.
 * @param string $step0: the step to skip (defaults to current step)
 * If defaulting to the current step, redirect to the next step.
 */
function skipStep($step0 = '') {
  global $mya;
  $step = $step0 ?: $mya->nextStep();
  $stepsDone = $mya->stepsDone;
  if ($stepsDone[$step]) return; // nothing to do
  unset($stepsDone[$step]);
  $stepsDone[$step] = FALSE; // put this step at the end
  $mya->update(compact('stepsDone'));
  if (!$step0) return w\go($mya->nextStepUrl());
}

/**
 * Return account and password fields for sign-in.
 */
function signinFlds($acct, $acctDesc = '', $passDesc = '') {
  $acct = textFld(t('Account ID:'), [t('Account ID'), $acctDesc], required($acct) + w\autofill('section-rSignin rAccount'));
  $pass = passFld(t('Password:'), [t('Password'), $passDesc], required() + w\autofill('section-rSignin rPass'));
  return [$acct, $pass];
}

/**
 * Format info about the new member's company.
 * @param assoc $info: company info from when the member signed up
 * @param string $company: (RETURNED) the company name
 */
function signupCoDisplay($info, &$company = '') {
  if (!@$info) return '';
  extract($info);
  return @$owner . ' ' . @$employee . ': ' . @$company;
}

/**
 * Get text from submitted submit button.
 */
function opText($sta) {
  $op = strtolower('edit-' . op($sta));
  foreach ($sta['buttons'] as $k => $one) if (@$one['#id'] == $op) return @$one['#value'];
  return '';
}

function backButton() {
  jsx('back-button'); 
  return '<button class="btn btn-back btn-xs btn-primary"><div>&laquo;</div><div>&nbsp;Go Back</div></button>';
}

// one-line functions that need no explanation

function disabled($value = NULL, $disabled = TRUE) {return attrib(compact('disabled')) + (!is_null($value) ? compact('value') : []);}
//function on($what, $do) {return w\attrib(["on$what" => $do]);}
//function onchange($do) {return on('change', $do);}
function dft($v) {return ray('default_value', $v);}
function weight($weight) {return compact('weight');}
function clas($class) {return ['class' => [$class]];}
function attrib($attributes) {return compact('attributes');}
function suffix($field_suffix = '') {return compact('field_suffix');}
function prefix($prefix) {return compact('prefix');}
function autofill($settings = 'on') {return attrib(['autocomplete' => $settings]);}
function required($dft = NULL) {return ['required' => TRUE] + (isset($dft) ? dft($dft) : []);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {global $testOp; return u\test() ? $testOp : substr(@$sta['input']['opid'], 5);} // without edit-
function verifyBy($byVoice) {return $byVoice ? t('Voice') : t('SMS');}
//function blurbLink() {global $rUrl; return "<a href=\"$rUrl/templates/invite.html\" target=\"_blank\">" . t('description of %PROJECT') . '</a>';}
function sayFieldErr($err, $field) {return say(strtoupper($field) . ': ' . t($err), $field);}
function rent($element) {return \render($element);} // Drupal makes render hard, requires pass by reference
function goNextStep($step, $msg = NULL, $info = []) {if ($step = r\acct()->nextStepUrl($step, $msg, $info)) w\go($step, $msg); elseif (@$msg) say($msg);}
function away() {return ['target' => '_blank'];}
function js($script, $args = '') {global $pageScripts; $pageScripts[$script] = http_build_query(rayy(func_get_args(), 1));} // see html.tpl
function jsx($script, $args = '') {global $scriptScraps; $scriptScraps[$script] = http_build_query(rayy(func_get_args(), 1));} // see html.tpl
function softErr($msg) {return w\go('empty', $msg, 'ERR');}
function clearHeaders() {if (!headers_sent()) header_remove();}