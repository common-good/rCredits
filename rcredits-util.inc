<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */
 
namespace rCredits\Utilities; // typically abbreviated as "u"
use rCredits as r; // get rid of this

/**
 * Split the given string into an array
 * delimited by space, comma, or semi-colon, followed by any number of spaces, or by a bar surrounded by optional spaces
 * Entries in a comma-delimited list that contain a colon will be split into key and value.
 */
function ray($s) {
  $pattern = strpos($s, '|') ? ' *\| *' : (strpos($s, ',') ? '\, *' : (strpos($s, ';') ? '; *' : (strpos($s, ' ') ? ' +' : '\, *')));
  $simple = mb_split($pattern, $s);
  if (strpos($s, '|') or !strpos($s, ',') or !strpos($s, ':')) return $simple;

  $array = array();
  foreach ($simple as $one) {
    list ($key, $value) = explode(':', $one);
    $array[$key] = $value;
  }
  return $array;
}

/**
 * Interpret the given transaction type
 * @param mixed $type: a number or string indicating the transaction type
 * @return a corresponding string or number (whatever the $type isn't)

function tx_type($type) {
  $types = ray(TX_TYPES);
  return ($type = (is_numeric($type)) ? $types[$type] : array_search($type, $types));
}
function tx_types($string) {return strtr($string, array_flip(ray(TX_TYPES)));} // just string, just one way, for now
*/

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * @param boolean $simplify: Omit the cents for whole dollar amounts
 *
 * @return
 *   the formatted amount
 */
function format_amount($amount, $simplify = TRUE) {
  $amount = '$' . number_format($amount, 2);
  return $simplify ? str_replace('.00', '', $amount) : $amount;
}

/**
 * Say whether the user name is valid.
 * @param string $name: the name to test
 * @param boolean $picky: whether to allow only 2-4 word names (default TRUE)
 */
function valid_name($name, $picky = TRUE) {
  $words = substr_count($name, ' ') + 1;
  if (preg_match("/[^\w\-\.,' ]/iu", $name)) return FALSE; // allows unicode chars and common punctuation
  if ($picky and ($words < 2 or $words > 4)) return FALSE;
  return TRUE;
}

/**
 * Return the standard 2-letter abbreviation for the country or vice versa
 * See the latest data on Wikipedia.
 * @todo: get this in agreement with countries.js (sorted by country name)
 */
function country_abbrev($country) {
  global $base_path;
//  include_once $base_path . 'inc/countries.php';
  include_once __DIR__ . '/../../../../inc/countries.php';
  return strlen($country) == 2 ? $countries[$country] : array_search($country, $countries);
}

/**
 * Return the standard 2-letter abbreviation for the (US) state or vice versa
 * @todo: add Canadian provinces
 */
function state_abbrev($state) {
  global $base_path;
  include_once __DIR__ . '/../../../../inc/states.php';
  return strlen($state) == 2 ? $states[$state] : array_search($state, $states);
}


/**
 * Calculate a short_name from the given full_name.
 */
function short_name($full_name) {
  return strtolower(preg_replace('/\W/', '', iconv('UTF-8', 'ASCII//TRANSLIT', $full_name)));
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function for_cash($for) {
  if (is_array($for)) return for_cash($for[0]) or for_cash($for[1]);
  return ($for == '' or preg_match('/(cash|credit|usd|gift)( |$)/i', $for));
}

/**
 * Return an alphabetic representation of the given integer.
 * A is the zero digit, B is 1, etc.
 * @param int $n: the integer to represent
 * @param int $len: the length of the string to return
 * @param int $base: the radix (defaults to 26).
 */
function n2a($n, $len, $base = 26) {
  $result = '';
  for ($i = 0; $i < $len; $i++) {
    $digit = $n % $base;
    $result = chr(ord("A") + $digit) . $result;
    $n = (int) ($n / $base);
  }
  return $result;
}

/**
 * Return the numeric equivalent of the given alphabetic string.
 * @see n2a
 */
function a2n($string, $base = 26) {
  $result = 0;
  for ($i = 0; $i < strlen($string); $i++) {
    $result = $result * $base + ord($string[$i]) - ord("A");
  }
  return $result;
}

/**
 * Format the given phone number as wanted
 * @param string $phone
 * @param string $how: how to format it
 *   '+n' +1dddddddddd
 *   'n'  dddddddddd
 *   '+'  (+1) ddd-ddd-dddd
 *   ''   ddd-ddd-dddd (the default)
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
function format_phone($phone, $how = '') {
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // possible phone number for the other party
  if (substr($phone, 0, 2) == '+1') $phone = substr($phone, 2);
//  if (!is_numeric($phone) or strlen($phone) != 10) return FALSE;
  if (!is_numeric($phone)) return FALSE; // temporary for debugging
  if ($how == 'n') return $phone;
  if ($how == '+n') return '+1' . $phone;
  $phone = substr($phone, 0, 3) . '-' . substr($phone, 3, 3) . '-' . substr($phone, 6);
  if ($how == '') return $phone;
  if ($how == '+') return '(+1) ' . $phone;
  return FALSE;
}

/**
 * Return the array with its keys prefixed by the given string
 */
function prefix_keys($prefix, $array) {
  if ($prefix == '') return $array;
  $new_array = array();
  foreach ($array as $key => $value) $new_array[$prefix . $key] = $value;
  return $new_array;
}

function array_prefix($prefix, $array) {
  foreach ($array as $key => $value) $array[$key] = $prefix . $value;
  return $array;
}

/**
 * Return the difference between two arrays (better than array_diff_assoc)
 */
function diff($a, $b) {
  $result = array();
  foreach ($a as $key => $one) if (!is_null($a[$key])) {
    if ($a[$key] != @$b[$key] or !isset($b[$key])) $result[] = $key . (isset($b[$key]) ? '' : ': missing in #2');
  }
  foreach ($b as $key => $one) if (!is_null($b[$key]) and !isset($a[$key])) $result[] = $key . ': missing in #1';
  return $result;
}

/**
 * Get the long version of the indicated message(s), make any substitutions,
 * and return the result.
 *
 * @param string $message
 *   name of the message (index into $GLOBALS['SMS messages'])
 *   or a list of such message indices, separated by "|"
 *   or a two-element array: array(message, subs)
 * @param array $subs (optional). '@' will be prepended to the keys.
 * @param string $interface: which message set to use (SMS messages provide a fallback)
 */   
function tt($message, $subs = array(), $interface = 'SMS') {
  if (is_array($message)) list ($message, $subs) = $message;
  if (!$subs) $subs = array();
  $messages = explode('|', $message);
  foreach ($messages as $i => $message) {
    if (isset($GLOBALS["$interface messages"][$message])) {
      $model = $GLOBALS["$interface messages"][$message];
    } else {    
      $model = $GLOBALS['SMS messages'][$message];
      if ($p = strpos($model, '. Type ')) $model = substr($model, 0, $p + 1); // don't tell the user to type something unless it's SMS
    }
    $messages[$i] = strtr($model, prefix_keys('@', $subs));
  }
  return join(' ', $messages);
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 * @todo: get these r\'s out of here.
 */
function nonce() {
  $offset = r\db_lookup('FLOOR(RAND() * COUNT(*))', 'r_nonces');
  $nonce = r\db_lookup('nonce', 'r_nonces', "1 LIMIT $offset, 1");

  $impossible = join(' ', $GLOBALS['SMS not a nonce']);
  if (strpos(". $impossible ", " $nonce ")) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long
  return $nonce;
}

// Single line functions that need no explanation
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
