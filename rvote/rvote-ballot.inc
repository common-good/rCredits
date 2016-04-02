<?php
namespace rCredits\Vote;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Web as w;
use rCredits\Vote as v;

/* Note that vetoing costs the voter a proportionate share of their votes. 
For example, if there are 10 items to choose from, vetoing any one of them reduces the voter's say in the other 9 by 10%
A comment explaining the rationale for the veto is required. Each voter gets only one veto per question, but the veto may be split among more than one option. For example, if the voter vetoes 3 options, then each veto counts as 1/3 veto.
*/

// Election can be verified with a list of voters (without ballot number) and a list of ballots (without voter id).
// List of ballots includes the ballot id, voted grade for each option and each question, and proxy ballot id if not direct.

function ballot(&$extras, $question) {
  global $quids;
  $mya = r\acct();

  list($quids, $qutypes) = getQuestions($mya->community); 
  $qcount = count($quids);
/**/  if(!$qcount) die("There is no vote in progress for your community.");

  // set up question to display
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
  $qndpy = $qnum + 1; // $qnum starts at 0
  $extras['question'] = w\hidFld($question);
  $extras['submit'] = w\submit(t('Done With Question #') . $qndpy);

  $qu = db\lookup('id AS question, text, detail, linkProposals, type, budget, minVeto, optOrder, endProposals, endVoting', 'r_questions', 'id=:question', compact('question'));
  $tableHeader = tableHeader($qu['type']);
  if($qu['linkProposals']) $qu['linkProposals'] = "<div class='discussionlink'> [<a href='$qu[linkProposals]' target='_blank'>See discussion</a>]</div>";
  $range = formatQuestionDates($qu['endProposals'], $qu['endVoting'], 1);

  // get current voted values for this question
  $defaults = getCurrentVote($extras, $mya->id, $question, $qu['type'], $qu['optOrder']);
  $optCount = $defaults['optCount'];
  $opts = getOptsBorM($mya->id, $question, $qu['type'], $defaults);

  w\js('js/rvote/ballot.js', 'file', 'footer');
  if($qu['type'] == 'B') {
    w\css('css/rvote/slider.css');
    foreach(u\ray('sliderutil slider questionB overview') as $k) js("js/rvote/$k.js", 'file', 'footer');
  } else w\js('js/rvote/questionM.js', 'file', 'footer');

  w\js("load$qu[type]($optCount);", 'inline', 'footer');

//<form name='ballotform' id='ballotform' action='ballot.php' method='$METHOD' onsubmit="return checkform(this, $optCount, '$qu[type]');">

  $bodymiddle = <<<EOF
<div class='ballot well q$qu[type]'>

<div class='ballotheader'>
<div class='electiontitle'>CGVoting: $range</div electiontitle>
<div class='questioncount'>(Question #$qndpy of $qcount)</div questioncount>
<div class='clearer'></div>
</div ballotheader>

$qu[linkProposals]
<div class='question'><b>QUESTION #$qndpy:</b> $qu[text]</div question>
<div class='questiondetail'>$qu[detail]</div questiondetail>
<div class='optionheader'>$tableHeader</div optionheader>
<div id='options'>
$opts
</div options>
</div ballot>
EOF;

  return $bodymiddle;
}

function processvote($args, $type) {
	global $quids, $now, $ermsg;

	if (!$optionCount = $args['optionCount']) return;

	for($opti = 0; $opti < $optionCount; $opti++) {
		$isVeto = $args["veto$opti"];
		$grade = $isVeto ? VETOGRADE :$args["option$opti"];
		if($type == 'B') {
			$grade *= BPCT2INT; // convert penny grade (pct) to integer
		} elseif(!$isVeto) $grade = round($grade * 3); // multiply letter grades by 3 to keep + and -
		$vote = $args["vote$opti"];
    db\update('r_votes', u\ray('id grade modified', $vote, $grade, time()), 'id');

		$veto = $args["veto$opti"];
		$vetoText = @$args["vetonote$opti"];
    $info = u\ray('id text modified', $veto, $vetoText, time());
		if($veto) {
      db\update('r_vetoes', $info, 'id');
		} elseif($isVeto) db\insert('r_vetoes', $info);
	}
	$ballot = $args['ballot'];
  $voter = db\lookup('voter', 'r_ballots', 'id=:ballot', compact('ballot'));
  db\update('r_ballots', u\ray('id proxy modified', $ballot, $voter, time()), 'id');
}

/**
 * Return information about the current vote, including how the voter already voted.
 */
function getCurrentVote(&$extras, $voter, $question, $type, $optOrder) {

	$optCount = db\count('r_options', 'question=:question', compact('question'));
	$extras['optionCount'] = w\hidFld($optCount);
	$ballot = db\lookup('id', 'r_ballots', 'question=:question AND voter=:voter', compact('question', 'voter'));
	$q = db\q('SELECT v.id AS vote, v.option, v.grade, x.id AS veto, x.text  FROM ((r_ballots b LEFT JOIN r_votes v ON v.ballot=b.id) LEFT JOIN r_options o ON v.option=o.id) LEFT JOIN r_vetoes x ON x.vote=v.id WHERE b.voter=:voter AND o.question=:question ORDER BY v.displayOrder', compact('voter', 'question'));

  $rowCount = $q->rowCount();
	if($rowCount < $optCount) { // vote records not set up yet (or interrupted)
		if($rowCount) db\q('DELETE v FROM r_votes v LEFT join r_options o ON o.id=v.option WHERE o.question=:question', compact('question'));
		// maybe delete orphaned veto records too?
		setupVote($extras, $voter, $question, $optOrder, $ballot);
	} else {
    for ($i = 0; $row = $q->fetchAssoc(); $i++) {
			extract($row);
			$isVeto = ($grade == VETOGRADE);
			if(!$isVeto) $grade /= (($type == 'B') ? BPCT2INT : 3);

			$res["option$i"] = $grade;
			$res["veto$i"] = $isVeto;
			$res["vetonote$i"] = $text;
      
      $extras["vote$i"] = w\hidFld($vote);
      $extras["veto$i"] = w\hidFld($veto);
		}
	}
	$extras['ballot'] = w\hidFld($ballot);

	return $res + compact('optCount');
}

function setupVote(&$extras, $voter, $question, $optOrder, &$ballot) {
	global $sql_record_id;

	if(!$ballot) { // no ballot yet, create one!
    $created = time();
    $ballot = db\insert('r_ballots', compact(u\ray('question voter created')));
	}

	$opts = setupOptions($question, $optOrder);

	for($i = 0; $i<count($opts); $i++) {
		setdbfield("option$i", BLANKGRADE);
		setdbfield("veto$i", false);
		setdbfield("vetonote$i", '');
		$option = $opts[$i]; 
		$grade = BLANKGRADE;
    $displayOrder = $i;
    $vote = db\insert('r_votes', compact(u\ray('ballot option grade displayOrder')));
    $extras["vote$i"] = w\hidFld($vote);
    $extras["veto$i"] = w\hidFld('');
	}
}

function setupOptions($question, $optOrder='') {
	$q = db\q('SELECT id AS `option`, displayOrder FROM r_options WHERE question=:question', compact('question'));
	$opts = [];
	while ($row = $q->fetchAssoc()) { // make a list of the options
		extract($row);
		$opts[$displayOrder ?: count($opts)] = $option;
	}

	if($optOrder == 'Z') array_reverse($opts);
	if($optOrder == 'S') shuffle($opts);
	return $opts;
}

function getOptsBorM($voter, $question, $type, $defaults) {
// RETURN the options in the order the voter is used to seeing them
	$q = db\q('SELECT o.text, o.detail FROM (r_options o INNER JOIN r_votes v ON v.option=o.id) INNER JOIN r_ballots b on b.id=v.ballot WHERE b.voter=:voter AND o.question=:question ORDER BY v.displayOrder', compact('voter', 'question'));
	for($opti = 0, $opts=''; $row = $q->fetchAssoc(); $opti++) {
		extract($row);
  	$rowclass = ($opti&1) ? 'odd' : 'even';
		$opts .= call_user_func("rCredits\\Vote\\getOpts$type", $opti, $rowclass, $text, $detail, $defaults);
	}
	return $opts;
}

function getOptsB($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$sliderv = "slider$opti";
	$slidpyv = "option$opti";
//<input id='slider$opti' class="CGslider orientation-horizontal target-option$opti size-200 position-0 stops-0 to-100" />
//<input id='option$opti' class='sliderdpy $rowclass' value='0' />

  $onchange = 'changepct($opti);';
  $slidervv = w\rendA($sliderv, w\textFld('', '', w\dft($defaults[$sliderv]) + w\clas('CGslider target-input_$slidpyv size-200')));
  $slidpyvv = w\rendA($slidpyv, w\textFld('', '', w\dft($defaults[$slidpyv]) + w\clas("sliderdpy $rowclass") + w\attrib(compact('onchange'))));
//	input($sliderv, 's', '', "class='CGslider target-input_$slidpyv size-200'"); $slidervv = vv("input_$sliderv");
//	input($slidpyv, 's', '0', "class='sliderdpy $rowclass' onchange='changepct($opti);'"); $slidpyvv = vv("input_$slidpyv");

	$optStart = optStart($opti, $rowclass, 'B');
	$optEnd = optEnd($opti, $rowclass, $opttext, $optdetail);

	return <<<EOF
$optStart
<div class='sliderandpct'>
<div class='slider'>
	<div id='sliderdiv$opti' style='float:left;'>$slidervv</div>
</div slider>
<div class='pct'>$slidpyvv%</div pct>
<nothing />
</div sliderandpct>
$optEnd
EOF;
}

function getOptsM($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$optStart = optStart($opti, $rowclass, 'M');
	$optEnd = optEnd($opti, $rowclass, $opttext, $optdetail);
	list ($gradeinputs, $gradehdr) = gradeInputs($opti, $defaults);

	return <<<EOF
$optStart
<div id='grades$opti' class='grades'>

<div class='gradeheader'><table><tr>
$gradehdr
</tr></table></div gradeheader>

<div class='gradeinputs'>
$gradeinputs
</div gradeinputs>
<nothing />
</div grades$opti>
$optEnd
EOF;

}

function gradeInputs($opti, $defaults) {
	$v = "option$opti";
	$oldval = $defaults[$v]; $oldvalint = ($oldval !== '') ? round(3 * $oldval) + 0 : '';

	for ($grade=0, $gradeinputs='', $gradehdr=''; $grade < strlen(GRADELTRS); $grade++) {
		$dpy = $ltr = substr(GRADELTRS, $grade, 1); // default display (dpy) is unadorned grade letter
		$checked = ''; // unchecked by default
    $onclick = "nudgegrade(this, $opti);";
    $vv = "<input type=\"radio\" name=\"$v\" onclick=\"$onclick\" value=\"$grade\" />";
//		input($v, 'r', '', "onclick=\"nudgegrade(this,$opti);\"", $grade); $vv = vv("input_$v");
		if ($oldvalint == 3 * $grade) {  // this works even for E (0)
			$newval = $oldval;
			$checked = ' CHECKED';
			if ($sign = $newval - $grade) $dpy .= '<sup>' . ($sign > 0 ? '+' : '<b>&ndash;</b>') . '</sup>';
		} else $newval = $grade - 0.1; // fake minus, so that first click sets unadorned choice
		$vv = str_replace("value='$grade'", "value='$newval'$checked", $vv);
//<input type='radio' name='option$opti' value='$grade' />
	//	$gradehdr .= "<div class='g$ltr'>$dpy</div>\n"; // div table-cell fails in IE
		$gradehdr .= "<td id='g$ltr$opti' class='g$ltr'>$dpy</td>\n";
		$gradeinputs .= "<div>$vv</div>\n";
	}
	return array($gradeinputs, $gradehdr);
}

function optStart($opti, $rowclass, $type) {
	$vetov = "veto$opti";	
//input($vetov, 'b', '', "onclick='vetoclick$type($opti);'"); $vetovv = vv("input_$vetov");
  $onclick = "vetoclick$type($opti);";
  $vetovv = w\rendA($vetov, w\boxFld($vetov, '', '', '', w\attrib(compact('onclick'))));
	return <<<EOF
<div class="optRow $rowclass"><div class='rowwrapper'>
<div class='veto'>$vetovv</div veto>
EOF;
//<div class='veto'><input type="checkbox" id="veto$opti" name="veto$opti" onclick="vetoclick($opti);" /></div>
}

function optEnd($opti, $rowclass, $opttext, $optdetail) {
	global $rUrl;
	$vetonotev = "vetonote$opti";	
//  input($vetonotev, 't', ''); $vetonotevv = vv("input_$vetonotev");
  $vetonotevv = w\rendA($vetonotev, w\areaFld());

	$expandimgstyle = $optdetail ? '' : " style='visibility:hidden;'";
	return <<<EOF
<div class='item'><img id='expand$opti' src='$rUrl/images/rvote/expand.gif' alt='show detail' title='show detail' width='13' height='13' onclick="expand($opti);"$expandimgstyle />&nbsp;
$opttext</div item>
<div id='optdetail$opti' class='optdetail'>
<div id='vetonotediv$opti' class='vetonotediv'><div class='vetonoteheader'>Reason for Veto*:</div vetonoteheader>$vetonotevv<br><span class='vetofootnote'>* How is this option evil, immoral or unethical?</span></div vetonotediv$opti>
<div class='optdetailheader'>OPTION DETAIL:</div optdetailheader><div id='optdetailtext$opti' class='optdetailtext'>$optdetail<br>&nbsp;</div optdetailtext$opti>
</div optdetail$opti>

</div rowwrapper></div optRow $rowclass>\n\n
EOF;
}

function tableHeader($type) {
	global $rUrl;
	$leftarrow = "<img src='$rUrl/images/rvote/arrow-left.gif' alt='' />";
	$rightarrow = "<img src='$rUrl/images/rvote/arrow-right.gif' alt='' />";

	return $type == 'M' 
	? "<div class='veto'>Veto</div>
		 <div class='grades'>$leftarrow Bad | Good $rightarrow</div>
		 <div class='item'>Options</div>"
	: "<div class='veto'>Veto</div>
		 <div class='slider'>$leftarrow Less | More $rightarrow</div>
		 <div class='pct'>Percent</div>
		 <div class='item'>Funding Option</div>
		 ";
}

/*
function specialhdrs($question_type) {
	global $rUrl/images/rvote;
	$ans = jscript('ballot.js');
	if($question_type == 'B') {
		$ans .= "<link rel='stylesheet' type='text/css' href='$rUrl/images/rvote/inc/slider.css' />";
		foreach(array('sliderutil.js', 'slider.js', 'questionB.js', 'overview.js') as $one) $ans .= jscript($one);
	} else $ans .= jscript('questionM.js') ;

	return $ans;
}
*/

function done() {
  global $proxiesForThisVoter;
  include_once __DIR__ . '/rvote-results.inc';

	$goform = goDemocracy();

	if(ISDEMO) {
    $mya = r\acct();
    $proxyCount = db\count('r_ballots', 'proxy=:myid', ['myid' => $mya->id]) - 1;
    
		$bodymiddle = <<<EOF
<h2>Thank You for Voting!</h2>

<p>Normally it is important not to show election results until the election is over. Otherwise you get a "sheep" effect &mdash; later voters favoring options popular with earlier voters. For the wisest possible results, Common Good Voting normally fosters independent judgment by saving the results until after everyone has voted.</p>

<p>However, since this is an ongoing demo, you get to see the results ahead of time. Imagine the election is over and take a peak at the table of results below.</p>

<p>Note that you currently represent $proxyCount other rCredits members with your vote.</p>

<h2>RESULTS</h2>
EOF;

		return $bodymiddle . showResults() . $goform; // show all results
	}

	$bodymiddle = <<<EOF
<h2>Thank You for Voting!</h2>
$goform
EOF;
	return $bodymiddle;
}
