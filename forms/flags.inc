<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show accounts and recent transactions and ACHs that carry risk of fraud.
 */
function formFlags($form, &$sta, $args = '') {
  global $mya;
  $MAXLINES = 20; // max number of lines to show in each section
  include R_ROOT . '/rweb/risk-descs.inc';

// fails  u\setDft($args['period'], 1);
  extract(just('period starting ending download downloaded', $args)); 
  if (!@$period) $args['period'] = $period = 7;
  extract(just('dateRange url starting ending', dateRange($args, 'community/flags', [], '', FALSE)));
  $url = hidFld($url);
  
  $title = t('Suspicious Activity Report'); //  . strftime('%A, %m/%d/%y')
  $subtext = t('<p>Any potentially suspicious accounts, transactions, and bank transfers (ACHs) are listed here, so all members can keep an eye on things. System administrators will investigate any transaction with a suspiciousness score of %K_RED or more (shown in red). If you don\'t see any red, that means nothing even remotely suspicious happened. But for the sake of transparency, we show the %MAXLINES top scores in each category anyway.</p><p>Green risks are "good" (they mitigate other risk factors). Hover over any risk for a brief description or click it for details. To see a company\'s name, hover over its account ID.</p>', compact('MAXLINES'));

  $achHeader = $txHeader = $acctHeader = '';
  $accts = $txs = $achs = '';
  $headers = ray(t('Score Account Type Created Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $acctHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $acctHeader = "<tr>$acctHeader</tr>";
  list ($cttyCrit, $cttysCrit) = (!$mya or $mya->admin)? [1, 1] 
    : ["community=$mya->community", "$mya->community IN (u1.community, u2.community)"];
  $sql = <<<EOF
    SELECT risk AS score,uid,IF(:IS_CO,'Co','Indiv') as type,created,risks FROM users 
    WHERE uid>0 AND :IS_OK AND $cttyCrit
    ORDER BY risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql);
  for ($i = 0; $line = riskLine($q, K_ACCT_RISKS, $red) and ($i < K_LINES); $i++) $accts .= $line;
  
  $headers = ray(t('Score Tx# Date Amount From To Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $txHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $txHeader = "<tr>$txHeader</tr>";
  $subs = compact('starting', 'ending');
  $sql = <<<EOF
    SELECT t.risk AS score,xid,t.created,amount,payer,payee,payeeFor AS description,t.risks 
    FROM r_txs t LEFT JOIN users u1 ON u1.uid=payer LEFT JOIN users u2 ON u2.uid=payee 
    WHERE (payer>0 OR ABS(amount)>:K_CTTY_THRESHOLD) 
    AND $cttysCrit AND t.created BETWEEN :starting AND :ending 
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $txs .= $line;

  $headers = ray(t('Score Tx# Date Amount Account Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $achHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $achHeader = "<tr>$achHeader</tr>";
  $sql = <<<EOF
    SELECT t.risk AS score,txid,t.created,ABS(t.amount) AS amount,payee,IF(t.amount>0,'in','out') AS way,t.risks 
    FROM r_usd t LEFT JOIN users u ON u.uid=payee
    WHERE t.created BETWEEN :starting AND :ending AND $cttyCrit
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $achs .= $line;
  
  $table = item(<<<EOF
  <center>
  <h3>ACCOUNTS</h3>
  <table id="accts">
  $acctHeader
  $accts
  </table>
  <h3>TRANSACTIONS</h3>
  <table id="txs">
  $txHeader
  $txs
  </table>
  <h3>ACHs</h3>
  <table id="achs">
  $achHeader
  $achs
  </table>
  </center>
EOF
  );

  if (@$downloaded) jsx('download', compact('url'));
  return cgform(compact('title') + $dateRange + compact('subtext', 'table'));
}

function formFlags_validate($form, &$sta) {w\dateRangeValidate($sta);}
function formFlags_submit($form, &$sta) {dateRangeRedirect('community/flags', $sta);}


/**
 * Return a formatted line for the next record, describing its risks.
 * @param SQLresult $q: the query result
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @param bool $red: (RETURNED) the
 * @return an HTML table row to show the record
 */
function riskLine($q, $descs, &$red = '') {
  global $riskUids; if (!@$riskUids) $riskUids = [];
  global $base_url; 
  $abridgable = ray(t('labor,automatic transfer,donation,deferred'));
  if (!$row = $q->fetchAssoc()) return FALSE;
  
  if (@$row['amount'] + 0 < 0) list ($row['payer'], $row['payee'], $row['amount']) = array(@$row['payee'], @$row['payer'], -$row['amount']); // don't show user negative amounts
  extract(just('uid xid created description score amount risks', $row));

  $extra = (($mya = r\acct()) and $mya->can(B_ADMIN2));
  global $riskQ, $riskCount; 
  if ($q == @$riskQ) {
    $riskCount++;
    if ($riskCount > K_LINES) return FALSE;
  } else {$riskQ = $q; $riskCount = 1;}
  
  u\setDft($score, '');
  $score = round($score) + 0; // +0 to avoid "-0"
  $red = ($score >= K_RED);
  if ($red and $descs != K_ACCT_RISKS) $score = "<span class=\"risky\">$score</span>"; // don't redden accounts
  if ($score <= K_GREEN) $score = "<span class=\"safe\">$score</span>";
  $row['score'] = $score;
  
  foreach (just('uid payer payee', $row) as $k => $v) {
    $qo = r\qo($v);
    $qid = $qo->qid;
    $a = r\acct($v);
    if ($extra) {
      list ($region, $local) = $qo->parse();
      $v = w\popHelp($v < 0 ? t('ctty') : $local, $a->fullName, "$base_url/summary/$qid");
    } else if ($v < 0) {
      $v = t('ctty');
    } else {
      $i = array_search($qid, $riskUids);
      if ($i === FALSE) {$i = count($riskUids); $riskUids[] = $qid;}
      $local = u\n2a($i * 5, 2);
      $v = $a->co ? w\popHelp($local, $a->fullName) : $local;
    }
    $row[$k] = $v;
  }
  
  if (isset($amount)) $row['amount'] = number_format($amount, 2);
  if ($extra and isset($xid)) $row['xid'] = "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";
  if (isset($created)) $row['created'] = strftime(@$uid ? '%d-%b-%Y' : '%d-%b', $created);
  if (isset($description)) foreach ($abridgable as $k) if (u\starts($description, $k)) $row['description'] = $k;
  $row['risks'] = r\riskList($risks, $descs);
  
  $line = '';
  foreach ($row as $k => $v) $line .= "<td class=\"k-$k\">$v</td>";
  return "<tr>$line</tr>\n";
}