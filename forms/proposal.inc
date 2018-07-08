<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

include_once R_ROOT . '/rvote/rvote.inc';
include_once R_ROOT . '/rvote/rvote-ballot.inc';
include_once R_ROOT . '/rvote/rvote-results.inc';

/**
 * Submit a community funding proposal.
 */
function formProposal($form, &$sta) {
  global $base_url;
  
  w\softErr(t('We\'re very sorry. The deadline for submitting proposals has passed. Try again in the next funding cycle!'));
  
  $options = [
    t('Renewable Energy') => 2366.69,
    t('Small Business Development') => 2326.40,
    t('Social Justice') => 2124.98,
    t('Food Systems') => 1832.92,
    t('The Arts') => 1420.01,
  ];
  $optionKeys = ray(t('energy business justice food arts'));
  $criteria = [
    '' => t('Points'),
    t('How well does the project support our %PROJECT community funding priorities?') => 20,
    t('Overall, how clearly doable is the project?') => 20,
    t('Ability to manage the project and funds effectively.') => 15,
    t('Evaluation plan.') => 10,
    t('Type of funding (investments and loans are preferred).') => 5,
    t('How close is the requested amount to the ideal amount ($1,000)?') => 5,
    t('How close is the requested amount to the ideal percentage (half the total project budget)?') => 5,
    t('How close is the total amount raised/committed so far to the ideal (half the total project budget)?') => 5,
    t('Ability to implement project with less funding than requested.') => 5,
    t('How soon does the project begin?') => 2.5,
    t('How soon does the project end?') => 2.5,
    t('How local is the applicant?') => 2.5,
    t('%PROJECT member sponsorship of the project.') => 2.5,
    ' ' => '____',
    t('Total') => 100,
  ];

  foreach ($options as $k => $v) {
    $v = u\fmtAmt($v, '$');
//    $list[] = "<div class=\"row\"><div>$k</div><div>$v</div></div>\n";
    $_list[] = "<li>$k</li>\n";
  }
//  $_list = '<div class="table">' . join('', $_list) . '</div>';
  $_list = '<ul>' . join('', $_list) . '</ul>';

  foreach ($criteria as $k => $v) $crit[] = <<<EOF
<div class="row"><div>$v</div><div>$k</div></div>\n
EOF;
  
  $crit = '<div id="critChart" class="table">' . join('', $crit) . '</div>';
  $critPop = item(w\modal('criteria', t('Evaluation Criteria for Funding Proposals'), '', $crit, 'X'));
  jsx('funding-criteria');

  $title = item(t('Community Funding Proposal'));
  $_aInfo = u\link(PROMO_URL);
  $_aCrit = 'a id="critLink"'; //  href="javascript:;
  
  $subtitle = item(t(<<< EOF
<p>Complete this form by December 15, 2016, to submit a proposal for funding from the Greenfield Area %PROJECT Community. Requests can be made for up to $10,000 for projects in the following categories:</p>
%list
<p>Please keep your responses concise. Remember someone has to read and evaluate all the proposals. If you can answer a question in a single sentence, great! Limit each response to each question to no more than 5 short paragraphs, in any case. Proposals will be evaluated following <%aCrit>these criteria</a>.</p>
<p><%aInfo>Click here for more information about %PROJECT</a> &mdash; the free local credit/debit card with incentive rewards and no fees for buyers or sellers. Amazingly, the %PROJECT system generates funding for local community projects without costing anyone anything.</p>
<p>Questions? Email %CGF_EMAIL or call %CGF_PHONE.</p>
EOF
  , compact(ray('_list _aInfo _aCrit'))));
 
  $project = textFld(t('Project title:'), [t('Project Title')], required());
  $categories = boxFlds('cats', t('Categories'), t('What category or categories are most applicable to this project:'), array_combine($optionKeys, array_keys($options)), required());

  $overview = areaFld(t('What:'), [t('Project Description'), t('Describe your project briefly. What specific actions will be taken?')], required());
  $purpose = areaFld(t('Why:'), [t('Purpose of Project'), t('Describe briefly the Project\'s purpose (what need does it address?)')], required());
  $where = textFld(t('Where:'), [t('Project Zipcode'), t('Where will the project take place? (zipcode)')], required());
  $when = textFld(t('When:'), [t('Project Start Date'), t('When will the project begin?')], required());
  $until = textFld(t('Until:'), [t('Project End Date'), t('When will the project be completed?')], required());
  $how = areaFld(t('How:'), [t('Implementation Plan'), t('Provide details of HOW your project will be implemented.')], required());
  $amount = textFld(t('Amount:'), [t('Amount to Fund'), t('How much funding are you proposing from the %PROJECT community?')], required());
  $typeDesc = <<<EOF
"Incentive" means a program of incentive rewards to %PROJECT members, to encourage a specific behavior; for example: voting, using alternative energy, or inviting people to join %PROJECT. If "Other", explain above under "What" (the project description).
EOF;
  $typeOptions = ray('Investment, Loan, Incentive, Grant, Other (explain)');
  $type = radiosFld(t('Type:'), $typeDesc, required(), $typeOptions);
  $budgetTotal = textFld(t('Budget:'), [t('Total Project Budget'), t('What is the Total Project Expense Budget? Type or paste a complete Income and Expense budget below. Include your top 3 sources and amounts of committed funds. Please do not show subtotals.')], required());
  $budget = areaFld('', [t('Itemized Income and Expense Budget')], required());
  //   $budget = htmlFld('', [t('Itemized Income and Expense Budget')], '', required());
  $contingency = areaFld(t('Contingency:'), [t('Contingency Plan, for Partial Funding'), t('Funds are limited, and projects may not receive the full amount requested. If you receive less than the amount requested, how will that impact your ability to conduct the project?')], required());
  $qualifications = areaFld(t('Qualifications:'), [t('Your Skills, Experience, and Qualities'), t('What specific skills, experience, or qualities of yours assure successful completion of the project?')], required());
  $evaluation = areaFld(t('Evaluation:'), [t('Evaluation Plan'), t('How will you measure and evaluate the project\'s success?')], required());

  $name = textFld(t('Applicant:'), [t('Applicant Name'), t('What is the applicant\'s full name (properly capitalized)?')], required());
  $contact = textFld(t('Contact:'), [t('Contact Name'), t('If you are applying on behalf of an organization or another individual, type their full name here. Otherwise type "self".')], required());
  $phone = textFld(t('Phone:'), [t('Contact Telephone Number')], required());
  $email = textFld(t('Email:'), [t('Contact Email Address')], required());
  $sponsor = textFld(t('Sponsor:'), [t('%PROJECT Member(s) Sponsoring this Funding Proposal'), t('Name(s) of %PROJECT member(s) sponsoring this funding proposal (or "self")')], required());
  $submitProposal = submit(t('Submit Proposal'));
  return cgform(compact(ray('title subtitle project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor submitProposal critPop')));
} 
  
function formProposal_validate($form, &$sta) {
  extract(just('cats when until amount budgetTotal phone email', $sta['input']));
  if (!@$cats) return say(t('You must choose at least one category.'), 'categories');
  if ($err = u\badDate($when, time())) return sayFieldErr($err, 'when');
  if ($err = u\badDate($until, $when)) return sayFieldErr($err, 'until');
  foreach (['amount', 'budgetTotal'] as $k) if ($err = amtErr($k, $sta)) return sayFieldErr($err, $k);
  if (!phoneOk($phone)) return;
  if (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
  u\preray(compact(ray('when until phone')), $sta['input']); // amounts get updated in $sta by amtErr()
}

function formProposal_submit($form, &$sta) {
  $input = $sta['input'];
  $dbFields = 'ctty project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor';
  global $mya;
  $input['ctty'] = $mya->community;
  $input['categories'] = join(' ', array_keys($input['cats']));
  db\insert('r_proposals', $info = just($dbFields, $input));
  r\tellCO('funding proposal received', $info);
  w\go('community', t('Your funding application has been submitted! The process of evaluating proposals may take several weeks. You will hear from us whether or not your proposal is chosen for funding.'));
}