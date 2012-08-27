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
  include __DIR__ . '/../../../../inc/countries.php';
  return strlen($country) == 2 ? $countries[$country] : array_search($country, $countries);
}

/**
 * Return the standard 2-letter abbreviation for the (US) state or vice versa
 * @todo: add Canadian provinces
 */
function state_abbrev($state) {
  global $base_path;
  include __DIR__ . '/../../../../inc/states.php';
  return strlen($state) == 2 ? $states[$state] : array_search($state, $states);
}


/**
 * Calculate a short_name from the given full_name.
 */
function short_name($full_name) {
  $deletions = ray('Inc. LLC Co. P.C. Corp. LLP');
  foreach ($deletions as $one) $full_name = str_replace(" $one", '', $full_name);
  $full_name = str_replace('&', 'and', $full_name);
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
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
    if (substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
//    if (strlen($phone) != 10) return FALSE;  // temporarily disabled for debugging
  } elseif (substr($phone, 0, 1) == '+') $foreign = substr($phone, 1);
  
  if (!is_numeric($phone)) return FALSE;

  if ($how == 'n') return $phone;
  if ($how == '+n') return ($foreign ? "+$foreign" : "+1$phone");
  $phone = substr($phone, 0, 3) . '-' . substr($phone, 3, 3) . '-' . substr($phone, 6);
  if ($how == '') return $phone;
  if ($how == '+') {
    $len = substr($foreign, 0, 1) == '7' ? 1 // get length of country prefix
      : (strpos('. 21 22 23 24 25 26 29 35 37 38 42 51 59 67 68 69 85 87 88 96 97 99 ', ' ' . substr($foreign, 0, 2) . ' ') ? 3 : 2);
    return $foreign ? ('(+' . substr($foreign, 0, $len) . ') ' . substr($foreign, $len)) : "(+1) $phone";
  }
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

function import_gfld() {
  $result = \db_query("SELECT * FROM companies ORDER BY full_name");
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $short_name = short_name($full_name);
    $phone = $phone ? "+1$phone" : NULL;
    $fax = $fax ? "+1$fax" : NULL;
    $state = state_abbrev($state);
    $website = str_replace('https://', '', str_replace('http://', '', str_replace('www.', '', $website)));
    $account_type = r\db_lookup(1, 'r_user_industries', "iid IN (121,61) AND uid=:uid", compact('uid')) ? 2 : 1;
    $sql = "UPDATE companies SET phone=:phone, fax=:fax, status=1, website=:website, name=:short_name, short_name=:short_name, state=:state, country='United States', timezone='America/New_York', account_type=:account_type WHERE uid=:uid";
    r\db_q($sql, compact(ray('short_name uid phone fax state website account_type')));
/*
    $cats = unserialize($categories);
    foreach ($cats as $industry) {
      $industry = ucwords(strtolower($industry));
      if (!($iid = r\db_lookup('iid', 'r_industries', 'industry=:industry', compact('industry')))) {
        $record = compact('industry');
        db_query("INSERT INTO r_industries (industry) VALUES (:industry)", $record);
//        db_insert('r_industries')->fields($record);
//        debug(drupal_write_record('r_industries', $record));
        $iid = $record['iid'];
      }
      db_query("INSERT INTO r_user_industries (iid, uid) VALUES (:iid, :uid)", prefix_keys(':', compact('iid', 'uid')));
//      drupal_write_record('r_user_industries', compact(ray('iid uid')));
    }
    */
  }
}

// Single line functions that need no explanation
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
