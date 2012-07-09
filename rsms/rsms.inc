<?php
namespace rCredits\SMS;
use \rCredits as r;
/**
 * @file
 * rSMS include file
 *
 * Utilities and definitions for the rSMS module
 * (anything that doesn't have to be in rsms.module and isn't appropriate to class call).
 */

define('RSMS_PREFIX', '[TRIAL] '); // the pre-launch trial
//define('RSMS_PREFIX_INACTIVE', '[TRIAL RUN] '); // pretend transactions by not-yet-active players (treat it as a trial run)
//define('RSMS_PREFIX_ASIF', 'PRETEND - '); // pretend transactions by Active Participants
define('RSMS_TEMPORARY_ACCOUNT_PREFIX', 'phone'); // user name is temporarily set to, for example, phone2345678901

$GLOBALS['SMS messages'] = array(
  // reports
  'account info' => t("Your balance for account @id is @balance, including @rewards of incentive rewards. Total demand for rCredits is @total_demand."),
  'your account is ready' => t('Your starting balance is @balance. Pretend to buy and sell as you would in a typical month (we need that information). Have fun! Type "how" to get started.'),
//  'your account is ready' => t("Thank you! Your practice balance is @balance."),
  'get usd' => t("Exchange approved. @amount will be transferred directly to your bank account within 24 hours. Your new balance is @balance."),
//  'get usd inactive' => t("If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant."),
  'report transaction' => t("You @type @other @amount (@reward_type: @reward_amount). Your new balance is @balance. Transaction #@tid"),
  'report short transaction' => t("You @type @other @amount (@short remains to be paid). Your new balance is @balance. Transaction #@tid"),
//  'transaction done asif' => t("@type: @amount @tofrom user#@other (@reward_type: @reward_amount). Your new provisional balance is @balance (If this had been a real transaction, your balance would be @amount @moreless). Transaction ID #@xid"),
  'report exchange' => t("You @type @other @amount @way. Your new balance is @balance. Transaction #@tid"),
  'your demand' => t("Your total request for rCredits is now @amount."),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. What's your name?"),
  "what's your name really?" => t("Hmm. I don't know if I believe that. What's your name really?"),
  "what's your email?" => t("Welcome to rCredits, @name (account id @id). Last question: What's your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // confirmations
  'confirm get usd' => t("Give r@amount in exchange for us@amount?"),
  'confirm payment' => t("Pay @other @amount for goods and services?"),
  'confirm charge' => t("Charge @other @amount for goods and services?"),
  'confirm credit for cash' => t("Give @other r@amount for cash?"),
  'confirm cash for credit' => t("Give @other @amount cash for rCredits?"),
  'please confirm' => t("To confirm, type @nonce."),
  
  // how messages
  'how to payment' => t("Payment examples: 12.34 to 413.772.0000 | .cornerstore | wgabcd | jd@example.com for labor (describe any real goods & services)"),
  'how to charge' => t("Charge examples: 12.34 from 413.772.0000 | wgabcd | jdough@example.com for labor (describe any real goods & services)"),
  'how to undo' => t("Examples: undo | undo to | undo 413.772.0000 | undo from wgabcd"),
  'how to get' => t("Examples: get r 12.34 | get usd 12.34"),
  'how to information' => t("Example: information (tells you your balance and the current demand for rCredits)"),
  'how to how' => t('Examples: @commands. For example, type "how undo".'),
  
  // error messages
  'syntax' => t('Syntax error.'),
  'duplicate name' => t('That name is already taken, in your region. Try again. Use a middle initial?'),
  'name too long' => t('That name is too long. Try again (keep it under 60 characters).'),
  //  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (@rewards to date), but you can't cash them out. Your balance available to exchange for US Dollars is @cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"@param" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Try again (start over).'),
  'no self-trading' => t("You can't exchange rCredits with yourself."),
  'duplicate email' => t("There is already an account with that email address. Visit rCredits.org to add this cell number to your account."),
  'unknown user' => t('Unknown user: "@user". Type "how pay".'),
  'short cash to' => t('You are r@short short, for that exchange.'),
  'short cash from' => t('@other does not have enough rCredits for that exchange.'),
);

$GLOBALS['SMS messages']['how to how no-error'] = t('Try something! ') . $GLOBALS['SMS messages']['how to how'];

$GLOBALS['SMS commands'] = array( // these can be abbreviated by the caller
  'get' => array('get %rusd %amount', 'get %amount %rusd'), // best before pay, so that "g r 10" is interpreted as "get r" not "give who"
  'payment' => array(
    '(?:(?:payment|give|send) )?%amount to %who(?: for %what)?', 
    '(?:to|payment|give|send) %who %amount(?: for %what)?',
  ),
  'charge' => array(
    '(?:(?:charge|invoice|take|chg)? )?%amount from %who(?: for %what)?', 
    '(?:from|charge|invoice|chg) %who %amount(?: for %what)?',
  ),
  'undo' => array(
    '(?:undo|reverse|canc)', // can't use CANCEL because Twilio reserves it
    '(?:undo|reverse|canc) %tofrom',
    '(?:undo|reverse|canc) %who',
    '(?:undo|reverse|canc) %tofrom %who',
  ),
  'information' => array('information', 'balance'),
  'how' => 'how(?: %feature)?',
  'stop' => 'stop|unsubscribe|cancel|quit', // passed from SMS Gateway
  'start' => 'start|yes', // passed from SMS Gateway
);
$GLOBALS['SMS Gateway words'] = explode(',', 'stop,unsubscribe,cancel,quit,start,yes,help'); // "help" never reaches us
$GLOBALS['SMS reserved words'] = explode(',', 'to,from,usd,balance,name,lost,be,give,send,invoice,take');
$GLOBALS['SMS reserved words'] = array_merge(array_keys($GLOBALS['SMS commands']), $GLOBALS['SMS reserved words']); 

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
 * Handle a user's request to receive no SMS messages from us (already handled by the gateway).
 * Only two gateway commands get passed through to us: start and stop (help does not get passed through).
 */
function do_stop() {
  user_role_revoke_permissions(DRUPAL_AUTHENTICATED_RID, array('receive sms'));
}

/**
 * Handle a user's request to (again) accept SMS messages from us (already handled by the gateway).
 */
function do_start() {
  user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, array('receive sms'));
}

/**
 * Get the long version of the indicated message(s), make any substitutions,
 * and return the result.
 *
 * @param string $message
 *   name of the message (index into $GLOBALS['SMS messages'])
 *   or a list of such message indices, separated by "|"
 *
 * @param array $substitutions (optional). '@' will be prepended to the keys.
 */   
function tt($message, $substitutions = array()) {
  $subs = array();
  foreach ($substitutions as $key => $value) $subs["@$key"] = $value;
  $messages = explode('|', $message);
  $result = array();
  foreach ($messages as $message) {
    $result[] = t($GLOBALS['SMS messages'][$message], $subs);
  }
  return join(' ', $result);
}

/**
 * Send an SMS message to someone.
 */
function send($number, $message0) {
  $message = substr($message0, 0, 160);
  if ($message != $message0) $message0 .= ' -- TOO LONG! (strlen ' . strlen($message0) . ')';
  if ($message != $message0) die ($message0); // debug
  \sms_send($number, $message);
  r\r_log('SMSout', $message0, $number, infer_nonce($message0));
}

/**
 * Return the nonce (if any) from within the given message.
 */
function infer_nonce($message) {
  $pattern = tt('please confirm', array('nonce' => '([A-Z]*?)'));
  preg_match("`$pattern`", $message, $match);
  return isset($match[1]) ? $match[1] : '';
}

/**
 * Remember what to do, once the user confirms.
 *
 * @parm string $number: the user's cell number
 * @parm string $todo: what to do once the user responds with the expected nonce (defaults to nothing)
 * @parm string $nonce: what to store as the nonce (default to 'whatever', meaning pick one at random)
 *
 * @return object(nonce, todo) if $todo is null, else the nonce
 */
function todo($number, $todo = 'nothing', $nonce = 'whatever') {
  if ($todo == 'nothing') {
    return (object) \db_query("SELECT nonce, todo FROM sms_user WHERE number = $number")->fetchAssoc();
  } else {
    $todo = addslashes($todo);
    $nonce = $nonce == 'whatever' ? nonce() : $nonce;
// FAILS because of {}    \db_query("UPDATE sms_user SET nonce = '$nonce', todo = '$todo' WHERE number = $number");
    \db_update('sms_user')->fields(compact('nonce', 'todo'))->condition('number', $number, '=')->execute();
    return $nonce;
  }
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 */
function nonce() {
  $offset = r\db_lookup('FLOOR(RAND() * COUNT(*))', 'r_nonces');
  $nonce = r\db_lookup('nonce', 'r_nonces', "1 LIMIT $offset, 1");

  if (strpos('  ' . join(' ', $GLOBALS['SMS reserved words']), ' ' . $nonce)) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long
  return $nonce;
}

/**
 * Say whether the given account name is a temporary SMS account name.
 */
function is_temporary_name($name) {
  $len = strlen(RSMS_TEMPORARY_ACCOUNT_PREFIX);
  return (substr($name, 0, $len) == RSMS_TEMPORARY_ACCOUNT_PREFIX and is_numeric(substr($name, $len)));
}

/**
 * Create a temporary account for the given cell phone number.
 */
function create_account($number) {
  try {
    $tx = \db_transaction();
    $credit_id = r\make_credit_id($number);
    $who = r\create_user(compact('number', 'credit_id'))->uid;
    todo($number, 'first call', '');
    return $who;
  } catch (\Exception $e) {
    $tx->rollback();
    throw new \Exception(tt($e->getMessage()));
  }
}

/**
 * Add the cell owner's full name to the account.
 * @return object: the account
 */
function setup_name($account, $name) {
  $name = str_replace('  ', ' ', trim($name));
  if (strlen($name) > 60) throw new \Exception(tt('name too long'));
  if (strtolower($name) == $name or strtoupper($name) == $name) $name = ucwords($name);
  if (!r\valid_name($name)) throw new \Exception(tt("what's your name really?"));
  if (r\db_lookup(1, 'users', "UPPER(name) = UPPER('$name')")) throw new \Exception(tt('duplicate name'));
  $edit = compact('name');
  return \user_save($account, $edit);
}

/**
 * Add the caller's email to the account.
 * @return object: the account
 */
function setup_email($account, $email) {
  if (!\valid_email_address($email)) throw new \Exception(tt("what's your email really?"));
  if (r\db_lookup(1, 'users', "mail = '$email'")) throw new \Exception(tt('duplicate email'));
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
 * Say whether the transaction description implies a cash transaction.
 */
function for_cash($for) {
  return ($for == '' or substr(strtolower($for), 0, 4) == 'cash');
}

function intent($intent, $command, $pattern, $message) {
  if ($command == 'payment' | $command == 'charge') {
    $abbrev = abbreviator($command == 'charge' ? 'from' : 'to');
    return preg_match("/ $abbrev /i", $message) ? $command : $intent;
  }
  $short_pattern = str_replace(')(', ')', '/^' . substr($pattern, 0, strpos("$pattern ", ' ')) . ' /i'); // )( happens in how command pattern
  return preg_match($short_pattern, $message, $matches) ? $command : $intent;
}

/**
 * Figure out who the caller meant.
 *
 * @parm string $val: how the user referred to the trader
 * @return: the trader's uid (NULL if not found)
 */
function identify($val) {
  $val = strtolower(str_replace("'", '', $val));
  if (is_numeric($val) and strlen($val) == 10) $val = '+1' . $val;
  if (substr($val, 0, 4) == 'www.') $val = substr($val, 4);
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {sms_user} s ON u.uid = s.uid 
          WHERE '$val' IN (LCASE(CONCAT('.', REPLACE(u.name, ' ', ''))), u.credit_id, s.number, LCASE(u.website), u.phone, LCASE(u.mail))";
  $result = db_query($sql)->fetchCol();
  return isset($result[0]) ? $result[0] : '';
}
