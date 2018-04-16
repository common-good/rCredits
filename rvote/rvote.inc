<?php
namespace rCredits\Vote;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Web as w;
use rCredits\Vote as v;

define('ISDEMO', FALSE);
define('GRADELTRS', 'EDCBA');
define('VETOGRADE', -2);
define('BLANKGRADE', -1); // means voter did nothing with this option (so don't count it)
define('BPCT2INT', 10); // what to multiply budget pct field by, to get an integer (how many decimal places in the percentage)
define('FUNDED', 'funded');

/**
 * @file
 * rVote subroutines
 */
 
/* @todo:
page selection (when too many questions)
caching results for each question (separate from question table)
dropdown selector for year or page (when there are a lot of questions)
*/


/**
 * Return a list of questions being considered by the member's community.
 * Provide details and links as appropriate to the current phase.
 * @param acct $eid: the event record ID
 * @param int $question: (optional) id of question member is focusing on
 * @param string $doWhat: (optional) what type of specifics to show (options or results)
 * @return [list, phase, endDate]
 *    list     array of questions for this community
 *    phase    what phase is happening now (G=grading V=voting ''=?)
 *    endDate  when this phase ends
 */
function questionList($eid, $question='', $doWhat='') {
  global $base_url;
	global $voting;

//	$q = db\q("SELECT id, (CASE WHEN (endGrading<>0 AND :now>endVoting) then 'G' WHEN :now>endVoting THEN 'R' WHEN :now>endProposals THEN 'V' WHEN :now>endIdeas THEN 'P' ELSE 'I' END) AS phase, repeats, repeatedBy, text, detail,  linkIdeas, linkProposals, created, (CASE WHEN endVoting=0 THEN 0 WHEN (endGrading<>0 AND :now>endVoting) THEN endGrading WHEN :now>endVoting THEN endVoting-:now WHEN :now>endProposals THEN endVoting WHEN :now>endIdeas THEN endProposals ELSE endIdeas END) AS endDate, budget, units FROM r_questions WHERE ctty IN (:ctty, 0) AND endIdeas<>0 ORDER BY endDate, id", ray('ctty now', $cttyA->id, $now = time()));

	$q = db\q("SELECT id, (CASE WHEN (endGrading<>0 AND :now>endVoting) then 'G' WHEN :now>endVoting THEN 'R' WHEN :now>endProposals THEN 'V' WHEN :now>endIdeas THEN 'P' ELSE 'I' END) AS phase, repeats, repeatedBy, text, detail,  linkIdeas, linkProposals, created, (CASE WHEN endVoting=0 THEN 0 WHEN (endGrading<>0 AND :now>endVoting) THEN endGrading WHEN :now>endVoting THEN endVoting-:now WHEN :now>endProposals THEN endVoting WHEN :now>endIdeas THEN endProposals ELSE endIdeas END) AS endDate, budget, units FROM r_questions WHERE e.id=:eid AND endIdeas<>0 ORDER BY endDate, id", ray('ctty now', $cttyA->id, $now = time()));

	$voting = false;
	$qdivs = $list = '';
	while($row = $q->fetchAssoc()) {
		extract($row); // get parms
		if($endDate < 0) $endDate = $now - $endDate; // correct for reverse sorting of closed votes (negative means number of days ago)

		if($phase == 'V') $voting = ($voting and $endDate) ? min($voting, $endDate) : $endDate; // report soonest closing date
		$endDatedpy = $endDate ? u\fmtDate($endDate) : '?';

		$budget = $budget ? (' (total budget: ' . ($units ? '' : '$') . number_format($budget, 0) . ($units ? " $units)" : ')')) : '';
		$phaselist = phaseDpy($id, $phase, $repeats, $repeatedBy, $linkIdeas, $linkProposals);
// <div class="details" onclick="$('#qdetails' + $id).toggle();">

		$list .= <<<EOF
<div class="qline clearfix">
<div class="phase">$phaselist</div phase>
<div class="details" data-toggle="collapse" data-target="#qdetails$id">
	<div class="id">$id</div id>
	<div class="text">$text$budget</div text>
	<div class="date">$endDatedpy</div date>
</div details>
</div qline>\n
EOF;

    $see = "<a href=\"$linkIdeas\">Ideas</a>";
    if(strpos('.PVR', $phase)) $see .= " | <a href=\"$linkProposals\">Proposals</a>";
    if(strpos('.VR', $phase)) $see .= " | <a  href=\"$base_url/community/democracy/question=$id&do=options\">Options</a>";
    if($phase == 'R') $see .= " | <a  href=\"$base_url/community/democracy/question=$id&do=results\">Results</a>";
    if($endDatedpy == '?') $endDatedpy = 'not set';

    if($id == $question) {
      $qstyle = 'blockish';
      $opts = ($doWhat == 'options') ? getOpts($id) : '';
      $results = ($doWhat == 'results' or true) ? getresults($id) : '';
    } else $qstyle = $opts = $results = '';

  //<div class='ballot' style='border:0;'>
    $qdivs .= <<<EOF
<div id="qdetails$id" class="ballot qdetails$qstyle">
<div class='question'><b>Question #$id:</b> $text$budget</div question>
<div class='see'><b>See:</b> $see</div see>
<div class='date'><b>Ending date:</b> $endDatedpy</div date>
<div class='detail'><b>DETAILS:</b> $detail</div detail>
<div class='options'>$opts</div options>
<div class='results'>$results</div results>
</div qdetails$id>
EOF;
//</div ballot>
	}

	$list = <<<EOF
<div id='questionlist'>
<div class='qline hdr clearfix'><div class='phase'>Phase</div><div class='id'>#</div><div class='text'>Question</div><div class='date'>Ends</div></div qline hdr>
$list
</div questionlist>
$qdivs
EOF;

	return array($list, @$phase, u\fmtDate(@$endDate));
}

/**
 * Return a list of options for the given question.
 */
function getOpts($question) {
	$q = db\q('SELECT text, detail FROM r_options WHERE question=:question ORDER BY displayOrder', compact('question'));

	$opts = "<br><b>OPTIONS:</b><br>\n";
  $opti = 0;
	while ($row = $q->fetchAssoc()) {
		extract($row);
		$opts .= getOpt($opti, $text, $detail);
    $opti++;
	}

	return $opts;
}

/**
 * Return the given option details, formatted.
 */
function getOpt($opti, $opttext, $optdetail) {
  global $rUrl;
  
//	$expandimgstyle = $optdetail ? '' : " style='visibility:hidden;'";
	$expandimgclass = $optdetail ? '' : " invisible";
	$rowclass = ($opti & 1) ? 'odd' : 'even';
//  $iDpy = $opti + 1;
//<div class="item"><img id="expand$opti" class="expand" index="$opti" src="$rUrl/images/rvote/expand.gif" alt="show detail" title="show detail" width="13" height="13" onclick="expandOpt($opti);"$expandimgstyle />&nbsp;
  
	return <<<EOF
<div class="optRow $rowclass">
<div class="rowwrapper">
<div class="item"><img src="$rUrl/images/rvote/expand.gif" id="expand$opti" class="expand$expandimgclass" index="$opti" alt="show detail" title="show detail" width="13" height="13" />&nbsp;
<b>Option:</b> $opttext</div item>
<div id="optdetail$opti" class="optdetail">
<div id="optdetailtext$opti" class="optdetailtext"><div class="optdetailheader">DETAIL:</div optdetailheader>$optdetail<br>&nbsp;</div optdetailtext$opti>
</div optdetail$opti>
</div rowwrapper></div row>\n\n
EOF;

}

function phaseDpy($qid, $phase, $repeats, $repeatedBy, $linkIdeas, $linkProposals) {
	return phaseRepeat($repeats, 'repeats')
			 . phaseOther('i', $phase, $linkIdeas)
			 . phaseOther('p', $phase, $linkProposals)
			 . phaseOther('v', $phase, $qid)
			 . phaseOther('r', $phase, $qid)
			 . phaseRepeat($repeatedBy, 'reconsidered in');
}

function phaseRepeat($qid, $title) {
// qid     question to repeat
	$title = ($qid ? "$title question #$qid" : '');
	$ltr = $qid ? '*' : '&nbsp;';
	$class = 'repeat inactive';
	return phaseDpy1($ltr, $qid, $class, $title);
}

function phaseOther($ltr, $phase, $href) {
  $phases = array(
  'I' => 'Ideas', // brainstorm relevant concerns, criteria, possible solutions (this section is closed when next starts)
  //'D' => 'Discussion', // argue the merits, organize and combine ideas, look for more than one good solution (no, go to proposals)
  'P' => 'Proposals', // present formal carefully-worded proposals (discussion & proposals are concurrent)
  //'D' => 'Discussion', // argue the merits of the final options (no, do this during voting, if at all)
  /* The strategy here is to start with wide-open, non-judgmental brainstorming, followed by discussion focused on developing multiple proposals that will satisfy all concerns. Once there is a sense of agreement on the options and their merits (in the judgment of the Division Advisors), then the options are formalized and voting begins. Discussion can continue, but should not repeat what has already been said, so it shouldn't be much. In fact, it is probably better to close the debate once voting begins. 
  */
  'V' => 'Vote', // voting in progress (show these questions first)
  'R' => 'Results'); // voting is over
	$LTR = strtoupper($ltr);
	$href = (strpos('IPVR', $phase) < strpos('IPVR', $LTR)) ? '' : str_replace('=LTR', "=$LTR", $href);
	if($phase == 'R') if($LTR == 'V') $href = ''; // don't allow voting once it's over
	$title = "$phases[$LTR]";
	$class = ($phase == $LTR) ? 'active' : 'inactive';
	return phaseDpy1($ltr, $href, $class, $phases[$LTR]);
}

function phaseDpy1($ltr, $href, $class, $title) {
  if (is_numeric($href)) {
    $class .= ' qdetailer';
    $index = " index=\"$href\"";
    $href = '';
  } else $index = '';
  
	if($href) {
//		if (is_numeric($href)) $href = "javascript:$('#qdetails' + $href).toggle();";
    $target = strpos($href, '?') ? "target=\"_blank\"" : '';
		return "<a href=\"$href\" $target>$ans</a>";
	} else return "<div class=\"$class\"$index title=\"$title\">$ltr</div>";
}

function getresults($question) {
	include_once __DIR__ . '/rvote-results.inc';

	$blurb = '';
	return $blurb . v\showResult($question);
//. goform($community_id, $voter_id, 'overview.php', 'Back to Overview');
}

function setdbfield($k, $v) {$_POST[$k] = $v; f('t.POST', [$k => $v]);}

function formatdate($dt='') {
	$fmt = 'Y-m-d H:i:s';
	return $dt ? date($fmt, $dt) : date($fmt);
}

function letterGrade($value, $full = TRUE) {
// value
// RETURN    letter grade representation of the value, eg "C+" (0.42)

/**/	if($value <= -5 or $value >= 5.5) die("bad grade: \"$value\"");
  $integer = round($value);
  $fraction = $value - $integer;
  $grade = substr(GRADELTRS, $integer, 1);
  $sign = $fraction >= .167 ? '+' : ($fraction <= -.167 ? '-' : '&nbsp;&nbsp;');
	return $grade . $sign . ($full ? ' (' . number_format($value, 2) . ')' : '');
}

function formatQuestionDates($dt1, $dt2, $startplusdays=0) {
	$dt1 += $startplusdays*DAY_SECS; 
  list($y1, $m1, $d1) = explode('/', date('Y/F/j', $dt1));
  $dt1 = "$m1 $d1, $y1";

	if ($dt2) {
    list($y2, $m2, $d2) = explode('/', date('Y/F/j', $dt2));
    $to = t('to');
  	if($y1 == $y2) {
      return ($m1 == $m2) ? "$m1 $d1-$d2, $y1" : "$m1 $d1 $to $m2 $d2, $y2";
    } else return "$dt1 $to $m2 $d2, $y2";
  } else return $dt1 . t(' to whenever');
}

/**
 * Return the questions and their types, for a given event.
 * @param int $eid: the event ID or empty
 * @param acct $mya: the relevant account, if $eid is empty
 * @return [qIds, qTypes] where
 *   qIds: array of question record IDs
 *   qTypes: corresponding array of question types
 */
function getQuestions($eid, $mya = NULL) {
	// note that if a ballot is kept open beyond the election end time, the following query will disallow it
//	if(!$crit) $crit = 'NOW() BETWEEN question_startdate AND question_enddate';
  if (!@$eid) {
    $event = v\getCurrentEvent($mya->community);
    extract(just('eid', $event));
  }
  $now = time();
//  $crit = 'ctty IN (:ctty, 0) AND :now>endProposals AND (:now<endVoting OR endVoting=0)';
	$q = db\q('SELECT id AS question, type FROM r_questions WHERE event=:eid', compact('eid'));

	while($row = $q->fetchAssoc()) {
		extract($row);
		$qIds[] = $question;
		$qTypes[$question] = $type;
	}

	return [@$qIds, @$qTypes];
}

/**
 * Return an HTML list of comments on a given proposal.
 */
function proposalComments($pid) {
  $sql = <<<EOF
    SELECT CONCAT('(q', v.option-11, ') ', v.text, IF(v.isVeto, ' VETO!', '')) AS notes FROM r_proposals p JOIN r_ballots b ON b.question=-p.id JOIN r_votes v ON v.ballot=b.id WHERE v.text <> '' AND p.id=:pid ORDER BY p.project, v.option
EOF;
  return db\q($sql, compact('pid'))->fetchCol();
}

/**
 * Return an array of proposal record IDs in the requested order.
 * @param mixed $order: an account object (meaning arbitrary persistent order for that account) or string (ORDER BY clause)
 */
function getProposals($mya, $order = '') {
  $ctty = $mya->community;
  
  if (!$order) {
    $plus = date('i', $mya->created);
    $mod = date('s', $mya->created);
    $order = "(id*$plus)%$mod, id"; // randomize but stay the same for this member
  }
  
  return db\col('id', 'r_proposals', 'ctty IN (:ctty, 0) ORDER BY ' . $order, compact('ctty')); 
}

/*
function qid($qid) {
	return substr($qid + 10000, 1);
}*/

function goDemocracy() {
  return <<<EOF
<br><a href="events" class="pull-left btn btn-xs btn-primary" role="button">&laquo; Common Good Democracy Events Home</a>
EOF;
}

function getCurrentEvent($ctty) {return db\lookup('id AS eid, type, event, details, start, end', 'r_events', 'ctty=:ctty AND end>:now ORDER BY end LIMIT 1', ray('ctty now', $ctty, time()));}

/**
 * Return fields from an event record.
 * @param string $fields: fields to return
 * @param int $eid: event record ID
 */
function event($fields, $eid) {return db\lookup($fields, 'r_events', 'id=:eid', compact('eid'));}

function question($question) {return db\lookup('q.id AS question, q.text, q.detail, q.linkDiscussion, q.type, q.budget, q.minVeto, q.optOrder, e.start, e.end', 'r_questions q LEFT JOIN r_events e ON e.id=q.event', 'q.id=:question', compact('question'));}