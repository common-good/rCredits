<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Web as w;
use rCredits\Vote as v;

/**
 * Provide infrastructure for members to discuss and vote.
 * @param string $args: miscellaneous parameters
 * @param string $qid: account QID when called from rdo() (from email link)
 */
function formDemocracy($form, &$sta, $args = '') {
  global $base_url;
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  include_once __DIR__ . '/../rvote/rvote-results.inc';
  
  extract(u\just('question do results noProxies', $args));

  $mya = w\eLinkAcct($sta, $args);
    
  $cttyA = $mya->cttyA;
  
  $title = w\item(t('Community Vote: ') . $cttyA->fullName);

  if ($mya->co) {
    $switch = r\changeAcctLink($mya->agentId, t('Switch to your personal account'));
    w\say(t('Companies cannot vote. ') . "$switch.", $mya->admin ? '' : 'err');
    if (!$mya->admin) return compact('title');
  }
  
  if (!@$do or $do == 'options' or $do == 'results') {  
    list($questionList, $voting) = v\questionList($cttyA, @$question, @$do);
  }
  $args = w\hidFld($args);
  
  if (@$voting) return v\votingWelcome();
  
  if (@$do == 'proposals') {
    
  } elseif (@$do == 'options' or @$do == 'results') {
    $middle = $questionlist;
  } elseif (@$do == 'ballot') {
    $middle = v\ballot($mya, $extraFields, @$question);
  } elseif (@$do == 'done') {
    $notes = db\col('DISTINCT text', 'r_votes v INNER JOIN r_ballots b ON v.ballot=b.id', "b.voter=:uid AND v.text<>''", ['uid' => $mya->id]);
    r\tellStaff($mya->fullName . t(' has voted. '), $notes);
    $middle = v\done($mya);
    if (!r\acct()) return doSay($middle, 'status');
  } else $middle = t('There is no vote in progress in your community.');
//  } else $middle = t('There are no questions pending in your community.');
  
  $middle = w\item(@$middle);

  js('js/rvote/ballot.js', 'file', 'footer');
  css('css/rvote.css');
  
  return labeled(compact(u\ray('title showVotes middle args')) + (@$extraFields ?: []));
}

function formDemocracy_submit($form, &$sta) {
  extract(u\just('question agree op', $sta['input']));
  
  $mya = w\eLinkAcct($sta, $args);
  $ctty = $mya->community;
  $op = (w\op($sta) == 'prev' and strpos($op, t('Previous')) === FALSE) ? 'doneWithQ' : w\op($sta);
  if ($op == 'showVotes') return w\say(v\showVotes($ctty));
  if (in_array($op, ['noDirect', 'noVote'])) return w\say(v\showNonVoters($ctty, $op == 'noDirect'));
  if ($op == 'results') return r\go("prox/page=Democracy&$args&results=1");
  if ($op == 'resultsNoProx') return r\go("prox/page=Democracy&$args&results=1&noProxies=1");
  
  if (@$agree) return r\go("prox/page=Democracy&do=ballot&$args");
  
  list($quids, $qutypes) = v\getQuestions($mya->community); 
  v\processvote($sta['input'], $qutypes[$question]); // any errors are handled by javaScript (only the user suffers if no JS)
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
///  [KEEP] if (w\op($sta) == 'submit') $qnum++; else $qnum--;
  if (@$op == 'prev') $qnum--; else $qnum++;

//  if ($qnum < 0) return say(t('Previous question has been deleted.')); // stay on current question
  $args .= $qnum < 0
  ? ''
  : ($qnum < count($quids) ? '&do=ballot&question=' . $quids[$qnum] : '&do=done');
  return r\go("prox/page=Democracy&$args"); 
}

/**
 * Submit a community funding proposal.
 */
function formProposal($form, &$sta) {
  global $base_url;
  
  $options = [
    'Renewable Energy' => 2366.69,
    'Small Business Development' => 2326.40,
    'Social Justice' => 2124.98,
    'Food Systems' => 1832.92,
    'The Arts' => 1420.01,
  ];
  $optionKeys = u\ray('energy business justice food arts');
  $criteria = [
    '' => 'Points',
    'How well does the project support our rCredits community funding priorities?' => 20,
    'How local is the applicant?' => 2.5,
    'How soon does the project begin?' => 2.5,
    'How soon does the project end?' => 2.5,
    'Overall, how clearly doable is the project?' => 20,
    'How close is the requested amount to the ideal ($1,000)?' => 5,
    'Type of funding (investments and loans are preferred).' => 5,
    'How close is the requested amount to the ideal (half the total project budget)?' => 5,
    'How close is the total amount raised/committed so far to the ideal (half the total project budget)?' => 5,
    'Ability to implement project with less funding than requested.' => 5,
    'Ability to manage the project and funds effectively.' => 15,
    'Evaluation plan.' => 10,
    'rCredits member sponsorship of the project.' => 2.5,
    ' ' => '____',
    'Total' => 100,
  ];

  foreach ($options as $k => $v) {
    $v = u\fmtAmt($v, '$');
//    $list[] = "<div class=\"row\"><div>$k</div><div>$v</div></div>\n";
    $list[] = "<li>$k</li>\n";
  }
//  $list = '<div class="table">' . join('', $list) . '</div>';
  $list = '<ul>' . join('', $list) . '</ul>';

  foreach ($criteria as $k => $v) $crit[] = <<<EOF
<div class="row"><div>$v</div><div>$k</div></div>\n
EOF;
  
  $crit = '<div id="critChart" class="table">' . join('', $crit) . '</div>';
  $critPop = item(w\modal('criteria', t('Evaluation Criteria for Funding Proposals'), '', $crit, 'X'));
  js("jQuery('#critLink').click(function () {jQuery('#criteria').modal('show');});", 'inline', 'footer', 9999);

  $title = item(t('Community Funding Proposal'));
  $subtitle = item(t(<<< EOF
<p>Complete this form by December 15, 2016, to submit a proposal for funding from the Greenfield Area rCredits Community. Requests can be made for up to $10,000 for projects in the following categories:</p>
!list
<p>Please keep your responses concise. Remember someone has to read and evaluate all the proposals. If you can answer a question in a single sentence, great! Limit each response to each question to no more than 5 short paragraphs, in any case. Proposals will be evaluated following <@aCrit>these criteria</a>.</p>
<p><@aInfo>Click here for more information about rCredits</a> &mdash; the free local credit/debit card with incentive rewards and no fees for buyers or sellers. Amazingly, the rCredits system generates funding for local community projects without costing anyone anything.</p>
<p>Questions? Email info@rcredits.org or call 413-628-1723.</p>
EOF
  , ['!list' => $list, '@aInfo' => 'a href=http://rCredits.org target=_blank', '@aCrit' => 'a id=critLink href=javascript:;']));
 
  $project = textFld(t('Project title:'), [t('Project Title')], required());
  $categories = boxFlds('cats', t('Categories'), t('What category or categories are most applicable to this project:'), array_combine($optionKeys, array_keys($options)), required());

  $overview = areaFld(t('What:'), [t('Project Description'), t('Describe your project briefly. What specific actions will be taken?')], required());
  $purpose = areaFld(t('Why:'), [t('Purpose of Project'), t('Describe briefly the Project\'s purpose (what need does it address?)')], required());
  $where = textFld(t('Where:'), [t('Project Zipcode'), t('Where will the project take place? (zipcode)')], required());
  $when = textFld(t('When:'), [t('Project Start Date'), t('When will the project begin?')], required());
  $until = textFld(t('Until:'), [t('Project End Date'), t('When will the project be completed?')], required());
  $how = textFld(t('How:'), [t('Implementation Plan'), t('Provide details of HOW your project will be implemented.')], required());
  $amount = textFld(t('Amount:'), [t('Amount to Fund'), t('How much funding are you proposing from the rCredits community?')], required());
  $typeDesc = <<<EOF
"Incentive" means a program of incentive rewards to rCredits members, to encourage a specific behavior; for example: voting, using alternative energy, or inviting people to join rCredits. If "Other", explain above under "What" (the project description).
EOF;
  $typeOptions = u\ray('Investment, Loan, Incentive, Grant, Other (explain)');
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
  $sponsor = textFld(t('Sponsor:'), [t('rCredits Member(s) Sponsoring this Funding Proposal'), t('Name(s) of rCredits member(s) sponsoring this funding proposal (or "self")')], required());
  $submitProposal = submit(t('Submit Proposal'));
  return labeled(compact(u\ray('title subtitle project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor submitProposal critPop')));
} 
  
function formProposal_validate($form, &$sta) {
  extract(u\just('cats when until amount budgetTotal phone email', $sta['input']));
  if (!@$cats) return say(t('You must choose at least one category.'), 'categories');
  if ($err = u\badDate($when, time())) return sayFieldErr($err, 'when');
  if ($err = u\badDate($until, $when)) return sayFieldErr($err, 'until');
  foreach (['amount', 'budgetTotal'] as $k) if ($err = amtErr($k, $sta)) return sayFieldErr($err, $k);
  if (!phoneOk($phone)) return;
  if ($err = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
  u\preray(compact(u\ray('when until phone')), $sta['input']); // amounts get updated in $sta by amtErr()
}

function formProposal_submit($form, &$sta) {
  $input = $sta['input'];
  $dbFields = 'ctty project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor';
  $mya = r\acct();
  $input['ctty'] = $mya->community;
  $input['categories'] = join(' ', array_keys($input['cats']));
  db\insert('r_proposals', $info = u\just($dbFields, $input));
  r\tellStaff('funding proposal received', $info);
  r\go('community', t('Your funding application has been submitted! The process of evaluating proposals may take several weeks. You will hear from us whether or not your proposal is chosen for funding.'));
}