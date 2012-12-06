<?php
/**
 * @file
 * All of the business logic common to the various rCredits interfaces
 * (SMS, web, smartphone, etc.)
 * EXCEPT:
 *   - the Drupalish callbacks, which are in rcredits.module
 *   - application interfaces to the user interfaces, which are in rcredits-backend.inc (i)
 *   - utilities, which are in rcredits-util.inc (u)
 */
 
namespace rCredits; // typically abbreviated as "r"
use rCredits\Util as u;
use rCredits\Backend as be;

define('R_VERSION', '0.1c'); // software version
define('R_SEPARATE_BACKEND', FALSE);
define('R_TEMP_NAME_PREFIX', 'unknown ');
define('R_ASIF_TEXT', ' "AS-IF"'); // how to identify "As If" transactions

// Filenames
define('R_TEMPLATE_DIR', '/templates');
define('R_EMAIL_FRAME', 'email-frame');

// Roles
define('R_ROLE_RTRADER', 4);

// Bank account states
define('R_BANK_VERIFIED', 1); // we have proof that the user owns the bank account

// Account types
define('R_PERSONAL', 0);
define('R_COMMERCIAL', 1);
define('R_NONPROFIT', 2);
$GLOBALS['account types'] = array(t('personal'), t('commercial'), t('non-profit'));

// Relationship permissions used in calls to acct::can()
define('PERM_RELATED', 0); // has a relationship record
define('PERM_READ', 1);
define('PERM_SELL', 2);
define('PERM_BUY', 3);
define('PERM_RESERVED4', 4);
define('PERM_MANAGE', 5);

define('PERM_COMPANY', 7);
$GLOBALS['share permissions'] = array('no access', 'read transactions', 'sell', 'buy and sell', 'other', 'manage account');
define('PERM_CHARGE', 8);
define('PERM_MEMBER', 10); // signed up for rCredits
define('PERM_PARTNER', 11); // signed the membership agreement and donated
define('PERM_STEWARD', 12); // is allowed to vote and is responsible for stewarding the Common Good Community
define('PERM_RTRADER', 14); // is allowed to use rCredits for real
define('PERM_CTTY_ADMIN', 99);
define('PERM_ADMIN', 100);

// Permissions, options, and flags (flags field in account record)
define('BIT_PARTNER', 1); // signed the membership agreement and donated
define('BIT_STEWARD', 2); // is allowed to vote and is responsible for stewarding the Common Good Community
define('BIT_RTRADER', 4); // is allowed to use rCredits for real
define('BIT_CHARGE', 8); // can charge customers without confirmation (treat this like a permission)
define('BIT_VIRTUAL_EMPLOYEES', 16); // pay employees virtually
define('BIT_VIRTUAL_ALL', 32); // pay everyone virtually
define('BIT_SECRET_BAL', 64); // don't let merchants tell me my balance on request
define('BIT_NOTIFY_EMAIL', 128); // notifications by email
define('BIT_NOTIFY_SMS', 256); // notifications by SMS
define('BIT_ADMIN', 32768); // is an administrator (not yet implemented)
define('BIT_DEFAULTS', BIT_NOTIFY_EMAIL | 0); // what bits to set when creating a new account

// Transaction states
define('TX_STATES', 'deleted done pending disputed denied');

define('TX_DELETED', 0);
define('TX_DONE', 1);
define('TX_PENDING', 2); // charged, waiting for confirmation
define('TX_DISPUTED', 3); // unilaterally charged, but disputed by the payer (the charge stands, until settled)
define('TX_DENIED', 4); // invoice rejected by the non-originator

// Transaction channels (roughly in order of simplicity and generality of messages)
define('TX_SMS', 1); 
define('TX_WEB', 2);
define('TX_SMART', 3); // smart phone or other computer-like device
define('TX_TOUCH', 4); // touch tone phone
define('TX_CHANNELS', 'None SMS Web Smart Touch');

// Transaction types
define('TX_SIGNUP', 0); // signup incentive reward
define('TX_TRANSFER', 1); // normal fund transfer (usually for actual goods and services)
define('TX_REBATE', 2);
define('TX_BONUS', 3);
define('TX_RESERVED4', 4); // not currently used
define('TX_AUTOMATIC', 4); // highest numbered automatic

define('TX_INFLATION', 5); // reward to conpensate for inflation
define('TX_GRANT', 6);
define('TX_LOAN', 7);
define('TX_RESERVED8', 8); // not currently used
define('TX_FINE', 9);
$GLOBALS['txTypes'] = array(
  t('signup'), t('transfer'), t('rebate'), t('bonus'), '', t('inflation'), t('grant'), t('loan'), '', t('fine'),
);

$GLOBALS['emailSubjects'] = array(
  'new-payment' => t('rCredits PAYMENT from {otherName}'),
  'new-charge' => t('rCredits CHARGE from {otherName}'),
  'charge-disputed' => t('rCredits Charge DISPUTED by {otherName}'),

  'new-invoice' => t('rCredits INVOICE from {otherName}'),
  'invoice-denied' => t('rCredits Invoice DENIED by {otherName}'),
  'invoice-canceled' => t('rCredits Invoice CANCELED by {otherName}'),

  'new-offer' => t('rCredits PAYMENT OFFER from {otherName}'),
  'offer-refused' => t('rCredits Payment REJECTED by {otherName}'),
  'offer-canceled' => t('rCredits Invoice CANCELED by {otherName}'),
);

include_once __DIR__ . '/rcredits-settings.inc'; // secret settings for this community's rCredits server
include_once __DIR__ . '/acct.class';
include_once __DIR__ . '/rcredits-circles.inc'; // treat this as an extension of this file, for testing

/**
 * Format and return args for reporting a transaction (old or new) to the current account.
 * Not all returned args are used for any particular type of transaction.
 * Note that templates and tests depend on the names of some of these fields
 * @param array $tx: the transaction array (including zxid)
 * @param int $getBalance: 0=don't get any balances, 1=get my balance and tid, 2=get other party's balance and tid also
 * @param float $shortfall: how much short my balance is, for a transaction (if any)
 * @return assoc array of args
 */
function txReportArgs($tx, $getBalance = 0, $shortfall = 0) {
  extract($tx); // any or all standard transaction fields
  if ($data) if ($data = unserialize($data)) extract($data, EXTR_PREFIX_ALL, 'data');
  
  $toMe = ($payee == acct()->id);
  $role = $toMe ? 'payee' : 'payer';
  list ($purpose, $otherPurpose) = u\order($toMe, $payee_for, $payer_for);
  list ($payerPurpose, $payeePurpose) = array($payer_for, $payee_for); // clearer names, for substitutions
  $tofrom = $toMe ? t('from') : t('to');
  $uid = $otherUid = ($toMe ? $payer : $payee);
  $action = $taking ? t('charged') : ($goods ? t('paid') : t('gave'));
  $confirmAction = $taking ? t('charge') : ($goods ? t('pay') : t('give'));
  $summary_word = $taking ? t('Charge') : t('Payment');
  $amount = u\formatAmount($amount);
  $short = u\formatAmount($shortfall);
  if (@$data_bonus) $rewardType = $taking ? t('bonus') : t('rebate');
  $rewardAmount = u\formatAmount($toMe ? @$data_bonus : @$data_rebate);
  $otherName = userField('full_name', @$data_foreign_uid ?: $otherUid);
  $created = strftime('%d-%b-%Y', $created);

  $real = ($zxid > 0);
  $asif = $real ? '' : R_ASIF_TEXT;
  if ($getBalance > 0) {
    list ($tid, $balance) = array(be\zxid2tid($zxid), be\creditInfo(compact('real'))->fancy['balance']);
    if ($getBalance > 1) {
      list ($otherTid, $otherBalance) = array(be\zxid2tid($zxid, $otherUid), be\creditInfo(compact('real', 'uid'))->fancy['balance']);
    }
  }
  
  $summary = "$asif $summary_word of $amount $tofrom $otherName for \"$purpose\" on $created";
  $success = TRUE;
  return array_merge($tx, compact(u\ray('created zxid action real role payerPurpose payeePurpose purpose otherPurpose toMe asif confirmAction amount tofrom rewardType rewardAmount balance otherBalance tid otherTid otherUid otherName short summary success')));
}

/**
 * Record a transaction between two participants
 * @see also transactionSetup(), r\transactionRewards(), and be\transfer()
 */
function transact($info) {
  $txs = array(transactionSetup($info));
  transactionRewards($txs);
  return txSave($txs);
}

/**
 * Save the transaction set
 * @todo: don't give real rewards to the passive party in an ASIF transaction (look retroactively)
 */
function txSave($txs) {
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  //*** Begin DBTX
  $dbtx = db_transaction();
  $table = zxs($my_payer, $my_payee);
  if (@$my_data['zxid'] < 0) $table = zxs(FALSE/2); // rare case: undoing an asif, even though both parties are now rTraders
  foreach ($txs as $key => $one) {
    if (empty($one)) continue; // skipping rebate or bonus
    $txs[$key]['serial'] = @$serial; // set serial for dependent transactions
    \drupal_write_record($table, $txs[$key]); // pass actual array element, to get xid
    if ($key == 0) {
      $txs[$key]['serial'] = $serial = $txs[0]['xid']; // set serial for main transaction
      dbQ("UPDATE $table SET serial=:serial WHERE xid=:serial", compact('serial'));
    }
  }

  unset($dbtx); // commit ($dbtx->commit() is disallowed)
  //*** End DBTX

  $txs[0]['zxid'] = zxid($txs[0]['xid'], $table); // report zxid to caller
  return $txs;
}

/**
 * Add rebate and bonus to the transaction set, if appropriate.
 */
function transactionRewards(&$txs) {
  extract($txs[0], EXTR_PREFIX_ALL, 'my');
  
  if ($my_type != TX_TRANSFER) return;
  if (!$my_goods) return; // quit unless rebates and bonuses apply
  if (owning($my_payer, $my_payee)) return; // no rewards for transactions between owner and owned
  $rewards = unserialize($my_data);
  
  $txs[] = txRewardSetup(TX_REBATE, $my_payer, $my_state, $rewards);
  $txs[] = txRewardSetup(TX_BONUS, $my_payee, $my_state, $rewards);
}

function txRewardSetup($type, $payee, $state, $rewards) {
  $payer = $payer_agent = $payee_agent = userField('community_uid', $payee);
  if ($payer == $payee) return NULL; // payee is a community, so no rewards

  $taking = FALSE;
  $goods = FALSE;
  $amount = $rewards[$type == TX_REBATE ? 'rebate' : 'bonus'];
  $for = $type == TX_REBATE ? t('rebate') : t('bonus');
  return transactionSetup(compact(u\ray('type amount goods payer payee payer_agent payee_agent for taking state')));
}

/**
 * Create a new transaction and set up all the important fields.
 * @param array $info: an associative array of transaction info. The following elements are required:
 *   amount: how much to transfer
 *   payer, payee: uids
 *   payer_agent, payee_agent: uids
 *   for: purpose of the transaction OR a simple array (payer's purpose, payee's purpose)
 *   taking (bool): did the payee initiate the transaction
 * @return the array, with some extra field information added
 */
function transactionSetup($info) {
  global $channel; // through what medium was the transaction requested (web, SMS, etc.).
  extract($info, EXTR_PREFIX_ALL, 'my');
  $required_params = compact(u\prefixValues('my_', u\ray('amount payer payee payer_agent payee_agent for taking')));
  u\EXPECT($required_params, 'float zid zid zid zid notempty bool');

  if (!isset($my_state)) $state = TX_DONE;
  if (!isset($my_goods)) $goods = FALSE; // set TRUE only in be\transfer()
  $created = REQUEST_TIME;
  $serial = NULL; // always set equal to xid of main transaction after saving
  
  list ($payer_for, $payee_for) = is_array($my_for) ? $my_for : array($my_for, $my_for);
  unset($info['for']);

  return array_merge($info, compact(u\ray('state goods created serial payer_for payee_for channel')));
}

/**
 * Set or report how much rCredits the user wants to buy with US Dollars.
 * @param integer $who: uid of the user
 * @param numeric $demand: the user's new demand amount (defaults to 0, meaning "just report")
 * @return the user's demand amount
 */
function demand($uid, $demand = FALSE) {
  if ($demand === FALSE) return userField('demand', $uid);
  $demand = min(R_MAX_AMOUNT, $demand);
  userUpdate(compact('demand'), $uid);
  return $demand;
}

/**
 * Calculate how much too little the payer has
 */
function shortfall($uid, $requestAmount, $preapproved, $real) {
  if (!$preapproved) return 0; // if this is just an invoice, funds don't have to be there yet (no shortfall)
  $available = be\creditInfo(compact('uid', 'real'))->available;
  $amount = min($requestAmount, $available);
  $shortfall = max(0, $requestAmount - $amount); // how much needs to be done in a separate currency
  if ($shortfall) {
    $shortfall = max(.01, $shortfall); // don't report a tiny shortfall by rounding down to zero
    $short = u\formatAmount($shortfall);
    $shortfall = ceil($shortfall); // shortfall should be a round figure (no cents)
  }
  return $shortfall;
}

/**
 * Pay each employee the amount
 */
function payEmployees($amount0, $for, $confirmed) {
  $employees = employees();
  if (!($employeeCount = count($employees))) return array('no employees', array(), array());
  if ($shortfall = shortfall(acct()->id, $amount0 * $employeeCount, TRUE, isRTrader())) {
     $short = u\formatAmount($shortfall);
     return array('short payment', compact('short'), FALSE); // no partial payment of employees
  }
  $total = u\formatAmount($amount0 * $employeeCount);
  $amount = u\formatAmount($amount0);
  if (!$confirmed) return array('pay employees', compact(u\ray('employeeCount amount total')), TRUE);
  $pay_all = TRUE;
  $results = array();
  $successes = 0;
  foreach ($employees as $acct) {
    $results[] = list ($message, $args, $confirm) = be\transfer('payment', $acct, $amount0, $for, !u\forCash($for), TRUE, compact('pay_all'));
    if ($args['success']) $successes++;
  }
  $success = ($successes > 0);
  return array('paid employees', compact(u\ray('total successes employeeCount results success')), FALSE);
}
 
/**
 * Notify the specified user, according to their preferences
 */
function notify($uid, $index, $info) {
  global $channel;
  $acct = acct($uid);
  extract(u\just('full_name mail', $account = $acct->account()));
  $fullName = @$full_name ?: t('rCredits Member');
  $standard_fields = array(
    'uid' => $uid,
    'fullName' => $fullName,
    'email' => $mail, // testing uses this
  );
  
  $info = array_merge($info, $standard_fields);
  if ($uid != acct()->id) $info['otherName'] = acct()->full_name; // this comes in backwards if notifying elseone

  $subs = array();
  if (!strlen($acct->pass)) {
    $warning = u\tt('no asif notify', compact('fullName'));
  } else {
    if ($acct->hasBit(BIT_NOTIFY_EMAIL) and !empty($mail)) {
      foreach ($info as $key => $value) $subs['{' . $key . '}'] = $value;
      $mailed = drupal_mail('rCredits', $index, $mail, user_preferred_language($account), $subs);
    }
    if ($acct->hasBit(BIT_NOTIFY_SMS)) {
      $number = dbLookup('number', 'r_sms', 'uid=:uid AND status=:SMS_PRIMARY', compact('uid'));
      if ($number) $smsed = SMS\send($number, u\tt($index, $info, TX_SMS));
    }
    $notification = str_replace('-', ' ', $index);
    if (!@$mailed and !@$smsed and $index != 'new-payment') $warning = u\tt('cannot contact', compact('fullName', 'notification'));
  }
  if (@$warning) {
    if ($channel == TX_WEB) Web\say($warning);
    if ($channel == TX_SMS) { } // dunno how to handle this (send a second message?)
    if ($channel == TX_SMART) { } // dunno how to handle this, but easier than SMS
  }
}

/**
 * Return a unique temporary name related to the given uniquely identifying parmeters 
 * in this order of preference: 
 *   phone: "unknown phone dddddddddd"
 *   email: "unknown whatever AT domain DOT ext"
 *   UNUSED (because it's hard to verify): website: "unknown domain DOT ext SLASH andsoforth"
 * If none, just pick a 10-character random string ("unknown 234h2khpsdf987")
 * @param array $info: associative array of fields, each of which uniquely identifies a user.
 * @return string: the temporary name
 */
function tempName($info) {
  extract($info, EXTR_PREFIX_ALL, 'my');
  $phone = @$my_number ?: (@$my_phone ?: '');
  if ($phone) return R_TEMP_NAME_PREFIX . u\formatPhone($phone, 'n');
  if (isset($my_email)) return R_TEMP_NAME_PREFIX . str_replace('@', ' AT ', str_replace('.', ' DOT ', $my_email));
// NO  if (isset($my_website)) return R_TEMP_NAME_PREFIX . str_replace('.', ' DOT ', str_replace('/', ' SLASH ', $my_website));
  return R_TEMP_NAME_PREFIX . substr(md5('zot' . strval(REQUEST_TIME)), 0, 10); // pseudo random
}

/**
 * Set a transaction field for the current user.
 * @param string $fieldName: the field to update
 * @param mixed $new_value: what to update it to
 * @param int $zxid: transaction id & table
 * @param bool $all_related: set all related transactions too (default FALSE)
 * NOTE: tx data changes(date, agent, field, oldvalue) (this gives a complete history of the transaction)
 *     (prefixes added to the payer_for or payee_for fields when changing state will not be included in changes)
 */
function setTxField($fieldName, $new_value, $zxid, $all_related = FALSE) {
  $cuid = acct()->id;
  u\EXPECT(compact(u\ray('fieldName zxid all_related')), 'name zid bool');
  
  list ($xid, $table) = azxid($zxid);
  $where = "xid=:xid and $cuid in (payer, payee)"; // "$cuid in" is a security measure
  list ($old_value, $data) = array_values(dbLookup("$fieldName,data", $table, $where, compact(u\ray('xid'))));
  $data = unserialize($data);
  $data['changes'][] = array(time(), acct()->agent, $fieldName, $old_value);
  $data = serialize($data);
  $sql = "UPDATE $table SET $fieldName=:new_value, data=:data WHERE $where";
  $sql2 = "UPDATE $table SET $fieldName=:new_value WHERE serial=:xid AND type<>:TX_TRANSFER";
//  $sql2 = str_replace(', data=:data', '', str_replace('xid=:xid', 'serial=:xid', $sql));
  
  /*** Begin DBTX ***/
  $dbtx = \db_transaction();
  dbQ($sql, compact(u\ray('new_value data xid')));
  if ($all_related) dbQ($sql2, compact(u\ray('new_value xid'))); // gotta do the auxiliary records separately so as not to affect their data field (separate query, to avoid "unbuffered query" error -- known Drupal bug)
  unset($dbtx);
  /*** End DBTX ***/
}

/**
 * Merge a temporary ("old") account into the current account (a permanent ("new") account).
 * @return 1 (for counting how many were merged)
 */
function mergeAccounts($oldUid) {
  $newUid = acct()->id;
  if (!be\isTempAccount($oldUid)) return; // don't merge non-temp account
//  u\EXPECT(be\access('manage account'), "unauthorized mergeAccounts: $newUid into $oldUid");
//  if (!isTempName(userField('name', $oldUid))) die('merging non-temp account');
  // there are no other records for temporary accounts, than those below

  $new = acct($newUid);
  $old = acct($oldUid);
  $update = array();
  foreach (u\ray('phone fax website address') as $one) if ($old->$one and !$new->$one) $update[$one] = $old->$one;
  
  /*** Begin DBTX ***/ $dbtx = \db_transaction();
  userUpdate($update, $newUid);
  $sql = <<<EOF
    UPDATE r_asif SET payer=:newUid WHERE payer=:oldUid;
    UPDATE r_asif SET payee=:newUid WHERE payee=:oldUid;
    DELETE FROM r_asif a LEFT JOIN r_asif b ON b.serial=a.serial WHERE b.payer=b.payee;
    DELETE FROM r_asif WHERE payer=payee;
    UPDATE r_sms SET uid=:newUid WHERE uid=:oldUid;
    UPDATE r_user_industries SET uid=:newUid WHERE uid=:oldUid;
    DELETE FROM users WHERE uid=:oldUid;
EOF;
  dbQ($sql, compact('newUid', 'oldUid'));
  /*** End DBTX ***/ unset($dbtx);
  return 1;
//  dbQ("UPDATE users SET uid=IF(uid>0,-uid,uid), status=0 WHERE uid=:oldUid", compact('oldUid'));
}

/**
 * Reset all rCredits data, to start development with a clean slate
 */
function reset() {
//  $ctty_uid = serverUid();
  $sqls = array(
    'TRUNCATE TABLE r_txs',
    'TRUNCATE TABLE r_asif',
    'TRUNCATE TABLE r_sms',
//    "DELETE FROM users WHERE uid > $ctty_uid",
    'TRUNCATE TABLE sessions',
  );
  foreach ($sqls as $sql) \db_query($sql);
  \rebuild_menus();
  drupal_goto('devel/cache/clear?destination=node');
}

/**
 * Return the real or ASIF transaction table name
 * @param int $uid1: uid of first member (DEFAULTs to current)
 *   OR boolean/2 $uid1: TRUE/2 if real (call with boolean/2 to allow administrator to do asif transactions)
 * @param int $uid2: uid of second member (DEFAULTs to current)
 * @return: the real table name if the one user or both users (if specified) are rTraders, otherwise the asif table name
 */
function zxs($uid1 = 'current', $uid2 = '') {
  if (((string) $uid1) == 'current') $uid1 = acct()->id;
  $real = ($uid1 >= 0 and $uid1 < 1) ? (2 * $uid1) : (isRTrader($uid1) and isRTrader($uid2)); // $real < 1 means boolean
  return $real ? 'r_txs' : 'r_asif';
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @params array $subs: optional substitutions -- keys will be prefixed by ':'
 */
function dbLookup($fieldName, $table_name, $where = '1', $subs = array()) {
  u\EXPECT(compact(u\ray('fieldName table_name where subs')), 'notempty notempty notempty assoc');
  $sql = "SELECT $fieldName FROM $table_name WHERE $where";
  if (!strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $result = dbQ($sql, $subs);
  return strpos($fieldName, ',') ? $result->fetchAssoc() : $result->fetchField();
}

/**
 * Return a qid for the zid
 * @param $zid: a zid
 * @param string $regionID: the 3-character regional server ID (defaults to this server)
 * @param int $min_len: minimum length of the part of the qid after the "." or ":"
 * @return the qid, FALSE if wrong format
 */
function qid($zid = '', $regionID = R_SERVER_ID, $min_len = 1) {
  if (!u\isZid($zid)) return FALSE;
  return $regionID . ($zid < 0 ? (':' . u\n2a(-$zid, -$min_len)) : ('.' . u\n2a($zid, -$min_len)));
}

/**
 * Return a zid and region for the qid
 * @return [zid, regionUid]
 */
function unQid($qid) {
  if (!u\isQid($qid)) return FALSE;
  list ($regionID, $type, $iid) = preg_split('/(\.+|=|:)/', $qid, 0, PREG_SPLIT_DELIM_CAPTURE); // allow a horizontal colon
  
  $id = u\a2n($iid);
  return array($type == '.' ? $id : -$id, serverUid($regionID ?: R_SERVER_ID));
}
  
/**
 * Convert uids to quid
 * Call by:
 *   quid()
 *   quid(uid) == quid(uid, uid)
 *   quid(uid, agentUid)
 *   quid(uid, agentUid, regionUid)
 * @param int uid: record id of the account (defaults to current account)
 * @param int agentUid: uid of agent (defaults to uid)
 * @param int $regionUid: uid of the region (always negative, defaults to current region)
 * @return the corresponding quid
 */
function quid($uid = '', $agentUid = '', $regionUid = '') {
  $cacct = acct();
  if ($agentUid) {
    if (!$regionUid) $regionUid = serverUid();
  } else list ($uid, $agentUid, $regionUid) = 
    $uid ? array($uid, $uid, serverUid()) 
         : array($cacct->id, $cacct->agent, $cacct->region);
  return qid($uid == $agentUid ? $uid : -relation('reid', $uid, $agentUid), u\n2a(-$regionUid, 3), 3);
}

function qxid($zxid, $regionID = R_SERVER_ID) {return qid($zxid, $regionID = R_SERVER_ID, 4);} // Convert zxid to qxid

/**
 * Convert a zxid to xid and table or vice-versa
 */
function zxid($xid, $table) {return $table == zxs(TRUE/2) ? $xid : -$xid;}
function azxid($zxid) {return $zxid > 0 ? array($zxid, zxs(TRUE/2)) : array(-$zxid, zxs(FALSE/2));}

/**
 * Interpret transaction record fields as an acct object
 * @param int/array $id: the account id or [id, agent, data]
 * @param int $agent: account id of the transaction agent
 * @param assoc $data: extra info from the transaction, including info about foreign parties to the transaction
 * @return acct
 */
function tx2acct($id, $agent = '', $data = '') {
  if (is_array($id)) list ($id, $agent, $data) = $id;
  if (is_array($data)) extract($data); // foreign_uid and/or foreign_agent (or neither)
  return acct(@$foreign_uid ? qid($foreign_uid, $id) : $id, @$foreign_agent ? qid($foreign_agent, $agent) : $agent);
}

/**
 * Determine whether the record can be undone
 * @param assoc $oldRecord: the transaction record or NULL (to make it easy when last_tx found no matching transaction)
 * @param assoc $subs: (RETURNED) necessary substitutions, if any
 * @return string: the error message (FALSE if the current agent CAN undo it on behalf of the current account)
 */
function nonoUndo($oldRecord, &$subs = array()) {
  if (!@$oldRecord) return 'undo no match';
  $cacct = acct();
  extract($oldRecord, EXTR_PREFIX_ALL, 'old');
  $data = unserialize($old_data);
  if (@$data['undone']) {
    $subs = array($old_tid => zxid2tid($old_zxid));
    return 'already undone';
  }

  $acct = tx2acct($old_toMe ? array($old_payee, $old_payee_agent, $data) : array($old_payer, $old_payer_agent, $data));
  if ($cacct->agent != $acct->agent) {
    $perm = $old_taking ? PERM_BUY : PERM_SELL;
    if (!acct($acct->id, $cacct->agent)->can($perm)) return $old_taking ? 'no buy' : 'no sell';
  }
  return FALSE;
}

/**
 * Create or update a reverse relation record for the given main_uid and agent_uid
 * if any of the given fields is positive
 * @param int $agent_uid: the main_uid (switching to agent)
 * @param int $main_uid: the agent_uid (switching to main)
 * @param assoc $data: (byRef) array of field values
 *   Returned with all fields listed in $fields unset (if any of those fields has a positive value)
 * @param string $fields: space-delimited list of fields to check
 * @return (none)
 */
function reverseRelations($agent_uid, $main_uid, &$data, $fields) {
  foreach (u\ray($fields) as $field) if (@$data[$field]) $stay = TRUE;
  if (!@$stay) return;
  $subs = compact('main_uid', 'agent_uid') + u\just($fields, $data);
  if ($reid = relation('reid', $main_uid, $agent_uid)) $subs += compact('reid');
  \drupal_write_record('r_relations', $subs, $reid ? 'reid' : array());
  foreach (u\ray($fields) as $field) unset($data[$field]); // don't get these fields mixed in with the reverse record
}

/**
 * Say whether one of the accounts owns the other.
 */
function owning($uid1, $uid2) {
  $where = 'is_owner AND ((main_uid=:uid1 AND agent_uid=:uid2) OR (main_uid=:uid2 AND agent_uid=:uid1))';
  return $uid1 == $uid2 ?: relation(1, $where, compact('uid1', 'uid2'));
}

/**
 * Say whether the first account is an employee of the second
 * @param int $company: defaults to current uid
 */
function isEmployee($uid, $company = '') {
  if (!$company) $company = acct()->id;
  return relation(1, $company, $uid, 'employer_ok AND employee_ok');
}

function communityUid($postal_code = '') {
  if (!$postal_code) return serverUid();
  $where = "country=':R_COUNTRY' AND :postal_code RLIKE postal_code AND uid=community_uid";
  return userField('uid', $where, compact(u\ray('postal_code'))) ?: serverUid();
}

function isForeign($uid) {
  $name = userField('name', $uid);
  return $name == (R_SERVER_ID . '.') ? FALSE : ($uid < 0 and preg_match('/[A-Z]{3}\./', $name));
}

/**
 * Given a full name, make a username and add digits until it is unique.
 */
function uniqueName($full_name) {
  $name = $name0 = u\shortName($full_name);
  for ($i = 2; ; $i++) {
    if (!userField(1, 'name=:name', compact('name'))) return $name;
    $name = $name0 . $i;
  }
}

/**
 * Return the uid for the given qid or (short) name.
 * @return the uid (FALSE if unsuccessful)
 */
function loginString($id) {
  if ($acct = acct(strtoupper($id))) return $acct->id; // try to interpret it as a qid
  return userField('uid', 'name=:id', compact('id'));
}
  
/**
 * Return a list of employee accts for the current account.
 */
function employees() {
  $cuid = acct()->id;
  $result = dbQ('SELECT agent_uid, foreign_uid FROM r_relations WHERE main_uid=:cuid AND employer_ok AND employee_ok', compact('cuid'));
  $accts = array();
  foreach ($result as $key => $one) {
    extract((array) $one); // agent_uid, foreign_uid
    $accts[] = $agent_uid > 0 ? acct($agent_uid) : acct(qid($foreign_uid, $agent_uid));
  }
  return $accts;
}

function isRTrader($uid = '') {
  if (!$uid and !($cacct = acct())) return FALSE;
  return (userField('flags', $uid ?: $cacct->id) & BIT_RTRADER);
}

/**
 * @param mixed $who: criteria or uid (defaults to $cuid)
 * @return:
 *   data from the field, if a single field is requested
 *   an associative array of the field data, if more than one field is requested
 */
function userField($field, $who = 'none', $subs = array()) {
  if ($who == 'none') $who = acct()->id;
  if (!$who) return FALSE;
  if (is_numeric($uid = $who) and !strpos($field, '(')) { // handle numbered accounts efficiently
    if (!($acct = acct($uid))) return FALSE;
    if (strpos($field, ',')) {
      $result = array();
      foreach (u\ray($field) as $one) {
        if (strpos($one, ' AS ')) list($one, $key) = explode(' AS ', $one); else $key = $one;
        $result[$key] = $acct->$one;
      }
      return $result;
    } else return $acct->$field;
  }
//  debug(compact(u\ray('field who subs')));
  $result = be\getRecords('users', is_numeric($who) ? "uid=$who" : $who, $subs, $field, TRUE);
  if (count($result) < 1) return FALSE;
  u\EXPECT(count($result) == 1, "ambiguous user in userField(): $who");
  u\EXPECT(is_object($result[0]), 'bad object array in userField(): ' . print_r($result, 1)); // keep this
  return strpos($field, ',') ? ((array) $result[0]) : $result[0]->$field;
}

/**
 * Return the field or fields from the relations table, for the given id and agent.
 * Call by:
 *   relation(field, id, agent, where, subs) OR
 *   relation(field, where, subs)
 */
function relation($field, $id, $agent = array(), $where = '1', $subs = array()) {
  if (is_numeric($id)) {
     $where = "main_uid=:id AND agent_uid=:agent AND ($where)";
     $subs += compact('id', 'agent');
  } else list ($where, $subs) = array($id, $agent); // called with alternate form
  return dbLookup($field, 'r_relations', $where, $subs);
}

function userUpdate($info, $uid = '') {
  if ($acct = acct($uid)) {
    $acct->update($info); 
  } else {
    die('in userUpdate (never gets called): ' . print_r(compact('info','uid'), 1)); // after testing, make this a 2-line function
    $info['uid'] = $uid;
    \drupal_write_record('users', $info, 'uid');
  }
}

/**
 * Go to the specified page
 * Accommodate testing, if that's what's happening.
 * Drupal urlencodes the parameters (= and &), so the target page will have to sort that out. (can't use Header() here)
 */
function go($page) {
  if (defined('TESTING')) {
    list ($form, $arg1) = ($page == 'pay' or $page == 'charge') ? array('tx', $page) : array($page, '');
    return Web\showForm($form, $arg1);
  }
  \drupal_goto($page);
}

/**
 * @todo: maybe move the processing of zxs to a wrapper function db_txq() that also does postprocessing on fetchAssoc()
 */
function dbQ($sql, $subs = array(), $die = FALSE) {
  u\EXPECT(compact(u\ray('sql subs')), 'notempty assoc');
  if (strpos($sql, '{zxs}')) {
    extract(u\just('zxid real', $subs));
    if (@$zxid) {
      list ($subs['xid'], $table) = azxid($zxid);
    } else $table = zxs((isset($real) ? $real : isRTrader())/2);
    unset($subs['zxid']);
    unset($subs['real']);
    $sql = strtr($sql, array('{zxs}' => $table, 'zxid' => 'xid'));
  }
//  debug(u\getConstants($sql));
//  debug($subs);
//  if ($die)
 //if (@$subs['info']) {print_r(u\getConstants($sql) . ' ' . print_r(u\prefixKeys(':', $subs), 1)); die();}
  return \db_query(u\getConstants($sql), u\prefixKeys(':', $subs));
}

function error($logid) {
  $info = dbLookup('info', 'r_log', 'logid=:logid', compact('logid'));
  if ($info) $info = json_decode($info); else $info = 'unknown error';
  print_r($info); // keep this for now
//  drupal_set_message(print_r($info, 1), 'error');
//  drupal_goto('');
}

// Single line functions that need no explanation
function rebate($uid, $amount = 1) {return round($amount * userField('rebate_percent', $uid) / 100, 2);}
function bonus($uid, $amount = 1) {return rebate($uid, 2 * $amount);}
// UNUSED tx_field($fieldName, $xid, $real) {return dbLookup($fieldName, zxs($real/2), 'xid=:xid', compact('xid'));}
function setTxState($new_value, $zserial) {setTxField('state', $new_value, $zserial, TRUE);}
function totalDemand() {return dbLookup('SUM(demand)', 'users');}
function serverUid($regionID = '') {return -u\a2n($regionID ?: R_SERVER_ID);}
function emailTemplate($key) {return file_get_contents(__DIR__ . R_TEMPLATE_DIR . "/$key.html");}
function emailBody($key) {return str_replace('{BODY}', emailTemplate($key), emailTemplate(R_EMAIL_FRAME));}
