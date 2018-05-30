<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  global $mya;

  $nameDesc = t('usename desc');
  if ($mya->ok) $nameDesc .= '<br>' . ($mya->co ? t('Your username and %PROJECT webpage address') : t('triggers new card', 'field', t('name')) . t('Your username')) . t(' will also change.');
  $title = t('Contact Information');
  $fullName = textFld($mya->co ? t('Company Name:') : t('Your Name:'), $nameDesc, required(@$mya->fullName));
//  $onchange = w\onchange("jQuery('.form-item-pass').show();");
  $email = textFld(t('Email:'), '', required(@$mya->email));
  if (!$mya->cAdmin) $pass = passFld(t('Password:'), t('Required for changing email address'));

  $submit = submit();
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost

  $form = compact(ray('title fullName email pass')) 
        + contactFields([], FALSE, $mya->id)
        + compact('submit');

        
  jsx('contact');
//  return cgform($form, 'return setPostalAddr(false);');
  return cgform($form);
}

function formContact_validate($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract(u\normalizeCase(just($normals = 'fullName city address postalAddr', $sta['input'])));
  extract(just('email pass phone country state zip', $sta['input']));

  // what's this doing in validate?!
//  js('countries.js', 'file', 'header'); // country and state dropdowns
//  js("print_country(\"$country\", \"$state\");", 'inline', 'footer'); // initialize dropdowns

  if ($fullName != $mya->fullName) {
    if ($err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');
    $name = u\shortName($fullName);
    $otherName = db\get('fullName', 'users', 'name=:name AND uid<>:myid', compact('name', 'myid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
  }

  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!emailOkay($email, $mya->co, $mya)) return; // say('bad email', ray('email', $email), 'email');
  if (!$mya->cAdmin and $email != $mya->email and !$mya->agentA->passwordOkay($pass, 'pass', $err)) return say($err, 'pass');
//$pin != $mya->agentA->pin) return say('bad pin', 'pin');
  if (!phoneOk($phone, $country)) return;
  if (u\poAddress($address)) return say('po in location', 'address');
  
  u\preray(compact(ray($normals . ' email phone name')), $sta['input']); // fix formatting and add name
}

function formContact_submit($form, &$sta) {
  global $partnerInfo; // information used within eachPartner function argument (so must be global)
  extract($info = just('fullName name email phone faxetc country zip state city address postalAddr owns', $sta['input']));
  unset($info['owns']);
  
  global $mya;
  if ($mya->id == 1) unset($info['name']); // don't change 'admin'
  
  $info = u\normalizeCase(just('fullName city address postalAddr', $info)) + $info;
  $partnerInfo = just('email phone address city state zip postalAddr', $info); // what to tell partner about

  $mya->eachPartner(function($coA, $custA, $custCode) { // tell partner what changed
    global $partnerInfo;
    foreach ($partnerInfo as $k => $v) if ($custA->$k == $v) unset($partnerInfo[$k]);
    if ($partnerInfo and $api = $coA->api()) u\post($api, ray('partnerCode qid op name', $coA->emailCode, $mya->mainQid, 'change', $custA->fullName) + $partnerInfo);
  });
  
  $mya->setRisk('rents', !@$owns); // @ for tests
  $mya->update($info);
  say('info saved');
  //  return w\goNextStep('contact', NULL, $info);
}