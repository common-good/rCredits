<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace rCredits\Testing;
use rCredits\Testing as t;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;
use rCredits\Backend as be;
use rCredits\Admin as a;

global $base_path;
define('BASE_PATH', $base_path);
/*
define('DW_TEST_PASS', 'Password1'); // usdPass for sandbox accounts
define('DW_TEST_PIN', '9999'); // usdPin
define('DW_TEST_ACCTS', DW_TESTER_ACCT . ' ' . '8127419900 8127431130 8127423717 8127418871 8127401872'); // usdAccount
define('DW_TEST_AUTHS',
  DW_TESTER_AUTH . ' '
. 'Y6dveGxn4tFpaLCpYPy5p0Dr+fKS5yt1sQcdnMuZwd2fEFeha3 '
. 'pxj/Lr+UEFRJIXdqykS6GK8eOwFb5RJacv6PRenp1c+GCJvTW6 '
. 'ekRW2CfBdsuezuuZapK2mnWK1pqy157zoZnXmUXYoL2rBEEYWw ' // x0QfZr7LuNwt4XFPMh%2FWhSRsOyYXsNW0Ye8z0BpMZ%2FusVEVdEk ?
. 'W6J/pu0Gu3aLD7htZ+2AKBjvi7dnHYXeU0VKHu0SRFoOZB3T7G '
. 'v8bSxiLSb58y42i1lBeH7x1zxUrnygEnpe3AKFSy7HCjhTsC7V '
);
define('T_BALANCE_FIELDS', 'id r usd balance dwolla floor rewards minimum share committed');
*/
define('T_BALANCE_FIELDS', 'id r balance floor rewards savings minimum share committed');
define('T_MSG_FILE', 'test-messages.txt');

/**
 * Allow mnemonic and abbreviated data in test scenario columns.
 * @param assoc $info: field names and values
 * @param bool $direct: directly to or from db, rather than user input (default FALSE)
 */
function fixData(&$info, $direct = FALSE) {
  global $rUrl;
  $fields = 'postalCode country state email phone dob federalId idProof flags adminable notices statements';
  extract(u\just($fields, $info));

  foreach (['flags', 'adminable'] as $k) if (isset($$k)) $$k = t\fixFlags($$k);
  if (isset($adminable)) {
    for ($i = 0; $i < B_MAX; $i++) if (u\getbit($adminable, $i)) $adminable2[$i] = TRUE;
    $adminable = @$adminable2 ?: [];
  }
  if (@$notices) $notices = array_search($notices, u\ray('daily weekly monthly'), TRUE);
  if (@$statements) $statements = array_search($statements, u\ray('electronic paper'), TRUE);
  if (@$email) $info['email'] = t\fixEmail($email);
  if (@$postalCode or @$country or @$state) {
    u\setDft($country, 'US');
    if ($country == 'US' or $country == US_COUNTRY_ID) u\setDft($state, 'MA');
  }
  if (@$country) $country = db\lookup('id', 'r_countries', ':country IN (name, iso_code)', compact('country'));
  if (@$state) $state = db\lookup('id', 'r_states', ':state IN (name, abbreviation) AND country_id=:country', compact('state', 'country'));
//  if (@$mail) t\fixEmail($mail);
  if (@$email) t\fixEmail($email);
  if (strlen(@$phone) == 1) u\prefix('+1413772000', $phone);
  if (@$idProof) u\prefix("$rUrl/images/", $idProof);
  if ($direct) {
    if (@$federalId) u\digits($federalId);
    if (@$dob) $dob = strtotime(str_replace('/', '-', $dob));
  }

  foreach (u\just('uid inviter invitee person proxy helper', $info) as $k => $v) {
    if (strpos($v, '.') !== FALSE or strpos($v, ':') !== FALSE) $info[$k] = t\uid($v) ?: 0;
  }
  $info = compact(u\ray($fields)) + $info;
}

/**
 * Allow abbreviated email address like "a@" meaning a@example.com. Also avoid weird characters.
 */
function fixEmail(&$email) {
  $email = str_replace("'", '&#039;', htmlspecialchars($email)); // Drupal does this in user_register()
  if (substr(@$email, -1, 1) == '@') $email .= 'example.com'; 
  return $email;
}

function doRecords($list, $type) {
  global $testOnly;
/**/    if ($testOnly) t\output("Expect $type: " . print_r($list, 1), 'expect');
/**/  u\EXPECT(!is_numeric(key($list[0])), 'doThing should be an assoc: ' . print_r($list, 1));

  $function = 'rCredits\\Testing\\do';
  $function .= function_exists($function . $type) ? $type : 'Thing';
  foreach ($list as $one) if (!$function($one, $type)) return FALSE;
  return TRUE;
}

/*
 * Generic record-maker/record-tester.
 */
function doThing($info, $type) {
  global $testOnly;
  $table = 'r_' . strtolower($type) . 's';
  if (@$info['id']) $info['uid'] = t\uid($info['id']); unset($info['id']);
  fixData($info);
  return $testOnly ? verifyRecord($table, $info) : db\insert($table, $info);
}

function invites($list) {return doRecords($list, 'Invite');}
function gifts($list) {return doRecords($list, 'Gift');}
function notices($list) {return doRecords($list, 'Notice');}
function proxies($list) {return doRecords($list, 'Proxie');}
function members($list) {return doRecords($list, 'Account');}
function invoices($list) {return doRecords($list, 'Invoice');}
function devices($list) {return doRecords($list, 'Boxe');}
function relations($list) {return doRecords($list, 'Relation');}
function transactions($list) {return doRecords($list, 'Tx');}
function balances($list) {return doRecords($list, 'Balance');}
function membersHave($list) {return doRecords($list, 'MemberFields');}
function selling($list) {return doRecords($list, 'Selling');}
function companyFlags($list) {return doRecords($list, 'CompanyFlags');}

/**
 * Create device (box) records.
 */
function doBoxe($info = []) {
  global $testOnly, $channel; // be careful not to overwrite channel with a local variable
  //$boxName = $info['boxName'];
  $info['channel'] = @strtr(@$info['channel'], u\ray('sms pos web', TX_SMS, TX_POS, TX_WEB)) ?: $channel;
  //if ($testOnly) $boxName = htmlspecialchars(@$boxName);
  return doThing($info, 'boxe');
}

/**
 * Fabricate a bogus account.
 * Minimum parameters: none
 * @return uid
 */
function doAccount($info = []) {
  global $testOnly;
/**/  if (!is_array($info)) {print_r($info); die(print_r(debug_backtrace(), 1)); } // keep for now
  extract($info); // any user table fields
  u\setDft($id, r\acct::nextId()); // default to first available number
  $uid = t\uid($id); // change .AAB to 1, etc.

//  if (@$email) $mail = $info['mail'] = $email;
  if (@$jid) $jid = t\uid($jid);
//  if (@$helper or !@$testOnly) $helper = t\uid(@$helper ?: '.AAA'); // everyone but .AAA should have a helper
  if (!@$helper) $helper = '.AAB'; // everyone but .AAB should have a helper

  if (!$testOnly) { // when making, set many defaults and mods
    $nmMap = u\ray('ZZA:Abe One,ZZB:Bea Two,ZZC:Cat Pub,ZZD:Dee Four,ZZE:Eve Five,ZZF:Fin Corp,ZZG:Glo Seven');
    u\setDft($fullName, substr($id, 0, 2) == 'ZZ' ? strtr($nmMap) : 'doAccount ' . randomString(20, 'word'));
    $name = u\shortName($fullName);
    u\setDft($legalName, $fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
    if (trim(@$pass) === '') $pass = '123';

    $zza = r\unQuid('.ZZA');
    $mailDft = ($zza <= $uid and $uid < $zza + 26) ? chr(ord('a') + $uid - $zza) : \user_password(10);
    u\setDft($email, $mailDft . '@example.com'); // a@example.com, etc.

    u\setDft($community, r\serverUid());
    u\setDft($postalCode, '01301');
    u\setDft($country, 'US');
    $state = r\realState(@$state, $country);
    u\setDft($postalAddr, @$address . ', ' . @$city . ", $state $postalCode");
    u\setDft($photoId, "$state-{$postalCode}999");

    $moneyFields = 'share committed floor r rewards'; // not minimum because NULL means not set, in Membership page
    foreach (u\ray($moneyFields) as $one) u\setDft($$one, 0);
    if (@$cc) $cardCode = $cc;
    if (@$cc2) $cardCode2 = $cc2;

    $fixFields = 'rebate fullName legalName name pass email secure community postalCode country postalAddr cardCode cardCode2 photoId ' . $moneyFields;
  } else $fixFields = '';

  if (isset($flags) and isset($acctType) and $acctType !== R_PERSONAL and $acctType != 'personal') $flags .= ',co';

  if (isset($risks)) $risks = getRisks($risks, K_ACCT_RISKS);
  
  foreach (u\ray('id acctType dw cc cc2') as $one) unset($info[$one]); // these are not real fields
  u\preray(compact(u\ray('uid jid helper flags risks ' . $fixFields)), $info); // add in changed fields
  fixData($info, TRUE);

  if ($testOnly) {
    if (!$acct = r\acct($uid)) return output("No such uid: $uid");
    $acct->reread(TRUE); // this is required, for example, for cron's Trust test
    foreach ($info as $key => $want) {
      if ($key == 'uid') $key = 'id';
      $actual = ($key == 'trust' or $key == 'risk') ? round($acct->$key, 2) : $acct->$key;
      if (!eq($actual, $want)) {
        if ($key == 'pass' and (!isset($pass) or r\Web\passwordOkay($acct, @$pass))) continue;
        return output("uid $uid -- Wrong value for $key. Expected:$want Got:$actual");
      }
    }
    return TRUE;
  }
  if (!$a = new r\acct($info)) return FALSE;

  if (@$picture) if (!makePicture($picture, $a)) return FALSE;

  return $uid;
}

/**
 * Interpret flag names
 */
function fixFlags($flags) {
  global $testOnly;
  
  $res = 0;
  if (@$flags) foreach (u\ray($flags) as $one) $res |= u\bit(u\consta('b', $one));
  if (!$testOnly) if ($res & u\bit(B_OK)) $res |= u\bit(B_MEMBER);
  return $res;
}    

/**
 * Fabricate a bogus invoice.
 * Minimum parameters: amount, from, to
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the record ID of the created invoices (treated as TRUE in test steps)
 */
function doInvoice($info, $type) {return doTx($info, $type);}

/**
 * Fabricate bogus transactions.
 * Minimum parameters: amount, from, to
 * @return:
 *   if $testOnly return TRUE if the data is as expected
 *   otherwise return the serial of the created transaction(s) (treated as TRUE in test steps)
 */
function doTx($info, $thing) {
  global $testOnly, $channel;
  
  $inv = ($thing == 'Invoice');
  extract($info);

  u\setDft($created, r\rTime());
  if (@$from) txUid($from, $payer, $fromAgent);
  if (@$to) txUid($to, $payee, $toAgent);
  
//  u\setDft($r, $amount);
//  if ($amount != $r) u\setDft($usdXid, 10000 + $xid);
  if (@$purpose) $for = $purpose; // ?: 'cash';
  if (strpos(@$for, ',')) $for = explode(',', $for); // specifying both payerFor and payeeFor
  foreach (u\ray('from to for') as $one) unset($info[$one]);
  if (isset($risks)) $risks = getRisks($risks, K_TX_RISKS);

  if ($inv) {
    u\setDft($goods, u\forCash(@$for) ? FOR_USD : FOR_GOODS);
  } else {
    if (!$testOnly) u\setDft($risk, NULL); // indicates that risk has not been calculated yet
    if (@$xid and !is_numeric($xid)) $xid = u\a2n(substr(strpbrk($xid, R_MEMBER_MARK . R_AGENT_MARK), 1));
    u\setDft($type, TX_TRANSFER);
    foreach (u\ray('type') as $one) if (!is_numeric($$one)) $$one = u\consta('tx', $$one);
    if (!$testOnly) {
      u\setDft($goods, ($type != TX_TRANSFER or u\forCash(@$for)) ? FOR_USD : FOR_GOODS);
      if ($goods == FOR_GOODS) list ($rebate, $bonus) = array(r\rebate($payer, $payee, $amount), r\bonus($payee, $payer, $amount));
    }
    $data = [];
    if (u\in(@$purpose, 'offline')) $data['force'] = 1;
    if (u\in(@$purpose, 'undoneBy:') or u\in(@$purpose, 'undoes:')) {
      list ($k, $v) = explode(':', @$purpose);
      $data += array($k => $v);
    }
    if (@$fromAgent or @$from) $payerAgent = uid(@$fromAgent ?: ($type == TX_TRANSFER ? $from : r\communityUid()));
    if (@$toAgent or @$to) $payeeAgent = uid(@$toAgent ?: $to );
    u\setDft($taking, 0);
    unset($info['purpose']);
  }

  $fields = 'xid payer payerAgent payee payeeAgent type goods taking data risk risks';
  if (!$testOnly) $fields .= $inv ? ' created' : ' created channel';
  $info = compact(u\ray($fields)) + $info;
  
  if ($testOnly or @$type == TX_REBATE or @$type == TX_BONUS) {
    if ($inv) return verifyRecord('r_invoices', $info);
    if ($testOnly and @$for) list ($info['payerFor'], $info['payeeFor']) = is_array($for) ? $for : array($for, $for);
    unset($info['for']);
    if (!$testOnly) unset($info['risk']);
    return verifyRecord('r_txs', $info); // fails if rebate/bonus description is wrong (so omit it on create)
  }
  
  if ($inv) {
    return db\insert('r_invoices', $info);
  } else {
    $x = new r\x($info + compact('for'));
    return $x->id;
  }
}

/**
 * Interpret an id in a transaction or invoice.
 * @param string $id: quid or partial quid (eg NEW.AAA or .AAA or NEW:AAA or :AAA)
 * @param int $main: (RETURNED) the payer or payee
 * @param int $other: (RETURNED) the payerAgent or payeeAgent
 */
function txUid($id, &$main, &$other) {
  if (strpos($id, R_AGENT_MARK) !== FALSE) {
    $reid = t\uid(str_replace(R_AGENT_MARK, R_MEMBER_MARK, $id));
    extract(r\relation('main,other', $reid));
  } else $main = $other = t\uid($id);
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are quids -- at least one must be local)
 */
function doRelation($info) {
  include_once __DIR__ . '/admin/admin-forms.inc';
  global $testOnly;
  extract($info);
  
  if (@$id) $reid = t\uid(str_replace(R_AGENT_MARK, R_MEMBER_MARK, $id));
//  u\num($id, u\a2n(substr($id, -3)));
  if (!$mainAcct = r\acct(t\uid($main))) return output("main:$main is not an id");
  if (!$agtAcct = r\acct(t\uid($agent))) return output("agent:$agent is not an id");
  //u\EXPECT(!r\foreignServer(r\uidRegion($mainAcct->id)) or !r\foreignServer(r\uidRegion($agtAcct->id)));
  u\EXPECT($mainAcct->proSe and $agtAcct->proSe);
  list ($main, $other) = array($mainAcct->id, $agtAcct->id);
  $info['permission'] = @$permission ? u\consta('b', $permission) - B_RELATED : 0;
  if (@$draw) $agtAcct->setBit(B_DRAWS);
  
  foreach (u\ray('id main agent rCard') as $one) unset($info[$one]);
  $info += compact(u\ray('reid main other'));
  
  if ($testOnly) {
    return verifyRecord('r_relations', $info);
  } else {
//    db\insert('r_relations', $info);
    r\acct($main)->newRelation($other, $info['permission'], @$employee, @$isOwner, @$draw, @$reid);
    if (@$rCard == 'yes') r\Web\memberId($id); // make a Company Agent rCard
    return TRUE;
  }
}

/**
 * List what a company sells
 */
function doSelling($info) {
  global $testOnly;
  extract(u\just('id selling', $info));
  $selling = str_replace(',', "\n", $selling);
  return r\acct(t\uid($id))->update(compact('selling'));
}

/**
 * List what flags the company has set
 */
function doCompanyFlags($info) {
  global $testOnly;
  extract(u\just('id flags', $info));
  $coFlags = 0;
  if ($flags) foreach (u\ray($flags) as $one) $coFlags |= u\bit(u\consta('co', $one));
  return r\acct(t\uid($id))->update(compact('coFlags'));
}

/**
 * Return TRUE if the notice has been queued to member $id
 */
function notice($notice, $id, $subs = []) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $base_url, $thatThing;

  $subs = @$subs[0];
  if (strpos($nm = @$subs['otherName'], 'href="do/')) {
    $subs['otherName'] = str_replace('href="do/', "href=\"$base_url/do/", $nm);
  }

  $uid = uid($id);
  if (is_array($subs)) foreach ($subs as $key => $value) {
    if (preg_match('/^\$(.*)(r|us)?$/U', $value, $matches)) {
      list ($num, $currency) = array($matches[1], @$matches[2]);
      if ($currency == 'us') $currency = 'u';
      $subs[$key] = u\fmtAmt($num, 's$' . $currency);
    }
  }
  
  $map = ['&nbsp;'=>'', R_BULLET=>''];
  $message = strtr(strip_tags(u\tt($notice, @$subs)), $map);
  $sql = 'SELECT message FROM r_notices WHERE uid=:uid AND sent=:sent ORDER BY msgid DESC';
  $info = u\ray('uid sent', $uid, @$subs['sent'] ?: 0);
  $msgs = db\q($sql, $info)->fetchCol();
///  debug(compact(u\ray('message sql info msgs')));
  foreach ($msgs as $msg) {
    if (strtr(strip_tags($msg), $map) == $message) {
      $thatThing['notice'] = $msg; // remember for "And that <thing> blah blah blah"
      return TRUE;
    }
    t\output("GOT notice: $msg");
  }

/**/  t\output('EXPECTED (but did not get) notice ' . print_r($info + compact('message'), 1));
  return FALSE;  
}

/**
 * Pretend the account has complete the given steps.
 */ 
function doneStep($uid, $step) {
  global $testOnly; if ($testOnly) return FALSE;
  if (strpos($step, ' ')) {
    foreach (u\ray($step) as $one) if (!doneStep($uid, $one)) return FALSE;
    return TRUE;
  }
  
  $a = r\acct($uid = t\uid($uid));
  @$a->stepDone($step); // might have to do the steps below also
  return TRUE;
  
  if ($step == 'photo') return t\makePicture('whatever', $a);
  if ($step == 'sign') return $a->update('signed', r\rTime());
  if ($step == 'donate') return db\insert('r_gifts', u\ray('uid amount', $uid, 123));
  if ($step == 'proxies') return db\q('INSERT INTO r_proxies (person, proxy) VALUES (:uid, 2),(:uid, 3)', compact('uid'));
  
  if ($step == 'contact') return $a->update('postalAddr', 'The World');
  if ($step == 'connect') return $a->update('bankAccount', 'USkk123456789whatever');
  if ($step == 'prefs') return $a->update('minimum', 0);
  //if ($step == 'invite') return db\insert('r_invites', u\ray('inviter email code', $uid, 'any@example.com', 'whatever'));
  if ($step == 'company') return $a->update('selling', 'stuff');
  // if ($step == 'relations') ?
  return FALSE;
}

/**
 * Make the member fields be thus and such OR test to see if they ARE thus and such.
 * "?" in a field means ignore its value when testing
 */
function doMemberFields($one) {
  global $testOnly;
  $balFields = u\just(T_BALANCE_FIELDS, $one);
  if (!doBalance($balFields, $testOnly)) return;

  $a = r\acct($id = t\uid($one['id']));
  foreach ($balFields as $k => $v) unset($one[$k]); // don't rehandle balance fields, and already got id
  fixData($one, TRUE);
  if (@$one['jid']) $one['jid'] = t\uid($one['jid']);

  if ($testOnly) {
    foreach ($one as $key => $value) if ($value != '?') {
      if (t\badEq("db $key", $a->$key, $value, $id)) return FALSE;
    }
    return TRUE;
  } else return ($one ? $a->update($one) : TRUE); // handle no fields other than balFields
}

/**
 * Check or make member field assignments.
 */
function memberFieldValues($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(u\just('id field value', $one));
    $a = r\acct(t\uid($id));
    if ($field == 'helper' or $field == 'community') $value = t\uid($value);
    if ($testOnly) {
      if (t\badEq('fieldValue wanted/actual:', $value, $a->$field, $id)) return FALSE;
    } else $a->update($field, $value);
  }
  return TRUE;
}

/**
 * Make the cached balances and other numeric fields be thus and such.
 * "?" in a field means ignore its value when testing
 */
function doBalance($one) {
  global $testOnly;

  extract(u\just(T_BALANCE_FIELDS, $one));
  if (!$a = r\acct($id = t\uid($id0 = $id))) return t\output("account $id0 does not exist");
  $a->reread(TRUE); // otherwise db updates are invisible
/// print_r(compact('a','id','id0'));
/// print_r(' unQuid=' . r\unQuid($id0));
  if (!$testOnly) { // setting the values
    unset($one['id']);
    return ($one ? $a->update($one) : TRUE); // handle only id being passed
  }

  // verify
  $calc = t\uidCredit($id);
  foreach (u\ray('r balance rewards') as $k) {
    if (isset($$k) and $$k != '?') {
//      $cachedVal = $k == 'balance' ? $a->r : $a->$k;
      $cachedVal = $a->$k;
      if (($id > 0) and t\badEq("cached $k", $cachedVal, $$k, $id)) return FALSE;
      if ($id > 0 or $k != 'rewards') if (!$a->jid) if (t\badEq("calc $k", $calc->$k, $$k, $id)) return FALSE;
    }
  }
  foreach (u\ray('floor minimum share committed') as $k) {
    if (isset($$k) and $$k != '?' and $id > 0) if (t\badEq("db $k", $a->$k, $$k, $id)) return FALSE;
  }
  return TRUE;
}

/**
 * Return a field for the account with the given cell number.
 */
function phoneField($field, $number) {
  return @r\acct(r\SMS\uid($number))->$field;
}

function usdTransferCount($where = '1') {
  global $testOnly;
  
  if($testOnly) {
    return db\count('r_usd', $where);
  } else {
    db\q("DELETE FROM r_usd WHERE $where");
    return 0; // 0 is the only thing we can make it!
  }
}

function usdTransfers($txs) {
  global $testOnly, $channel; $channel0 = $channel;
  
  foreach ($txs as $tx) { // do not extract $tx, because setDft will fail after the first unless we unset vars
    if (@$tx['payer']) $tx['payer'] = $payer = t\uid($tx['payer']);

    if ($testOnly) {
      if (isset($tx['risks'])) $tx['risks'] = getRisks($tx['risks'], K_TX_RISKS);
      if (!verifyRecord('r_usd', $tx)) return FALSE;
    } else {
      $a = r\acct($tx['payer']);
      $tx['bankAccount'] = @$a->secure['bankAccount'];
      u\setDft($tx['txid'], t\seq());
      u\setDft($tx['risk'], NULL);
      u\setDft($tx['created'], @$tx['completed']);
      if (!db\insert('r_usd', $tx)) return FALSE;
      if (@$tx['completed']) $a->update('r', $a->r - $tx['amount']);
    }
  }
  return TRUE;
}

/**
 * Run the op in the background, return TRUE.
   include_once __DIR__ . '/../rcredits/rcredits-testing.inc';
   f('t.cronRuns', 'op');
 */
function cronRuns($op) {
  global $testOnly; if(@$testOnly) return FALSE;
  global $cronOp; $cronOp = $op;
  $func = "rCredits\\Cron\\$op";

  $now = r\rTime();
  \variable_set('r_last_cron', u\ray('day week month', $now, $now, $now)); // normally don't run periodics

  if ($op == 'ALL' or strpos(R_DAILY_TASKS, " $op ")) {
/**/  if (strftime('%H') < R_DAILY_HOUR) die('Developer: For testing between midnight and :R_DAILY_HOUR:00, you need to temporarily change R_DAILY_HOUR in -settings.');
    db\q("DELETE FROM queue WHERE name='q'");
    \variable_set('r_last_cron', u\ray('day week month', $now - DAY_SECS, 0, 0));
    \drupal_cron_run();
  } elseif ($op == '') {
    \drupal_cron_run();
  } elseif ($op == 'everyWeek') {
    $func(); // queue up weekly tasks
    \drupal_cron_run();
  } else $func();

  return TRUE;
}

/**
 * Return the credit info for the given uid.
 * To get just the r balance, use uidCredit(uid)->r
 * @param int $uid: return credit info for this account id
 */
function uidCredit($uid) {
  $info = be\creditInfo(u\ray('uid noj', $uid, TRUE));
  $a = r\acct($uid);
  if ($uid > 0 and !$a->jid) foreach (u\ray('r rewards') as $k) { // (community amounts don't get cached)
    if (!t\eq($v1 = $info->$k, $v2 = $a->$k + 0)) {
      output("Calculated $k different from cache: $v1 <> $v2");
      $info->$k = 'error';
    }
  }
  return $info;
}

/**
 * Interpret the given identifier (int, full quid, or local quid) as an account ID.
 */
function uid($id) {
  return is_numeric($id) ? $id
    : (($id == 'community' or $id == 'ctty') ? r\serverUid() 
    : ($id == 'cgf' ? r\cgfId()
    : r\unQuid($id)));
}

function getErrors() {
  $errors = \drupal_get_messages('error', FALSE);
  $errors = @$errors['error'] ?: [];
  foreach ($errors as $key => $value) {
    if (u\abbreviates('Unable to send e-mail', $value) or strpos(strip_tags($value), 'expected to be a reference, value given in drupal_retrieve_form()')) unset($errors[$key]); // ignore these when testing
  }
  return $errors;
}

/**
 * Find the message in the log table.
 * @param assoc $subs: subs for message iff message index is passed.
 */
function findMessage($type, $fields, $message = '', $subject = '', $subs = []) {
  global $channel;

  $mya = r\acct();
  $fields = @$fields[0] ?: $fields;
  $subs = @$subs[0] ?: @$subs;
  $agent = $mya ? $mya->agentId : 0;
  $myid = $mya ? $mya->id : 0;


  if ($message = ($message ?: @$fields['message'])) {
    if ($channel != TX_SMS and strlen($message < 30)) $message = u\tt($message, $subs);
    $fields['message'] = $message;
  } else unset($fields['message']); // empty message in features means NO message
//  output(array('EXPECT' => '') + $fields, @$subject ? 'email' : 'output'); // keep this
  output(array('EXPECT' => '') + $fields, 'output'); // keep this
  $info = u\jsonEncode($fields);
  $crit = "channel=:channel AND type=:type AND myid<=>:myid AND agent<=>:agent ORDER BY logid DESC";
/// debug("info=:info AND $crit"); debug(compact(u\ray('info type myid agent channel')));
  return db\exists('r_log', "info=:info AND $crit", compact(u\ray('info type myid agent channel')));
}

function weTellStaff($topic, $subs = []) {
  global $testOnly; if (!$testOnly) return FALSE;

  if (!@$subs[0]) $subs[0] = [];
  $regionEmail = r\regionfield('email');
  return t\findEmail('tell-staff', $regionEmail, $subs[0] + u\ray('topic noFrame message', $topic, TRUE, '?'));
/*  
  foreach ($subs as $one) {
    $index = 'tell-staff';
    $subject = str_replace('{topic}', $topic, $GLOBALS['emailSubjects'][$index]);
    
    $find = '%"index":"@index","email":"@regionEmail","subject":"@subject"%';
    if ($id = @$one['id']) {
      $id = t\uid($id);
      $find .= "(#$id)%";
      unset($one['id']);
    }
    $find = u\tt($find, compact(u\ray('index subject regionEmail')));
    foreach ($one as $k => $value) $find .= "$value%";
    if (!$last = db\lookup('info', 'r_log', "info LIKE '$find' ORDER BY logid DESC")) {
      if (count($subs) > 1) output("tell-staff can't find $find");
      return FALSE;
    }
  }
  return TRUE;
  */
}

/**
 * Say whether any such email has been sent to the specified address.
 */
function noSuchEmail($index, $email) {
  global $testOnly; if (!$testOnly) return FALSE;
//  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
  $pattern = strtr('%"index":"INDEX","email":"EMAIL"%', u\ray('INDEX EMAIL', $index, $email));
  return !db\exists('r_log', 'info LIKE :pattern', compact('pattern'));
}

/** 
 * See whether the specified email has been sent (by searching the log file)
 */
function findEmail($index, $email, $subs) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testEmails; if (!@$testEmails) return FALSE;
///  debug(u\jsonEncode($testEmails));

  $subs = is_array(@$subs[0]) ? $subs[0] : @$subs;
  t\fixEmail($email);
  if ($a = r\acct($email)) $subs['quid'] = $a->mainQid;
  
  foreach ($subs as $k => $v) {
    if ($k == 'nudge') $subs[$k] = $v = u\tt($v);
    $esubs['{' . $k . '}'] = $v;
  }
  $message = @$subs['noFrame'] ? r\emailTemplate($index) : r\emailBody($index);
  $message = str_replace("\r\n", '', $message);
  $messageDpy = t\showSubs($message, $esubs);
  $esubs['{companies}'] = ''; // don't test this
  $message = strtr($message, $esubs);
  $subject = @$info['subject'] ?: $GLOBALS['emailSubjects'][$index];
  $subject = $GLOBALS['emailSubjects'][$index];
  $subjectDpy = t\showSubs($subject, $esubs);
  $subject = strtr($subject, $esubs);

  $pattern = '/\{[A-Z]+[A-Z0-9]*\}/i';
  if (preg_match($pattern, $message . $subject)) {
    t\output("message or subject contains unhandled subs in email \"$index\"");
  } else {
    $map = ["\n"=>'', R_BULLET=>'*', '&nbsp;'=>' '];
    
    foreach ($subs as $k => $v) {
      if ($v == '?') {
        $ignore[] = $k;
        unset($subs[$k]);
      } else $subs[$k] = strtr(strip_tags($v), $map);
    }
    ksort($subs);
    
    foreach ($testEmails as $one) {
      extract($one, EXTR_PREFIX_ALL, 't'); // t_ means actual values
      if ($t_index == $index and $t_toEmail == $email) {
        foreach ($t_subs as $k => $v) if (!is_array($v)) $t_subs[$k] = strtr(strip_tags($v), $map);
        if (@$ignore) foreach ($ignore as $k) unset($t_subs[$k]);
        ksort($t_subs);
        $wantedInActual = u\just(array_keys($subs), $t_subs);
        $actualInWanted = u\just(array_keys($t_subs), $subs);
        if ($wantedInActual == $actualInWanted) return TRUE; // actual is exactly what was wanted
        t\output(compact('wantedInActual', 'actualInWanted'), 'email');
        foreach ($subs as $k=>$v) if (isset($t_subs[$k]) and $v != $t_subs[$k]) t\output("k=$k v=$v tv=" . @$t_subs[$k], 'email');
        foreach ($t_subs as $k=>$v) if (isset($subs[$k]) and $v != $subs[$k]) t\output("k=$k tv=$v v=" . @$subs[$k], 'email');
      }
    }
  }
  
  output(['EXPECTED (but did not get)'=>''] + compact(u\ray('index email subjectDpy subs messageDpy')), 'email');
  return FALSE;
}

/**
 * Highlight the substitutions in a given text.
 * @param string $s: the text
 * @param assoc $subs: the substitutions
 * @return the text with substitutions highlighted
 */
function showSubs($s, $subs) {
  foreach ($subs as $k=>$v) $subs[$k] = "<span style=\"background-color:yellow;\">$v</span>";
  return strtr($s, $subs);
}

function memberVisitsPage($id, $page) { // pass options in URL
  global $testOnly; if ($testOnly) return FALSE;
  if (!t\login($id)) return FALSE;
  t\pageForm(t\fixPageArgs($page), FALSE);
  return TRUE;
}

function weShowWith($title, $content = FALSE, $showDebugs = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($title and !weShow($title, $showDebugs)) return FALSE;
  return weShow($content, $showDebugs);
}

function weShowWithSubs($page, $subs) {
  global $testOnly;
  global $testConfirmation; // confirmation message output, if any
  global $formOut; // for testing

  $subs = @$subs[0] ?: []; // comes in as "" if no subs
  if (strpos($page, 'confirm ') !== FALSE) {
    foreach (u\just(R_DATE_FIELDS, $subs) as $k => $v) if (is_numeric($v)) $subs[$k] = u\fmtDate($v);
    $wanted = u\tt($page, $subs);
    t\output("<b>WANTED: </b>$wanted", 'screen');
    $res = ($wanted == $testConfirmation);
/**/ if (!$res) debug(compact('wanted', 'subs', 'testConfirmation'));
    return $res;
  }
  $form = strtr(t\strip(r\Web\showForm($page, @$subs['arg1'], @$subs['arg2'])), $subs);
  t\output("<b>WANTED: </b>$form", 'screen');
  if ($form != $formOut['text']) return FALSE;
  unset($subs['arg1']);
  unset($subs['arg2']);
  foreach ($subs as $one) if (strpos($form, $one) === FALSE) return FALSE; // sometimes we dunno what to sub for
  return TRUE;
}

function makePicture($photo, $a) {
//  $filename = DRUPAL_ROOT . $a->photoFilename('picture2', TRUE);
//  return (bool) file_put_contents($filename, $picture);
  return $a->update(compact('photo'));
}

// UNUSED
function getPicture($picture) {
  return file_get_contents(DRUPAL_ROOT . R_PICTURE_DIR . "small/$picture.jpg");
}

function lastMinuteSubs($string) {
  if (u\abbreviates('%ymd', $string)) return subAgo($string, '%Y-%m-%d');
  if (u\abbreviates('%dmy', $string)) return subAgo($string, '%d-%b-%Y');
  if (u\abbreviates('%dm', $string)) return subAgo($string, '%d-%b');
  if (u\abbreviates('%md', $string)) return subAgo($string, '%b %d');
  if (u\abbreviates('%can_', $string)) {
    $perm = u\consta('b', substr($string, 5));
    return htmlspecialchars($GLOBALS['share permissions'][$perm - B_RELATED]);
  }
  if ($string == '%chk') return '&#10004;';
  if ($string == '%ctty') return r\regionField('fullName');
  return $string;
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
///  debug(compact(u\ray('string fmt lastC a b bPreC period')));
  return (strftime($fmt, $period == 'months' ? u\plusMonths(-$bPreC) : strtotime("$bPreC $period ago")));
}

/**
 * Send an RPC API request to the server, for rPOS.
 */
function hitServer($op, $extra = [], $agent, $device = '', $version = '999') {
  global $rposResult;
  $agent = r\fullQid($agent);
  if (@$extra['member']) $extra['member'] = r\fullQid($extra['member']);
  $input = compact(u\ray('op agent device version')) + $extra;
/**/ t\output('request: ' . print_r($input, 1), 'input'); // keep this
  $rposResult = r\Smart\pos($input);
  if ($op != 'photo') $rposResult = (array) json_decode($rposResult);
/**/ t\output('answer: ' . print_r($rposResult, 1), 'output');
  return TRUE;
}

function parseQr($qr) {
  list ($member, $m, $code) = preg_split('/([\.-])/', $qr, 0, PREG_SPLIT_DELIM_CAPTURE);
  $member = R_SERVER_ID . ($m == '-' ? ':' : $m) . $member;
  return array($member, $code);
}

function snapShot(&$form) { // (additional sta values have no effect at this stage)
  addAttribs($form);
  return $form; // the rest is redundant
  //global $formOut['text']; $zot = $form; $formOut['text'] = strip(\render($zot)); // for testing
  //output('ACTUAL or EXPECTED: ' . $formOut['text'], 'screen');
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
  global $boxUser;
  if ($id != '?') {
    r\acct::setDefault($a = r\acct(r\fullQid($id))); // fullQid not uid
    $boxUser = r\Web\boxUser(r\Web\box(), $a->id);
  }
  return TRUE;
}

/**
 * Compare the data to a corresponding record in the table.
 * Return TRUE if the record matches the data.
 * Ignore fields with value "?".
 * Example call:
 *   $info = doRelation($info, TRUE);
 *   return verifyRecord('r_relations', $info);
 */
function verifyRecord($table, $info) {
///  debug(compact('table','info'));
/**/  if (!is_array($info)) die('in verifyRecord: ' . print_r(debug_backtrace(), 1)); // keep
  
  unset($info['data']);
  foreach ($info as $key => $value) {
    if ($value == '?') {unset($info[$key]); continue;}
    $crit[] = $value === '' ? "($key IS NULL or $key=:$key)"
    : ((in_array($key, u\ray(R_DATE_FIELDS)) or stripos($key, 'date') !== FALSE) ? 
      "DATE(FROM_UNIXTIME($key))<=>DATE(FROM_UNIXTIME(:$key))" 
    : (in_array($key, u\ray('trust risk')) ? "CAST(ROUND($key, 2) AS CHAR)<=>:$key"
    : "$key<=>:$key")); // null-safe ==
  }

  $crit = join(' AND ', $crit);
  if (!$result = db\exists($table, $crit, $info)) {
    foreach ($info as $k => $value) $subs[$k] = "'$value'";
    $sql = "SELECT * FROM $table WHERE " . strtr(u\SUBS($crit), u\prefixKeys(':', $subs));
/**/ output("verifyRecord failed. sql=$sql<br>table=$table crit=$crit EXPECTED info=" . print_r($info, 1));
  }
  return $result;
}

/**
 * Simulate completion (and possible confirmation) of a form.
 * @param string $id: quid for the user (or company agent) completing the form
 * @param string $page: path to form from drupal root
 * @param array $values: array of assoc of field names/values (or empty)
 * @param bool $confirmed: whether the user has confirmed the values (default FALSE)
 * @return TRUE on success
 *
 * @todo: Rewrite this whole form testing system to use just menu_execute_active_handler,
 *   drupal_validate_form, etc. (assuring official behavior)
 */
function completeForm($id, $page, $values = '', $confirmed = FALSE) {
  global $testOnly; if ($testOnly) return FALSE;
  global $fieldErr;
  global $testOp; $testOp = @$values[0]['op'];
  global $testConfirmation; // confirmation message output, if any
  global $formSta, $lastGo;
  $xlastGo = @$lastGo; // notice whether a goto happens, so we can skip post-processing
///  debug(compact(u\ray('id page values confirmed testOnly')));

  $values = @$values[0] ?: [];
  t\fixData($values);
  if (isset($values['amount']) and !is_numeric($values['amount'])) {
    $a = r\acct(r\fullQid($id));
    list ($values['amount']) = t\parseAmt($values['amount'], $a);
  }
//  if (@$values['email']) $values = u\changeKey('email', 'mail', $values);
  
  t\logIn($id);

  $validate = TRUE;
  if (u\starts($page, 'history/') and $confirmed and !strpos($page, '&do=')) { // re-examine this
    $validate = FALSE;
    $formSta['confirm'] = TRUE;
  }

  t\POST($values);
  if (!$form = t\pageForm(t\fixPageArgs($page), FALSE)) return FALSE; // probably access error
  $sta = $formSta; $formSta = NULL; // don't let $formSta influence other forms that get invoked
  
  if ($page == 'pay' or $page == 'charge') {
    $page = 'Tx';
    if ($confirmed) $validate = FALSE;
  }
  
  $dfts = [];
  foreach ($form as $key => $field) if (substr($key, 0, 1) != '#') {
    if (@$field['#type'] == 'hidden') $dfts[$key] = $field['#value'];
  }
  $sta['values'] = $sta['input'] = $values + $dfts; // add to whatever status the form left off in
  $fieldErr = FALSE; // no field errors yet
  
  if ($validate and @$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'validate') as $one) $one($form, $sta);
  
  $lastIdentifierTag = '</div identifiers>';
  if ($i = strpos($testConfirmation, $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
  if ($fieldErr) return TRUE; // no submit if field error
  if (@$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'submit') as $func) $func($form, $sta);

  return TRUE;
}

/**
 * Simulate a visit to the page, return its form.
 * @param string $page: the Drupal URL to visit
 * @param string $msg: message to output, if any
 * @param string $field: error field, if any
 * @return: the form before rendering
 */
function pageForm($page, $msg = '', $field = '') {
  global $formArray, $formSta, $lastGo;

  if (!$page) $page = 'summary';
  if ($msg) r\Web\say($msg, $field);
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);
  $xlastGo = @$lastGo; // notice whether a goto happens. ignore all but the deepest (last).

/*
  if (u\abbreviates('/user/', $page)) {
//    $login = u\in($page, 'user/login');
    $sta = @$formSta ?: [];
    list ($form, $function) = FALSE ? array('user_register_form', 'formSignup')
      : ($login ? array('user_login', 'formSignin') : []);
    $function = 'rCredits\\Web\\' . $function;
    // (FAILS for user_login) $form = \drupal_get_form($form, $args);
    $form = $login ? [] : \drupal_get_form($form, $args);
    $function($form, $sta, $args);
//    if ($login and @$form['newPass']) $form['newPass'] = \form_process_password_confirm($form['newPass']);
    $form2 = $form; $rent = \drupal_render($form2);
    if (@$lastGo == $xlastGo) setFormOut($rent);
    return $form;
  }
*/
  $result = \menu_execute_active_handler($page, FALSE);

  if (is_numeric($result)) {
    $result = $result == MENU_NOT_FOUND ? 'not found' 
      : ($result == MENU_ACCESS_DENIED ? 'access error' : "error #$result");
    t\output("page \"$page\": $result in t\\pageForm()");
  } elseif (@$lastGo == $xlastGo) setFormOut($result);
  return $formArray;
}

function setFormOut($out) {
  global $formOut;

  $formOut = [];
  $formOut['options'] = t\dropdownOptions($out) . PHP_EOL . t\radioOptions($out);
  $formOut['steps'] = t\getSteps($out);
  $formOut['done'] = join('', t\getSteps($out, TRUE));
  $formOut['text'] = t\strip($out);
  $formOut['notags'] = strip_tags($out);
  t\output('ACTUAL: ' . $formOut['text'], 'screen');
}

/**
 * Return an array of steps in the output
 * @param string $out: the rendered form
 * @param bool $set: TRUE = return just the done steps, FALSE = just the not-done steps, DEFAULT = both
 * @return: array of step numbers
 */
function getSteps($out, $set = NULL) {
  if (!@$set) $set = isset($set) ? '>' : '';
  if (!preg_match_all("/done-(\d) state$set/", $out, $matches)) return [];
  return $matches[1];
}

function postProcess($form, $formName, $type) {
  $functions = @$form["#$type"];
  if (!@$functions or @($functions == '<')) {
    $function = "rCredits\\Web\\form{$formName}_$type";
    $functions = function_exists($function) ? array($function) : [];
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
          $_SESSION['messages'][$type] = array_merge((@$msgs[$type] ?: []), (@$_SESSION['messages'][$type] ?: []));
          if (empty($_SESSION['messages'][$type])) unset($_SESSION['messages'][$type]);
        }
      }
//      cache_set('t_messages', FALSE);
//u\deb('postLogout returning TRUE, skipToStep was ='.@$skipToStep.' (now NULL)');
      $skipToStep = NULL; // strangely, this has to be = NULL not unset()
      return TRUE;
    }
  }
  */
//u\deb('postLogout returning FALSE, about to save skiptostep '.$function);
  $vars = array(
    'msgs' => @$_SESSION['messages'] ?: [],
    'skipToStep' => $function, // tell gherkin function to skip steps in interrupted scene until this step
  );
//  cache_set('t_messages', $vars); // save the test results so far
  return FALSE;
}

/**
 * Assign a name to a variable that can be used in tests using the "%varable" pseudo-Gherkin syntax.
 * @param string $varName: variable name
 * @param string $index: a string or a message index
 * @param assoc $subs: substitutions (if any)
 * @return TRUE
 */
function varIs($varName, $index, $subs = []) {
  global $testOnly; if ($testOnly) return FALSE;
  global $sceneTest;
  $$varName = u\tt($index, $subs[0]);
  $sceneTest->subs += ["%$varName" => $$varName];
  return TRUE;
}

function dropdownOptions($form) {
  if (!preg_match_all('~<option [^>]*>.*?</option>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function setChecked($name) {global $tChecked; $tChecked[] = $name;}

function radioOptions($form) {
  if (!preg_match_all('~<label class="option" [^>]*>.*? </label>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function strip($form) {

  $form = str_replace('&nbsp;', ' ', $form);
  $form = preg_replace('~<fieldset id="help".*?</fieldset>~sm', '', $form); // remove help div
  $form = preg_replace('~<div[^>]*class="invisible.*?</div>~sm', '', $form); // remove invisible divs (not nested)
  $form = preg_replace('~<option value="[^"]*">.*?</option>~sm', '', $form); // remove unselected options (fails for dropdowns created on the fly, eg state and country)
  $form = preg_replace('~<input type="hidden".*?/>~sm', '', $form); // remove hidden inputs
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/sm', '$1', $form); // show button labels
  $form = preg_replace('/> *</sm', '> <', $form); // make sure there's exactly one space between things
  return preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form));
}

function eq($a, $b, $offBy = .0001) {
  $result = (is_numeric($a) and is_numeric($b)) ? 
      (abs($a - $b) < $offBy)
    : ($a == $b or (empty($a) and empty($b)));
/// if (!$result) echo 'not equal: ' . print_r(compact('a','b'), 1); // keep for now
  if (!$result) output("unequal: a=$a b=$b", 'error');
  return $result;
}

/**
 * Fake $_SERVER params for testing
 */
function SERVER($key, $value = NULL) {
  if (!u\test()) return $_SERVER[$key];
  global $T_SERVER;
  if (isset($value)) $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * Fake $_POST params for testing
 */
function POST($value = NULL) {
  if (!u\test()) return $_POST;
  global $T_POST;
  if (isset($value)) $T_POST = $value; else return $T_POST;
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 * Not to be confused with the color() function in test.php
 */
function color($msg, $color = 'normal') {
  $colors = array(
    '' => 'ivory',
    'normal' => 'ivory',
    'expect' => 'cyan',
    'email' => 'lightblue',
    'input' => 'orange',
    'output' => 'thistle',
    'screen' => 'burlywood',
    'error' => 'salmon',
  );
  $color = @$colors[$color] ?: $color;
/**/ $msg = print_r($msg, 1); // keep this
  return "<pre style='background-color:$color;'>$msg</pre>";
}

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function output($msg, $color = 'error') {
  global $testModule;
  if (u\test() and !@$testModule) \drupal_set_message(color($msg, $color));
  return $color == 'error' ? FALSE : TRUE;
}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch = 0) {
  include_once __DIR__ . '/admin/admin.inc';
  global $lastGo, $formOut, $formArray, $formSta, $testOp, $testConfirmation, $tChecked;
  global $sms_devel; $sms_devel = TRUE;
    
  $lastGo = $formOut = $formArray = $formSta = $testOp = $testConfirmation = $tChecked = NULL;
  global $sms_devel; $sms_devel = TRUE;
  global $testSeq; $testSeq = NULL;
  global $channel; $channel = $ch;
  global $testMessages; $testMessages = [];

  $cgfUid = r\cgfId();
  $tables = 'r_boxes r_do r_gifts r_invites r_invoices r_log r_notices r_proxies r_relations r_stats r_txs r_usd r_user_industries queue';

  db\q('DELETE FROM users WHERE uid>1 or uid<0');
  a\setupBasicAccounts();

  foreach (u\ray($tables) as $one) db\q("TRUNCATE $one");
  db\q('UPDATE users SET floor=0,minimum=NULL,r=0,rewards=0,committed=0');

  ctty('aab')->setBit(B_OK);
  ctty('aab')->setBit(B_UP);

  t\log('clearing');
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
    if ($word == 'flow' and !is_numeric($what)) $what = u\consta('flow', $what);
    $page = preg_replace($pat, "$start$word=$what$end", $page);
  }
  return $page;
}

/**
 * Interpret the risk bits.
 * @param string $risks0: comma-separated list of names of risk bits to set
 * @param string $descs: comma-separated list of all possible risk bit names, with weights
 * @return an integer bit array of the desired risks
 */
function getRisks($risks0, $descs) {
  if (!$risks0) return 0;
  $bits = array_flip(array_keys(u\ray($descs)));
  $risks = 0;
  foreach (u\ray($risks0) as $bit) $risks |= u\bit($bits[$bit]);
  return $risks;
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
 *//*
function parseAmt($var, $a) {
  u\EXPECT(!is_numeric($var), 'expected non-numeric var');
  
  $bal = $a->actualUsd();
///  debug(compact('a','bal','var'));

  if (($i = strpos($var, '+')) or ($i = strpos($var, '-'))) {
    $op = substr($var, $i, 1);
    list ($var, $value) = explode($op, $var);
    if ($op == '-') $value = -$value;
  } else $value = 0;
  if (substr($var, 0, 1) == '+') list ($set, $var) = array(TRUE, substr($var, 1));
  global $$var;
  if (@$set) $$var = $bal; // not defined yet, set the var to actual USD balance
///  debug(compact(u\ray('var a bal i op value')));
  return array($$var + $value, $bal);
}
*/

function badEq($type, $v1, $v2, $id) {
  if (t\eq($v1, $v2)) return FALSE;
  t\output("bad $type v1=$v1, expected v2=$v2 uid=$id");
  return TRUE;
}

/**
 * Say whether the most recent page included the given text or options.
 * @param array $content: array of assocs -- named values to check for
 *   The names (labels) will be sought once also, unless preceded by underscore (_).
 *   The array can go either way, horizontally or vertically
 * @param string $type: description of what to check for (default 'text')
 * @param bool $showDebugs: whether to report debugging info while looking
 * @return TRUE if all the content is included
 */
function shownWith($content, $type = 'text', $showDebugs = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $formOut;
  $model = $formOut[$type];
///  debug(compact('content','type','model','formOut'));

  if (!$content) return FALSE; // probably an accidental single-row array with no labels
/**/  u\EXPECT(is_numeric(key($content[0])), 'shown with should be array, not assoc: ' . print_r($content, 1));
  $A = call_user_func_array('array_merge', $content);
  $xi = $i = 0;
  foreach ($A as $one) {
    if ($one === '' or substr($one, 0, 1) == '_') continue; // ignore _labels
    if (u\in($one, '%')) $one = t\lastMinuteSubs(htmlspecialchars($one));
    if ($showDebugs) t\output("EXPECT page with " . u\purify($one, ''));
    if (($one = u\redash($one)) !== '') $i = strpos($model, $one, $xi = $i);
    if ($showDebugs) t\output(compact('xi','i','one'));
    if ($i === FALSE) {
      if ($showDebugs) t\output(substr($model, 0, $xi) . ' <b style="font-size:200%;">[OK until here]</b> ' . substr($model, $xi));
      return FALSE;
    } else $i += strlen($one);
  }
  return TRUE;
}

function constants($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(u\just('name value', $one));
    
  }
}

/**
 * Say whether the member has shown a photo ID or (if not just testing) make it so.
 * @param string $id: abbreviated member ID
 */
function noPhotoId($id) {
  global $testOnly;
  $a = r\acct(t\uid($id));
  return $testOnly ? !$a->photoId : $a->update('photoId', '');
}

function weShow($what, $show = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($what === '') return TRUE;
  return t\shownWith(is_array($what) ? $what : array(array($what)), 'text', $show);
}

function weMessage($index, $id, $subs) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testMessages;
  
  $subs = $subs[0];
  $uid = t\uid($id);
  
  foreach ($testMessages as $one) {
    extract($one, EXTR_PREFIX_ALL, 't');
    if ($t_uid == $uid and $t_index == $index) {
      $matching = true;
      foreach ($subs as $k => $v) if ($v != '?' and @$t_subs[$k] != $v) {
        $matching = FALSE; break;
      }
      if ($matching and !preg_match('/[@\{]/', @$t_topic)) return TRUE;
    }
  }
/**/ return t\output('Expected: ' . print_r(compact(u\ray('index id uid subs')), 1) . '<br>Got: ' . print_r($testMessages, 1));;
}
    
/**
 * Check the linked URL content and any Drupal messages (which don't show up in file_get_contents) against expectations.
 */
function hasLinkResults($thing, $info) {
  global $formOut;
  global $testOnly; if (!$testOnly) return FALSE;

  if (!preg_match('/ href="([^"]*)"/', t\that($thing), $matches)) return FALSE;
  
  $s = file_get_contents($matches[1]);
  foreach (u\ray('error status') as $one) $$one = @file_get_contents(t\dosayFilename($one));
  $formOut['text'] = str_replace($was = '#header -->', "$was$error$status", $s);
  return t\shownWith($info, 'text', TRUE);
}

function seq($table = 'r_usd') {
  global $testSeq;
  $id = $testSeq[$table] = @$testSeq[$table] + 1;
  $k = $table == 'r_usd' ? 'txid' : '';
  t\log("seq table=$table id=$id");
  u\EXPECT(!db\exists('r_usd', "$k=:id", compact('id')), "duplicate seq: $id in table $table");
///  print_r(compact('table','testSeq','id','k'));
  return $id;
}

function log($msg) {if (u\test()) u\log($msg, 'debug');}
function that($thing) {global $thatThing; return $thatThing[$thing];}
function dosayFilename($type) {return __DIR__ . "/../dosay-$type.txt";}
function goodsBit($s) {return $s == 'goods' ? "1" : ($s == 'cash' ? "0" : "ERROR");} // no 3rd why for rSmart
function approxTime($a, $b) {return t\eq($a, $b, 10000);}
