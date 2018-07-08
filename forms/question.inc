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
 * Add or edit a question.
 */
function formQuestion($form, &$sta, $args = '') {
  global $base_url;
  
  extract(just('eid qid', $args));
  
  global $mya;
  if (!$add = empty($qid)) {
    $fields = 'e.id AS eid, e.ctty, q.text, q.detail, q.linkDiscussion, q.type, q.units, q.budget, q.optOrder';
    if (is_numeric($qid) and $question = db\get($fields, 'r_questions q LEFT JOIN r_events e ON e.id=q.event', 'q.id=:qid', compact('qid'))) extract($question); else $add = FALSE;
    if ($ctty != $mya->community) return w\go('', t('Wrong community.'));

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
  } elseif (empty($eid)) return w\go('', t('Missing event id.'));

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
  extract(just('type budget optOrder linkDiscussion', $sta['input']));
  if ($type == 'M' or $type == 'B') foreach (['type', 'optOrder'] as $k) if (w\blank_field(compact($k))) return NULL;
  if (!@$budget) $budget = 0;
	if ($linkDiscussion and !u\starts(strtolower($linkDiscussion), 'http')) $linkDiscussion = 'http://' . $linkDiscussion;
  if ($err = u\badAmount($budget)) return say($err, 'budget');
  u\preray(compact(ray('budget linkDiscussion')), $sta['input']);
}

function formQuestion_submit($form, &$sta) {
  global $base_url;
  extract($info = just('id event text detail linkDiscussion type budget optOrder', $sta['input']));
  db\update('r_questions', $info, @$id ? 'id' : '', TRUE);
  w\go("community/events/event/do=edit&eid=$event", 'info saved');
}
