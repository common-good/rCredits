<?php

use rCredits as r;
use rCredits\Util as u;

/**
 * @file
 * List of queries for member companies.
 * These special parameters are recognized:
 *  :ctty  the current account's community ID
 *  :myid  the current account's record ID
 * Note: query names cannot contain any of the following characters: /&? (because they confuse the url).
 */

$name = u\sqlSerialValue('r.data', 2); // used in Customers query
$phone = u\sqlSerialValue('r.data', 6);
 
$queries = [
  t('Customers') =>
  "SELECT IFNULL(uid, (SELECT uid FROM users WHERE fullName=$name)) AS account,
    code AS customer, IFNULL(fullName, $name) AS name, IF(u.activated, '', $phone) AS phone,
    FROM_UNIXTIME(r.created, '%m/%d/%Y') AS created,
    IF(u.uid IS NULL OR u.activated=0,
      IF(u.uid IS NULL OR u.uid=0, 'NOT BEGUN', 'in process'), 
      FROM_UNIXTIME(u.activated, '%m/%d/%Y')) AS completed
  FROM r_relations r LEFT JOIN users u ON u.uid=r.other WHERE r.main=:myid AND isCustomer ORDER BY u.activated,r.created"
  . t('~<p>A list of your customers who joined through a shared sign-up, showing progress opening their %PROJECT account.</p>'),

  t('Daily Totals') =>
  "SELECT CONCAT_WS(' ', FROM_UNIXTIME(created, '%m/%d'), LEFT(FROM_UNIXTIME(created, '%W'), 3)) AS `day`,
  SUM(IF(payee=:myid, amount, 0)) AS `total in`,
  SUM(IF(payer=:myid, amount, 0)) AS `total out`
  FROM r_txs WHERE :myid IN (payer, payee) AND created>:REQUEST_TIME-190*:DAY_SECS
  GROUP BY FROM_UNIXTIME(created, '%Y-%m-%d')
  ORDER BY created DESC",
];
