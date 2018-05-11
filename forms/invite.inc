<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 * NOTE: We allow agents to send invitations on behalf of a company, but be aware that companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever it invites -- something usually reserved for humans.
 */
function formInvite($form, &$sta) {
  global $base_url;
//  if ($agent = r\agent()) {
  global $mya;
  if ($mya->cAdmin and !$mya->proSe) say(t('Note: Admins cannot invite on behalf of someone.'), 'err');
  $stepsDone = (object) $mya->stepsDone;
  $nearly = ($mya->admin or $mya->can(B_MEMBER)) ?: (@$stepsDone->sign and @$stepsDone->donate);
  
  $title = t('Invite Someone to %PROJECT');
  $_a1 = w\atagB('/community/invite/waiting');
  $_a2 = w\atagB('/community/invite/invited-whom');
  $where = r\location($mya->cttyA); // currently UNUSED
  $inviteCode = $mya->iCardCode(IBY_EMAIL);
  $_inviteLink = PROMO_URL . "/signup/code=$inviteCode";
  $warning = $mya->co ? t('<p>Note you are sending an invitation on behalf of a company. The company is "trusting" whoever you invite. We at %PROJECT recognize that companies are not people, and that it\'s a bit odd to allow them a sentiment usually reserved for humans.</p>') : '';
  //Plus $@R_COUNTED_BONUSr for each employee, if they open a company account. 
  $subtext1 = t('We\'re sorry, you cannot invite someone to sign up until you have signed the <%aSign>Agreement</a> and made a <%aDonate>donation</a>.', '_aSign _aDonate', atag('/community/agreement'), atag('/community/donate'));
  //<br><br>Return to the <a href=\"%BASE_URL/status\">Membership Steps</a> page to see where you stand.
  //      <li>Order %orderCount sheets of 10 invitation cards by mail (free!). %orderSubmit</li>
//      <li><b>OR</b> Print %printCount sheets of 10 invitation cards. %printFront %printBack</li>

  $_aFront = atag('/community/invite/print', w\away());
  $_aBack = atag($mya->invitationBack());
//  list ($region, $zot) = explode(R_MEMBER_MARK, $mya->mainQid);
  $_aMailInvites = atag('mailto:' . CGF_EMAIL . '?subject=' . t('%PROJECT Invitation Card request') . '&body=' . t('Please send me some more %PROJECT invitation cards. Thanks!') . " -- $mya->fullName (account #$mya->mainQid)");
  $region = $mya->qo->region;
  foreach (ray('NEW MIW') as $k) if ($region == $k) {
    list ($nm, $zot) = explode('@', $mya->cttyA->email);
//    $aBack = str_replace('-back', '-back-' . $nm, $aBack);
  }
  $friend = $mya->co ? t('customer, employee, or supplier') : t('friend');
  $_size = $mya->co ? '4.25" &times; 2.75"' : '3.5" &times; 2"';
  
  $subtext2 = $mya->cttyRewardy ? t('<p>Get a $%R_HELPER_BONUS reward for each %friend you invite, who signs up!</p>', compact('friend')) : '';
  $subtext2 .= t(<<<EOF
<p>Invite <b>only people you trust</b> who live, shop, or work in your community. You may need to help or nudge the people you invite. <%a2>Who you have invited</a> <%a1>People waiting to be invited</a></p>
  <br>
  <p>Here are three different ways to invite someone:</p>
    <ul>
      <li><b>BEST: Print a sheet</b> of invitation cards, %size, on card stock: <%aFront>front</a>, <%aBack>back</a>.<br>(or <%aMailInvites>ask to have some mailed to you</a> at no charge)</li>
      <li><b>Tell your %friend</b> to sign in at %CG_DOMAIN with invitation #&nbsp;%inviteCode or send them this link: <div id=inviteLink>%inviteLink</div></li>
      <li><b>Complete the form below</b> to send an invitation NOW:</li>
    </ul>
EOF
  , compact(ray('_a1 _a2 _size where _aFront _aBack _aMailInvites _inviteLink inviteCode friend')));
//  (Later, you will need to confirm you know and trust them.)
  $subtext = !@$nearly ? $subtext1 : ($subtext2 . $warning);
//  $contact = textFld(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());

  $email = textFld(t('Email:'), [t('Their email address'), t('An email address for the person you want to invite')]);
//  $emails = areaFld(t('Email(s):'), t('A comma-separated list of email addresses to invite'));
  if ($mya->cAdmin) $dupsOk = boolFld(t('Dups Okay?'), '', FALSE);
  $trusted = boolFld(t('Trusted?'), t('If you lent this person (or people) $250, would you trust them to pay it back?'), $mya->cAdmin ?: NULL);
  $subject = textFld(t('Subject:'), t('The subject of your message'), required($mya->fullName . t(' invites you to %PROJECT')));

  $default = @$mya->data['inviteBody'] ?: t('invite default', 'name', $mya->fullName);
  $messageHelp = t('Invite them to sign up, and say why. A <%a>description of %PROJECT</a> will follow your message.', '_a', atag('/community/invite/example', w\away()));
  $message = areaFld(t('Message:'), $messageHelp, required($default));
  if ($mya->proSe or !$mya->cAdmin) $submit = t('Send'); // don't let cAdmins send on behalf of anyone
  jsx('invite-link');
  
  $form = $nearly ? 
      cgform(compact(ray('title subtext email dupsOk trusted subject message submit')))
    : compact(ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(just('email emails trusted', $sta['input']));
  if (op($sta) == 'submitN') {
    if (!$emails) return say('bad email', compact('emails'), 'emails');
    foreach (ray($emails) as $one) if (!u\validEmail($one)) return say(t('bad email: %one', compact('one')), 'emails');
  } elseif (!u\validEmail($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  global $base_url;
  extract(just('email emails subject message dupsOk', $sta['input']));
  global $mya;

  if (op($sta) == 'submitN') {
//    foreach (ray($email) as $one) sendInvite($one, $subject, $message, @$dupsOk);
    $code = ''; // get a random code just once
    foreach (ray($emails) as $email) {
      $code = r\invite($email, $mya->id, $code, $subject, $message);
      $links[] = "$base_url/signup/code=$code/e=$email";
    }
    $links = join('<br>', $links);
    say('invite links', compact('links'));
  } else {
    $email = strtolower($email); // otherwise constantSubs breaks on cap after @
    sendInvite($email, $subject, $message, @$dupsOk);
    $data = ray('inviteBody', substr($message, 0, 10000)) + $mya->data;
    $mya->update(compact('data'));
  }
}