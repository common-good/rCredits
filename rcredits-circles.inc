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
 * Why? Because we don't want anyone dropping out. 
 * So we want the participant who is most likely to drop out to have the best experience possible (so they won't drop out).
 * A good experience generally means they have plenty of places to spend their rCredits.
 *
 * In order to find the best candidate or group of candidates, we look at what happens if we bring them in:
 * How many places does the (possibly new) weakest link now have, to spend. Then how about the second weakest link, etc.
 * We compare that score with the projection for other candidates and groups of candidates. The best score wins.
 *
 * Here are some more specifics:
 *   A. Eligible employees of rTrader companies get in automatically (eligible means they have 3+ places to spend). 
 *   B. Companies
 *     Combine employers with their eligible employees (multiple employers, including self, can be credited with the same employee)
 *     Together they are a single candidate
 *     (Even a self-employed person has to have a separate business account)
 *   C. Clumping (this also works to get started, at the very beginning)
 *     Focus first on the candidates. We may need to bring them in in clumps (for example a group from a disjoint community).
 *     Give each candidate a score (in relation to the set of candidates).
 *     Start with the candidate that has the best score all by itself. If zero, pick the one that the most other candidates buy from.
 *     Add the best next candidate. Repeat until the best new score is less than the current score.
 *     (An improved score means the worst score in the group improves when the candidate comes in.)
 *     Don't let the groups get bigger than the desired number of new participants.
 *     Include this group as a single candidate in part D (exactly as though it were a single company)
 *     Repeat this Clumping step with the remaining candidates (for example, for additional communities). 
 *     Stop when there are no more candidates or the clump is a single candidate (company + employees)
 *   B. Adding to the existing group (growing after the system has been running for a month or more)
 *     Give each candidate (including the clumps from above) a score (this time in relation to the existing rTraders).
 *     Omit from consideration any candidates with less than three employees or other places to spend (whether rTraders or not)
 *     Remember: an individual can be part of more than one company or clump.
 *     Promote to rTrader the highest scoring candidate.
 *     Repeat until we have approximately the target number of rTraders or until no candidate gives a score of 3 or better.
 *   C. Buy-in
 *     If the steps above do not result in the target number of new participants, let supporters buy rCredits with US Dollars.
 *     They don't get any rebates until they have a way to receive rCredits as payment. 
 *   D. Once a Common Good Community is 2/3 rTraders, everybody in that community gets in automatically (regardless of the target).
 *
 * @todo: null out r_sms todo when someone becomes an rTrader
 */

namespace rCredits; // typically abbreviated as "r"
use rCredits\API as api;

define('R_MIN_BUYS', 3); // you can't be an rTrader unless you have at least this many places to spend
define('R_MAX_SCORE', 12); // any more places to spend is effectively the same: enough
define('IS_RTRADER', '(u.flags&' . BIT_RTRADER . ')');
/**
 * Bump members up to rTrader status, as appropriate.
 * @todo: do something with the results of identify_rtraders (show some statistics?)
 */
function grow() {
  list ($old_rtraders, $candidates) = identify_rtraders();
  $baseline = scores($old_rtraders, 'out', TRUE); // score the existing group (for statistics only)
  $new_rtraders = best_new_rtraders(100);
  
  foreach ($new_rtraders as $uid) { 
    debug(r\user_field('full_name', $uid)); // keep this for now
  }
}

/**
 * Return a list of rTraders and a list of non-rTraders
 * @todo: include individuals with income in candidates -- get this from r_asif
 */
function identify_rtraders($old_rtraders, $candidates) {
  $old_rtraders = db_q('SELECT u.uid FROM users u WHERE :IS_RTRADER')->fetchCol();
  $candidates = db_q('SELECT u.uid FROM users u WHERE u.account_type<>:R_PERSONAL AND NOT :IS_RTRADER')->fetchCol();
  return array($old_rtraders, $candidates);
}

/**
 * Return an array of uids for the best new rTraders
 * @param numeric $target: how many new rTraders to return (approximately)
 * @todo: add clumping (described above). Perhaps by creating a second employs table for just the candidate businesses and clumps
 * @todo: also copy non-company earners to the temp table (get from r_asif)
 */
function best_new_rtraders($target) {
  // NEW (10/24/2012)
  // Create a table of candidates
  $sql = <<<EOF
    TRUNCATE r_candidates
    INSERT INTO r_candidates
    SELECT u.uid, (u.account_type<>:R_PERSONAL) AS is_company
    FROM users u
    WHERE NOT :IS_RTRADER
EOF;
    /*
  $sql = <<<EOF
    TRUNCATE r_employs_temp;
    INSERT INTO r_employs_temp
    SELECT e.reid AS eid, e.main_uid AS company, e.agent_uid AS employee, (s.rid<=>:R_ROLE_RTRADER) AS employee_is_rtrader 
    FROM (r_relations e LEFT JOIN users u ON u.uid=e.main_uid) 
    WHERE e.is_employee AND NOT :IS_RTRADER
EOF;
*/
  r\do_q($sql); // copy only the non-rTrader company records to temp table
  
  // handle clumps here
 
  $previous_nubes = -1;
  $nubes = 0;
  while ($nubes > $previous_nubes and $nubes < $target) { // until we reach the target number or no viable candidates
    $best = scores(); // get the best scores
    foreach ($best as $uid => $score) { 
      if ($score < R_MIN_BUYS) break; // not good enough
      $previous_nubes = $nubes;
      $nubes += promote($uid); // bring them in one by one (with all eligible employees) 
      if ($nubes >= $target) break;
    }
  }

/*
  $candidates = scores($candidates, 'out', FALSE, array_merge($candidates, $old_rtraders)); // 
  foreach ($candidates as $one => $customer_count) if ($customer_count < R_MIN_BUYS) unset($candidates[$one]); // not eligible
  */
}

function promote($uid) {
// remember to delete the promoted candidates from r_candidates
  $results = r\do_q("SELECT employee FROM r_employs_temp WHERE company=$uid")->fetchColumn();
  $results[] = $uid;
  $nubes = count($results);
  $role = ',' . R_MIN_BUYS;
  $results = join($role . '),(', $results) . $role;
  $nubes = 0;
  r\do_q("INSERT INTO users_roles (uid, rid) VALUES ($results)");
  return $nubes;
}

/**
 * Score
 */
function scores() {
  // First add unemployed people to r_employs_temp (eg people with rental income) -- treat them as self-employed
  // Count number of rTrader places that each candidate ASIF buys (to eliminate those that buy from too few places)
  // Include pending ASIF transactions because that gives the information about intent
  $count_sql = <<<EOF
    SELECT COUNT(DISTINCT a.payee)
    FROM (r_asif a INNER JOIN r_candidates c ON c.uid=a.payer)
    LEFT JOIN users u ON u.uid=a.payee
    GROUP BY c.uid
    WHERE a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND :IS_RTRADER
EOF;
/*
  $count_sql = <<<EOF
    SELECT COUNT(DISTINCT a.payee)
    FROM (r_asif a INNER JOIN r_employs_temp e ON e.employee=a.payer)
    LEFT JOIN users u ON u.uid=a.payee
    GROUP BY e.employee
    WHERE a.type=:TX_TRANSFER 
    AND a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND NOT e.employee_is_rtrader
    AND :IS_RTRADER
EOF;
*/
  r\do_q("UPDATE r_employs_temp SET ignore=IF(($count_sql) < :R_MIN_BUYS, 1, 0)"); // mark ineligible non-rTrader employees

  // Eliminate companies that buy from too few places
  $eligible_employees_sql = 'SELECT employee FROM r_employs_temp f WHERE f.company=e.company AND NOT ignore';
  $count_sql = <<<EOF
    SELECT COUNT(DISTINCT a.payee) as count
    FROM (r_asif a INNER JOIN r_employs_temp e ON e.company=a.payer)
    INNER JOIN users u ON u.uid=a.payee
    GROUP BY e.company
    WHERE a.type=:TX_TRANSFER 
    AND a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND (:IS_RTRADER OR a.payee IN ($eligible_employees_sql)
EOF;
  r\do_q("UPDATE r_employs_temp SET ignore=1 WHERE ($count_sql) < :R_MIN_BUYS"); // mark ineligible companies
  
  // Find out how many places each rTrader can ALREADY spend
  // might want to INSERT these into a temp table for efficiency (rather than reusing the SQL, below)
  $rTrader_scores_sql = <<<EOF
    SELECT t.payer, MIN(:R_MAX_SCORE, COUNT(DISTINCT t.payee)) as count
    FROM r_txs t GROUP BY t.payer
    WHERE t.type=:TX_TRANSFER
    AND t.goods
    AND t.state IN (:TX_DONE, :TX_PENDING)
EOF;
  $baseline = r\do_q($rTrader_scores_sql)->fetchAllKeyed(0, 1); // each rTrader already buys from how many
//  sort($baseline); // worst score first (that's the one we most want to improve)
  $base_score = score($baseline); // probably no use for this but good for statistics
  
  // For each number of places that rTraders already spend (3, 4, etc):
  // For each candidate, count the number of rTraders in that spending group that would buy from them
  $sql = <<<EOF
    TRUNCATE r_scores;
    INSERT INTO r_scores
    SELECT e.company, r.count, COUNT(DISTINCT r.payer) AS score
    FROM ($rTrader_scores_sql) r
    INNER JOIN r_asif a ON a.payer=r.payer
    INNER JOIN r_employs_temp e ON e.company=a.payee
    GROUP BY r.count, e.company
    WHERE a.type=:TX_TRANSFER 
    AND a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND NOT e.ignore
EOF;

  // Score each candidate, giving the most points for benefiting the low-count rTrader groups
  // That is, give the highest score to the candidate companies that would provide the most opportunity to spend,
  // to the rTraders who currently have the fewest places to spend
  $total_score = 'SUM(score * POWER(:R_MAX_SCORE, :R_MAX_SCORE/2 - count))';
  $sql = "SELECT company, $total_score AS total_score FROM r_scores GROUP BY company ORDER BY total_score DESC";
  return r\do_q($sql)->fetchAllKeyed(0, 1); // total score for each company
}

/**
 * Return the potential-new-rTrader score for a candidate or group of candidates.
 * Scores are returned as an array of the number of new places each rTrader would be able to buy (or sell), least first.
 * Each array entry is the number of places the candidate "AS IF" bought (or sold) goods and services from -- ever. 
 *
 * @param array $rtraders: uid list of actual and tentative rTraders
 * @param string $inout: return score for transactions 'out' (places to buy)(the default) or 'in' (places to sell)
 * @param int $real: if TRUE, return number of real transactions instead (default FALSE)
 * @return array: a sorted array of results, least first (big is good)
 *   The array is keyed by uid, just in case that's useful.
 * @todo: Once rCredits is roaring, we may want to require repeated AS IFs and count only repeated purchases.
 * @todo: if $rtraders is specified, do IN (SELECT...) rather than IN (<list>) or maybe pass the query as the param.
 * test with: $a = array(129, 130); print_r(rCredits\scores($a, 1));
 */
function scoresOLD($candidates, $inout = 'out', $real = FALSE, $rtraders = '') {
  $candidates = join(',', $candidates);
  $date_range = '1'; // all transactions ever, for now
  $rtraders = $rtraders ? join(',', $rtraders) : '';
  $table = $real ? 'r_txs' : 'r_asif';
  
  // First find all payer/payee pairs (where payee is
  // NOTE: word has it SELECT COUNT(field) or SUM(field) FROM (SELECT DISTINCT) is faster than COUNT(DISTINCT field)
  $countable_sql = <<<EOF
    SELECT DISTINCT IF(e.company IS NULL, t.payer, e.company) AS payer, 
    FROM $table t LEFT JOIN r_relations e ON e.employee=t.payer
    WHERE t.payer IN ($candidates) 
      AND t.payee IN ($rtraders) 
      AND t.goods
      AND t.state IN (:TX_DONE, :TX_PENDING) 
      AND $date_range
EOF;
  $sql = "SELECT t2.payee, COUNT(*) AS count FROM ($countable_sql) t2 GROUP BY t2.payee";
  if ($inout == 'in') $sql = strtr($sql, array('payee' => 'PAYER', 'payer' => 'payee', 'PAYER' => 'payer')); // payer <-> payee

  $result = db_q($sql)->fetchAllKeyed(0, 1);
  sort($result); // put the worst first (returns TRUE on success)
  return $result;
}

/**
 * Say whether the first array is less than the second.
 * The array represents an arbitrarily large radix integer with fractional digits.
 * Each entry is a digit, with the 0th being the high-order digit.
 * The 0th entries of the two arrays are aligned, so if one is longer the other is (conceptually) padded with zeroes.
 * Array keys are ignored.
 */
function array_lt($a, $b) {
  $a = array_values($a);
  $b = array_values($b);
  for ($i = 0; $i < max(count($a), count($b)); $i++) {
    if (@$a[$i] != @$b[$i]) return (@$a[$i] < @$b[$i]);
  }
  return FALSE; // arrays are equal
}