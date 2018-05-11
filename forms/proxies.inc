<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Choose default proxies.
 */
function formProxies($form, &$sta, $args = '') {
  global $base_url;
  $mya = w\eLinkAcct($sta, $args);

  foreach ([1, 2] as $i) {
    if ($p[$i] = $mya->proxy($i)) {
      $pi = r\acct($p[$i])->fullName;
    } else $pi = t('(none chosen)');
//    $proxy[$i] = $pi . t(' [<@a>show choices</a>]', ['@a'=>"a href=$base_url/prox/page=Proxy&priority=$i&$args"]);
    $proxy[$i] = $pi . ' ' . spinLink("/prox/page=Proxy&priority=$i&$args", t('Show Choices'), '', 'primary', 'xs');
  }
  $gotBoth = ($p[1] and $p[2]);
  
  $title = t('Proxies');
  $afterOk = $mya->ok ? '' : ', once your account is approved,';
  if ($mya->ok or !$gotBoth) $subtext = t('proxy help') . t(<<<EOF
    <p>If you don't see listed here the people you want, choose someone else temporarily, then%afterOk click <%a>Invite</a> on the Community menu to invite who you most want to represent you.</p>
EOF
  , '_a afterOk', r\acct() ? atag('/community/invite') : '', $afterOk);

//  if ($gotBoth) $mya->stepDone('proxies'); // sometimes doesn't get set in formProxy (dunno why)
  $proxy1 = item($proxy[1], t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'));
  $proxy2 = item($proxy[2], t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'));
  $form = compact(ray('title subtext proxy1 proxy2')) + ($mya->member ? [] : w\setupFoot());
  return cgform($form);
}

function formProxies_validate($form, &$sta) {
  if (db\count('r_proxies', 'person=:id', ['id' => r\acct()->id]) < 2) return say(t('You need to choose both a primary and an alternate proxy.'), 'err');
}

function formProxies_submit($form, &$sta) {
  return w\goNextStep('proxies');
}