<?php
namespace rCredits\SMS;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;

/**
 * @file
 * rSMS include file
 *
 * Utilities and definitions for the rSMS module
 * (anything that doesn't have to be in rsms.module and isn't appropriate to class call).
 */

// Statuses
define('SMS_NO', 0); // not verified (maybe lost?)
define('SMS_PRIMARY', 1); // primary cell phone for notifications, etc. (and verified)
define('SMS_OK', 2); // verified
define('SMS_LOST', 9); // user lost phone (NOT YET USED)

define('RSMS_PREFIX', 'r: '); // identify us
//define('RSMS_PREFIX_INACTIVE', '[TRIAL RUN] '); // pretend transactions by not-yet-active players (treat it as a trial run)

require_once __DIR__ . '/../rcredits-strings.inc';
require_once __DIR__ . '/../rcredits-backend.inc'; // Drupal bug (needs this explicit)
require_once __DIR__ . '/../rcredits-util.inc';

$GLOBALS['tStrings'] += array(
//  'duplicate email' => t('There is already an account with that email address. Visit %CG_DOMAIN to add this cell number to your account.'),
);

//$GLOBALS[TX_SMS . ' messages'] = array_merge($GLOBALS[TX_SMS . ' messages'], $GLOBALS['SMS help']); 
$GLOBALS['tStrings']['help helpme no-error'] = t('Try something! ') . $GLOBALS['tStrings']['help helpme'];
foreach (ray('more stop start') as $one) $GLOBALS['tStrings']["help $one"] = $GLOBALS['tStrings']['help helpme'];

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
$GLOBALS['SMS gateway words'] = ray('stop unsubscribe cancel quit start yes help'); // "help" never reaches us
$GLOBALS['SMS not a nonce'] = array_merge(
  ray('char fro'), 
  array_keys($GLOBALS['SMS commands']),
  $GLOBALS['SMS gateway words']
); 
$GLOBALS['SMS reserved words'] = array_merge(
  array_keys($GLOBALS['SMS commands']),
  ray('okay approve reverse contest refuse reject deny to from balance give send invoice take as usd'), 
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
  u\loga('out', compact(ray('number message nonce')), TX_SMS); // "via chTX_SMS" in case called from another interface
}

/**
 * Return the nonce (if any) from within the given message.
 */
function infer_nonce($message) {
  $pattern = t('please confirm', 'nonce', '([A-Z]*)');
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
  u\EXPECT(compact(ray('number uid status')), 'string&notempty id int');
  $number = u\fmtPhone($number, '+n');
///  debug("creating SMS record for number $code, uid $uid");
  return r\makeDevice($uid, $number, TX_SMS);
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
 *   t($array); OR
 *   t($message); OR t($message, $subs);
 * @param array $array: array($message, $subs)
 * @param string $message: index to message to display
 * @param array $subs: associative array of substitutions (@ will be prepended to each key)
 * return string: the message after any substitutions
 */
//function t($message, $subs = []) {return t($message, $subs, TX_SMS);} // TX_SMS in case called from another interface

// one-line functions that need no explanation
function uid($number) {return (int) db\lookup('uid', 'r_boxes', 'code=:number', compact('number'));}
