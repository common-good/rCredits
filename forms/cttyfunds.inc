<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;



function formCttyFunds($form, &$sta) {
  global $mya;
  $title = t('Funds By Community');
  $rewardsDesc = @$mya->cttyRewardy ? t('Incentive Rewards') : t('an automated increase in a member\'s credit line');
  $subtext = t('<p>How %RCREDITS came in or out of each community. That is, how many were used, received, and issued by each community &mdash; either in exchange for US Dollars (green), as %rewardsDesc (red), or allocated democratically (blue). A community\'s US Dollar Pool balance is the sum of the green column amounts.</p><p>Hover over headings for more information.</p>', compact('rewardsDesc'));
  $heads = ray(t('Community USD Trade-IN Trade-OUT Signup+ Purchase+ Invite+ Inflate+ Spent Grant Loan Invest Fees TOTAL'));
  $headHelps = ray(t('Community name (abbreviated),In exchange for US Dollars,Value of goods and services imported from other communities (negative because credit leaves the community),Value of goods and services exported to other communities,Signup bonuses (incentive rewards for opening a %PROJECT Account),Purchase rewards (incentive rewards for buying or selling using %PROJECT),Incentive rewards for inviting (and helping) a new member to open a %PROJECT Account,Monthly inflation adjustments,%PROJECT community spending (net),%PROJECT community grants,Zero-interest loans from the %PROJECT community,Investments by the %PROJECT community,Fees collected by the %PROJECT community,Total %RCREDITS issued to date'));
  $classes = ray(' usd usd usd reward reward reward reward ctty ctty ctty ctty ctty none');
  $headers = '<tr>';
  foreach ($heads as $k => $v) {
    $head = w\popHelp($v, $headHelps[$k]);
    $class = $classes[$k];
    $headers .= "<th class=\"$class\">$head</th>";
  }
  $headers .= '</th></tr>';
//  $headers = '<tr><th>' . join('</th><th>', $headers) . '</th></tr>';
  
  $q = db\q("SELECT DISTINCT ctty FROM r_stats s LEFT JOIN users u ON u.uid=ctty ORDER BY ctty=0, u.name NOT LIKE '%.', u.fullName");
  while ($row = $q->fetchAssoc()) { // not fetchField because one value is 0
    extract($row);
    $stats = []; // GET FROM each ctty's stats field NOT w\fundStats(r\stats($ctty), FALSE, '1');
    $stats['TOTAL'] = $stats['r']; unset($stats['r']); // change column name and move to end
    $list[] = '<tr><td>' . join('</td><td>', $stats) . '</td></tr>';
  }
  $list = join("\n", @$list ?: ['<b>' . t('No statistics are available.') . '</b><br>&nbsp;']); // @ is for tests
  
  $list = <<<EOF
<table id="cttyFunds">
$headers
$list
</table>
EOF;
  return compact(ray('title subtext list'));
}