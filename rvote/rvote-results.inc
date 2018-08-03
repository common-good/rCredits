<?php
namespace CG\Vote;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

/**
 * Return a display of Vote or Grading results for one event.
 * @param int $eid: event record ID OR community record ID (negative)
 * @return formatted event results
 */
function showResults($eid, $noProxies = FALSE) {
  if ($eid < 0 and !$eid = db\max('id', 'r_events', ray('ctty', $eid))) return t('This community has no events');
  list($quids, $qutypes) = getQuestions($eid); 
	$ans = '';
  if (@$quids) {
    $ctty = v\event('ctty', $eid);
	  foreach($quids as $i => $k) $ans .= showResult($ctty, $i + 1, $k, $noProxies, TRUE);
  }
	return $ans;
}

/**
 * Return results for one question.
 * @param int $ctty: community of the event
 * @param int $qi: question number (1, 2, etc.)
 * @param int $question: question record ID
 * @param bool $noProxies: <don't include proxy votes>
 * @param bool $withheader: <show a header above results for each question> (do we ever not want this?)
 * @return HTML for the voting results on the given question
 * to test: SELECT fullName,b2.proxy,COUNT(b2.id) AS weight,grade,gradeMax,grade*COUNT(b2.id)/287 AS low,gradeMax*COUNT(b2.id)/287 AS hi FROM r_ballots b JOIN r_votes v ON v.ballot=b.id LEFT JOIN users u ON u.uid=b.voter LEFT JOIN r_ballots b2 ON (b2.proxy=b.voter AND b2.question=b.question) WHERE v.option=31 GROUP BY b2.proxy
 */
function showResult($ctty, $qi, $question, $noProxies, $withheader = TRUE) {
	global $thisvoter, $mya; // only if ISDEMO
//    u\loga("0-$question", 'top');
	$qu = db\get('id, type, text, detail, budget, minveto, result', 'r_questions', 'id=:question', compact('question'));
  extract($qu);
  
  if ($mya->superAdmin or !@$result) {
//    if ($type == 'E') return ''; // don't show essay results (and don't create ballots for them)
    
    $opts = optList($question);

    $voters = getVoters($ctty, $question, $noProxies); // create proxy ballots, get voters and their weights

  //	global $proxiesForThisVoter; if(ISDEMO) if($thisvoter) $voters[$thisvoter] = $proxiesForThisVoter;
    $votesDirect = count($voters);
///		debug(compact(ray('votesDirect ctty question noProxies')));
    $votesAll = array_sum($voters); // total weights. not necessarily equal to total weight used, since vetos reduce effective weight
    $newtot = tallyResults($voters, $type, $budget, $minveto, $opts);
//    u\loga("1-$question", compact(ray('question text')));
    $result = getResult($qi, $text, $detail, $type, $budget, $newtot, $votesDirect, $votesAll, $withheader); // show result of one question, with links to audit info
//    u\loga("99-$question", $question);
    db\update('r_questions', compact('id', 'result'), 'id');
  }
  return $result;
}

function optList($question) {
	$q = db\q('SELECT id FROM r_options WHERE question=:question ORDER BY displayOrder', compact('question'));
	while($row = $q->fetchAssoc()) $opts[] = $row['id'];

	return @$opts ?: [];
}

function getResult($qi, $quText, $quDetail, $quType, $budget, $tot, $votesDirect, $votesAll, $withheader) {
	$optCount = count($tot);

	for($i=0, $optTextMaxLen=0; $i < $optCount; $i++) {
		$otxlen = strlen($tot[$i]->opttext);
		if($otxlen > $optTextMaxLen) $optTextMaxLen = $otxlen;
	}
//	$separateOptTexts = ($optTextMaxLen + $optCount * 7 > 100); // keep lines short by separating out the descriptions
//  $separateOptTexts = TRUE; // FAILS otherwise
  
  $prefHeads[1] = [
  'M' => "<th class='resultOption'></th><th colspan='$optCount'>Preferred Over:</th><th class='resultAverage'>Average</th><th></th>",
  'B' => "<th class='resultOption'></th><th></th><th></th><th>Percent</th><th colspan='2'>Budget</th><th class='resultAverage'>Average</th><th></th>",
  'R' => "<th colspan='2'>Range</th><th></th><th></th><th>Average</th><th>Average</th><th colspan='2'>Vetoed by 5%</th>",
  'E' => "",
	'Y' => "<th class='resultOption'>Yes</th><th>No</th>",
  ];
  
  $prefHeads[2] = [
  'M' => "<th class='resultOption'>Option</th>",
  'B' => "<th class='resultOption'>Option</th><th>Result</th><th>Amount</th><th>of total</th><th>Min</th><th>Max</th><th class='resultVote'>Vote</th><th>Vetos</th>",
  'R' => "<th>Min</th><th>Max</th><th>Result</th><th>Acceptance</th><th>Min Chosen</th><th>Max Chosen</th><th>Min</th><th>Max</th>",
  'E' => "",
  'Y' => "",
  ];

  foreach ([1, 2] as $i) $prefHead[$i] = $prefHeads[$i][$quType];

	for ($i = 0, $optTexts = $middle = ''; $i < $optCount; $i++) {
		$inum = $i+1; // option number for display
		$toti = $tot[$i];
		$otx = $toti->opttext;
		$optlinehdr = "<b>#$inum</b>";
		$vetos = number_format($toti->vetos, $quType == 'M' ? 0 : 1);

    if ($comments = join('</li><li>', array_merge(@$toti->canceled_vetotexts ?: [], @$toti->veto_texts ?: []))) {
      $showComments = w\btnShowDiv("comments$qi-$inum", t('comments'), 'warning');
      $comments = "<div id=\"comments$qi-$inum\" class=\"comment collapse\"><ul><li>$comments</li></ul></div>";
    } else $showComments = '';
    if ($detail = $toti->optdetail) {
      $showDetail = w\btnShowDiv("detail$qi-$inum", t('detail'), 'success');
      $detail = "<div id=\"detail$qi-$inum\" class=\"detail collapse\">$detail</div>";
    } else $showDetail = '';

//		  if($separateOptTexts) $optTexts .= "$prefix$otx $showDetail $showComments<br>\n$detail$comments"; else $optlinehdr .= ": $otx";
		$prefix = in($quType, 'E Y R') ? '' : "<b>$inum:</b> "; // don't number a single option
    $optTexts .= "$prefix$otx $showDetail $showComments<br>\n$detail$comments";
	
    $budgetdpy = '';

		if ($quType == 'M') {
			$prefHead[2] .= "<th>#$inum</th>";
			$mid = result_midM($toti, $inum);
  		$middle .= "<tr><td class='optlinehdr'>$optlinehdr</td>$mid<td>$vetos</td></tr>\n";
		} elseif ($quType == 'B') {
			$min = $toti->minimum ? (number_format($toti->minimum, 0) . ($toti->mandatory ? '!' : '&nbsp;')) : '- &nbsp;';
			$max = $toti->maximum ? number_format($toti->maximum, 0) : '- &nbsp;';
			$funds = $budget ? number_format($toti->funds, 0) : '? &nbsp;';
			$grade = number_format($toti->raw/BPCT2INT, 1) . '%'; // $toti->grade is irrelevant if we recursed
			$pct = $budget ? (number_format(100*$toti->funds/$budget, 1) . '%') : $grade;
			$mid = "<td>$toti->result</td><td>$funds</td><td>$pct</td><td>$min</td><td>$max</td><td>$grade</td>";
			$budgetdpy = ' (total budget: $' . number_format($budget, 0) . ')';
  		$middle .= "<tr><td class='optlinehdr'>$optlinehdr</td>$mid<td>$vetos</td></tr>\n";
		} elseif ($quType == 'R') {
      $fields = ray('minimum maximum result acceptance lowAvg highAvg lowVeto highVeto');
      foreach ($fields as $k) {
        $v = $toti->$k === '' ? '-' 
        : (($k == 'result' and strpos($toti->$k, '-')) ? $toti->$k // allow a-b
        : number_format($toti->$k, 0));
//        if ($k == 'result' and $toti->acceptance+0 < (1 - $toti->qminveto)) $v = t('none');
        if ($k == 'acceptance') $v = number_format($toti->$k * 100, 1) . '%';
        $middle .= "<td>$v</td>";
      }
      $middle = "<tr>$middle</tr>\n";
    } elseif ($quType == 'Y') {
			foreach (ray('yes no') as $k) {
				$v = $toti->$k;
				$pct = $votesAll ? u\fmtAmt($v / $votesAll, '%') : '0%';
				$v = number_format($v);
				$middle .= "<td>$v ($pct)</td>";
			}
		}
	}
  
  if ($quType == 'M') $prefHead[2] .= "<th class='resultVote'>Vote</th><th>Vetos</th>";

  $optTexts = $optTexts ? "<blockquote>$optTexts</blockquote>" : '<br>';

  if ($quDetail) {
    $_showDetail = w\btnShowDiv("detail$qi", t('details'), 'success');
    $quDetail = "<div id=\"detail$qi\" class=\"detail collapse\">$quDetail</div>";
  } else $_showDetail = '';
  
	$hdr = ($withheader ? t('<h4>Question %qi: %quText <small> %showDetail</small></h4>', compact(ray('qi quText _showDetail'))) : t('<br><b>RESULTS:</b><br>')) . "$quDetail<br>\n";

  $votesIndirect = $votesAll - $votesDirect;

	return <<<EOF
<div class="well">
$hdr
$votesAll voters: $votesDirect voted directly, $votesIndirect by proxy $budgetdpy<br>
$optTexts

<table class="resultstable">
<tr>$prefHead[1]</tr>
<tr>$prefHead[2]</tr>
$middle
</table>
</div>
<br>
EOF;
}

function result_midM($toti, $i) {
//  ksort($toti->prefs);
	for ($j = 1, $eachpref = ''; $j <= count($toti->prefs); $j++) {
		$prefij = ($i == $j) ? '- &nbsp;' : number_format(@$toti->prefs[$j], 0);
//		$prefij = ($i == $j) ? '- &nbsp;' : number_format($v, 0);
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

	if ($qtype == 'B') {
		if ($onescore < 0) { // a bad worst score means redo (mark as vetoed or underfunded/overfunded)
			if(substr($badtot->result, 0, strlen(FUNDED)) != FUNDED) $badtot->funds = 0; // vetoed or underfunded
			if(count($opts) > 1) {
				$newtot = tallyResults($voters, $qtype, $qbudget - $badtot->funds, $qminveto, $opts, $badtot->id); // refigure w/o vetoed/misfunded opt
				foreach($newtot as $one) $one->raw = $tot[$one->id]->raw; // put back the original grades
				$newtot[] = $badtot; // tack the handled option back on (last)
			}
		} elseif (!$badtot->funds) $badtot->result = 'unfunded';
	} elseif ($qtype == 'M') {
		foreach ($newtot as $k => $one) {
      foreach ($newKeys as $index => $i) $newPrefs[$index + 1] = @$one->prefs[$i];
      $newtot[$k]->prefs = $newPrefs;
    }
	} elseif ($qtype == 'R') {
    $opt = $optlist; // only one option record
    if ($voters) {
      $hilo = <<<EOF
        SELECT v.grade AS low, v.gradeMax AS high,
          (SELECT COUNT(*) FROM r_votes WHERE `option`=$opt AND v.grade BETWEEN grade AND gradeMax) AS lowOk,
          (SELECT COUNT(*) FROM r_votes WHERE `option`=$opt AND v.gradeMax BETWEEN grade AND gradeMax) AS highOk
        FROM r_votes v WHERE v.option =$opt
EOF;
      $sql = <<<EOF
        SELECT low, high, lowOk, highOk, IF(highOk < lowOk, 'low', 'high') AS hilo
        FROM ($hilo) x ORDER BY GREATEST(highOk, lowOk) DESC
EOF;

      $q = db\q($sql);
      $res = $q->fetchAssoc(); // get one maximally accepted amount
      extract($res);
      list ($result, $accepted) = [$$hilo, ${$hilo . 'Ok'}];
      $min = $max = $result;
      
      while ($row = $q->fetchAssoc() and max($row['lowOk'], $row['highOk']) == $accepted) { // get whole maximally accepted range
        extract($row);
        if ($hilo == 'low') $min = min($min, $low); else $max = max($max, $high);
      }
      if ($min != $max) $result = "$min-$max";
      $acceptance = $accepted / count($voters);
    } else $result = $acceptance = 0;

    $sql = <<<EOF
      SELECT AVG(grade) AS lowAvg, AVG(gradeMax) AS highAvg
      FROM r_votes WHERE `option` IN($optlist)
EOF;
    $res = db\q($sql)->fetchAssoc();
    extract($res);
    
    $vetoLimit = count($voters) * $qminveto; // must have fewer voters than this vetoing the min and max
    $vetoCount = <<<EOF
      SELECT COUNT(*) FROM r_votes v2 
      WHERE v2.isVeto AND v2.gradeMax <= v.gradeMax AND v2.option IN($optlist)
EOF;
    $sql = <<<EOF
      SELECT gradeMax AS highVeto FROM r_votes v 
      WHERE v.isVeto AND ($vetoCount) >= $vetoLimit AND v.option IN($optlist)
EOF;
    if ($res = db\q($sql)->fetchAssoc()) extract($res); else $highVeto = '';

    $vetoCount = <<<EOF
      SELECT COUNT(*) FROM r_votes v2 
      WHERE v2.isVeto AND v2.grade >= v.grade AND v2.option IN($optlist)
EOF;
    $sql = <<<EOF
      SELECT grade AS lowVeto FROM r_votes v 
      WHERE v.isVeto AND ($vetoCount) >= $vetoLimit AND v.option IN($optlist)
EOF;
    if ($res = db\q($sql)->fetchAssoc()) extract($res); else $lowVeto = '';
    
    u\preray(compact(ray('result acceptance lowAvg highAvg lowVeto highVeto qminveto')), $newtot[0]);
  } elseif ($qtype == 'Y') {
    if ($voters) {
			$sql = 'SELECT SUM(IF(grade=1, 1, 0)) AS yes, SUM(IF(grade=0, 1, 0)) AS no FROM r_votes WHERE `option`=:opt';
			$res = db\q($sql, ray('opt', $optlist))->fetchAssoc(); // only one option record
			u\preray($res, $newtot[0]);
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

	$score = ($optCount == 1) ? 1 : ($totalGrades ? $toti->grade/$totalGrades : 0);
	$toti->funds = $funds = $score * $budget; // how much funding did this get? (score negative the special cases)
	$toti->result = FUNDED;

	if($toti->vetos and ($toti->vetos >= $votesAll*$minveto/100)) { // first flag vetos
		$toti->result = 'vetoed'; 
    return -4;
	} elseif($toti->maximum and ($funds > $toti->maximum)) { // next flag too much funding (redistribute in any order)
		$toti->result .= '!'; // indicate full funding (not sure about this)
		$toti->funds = $toti->maximum; 
    return -3; 
	} elseif($funds < $toti->minimum) {
		if($toti->mandatory) { // flag underfunded mandatories (any order)
			$toti->funds = $toti->minimum; 
      return -2;
		} else { // and last flag the most insufficient funding
			$toti->result = 'not&nbsp;enough'; 
      return $toti->minimum ? $funds / $toti->minimum - 1 : 0;
		}
	} else return $score;
}

/**
 * Create proxy ballots. Return an array of voter weights indexed by voter record ID.
 */
function getVoters($ctty, $question, $noProxies = FALSE) {
  global $proxiesForThisVoter, $thisvoter;

  // clear any proxies and blank votes
  db\q('DELETE FROM r_ballots WHERE proxy<>voter AND question=:question', compact('question')); // remove proxy votes
  db\q("DELETE r_votes FROM r_votes LEFT JOIN r_ballots ON ballot=r_ballots.id WHERE grade=:BLANKGRADE AND (text='' OR text IS NULL) AND question=:question", compact('question')); // remove blank votes
	db\q('DELETE b FROM r_ballots b LEFT JOIN r_votes v ON v.ballot=b.id WHERE b.proxy=b.voter AND question=:question AND v.id IS NULL', compact('question')); // remove unused ballots
  
  // Create ballots for all nonvoters whose proxy has voted (first level proxy first, if both proxies have voted)
  $proxiesTable = <<<EOF
    SELECT p2.person, p2.proxy, u.community FROM r_proxies p2 
    INNER JOIN users u ON u.uid=p2.person 
    WHERE u.:IS_MEMBER AND p2.priority=:priority
EOF;

  $sql = <<<EOF
    INSERT INTO r_ballots (question, voter, proxy, created)
    SELECT DISTINCT :question, p.person, b.proxy, :now
    FROM ($proxiesTable) p
    INNER JOIN r_ballots b ON (b.voter=p.proxy AND b.created<>:justCreated AND b.question=:question)
    INNER JOIN r_questions q ON (q.id=b.question)
    LEFT JOIN r_ballots b0 ON (b0.voter=p.person AND b0.question=q.id)
    WHERE b0.id IS NULL AND p.community=:ctty
EOF;

  $priority = 1; // start with first choice
  $failsInARow = 0;
  if (!$noProxies) do {
    $justCreated = ($priority == 1) ? time() + 999 : $now; // "now" is time of previous query
    $now = time();
    $q = db\q($sql, $subs = compact(ray('question now justCreated priority ctty')));
    $priority = 3 - $priority; // toggle 1 or 2
    if ($failsInARow = $q->rowCount() ? 0 : ($failsInARow + 1)) sleep(1); // let the creation time progress
///    debug(compact('sql', 'subs', 'failsInARow', 'question'));
///    debug($q->rowCount());
  } while ($failsInARow < 2);
  
  $fld = $noProxies ? 'voter' : 'proxy';
	$q = db\q("SELECT $fld AS voter, COUNT(id) AS weight FROM r_ballots WHERE question=:question GROUP BY $fld", compact('question'));
	while($row = $q->fetchAssoc()) { // for each voter
		extract($row); 
//		if(ISDEMO) $weight = $voter == $thisvoter ? $proxiesForThisVoter : rand(1, 20);
		$voters[$voter] = $weight;
	}
	return @$voters ?: [];
}

function tallyVoter(&$tot, $voter, $quType, $optlist, $weight) {
	$voterGrades = array();
	$tnm = 'r_votes v LEFT JOIN r_ballots b ON b.id=v.ballot';
//	$crit = "voter='$voter' AND question='$question'";
	$crit = "b.voter=:voter AND v.option IN($optlist)";
  $sql = <<<EOF
    SELECT v.option, v.grade, v.gradeMax, v.text, v.isVeto
    FROM ($tnm) WHERE $crit ORDER BY v.option
EOF;
	$q = db\q($sql, compact('voter'));
	if ($vetoCount = db\count($tnm, "$crit AND v.isVeto", compact('voter'))) {
	  $vetoWeight = $weight / $vetoCount; // split one veto among the vetoed options
	  $weight -= $weight * ($vetoCount / $q->rowCount()); // reduce weight proportionately for each veto
  } else $vetoWeight = 0;
  
	while ($row = $q->fetchAssoc()) {
    tallyVote($tot, $quType, $row, $weight, $vetoWeight, $voterGrades);
  }
}

/**
 * Tally each option as voted
 */
function tallyVote(&$tot, $quType, $info, $weight, $vetoWeight, &$voterGrades) {
  extract(just('option grade gradeMax text isVeto', $info));
	if(($quType == 'B') or ($grade != BLANKGRADE)) { // don't count ungraded opts; but blank B option means 0%, so count it
		$tot[$option]->weight += $weight;
		if (@$isVeto) $tot[$option]->vetos += $vetoWeight; else $tot[$option]->grade += $weight * $grade;
	}
	if($quType == 'M') foreach($voterGrades as $otheropt => $othergrade) { // for each other option already tallied
		if($othergrade != $grade) {
			list ($i, $j) = u\order($grade < $othergrade, $otheropt, $option); // i is preferred to j
			if ($grade != BLANKGRADE and $othergrade != BLANKGRADE) { // ignore blanks
        $tot[$i]->prefs[$j] = @$tot[$i]->prefs[$j] + $weight;
      }
		}
	}
	$voterGrades[$option] = $grade;
  if ($quType == 'R') $voterGrades['max'] = $gradeMax;
	if(@$text) {
		if(@$isVeto) $tot[$option]->veto_texts[] = $text; else $tot[$option]->canceled_vetotexts[] = $text;
	}
}

/**
 * Return details for a vote or grading event.
 *//*
function getDetails($eid) {
  list($quids, $qutypes) = getQuestions($eid); 
	$ans = '';
  if (@$quids) foreach($quids as $i => $question) $ans .= getQuestionDetail($question, $i + 1);
	return $ans;
}
*/
/**
 * Return details for one question.
 *//*
function getQuestionDetail($question, $qnum) {
  $qu = v\question($question);
  $tableHeader = tableHeader($qu['type']);
  if($qu['linkDiscussion']) $qu['linkDiscussion'] = "<div class='discussionlink'> [<a href='$qu[linkDiscussion]' target='_blank'>See discussion</a>]</div>";

  $opts = optList($question);
  foreach ($opts as $i => $opt) {
  SELECT o.text, o.detail FROM (r_options 
  $qu['detail'] = u\lf2br($qu['detail']);

}*/