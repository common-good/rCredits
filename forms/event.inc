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
 * Add or Edit an event
 * @param int $eid: event record ID: empty means Add, else do $do (defaults to edit)
 
 */
function formEvent($form, &$sta, $args='') {
  include_once R_ROOT . '/rvote/rvote.inc';
  include_once R_ROOT . '/rvote/rvote-results.inc';
  global $base_url, $mya;
	
	extract(just('do eid', $args));

  $typeDescs = ray(t('I:Meeting,V:Vote,G:Grading,P:RFP'));
  if (strpos(@$eid, '=')) extract(just('eid do', $eid)); // passed from rdo()
  
  if (isset($eid)) {
    $fields = 'type,event,details,start,end';
    if (is_numeric($eid) and $event = v\event($fields, $eid)) {
      extract($event); 
      $subtitle = item(t('for vote ending ') . u\fmtDate($end));
    }
  }

  $vote = (@$type == 'V');
	$grading = (@$type == 'G');
	
  if ($do == 'report') {
    include_once R_ROOT . '/rvote/rvote-results.inc';
    $title = item($typeDescs[$type] . t(' Results'));
    if ($vote or $grading) {
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
    if ($vote or $grading) { // must precede setting $type field
      list ($header, $page) = $vote ? [t('Questions'), 'question'] : [t('Proposals'), 'proposal'];
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
//		if ($vote) $subtext = btn("/community/events/question/eid=$eid", t('Add Question'), 'warning');
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
	$back = button(t('Back to Events'), "$base_url/community/events");
  
  return cgform(compact(ray('title back id list type event details start end submit')));
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
  w\go('community/events', 'info saved');
}
