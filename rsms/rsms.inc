<?php
namespace rCredits\SMS;
use rCredits as r;
use rCredits\API as api;
use rCredits\Util as u;
require_once __DIR__ . '/../rcredits-api.inc'; // Drupal bug (needs this explicit)
require_once __DIR__ . '/../rcredits-util.inc';

/**
 * @file
 * rSMS include file
 *
 * Utilities and definitions for the rSMS module
 * (anything that doesn't have to be in rsms.module and isn't appropriate to class call).
 *
 if ($account->sms_user[0]['status'] == SMS_USER_CONFIRMED) {
 sms_send($account->sms_user[0]['number'], $message, $account->sms_user[0]['gateway']);
 
       if ($account = sms_user_authenticate($number)) {
        $metadata = array(
          'login' => !sms_devel_gateway(),
          'number' => $number,
          'message' => $message,
          'options' => $options,
        );
        sms_user_login_metadata($account->uid, $metadata);
        $edit = array();
        if (!sms_devel_gateway()) user_module_invoke('login', $edit, $account); // ws: avoid login when using sms devel
      } elseif (variable_get('sms_user_registration_enabled', 0) && $number) {
        if ($account = sms_user_register_new_user($number, $message, $options)) {
          // Send the new user the registration message if one exists.
          if ($message = variable_get('sms_user_new_account_message', '')) {
            sms_user_send($account->uid, $message); // just calls sms_send
          }
          
              watchdog('sms', '%name was authenticated using SMS.', array('%name' => $user->name));

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
//define('RSMS_PREFIX_ASIF', 'AS IF: '); // pretend transactions by Active Participants

$GLOBALS[TX_SMS . ' messages'] = array(
  // reports
  'account info' => t("Your@asif balance for account @quid is @balance, including @rewards of incentive rewards. Total demand for rCredits is @total_demand."),
//  'account info2' => t("More account info"), // not used yet
  'your account is ready' => t('Your starting balance is @balance. Pretend to buy and sell as you would in a typical month (we need that info). Have fun! Type "helpme" to get started.'), // separate out the last sentence

  // reporting
  'report transaction' => t("You@asif @action @other_name @amount (@reward_type: @reward_amount). Your new balance is @balance. Transaction #@tid"),
  'report invoice' => t("You@asif @action @other_name @amount. Your balance is unchanged, pending payment. Invoice #@tid"),
  'report short transaction' => t("SPLIT! You@asif @action @other_name @amount (@short remains to be paid). Your new balance is @balance. Transaction #@tid"),
//  'transaction done asif' => t("@action: @amount @tofrom user#@other_name (@reward_type: @reward_amount). Your new provisional balance is @balance (If this had been a real transaction, your balance would be @amount @moreless). Transaction ID #@tid"),
  'report exchange' => t("You@asif @action @other_name r@amount for cash/loan/gift/etc. Your new balance is @balance. Transaction #@tid"),
  'report exchange request' => t("You@asif requested r@amount from @other_name. Your balance is unchanged, pending approval. Invoice transaction #@tid"),
  'paid employees' => t('You distributed @total to @successes of @employee_count employees. See your transactions page for details.'),
  'new-payment' => t('You have received an@asif offer of @amount from @other_name for @purpose. Type "accept" or "undo".'),
  'report undo' => t("@asif Transaction has been @solution."),
  'report get usd' => t("@Exchange approved. @request will be transferred directly to your bank account within 24 hours. Your new balance is @balance."),
  'get usd inactive' => t("If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant."),
  'your demand' => t("Your total request for rCredits is now @request."),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. First, what's your name?"),
  "what's your name really?" => t("Hmm. I don't know if I believe that. What's your name really?"),
  "what's your email?" => t("Welcome to rCredits, @full_name (account ID @quid). Last question: What's your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // confirmations
  'confirm get usd' => t("@asif Trade r@request for us@request?"),
  'confirm payment' => t("@asif Pay @other_name @amount for goods and services?"),
  'confirm pay invoice' => t("@asif Pay @other_name @amount for goods and services (invoice @created)?"),
  'confirm charge' => t("@asif Charge @other_name @amount for goods and services?"),
  'confirm exchange' => t("Okay to@asif @confirm_action @other_name @amount in rCredits (NOT for goods or services)?"),
  'confirm pay employees' => t('Pay @employee_count employees @amount each (a total of @total)?'),
  'confirm undo' => t("Undo @created@asif transfer of @amount @tofrom @other_name?"),
  'please confirm' => t("To confirm, type @nonce."),
  'verification code' => 'Your verification code for using rCredits online is: @nonce', // used in rWeb
  
  // error messages
  'syntax' => t('Wrong syntax!'),
  'nyi' => t('That feature is not yet available.'),
  'already started' => t('You are already an rCredits member. For help, type "help me".'),
  'duplicate name' => t('That name is already taken, in your region. Try again. Use a middle initial?'),
  'name too long' => t('That name is too long. Try again (keep it under 60 characters).'),
  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (@rewards to date), but you can't cash them out. Your balance available to exchange for US Dollars is @cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  "can't pay asif invoice" => t('You cannot pay an "As-If" invoice, once both parties are rTraders.'),
  'not a feature' => t('"@feature" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Try again (start over).'),
  'no self-trading' => t("You can't exchange rCredits with yourself."),
  'no self-relation' => t("You can't relate yourself to your own account."),
  'no company relations' => t("Only individuals can be related to this account."),
// not used  'no self-undo' => t("You can't undo transactions with yourself."),
  'no matching invoice' => t("There is no such invoice to be paid."),
  'undo no match' => t("You have no matching transactions to undo."),
  'undo moving target' => t("Can't undo: the last matching transaction has changed."),
  'duplicate email' => t("There is already an account with that email address. Visit rCredits.org to add this cell number to your account."),
  'unknown member' => t('Unknown member: "@who" (they need to sign up before we can recognize them). Type "helpme pay".'),
  'not an account' => t('That account does not exist.'), // this is usually a system error (or REST error)
  'non trader' => t('@other_name is not an authorized rTrader.'),
  'ambiguous other' => t('"@who" is associated with more than one rCredits account. You will need to identify them differently. Type "more" for options.'),
  'ambiguous other2' => t('You can identify an rTrader by phone, email, fullname (no punctuation or spaces), or account ID.'),
  'what relationship' => t('You forgot to say whether you are an employee, owner, or both.'),
  'short cash to' => t('You are r@short short, for that exchange.'),
  'short cash from' => t('@other_name does not have enough rCredits for that exchange.'),
  'short payment' => t('You are r@short short, for that payment. Get more rCredits first, or pay less.'),
  'broke to' => t('You are completely out of rCredits, so that transaction cannot be completed. Type "get r <amount>" to get more rCredits.'),
  'broke from' => t('@other_name does not have enough rCredits for that transaction at this time. But your request has been noted.'),
  'demand too small' => t('NOTE: The minimum amount of rCredits you can actually buy at a time is $@min.', array('@min' => R_MIN_DEMAND)),
  'no access get' => t('You do not have permission to exchange rCredits for US Dollars or vice-versa.'),
//  'no access create' => t('You do not have permission to create transactions.'),
  'no sell' => t('You do not have permission to make sales for this account.'),
  'no buy and sell' => t('You do not have permission to make purchases for this account.'),
  'other no sell' => t('That person does not have permission to make sales for that account.'),
  'other no buy and sell' => t('That person does not have permission to make purchases for that account.'),
  'nonpositive transfer' => t('You may not transfer a negative amount.'),
  'amount too big' => t('Transactions larger than $@max are not permitted at this time.', array('@max' => R_MAX_AMOUNT)),
  'already undone' => t('Transaction is already undone.'),
  'no such tx' => t('That transaction does not exist.'),
  'bank unverified' => t('You cannot exchange rCredits for US Dollars until your bank account has been verified.'),
  'no ALL' => t('This action is not allowed for "ALL" (meaning all employees)'),
  'no employees' => t('There are no employees for this account.'),
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
  'help undo2' => t('Examples: undo | undo janedough | undo to | undo from | undo to janedough | undo from janedough -- Type "more" for details.'),
  'help undo22' => t('"undo janedough" undoes your most recent transaction with Jane Dough. You can also identify her by phone, email, or account ID.'),
  'help get' => t('To get $200 more in rCredits, type "get r 200". To send $50 in rCredits to your US bank account, type "get usd 50".'),
  'help information' => t('Just type "info" (tells you your balance and the current demand for rCredits)'),
  'help helpme' => t('Options: pay | charge | undo | info | get | helpme. For details, type "helpme pay" and so forth.'),
);

$GLOBALS[TX_SMS . ' messages'] = array_merge($GLOBALS[TX_SMS . ' messages'], $GLOBALS['SMS help']); 
$GLOBALS[TX_SMS . ' messages']['help helpme no-error'] = t('Try something! ') . $GLOBALS['SMS help']['help helpme'];
foreach (array('more', 'stop', 'start') as $one) $GLOBALS[TX_SMS . ' messages']["help $one"] = $GLOBALS['SMS help']['help helpme'];

$GLOBALS['SMS commands'] = array( // these can be abbreviated by the caller
  'get' => array('get %rusd %amount', 'get %amount %rusd'), // best before pay, so that "g r 10" is interpreted as "get r" not "give who"
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
  '%rusd' => '(r|usd)',
  '%currency' => '([a-z]+)',
  '%feature' => '([^ ]+)',
);

/**
 * Send an SMS message to someone.
 */
function send($number, $message) {
  global $sms_devel;
  $number = u\formatPhone($number, '+n');
  $msg = substr($message, 0, 160-1); // minus one because Twilio splits messages 160 chars long (and may discard the first part)
  if ($msg != $message) $message .= ' -- TOO LONG! (strlen ' . strlen($message) . ')';
  if (@$sms_devel) drupal_set_message("To $number: $message"); else \sms_send($number, $msg);
  $nonce = infer_nonce($message);
  u\log('out', compact(u\ray('number message')), $nonce);
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
  $number = u\formatPhone($number, '+n');
//  debug("creating SMS record for number $number, uid $uid");
  $info = compact(u\ray('number uid status'));
  return \drupal_write_record('r_sms', $info);
}

/**
 * Add the cell owner's full name to the account.
 * @return the full name
 */
function setup_name($acct, $full_name) {
  $full_name = str_replace('  ', ' ', trim($full_name));
  if (strlen($full_name) > 60) throw new rSMSexception(tt('name too long'));
  if (!u\validName($full_name)) throw new rSMSexception(tt("what's your name really?"));
  u\normalizeCase($full_name);
  $name = r\uniqueName($full_name);
//  if (r\userField(1, 'name=:name', compact('name'))) throw new rSMSexception(tt('duplicate name'));
  $acct->update(compact('name', 'full_name'));
  return $full_name;
}

/**
 * Add the caller's email to the account.
 * @return object: the account
 */
function setup_email($acct, $email) {
  if (!\valid_email_address($email)) throw new rSMSexception(tt("what's your email really?"));
  if (r\userField(1, 'mail=:email', compact('email'))) throw new rSMSexception(tt('duplicate email'));
  $mail = $init = $email;
  return $acct->update(compact('mail', 'init'));
}

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
function tt($message, $subs = array()) {return u\tt($message, $subs);}

// one-line functions that need no explanation
function uid($number) {return (int) r\dbLookup('uid', 'r_sms', 'number=:number', compact('number'));}
