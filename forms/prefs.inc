<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formPrefs($form, &$sta) {
  global $base_url;
  global $mya;
  $title = t('Account Preferences');
//  $virtualMaxMessage = $mya->co ? t(' If you choose to enable payment exchanges, this setting is moot (irrelevant and unused).') : '';
//  $subtext = item(t('The suggested settings are best for most people. You can change your choices at any time.'));

  $roundup = w\roundupFld($mya);
  $crumbs = $mya->crumbs * 100;
  $crumbs = textFld(t('Crumbs (%):'), [t('Donate a small percentage of what you receive.'), t('Another way to fund the common Good! Contribute a small percentage of every payment you receive. For example, "1%" means donate one penny for each dollar you receive. Smaller amounts such as 0.5% are also very welcome.')], dft($crumbs ? "$crumbs%" : ''));

  $mediumRadios = array(
    t('I will accept electronic statements**.'), 
    t('I choose to receive printed statements by mail. <div>I agree to pay $%R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>')
  );
  $statements = radiosFld(t('Statements:'), '', dft((int) $mya->can(B_PAPER)), $mediumRadios);

  $showAdvancet = item(t('<a>Show advanced options</a>'), ' '); // different name from showAdvanced (for CSS)

  // Advanced settings
//  if ($mya->ok and $mya->share) $share = w\shareFld($mya); // don't show this during signup (it's on the donation page)

  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));
  $dft = $mya->can(B_WEEKLY) ? 1 : ($mya->can(B_MONTHLY) ? 2 : 0);
  $notices = radiosFld(t('Email Notices:'), t('How often, at most, do you want an email digest of your %PROJECT notices? (No email if no notices.)'), dft($dft), $noticeRadios);
//  $smsNotices2 = boxFld('smsNotices', t('SMS notices:'), t('Send me a text message when I receive a payment, charge, or invoice (requires a <a href="' . $base_url . '/settings/boxes">connected cell phone</a>).'), $mya->can(B_BYSMS)); // (currently disabled)

  $secretBal2 = boxFld('secretBal', t('Secret Balance:'), t("Don't let merchants see my balance, even if I ask."), $mya->can(B_SECRET));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $nosearch2 = boxFld('nosearch', t('No Search:'), t("Require other members to identify my account by name or account ID only &mdash; not by phone or email."), $mya->can(B_NOSEARCH));
  $debtOk2 = boxFld('debtOk', t('Debt Okay:'), t('Let me use my %PROJECT card as a <i>credit</i> card, allowing my account balance to go negative, limited by my credit limit (currently %limit), based on my average monthly account activity. I understand when I use my credit line  (that is, when my balance goes negative) it is a loan from my %PROJECT community and I will aim to bring my balance back up above zero within 30 days.', 'limit', u\fmtAmt(-$mya->floor)), $mya->can(B_DEBT));
  $posttext = item(t(<<<EOF
<h4>** Disclaimers required by the IRS:</h4>
<p>Your choice of how to receive statements applies to all statements you receive from now on. You can change your preference at any time by returning to this Preferences page (on the Settings menu) or by notifying the regional administrator by mail: %regName, %regAddr. Any change will be confirmed in writing (electronically if you accept electronic statements, otherwise by US Mail).</p>
<p>To view your electronic statements, you need a typical computer or mobile device connected to the internet, including an email program and a web browser. To print the statements, you need a printer.</p>
<p>Your annual tax statements will be accessible online from January through December.</p>
EOF
  , ray('regName regAddr', r\regionfield('legalName'), r\regionfield('postalAddr'))));

  $advancedFields = 'achMin share smsNotices2 notices secretBal2 nosearch2';
  
//  $submit = submit(t('Update Preferences'));
  

  jsx('advanced-prefs', compact('advancedFields'));
  $form = compact(ray('title subtext roundup crumbs debtOk2 statements showAdvancet ' . $advancedFields)) + w\setupFoot() + compact('posttext');
  return cgform($form);
}

function formPrefs_validate($form, &$sta) {
  extract(just('crumbs', $sta['input']));
//  global $mya;
  if (!$crumbs = str_replace('%', '', $crumbs)) $crumbs = 0;
  if ($err = u\badAmount($crumbs, '>=0', 1, 100)) return say($err, 'crumbs');
//  if ($mya->ok and $err = w\badShare($sta['input']['share'], TRUE)) return say($err, 'share');
  u\preray(compact('crumbs'), $sta['input']); // update trimmed percentage
}

function formPrefs_submit($form, $sta) {
  include_once __DIR__ . '/../admin/admin.inc';
  global $mya;

  $fields = 'share crumbs';
  extract(just($fields . ' roundup smsNotices notices statements nosearch debtOk secretBal', $sta['input']));
//  $mya->setBit(B_NOTIFY_EMAIL, $notifyBy & 1, FALSE);
//  $flags = $mya->flags;
  foreach (ray($fields) as $k) $old["_old $k"] = $mya->$k;
  $old += ['_old flags' => u\bits($mya->flags)];
  $bits = ray('roundup weekly monthly paper secret debt nosearch', $roundup, @$notices == 1, @$notices == 2, @$statements, @$secretBal, @$debtOk, @$nosearch);
  $crumbs /= 100; // convert percentage to decimal
  $info = compact(ray($fields)) + $bits;

  if (@$statements) r\tellAdmin(t('%who wants paper statements', 'who', $mya->mainQid), ['topic' => t('paper')]);
  
  $mya->update($info);
//  $mya->stepDone('prefs', $details = $old + $info + ['flags' => u\bits($mya->flags)]);
  return w\goNextStep('prefs');
//  return $mya->ok ? say('options saved') : w\go('status', 'options saved|step completed');
}