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

function ballot($mya, &$extras, $question) {
  global $quids;
  
  list($quids, $qutypes) = getQuestions($mya->community); 
  if (!$qcount = count($quids)) return t('There is no vote in progress for your community.');
  u\setDft($question, $quids[0]);

  // set up question to display
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
  $qndpy = $qnum + 1; // $qnum starts at 0
  $extras['question'] = w\hidFld($question);
  $submit = w\submit(BARE . t('Done With Question #') . $qndpy, 'primary', 'md', w\clas('pull-right'));
  $extras['prev'] = w\submit('&laquo; ' . t('Previous'), 'primary', 'md', w\suffix(w\rendA('submit', $submit)));

  $qu = db\lookup('id AS question, text, detail, linkProposals, type, budget, minVeto, optOrder, endProposals, endVoting', 'r_questions', 'id=:question', compact('question'));
  $tableHeader = tableHeader($qu['type']);
  if($qu['linkProposals']) $qu['linkProposals'] = "<div class='discussionlink'> [<a href='$qu[linkProposals]' target='_blank'>See discussion</a>]</div>";
  $range = formatQuestionDates($qu['endProposals'], $qu['endVoting'], 1);

  // get current voted values for this question
  $defaults = getCurrentVote($extras, $mya->id, $question, $qu['type'], $qu['optOrder']);
///  debug(compact('defaults'));
  $optCount = $defaults['optCount'];
  $opts = getOptsX($mya->id, $question, $qu['type'], $defaults);

  w\js('js/rvote/ballot.js', 'file', 'footer', 0);
  
  if(strpos(' BR', $qu['type'])) {
    w\css('css/bootstrap-slider.min.css');
    w\js("js/rvote/bootstrap-slider.min.js", 'file', 'footer', 0);
  }

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
    $isVeto = @$args["veto$opti"] ? 1 : 0;
    if ($type != 'E') {

      $grade = ($isVeto and $type != 'R') ? VETOGRADE : @$args["option$opti"];
      if($type == 'B') {
        $grade *= BPCT2INT; // convert penny grade (pct) to integer
      } elseif($type == 'M' and !$isVeto) $grade = round($grade * 3); // multiply letter grades by 3 to keep +/-
      if ($type == 'R') $gradeMax = @$args['max'];
    }
      
    list ($id, $text, $modified) = [$args["vote$opti"], @$args["votenote$opti"], time()];
    db\update('r_votes', compact(u\ray('id grade gradeMax text isVeto modified')), 'id');
	}
	$ballot = $args['ballot'];
  $voter = db\lookup('voter', 'r_ballots', 'id=:ballot', compact('ballot'));
  db\update('r_ballots', u\ray('id proxy modified', $ballot, $voter, time()), 'id');
}

/**
 * Return information about the current vote, including how the voter already voted.
 * If the voter has no vote record yet, create it.
 * @param assoc $extras: (RETURNED) extra fields to add to the form
 * @param int $voter: who's voting (user record ID)
 * @param int $question: question ID
 * @param char $type: question type (M=multiple-choice B=budget R=range E=essay)
 * @param char $optOrder: how to order the options
 */
function getCurrentVote(&$extras, $voter, $question, $type, $optOrder) {

	$optCount = db\count('r_options', 'question=:question', compact('question'));
	$extras['optionCount'] = w\hidFld($optCount);
	if (!$ballot = db\lookup('id', 'r_ballots', 'question=:question AND voter=:voter', compact('question', 'voter'))) {
    $created = time();
    $ballot = db\insert('r_ballots', compact(u\ray('question voter created')));
  }
  
  $sql = <<<EOF
    SELECT v.id AS vote, v.option, v.grade, v.gradeMax, o.minimum AS min, o.maximum AS max, v.isVeto, v.text
    FROM r_ballots b LEFT JOIN r_votes v ON v.ballot=b.id LEFT JOIN r_options o ON v.option=o.id 
    WHERE b.voter=:voter AND o.question=:question ORDER BY v.displayOrder
EOF;
	$q = db\q($sql, $subs = compact('voter', 'question'));

  $rowCount = $q->rowCount();
	if($rowCount < $optCount) { // vote records not set up yet (or interrupted)
		if($rowCount) db\q('DELETE v FROM r_votes v LEFT join r_options o ON o.id=v.option WHERE o.question=:question', compact('question'));
		// maybe delete orphaned veto records too?
		setupVote($extras, $voter, $question, $optOrder, $ballot);
  	$q = db\q($sql, $subs = compact('voter', 'question'));
	}

  $res = [];
  for ($i = 0; $row = $q->fetchAssoc(); $i++) {
    extract($row);

    if ($type == 'R') { // only one option record for range-type votes
      foreach (['min', 'max'] as $k) $res[$k] = (int) $$k;
      $res['minDft'] = $grade;
      $res['maxDft'] = $gradeMax == BLANKGRADE ? $max : $gradeMax;
    } elseif ($type == 'M') {
      if (!$isVeto) $grade /= 3;
    } elseif ($type == 'B') {
      if (!$isVeto) $grade = ($grade == BLANKGRADE) ? 100 / $optCount : ($grade / BPCT2INT);
    }

    $res["option$i"] = $grade;
    $res["veto$i"] = $isVeto;
    $res["votenote$i"] = $text;
    
    $extras["vote$i"] = w\hidFld($vote);
//    $extras["note$i"] = w\hidFld($note);
	}
	$extras['ballot'] = w\hidFld($ballot);

	return $res + compact('optCount');
}

function setupVote(&$extras, $voter, $question, $optOrder, &$ballot) {
	global $sql_record_id;

	$opts = setupOptions($question, $optOrder);

	for($i = 0; $i<count($opts); $i++) {
		setdbfield("option$i", BLANKGRADE);
		setdbfield("veto$i", false);
		setdbfield("votenote$i", '');
		$option = $opts[$i];
		$grade = BLANKGRADE;
    $gradeMax = BLANKGRADE;
    $displayOrder = $i;
    $vote = db\insert('r_votes', compact(u\ray('ballot option grade gradeMax displayOrder')));
    $extras["vote$i"] = w\hidFld($vote);
//    $extras["note$i"] = w\hidFld('');
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

function getOptsX($voter, $question, $type, $defaults) {
// RETURN the options in the order the voter is used to seeing them
  $sql = 'SELECT o.text, o.detail FROM (r_options o INNER JOIN r_votes v ON v.option=o.id) INNER JOIN r_ballots b on b.id=v.ballot WHERE b.voter=:voter AND o.question=:question ORDER BY v.displayOrder';
	$q = db\q($sql, compact('voter', 'question'));
	for($opti = 0, $opts=''; $row = $q->fetchAssoc(); $opti++) {
		extract($row);
  	$rowclass = ($opti&1) ? 'odd' : 'even';
		$opts .= call_user_func("rCredits\\Vote\\getOpts$type", $opti, $rowclass, $text, $detail, $defaults);
	}
	return $opts;
}

function getOptsB($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$slider = "slider$opti";
	$option = "option$opti";
//  $onchange = 'changepct($opti);';

  $dft = @$defaults[$option];
  $sliderHtml = <<<EOF
<input id="$slider" data-slider-id='$slider' type="text" data-slider-min="0" data-slider-max="100" data-slider-step="1" data-slider-value="$dft" data-slider-handle="triangle" />
EOF;
  $optionHtml = w\rendA($option, w\hidFld($dft));

	$optStart = optStart($opti, $rowclass, 'B', $defaults);
	$optEnd = optEnd($opti, $rowclass, $opttext, $optdetail, $defaults);

	return <<<EOF
$optStart
<div class='sliderandpct'>
<div class='gliss'>
	<div id='sliderdiv$opti' class='sliderN'>$sliderHtml</div>
</div gliss>
<div class='pct'>$optionHtml</div pct>
<nothing />
</div sliderandpct>
$optEnd
EOF;
}

/**
 * Return HTML for a "range" type question.
 */
function getOptsR($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$slider = "slider$opti";
	$option = "option$opti";
  $onchange = 'changepct($opti);';

  extract(u\just('min max minDft maxDft', $defaults));
  $sliderHtml = <<<EOF
<input id="$slider" data-slider-id='$slider' type="text" data-slider-min="$min" data-slider-max="$max" data-slider-step="1" data-slider-value="[$minDft, $maxDft]" data-slider-handle="triangle" />
EOF;
  $optionHtml = w\rendA($option, w\hidFld($minDft))
              . w\rendA('max', w\hidFld($maxDft));

	$optStart = optStart($opti, $rowclass, 'B', $defaults);
	$optEnd = optEnd($opti, $rowclass, $opttext, $optdetail, $defaults);

	return <<<EOF
$optStart
<div class='sliderandpct'>
<div class='gliss'>
	<div id='sliderdiv$opti' class='sliderN'>$sliderHtml</div>
</div gliss>
<div class='pct'>$optionHtml</div pct>
<nothing />
</div sliderandpct>
$optEnd
EOF;
}

/**
 * Return HTML for an "essay" type question.
 */
function getOptsE($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$votenote = "votenote$opti";	
  $votenoteHtml = w\rendA($votenote, w\areaFld(BARE, '', @$defaults[$votenote]));

	return <<<EOF
<div class="optdetailE">
<div id="votenotediv$opti">$votenoteHtml</div votenotediv$opti>
</div><!--optdetailE-->
EOF;
}

function getOptsM($opti, $rowclass, $opttext, $optdetail, $defaults) {
	$optStart = optStart($opti, $rowclass, 'M', $defaults);
	$optEnd = optEnd($opti, $rowclass, $opttext, $optdetail, $defaults);
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
	$oldval = $defaults[$v]; 
//  $oldvalint = ($oldval !== '') ? round(3 * $oldval) + 0 : '';

	for ($grade=0, $gradeinputs='', $gradehdr=''; $grade < strlen(GRADELTRS); $grade++) {
		$dpy = $ltr = substr(GRADELTRS, $grade, 1); // default display (dpy) is unadorned grade letter
		$checked = ''; // unchecked by default
    $onclick = "opts[$opti].nudgeGrade(this);";
    $vv = "<input type=\"radio\" name=\"$v\" onmousedown=\"$onclick\" value=\"$grade\" />";
//		input($v, 'r', '', "onclick=\"nudgeGrade(this,$opti);\"", $grade); $vv = vv("input_$v");
		if (round($oldval) == $grade and $oldval >= 0) {  // this works even for E (0)
			$val = $oldval;
			$checked = ' CHECKED';
			if ($sign = $val - $grade) $dpy .= '<sup>' . ($sign > 0 ? '+' : '<b>&ndash;</b>') . '</sup>';
		} else $val = $grade;
		$vv = str_replace("value=\"$grade\"", "value='$val'$checked", $vv);
///    debug(compact(u\ray('v oldval oldvalint grade val checked sign dpy vv')));
//<input type='radio' name='option$opti' value='$grade' />
	//	$gradehdr .= "<div class='g$ltr'>$dpy</div>\n"; // div table-cell fails in IE
		$gradehdr .= "<td id='g$ltr$opti' class='g$ltr'>$dpy</td>\n";
		$gradeinputs .= "<div>$vv</div>\n";
	}
	return array($gradeinputs, $gradehdr);
}

function optStart($opti, $rowclass, $type, $defaults) {
  $html = '';
  foreach (['note', 'veto'] as $k) {
    $v = "$k$opti";
    $label = $k == 'note' ? t('Comment') : t('Veto');
    $html .= w\rendA($v, w\boxFld($v, '', "<b>$label</b>", @$defaults[$v], w\onchange("opts[$opti].noteClick();")));
  }
	return <<<EOF
<div id="#optRow$opti" class="optRow $rowclass">
<div class='veto'>$html</div veto>
EOF;
}

function optEnd($opti, $rowclass, $opttext, $optdetail, $defaults) {
	global $rUrl;
	$votenote = "votenote$opti";	
  $votenoteHtml = w\rendA($votenote, w\areaFld(BARE, '', @$defaults[$votenote]));

	return <<<EOF
<div class="item">
  <div class="optImgWrap">
    <img id="expand$opti" class="optImg" src="$rUrl/images/rvote/expand.gif" alt="show detail" title="show detail" width="13" height="13" onclick="expand($opti);" />
  </div><!--optImgWrap-->
$opttext</div><!--item-->

<div id="optdetail$opti" class="optdetail">
  <div id="votenotediv$opti" class="votenotediv">$votenoteHtml</div><!--votenotediv$opti-->
  <div class="optdetailheader">OPTION DETAIL:</div><!--optdetailheader--><div id="optdetailtext$opti" class="optdetailtext">$optdetail<br>&nbsp;</div><!--optdetailtext$opti-->
</div><!--optdetail$opti-->

</div><!--optRow$opti-->\n\n
EOF;
}

function tableHeader($type) {
	global $rUrl;
	$leftarrow = "<img src='$rUrl/images/rvote/arrow-left.gif' alt='' />";
	$rightarrow = "<img src='$rUrl/images/rvote/arrow-right.gif' alt='' />";

	if ($type == 'M') return <<<EOF
<div class='grades'>$leftarrow Bad | Good $rightarrow</div>
EOF;
  return '';
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

function done($mya) {
  global $proxiesForThisVoter;
  include_once __DIR__ . '/rvote-results.inc';

	if(ISDEMO) {
    $proxyCount = db\lookup('COUNT (DISTINCT voter)', 'r_ballots', 'proxy=:myid', ['myid' => $mya->id]) - 1;
    
		$bodymiddle = <<<EOF
<h2>Thank You for Voting!</h2>

<p>Normally it is important not to show election results until the election is over. Otherwise you get a "sheep" effect &mdash; later voters favoring options popular with earlier voters. For the wisest possible results, Common Good Voting normally fosters independent judgment by saving the results until after everyone has voted.</p>

<p>However, since this is an ongoing demo, you get to see the results ahead of time. Imagine the election is over and take a peak at the table of results below.</p>

<p>Note that you currently represent $proxyCount other rCredits members with your vote.</p>

<h2>RESULTS</h2>
EOF;

		return $bodymiddle . showResults($mya->community); // show all results
	}

	$bodymiddle = <<<EOF
<h2>Thank You for Voting!</h2>
<p>Results will be announced here when the polls close. In the meantime, you may feel free to change any of your votes (simply sign in and select "Discussion & Voting" again on the Community menu).</p>
EOF;
	return $bodymiddle;
}

/**
 * Show (to admin) a list of who voted and how they voted.
 * This helps analyze flaws in the voting system (including question language).
 * @param int $ctty: community record ID
 * @param mixed $byVoter: boolean <order first by voter (not by question)> OR int single voter's record ID
 * @param string $proxies: whether to include direct votes and proxy votes (direct, proxy, or both)
 * @return the viewable list of votes
 *
 * @todo: For nonvoters maybe return <br>(You did not vote yet and neither did your proxy or alternate proxy, because they are not members of Greenfield rCredits.) 
 */
function showVotes($ctty, $byVoter = TRUE, $proxies = 'both') {
  $subs = compact('ctty');
  $crit = $proxies == 'direct' ? ' AND b.proxy=b.voter' : ($proxies == 'proxy' ? ' AND b.proxy<>b.voter' : '');
  if ($byVoter > 1) { // specifying a single voter
    $crit .= ' AND b.voter=:byVoter';
    $subs += compact('byVoter');
  } else v\showResults($ctty); // refigure results if showing more than one
  
  $order = $byVoter ? 'u.fullName,q.id' : 'q.id,u.fullName';
//  $ballots = $byVoter > 1 ? '(SELECT b2.id, b1.voter, b2.question FROM r_ballots b1 INNER JOIN r_ballots b2 ON (b2.voter=b1.proxy AND b2.question=b1.question))' : 'r_ballots';

  $sql = <<<EOF
    SELECT DISTINCT q.id AS qId, b.id AS bId, b.voter, b.proxy, q.type AS qType, u.fullName, u.trust, b.question
    FROM r_questions q 
    INNER JOIN r_ballots b ON b.question=q.id 
    INNER JOIN users u ON u.uid=b.voter
    WHERE ctty IN (:ctty, 0)$crit ORDER BY $order
EOF;

  $qvQ = db\q($sql, $subs);
  while ($qvRow = $qvQ->fetchAssoc()) {
    extract($qvRow); // qId, bId, voter, qType, fullName, trust, question
    if ($voter != $proxy) $bId = db\lookup('id', 'r_ballots', 'voter=:proxy AND question=:question', compact('proxy', 'question')); // this could be incorporated into query for efficiency
    if (!@$qId1) $qId1 = $qId; // remember first question number
    $grade = $gradeMax = $text = $isVeto = [];
    $sql = <<<EOF
      SELECT v.grade, v.gradeMax, v.text, v.isVeto 
      FROM r_votes v INNER JOIN r_options o ON v.option=o.id 
      WHERE v.ballot=:bId ORDER BY o.id
EOF;
    $vQ = db\q($sql, compact('bId'));
    while ($vRow = $vQ->fetchAssoc()) {
      $grade[] = $vRow['grade'];
      $gradeMax[] = $vRow['gradeMax'];
      $text[] = $vRow['text'];
      $isVeto[] = $vRow['isVeto'];
/*      foreach ($vRow as $k => $v) {
        ; // $$k[] = $v not allowed, $$k[$i] = $v fails
        $i++;
      } */
    }
    $line = $texts = '';
    for ($i = 0; $i < count($grade); $i++) {
      if ($isVeto[$i] or $grade[$i] == BLANKGRADE) {
        list ($char, $color) = $isVeto[$i] ? ['V', 'red'] : ['x', 'green'];
        $line .= "<b style=\"color:$color;\">$char</b> &nbsp;";
      } else {
        if ($qType == 'M') $line .= v\letterGrade($grade[$i] / 3, FALSE) . ' ';
        if ($qType == 'B') $line .= number_format($grade[$i] / BPCT2INT) . '&nbsp;.&nbsp;';
      }
      
      if ($qType == 'R') $line .= ($grade[$i] == BLANKGRADE ? '' : number_format($grade[$i])) . ' - ' . number_format($gradeMax[$i]);
      if ($text[$i]) $texts .= '--- ' . $text[$i] . '<br>';
    }
    if (strpos($line, '>V')) $line .= ' (V=veto)';
    if (strpos($line, '>x')) $line .= ' (x=left blank)';
    $line = $byVoter > 1 ? ($qType == 'E' ? '' : "$qId: $line<br>") : ("$qId: $fullName ($trust) -- $line<br>" . $texts);
    $lines[] = $qId == $qId1 ? "<br>$line" : $line;
  }
  return join('', @$lines ?: []);
}

/**
 * Return a list of members who haven't voted yet.
 */
function showNonVoters($ctty, $direct = TRUE) {
  $subs = compact('ctty');
  $voterFromBallots = $direct ? 'b.voter FROM r_ballots b' : 'b2.voter FROM r_ballots b2 INNER JOIN r_ballots b ON b.voter=b2.proxy';
  $sql = <<<EOF
    SELECT DISTINCT u.uid, trust FROM users u
    LEFT JOIN (
      SELECT $voterFromBallots INNER JOIN r_votes v ON v.ballot=b.id WHERE v.grade<>:BLANKGRADE
    ) bv ON bv.voter=u.uid
    WHERE u.community IN (:ctty, 0) AND bv.voter IS NULL AND !:IS_CO AND :IS_OK ORDER BY u.trust DESC
EOF;

  $q = db\q($sql, $subs);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $a = r\acct($uid);
    $lines[] = u\fmtPhone($a->phone) . ": $a->fullName<br>\n";
  }
  return join('', @$lines ?: []);
}    