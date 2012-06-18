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

define('RSMS_PREFIX', '[TRIAL RUN] '); // the pre-launch trial
//define('RSMS_PREFIX_INACTIVE', '[TRIAL RUN] '); // pretend transactions by not-yet-active players (treat it as a trial run)
//define('RSMS_PREFIX_ASIF', 'PRETEND - '); // pretend transactions by Active Participants

$GLOBALS['SMS commands'] = ''; // defined in $this->parse()
$GLOBALS['SMS messages'] = array(
  // reports
  'account info' => t("Your balance is @balance, including @rewards of incentive rewards. The current demand for rCredits is @demand."),
  'your account is ready' => t('Thank you! Your Trial Run starting balance is @balance. Pretend to buy and sell, but be real. Do as you would expect to do in a typical month. That gives us the information we need, so that you can become an Active Participant. Type "help" to get started. And have fun.'),
//  'your account is ready' => t("Thank you! Your practice balance is @balance."),
  'get usd' => t("Exchange approved. @amount will be transferred directly to your bank account within 24 hours. Your new balance is @balance."),
//  'get usd inactive' => t("If you were an Active Participant in the rCredits system, your request would transfer @amount directly to your bank account. Alas, you are not yet an Active Participant."),
  'report transaction' => t("You @type @other @amount (@reward_type: @reward_amount). Your new balance is @balance. Transaction #@tid"),
//  'transaction done asif' => t("@type: @amount @tofrom user#@other (@reward_type: @reward_amount). Your new provisional balance is @balance (If this had been a real transaction, your balance would be @amount @moreless). Transaction ID #@xid"),
  'report exchange' => t("You @type @other @amount @way. Your new balance is @balance. Transaction #@tid"),
  'your demand' => t("Your total request for rCredits is now @amount."),

  // questions
  "what's your name?" => t("To set up your rCredits account, we need your full name and email address. What's your name?"),
  "what's your email?" => t("Welcome to rCredits, @name. Last question: What is your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  'please confirm' => t("To confirm, type @nonce."),
  
  // help messages
  'help for payment' => t("Payment examples: 12.34 to 413.772.0000 | 12.34 to .cornerstore | 12.34 to wgabcd for cash | 12.34 to jdough@example.com for wine and cheese (Include a description ONLY if it's \"cash\" or if you are providing real goods and services.)"),
  'help for charge' => t("Charge examples: 12.34 from 413.772.0000 | 12.34 from wgabcd for cash | 12.34 from jdough@example.com for wine and cheese (Include a description ONLY if it's \"cash\" or if real goods and services are provided.)"),
  'help for undo' => t("Examples: undo | undo to | undo 413.772.0000 | undo from wgabcd"),
  'help for get' => t("Examples: get r 12.34 | get usd 12.34"),
  'help for information' => t("Example: information (tells you your balance and the current demand for rCredits)"),
  'help for help' => t('Examples: @commands. For example, type "help undo".'),
  
  // error messages
  'syntax' => t('Syntax error.'),
//  'no SMS permission' => t('Alas, you do not have permission to use the rCredits SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (@rewards to date), but you can't cash them out. Your balance available to exchange for US Dollars is @cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (@balance to date), but you can't cash them out. Your balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"@param" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Start over and try again.'),
  'no self-trading' => t("You can't do rCredits business with yourself."),
);

$GLOBALS['SMS messages']['help for help no-error'] = t('Try something! ') . $GLOBALS['SMS messages']['help for help'];

/**
 * TT (text)
 *
 * Get the long version of the indicated message(s), make any substitutions,
 * and return the result.
 *
 * @param string $message
 *   name of the message (index into $GLOBALS['SMS messages'])
 *   or a list of such message indices, separated by "|"
 *
 * @param array $substitutions (optional)
 */   
function tt($message, $substitutions = array()) {
  $messages = explode('|', $message);
  $result = array();
  foreach ($messages as $message) {
    $result[] = t($GLOBALS['SMS messages'][$message], $substitutions);
  }
  return join(' ', $result);
}

/**
 * Send
 *
 * Send an SMS message to someone.
 */
function send($number, $message) {
  \sms_send($number, $message);
  r\r_log('SMSout', $message, $number, infer_nonce($message));
}

/**
 * Infer Nonce: Return the nonce (if any) from within the given message.
 */
function infer_nonce($message) {
  $pattern = tt('please confirm', array('@nonce' => '(.*?)'));
  preg_match("`$pattern`", $message, $match);
  return isset($match[1]) ? $match[1] : '';
}

/**
 * To Do
 *
 * Remember what todo, once the user confirms.
 *
 * @parm string $number: the user's cell number
 * @parm string $todo: what to do once the user responds with the expected nonce (defaults to nothing)
 *
 * @return array(nonce, todo) if $todo is null
 */
function todo($number, $todo = '') {
  if ($todo) {
    $todo = addslashes($todo);
    $nonce = 'mango'; // random dictionary lookup of 3-6 chars
    \db_query("UPDATE sms_user SET nonce = '$nonce', todo = '$todo' WHERE number = $number");
  } else {
    return \db_query("SELECT nonce, todo FROM sms_user WHERE number = $number")->fetchAssoc();
  }
}
