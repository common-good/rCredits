<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Request an employee ID card (for buying on behalf of the company).
 */
function formRequestRCard($form, &$sta, $other = '') {
  global $mya;
  if (!$acct = r\acct($mya->id, @$other)) return hack('request rCard');
//  if (!$mya->co or !$mya->can(B_REFUND)) return say('no account permission');
  if (!$mya->can(B_SCAN)) return say('no account permission', 'err');
  $fullName = $acct->fullName;
  
  if ($mya->can(B_BUY)) {
    $title = t('Request Employee %PROJECT Card');
    $subtext = t('A Company %PROJECT Card will be US Mailed to %fullName, to be used for making in-person purchases on behalf of the company.', compact('fullName'));
  } else { // requesting cashier card -- not for purchases
    $title = t('Request Cashier %PROJECT Card');
    $subtext = t('A %PROJECT Cashier %PROJECT Card will be US Mailed to %fullName, to be used for charging customers on behalf of the company.', compact('fullName'));
  }
  
  $subtext .= t(' Your company account will be charged $@R_CARD_COST.');
  $other = hidFld($other);
  $request = submit(t('Request Card'));
  $cancel = submit(t('Cancel'));

  return cgform(compact(ray('title subtext other request cancel')));
}

function formRequestRCard_submit($form, &$sta) {
  global $base_url;
  extract(just('other', $sta['input']));
  if (op($sta) == 'request') {
    global $mya;
    $main = $mya->id;
    $company = $mya->fullName;
    $otherName = r\acct($other)->fullName;
    $address = $mya->postalAddr; // was r\location($mya, TRUE);
    $_a = atag("/print-rcard/$main/$other");
    r\tellAdmin($mya->can(B_BUY) ? t('company rcard') : t('cashier card'), compact(ray('main other _a manager otherName address')));
    say('card ordered', compact('otherName', 'company'));
  } else say('no card ordered');
  return w\go('settings/relations');
}