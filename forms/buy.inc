<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once __DIR__ . '/signin.inc';
define ('AUTHCODE_TIMEOUT', 10 * 60); // number of seconds before authcode expires

/**
 * Pay with Common Good Credits, arriving from some other site or from rDo().
 * @param string $doCode: the encrypted code from rDo, if any, or 'done'
 */
function formBuy($form, &$sta, $doCode = '') {
  if ($doCode == 'done') return []; // shows result message and nothing else
  
  $required = 'company code';
  $fields = $required . ' item amount name account zip verb request api return cancel err';
//  $_POST = ray($fields, 'Cool Item #1 with a really REALLY long description', '12.34', 'NEWZZC', 'cCode', 'Pay', 'http://google.com', '01330', 'wspademan@gmail.com');
  extract($args = just($fields, $doCode ? r\doCode($doCode) : (@$sta['input'] ?: ($_POST ?: $_GET))));

  foreach (ray($required) as $k) if (!@$$k) return w\softErr(t('missing field', 'field', $k)); // handle hackers gracefully
  if (!u\isQid(@$company) or !$co = r\acct($company) or !$co->co) return w\softErr('bad account id');
  if (strtolower($code) != t('demo')) { // not looking at the demo
    if (strpos($code, ' ') or !$cocodes = substr(u\decry('P', CRYPT_FLAG . $code), 1) or !$cocodes = unserialize($cocodes) or @$cocodes['cocode'] != $co->emailCode) return w\softErr('bad co code'); // space in code is probably "bad company ID or cocode" message
    $t = time() - @$cocodes['now'];
    if ($t < 0 or $t > AUTHCODE_TIMEOUT) return w\softErr('co code timeout');
  } else $tst = t('<span class="loud">DEMO</span>');
  
  if ($amount0 = @$amount) {
    if ($err = u\badAmount($amount, '>0')) return w\softErr($err);
    $args['amount'] = $amount; // in case format got fixed
    $amount = u\fmtAmt($amount);
  } else $amount = '$' . w\rendA('amount', textFld('', '', required()));
//  if (@$api and !@$request) return w\softErr(t('You must specify the request parameter, with the api parameter.'));
  u\setDft($request, $code);

//  if ($code != @$co->data['cardCode']) return w\go('', 'bad account ID', 'zot');
  $account = trim(strpos($account, '@') ? strtolower($account) : strtoupper($account));
  if ($a = r\acct($account)) if ((@$name and !u\similar($a->fullName, $name)) or (@$zip and !u\starts($zip, $a->zip))) $a = NULL;
  $subtext = $a 
//  ? t('<h4>Hello %name!</h4><small>(Account %acct)</small>', 'name acct', $a->bestName, $a->mainQid) . @$tst
  ? t('<h4>Hello %name!</h4>', 'name acct', $a->bestName, $a->mainQid) . @$tst
  : (t('<h4>Hello UNKNOWN MEMBER</h4>') . (@$tst ?: t('<span class="loud">Make sure this is really %PROJECT!</span>')));
  list ($name, $pass) = w\signinFlds($a ? $a->mainQid : '');

//  if (($a = r\acct()) ? !u\similarEmail($a->email, $email) : (!$a = r\acct($email))) return w\go('', t('bad buy info', ray('thing', t('email'))), 'err'); // signed in
//  if (!in_array(strtolower($cust_name), array(strtolower($a->fullName), strtolower($a->legalName)))) return w\go('', t('bad buy info', ray('thing', t('name'))), 'zot');

// NO (return omits domain)  if (!\valid_url($return, TRUE)) return w\go('', 'bad url', 'zot');

  $verbs = t('Donate, Donate, Pay');
  $verb = in(ucwords(@$verb), $verbs) ? ucwords($verb) : t('Pay');
  if (@$item) $item = t('for ') . $item;
  $payment = item(t('%amount to %who<br>%what', '_amount who what', $amount, @$co->fullName ?: t('co'), @$item), "$verb:");
//  if (@$zip and substr($zip, 0, 5) != substr($a->zip, 0, 3)) $answer = textFld(t('Answer:'), $a->question, required());

  $hid = hidFld(plain(serialize($args)));
//  $option = $a ? t('OR <%a>email me an invoice</a>', '_a', 'a') : t('OR <%a>signup</a>', '_a', atag('/signup'));
  $option = $a ? '' : t('OR <%a>signup</a>', '_a', atag('/signup'));
  $submit = submit($verb, '', 'md', w\suffix($option));
  focusOn($amount0 ? 'name' : 'amount');
  
  return cgform(compact(ray('title subtext payment name pass hid submit')));
}

function formBuy_validate($form, &$sta) {
  extract(just('name hid amount', $sta['input']));
  if (isset($amount) and $err = u\badAmount($sta['input']['amount'], '>0')) return say($err, 'amount'); // fix amount
  if (!formSignin_validate($form, $sta)) return buySay('', unserialize(unplain($hid)) + ['account' => $name]);
}

function formBuy_submit($form, &$sta) {
  
  extract(just('hid name amount', $sta['input']));
  $account = strpos($name, '@') ? $name : strtoupper($name);
  extract(just('name amount item company cancel return api request', $args = unserialize(unplain($hid))));
  u\setDft($request, '');
  
  list ($a, $co) = [r\acct($account), r\acct($company)];
  
  // check for op not 'submit' here, to send an invoice
  list ($index, $subs) = $a->pay($co, $amount, @$item ?: t('donation'));
  $msg = tr($index, $subs);
  if ($ok = @$subs['success']) $msg = t('<h3>Success!</h3>') . $msg;
//  . t('<br><br><%a>Click here</a> to return to the %companyName website.', '_a companyName', atag($return), $co->fullName);
  if (@$api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
  foreach (ray('return cancel') as $k) if (@$$k) $$k = tr($$k, $args + compact(ray('msg ok')));
  
  if (!$ok) return softErr($msg);
  if ($ok and @$return) return w\goFar($return, "request=$request");
  if (!$ok and @$cancel) return w\goFar($cancel, "request=$request");

  return buySay($msg, $ok ? '' : 'err');
}

/**
 * Show an error or success message on the Pay-with-CG page.
 * @param string $msg: message to show
 * @param mixed $info: '' or 'err' (in which case show just the message) or assoc of parameters (allows retry)
 */
function buySay($msg, $info) {
  list ($code, $info) = is_array($info) ? [r\makeDoCode($info), 'err'] : ['done', $info];
  return w\go("pay-with-cg/$code", $msg, $info);
}