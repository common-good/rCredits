<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */

namespace rCredits\Util; // typically abbreviated as "u"
use rCredits as r; // get rid of this
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Testing as t;

define ('URL_CHARS', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_~'); // 66 valid URLchars
define ('HEX_CHARS', '0123456789abcdef');

// Some useful Drupal db methods: fetchAllKeyed(m, n), fetchAssoc and fetchAllAssoc(fld), fetchObject and fetchAll(), fetchCol(n), fetchField()

/**
 * Return an array built from the argument(s).
 * Call by:
 *    ray(keys, value1, value2, ...) // return an associative array
 *    ray(keys) // return the given string split into an array
 *
 * Either way, keys may be delimited by space, comma, or semi-colon, 
 *   followed by any number of spaces, or by a bar surrounded by optional spaces
 *   Entries in a comma-delimited simple list that contain a colon will be split into key and value.
 * @param string $keys: delimited list of identifier names
 * @param mixed $value1,...: arbitrary values
 */
function ray($s, $value1 = NULL) {
  if (func_num_args() > 1) { // multi-argument call -- return an associative array
    $args = func_get_args();
    $keys = u\ray(array_shift($args));
    u\EXPECT(count($keys) == count($args), 'assoc arg count mismatch');
    return array_combine($keys, $args);
  }

  u\EXPECT(compact('s'), 'string');
  $pattern = strpos($s, "\n") ? '/\R/' : (strpos($s, '|') ? ' *\| *' : (strpos($s, ',') ? '\, *' : (strpos($s, ';') ? '; *' : (strpos($s, ' ') ? '  *' : FALSE)))); // strangely ' +' fails but '  *' works
  $simple = $pattern ? mb_split($pattern, $s) : array($s);
  if (!strpos($pattern, ',') or !strpos($s, ':')) return $simple; // no subargs

  $array = array();
  foreach ($simple as $one) {
    list ($key, $value) = explode(':', $one);
    $array[$key] = $value;
  }
  return $array;
}

/**
 * Let a called function receive a single assoc arg or several named args, simulating a call with u\ray(s,...).
 * call by extract(u\rayy(func_get_args()));
 * The called function cannot otherwise accept an assoc as its first argument.
 * @param array $args: func_get_args() from the called function
 * @return u\ray called with those args if viable, else array()
 */
function rayy($args) {
  return (count($args) >= 2 and is_string($args[0])) ? call_user_func_array('rCredits\\Util\\ray', $args) : array();
}

/* this is probably a bad idea
function preK($prefix, $s) {
  if (is_array($s)) {
    $new = array();
    foreach ($s as $key => $value) $new[$prefix . ucwords($key)] = $value;
    return $new;
  } else return $prefix . ucwords($s);
}
*/

/**
 * Return just the specified key/value pairs from the given array, trimmed.
 * Typical use: extract(just('key2 key4 key 7', $info));
 * If a key did not exist in the original array, it will no be in the returned array either.
 * @param string/array $these: array or space-delimited array of field names
 * @param array/object/string $args: possible key/value pairs to return (passed as array, object, or UrlArgString)
 * @return array of the results, trimmed
 */
function just($these, $args) {
  $result = array();
  if (is_string($args)) {
    parse_str(strpos($args, urlencode('=')) ? urldecode($args) : $args, $array);
  } else $array = (array) $args; // in case it was an object
  if (!is_array($these)) $these = ray($these);
  foreach ($these as $one) if (isset($array[$one])) $result[$one] = is_string($array[$one]) ? trim($array[$one]) : $array[$one];
  return $result;
}

/**
 * array_splice() for associative arrays.
 * If the key is not found, $input is unchanged and return NULL
 */
function splice(&$input, $key, $len = 0, $replacement = NULL) {
  $i = array_search($key, $keys = array_keys($input));
  if ($i === FALSE) return NULL;
  $values = array_values($input);
  $iKeys = array_splice($keys, $i, $len, @array_keys($replacement));
  $iValues = array_splice($values, $i, $len, @array_values($replacement));
  $input = array_combine($keys, $values);
  return array_combine($iKeys, $iValues);
}

/**
 * Return the amount string, cleaned up (leading $ and any commas or spaces are discarded)
 */
function cleanAmount($amount) {
  $amount = trim($amount);
  if (substr($amount, 0, 1) == '$') $amount = substr($amount, 1);
  if (u\nonish($amount)) $amount = 0;
  return preg_replace('/[ \,]/', '', $amount); // ignore spaces and commas
}

/**
 * Return an error message if the amount is bad.
 * @param string $amount: the questionable amount string (leading $ and any commas or spaces are ignored)
 *   The cleaned-up amount is returned by ref.
 * @param string $restriction: none (the default), >0, or >=0
 * @param bool $fractionLimit: maximum digits after the decimal (default 2)
 * @return: FALSE if not a bad amount, else error message
 */
function badAmount(&$amount, $restriction = '', $fractionLimit = 2) {
  if (!is_numeric($new = cleanAmount($amount))) return 'bad amount';

  $amount = $new;
  $dotPos = strpos($amount, '.');
  if ($dotPos !== FALSE and strlen($amount) > $dotPos + 1 + $fractionLimit) return 'too fractional';
  if ($amount > R_MAX_AMOUNT) return 'amount too big';
  if ($restriction == '>0' and $amount <= 0) return 'nonpositive amount';
  if ($restriction == '>=0' and $amount < 0) return 'negative amount';
  return FALSE;
}

/**
 * Say whether the given SSN is bad.
 * @param string $ssn: the social security number to test (RETURNED stripped of all but the digits)
 * @param int $dob: the person's birth date (null if the ssn is actually an EIN)
 * @return an error index if the SSN is bad, else FALSE
 * From the SSA site:
     We do not assign a Social Security number with the first three digits of: 000, 666, 900 - 999.
     Nor do we assign Social Security number with "00" in the fourth and fifth position, 
     or "0000" in the last four positions of the numbers.
     Additionally, prior to June 25, 2011, we had never assigned a Social Security number with the first three digits of: 000, 666, 772 - 799, 800 - 899, 900 - 999.
 */
function badSsn(&$ssn, $dob) {
  if (in_array($ssn, u\ray('5551234567 777777777 888888888 999999999'))) return FALSE; // test numbers
  if (strlen(u\digits($ssn)) != 9) $bad = TRUE; // zap all but the digits (call by ref)
  $c1 = substr($ssn, 0, 1);
  $s1 = substr($ssn, 0, 3);
  $s2 = substr($ssn, 3, 2);
  $s3 = substr($ssn, 5);
  if (@$dob) { // ssn (not ein)
    if ($c1 == '9' or $s1 == '000' or $s1 == '666' or $s2 == '00' or $s3 == '0000') $bad = TRUE;
    if (substr($ssn, 0, 8) == '98765432') $bad = TRUE; // advertising numbers, according to Wikipedia
    if ($dob < strtotime('25Jun2011')) if ($c1 == '8' or ($c1 == '7' and $s1 > '771')) $bad = TRUE;
  }
  return @$bad ? (@$dob ? 'bad ssn' : 'bad federal id') : FALSE;
}

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * $param string $options (default s$): 
 *   s = simplify: Omit the cents for whole dollar amounts
 *   $ = include initial dollar sign
 *   r = include a trailing r
 *   u = include a trailing us
 * @return
 *   the formatted amount
 */
function fmtAmt($amount, $options = 's$') {
  $options = ' ' . $options; // simplify strpos check
  $amount = cleanAmount(nn($amount, 0));
//  if (!is_numeric($amount)) $amount = -999999;
  u\EXPECT(compact('amount'), 'float', 'bad num');
  $amount = number_format($amount, 2);
  if (strpos($options, '$')) $amount = '$' . $amount;
  if (strpos($options, 's')) $amount = str_replace('.00', '', $amount);
  if (strpos($options, 'r')) $amount .= 'r';
  if (strpos($options, 'u')) $amount .= 'us';
  return $amount;
}

/**
 * Return HTML for a visual calculator.
 */
function calculator($field = 'amount') {
  $s = '<div id="calc">';
  $symbols = '123|456|789|c0b';
  $field = "this.form.$field.value";
  for ($i = 0; $i < strlen($symbols); $i++) {
    $c = substr($symbols, $i, 1);
    $formula = "$field=commafy(10*$field.replace(/,/, '')+.0$c);"; // default to adding a digit
    $class = "calc$c";
    if ($c == '|') {
      $s .= "<br>";
      continue;
    } elseif ($c == 'c') {
      $formula = "$field='0.00';";
    } elseif ($c == 'b') {
      $formula = "$field=commafy(parseFloat($field.slice(0, -1).replace(/,/, ''))/10);"; // remove last digit
      $c = '&#x25C0;'; // unicode left arrow head
    }
    $s .= "<button id=\"$class\" type=\"button\" onclick=\"$formula\">$c</button>"; // iPhone <input> looks bad
  }
  return $s . '</div>';
}

/**
 * Say whether the user name is valid.
 * @param string $name: the name to test
 * @param boolean $picky: whether to allow only 2-4 word names (default TRUE)
 */
function validName($name, $individual = TRUE, $picky = TRUE) {
  $name = trim($name);
  $words = substr_count($name, ' ') + 1;
  $pattern = $individual ? "[^\w\-\.,' ]" : '[^\w~!@#\$%\^&\*\(\)\-=\+\\\\;:\'",<\.>/\? ]';
  if (preg_match("`$pattern`iu", $name)) return FALSE; // allows unicode chars and common punctuation
  if (isQid(strtoupper($name))) return FALSE; // never allow a QID as a name
  if ($picky and ($words < 2 or $words > 4)) return FALSE;
  return TRUE;
}

/**
 * Calculate a short_name from the given fullName.
 */
function shortName($fullName, $filler = '') {
  $deletions = ray('Inc. LLC Co. P.C. Corp. LLP');
  foreach ($deletions as $one) $fullName = str_replace(" $one", '', $fullName);
  $fullName = str_replace('&', 'and', $fullName);

  ini_set('mbstring.substitute_character', 'none'); 
  $fullName = mb_convert_encoding($fullName, 'UTF-8', 'UTF-8'); // avoid iconv failure (bug in PHP 5.4.3)

  $result = preg_replace('/\W/', $filler, iconv('UTF-8', 'ASCII//TRANSLIT', $fullName));
  return strtolower($result === '' ? u\code() : $result);
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function forCash($for) {
  if (is_array($for)) return forCash($for[0]) or forCash($for[1]);
  return ($for == '' or preg_match('`^(rCredits/USD|automatic transfer|cash|usd|loan)(\W|$)`i', $for)); // empty or a non-goods word at the beginning
}

/**
 * Return an alphabetic representation of the given non-negative integer.
 * A is the zero digit, B is 1, etc.
 * @param int $n: the integer to represent
 * @param int $len: the length of the string to return
 *   <=0 means length >=-$len
 * @param int $base: the radix (defaults to 26).
 */
function n2a($n, $len, $base = 26) {
  $result = '';
  for ($i = 0; ($len > 0 ? ($i < $len) : ($n > 0 or $i < -$len)); $i++) {
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
  $string = strtoupper($string);
  $result = 0;
  for ($i = 0; $i < strlen($string); $i++) {
    $result = $result * $base + ord($string[$i]) - ord("A");
  }
  return $result;
}

/**
 * Encode 3 hex digits as 2 legal URL characters: [A-Za-z0-9.-]
 * @param string $s: the three digits to convert
 * rCredits\Util\x32a2(rCredits\Util\a22x3('B7'));
 */
function x32a2($s) {
  $chars = URL_CHARS;
  foreach (str_split($s) as $c) $cs[] = stripos(HEX_CHARS, $c); // get hex values
  return $chars[($cs[0] << 2) + ($cs[1] >> 2)] . $chars[(($cs[1] % 4) << 4) + $cs[2]];
}

/**
 * Decode 2 legal URL characters as 3 hex digits (see x32a2, above).
 * @param string $s: the two characters to convert -- only the first 64 chars in URL_CHARS are allowed,
 */
function a22x3($s) {
  $chars = HEX_CHARS;
  foreach (str_split($s) as $c) $cs[] = strpos(URL_CHARS, $c);
  return $chars[$cs[0] >> 2] . $chars[(($cs[0] % 4) << 2) + ($cs[1] >> 4)] . $chars[$cs[1] % 16];
}

/**
 * URL encode an arbitrary non-empty ascii string compactly
 */
function urlify($s) {
  u\EXPECT($s !== '', 'empty string');
  $len = strlen($s = bin2hex($s));
  for ($i = 0, $result = ''; $i < $len - 2; $i += 3) $result .= x32a2(substr($s, $i, 3));
  if ($taili = $len % 3) $result .= '~' . substr($s, -$taili);
  return $result;
}

/**
 * URL decode an arbitrary ascii string that was encoded with urlify (see above).
 * @param string $s: the URLified string
 * @return string: the original un-encoded string (FALSE if error)
 */
function deurlify($s) {
  list ($s, $tail) = explode('~', $s . '~');
  for ($i = 0, $result = ''; $i < strlen($s) - 1; $i += 2) $result .= a22x3(substr($s, $i, 2));
  return hex2bin($result . $tail);
}

/**
 * Return the ein in standard format (xx-xxxxxxx)
 */
function fmtEin($ein) {
  $ein = str_replace('-', '', $ein);
  return substr($ein, 0, 2) . '-' . substr($ein,2);
}

/**
 * Format the given phone number as wanted
 * @param string $phone
 * @param string $how: how to format it
 *   '+n' +1dddddddddd (AKA 'internal')
 *   'n'  dddddddddd
 *   '+'  +1 ddd ddd dddd (the international standard with spaces added) (the default)
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
define('MAX_PHONE_DIGITS', 15);
define('MIN_PHONE_DIGITS', 7);

function fmtPhone($phone, $how = '+') {
  $sep = ' '; // digit group separator
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
  } elseif (substr($phone, 0, 1) == '1') {
    $phone = substr($phone, 1);
  } elseif (substr($phone, 0, 1) == '+') $foreign = $phone = substr($phone, 1);

  if (!is_numeric($phone) or (!t\est() and strlen($phone) < MIN_PHONE_DIGITS) or strlen($phone) > MAX_PHONE_DIGITS) return FALSE;
  if (!$foreign and substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
  if (!$foreign and strlen($phone) != 10) return FALSE;

  if ($how == 'n') return $phone;
  if ($how == '+n' or $how == 'internal') return ($foreign ? "+$foreign" : "+1$phone");

  $phone = substr($phone, 0, 3) . $sep . substr($phone, 3, 3) . $sep . substr($phone, 6);
//  if ($how == '') return $phone;
  if ($how == '+') {
    $len = substr($foreign, 0, 1) == '7' ? 1 // get length of country prefix
      : (strpos('. 21 22 23 24 25 26 29 35 37 38 42 51 59 67 68 69 85 87 88 96 97 99 ', ' ' . substr($foreign, 0, 2) . ' ') ? 3 : 2);
    return $foreign ? ('+' . substr($foreign, 0, $len) . ' ' . substr($foreign, $len)) : "+1 $phone";
  }
  return FALSE;
}

/**
 * Return the array with its keys prefixed by the given string
 */
function prefixKeys($prefix, $array) {
  EXPECT(compact('prefix', 'array'), 'string array');
  if ($prefix == '') return $array;
  $new_array = array();
  foreach ($array as $key => $value) $new_array[$prefix . $key] = $value;
  return $new_array;
}

function prefixValues($prefix, $array) {
  foreach ($array as $key => $value) $array[$key] = $prefix . $value;
  return $array;
}

function changeKey($oldKey, $newKey, &$array) {
  $array[$newKey] = $array[$oldKey];
  unset($array[$oldKey]);
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
 * Get the long version of the indicated message(s) (if any), make any substitutions,
 * and return the result.
 * Channel determines which message set to use (each simpler channel provides a fallback)
 *
 * @param string $message
 *   name of the message (index into $GLOBALS[channel# . ' messages'])
 *   or a list of such message indices, separated by "|"
 *   or a two-element array: array(message, subs)
 * @param array $subs (optional). '@' will be prepended to the keys.
 * @todo get the r\'s out of this utilities file OR stop trying to separate u\ from r\
 */   
function tt($message, $subs = array(), $adhocChannel = '') {
  global $channel;
 
  if (!$adhocChannel) $adhocChannel = $channel;
  if (is_array($message)) list ($message, $subs) = $message;
  if (!$subs) $subs = array(); // accept ''
  $messages = explode('|', $message);
  $channels = ray(TX_CHANNELS);

  foreach ($messages as $i => $message) {
    $ch1 = $adhocChannel ?: (count($channels) - 1);
    for ($ch = $ch1; $ch > 0; $ch--) if ($model = @$GLOBALS["$ch messages"][$message]) break;
    if (!@$model) $model = u\redw($message);
    if($p = strpos($model, '. Type ')) {
      if ($channel != TX_SMS and $adhocChannel != TX_SMS) $model = substr($model, 0, $p + 1); // don't tell the user to type something unless it's SMS
    }
    if (@$subs['href']) $model = str_replace('<a>', '<a href="@href">', $model);
// NO (use raw message instead)   EXPECT(@$model, "missing message '$message'"); 
    $messages[$i] = SUBS(strtr($model, prefixKeys('@', $subs)), '@');
  }
  return join(' ', $messages);
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 * @todo: get these r\'s out of here.
 */
function nonce($minLen = 1) {
  list ($where, $subs) = array('LENGTH(nonce)>=:minLen', compact('minLen'));
  $offset = db\lookup('FLOOR(RAND() * COUNT(*))', 'r_nonces', $where, $subs);
  $nonce = db\lookup('nonce', 'r_nonces', "$where LIMIT $offset, 1", $subs);

  $impossible = join(' ', $GLOBALS['SMS not a nonce']);
  if (strpos(". $impossible ", " $nonce ")) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long (currently never happens because all are <= 6)
  return $nonce;
}

function ignoreSuffix($s, $suffixes) {
  $suffixes = preg_replace('/\W+/', ' ', $suffixes);
  $pattern = str_replace(' ', ' |', trim($suffixes));
  return trim(preg_replace("`([^ ])($pattern )`sim", '$1 ', " $s "));
}

/**
 * An HTTP REST requester
 * modified from Wez Furlong's Generic REST Helper (http://wezfurlong.org/blog/2006/nov/http-post-from-php-without-curl/)

Wez writes:
PHP's HTTP wrapper will automatically fill out the Content-Length header based on the length of the $content that you pass in. It will also automatically set the Content-Type to application/x-www-form-urlencoded if you don't specify one in the $extra_headers.
...
Many web services offer a REST-ful interface for consuming their data, using GET requests for information retrieval and POST requests for making changes. Below you'll find a helper function that can very easily be used to consume a REST API.

The $url parameter is the HTTP or HTTPS URL for the web service. $content is an associative array of form parameters to pass to the web service; they will be passed as _GET parameters for GET requests or _POST parameters for POST requests. The $method parameter can be GET or POST (and presumably any other valid HTTP REQUEST method, such as PUT or DELETE, although I haven't tried those and can't say whether they will work as expected). The $format parameter can be "json" or "xml" and will automatically return a decoded json or XML document, respectively.

I've used simplexml here because it is... simple. You could very easily add a "dom" format to return the object using the richer and more complex DOM API instead.

This function uses the ignore_errors context parameter. Without this set (the default is FALSE), PHP will treat 400 and 500 HTTP status codes as a failure to open the stream and won't return you any data. This is usually what you want when using fopen or file_get_contents, but REST services tend to set the HTTP status to indicate the error and will usually send back a payload that describes the error. We turn on ignore_errors so that we treat any returned payload as json or xml.

When using POST with REST, take care: PHP's HTTP redirection handler will drop your POST payload if the endpoint issues a redirect. If you experience problems using POST with the function below, it might be due to redirects. Most of the POST calls I've run into issue redirects if the URL is missing a trailing '/' character. In other words, if you experience problems where it seems like your parameters are not being sent in, try appending a '/' to the end of the URL and try it again.
 */
function httpRequest($url, $content = FALSE, $responseFormat = 'raw', $extra_headers = array(), $method = 'POST', $request_format = 'url') {
  $ignore_errors = TRUE;
  $headers = array('Content-Type' => $request_format == 'json' ? 'application/json' : 'application/x-www-form-urlencoded') + $extra_headers;

  $header = '';
  foreach ($headers as $key => $value) $header .= "$key: $value\n";
  $http = compact(ray('method ignore_errors header'));

  if ($content) {
    $content = http_build_query($content);
    if ($method == 'POST') {
      if ($request_format == 'json') $content = u\jsonEncode($content);
      $http += compact('content');
    } else $url .= '?' . $content;
  }

  $context = stream_context_create(compact('http'));
  if ($fp = fopen($url, 'rb', FALSE, $context)) {
    // If you're troubleshooting, uncomment the next line to see the HTTP response headers across all redirects:
    // $meta = stream_get_meta_data($fp); var_dump($meta['wrapper_data']);
    $response = stream_get_contents($fp);
  } else $response = FALSE;

  if ($response === FALSE) throw new \Exception("$method $url failed: $php_errormsg");
  $result = $responseFormat == 'xml' ? simplexml_load_string($response)
    : ($responseFormat == 'json' ? json_decode(utf8_encode($response))
    : ($response) );
  EXPECT(!is_null($result), "$method $url returned error: $response");
  return $result;
}

function randomInt($len = NULL) {
  $result = '';
  while (strlen($result) < $len) $result .= mt_rand(100000000, 999999999);
  return substr($result, 0, $len);
}

/**
 * Translate constant parameters in a string.
 * For example, "SELECT * FROM table WHERE zot IN (:R_THIS, :R_THAT)" fills in the constants.
 * Constants of the form "IS_WHATEVER" are a special case. They are taken to mean the corresponding
 *   bit number B_WHATEVER in a field called $flagField
 *   NOTE to developer: this could be expanded later to handle a second flag field named {$flagField}2
 *     by changing the expression to "IF(:B_$1<32,$flagField&(1<<:B_$1),{$flagField}2&(1<<(:B_$1-32)))".
 * @param string $string: the string to fix
 * @param string $flagField: name of the flags field for bit constants
 * @return string: the string with constant names replaced by their values
 */
function SUBS($string, $prefix = ':', $flagField = 'flags') {
  $string = preg_replace("/([a-z]+[0-9]?\.)?{$prefix}IS_([A-Z0-9_]*)/ms", "($1$flagField&(1<<:B_$2))", $string);
//  $string = preg_replace("/\\{$prefix}IS_([A-Z0-9_]*)/ms", "($flagField&(1<<:B_$1))", $string);
  preg_match_all("/\\$prefix([A-Z_][A-Z0-9_]*)/ms", $string, $matches);
  $map = array();
  foreach ($matches[1] as $one) if (defined($one)) $map["$prefix$one"] = constant($one);
  return strtr($string, $map);
}

/**
 * Replace some common unprintable characters with their HTML equivalent
 * (dash, magic quotes, )
 *
function wentities($s) {
  return strtr($s, u\ray('— ‘ ’ “ ”'', '&mdash;', 
  */

/**
 * Assert an expectation or check parameters for validity
 * Throw an Exception if the expectation is not met.
 * call by:
 *   EXPECT($assertion, $message, $ret) OR
 *   EXPECT($args, $types, $ret) eg EXPECT(compact(u\ray('arg1 arg2')), 'bool float');
 * @param boolean $assertion: a statement to assert
 * @param string $message: message to display if assertion is FALSE
 *
 * @param assoc $args: variables, indexed by variable name, for which to check types
 * @param string $types: a corresponding space-delimited array of expected types, 
 *   each of which may in turn be a list of types delimited by '&' or a list of such lists as options, delimited by '|'.
 *
 * @param bool $ret: return the error message rather than throwing it
 * @return string (if $ret is TRUE): an error message or '' (no error)
 * This function name is capitalized so that it stands apart from the actual code.
 * Best practice: don't put any functional code in the EXPECT call (eg don't do EXPECT($i = function($blue)) )
 */
function EXPECT($assertion, $message = '', $ret = FALSE) {
  if (is_array($assertion)) {
    list ($args, $types) = array($assertion, $message); // for clarity
///    EXPECT(compact(ray('args types')), 'assoc string'); // can't do this because infinite recursion
///    EXPECT(count($args) != count($types = explode(' ', $types)), 'wrong number of types expected'); // infinite recursion
///    if(count($args) != count($types = explode(' ', $types))) die('wrong number of types expected: ' . print_r(debug_backtrace(), 1));
/**/ if (isDEV and count($args) != count(explode(' ', $types))) debug(debug_backtrace()); // keep
    $types = array_combine(array_keys($args), explode(' ', $types)); // not ray() here, $types may have '|'
    foreach ($args as $key => $value) if ($error = expect1($key, $value, $types[$key], $ret)) return $error;
  } elseif (!$assertion) {
    if ($ret) return $message; else throw new \Exception($message);
//    EXPECT(compact(ray('assertion message')), 'bool string&notempty'); // can't do this because infinite recursion
  }
  return ''; // no error
}  

function expect1($arg, $value, $type, $ret) {
//  EXPECT(compact(ray('arg type')), 'name string'); // can't do this because infinite recursion

/**/ $msg = "Expected \$$arg ('" . print_r($value, 1) . "') to be type '$type'."; // standard err message (keep this)
  if (strpos($type, '|')) {
    foreach (explode('|', $type) as $one) if (expect1($arg, $value, $one, TRUE) == '') return '';
    EXPECT(FALSE, $msg, $ret);
  }
  if (strpos($type, '&')) {
    foreach (explode('&', $type) as $one) if ($error = expect1($arg, $value, $one, $ret)) return $error;
    return ''; // no error
  }
  
  switch ($type) {
    case 'array': return EXPECT(is_array($value), $msg, $ret);
    case 'assoc': 
      if ($msg2 = EXPECT(is_array($value), $msg, $ret)) return $msg2;
      foreach ($value as $key => $one) expect1("$arg key", $key, 'name', $ret);
      return;
    case 'bool': return EXPECT(is_bool($value) or $value == 0 or $value == 1, $msg, $ret);
    case 'email': return EXPECT(\valid_email_address($value), $msg, $ret);
    case 'empty': return EXPECT(empty($value), $msg, $ret);
    case 'float': return EXPECT(myFloat($value), $msg, $ret); // don't use is_float()?
    case 'id': return EXPECT(isId($value), $msg, $ret);
    case 'int': return EXPECT(myInt($value), $msg, $ret); // don't use is_int()
    case 'name': return EXPECT(isName($value), $msg, $ret);
    case 'notempty': return EXPECT(!empty($value), $msg, $ret);
    case 'notnull': return EXPECT(!is_null($value), $msg, $ret);
    case 'qid': return EXPECT(isQid($value), $msg, $ret);
    case 'string': return EXPECT(is_string($value), $msg, $ret);
    case 'zid': return EXPECT(isZid($value), $msg, $ret);
    default: return EXPECT(is_object($value) and get_class($value) == "rCredits\\$type", $msg, $ret);
  }
  throw new \Exception("bad type \"$type\" ($msg)");
}

//function gripe($msg) {throw new \Exception($msg);}
//function gripe($msg) {throw new \Exception(serialize(debug_backtrace()));}

/** 
 * Log the info in both the log file and database.
 * This makes it easy for tests to see what was done
 *
 * @param string $type: type of log entry (if FALSE, don't log anything)
 * @param array $info: the information to log
 * @param string $special: a special value if any (for example the nonce or an account object)
 * @param int $adhocChannel: a second channel used from within the primary channel
 * @return the log record id (for exception handler and shutdown function)
 */  
function loga($type, $info = array(), $special = '', $adhocChannel = FALSE) {
/**/  if (strlen($type) >= 60) die('in loga: ' . $type); // (KEEP) don't use EXPECT here
  global $channel;
  u\setDft($channel, -1);
  $useTextLogToo = FALSE; // isDEV;

  if (is_object($special)) {
    $mya = $special;
    $special = '';
  } else $mya = r\acct();
  $myid = @$mya->id;
  $agent = @$mya->agentId;
  $time = time();
  $date = fmtDate();
  if ($adhocChannel and $adhocChannel != $channel and !$special) $special = "via ch$adhocChannel";
  
  // make it easy to match the message when searching
  if (isNonce($special) and @$info['message']) $info['message'] = str_replace(": $special", ': @nonce', $info['message']);
  // don't make code, password, and secret links any more accessible than they are
  foreach (u\ray(R_NOLOG) as $one) if (@$info[$one]) $info[$one] = '(?)';
  $info = u\jsonEncode($info);
  $channels = ray(TX_CHANNELS); 
  if ($useTextLogToo) file_put_contents($channels[$channel + 0] . '.log', "$date $type: $info\n", FILE_APPEND);
  $record = compact(ray('time channel type myid agent info special'));

  if ($type == 'FATAL' and strpos($info, 'calling db_query() ')) {
    return '';
  } else {
    $return = \Database::RETURN_INSERT_ID; // do NOT use db\insert here (avoid loop in error handling)
    return \db_query('INSERT INTO r_log (time, channel, type, myid, agent, info, special) VALUES (:time, :channel, :type, :myid, :agent, :info, :special)', u\prefixKeys(':', $record), compact('return'));
  }
//  return db\insert('r_log', $record);
}  

function seeAssoc($info) {
  $result = '';
/**/ foreach ($info as $key => $value) $result .= "<b>$key</b>: " . print_r($value, 1) . "<br>\n"; // keep
  return "\n<br>$result";
}

/**
 * Return the first unused id.
 * @param string $key: name of field to search
 * @param string $table: table name
 * @param int $starting: lowest id to use (defaults to 1)
 * @param int $before: first id not to use (if any)
 * @param string $where: other criteria
 * @param assoc $subs: substitutions for the other criteria
 * @return: the id (null if none available)
 */
function firstUnusedId($key, $table, $starting = 1, $before = PHP_INT_MAX, $where0 = '1', $subs = array()) {
  if (!db\exists("$table t1", "$where0 AND t1.$key=$starting", $subs)) return $starting;
  $where = "$where0 AND t1.$key>=$starting AND t1.$key<$before-1 AND t2.$key IS NULL ORDER BY t1.$key";
  $prev = db\lookup("t1.$key", "$table t1 LEFT JOIN $table t2 ON t2.$key=(t1.$key+1)", $where, $subs);
  return $prev ? ($prev + 1) : NULL;
}
 
/**
 * Normalize an image file to the given aspect
 * @param string $filename: path of file to normalize (file will be replaced)
 * @param string $aspect: the desired width:height ratio
 * @param int $factor: normalized size is the aspect times $factor.
 * @param string $error: (returned) the error message, if any
 * @return: TRUE for success, else FALSE
 * @todo: handle webp and xpm images?
 */
function fixPicture($filename, $aspect, $factor, &$error) {
  $acceptableImages = IMAGETYPE_GIF | IMAGETYPE_JPEG | IMAGETYPE_PNG | IMAGETYPE_WBMP | IMAGETYPE_XBM;
  if (!$info = @getimagesize($filename)) return (!$error = 'not an image');
  list ($w, $h, $type, $attr) = $info;
  $ext = image_type_to_extension($type, FALSE);
  if (!(imagetypes() & $type)) return (!$error = 'not an image');
  if (!($acceptableImages & $type)) return (!$error = 'not an image');
  list ($w0, $h0) = explode(':', $aspect);
  $off = $w/$h - $w0/$h0; // how much is the aspect off by?
  if (abs($off) > .35) return (!$error = 'aspect');
  $k = 1 - $off * $h / $w; // wFactor, if we need to correct width
  list ($wFactor, $hFactor) = $off > 0 ? array($k, 1) : array(1, 1 / $k);
  
  $function = 'imagecreatefrom' . ($ext == 'bmp' ? 'wbmp' : $ext);
  if (!$img = $function($filename)) return (!$error = 'file save error');
  
  cropImage($img, ($w-$w*$wFactor)/2, ($h-$h*$hFactor)/2, $w*$wFactor, $h*$hFactor); // narrow or shorten
  resizeImage($img, $w0 * $factor, $h0 * $factor);
  if (!imagejpeg($img, $filename, 100)) return (!$error = 'file save error');
  return TRUE;
}

function cropImage(&$img, $x, $y, $w, $h) {  
  $canvas = imagecreatetruecolor($w, $h); // crop
  $result = imagecopy($canvas, $img, 0, 0, $x, $y, $w, $h);
  // NO! imagedestroy($canvas);
  $img = $canvas;
  return $result;
}

function resizeImage(&$img, $w, $h) {
  $canvas = imagecreatetruecolor($w,$h);
  $result = imagecopyresampled($canvas, $img, 0, 0, 0, 0, $w, $h, imagesx($img), imagesy($img));
  $img = $canvas;
  return $result;
}

/**
 * Encrypt the given file and save in new path.
 * @param bool $delete: whether to delete the source file
 * @return: the encryption key (FALSE if unsuccessful)
 */
function encryptFile($filepath, $newFilepath = '', $passSalt, $delete = TRUE) {
  if ($newFilepath === '') $newFilepath = $filepath;
  $content = file_get_contents($filepath);
  $destination = dirname($newFilepath);
  if (!is_dir($destination)) mkdir($destination);

  list ($encryptedFile, $encryptionKey) = encode($content, $passSalt);
  if (file_put_contents($newFilepath, $encryptedFile)) {
    if ($delete) unlink($filepath);
    return $encryptionKey;
  } else return FALSE;
}

/**
 * Encrypt the content with the password and salt.
 * @param mixed $content: what to encrypt
 * @param assoc $passSalt: [pass]=password, [salt]=salt
 * @return array(encryptedContent, Key)

$filepath = 'zot.txt';
$newFilepath = 'zot2.txt';
$pass = 'dlfkdj';
$salt = 'sdfdlkjl';
$passSalt = compact('pass', 'salt');
$delete = TRUE;

$text = 'this is a test';
file_put_contents($filepath, $text);

  $content = file_get_contents($filepath);

  list ($encryptedFile, $encryptionKey) = rCredits\Util\encode($content, $passSalt);
  if (file_put_contents($newFilepath, $encryptedFile)) {
    if ($delete) unlink($filepath);
    $key = $encryptionKey;
  }
  
//list ($stuff, $key) = rCredits\Util\encode('this is a test', compact('pass', 'salt'));
//file_put_contents($file, $stuff);
$stuff2 = file_get_contents($newFilepath);
//// echo rCredits\Util\decode($stuff2, $key);

 */
function encode($content, $passSalt) {
  require_once __DIR__ . '/cryptastic.php';
  $cryptastic = new \cryptastic;
  $encryptionKey = $cryptastic->pbkdf2($passSalt['pass'], $passSalt['salt'], 1000, 32);
  EXPECT($encryptionKey, 'Failed to generate secret key.');
  $encryptedContent = $cryptastic->encrypt($content, $encryptionKey);
  EXPECT($encryptedContent, 'Failed to complete encryption.');
  return array($encryptedContent, $encryptionKey);
}

function decode($crypt, $key) {
  require_once __DIR__ . '/cryptastic.php';
  $cryptastic = new \cryptastic;
  $result = $cryptastic->decrypt($crypt, $key);
  EXPECT($result, 'Failed to complete decryption');
  return $result;
}

/**
 * Quicker encryption for often-used fields.
 * @param string $key: a base64-encoded encryption key (maximum 24 characters, so the resultant key is at most 16 chars)
 * @return the encrypted string (or the decrypted string with trailing nulls deleted, if $encrypt is FALSE)
 */
function ezencrypt($data, $key = R_WORD, $encrypt = TRUE) {
  if (empty($data)) return FALSE;
  u\EXPECT(compact('data'), 'string');
  $function = $encrypt ? 'mcrypt_encrypt' : 'mcrypt_decrypt';
  $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB); 
  $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND); 
  $data = $function(MCRYPT_RIJNDAEL_256, base64_decode(substr($key, 0, 24)), $data, MCRYPT_MODE_ECB, $iv); 
  return $encrypt ? $data : rtrim($data, "\0");
} 

function ezdecrypt($data, $key = R_WORD) {return ezencrypt($data, $key, FALSE);}

/**
 * Reword a Dwolla error message.
 * @param string $msg: the original message
 * @return string: the revised message
 */
function redw($msg) {
  $lame = t(' Some operations will be unavailable.');
  $map = array(
    'Access token is empty.' => t('Your account is not complete.' . $lame),
    'Request failed. Server responded with: 0' => t('Our US Dollars module is temporarily offline.' . $lame),
    'Unexpected error has occurred.' => t('Part of your current operation may not have completed. Try again?'),
    'User must be verified.' => t('You have run into a temporary roadblock, due to a recent change in rCredits signup automation. An administrator will make the necessary adjustment and will send you an email when you can proceed with connecting your bank account &mdash; probably within a few hours. We apologize for the delay.'),
  );
  return strtr($msg, $map);
}

/**
 * Return a random ascii string (good for mcrypt key).
 * @param int $len: length of result string
 * @param string $choices: restrict characters to this subset. If empty (the default), use anything but \0
 *   accepted named subsets:
 *     lower, upper, digits (the obvious)
 *     word = upper, lower, and digits
 */
function randomString($len = 16, $choices = '') {
  $lower = 'abcdefghijklmnopqrstuvwxyz';
  $upper = strtoupper($lower);
  $digits = '0123456789';
  $word = $lower . $upper . $digits;
  if (@$$choices) $choices = $$choices;
  $s = '';
  for ($i=0; $i < $len; $i++) $s .= $choices ? substr($choices, rand(0, strlen($choices) - 1), 1) : chr(rand(1, 255));
  return $s;
}

/**
 * Verify the syntax of the given name. (replacement for \user_validate_name() )
 * Accept any printable character.
 * @param string $name: the field to test
 * @return: the error message (FALSE if $name is okay)
 */
function badName($name) {
  if (strpos($name, '  ') !== FALSE) return 'multiple spaces';
  if (!ctype_print($name)) return 'illegal char';
  if (\drupal_strlen(u\shortName($name)) > USERNAME_MAX_LENGTH) return 'too long';
  return FALSE;
}

/**
 * Say whether the string date is bad.
 * @param string $s: typed date (returned with date converted to internal format, unless error)
 * @param int $min: minimum date (if any)
 * @param int $max: maximum date (if any)
 * @return FALSE if date is okay, else an error index
 */
function badDate(&$s, $min = '', $max = '') {
  if (!$dt = u\s2t(@$s)) return 'bad date';
  if ($min !== '' and ($dt < $min or $dt > $max)) {
    $err = $dt < $min ? 'date too early' : 'date too late';
    if (!is_numeric(@substr($s, -3, 1))) $err .= '|year is 4 digits';
    return $err;
  }
  $s = $dt;
  return FALSE;
}

/**
 * Parse the name, returning the first and last names separately.
 * @param string $name: the name to parse
 * @return array: [$first, $last]
 */
function parseName($name) {
  $name = trim(preg_replace('/  /', ' ', $name));
  if ($i = strpos($name, ',')) {
    $suffix = substr($name, $i);
    $name = substr($name, 0, $i);
  }
  $names = explode(' ', $name);
  $last = array_pop($names) . @$suffix;
  return array(join(' ', $names), $last);
}

/**
 * Change something to the proper case, unless it is already right.
 * @param mixed $what: a string or array of strings to fix
 * @param string $proper: the PHP function to use, to fix the case
 * @return: the argument with case fixed
 */
function normalizeCase($what, $proper = 'ucwords') {
  if (is_array($what)) {
    foreach ($what as $key => $value) $what[$key] = normalizeCase($what[$key], $proper);
    return $what;
  }
  if ($proper = 'ucwords') {
    if ($what != strtolower($what) and $what != strtoupper($what)) return $what; // already mixed case
    if (substr($what, 0, 4) == 'POB ') return $what; // special case for PO boxes
  }
  return $proper(strtolower($what));
}

define('R_SMALL_WORDS', 'of|a|the|and|an|or|nor|but|is|if|then|else|when|at|from|by|on|off|for|in|out|over|to|into|with');

/**
 * Convert a string to title case.
 */
function titleCase($s) {
  return preg_replace_callback('/\b(' . R_SMALL_WORDS . ')\b/i', function ($m) {return strtolower($m[0]);}, ucwords(strtolower($s)));
}

function gzdecode($data){
    $g=tempnam('./','gz');
    file_put_contents($g,$data);
    ob_start();
    readgzfile($g);
    $d=ob_get_clean();
    unlink($g);
    return $d;
}

/*
function weakPass($pass) {  
  return strlen($pass) < 6 
    or !preg_match('/[A-Z]/', $pass)
    or !preg_match('/[a-z]/', $pass)
    or !preg_match('/\d/', $pass)
    or !preg_match('/[^A-Za-z\d]/', $pass);
}
*/

/**
 * Return query SUM fields for each field in the list.
 * @param string $list: space-delimited list of fields
 * @param string $prefix: table (and dot) prefix for the summed field (eg SUM(u.floor) AS floor)
 */
function sumAs($list, $prefix = '') {
  foreach (u\ray($list) as $one) $result[] = "SUM(IFNULL($prefix$one, 0)) AS $one";
  return join(', ', $result);
}

function fsize($url) {
  if ($size = @filesize($url)) return $size;
  $s = file_get_contents($url);
  return strlen($s);
}

function trimAll(&$array) {
  foreach ($array as $key => $value) if (is_string($value)) $array[$key] = trim($value);
  return $array;
}

function deb($arg = '') {
  global $counterI;
  $msgCount = count(@$_SESSION['messages']['status'] ?: array());
/**/ log(($counterI = @$counterI + 1) . " ($msgCount messages): " . print_r($arg, 1), 'debug'); // keep
}

/**
 * Insert the bundle (associative array) before the needle in haystack
 * @param assoc $bundle: array to insert
 * @param assoc $haystack: array into which to insert
 * @param string $needle: key before which to insert
 * @return assoc: the resultant array (FALSE if the needle is not found)
 */
function insertBefore($bundle, $haystack, $needle) {
  if (!$i = array_search($needle, array_keys($haystack))) return FALSE;
  return array_slice($haystack, 0, $i) + $bundle + array_slice($haystack, $i);
}
 
function formatNum($num, $format) {
  $num = digits($num);
  if ($format == 'dwolla') $format = 'ddd-ddd-dddd';
  if ($format == 'ein') $format = 'dd-ddddddd';
  if ($format == 'ss') $format = 'ddd-dd-dddd';
  $oldFormat = '(' . str_replace('d', '\\d', str_replace('-', ')(', $format)) . ')'; // eg (\d\d\d)(\d\d\d)(\d\d\d\d)
  $newFormat = '$1';
  for ($i = 2; $i <= substr_count($format, '-') + 1; $i++) $newFormat .= '-$' . $i;
  return preg_replace("|\\b$oldFormat\\b|", $newFormat, $num);
}  

/**
 * Get State and City for a given postal code (using the USPS API)
 * @param string $zip: the US postal code to interpret
 * @return assoc [state->2-letter state code (or NULL), city->city name (or NULL)]
 */
function parseZip($zip) {
  $server = 'http://production.shippingapis.com/ShippingAPI.dll';
  $userId = R_USPS_WEB_TOOLS_ID;
  $zip = substr($zip, 0, 5);
  $request = <<<EOF
?API=CityStateLookup&XML=<CityStateLookupRequest USERID="$userId">
     <ZipCode ID ="0">
         <Zip5>$zip</Zip5>
     </ZipCode>
</CityStateLookupRequest>
EOF;
  if (!$result = simplexml_load_file($server . $request)) return FALSE;
  $result = (array) $result->ZipCode;
  return u\ray('state city', @$result['State'], ucwords(strtolower(@$result['City'])));
}  

/**
 * Return a random upper/lowercase code.
 * @param int $len: how long (defaults to 20)
 */
function code($len = 20) {
  global $nextCode;
  return (t\est() and @$nextCode) ? $nextCode : \user_password($len);
}

function caller($skip = 0) {
  $trace = debug_backtrace();
  for ($i = 0; $i <= $skip; $i++) array_shift($trace); // discard THIS call
  $caller = (object) array_shift($trace); $file = basename(@$caller->file);
  $precaller = (object) array_shift($trace); $prefile = basename(@$precaller->file);
/**/ $args = print_r(@$caller->args, 1);
/**/ $preargs = print_r(@$precaller->args, 1);
  return " called $caller->function($args) ($caller->line in $file), from $precaller->function($preargs) ($precaller->line in $file)\n";
}
   
function purify($s) {
  if (is_array($s)) {
    foreach ($s as $key => $value) $s[$key] = purify($value);
    return $s;
  } return preg_replace('/( [\x00-\x7F] | [\xC0-\xDF][\x80-\xBF] | [\xE0-\xEF][\x80-\xBF]{2} | [\xF0-\xF7][\x80-\xBF]{3} ) | ./x', '$1', $s);
}

/**
 * Return the name of the next business day. (for testing, all days are business days)
 * @param int $start: how many days from now to start counting.
 * @return the common speech term for the next business day starting then.
 */
function nextBusDay($start = 0) {
  return (strftime('%u', REQUEST_TIME + $start) > 5 and !t\est()) ? t('Monday')
  : ($start == 0 ? t('today') : ($start == 1 ? t('tomorrow') : strftime('%A', REQUEST_TIME + $start)));
}

// Single line functions that need little or no explanation
function nn($a, $else = '') {return is_null($a) ? $else : $a;}
function num($a, $else = 0) {return is_numeric($a) ? $a : $else;}
function eq($a, $b, $else = FALSE) {return $a == $b ? $a : $else;}
function neq($a, $b, $else = FALSE) {return $a != $b ? $a : $else;}
function nray($a, $else = FALSE) {return !is_array($a) ? $a : $else;}
function small($n) {return (abs($n) < .001);}
function equy($a, $b) {return small($a - $b);} // sort of equal
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
/**/ function prlog($log = 'rCredits') {print_r(loga($log));} // keep this
function order($one1st, $one, $two) {return $one1st ? array($one, $two) : array($two, $one);}
function both(&$vars, $k, $one1st, $one, $two) {list ($vars[$k], $vars['other' . ucwords($k)]) = u\order($one1st, $one, $two);}
function wargs($s, $args, $prefix = '') {return strtr($s, prefixKeys($prefix, $args));} // for debugging
function myInt($n) {return (is_numeric($n) and $n == (int) $n);}
function myFloat($n) {return (is_numeric($n) and $n == (float) $n);}
function isId($id) {return (myInt($id) and $id > 0);}
//function is_ruid($ruid) {return (is_array($ruid) and array_keys($ruid) == array(0, 1, 2) and isZid($ruid[0]) and isZid($ruid[1]) and isZid($ruid[2]));}
function isQid($qid) {return preg_match('/^[A-Z]{3}' . R_MARKS . '[A-Z]{3,}$/', $qid);}
function isZid($zid) {return (myInt($zid) and $zid !== 0);}
function isNonce($nonce) {return preg_match('/^[A-Z]{3,}$/', $nonce);}
function isName($value) {return preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $value);}
function isAcct($acct) {return (is_object($acct) and get_class($acct) == 'rCredits\acct');}
function hug($s, $arms = '()') {return substr($arms, 0, 1) . $s . substr($arms, -1, 1);}
function s2t($s) {return strtotime($s) ?: strtotime(str_replace('-', '/', $s));}
function fmtDate($time = REQUEST_TIME) {return strftime('%d-%b-%Y', $time);}
function fmtTime($time = REQUEST_TIME) {return strtolower(strftime('%I:%M%p', $time));}
function lastCall($far = 4) {$back = debug_backtrace(); return array_slice($back, 1, $far);}
function digits(&$n) {return $n = preg_replace('/\D/', '', @$n);}
function monthDay1($time = REQUEST_TIME) {return strtotime(strftime('%b01', $time));}
function prefix($prefix, &$var) {$var = $prefix . $var;}
function preray($prefix, &$r0) {$r = $prefix + (array) $r0; $r0 = is_object($r0) ? (object) $r : $r;}
function setDft(&$param, $value) {if (!isset($param)) $param = $value;}
function bit($bit) {return is_array($bit) ? (bit(array_shift($bit)) | ($bit ? bit($bit) : 0)) : (1 << $bit);}
function log($message, $type = 'state', $info = array()) {return u\loga($type, compact('message') + $info);}
function consta($prefix, $var) {return constant(strtoupper(str_replace(' ', '_', $prefix . "_$var")));}
function isIPhone() {return preg_match('/iPhone/i', $_SERVER['HTTP_USER_AGENT']);}
function jsonEncode($s) {return json_encode($s) ?: json_encode(u\purify($s));} // , JSON_UNESCAPED_SLASHES
function redash($s) {return str_replace('—', '&mdash;', $s);}
function backButton() {return '<button onclick="history.go(-1); return false;">Go Back</button>';}
function nonish($s) {return in_array(strtolower($s), array('n/a', 'nothing', 'na', 'none')); }
function padto($s, $n) {return str_pad(substr($s, $n), $n);}
function similarEmail($e1, $e2) {return (strtolower(preg_replace('/\+.*@/', '@', $e1)) == strtolower(preg_replace('/\+.*@/', '@', $e1)));}
//function rayo() {return (object) call_user_func_array('rCredits\\Util\\ray', func_get_args());}
function rayv($a, $i) {return $a[$i];} // useful when $a is returned from a function
function strip($a) {foreach ($a as $i => $v) $a[$i] = strip_tags($v); return $a;}
function initials($s) {return preg_replace('/[^A-Z]/', '', ucwords(strtr($s, u\ray('Jr', ''))));}
function roughName($name) {return preg_replace("/[^A-Z '\-]/", '', strtoupper(trim(str_replace('  ', ' ', $name))));}
function urlArgs() {return basename(t\est() ? t\SERVER('REQUEST_URI') : $_SERVER['REQUEST_URI']);}