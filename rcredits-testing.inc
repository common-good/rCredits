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

function makeRecords($list, $type, $testSeparate = FALSE) {
  global $testOnly;
  $table = 'r_' . strtolower($type) . 's';
  $function = 'rCredits\\Testing\\';
  if (!function_exists($function . "make$type")) $type = 'Thing';
  $function .= ($testSeparate and $testOnly) ? "verify$type" : "make$type";
  foreach ($list as $one) if (!$function($one, $testOnly, $table)) return FALSE;
  return TRUE;
}

/*
 * Generic record-maker/record-tester.
 */
function makeThing($info, $testOnly, $table) {
  extract($info);
  $info['uid'] = t\uid($id); unset($info['id']);
  return $testOnly ? verifyRecord($table, $info) : r\dbInsert($table, $info);
}

function gifts($list) {return makeRecords($list, 'Gift');}
function notices($list) {return makeRecords($list, 'Notice');}
function members($list) {return makeRecords($list, 'Account');}
function relations($list) {return makeRecords($list, 'Relation');}
function transactions($list) {return makeRecords($list, 'Tx', TRUE);}
function balances($list) {return makeRecords($list, 'Balances');}

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function makeAccount($info = array(), $testOnly = FALSE) {
  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info);

  u\setDft($id, r\acct::nextId()); // default to first available number
  $uid = uid($id); // change .AAB to 1, etc.
  if (@$email) $mail = $info['mail'] = $email;

  if (!$testOnly) { // when making, set many defaults
    u\setDft($rebate, R_REBATE); 
    u\setDft($fullName, 'makeAccount ' . randomString(20, 'word'));
    $name = u\shortName($fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
    u\setDft($pass, '123');
    u\setDft($mail, ''); // Don't use "$name@example.com"
    u\setDft($community, r\serverUid());
    u\setDft($postalCode, '01301');
    $moneyFields = 'share committed floor r usd rewards'; // not minimum because NULL means not set, in Membership page
    foreach (u\ray($moneyFields) as $one) u\setDft($$one, 0);
    $testFields = ' rebate fullName name pass mail community ' . $moneyFields;
  } else $testFields = '';

  $flags0 = @$flags;
    /*if (strpos($flags0, '%') !== FALSE) {
    $flags = B_DFT | u\bit(isset($flags0) ? @$flags0 : (u\bit(B_MEMBER) | u\bit(B_OK)));
  } else { */
  $flags = 0;
  if (@$flags0) foreach (u\ray($flags0) as $one) {
    $bit = constant('B_' . strtoupper($one));
    $flags |= u\eq($bit, B_DFT, u\bit($bit));
  }
  if ($flags & u\bit(B_OK)) $flags |= u\bit(B_MEMBER);

  if (isset($acctType)) {
    if ($acctType <= R_SELF_EMPLOYED) $flags |= u\bit(B_PERSONAL);
    if ($acctType >= R_SELF_EMPLOYED) $flags |= u\bit(B_COMPANY);
  }
  
  fixCountryAndState($info);

  foreach (u\ray('id email acctType dwolla') as $one) unset($info[$one]);
  $info = compact(u\ray('uid flags ' . $testFields)) + $info;
  if ($testOnly) {
    if (!$acct = r\acct($uid)) return testOutput("No such uid: $uid");
    foreach ($info as $key => $value) {
      if (!eq($acct->$key, $value)) {
        if ($key == 'pass' and (!isset($pass) or r\Web\passwordOkay($acct, @$pass))) continue;
        return testOutput("Wrong value for $key. Expected:$value Got:" . $acct->$key);
      }
    }
    return TRUE;
  }
  
  if (!$acct = new r\acct($info)) return FALSE;

  if ($usdAccount = str_replace('-', '', @$dwolla)) {
    $acct->update(compact('usdAccount')); // this gets crunched, so save before compare
    if ($usdAccount != DW_TEST_ACCT) {
      $num = r\dbLookup('usdAccount', 'users', 'uid=:uid', compact('uid')); // get crunched version
      if ($tester = r\dbLookup('uid', 'users', 'usdAccount=:num', compact('num'))) {
        $secure = r\acct($tester)->secure;
        $acct->update(compact('secure'));
      }
    }
  }
  
  if (@$picture) if (!makePicture($picture, $acct)) return FALSE;

  return $uid;
}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if called with $testOnly TRUE, return assoc with the information actually written (called from verifyTxs())
 *   otherwise return serial (treated as TRUE in test steps)
 */
function makeTx($info, $testOnly = FALSE) {
  global $channel;
  extract($info);

  if (@$xid and !is_numeric($xid)) $xid = u\a2n(substr(strpbrk($xid, R_MEMBER_MARK . R_AGENT_MARK), 1));
  u\setDft($type, TX_TRANSFER);
  u\setDft($state, TX_DONE);
  u\setDft($created, REQUEST_TIME);
  foreach (u\ray('type state') as $one) if (!is_numeric($$one)) $$one = constant('TX_' . strtoupper($$one));
  $payer = uid($from);
  $payerAgent = uid(@$fromAgent ?: ($type == TX_TRANSFER ? $from : r\communityUid()));
  $payee = uid($to); 
  $payeeAgent = uid(@$toAgent ?: $to );
  u\setDft($r, $amount);
  u\setDft($taking, 0);
  
  $for = @$purpose ?: 'cash';
  if (strpos($for, ',')) $for = explode(',', $for); // specifying both payerFor and payeeFor

  u\setDft($goods, ($type != TX_TRANSFER or u\forCash($for)) ? 0 : 1);
  if ($goods) list ($rebate, $bonus) = array(r\rebate($payer, $r), r\bonus($payee, $r));
  foreach (u\ray('from to purpose for') as $one) unset($info[$one]);
  
  $info = compact(u\ray('xid created r payer payerAgent payee payeeAgent for type state channel goods taking data')) + $info;
  if ($testOnly or $type == TX_REBATE or $type == TX_BONUS) {
    list ($info['payerFor'], $info['payeeFor']) = is_array($for) ? $for : array($for, $for);
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
function makeRelation($info, $testOnly = FALSE) {
  extract($info);
  
  $reid = u\num($id, u\a2n(substr($id, -3)));
  if (!$mainAcct = r\acct(t\uid($main))) return testOutput("main:$main is not an id");
  if (!$agtAcct = r\acct(t\uid($agent))) return testOutput("agent:$agent is not an id");
  u\EXPECT(!r\foreignServer(r\uidRegion($mainAcct->id)) or !r\foreignServer(r\uidRegion($agtAcct->id)));
  u\EXPECT($mainAcct->proSe() and $agtAcct->proSe());
  list ($main, $other) = array($mainAcct->id, $agtAcct->id);
  $info['permission'] = @$permission ? constant('PERM_' . strtoupper($permission)) : 0;
  if (@$draw) $agtAcct->setBit(B_DRAWS);
  
  foreach (u\ray('id main agent') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other'));
  return $testOnly ? verifyRecord('r_relations', $info) : r\dbInsert('r_relations', $info);
}

/**
 * Return TRUE if the notice has been queued to member $id
 */
function notice($notice, $id, $subs) {
  global $testOnly; if (!$testOnly) return FALSE;
  $uid = uid($id);
  if (is_array($subs = @$subs[0])) foreach ($subs as $key => $value) {
    if (preg_match('/^\$(.*)(r|us)?$/U', $value, $matches)) {
      list ($num, $currency) = array($matches[1], @$matches[2]);
      if ($currency == 'us') $currency = 'u';
      $subs[$key] = u\formatAmount($num, 's$' . $currency);
    }
  }
  $message = u\tt($notice, @$subs);
  $sent = @$subs['sent'] ?: 0;
  $info = compact(u\ray('uid message sent'));
  return r\dbExists('r_notices', $info) ?: t\testOutput('Expected notice ' . print_r($info, 1));
}
  
/**
 * Make the cached balances and other numeric fields be thus and such.
 * Note that this will be at odds with any calculated balances, 
 *   so a scenario cannot test r, usd, and rewards with "balances:" after setting them up this way.
 */
function makeBalances($one, $testOnly) {
  extract(u\just('id r usd rewards minimum share committed', $one));
  $a = r\acct($id = t\uid($id));
  if (!$testOnly) {
    if (!is_array($one)) echo 'makeBalances: ' . print_r($one, 1) . print_r(debug_backtrace(), 1);
    unset($one['id']);
    if (@$usd and !is_numeric($usd)) list ($one['usd']) = t\parseAmt($usd, $a);
    return ($a->update($one));
  }

  // verify
  $calc = t\uidCredit($id);
  if (isset($r)) {
    if (t\badEq('calc r', $calc->r, $r, $id)) return FALSE;
    if ($id > 0 and t\badEq('cached r', $a->r, $r, $id)) return FALSE; // (ctty not cached)
  }
  if ($id < 0) return TRUE; // only r is tested for community (nothing is cached for ctty)
  if (isset($rewards)) {
    if (t\badEq('calc rewards', $calc->rewards, $rewards, $id)) return FALSE;
    if (t\badEq('cached rewards', $a->rewards, $rewards, $id)) return FALSE;
  }
  if (isset($usd)) {
    if (!is_numeric($usd)) {
      list ($usd, $bal) = t\parseAmt($usd, $a);
      if (t\badEq('REAL usd', $bal, $usd, $id)) return FALSE;
    }
// (can't tell because it starts nonzero) if (!t\eq($calc->usd, $usd)) return t\testOutput('bad calc usd');
    if (t\badEq('cached usd', $a->usd, $usd, $id)) return FALSE;
  }
  foreach (u\ray('minimum share committed') as $one) {
    if (isset($$one)) if (t\badEq("db $one", $a->$one, $$one, $id)) return FALSE;
  }
  return TRUE;
}

function usdTransferCount($where = '') {return r\dbLookup('COUNT(*)', 'r_usd', 'payee=0');}

function usdTransfers($tx) {
  global $testOnly; if(!$testOnly) return FALSE;
  extract(u\just('payer payee amount', $tx[0]));
  $payer = t\uid($payer);
  $payee =(@$payee and !is_numeric($payee)) ? t\uid($payee) : 0;
  return r\dbExists('r_usd', 'payer=:payer AND amount=:amount and payee=:payee', compact(u\ray('payer payee amount')));
}

function usd($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(u\just('id usd', $one));
    $a = r\acct(t\uid($id));
    if (!is_numeric($usd)) list ($usd) = t\parseAmt($usd, $a);
    if ($testOnly ? ($a->usd != $usd) : !$a->update(compact('usd'))) return FALSE;
  }
  return TRUE;
}

/**
 * Return the credit info for the given uid.
 * To get just the r balance, use uidCredit(uid)->r
 * @param int $uid: return credit info for this account id
 */
function uidCredit($uid) {
  $info = be\creditInfo(compact('uid'));
  $a = r\acct($uid);
  if ($uid >0) foreach (u\ray('r rewards') as $one) { // (community amounts don't get cached) (don't include usd)
    if (!t\eq($v1 = $info->$one, $v2 = $a->$one + 0)) {
      testOutput("Calculated $one different from cache: $v1 <> $v2");
      $info->$one = 'error';
    }
  }
  return $info;
}

function uid($id) {
  return is_numeric($id) ? $id
    : (($id == 'community' or $id == 'ctty') ? r\serverUid() 
    : ($id == 'cgf' ? \variable_get('cgf_uid')
    : r\unQuid(fullQid($id))));
}

/**
 * Accept abbreviations of alphabetic ids for accounts, transactions, and relations
 * @param string $q: the possibly-abbreviated id
 * @return: the full id (if $q is numeric or an already-full alphabetic id, return it unchanged)
 */
function fullQid($q) {
  $c1 = substr($q, 0, 1);
  return ($c1 == R_MEMBER_MARK or $c1 == R_AGENT_MARK) ? (R_SERVER_ID . $q) : $q;
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
  $crit = "channel=:channel AND type=:type AND myid<=>:myid AND agent<=>:agent ORDER BY logid DESC";
// debug(compact(u\ray('info type myid agent channel crit'))); // for debugging mysterious email mismatches
// debug(compact(u\ray('info type myid agent channel')), '', 1);
// debug("info=:info AND $crit"); debug(compact(u\ray('info type myid agent')));
  return r\dbExists('r_log', "info=:info AND $crit", compact(u\ray('info type myid agent channel')));
}

function weTellStaff($topic, $subs = array()) {
  global $testOnly; if (!$testOnly) return FALSE;
  if (!@$subs[0]) $subs[0] = array();
  foreach ($subs as $one) {
    $index = 'tell-staff';
    $subject = str_replace('{topic}', $topic, $GLOBALS['emailSubjects'][$index]);
    $find = '%"index":"@index","email":"@R_ORG_EMAIL","subject":"@subject"%';
    if ($id = @$one['id']) {
      $id = t\uid($id);
      $find .= "(#$id)%";
      unset($one['id']);
    }
    $find = u\tt($find, compact('index', 'subject'));
    foreach ($one as $k => $value) $find .= "$value%";
    if (!$last = r\dbLookup('info', 'r_log', "info LIKE '$find' ORDER BY logid DESC")) {
      if (count($subs) > 1) testOutput("tell-staff can't find $find");
      return FALSE;
    }
  }
  return TRUE;
}

function findEmail($index, $email, $subs, $noBody = FALSE) {
  global $testOnly; if (!$testOnly) return FALSE;
  $subs = @$subs[0] ?: @$subs;
  $esubs = array();
  foreach ($subs as $k => $value) {
    if (strpos($value, ',')) $value = '<li>' . join("</li>\n<li>", explode(',', $value)) . "</li>\n";
    $esubs['{' . $k . '}'] = $value;
  }
  $message = strtr($noBody ? r\emailTemplate($index) : r\emailBody($index), $esubs);
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
  return file_get_contents(DRUPAL_ROOT . R_PICTURE_DIR . "/$picture.jpg");
}

function lastMinuteSubs($string) {
  if (u\abbreviates('%ymd', $string)) return subAgo($string, '%Y-%m-%d');
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if ($string == '%chk') return '&#10004;';
  if ($string == '%ctty') return R_REGION_NAME;
}
  
/**
 * Return the date, formatted as desired.
 * @param string $string: a named date format, dash, how long ago (eg %dmy-3d means 3 days ago "dmy" format)
 * @param string $fmt: what strftime format to use
 */ 
function subAgo($string, $fmt) {
  if (!strpos($string, '-')) $string .= '-0d';
  $lastC = substr($string, strlen($string) - 1);
  list ($a, $b) = explode('-', $string); // $a is ignored
  $bPreC = substr($b, 0, strlen($b) - 1);
  $periods = array('d' => 'days', 'w' => 'weeks', 'm' => 'months', 'y' => 'years');
  $period = $periods[$lastC];
//  debug(compact(u\ray('string fmt lastC a b bPreC period')));
  return (strftime($fmt, strtotime("$bPreC $period ago")));
}

function hitServer($op, $agent, $code = '', $account_id = '', $extra = array()) {
  if (!$code) unset($code);
  if (!$account_id) unset($account_id); else $account_id = trim($account_id);
  if ($op != 'startup') { // startup RETURNS my_id rather than asking for it
    $agent = trim($agent);
    if (strpos($agent, R_AGENT_MARK) or !@$code) {
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

/**
 * Log the given id in.
 * @param string $id: full or partial quid or '?' (meaning anonymous user, so don't log in)
 * @return: successful (always successful for anonymous)
 */
function logIn($id) {
  return $id == '?' ? TRUE : r\acct::setDefault(r\acct(t\fullQid($id))); // fullQid not uid
}

function verifyTx($info) {
  $info2 = makeTx($info, TRUE);
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
  foreach ($info as $key => $value) {
    $crit[] = $value === '' ? "($key IS NULL or $key='')"
    : ((in_array($key, u\ray(R_DATE_FIELDS)) or stripos($key, 'date') !== FALSE) ? 
      "$key-($key%:DAY_SECS)<=>:$key-(:$key%:DAY_SECS)" 
      : "$key<=>:$key"); // null-safe ==
  }
  $crit = join(' AND ', $crit);
  $result = r\dbExists($table, $crit, $info);
  if (!$result) testOutput("verifyRecord failed. table=$table crit=$crit EXPECTED info=" . print_r($info, 1)); // keep
  return $result;
}

/**
 * Simulate completion (and possible confirmation) of a form.
 * @param string $id: quid for the user (or company agent) completing the form
 * @param string $page: path to form from drupal root
 * @param array $values: field names/values
 * @param bool $confirmed: whether the user has confirmed the values (default FALSE)
 * @return TRUE on success
 */
function completeForm($id, $page, $values, $confirmed = FALSE) {
  global $testOnly; if ($testOnly) return FALSE;
  global $testOp; $testOp = @$values[0]['op'];
  global $testConfirmation; // confirmation message output, if any
  global $formSta;
// debug(compact(u\ray('id page values confirmed testOnly'))); // keep

  $values = $values[0];
  t\fixCountryAndState($values);
  if (isset($values['amount']) and !is_numeric($values['amount'])) {
    $a = r\acct(t\fullQid($id));
    list ($values['amount']) = t\parseAmt($values['amount'], $a);
  }
  if (@$values['email']) $values = u\changeKey('email', 'mail', $values);
  
  t\logIn($id);

  $validate = TRUE;
  if (!$form = t\pageForm(t\fixPageArgs($page), FALSE)) {
    return FALSE; // probably access error
  }
  
  if ($page == 'pay' or $page == 'charge') {
    $page = 'Tx';
    if ($confirmed) $validate = FALSE;
  }

  $errorsBefore = t\getErrors();
  
  $dfts = array();
  foreach ($form as $key => $field) if (substr($key, 0, 1) != '#') {
//    u\setDft($dft, @$field['#default_value']);
    if ($field['#type'] == 'hidden') $dfts[$key] = $field['#value'];
  }
  $formSta['values'] = $formSta['input'] = $values + $dfts; // add to whatever status the form left off in
  if ($validate) foreach (t\postProcess($form, $page, 'validate') as $one) $one($form, $formSta);

  if (count($errors = t\getErrors()) > count($errorsBefore)) {
    // NO! if (!empty($errors)) return FALSE;
// debug(compact(u\ray('id page values confirmed one errors')));
  } else {
    $testConfirmation = @$formSta['confirm'];
    $lastIdentifierTag = '</div identifiers>';
    if ($i = strpos($testConfirmation, $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
    foreach (t\postProcess($form, $page, 'submit') as $one) {
      $one($form, $formSta);
    }
    if ($page == '/user/login') { // fudge this test until we can take over the login process from Drupal
      if (!$id = @$formSta['uid']) return FALSE;
      t\logIn($id);
      r\Web\loginFollowup($formSta, r\acct($id)->account());
    }
  }

  return TRUE;
}

/**
 * Simulate a visit to the page, return its form.
 * @param string $page: the Drupal URL to visit
 * @return: the form before rendering
 */
function pageForm($page) {
  global $formArray, $formSta;
  
  $login = (strpos($page, 'user/login') !== FALSE);
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);

  if (u\abbreviates('/user/', $page)) {
    $sta = array();
    list ($form, $function) = strpos($page, '/register') ? array('user_register_form', 'formRegister')
      : ($login ? array('user_login', 'formLogin') : array());
    $function = 'rCredits\\Web\\' . $function;
    // (FAILS for user_login) $form = \drupal_get_form($form, $args);
    $form = $login ? array() : \drupal_get_form($form, $args);
    $function($form, $sta, $args);
    if ($login and @$form['newPass']) $form['newPass'] = \form_process_password_confirm($form['newPass']);
    $form2 = $form; $rent = \drupal_render($form2);
    getFormOut($rent);
    return $form;
  }     

  $result = \menu_execute_active_handler($page, FALSE);
  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found' 
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\testOutput("page \"$page\": $result in t\\pageForm()");
  } else getFormOut($result);
  return $formArray;
}

function getFormOut($out) {
  global $formOut;
  $formOut = array();
  $formOut['options'] = t\dropdownOptions($out) . PHP_EOL . t\radioOptions($out);
  $formOut['steps'] = t\getSteps($out);
  $formOut['done'] = join('', t\getSteps($out, TRUE));
  $formOut['text'] = t\strip($out);
  $formOut['notags'] = strip_tags($out);
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

function radioOptions($form) {
  if (!preg_match_all('~<label class="option" [^>]*>.*? </label>~sm', $form, $matches)) return '';
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
      (abs($a - $b) < .0001)
    : ($a == $b or (empty($a) and empty($b)));
  if (!$result) echo 'not equal: ' . print_r(compact('a','b'), 1); // keep for now
  if (!$result) testOutput("unequal: a=$a b=$b", 'error'); // keep for now
  return $result;
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  if (!t\est()) return $_SERVER[$key];
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
  if (t\est()) \drupal_set_message(color($msg, $color));
  return $color == 'error' ? FALSE : TRUE;
}

/**
 * Return 't' (also meaning TRUE) if we are testing, otherwise FALSE. The 't' is used with USD_NOTE_PREFIX
 * call by t\est()
 */
function est() {return (@TESTING == 1 ? 't' : FALSE);}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch) {
  global $channel; 
  $channel = $ch;
  $cgfUid = \variable_get('cgf_uid');
  $tables = 'r_notices r_sms r_txs r_relations r_log r_invites r_companies r_gifts r_smarts r_proxies r_usd r_user_industries';
  global $sms_devel; $sms_devel = TRUE;

  r\dbQ('DELETE FROM users WHERE uid>:cgfUid OR (uid>1 AND uid<:cgfUid-1)', compact('cgfUid'));
  foreach (u\ray($tables) as $one) r\dbQ("TRUNCATE $one");
  r\dbQ('UPDATE users SET floor=0,minimum=NULL,r=0,usd=0,rewards=0,committed=0');

  $files = glob(DRUPAL_ROOT . R_PICTURE_DIR . "/*"); // get all picture file names
  foreach($files as $file){ // iterate files
    if (is_numeric($basename = basename($file)) and $basename > $cgfUid) unlink($file);
  }

  r\acct::_clear(); // empty the acct cache
  \drupal_static_reset(); // clear out drupal central storage
}

function fixPageArgs($page) {
  $special = array(
    '(by|id)=([A-Z]{3}?' . R_MARKS . '[A-Z]{3})',
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
 * Parse a partially-USD amount expression, such as "AMT+34.21"
 * @param string $var: the expression to parse -- a simple addition or subtraction to/from a named variable
 *   representing the account's initial USD balance -- or just the named variable itself.
 *   if the variable name is preceded by '+', the variable is set to the USD balance
 * @param acct $a: the account whose USD balance might be a parameter in the expression $s
 * @return: [$value, $bal], where
 *   $bal is the current actual USD balance for account $a
 *   $value is the value of the expression
 */
function parseAmt($var, $a) {
  u\EXPECT(!is_numeric($var), 'expected non-numeric var');
  
  $bal = $a->actualUsd();
//  debug(compact('a','bal','var'));

  if (($i = strpos($var, '+')) or ($i = strpos($var, '-'))) {
    $op = substr($var, $i, 1);
    list ($var, $value) = explode($op, $var);
    if ($op == '-') $value = -$value;
  } else $value = 0;
  if (substr($var, 0, 1) == '+') list ($set, $var) = array(TRUE, substr($var, 1));
  global $$var;
  if (@$set) $$var = $bal; // not defined yet, set the var to actual USD balance
//  debug(compact(u\ray('var a bal i op value')));
  return array($$var + $value, $bal);
}

function badEq($type, $v1, $v2, $id) {
  if (t\eq($v1, $v2)) return FALSE;
  t\testOutput("bad $type v1=$v1 v2=$v2 uid=$id");
  return TRUE;
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
    foreach ($fields as $key => $one) if (strpos($one, '%') !== FALSE) $fields[$key] = t\lastMinuteSubs($one);
/**/ if ($showDebugs) debug("EXPECT page with " . print_r($fields, 1)); // keep this
    foreach ($fields as $one) { // ignore the keys here
      if ($one !== '') $i = strpos($model, $one, $xi = $i);
      if ($showDebugs) t\testOutput(compact('xi','i','one'));
      if ($i === FALSE) {
        if ($showDebugs) t\testOutput(substr($model, 0, $xi) . ' <b style="font-size:200%;">[OK until here]</b> ' . substr($model, $xi));
// debug($formOut['notags']);
        return FALSE;
      } else $i += strlen($one);
    }
  }
  return TRUE;
}

function constants($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(u\just('name value', $one));
    
  }
}

function seq($i = 0) {
  global $testSeq;
  return u\n2a($testSeq[$i] = @$testSeq[$i] ? $testSeq[$i] + 1 : 1, -4);
}