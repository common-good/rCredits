<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */

namespace rCredits\Util; // typically abbreviated as "u"
use rCredits as r; // get rid of this

// Some useful Drupal db methods: fetchAllKeyed(m, n), fetchAssoc and fetchAllAssoc(fld), fetchObject and fetchAll(), fetchCol(n), fetchField()
/**
 * Split the given string into an array
 * delimited by space, comma, or semi-colon, followed by any number of spaces, or by a bar surrounded by optional spaces
 * Entries in a comma-delimited list that contain a colon will be split into key and value.
 */
function ray($s) {
  EXPECT(compact('s'), 'string');
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
 * Return an associative array built from the arguments.
 * Call by:
 *   ass(keys, value1, value2, ...)
 * @param string $keys: space-delimited list of identifier names
 * @return assoc array whose keys and values are as listed
 */
function ass($keys, $value1) {
  $args = func_get_args();
  $keys = array_shift($args);
  return array_combine(ray($keys), $args);
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
 * @return array of the results
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
 * array_splice for associative arrays.
 * If the key is not found, $input is unchanged and return NULL
 */
function splice(&$input, $key, $len = 0, $replacement = NULL) {
  $i = array_search($key, array_keys($input));
  return $i === FALSE ? NULL : array_splice($input, $i, $len, $replacement);
}

function cleanAmount($amount) {
  $amount = trim($amount);
  if (substr($amount, 0, 1) == '$') $amount = substr($amount, 1);
  return preg_replace('/[ \,]/', '', $amount); // ignore spaces and commas
}

/**
 * Return an error message if the amount is bad.
 * @param string $restriction: none, >0, or >=0 (the default)
 */
function badAmount(&$amount, $restriction = '') {
  if (!is_numeric($new = cleanAmount($amount))) return 'bad amount';

  $amount = $new;
  $dotPos = strpos($amount, '.');
  if ($dotPos !== FALSE and strlen($amount) > $dotPos + 3) return 'fractional cents';
  if ($amount > R_MAX_AMOUNT) return 'amount too big';
  if ($restriction == '>0' and $amount <= 0) return 'nonpositive amount';
  if ($restriction == '>=0' and $amount < 0) return 'negative amount';
  return FALSE;
}

/**
 * Format the given amount as currency.
 *
 * @param float $amount
 * @param boolean $simplify: Omit the cents for whole dollar amounts
 *
 * @return
 *   the formatted amount
 */
function formatAmount($amount, $simplify = TRUE) {
  $amount = cleanAmount(nn($amount, 0));
//  if (!is_numeric($amount)) $amount = -999999;
  $amount = '$' . number_format($amount, 2);
  return $simplify ? str_replace('.00', '', $amount) : $amount;
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
 * Return the rough location of the entity with the given account (city, state, non-US country).
 */
function location($acct, $complete = FALSE) {
  $isUS = ($acct->country == R_COUNTRY);
  
  $result = array();
  if ($complete) $result[] = (string) $acct->address; // (string) in case address is NULL
  $result[] = $acct->city;
  $result[] = ($isUS ? stateAbbrev($acct->state) : $acct->state) . (($complete and $acct->postal_code) ? ' ' . $acct->postal_code : '');
  if (!$isUS) $result[] = strtoupper($acct->country);
  foreach ($result as $key => $value) if ($value === '') unset($result[$key]);
  return join(', ', $result);
}

/**
 * Return the standard 2-letter abbreviation for the country or vice versa
 * See the latest data on Wikipedia.
 * @todo: get this in agreement with countries.js (sorted by country name)
 */
function countryAbbrev($country) {
  global $base_path;
//  include_once $base_path . 'inc/countries.php';
  include __DIR__ . '/inc/countries.php';
  return strlen($country) == 2 ? $countries[$country] : array_search($country, $countries);
}

/**
 * Return the standard 2-letter abbreviation for the (US) state or vice versa
 * @todo: add Canadian provinces
 */
function stateAbbrev($state) {
  global $base_path;
  include __DIR__ . '/inc/states.php';
  return strlen($state) == 2 ? $states[$state] : array_search($state, $states);
}


/**
 * Calculate a short_name from the given full_name.
 */
function shortName($full_name, $filler = '') {
  $deletions = ray('Inc. LLC Co. P.C. Corp. LLP');
  foreach ($deletions as $one) $full_name = str_replace(" $one", '', $full_name);
  $full_name = str_replace('&', 'and', $full_name);
  return strtolower(preg_replace('/\W/', $filler, iconv('UTF-8', 'ASCII//TRANSLIT', $full_name)));
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function forCash($for) {
  if (is_array($for)) return forCash($for[0]) or forCash($for[1]);
  return ($for == '' or preg_match('/^(cash|usd|loan)(\W|$)/i', $for)); // empty or a non-goods word at the beginning
}

/**
 * Return an alphabetic representation of the given integer.
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
 * Format the given phone number as wanted
 * @param string $phone
 * @param string $how: how to format it
 *   '+n' +1dddddddddd (AKA 'internal')
 *   'n'  dddddddddd
 *   '+'  +1 ddd ddd dddd (the international standard with spaces added)
 *   ''   +1 ddd.ddd.dddd (the default)
 * @return the formatted phone number (FALSE if the $phone is not a phone number or $how is bad)
 */
function formatPhone($phone, $how = '') {
  $sep = '.'; // digit group separator
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
  } elseif (substr($phone, 0, 1) == '+') $foreign = substr($phone, 1);

  if (!is_numeric($phone)) return FALSE;
  if (!$foreign and substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
  if (!$foreign and strlen($phone) != 10) return FALSE;

  if ($how == 'n') return $phone;
  if ($how == '+n' or $how == 'internal') return ($foreign ? "+$foreign" : "+1$phone");

  $phone = substr($phone, 0, 3) . $sep . substr($phone, 3, 3) . $sep . substr($phone, 6);
//  if ($how == '') return $phone;
  if ($how == '+' or $how == '') {
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
 * Get the long version of the indicated message(s), make any substitutions,
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
  if (!$subs) $subs = array();
  $messages = explode('|', $message);
  $channels = ray(TX_CHANNELS);
  foreach ($messages as $i => $message) {
    $model = $message; // the default
    for ($ch = ($adhocChannel ?: (count($channels) - 1)); $ch > 0; $ch--) {
      if ($model = @$GLOBALS["$ch messages"][$message]) {
        if($p = strpos($model, '. Type ')) {
          if ($channel != TX_SMS and $adhocChannel != TX_SMS) $model = substr($model, 0, $p + 1); // don't tell the user to type something unless it's SMS
        }
        if (strpos($model, '@asif') !== FALSE and !isset($subs['asif'])) {
          $real = isset($subs['real']) ? $subs['real'] : r\isRTrader();
          $subs['asif'] = $real ? '' : R_ASIF_TEXT;
        }
        break;
      }
    }
// NO (use raw message instead)   EXPECT(@$model, "missing message '$message'"); 
    if (@$model) $messages[$i] = getConstants(strtr($model, prefixKeys('@', $subs)), '@');
  }
  return join(' ', $messages);
}

/**
 * Return random uppercase dictionary word of 3-6 chars
 * The word will not be a valid command or command abbreviation.
 * @todo: get these r\'s out of here.
 */
function nonce() {
  $offset = r\dbLookup('FLOOR(RAND() * COUNT(*))', 'r_nonces');
  $nonce = r\dbLookup('nonce', 'r_nonces', "1 LIMIT $offset, 1");

  $impossible = join(' ', $GLOBALS['SMS not a nonce']);
  if (strpos(". $impossible ", " $nonce ")) return nonce(); // reserved word? try again
  if (strlen($nonce) > 6) return nonce(); // too long
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

The $url parameter is the HTTP or HTTPS URL for the web service. $params is an associative array of form parameters to pass to the web service; they will be passed as _GET parameters for GET requests or _POST parameters for POST requests. The $method parameter can be GET or POST (and presumably any other valid HTTP REQUEST method, such as PUT or DELETE, although I haven't tried those and can't say whether they will work as expected). The $format parameter can be "json" or "xml" and will automatically return a decoded json or XML document, respectively.

I've used simplexml here because it is... simple. You could very easily add a "dom" format to return the object using the richer and more complex DOM API instead.

This function uses the ignore_errors context parameter. Without this set (the default is FALSE), PHP will treat 400 and 500 HTTP status codes as a failure to open the stream and won't return you any data. This is usually what you want when using fopen or file_get_contents, but REST services tend to set the HTTP status to indicate the error and will usually send back a payload that describes the error. We turn on ignore_errors so that we treat any returned payload as json or xml.

When using POST with REST, take care: PHP's HTTP redirection handler will drop your POST payload if the endpoint issues a redirect. If you experience problems using POST with the function below, it might be due to redirects. Most of the POST calls I've run into issue redirects if the URL is missing a trailing '/' character. In other words, if you experience problems where it seems like your parameters are not being sent in, try appending a '/' to the end of the URL and try it again.
 */
function httpRequest($url, $params = FALSE, $extra_headers = array(), $method = 'POST', $format = 'json', $request_format = 'url') {
  $ignore_errors = TRUE;
  $headers = array('Content-Type' => $request_format == 'json' ? 'application/json' : 'application/x-www-form-urlencoded') + $extra_headers;
  $header = '';
  foreach ($headers as $key => $value) $header .= "$key: $value\n";
  $http = compact(ray('method ignore_errors header'));

  if ($params) {
    if ($method == 'POST') {
      $http['content'] = $request_format == 'json' ? json_encode2($params, JSON_UNESCAPED_SLASHES) : http_build_query($params);
    } else $url .= '?' . http_build_query($params);
  }

  $context = stream_context_create(compact('http'));
  $fp = fopen($url, 'rb', FALSE, $context);
  if (!$fp) {
    $res = FALSE;
  } else {
    // If you're troubleshooting, uncomment the next line to see the HTTP response headers across all redirects:
    // $meta = stream_get_meta_data($fp); var_dump($meta['wrapper_data']);
    $res = stream_get_contents($fp);
  }
  if ($res === FALSE) throw new \Exception("$method $url failed: $php_errormsg");
  
  $result = $format == 'xml' ? simplexml_load_string($res) : json_decode($res);
  if (is_null($result)) throw new \Exception("$method $url returned error: $res");
  
  return $result;
}

/**
 * Translate constant parameters in a string.
 * For example, "SELECT * FROM table WHERE zot IN (:R_THIS, :R_THAT)" fills in the constants.
 * @param string $string: the string to fix
 * @return string: the string with constant names replaced by their values
 */
function getConstants($string, $prefix = ':') {
  preg_match_all("/\\$prefix([A-Z_][A-Z0-9_]*)/ms", $string, $matches);
  $map = array();
  foreach ($matches[1] as $one) $map["$prefix$one"] = constant($one);
  return strtr($string, $map);
}

/**
 * Assert an expectation or check parameters for validity
 * Throw an Exception if the expectation is not met.
 * call by:
 *   EXPECT($assertion, $message, $ret)
 *   EXPECT($args, $types, $ret) OR
 * @param boolean $assertion: a statement to assert
 * @param string $message: message to display if assertion is FALSE
 *
 * @param array $args: associative array of variables, indexed by variable name, for which to check types
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
//    EXPECT(compact(ray('args types')), 'assoc string'); // can't do this because infinite recursion
//if (strpos($types, '|')) {debug(array_keys($args)); debug($types);}
//    EXPECT(count($args) != count($types = explode(' ', $types)), 'wrong number of types expected'); // infinite recursion
//    if(count($args) != count($types = explode(' ', $types))) die('wrong number of types expected: ' . print_r(debug_backtrace(), 1));
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

  $msg = "Expected \$$arg ('" . print_r($value, 1) . "') to be type '$type'."; // standard err message (keep this)
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
    case 'bool': return EXPECT(is_bool($value), $msg, $ret);
    case 'email': return EXPECT(valid_email_address($value), $msg, $ret);
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
 * @param string $special: a special value if any (for example the nonce)
 * @return the log record id (for exception handler and shutdown function)
 */  
function log($type, $info = array(), $special = '', $adhocChannel = '') {
  global $channel;

  if (empty($info)) list ($type, $info) = array('-', array($type));
  if ($adhocChannel and $adhocChannel != $channel and !$special) $special = "via ch$adhocChannel";
  $cuid = @r\acct()->id;
  $agent = @r\acct()->agentId;
  $log_filename = @$channel ? "$channel.log" : 'rcredits.log';
  $date = formatDate();
  $channels = ray(TX_CHANNELS); 
  $channel_name = @$channel ? $channels[$channel] : 'none';
  if (isNonce($special) and @$info['message']) $info['message'] = str_replace($special, '@nonce', $info['message']); // make it easy to match
  if (@$info['code']) $info['code'] = '(the code)'; // don't make code and password any more accessible than they are
  if (@$info['password']) $info['password'] = '(the password)';
  $info = json_encode2($info, JSON_UNESCAPED_SLASHES);
  file_put_contents($log_filename, "$date ($channel_name) $type: $info\n", FILE_APPEND);
  $time = time();
  $record = compact(ray('time channel type cuid agent info special'));
  drupal_write_record('r_log', $record);
  return $record['logid'];
}  

/**
 * Encrypt the given file and save in new path.
 * to decrypt:
 *   $decrypted = $cryptastic->decrypt($encrypted, $encryptionKey) or die("Failed to complete decryption");
 * @return: the encryption key (FALSE if unsuccessful)
 */
function encryptFile($filepath, $newFilepath = '', $pass, $salt) {
  require __DIR__ . '/cryptastic.php';
  if ($newFilepath === '') $newFilepath = $filepath;
  $content = file_get_contents($filepath);
  $destination = dirname($newFilepath);
  if (!is_dir($destination)) mkdir($destination);

  $cryptastic = new \cryptastic;
  $encryptionKey = $cryptastic->pbkdf2($pass, $salt, 1000, 32) or die("Failed to generate secret key.");
  $encryptedFile = $cryptastic->encrypt($content, $encryptionKey) or die("Failed to complete encryption.");

  if (file_put_contents($newFilepath, $encryptedFile)) {
    return $encryptionKey;
  } else return FALSE;
}
 
/**
 * Change something to the proper case, unless it is already right.
 * @param mixed $what: a string or array of strings to fix
 * @param string $proper: the PHP function to use, to fix the case
 * @return: the argument with case fixed (the argument itself is changed directly also)
 */
function normalizeCase(&$what, $proper = 'ucwords') {
  if (is_array($what)) {
    foreach ($what as $key => $value) normalizeCase($what[$key]);
    return $what;
  }
  $upper = strtoupper($what);
  $lower = strtolower($what);
  if ($proper = 'ucwords' and $what != strtolower($what) and $what != strtoupper($what)) return $what; // already mixed case
  return ($what = $proper($what));
}

/*
function import_gfld() {
  $result = \db_query("SELECT * FROM companies ORDER BY full_name");
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $short_name = shortName($full_name);
    $phone = $phone ? "+1$phone" : NULL;
    $fax = $fax ? "+1$fax" : NULL;
    $state = stateAbbrev($state);
    $website = str_replace('https://', '', str_replace('http://', '', str_replace('www.', '', $website)));
    $account_type = r\dbLookup(1, 'r_user_industries', "iid IN (121,61) AND uid=:uid", compact('uid')) ? 2 : 1;
    $sql = "UPDATE companies SET phone=:phone, fax=:fax, status=1, website=:website, name=:short_name, short_name=:short_name, state=:state, country='United States', timezone='America/New_York', account_type=:account_type WHERE uid=:uid";
    r\dbQ($sql, compact(ray('short_name uid phone fax state website account_type')));

    $cats = unserialize($categories);
    foreach ($cats as $industry) {
      $industry = ucwords(strtolower($industry));
      if (!$iid = r\dbLookup('iid', 'r_industries', 'industry=:industry', compact('industry'))) {
        $record = compact('industry');
        db_query("INSERT INTO r_industries (industry) VALUES (:industry)", $record);
//        db_insert('r_industries')->fields($record);
//        debug(drupal_write_record('r_industries', $record));
        $iid = $record['iid'];
      }
      db_query("INSERT INTO r_user_industries (iid, uid) VALUES (:iid, :uid)", prefixKeys(':', compact('iid', 'uid')));
//      drupal_write_record('r_user_industries', compact(ray('iid uid')));
    }
  }
}
*/

function gzdecode($data){
    $g=tempnam('./','gz');
    file_put_contents($g,$data);
    ob_start();
    readgzfile($g);
    $d=ob_get_clean();
    unlink($g);
    return $d;
}

// rCredits\Util\get_company_pictures();
function get_company_pictures() {
  $result = r\dbQ("SELECT uid,website,data,description FROM users WHERE uid>791 AND website<>'' ORDER BY uid");
  while ($row = $result->fetchAssoc()) {
    extract($row);
    if (in_array($uid, array(549, 551, 552, 779, 791))) continue; // these apparently kill it
    echo "$uid<br>"; // in case it dies

    if (strlen($website) < 4) {debug($uid); continue;} // keep this
    $homepage = 'http://' . $website;
    $s = @file_get_contents($homepage);
    if (!$s) $s = @file_get_contents($homepage = ('http://www.' . $website));
    if (!$s) {debug("Can't open $homepage"); continue;} // keep this
    
    if (!$description) {
      $got = preg_match('/<meta\s+name\s*=\s*[\'"]description[\'"]\s+content\s*=\s*[\'"](.*?)[\'"]/msi', $s, $matches);
      if (!$got) {
        $s = gzdecode($s);
        if (!preg_match('/<meta\s+name\s*=\s*[\'"]description[\'"]\s+content\s*=\s*[\'"](.*?)[\'"]/msi', $s, $matches)) continue;
      }
      $description = htmlspecialchars_decode(@$matches[1]);
      if ($description) r\dbQ('UPDATE users SET description=:description WHERE uid=:uid', compact(ray('description uid')));
    }

    if (strlen($data) > 3) continue;    
    preg_match_all('/<img\s.*?src\s*?=\s*?[\'"](.*?)[\'"]/ism', $s, $matches);
    $biggest = 0; $best = '';
    foreach ($matches[1] as $one) {
      if (!stripos($one, '.jpg') and !stripos($one, '.png')) continue;
      if (strpos($one, 'http:') === FALSE) $one = "$homepage/$one";
      $info = @getimagesize($one);
      list ($width, $height) = is_array($info) ? $info : array(0, 0);
//      if (($len = fsize($one)) > @$biggest) list ($biggest, $best) = array($len, $one);
      if (($len = $width * $height) > @$biggest) list ($biggest, $best) = array($len, $one);
      if ($biggest > 50000 and $width > 100 and $height > 100 and $width*$height > 20000) break;
    }
    if (@$biggest) {
      $picture = $best;
      $data = serialize(compact(ray('picture width height')));
      r\dbQ('UPDATE users SET data=:data WHERE uid=:uid', compact(ray('data uid')));
    }
  }
  debug('done'); // keep this
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
  log(($counterI = @$counterI + 1) . " ($msgCount messages): " . print_r($arg, 1));
}

/**
 * Insert the bundle (associative array) before the needle in haystack
 * @param assoc $bundle: array to insert
 * @param assoc $haystack: array into which to insert
 * @param string $needle: key before which to insert
 * @return assoc: the resultant array (FALSE if the needle is not found)
 */
function insertBefore($bundle, $haystack, $needle) {
  if (!$i = array_search($needle, $haystack)) return FALSE;
  return array_slice($haystack, 0, $i+1) + $bundle + array_slice($haystack, $i+1);
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

function switchOnNEW() {
/*
SELECT DISTINCT
  c.id, c.display_name AS full_name, c.contact_type AS account_type,
  e.email,
  w.url AS website,
  p.phone,
  a.street_address AS address, a.city, CONCAT('"',a.postal_code,'"') AS zip, IF(a.country_id=1228 OR a.country_id IS NULL, 'US', a.country_id) AS country, a.state_province_id AS state,
  x.company_43 AS company, x.types_42 AS types, x.created_37 AS created
  FROM ((((((`civicrm_contact` c INNER JOIN `civicrm_entity_tag` t ON t.entity_id=c.id)
  LEFT JOIN civicrm_email e ON e.contact_id=c.id)
  LEFT JOIN civicrm_website w ON w.contact_id=c.id)
  LEFT JOIN civicrm_phone p ON p.contact_id=c.id)
  LEFT JOIN civicrm_address a ON a.contact_id=c.id)
  LEFT JOIN civicrm_value_extra_contact_fields_6 x ON x.entity_id=c.id)
  WHERE t.tag_id=54
  AND NOT c.is_deleted
  AND (e.contact_id IS NULL OR e.is_primary=1)
  AND (p.contact_id IS NULL OR p.is_primary=1)
  ORDER BY display_name
EOF;
*/
}

// Single line functions that need no explanation
function nn($a, $else = '') {return is_null($a) ? $else : $a;}
function num($a, $else = 0) {return is_numeric($a) ? $a : $else;}
function eq($a, $b, $else = FALSE) {return $a == $b ? $a : $else;}
function neq($a, $b, $else = FALSE) {return $a != $b ? $a : $else;}
function small($n) {return (($n < 0 ? -$n : $n) < .001);}
function equy($a, $b) {return small($a - $b);} // sort of equal
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
function prlog($log = 'rCredits') {print_r(log($log));} // keep this
function order($first_first, $choice1, $choice2) {return $first_first ? array($choice1, $choice2) : array($choice2, $choice1);}
function wargs($s, $args, $prefix = '') {return strtr($s, prefixKeys($prefix, $args));} // for debugging
function myInt($n) {return (is_numeric($n) and $n == (int) $n);}
function myFloat($n) {return (is_numeric($n) and $n == (float) $n);}
function isId($id) {return (myInt($id) and $id > 0);}
//function is_ruid($ruid) {return (is_array($ruid) and array_keys($ruid) == array(0, 1, 2) and isZid($ruid[0]) and isZid($ruid[1]) and isZid($ruid[2]));}
function isQid($qid) {if(is_object($qid)) debug(lastCall(6)); return preg_match('/^[A-Z]{3}(\.|:|=|\.\.)[A-Z]+$/', $qid);}
function isZid($zid) {return (myInt($zid) and $zid !== 0);}
function isNonce($nonce) {return preg_match('/^[A-Z]{3,}$/', $nonce);}
function isName($value) {return preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $value);}
function isAcct($acct) {return (is_object($acct) and get_class($acct) == 'rCredits\acct');}
function hug($s, $arms = '()') {return substr($arms, 0, 1) . $s . substr($arms, -1, 1);}
function formatDate($time = REQUEST_TIME) {return strftime('%d-%b-%Y', $time);}
function formatTime($time = REQUEST_TIME) {return strtolower(strftime('%I:%M%p', $time));}
function lastCall($far = 4) {$back = debug_backtrace(); return array_slice($back, 1, $far);}
function digits($n) {return preg_replace('/\D/', '', $n);}
function monthDay1($time = REQUEST_TIME) {return strtotime(strftime('%b01'));}