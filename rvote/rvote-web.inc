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
 * In MailChimp, use this link for voting links or buttons: https://new.rcredits.org/do/doDemocracy~*|ID|*~*|Ecode|*~~
 
To get a list of proposal comments:

  $sql = <<<EOF
    SELECT p.id, p.project, CONCAT('(', v.option-11, ') ', v.text, IF(v.isVeto, ' VETO!', '')) AS notes FROM r_proposals p JOIN r_ballots b ON b.question=-p.id JOIN r_votes v ON v.ballot=b.id WHERE v.text <> '' AND v.id>2425 ORDER BY p.project, v.option
EOF;
  $q = f('db.q', $sql);
  $oldId = -1;
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($id != $oldId) {
      if ($oldId != -1) $res[] = "$name:\n$allNotes";
      $name = $project;
      $allNotes = $notes;
      $oldId = $id;
    } else $allNotes .= " $notes";
  }
  if (@$name) $res[] = "$name\n$allNotes";
  
///  debug(join("\n\n", $res));

For list of scorers:
SELECT fullName, Count(b.id) AS votes FROM `users` u INNER JOIN r_ballots b ON b.voter=u.uid LEFT JOIN r_votes v ON v.ballot=b.id WHERE b.question<0 GROUP BY uid

SELECT fullName, b.id, COUNT(v.id) as votes FROM `users` u INNER JOIN r_ballots b ON b.voter=u.uid LEFT JOIN r_votes v ON v.ballot=b.id GROUP BY b.id ORDER BY COUNT(v.id)
  
 */
function formDemocracy($form, &$sta, $args = '') {
  global $base_url;
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  include_once __DIR__ . '/../rvote/rvote-results.inc';
  
  extract(u\just('question do results noProxies', $args));

  $mya = w\eLinkAcct($sta, $args, 'acct'); // allow any signed in account, so user can switch to personal account, below
    
  if (!$mya->proSe) {
    $switch = r\changeAcctLink($mya->agentId, t('Switch to your personal account'));
    $msg = $mya->co ? t('Companies cannot vote.') : t('You cannot vote on behalf of someone else.');
    w\say("$msg|$switch.", $mya->superAdmin ? '' : 'err');
    if (!$mya->superAdmin) return compact('title');
  }
  
  $cttyA = $mya->cttyA;
  
  $title = w\item(t('Community Democracy'));
  $subtitle = w\item($cttyA->fullName);

  if (!@$do or $do == 'options' or $do == 'results') {  
    list($questionList, $phase, $endDate) = v\questionList($cttyA, @$question, @$do);
  }
  
  if ($mya->community == -26742000000002) {
    $middle = file_get_contents(__DIR__ . '/greenfieldma2017.html');
//    $middle = v\showProposalGrades($mya, $args); // FUDGE for 19 Jan 2017 (pending reworking the overall flow and logic of voting phases)
  } else if (@$phase == 'V') { // voting
    return v\votingWelcome($endDate);
  } elseif (@$phase == 'G') { // grading specific proposals within voted budget
    return v\gradingWelcome($endDate);
  } elseif (@$do == 'proposals') {
    
  } elseif (@$do == 'options' or @$do == 'results') {
    $middle = $questionlist;
  } elseif (@$do == 'ballot') {
    $middle = v\ballot($mya, $extraFields, @$question);
  } elseif (@$do == 'done') {
    $notes = db\col('DISTINCT text', 'r_votes v INNER JOIN r_ballots b ON v.ballot=b.id', "b.voter=:uid AND v.text<>''", ['uid' => $mya->id]);
    r\tellCO($mya->fullName . t(' has voted. '), $notes);
    $middle = v\done($mya);
    if (!r\acct()) return doSay($middle, 'status');
  } elseif (@$do == 'grade') {
    $middle = v\showProposal($mya, $extraFields, @$question, TRUE);
  } else $middle = t('There is no vote in progress in your community.');
//  } else $middle = t('There are no questions pending in your community.');
  
  $middle = w\item(@$middle);
  $args = w\hidFld($args);

  w\js('js/rvote/ballot.js', 'file', 'footer');
  w\css('css/rvote.css');
  
  return labeled(compact(ray('title subtitle showVotes middle args')) + (@$extraFields ?: []));
}

function formDemocracy_validate($form, &$sta) {
  extract(u\just('question agree op', $sta['input']));
  $mya = w\eLinkAcct($sta, $args);
  if (@$question < 0 and w\op($sta) == 'submit') {
    // for each option, if blank then error
  }
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
  
  if ($op == 'gradeIt') return r\go("prox/page=Democracy&do=grade&$args");
  if (@$agree) return r\go("prox/page=Democracy&do=ballot&$args");
  
  // process ballot submission
  if (@$question <= -1) {
    $type = 'M';
    $do = 'grade';
    $question = -bumpQnum(-$question, v\getProposals($mya), $op);
  } else {
    list($quids, $qutypes) = v\getQuestions($ctty); 
    $type = $qutypes[$question];
    $do = 'ballot';
    $question = bumpQnum($question, $quids, $op);
  }
  v\processvote($sta['input'], $type); // any errors are handled by javaScript (only the user suffers if no JS)

  if ($question == 'done') w\say(t('That was the last proposal. Thank you bigly for grading!'));
  if ($question != 'before') $args .= $question == 'done' ? '&do=done' : "&do=$do&question=$question";
  return r\go("prox/page=Democracy&$args"); 
}

/**
 * Show a single proposal report.
 */
function formProposalReport($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  extract(u\just('p', $args));
  
  $mya = w\eLinkAcct($sta, $args, 'acct'); // allow any signed in account, so user can switch to personal account, below

  if (!@$p or !$project = db\lookup('project', 'r_proposals', 'id=:p', compact('p'))) return r\go('empty', t('No such proposal.'));
  
  v\showProposal($mya, $extra, -$p);

  $form['title'] = w\item(t('Project Proposal Details'));
  $form['project'] = w\item($project, t('Project:'));
//  for ($i = 0; isset($extra["f$i"]); $i++) $form["f$i"] = $extra["f$i"];
  $form += $extra ?: [];
  $form['back'] = w\item('<p>' . u\backButton() . '</p>');

  w\css('css/rvote.css');

  return w\labeled($form);
}

/**
 * Show notes for a single proposal report.
 */
function formProposalNotes($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  extract(u\just('p', $args));

  if (!@$p or !$project = db\lookup('project', 'r_proposals', 'id=:p', compact('p'))) return r\go('empty', t('No such proposal.'));

  $notes = v\proposalComments($p) ?: [];
//  $notes = ['note1', 'note2'];
  $questions = ray('our priorities,doable,management,evaluation,doable with less');
  $list = '';
  foreach ($notes as $note) $list .= '<li>' . $note . '</li>';
  foreach ($questions as $i => $q) $map['(q' . ($i+1) . ')'] = "<b>$q?</b>";
  $list = strtr($list, $map);
  $list = w\item("<ul id=\"proposalNotesList\">$list</ul>"); // , t('Comments:')

  $title = w\item(t('Project Proposal Comments'));
  $project = w\item($project, t('Project:'));
  $back = w\item('<p>' . u\backButton() . '</p>');

  w\css('css/rvote.css');
  
  return w\labeled(compact(ray('title project list back')));
}

/**
 * Get next or previous question number.
 * @param int $question: current question number
 * @param array $quids: array of question numbers
 * @param string $op: submitted operation button
 * @return next question number ('done' if past last, 'before' if before first)
 */
function bumpQnum($question, $quids, $op) {
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
  if (@$op == 'prev') $qnum--; else $qnum++;
  return $qnum < 0 ? 'before' : ($qnum >= count($quids) ? 'done' : @$quids[$qnum]);
}

/**
 * Submit a community funding proposal.
 */
function formProposal($form, &$sta) {
  global $base_url;
  
  r\go('empty', t('We\'re very sorry. The deadline for submitting proposals has passed. Try again in the next funding cycle!'));
  
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
  js("jQuery('#critLink').click(function () {jQuery('#criteria').modal('show');});", 'inline', 'footer', 9999);

  $title = item(t('Community Funding Proposal'));
  $_aInfo = u\link(PROMO_URL);
  $_aCrit = 'a id="critLink" href="javascript:;"';
  
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
  return labeled(compact(ray('title subtitle project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor submitProposal critPop')));
} 
  
function formProposal_validate($form, &$sta) {
  extract(u\just('cats when until amount budgetTotal phone email', $sta['input']));
  if (!@$cats) return say(t('You must choose at least one category.'), 'categories');
  if ($err = u\badDate($when, time())) return sayFieldErr($err, 'when');
  if ($err = u\badDate($until, $when)) return sayFieldErr($err, 'until');
  foreach (['amount', 'budgetTotal'] as $k) if ($err = amtErr($k, $sta)) return sayFieldErr($err, $k);
  if (!phoneOk($phone)) return;
  if ($err = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
  u\preray(compact(ray('when until phone')), $sta['input']); // amounts get updated in $sta by amtErr()
}

function formProposal_submit($form, &$sta) {
  $input = $sta['input'];
  $dbFields = 'ctty project categories overview purpose where when until how amount type budgetTotal budget contingency qualifications evaluation name contact phone email sponsor';
  $mya = r\acct();
  $input['ctty'] = $mya->community;
  $input['categories'] = join(' ', array_keys($input['cats']));
  db\insert('r_proposals', $info = u\just($dbFields, $input));
  r\tellCO('funding proposal received', $info);
  r\go('community', t('Your funding application has been submitted! The process of evaluating proposals may take several weeks. You will hear from us whether or not your proposal is chosen for funding.'));
}