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
  if (!$choices = w\proxyChoices($mya, [$dft, $dftOther], $choiceList)) return r\go("prox/page=Proxies&$args", 'no local proxy');
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
    return r\go("prox/page=Proxies&$args", 'doubled proxy', 'err');
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
    return $priority == 1 ? r\go("/prox/page=Proxy&priority=2&$args") : w\doSay(t('Success!'), 'ok');
  } elseif ($mya->proxy(1+2 - $priority)) { // do we already have the other proxy?
//    return r\go($mya->ok ? '/settings/proxies' : 'status');
  } else say('proxy to go');

  return r\go('/settings/proxies', 'info saved');
}