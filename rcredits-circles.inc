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
 * @todo: optionally make the list of predicted new rTraders without touching their rTrader bit, 
 *   so we can see how it's going halfway through the month.
 * @todo: candidates are only companies and individual Partners/Stewards
 * @todo: clumps
 * @todo: handle foreign relations (need to know whether the foreign employees are rTraders/Stewards/Partners)
 * @todo: handle foreign payees. need to know whether each one is an rTrader? maybe just assume they are or assume they aren't
 */

namespace rCredits\Admin;
use rCredits as r;
use rCredits\API as api;
use rCredits\Testing as t;
use rCredits\Util as u;

define('R_MIN_BUYS', 2); // you can't be an rTrader unless you have at least this many places to spend
define('R_MAX_SCORE', 12); // any more places to spend is effectively the same: enough
define('IS_RTRADER', '(u.flags&' . BIT_RTRADER . ')');

define('UID1', 476); // these are the contiguously numbered records we have at the moment
define('UID9', 950);

/**
 * Bump members up to rTrader status, as appropriate.
 * @todo: do something with the results of identifyCandidates (show some statistics?)
 */
function grow() {
  makeRandomIndividuals(); // DEBUG
  makeRandomRelations(); // DEBUG
  makeRandomRTraders(); // DEBUG
  makeRandomTransactions(); // DEBUG
//  $oldRTraders = r\dbQ('SELECT u.uid FROM users u WHERE :IS_RTRADER')->fetchCol();
//  $baseline = scores($oldRTraders, 'out', TRUE); // score the existing group (for statistics only)

  $newRTraders = bestNewRTraders(10);
  
  foreach ($newRTraders as $uid => $score) { 
    debug(r\userField('full_name', $uid)); // keep this for now
  }
}

/**
 * Return an array of uids for the best new rTraders
 * @param numeric $target: how many new rTraders to return (approximately)
 * @todo: add clumping (described above). Perhaps by creating a second candidates table for just the candidate businesses and clumps
 */
function bestNewRTraders($target = 100) {
  // NEW (10/24/2012)
  // Create a table of candidates
  $sql = <<<EOF
    TRUNCATE r_candidates;
    INSERT INTO r_candidates (uid, is_company)
    SELECT u.uid, (u.account_type<>:R_PERSONAL) AS is_company
    FROM users u
    WHERE u.uid > 1 AND NOT :IS_RTRADER
EOF;
  r\dbQ($sql); // include both individuals and companies (we don't currently use is_company)

  // handle clumps here
  $minScore = R_MIN_BUYS * pow(R_MAX_SCORE, -R_MAX_SCORE/2);
  $nubes = array();
  while (count($nubes) < $target) { // until we reach the target number or no viable candidates
    $best = scores(); // get the best scores, best first
    list ($uid, $score) = $best; // take just the first
    if ($score < $minScore) break; // not good enough (less than R_MIN_BUYS from the least important rTraders)
    $nubes += promote($uid, $score); // bring them in one by one (along with all eligible employees) 
  }
  return $nubes;
}
//    foreach ($best as $uid => $score) { // do this only if scoring take a very long time
//      if (count($nubes) >= $target) break;

function promote($uid, $score) {
  $sql = <<<EOF
    SELECT r.agent_uid FROM r_relations r
    INNER join r_candidates c ON c.uid=r.agent_uid
    WHERE r.main_uid=:uid AND NOT c.ignore
EOF;
  $nubes = r\dbQ($sql, compact('uid'))->fetchCol();
  $nubes[] = $uid;
  foreach ($nubes as $one) r\acct($one)->setBit(BIT_RTRADER);
  $list = join(', ', $nubes);
  r\dbQ('DELETE from r_candidates WHERE uid IN (:list)', compact('list'));
  
  $score = number_format($score, 0);
  debug("New rTraders (score: $score): $list");
  return $nubes;
}

/**
 * Score
 */
function scores() {

  r\dbQ('UPDATE r_candidates SET `ignore`=0 WHERE 1'); // this function gets called repeatedly, so we need to reset

  // First add unemployed people to r_candidates (eg people with rental income) -- treat them as self-employed
  // Count number of rTrader places that each candidate ASIF buys (to eliminate those that buy from too few places)
  // Include pending ASIF transactions because that gives the information about intent
  $countSQL = <<<EOF
    DROP TEMPORARY TABLE IF EXISTS r_temp;
    CREATE TEMPORARY TABLE r_temp
    SELECT c.uid, COUNT(DISTINCT a.payee) AS count
    FROM (r_asif a INNER JOIN r_candidates c ON c.uid=a.payer)
    LEFT JOIN users u ON u.uid=a.payee
    WHERE a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND u.account_type=:R_PERSONAL
    AND :IS_RTRADER
    GROUP BY c.uid
EOF;

  $ignoreSQL = 'UPDATE r_candidates c INNER JOIN r_temp tmp ON tmp.uid=c.uid SET c.ignore=1 WHERE tmp.count<:R_MIN_BUYS';

  r\dbQ($countSQL);
  if (\db_table_exists('r_temp')) r\dbQ($ignoreSQL); // mark ineligible individual candidates
// FAILS  r\dbQ("UPDATE r_candidates SET `ignore`=1 WHERE ($countSQL) < :R_MIN_BUYS"); // mark ineligible individual candidates

  // Eliminate candidate companies that buy from too few places (including their candidate employees)
  $eligibleEmployeesSQL =<<<EOF
    SELECT r.agent_uid AS employee 
    FROM r_candidates d 
    INNER JOIN r_relations r ON d.uid=r.main_uid
    WHERE d.uid=c.uid AND NOT `ignore`
EOF;

  $countSQL = <<<EOF
    DROP TEMPORARY TABLE IF EXISTS r_temp;
    CREATE TEMPORARY TABLE r_temp
    SELECT c.uid, COUNT(DISTINCT a.payee) as count
    FROM (r_asif a INNER JOIN r_candidates c ON c.uid=a.payer)
    INNER JOIN users u ON u.uid=a.payee
    WHERE a.type=:TX_TRANSFER 
    AND a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND u.account_type<>:R_PERSONAL
    AND (:IS_RTRADER OR a.payee IN ($eligibleEmployeesSQL))
    GROUP BY c.uid
EOF;
  r\dbQ($countSQL);
  if (\db_table_exists('r_temp')) r\dbQ($ignoreSQL); // mark ineligible companies
// FAILS  r\dbQ("UPDATE r_candidates SET `ignore`=1 WHERE ($countSQL) < :R_MIN_BUYS"); // mark ineligible companies

  // Find out how many places each rTrader can ALREADY spend
  // might want to INSERT these into a temp table for efficiency (rather than reusing the SQL, below)
// FAILS    SELECT t.payer, MIN(:R_MAX_SCORE, COUNT(DISTINCT t.payee)) as count
  $rTraderScoresSQL = <<<EOF
    DROP TEMPORARY TABLE IF EXISTS r_temp;
    CREATE TEMPORARY TABLE r_temp
    SELECT t.payer, LEAST(:R_MAX_SCORE, COUNT(DISTINCT t.payee)) as count
    FROM r_txs t
    WHERE t.type=:TX_TRANSFER
    AND t.goods
    AND t.state IN (:TX_DONE, :TX_PENDING)
    GROUP BY t.payer
EOF;
  r\dbQ($rTraderScoresSQL); // each rTrader already buys from how many

//  $baseline = r\dbQ('SELECT * FROM r_temp')->fetchAllKeyed(0, 1); // each rTrader already buys from how many
//  $baseline = r\dbQ($rTraderScoresSQL)->fetchAllKeyed(0, 1); // each rTrader already buys from how many
//  sort($baseline); // worst score first (that's the one we most want to improve)
//  $base_score = score($baseline); // probably no use for this but good for statistics

  // For each number of places that rTraders already spend (3, 4, etc):
  // For each candidate, count the number of rTraders in that spending group that would buy from them
  $sql = <<<EOF
    TRUNCATE r_scores;
    INSERT INTO r_scores
    SELECT c.uid, tmp.count, COUNT(DISTINCT tmp.payer) AS score
    FROM r_temp tmp
    INNER JOIN r_asif a ON a.payer=tmp.payer
    INNER JOIN r_candidates c ON c.uid=a.payee
    WHERE a.type=:TX_TRANSFER 
    AND a.goods
    AND a.state IN (:TX_DONE, :TX_PENDING)
    AND NOT c.ignore
    GROUP BY tmp.count, c.uid
EOF;
  r\dbQ($sql);

  // Score each candidate, giving the most points for benefiting the low-count rTrader groups
  // That is, give the highest score to the candidate companies that would provide the most opportunity to spend,
  // to the rTraders who currently have the fewest places to spend
  $totalScore = 'SUM(score * POWER(:R_MAX_SCORE, :R_MAX_SCORE/2 - count))';
  $sql = "SELECT company, $totalScore AS totalScore FROM r_scores GROUP BY company ORDER BY totalScore DESC";
  return r\dbQ($sql)->fetchAllKeyed(0, 1); // total score for each company
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
 /*
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

  $result = dbQ($sql)->fetchAllKeyed(0, 1);
  sort($result); // put the worst first (returns TRUE on success)
  return $result;
}
*/

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

function makeRandomIndividuals() {
  for ($uid = UID1; $uid <= UID9; $uid++) {
    $account_type = rand(1, 4) == 2 ? R_COMMERCIAL : R_PERSONAL;
    \drupal_write_record('users', compact('uid', 'account_type'), 'uid');
  }
}

function makeRandomRelations() {
  r\dbQ('TRUNCATE r_relations');
  $companies = (array) r\dbQ('SELECT uid FROM users WHERE uid BETWEEN :UID1 AND :UID9 AND account_type=:R_COMMERCIAL')->fetchCol();
  for ($agent_uid = UID1; $agent_uid <= UID9; $agent_uid++) {
    if (!in_array($agent_uid, $companies)) {
      $main_uid = $companies[rand(0, count($companies) - 1)];
      $is_employee = TRUE;
      \drupal_write_record('r_relations', compact(u\ray('main_uid agent_uid is_employee')));
    }
  }
}

function makeRandomRTraders($target = 100) {
  for ($uid = UID1; $uid <= UID9; $uid++) r\acct($uid)->setBit(BIT_RTRADER, 0);

  for ($uid = 0; $uid < $target; $uid++) {
    while (TRUE) {
      $acct = r\acct(rand(UID1, UID9));
      if ($acct and !$acct->hasBit(BIT_RTRADER)) break;
    }
    $acct->setBit(BIT_RTRADER);
  }
}

function makeRandomTransactions($count = 5000) {
  r\dbQ('TRUNCATE r_asif');
  r\dbQ('TRUNCATE r_txs');
  for ($i = 1; $i < $count; $i++) {
    $amount = rand(10,200);
    $to = $from = $payer = rand(UID1, UID9);
    while ($to == $from) $to = $payee = rand(UID1, UID9);
    $taking = rand(0, 1);
    $state = rand(1, 12) == 6 ? TX_PENDING : TX_DONE;
    $goods = rand(1, 20) == 10 ? 0 : 1;
    $purpose = $goods ? 'stuff' : 'cash';
    $real = (r\isRTrader($payer) and r\isRTrader($payee));
    $serial = t\makeTransaction(compact(u\ray('amount to from purpose taking state goods')), FALSE, $real);
    if ($goods) {
      list ($goods, $purpose, $from) = array(0, 'reward', r\communityUid());
      list ($amount, $to) = array(round(.05 * $amount, 2), $payer);
      t\makeTransaction(compact(u\ray('serial amount to from purpose taking state goods')), FALSE, $real);
      list ($amount, $to) = array((int) (2 * $amount), $payee);
      t\makeTransaction(compact(u\ray('serial amount to from purpose taking state goods')), FALSE, $real);
    }
  }
}