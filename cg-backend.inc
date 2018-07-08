<?php
/**
 * @file
 * Top level application interfaces (SMS, web, smartphone, etc.) to rCredits.
 * These high-level business functions are called by more than one interface.
 */

namespace CG\Backend; // typically abbreviated as "be"
use CG as r;
use CG\DB as db;
use CG\Util as u;
use CG\Backend as be;
use CG\Web as w;

define('REAL', @$_SERVER['SystemRoot'] != 'C:\\Windows'); // used?
// require_once __DIR__ . '/cg-util.inc';

/**
 * Return the user's transactions for the given period (inclusive)
 * @param int $starting: starting time (the beginning of some day)
 * @param int $ending: ending time (the end of some day)
 * @param mixed $uid: account record ID for which to return transactions
 *   'ctty' means return transactions for the current account's whole community
 * @param int $jid: account record ID for joined account, if any
 * @return the recordset (FALSE if none)
 * NOTE: Denied invoices are included only for the payee, otherwise all complete or tentative transactions
 * other than rebates and bonuses are included (even those will be included if the 'include_bonuses' option is set).
 */
function myTransactions($starting, $ending, $uid, $jid = 0) {
  $subs = compact(ray('starting ending uid jid'));
  
  if ($uid == 'ctty') { // all transactions in the community or region (not currently used)
    global $mya;
    $uid = $mya->community;
    $where = 't.created>=:starting AND t.created<=:ending'
    . ' AND t.type>=0' // omit invisible transactions
    . ' ORDER BY t.created DESC, t.xid DESC';
    $sql = <<<EOF
      SELECT t.*, IF(t.:IS_TAKING, t.payeeFor, t.payerFor) AS purpose
      FROM r_txs t LEFT JOIN users u1 ON u1.uid=t.payer LEFT JOIN users u2 ON u2.uid=t.payee
      WHERE :uid IN (u1.community, u2.community) AND :jid=0
      AND $where
EOF;
  } else { // normal account (member or community)
  /*
    $isPayer = '(t.payer IN (:uid, :jid) AND t.payee<>:uid)';

    $sql = <<<EOF
      SELECT t.*,
      (SELECT fullName FROM users WHERE uid=IF($isPayer, t.payee, t.payer)) AS name
      FROM r_txs t 
      WHERE ($isPayer OR t.payee IN (:uid, :jid))
      AND $where
EOF;
*/
    $iPay = '(payer<>0 AND payer IN (:uid, :jid))';
    $iGet = 'payee IN (:uid, :jid)';
//    foreach (ray(TX_FLAGS) as $k) $$k = u\bit(u\consta('b', $k)); // taking and roundup flags
    list ($fromBank, $toBank) = [t('from bank'), t('to bank')];
    $txFields = "xid, amount, t1.flags, IF($iPay, amount, 0) AS fromyou, IF($iPay, 0, amount) AS toyou, IF($iPay, payerReward, payeeReward) AS reward, payer, payee, type, t1.created, t1.data, $iPay XOR t1.:IS_TAKING AS byMe, t1.:IS_ROUNDUP AS roundup, t1.:IS_DISPUTED AS disputed, $iGet AS toMe, IF($iPay, payerFor, payeeFor) AS description, IF($iPay, payerAgent, payeeAgent) AS agent, IF(:uid<0, xid, IF($iPay, payerTid, payeeTid)) AS tid, goods, (goods=:FOR_SHARE) AS sharing, channel";
    $usdFields = "txid AS xid, amount, 0 AS flags, IF(:DEPOSIT_SQL, 0, -amount) AS fromyou, IF(:DEPOSIT_SQL, amount, 0) AS toyou, 0 AS reward, 0 AS payer, payee, :TX_BANK AS type, completed AS created, '' AS data, TRUE AS byMe, FALSE AS roundup, FALSE AS disputed, IF(:DEPOSIT_SQL, 1, 0) AS toMe, IF(:DEPOSIT_SQL, '$fromBank', '$toBank') AS description, '' AS agent, txid AS tid, :FOR_USD AS goods, 0 AS sharing, channel";
    $where = "created BETWEEN :starting AND :ending AND type>=0 AND ($iPay XOR $iGet)"; // omit invisible transactions

    $sql = <<<EOF
      SELECT * FROM
        (SELECT $txFields, u.fullName AS name FROM r_txs t1
         LEFT JOIN users u ON uid=IF($iPay, payee, payer)
         UNION SELECT $usdFields, ':R_NONE' AS name FROM r_usd) t
      WHERE $where ORDER BY created DESC, xid DESC;
EOF;
  }
  return db\q($sql, $subs); 
}

/**
 * Return an array of member names (called with ajax, so current user is passed as arguments).
 * POLICY QUESTION: Restrict to member's community? (yes for now)
 * @param int $myid: account record ID
 * @param int $aid: agent account record ID
 */
function memberRay($myid, $aid, $coOnly = FALSE) {
  if (!$a = r\acct($myid, $aid)) return [];
  $where = $coOnly ? ':IS_CO' : ($a->admin ? '1' : "community=$a->community OR :IS_CO");
  return db\q("SELECT fullName FROM users WHERE :IS_OK AND $where ORDER BY fullName")->fetchCol();
}

/**
 * Return the next relevant transaction for form 1099b, for an account.
 * @param int $payee: the account record ID
 * @param int $year: year or end-of-year date
 * @return the next relevant row (NULL if no more)
 * Note that the IRS prohibits reporting non-positive transactions (but requires all the others). So we compromise by weeding out the transactions with "undoneBy" or "undoes" in the data field. But we return other negative transactions so we can warn the member to report the total negative amount on line 2, Form C.
 * Also note that this report is always for just one account (and for just one SSN or EIN) 
 *   even if the account is joint.
 */
function get1099b($payee, $year = NULL) {
  global $q1099, $ignore1099;
  
  if (!@$q1099) {
    u\setDft($year, strtotime('last day of December last year'));
    $end = strlen($year) < 6 ? strtotime('1/1/' . ($year + 1)) - 1 : strtotime('tomorrow', $year);
    $start = strtotime('1 year ago', $end) + 1;
    $sql = <<<EOF
      SELECT created, amount, payer, payeeFor AS `for`
      FROM r_txs WHERE payee=:payee AND NOT :IS_UNDONE AND NOT :IS_UNDOES AND created BETWEEN $start AND $end 
      AND type NOT IN (:TX_REWARDS)
      ORDER BY created
EOF;
//      AND (goods=:FOR_GOODS OR type<>:TX_TRANSFER)
//      AND (goods OR (type NOT IN (:TX_TRANSFER, :TX_REFUND)))
    $q1099 = db\q($sql, compact('payee'));
  }
  return $q1099->fetchAssoc() ?: ($q1099 = NULL);
}

/**
 * Update relations
 * @param array $updates: list of records to update, indexed by reid
 *   each element is an associative array of new values, keyed by field name
 *   if a record is to be updated to all zeros, it is simply deleted.
 * @return string: a report of what got updated
 */
function updateRelations($updates) {
///  debug($updates);
  $myid = r\acct()->id;
  $report = '';
  foreach ($updates as $reid => $data) {
    extract(just('main other fullName permission flags', $data['original']));
    $wasEmpty = (!@$permission and !@$flags);
    unset($data['original']);
    extract(just('permission flags', $data));
//    foreach (ray('employee owner customer draws') as $k) u\setBit($flags, $k, @$data[$k]);
    foreach (ray('employee owner customer draw') as $k) u\setBit($flags, $k, @$data[$k]); // drawS?
    if (!u\getBit($flags, 'customer')) u\setbit($flags, 'autopay', FALSE);

    if (!@$permission and !@$flags and !$wasEmpty) { // empty now and wasn't before
      db\del('r_relations', $reid);
      $msg = 'deleted relation';
    } else {
      $a = r\acct($main, $other);
      if (@$data) $a->setRel($data); // db\update('r_relations', $data + $reidSub, 'reid');
      if (@$data['draw']) r\acct($other)->setBit(B_DRAWS);
      $msg = 'updated relation';
    }
    $otherName = $fullName;
    $report .= tr($msg, compact('otherName')) . '<br>';
  }
  return $report;
}

/**
 * Undo the given transaction.
 * Situations:
 *     I paid someone:            REVERSE (with reverse rewards)
 *     I charged someone:         REVERSE
 *     Someone paid me:           REVERSE
 *     Someone invoiced me and I approved it: DISPUTED
 *     Someone charged me unilaterally: DISPUTED
 * @param int $xid: the transaction number
 * @param string $where: additional (security check) criteria, if any
 * @param assoc $subs: additional substitions for $where
 * @param bool $force: <reverse despite any shortfall or lack of permission>
 * @return array [message, subs] where subs is an assoc of [
 */
function undoTx($xid, $where = '1', $subs = [], $force = FALSE) {
  global $mya;
  if (!$myx = r\MyX::lastTx($mya, "xid=:xid AND $where", compact('xid') + $subs)) return ['undo no match', $subs];
///  debug(compact('xid','where','subs','tx'));
  if (!$force and $error = $myx->nonoUndo($subs)) return array($error, $subs); // check nono before confirm AND here

//  $myx = r\x($tx, $mya);
  $data = $myx->data;

  if (!$myx->toMe and !$myx->byMe) { // someone charged (either unilaterally or I approved it)
    r\disputeTx($xid); // mark it disputed
    $solution = t('marked "disputed"');
    r\notify($myx->them, 'charge disputed', $myx->reportArgs());
  } else {
    $type = $myx->toMe ? 'charge' : 'payment';
//    $amounts = just('amount payerReward payeeReward', $myx->ray);
    $amounts = just('amount', $myx->ray);
    $what = [$myx->payerFor, $myx->payeeFor];
    return reverse($xid, $amounts, $what, $myx->a2, $data, $myx->goods, $myx->tid, $type, $myx->tid2, $force); // create an offsetting transaction
  }

  $amount = u\fmtAmt($myx->amount);
  $tofrom = !$myx->toMe ? t('to') : t('from');
  $otherUid = $myx->otherUid;
  $other = r\acct($otherUid)->fullName;
  $success = TRUE;
  return ['report undo', compact(ray('success amount other tofrom solution otherUid'))];
}

/**
 * Create an offsetting transaction (nearly the same as the original but with a negative amount).
 */
function reverse($xid, $amounts, $what, $acct, $data, $old_goods, $tid, $type, $otherTid, $force) {
  global $mya;
///    print_r(compact(ray('xid amount acct data tid toMe'))); die();
  u\EXPECT(compact(ray('xid amounts what acct data tid type otherTid')), 'id assoc array acct assoc|empty int string int'); // otherTid may be 0 (for reversing rewards)
  
  if (!is_array($data)) $data = [];
  $old_data = $data; // remember original data from transaction we're reversing
//  extract(just('rebate bonus xfee', $data));
  extract(just('xfee coupon', $data));
  $tids = u\order($type == 'payment', $tid, $otherTid);
  for ($i = 0; $i < 2; $i++) $what[$i] .= t(' (reverses #%tid)', 'tid', $tids[$i]);
//  $otherFor = str_replace($tid, $otherTid, $myFor);
//  $what = u\order($type == 'payment', $myFor, $otherFor);
//  foreach (ray('rebate bonus xfee') as $k) if (@$$k) $data[$k] = -$$k;
  if (@$xfee) $data['xfee'] = -$xfee;
  if (@$coupon) {
    $data['coupon'] = -$coupon;
    $payer = $type == 'charge' ? $acct->id : $mya->id;
    db\q('UPDATE r_coupated SET uses=uses-:duse WHERE uid=:payer AND coupid=:coupid', ray('duse payer coupid', ($coupon < 0 ? -1 : 1), $payer, $data['coupid']));
  }
  $data['undoes'] = $xid; // remember what we're reversing (especially to get the realness right)
  foreach ($amounts as $k => $v) $amounts[$k] = -$v;
  if ($force) u\preray(compact('force'), $data);

  $data['goods'] = $old_goods;
  list ($message, $args) = be\transfer($type, $mya, $acct, $amounts, $what, $data);

  if (@$args['success']) {
    extract(just('xid tid otherTid', $args), EXTR_PREFIX_ALL, 'new');
    $old_data['undoneBy'] = $new_xid;
    $old_data['changes'][] = array(r\rTime(), $mya->agentId, 'undo', $new_xid);
    $old_data = serialize($old_data);
    $myFor = " (reversed by #$new_tid)";
    $otherFor = str_replace($new_tid, $new_otherTid, $myFor);
    list ($payerFor, $payeeFor) = u\order($type == 'payment', $myFor, $otherFor);
    $sql = <<<EOF
      UPDATE r_txs SET 
        payerFor=CONCAT(payerFor, :payerFor), 
        payeeFor=CONCAT(payeeFor, :payeeFor),
        data=:old_data,
        flags=flags|(1<<:B_UNDONE)
      WHERE xid=:xid
EOF;
    $subs = compact(ray('payerFor payeeFor xid old_data'));
    db\logSql(compact(ray('sql subs')));
    db\q($sql, $subs);
    
    $message = 'report undo|' . $message;
    $args['solution'] = 'reversed';
  }
  
  return [$message, $args];
}


/**
 * Pay the given invoice, possibly without being signed in.
 * @param int $nvid: record ID of invoice to pay
 * @param string $msg: success or error message
 * @return TRUE if the payment was successful
 */
function payInvoice($nvid, &$msg = '') {
  if ($msg = r\badUnpaidInv($nvid, $inv)) return FALSE;
  extract(just('payer payee amount goods purpose data', $inv));
  $payerA = r\acct($payer);
  $payeeA = r\acct($payee);
  
  $DBTX = \db_transaction();
  $purpose .= ' (' . PROJECT . " inv#$nvid)";
  list ($index, $subs) = $payerA->pay($payeeA, $amount, $purpose, ray('inv goods', $nvid, $goods));
  if ($ok = @$subs['success']) //db\q('UPDATE r_invoices SET status=:xid WHERE nvid=:nvid', ray('xid nvid', @$subs['xid'], $nvid));
  db\update('r_invoices', ray('status nvid', @$subs['xid'], $nvid), 'nvid');
  if ($goods == FOR_INVESTMENT) { // investment club reclaiming an investment
/*
    $res = db\get('shid,vestid', 'r_shares s LEFT JOIN r_investments i ON i.vestid=s.vestid', 'coid=:coid AND clubid=:clubid AND pending<0', ray('coid clubid', $payer, $payee));
    if ($res) {
      extract($res); // shid vestid
      */
    if ($data = unserialize($data) and $shid = $data('shid')) { // record the sale
      db\update('r_shares', ray('shid shares pending when', $shid, -$amount, 0, NOW), 'shid');
    } else r\tellCo(t('Invoice payment marked as "for investment (repayment)" has no corresponding SELL request in shares table.'), $inv, $payee);
  }
  unset($DBTX);
  
  $msg = t($index, $subs);
  return $ok;
}

/**
 * Create a transaction
 * call by: list ($msg, $args) = be\transfer(...);
 * @param string $ttype: payment, charge, or invoice payment
 * @param acct $a1: acct object for first party (usually current user)
 * @param acct $a2: acct object for the other party
 *   OR 'ALL' (paying employees) -- currently UNUSED
 * @param mixed $amount: the transaction amount OR an assoc of numeric fields for the transaction (amount and rewards)
 * @param string $for OR array($payerFor, $payeeFor): description of the transaction's purpose (user input)
 * @param bool $oked: UNUSED has this transaction been confirmed by the user? (default TRUE)
 * @param array $data: associative array of info about the transaction this request is reversing (if any), 
 *   indexed by any of:
 *   'rebate': the rebate amount for this request (should be negative the original bonus amount)
 *   'bonus': the bonus amount (should be negative the original rebate amount)
 *   'undoes': transaction id of the related transaction (transaction being undone / invoice being paid)
 *   'goods': 0=for USD, 1=for real goods and services, 2=other
 *   'created': time of transaction (defaults to current time)
 *   'force': <do the transaction despite any shortfall (1 for original tx, -1 for forced reversal)>
 *   'offline'
 *   'investment'
 *   'inv': record number of the invoice for which this is payment
 *   'isGift': UNUSED <the payment is a gift to a community or nonprofit>
 *   'roundups', 'crumbs', 'patronage', 'gift': gift info
 *   'roundups': the payment is change being donated to the community
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 *    confirm: UNUSED boolean "this needs to be confirmed"
 *
 * Do something different, based on these boolean values: 
 *   $taking (charge vs. payment), 
 *   $oked UNUSED (user confirmed the transaction request),
 *   $goods (the transaction is an exchange for real stuff),
 *   $shortfall (how inadequate the payer's balance is)
  */
function transfer($ttype, $a1, $a2, $amount, $for, $data = []) {
  global $scanned, $channel;
  u\EXPECT(compact(ray('a1 a2 data')), 'acct acct assoc');
  list ($created, $goods) = [r\rTime(), FOR_GOODS]; // defaults
  extract(just('created goods', $data));
  u\setDft($for, ray(R_WHYS)[$goods]);

  $type = TX_TRANSFER;
//  if (is_array($amount)) extract(just('amount payerReward payeeReward', $amount));
  if (is_array($amount)) extract(just('amount', $amount));
  list ($myid, $uid) = array($a1->id, $a2->id);
  foreach (just('force offline investment created goods', $data) as $k => $v) {
    $$k = $v;
    if ($k != 'force') unset($data[$k]); // keep force in data also
  }
  if (@$force == 1) $offline = TRUE; // temporary until app handles this right
  $taking = ($ttype == 'charge');
  $erInfo = compact(ray('ttype amount goods for force')) + ray('a1 a2 op', $a1->fullName, $a2->fullName, $taking ? t('charged') : t('paid'));
  $amount = round($amount, 2); // ignore fractions of cents
  if (!@$amount) {
/**/  r\tellAdmin('zero transaction', $erInfo + ['stack' => trace()]);
    return r\txRet('zero transaction', $erInfo);
  }
  $reallyTaking = ($taking xor $amount < 0); // not taking if charging a negative amount

  list ($loserA, $gainerA) = u\order($reallyTaking, $a2, $a1);
  $erInfo += ['loserId' => $loserA->id];
  list ($payer, $payee) = u\order($taking, $uid, $myid); // Analyze who's buying or selling
  list ($payerAgent, $payeeAgent) = u\order($taking, $a2->agentId, $a1->agentId);

  if (!@$force and !@$data['inv'] and r\dupTx($uid, $myid, $taking ? $amount : -$amount)) return r\txRet('duplicate transaction', $erInfo);
  if ($err = r\txPermErr($a1, $a2, $taking, $amount < 0)) { // Check permissions
    list ($erMsg, $erSubs) = $err;
    if (@$force) {
      $msg = 'forced without perm';
      $erInfo += ['date' => u\fmtDate($created)];
      foreach (['agent1'=>$a1, 'agent2'=>$a2] as $k => $a) $erInfo += [$k => $a->proSe ? t('self') : $a->fullName];
      foreach ([$a1, $a2] as $a) if (!$a->proSe) r\message($a->id, $msg, $erInfo);
      r\tellAdmin($msg . ' ' . t($erMsg, $erSubs), $erInfo + $erSubs);
    } else return r\txRet($erMsg, $erInfo + $erSubs);
  }
  
  foreach ([$a1, $a2] as $a) if (!$a->co and !($channel == TX_POS ? $a->agentA->ided : $a->ided)) {
    if (@$force) {
      r\tellAdmin('forced without photoId', $erInfo); 
    } elseif ($gainerA->id != r\cgfId()) { // and !$gainerA->coCan(CO_FAST)
//      r\tellAdmin('tx without photoId', $erInfo); 
//      return r\txRet($a == $a1 ? 'no photoid' : 'other no photoid', $erInfo + ['who' => $a2->fullName]);
    }
  }
  
  // Check the amount and calculate any balance shortfall
  if ($error = u\badAmount($amount)) return r\txRet($error, $erInfo);
//  $need = $amount - @$payerReward; // leave room for reversing rewards (minus a negative)

  if ($shortfall = $loserA->id < 0 ? 0 : $loserA->shortfall($goods, abs($amount), TRUE, $a1)) {
    if (@$force) {
      $short = TRUE; // flag overdraft
    } else {
      $short = u\fmtAmt($shortfall);
      $otherName = $a2->fullName;
      $msg = $reallyTaking ? ($a2->can(B_SECRET) ? 'short from vague' : 'short from') : 'short to';
      if ($channel == TX_WEB) {
// to complex        if ($msg == 'short to' and $channel == TX_WEB and r\signedIn()) $msg .= '|increase min';
        if ($goods == FOR_GOODS) {
          if (!$loserA->can(B_DEBT) and $loserA->floor < 0 and $channel == TX_WEB) $msg .= '|try debt';
          $_aDebt = w\atag('/settings/preferences');
        } //elseif ($a1->cttyRewardy) $msg .= '|short cash help|maybe not cash';
      }
      r\suggestMin($loserA);
      $details = t('|%loser was short by %short trying to pay %gainer %amount', ray('loser short gainer amount', $loserA->fullName, $short, $gainerA->fullName, u\fmtAmt($amount)));
      return r\txRet($msg, compact(ray('otherName short how details _aDebt')));
    }
  }
//  $roundup = (r\acct($payer)->roundup and !@$data['isGift'] and !@$data['roundups'] and $amount > 0); // payer, not a1 or loser
  $roundup = (r\acct($payer)->roundup and !@$data['roundups'] and $amount > 0); // payer, not a1 or loser
  if ($amount > 0 AND $goods == FOR_GOODS) {
    $price = $loserA->applyCoupon($gainerA, $amount, $coupon, $coupid);
    if ($coupon) $data += compact(ray('coupon coupid'));
    if ($pot = $loserA->giftPot) $loserA->update('giftPot', max(0, $pot - $price)); // use up gift coupons, if any (if tx gets reversed, gift recipient can cash it out, but that's no biggie)
  }
  
//  $info = compact(ray('type created amount payer payee payerReward payeeReward payerAgent payeeAgent for data taking roundup goods force offline investment short'));
  $info = compact(ray('type created amount payer payee payerAgent payeeAgent for data taking roundup goods force offline investment short'));
  if (!$myx = new r\MyX($info, $a1, TRUE)) return r\txRet('tx save err', $erInfo);
  
  // Report results
  $args = $myx->reportArgs(TRUE, $shortfall);

  // notify other party by email about any completed transaction
  foreach ([$a1, $a2] as $a) if (!$a->confirmed) $a->setBit(B_CONFIRMED);
  $notice = $reallyTaking ? 'new charge'
    : (($amount < 0 and $goods == FOR_GOODS) ? 'new refund' : 'new payment');
  if ($notice == 'new payment' and $a2->co) { // not "new charge" (the most common); payment at payer's initiative
    $notice = 'new payment linked';
    $args['_aPayLink'] = $a2->makeDo('addr', $myid, 0); // link to address, so merchant can ship or nonprofit can thank
//      "<$atag>" . check_plain($args['myName']) . '</a>';
  }
//  if (@$args['otherRewardAmount'] and $a1->cttyRewardy) $notice .= '|reward other';
  r\notify($uid, $notice, $args);
  $msg = 'report tx';
/*  if ($a1->cttyRewardy) {
    $msg .= '|for why';
    if ($goods == FOR_GOODS) $msg .=  ($channel == TX_POS and $a1->co) ? '|reward customer' : '|the reward';
  } */
//  } else $msg = $ttype;

//  if (isPRODUCTION) r\tellAdmin('tx', compact('msg') + $args); // a backup record of all txs (until the backup server works)
  return r\txRet($msg, $args);
}

/**
 * Create an invoice
 * call by: list ($msg, $args) = be\invoice(...);
 * @param acct $a1: acct object for payee (usually current user)
 * @param acct $a2: acct object for payer
 * @param numeric $amount: the invoiced amount
 * @param string $purpose: description of the transaction's purpose (user input)
 * @param assoc $extra:
 *    goods: FOR_GOODS, etc.
 *    shid: record id of related record in r_shares
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    subs: replacement parameters, specific to the message (in particular, success=TRUE or FALSE)
 */
function invoice($a1, $a2, $amount, $purpose = NULL, $extra = []) {
  global $channel;
  
  u\EXPECT(compact(ray('a1 a2 amount purpose')), 'acct acct float string');
  extract(just('goods shid', $extra));
  list ($payee, $payer) = array($a1->id, $a2->id);
  u\setDft($goods, FOR_GOODS);
  u\setDft($purpose, $why = ray(R_WHYS)[$goods]);
  $subs = ray('myName otherName otherEmail otherPhone amount why purpose did op', $a1->fullName, $a2->fullName, $a2->email, u\fmtPhone($a2->phone), u\fmtAmt($amount), $why, $purpose, t('charged'), t('charged'));
  if ($err = u\badAmount($amount, '>0')) return r\txRet($err, $subs);
  if ($err = r\txPermErr($a1, $a2, TRUE)) return r\txRet($err, $subs);
  if (r\dupTx($payee, $payer, -$amount)) return r\txRet('duplicate transaction', $subs);
  
  $created = r\rTime();
  $status = ($channel != TX_FOREIGN and r\relation(':IS_AUTOPAY', $payee, $payer)) ? TX_APPROVED : TX_PENDING;
  if (@$shid) $data = serialize(compact('shid'));
  $info = compact(ray('amount status payer payee goods purpose created data'));
  $nvid = db\insert('r_invoices', $info);
  $a1->update('lastTx', [$nvid, $payer - $payee, round(-$amount, 2), $created, 'inv']);

  if ($status == TX_PENDING) {
    $link = $a1->makeDo('inv', $nvid); // link to pay/deny
    //$a2->update(ray('lastTx', array($nvid, $payee - $payer, round($amount, 2), $created, 'inv')));
    $subs += ray('_a1 reply email phone', $link, $payee, $a1->email, u\fmtPhone($a1->phone));
    r\message($payer, 'new invoice', $subs); // tell other party by email about this invoice
  } else { // pre-approved!
    if (!$a2->shortfall(FOR_GOODS, $amount)) { // don't try to pay yet if short
      $ok = be\payInvoice($nvid, $msg);
      if (!$ok) r\tellAdmin(t('invoice payment failed'), compact('msg') + $subs + $info);
    }
  }

  $msg = 'report tx';
//  if ($a1->cttyRewardy) $msg .= '|for why';
  if ($amount > 0) $msg .= '|balance unchanged';
  return array($msg, $subs + ['success'=>TRUE]);
}

/**
 * Send an email invoice to a nonmember, inviting them to join.
 * @param string $name: name of person to invoice
 * @param assoc $info: assoc [email, amount, goods, purpose] -- these params may also be specified individually
 */
function invoiceNonmember($name, $info) {
  extract(just('email amount goods purpose', rayy(func_get_args(), 1)));
  
}
  
/**
 * Record a transfer (of cgCredits) to or from the community.
 * @param int $uid: uid of the account to credit or debit
 * @param int $type: what type of transfer
 * @param int $amount: how much to transfer
 * @param string $for: purpose of funding
 * @param bool $oked: TRUE actually do it? (otherwise just see if it can be done)
 * @return int: the incentive amount (FALSE on failure)
 *//*
function fund($uid, $type, $amount, $for) {
  $payeeAgent = $payee = $uid;
  global $mya; // (null, if just registering or running as cron)
  $isReward = r\isReward($type);
  u\EXPECT($isReward or @$mya->can(B_MANAGING_CTTY), 'non-automatic reward by non ctty agent');
  if ($type == TX_FINE) $amount = -$amount;
//  if (!$oked) return $amount;
  if ($isReward) list ($payerReward, $payeeReward, $amount) = [0, $amount, 0];
///    debug(compact(ray('uid type amount for isReward payeeReward')));
  
  $a = r\acct($payee);
  $payer = $a->community;
  $payerAgent = (@$mya and $mya->id < 0) ? $mya->agentId : $payer; // take note if funded by community admin
  
  if (!new r\X(compact(ray('type amount payer payee payerReward payeeReward payerAgent payeeAgent for taking')))) return FALSE;
  $types = ray(TX_TYPES);
  $thing = $isReward ? t('reward') : $types[$type];
  $subs = ray('amount purpose thing', u\fmtAmt(abs($amount)), $for, $thing);
  $index = $type == TX_FINE ? 'got fine' : 'got funding';
  if ($type == TX_INFLATION) $index .= '|inflation details';
  if ($a->cttyRewardy or !$isReward) r\notify($uid, $index, $subs); // don't report rewards unless rewarding
  return $amount;
}
*/

/**
 * @return FALSE if success; else array($error_message, $subs)
 */
function addCell($number, $validate = FALSE) {
  global $mya;
  $myid = $mya->id;

  $numberInternal = u\fmtPhone($number, '+n');
  $numberPretty = u\fmtPhone($number);
  $subs = compact('number'); // original format

  if ($validate) {
    if (!$numberPretty) return array('bad phone', $subs);
    if ($result = db\q('SELECT * FROM r_boxes WHERE code=:numberInternal', compact('numberInternal'))->fetchAssoc()) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return array('already cell', $subs);
      $accountName = accountName($old_uid);
//      if (!isTempAccount($old_uid)) return array('cell taken', compact(ray('number accountName')));
    }
    return NULL;
  }
  
  //$status = SMS_PRIMARY;
  r\makeDevice($myid, $numberInternal, TX_SMS);
  if (!$mya->phone) $mya->update('phone', $numberInternal); // use it as contact phone, if none yet
  return array('report new cell', ray('number', $numberPretty));
}

function deleteCell($number) {
  $number = u\fmtPhone($number, '+n'); // internal format
  db\q('DELETE FROM r_boxes WHERE code=:number LIMIT 1', compact('number'));
  $number = u\fmtPhone($number); // pretty format
  return array('report delete cell', compact('number'));
}

/**
 * Figure out who the user meant. Only OK people in the user's community are eligible (except admin sees all).
 *
 * @param string $who: how the user referred to the trader (name, phone, email, uid, or temporary code)
 * @param int $myid: current user's account record ID
 * @param bool $paying: <we're preparing to pay this person> (in which case the person has to have an active account)
 * @param string $self_message: index to error message for identifying oneself
 *   If true, the other trader will be created only if specified by email or phone
 * @return one of the following:
 *   the other trader's acct
 *   array(error index, array of substitutions, array of possible fullNames (0 or more) indexed by uid)
 */
function identify($who, $myid, $paying, $self_message = 'no self-trading') {
  $who = trim($who);
  $subs = compact('who');
  if (!$mya = r\acct($myid)) return ['search forbidden', [], []];
  if ($phone = u\fmtPhone($who, '+n')) {
    $who = $phone; $cphone = u\cry('P', $phone);
  } else $cphone = '-';
  
  if (!$short = u\shortName(" $who ", '%')) $short = '-'; // abbreviations must not be null or zeroes
  $cmail = strpos($who, '@') ? u\cry('P', $who) : '-';

  $cttyCrit = $mya->admin 
  ? '1' 
  : (($paying ? ':IS_OK AND ' : '') . "(:IS_CO OR community=$mya->community)"); // restrict to own community and approved (as of 7/6/2017)
  
  $sql = <<<EOF
    SELECT DISTINCT uid,fullName
    FROM users 
    WHERE (name LIKE :short OR :who=fullName OR (!:IS_NOSEARCH AND (:cphone=phone OR :cmail=mail)))
    AND $cttyCrit
    ORDER BY fullName
EOF;

// LEFT JOIN r_boxes s ON u.uid = s.uid ... OR :who=s.code (search for cell not allowed if it isn't the account's primary phone)

  $result = db\q($sql, compact(ray('who short cphone cmail')))->fetchAllKeyed(0, 1);
  
  // QID? if not, maybe first word is a QID if this is from a list showing QID and fullName
  $qid = trim(($i = strpos($who, ':')) ? substr($who, 0, $i) : strtoupper($who));
  if ($a = r\acct($qid)) $result[$a->id] = $a->fullName; // explicit QID is a result, regardless of community
  if (count($result) == 1 and @$result[$myid]) return [$self_message, [], []];
  unset($result[$myid]); // self is one of many results, so ignore it
  if (!$result) return [($mya->admin ? 'unknown member' : 'unknown ctty member') . ("$cphone$cmail" == '--' ? '' : '|no search'), $subs, []];
  if (count($result) == 1) return r\acct(key($zot = $result)); // get the one entry (zot resets the pointer)
  return ['ambiguous other', $subs, $result];
}

/**
 * Return financial information about the given account on this server
 * @param assoc $args: options:
 *   uid: uid for the account to return information for (DEFAULTS to myid)
 *   pending: show pending transactions (applicable only to bank transfers)
 *   asof: last date to include (defaults to 24 hours from now, meaning unrestricted)
 *   noj: ignore any joined account
 * @return
 *   standard object of credit account information (see $resultKeys, below)
 *   including the overall demand for rCredits
 */
function creditInfo($args = []) {
  extract(just('uid pending asof noj', $args));
  if (!$a = r\acct(@$uid)) return NULL; else $uid = $a->id;
  $jid = @$noj ? 0 : ($a->jid ?: 0);

  u\setDft($asof, $asof0 = r\rTime() + DAY_SECS);
  $mday1 = u\monthDay1($asof); // start of relevant month, for partial (not-yet-given) roundups calculation
  if ($asof == u\plusMonths(1, $mday1) - 1) $mday1 = $asof + 1; // for asof the end of month, there are no partRounds
  $subs = compact(ray('uid jid asof'));
  $isPayer = 't.payer IN (:uid, :jid)';
  $isPayee = 't.payee IN (:uid, :jid)'; // account can be both payer and payee, if joint account
  $signedAmt = "IF($isPayer, -amount, amount)";
//    SUM(IF(t.type IN (:TX_REWARDS), amount, 0)) AS rewards,
  
  $sql = <<<EOF
    SELECT 
    SUM(IF($isPayee, amount, 0)) AS `in`,
    SUM(IF($isPayer, amount, 0)) AS `out`, 
    SUM(IF($isPayer AND :uid>0, payerReward, payeeReward)) AS rewards,
    SUM(IF(t.:IS_ROUNDUP AND $isPayer AND MOD(amount, 1)>0, 1-MOD(amount, 1), 0)) AS roundups,
    SUM(IF(t.:IS_ROUNDUP AND $isPayer AND MOD(amount, 1)>0 AND t.created>=$mday1, 1-MOD(amount, 1), 0)) AS partRounds,
    SUM(IF(t.:IS_ROUNDUPS AND $isPayer, amount, 0)) AS roundupsPaid,
    SUM(IF(t.goods=:FOR_SHARE, $signedAmt, 0)) AS share,
    SUM(IF(t.type=:TX_XFEE, $signedAmt, 0)) AS xfees,
    SUM(IF(:HAS_XFEE, $signedAmt, 0)) AS xin
    FROM r_txs t
    WHERE ($isPayer OR $isPayee) AND t.created<=:asof
EOF;
  
  $sums = @$pending ? ray('in out rewards share xfees xin', 0, 0, 0, 0, 0, 0) : db\q($sql, $subs)->fetchAssoc();

  $where = @$pending ? 'created<=:asof AND NOT completed' : 'completed AND completed<=:asof';
  $sql = "SELECT SUM(amount) AS bank FROM r_usd t WHERE $isPayee AND $where";
  $sums += db\q($sql, $subs)->fetchAssoc();

  foreach ($sums as $k => $v) $sums[$k] = $v + 0; // no nulls
  extract($sums);

//  $bank = -$bank; // make it "from bank" instead of "to bank"
  $balance = ($in - $out) + $bank;
//  $r = $balance + $rewards;
//  $in -= $xin; $out -= $xin;
  $outProper = $out;
  $inProper = $in; // - $rewards;
  if ($uid == r\cgfId()) $inProper -= $share; else $outProper += $share;
  $rewards += $share; // share is negative for everyone but CGF
  
  // figure savings
  if ($asof == $asof0) {
//    $savings = $a->j_savings;
  } else {
//    $savings = $rewards; // tentative savings before additions ( = oldSavings - oldSavingsAdd)
    foreach ($jid ? [$a, r\acct($jid)] : [$a] as $ai) if ($chs = $ai->changes) {
      $addOLD = $ai->savingsAdd; // start with present
      foreach ($chs as $dt => $info) if ($dt > $asof) $addOLD = @$info['savingsAdd']; else break;
//      $savings += $addOLD;
    }
  }
//  $balance = $r - $savings;
  $savings = $rewards; // tentative savings before additions ( = oldSavings - oldSavingsAdd) (used in transaction summary)
  
  $resultKeys = 'in out rewards share refunds fees xfees xin inProper outProper balance r bank savings roundups partRounds roundupsPaid';
  $result = compact(ray($resultKeys));
  
  foreach ($result as $k => $v) $result[$k] = $v = round($v, 2);

  return (object) $result;
}

/**
 * Return a list of account choices for the current user (agent of the current account)
 * @return an assoc of names keyed by account record IDs (FALSE if none)
 */
function accountChoices() {
  if (!$mya = r\acct()) return FALSE;
  $result[$aid = $mya->agentId] = $mya->agentA->fullName;
  $nameOrder = "IF(MID(fullName, 2, 2)='. ', MID(fullName, 4), IF(MID(fullName, 2, 1)=' ', MID(fullName, 3), IF(MID(fullName, 1, 4)='the ', MID(fullName, 5), fullName)))"; // ignore "the ", "a ", and initialled first name (eg "X." or "X")
  $sqlNormal = 'SELECT r.main AS id, u.fullname FROM r_relations r LEFT JOIN users u ON u.uid=r.main WHERE r.other=:aid AND r.permission>0'; // normal selection SQL

  if ($mya->admin or $mya->isAdmin2) {
    $where = $mya->superAdmin ? '1' : 'uid NOT IN (0,1,2,3)';
    $q = @db\q("SELECT uid AS id, fullName FROM users WHERE $where ORDER BY (uid<0) DESC, $nameOrder");
  } elseif ($mya->cAdmin) {
    $sql = <<<EOF
SELECT DISTINCT id, fullName FROM
  (SELECT uid AS id, fullName FROM users WHERE community=:ctty UNION $sqlNormal) w
ORDER BY (id<0) DESC, $nameOrder
EOF;
    $q = @db\q($sql, ray('ctty aid', $mya->community, $aid));
  } else $q = @db\q("$sqlNormal ORDER BY $nameOrder", compact('aid')); // the usual case

  return $result + $q->fetchAllKeyed();
}

/**
 * Remember what to do, once the user confirms.
 *
 * @parm string $number: the user's cell number (in standard format +1dddddddddd)
 * @parm string $todo: what to do once the user responds with the expected nonce (defaults to 'nothing')
 * @parm string $nonce: what to store as the nonce (defaults to 'whatever', meaning pick one at random)
 *
 * @return object(nonce, todo) if $todo is 'nothing', else the nonce
 */
function todo($number, $todo = 'nothing', $nonce = 'whatever') {
  if ($todo == 'nothing') {
    return (object) db\get('nonce,todo', 'r_boxes', 'code=:number', compact('number'));
  } else {
    $nonce = $nonce == 'whatever' ? u\nonce() : $nonce;
    \db_update('r_boxes')->fields(compact('nonce', 'todo'))->condition('code', $number)->execute();
    u\EXPECT(todo($number)->nonce == $nonce, 'setting nonce failed');
    return $nonce;
  }
}

function accountName($uid) {
  $acct = r\acct($uid);
  return $acct->fullName . ' ' . u\hug($acct->mainQid);
}
