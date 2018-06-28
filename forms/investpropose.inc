<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

define('INVEST_COFLDS', 'fullName city serviceArea dob gross'); // investment flds in users table
define('INVEST_FLDS', 'clubid investment offering price return types terms assets character strength web history soundness'); // investment flds in r_investments
define('INVEST_ALLFLDS', 'company ' . INVEST_COFLDS . ' bizCats ' . INVEST_FLDS);

/**
 * Propose a new Investment.
 */
function formInvestPropose($form, &$sta, $args = '') {
  extract(just('clubid', $args));
  if (!@$clubid) return w\go('/invest', t('bad clubid'));
  
  $title = t('Propose a new Investment');
  $subtext = t('Complete this form if you have been assigned by your Investment Club steering committee to research the soundness of a proposed investment. <%a>Here is a printable worksheet</a> you can use to gather the necessary information.', '_a', atag('http://cg4.us/docs/investment-worksheet.pdf'));
  $company = textFld(t('Company:'), [t('Company to invest in'), t('Choose a %PROJECT member company (or type its account ID) or <%a>click here</a> to add them.', '_a', 'a id="add-co"')]);
  $fullName = textFld(t('Company:'), [t('The company\'s full name')]);
  $city = textFld(t('City:'), [t('City where company is located')]);
  $serviceArea = textFld(t('Service area:'), [t('Company service area')]);
  $dob = textFld(t('Founded:'), [t('Founding date'), t('When did this company begin, approximately (mm/dd/yyyy)')]);
  $gross = textFld(t('Annual Gross:'), [t('Gross income')]);
  $bizCats = selectFld(t('Biz categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE], coCats());
  $investment = areaFld(t('Investment:'), [t('Description of investment')], required());
  $offering = textFld(t('Offering size:'), [t('Offering size'), t('Total size of offering, in dollars')], required());
  $price = textFld(t('Share price:'), [t('Price per share')], required());
  $return = textFld(t('Return:'), [t('Predicted APR')], required() + suffix('%'));
  $types = textFld(t('Types:'), [t('Type codes'), t('D=dividends I=interest T=tax-exempt interest')], required());
  $terms = areaFld(t('Terms:'), [t('Terms of the investment'), t('Minimum investment, minimum length of time, and any other details about the deal.')], required());
  $assets = textFld(t('Company assets:'), [t('Company assets'), t('Company assets, bond, or collateral (dollar value)')], required());
  $character = textFld(t('Owner character:'), [t('Character of owners'), t('What is your assessment of the integrity and determination of the owners')], required());
  $strength = textFld(t('Strength:'), [t('Financial strength'), t('Rate the company\'s financial strength, based on quality and content of financial statements (0 to 100)')], required());
  $web = textFld(t('Web presence:'), [t('Web presence'), t('How impressive does the company look on Internet buzz (0 to 100)')], required());
  $history = textFld(t('History:'), [t('Repayment history'), t('Rate the company\'s past success repaying loans and investments (0 to 100, where 50 means no history)')], required());
  $soundness = textFld(t('Soundness:'), [t('Overall soundness'), t('Overall, how sound is this investment (0-100)')], required());
  $clubid = hidFld($clubid);

  $submit = t('Propose');

  w\whoFldSubmit('company', t('Propose this investment in %name?'), ray('coOnly', TRUE));
  
  jsx('invest-proposal');
  
  return cgform(compact(ray('title subtext ' . INVEST_ALLFLDS . ' submit clubid')));
}

function formInvestPropose_validate($form, &$sta) {
  extract($info = just(INVEST_ALLFLDS, $sta['input']));

  if (@$company) {
    if (!$a = whois($company, 'company', $info, t('no self-investing'))) return FALSE;
    $company = $a->id;
  } else {
    if ($err = u\badDate($dob, '', time())) return say($err, 'dob');
    if ($err = u\badAmount($gross, '>=0', 2, '')) return sayFieldErr($err, 'gross');
  }
  $toRate = 'strength web history soundness';
  foreach (ray('offering price return assets ' . $toRate) as $k) {
    $max = in($$k, $toRate) ? 100 : ''; // '' not NULL (no limit)
    if ($err = u\badAmount($$k, '>=0', 2, $max)) return sayFieldErr($err, $k);
  }
  u\preray(compact(ray(INVEST_ALLFLDS)), $sta['input']); // update changed fields
}

function formInvestPropose_submit($form, &$sta) {
  global $mya;
  extract(just(INVEST_ALLFLDS, $sta['input']));
  
  if (@$fullName) {
    $a = new r\Acct(compact(ray(INVEST_COFLDS)));
    $a->setCategories($bizCats);
  } else $a = r\acct($company);

  list ($coid, $proposedBy) = [$a->id, $mya->id];
  db\insert('r_investments', compact(ray('coid proposedBy ' . INVEST_FLDS)));
  w\go("/invest/list/clubid=$clubid", t('Your proposed investment has been saved.'));
}
