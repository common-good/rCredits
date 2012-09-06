<?php
/**
 * @file
 * rCredits circles
 * The business logic in this file is used to determine who are the best members to be granted rTrader status next.
 *
 * The central idea for optimizing economic circles is to make the rCredits system stronger and stronger by maximizing
 * their minimum spendability. That is, the person who has the least places to spend rCredits has as many places as we can manage.
 * Each time an rTrader is added to the system, we choose the candidate or group of candidates that will result in the most places to spend
 * for the participant who has the least places to spend. 
 *
 * Why? We don't want anyone dropping out. So we want the participant who is most likely to drop out to have the best experience possible
 * (so they won't drop out).
 *
 * In order to find the best candidate or group of candidates, we look at what happens if we bring them in:
 *   How many places does the (possibly new) weakest link now have, to spend. Then how about the second weakest link, etc.
 * We compare that score with the projection for other candidates and groups of candidates. The best score wins.
 *
 * Here are some more specifics:
 *   Start by combining employers with their employees (multiple employers, including self, can be credited with the same employee)
 *   Omit from consideration any candidates with less than three employees or other places to spend (whether rTraders or not)
 *   Next give every candidate a score.
 *   Starting with the highest scoring candidates, try bringing them in in 2s, then 3s, etc. up to 25s or so, to see if it improves their score.
 *      This is maybe a looser combination than the combination of employees into employers ?
 *      An improved score means the worst score in the group improves when they come in together.
 *      Once they are combined they are considered as an economic unit (consider it again in 2s, 3s, etc)
 *      Don't let the groups get bigger than the desired number of new participants.
 *   Promote to rTrader the highest scoring groups until we have approximately the desired number.
 */

namespace rCredits; // typically abbreviated as "r"
use \rCredits\API as api;

/**
 * Bump members up to rTrader status, as appropriate.
 */
function grow() {
  $new_rtraders = new_rtraders(100);
  
  foreach ($new_rtraders as $uid) { 
    debug(r\user_field('full_name', $uid));
  }
}

/**
 * Return a list of rTraders and a list of non-rTraders
 */
function identify_rtraders(&$old_rtraders, &$candidates) {
  $rid = RCREDITS_ROLE_RTRADER;
  $old_rtraders = db_q('SELECT DISTINCT uid FROM users_roles WHERE rid=:rid', compact('rid'))->fetchCol();
  $candidates = db_q('SELECT u.uid FROM users u LEFT JOIN users_roles r WHERE r.rid IS NULL')->fetchCol();
}

/**
 * Return an array of uids for the best new rTraders
 * @param numeric $target: how many new rTraders to return (approximately)
 */
function new_rtraders($target) {
  list ($old_rtraders, $candidates) = identify_rtraders();
  foreach ($candidates as $one => $customer_count) if ($customer_count < 3) unset($candidates[$one]); // not eligible
}

/**
 * Return the potential-new-rTrader score for all candidates.
 * Scores are calculated as the total number of rTraders or tentative rTraders the candidate "AS IF" bought goods and services from -- ever. Ironically, nobody gets points for actual transactions.
 * NOTE: Once rCredits is roaring, we may want to require repeated AS IFs and count only repeated purchases.
 *
 * @param array $rtraders: uid list of actual and tentative rTraders
 * @param string $inout: return score for transactions 'in' or 'out'
 * @param int $realness: 1=return number of real transactions, 2=return the number of ASIF transactions (default), 3=any
 * test with: $a = array(129, 130); print_r(rCredits\scores($a, 1));
 */
function scores(&$rtraders, $inout, $realness = 2) {
  $choices = join(',', $rtraders);
  $date_range = '1'; // all transactions ever, for now
  
  $countable_sql = <<<EOF
    SELECT DISTINCT t.payer, IF(e.company IS NULL, t.payee, e.company) AS payee, IF(t.real=3,1,0) AS reals, IF(t.real=3,0,1) AS asifs
    FROM r_txs t LEFT JOIN r_employees e ON e.employee=t.payee
    WHERE t.payer IN($choices) AND t.state>0 AND $date_range
EOF;
  // word has it SELECT COUNT(field) or SUM(field) FROM (SELECT DISTINCT) is faster than COUNT(DISTINCT field)
  $sql = <<<EOF
    SELECT t2.payee, SUM(reals) AS reals, SUM(asifs) AS asifs, SUM(reals+asifs) AS any
    FROM ($countable_sql) t2 GROUP BY t2.payee
EOF;
  if ($inout == 'out') $sql = strtr($sql, array('payee' => 'PAYER', 'payer' => 'payee', 'PAYER' => 'payer'));
  
  return db_q($sql)->fetchAllKeyed(0, $realness);
}
