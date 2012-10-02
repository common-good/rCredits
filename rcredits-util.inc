<?php
/**
 * @file
 * rCredits utilities
 * This file may be both on the core VPS and the user interface VPS
 */

namespace rCredits\Utilities; // typically abbreviated as "u"
use rCredits as r; // get rid of this

// Some useful Drupal db methods: fetchAllKeyed(m, n), fetchAssoc and fetchAllAssoc(fld), fetchObject and fetchAll(), fetchCol(n), fetchField()
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
function valid_name($name, $individual = TRUE, $picky = TRUE) {
  $name = trim($name);
  $words = substr_count($name, ' ') + 1;
  $pattern = $individual ? "[^\w\-\.,' ]" : '[^\w~!@#\$%\^&\*\(\)\-=\+\\\\;:\'",<\.>/\? ]';
  if (preg_match("`$pattern`iu", $name)) return FALSE; // allows unicode chars and common punctuation
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
function short_name($full_name, $filler = '') {
  $deletions = ray('Inc. LLC Co. P.C. Corp. LLP');
  foreach ($deletions as $one) $full_name = str_replace(" $one", '', $full_name);
  $full_name = str_replace('&', 'and', $full_name);
  return strtolower(preg_replace('/\W/', $filler, iconv('UTF-8', 'ASCII//TRANSLIT', $full_name)));
}

/**
 * Say whether the transaction description implies a cash transaction.
 */
function for_cash($for) {
  if (is_array($for)) return for_cash($for[0]) or for_cash($for[1]);
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
function format_phone($phone, $how = '') {
  $sep = '.'; // digit group separator
  $foreign = ''; // set to 'n' format if the phone is outside US
  $phone = (preg_replace('/[\(\) \-\.]/', '', $phone)); // ignore typical punctuation
  if (substr($phone, 0, 2) == '+1') {
    $phone = substr($phone, 2);
    if (substr($phone, 0, 1) < 2) return FALSE; // US phone numbers cannot start with 0 or 1
//    if (strlen($phone) != 10) return FALSE;  // temporarily disabled for debugging
  } elseif (substr($phone, 0, 1) == '+') $foreign = substr($phone, 1);
  
  if (!is_numeric($phone)) return FALSE;

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
 * @todo get the r\'s out of this utilities file OR stop trying to separate u\ from r\
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
    if (strpos($model, '@asif') !== FALSE and !@$subs['asif']) {
      $real = isset($subs['real']) ? $subs['real'] : r\is_rTrader();
      $subs['asif'] = $real ? '' : R_ASIF_TEXT;
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

function ignore_suffix($s, $suffixes) {
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
function http_request($url, $params = FALSE, $extra_headers = array(), $method = 'POST', $format = 'json', $request_format = 'url') {
  $ignore_errors = TRUE;
  $headers = array('Content-Type' => $request_format == 'json' ? 'application/json' : 'application/x-www-form-urlencoded') + $extra_headers;
  $header = '';
  foreach ($headers as $key => $value) $header .= "$key: $value\n";
  $http = compact(ray('method ignore_errors header'));

  if ($params) {
    if ($method == 'POST') {
      $http['content'] = $request_format == 'json' ? json_encode($params) : http_build_query($params);
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
function get_constants($string) {
  preg_match_all('/:([A-Z_]+)/ms', $string, $matches);
  $map = array();
  foreach ($matches[1] as $one) $map[":$one"] = constant($one);
  return strtr($string, $map);
}

/**
 * Assert an expectation or check parameters for validity
 * Throw an Exception if the expectation is not met.
 * call by:
 *   expect($assertion, $message, $return)
 *   expect($args, $types, $return) OR
 * @param boolean $assertion: a statement to assert
 * @param string $message: message to display if assertion is FALSE
 *
 * @param array $args: associative array of variables, indexed by variable name, for which to check types
 * @param string $types: a corresponding space-delimited array of expected types, 
 *   each of which may in turn be a list of types delimited by '&' or a list of such lists as options, delimited by '|'.
 *
 * @param bool $return: return the error message rather than throwing it
 * @return string (if $return is TRUE): an error message or '' (no error)
 */
function expect($assertion, $message = '', $return = FALSE) {
  if (is_array($assertion)) {
    list ($args, $types) = array($assertion, $message); // for clarity
//    expect(compact(ray('args types')), 'assoc string'); // can't do this because infinite recursion
    $types = array_combine(array_keys($args), explode(' ', $types)); // not ray() here, $types may have '|'
    foreach ($args as $key => $value) if ($error = expect1($key, $value, $types[$key], $return)) return $error;
  } elseif (!$assertion) {
    if ($return) return $message; else throw new \Exception($message);
//    expect(compact(ray('assertion message')), 'bool string&notempty'); // can't do this because infinite recursion
  }
  return '';
}  

function expect1($arg, $value, $type, $return) {
//  expect(compact(ray('arg type')), 'name string'); // can't do this because infinite recursion

  $msg = "Expected $arg ('$value') to be type '$type'.";
  if (strpos($type, '|')) {
    foreach (explode('|', $type) as $one) if (expect1($arg, $value, $one, TRUE) == '') return '';
    expect(FALSE, $msg);
  }
  if (strpos($type, '&')) {
    foreach (explode('&', $type) as $one) if ($error = expect1($arg, $value, $one, $return)) return $error;
    return '';
  }
  
  switch ($type) {
    case 'array': return expect(is_array($value), $msg, $return);
    case 'assoc': 
      expect(is_array($value), $msg, $return);
      foreach ($value as $key => $one) expect1("$arg key", $key, 'name', $return);
      return;
    case 'bool': return expect(is_bool($value), $msg, $return);
    case 'email': return expect(valid_email_address($value), $msg, $return);
    case 'empty': return expect(empty($value), $msg, $return);
    case 'float': return expect(is_float($value), $msg, $return); // don't use is_float()?
    case 'id': return expect(is_id($value), $msg, $return);
    case 'int': return expect(my_int($value), $msg, $return); // don't use is_int()
    case 'name': return expect(preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $value), $msg, $return);
    case 'notempty': return expect(!empty($value), $msg, $return);
    case 'notnull': return expect(!is_null($value), $msg, $return);
    case 'qid': return expect(is_qid($value), $msg, $return);
    case 'rid': return expect(is_rid($value), $msg, $return);
    case 'string': return expect(is_string($value), $msg, $return);
    case 'zid': return expect(my_int($value) and $value !== 0, $msg, $return);
  }
  throw new \Exception('bad type: ' . $msg);
}

/** 
 * Log the message and return it.
 * This makes it easy for tests to see what was done
 *
 * @param string $log: which log (rCredits, rSMS, rWeb, etc.)
 * @param string $type: type of log entry (if FALSE, don't log anything)
 * @param array $info: the information to log
 * @return the (possibly updated) log array
 */  
function log($type = FALSE, $info = FALSE) {
  global $channel; $log = $channel;
 //$type='', $message='', $number='', $nonce=''
  $log_filename = "$log.log";
//    $log = file_exists($log_filename) ? file_get_contents($log_filename) : '';
//    file_put_contents($log_filename, $log . "$type, $number, $message\n");
  $log = file_exists($log_filename) ? unserialize(file_get_contents($log_filename)) : '';
  if ($type) {
    extract($info, EXTR_PREFIX_ALL, 'my');
    $info['type'] = $type;
    $log[] = $info; //compact('type', 'message', 'number');
    if (@$my_nonce and @$my_message) {
      $info['message'] = str_replace($my_nonce, '@nonce', $my_message); // also store without the nonce for testing, because the nonce is unpredictable
      $log[] = $info;
    }
    file_put_contents($log_filename, serialize($log)); //  . "\n"); ?
  }
  return $log;
}  

/*
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

// rCredits\Utilities\get_company_pictures();
function get_company_pictures() {
  $result = r\db_q("SELECT uid,website,data,description FROM users WHERE uid>552 AND website<>'' ORDER BY uid");
  while ($row = $result->fetchAssoc()) {
    extract($row);
    if (in_array($uid, array(549, 551, 552))) continue; // these apparently kill it
    echo "$uid<br>"; // in case it dies

    if (strlen($website) < 4) {debug($uid); continue;}
    $homepage = 'http://' . $website;
    $s = @file_get_contents($homepage);
    if (!$s) $s = @file_get_contents($homepage = ('http://www.' . $website));
    if (!$s) {debug("Can't open $homepage"); continue;}
    
    if (!$description) {
      $got = preg_match('/<meta\s+name\s*=\s*[\'"]description[\'"]\s+content\s*=\s*[\'"](.*?)[\'"]/msi', $s, $matches);
      if (!$got) {
        $s = gzdecode($s);
        if (!preg_match('/<meta\s+name\s*=\s*[\'"]description[\'"]\s+content\s*=\s*[\'"](.*?)[\'"]/msi', $s, $matches)) continue;
      }
      $description = htmlspecialchars_decode(@$matches[1]);
      if ($description) r\db_q('UPDATE users SET description=:description WHERE uid=:uid', compact(ray('description uid')));
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
      r\db_q('UPDATE users SET data=:data WHERE uid=:uid', compact(ray('data uid')));
    }
  }
  debug('done');
}

function fsize($url) {
  if ($size = @filesize($url)) return $size;
  $s = file_get_contents($url);
  return strlen($s);
}

function trim_all(&$array) {
  foreach ($array as $key => $value) $array[$key] = trim($value);
  return $array;
}

// Single line functions that need no explanation
function valid_url($url) {return (\valid_url($url) and preg_match('/^[A-Z0-9\-_]+\.[A-Z0-9\-_]+/i', $url) and strpos($url, '@') === FALSE);}
function abbreviates($s, $string) {return (substr($string, 0, strlen($s)) == $s);}
function prlog($log = 'rcredits') {print_r(log($log));}
function order($first_first, $choice1, $choice2) {return $first_first ? array($choice1, $choice2) : array($choice2, $choice1);}
function wargs($s, $args, $prefix = '') {return strtr($s, prefix_keys($prefix, $args));} // for debugging
function my_int($n) {return (is_numeric($n) and $n == (int) $n);}
function is_id($id) {return (my_int($id) and $id > 0);}
function is_rid($rid) {return (is_array($rid) and array_keys($rid) == array(0, 1) and is_id($rid[0]) and is_id($rid[1]));}
function is_qid($qid) {return preg_match('/^[A-Z]{3}(\.|\:|\.\.)[A-Z]+$/', $qid);}