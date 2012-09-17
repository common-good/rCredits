<?php
namespace rCredits\SMS; // typically abbreviated as "rSMS"
use \rCredits\API as api;
use \rCredits\Utilities as u;
require_once __DIR__ . '/../rcredits-api.inc'; // Drupal bug (needs this explicit)
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

define('RSMS_PREFIX', 'TEST: '); // the pre-launch trial
//define('RSMS_PREFIX_INACTIVE', '[TRIAL RUN] '); // pretend transactions by not-yet-active players (treat it as a trial run)
//define('RSMS_PREFIX_ASIF', 'AS IF: '); // pretend transactions by Active Participants

$GLOBALS['SMS messages'] = array(
  // reports
  'account info' => t("Your balance for account @qid is @balance, including @rewards of incentive rewards. Total demand for rCredits is @total_demand."),
//  'account info2' => t("More account info"), // not used yet
  'your account is ready' => t('Your starting balance is @balance. Pretend to buy and sell as you would in a typical month (we need that info). Have fun! Type "helpme" to get started.'), // separate out the last sentence

  // reporting
  'report transaction' => t("You @type @other @amount (@reward_type: @reward_amount). Your new balance is @balance. Transaction #@tid"),
  'report invoice' => t("You @type @other @amount. Your balance is unchanged, pending payment. Invoice #@tid"),
  'report short transaction' => t("You @type @other @amount (@short remains to be paid). Your new balance is @balance. Transaction #@tid"),
//  'transaction done asif' => t("@type: @amount @tofrom user#@other (@reward_type: @reward_amount). Your new provisional balance is @balance (If this had been a real transaction, your balance would be @amount @moreless). Transaction ID #@xid"),
  'report exchange' => t("You @type @other r@amount for cash/loan/gift/etc. Your new balance is @balance. Transaction #@tid"),
  'report exchange request' => t("You requested r@amount from @other. Your balance is unchanged, pending approval. Pending Transaction #@tid"),
//  'report undo' => t("You cancelled transfer of @amount from @payer to @payee on @date."),
  'report undo' => t("Transaction has been @action. "),
  'report get usd' => t("Exchange approved. @request will be transferred directly to your bank account within 24 hours. Your new balance is @balance."),
//  'get usd inactive' => t("If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant."),
  'your demand' => t("Your total request for rCredits is now @request."),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. First, what's your name?"),
  "what's your name really?" => t("Hmm. I don't know if I believe that. What's your name really?"),
  "what's your email?" => t("Welcome to rCredits, @name (account ID @qid). Last question: What's your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // confirmations
  'confirm get usd' => t("Trade r@request for us@request?"),
  'confirm payment' => t("Pay @other @amount for goods and services?"),
  'confirm invoice' => t("Pay @other @amount for goods and services (invoice @date)?"),
  'confirm charge' => t("Charge @other @amount for goods and services?"),
  'confirm exchange' => t("Okay to @type @other @amount in rCredits (not for goods or services)?"),
  'confirm undo' => t("Undo @date transfer of @amount @tofrom @other?"),
  'please confirm' => t("To confirm, type @nonce."),
  'verification code' => 'Your verification code for using rCredits online is: @nonce', // used in rWeb
  
  // error messages
  'syntax' => t('Wrong syntax!'),
  'duplicate name' => t('That name is already taken, in your region. Try again. Use a middle initial?'),
  'name too long' => t('That name is too long. Try again (keep it under 60 characters).'),
  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (@rewards to date), but you can't cash them out. Your balance available to exchange for US Dollars is @cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"@feature" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Try again (start over).'),
  'no self-trading' => t("You can't exchange rCredits with yourself."),
  'no self-sharing' => t("You can't share an account with itself."),
// not used  'no self-undo' => t("You can't undo transactions with yourself."),
  'no matching invoice' => t("There is no such invoice to be paid."),
  'undo no match' => t("You have no matching transactions to undo."),
  'undo moving target' => t("Can't undo: the last matching transaction has changed."),
  'duplicate email' => t("There is already an account with that email address. Visit rCredits.org to add this cell number to your account."),
  'unknown user' => t('Unknown rTrader: "@who" (they need to sign up before we can recognize them). Type "helpme pay".'),
  'non trader' => t('@other is not an authorized rTrader.'),
  'ambiguous other' => t('"@who" is associated with more than one rCredits account. You will need to identify them differently. Type "more" for options.'),
  'ambiguous other2' => t('You can identify an rTrader by phone, email, account ID, or .fullname (with a dot before the name, no spaces).'),
  'short cash to' => t('You are r@short short, for that exchange.'),
  'short cash from' => t('@other does not have enough rCredits for that exchange.'),
  'short invoice' => t('You are r@short short, to pay that invoice. Get more rCredits first, or undo the invoice and pay part in US Dollars.'),
  'broke to' => t('You are completely out of rCredits, so that transaction cannot be completed. Type "get r <amount>" to get more rCredits.'),
  'broke from' => t('@other does not have enough rCredits for that transaction at this time. But your request has been noted.'),
  'demand too small' => t('NOTE: The minimum amount of rCredits you can actually buy at a time is $@min.', array('@min' => R_MIN_DEMAND)),
  'no access get' => t('You do not have permission to exchange rCredits for US Dollars or vice-versa.'),
  'no access create' => t('You do not have permission to create transactions.'),
  'negative transfer' => t('You may not transfer a negative amount.'),
  'already undone' => t('Transaction is already undone.'),
  'no such tx' => t('That transaction does not exist.'),
  'bank unverified' => t('You cannot exchange rCredits for US Dollars until your bank account has been verified.'),
);

  // help messages (for use with the help command)
$GLOBALS['SMS help'] = array(
  'help payment' => t('To pay Jane Dough $30 for pie, type "30 to .janedough for pie" (with a dot before the name, no spaces). Type "more" for options.'),
  'help payment2' => t('You can also identify the recipient by phone, email, or account ID (withOUT a dot before any of these). Type "more" for more details.'),
  'help payment22' => t('If you are just exchanging rCredits for cash, leave out the description. For example, type "43.12 to jdough@example.com".'),
  'help charge' => t('To charge Food Corp $90 for labor, type "90 from .foodcorp for labor" (with a dot before the name, no spaces). Type "more" for options.'),
  'help charge2' => t('You can also identify the payer by phone, email, or account ID (withOUT a dot before any of these). Type "more" for more details.'),
  'help charge22' => t('If you are just exchanging cash for rCredits, leave out the description. For example, type "43.12 from 413-772-6666".'),
  'help undo' => t('To undo your most recent transaction, type "undo". Type "more" for options to undo your most recent transaction of a certain type.'),
  'help undo2' => t('Examples: undo | undo .janedough | undo to | undo from | undo to .janedough | undo from .janedough -- Type "more" for details.'),
  'help undo22' => t('"undo .janedough" undoes your most recent transaction with Jane Dough. You can also identify her by phone, email, or account ID.'),
  'help get' => t('To get $200 more in rCredits, type "get r 200". To send $50 in rCredits to your US bank account, type "get usd 50".'),
  'help information' => t('Just type "info" (tells you your balance and the current demand for rCredits)'),
  'help helpme' => t('Options: pay | charge | undo | info | get | helpme. For details, type "helpme pay" and so forth.'),
);

$GLOBALS['SMS messages'] = array_merge($GLOBALS['SMS messages'], $GLOBALS['SMS help']); 
$GLOBALS['SMS messages']['help helpme no-error'] = t('Try something! ') . $GLOBALS['SMS help']['help helpme'];
foreach (array('more', 'stop', 'start') as $one) $GLOBALS['SMS messages']["help $one"] = $GLOBALS['SMS help']['help helpme'];

$GLOBALS['SMS commands'] = array( // these can be abbreviated by the caller
  'get' => array('get %rusd %amount', 'get %amount %rusd'), // best before pay, so that "g r 10" is interpreted as "get r" not "give who"
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
  // be
  // lost
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
  $number = u\format_phone($number, '+n');
  $msg = substr($message, 0, 160-1); // minus one because Twilio splits messages 160 chars long (and may discard the first part)
  if ($msg != $message) $message .= ' -- TOO LONG! (strlen ' . strlen($message) . ')';
  if (sms_devel_gateway()) if ($msg != $message) debug($message); // debug
  \sms_send($number, $msg);
  $nonce = infer_nonce($message);
  u\log('rSMS', 'out', compact(u\ray('type message number nonce')));
}

/**
 * Return the nonce (if any) from within the given message.
 */
function infer_nonce($message) {
  $pattern = tt('please confirm', array('nonce' => '([A-Z]*?)'));
  preg_match("`$pattern`", $message, $match);
  return @$match[1];
}

/**
 * Add a cell phone to the given account.
 * for now, with everything through SMS, we want to be able to send to anyone
 */
function create($number, $uid, $status = SMS_USER_CONFIRMED) {
  $number = u\format_phone($number, '+n');
  \drupal_write_record('sms_user', compact(u\ray('number uid status')));
}

/**
 * Add the cell owner's full name to the account.
 * @return the full name
 */
function setup_name($account, $name) {
  $name = str_replace('  ', ' ', trim($name));
  if (strlen($name) > 60) throw new \Exception(tt('name too long'));
  if (strtolower($name) == $name or strtoupper($name) == $name) $name = ucwords($name);
  if (!u\valid_name($name)) throw new \Exception(tt("what's your name really?"));
  $name = $short_name = u\short_name($full_name = $name);
  if (r\user_field(1, "short_name='$short_name'")) throw new \Exception(tt('duplicate name'));
  $edit = compact('name', 'short_name', 'full_name');
  return \user_save($account, $edit)->full_name;
}

/**
 * Add the caller's email to the account.
 * @return object: the account
 */
function setup_email($account, $email) {
  if (!\valid_email_address($email)) throw new \Exception(tt("what's your email really?"));
  if (r\user_field(1, "mail = '$email'")) throw new \Exception(tt('duplicate email'));
  $edit = array(); // no edits yet
  $edit['mail'] = $edit['init'] = $email;
  return \user_save($account, $edit);
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

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs, 'SMS');}
