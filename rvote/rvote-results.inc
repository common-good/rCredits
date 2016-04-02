<?php
namespace rCredits\Vote;
use rCredits as r;
use rCredits\DB as db;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\Web as w;
use rCredits\Vote as v;

function showResults() { // called from ballot if ISDEMO
  $mya = r\acct();
  list($quids, $qutypes) = getQuestions($mya->community); 

	$ans = '';
	foreach($quids as $one) $ans .= showResult($one, true) . '<hr>';
	return $ans;
}

function showResult($question, $withheader=false) {
	global $thisvoter; // only if ISDEMO

	$qu = db\lookup('type, text, budget, minveto', 'r_questions', 'id=:question', compact('question'));
	$opts = optList($question);
	$voters = getVoters($question);
//	global $proxiesForThisVoter; if(ISDEMO) if($thisvoter) $voters[$thisvoter] = $proxiesForThisVoter;
	$votes_direct = count($voters);
	$votesAll = array_sum($voters); // total weights. not necessarily equal to total weight used, since vetos reduce effective weight
	$newtot = tallyResults($voters, $qu['type'], $qu['budget'], $qu['minveto'], $opts);

	return getResult(qid($question), $qu['text'], $qu['type'], $qu['budget'], $newtot, $votes_direct, $votesAll, $withheader); // show result of one question, with links to audit info
}

function optList($question) {
	$q = db\q('SELECT id FROM r_options WHERE question=:question ORDER BY displayorder', compact('question'));
	while($row = $q->fetchAssoc()) $opts[] = $row['id'];

	return @$opts ?: [];
}

function getResult($qid, $question_text, $question_type, $budget, $tot, $votes_direct, $votesAll, $withheader) {
	$optCount = count($tot);

	for($i=0, $optTextMaxLen=0; $i < $optCount; $i++) {
		$otxlen = strlen($tot[$i]->opttext);
		if($otxlen > $optTextMaxLen) $optTextMaxLen = $otxlen;
	}
	$separateOptTexts = ($optTextMaxLen + $optCount * 7 > 100); // keep lines short by separating out the descriptions
  $separateOptTexts = TRUE; // FAILS otherwise
  
	if($question_type == 'M') {
		$prefheader1 = "<th colspan='$optCount'>Preferred Over:</th>";
		$prefheader2 = '';
	} else {
		$prefheader1 = "<th></th><th></th><th>Percent</th><th colspan='2'>Budget</th>";
		$prefheader2 = "<th>Result</th><th>Amount</th><th>of total</th><th>Min</th><th>Max</th>";
	}

	for ($i = 0, $optTexts = $middle = ''; $i < $optCount; $i++) {
		$inum = $i+1; // option number for display
		$toti = $tot[$i];
		$otx = $toti->opttext;
		$optlinehdr = "<b>#$inum</b>";
		$vetos = number_format($toti->vetos, 1);
		if($separateOptTexts) $optTexts .= "<b>Option #$inum:</b> $otx<br>\n"; else $optlinehdr .= ": $otx";

		if($question_type == 'M') {
			$prefheader2 .= "<th>#$inum</th>";
			$mid = result_midM($toti, $i);
			$budgetdpy = '';
		} else {
			$min = $toti->minimum ? (number_format($toti->minimum, 0) . ($toti->mandatory ? '!' : '&nbsp;')) : '- &nbsp;';
			$max = $toti->maximum ? number_format($toti->maximum, 0) : '- &nbsp;';
			$funds = $budget ? number_format($toti->funds, 0) : '? &nbsp;';
			$grade = number_format($toti->raw/BPCT2INT, 1) . '%'; // $toti->grade is irrelevant if we recursed
			$pct = $budget ? (number_format(100*$toti->funds/$budget, 1) . '%') : $grade;
			$mid = "<td>$toti->result</td><td>$funds</td><td>$pct</td><td>$min</td><td>$max</td><td>$grade</td>";
			$budgetdpy = '; &nbsp; total budget: $' . number_format($budget, 0);
		}
		$middle .= "<tr><td class='optlinehdr'>$optlinehdr</td>$mid<td>$vetos</td></tr>\n";
	}

  $optTexts = $optTexts ? "<blockquote>$optTexts</blockquote>" : '<br>';
	$hdr = $withheader ? "<h4>Question $qid: \"$question_text\"</h4>" : '<br><b>RESULTS:</b><br>';

	return <<<EOF
<div class="well">
$hdr
$votesAll voters ($votes_direct direct)$budgetdpy<br>
$optTexts

<table class="resultstable">
<tr><th class="resultOption"></th>$prefheader1<th class="resultAverage">Average</th><th></th></tr>
<tr><th class="resultOption">Option</th>$prefheader2<th class="resultVote">Vote</th><th>Vetos</th>
$middle
</table>
</div>
<br>
EOF;
}

function result_midM($toti, $i) {
	for ($j = 0, $eachpref = ''; $j < count($toti->prefs); $j++) {
		$prefij = ($i == $j) ? '- &nbsp;' : number_format(@$toti->prefs[$j], 1);
		$eachpref .= "<td>$prefij</td>";
	}

	$grade = ($toti->grade == 0) ? '- &nbsp; &nbsp; &nbsp;' : letterGrade($toti->grade/3);

	return "$eachpref<td>$grade</td>";
}

// for each question (voter may complete only some of the questions)
/* first figure out who the proxy is for each non-voter (skip in demo) like this:
		start with voter records (and proxy records) for all authorized voters
		for each proxy level 0-999 (until no more levels or no more nonvoters)
		select nonvoters whose proxy has voted: create a ballot for them using their proxy's proxy (now they have voted)
		when done, all proxy fields will show who actually cast the vote
*/

// list of names is public within each community (private from others)
// cgv site can fetch that list with id#s, to provide a proxy-selection form and at vote end to count proxies

function tallyResults($voters, $qtype, $qbudget, $qminveto, $opts, $skipoptid='') {
  include_once __DIR__ . '/opttotals.class';

	if($skipoptid) {$i = array_search($skipoptid, $opts); unset($opts[$i]);}
	$tot = array();	

	foreach ($opts as $optid) $tot[$optid] = new optTotals($optid, $opts);	// get empty total objects
	$optlist = join(',', $opts);

	foreach($voters as $voter=>$weight) { // foreach voter
		tallyVoter($tot, $voter, $qtype, $optlist, $weight); // tally this voter's ballot on this question
	}
	$score = score($tot, $qtype, $qbudget, $qminveto);
	$newtot = array();

  $newI = 0;
	foreach ($score as $optid => $onescore) {
		$tot[$optid]->raw = $tot[$optid]->grade; // remember results from first pass
		$newtot[$newI] = $tot[$optid]; // arrange best first (option id is stored in object)
    $newKeys[$newI] = $optid; // track new order
    $newI++;
	}
	$badtot = $tot[$optid]; // worst option

	if($qtype == 'B') {
		if ($onescore < 0) { // a bad worst score means redo (mark as vetoed or underfunded/overfunded)
			if(substr($badtot->result, 0, strlen(FUNDED)) != FUNDED) $badtot->funds = 0; // vetoed or underfunded
			if(count($opts) > 1) {
				$newtot = tallyResults($voters, $qtype, $qbudget - $badtot->funds, $qminveto, $opts, $badtot->id); // refigure w/o vetoed/misfunded opt
				foreach($newtot as $one) $one->raw = $tot[$one->id]->raw; // put back the original grades
				$newtot[] = $badtot; // tack the handled option back on (last)
			}
		} elseif (!$badtot->funds) $badtot->result = 'unfunded';
	} else {
		foreach($newtot as $k => $one) {
      $one->prefs = $one->prefs ? array_combine($newKeys, $one->prefs) : array_fill(0, $newI, 0);
      $newtot[$k] = $one;
    }
	}

	return $newtot;
}

/**
 * Score the options for a multiple-choice question (highest score for the best option, etc.)
 *
 * Scoring uses a variation on the Condorcet method:
 * 1. The option with the greatest net preference wins. The net preference of option A over option B is the number of voters who prefer A to B, minus the number who prefer B to A. 
 * 2. If voter preferences are circular (that is, voters prefer A over B, B over C, and C over A):
 *   o If any option is undefeated in #1, the option with the largest least-decisive pairwise victory wins.
 *   o Otherwise the option with the smallest worst pairwise defeat wins.
 * 3. In the rare event this result is ambiguous (tied), the less-vetoed option wins.
 * 4. If the veto counts are equal, the option with the highest average grade wins.
 * 5. If even the average grade is tied, the option with the greatest total of net preferences wins.
 * 6. If that too is a tie, the winner is chosen arbitrarily.
 */
function score($tot, $qtype, $qbudget, $qminveto) {
	$totalGrades = 0;
	foreach(array_keys($tot) as $i) {
		$toti = &$tot[$i];
		if($toti->weight) $toti->grade /= $toti->weight; // get average grade for this option
		$totalGrades += $toti->grade; // and total of averages
	}

	foreach(array_keys($tot) as $i) {
		$score[$i] = $qtype == 'M'
		? scoreM($tot, $i)
		: scoreB($tot[$i], $qbudget, $qminveto, count($tot), $totalGrades);
	}
	arsort($score);
	return $score; // sorted array of option ids: best first
}

function scoreM(&$tot, $i) {
	global $votesAll;

	$toti = &$tot[$i];

	foreach(array_keys($tot) as $j) $toti->prefdifs[$j] = @$toti->prefs[$j] - @$tot[$j]->prefs[$i]; // get difs
	$toti->minprefdif = min($toti->prefdifs);
	$toti->sumprefdif = array_sum($toti->prefdifs);
	return $toti->minprefdif + (($toti->vetos  + $toti->grade / 30) / ($votesAll+1) + $toti->sumprefdif) / ($votesAll+1);
}

function scoreB(&$toti, $budget, $minveto, $optCount, $totalGrades) {
	global $votesAll;

	$score = ($optCount == 1) ? 1 : $toti->grade/$totalGrades;
	$toti->funds = $funds = $score * $budget; // how much funding did this get? (score negative the special cases)
	$toti->result = FUNDED;

	if($toti->vetos and ($toti->vetos >= $votesAll*$minveto/100)) { // first flag vetos
		$toti->result = 'vetoed'; return -4;
	} elseif($toti->maximum and ($funds > $toti->maximum)) { // next flag too much funding (redistribute in any order)
		$toti->result .= '!'; // indicate full funding (not sure about this)
		$toti->funds = $toti->maximum; return -3; 
	} elseif($funds < $toti->minimum) {
		if($toti->mandatory) { // flag underfunded mandatories (any order)
			$toti->funds = $toti->minimum; return -2;
		} else { // and last flag the most insufficient funding
			$toti->result = 'not&nbsp;enough'; return $funds / $toti->minimum - 1;
		}
	} else return $score;
}

function getVoters($question) {
  global $proxiesForThisVoter, $thisvoter;

  // Create ballots for all nonvoters whose proxy has voted (first level proxy first, if both proxies have voted)
  $proxiesTable = <<<EOF
    SELECT p2.person, p2.proxy, u.community FROM r_proxies p2 
    INNER JOIN users u ON u.uid=p2.person 
    WHERE u.:IS_MEMBER
    ORDER BY p2.priority
EOF;

  $sql = <<<EOF
    INSERT INTO r_ballots (question, voter, proxy, created)
    SELECT DISTINCT :question, p.person, b.proxy, :now
    FROM ($proxiesTable) p
    INNER JOIN r_ballots b ON b.id=p.proxy
    INNER JOIN r_questions q ON q.id=b.question
    LEFT JOIN r_ballots b0 ON b0.id=p.person
    WHERE b.question=:question AND p.community=q.ctty AND b0.id IS NULL
EOF;

  do {
    $q = db\q($sql, u\ray('question now', $question, time()));
  } while ($q->rowCount());
  
	$q = db\q('SELECT proxy AS voter, COUNT(id) AS weight FROM r_ballots WHERE question=:question GROUP BY proxy', compact('question'));
	while($row = $q->fetchAssoc()) { // for each voter
		extract($row); 
//		if(ISDEMO) $weight = $voter == $thisvoter ? $proxiesForThisVoter : rand(1, 20);
		$voters[$voter] = $weight;
	}
	return @$voters ?: [];
}

function tallyVoter(&$tot, $voter, $question_type, $optlist, $weight) {
	$voterGrades = array();
	$tnm = 'r_votes v LEFT JOIN r_ballots b ON b.id=v.ballot';
//	$crit = "voter='$voter' AND question='$question'";
	$crit = "b.voter=:voter AND v.option IN($optlist)";
  $sql = <<<EOF
    SELECT v.option, v.grade, x.text 
    FROM ($tnm) LEFT JOIN r_vetoes x ON x.vote=v.id 
    WHERE $crit ORDER BY v.option
EOF;
	$q = db\q($sql, compact('voter'));
	if ($vetoCount = db\count($tnm, "$crit AND v.grade=:grade", u\ray('voter grade', $voter, VETOGRADE))) {
	  $vetoWeight = $weight / $vetoCount; // split one veto among the vetoed options
	  $weight -= $weight * ($vetoCount / $q->rowCount()); // reduce weight proportionately for each veto
  } else $vetoWeight = 0;
  
	while ($row = $q->fetchAssoc()) {
    extract($row);
    tallyVote($tot, $question_type, $option, $grade, $text, $weight, $vetoWeight, $voterGrades);
  }
}

/**
 * Tally each option as voted
 */
function tallyVote(&$tot, $question_type, $option, $grade, $text, $weight, $vetoWeight, &$voterGrades) {
	if(($question_type == 'B') or ($grade != BLANKGRADE)) { // don't count ungraded opts; but blank B option means 0%, so count it
		$tot[$option]->weight += $weight;
		if($grade == VETOGRADE) $tot[$option]->vetos += $vetoWeight; else $tot[$option]->grade += $weight * $grade;
	}
	if($question_type == 'M') foreach($voterGrades as $otheropt => $othergrade) { // for each other option already tallied
		if($othergrade != $grade) {
			list ($i, $j) = u\order($grade < $othergrade, $otheropt, $option); // i is preferred to j
			if ($grade != BLANKGRADE and $othergrade != BLANKGRADE) {
        $tot[$i]->prefs[$j] = @$tot[$i]->prefs[$j] + $weight; // ignore blanks
      }
		}
	}
	$voterGrades[$option] = $grade;
	if($text) {
		if($grade == VETOGRADE) $tot[$option]->veto_texts[] = $text; else $tot[$option]->canceled_vetotexts[] = $text;
	}
}
