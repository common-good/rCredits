<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Buy or sell in investment club investment.
 */
function formInvestBuySell($form, &$sta, $args = '') {
  global $mya;
  extract(just('vestid', $args));
  if (!@$vestid) return r\go('/page-not-found');
  $q = db\q('SELECT i.coid, i.clubid, COUNT(shid) AS `count`, SUM(shares) AS shares, SUM(sell) AS sell FROM r_investments i LEFT JOIN r_shares s ON s.vestid=i.vestid WHERE i.vestid=:vestid', compact('vestid'));
  if ($res = $q->fetchAssoc()) {
    extract($res);
    $sell += 0; $shares += 0; // convert to numeric
  } else $sell = $shares = 0; // no shares yet (but we do have coid)
  
  $title = t('Change Club Investment #%vestid', compact('vestid'));
  
  $subtext = t(<<< X
  <p>Complete this form to increase or decrease the club's investment shares in %coName.</p>
  <p>Once you have finished all your club's planned selling and buying for this cycle, click <%a>Distribute Profits</a> here or on the main Investment page, to share the club's profits with club members and %PROJECT.</p>
X
    , 'coName _a', r\acct($coid)->fullName, atag('/invest/distribute'));

  $ifSelling = $shares ? t('If you reduce your shares, it will take effect when the company pays for them.') : '';
  $ifSellingMore = $sell ? t('  Whatever amount you sell here will be in addition to your current pending sales.') : '';
  if ($shares) $selling = item($sell, t('Pending sales:'));
  $buy = submi(t('Buy'), '', 'md');
  $sell = $shares ? submi(t('Sell'), '', 'md') : NULL;
  $submit = fieldSet('submit2', compact('buy', 'sell'), ' ');
  $ourShares = item($shares, t('Current shares:'));
  $shares = textFld($shares ? t('Buy or sell:') : t('Shares to buy:'), [t('Number of shares'), $ifSelling . $ifSellingMore], required());
  $old = hidFld(serialize(compact(ray('vestid coid clubid count'))));

  return cgform(compact(ray('title subtext ourShares selling shares submit old')));
}

function formInvestBuySell_validate($form, &$sta) {
  extract(just('shares', $sta['input']));
  if ($err = u\badAmount($shares, '>0', 0)) return say($err, 'shares');
  u\preray(compact('shares'), $sta['input']); // update with any format corrections
}

function formInvestBuySell_submit($form, &$sta) {
  extract(just('shares old', $sta['input']));
  extract(just('vestid coid clubid count', unserialize($old)));
  
  $price = db\lookup('price', 'r_investments', compact('vestid'));
  $total = round($shares * $price, 2);
  $sharesDpy = number_format($shares, 0);
  $priceDpy = u\fmtAmt($price);
  $clubA = r\acct($clubid);
  $coA = r\acct($coid);
  
  $DBTX = \db_transaction();
  if (op($sta) == 'buy') {
    list ($index, $subs) = $clubA->pay($coA, $total, t('investment'), ray('investment goods', 1, FOR_USD));
    $extraMsg = @$subs['success'] ? t('|You successfully purchased %amt shares of %co at %price.', 'amt co price', $sharesDpy, $coA->fullName, $priceDpy) : t('|No shares were purchased.');
    db\insert('r_shares', ray('vestid shares bought', $vestid, $shares, time()));
  } else { // sell
//    $shid = db\min('shid', 'r_shares', compact('vestid'));
    $for = t('reclaim investment: %amt at %price', 'amt price', $sharesDpy, $priceDpy);
    list ($index, $subs) = $clubA->invoice($coA, $total, $for, ray('goods', FOR_INVESTMENT));
//    if (@$count + 0 == 1) { // the usual case -- one shares record
    db\insert('r_shares', ray('vestid shares sell bought', $vestid, 0, $shares, time()));
  }
  unset($DBTX);

  say($index . @$extraMsg, $subs, @$subs['success'] ? NULL : 'err');
  return r\go("/invest/list/actual=1&clubid=$clubid");
}
