<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Admin as a;
use CG\Risk as k;

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 * UNUSED @param string $qid: (optional) ID or qid of the account to summarize and manage (for cAdmin)
 */
function formSummary($form, &$sta) {
  global $base_url;
//  if (!$mya = r\acct($qid)) return w\go(isGAME ? 'community/game' : PROMO_URL);
  
  global $mya; $myid = $mya->id;

//  if ($qid and $mya and $a = r\acct($qid, $myid)) return w\changeWho("acct=$qid");

//  if (($mya->cAdmin or $mya->isAdmin2) and $qid and $a = r\acct($qid) and ($mya->admin or $mya->isAdmin2 or $a->community == $mya->agentA->community)) {
/*    w\setAcct($a->id); // switch to this account until further notice
    w\go('summary'); // refresh page, switching to new account
  } */

  if ($mya->closed) say(t('Your account is closed, but you can still see your account history.'));

  $stepUrl = $mya->nextStepUrl('', $msg);
  if ($stepUrl == 'signup' and $info = r\relation('reid,code', "other=:myid AND :IS_CUSTOMER AND code<>''", compact('myid'))) {
    extract($info);
    $stepUrl .= "/reid=$reid&customer=$code";
  }
  
  $extras = '<br>' . u\fmtPhone($mya->phone) . " ($mya->faxetc)<br>$mya->email ";

  if ($mya->admin2 and !$mya->proSe) { // some kind of admin (but not self)
    include_once __DIR__ . '/../admin/admin-web.inc';  
    $form = w\adminSummary();
    foreach (ray('photo legalName shortName federalId community') as $k) if (@$form[$k]) { // move these up (see return line)
      $$k = $form[$k];
      unset($form[$k]);
    }

    if (!$mya->isCtty and $mya->cAdmin and !$mya->co) $extras .= a\cardLink($mya->id) . ' ' . a\recropLink();
    
    if ($stepUrl == 'summary') {
      if (@$msg) say(t('New member is told: ') . t($msg));
    } elseif ($stepUrl) say(t('New member will be sent <a href="%stepUrl">there</a> upon signing in.', 'stepUrl', "$base_url/$stepUrl"));
    
  } else { // normal
    if ($stepUrl and $stepUrl != 'summary') return w\go($stepUrl);
    if (@$msg) say($msg);
    
    if ($mya->legalName != $mya->fullName) $legalName = item($mya->legalName, t('Legal Name:'));    
    //extract(just('r usd pAccts', r\stats($mya->community)));
// Don't show return because that's not the point of rCredits
//    $returnMo = item("$returnMo% APR " . t('over the past month'), t('Your return:'));
//    $return = item("$return% APR " . t('overall (ever)'), ' ', t('Your effective rate of return on your average balance [<@a>details</a>]', ray('@a', 'a href=help/your-return')));
  }

  extract(just('returnMo return cttyBeneMo cttyBene', $mya->j_stats()));
  $gifts = u\fmtAmt($mya->totalGifts());
//  $cttyBene = item("$cttyBeneMo " . t('over the past month') . "<br>$cttyBene " . t('overall (ever), including %gifts in contributions', compact('gifts')), t('Social return:'), t('Financial benefit to your community to date, from your participation together with other members, empowering economic democracy! [<%a>details</a>]', '_a', atag('/help/social-return')));  
  $cttyBene = NULL;
//  $cttyBene = item("$cttyBene " . t('overall, including your %gifts in contributions', compact('gifts')), t('Social return:'), t('Financial benefit to your community to date, from your participation together with other members, empowering economic democracy! [<%a>details</a>]', '_a', atag('/help/social-return')));  
    
  // $showCustomers = item(w\btn('/community/data/qName=6Customers', t('Customer List')));

  if ($mya->canManage) {
    $hasMultipleAccounts = ($mya->co or db\exists('r_relations r LEFT JOIN users u ON u.uid=r.main', 'other=:id AND permission>0 AND u.:IS_CO', ['id' => $mya->agentId]));
    $another = $hasMultipleAccounts ? t('Another') : t('a');
    $makeCo = w\btn('/signup-company', t('Open %another Company Account', compact('another')), 'primary', 'xs');
//    if (!$mya->jid and !$mya->co) $makeJoint = w\btn('/signup/joint=1', t('Make this a joint account'), 'primary', 'xs');
    if ($mya->member and !$mya->jid and !$mya->co) $makeJoint = w\btn('/prejoin', t('Make this a joint account'), 'primary', 'xs');
    if (!$mya->co and $hasMultipleAccounts) $clickPhoto = item(t('Click your photo%where to manage your other account(s).', 'where', $mya->cAdmin ? t(' in the upper right corner') : ''));
  }

  if ($mya->co) $reports = w\btn('/company-reports', t('Reports'));
  if ($mya->id == r\cgfId()) $deposits = w\btn('/sadmin/show-deposits', t('Deposits'));
  $topButtons = item(@$makeCo . @$makeJoint . @$reports . @$deposits);

  $title = t('Account Summary');
  if (isDEV and !$mya->proSe) $agentName = ' &nbsp; &nbsp; ' . t('<b>Agent: </b>') . $mya->agentA->fullName;
  $accountName = item($mya->j_fullName . ($mya->jid ? '<br>' : ' ') . "($mya->j_name)" . @$agentName, t('Name:'));
  $address = item($mya->postalAddr . @$extras, t('Postal Addr:'));
  if (!$mya->isCtty) { // unless this is a community or region account
    $acctType = $mya->co ? t('company account') : ($mya->jid ? t('joint account') : t('personal account'));
    $balance = u\fmtAmt($mya->balDpy);
//    $totalDesc = u\fmtAmt($mya->r);
//    $balanceDesc = $mya->savings ? t(' plus your savings, for a total of @total', ['@total' => $totalDesc]) : '';
    $balanceDesc = '';
    if ($mya->rewards) $balanceDesc .= t(' (see <%a>History</a>)', '_a', atag('/history'));
    $balance = item($balance . "<small>$balanceDesc</small>", t('Balance:'));
//    $savings = item(u\fmtAmt($mya->savings) . t('<small> (your total incentive rewards to date)</small>', [
//    list ($rewardNote, $rewardDesc) = $mya->cttyRewardy ? [t('<small> (your total incentive rewards to date)</small>'), t('This amount will be yours to keep, once your community reaches a certain size.')] : ['', t('Some of this amount may be yours to keep, at your %PROJECT community\'s decision, once membership reaches a certain size.')];
//    $savings = item(u\fmtAmt($mya->rewards) . $rewardNote, t('Credit Line:'), $rewardDesc);
    $suffix = $mya->debt ? '' : t(' (<%a>Click here</a> to let your balance go negative.)', '_a', 'a id="activate-credit"');
    $creditLine = item(u\fmtAmt(-$mya->floor), t('Credit Line:'), t('Based on your account\'s average monthly activity over the past 6 months.'), w\suffix($suffix));
//    $committed = item(u\fmtAmt(max(0, $mya->committed)), t('Committed:'), t('reserved for donation to Common Good Finance'));

    $successes = db\count('users', 'helper=:myid AND activated<>0', compact('myid'));
    $invites = max($successes, db\count('r_invites', 'inviter=:myid', compact('myid')));
    $doInvite = $mya->cAdmin 
    ? w\btn('/community/invite/print', t('front')) . ' ' . w\btn($mya->invitationBack(), t('back'))
    : t('<small>[<%a>invite someone</a>]</small>', '_a', atag('/community/invite'));
    $invites = item(t('You have invited at least %invites people (%successes are now members). ', compact('invites', 'successes')) . $doInvite, t('Invitations:'));
  } else $acctType = t('community account');

  if ($mya->cAdmin) {
    $extra = " id#$mya->id";
    if (FALSE and $bankInfo = $mya->bankInfo) {
      extract($bankInfo, EXTR_PREFIX_ALL, 'b');
      $bankInfo = "$b_name (routing #$b_route)\n$b_address, $b_city, $b_state $b_zip";
      $extra .= ' ' . w\popHelp(t('bank info'), $bankInfo, '#'); //strtr(t(' <@a>bank info</a>'), ['@a'=>$bankInfo]); // strtr circumvents Drupal crap
    }
  } else $extra = '';
  $accountId = item(($mya->mainQid ?: 'N/A') . ' (' . $acctType . ')' . $extra, t('ID:'));

  $fields = 'photo title clickPhoto showCustomers topButtons accountId accountName details address legalName shortName federalId community balance creditLine creditLimit committed invites cttyBene';
  foreach (ray('details') as $k) $$k = @$form[$k]; unset($form[$k]); // put these on the left if admin
  $fields = compact(ray($fields));
  
//  if ($mya->superAdmin and !$mya->ok) $onSubmit = w\whoFldSubmit('helper', t('This person was invited by %name, yes?'));
  if ($mya->admin2 and !$mya->proSe) {
    $class = ['class' => ['admin']];
    $right = w\fieldSet('right2', $form, '', $class); // the admin part
    $left = w\fieldSet('left2', $fields, '', $class);
    $form = compact('right', 'left');
    $form['none'] = submit('none'); // force _submit despite lack of submit button    
  } else $form = $fields;

  if ($mya->superAdmin and $mya->proSe) $form = just('title accountId accountName', $form);
  
  jsx('summary');
  
  return cgform($form, '', TRUE);
}

function formSummary_validate($form, &$sta) {
//  if (!r\acct()->cAdmin2) return hack('non-admin submitted summary form');
  if (in(op($sta), 'openCompany openJoint')) return;

  extract(just('uid legalName federalId creditLimit rTrader helper mediaConx moves tickle adminable', $sta['input']));
//  $a = r\acct(@$uid);
  global $mya;
  
  if (@$tickle !== '' and $err = u\badDate($tickle)) return say($err, 'tickle');
  
  if (@$federalId == R_ON_FILE or @$federalId == t('SSN')) {
    unset($sta['input']['federalId']);
  } elseif (@$federalId and $err = u\badSsn($federalId, $mya->dob)) return say($err, ['what' => 'federalId'], 'federalId');

  if (@$helper) {
    if (!$h = whois($helper, 'helper', $sta['input'])) return say(t('I don\'t know what helper account you mean.'), 'helper');
    $helper = $h->id;
  } else {unset($helper); unset($sta['input']['helper']);}

  if (@$rTrader and !$mya->ok and !$mya->isCtty) {
    if (!@$helper) return say(t('You must specify who gets the helper reward.'), 'helper');
//    if (!@$helper or $helper == 1) return say(t('You must specify who gets the helper reward.'), 'helper');
    if (is_null(@$adminable[B_MEMBER])) return say(t('That account is not READY (not a member).'), 'adminable'); // test "is_null" because 0 is the value when set
    if (!$mya->confirmed) say(t('NOTE: Member %helperName has not yet confirmed this invitation.', 'helperName', $h->fullName));
  }

  foreach (ray('creditLimit') as $k) {
    if (isset($$k)) {
      if ($err = u\badAmount($$k)) return sayFieldErr($err, $k); 
      $$k = u\cleanAmount($$k);
    }
  }
  if (@$rTrader and isset($mediaConx) and $err = u\badAmount($mediaConx, '>=0', 0)) return sayFieldErr($err, 'mediaConx');
  if (isset($moves) and $err = u\badAmount($moves, '>=0', 0)) return sayFieldErr($err, 'moves');
//  if (@$who and !$who = whois($who, 'who', $sta['input'])) return say(t('I don\'t know what account you mean to go to.'), 'who');
  u\preray(compact(ray('tickle helper who creditLimit')), $sta['input']);
}

function formSummary_submit($form, &$sta) {
  global $mya;
  $op = op($sta);

  if ($op == 'secureMsgs') return w\go('', showSecureMessages());
  if (!$mya->admin2) return; // hack attempt

  extract($args = just('uid who legalName federalId creditLimit tickle note notes helper mediaConx moves rTrader risks adminable', $sta['input']));
/**/  if (!$mya->cAdmin) u\EXPECT(!array_diff(array_keys($args), ray('uid tickle note')), 'too many unpermitted changing fields in ' . print_r($args, 1));
//  $a = r\acct(@$uid); // same as $mya if no uid passed

  if (!@$tickle) {
    $tickleDone = $mya->ok ? ($note and !$tickle and $mya->tickle) : 'activate';
    list ($zot, $tickle) = $mya->nextTickle($tickleDone);
//    if ($tickleDone) $note = trim(t('(%typeDone) ', 'type', @$note ? '': "$tickleType ") . @$note);
//    if ($tickleDone === TRUE) $note = trim(t('Done ') . @$note);
  }
  if (@$note) $notes = $mya->stampNote($note) . (isset($notes) ? $notes : $mya->notes);

  $aa = $mya->agentA;
  /*
  if (@$who and ($who->community == $aa->community or $aa->id == 1)) {
    svar('myid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $mya->fullName);
    return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
  } 
  */
  
  $DBTX = \db_transaction();

  if (!$mya->isCtty) {
    $fullName = $mya->fullName;
    if (FALSE and !$mya->confirmed and @$helper and @$helper != $mya->helper) { // helper has not confirmed yet (must be self-signup)
      $_a1 = r\acct($helper)->makeDo('confirmInvite', $mya->id); // link to confirm invitedness
      r\message($helper, 'confirm invite', compact('_a1', 'fullName')); // ask inviter to confirm
    }

    if (@$rTrader and !$mya->ok) {
      $mya->activate($helper);
    } elseif (@$federalId and $federalId != $mya->federalId) $mya->update('ssnData', '');
  }

  if (isset($rTrader)) $mya->setBit(B_OK, @$rTrader, TRUE);
  if ($mya->cAdmin or $mya->admin) setRisksAndFlags($mya, @$risks, @$adminable); // don't test for isset (allow turning off all bits)
  if ($mya->helper and ($mya->ok or !@$helper)) unset($helper); // don't overwrite if too late or empty
//  $floor = -$creditLimit; // floor is calculated automatically
  $mya->update(compact(ray('legalName federalId tickle notes helper mediaConx moves')));
  
  unset($DBTX);
  
  say('info saved');
}

function setRisksAndFlags($a, $risks, $adminable) {
  $risks0 = $a->risks;
  $a->setRisks($risks); // set or unset the adminable risk bits
  if ($a->risks != $risks0) { // if anything changed, refigure ALL risks (some affect others)
    include_once __DIR__ . '/../cg-risk.inc';
    k\acctRisk1($a->id);
  }
  $a->setBits(@$adminable);
}
