<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */

namespace CG\Util; // typically abbreviated as "u"
use CG as r; // get rid of this
use CG\DB as db;
use CG\Util as u;

define ('URL_CHARS', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_~'); // 66 valid URLchars
define ('HEX_CHARS', '0123456789abcdef');

// Some useful Drupal db methods: fetchAllKeyed(m, n), fetchAssoc and fetchAllAssoc(fld), fetchObject and fetchAll(), fetchCol(n), fetchField()

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
  $amount = str_replace(R_NONE, '', $amount); // useful for monthly statements
  return preg_replace('/[ \,]/', '', $amount); // ignore spaces and commas
}

/**
 * Return an error message if the amount is bad.
 * @param string $amount: the questionable amount string (leading $ and any commas or spaces are ignored)
 *   The cleaned-up amount is returned by ref.
 * @param string $restriction: none (the default), >0, !=0, or >=0
 * @param bool $fractionLimit: maximum digits after the decimal (default 2)
 * @param numeric $max: maximum amount ('' if no maximum) (defaults to R_MAX_AMOUNT)
 * @return: FALSE if not a bad amount, else error message
 */
function badAmount(&$amount, $restriction = '', $fractionLimit = 2, $max = NULL) {
  if (!is_numeric($new = cleanAmount($amount))) return 'bad amount';

  $amount = $new;
  $dotPos = strpos($amount, '.');
  if ($dotPos !== FALSE and strlen($amount) > $dotPos + 1 + $fractionLimit) return 'too fractional';
  if (is_null($max)) $max = isGAME ? PHP_INT_MAX : R_MAX_AMOUNT;
  if ($max !== '' and abs($amount) > $max) return t('amount too big', compact('max'));
  if ($restriction == '>0' and $amount <= 0) return 'nonpositive amount';
  if ($restriction == '>=0' and $amount < 0) return 'negative amount';
  if ($restriction == '!=0' and $amount == 0) return 'zero amount';
  return FALSE;
}

/**
 * Say whether a given date of birth is bad. Convert dob to internal format.
 */
function badDob(&$dob) {
  list ($min, $max) = array(r\rTime() - 150 * 365.25 * DAY_SECS, r\rTime() + 9 * 31 * DAY_SECS);
  return u\badDate($dob, $min, $max); // badDate makes dob internal
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
function badSsn(&$ssn, $dob = '') {
//  if (in_array($ssn, ray('5551234567 777777777 888888888 999999999'))) return FALSE; // test numbers
  if (strlen(u\digits($ssn)) != 9) $bad = TRUE; // zap all but the digits (call by ref)
  if (FALSE AND @$dob) { // ssn (not ein) -- We once got a valid SSN that violated these rules, so skip this.
    $c1 = substr($ssn, 0, 1);
    $s1 = substr($ssn, 0, 3);
    $s2 = substr($ssn, 3, 2);
    $s3 = substr($ssn, 5);
    if ($c1 == '9' or $s1 == '000' or $s1 == '666' or $s2 == '00' or $s3 == '0000') $bad = TRUE;
    if (substr($ssn, 0, 8) == '98765432') $bad = TRUE; // advertising numbers, according to Wikipedia
    if ($dob < strtotime('25Jun2011')) if ($c1 == '8' or ($c1 == '7' and $s1 > '771')) $bad = TRUE;
  }
  return @$bad ? (@$dob ? 'bad ssn' : 'bad federal id') : FALSE;
}

function ssnName($v, &$firstname = '', &$middlename = '', &$lastname = '') {
  foreach (ray('firstname middlename lastname') as $k) {
    if (is_array($v->$k)) $v->$k = join(' ', $v->$k); // concatenate multiple first, middle, or last names
    if ($$k = ucWords(strtolower(str_replace('-', '', $v->$k)))) $res[] = $$k;
  }
  return @$res ? join(' ', $res) : '';
}
function ssnAddr($v) {return ucWords(strtolower(@"$v->address, $v->city, $v->state $v->zip"));}

/**
 * Say whether the given postal code is bad.
 * @param string $z: (MODIFIED) the postal code to test, returned with extraneous punctuation removed
 * @param int $country: country code (defaults to US)
 * @return the error index if the postal code is bad, otherwise FALSE
 */
function badZip(&$z, $country = US_COUNTRY_ID) {
  $z = preg_replace('/[^A-Za-z0-9\\-]/', '', $z);
  $pattern = $country == US_COUNTRY_ID ? '/^\\d{5}(\\-\\d{4})?$/' : '/*/'; // allow any postal code for other countries, for now
  return (!preg_match($pattern, $z)) ? 'bad zip' : FALSE;
}

/**
 * Say where the given address contains a PO box.
 */
function poAddress($address) {
  return (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE);
}

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * $param string $opt (default s$):
 *   1 = round to nearest dollar
 *   s = simplify: Omit the cents for whole dollar amounts
 *   $ = include initial dollar sign
 *   r = include a trailing cg
 *   u = include a trailing us
 *   % = percentage
 * @return
 *   the formatted amount
 */
function fmtAmt($amount, $opt = 's$') {
  $opt = ' ' . $opt; // simplify strpos check
  $amount0 = $amount = cleanAmount(nn($amount, 0));
  if ($pct = strpos($opt, '%')) $amount *= 100;
//  if (!is_numeric($amount)) $amount = -999999;
  u\EXPECT(compact('amount'), 'float', 'bad num');
  $amount = number_format($amount + 0, strpos($opt, '1') ? 0 : ($pct ? 1 : 2));
  if ($amount == '-0.00') $amount = strpos($opt, '1') ? '0' : '0.00'; // this happens, for small negative amounts
  if (strpos($opt, '$')) $amount = '$' . $amount;
  if (strpos($opt, 's')) $amount = preg_replace('/\\.0+$/', '', $amount);
  if (strpos($opt, 'r')) $amount .= 'cg';
  if (strpos($opt, 'u')) $amount .= 'us';
  if ($pct) $amount .= '%';
  return $amount;
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
  if (!isGAME) $fullName = mb_convert_encoding($fullName, 'UTF-8', 'UTF-8'); // avoid iconv failure (bug in PHP 5.4.3)

  $result = preg_replace('/\W/', $filler, iconv('UTF-8', 'ASCII//TRANSLIT', $fullName));
  return strtolower($result === '' ? u\code() : $result);
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function forCash($for) {
  if (is_array($for)) return forCash($for[0]) or forCash($for[1]);
  return ($for == '' or preg_match('`^(' . t('|credits|rCredits|USD|automatic transfer|cash|usd') . ')(\W|$)`i', $for)); // empty or a non-goods word at the beginning
}

/**
 * Return an alphabetic representation of the given non-negative integer.
 * A is the zero digit, B is 1, etc.
 * @param int $n: the integer to represent
 * @param int $len: the length of the string to return
 *   <=0 means length >=-$len
 * @param int $base: the radix (defaults to 26). If base is between 27 and 35, digits 1-9 are used next (no zero)
 */
function n2a($n, $len, $base = 26) {
  $result = '';
  for ($i = 0; ($len > 0 ? ($i < $len) : ($n > 0 or $i < -$len)); $i++) {
    $digit = $n % $base;

    if ($base == 36 or $base == 16) { // special case for hex or alpha+digits (start with digits)
      $result = ($digit < 10 ? $digit : chr(ord("A") + $digit - 10)) . $result;
    } else $result = ($digit < 26 ? chr(ord("A") + $digit)
    : ($digit < 36 ? ($digit + ($base < 36 ? 1 : 0) - 26)
    : chr(ord("a") + $digit)))
    . $result;
    
    $n = (int) ($n / $base);
  }
  return $result;
}

/**
 * Return the numeric equivalent of the given alphabetic string. 
 * In base 27 to 35, zeros are understood as Os (so "1" comes after "Z").
 * @see n2a
 */
function a2n($string, $base = 26) {
  if (26 < $base and $base < 36) $string = str_replace('0', 'O', strtoupper($string));
  $result = 0;
  for ($i = 0; $i < strlen($string); $i++) {
    $n = ord($string[$i]);
    if ($base == 36 or $base == 16) {
      $result = $result * $base + $n - ($n >= ord("A") ? ord("A") - 10 : ord("0"));
    } else $result = $result * $base + $n - 
      ($n >= ord("a") ? ord("a") - 36
       : ($n >= ord("A") ?  ord("A") 
       : ord($base < 36 ? "1" : "0") - 26)
      );
  }
  return $result;
}

/**
 * Return a radix-35 version of the number (and vice versa) for invitation codes.
 * @param int $n: the number
 * @param int $len: length (or minimum length -- see n2a above)
 */
function n2ai($n, $len) {return n2a($n, $len, 35);}
function ai2n($string) {return a2n($string, 35);}

/**
 * Return a radix-62 version of the number (and vice versa) for emailed URL codes.
 */
function n2ae($n, $len) {return n2a($n, $len, 62);}
function ae2n($string) {return a2n($string, 62);}

/**
 * Encode 3 hex digits as 2 legal URL characters: [A-Za-z0-9.-]
 * @param string $s: the three digits to convert
 * u\x32a2(u\a22x3('B7'));
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
 * Return an HTML attribute string for the given array.
 * @param assoc $ray: list of attribute names and values (value can be an array)
 */
function tribs($ray) {
  foreach ($ray as $k => $v) if (!is_null($v)) {
    $tribs[$k] = "$k=\"" . plain(is_array($v) ? join(' ', $v) : $v) . '"';
  }
  return join(' ', @$tribs ?: []);
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
 *   '-'  ddd-ddd-dddd
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
define('MAX_PHONE_DIGITS', 15);
define('MIN_PHONE_DIGITS', 7);

function fmtPhone($phone, $how = '+') {
  $sep = $how == '-' ? '-' : ' '; // digit group separator
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('~[\(\) \-\./]~', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
  } elseif (substr($phone, 0, 1) == '1') {
    $phone = substr($phone, 1);
  } elseif (substr($phone, 0, 1) == '+') $foreign = $phone = substr($phone, 1);

  if (!is_numeric($phone) or (!u\test() and strlen($phone) < MIN_PHONE_DIGITS) or strlen($phone) > MAX_PHONE_DIGITS) return FALSE;
  if (!$foreign and substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
  if (!$foreign and strlen($phone) != 10) return FALSE;

  if ($how == 'n') return $phone;
  if ($how == '+n' or $how == 'internal') return ($foreign ? "+$foreign" : "+1$phone");

  $phone = substr($phone, 0, 3) . $sep . substr($phone, 3, 3) . $sep . substr($phone, 6);
  if ($how == '-') return $phone;
  if ($how == '+' or $how == '') {
    $len = substr($foreign, 0, 1) == '7' ? 1 // get length of country prefix
      : (strpos('. 21 22 23 24 25 26 29 35 37 38 42 51 59 67 68 69 85 87 88 96 97 99 ', ' ' . substr($foreign, 0, 2) . ' ') ? 3 : 2);
    return $foreign ? substr($foreign, 0, $len) . ' ' . substr($foreign, $len) : ($how ? "+1 $phone" : $phone);
  }
  return FALSE;
}

/**
 * Return the array with its keys prefixed by the given string (and maybe suffixed, too)
 * @param string $prefix: what to prefix each key with
 * @param assoc $ray: the array
 * @param bool $noArray: <make sure no element of the result is an array>
 * @param string $suffix: optional suffix for each key
 */
function prefixKeys($prefix, $ray, $noArray = FALSE, $suffix = '') {
//  EXPECT(compact('prefix', 'ray'), 'string array');
  if ($prefix == '') return $ray;
  $newRay = [];
/**/ foreach ($ray as $k => $v) $newRay[$prefix.$k.$suffix] = ($noArray and is_array($v)) ? print_r($v, 1) : $v;
  return $newRay;
}

function prefixValues($prefix, $ray) {
  foreach ($ray as $k => $v) $ray[$k] = $prefix . $v;
  return $ray;
}

function changeKey($oldKey, $newKey, &$ray) {
  $ray[$newKey] = $ray[$oldKey];
  unset($ray[$oldKey]);
  return $ray;
}

/**
 * Return the difference between two arrays (better than array_diff_assoc)
 */
function diff($a, $b) {
  $result = [];
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
 * @param assoc $subs: (optional) parameter values keyed by name
 * @param string $mark: parameter mark (normally '@') will be prepended to the keys.
 * @todo get the r\'s out of this utilities file OR stop trying to separate u\ from r\
 */   /*
function t($message, $subs = [], $adhocChannel = '', $mark = '@') {
  global $channel;
 
  if (!$adhocChannel) $adhocChannel = $channel;
  if (is_array($message)) list ($message, $subs) = $message;
  u\EXPECT(!is_array($message), 'message array');
  if (!$subs) $subs = []; // accept ''
  $messages = explode('|', $message);
  $channels = ray(TX_CHANNELS);

  foreach ($messages as $i => $message) {
    $ch1 = $adhocChannel ?: (count($channels) - 1);
    for ($ch = $ch1; $ch > 0; $ch--) if ($model = @$GLOBALS["$ch messages"][$message]) break;
    if (!@$model) $model = $message;
    if($p = strpos($model, '. Type ')) {
      if ($channel != TX_SMS and $adhocChannel != TX_SMS) $model = substr($model, 0, $p + 1); // don't tell the user to type something unless it's SMS
    }
    if (@$subs['href']) $model = str_replace('<a>', '<a href="@href">', $model);
// NO (use raw message instead)   EXPECT(@$model, "missing message '$message'"); 
//    u\EXPECT(!is_array(@$subs['message']), 'subs message array');
//    u\EXPECT(!is_array($model), 'model array');
//    u\EXPECT(!is_array($i), 'i array');
    $messages[$i] = SUBS(strtr($model, prefixKeys($mark, $subs, 'noArray')), $mark);
  }
  return str_replace(' .', '.', join(' ', $messages));
}
*/

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 * @todo: get these r\'s out of here.
 */
function nonce($minLen = 1) {
  list ($where, $subs) = array('LENGTH(nonce)>=:minLen', compact('minLen'));
  $offset = db\get('FLOOR(RAND() * COUNT(1))', 'r_nonces', $where, $subs);
  $nonce = db\get('nonce', 'r_nonces', "$where LIMIT $offset, 1", $subs);

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
function post($url, $content = FALSE, $responseFormat = 'raw', $extra_headers = [], $method = 'POST', $request_format = 'url') {
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
  $response = FALSE;
  if ($fp = @fopen($url, 'rb', FALSE, $context)) {
    // If you're troubleshooting, uncomment the next line to see the HTTP response headers across all redirects:
    // $meta = stream_get_meta_data($fp); var_dump($meta['wrapper_data']);
    $response = stream_get_contents($fp);
  } elseif (u\test()) return FALSE;

//  EXPECT($response !== FALSE, "$method $url failed");
  if ($response === FALSE) r\tellAdmin("$method $url failed");
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
 * For example, SUBS("SELECT * FROM table WHERE zot IN (:R_THIS, :R_THAT)") fills in the constants.
 * Constants of the form "IS_WHATEVER" are a special case. They are taken to mean the corresponding
 *   bit number B_WHATEVER in a field called $flagField. Identifiers of the form x.:IS_WHATEVER are permitted.
 *   NOTE to developer: this could be expanded later to handle a second flag field named {$flagField}2
 *     by changing the expression to "IF(:B_$1<32,$flagField&(1<<:B_$1)<>0,{$flagField}2&(1<<(:B_$1-32))<>0)".
 * @param string $string: the string to fix
 * @param string $flagField: name of the flags field for bit constants
 * @return string: the string with constant names replaced by their values
 */
function SUBS($string, $prefix = ':', $flagField = 'flags') {
  if ($prefix == ':') $string = preg_replace("/([a-z]+[0-9]?\.)?{$prefix}IS_([A-Z0-9_]*)/ms", "($1$flagField&(1<<:B_$2)<>0)", $string); // only for queries
//  $string = preg_replace("/\\{$prefix}IS_([A-Z0-9_]*)/ms", "($flagField&(1<<:B_$1))", $string);
  $subs = constantSubs($string, $prefix);
  return $subs ? strtr($string, $subs) : $string;
}

/**
 * Assert an expectation or check parameters for validity
 * Throw an Exception if the expectation is not met.
 * call by:
 *   EXPECT($assertion, $message, $ret) OR
 *   EXPECT($args, $types, $ret) eg EXPECT(compact(ray('arg1 arg2')), 'bool float');
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
    list ($args, $types) = array($assertion, $message); // rename for clarity
///    EXPECT(compact(ray('args types')), 'assoc string'); // can't do this because infinite recursion
///    EXPECT(count($args) != count($types = explode(' ', $types)), 'wrong number of types expected'); // infinite recursion
    $vs = explode(' ', $types);
/**/ if (isDEV and count($args) != count($vs)) debug(debug_backtrace());
    $types = array_combine(array_keys($args), $vs); // not ray() here, $types may have '|'
    foreach ($args as $key => $value) if ($error = expect1($key, $value, $types[$key], $ret)) return $error;
  } elseif (!$assertion) {
    if ($ret) return $message; else throw new \Exception($message);
//    EXPECT(compact(ray('assertion message')), 'bool string&notempty'); // can't do this because infinite recursion
  }
  return ''; // no error
}  

function expect1($arg, $value, $type, $ret) {
//  static $ecnt; $ecnt = @$ecnt + 1; u\EXPECT($ecnt < 10000, 'loop');
//  EXPECT(compact(ray('arg type')), 'var string'); // can't do this because infinite recursion

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
      foreach ($value as $key => $one) expect1("$arg key", $key, 'var', $ret);
      return;
    case 'bool': return EXPECT(is_bool($value) or $value == 0 or $value == 1, $msg, $ret);
    case 'email': return EXPECT(\valid_email_address($value), $msg, $ret);
    case 'empty': return EXPECT(empty($value), $msg, $ret);
    case 'float': return EXPECT(myFloat($value), $msg, $ret); // don't use is_float()?
    case 'id': return EXPECT(isId($value), $msg, $ret);
    case 'int': return EXPECT(myInt($value), $msg, $ret); // don't use is_int()
    case 'name': return EXPECT(isName($value), $msg, $ret);
    case 'var': return EXPECT(isVarname($value), $msg, $ret);
    case 'notempty': return EXPECT(!empty($value), $msg, $ret);
    case 'notnull': return EXPECT(!is_null($value), $msg, $ret);
    case 'qid': return EXPECT(isQid($value), $msg, $ret);
    case 'string': return EXPECT(is_string($value), $msg, $ret);
    case 'zid': return EXPECT(isZid($value), $msg, $ret);
    default: return EXPECT(is_object($value) and strtolower(get_class($value)) == "cg\\$type", $msg, $ret);
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
 * @param mixed $info: the information to log (string or assoc)
 * @param int $adhocChannel: a second channel used from within the primary channel
 * @return the log record id (for exception handler and shutdown function)
 */  
function loga($type, $info = [], $adhocChannel = FALSE) {
  global $mya;
/**/  if (strlen($type) >= 60) die('in loga: ' . $type); // (KEEP) don't use EXPECT here
//  if ($type == 'FATAL' and strpos($info, 'calling db_query() ')) return '';

  global $channel;
  $channels = ray(TX_CHANNELS);
  $ch = @$channel ? $channels[$channel] : t('hack');
//  $useTextLogToo = FALSE; // isDEV;

  $qid = @$mya ? $mya->qid . (($mya->cAdmin2 and !$mya->proSe) ? strtolower($mya->agentA->mainQid) : '') : t('nobody');
  $time = date('d H:i:s');

  if (is_array($info)) {
    $map = ["\r" => '', "\n" => '', '    ' => ' '];
    if ($msg = @$info['message']) $info['message'] = strtr(strip_tags(str_replace('>', '>|', $msg)), $map);
    if (@$info['form_id']) {$type .= str_replace('CG\\Web\\form', '', $info['form_id']);}
    if (@$info['toEmail']) $info = just('index subject', $info); // don't show whole email
    foreach (ray('form_id form_build_id form_token orig') as $k) unset($info[$k]);
//    foreach (ray(R_NOLOG) as $k) if ($k != 'email' and @$info[$k]) $info[$k] = '(?)';
  //  $record = compact(ray('time channel type myid agent info'));
    $info = json_encode(u\noSecrets($info), JSON_UNESCAPED_SLASHES);
  }
/**/ flog("$qid/$time/$ch/$type/$info");
}  

/**
 * Return an email a tag.
 */
function emailTag($email = NULL, $subject = '', $body = '', $name = '', $extra = '') {
  u\setDft($email, r\regionField('email'));
  if ($name) $email = urlencode("\"name\"") . ' ' . urlencode("<$email>");
  $res = "mailto:$email";
  if ($subject) $res .= '?subject=' . urlencode($subject);
  if ($body) $res .= '&body=' . urlencode($body);
  return "a href=\"$res\" target=\"_blank\" $extra";
}

function seeAssoc($info) {
  $result = '';
/**/ foreach ($info as $key => $value) $result .= "<b>$key</b>: " . print_r($value, 1) . "<br>\n"; // keep
  return "\n<br>$result";
}

/**
 * Return the distance between two points on Earth.
 * @param
 * @param float $lat1, $lon1 = Latitude and Longitude of point 1 (in decimal degrees)
 * @param float $lat2, $lon2 = Latitude and Longitude of point 2 (in decimal degrees)
 * @param string $unit = the unit you desire for results                             
 *                    where: 'M' is statute miles                      
 *                           'K' is kilometers (default)                                 
 *                           'N' is nautical miles  
 */ 
function distance($lat1, $lon1, $lat2, $lon2, $unit = 'K') {
  $theta = $lon1 - $lon2;
  $dist = sin(deg2rad($lat1)) * sin(deg2rad($lat2)) +  cos(deg2rad($lat1)) * cos(deg2rad($lat2)) * cos(deg2rad($theta));
  $dist = acos($dist);
  $dist = rad2deg($dist);
  $miles = $dist * 60 * 1.1515;
  $unit = strtoupper($unit);

  if ($unit == 'K') return ($miles * 1.609344);
  if ($unit == 'N') return ($miles * 0.8684);
  if ($unit == 'M') return $miles;
  return FALSE;
}

/**
 * Look up the given SSN and return xml data.
 */
function ssnLookup($ssn) {
  $xml = file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . '/.ssncheck');
  $xml = strtr($xml, ray('[ssnUser] [ssnPass] [ssn]', R_SSN_USER, R_SSN_PASS, $ssn));
  if (NOT_PRODUCTION) return @file_get_contents('ssnresult.txt'); // testing
  return @file_get_contents("https://www.nationalpublicdata.com/feeds/FDSFeed.cfm?xml=" . urlencode($xml));
}

/**
 * Parse the bank ID number (BIN) into a US routing number and account number.
 * @param string $bankAccount: the international bank account number
 * @return array [$routing, $account]
 */
function parseBankAccount($bankAccount) {
  $route = substr(@$bankAccount, 4, 9); // chop of USkk
  $account = substr(@$bankAccount, 4 + 9); // everything after the routing number
  return [$route, $account];
}

/**
 * Return the first unused id.
 * @param string $key: name of field to search
 * @param string $table: table name
 * @param int $start: first id to use (defaults to 1)
 * @param int $end: last id to use (if any)
 * @param string $where: other criteria
 * @param assoc $subs: substitutions for the other criteria
 * @return: the id (null if none available)
 */
function firstUnusedId($key, $table, $start = 1, $end = 0, $where0 = '1', $subs = []) {
  if (!$end) $end = PHP_INT_MAX - 1;
  if (!db\exists("$table t1", "$where0 AND t1.$key=$start", $subs)) return $start;
  list ($DESC, $le, $offset) = $start > $end ? ['DESC', '>=', -1] : ['', '<=', 1];    
  $where = "$where0 AND $start $le t1.$key AND t1.$key $le $end AND t2.$key IS NULL ORDER BY t1.$key $DESC";
  $beyond = $DESC ? "-1+" : "1+";
  $prev = db\get("t1.$key", "$table t1 LEFT JOIN $table t2 ON t2.$key=(t1.$key + $offset)", $where, $subs);
  return $prev ? ($prev + $offset) : NULL;
}
 
/**
 * Alter an image by resizing, cropping, translation, and/or rotation.
 * @param image $img0: the image to change
 * @param int ($w, $h): new width and height
 * @param int ($x1, $y1): where to begin cropping
 * @param int ($cropW, $cropH): how big an image to end up with
 * @param int $angle: how far to rotate the image (in degrees)
 * @return the modified image
 */
function alterImg($img0, $w, $h, $x1 = 0, $y1 = 0, $cropW = 0, $cropH = 0, $angle = 0) {
  list ($w0, $h0) = [imagesx($img0), imagesy($img0)];
  
  if ($w0 != $w or $h0 != $h) { // resize
    $img1 = imagecreatetruecolor($w, $h); // resize the original image to size of editor
	  imagecopyresampled($img1, $img0, 0, 0, 0, 0, $w, $h, $w0, $h0);
  } else $img1 = $img0;

  if ($angle) { // rotate
    $img1 = imagerotate($img1, -$angle, 0); // rotate the resized image
    $x1 += (imagesx($img1) - $w) / 2; // diff between rotated & original sizes
    $y1 += (imagesy($img1) - $h) / 2;
  }
	
  if ($cropW or $cropH) { // crop
	  $img2 = imagecreatetruecolor($cropW, $cropH); // crop image into selected area
	  imagecolortransparent($img2, imagecolorallocate($img2, 0, 0, 0));
	  imagecopyresampled($img2, $img1, 0, 0, $x1, $y1, $cropW, $cropH, $cropW, $cropH);
  } else $img2 = $img1;
  
  return $img2;
}

define('CRYPT_FLAG', '`!'); // flag start of encrypted data (add a single type letter)

/**
 * Perform the given type of encryption (or decryption).
 * @param char $type: purpose of the encryption / decryption, as follows (each has its own methods, password, and IV):
 *   R regional admin private key for very secure data (the key is encrypted on the admin's flash drive)
 *   V very secure data (SSNs, etc.)
 *   S standard secure data
 *   P searchable data (last step of encryption is b64)
 *   C cookie
 *   H photo
 * @param string $data: the data to be encrypted / decrypted
 * @param bool $de: <decrypt rather than encrypt>
 * @param string $pwInstead: password to use instead of the one in the password file (for asymmetric encryption or decryption)
 * @return the encrypted (or decrypted) data.
 * TEST with foreach (ray('S V R C P H') as $k) print "k=$k " . u\decry($k, u\cry($k, 'abcd', FALSE, $k == 'R' ? a(1)->vKeyPw : ''), $k == 'V' ? r\vKey() : ($k == 'R' ? a(1)->vKeyPw : ''));
 */
function cry($type, $data, $de = FALSE, $pwInstead = '') {
	global $cryHows, $mya;
	
	if ($de) u\EXPECT(u\starts($data, CRYPT_FLAG . $type), 'bad crypted');
	if ($de) {
    if ($type == 'V' and !$vKey = r\vKey()) return $data; // if we can't decrypt, return the data as is (for acct::recordChanges)
    $data = substr($data, strlen(CRYPT_FLAG . $type));
  }

	$chow = $cryHows[$type];
	foreach ($chow as $i => $v) if ($chow0 = @$cryHows[$v]) $chow[$i] = $chow0[$i]; // allow reference to other how
	list ($hows, $pw, $iv) = $chow;
	$pw = u\b64decode($pwInstead ?: $pw);
	u\EXPECT($pw, 'missing pw in cry');
	
	if ($iv == 'NONCE') {
		if ($de) {
			$iv = substr($data, 1, $nonceLen = ord($data[0]));
			$data = substr($data, 1 + $nonceLen);
		} else {
    	$nonceLen = 37; // arbitrary length of nonce (must be >= 32)
			$iv = openssl_random_pseudo_bytes($nonceLen);
			$nonce = chr($nonceLen) . $iv; // since our IVs also serve as an arbitrary length mask, record its length
		}
  } else $iv = u\b64decode($iv);
	
	$hows = $de ? array_reverse(ray($hows)) : ray($hows);
	
	foreach ($hows as $how) {
    switch ($how) {
			case 'xor': $data = strXor($data, $pw . $iv); break; // deXor is the same
			case 'rot': $data = strRotate($data, min(PHP_INT_MAX, hexdec(bin2hex(substr($pw, 0, 4)))) * ($de ? -1 : 1)); break;
			case 'scram': $data = strScramble($data, $pw . $iv, $de); break;
			case 'b64': $data = $de ? u\b64decode($data) : u\b64encode($data); break; // final encrypt step if present
			case 'pgp':
				$trashLen = 30; // do not change this, without re-encrypting everything encrypted with its previous value
				// NOTE: whether this number is 0 or 200 makes ZERO difference in the strlen of the encrypted value 30 is plenty
				if ($de) { // decrypt with supplied private key
  			  $success = openssl_private_decrypt($data, $res, u\fmtKey($vKey, 'PRIVATE')) ? $res : FALSE;
///					if (!$success) die(print_r(debug_backtrace(), -1));
				  $data = $success ? substr_replace($res, '', -$trashLen) : '';
				} else { // encrypt with public key
  				$data .= openssl_random_pseudo_bytes($trashLen); // add randomness
    			$success = openssl_public_encrypt($data, $res, u\fmtKey($pw, 'PUBLIC')) ? $res : FALSE;
					$data = $res;
				}
				break;
			default: $data = crypt($data, $how, $pw, $iv, $de); // any standard AES
		}
	}

	return $de ? $data : (CRYPT_FLAG . $type . @$nonce . $data);
}

function crypted($type, $s) {return u\starts($s, CRYPT_FLAG . $type);}

function decry($type, $data, $pwInstead = '') {
	return u\starts($data, CRYPT_FLAG) ? cry($type, $data, TRUE, $pwInstead) : $data;
}

// NEW encryption
function crypt($data, $how, $key, $iv, $decrypt = FALSE) {
  if (empty($data)) return FALSE;
  u\EXPECT(compact(ray('data how key iv')), 'string string string string');

  $ivSize = openssl_cipher_iv_length($how);
	$iv = substr($iv, 0, $ivSize);
  $key = substr($key, 0, 32); // 32 bytes for 256 bits

  $function = $decrypt ? 'openssl_decrypt' : 'openssl_encrypt';
  $data = $function($data, $how, $key, 0, $iv);
  return $data;
} 

/**
 * Initiate CSV download.
 * @param string $filename: pathname of the file that will download itself upon exit()
 */
function csvStart($filename) {
  global $csvFile;
  global $testCSVname; $testCSVname = $filename;
  global $testCSV; $testCSV = [];

  if (u\test()) return;  
  header('Content-type: text/plain');
  header("Content-Disposition: attachment; filename=\"$filename\";");
  header('Content-Type: application/csv; charset=UTF-8'); // tell the browser it's going to be a csv file
  $csvFile = fopen('php://output', 'w');
}

/**
 * Output to the csv file.
 * @param array $a: the line to output
 * Note we use fputs to work around PHP's inability to omit enclosure for fputcsv.
 */
function csv($a, $delimiter = ',') {
  global $csvFile, $testCSV;
  if (u\test()) {$testCSV[] = $a; return;}
  if ($delimiter == ',') fputcsv($csvFile, $a, $delimiter); else fputs($csvFile, join($delimiter, $a) . "\n");
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
  global $nextCode;
  if (u\test() and @$nextCode) return $nextCode;

  $lower = 'abcdefghijklmnopqrstuvwxyz';
  $upper = strtoupper($lower);
  $digits = '0123456789';
  $word = $lower . $upper . $digits;

  if (@$$choices) $choices = $$choices;
  $chlen = strlen($choices);

  for ($s = '', $i = 0; $i < $len; $i++) {
    do {$c = openssl_random_pseudo_bytes(1);} while ($choices and ord($c) >= $chlen); // get a random character or index
    $s .= $choices ? $choices[ord($c)] : $c;
  }
  return $s;
}

/**
 * Verify the syntax of the given name. (replacement for \user_validate_name() )
 * Accept any printable character.
 * @param string $name: the field to test (RETURNED) field with minor errors fixed
 * @return: the error message (FALSE if $name is okay)
 */
function badName(&$name) {
  $name = trim(preg_replace('/ +/', ' ', $name));
  if ($name and !ctype_print($name)) {
/**/  r\tellAdmin('bad char', ray('name trace', bin2hex($name), trace()));
    return 'illegal char';
  }
  if (strlen(u\shortName($name)) > USERNAME_MAX_LENGTH) return 'too long';
  return FALSE;
}

/**
 * Say whether the string date is bad.
 * @param string $s: typed date (returned with date converted to internal format, unless error)
 * @param int $min: minimum date (if any)
 * @param int $max: maximum date (if any)
 * @return FALSE if date is okay, else an error index
 */
function badDate(&$s, $min = '', $max = PHP_INT_MAX) {
  if (!$dt = u\s2t(@$s)) return 'bad date'; // slashes makes it mdy (else dmy assumed)
  if ($min !== '' and ($dt < $min or $dt > $max)) {
    $err = $dt < $min ? 'date too early' : 'date too late';
    if (!is_numeric(@substr($s, -3, 1))) $err .= '|year is 4 digits';
    return $err;
  }
  $s = $dt;
  return FALSE;
}

/**
 * Return a table row with the given data (header or not).
 * @param mixed $row: an array with field names or assoc with field names and values
 */
function tableRow($row) {
  if ($th = is_numeric(key($row))) $row = array_flip($row);
  $m = $th ? 'th' : 'td';
  $res = '';
  foreach ($row as $k => $v) {
    $res .= tr('<%m class="k-%k">%v</%m>', 'm k _v', $m, $k, $th ? ucwords($k) : $v);
  }
  return "<tr>$res</tr>\n";
}
  
/**
 * Parse the name, returning the first and last names separately. Underlines in last name are treated as spaces.
 * @param string $name: the name to parse
 * @param bool $withMiddles: <separate out the middle name(s)>
 * @return array: [$first, $last] or [$first, $last, $middles]
 */
function parseName($name, $withMiddles = FALSE) {
  $name = trim(preg_replace('/  /', ' ', $name));
  if ($i = strpos($name, ',')) {
    $suffix = substr($name, $i);
    $name = substr($name, 0, $i);
  } elseif (preg_match('/( Jr?)$/', $name, $match)) { // handle common suffixes with no comma
    $suffix = $match[1];
    $name = preg_replace('/( Jr?)$/', '', $name);
  }
  $names = explode(' ', $name);
  $last = str_replace('_', ' ', array_pop($names) . @$suffix);
  if ($withMiddles) {
    $first = array_shift($names);
    return [$first, $last, join(' ', $names)];
  } else return [join(' ', $names), $last];
}

/**
 * Parse the mailing address into its constituent parts.
 * @param string $address: the address to parse
 * @return [address, city, state, zip, $country] (one or more will be empty or ER, if error)
 */
function parseAddr($address) {
  $parts = explode(',', $address);
  if (!$p = count($parts)) return ['', '', '', ''];
  list ($state, $zip, $country) = explode(' ', strtoupper(@trim(array_pop($parts))) . '  ');
  if (!$country and !preg_match('/^[A-Z][A-Z]$/', $state)) $state = 'ER';
  $city = trim(array_pop($parts));
  $address = join(', ', $parts);
  return [$address, $city, $state, $zip, $country];
}

/**
 * Change something to the proper case, unless it is already right.
 * @param mixed $what: a string or array of strings to fix
 * @param string $proper: the PHP function to use, to fix the case
 * @return: the argument with case fixed, utf8-encoded (ready to add to database)
 */
function normalizeCase($what, $proper = 'ucwords') {
  if (is_array($what)) {
    foreach ($what as $key => $value) $what[$key] = normalizeCase($what[$key], $proper);
    return $what;
  }
  if ($proper = 'ucwords') {
    if ($what != strtolower($what) and $what != strtoupper($what)) return utf8_encode($what); // already mixedcase
    if (substr($what, 0, 4) == 'POB ') return $what; // special case for PO boxes
  }
  return utf8_encode($proper(strtolower($what)));
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
  foreach (ray($list) as $one) $result[] = "SUM(IFNULL($prefix$one, 0)) AS $one";
  return join(', ', $result);
}

function fsize($url) {
  if ($size = @filesize($url)) return $size;
  $s = file_get_contents($url);
  return strlen($s);
}

function trimAll(&$ray) {
  foreach ($ray as $key => $value) if (is_string($value)) $ray[$key] = trim($value);
  return $ray;
}

function deb($arg = '') {
  global $counterI;
  $msgCount = count(@$_SESSION['messages']['status'] ?: []);
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
  if ($format == 'ein') $format = 'dd-ddddddd';
  if ($format == 'ss') $format = 'ddd-dd-dddd';
  $oldFormat = '(' . str_replace('d', '\\d', str_replace('-', ')(', $format)) . ')'; // eg (\d\d\d)(\d\d\d)(\d\d\d\d)
  $newFormat = '$1';
  for ($i = 2; $i <= substr_count($format, '-') + 1; $i++) $newFormat .= '-$' . $i;
  return preg_replace("|\\b$oldFormat\\b|", $newFormat, $num);
}  

/**
 * Return a table display of an array.
 * @param assoc $ray: the array
 * @param bool $across: if TRUE (the default) show labels on top, otherwise on the left
 * @param bool $recurse: show array elements as another table
 * @return HTML for a table displaying the labeled values
 */
function rayTable($ray, $across = TRUE, $recurse = FALSE) {
  $heads = $values = $lines = '';
  foreach ($ray as $k => $v) {
    if (is_array($v) and $recurse) $v = rayTable($v, !$across, TRUE);
    $v = plain($v);
    if ($across) {
      $heads .= "<th>$k</th>";
      $values .= "<td>$v</td>";
    } else $lines .= "<tr><th>$k</th><td>$v</td></tr>\n";
  }
  if ($across) $lines = "<tr>$heads</tr>\n<tr>$values</tr>";
  return "<table>\n$lines</table>\n";
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
  return ray('state city', @$result['State'], ucwords(strtolower(@$result['City'])));
}  

/**
 * Return a random upper/lowercase code.
 * @param bool $upper: use only uppercase
 * @param int $len: how long
 */
function code($upper = FALSE, $len = R_CODE_LEN) {
  global $nextCode;
  if (u\test() and @$nextCode) return $nextCode;
  $res = u\randomString($len, 'word');
  return $upper ? strtoupper($res) : $res;
}

function showCaller($skip = 0) {
  $trace = debug_backtrace();
  for ($i = 0; $i <= $skip; $i++) array_shift($trace); // discard THIS call
  $caller = (object) array_shift($trace); $file = basename(@$caller->file);
  $precaller = (object) array_shift($trace); $prefile = basename(@$precaller->file);
/**/ $args = print_r(@$caller->args, 1);
/**/ $preargs = print_r(@$precaller->args, 1);
  return " called $caller->function($args) ($caller->line in $file), from $precaller->function($preargs) ($precaller->line in $file)\n";
}

/**
 * Show what bits are set in the given integer.
 * @param int $n: the integer bit array
 * @param string $descs: string array of bit names ('': use bit numbers, default: flag bit names)
 * @return: a list of the bits set n the integer
 */ 
function bits($n, $descs = B_LIST) {
  $res = '';
  if ($descs) {
    $descs = ray($descs);
    if (!is_numeric(@key($descs))) $descs = array_keys($descs); // ignore extra info after colon in bit name
  }
  for($i = 0; $i < 32; $i++) if ($n & (1 << $i)) $res .= ' ' . ($descs ? $descs[$i] : $i);
  return $res;
}

/**
 * Tell the browser to download subsequent output. (end with exit())
 * @param string $filename: what to name the downloaded file
 */
function beginDownload($filename) {  
  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=$filename");
  header("Content-Type: application/octet-stream");
  header("Content-Transfer-Encoding: binary");
}

function purify($s) {
  if (is_array($s)) {
    foreach ($s as $key => $value) $s[$key] = purify($value);
    return $s;
  } else return preg_replace('/( [\x00-\x7F] | [\xC0-\xDF][\x80-\xBF] | [\xE0-\xEF][\x80-\xBF]{2} | [\xF0-\xF7][\x80-\xBF]{3} ) | ./x', '$1', $s);
}

/**
 * Return the obvious WHERE clause for the given fields (fieldname1=:fieldname1 AND ...).
 */
function where($ray) {
  if (!is_numeric(key($ray))) $ray = array_keys($ray);
  foreach ($ray as $k) $ret[] = "$k=:$k";
  return join(' AND ', $ret);
}

/**
 * Return the name of the next business day. (for testing, all days are business days)
 * @param int $start: how many days from now to start counting.
 * @return the common speech term for the next business day starting then.
 */
function nextBusDay($start = 0) {
  return (strftime('%u', r\rTime() + $start) > 5 and !u\test()) ? t('Monday')
  : ($start == 0 ? t('today') : ($start == 1 ? t('tomorrow') : strftime('%A', r\rTime() + $start)));
}

define('ENGLISH_ONES', t(' One Two Three Four Five Six Seven Eight Nine Ten Eleven Twelve Thirteen Fourteen Fifteen Sixteen Seventeen Eighteen Nineteen'));
define('ENGLISH_TENS', t(' X Twenty Thirty Forty Fifty Sixty Seventy Eighty Ninety'));
define('ENGLISH_LIONS', t(' Thousand Million Billion Trillion Quadrillion Quintillion Sextillion Septillion Octillion Nonillion'));

/**
 * Return the number as an anglicized string.
 * @param int $num: the number to convert.
 * @param int $tri: recursion depth (handle three digits at a time)
 */
function n2english($num, $tri = 0) {
  if ($num < 0) return t('Negative ') . n2english(-$num);
  if ($num == 0) return t('Zero');
 
  $ones = ray(ENGLISH_ONES);
  $tens = ray(ENGLISH_TENS);
  $lions = ray(ENGLISH_LIONS);
  $r = floor($num / 1000); // chunk the number, ...rxyy
  $x = ($num / 100) % 10;
  $y = $num % 100;

  $res = ''; // the result

  if ($x > 0) $res = $ones[$x] . t(' Hundred '); // do hundreds
  if ($y >= 20) { // ones and tens
    $res .= $tens[floor($y / 10)];
    if ($y = $y % 10) $res .= '-' . $ones[$y];
  } elseif ($y > 0) $res .= $ones[$y]; // just ones
  if ($res != '') $res .= ' ' . $lions[$tri]; // add triplet modifier only if there is something left

  return str_replace('  ', ' ', trim($r > 0 ? n2english($r, $tri + 1) . ' ' . $res : $res));
}

/**
 * Say whether one string abbreviates another.
 * @param string $starts: the possible abbreviation
 * @param string $s: the possibly abbreviated string
 * @param bool $noCase: <ignore case>
 * @param int $starting: earliest location the potential match can start (0 for starting, -strlen($starts) for ending)
 * @return <the first string abbreviates the second>
 */
function abbreviates($starts, $s, $noCase = FALSE, $starting = 0) {
  if (!@$s or is_numeric($s)) $s = @$s . '';
  u\EXPECT(compact('starts', 's', 'noCase'), 'string string bool');
  if ($noCase) list ($starts, $s) = array(strtolower($starts), strtolower($s));
  return (substr($s, $starting, strlen($starts)) == $starts);
}
function starts($s, $starts, $noCase = FALSE) {return abbreviates($starts, $s, $noCase);} // near synonym
function ends($s, $ends, $noCase = FALSE) {return abbreviates($ends, $s, $noCase, -strlen($ends));}

/**
 * Return the time with some number of months added (or subtracted)
 * @param int $n: how many months to add (may be negative)
 * @param int $time: starting time (defaults to current time)
 * @return int: the resulting time, same day of month if possible, otherwise last day of month.
 * strtotime() should do this, but it actually returns March 2nd for strtotime('-1 month', strtotime('3/30/2014'))
 */
function plusMonths($n, $time = '') {
  if ($time === '') $time = time();
  if ($n > 0) $n = '+' . $n;
  $res = strtotime($n . ' months', $time);
  $day = date('d', $res);
  return $day == date('d', $time) ? $res : strtotime(-$day, $res); // use last day of month if same day fails
}

/**
 * XOR a string of bits with a mask
 */
function strXor($a, $mask) {
	$mlen = strlen($mask);
	u\EXPECT($mlen, 'empty strXor mask');
	$res = '';
	for ($i = 0; $i < strlen($a); $i++) $res .= $a[$i] ^ $mask[$i % $mlen];
	return $res;
}

/**
 * Rotate a string of bits to the left
 */
function strRotate($s, $n) {
	if (!$blen = strlen($s) * 8) return '';
	$n = ($n % $blen) + ($n < 0 ? $blen : 0);
	$chars = phpversion() >= 7 ? intdiv($n,	8) : (int) ($n / 8);
	$s = substr($s, $chars) . substr($s, 0, $chars);
	if (!$bits = $n & 7) return $s; // rotation fell on a character boundary
	
	$s .= substr($s, 0, 1);
	$res = '';
	for ($i = 0; $i < strlen($s) - 1; $i++) {
		$res .= chr((ord($s[$i]) << $bits) + (ord($s[$i + 1]) >> (8 - $bits)));
	}
	return $res;
}

/**
 * Scramble a string by swapping each character with one some distance to its right (0 to 255).
 */
function strScramble($s, $key, $un = FALSE) {
	if (!$slen = strlen($s)) return '';
	$klen = strlen($key);
	u\EXPECT($klen, 'empty scramble key');
	
	for ($i0 = 0; $i0 < $slen; $i0++) {
		$i = $un ? $slen - $i0 - 1 : $i0;
		$j = ord($key[$i % $klen]) % $slen; // all modulus params in this function are >0 (no worries that in PHP -8 % 5 is -3
		list ($s[$i], $s[$j]) = [$s[$j], $s[$i]];
	}
	return $s;
}

function strUnscramble($s, $key) {return strScramble($s, $key, TRUE);}

/**
 * Return the array with secret values hidden.
 */
function noSecrets($ray) {
  foreach ($ray as $k => $v) if (in($k, R_NOLOG)) $ray[$k] = '(?)';
  return $ray;
}

/**
 * Return formatted parsedown text.
 */
function parseUp($s) {
	require_once DRUPAL_ROOT . '/vendor/parsedown/Parsedown.php'; // normally only once per form

	$pd = new \Parsedown();
	$pd->setSafeMode(TRUE);
	return strtr($pd->text($s), ["\\\n" => "<br>", '\\' => "<br>"]);
}

/*
function unSerialFix($s) {
	$s = html_entity_decode($s, ENT_QUOTES, 'UTF-8');
	$s = preg_replace('!s:(\d+):"(.*?)";!e', "'s:'.strlen('$2').':\"$2\";'", $s );
	$s = unserialize($s);
}
*/

/**
 * Return the value of the given bit.
 * @param mixed $bit:
 *   string - a defined bit name (lowercase without the initial B_)
 *   int - return the numeric value of that bit number (power of 2)
 *   ray - return the array of bit numbers as an integer bit array (recursively)
 */
function bit($b) {return is_array($b) ? (bit(array_shift($b)) | ($b ? bit($b) : 0)) : (1 << (is_numeric($b) ? $b : u\consta('b', $b)));}

function getBit($n, $b = NULL) {return is_null($b) ? $n : (bool) ($n & u\bit($b));}
function setBit(&$n, $b, $on = TRUE) {return $n = $on ? (@$n | u\bit($b)) : (@$n & ~u\bit($b));}

// Single line functions that need little or no explanation
function test() {return (@TESTING == 1);} // set in test.php (when running an acceptance test)
function nn($a, $else = '') {return is_null($a) ? $else : $a;}
function num($a, $else = 0) {return is_numeric($a) ? $a : $else;}
function eq($a, $b, $else = FALSE) {return $a == $b ? $a : $else;}
function neq($a, $b, $else = FALSE) {return $a != $b ? $a : $else;}
function nray($a, $else = FALSE) {return !is_array($a) ? $a : $else;}
function small($n) {return (abs($n) < .001);}
function equy($a, $b) {return small($a - $b);} // sort of equal
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
/**/ function prlog($log = PROJECT) {print_r(loga($log));} // keep this
function order($one1st, $one, $two) {return $one1st ? array($one, $two) : array($two, $one);}
function both(&$vars, $k, $one1st, $one, $two) {list ($vars[$k], $vars['other' . ucwords($k)]) = u\order($one1st, $one, $two);}
function wargs($s, $args, $prefix = '') {return strtr($s, prefixKeys($prefix, $args));} // for debugging
function myInt($n) {return (is_numeric($n) and $n == (int) $n);}
function myFloat($n) {return (is_numeric($n) and $n == (float) $n);}
function isId($id) {return (myInt($id) and $id > 0);}
//function isQid($qid) {return preg_match('/(^[A-Z]{6,8}(-[A-Z]{1,8})?$)|(^[A-Z]{3}' . R_MARKS . '[A-Z]{3,}$)|(^![A-Z]{3}(\.?[A-Z]{3,})?$)/', $qid);} // middle choice is deprecated (gone after 2016)
function isQid($qid) {return preg_match('/(^[A-Z]{6,8}(-[A-Z]{1,8})?$)|(^![A-Z]{3}(\.?[A-Z]{3,})?$)/', $qid);}
function isZid($zid) {return (myInt($zid) and $zid !== 0);}
function isNonce($nonce) {return preg_match('/^[A-Z]{3,}$/', $nonce);}
function isVarname($value) {return preg_match('/^[a-z_][a-z_\-0-9]*$/i', $value);}
function isName($value) {return preg_match('/^[ \\x{00c0}-\\x{01ff}a-zA-Z0-9\,\.\'\\-]+$/u', $value);}
///function isName($value) {return preg_match('/^([ \\p{InCJK_Radicals_Supplement}\\p{InKangxi_Radicals}\\p{InIdeographic_Description_Characters}\\p{InCJK_Symbols_and_Punctuation}\\p{InHiragana}\\p{InKatakana}\\p{InBopomofo}\\p{InHangul_Compatibility_Jamo}\\p{InKanbun}\\p{InBopomofo_Extended}\\p{InKatakana_Phonetic_Extensions}\\p{InEnclosed_CJK_Letters_and_Months}\\p{InCJK_Compatibility}\\p{InCJK_Unified_Ideographs_Extension_A}\\p{InYijing_Hexagram_Symbols}\\p{InCJK_Unified_Ideographs}a-zA-Z\'\\-])+$/', $value);}
function isAcct($acct) {return (is_object($acct) and get_class($acct) == 'CG\\Acct');}
function hug($s, $arms = '()') {return substr($arms, 0, 1) . $s . substr($arms, -1, 1);} // deprecated
function s2t($s) {return strtotime(preg_match('/[A-Z]/i', $s) ? $s : str_replace('-', '/', $s));}
function fmtDate($time = NULL, $numeric = FALSE) {return strftime($numeric ? '%m/%d/%Y' : '%d%b%Y', isset($time) ? $time : r\rTime());}
function fmtTime($time = NULL) {return strtolower(strftime('%I:%M%p', isset($time) ? $time : r\rTime()));}
function lastCall($far = 4) {$back = debug_backtrace(); return array_slice($back, 1, $far);}
function digits(&$n) {return $n = preg_replace('/\D/', '', @$n);}
function monthDay1($time = NULL) {return strtotime(strftime('1%b%Y', isset($time) ? $time : r\rTime()));}
function prefix($prefix, &$var) {$var = $prefix . $var;}
function preray($prefix, &$r0) {u\EXPECT(is_array($prefix) and (is_array($r0) or is_object($r0)), 'badray');$r = $prefix + (array) $r0; $r0 = is_object($r0) ? (object) $r : $r;}
function setDft(&$param, $value) {if (!isset($param)) $param = $value;}
function ifSet(&$param, $value) {if (isset($param)) $param = $value;} // opposite of setDft
function log($message, $type = 'state', $info = []) {return u\loga($type, compact('message') + $info);}
function consta($prefix, $var) {return constant(strtoupper(str_replace(' ', '_', $prefix . "_$var")));}
function isIPhone() {return preg_match('/iPhone/i', $_SERVER['HTTP_USER_AGENT']);}
function jsonEncode($s) {return json_encode($s) ?: json_encode(u\purify($s));} // , JSON_UNESCAPED_SLASHES
function redash($s) {return str_replace('—', '&mdash;', $s);}
function nonish($s) {return in(strtolower($s), 'n/a nothing na none'); }
function padto($s, $n) {return str_pad(substr($s, 0, $n), $n);}
function lpad($s, $n) {return str_pad(substr($s, 0, $n), $n, '0', STR_PAD_LEFT);}
function similarEmail($e1, $e2) {return (strtolower(preg_replace('/\+.*@/', '@', $e1)) == strtolower(preg_replace('/\+.*@/', '@', $e1)));}
function rayv($a, $i) {return $a[$i];} // useful when $a is returned from a function
function strip($a) {foreach ($a as $i => $v) $a[$i] = strip_tags($v); return $a;}
function initials($s) {return preg_replace('/[^A-Z]/', '', ucwords(strtr($s, ray('Jr', ''))));}
function roughName($name) {return preg_replace("/[^A-Z '\-]/", '', strtoupper(trim(str_replace('  ', ' ', $name))));}
function urlArgs() {return basename($_SERVER['REQUEST_URI']);}
function uc1($s) {return ucwords(substr($s, 0, 1)) . substr($s, 1);}
function nextAfter($needle, $hay) {return ($i = array_search($needle, $hay)) === FALSE ? NULL : $hay[$i + 1];}
function caller($depth = 1) {return debug_backtrace()[$depth + 1]['function'];}
function rotateBits($b, $i) {return ($i = $i % strlen($b)) ? substr($b, $i) . substr($b, 0, $i) : $b;}
function xorBits($b1, $b2) {$res = ''; for ($i = 0; $i < strlen($b1); $i++) $res .= (@$b1[$i] xor @$b2[$i]) ? 1 : 0; return $res;}
function a2b($s) {$b = ''; for ($i = 0; $i < strlen($s); $i++) $b .= lpad(decbin(ord($s[$i])), 8); return $b;}
function randomFloat($min = 0, $max = 1) {return $min + mt_rand() / mt_getrandmax() * ($max - $min);}
function hash($s) {return \hash('sha256', $s, false);}
function day2n($day) {return array_search($day, ray('Sun Mon Tue Wed Thu Fri Sat'));} // workaround %u and %w failure
function lf2br($s) {return strtr($s, ["\r" => '', "\n" => '<br>']);}
function sqlSerialValue($s, $n) {return "SUBSTRING_INDEX(SUBSTRING_INDEX($s, '\\\"', $n * 2), '\\\"', -1)";}
function similar($s1, $s2, $chars = 5, $pct0 = .1) {return (similar_text($s1, $s2, $pct) > $chars and $pct > $pct0);}
function unite($a, $b, $m = '') {return ($a and $b) ? tr('%a%m%b', '_a _m _b', $a, $m ?: t(' and '), $b) : ($a ?: $b);}
function honorDesc($honor) {return $honor == 'other' ? ' ' : t(' in %honor of ', compact('honor'));}
function inList($needle, $hay) {return (strpos(" $hay ", " $needle ") !== FALSE or strpos(",$hay,", ",$needle,") !== FALSE);}
function weakPass($pw) {return (strlen($pw) < 8 or !preg_match('/\d/', $pw) or !preg_match('/[A-Z]/', $pw) or !preg_match('/[a-z]/', $pw) or !preg_match('/[^0-9A-Za-z]/', $pw));}
function b64encode($data) {return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');}
function b64decode($data) {return base64_decode(strtr($data, '-_', '+/'));}
function fmtKey($k, $f) {$d = '-----'; $k = chunk_split(base64_encode($k), 64); return "{$d}BEGIN $f KEY$d\n$k{$d}END $f KEY$d";}
function unfmtKey($k) {return base64_decode(preg_replace('/(-+[^-]+-+|[\r\n ])/', '', strpos($k, ':') ? file_get_contents($k) : $k));}
function pubKey($k) {return openssl_pkey_get_details(openssl_pkey_get_private(fmtKey(unfmtKey($k), 'PRIVATE')))['key'];}
function validEmail($email) {return filter_var($email, FILTER_VALIDATE_EMAIL);}