<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Pay with Common Good Credits, arriving from some other site or from rDo().
 * @param string $doCode: the encrypted code from rDo, if any, or 'done'
 */
function formBuy($form, &$sta, $doCode = '') {
  if ($doCode == 'done') return []; // shows result message and nothing else
  
  $required = 'company code';
  $fields = $required . ' item amount name account zip verb return cancel err';
//  $_POST = ray($fields, 'Cool Item #1 with a really REALLY long description', '12.34', 'NEWZZC', 'cCode', 'Pay', 'http://google.com', '01330', 'wspademan@gmail.com');
  extract($args = just($fields, $doCode ? r\doCode($doCode) : (@$sta['input'] ?: ($_POST ?: $_GET))));
  foreach (ray($required) as $k) if (!@$$k) return r\go('empty', t('missing field', 'field', $k), 'err'); // handle hackers gracefully
  if (!u\isQid(@$company) or !$co = r\acct($company) or !$co->co) return r\go('empty', 'bad account id', 'err');
  if ($code != $co->emailCode) { // not testing
    if (!$cocode = substr(r\decry('P', $code), 1) or $cocode != $co->emailCode) return r\go('empty', 'bad co code', 'err');
  } else $tst = t('<span class="loud">TESTING</span>');
  
  if ($amount0 = @$amount) {
    if ($err = u\badAmount($amount, '>0')) return r\go('empty', $err, 'err');
    $args['amount'] = $amount; // in case format got fixed
    $amount = u\fmtAmt($amount);
  } else $amount = '$' . w\rendA('amount', textFld('', '', required()));
//  if (@$api and !@$request) return r\go('empty', t('You must specify the request parameter, with the api parameter.'), 'err');
  u\setDft($request, $code);

//  if ($code != @$co->data['cardCode']) return r\go('', 'bad account ID', 'zot');
  $account = trim(strpos($account, '@') ? strtolower($account) : strtoupper($account));
  if ($a = r\acct($account) and (!u\similar($a->fullName, $name) or !u\starts($zip, $a->zip))) $a = NULL;
  $subtext = $a 
  ? t('<h4>Hello %name!</h4><small>(Account %acct)</small>', 'name acct', $a->bestName, $a->mainQid) . @$tst
  : (t('<h4>Hello UNKNOWN member</h4>') . (@$tst ?: t('<span class="loud">Make sure this is really %PROJECT!</span>')));
  list ($name, $pass) = w\signinFlds($a ? $a->mainQid : '');

//  if (($a = r\acct()) ? !u\similarEmail($a->email, $email) : (!$a = r\acct($email))) return r\go('', t('bad buy info', ray('thing', t('email'))), 'err'); // signed in
//  if (!in_array(strtolower($cust_name), array(strtolower($a->fullName), strtolower($a->legalName)))) return r\go('', t('bad buy info', ray('thing', t('name'))), 'zot');

// NO (return omits domain)  if (!\valid_url($return, TRUE)) return r\go('', 'bad url', 'zot');

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
  $account = $name;
  extract(just('name amount item company cancel return api request', $args = unserialize(unplain($hid))));
  
  list ($a, $co) = [r\acct($account), r\acct($company)];
  
  // check for op not 'submit' here, to send an invoice
  list ($index, $subs) = $a->pay($co, $amount, @$item ?: t('donation'));
  $msg = tr($index, $subs);
  if ($ok = @$subs['success']) $msg = t('<h3>Success!</h3>') . $msg;
//  . t('<br><br><%a>Click here</a> to return to the %companyName website.', '_a companyName', atag($return), $co->fullName);
  if (@$api) u\post($api, ray('request ok msg', $request, (int) $ok, $msg));
  foreach (ray('return cancel') as $k) if (@$$k) $$k = tr($$k, $args + compact(ray('msg ok')));
  if ($ok and @$return) return r\go($return);
  if (!$ok and @$cancel) return r\go($cancel);
  return buySay($msg, $ok ? '' : 'err');
}