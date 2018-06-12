<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

function formCompany($form, &$sta, $arg = '') {
  global $base_url;
  global $mya;
  $myid = $mya->id;
//  if ($arg == 'gotPhoto') say('got photo');

  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $links = t('<%a>See your public %PROJECT web page</a> (in a new window)', 
    '_a', atag("/member/$mya->name", ['id' => 'show-webpage'] + w\away()));
  $code = item($mya->emailCode, t('Secret Code:'), t('Use this code for a <%a>Pay with Common Good</a> button on your website.', '_a', atag('https://docs.google.com/document/d/1QpOW0WU7RiUV900PMekyH7ve3-_9yjCnQn627cWovOQ')));

  $canDescs = ray(APP_CAN_BIT_DESCS);
  $permissions = '';
  if (!$mya->member and !$mya->stepDone['company'] and !$mya->coCan(APP_CANS + APP_CAN_BITS)) foreach (ray(CO_DFTS) as $i) $mya->setCoBit(APP_CANS + $i); // set defaults if doing initial account setup AND manager has no permission to charge custs yet
  
  foreach ($canDescs as $i => $desc) {
    $outbox = <<<EOF
<div class="form-item form-type-checkbox">
  <input type="checkbox" name="can[$i]" CHECKED class="form-checkbox" />
  <div class="description"><div class="box"></div></div>
</div>
EOF;
    $j = $i + APP_CAN_BITS; // convert signed out to signed in index
    $inbox = str_replace("can[$i]", "can[$j]", $outbox);
    if (!$mya->coCan(APP_CANS + $i)) $outbox = str_replace('CHECKED ', '', $outbox); // uncheck if not set
    if (!$mya->coCan(APP_CANS + $i + APP_CAN_BITS)) $inbox = str_replace('CHECKED ', '', $inbox);
    $permissions .= "<tr><th>$desc</th><td>$inbox</td><td>$outbox</td></tr>\n";
  }

  $permissions = <<<EOF
<div id="permissions"><table>
<tr id="perm-head"><td></td><th>Signed IN</th><th>Signed OUT</th></tr>
$permissions
</table></div>
EOF;

  $title = t('Company Information');
  $links = item($links, ' ');
  $name = item($mya->fullName, t('Company name:'));
  $photo = item(t('<%a>Change your company photo</a>', '_a', atag('/settings/photo?' . rand())), t('Photo:'));
  $private = boxFld('private', t('Private:'), t('Hide this company from the public (omit from public listings)'), $mya->coCan(CO_PRIVATE) ?: 0);
  $categories = selectFld(t('Categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE] + dft($myCats), coCats());
  $selling = areaFld(t('Selling:'), [t('Products'),t('Enter one or more <b>VERY SHORT</b> transaction descriptions (for example "food") for when you make a sale &mdash; one line for each. Put the most common one first. This will appear in the description of the transaction for both you and the customer (for example "$20 for <i>food</i> on 1/23/2014")')], required($mya->selling));
  $can2 = item($permissions, t('App permissions:'), t('Download the CGPay app from Google Play Store. Limit app permissions here, or go to <%aRel>Relations</a> settings to limit them for each agent.', '_aRel', atag('/settings/relations')));
//    $shareRpos = boolFld(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), $mya->coCan(CO_SHARE_RPOS) ?: 0);
//    $requireCashier = boolFld(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), $mya->coCan(CO_REQUIRE_CASHIER) ?: 0);
  $website = textFld(t('Website:'), [t('Website address'), t('Especially if you want to accept payments on your website, be sure to list it here.')], dft($mya->website));
  $descPreface = item(t('<br>What does the company do? This description will appear on your %PROJECT web page.')); //, '', ['class'=>'help-block']);
//  $description = fld('text_format', t('Description:'), [t('Arbitrarily complex description')], ['format' => 'filtered_html'] + dft(strtr($mya->description, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])));
  $description = areaFld(t('Description:'), t('Arbitrarily complex description, using <%a>markdown</a>. Use "&#92;" for line breaks.', '_a', atag('http://commonmark.org/help/')), dft($mya->description));
  $shortDesc = textFld(t('Short Desc:'), [t('Short description'), t('Short description of your company\'s products and services. Maximum 35 characters.')], attrib(['maxlength'=>35]) + required($mya->shortDesc));
  $employees = textFld(t('Employees:'), [t('Employee count'), t('How many employees do you have?')], required($mya->employees));
  $gross = textFld(t('Annual Gross:'), [t('Approximate annual gross income')], required($mya->gross));
  $tips = boolFld(t('Tips:'), t('After each %PROJECT card sale, ask the customer to choose a tip percentage (or no tip)'), $mya->coCan(CO_TIP) ?: 0);
  if ($mya->cAdmin) {
    if (FALSE) $fast = boolFld(t('No ID Chk:'), t('Never ask customer for photo ID (set by admin)'), $mya->coCan(CO_FAST) ?: 0); // no exemptions
    $atm = boolFld(t('ATM service:'), t('Allow company to trade %RCREDITS for cash even if their balance is less than their rewards (in-person only).'), $mya->coCan(CO_ATM) ?: 0);
  }
//  $submit = submit();
  return cgform(compact(ray('title links photo name private categories selling shortDesc employees gross website code descPreface description can2 tips fast atm')) + w\setupFoot(''));
}

function formCompany_validate($form, &$sta) {
  extract(just($fields = 'selling employees gross', $sta['input']));
  $selling = explode("\n", $selling);
  foreach ($selling as $one) {
    $one = trim($one); // don't allow empty or reserved values
    if (!u\nonish($one) and !in($one, ['', S_REFUND, S_USD_OUT]) and !r\usdin($one)) $new[] = strtolower($one);
  }
  if (!@$new) return say('missing field', ray('field', 'selling'), 'selling');
  $selling = join("\n", $new);
  if ($err = u\badAmount($employees, '>=0')) return sayFieldErr($err, 'employees');
  if ($err = u\badAmount($gross, '', 2, '')) return sayFieldErr($err, 'gross');
  u\preray(compact(ray($fields)), $sta['input']);
}

function formCompany_submit($form, &$sta) {
  global $mya;

  extract($info = just('private description shortDesc website selling categories can requireCashier employees gross tips fast atm', $sta['input']));
  
  $website = preg_replace('~https?://~i', '', @$website);

  $mya->setCoCans(@$can);
  //$mya->setBits(APP_CAN_BITS, $can, 'co');
//  $mya->setBitx(CO_REQUIRE_CASHIER, @$requireCashier, 'coFlags');
  $mya->setCoBit(CO_PRIVATE, @$private);
  $mya->setCoBit(CO_TIP, $tips);
  if ($mya->cAdmin) $mya->setCoBit(CO_FAST, @$fast);
  if ($mya->cAdmin) $mya->setCoBit(CO_ATM, $atm);
  $mya->update(compact(ray('description shortDesc website selling employees gross')));
  
  if (@$categories) $mya->setCategories($categories);

//  $mya->stepDone('company', $info);
//  return $mya->ok ? say('options saved') : w\go('status', 'options saved|step completed');
  return w\goNextStep('company', NULL, $info);
}
