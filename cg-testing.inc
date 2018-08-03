<?php
/**
 * @file
 * Subroutines for testing steps, for all interfaces.
 */

namespace CG\Testing;
use CG\Testing as t;
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Admin as a;
use CG\Cron as cr;
use CG\Web as w;

global $base_path;
define('BASE_PATH', $base_path);
define('T_BALANCE_FIELDS', 'id balance floor rewards savingsAdd saveWeekly minimum share committed');
define('T_ID_FIELDS', 'id uid jid inviter invitee person proxy helper from to partner myid reid main agent other ctty fromId');

/**
 * Allow mnemonic and abbreviated data in test scenario columns.
 * @param assoc $info: field names and values
 * @param bool $direct: directly to or from db, rather than user input (default FALSE)
 */
function fixData(&$info, $direct = FALSE) {
  global $rUrl, $testVars;
  $fields = 'zip country state email phone dob federalId idProof flags adminable notices statements who rewards risks toEmail';
  if (@$testVars) foreach ($info as $k => $v) if (strpos($v, '@') !== FALSE) {
    foreach ($testVars as $varName => $var) $info[$k] = str_replace("@$varName", $var, $v);
  }
  extract(just($fields, $info));

  if (@$who) $who = t\fullQid($who);
  foreach (['flags', 'adminable'] as $k) if (isset($$k)) $$k = t\fixFlags($$k);
  if (isset($adminable)) {
    for ($i = 0; $i < B_MAX; $i++) if (u\getBit($adminable, $i)) $adminable2[$i] = TRUE;
    $adminable = @$adminable2 ?: [];
  }
  foreach (['rewards'] as $k) if (isset($$k)) $$k += 0; // make sure numeric fields are numeric.
  if (@$notices) $notices = array_search($notices, ray('daily weekly monthly'), TRUE);
  if (@$statements) $statements = array_search($statements, ray('electronic paper'), TRUE);
  if (@$email) $info['email'] = t\fixEmail($email);
  if (@$zip or @$country or @$state) {
    u\setDft($country, 'US');
    if ($country == 'US' or $country == US_COUNTRY_ID) u\setDft($state, 'MA');
  }
  if (@$country) $country = db\get('id', 'r_countries', ':country IN (name, iso_code)', compact('country'));
  if (@$state) $state = db\get('id', 'r_states', ':state IN (name, abbreviation) AND country_id=:country', compact('state', 'country'));
//  if (@$mail) t\fixEmail($mail);
  if (@$email) t\fixEmail($email);
  if (strlen(@$phone) == 1) u\prefix('+1413772000', $phone);
  if (@$idProof) u\prefix("$rUrl/images/", $idProof);
  if ($direct) {
    if (@$federalId) u\digits($federalId);
    if (@$dob) $dob = strtotime(str_replace('/', '-', $dob));
  }

  foreach (just(T_ID_FIELDS, $info) as $k => $v) {
    if (preg_match('/^(\.|[A-Z]{3})[A-Z]{3}$/', $v) or $v == 'ctty') $info[$k] = t\uid($v) ?: 0;
  }
  if (isset($risks)) $risks = getRisks($risks, K_ACCT_RISKS);
  
  $info = compact(ray($fields)) + $info;
}

/**
 * Allow abbreviated email address like "a@" meaning a@example.com. Also avoid weird characters.
 */
function fixEmail(&$email) {
  if (substr(@$email, -1, 1) == '@') $email .= EMAILX; 
  return plain($email);
}

function doRecords($list, $type) {
  global $testOnly;
//  $list = array_values((array) json_decode($list));
//  foreach ($list as $k => $v) if (is_object($v)) $list[$k] = (array) $v;
/**/    if ($testOnly) t\output("Expect $type: " . print_r($list, 1), 'expect');
  if (is_object($list)) $list = (array) $list;
/**/  u\EXPECT(is_array(@$list) and !is_numeric(@key(@$list[0])), 'doThing object should be an assoc: ' . print_r($list, 1));
  $function = 'CG\\Testing\\do';
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
  fixData($info);
  if (@$info['id']) {$info['uid'] = $info['id']; unset($info['id']);}
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
function membersHave($list) {return doRecords($list, 'MembersHave');}
function selling($list) {return doRecords($list, 'Selling');}
function companyFlags($list) {return doRecords($list, 'CompanyFlags');}
function coupons($list) {return doRecords($list, 'Coupon');}
function stats($list) {return doRecords($list, 'Stat');}

/**
 * Create device (box) records.
 */
function doBoxe($info = []) {
  global $testOnly, $channel; // be careful not to overwrite channel with a local variable
  //$boxName = $info['boxName'];
  $info['channel'] = @strtr(@$info['channel'], ray('sms pos web', TX_SMS, TX_POS, TX_WEB)) ?: $channel;
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

  if (is_object($info)) $info = (array) $info;
/**/  if (!is_array($info)) {echo 'not array'; print_r($info); die(trace()); } // keep for now
  extract($info); // any user table fields
  $id0 = @$id;
  u\setDft($id, r\Acct::nextId()); // default to first available number
  $uid = t\uid($id); // change .AAB to 1, etc.

  if (@$jid) $jid = t\uid($jid);

  if (!$testOnly) { // when making, set many defaults and mods
    $nmMap = ray('ZZA:Abe One,ZZB:Bea Two,ZZC:Cat Pub,ZZD:Dee Four,ZZE:Eve Five,ZZF:Fin Corp,ZZG:Glo Seven');
    u\setDft($fullName, substr($id, 0, 2) == 'ZZ' ? strtr($nmMap) : 'doAccount ' . u\randomString(20, 'word'));
    $name = u\shortName($fullName);
    u\setDft($legalName, $fullName);
  //  $pass = @$my_password ?: '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae'; // default to 123
    if (trim(@$pass) === '') $pass = '123';

    $zza = r\qo('.ZZA')->id;
    $mailDft = ($zza <= $uid and $uid < $zza + 26) ? chr(ord('a') + $uid - $zza) : u\randomString(10, 'word');
    u\setDft($email, $mailDft . '@' . EMAILX); // a@example.com, etc.

    u\setDft($community, r\serverUid());
    u\setDft($zip, '01301');
    u\setDft($country, 'US');
    $state = r\realState(@$state, $country);
    u\setDft($postalAddr, @$address2 ? "$address2, $city2, $state2 $zip2" : @$address . ', ' . @$city . ", $state $zip");
//    u\setDft($photoId, "$state-{$zip}999");
    u\setDft($helper, '.AAB'); // everyone but .AAB should have a helper
/**/  if ($helper === '') die(trace()); // set helper to zero if none, never blank
    u\setDft($last4bank, @substr(@$bankAccount, -4, 4));

    $moneyFields = 'share committed floor rewards'; // not minimum because NULL means not set, in Membership page
    foreach (ray($moneyFields) as $one) u\setDft($$one, 0);
    if (@$cc) $cardCode = $cc;
    if (@$cc2) $cardCode2 = $cc2;

    $fixFields = 'rebate fullName legalName name pass email secure community zip country postalAddr cardCode cardCode2 ' . $moneyFields;
  } else $fixFields = '';

  if (isset($flags) and isset($acctType) and $acctType !== CO_PERSONAL and $acctType != 'personal') $flags .= ',co';

//  if (isset($risks)) $risks = getRisks($risks, K_ACCT_RISKS);
  
  foreach (ray('id acctType dw cc cc2') as $one) unset($info[$one]); // these are not real fields
  u\preray(compact(ray('uid jid helper flags risks last4bank ' . $fixFields)), $info); // add in changed fields
  fixData($info, TRUE);

  if ($testOnly) {
    if (!$a = r\acct($uid)) return output("No such uid: $uid");
    $a->reread(TRUE); // this is required, for example, for cron's Trust test
    foreach ($info as $key => $want) {
      if ($key == 'uid') continue;
      $actual = ($key == 'trust' or $key == 'risk') ? round($a->o($key), 2) : $a->o($key);
      if (!eq($actual, $want)) {
        if ($key == 'pass' and (!isset($pass) or $a->passwordOkay(@$pass, 'pass', $err))) continue;
        if ($key == 'flags') list ($want, $actual) = [u\bits($want, B_LIST), u\bits($actual, B_LIST)];
        return output("uid $uid -- Wrong value for $key. Expected:$want Got:$actual");
      }
    }
    return TRUE;
  }

/**/ if (!$a = new r\Acct($info)) return t\output('Cannot create account: ' . print_r($info, 1));
  if (@$picture) if (!makePicture($picture, $a)) return FALSE;
  $a->stepDone('signup');
  if ($id0 == $id) $a->setBit(B_IDED); // assume we have a photoId for givens unless a step says otherwise

  return $uid;
}

/**
 * Interpret flag names
 */
function fixFlags($flags) {
  global $testOnly;

  if (!@$flags or is_numeric($flags)) return $flags + 0;  
  $res = 0;
  foreach (ray($flags) as $one) $res |= u\bit(u\consta('b', $one));
// NO! fails for r_txs and r_relations flags  if ($res & u\bit(B_OK)) $res |= u\bit(B_MEMBER);
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
 * Fabricate transactions or invoices OR test them.
 * @param assoc $info: transaction or invoice data to create or test
 *   Minimum parameters: amount, from, to
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
  
  if (isset($risks)) $risks = getRisks($risks, K_TX_RISKS);

  if ($inv) {
    u\setDft($goods, u\forCash(@$for) ? FOR_USD : FOR_GOODS);
  } else {
    $data = [];
    if (!$testOnly) u\setDft($risk, NULL); // indicates that risk has not been calculated yet
//    if (@$xid and !is_numeric($xid)) $xid = u\a2n(substr(strpbrk($xid, R_MEMBER_MARK . R_AGENT_MARK), 1));
//    if (@$xid and !is_numeric($xid)) $xid = u\a2n($xid);
    u\setDft($type, TX_TRANSFER);
    foreach (ray('type') as $one) if (!is_numeric($$one)) $$one = u\consta('tx', $$one);
    if (r\isReward($type) and $amount) list ($payerReward, $payeeReward, $amount) = [0, $amount, 0];
    if (isset($rebate)) $payerReward = $rebate;
    if (isset($bonus)) $payeeReward = $bonus;
    
    if (is_array(@$for) or @$for != 'roundups desc') if (@$payer) $roundup = r\acct($payer)->roundup;
    
    if (!$testOnly) {
      u\setDft($goods, ($type != TX_TRANSFER or u\forCash(@$for)) ? FOR_USD : FOR_GOODS);
      if ($goods == FOR_GOODS) {
        u\setDft($payerReward, r\reward($payer, $payee, $amount, TRUE));
        u\setDft($payeeReward, r\reward($payer, $payee, $amount, FALSE));
      }
    }
    if (in(@$purpose, 'offline')) $data['force'] = 1;
    if (in(@$purpose, 'undoneBy:') or in(@$purpose, 'undoes:')) {
      list ($k, $v) = explode(':', @$purpose);
      $data += array($k => $v);
    }
    if (@$fromAgent or @$from) $payerAgent = t\agent(@$fromAgent ?: ($type == TX_TRANSFER ? $from : r\communityUid()));
    if (@$toAgent or @$to) $payeeAgent = t\agent(@$toAgent ?: $to );
    unset($info['purpose']);
    u\setDft($taking, 0);
    if (!$flags0 = @$flags) $flags = 0;
    foreach (ray(TX_FLAGS) as $k) if (@$$k or $$k = in($k, $flags0)) u\setBit($flags, $k, $$k);
  }
  
  $fields = 'xid payer payerAgent payee payeeAgent amount payerReward payeeReward type goods data risk risks';
  if (!$testOnly) $fields .= $inv ? ' created' : ' created channel'; // when creating, add these if not already explicit
  if (!$inv) $fields .= ' flags';
  
  $info = compact(ray($fields)) + $info; // apply modifications

  foreach (ray('from to for rebate bonus ' . TX_FLAGS) as $one) unset($info[$one]);

  if (!@$inv) {
    if (@$for and !@$payerFor and !@$payeeFor) list ($payerFor, $payeeFor) = is_array($for) ? $for : array($for, $for);
    foreach (['payerFor', 'payeeFor'] as $k) if (@$$k) $info[$k] = t($$k);
  } elseif (@$for and !@$purpose) $info['purpose'] = t($for);
  
//  if ($testOnly or @$type == TX_REBATE or @$type == TX_BONUS) {
  if ($testOnly) {
    if ($inv) return verifyRecord('r_invoices', $info);
    return verifyRecord('r_txs', $info);
  }
  
  if ($inv) {
    return db\insert('r_invoices', $info);
  } else {
    $x = new r\X($info);
    return $x->xid;
  }
}

/**
 * Interpret an id in a transaction or invoice.
 * @param string $id: qid or partial qid (eg NEWAAA or .AAA or C:A or NEWZZC-A)
 * @param int $main: (RETURNED) the payer or payee
 * @param int $other: (RETURNED) the payerAgent or payeeAgent
 */
function txUid($id, &$main, &$other) {
  list ($main, $other) = [t\uid($id), t\agent($id)];
}

/**
 * Fabricate bogus relations.
 * Minimum parameters: main, agent (both are qids -- at least one must be local)
 */
function doRelation($info) {
  include_once __DIR__ . '/admin/admin-forms.inc';
  global $testOnly, $memberIdCalled;

  t\fixData($info);
  extract($info);
  if (@$agent) {
    $info['other'] = $other = $agent;
    unset($info['agent']);
  }

  list ($mainA, $agtA) = [r\acct($main), r\acct(@$other)];
  foreach ([$main => $mainA, $other => $agtA] as $i => $a) if (!$a or !$a->proSe) return t\output("bad rel acct: $i");

  if (isset($id) and !@$reid) $reid = $id;
  u\setDft($otherNum, @$num ?: 0);
  $info['permission'] = @$permission ? u\consta('b', $permission) - B_RELATED : 0;

  if (@$draw) $agtA->setBit(B_DRAWS);
  foreach (ray(REL_FLAGS) as $k) if (@$$k) u\setBit($flags, $k);
  
  foreach (ray('id agent num rCard ' . REL_FLAGS) as $k) unset($info[$k]);
  $info += compact(ray('reid main other otherNum flags'));
  
  if ($testOnly) {
    return verifyRecord('r_relations', $info);
  } else {
    $mainA->newRelation($info);
    if (@$rCard == 'yes') {
      if (!$agtA->cardCode2) r\acct($main, $other)->makeCardCode();
      if (!@$memberIdCalled) r\Web\memberId($main, $other); // make a Company Agent rCard
      $memberIdCalled = TRUE;
    }
    return TRUE;
  }
}

/**
 * List what a company sells
 */
function doSelling($info) {
  global $testOnly;
  extract(just('id selling', $info));
  $selling = str_replace(',', "\n", $selling);
  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  return $a->update(compact('selling'));
}

/**
 * List what flags the company has set
 */
function doCompanyFlags($info) {
  global $testOnly;
  extract(just('id flags', $info));
  $coFlags = 0;
  if ($flags) foreach (ray($flags) as $one) {
    $flag = u\bit(u\consta('co', $one));
    if (in($one, APP_CAN_BIT_NAMES)) $flag <<= APP_CANS;
    $coFlags |= $flag;
  }
  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  return $a->update(compact('coFlags'));
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
  $message = strtr(strip_tags(tr($notice, @$subs)), $map);
  $sql = 'SELECT message FROM r_notices WHERE uid=:uid AND sent=:sent ORDER BY msgid DESC';
  $info = ray('uid sent', $uid, @$subs['sent'] ?: 0);
  $msgs = db\q($sql, $info)->fetchCol();
///  debug(compact(ray('message sql info msgs')));
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
function doneStep($id, $step) {
  global $testOnly; if ($testOnly) return FALSE;
  if (strpos($step, ' ')) {
    foreach (ray($step) as $one) if (!doneStep($id, $one)) return FALSE;
    return TRUE;
  }

  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  @$a->stepDone($step); // might have to do the steps below also
  return TRUE;
}

/**
 * Make the member fields be thus and such OR test to see if they ARE thus and such.
 * "?" in a field means ignore its value when testing
 */
function doMembersHave($info) {
  global $testOnly;
  if ($balFields = just(T_BALANCE_FIELDS, $info) and !doBalance($balFields, TRUE)) return FALSE;

  $a = r\acct($id = t\uid($info['id']));
  $info = justNOT(T_BALANCE_FIELDS, $info); // don't rehandle balance fields, and already got id
  fixData($info, TRUE);
  if ($testOnly) {
    foreach ($info as $k => $v) if ($v != '?') {
      if (t\badEq("db $k", $a->o($k), $v, $id)) return FALSE;
    }
    return TRUE;
  } else return ($info ? $a->update1($info) : TRUE); // handle no fields other than balFields
}

/**
 * Check or make member field assignments.
 */
function memberFieldValues($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(just('id field value', $one));
    $a = r\acct(t\uid($id));
    if ($field == 'helper' or $field == 'community') $value = t\uid($value);
    if ($testOnly) {
      if (t\badEq('fieldValue wanted/actual:', $value, $a->o($field), $id)) return FALSE;
    } else $a->update1($field, $value);
  }
  return TRUE;
}

/**
 * Make the cached balances and other numeric fields be thus and such.
 * "?" in a field means ignore its value when testing
 */
function doBalance($info) {
  global $testOnly;

/**/  if ($info != just(T_BALANCE_FIELDS, $info)) return t\output("spurious field in balances: " . print_r($info, 1));
  $id = t\uid($id0 = $info['id']);

  if (!$a = r\acct($id)) return t\output("account $id0 does not exist");
  $a->reread(TRUE); // otherwise db updates are invisible

  if (isset($info['r']) and !isset($info['balance'])) $info['balance'] = $info['r'] - @$info['rewards']; // 20170515
  t\fixData($info);
/// print_r(compact('a','id','id0'));
  if (!$testOnly) { // setting the values
    unset($info['id']);
    return ($info ? $a->update($info) : TRUE); // handle only id being passed
  }

  // verify
  if (!$calc = t\uidCredit($id)) return FALSE;
  foreach (ray($cashies = 'balance rewards') as $k) {
    $v = @$info[$k];
    if (isset($v) and $v != '?') if ($id > 0 or $k != 'rewards') {
      $cachedVal = $a->o($k);
      if (t\badEq("cached $k", $cachedVal, $v, $id)) return FALSE;
      if (!$a->jid and t\badEq("calc $k", $calc->$k, $v, $id)) return FALSE;
    }
  }
  foreach ($not = justNOT($cashies . ' id', $info) as $k => $v) {
    if ($v != '?' and $k != 'committed' and $id > 0) if (t\badEq("db $k", $a->o($k), $v, $id)) return FALSE;
  }
  return TRUE;
}

/**
 * Return a field for the account with the given cell number.
 */
function phoneField($field, $number) {
  return ($mya = r\acct(r\SMS\uid($number))) ? $mya->$field : NULL;
}

function usdTransferCount($where = '1') {
/**/  if (!isDEV or isPRODUCTION) die('usdTransferCount ' . DEV_ONLY_MSG);
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
    if (@$tx['payee']) $payee = $tx['payee'] = t\uid($tx['payee']);

    if ($testOnly) {
      if (isset($tx['risks'])) $tx['risks'] = getRisks($tx['risks'], K_TX_RISKS);
      if (!verifyRecord('r_usd', $tx)) return FALSE;
    } else {
      if (!$a = r\acct(@$payee)) return t\output('Bad payee: ' . @$payee);
      $tx['bankAccount'] = @$a->vsecure['bankAccount'];
      u\setDft($tx['txid'], t\seq());
      u\setDft($tx['risk'], NULL);
      u\setDft($tx['created'], @$tx['completed']);
      if (!db\insert('r_usd', $tx)) return FALSE;
//      if (@$tx['completed']) $a->update('r', $a->o_r - $tx['amount']);
      if (@$tx['completed']) $a->update('balance', $a->o_balance + $tx['amount']);
    }
  }
  return TRUE;
}

/**
 * Run the op in the background, return TRUE.
   include_once __DIR__ . '/../rcredits/cg-testing.inc';
   f('t.cronRuns', 'op');
 */
function cronRuns($op) {
  global $testOnly; if(@$testOnly) return FALSE;
  global $cronOp; $cronOp = $op;
  include_once __DIR__ . '/rcron/rcron.inc';
  
  $now = r\rTime();
  \variable_set('r_last_cron', ray('day week month', $now, $now, $now)); // normally don't run periodics

  if ($op == 'ALL') { //  or strpos(R_DAILY_TASKS, " $op ")
/**/  if (strftime('%H') < R_DAILY_HOUR) die('Developer: For testing between midnight and :R_DAILY_HOUR:00, you need to temporarily change R_DAILY_HOUR in -settings.');
    db\q('TRUNCATE queue');
    \variable_set('r_last_cron', ray('day week month', $now - DAY_SECS, 0, 0));
  } elseif ($op) cr\doTask(compact('op'));
  cr\run();
  
  return TRUE;
}

/**
 * Return the credit info for the given uid.
 * To get just the balance, use uidCredit(uid)->balance
 * @param int $uid: return credit info for this account ID (FALSE if there is an error)
 * @param bool $ignoreCalc: <ignore errors when comparing with calculated values>
 */
function uidCredit($uid, $ignoreCalc = TRUE) {
  $info = be\creditInfo(ray('uid noj', $uid, TRUE));
  $a = r\acct($uid);
  if ($uid > 0 and !$a->jid) foreach (ray('balance') as $k) { // (community amounts don't get cached)
    if (!t\eq($v1 = $info->$k, $v2 = $a->o($k) + 0)) {
      output("$uid Calculated $k different from cache: $v1 <> $v2");
      if (!$ignoreCalc) return FALSE;
    }
  }
  return $info;
}

/**
 * Accept abbreviations of alphabetic ids for accounts, transactions, and relations
 * @param string $q: the possibly-abbreviated id
 * @return: the full id (if $q is numeric or an already-full alphabetic id, return it unchanged)
 * For rSmart tests, the qid always has a dot.
 */
function fullQid($q) {
  global $channel;
  if (is_numeric($q) or !$q) return $q;
  if (substr($q, 0, 1) == '.') $q = r\qo($q)->qid;

  return ($channel == TX_POS and !strpos($q, '.')) ? substr($q, 0, 3) . substr($q, 3) : $q;
}

/**
 * Interpret the given identifier (int, full qid, or local qid) as an account ID.
 */
function uid($id, $field = 'id') {
//  u\EXPECT($id, 'empty id'); // otherwise site loops (?)
  if (is_numeric($id) or empty($id)) return $id;
  if ($id == 'community' or $id == 'ctty') return r\serverUid();
  if ($id == 'cgf') return r\cgfId();
//  return r\qo($id)->$field; FAILS
  return r\qo(t\fullQid2($id))->$field;
}
function agent($id) {return t\uid($id, 'agent');}

function getErrors() {
  $errors = \drupal_get_messages('error', FALSE);
  $errors = @$errors['error'] ?: [];
  foreach ($errors as $key => $value) {
    if (u\abbreviates('Unable to send e-mail', $value) or strpos(strip_tags($value), 'expected to be a reference, value given in drupal_retrieve_form()')) unset($errors[$key]); // ignore these when testing
  }
  return $errors;
}

/**
 * Say whether we sent the given email to admin or ctty admin.
 * @param string $topic: message topic
 * @param assoc $subs: message parameters
 * @param int $coUid: <tell ctty admin instead of overall admin>
 */
function weTellAdmin($topic, $subs = [], $coUid = FALSE) {
  global $testOnly; if (!$testOnly) return t\output('Cannot MAKE tellStaff!');

  if (!@$subs[0]) $subs[0] = [];
//  $regionEmail = r\regionfield('email');
//  $email = $coUid ? r\acct($coUid)->cttyA->email : R_ADMIN_EMAIL;
  $email = $coUid ? SYS_EMAIL : R_ADMIN_EMAIL;
  return t\findEmail('tell-staff', $email, $subs[0] + ray('topic noFrame message', $topic, TRUE, '?'));
}

/**
 * Say whether any such email has been sent to the specified address.
 */
function noSuchEmail($index, $email) {
  global $testOnly; if (!$testOnly) return FALSE;
  return !findEmail($index, $email, []);
//  $subject = strtr($GLOBALS['emailSubjects'][$index], $esubs);
//  $pattern = strtr('%"index":"INDEX","email":"EMAIL"%', ray('INDEX EMAIL', $index, $email));
//  return !db\exists('r_log', 'info LIKE :pattern', compact('pattern'));
}

/** 
 * See whether the specified email has been sent (by searching the log file)
 * @param string $index: email message index
 * @param string $email: the email address to find
 * @param assoc $subs: parameters within the message
 * @return <email was sent>
 */
function findEmail($index, $email, $subs) {
  global $testOnly; if (!$testOnly) return t\output('Cannot MAKE findEmail!');
  global $testEmails; if (!@$testEmails) return t\output('There are no testEmails!');
///  debug(u\jsonEncode(compact(ray('index email subs testEmails'))));

  $subs = is_array(@$subs[0]) ? $subs[0] : @$subs;
  t\fixEmail($email);
  t\fixData($subs);
  if ($a = r\acct($email)) $subs['qid'] = $a->mainQid;
  
  foreach ($subs as $k => $v) {
    if ($k == 'nudge') $subs[$k] = $v = t($v);
    $esubs['{' . $k . '}'] = $v;
  }
  $message = @$subs['noFrame'] ? r\emailTemplate($index) : r\emailBody($index);
  $message = str_replace("\r\n", '', $message);
  $messageDpy = strtr(t\showSubs($message, $esubs), ray('href= style= src=', 'hrref=', 'styyle=', 'srrc='));
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
        if (isset($t_subs['site'])) $subs['site'] = BASE_URL; // site parameter is set in boot.inc so compiler.php can't see it.
///        print_r($subs); die('here');
        if (@$ignore) foreach ($ignore as $k) unset($t_subs[$k]);
        ksort($t_subs);
        $wantedInActual = just(array_keys($subs), $t_subs);
        $actualInWanted = just(array_keys($t_subs), $subs);
        if ($wantedInActual == $actualInWanted) return TRUE; // actual is exactly what was wanted
        t\output(compact('wantedInActual', 'actualInWanted'), 'email');
        foreach ($subs as $k=>$v) if (isset($t_subs[$k]) and $v != $t_subs[$k]) t\output("k=$k v=$v tv=" . @$t_subs[$k], 'email');
        foreach ($t_subs as $k=>$v) if (isset($subs[$k]) and $v != $subs[$k]) t\output("k=$k tv=$v v=" . @$subs[$k], 'email');
      }
    }
  }
  
  output(['EXPECTED (but did not get)'=>''] + compact(ray('index email subjectDpy subs messageDpy')), 'email');
  return FALSE;
}

/**
 * Say whether any email was sent to the given address.
 * @return FALSE if any email was sent, else TRUE.
 */
function noEmailTo($email) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $testEmails;

  if (@$testEmails) foreach ($testEmails as $one) if ($one['toEmail'] == $email) return FALSE;
  return TRUE;
}

/**
 * Highlight the substitutions in a given text.
 * @param string $s: the text
 * @param assoc $subs: the substitutions
 * @return the text with substitutions highlighted
 */
function showSubs($s, $subs) {
  foreach ($subs as $k=>$v) $subs[$k] = "<span class=\"test-highlight\">$v</span>";
  return strtr($s, $subs);
}

/**
 * Simulate posting to a page from some other site (or app).
 */
function postToPage($page, $args) {
  global $testOnly; if ($testOnly) return FALSE;
  $params = t\ray2assoc($args);
  t\POST($params);
  return t\memberVisitsPage('?', $page);
}

function memberVisitsPage($id, $page) { // pass options in URL
  global $testOnly; if ($testOnly) return FALSE;
  if (!t\login($id)) return FALSE;
  t\pageForm(t\fixPageArgs($page), FALSE);
  return TRUE;
}

function weShowWith($title, $content = FALSE, $mustBePresent = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($title and !weShow($title, $mustBePresent)) return FALSE;
  return weShow($content, $mustBePresent);
}

function weShowWithSubs($page, $subs) {
  global $testOnly;
  global $testConfirmation; // confirmation message output, if any
  global $formOut; // for testing

  $subs = @$subs[0] ?: []; // comes in as "" if no subs

  if (strpos($page, 'confirm ') !== FALSE) {
    foreach (just(R_DATE_FIELDS, $subs) as $k => $v) if (is_numeric($v)) $subs[$k] = u\fmtDate($v);
    $wanted = t($page, $subs);
    t\output("<b>WANTED: </b>$wanted", 'screen');
    $res = ($wanted == $testConfirmation);
/**/ if (!$res) debug(compact('wanted', 'subs', 'testConfirmation'));
    return $res;
  }
  
  if (strpos($page, '/statement')) {
    $form = @file_get_contents(TEST_PDF_FLNM);
  } else {
    $form = strtr(t\strip(r\Web\showForm($page, @$subs['arg1'], @$subs['arg2'])), $subs);
    t\output("<b>WANTED: </b>$form", 'screen');
    if ($form != $formOut['text']) return FALSE;
  }
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

/* UNUSED
function getPicture($picture) {
  return file_get_contents(DRUPAL_ROOT . R_PICTURE_DIR . "small/$picture.jpg");
}*/

/**
 * Send an RPC API request to the server, for rPOS.
 * @todo: after everyone is up to version 217, add penultimate param: $proof = 'calc'
 */
function hitServer($op, $extra = [], $agent, $device = '', $version = 221) {
  global $rposResult;
  if ($agent) $agent = t\fullQid2($agent);
  extract(just('amount member code created proof goods', $extra));
  if (is_numeric(@$amount)) $amount = number_format($amount, 2);
  if (@$member) $member = t\fullQid2($member);
  if ($version >= 217) foreach (['agent', 'member'] as $k) if (@$$k) $$k = str_replace('.', '', $$k); 
  if (@$proof == 'calc' and @$member) {
    if ($a = r\acct($member)) u\setDft($code, $a->cardCode());
    $company = ($i = strpos($agent, '-')) ? substr($agent, 0, $i) : $agent;
    $proof = u\hash($hashOf = "$company$amount$member$code$created");
    t\output("proof is hash of $hashOf", 'normal');
    unset($extra['code']);
  }
  if (@$goods and !is_numeric($goods)) $goods = t\goodsBit($goods);

  $input = compact(ray('op agent device version amount member proof goods')) + $extra;
/**/ t\output('request: ' . print_r($input, 1), 'input'); // keep this
  $rposResult = r\Smart\pos($input);
  if ($op != 'photo') $rposResult = (array) json_decode($rposResult);
/**/ t\output('answer: ' . print_r($rposResult, 1), 'output');
  return TRUE;
}

function txCountIs($n) {
  global $testOnly; if (!$testOnly) return FALSE;
  return (db\count('r_txs') == $n);
}

/**
 * Allow member or agent code to be abbreviated (without the region) as .YYY or x:y,
 * where ZZx is the company abbreviation and ZZy is the agent
 */
function fullQid2($agent) {
// FAILS if (strlen($agent) > 7) return $agent;
  if (strpos($agent, ':')) { // company:agent
    list ($cpy, $agt) = explode(':', $agent);
    $cpy = t\fullQid('.ZZ' . $cpy);
    if (!is_numeric($agt)) $agt = t\fullQid('.ZZ' . $agt); // numeric agt actually fails anyway :(
    return t\fullQid(r\qid($cpy, $agt));
  } elseif (strpos($agent, '#')) { // company#agentNum (deprecated)
    list ($cpy, $agt) = explode('#', $agent);
    $cpy = t\fullQid('.ZZ' . $cpy);
    if (is_numeric($agt)) $agt = u\n2a($agt, 1, 36);
    return "$cpy-$agt";
  } else return t\fullQid($agent);
}

function parseQr($qr) {
  list ($member, $code) = explode(',', $qr . ',');
  return [t\fullQid2($member), $code];
//  list ($member, $m, $code) = preg_split('/([\.-])/', $qr, 0, PREG_SPLIT_DELIM_CAPTURE);
//  $member = R_SERVER_ID . ($m == '-' ? ':' : $m) . $member;
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
 * @param string $id: full or partial qid or '?' (meaning anonymous user, so don't log in)
 * @return: successful (always successful for anonymous)
 */
function logIn($id) {
  global $boxUser;
  if ($id != '?') {
    r\Acct::setDefault($a = r\acct(t\fullQid2($id))); // fullQid not uid
    if (!u\isAcct($a)) return t\output("id not account: $id (" . t\fullQid2($id) . ')');
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
/**/  if (!is_array($info)) die('in verifyRecord: ' . trace()); // keep
  unset($info['data']);
  foreach ($info as $key => $value) {
    if ($value == '?') {unset($info[$key]); continue;}
    $crit[] = $value === '' ? "($key IS NULL or $key=:$key)"
    : ((in($key, R_DATE_FIELDS) or stripos($key, 'date') !== FALSE) ? 
      "DATE(FROM_UNIXTIME($key))<=>DATE(FROM_UNIXTIME(:$key))" 
    : (in($key, 'trust risk') ? "CAST(ROUND($key, 2) AS CHAR)<=>:$key"
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
 * @param string $id: qid for the user (or company agent) completing the form
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
	$lastGo = NULL;
///  debug(compact(ray('id page values confirmed testOnly')));

  $values = @$values[0] ?: [];

  t\logIn($id);

  $validate = TRUE;
  if (u\starts($page, 'history/') and $confirmed and !strpos($page, '&do=')) { // re-examine this
    $validate = FALSE;
    $formSta['confirm'] = TRUE;
  }

  $values = t\POST($values);

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
  if ($validate) if (!@$lastGo or @$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'validate') as $one) $one($form, $sta);
  $lastIdentifierTag = '</div identifiers>';
  if ($i = strpos($testConfirmation, $lastIdentifierTag)) $testConfirmation = substr($testConfirmation, $i + strlen($lastIdentifierTag));
  if ($fieldErr) return TRUE; // no submit if field error
  if (!@$lastGo or @$lastGo == $xlastGo) foreach (t\postProcess($form, $page, 'submit') as $func) $func($form, $sta);

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
  global $formArray, $formSta, $lastGo, $formOut;
  
  $formArray = $formSta = $formOut = NULL; // start with a clean slate

  if (!$page) $page = 'summary';
  if ($msg) r\Web\say($msg, $field);
  t\SERVER('REQUEST_URI', $page);
  $args = basename($page);
  $xlastGo = @$lastGo; // notice whether a goto happens. ignore all but the deepest (last).

/*
  if (u\abbreviates('/user/', $page)) {
//    $login = in($page, 'user/login');
    $sta = @$formSta ?: [];
    list ($form, $function) = FALSE ? array('user_register_form', 'formSignup')
      : ($login ? array('user_login', 'formSignin') : []);
    $function = 'CG\\Web\\' . $function;
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
    $function = "CG\\Web\\form{$formName}_$type";
    $functions = function_exists($function) ? array($function) : [];
  }
  return $functions;
}

function postLogout($function) {
  return TRUE; // never actually log out
  global $skipToStep;
/*  if ($messages = @cache_get('t_messages')->data) {
    extract(just('msgs skipToStep', $messages));
    if ($skipToStep == $function) {
      if (strlen(serialize($msgs)) > strlen(serialize(@$_SESSION['messages']))) {
        foreach (ray('status error notice') as $type) {
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
  global $testVars;
  $testVars[$varName] = t($index, $subs[0]);
//  $sceneTest->subs += ["%$varName" => $$varName];
  return TRUE;
}

function dropdownOptions($form) {
  if (!preg_match_all('~<option [^>]*>.*?</option>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function setChecked($name) {global $tChecked; $tChecked[] = $name;}
function setRadio($name, $title) {global $tRadios; $tRadios[$name] = $title;}

function radioOptions($form) {
  if (!preg_match_all('~<label class="option" [^>]*>.*? </label>~sm', $form, $matches)) return '';
  return strip_tags(join(PHP_EOL, $matches[0]));
}

function strip($form) {
  $form = str_replace('&nbsp;', ' ', $form);
  $form = preg_replace('~<fieldset id="help".*?</fieldset>~sm', '', $form); // remove help div
  $form = preg_replace('~<div[^>]*class="gone.*?</div>~sm', '', $form); // remove hidden divs (not nested)
  $form = preg_replace('~<option value="[^"]*">.*?</option>~sm', '', $form); // remove unselected options (fails for dropdowns created on the fly, eg state and country)
  $form = preg_replace('~<input type="hidden".*?/>~sm', '', $form); // remove hidden inputs
  $form = preg_replace('/<input[^>]* value=" *(.*?) *".*?>/sm', '$1', $form); // show button labels
  $form = preg_replace('/<input[^>]* checked[^>]* data-on="([^"]*)"[^>]*>/sm', '$1', $form); // bootstrap toggle on
  $form = preg_replace('/<input[^>]* data-off="([^"]*)"[^>]*>/sm', '$1', $form); // bootstrap toggle off
  $form = preg_replace('/> *</sm', '> <', $form); // make sure there's exactly one space between things
  $form = preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($form));
///     if (strpos($form, '<html') !== FALSE) die($form);
  return $form;
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
  if (isset($value)) $_SERVER[$key] = $T_SERVER[$key] = $value; else return $T_SERVER[$key];
}

/**
 * Fake $_POST params for testing
 */
function POST($values = NULL) {
  if (!u\test()) return $_POST;
  global $T_POST;
  if (isset($values)) {
    t\fixData($values);
    $_POST = $T_POST = $values;
  }
  return $T_POST;
}

/**
 * @param mixed $msg: what to give a background color to (can be any data type)
 * Not to be confused with the color() function in test.php
 */
function color($msg, $color = 'normal') {
/**/ $msg = print_r($msg, 1); // keep this
  return "<pre class=\"test-$color\">$msg</pre>";
}

/**
 * Output the message with the given background color.
 * Return FALSE (meaning error in returns from callers) unless a color is specified.
 */
function output($msg, $color = 'error') {
  global $wholeModule;
  if (u\test() and !@$wholeModule) \drupal_set_message(color($msg, $color));
  return $color == 'error' ? FALSE : TRUE;
}

/**
 * Clear out all possible data, so as to start the test fresh
 * @param int $ch: the channel (web, sms, etc.)
 */
function clear($ch = 0) {
/**/  if (!isDEV or isPRODUCTION) die('clear ' . DEV_ONLY_MSG);
  include_once __DIR__ . '/admin/admin.inc';
  global $lastGo, $formOut, $formArray, $formSta, $testOp, $testConfirmation, $tChecked, $tRadios, $testEmails;
  global $nextCode; $nextCode = NULL;
  global $sms_devel; $sms_devel = TRUE;
  global $testRewardStep; $testRewardStep = 500;
//  global $testRewarding; $testRewarding = TRUE; // normally set this true in tests only if testing this feature
  global $T_POST; $T_POST = $_POST = [];
  $lastGo = $formOut = $formArray = $formSta = $testOp = $testConfirmation = $tChecked = $tRadios = NULL;
  $formOut['text'] = '';
  $testEmails = NULL;

  global $sms_devel; $sms_devel = TRUE;
  global $testSeq; $testSeq = NULL;
  global $channel; $channel = $ch;
  global $testMessages; $testMessages = [];
  global $sceneTest; 
/**/ if (u\test() and !strpos($sceneTest->sceneName, 'Statement')) echo "scene: $sceneTest->sceneName<br>\n"; // helpful when test dies
///   file_put_contents(flog(), ''); // clear log file on disk

  $cgfUid = r\cgfId();
  $tables = 'r_bad r_ballots r_boxes r_stakes r_coupons r_coupated r_do r_gifts r_investments r_invites r_invoices r_near r_notices r_options r_pairs r_proposals r_questions r_proxies r_ratings r_request r_relations r_shares r_stats r_tous r_txs r_usd r_user_industries x_users x_relations x_invoices x_txs x_usd x_photos queue sessions cache';
  foreach (ray('bootstrap form menu') as $k) $tables .= " cache_$k";
  foreach (ray($tables) as $table) if (db\exists($table)) db\q("TRUNCATE $table");

  db\q('DELETE FROM users WHERE uid<>0');
  a\setupBasicAccounts();

  db\q('UPDATE users SET floor=0,balance=0,minimum=NULL,rewards=0,committed=0');

  ctty('aab')->setBit(B_OK);
  ctty('aab')->setBit(B_UP);
// NO  ctty('aab')->setBit(B_REWARDY); // do rewards by default

  t\log('clearing');
  r\Acct::_clear(); // empty the acct cache
  \drupal_static_reset(); // clear out drupal central storage
}

function fixPageArgs($page) {
  $special = array(
//    '(by|id)=([A-Z]{3}?' . R_MARKS . '[A-Z]{3})',
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
  $bits = array_flip(array_keys(ray($descs)));
  $risks = 0;
  foreach (ray($risks0) as $bit) $risks |= u\bit($bits[$bit]);
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
///  debug(compact(ray('var a bal i op value')));
  return array($$var + $value, $bal);
}
*/

function badEq($type, $v1, $v2, $id) {
  if (t\eq($v1, $v2)) return FALSE;
  t\output("uid=$id badEq $type v1=$v1, expected v2=$v2");
  return TRUE;
}

/**
 * Say whether the most recent page included the given text or options.
 * @param array $content: array of assocs -- named values to check for
 *   The names (labels) will be sought once also, unless preceded by underscore (_).
 *   The array can go either way, horizontally or vertically
 * @param string $type: description of what to check for (default 'text')
 * @param bool $mustBePresent: whether to complain about missing info (otherwise complain about info that is present)
 * @return TRUE if all the content is included
 */
function shownWith($content, $type = 'text', $mustBePresent = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  global $formOut;

  $model = t\strip($formOut[$type]);
//  if (!$model and $type == 'text') $model = file_get_contents(TEST_PDF_FLNM);

  if (!$content) return FALSE; // probably an accidental single-row array with no labels
/**/  u\EXPECT(is_numeric(key($content[0])), 'shown with should be array, not assoc: ' . print_r($content, 1));
  $ray = call_user_func_array('array_merge', $content);

  $xi = $i = 0;
  foreach ($ray as $one) {
    if ($one === '' or substr($one, 0, 1) == '~') continue; // ignore ~labels
//    if (in($one, '%')) $one = t\lastMinuteSubs(htmlspecialchars($one));
    $one = t($one); // translate constants in member
    if (($one = u\redash($one)) === '') continue;
    $i = strpos($model, $one, $xi = $i);
///    debug(compact(ray('model one xi i')));
    if ($i === FALSE) $i = strpos($model, plain($one), $xi);
    if ($i === FALSE xor !$mustBePresent) {
      t\output(t('EXPECT page %with %this', 'with this', $mustBePresent ? t('with') : t('without'), u\purify($one, '')));
      t\output(compact('xi','i','one'));
      if (!$mustBePresent) $xi = $i; // show what's found if it's not supposed to be there
      t\output(substr($model, 0, $xi) . ' <b class="test-doublesize">[OK until here]</b> ' . substr($model, $xi));
      return !$mustBePresent;
    } else $i += strlen($one);
  }
  return $mustBePresent;
}

/**
 * Say whether we have shown the given text.
 * @param bool $show: <complain if not shown> (else complain if shown)
 */
function weShow($what, $show = TRUE) {
  global $testOnly; if (!$testOnly) return FALSE;
  if ($what === '') return TRUE;
  return t\shownWith(is_array($what) ? $what : [[t($what)]], 'text', $show);
}

function constants($list) {
  global $testOnly;
  foreach ($list as $one) {
    extract(just('name value', $one));
    
  }
}

/**
 * Say whether the member has shown a photo ID or (if not just testing) make it so.
 * @param string $id: abbreviated member ID
 */
function noPhotoId($id) {
  global $testOnly;
  if (!$a = r\acct(t\uid($id))) return t\output("Not an account: $id");
  return $testOnly ? !$a->ided : $a->setBit(B_IDED, FALSE);
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
/**/ return t\output('Expected: ' . print_r(compact(ray('index id uid subs')), 1) . '<br>Got: ' . print_r($testMessages, 1));;
}
    
/**
 * Check the linked URL content and any Drupal messages (which don't show up in file_get_contents) against expectations.
 */
function hasLinkResults($thing, $info) {
  global $formOut;
  global $testOnly; if (!$testOnly) return FALSE;

  if (!preg_match('/ href="([^"]*)"/', t\that($thing), $matches)) return FALSE;
  
  $s = file_get_contents($matches[1]);
  foreach (ray('error status') as $one) $$one = @file_get_contents(t\dosayFilename($one));
  $formOut['text'] = str_replace($was = '#header -->', "$was$error$status", $s);
  return t\shownWith($info, 'text', TRUE);
}

function seq($table = 'r_usd') {
  global $testSeq;
  $id = $testSeq[$table] = @$testSeq[$table] + 1;
  $k = $table == 'r_usd' ? 'txid' : '';
//  t\log("seq table=$table id=$id");
  u\EXPECT(!db\exists('r_usd', "$k=:id", compact('id')), "duplicate seq: $id in table $table");
///  print_r(compact('table','testSeq','id','k'));
  return $id;
}

function log($msg) {if (u\test()) u\log($msg, 'debug');}
function that($thing) {global $thatThing; return $thatThing[$thing];}
function dosayFilename($type) {return DRUPAL_ROOT . R_PATH . "/dosay-$type.txt";}
function goodsBit($s) {return $s == 'goods' ? "1" : ($s == 'cash' ? "0" : "ERROR");} // no 3rd why for rSmart
function approxTime($a, $b) {return t\eq($a, $b, 10000);}
function rewarding() {return ctty('aab')->cttyRewardy;}
function ray2assoc($ray) {foreach ($ray as $one) $res[$one[0]] = $one[1]; return @$res ?: [];}