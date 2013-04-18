<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace rCredits\Testing;
use rCredits\Testing as t;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Backend as be;

global $base_path;
define('BASE_PATH', $base_path);

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function makeAccount($info = array(), $testOnly = FALSE) {
  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info, EXTR_PREFIX_ALL, 'my');

  if (!@$my_id) $my_id = r\acct::nextId(); // default to first available number
  $uid = uid($my_id); // change .AAB to 1, etc.
  $rebate = isset($my_rebate) ? $my_rebate: R_REBATE; 
  $fullName = @$my_fullName ?: ('makeAccount ' . randomString(20, 'A'));
  $name = u\shortName($fullName);
//  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
  $pass = @$my_password ?: '123';
  $mail = @$my_email ?: ''; // Don't use "$name@example.com"
  if (strpos(@$my_flags, '%') !== FALSE) {
    $flags = B_DFT | u\bit(isset($my_flags) ? @$my_flags : (B_MEMBER | B_OK));
  } else {
    $flags = 0;
    if (@$my_flags) foreach (u\ray($my_flags) as $one) {
      $bit = constant('B_' . strtoupper($one));
      $flags |= u\eq($bit, B_DFT, u\bit($bit));
    }
  }
  
  $community = @$my_communityUid ?: r\serverUid();
  u\EXPECT(!empty($community), 'empty community uid');

  if (isset($my_acctType)) {
    if ($my_acctType <= R_SELF_EMPLOYED) $flags |= u\bit(B_PERSONAL);
    if ($my_acctType >= R_SELF_EMPLOYED) $flags |= u\bit(B_COMPANY);
  }
  
  fixCountryAndState($info);

  foreach (u\ray('id email password acctType') as $one) unset($info[$one]);
  $info = array_merge($info, compact(u\ray('uid flags rebate name pass fullName mail community')));
  if ($testOnly) {
    if (!$acct = r\acct($uid)) return testOutput("No such uid: $uid");
    foreach ($info as $key => $value) {
      if (!eq($acct->$key, $value)) {
        if ($key == 'pass' and (!isset($my_password) or $acct->passwordOkay(@$my_password))) continue;
        return testOutput("Wrong value for $key. Expected:$value Got:" . $acct->$key);
      }
    }
    return TRUE;
  }
  
  if (!$acct = new r\acct($info)) return FALSE;
  
  if (@$my_picture) if (!makePicture($my_picture, $acct)) return FALSE;

  return $uid;
}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if called with $infoOnly TRUE, return assoc with the information actually written (called from verifyTx())
 *   otherwise return serial (treated as TRUE in test steps)
 */
function makeTransaction($info, $infoOnly = FALSE) {
  global $channel;
  extract($info, EXTR_PREFIX_ALL, 'my');

  if (@$my_tx_id) $xid = u\a2n(substr($my_tx_id, 4));
  $type = isset($my_type) ? $my_type : TX_TRANSFER;
  $created = @$my_created ? strtotime($my_created) : REQUEST_TIME;
  
  $payer = uid($my_from);
  $payerAgent = uid(@$my_from_agent ?: ($type == TX_TRANSFER ? $my_from : r\communityUid()));
  $payee = uid($my_to); 
  $payeeAgent = uid(@$my_to_agent ?: ($type == TX_TRANSFER ? $my_to : r\communityUid()));
  
  $for = @$my_purpose ?: 'cash';
  $goods = @$my_goods ?: (($type != TX_TRANSFER or u\forCash($for)) ? 0 : 1);
  $taking = ($my_from == 'community' or @$my_type == 'payment') ? 0 : 1;
  $r = round($my_amount - @$my_usd, 2);
  if ($goods) {
    $rebate = r\rebate($payer, $my_amount);
    $bonus = r\bonus($payee, $my_amount);
    $data = serialize(compact(u\ray('rebate bonus')));
  }
  foreach (u\ray('created from to purpose usd tx_id') as $one) unset($info[$one]);
  
  $info += compact(u\ray('xid created r payer payerAgent payee payeeAgent for type channel goods taking data'));
  if ($infoOnly or $type == TX_REBATE or $type == TX_BONUS) {
    $info['payerFor'] = $info['payeeFor'] = $for;
    unset($info['for']);
    return $info;
  }
  
  $txs = r\transact($info);
  return $txs[0]['xid'];
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function makeRelation($info, $infoOnly = FALSE) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $reid = u\num($my_id, u\a2n(substr($my_id, -3)));
  if (!$mainAcct = r\acct(t\uid($my_main))) return testOutput("main:$my_main is not an id");
  if (!$agtAcct = r\acct(t\uid($my_agent))) return testOutput("agent:$my_agent is not an id");
  u\EXPECT(!r\foreignServer(r\uidRegion($mainAcct->id)) or !r\foreignServer(r\uidRegion($agtAcct->id)));
  u\EXPECT($mainAcct->proSe() and $agtAcct->proSe());
  list ($main, $other) = array($mainAcct->id, $agtAcct->id);
  $info['permission'] = @$my_permission ? constant('PERM_' . strtoupper($my_permission)) : 0;
  
  foreach (u\ray('id main agent') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other'));
  return $infoOnly ? verifyRecord('r_relations', $info) : r\dbInsert('r_relations', $info);
}

/**
 * Return the credit info for the given uid.
 * To get just the balance, use uidCredit(uid)->balance
 * @param int $uid: return credit info for this account id
 */
function uidCredit($uid) {
  $info = be\creditInfo(compact('uid'));
  $balance = r\acct($uid)->balance();
  if ($uid >0 and $info->balance != $balance) { // (community amounts don't get cached)
    testOutput("Calculated balance different from cache: $info->balance <> $balance");
    return (object) array('balance' => 'error');
  } else return $info;
}

function uid($id) {
  return is_numeric($id) ? $id 
    : ($id == 'community' ? r\serverUid() 
    : r\unQuid(fullQid($id)));
}

/**
 * Accept abbreviations of alphabetic ids for accounts, transactions, and relations
 * @param string $q: the possibly-abbreviated id
 * @return: the full id (if $q is numeric or an already-full alphabetic id, return it unchanged)
 */
function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == '.' or $c1 == ':') ? (R_SERVER_ID . $q) : $q;
}

function getErrors() {
  $errors = \drupal_get_messages('error', FALSE);
  $errors = @$errors['error'] ?: array();
  foreach ($errors as $key => $value) {
    if (u\abbreviates('Unable to send e-mail', $value) or strpos(strip_tags($value), 'expected to be a reference, value given in drupal_retrieve_form()')) unset($errors[$key]); // ignore these when testing
  }
  return $errors;
}

/**
 * Find the message in the log table.
 * @param assoc $subs: subs for message iff message index is passed.
 */
function findMessage($type, $fields, $message = '', $subject = '', $subs = array()) {
  global $channel;

  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $myid = @r\acct()->id;
  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $fields['message'] = $message;
    if ($subject) { // must be email
      $fields['message'] = str_replace("\r\n", '', $fields['message']);
      $fields['subject'] = $subject;
    }
  } else unset($fields['message']); // empty message in features means NO message
  $agent = @r\acct()->agentId;
  testOutput(array('EXPECT' => '') + $fields, @$subject ? 'email' : 'output'); // keep this
  $info = json_encode($fields, JSON_UNESCAPED_SLASHES);
  $crit = "channel=:channel AND type=:type AND myid=:myid AND agent=:agent ORDER BY logid DESC";
//  debug(compact(u\ray('info type myid agent channel crit'))); // for debugging mysterious email mismatches
//  debug(compact(u\ray('info type myid agent channel')), '', 1);
//  debug("info=:info AND $crit"); debug(compact(u\ray('info type myid agent')));
  return r\dbExists('r_log', "info=:info AND $crit", compact(u\ray('info type myid agent channel')));
}

function findEmail($index, $email, $subs) {
  $subs = @$subs[0] ?: @$subs;
/* REDUNDANT? foreach (array('payerPurpose', 'payeePurpose') as $field) {
    if (($purpose = @$subs[$field]) and !$real) $subs[$field] = str_replace('#', '#a', $purpose); // would realFix() handle this?
  } */
  $esubs = array();
  foreach ($subs as $k => $value) $esubs['{' . $k . '}'] = $value;
  $message = strtr(r\emailBody($index), $esubs);
  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
  return findMessage('email', compact(u\ray('index email subject')), $message, $subject, $subs);
}

/**
 * Convert country and state names or abbreviations to the corresponding indices
 */
function fixCountryAndState(&$info) {
  extract(u\just('country state', $info));
  if (@$country) $info['country'] = $country = r\dbLookup('id', 'r_countries', ':country IN (name, iso_code)', compact('country'));
  if (@$state) $info['state'] = r\dbLookup('id', 'r_states', ':state IN (name, abbreviation) AND country_id=:country', compact('state', 'country'));
}

function makePicture($picture, $acct) {
  $filename = "$picture.jpg";
  $uri = "public://pictures/$filename";
  $filemime = 'image/jpeg';
  $status = 1;
  $picture = $uid = $fid = $acct->id;
  $info = compact(u\ray('fid uid filename uri filemime status'));
//  if (!r\dbInsert('file_managed', $info)) return FALSE;
//  if (!$acct->update(compact('picture'))) return FALSE;
  return TRUE;
}

function getPicture($picture) {
  return file_get_contents(R_PICTURE_DIR . "/$picture.jpg");
}

function lastMinuteSubs(&$string) {
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if ($string == '%chk') return ($string = '&#10004;');
  if ($string == '%ctty') return ($string = R_REGION_NAME);
}
  
  
function subAgo(&$string, $fmt) {
  if (!strpos($string, '-')) $string .= '-0d';
  $lastC = substr($string, strlen($string) - 1);
  list ($a, $b) = explode('-', $string);
  $bPreC = substr($b, 0, strlen($b) - 1);
  $periods = array('d' => 'days', 'w' => 'weeks', 'm' => 'months', 'y' => 'years');
  $period = $periods[$lastC];
//  debug(compact(u\ray('string fmt lastC a b bPreC period')));
  return ($string = strftime($fmt, strtotime("$bPreC $period ago")));
}

function hitServer($op, $agent, $code = '', $account_id = '', $extra = array()) {
  if (!$code) unset($code);
  if (!$account_id) unset($account_id); else $account_id = trim($account_id);
  if ($op != 'startup') { // startup RETURNS my_id rather than asking for it
    $agent = trim($agent);
    if (strpos($agent, ':') or !@$code) {
      $my_id = $agent;
    } else {
      $acct = r\acct(r\dbLookup('owner', 'r_smarts', 'code=:code', compact('code')), $agent);
      $my_id = $acct ? $acct->qid() : $agent; // accommodate bad agents
    }
  }
  $json = json_encode(compact(u\ray('op my_id code account_id')) + $extra, JSON_UNESCAPED_SLASHES);
  testOutput($json, 'input'); // keep this
  r\Smart\api(compact('json'));
//  be\send($json); // remember the server's response (this is the real test, once most bugs are gone)
  return TRUE;
}

function snapShot(&$form) { // (additional sta values have no effect at this stage)
  addAttribs($form);
  return; // the rest is redundant
  //global $formOut['text']; $zot = $form; $formOut['text'] = strip(\render($zot)); // for testing
  //testOutput('ACTUAL or EXPECTED: ' . $formOut['text'], 'screen');
}

function addAttribs(&$thing) {
  foreach ($thing as $key => $value) { // else Drupal complains when rendering
    if (substr($key, 0, 1) == '#') continue;
    if (!is_array($value)) continue;
    if (!isset($value['#attributes']['id'])) $thing[$key]['#attributes']['id'] = 'edit-' . strtolower($key);
    if (!isset($value['#title_display'])) $thing[$key]['#title_display'] = 'before';
    addAttribs($thing[$key]);
  }
}

function logIn($id) {
  return $id == '?' ? '' : r\acct::setDefault(r\acct(t\uid($id)));
}

function verifyTx($info) {
  $info2 = makeTransaction($info, TRUE);
  return verifyRecord('r_txs', $info2);
}

/**
 * Compare the data to a corresponding record in the table.
 * Return TRUE if the record matches the data.
 * Example call:
 *   $info = makeRelation($info, TRUE);
 *   return verifyRecord('relations', $info);
 */
function verifyRecord($table, $info) {
  if (!is_array($info)) die('in verifyRecord: ' . print_r(debug_backtrace(), 1)); // keep
  unset($info['data']);
  unset($info['created']);
  foreach ($info as $key => $value) $crit[] = "$key<=>:$key"; // null-safe ==
  $crit = join(' AND ', $crit);
  $result = r\dbExists($table, $crit, $info);
  if (!$result) testOutput("verifyRecord failed. table=$table crit=$crit EXPECTED info=" . print_r($info, 1));
  return $result;
}

/**
 * Simulate a visit to the page, return its form.
 * @param string $page: the Drupal URL to visit
 * @return: the form that would normally be displayed by the browser
 */
function pageForm($page) {
  global $formOut;

  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);
  if (u\abbreviates('/user/', $page)) {
    $sta = array();
    list ($form, $function) = strpos($page, '/register') ?
        array('user_register_form', 'formRegister')
      : (strpos($page, '/login') ?
        array('user_login', 'formLogin') : array());
    $function = 'rCredits\\Web\\' . $function;
    $form = strpos($page, '/login') ? array() : \drupal_get_form($form, $args);
    $function($form, $sta, $args);
    $form2 = $form; $rent = render($form2);
    getFormOut($rent);
    return $form;
  }     

  $result = menu_execute_active_handler($page, FALSE);
  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found' 
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\testOutput("page \"$page\": $result in t\\pageForm()");
  } else getFormOut($result);
  return $result;
}

function getFormOut($out) {
  global $formOut;
  $formOut = array();
  $formOut['options'] = t\dropdownOptions($out);
  $formOut['steps'] = t\getSteps($out);
  $formOut['done'] = join('', t\getSteps($out, TRUE));
  $formOut['text'] = t\strip($out);
  t\testOutput('ACTUAL: ' . $formOut['text'], 'screen');
}

/**
 * Return an array of steps in the output
 * @param string $out: the rendered form
 * @param bool $set: TRUE = return just the done steps, FALSE = just the not-done steps, DEFAULT = both
 * @return: array of step numbers
 */
function getSteps($out, $set = NULL) {
  if (!@$set) $set = isset($set) ? '>' : '';
  if (!preg_match_all("/done-(\d) state$set/", $out, $matches)) return array();
  return $matches[1];
}

function postProcess($form, $formName, $type) {
  $functions = @$form["#$type"];
  if (!@$functions or @($functions == '<')) {
    $function = "rCredits\\Web\\form{$formName}_$type";
    $functions = function_exists($function) ? array($function) : array();
  }
  return $functions;
}

function postLogout($function) {
  return TRUE; // never actually log out
  global $skipToStep;
/*  if ($messages = @cache_get('t_messages')->data) {
    extract(u\just('msgs skipToStep', $messages));
    if ($skipToStep == $function) {
      if (strlen(serialize($msgs)) > strlen(serialize(@$_SESSION['messages']))) {
        foreach (u\ray('status error notice') as $type) {
          $_SESSION['messages'][$type] = array_merge((@$msgs[$type] ?: array()), (@$_SESSION['messages'][$type] ?: array()));
          if (empty($_SESSION['messages'][$type])) unset($_SESSION['messages'][$type]);
        }
      }
//      cache_set('t_messages', FALSE);
u\deb('postLogout returning TRUE, skipToStep was ='.@$skipToStep.' (now NULL)');
      $skipToStep = NULL; // strangely, this has to be = NULL not unset()
      return TRUE;
    }
  }
  */
u\deb('postLogout returning FALSE, about to save skiptostep '.$function);
  $vars = array(
    'msgs' => @$_SESSION['messages'] ?: array(),
    'skipToStep' => $function, // tell gherkin function to skip steps in interrupted scene until this step
  );
//  cache_set('t_messages', $vars); // save the test results so far
  return FALSE;
}

function dropdownOptions($form) {
  if (!preg_match_all('~<option [^>]*>.*?</option>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function strip($form) {

  $form = str_replace('&nbsp;', ' ', $form);
  $form = preg_replace('~<fieldset id="help".*?</fieldset>~sm', '', $form); // remove help div
  $form = preg_replace('~<div[^>]*class="invisible.*?</div>~sm', '', $form); // remove invisible divs (not nested)
  $form = preg_replace('~<option value="[^"]*">.*?</option>~sm', '', $form); // remove unselected options
  $form = preg_replace('~<input type="hidden".*?/>~sm', '', $form); // remove hidden inputs
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/sm', '$1', $form); // show button labels
  $form = preg_replace('/> *</sm', '> <', $form); // make sure there's exactly one space between things
  return preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form));
}

function eq($a, $b) {
  $result = (is_numeric($a) and is_numeric($b)) ? 
      (($a - $b) * ($a - $b) < .001)
    : ($a == $b or (empty($a) and empty($b)));
  if (!$result) echo 'not equal: ' . print_r(compact('a','b'), 1); // keep for now
  if (!$result) testOutput("unequal: a=$a b=$b", 'error'); // keep for now
  return $result;
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  if (!t\EST()) return $_SERVER[$key];
  global $T_SERVER;
  if (isset($value)) $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 * Not to be confused with the color() function in test.php
 */
function color($msg, $color = 'normal') {
  $colors = array(
    'normal' => 'ivory',
    'email' => 'lightblue',
    'input' => 'orange',
    'output' => 'thistle',
    'screen' => 'burlywood',
    'error' => 'salmon',
  );
  $color = @$colors[$color] ?: $color;
  $msg = print_r($msg, 1); // keep this
  return "<pre style='background-color:$color;'>$msg</pre>";
}

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function testOutput($msg, $color = 'error') {
  if (t\EST()) \drupal_set_message(color($msg, $color));
  return $color == 'error' ? FALSE : TRUE;
}

function EST() {return (@TESTING == 1);}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch) {
  global $channel; 
  $channel = $ch;
  $cgfUid = \variable_get('cgf_uid');
  $files = 'r_sms r_txs r_relations r_log r_invites r_companies r_gifts r_smarts r_proxies r_usd r_user_industries';

  global $sms_devel; $sms_devel = TRUE;
  r\dbQ('DELETE FROM users WHERE uid>:cgfUid OR (uid>1 AND uid<:cgfUid-1)', compact('cgfUid'));
  foreach (u\ray($files) as $one) r\dbQ("TRUNCATE $one");

  r\acct::_clear(); // empty the acct cache
  \drupal_static_reset(); // clear out drupal central storage
}

function fixPageArgs($page) {
  $special = array(
    '(by|id)=([A-Z]{3}?[\.:][A-Z]{3})',
    '(flow)=(.*?)',
  );
  foreach ($special as $one) if (preg_match($pat = "`(/|&)$one(&|$)`", $page, $matches)) {
    list ($zot, $start, $word, $what, $end) = $matches;
    if ($word == 'by' or $word == 'id') $what = t\uid($what);
    if ($word == 'flow') $what = constant('FLOW_' . strtoupper($what));
    $page = preg_replace($pat, "$start$word=$what$end", $page);
  }
  return $page;
}

/**
 * Say whether the most recent page included the given text or options.
 * @param array $content: array of assocs -- named values to check for (ignore the keys)
 * @param string $type: description of what to check for (default 'text')
 * @param bool $showDebugs: whether to report debugging info while looking
 * @return TRUE if all the content is included
 */
function shownWith($content, $type = 'text', $showDebugs = TRUE) {
  global $formOut;
  $model = $formOut[$type];
    
  $xi = $i = 0;

  foreach ($content as $fields) {
    foreach ($fields as $key => $one) if (strpos($one, '%') !== FALSE) t\lastMinuteSubs($fields[$key]);
    if ($showDebugs) debug("EXPECT page with " . print_r($fields, 1)); // keep this
    foreach ($fields as $one) { // ignore the keys here
      if ($one !== '') $i = strpos($model, $one, $xi = $i);
      if ($showDebugs) t\testOutput(compact('xi','i','one'));
      if ($i === FALSE) {
        if ($showDebugs) t\testOutput(substr($model, 0, $xi) . ' <b style="font-size:200%;">[OK until here]</b> ' . substr($model, $xi));
        return FALSE;
      } else $i += strlen($one);
    }
  }
  return TRUE;
}
