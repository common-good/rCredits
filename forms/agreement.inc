<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Give member a chance to sign the rCredits Agreement. (agreement revised 4/17/2014)
 */
function formAgreement($form, &$sta) {
  global $mya;
  $title = t('%PROJECT Agreement');
  $name = 'checkNUMBER';
//  $id = "edit-$name";
//  $check = boxFld('checkNUMBER', '', '', attrib(compact('id')));
  $model = @\render(boxFld('checkNUMBER'));
  $_instructions = '';

  if (!$mya) {
    // nothing to be done if signed out
  } elseif ($date = $mya->signed) {
    $instructions = '';
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif (!$mya->co and !$mya->proSe and !$mya->cAdmin) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $signerA = $mya->cAdmin ? ($mya->co ? r\acct($mya->helper) : $mya) : $mya->agentA;
    $behalf = $mya->co ? t('(on behalf of %co) ', 'co', $mya->fullName) : '';
    $signedBy = textFld(t('Signed:'), $behalf . t('Type your full name here ') . "($signerA->legalName)", required($mya->cAdmin ? $signerA->legalNameDpy : ''));
    $setupFoot = w\setupFoot();

    $_addendum = $mya->cAdmin ? t('<p>ADMINISTRATORS need not put checkmarks in the boxes.</p>') : '';
    
    $instructions = t(<<<EOF
      <div id="instructions" class="well">
      <p><b>Instructions:</b> Put a check mark in each checkbox to indicate you have read and understand that section. If there are no participating businesses near you, some of this Agreement will not apply to you &mdash; <b class="loud">YET!</b> But when enough people have joined, you will be able to start a %PROJECT Community together quickly, to enjoy the benefits of economic democracy.</p>
      <p><b>Summary.</b> In the first part of the agreement (A), you acknowledge the intent and purpose of the %PROJECT system. In the second part (B), you agree to use the %PROJECT system the way it is designed to work.</p>
      %addendum
      </div>
EOF
    , compact('_addendum'));
  }

  $_personOnly = ($mya and $mya->co) ? '' : t(<<<EOF
  <li>[7] I agree to <%aParticipate>participate</a> in managing the Common Good Economy to the best of my ability and as my life circumstances allow.</li>
EOF
  , '_ol1 _aParticipate _aDecision _ol2 _aOther', 
    'ol class="abc"', atag('/help/participate'), atag('/help/decision-principles'), 'ol class="roman"', atag('/help/other-decisions'));

  $prolog = ($mya and $mya->co) ? t('<h4>As a responsible participant in the Common Good Economy:</h4>') : t(<<<EOF
%instructions
<h4>A. I recognize that:</h4>
<ul>
  <li>[-2] %RCREDITS are intended as the basis for the Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1]When I pay or accept payment in Common Good Credits, I am building the Common Good Economy.</li>
  <li>[0]When I pay attention, make suggestions for improvement, and participate in decisions with other Common Good members, I am building the Common Good Economy.</li>
</ul>

<h4>B. As a responsible participant in the Common Good Economy:</h4>
EOF
  , ray('_instructions _aIssue', @$instructions, atag('/help/issuing-credits')));
  
  $agreement = $prolog . t(<<<EOF
<ol>
  <li>[1] As long as I can spend my %RCREDITS easily or exchange them easily for US Dollars, I will accept them without limit as payment for my goods and services (or labor). I will exchange %RCREDITS for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will accept %RCREDITS from members of any <%aCGC>Common Good Community</a> in good standing.</li>
  <li>[3] If <%aCrunch>needed</a> I will <%aBack>back</a> %RCREDITS up to my average monthly use over the past six months.</li>
  <li>[4] Whenever I exchange %RCREDITS for US Dollars or vice versa, I will exchange them one for one, and I will price my goods and services (or labor) the same in %RCREDITS and in US Dollars. </li>
  <li>[5] I understand I may receive from the community a credit line that I can spend but cannot <%aCashOut>cash out</a>. If I use my credit line and then stop participating (or if my %PROJECT Community or the entire %PROJECT System <%aGrace>closes down</a>), I will bring my negative balance up to zero as soon as possible.</li>
  <li>[6] When there is a dispute in my dealings with other %PROJECT participants, I will follow the <%aDispute>Dispute Resolution Process</a> and will honor its outcome.</li>
  %personOnly
</ol>
</div>
EOF
  , '_div _aCGC _aCrunch _aBack _aCashOut _aGrace _aEthics _aDispute _personOnly',
  'div id="agreement"', atag('/help/common-good-community'), atag('/help/cashflow'), atag('/help/backing-common-good-credits'), atag('/help/cashing-out'), atag('/help/closing-down'), atag('/help/ethics'), atag('/help/dispute-resolution-process'), $_personOnly);

  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", (!$mya or !$mya->can(B_MANAGE) or @$date) ? '' : $box, $agreement);
  }
  $agreement = fld('item', '', '', $agreement);
  
  return cgform(compact(ray('title agreement signedBy')) + (@$setupFoot ?: []));
}

function formAgreement_validate($form, &$sta) {
  global $mya;
  list ($i0, $i9) = $mya->co ? [1, R_AGREE_9 - 1] : [R_AGREE_0, R_AGREE_9]; // companies omit prolog and democracy question
  if (!$mya->cAdmin) for ($i = $i0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  if (w\badSignature($sta)) return;
}

function formAgreement_submit($form, &$sta) {
  global $mya;
  $signed = r\rTime();
  extract(just('signedBy', $sta['input']));
  r\acct()->update($info = compact('signed', 'signedBy'));
  return w\goNextStep('sign');
}