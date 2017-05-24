<?php

/**
 * @file
 * List of queries for the Community and Money Data page on the Community menu.
 * Queries beginning with a digit are for admins only: 0=superAdmin 1=admin 2=cAdmin or cAdmin2 B_CO=companies
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url)
 */

$bizQ = 'SELECT payee, goods, SUM(amount) AS tot, COUNT(*) AS cnt FROM r_txs WHERE goods=:FOR_GOODS AND created BETWEEN';

$queries = [
  '2Most Trusted Members' => 'SELECT fullName, mail AS email, phone, COUNT(b.voter) AS proxiedFor, trust FROM users u LEFT JOIN r_ballots b ON b.proxy=u.uid AND b.created>:REQUEST_TIME-:YEAR_SECS WHERE 1 AND u.:IS_OK AND NOT u.:IS_CO GROUP BY IFNULL(b.proxy, u.uid) ORDER BY COUNT(b.voter) DESC, u.trust DESC',

  B_CO . 'Monthly Business Income (30 day totals)' => "SELECT LEFT(u.fullName, 42) AS company, 
  t0.tot AS `total $`, t0.cnt AS `count`,
  t6.tot AS `total $ 6mos ago`, t6.cnt AS `count6`,
  t12.tot AS `total $ 12mos ago`, t12.cnt AS `count12`
  FROM users u 
  LEFT JOIN ($bizQ :NOW-30*:DAY_SECS AND :NOW GROUP BY payee) t0 ON t0.payee=u.uid
  LEFT JOIN ($bizQ :NOW-(180+30)*:DAY_SECS AND :NOW-180*:DAY_SECS GROUP BY payee) t6 ON t6.payee=u.uid
  LEFT JOIN ($bizQ :NOW-:YEAR_SECS-30*:DAY_SECS AND :NOW-:YEAR_SECS GROUP BY payee) t12 ON t12.payee=u.uid
  WHERE 1 AND :IS_CO AND :IS_OK AND t0.cnt>0 AND fullName NOT LIKE '%457%' ORDER BY u.fullName",

  'Positive and Negative Balance Totals' => 'SELECT c.fullName AS community, 
  SUM(IF(u.r-u.rewards<0, 1, 0)) AS negativeBalanceCount, SUM(IF(u.r-u.rewards<0, u.r-u.rewards, 0)) AS negativeBalanceTotal,
  SUM(IF(u.r-u.rewards>=0, 1, 0)) AS positiveBalanceCount, SUM(IF(u.r-u.rewards>=0, u.r-u.rewards, 0)) AS positiveBalanceTotal
  FROM users u LEFT JOIN users c ON c.uid=u.community WHERE u.:IS_OK GROUP BY c.fullName',
  
  'Company and Member Balances and Credit Lines' => <<<EOF
    SELECT c.fullName AS community,
    SUM(IF(u.:IS_CO, 0, u.balance)) AS memBalances, SUM(IF(u.:IS_CO, 0, u.rewards)) AS memCredit, 
      SUM(IF(u.:IS_CO, 0, IF(u.:IS_REFILL, u.minimum, 0))) AS memTargetBalances,
    SUM(IF(u.:IS_CO, u.balance, 0)) AS coBalances, SUM(IF(u.:IS_CO, u.rewards, 0)) AS coCredit, 
      SUM(IF(u.:IS_CO, IF(u.:IS_REFILL, u.minimum, 0), 0)) AS coTargetBalances 
    FROM users u LEFT JOIN users c ON c.uid=u.community WHERE c.uid IS NOT NULL GROUP BY c.fullName
EOF
  ,

  'Donations to Common Good and Community' => <<<EOF
    SELECT CONCAT_WS(' ', YEAR(FROM_UNIXTIME(created)), MONTHNAME(FROM_UNIXTIME(created))) AS month,
    SUM(IF(:IS_ROUNDUPS,amount,0)) AS roundups,
    SUM(IF(:IS_CRUMBS,amount,0)) AS crumbs,
    SUM(IF(:IS_PATRONAGE,amount,0)) AS recurring,
    SUM(IF(:IS_GIFT AND NOT :IS_ROUNDUPS AND NOT :IS_CRUMBS AND NOT :IS_PATRONAGE,amount,0)) AS other
    FROM r_txs WHERE 1 AND :IS_GIFT 
    GROUP BY YEAR(FROM_UNIXTIME(created)) DESC, MONTH(FROM_UNIXTIME(created)) DESC
EOF
  ,
];
