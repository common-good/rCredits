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
 * Show notes for a single proposal report.
 */
function formProposalNotes($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  extract(just('p', $args));

  if (!@$p or !$project = db\get('project', 'r_proposals', 'id=:p', compact('p'))) return w\softErr(t('No such proposal.'));

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
