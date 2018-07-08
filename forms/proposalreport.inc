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
 * Show a single proposal report.
 */
function formProposalReport($form, &$sta, $args = '') {
  include_once __DIR__ . '/../rvote/rvote.inc';
  include_once __DIR__ . '/../rvote/rvote-ballot.inc';
  extract(just('p', $args));
  
  $mya = w\eLinkAcct($sta, $args, 'acct'); // allow any signed in account, so user can switch to personal account, below

  if (!@$p or !$project = db\get('project', 'r_proposals', 'id=:p', compact('p'))) return w\softErr(t('No such proposal.'));
  
  v\showProposal($mya, $extra, -$p);

  $form['title'] = item(t('Project Proposal Details'));
  $form['project'] = item($project, t('Project:'));
//  for ($i = 0; isset($extra["f$i"]); $i++) $form["f$i"] = $extra["f$i"];
  $form += $extra ?: [];
  $form['back'] = item('<p>' . backButton() . '</p>');

  w\css('rvote.css');

  return cgform($form);
}
