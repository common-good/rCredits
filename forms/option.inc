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
function formOption($form, &$sta, $args = '') {
  global $base_url;

  extract(just('qid oid', $args));
  
  global $mya;
  if (!$add = empty($oid)) {
    $fields = 'e.id AS eid, e.ctty, q.id AS qid, o.text, o.detail';
    if (is_numeric($oid) and $option = db\get($fields, 'r_options o LEFT JOIN r_questions q ON q.id=o.question LEFT JOIN r_events e ON e.id=q.event', 'o.id=:oid', compact('oid'))) extract($option); else $add = FALSE;
    if ($ctty != $mya->community) return w\go('', t('Wrong community.'));

    $id = hidFld($oid);
  } elseif (empty($qid)) return w\go('', t('Missing question id.'));

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
  w\go("community/events/question/qid=$question", 'info saved');
}
