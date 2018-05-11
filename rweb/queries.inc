<?php

use CG as r;
use CG\Util as u;

/**
 * @file
 * List of queries for the Community and Money Data page on the Community menu (for members only).
 * Queries beginning with a digit are for admins or companies only: 0=superAdmin 1=admin 2=cAdmin or cAdmin2 B_CO=companies
 * These special parameters are recognized:
 *  :ctty  the current account's community ID
 *  :myid  the current account's record ID
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url).
 */

$cgf = r\cgfId();
$bizQ = 'SELECT payee, goods, SUM(amount) AS tot, COUNT(*) AS cnt FROM r_txs WHERE goods=:FOR_GOODS AND created BETWEEN';

$queries = [
  2 . 'Most Trusted Members' => 
    "SELECT fullName, mail AS email, phone, COUNT(b.voter) AS proxiedFor, trust FROM users u LEFT JOIN r_ballots b ON b.proxy=u.uid AND b.created>:REQUEST_TIME-:YEAR_SECS WHERE u.community=:ctty AND u.:IS_OK AND NOT u.:IS_CO GROUP BY IFNULL(b.proxy, u.uid) ORDER BY COUNT(b.voter) DESC, u.trust DESC",

  B_CO . 'Monthly Business Income (30 day totals)' => 
    "SELECT LEFT(u.fullName, 42) AS company, 
    t0.tot AS `total $`, t0.cnt AS `count`,
    t6.tot AS `total $ 6mos ago`, t6.cnt AS `count6`,
    t12.tot AS `total $ 12mos ago`, t12.cnt AS `count12`
    FROM users u 
    LEFT JOIN ($bizQ :NOW-30*:DAY_SECS AND :NOW GROUP BY payee) t0 ON t0.payee=u.uid
    LEFT JOIN ($bizQ :NOW-(180+30)*:DAY_SECS AND :NOW-180*:DAY_SECS GROUP BY payee) t6 ON t6.payee=u.uid
    LEFT JOIN ($bizQ :NOW-:YEAR_SECS-30*:DAY_SECS AND :NOW-:YEAR_SECS GROUP BY payee) t12 ON t12.payee=u.uid
    WHERE u.community=:ctty AND :IS_CO AND :IS_OK AND t0.cnt>0 AND fullName NOT LIKE '%457%' ORDER BY u.fullName",

  'Positive and Negative Balance Totals' => 
    "SELECT c.fullName AS community, 
    SUM(IF(u.balance<0, 1, 0)) AS negativeBalanceCount, SUM(IF(u.balance<0, u.balance, 0)) AS negativeBalanceTotal,
    SUM(IF(u.balance>=0, 1, 0)) AS positiveBalanceCount, SUM(IF(u.balance>=0, u.balance, 0)) AS positiveBalanceTotal
    FROM users u LEFT JOIN users c ON c.uid=u.community WHERE u.:IS_OK GROUP BY c.fullName",
  
  'Company and Member Balances and Credit Lines' =>
    "SELECT c.fullName AS community,
    SUM(IF(u.:IS_CO, 0, u.balance)) AS memBalances, SUM(IF(u.:IS_CO, 0, -u.floor)) AS memCredit, 
      SUM(IF(u.:IS_CO, 0, IF(u.:IS_REFILL, u.minimum, 0))) AS memTargetBalances,
    SUM(IF(u.:IS_CO, u.balance, 0)) AS coBalances, SUM(IF(u.:IS_CO, -u.floor, 0)) AS coCredit, 
      SUM(IF(u.:IS_CO, IF(u.:IS_REFILL, u.minimum, 0), 0)) AS coTargetBalances 
    FROM users u LEFT JOIN users c ON c.uid=u.community WHERE c.uid<0 GROUP BY c.fullName",

//     SUM(IF(t.:IS_GIFT OR t.:IS_ROUNDUPS OR t.:IS_CRUMBS OR t.:IS_PATRONAGE,0,amount)) AS `payments to CG`
  'Donations to Common Good and Community' =>
    "SELECT CONCAT_WS(' ', YEAR(FROM_UNIXTIME(t.created)), MONTHNAME(FROM_UNIXTIME(t.created))) AS month,
    SUM(IF(t.:IS_ROUNDUPS,amount,0)) AS roundups,
    SUM(IF(t.:IS_CRUMBS,amount,0)) AS crumbs,
    SUM(IF(t.:IS_PATRONAGE,amount,0)) AS recurring,
    SUM(IF(t.:IS_GIFT AND NOT t.:IS_ROUNDUPS AND NOT t.:IS_CRUMBS AND NOT t.:IS_PATRONAGE,amount,0)) AS other
    FROM r_txs t LEFT JOIN users u ON u.uid=t.payer
    WHERE u.community=:ctty AND (t.:IS_GIFT OR t.:IS_ROUNDUPS OR t.:IS_CRUMBS OR t.:IS_PATRONAGE) AND t.payee=$cgf
    GROUP BY YEAR(FROM_UNIXTIME(t.created)) DESC, MONTH(FROM_UNIXTIME(t.created)) DESC",

];

//    IFNULL(u.fullName, REPLACE(SUBSTRING(r.data, 27, LOCATE('\"', SUBSTRING(r.data, 28))), '\"', '')) AS name,
//    SUBSTRING_INDEX(SUBSTRING_INDEX(r.data, '\"', 12), '\"', -1) AS phone,

