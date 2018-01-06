<?php
namespace rCredits\Risk; // typically abbreviated as "k"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;

/**
 * @file
 * Calculate risks of financial crime
 */

/*
 * Calculate a risk score for the given account.
 * @param int $uid: account record ID
 * @param numeric $risk: (RETURNED) the normalized risk score
 * @param numeric $calc: (RETURNED) the original calculated risk
 */ 
function acctRisk1($uid, &$risk = 0, &$calc = 0) {
  global $_risk; $_risk = '';
  
  $a = r\acct($uid);
  global $_qid; $_qid = $a->mainQid; // for debugging
///  if ($a->mainQid != 'NEW.ZZA') return; // debug(
  $idSub = ray('id', $a->id);
  
  setrisk('adminOk', $a); // administrator blessed this account
//  if ($a->community <> r\serverUid()) setRisk('geography'); // HIDTA (Washtenaw cty, MI)
  if (!$a->cttyA or $a->cttyA->risk('geography')) setRisk('geography'); // HIDTA (eg Washtenaw cty, MI) (!$a->cttyA is for tests)
  // criminal
  setRisk('cashCo', $a); // high-risk company type according to regs
  $tenure = $a->tenure / 12 + (r\rTime() - $a->created) / YEAR_SECS; // years of residence ($a->tenure is in months)
  setRisk('new', $tenure ? .5 / ($tenure * $tenure) : 99999, $tenure < 1); // half a year is the standard
  setRisk('moves', $a->moves, $a->moves > 0); // moves over past decade
  setRisk('rents', $a);
  
  $trusted = $a->trust - 1; // how well trusted (trust is always >=1)
  $inviteTrust = db\lookup('SUM(trust - 1)', 'users u INNER JOIN r_invites i ON i.inviter=u.uid', 'i.invitee=:id OR i.email=:email', ray('id email', $a->id, $a->mail)); // trustedness of accounts that invited this one
  $trusted += $inviteTrust; // invited by trusted people
  $relations = db\count('r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>0 AND NOT r.isOwner AND r.other=:id AND u.risk<:K_GREEN', $idSub);
  $trusted += $relations; // access to (trusted by) non-suspect businesses
  setRisk('trusted', $trusted, $trusted > 1.5);
  
  $conxRisk = db\count('users u INNER JOIN r_invites i ON i.invitee=u.uid', "i.inviter=:id AND u.risk>=:K_RED", $idSub); // invited by this account
  $relationsRisk = db\count('r_relations r LEFT JOIN users u ON u.uid=r.main', 'r.permission>0 AND r.other=:id AND u.risk>=:K_RED', $idSub); // access to (trusted by) suspect businesses
  setRisk('badConx', $conxRisk + $relationsRisk, 'ifany'); // invited suspicious accounts

  setRisk('socialConx', $a->mediaConx, 'ifany'); // 1st or 2nd-degree social media connections to administrator

//  if ($a->co and db\exists('users', 'uid<>:id AND address=:address AND zip=:zip', ray('id address zip', $a->id, $a->address, $a->zip))) setRisk('homeCo'); // company shares physical address
  if ($a->co and $a->dupAddr()) setRisk('homeCo'); // company shares physical address (encryption requires func)
  setRisk('shady', $a); // no substantial internet search results and/or no SSN history
//  if ($a->can(B_BANK)) setRisk('hasBank'); // has a connected bank account
  setRisk('hasBank', $a); // has a connected bank account
  if ($uid != r\cgfId() and db\lookup('SUM(amount*IF(often=1,1,12))', 'r_gifts', 'share>=0 AND uid=:id', $idSub) + 0 < K_MISER_DONATION and !$a->roundup and !$a->crumbs) setRisk('miser'); // unusually small donation to CGF
  //  and $a->share < K_MISER_SHARE
  
  setRisk('photoOff', $a); // photo mismatches ID or DOB
  setRisk('addrOff', $a); // addr mismatches ID
  if (strpos($a->postalAddr, $a->address) === FALSE or strpos($a->postalAddr, 'box') !== FALSE) setRisk('poBox'); // separate postal address
  setRisk('ssnOff', $a); // weird, conflicting, or unknown SSN history
  setRisk('dobOff', $a); // birth date unverifiable in SSN check
    // activity
  setRisk('fishy', $a); // something else fishy
  setBigRisks($uid);

  extract(setRisk());
  $calc = $risk;
  $risk *= K_RED; // normalize
///  u\deb(compact('risk','risks'));
//  if ($a->mainQid == R_ADMIN_QID) $risk = 0; // otherwise sketchy invites skew all the calculations
  $a->update(compact('risk','risks'));
}

/*
 * Calculate a risk score for the given transaction.
 * @param assoc $tx: the transaction array
 * @param numeric $risk: (RETURNED) the normalized risk score
 * @param numeric $calc: (RETURNED) the original calculated risk
 */ 
function txRisk1($tx, &$risk = 0, &$calc = 0) {
  global $_risk; $_risk = '';
  extract($tx); // all tx fields
  global $_xid; $_xid = $xid; // for debugging
  extract(just('force undoneBy undoes xfee', unserialize($data)));

  $cttyPayer = ($payer < 0);
  if ($neg = ($amount < 0)) list ($payer, $payee, $amount) = array($payee, $payer, -$amount);
  list ($a1, $a2) = array(r\acct($payer), r\acct($payee));
  
  if ($risks & u\bit(K_ADMINOK)) setRisk('txAdminOk'); // administrator blessed this transaction
  if (@$undoes and $undoneData = @unserialize(db\lookup('data', 'r_txs', 'xid=:undoes', compact('undoes')))) {
    if (@$undoneData['undoes']) setRisk('redo');
  }
  
  if (!$cttyPayer) { // most transfers to/from the community should be ignored
    if ($goods != FOR_GOODS) {
      if ($neg ? $a1->co : $a2->co) {
        setRisk((!$neg and $channel == TX_POS and r\usdin($payeeFor)) ? (@$xfee ? 'bankIn' : 'cashIn') : 'cashOut');
      } else setRisk('exchange');
      if ($a1->risk >= K_RED) setRisk('suspectOut');
    } else {
      if ($b2p = ($a1->co and !$a2->co and !r\isEmployee($payee, $payer))) setRisk('b2p'); // company payment to non-employee individual
    }
    $samePostal = ($a1->postalAddr == $a2->postalAddr);
    $withCo = ($a1->co or $a2->co); // at least one party is a company
    if (!$withCo and !$samePostal) setRisk('p2p'); // transaction between individuals at different postal addresses
    if ($withCo and (r\owning($payer, $payee) or $samePostal or r\isEmployee($payer, $payee))) setRisk('inhouse'); // transaction with owner or family member same address or payment by employee

    $since = $created - WEEK7_SECS; // just recent activity by customers, in case many are new
    $count = db\count('r_txs', 'payer=:payer AND payee=:payee AND created BETWEEN :since AND :created', compact(ray('payer payee since created')));
    setBigToFrom('From', 'payer', $payer, $amount, $created, $count);
    setBigToFrom('To', 'payee', $payee, $amount, $created, $count);

    setOriginRisk($amount < 0 ? $payee : $payer, $created, $amount);

    if ($channel == TX_WEB or $channel == TX_SMS) {
      setRisk('absent'); // transaction by web or SMS (no in person)
      if (strpos($payeeFor.$payerFor, 'inv#') === FALSE and strpos($payeeFor.$payerFor, 'invoice') === FALSE and !r\isEmployee($payee, $payer)) setRisk('invoiceless'); // payment to non-employee without an invoice
    }
  }

  setRisk('fromSuspect', $a1->risk / K_RED, $a1->risk >= K_RED); // payment from suspicious account
  setRisk('toSuspect', $a2->risk / K_RED, $a2->risk >= K_RED); // payment to suspicious account
  
  if (@$force) { // offline
    setRisk('offline');
    if (!db\exists('r_txs', 'payer=:payer AND payee=:payee AND xid<>:xid', compact('payer', 'payee', 'xid'))) setRisk('firstOffline'); // offline for first payment to this company
  }
  
  extract(setRisk());
  $calc = $risk;
  if ($risk > 0) $risk *= $amount; // (don't exaggerate negative risk)
  $risk *= K_RED / K_THRESHOLD; // normalize
  db\update('r_txs', compact('xid', 'risk', 'risks'), 'xid');
}

/*
 * Calculate a risk score for the given ACH.
 * @param assoc $tx: the ACH transaction array
 * @param numeric $risk: (RETURNED) the normalized risk score
 * @param numeric $calc: (RETURNED) the original calculated risk
 */ 
function achRisk1($tx, &$risk = 0, &$calc = 0) {
  global $_risk; $_risk = '';

  extract($tx); // all tx fields
  global $_txid; $_txid = $txid; // for debugging
  
  $a = r\acct($payee);
  if ($amount < 0) setRisk($amount < 0 ? 'toBank' : 'fromBank'); // mark only transfers OUT
  setRisk($amount < 0 ? 'suspectOut' : 'toSuspect', $a->risk / K_RED, $a->risk >= K_RED);
  
  if ($amount < 0) setOriginRisk($payee, $created, $amount);
  
  extract(setRisk());
  $calc = $risk;
  if ($risk > 0) $risk *= abs($amount); // (don't exaggerate negative risk)
  $risk *= K_RED / K_THRESHOLD; // normalize
  db\update('r_usd', compact('txid', 'risk', 'risks'), 'txid');
}


/**
 * Update the risk variables.
 * call by:
 *   setrisk(bitName, multiplier, setBit); OR
 *   setrisk(bitName, $a);
 * @param string $bitName: what risk type -- allows lookup of risk weight and bit number. 
 *   UNUSED: (if risk weight is non-numeric, it does not contribute to the risk score)
 * @param float $multiplier: what to multiply inverted weight by, before adding to risk score
 * @param bool $setBit: UNUSED <set the named bit in $_risks> ("ifany" means set the bit if $multiplier nonzero
 * @param acct $a: add the standard risk score for this risk type, if the bit is already set in this acct
 * @return assoc [risk, risks] if called with no params
 */
function setRisk($bitName = NULL, $multiplier = 1, $zot = '') {
  global $showRiskCalc;
  global $_risk, $_risks; // use _ to distinguish from extracted risk and risks fields (not needed, but clearer)
  global $_acctRisks, $_txRisks, $_acctRiskBits, $_txRiskBits;
/// log("bitName=$bitName multiplier=$multiplier _risk=$_risk _risks=$_risks"); // debug()
  
  if (@$_risk === '') {
    $_risk = $_risks = 0;
    $_acctRisks = ray(K_ACCT_RISKS);
    $_txRisks = ray(K_TX_RISKS);
    $_acctRiskBits = array_flip(array_keys($_acctRisks));
    $_txRiskBits = array_flip(array_keys($_txRisks));
  }
  if (is_null($bitName)) return ray('risk risks', $_risk, $_risks); // must come after $_risks=0
  
  if (is_numeric($bit = @$_acctRiskBits[$bitName])) {
    $weight = $_acctRisks[$bitName];
  } else {$bit = $_txRiskBits[$bitName]; $weight = $_txRisks[$bitName];}

//  if (is_object($a = $multiplier)) list ($multiplier, $setBit) = array($a->risk($bit) ? 1 : 0, 'ifany');
  if (is_object($a = $multiplier)) $multiplier = $a->risk($bit) ? 1 : 0;
//  if (is_numeric($weight) and $weight) {
  $_risk += ($risk = $multiplier / $weight);
//  if ($setBit == 'ifany' ? ($multiplier != 0) : $setBit) u\setBit($_risks, $bit);
  $setBit = (abs($risk) >= K_BIT_MIN and $multiplier > 0) ? t('yes') : ''; // don't show bit if risk is reversed
  if ($setBit or (is_object($a) and $multiplier > 0)) u\setBit($_risks, $bit);
//  }

  if (isset($showRiskCalc) and $risk) {
    $class = $weight < 0 ? 'goodRisk' : 'badRisk';
    $showRiskCalc .= <<< EOF
<tr>
  <td class="$class">$bitName</td>
  <td class="weight">[$weight]</td>
  <td class="multiplier">$multiplier</td>
  <td class="risk">$risk</td>
  <td class="set">$setBit</td>
</tr>
EOF;
  }

///  global $_xid;  if ($_xid==1) debug(compact(ray('_xid bitName multiplier setBit bit weight _risk _risks')));
}

/**
 * Set one of the "big" risk bits and/or the "more" bits.
 * @param int $uid: the account record ID
 * @param int $ago: how many seconds ago to start the period to be analyzed
 * @param int $threshold: max total dollar amount for an account's activity before it is flagged as suspicious
 * @param string $bitName: the bit to set
 */
function setBigRisk($uid, $ago, $threshold, $bitName) {
  global $bigYearIn, $bigYearOut;
  $now = r\rTime();
  $since = $now - $ago - 2 * HOUR_SECS; // an extra couple hours to handle daylight time changes and cron timing
  $mySql =<<<EOF
    SELECT COUNT(*) AS count0, 
      SUM(IF(:DEPOSIT_SQL,amount,0)) AS `in`, 
      SUM(IF(:DEPOSIT_SQL,0,-amount)) AS `out`
    FROM (SELECT txid,amount,created FROM r_usd 
      WHERE payee=:uid AND completed>=created
    UNION SELECT 0 AS txid,IF(payee=:uid, amount, -amount) AS amount,created FROM r_txs 
       WHERE payer=:uid OR payee=:uid) t
    WHERE created BETWEEN :since AND :now
EOF;
  if (!$res = db\q($mySql, compact(ray('uid since now')))->fetchAssoc()) return;
  extract($res);
  $a = r\acct($uid);
  if (max($in, $out) > $threshold * max(1, $a->employees)) setRisk($bitName);
  if ($ago == YEAR_SECS) list ($bigYearIn, $bigYearOut) = array($in, $out);
  if ($ago == WEEK7_SECS) {
    $firstTxDate = db\lookup('MIN(created)', 'r_txs', ':uid IN (payer, payee)', compact('uid')) ?: $now;
    if ($yearLen = min(YEAR_SECS, $now - $firstTxDate)) {
      if ($in > (1 + K_UNUSUAL_BUMP / 100) * $bigYearIn * $ago / $yearLen) setRisk('moreIn');
      if ($out > (1 + K_UNUSUAL_BUMP / 100) * $bigYearOut * $ago / $yearLen) setRisk('moreOut');
    }
  }
}

/**
 * Set the various "big" risk bits as appropriate.
 * @param int $uid: the account record ID
 */
function setBigRisks($uid) {
  setBigRisk($uid, YEAR_SECS, K_YEAR_THRESHOLD, 'bigYear'); // must precede big7Week
  setBigRisk($uid, WEEK7_SECS, K_7WEEK_THRESHOLD, 'big7Week');
  setBigRisk($uid, WEEK_SECS, K_WEEK_THRESHOLD, 'bigWeek');
  setBigRisk($uid, DAY_SECS, K_DAY_THRESHOLD, 'bigDay');
}

/**
 * Set risks according to an accounts standard transaction size.
 * @param string $way: To or From (for risk names)
 * @param string $field: name of field to examine (payer or payee)
 * @param string $uid: value of that field
 * @param numeric $amount: transaction amount
 * @param int $created: transaction creation date
 * @param int $count: number of transactions between payer and payee in the past 7 weeks
 */
function setBigToFrom($way, $field, $uid, $amount, $created, $count) {
  $since = $created - WEEK7_SECS; // just recent activity by customers, in case many are new
  $other = $field == 'payer' ? 'payee' : 'payer';
  
  $subtotals =<<<EOF
    SELECT AVG(ABS(amount)) as avg0, MAX(ABS(amount)) AS max0, COUNT(*) AS count0 
    FROM r_txs 
    WHERE payer>0 AND IF(amount<0,$other,$field)=:uid AND created>:since 
    GROUP BY IF(amount<0,$field,$other)
EOF;
  $sql = <<< EOF
    SELECT SUM(IF(:amount<avg0, 1, 0)) AS less, 
    MAX(max0) AS maxAmt, 
    SUM(IF(count0<:count, 1, 0)) AS fewer, COUNT(*) AS others 
    FROM ($subtotals) s
EOF;
  if ($res = db\q($sql, compact(ray('amount count since uid')))->fetchAssoc()) {
    extract($res); // average amount and number of purchases per other
    if ($less / $others > .90) setRisk("big$way"); // unusually big for this payer/payee (top decile)
    if (abs($amount) >= $maxAmt) setRisk("biggest$way"); // biggest ever for this payer/payee
    if ($fewer / $others > .90) setRisk("often$way"); // unusually frequent for this payer/payee
  }
}

/**
 * Set the "origins" risk bit in the current transaction or outgoing ACH, if appropriate.
 * @param int $uid: the account record ID
 * @param int $created: transaction date
 * @param currency $amount: transaction amount
 */
function setOriginRisk($uid, $created, $amount) {
  $since = $created - YEAR_SECS;
  $table = 'SELECT IF(amount<0,payer,payee) AS uid,risk,created,amount FROM r_txs';
  $where = 'uid=:uid AND risk IS NOT NULL AND created BETWEEN :since AND :created';
  $avgRisk = db\lookup('AVG(risk/ABS(amount))', "($table) t", $where, compact(ray('uid since created'))) / (K_RED / K_THRESHOLD);
///  debug(compact('uid','amount','since','table','where','avgRisk'));
  if ($avgRisk > 0) setRisk('origins', $multiplier = $avgRisk, $multiplier > 1/2);
}
