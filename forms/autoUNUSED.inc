<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Generate choices for a member typeahead field
 * @param string $string: what the user has typed so far
 * @param string $role: 'payer', 'payee', 'industry', 'company', or '' (other), depending on the role of the user
 * @param int $myid: the current account (globals and session variables are inappropriate here)
 * @todo: ditch this altogether or make it efficient with separate rCredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the typeahead URL to the backend? privacy is not crucial here
 * @patched: Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($role = '', $myid = 0, $string = '') {
//  return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string"));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\shortName($string, '%') . '%'; // this also prevents malicious database injection
//  $string = str_replace(' ', '%', \db_like(" $string "));
  if (!is_numeric($myid)) return;
  $params = 'string'; // list of substitution parameters for query (each section below can add additional params)
  $cttyCrit = ($myid AND $mya = r\acct($myid) and $cttyId = $mya->community) ? "community=$cttyId" : '1';

  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
    $selection = <<<EOF
      u.uid, u.fullName AS display 
      FROM users u WHERE ((u.name LIKE :string OR u.email=:string) AND $cttyCrit)
EOF;
//      FROM users u WHERE (u.uid>1 AND (u.name LIKE :string OR u.email LIKE :string))
  } elseif ($role == 'industry') { // the one place we don't need to include the qid (for definiteness)
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry COLLATE utf8_general_ci AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName COLLATE utf8_general_ci AS display FROM users WHERE :IS_CO AND name LIKE :string
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE :IS_CO AND name LIKE :string';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $personal = $role == 'ctty' ? '' : ' AND NOT :IS_CO';
    $selection = "uid, fullName AS display FROM users WHERE name LIKE :string $cttyCrit $personal";
  } else $selection = 'uid, fullName AS display FROM users WHERE name LIKE :string';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>:CANONIC_ACCTS ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string sql=$selection"));
///   return drupal_json_output(array('zot' => "subs=" . print_r(compact(ray('string myid')), 1)));

  $result = db\q($sql, compact(ray($params)));
  $matches = [];
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = r\qid($uid) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = plain($display);
//    return drupal_json_output(array('zot' => "display=$display"));
  }
  \drupal_json_output($matches);
}
