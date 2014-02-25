<?php
namespace rCredits\SMS;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
require_once __DIR__ . '/../rcredits-backend.inc'; // Drupal bug (needs this explicit)
require_once __DIR__ . '/../rcredits-util.inc';

/**
 * @file
 * rSMS include file
 *
 * Utilities and definitions for the rSMS module
 * (anything that doesn't have to be in rsms.module and isn't appropriate to class call).
 */

ini_set('session.cookie_lifetime', 0); // browser cookie deletion on browser close (this fails)
ini_set('session.gc_maxlifetime', 600); // "600" = 10 minutes
ini_set('session.gc_probability', 1); // these two lines
ini_set('session.gc_divisor', 100); // together will make garbage collection run on 1% of the requests

// Statuses
define('SMS_NO', 0); // not verified (maybe lost?)
define('SMS_PRIMARY', 1); // primary cell phone for notifications, etc. (and verified)
define('SMS_OK', 2); // verified
define('SMS_LOST', 9); // user lost phone (NOT YET USED)

define('RSMS_PREFIX', 'r: '); // identify us
//define('RSMS_PREFIX_INACTIVE', '[TRIAL RUN] '); // pretend transactions by not-yet-active players (treat it as a trial run)

$GLOBALS[TX_SMS . ' messages'] = array(
  // reports
  'system down' => t('The rCredits system is under maintenance at the moment. Try again in a minute or two?'),
  'account info' => t("Your balance for account @quid is @balance, including @rewards of incentive rewards. Total demand for rCredits is @totalDemand."),
//  'account info2' => t("More account info"), // not used yet
  'your account is ready' => t('Your starting balance is @balance. Pretend to buy and sell as you would in a typical month (we need that info). Have fun! Type "helpme" to get started.'), // separate out the last sentence
  'report transaction' => t("You @did @otherName @amount (@rewardType: @rewardAmount)."),
  'report offer' => t("You offered @otherName @amount (@rewardType: @rewardAmount). Your balance is unchanged, pending acceptance."), //  Transaction #@tid
  'report invoice' => t("You @did @otherName @amount. Your balance is unchanged, pending agreement."), // Invoice #@tid
  'report broke invoice' => t("@otherName does not have enough rCredits at this time, so an invoice has been sent, for @short."), // UNUSED probably // Invoice #@tid
//  'report short payment' => t("SPLIT! You paid @otherName @amount (@short remains to be paid). Transaction #@tid"),
//  'report short invoice' => t("SPLIT! You charged @otherName @amount (@short has been invoiced). Transaction #@tid, Invoice #@t2id"),
//  'report short invoice' => t("SPLIT! You charged @otherName @amount (@short remains to be paid). Transaction #@tid"),
   'report exchange' => t("You @did @otherName @amount for cash/loan/gift/etc."),
  'report exchange request' => t("You requested @amount from @otherName. Your balance is unchanged, pending approval."), //  Invoice #@tid
  'report funding' => t("You successfully @did @amount to @otherName."),
  'paid employees' => t('You distributed @total to @successes of @employeeCount employees. See your transactions page for details.'),
//  'new-invoice' => t('@otherName requests @amount for "@payerPurpose". Type "pay" or "undo".'),
//  'new-offer' => t('@otherName is offering you a payment exchange of @amount for "@payeePurpose". Type "accept" or "undo".'),
  'report undo' => t('Transaction has been @solution.'),
//  'report get usd' => t('@Exchange approved. @request will be transferred directly to your bank account within 24 hours.'),
//  'get usd inactive' => t('If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant.'),
  'your demand' => t('Your total request for rCredits is now @request.'),
  'updated relation' => t('Updated settings for relation to @otherName'),
  'deleted relation' => t('Deleted relation with @otherName'),
  'customer balance' => t('The current balance for @otherName is @balance.'),
  'info saved' => t('Your information has been saved.'),
  'not saved' => t('No change has been made to your information.'),
//  'payment accepted' => t('rCredits payment accepted. In exchange, us@amount will be refunded to the payer, from your account.'), // UNUSED
  'charge accepted' => t('Your charge to @who\'s account has been approved (it is no longer in dispute). No action is necessary.'),
  'gift successful' => t('Your contribution of @amount is successful. Thank you!'),
  'signup bonus' => t('-- and you got your signup bonus!'), // keep this short, cause it's an add-on
  'nothing done' => t('Nothing was done.'),
  'nothing undone' => t('Nothing undone.'),
  'event' => t('Account @fullName (@quid) is now @status.'),
  'approved' => t('Your account has been approved! You may now buy and sell through the rCredits system, including using your rCard for purchases at member businesses.'),
  'got funding' => t('You have received a @type of @amount from your rCredits community.'),
  'gift sent' => t('Your chosen gift of @amount to Common Good Finance has gone through. Your rebate is @rewardAmount.'),
  'tx summary' => t('@otherName @otherDid you @amount for "@purpose" on @created.'),
  'share gift' => t('Your chosen ongoing gift of @share% of your rewards has been initiated for this month.'),
  'account suspended' => t('Your account has been temporarily suspended @why'),
  'to protect data integrity.' => t('to protect data integrity.'),
  'account funded' => t('Funds have arrived in your rCredits account! You may now use your rCard for purchases.'),
  'transfer complete' => t('Your requested@auto transfer of @amount (into your rCredits account) has completed.'),
  'triggers new card' => t('<b>NOTE:</b> If you change your @field, a new rCard will be mailed to you and your account will be charged $@R_CARD_COST. '),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. First, what's your name?"),
  "what's your name really?" => t("Hmm. I don't know if I believe that. What's your name really?"),
  "what's your email?" => t("Welcome to rCredits, @fullName (account ID @quid). Last question: What's your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // confirmations
//  'confirm get r' => t('Trade us@request for r@request?'),
//  'confirm get usd' => t('Trade r@request for us@request?'),
  'confirm payment' => t('Pay @otherName @amount for goods and services?'),
  'confirm pay invoice' => t('Pay @otherName @amount for goods and services (invoice @created)?'),
  'confirm charge' => t('Charge @otherName @amount for goods and services?'),
  'confirm exchange' => t('Okay to @confirmAction @otherName @amount in rCredits (for CASH/LOAN/ETC.)?'),
  'confirm pay employees' => t('Pay @employeeCount employees @amount each (a total of @total)?'),
  'confirm undo' => t('Undo @created transfer of @amount @tofrom @otherName?'),
  'confirm funding' => t('@confirmAction @amount to @otherName?'),
  'please confirm' => t('To confirm, type: @nonce'),
  'verification code' => t('Your verification code for using rCredits online is: @nonce'), // used in rWeb
  'confirm tx action' => t('Okay to @txAction?'),
  
  // error messages
  'syntax' => t('Wrong syntax!'),
  'nyi' => t('That feature is not available through the SMS interface.'),
  'already started' => t('You are already an rCredits member. For help, type "helpme".'),
  'duplicate name' => t('That name is already taken, in your region. Try again. Use a middle initial?'),
  'name too long' => t('That name is too long. Try again (keep it under 60 characters).'),
  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (@rewards to date), but you can't cash them out. Your rCredits balance available to exchange for US Dollars is @cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your rCredits balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"@feature" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Try again (start over).'),
  'no self-trading' => t("You can't exchange credit with yourself."),
  'no self-relation' => t("You can't relate yourself to your own account."),
  'funding error' => t("You do not have permission to fund that person."),
//  'no company relations' => t("Only individuals can be related to this account."),
// not used  'no self-undo' => t("You can't undo transactions with yourself."),
  'no matching invoice' => t('There is no such invoice to be paid. Type "helpme pay".'),
  'undo no match' => t("You have no matching transactions to undo."),
  'undo moving target' => t("Can't undo: the last matching transaction has changed."),
  'duplicate email' => t('There is already an account with that email address. Visit rCredits.org to add this cell number to your account.'),
  'unknown member' => t('Unknown member: "@who" (they need to sign up before we can recognize them). Type "helpme pay".'),
  'not an account' => t('That account does not exist.'), // this is usually a system error (or REST error)
  'non member' => t('@otherName does not have an active rCredits account.'),
  'not yet active' => t('@name\'s account is not yet activated.'),
  'ambiguous other' => t('"@who" is associated with more than one rCredits account. You will need to identify them differently. Type "more" for options.'),
  'ambiguous other2' => t('You can identify a member by phone, email, fullname (no punctuation or spaces), or account ID.'),
  'what relationship' => t('You forgot to say whether you are an employee, owner, or both.'),
  'short put' => t('The most you can transfer to your bank at this time is @max.'),
  'short cash to' => t('You are @short short, for that exchange.'),
  'short cash from' => t('@otherName does not have enough rCredits for that exchange.'),
  'short to' => t('You are @short short, for that payment. Increase your minimum!'),
  'short from' => t('@otherName does not have enough funds for this request.'),
  'min too low' => t('Your minimum is set too low, causing conflicting deposits and withdrawals from your bank account. To increase your minimum to @suggestion, visit the @page page (under Settings).'),
  'you were short' => t('You tried to make a purchase but did not have enough funds in your account. A higher "minimum" setting might keep this from happening again. To increase your minimum to the recommended setting of @suggested, <@a1>click here</a>.'),
  'click option' => t('OR <a href="@url">click here</a>'),
//  'shortr to' => t('You do not have enough rCredits, for that transaction. Type "get r <amount>" to get more rCredits.'),
//  'shortr from' => t('@otherName does not have enough rCredits for that transaction at this time.'),
//  'demand too small' => t('NOTE: The minimum amount of rCredits you can actually buy at a time is $@min.', array('@min' => R_MIN_DEMAND)),
  'no access get' => t('You do not have permission to exchange rCredits for US Dollars or vice-versa.'),
//  'no access create' => t('You do not have permission to create transactions.'),
  'no sell' => t('You do not have permission to make sales for this account.'),
  'no buy' => t('You do not have permission to make purchases for this account.'),
  'other no sell' => t('@otherName does not have permission to make sales for that account.'),
  'other no buy' => t('@otherName does not have permission to make purchases for that account.'),
  'bad amount' => t('The amount must be a number.'),
  'bad date' => t('That is not a valid date.'),
  'nonpositive amount' => t('The amount must be a positive number.'),
  'negative amount' => t('The amount cannot be negative.'),
//  'fractional cents' => t('The amount must be a whole number of cents.'),
  'too fractional' => t('You have too many digits after the decimal point.'),
  'amount too big' => t('Transactions larger than $@max are not permitted at this time.', array('@max' => R_MAX_AMOUNT)),
  'already undone' => t('Transaction is already undone.'),
  'no such tx' => t('That transaction does not exist.'),
  'bank unverified' => t('You cannot exchange rCredits for US Dollars until your bank account has been verified.'),
  'no ALL' => t('"ALL" (meaning all employees) is not allowed here.'),
  'no employees' => t('There are no employees related to this account.'),
  'shortname taken' => t('There is already an account with a nearly identical name ("@otherName").'),
  'cannot contact' => t('We are unable to contact @fullName by email or text message. You may want follow up directly.'),
  'no usd account' => t('To exchange rCredits for US Dollars (or vice versa), you must first connect your rCredits account to Dwolla (online).'),
  'no last customer' => t('The system does not know who your last customer was.'),
  'max daily usd' => t('The amount you can transfer or exchange in one day is limited to $@R_MAX_DAILY_EXCHANGE'),
  'beyond today' => t('The date cannot be any later than today.'),
  'must trust' => t('Do not invite someone unless you trust their integrity.'),
  'payment exchange error' => t('An unknown technical problem prevented your payment exchange @way @other.'),
  'no buyers' => t('Uh oh. Nobody wants to buy rCredits. Needed: $@amount.'),
  'usd account taken' => t('That Dwolla account number is already used by rCredits account #@other.'),
  'bad phone code' => t('That is not the correct verification code.'),

  'cannot offload' => t('A temporary technical problem prevented some of your excess rCredits from being exchanged for US Dollars. The system will try again tomorrow.'),
  'usd failed' => t('A temporary technical problem prevented the transfer of US Dollars. No transaction took place.'),
  'cannot bank' => t('The rCredits system was unable to @action your bank: @error'),
  'no source' => t('there is no valid funding source'),
  'not verified' => t('you have not yet verified the connection to your bank account. Check your bank statement online or call your bank (2-5 days after giving your routing and account numbers). There will be two small deposits made to your account, each less than @DW_VERIFY_ROOF cents. Go back to rCredits.org, click "Members\' Site", sign in, then follow the instructions there to supply those two deposit amounts.'),
  'agreement required' => t('You must agree to Dwolla\'s terms of service or you cannot participate in the rCredits system.'),
  'year is 4 digits' => t('Be sure to type all four digits of the year.'),
  'date too early' => t('That date is too early.'),
  'date too late' => t('That date is too late.'),

  'reward' => t('Your @rewardType is @rewardAmount.'),
  'reward other' => t('Your @otherRewardType is @otherRewardAmount.'),
  'new charge' => t('<b>Charge:</b> @otherName charged you @amount for "@payerPurpose".'),
  'new refund' => t('<b>Refund:</b> @otherName refunded you @amount for "@payerPurpose".'),
  'new payment' => t('<b>Payment:</b> @otherName paid you @amount for "@payeePurpose".'),
  'new invoice' => t('<b>Invoice:</b> @otherName invoiced you @amount for "@payerPurpose". To pay (or deny) this invoice, visit your Transactions page.'),
  'invoice denied' => t('<p><b>Invoice DENIED:</b> Your customer @otherName has declined to pay the following invoice:</p>
  <blockquote>@created @amount for "@payerPurpose"</blockquote>
  <p>You may want to contact the customer directly, to resolve any dispute.</p>'),
  'charge disputed' => t('<p><b>Charge DISPUTED:</b> Your customer @otherName disputes the following charge:</p>
  <blockquote>@created @amount for "@payeePurpose"</blockquote>
  <p>You may want to contact the customer directly, to resolve this dispute.</p>'),
  'invoice canceled' => t('<p><b>Invoice CANCELED:</b> Your invoice from @otherName has been canceled:</p>
  <blockquote>@created @amount for "@payerPurpose"</blockquote>
  <p>This invoice will no longer appear in your list of transactions.'),
  'offer refused' => t('@otherName has declined to accept your @created offer of @amount for "@payerPurpose".'),
  'offer canceled' => t('NYI'),
  'minmax status' => t('Your balance is @status.'),
  'banked' => t('A request has been made to @action your bank a total of @amount. This request may take up to 5 days to go through.'),
  'payment exchange received' => t('You received from @fullName a payment exchange of @amount -- that is, you traded that much US Dollars for rCredits, receiving a bonus of @bonus on the exchange.'),
  'payment exchanges offered' => t('You made payment exchanges to @offers @whom, totaling @total.'),
  'card ordered' => t('A Company Agent rCard will be US mailed to @name.'),
  'balances' => t('Your balance available to spend is @toSpend. You can move up to @toCash today to your bank account.'),
  'you sold out' => t('This week you traded @total worth of rCredits for US Dollars, missing an opportunity for @rebates worth of rebates. To keep this from happening so much, <@a1>click here</a> to change your settings to the recommended minimum balance of @suggested.'),
  'restricted' => t('You are not authorized to use this device.'),
  'cannot create account' => t('An error is preventing your account from being created.'),
  'wrong pin' => t('That is not your PIN.'),
  'wrong answer' => t('That is the wrong answer.'),
  'bad url' => t('That is not a valid URL.'),
  'no charging' => t('Invoicing another member is not permitted at this time.'),
  
  // notices to staff
  'cache mismatch' => 'Cron cache mismatch: @key=@is (cache) <> @shouldBe (calc)',
  'not enough usd' => 'Not enough usd (@usd) to send @need.',
);

  // help messages (for use with the help command)
$GLOBALS['SMS help'] = array(
  'help payment' => t('To pay Jane Dough $30 for pie, type "30 to janedough for pie" (no spaces in janedough). Type "more" for options.'),
  'help payment2' => t('You can also identify the recipient by phone, email, or account ID (with or without the first three letters). Type "more" for more details.'),
  'help payment22' => t('If you are just exchanging rCredits for cash, leave out the description. For example, type "43.12 to jdough@example.com".'),
  'help charge' => t('To charge Food Corp $90 for labor, type "90 from foodcorp for labor" (no spaces in foodcorp). Type "more" for options.'),
  'help charge2' => t('You can also identify the payer by phone, email, or account ID (with or without the first three letters). Type "more" for more details.'),
  'help charge22' => t('If you are just exchanging cash for rCredits, leave out the description. For example, type "43.12 from 413-772-6666".'),
  'help undo' => t('To undo your most recent transaction, type "undo". Type "more" for options to undo your most recent transaction of a certain type.'),
  'help undo2' => t('Examples: undo | undo janedough | undo to | undo from | undo to janedough | undo from janedough — Type "more" for details.'),
  'help undo22' => t('"undo janedough" undoes your most recent transaction with Jane Dough. You can also identify her by phone, email, or account ID.'),
//  'help get' => t('To get $200 more in rCredits, type "get r 200". To send $50 in rCredits to your US bank account, type "get usd 50".'),
//  'help get' => t('To get $200 more in rCredits, type "get r 200".'),
  'help information' => t('Just type "info" (tells you your balance and the current demand for rCredits)'),
  'help helpme' => t('Options: pay | charge | undo | info | helpme. For details, type "helpme pay" and so forth.'),
);

$GLOBALS[TX_SMS . ' messages'] = array_merge($GLOBALS[TX_SMS . ' messages'], $GLOBALS['SMS help']); 
$GLOBALS[TX_SMS . ' messages']['help helpme no-error'] = t('Try something! ') . $GLOBALS['SMS help']['help helpme'];
foreach (array('more', 'stop', 'start') as $one) $GLOBALS[TX_SMS . ' messages']["help $one"] = $GLOBALS['SMS help']['help helpme'];

$GLOBALS['SMS commands'] = array( // these can be abbreviated by the caller
//  'get' => array('get %rusd %amount', 'get %amount %rusd'), // best before pay, so that "g r 10" is interpreted as "get r" not "give who"
  'be' => array('(?:as|be) %who'),
  'payment' => array(
    '(?:payment|okay|approve)',
    '(?:payment|okay|approve)(?: %who)',
    '(?:(?:payment|give|send) )?%amount to %who(?: for %what)?', 
    '(?:to|payment|give|send) %who %amount(?: for %what)?',
  ),
  'charge' => array(
    '(?:(?:charge|invoice|take|chg) )?%amount from %who(?: for %what)?', 
    '(?:from|charge|invoice|chg) %who %amount(?: for %what)?',
  ),
  'undo' => array(
    '(?:undo|reverse|cancelit|contest|refuse|reject|deny|dispute)', // can't use CANCEL because Twilio reserves it
    '(?:undo|reverse|cancelit|contest|refuse|reject|deny|dispute) %tofrom',
    '(?:undo|reverse|cancelit|contest|refuse|reject|deny|dispute) %who',
    '(?:undo|reverse|cancelit|contest|refuse|reject|deny|dispute) %tofrom %who',
  ),
  'information' => array('information', 'balance'),
  // name
  // lost (only need this for phone)
  'helpme' => '(?:help me|helpme)?(?: %feature)?',
  'more' => 'more', // continuation of help
  'stop' => 'stop|unsubscribe|cancel|quit', // passed from SMS Gateway
  'start' => 'start|yes', // passed from SMS Gateway
);
$GLOBALS['SMS gateway words'] = u\ray('stop unsubscribe cancel quit start yes help'); // "help" never reaches us
$GLOBALS['SMS not a nonce'] = array_merge(
  u\ray('char fro'), 
  array_keys($GLOBALS['SMS commands']),
  $GLOBALS['SMS gateway words']
); 
$GLOBALS['SMS reserved words'] = array_merge(
  array_keys($GLOBALS['SMS commands']),
  u\ray('okay approve reverse contest refuse reject deny to from balance give send invoice take as usd'), 
  $GLOBALS['SMS gateway words']
); 
$GLOBALS['SMS command tokens'] = array(
  '%tofrom' => '(to|from)',
  '%amount' => '([0-9]+(?:.[0-9][0-9]?)?)',
  '%who' => '([^ ]+)',
  '%what' =>  '(.+)',
  '%rusd' => '(r)', // was (r|usd)
  '%currency' => '([a-z]+)',
  '%feature' => '([^ ]+)',
);

/**
 * Send an SMS message to someone.
 */
function send($number, $message) {
  global $sms_devel;
  $number = u\fmtPhone($number, '+n');
  $msg = substr($message, 0, 160-1); // minus one because Twilio splits messages 160 chars long (and may discard the first part)
  if ($msg != $message) $message .= ' — TOO LONG! (strlen ' . strlen($message) . ')';
  if (@$sms_devel) drupal_set_message("To $number: $message"); else \sms_send($number, $msg);
  $nonce = infer_nonce($message);
  u\loga('out', compact(u\ray('number message')), $nonce, TX_SMS); // "via chTX_SMS" in case called from another interface
}

/**
 * Return the nonce (if any) from within the given message.
 */
function infer_nonce($message) {
  $pattern = tt('please confirm', array('nonce' => '([A-Z]*)'));
  preg_match("`$pattern`", $message, $match);
  return @$match[1];
}

/**
 * Add a cell phone to the given account.
 * for now, with everything through SMS, we want to be able to send to anyone
 * @param string $number: phone number
 * @param int $uid: account record id
 * @param int $status: cell phone status (DEFAULTS to active)
 * @return bool success
 */
function create($number, $uid, $status = SMS_PRIMARY) {
  u\EXPECT(compact(u\ray('number uid status')), 'string&notempty id int');
  $number = u\fmtPhone($number, '+n');
///  debug("creating SMS record for number $code, uid $uid");
  return r\makeDevice($uid, $number, TX_SMS);
}

/**
 * Add the cell owner's full name to the account.
 * @return the full name
 *//*
function setup_name($acct, $fullName) {
  $fullName = str_replace('  ', ' ', trim($fullName));
  if (strlen($fullName) > 60) throw new rSMSexception(tt('name too long'));
  if (!u\validName($fullName)) throw new rSMSexception(tt("what's your name really?"));
  $fullName = u\normalizeCase($fullName);
  $name = r\uniqueName($fullName);
//  if (r\userField(1, 'name=:name', compact('name'))) throw new rSMSexception(tt('duplicate name'));
  $acct->update(compact('name', 'fullName'));
  return $fullName;
}
*/

/**
 * Add the caller's email to the account.
 * @return object: the account
 *//*
function setup_email($acct, $email) {
  if (!\valid_email_address($email)) throw new rSMSexception(tt("what's your email really?"));
  if (r\userField(1, 'mail=:email', compact('email'))) throw new rSMSexception(tt('duplicate email'));
  $mail = $init = $email;
  return $acct->update(compact('mail', 'init'));
}*/

/**
 * Return a pattern that includes all abbreviations of the word.
 */
function abbreviator($word) {
  $result = substr($word, 0, 1);
  for ($i = 2; $i <= strlen($word); $i++) {
    $result .= '|' . substr($word, 0, $i);
  }
  return "(?:$result)";
}

/**
 * Return the command if it was intended by the given message.
 * Otherwise return the standing intent.
 *
 * @parm string $intent: the standing intent (FALSE if none)
 * @parm string $command: the command to test against the message
 * @parm string $pattern: the command pattern(s) for the given command
 * @parm string $message: the message to interpret
 */
function intent($intent, $command, $pattern, $message) {
  if ($command == 'payment' | $command == 'charge') {
    $abbrev = abbreviator($command == 'charge' ? 'from' : 'to');
    if (preg_match("/(^| )$abbrev( |$)/i", $message)) return $command;
  }
  $short_pattern = preg_replace('~\(\?\:|\)~', '', substr($pattern, 0, strpos("$pattern ", ' '))); // isolate command words
  return preg_match("/^($short_pattern)( |$)/i", $message, $matches) ? $command : $intent;
}

/**
 * Return a message.
 * Call by:
 *   tt($array); OR
 *   tt($message); OR tt($message, $subs);
 * @param array $array: array($message, $subs)
 * @param string $message: index to message to display
 * @param array $subs: associative array of substitutions (@ will be prepended to each key)
 * return string: the message after any substitutions
 */
function tt($message, $subs = array()) {return u\tt($message, $subs, TX_SMS);} // TX_SMS in case called from another interface

// one-line functions that need no explanation
function uid($number) {return (int) db\lookup('uid', 'r_boxes', 'code=:number', compact('number'));}
