<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * List potential or actual investments.
 */
function formInvestList($form, &$sta, $args = '') {
  extract(just('actual clubid', $args));

  list ($actualInv, $proposedInv) = [t('Actual Investments'), t('Proposed Investments')];
  list ($title, $params, $lnkText, $staked) = @$actual
  ? [$actualInv, '', $proposedInv, 'NOT NULL']
  : [$proposedInv, 'actual=1', $actualInv, 'NULL'];
  $lnk = lnk("/invest/list/clubid=$clubid&$params", $lnkText);
  $subtext = t('(click to see %lnk instead)<br>Choose an investment to see details%ratable.', '_lnk ratable', $lnk, @$actual ? '' : t(' or to rate it'));
  
  $sql = <<< X
    SELECT i.vestid, u.fullName, i.investment, i.return, i.soundness AS sound, 
      AVG(r.good) AS good, SUM(s.shares*i.price) AS value
    FROM r_investments i 
    LEFT JOIN users u ON u.uid=i.coid 
    LEFT JOIN r_ratings r ON r.vestid=i.vestid 
    LEFT JOIN r_shares s ON s.vestid=i.vestid
    WHERE s.shid IS $staked AND i.clubid=:clubid
    GROUP BY i.vestid
    ORDER BY u.fullName
X;
  $q = db\q($sql, compact('clubid'));
///  debug(compact(ray('sql clubid')));
  $fields = ray('investment return ' . (@$actual ? 'value' : 'sound good'));
  $hdrs = u\tableRow($fields);
  
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $return = number_format($return, 1) . '%';
    $investment = substr("$fullName: $investment", 0, 80);
    $href = BASE_URL . "/invest/rate/vestid=$vestid&clubid=$clubid";
    $investment = tags('a', $investment, compact('href'));
    $good = $good ? number_format($good, 1) : '?';
    $value = $value ? number_format($value, 0) : '&mdash;';
    $rows[] = u\tableRow(compact($fields));
  }

  if (@$rows) {
    $rows = join($rows, "\n");
    $list = <<< X
    <table id="investments">
    $hdrs
    $rows
    </table>
X;
  } else $list = t('There are no investments at this time.');

  $back = backButton();
  
  return cgform(compact(ray('title subtext back list')));
}
