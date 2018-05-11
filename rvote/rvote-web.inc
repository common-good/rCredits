<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

include_once __DIR__ . '/rvote.inc';
include_once __DIR__ . '/rvote-ballot.inc';
include_once __DIR__ . '/rvote-results.inc';

/**
 * Provide infrastructure for members to discuss and vote.
 * @param string $args: miscellaneous parameters
 * @param string $qid: account QID when called from rdo() (from email link)
 * In MailChimp, use this link for voting links or buttons: https://new.CommonGood.earth/do/doEvents~*|ID|*~*|Ecode|*~~
 
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
function formEvents($form, &$sta, $args = '') {
  global $base_url;
  
  extract(just('question do results noProxies', $args));

  $mya = w\eLinkAcct($sta, $args, r\acct() ? 'acct' : 'individual'); // allow any signed in account, so user can switch to personal account, below
  $cttyA = $mya->cttyA;

  $title = item(t('Community Democracy') . "<br><small>$cttyA->fullName</small>");
//  $subtitle = item($cttyA->fullName);

  if ($event = v\getCurrentEvent($cttyA->id)) {
    extract($event); // this overwrites $event with short name of event (for example "Voting on 3 questions")

    $end = u\fmtDate($end);
    $begun = ($start <= time()); // event has begun

    if (strpos(' VG', $type) and $begun) {
      if (!$mya->proSe or $mya->co) { // "or" is necessary if a voting link is accidentally sent to a company
        $switch = r\changeAcctLink($mya->agentId, t('Switch to your personal account'));
        $msg = $mya->co ? t('Companies cannot vote.') : t('You cannot vote on behalf of someone else.');
        w\say("$msg|$switch.", $mya->superAdmin ? '' : 'err');
        if (!$mya->superAdmin) return compact('title');
      }

      if (!@$do) $voting = $type == 'V' ? v\votingWelcome($end) : v\gradingWelcome($end); // end is displayable end date
    }

    $start = $start <= time() ? t('NOW') : u\fmtDate($start);
    $status0 = t('<b>Status:</b> %event from %start to %end', compact(ray('event start end')));
    if ($details) $status0 .= u\lf2br("\n\n$details");
  } else $status0 = t('There are no upcoming %PROJECT events scheduled in your community at this time.');
  
  if (!@$do) $status = item($status0);

  if (@$do == 'options' or @$do == 'results') {  
    list($questionList, $phase, $endDate) = v\questionList(@$eid, @$question, @$do);
  }
  
  if (FALSE and $mya->community == -26742000000002) {
    $middle = file_get_contents(__DIR__ . '/greenfieldma2017.html');
//    $middle = v\showProposalGrades($mya, $args); // FUDGE for 19 Jan 2017 (pending reworking the overall flow and logic of voting phases)
  } elseif (@$do == 'proposals') {
    
  } elseif (@$do == 'options' or @$do == 'results') {
    $middle = $questionlist;
  } elseif (@$do == 'ballot') {
    $middle = v\ballot($mya, $extraFields, @$question);
  } elseif (@$do == 'done' and @$eid) { // @todo: handle proposals too
    $table = "r_votes v INNER JOIN r_ballots b ON v.ballot=b.id INNER JOIN r_questions q ON q.id=b.question";
    $notes = db\col('DISTINCT v.text', $table, "q.event=:eid AND b.voter=:uid AND v.text<>''", ray('uid eid', $mya->id, $eid));
    r\tellCO($mya->fullName . t(' has voted. '), $notes);
    $middle = v\done($mya);
    if (!r\acct()) return doSay($middle, 'status');
  } elseif (@$do == 'grade') {
    $middle = v\showProposal($mya, $extraFields, @$question, TRUE);
  } else {
    $middle = t('Event History');
    if ($mya->cAdmin) $middle .= ' ' . button(t('Add'), "$base_url/community/events/event/", '', 'warning');
    $middle = "<h4>$middle</h4>";
    $sql = <<<EOF
      SELECT id, end, event FROM r_events ORDER BY end DESC, id DESC
EOF;
    $q = db\q($sql);
    $list = '';
    while ($row = $q->fetchAssoc()) {
      extract($row, EXTR_PREFIX_ALL, 'e');
      $buttons = $mya->cAdmin ? button(t('Edit'), "$base_url/community/events/event/$e_id", '', 'warning') : '';
      if ($e_end < time()) $buttons .= ' &nbsp; ' . button(t('Results'), "$base_url/community/events/event/$e_id/report", '', 'success');
      $e_end = u\fmtDate($e_end);
      $list .= <<<EOF
<tr><td>$e_end</td><td>$e_event</td><td>$buttons</td></tr>
EOF;
    }
    if ($list) $list = "<table><tr><th>Ends</th><th>Event</th></tr>\n$list</table>";
    $middle .= $list ?: t('There have not yet been any %PROJECT events in your community.');
  }
  
//  } else $middle = t('There is no vote in progress in your community.');
//  } else $middle = t('There are no questions pending in your community.');
  
  $middle = item(@$middle);
  $args = hidFld($args);

  w\js('ballot');
  w\css('rvote.css');
  
  return cgform(compact(ray('args title subtitle status showVotes')) + (@$voting ?: []) + compact('middle') + (@$extraFields ?: []));
}

function formEvents_validate($form, &$sta) {
  extract(just('question agree op', $sta['input']));
  $mya = w\eLinkAcct($sta, $args);
  if (@$question < 0 and w\op($sta) == 'submit') {
    // for each option, if blank then error
  }
}

function formEvents_submit($form, &$sta) {
  extract(just('question agree op', $sta['input']));
  $mya = w\eLinkAcct($sta, $args);
  $ctty = $mya->community;

  $op = (w\op($sta) == 'prev' and strpos($op, t('Previous')) === FALSE) ? 'doneWithQ' : w\op($sta);
  if ($op == 'showVotes') return w\say(v\showVotes($ctty));
  if (in_array($op, ['noDirect', 'noVote'])) return w\say(v\showNonVoters($ctty, $op == 'noDirect'));
  if ($op == 'results') return r\go("prox/page=Events&$args&results=1");
  if ($op == 'resultsNoProx') return r\go("prox/page=Events&$args&results=1&noProxies=1");
  
  if ($op == 'gradeIt') return r\go("prox/page=Events&do=grade&$args");
  if (@$agree) return r\go("prox/page=Events&do=ballot&$args");
  
  // process ballot submission
  if (@$question <= -1) {
    $type = 'M';
    $do = 'grade';
    $question = -bumpQnum(-$question, v\getProposals($mya), $op);
  } else {
    list($quids, $qutypes) = v\getQuestions('', $mya); 
    $type = $qutypes[$question];
    $do = 'ballot';
    $question = bumpQnum($question, $quids, $op);
  }
  v\processvote($sta['input'], $type); // any errors are handled by javaScript (only the user suffers if no JS)

  if ($question == 'done') w\say(t('That was the last proposal. Thank you bigly for grading!'));
  if ($question != 'before') $args .= $question == 'done' ? '&do=done' : "&do=$do&question=$question";
  return r\go("prox/page=Events&$args"); 
}

/**
 * Add or Edit an event
 * @param int $eid: event record ID: empty means Add, else do $do (defaults to edit)
 
 */
function formEvent($form, &$sta, $eid = NULL, $do = 'edit') {
  include_once __DIR__ . '/rvote.inc';
  global $base_url, $mya;

  $typeDescs = ray(t('I:Meeting,V:Vote,G:Grading,P:RFP'));
  if (strpos($eid, '=')) extract(just('eid do', $eid)); // passed from rdo()
  
  if (isset($eid)) {
    $fields = 'type,event,details,start,end';
    if (is_numeric($eid) and $event = v\event($fields, $eid)) {
      extract($event); 
      $subtitle = item(t('for vote ending ') . u\fmtDate($end));
    } else $do = 'add';
  } else $do = 'add';

  
  if ($do == 'report') {
    include_once __DIR__ . '/rvote-results.inc';
    $title = item($typeDescs[$type] . t(' Results'));
    if (strpos('.VG', $type)) {
      $results = v\showResults($eid);
    } else {
      if ($results = @file_get_contents(DRUPAL_ROOT . "/reports/$eid")) {
        require_once DRUPAL_ROOT . '/vendor/htmlpurifier/library/HTMLPurifier.auto.php';
        $purifier = new HTMLPurifier();
        $results = $purifier->purify($results);    
      } else $results = t('There is no report yet for this event.');
    }
    $results = item($results);
    return compact(ray('title subtitle results'));
  } elseif ($do == 'details') {



  
  } elseif (!$mya->cAdmin) return w\hack('event permission');

  if ($do == 'edit') {
    if (strpos(' VG', $type)) { // must precede setting $type field
      list ($header, $page) = $type == 'V' ? [t('Questions'), 'question'] : [t('Proposals'), 'proposal'];
      if ($mya->cAdmin) $header .= ' ' . button(t('Add'), "$base_url/community/events/$page/eid=$eid", '', 'warning');

      $q = db\q('SELECT id AS qid,text,detail,type,budget FROM r_questions WHERE event=:eid ORDER BY id', compact('eid'));
      while ($row = $q->fetchAssoc()) {
        extract($row, EXTR_PREFIX_ALL, 'q');
        $budget = $q_budget ? u\fmtAmt($q_budget) : '-';
        $buttons = $mya->cAdmin ? button(t('Edit'), "$base_url/community/events/$page/qid=$q_qid", '', 'warning') : '';
        $list[] = <<<EOF
<tr><td>$q_text</td><td>$q_type</td><td>$budget</td><td>$buttons</td></tr>
EOF;
      }
      if (@$list) {
        $list = join("\n", $list);
        $list = <<<EOF
<table>
<tr><th>Question</th><th>Type</th><th>Budget</th></tr>
$list
</table>
EOF;
      } else $list = t('no questions yet');
      
      $list = item('<h4>' . $header . '</h4>' . $list);
    }
    $id = hidFld($eid);
  }
  
  // add or edit
  $title = item($do == 'add' ? t('Add Event') : t('Edit Event'));
  $types = ray(' :Choose Type,I:In-person,V:Vote,G:Grade Proposals,P:Request For Proposals');
  $type = selectFld(t('Type:'), '', required(@$type), $types);
  $event = textFld(t('Event name:'), [t('Short Descriptive Event Name'), t('For example, "Voting on 3 questions".')], required(@$event));
  $details = areaFld(t('Details:'), [t('Event Details')], dft(@$details));
  $start = textFld(t('Start:'), [t('Start Date')], required(@$start ? u\fmtDate($start, TRUE) : ''));
  $end = textFld(t('End:'), [t('End Date')], required(@$end ? u\fmtDate($end, TRUE) : ''));
  if ($mya->cAdmin) $submit = submit();
  
  return cgform(compact(ray('title id type event details start end submit list')));
}

function formEvent_validate($form, &$sta) {
  extract(just('type start end', $sta['input']));
  if (w\blank_field(compact('type'))) return NULL;
  foreach (['start', 'end'] as $k) if ($err = u\badDate($$k, strtotime('-1 day'), strtotime('+1 year'))) return w\say($err, $k);
  u\preray(compact('start', 'end'), $sta['input']);
}

function formEvent_submit($form, &$sta) {
  global $base_url;
  extract($info = just('id type event details start end', $sta['input']));
  global $mya;
  $info['end'] = strtotime('+1 day', $end) - 1; // just before the start of the next day
  db\update('r_events', $info + ['ctty' => $mya->community], @$id ? 'id' : '', TRUE);
  r\go('community/events', 'info saved');
}

/**
 * Add or edit a question.
 */
function formQuestion($form, &$sta, $args = '') {
  global $base_url;
  
  extract(just('eid qid', $args));
  
  global $mya;
  if (!$add = empty($qid)) {
    $fields = 'e.id AS eid, e.ctty, q.text, q.detail, q.linkDiscussion, q.type, q.units, q.budget, q.optOrder';
    if (is_numeric($qid) and $question = db\lookup($fields, 'r_questions q LEFT JOIN r_events e ON e.id=q.event', 'q.id=:qid', compact('qid'))) extract($question); else $add = FALSE;
    if ($ctty != $mya->community) return r\go('', t('Wrong community.'));

    $id = hidFld($qid);

    $header = t('Options');
    if ($mya->cAdmin) $header .= ' ' . button(t('Add'), "$base_url/community/events/option/qid=$qid", '', 'warning');
    
    $q = db\q('SELECT id AS oid,text,detail FROM r_options WHERE question=:qid ORDER BY id', compact('qid'));
    $list = '';
    while ($row = $q->fetchAssoc()) {
      extract($row, EXTR_PREFIX_ALL, 'o');
      $buttons = $mya->cAdmin ? button(t('Edit'), "$base_url/community/events/option/oid=$o_oid", '', 'warning') : '';
      $list .= "<tr><td>$o_text</td><td>$buttons</td></tr>";
    }
    if ($list) {
      $list = "<table><tr><th>Option</th></tr>$list</table>";
    } else $list = t('no options yet');
    
    $list = item('<h4>' . $header . '</h4>' . $list);    
  } elseif (empty($eid)) return r\go('', t('Missing event id.'));

  $title = item($add ? t('Add Question') : t('Edit Question'));
  $event = hidFld($eid);
  $text = textFld(t('Question:'), [t('Question Text')], required(@$text));
  $detail = areaFld(t('Detail:'), [t('Question Detail')], dft(@$detail));
  $linkDiscussion = textFld(t('Discussion Link:'), [t('Link to Discussion')], dft(@$linkDiscussion));
  $types = ray(' :Choose Type,M:Multiple choice,B:Budget (penny vote among various options),R:Range,E:Essay');
  $type = selectFld(t('Type:'), '', required(@$type), $types);
//  $units = textFld(t('Units:'), [t('Units'), t('Units for Budget or Range question. Defaults to dollars.'), dft(@$units));
  $budget = textFld(t('Budget:'), [t('Budget Amount'), t('If this is a Budget question, how much (money) is to be budgeted.')], dft(@$budget));
  $optOrders = ray(' :Choose Option Order,S:Shuffle (randomize),Z:Reverse sometimes,N:No change (static)');
  $optOrder = selectFld(t('Option order:'), ['', t('In a Multiple choic or Budget question, how to scramble the options, if at all')], dft(@$optOrder), $optOrders);
  
  if ($mya->cAdmin) $submit = submit();
  
  return cgform(compact(ray('title id event text detail linkDiscussion type budget optOrder submit list')));
}

function formQuestion_validate($form, &$sta) {
  extract(just('type budget optOrder', $sta['input']));
  if ($type == 'M' or $type == 'B') foreach (['type', 'optOrder'] as $k) if (w\blank_field(compact($k))) return NULL;
  if (!@$budget) $budget = 0;
  if ($err = u\badAmount($budget)) return say($err, 'budget');
  u\preray(compact('budget'), $sta['input']);
}

function formQuestion_submit($form, &$sta) {
  global $base_url;
  extract($info = just('id event text detail linkDiscussion type budget optOrder', $sta['input']));
  db\update('r_questions', $info, @$id ? 'id' : '', TRUE);
  r\go("community/events/event/$event", 'info saved');
}

/**
 * Add or edit a question.
 */
function formOption($form, &$sta, $args = '') {
  global $base_url;

  extract(just('qid oid', $args));
  
  global $mya;
  if (!$add = empty($oid)) {
    $fields = 'e.id AS eid, e.ctty, q.id AS qid, o.text, o.detail';
    if (is_numeric($oid) and $option = db\lookup($fields, 'r_options o LEFT JOIN r_questions q ON q.id=o.question LEFT JOIN r_events e ON e.id=q.event', 'o.id=:oid', compact('oid'))) extract($option); else $add = FALSE;
    if ($ctty != $mya->community) return r\go('', t('Wrong community.'));

    $id = hidFld($oid);
  } elseif (empty($qid)) return r\go('', t('Missing question id.'));

  $title = item($add ? t('Add Option') : t('Edit Option'));
  $question = hidFld($qid);
  $text = textFld(t('Option:'), [t('Option Text')], required(@$text));
  $detail = areaFld(t('Detail:'), [t('Option Detail')], dft(@$detail));
  
  if ($mya->cAdmin) $submit = submit();
  
  return cgform(compact(ray('title id question text detail submit')));
}

function formOption_validate($form, &$sta) {
}

function formOption_submit($form, &$sta) {
  global $base_url;
  extract($info = just('id question text detail', $sta['input']));
  db\update('r_options', $info, @$id ? 'id' : '', TRUE);
  r\go("community/events/question/qid=$question", 'info saved');
}

/**
 * Show a single proposal report.
 */
function formProposalReport($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  extract(just('p', $args));
  
  $mya = w\eLinkAcct($sta, $args, 'acct'); // allow any signed in account, so user can switch to personal account, below

  if (!@$p or !$project = db\lookup('project', 'r_proposals', 'id=:p', compact('p'))) return r\go('empty', t('No such proposal.'));
  
  v\showProposal($mya, $extra, -$p);

  $form['title'] = item(t('Project Proposal Details'));
  $form['project'] = item($project, t('Project:'));
//  for ($i = 0; isset($extra["f$i"]); $i++) $form["f$i"] = $extra["f$i"];
  $form += $extra ?: [];
  $form['back'] = item('<p>' . backButton() . '</p>');

  w\css('rvote.css');

  return cgform($form);
}

/**
 * Show notes for a single proposal report.
 */
function formProposalNotes($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  extract(just('p', $args));

  if (!@$p or !$project = db\lookup('project', 'r_proposals', 'id=:p', compact('p'))) return r\go('empty', t('No such proposal.'));

  $notes = v\proposalComments($p) ?: [];
//  $notes = ['note1', 'note2'];
  $questions = ray('our priorities,doable,management,evaluation,doable with less');
  $list = '';
  foreach ($notes as $note) $list .= '<li>' . $note . '</li>';
  foreach ($questions as $i => $q) $map['(q' . ($i+1) . ')'] = "<b>$q?</b>";
  $list = strtr($list, $map);
  $list = item("<ul id=\"proposalNotesList\">$list</ul>"); // , t('Comments:')

  $title = item(t('Project Proposal Comments'));
  $project = item($project, t('Project:'));
  $back = item('<p>' . backButton() . '</p>');

  w\css('rvote.css');
  
  return cgform(compact(ray('title project list back')));
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
  r\go('community', t('Your funding application has been submitted! The process of evaluating proposals may take several weeks. You will hear from us whether or not your proposal is chosen for funding.'));
}