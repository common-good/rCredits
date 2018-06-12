<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Choose one proxy.
 * $args:
 *   @param int $priority: 1 for proxy, 2 for alternate
 * uid is not passed, for security.
 */
function formProxy($form, &$sta, $args = '') {
  extract(just('priority', $args));
  if ($priority != 1 and $priority != 2) w\hack('bad proxy priority');
  $mya = w\eLinkAcct($sta, $args);
  $dft = $mya->proxy($priority);
  $dftOther = $mya->proxy(3 - $priority);
  if (!$choices = w\proxyChoices($mya, [$dft, $dftOther], $choiceList)) return w\go("prox/page=Proxies&$args", 'no local proxy');
  if (!$mya->member and count($choiceList) <= 2) { // no real choices, so don't make the member choose
    if (!$dft) $mya->proxy($priority, $choiceList[0]);
    if (!$dftOther) $mya->proxy(3 - $priority, $choiceList[$dft ? 0 : 1]);
    say('no local proxy');
    return w\goNextStep('proxies');
  }

  $title = $priority == 1 ? t('Your #1 Proxy') : t('Your <%b>Alternate</b> Proxy', '_b', 'b class="loud"');
  $title = item($title . t(' (choose one)'), BARE);
  foreach (['warning', 'success', 'primary', 'slight'] as $i => $v) {
    $b["_b$i"] = "b class=\"btn btn-xs btn-$v\"";
  }
  
  $limitedChoice = $mya->cttyA->isRegion ? t('<p>You are outside any organized %PROJECT Community, so your choices will be very limited until more people sign up in your area. But no worries. You will have the opportunity to revise your proxy choices before your community votes on anything.</p>') : '';
  $subtext = item(t('<p>Choose and click someone you know and trust. Top suggestions for you are <%b1>green</b>, <%b2>blue</b>, and <%b3>peach</b> (in that order).</p>', $b) . (($dft or $dftOther) ? t('<p>Your current selection(s), if any, is/are <%b0>orange</b>.</p>', $b) : '') . $limitedChoice, BARE);
  $list = $choices;
  $priority = hidFld($priority);
  return cgform(compact(ray('title subtext list priority')), '', FALSE);
}

function formProxy_validate($form, &$sta) {
  extract(just('priority proxyChoice', $sta['input']));
  $mya = w\eLinkAcct($sta, $args); 
  $choice = key($sta['input']);
  u\EXPECT(substr($choice, 0, 2) == 'i-', 'missing proxy choice');
  $proxyChoice = substr($choice, 2);
  if (@$proxyChoice == $mya->proxy(1+2 - $priority)) {
    return w\go("prox/page=Proxies&$args", 'doubled proxy', 'err');
  }
  $sta['input'] += compact('proxyChoice');
  return formProxy_submit($form, $sta); // oddly, _submit never gets called otherwise
}

function formProxy_submit($form, &$sta) {
  extract(just('priority proxyChoice', $sta['input']));
  $mya = w\eLinkAcct($sta, $args); 

  $mya->proxy($priority, $proxyChoice);
//  say('info saved');

  if (!r\acct()) { // called from email link
    return $priority == 1 ? w\go("/prox/page=Proxy&priority=2&$args") : w\doSay(t('Success!'), 'ok');
  } elseif ($mya->proxy(1+2 - $priority)) { // do we already have the other proxy?
//    return w\go($mya->ok ? '/settings/proxies' : 'status');
  } else say('proxy to go');

  return w\go('/settings/proxies', 'info saved');
}

/**
 * Return a list of proxy choices for the current account.
 * @param acct $a: the "current" account (whether signed in or not)
 * @param int $dfts: the current proxy choices, if any
 * @param array $choiceList: (RETURNED) array of account record IDs to choose from, not including current choice
 */
function proxyChoices($a, $dfts, &$choiceList) {
  $uid = $a->id;
  $table = <<<EOF
    (SELECT DISTINCT IF(inviter=:uid, invitee, inviter) AS uid FROM r_invites 
      WHERE :uid IN (inviter, invitee) AND invitee>0
    UNION SELECT DISTINCT IF(person=:uid, proxy, person) AS uid FROM r_proxies 
      WHERE :uid IN (person, proxy)) q
EOF;

  $cttyCrit = $a->cttyA->isRegion  // must have same params either way (hence :ctty=:ctty)
  ? "((:ctty=:ctty AND city='$a->city' AND state=$a->state AND :IS_OK) OR u.uid IN (:DEFAULT_PROXIES))"
  : '(community=:ctty AND :IS_OK)';
  
  if (!$friends = db\lst('uid', $table, 1, compact('uid'))) $friends = '0'; // list of account's friends
  $sql = <<<EOF
    SELECT u.uid, fullName, city, state, SUM(w) AS weight FROM (
      SELECT uid, :PHP_INT_MAX/2 AS w FROM users 
      WHERE uid IN ($friends)
      UNION
      SELECT uid, :PHP_INT_MAX/4 AS w FROM users 
      WHERE (state=:state AND city=:city) OR zip=:zip
      UNION
      SELECT IF(uid1 IN ($friends), uid2, uid1) AS uid, weight AS w FROM r_near
      WHERE uid1 IN ($friends) OR uid2 IN ($friends)
      UNION
      SELECT uid, 0 AS w FROM users
    ) n LEFT JOIN users u ON u.uid=n.uid
    WHERE $cttyCrit AND u.uid>1 AND u.uid<>:uid AND NOT :IS_CO
    GROUP BY u.uid, u.state, u.city 
    ORDER BY state, city, fullName
EOF;
//    ORDER BY GREATEST(SUM(w), :PHP_INT_MAX/4) DESC, state<>:state, state, city, SUM(w) DESC, fullName
  $q = db\q($sql, ray('uid state city zip ctty', $uid, $a->state, $a->city, $a->zip, $a->community));
  $res = '';
  $cityX = '(none)'; // required for placeholders to work right
  while ($row = $q->fetchAssoc()) { // show choices in reverse order, so inviter(s) are at top
    extract($row);
    if (!$current = in($uid, $dfts)) $choiceList[] = $uid;
    $state = ($state == $a->state or !$state) ? '' : (', ' . r\realState($state));
/*    if ($weight < PHP_INT_MAX / 2) { // show city and maybe state
      $state = $state == $a->state ? '' : (', ' . r\realState($state));
    } else list ($city, $state) = [t('Top Suggestions'), '']; */
    if ($city <> $cityX) {
      if ($cityX) $res .= "</td></tr>\n"; // finish previous line
      $res .= "<tr><th>$city$state</th><td>";
      $cityX = $city;
    }
    $style = $weight < PHP_INT_MAX / 4 ? 'default'
    : ($weight == PHP_INT_MAX / 4 ? 'slight'
    : ($weight <= PHP_INT_MAX / 2 ? 'primary'
    : 'success' ));
    if ($current) $style = 'warning';
///    debug(compact('row','city','cityX','friends') + ['max/2-w'=>PHP_INT_MAX / 2 - $weight]);
    $res .= <<<EOF
<button type="submit" name="i-$uid" class="btn btn-xs btn-$style ladda-button" data-style="zoom-out">
    <span class="ladda-label">$fullName</span></button>    
EOF;
  }
  if ($res) $res .= '</td></tr>'; // finish last line
  return "<div class=\"well\"><table>$res</table></div>";
}