<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Accept a grant application.
 */
function formGrant($form, &$sta) {
  global $mya;
  $title = t('Organizer Grant Application');
  $subtext = t('<p>Are you passionate about democratic economics and the potential of %PROJECT to create a society to benefit everyone? Do you have the skills to organize a group to begin the process of becoming a Common Good Community? Great! Common Good Finance has a grant for you.</p><p>Open a %PROJECT Account, form an organizing committee of 3 or more people, get one retail business and 10 other people to sign up, then fill out this form to request a grant of up to $5,000. Please also email us your resume, CV, or equivalent.</p><p>Remember the board may not be familiar with your work or your local community, so explain as needed.</p>', 'iReward', R_HELPER_BONUS);
  $did10 = boolFld(t('Got 10/1:'), t('Have you successfully recruited 10 %PROJECT members and at least 1 %PROJECT business?'));
  $biz = textFld(REQ . t('Biz:'), [t('Business name(s)'), t('Which business(es) did you recruit?')]);
  $amount = textFld(REQ . t('Request:'), [t('Requested grant amount'), t('How much grant funding are you requesting (up to $5,000)?')]);
  
  $class = ['class' => ['inline']];
  foreach ($gets = ray('_getOrganizers _getIndividuals _getCompanies') as $k) {
    $$k = w\rendA($k, textFld(BARE, [t('How many')], $class));
  }

  $_getPartners = w\rendA('getPartners', textFld(BARE, [t('Organization name(s)')], $class));
  $_getOther = w\rendA('getOther', textFld(BARE, [t('Describe')], $class));
  $_start = w\rendA('start', textFld(BARE, [t('Start date')], $class));
  $_end = w\rendA('end', textFld(BARE, [t('End date')], $class));

  $plan = item(t(<<<EOF
<p>What do you plan to accomplish with this funding and over what (near future) time period?<br>
<ul>
<li>Inspire %getOrganizers community members to meet regularly, to form a Common Good Community.</li>
<li>Recruit %getIndividuals more individuals to sign up.</li>
<li>Recruit %getCompanies more companies to sign up.</li>
<li>Develop partnerships with these organizations: %getPartners.</li>
<li>Other: %getOther</li>
<li>Grant period starts %start, ends %end.</li>
</ul>
EOF
  , compact(array_merge(ray('_getOrganizers _getIndividuals _getCompanies _getPartners _getOther _start _end'), $gets))), t('Plan:')); // + fails here (PHP bug)

//  foreach ($accountables = ray('meet report sayOther') as $k) $$k = w\rendA([$k => boxFld($k, BARE)]);
//  foreach ($accountables = ray('meet report sayOther') as $k) $$k = "<input type=\"checkbox\" id=\"edit-$k\" name=\"$k\" class=\"inline\" />";
  $_sayWhat = w\rendA('sayWhat', textFld(BARE, [t('Describe')], $class));
  $accountables = ray('meet report sayOther', t('Attend most weekly organizer conference calls? (We might ask you to participate in these meetings <i>before</i> we consider your grant request.)'), t('Report progress monthly by email?'), t('Other (work this out with %PROJECT staff): %sayWhat', compact('_sayWhat')));
  $accountable = w\boxFlds('crits', t('Reporting:'), t('How will you be accountable to %PROJECT and to other %PROJECT organizers?'), $accountables);
/*  $accountable = item(t(<<<EOF
<p>How will you be accountable to Common Good Finance and to other rCredits organizers?<br>
<ul>
<li>@meet Attend most weekly organizer conference calls? (We might ask you to participate in these meetings <i>before</i> we consider your grant request.)</li>
<li>@report Report progress monthly by email?</li>
<li>@sayOther Other (work this out with CGF staff): @sayWhat</li>
</ul>
EOF
  , compact($accountables) + compact('sayWhat')), t('Reporting:'));
  */
  $refs = textFld(REQ . t('References:'), [t('Work/Organizing references'), t('Please give at least two work or community organizing references (with contact information). We will also contact the business(es) you have recruited.')]);
  if ($mya) {
    $submit = t('Submit Grant Request');
  } else say(t('You must sign in, to apply for a grant.'));
  
  return cgform(compact(ray('title subtext did10 biz amount plan accountable refs submit')));

// CGF: ask for resume, references, and what do you think it will take to get 50 people and 5 retail businesses and some of their employees and suppliers signed up? what's your strategy?
}

function formGrant_submit($form, &$sta) {
  $aA = r\agent();
  $helperCount = db\count('users', ':IS_OK AND NOT :IS_CO AND helper=:myid', ['myid' => $aA->id]);
  extract($params = $sta['input'], EXTR_PREFIX_ALL, 'i');
  extract($i_crits, EXTR_PREFIX_ALL, 'i');
  foreach (ray('i_did10 i_meet i_report i_sayOther') as $k) $$k = @$$k ? t('YES') : t('NO');

  $th = 'th class="grant-th"';
  $td = 'td class="grant-td"';
  $msg = <<<EOF

<h4>GRANT APPLICATION</h4>

<table id="app" border="1" cellspacing="3" cellpadding="0">
<tr><$th>Name:</th><$td>$aA->fullName</td></tr>
<tr><$th>Amount requested:</th><$td>$i_amount</td></tr>
<tr><$th>At least 10 individuals recruited so far:</th><$td>$i_did10 (actual count: $helperCount)</td></tr>
<tr><$th>Businesses recruited so far:</th><$td>$i_biz</td></tr>
<tr><$th>Grant period:</th><$td>$i_start to $i_end</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$td colspan="2">What do you plan to accomplish with this funding?</td></tr>
<tr><$th>Inspire local organizers to meet regularly, to form a Common Good Community:</th><$td>$i_getOrganizers</td></tr>
<tr><$th>Recruit more individuals:</th><$td>$i_getIndividuals</td></tr>
<tr><$th>Recruit more companies:</th><$td>$i_getCompanies</td></tr>
<tr><$th>Develop partnerships with these organizations:</th><$td>$i_getPartners</td></tr>
<tr><$th>Other:</th><$td>$i_getOther</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$td colspan="2">How will you be accountable to Common Good and to other Common Good organizers?
<tr><$th>Attend most weekly organizer conference calls?</th><$td>$i_meet</td></tr>
<tr><$th>Report progress monthly by email?</th><$td>$i_report</td></tr>
<tr><$th>Other:</th><$td>$i_sayWhat</td></tr>
<tr><$td colspan="2">&nbsp;</td></tr>
<tr><$th>References:</th> <$td>$i_refs</td></tr>
</table>
EOF;

  r\tellAdmin('organizer grant request', $params + compact('msg'));
  return w\go('', t('Your grant request has been submitted! You will be hearing from %PROJECT within a week or two.'));
}
