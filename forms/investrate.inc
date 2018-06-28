<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * View or rate an investment.
 */
function formInvestRate($form, &$sta, $args = '') {
  global $mya;
  extract(just('vestid clubid', $args));
  if (!@$vestid or !@$clubid) return w\go('/page-not-found');
  $title = t('View or Rate Investment #%vestid', compact('vestid'));
  $subtext = t('Numeric ratings are on a scale of 0-100');

  $res = db\get('*', 'r_investments', compact('vestid'));
  extract($res);
  $coA = r\acct($coid);

  $ks = ray('company city area dob gross desc');
  $fields = ray('investment offering price return terms assets character strength web history soundness');
  $ds = ray(t('Company, City, Service area, Founded, Annual gross, Co description, Investment, Offering size, Share price, Predicted return, Terms, Company assets, Owner character, Financial strength, Web presence, Repayment history, Overall soundness'));
  $ds = array_combine(array_merge($ks, $fields), $ds);

  $vs = [$coA->fullName, $coA->city, $coA->serviceArea, u\fmtDate($coA->dob, TRUE), $coA->gross, $coA->coDesc()];
  $vs = array_combine($ks, $vs) + just($fields, $res, TRUE); 

  foreach (ray('gross assets offering') as $k) $vs[$k] = u\fmtAmt($vs[$k]);
  foreach ($vs as $k => $v) $ds[$k] = item($vs[$k], $ds[$k] . ':');

  if ($shares = db\get('shares', 'r_shares', compact('vestid'))) { // already invested
    $value = item(u\fmtAmt($shares * $price), t('Value:'));
    $shares = item(u\fmtAmt($shares), t('Club\'s stake:'));
  } elseif ($good = db\get('good', 'r_ratings', ray('vestid uid', $vestid, $mya->id))) { // rated
    $good = item($good, t('Your rating:'));
  } else { // not rated yet
    $good = textFld(t('Common Goodness:'), [t('Common Goodness'), t('Rate this investment\'s benefit to the community and the common good (0-100)')], required());
    $patronage = textFld(t('Your patronage:'), [t('Monthly spending'), t('How much you will probably spend monthly, paying this company for its new or expanded services (estimate)')], required());
    $comments = areaFld(t('Comments:'), [t('Comments')]);
    $submit = t('Rate it');
  }

  $buttons = backButton();
  if ($mya->icAdmin or $mya->cAdmin) $buttons .= btn("/invest/buy-or-sell/vestid=$vestid", t('Buy or Sell This'), 'warning');
  $vestid = hidFld($vestid);
  $clubid = hidFld($clubid);
  
  return cgform(compact(ray('title subtext buttons vestid')) + $ds + compact(ray('stake value good patronage comments submit clubid')));
}

function formInvestRate_validate($form, &$sta) {
  extract(just($fields = 'good patronage', $sta['input']));
  if ($err = u\badAmount($good, '>=0', 0, 100)) return sayFieldErr($err, 'good');
  if ($err = u\badAmount($patronage, '>=0', 2, '')) return sayFieldErr($err, 'patronage');
  u\preRay(compact(ray($fields)), $sta['input']);
}

function formInvestRate_submit($form, &$sta) {
  global $mya;
  
  $vs = just('vestid good patronage comments', $sta['input']);
  db\insert('r_ratings', $vs + ray('uid', $mya->id));
  $clubid = $sta['input']['clubid'];
  return w\go("/invest/list/clubid=$clubid", t('You have successfully rated investment #%vestid', just('vestid', $vs)));
}
