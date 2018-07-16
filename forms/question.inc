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
  global $base_url, $mya;
  
  extract(just('eid qid', $args));
  
  if (!$add = empty($qid)) {
    $fields = 'e.id AS eid, e.ctty, q.text, q.detail, q.linkDiscussion, q.type, q.units, q.budget, q.optOrder';
    if (is_numeric($qid) and $question = db\get($fields, 'r_questions q LEFT JOIN r_events e ON e.id=q.event', 'q.id=:qid', compact('qid'))) extract($question); else $add = FALSE;
    if ($ctty != $mya->community) return w\go('', t('Wrong community.'));

    $id = hidFld($qid);
    $q = db\q('SELECT id AS oid,text,detail FROM r_options WHERE question=:qid ORDER BY id', compact('qid'));
    $button0 = button('%text', "$base_url/community/events/option/oid=%oid", '', 'warning');

    if ($realOpts = !in($type, 'R Y E')) {
			$header = t('Options') . ' ' . button(t('Add'), "$base_url/community/events/option/qid=$qid", '', 'warning');
			$list = '';
			while ($row = $q->fetchAssoc()) {
				extract($row, EXTR_PREFIX_ALL, 'o');
				$buttons = tr($button0, 'text oid', t('Edit'), $o_oid);
				$list .= "<tr><td>$o_text</td><td>$buttons</td></tr>";
			}
			if ($list) {
				$optHdr = $realOpts ? '<tr><th>Option</th></tr>' : '';
				$list = "<table>$optHdr$list</table>";
			} else $list = t('no options yet');
			
			$list = item('<h4>' . $header . '</h4>' . $list);
		} else { // single option is really just details
			$row = $q->fetchAssoc();
			$list = tr($button0, 'text oid', t('Edit Details'), $row['oid']);
		}
  } elseif (empty($eid)) return w\go('', t('Missing event id.'));

  $title = item($add ? t('Add Question') : t('Edit Question'));
  $event = hidFld($eid);
  $text = textFld(t('Question:'), [t('Question Text')], required(@$text));
  $detail = areaFld(t('Detail:'), [t('Question Detail'), t('Arbitrarily complex description, using <%a>markdown</a>. Use "&#92;" for line breaks.', '_a', atag('http://commonmark.org/help/'))], dft(@$detail));

  $linkDiscussion = textFld(t('Discussion Link:'), [t('Link to Discussion')], dft(@$linkDiscussion));
  $types = ray(' :Choose Type,M:Multiple choice,B:Budget (penny vote among various options),R:Range,Y:Yes/No,E:Essay');
  $type = selectFld(t('Type:'), '', required(@$type), $types);
//  $units = textFld(t('Units:'), [t('Units'), t('Units for Budget or Range question. Defaults to dollars.'), dft(@$units));
  $budget = textFld(t('Budget:'), [t('Budget Amount'), t('If this is a Budget question, how much (money) is to be budgeted.')], dft(@$budget));
  $optOrders = ray('N:No change (static),S:Shuffle (randomize),Z:Reverse sometimes');
  $optOrder = selectFld(t('Option order:'), ['', t('In a Multiple choice or Budget question, how to scramble the options, if at all')], dft(@$optOrder), $optOrders);
  
  if ($mya->cAdmin) $submit = submit();
	$back = button(t('Back to Event'), "$base_url/community/events/event/do=edit&eid=$eid");
  
  return cgform(compact(ray('title back id list type event text detail linkDiscussion budget optOrder submit')));
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
	
	$qid = db\update('r_questions', $info, @$id ? 'id' : '', TRUE); // qid is valid only if adding a new question (!$id)
	if (!v\realOpts($type) and !@$id) { // these question types have a single option
		db\insert('r_options', ray('question text maximum', $qid, '', $type == 'R' ? $budget : 0));
	}
	
	//  w\go("community/events/event/do=edit&eid=$event", 'info saved');
  w\go('community/events/question/qid=' . (@$id ?: $qid), 'info saved');
}
