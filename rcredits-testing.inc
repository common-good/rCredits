<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace rCredits\Testing;
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
  $rebate_percent = isset($my_rebate) ? $my_rebate: (R_REBATE * 100); 
  $full_name = @$my_full_name ?: ('makeAccount ' . randomString(20, 'A'));
  $name = u\shortName($full_name);
  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
  $mail = @$my_email ?: ''; // Don't use "$name@example.com"
  $flags = BIT_DEFAULTS | (isset($my_flags) ? $my_flags : 0);
  $community_uid = @$my_communityUid ?: r\serverUid();
  u\EXPECT(!empty($community_uid), 'empty community uid');
  fixCountryAndState($info);

  foreach (u\ray('id email password') as $one) unset($info[$one]);
  
  $info = array_merge($info, compact(u\ray('uid flags rebate_percent name pass full_name mail community_uid')));
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
  $state = TX_DONE;
  $created = @$my_created ? strtotime($my_created) : REQUEST_TIME;
  
  $payer = uid($my_from);
  $payer_agent = uid(@$my_from_agent ?: ($type == TX_TRANSFER ? $my_from : r\communityUid()));
  $payee = uid($my_to); 
  $payee_agent = uid(@$my_to_agent ?: ($type == TX_TRANSFER ? $my_to : r\communityUid()));
  $real = strpos(@$my_tx_id, ':') !== FALSE ? FALSE : (r\isRTrader($payer) and r\isRTrader($payee)); // even handles asif signup bonus to rTrader

  //  debug(compact('real','payer','payee','my_tx_id','xid'));
//  debug(r\isRTrader($payer));
//  debug(r\isRTrader($payee));
  
  $payer_for = @$my_purpose ?: 'cash';
  $payee_for = @$my_purpose ?: 'cash';
  $goods = @$my_goods ?: (($type != TX_TRANSFER or u\forCash($payer_for)) ? 0 : 1);
  $taking = ($my_from == 'community' or @$my_type == 'payment') ? 0 : 1;
  if ($goods) {
    $rebate = r\rebate($payer, $my_amount);
    $bonus = r\bonus($payee, $my_amount);
    $data = serialize(compact(u\ray('rebate bonus')));
  }
  foreach (u\ray('created from to purpose tx_id') as $one) unset($info[$one]);
  $info += compact(u\ray('xid created payer payer_agent payee payee_agent payer_for payee_for type channel goods taking state data'));
  if ($infoOnly) return $info;
  
  \drupal_write_record($real ? 'r_txs' : 'r_asif', $info);
  if (!@$my_serial) {
    $serial = $xid = $info['xid'];
    r\dbQ('UPDATE {zxs} SET serial=:serial WHERE xid=:xid', compact(u\ray('serial xid real')));
  } else $serial = $my_serial;
  return $serial;
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function makeRelation($info, $infoOnly = FALSE) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  
  $reid = u\num($my_id, u\a2n(substr($my_id, -3)));
  if (!$main = r\acct(fullQid($my_main))) return testOutput("main:$my_main is not an id");
  if (!$agent = r\acct(fullQid($my_agent))) return testOutput("agent:$my_agent is not an id");
  u\EXPECT(!r\isForeign($main->regionId) or !r\isForeign($agent->regionId));
  u\EXPECT($main->proSe() and $agent->proSe());
  list ($main, $other) = array($main->id, $agent->id);
  if (r\isForeign($main->regionId)) list ($main, $foreign_uid) = array($main->regionId, $main->id);
  if (r\isForeign($agent->regionId)) list ($other, $foreign_uid) = array($agent->regionId, $agent->id);
  
  if (@$my_permission) {
    $perms = array_flip($GLOBALS['share permissions']);
    $info['permission'] = $perms[$my_permission]; 
  }
  
  foreach (u\ray('id main agent') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other foreign_uid'));
  return $infoOnly ? verifyRecord('r_relations', $info) : \drupal_write_record('r_relations', $info);
}

/**
 * Return the credit info for the given uid.
 * To get just the balance, use uidCredit(uid)->balance
 * @param int $uid: return credit info for this account id
 * @param string $real: "asif" or "real" or NULL (the default, means real iff uid is an rTrader)
 */
function uidCredit($uid, $real = NULL) {
  $real = isset($real) ? ($real == 'real') : r\isRTrader($uid);
  return be\creditInfo(compact('uid', 'real'));
}

function uid($id) {
  return is_numeric($id) ? 
      $id 
    : ($id == 'community' ? 
        r\serverUid() 
    : u\a2n(substr(str_replace(R_SERVER_ID . '.', '.', $id), 1) 
  ));
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
 * @param assoc $subs: subs for message iff message index is passed. Also may contain [real]
 */
function findMessage($type, $fields, $message = '', $subject = '', $subs = array()) {
  global $channel;

  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $cuid = @r\acct()->id;
  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $real = isset($subs['real']) ? $subs['real'] : NULL;
    $fields['message'] = realFix($message, $real);
    if ($subject) { // must be email
      $fields['message'] = str_replace("\r\n", '', $fields['message']);
      $fields['subject'] = realFix($subject, $real);
    }
  } else unset($fields['message']); // empty message in features means NO message
  $agent = @r\acct()->agentId;
  \drupal_set_message(color(array('EXPECT' => '') + $fields, @$subject ? 'email' : 'output')); // keep this
  $info = json_encode($fields, JSON_UNESCAPED_SLASHES);
  $crit = "channel=:channel AND type=:type AND cuid=:cuid AND agent=:agent ORDER BY logid DESC";
//  debug(compact(u\ray('info type cuid agent channel crit'))); // for debugging mysterious email mismatches
//  debug(compact(u\ray('info type cuid agent channel')), '', 1);
//  debug("info=:info AND $crit"); debug(compact(u\ray('info type cuid agent')));
  return r\dbLookup(1, 'r_log', "info=:info AND $crit", compact(u\ray('info type cuid agent channel')));
}

function findEmail($key, $email, $subs, $real = FALSE) {
  $subs = @$subs[0] ?: @$subs;
  debug($subs);
/* REDUNDANT? foreach (array('payerPurpose', 'payeePurpose') as $field) {
    if (($purpose = @$subs[$field]) and !$real) $subs[$field] = str_replace('#', '#a', $purpose); // would realFix() handle this?
  } */
  $esubs = array();
  foreach ($subs as $k => $value) $esubs['{' . $k . '}'] = $value;
  $message = strtr(r\emailBody($key), $esubs);
  debug($esubs);
  $subject = strtr($GLOBALS['emailSubjects'][$key], $esubs);
  return findMessage('email', compact(u\ray('key email subject')), $message, $subject, $subs);
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
  if (!\drupal_write_record('file_managed', $info)) return FALSE;
  if (!$acct->update(compact('picture'))) return FALSE;
  return TRUE;
}

function getPicture($picture) {
  global $picturePath;
  return file_get_contents("$picturePath/$picture.jpg");
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
  \drupal_set_message(color($json, 'input')); // keep this
  r\Smart\api(compact('json'));
//  be\send($json); // remember the server's response (this is the real test, once most bugs are gone)
  return TRUE;
}

function snapShot(&$form) { // (additional sta values have no effect at this stage)
  addAttribs($form);
  global $formOutput; $zot = $form; $formOutput = strip(\render($zot)); // for testing
  testOutput('ACTUAL or EXPECTED: ' . $formOutput, 'screen');
}

function addAttribs(&$thing) {
  foreach ($thing as $key => $value) { // else Drupal complains when rendering
    if (substr($key, 0, 1) == '#') continue;
    if (!isset($value['#attributes']['id'])) $thing[$key]['#attributes']['id'] = "edit-$key";
    if (!isset($value['#title_display'])) $thing[$key]['#title_display'] = 'before';
    addAttribs($thing[$key]);
  }
}

function logIn($id) {
u\deb('before logIn');
  if ($id != '?') r\acct::setDefault(r\acct($id));
u\deb('after logIn');
}

function verifyTx($info) {
  $info2 = makeTransaction($info, TRUE);
  $info2['real'] = (r\isRTrader($info2['payer']) and r\isRTrader($info2['payee'])) ? 1 : 0;
  return verifyRecord('{zxs}', $info2);
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
  foreach ($info as $key => $value) if (!($table == '{zxs}' and $key == 'real')) $crit[] = "$key<=>:$key"; // null-safe ==
  $crit = join(' AND ', $crit);
  $result = r\dbLookup(1, $table, $crit, $info);
  if (!$result) testOutput("verifyRecord failed. table=$table crit=$crit info=" . print_r($info, 1));
  return $result;
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
  global $skipToStep;
  if ($messages = @cache_get('t_messages')->data) {
    extract(u\just('msgs skipToStep', $messages));
    if ($skipToStep == $function) {
      if (strlen(serialize($msgs)) > strlen(serialize(@$_SESSION['messages']))) {
        foreach (u\ray('status error notice') as $type) {
          $_SESSION['messages'][$type] = array_merge((@$msgs[$type] ?: array()), (@$_SESSION['messages'][$type] ?: array()));
          if (empty($_SESSION['messages'][$type])) unset($_SESSION['messages'][$type]);
        }
      }
      cache_set('t_messages', FALSE);
u\deb('postLogout returning TRUE, skipToStep was ='.@$skipToStep.' (now NULL)');
      $skipToStep = NULL; // strangely, this has to be = NULL not unset()
      return TRUE;
    }
  }
u\deb('postLogout returning FALSE, about to save skiptostep '.$function);
  $vars = array(
    'msgs' => @$_SESSION['messages'] ?: array(),
    'skipToStep' => $function, // tell gherkin function to skip steps in interrupted scene until this step
  );
  cache_set('t_messages', $vars); // save the test results so far
  return FALSE;
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
  if (!$result) \drupal_set_message(color("unequal: a=$a b=$b", 'error')); // keep for now
  return $result;
}

function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == '.' or $c1 == ':') ? (R_SERVER_ID . $q) : $q;
}

function realFix($msg, $real = FALSE) {
  $mapReal = array(
    '@asif' => '', 
    '{asif}' => '',
  );
  $mapNot = array(
    'ransaction #' => 'ransaction #a', 
    'reverses #' => 'reverses #a', 
    'nvoice #' => 'nvoice #a', 
    '@asif' => R_ASIF_TEXT, 
    '{asif}' => R_ASIF_TEXT,
  );
  return strtr($msg, $real ? $mapReal : $mapNot);
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  global $T_SERVER;
  if (isset($value)) $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 */
function color($msg, $color = 'normal') {
  $colors = array(
    'normal' => 'ivory',
    'email' => 'lightblue',
    'input' => 'orange',
    'output' => 'thistle',
    'screen' => 'burlywood',
  );
  $color = @$colors[$color] ?: $color;
  $msg = print_r($msg, 1); // keep this
  return "<pre style='background-color:$color;'>$msg</pre>";
}

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function testOutput($msg, $color = 'normal') {
  if (@TESTING == 1) \drupal_set_message(color($msg, $color));
  return $color == 'normal' ? FALSE : TRUE;
}
