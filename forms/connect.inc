<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Connect a bank account.
 */
function formConnect($form, &$sta, $args = '') {
  global $rUrl;
  global $mya;
//  $mya->stepDone('connect', compact('connect')); // just visiting this page is enough

  $usa = ($mya->country == US_COUNTRY_ID);
  $usa = TRUE; // always true for now

  $title = t('Banking Settings');

  $refills = boolFld(t('Refills:'), t('Refill your %PROJECT Account automatically when it goes below a chosen "Target Balance"? Say no if you tend to bounce checks or live close to the wire.'), $mya->refill);
  if ($mya->co) $cashout = radiosFld(t('Cashout:'), t('If you choose weekly or monthly automatic transfers to your bank account, the system will leave a balance in your %PROJECT account equal to your average weekly payments received, over the past six months.'), required(), ray('0 1 2', t('no automatic cashout'), t('weekly (automatic)'), t('monthly (automatic)')));
  $autopay = autopays($orig);
  $autoboxes = hidFld(serialize($orig));
  $minDesc = t('How much to keep in your %PROJECT Account, for purchases. When your balance goes below this amount, the system automatically refills it for you from your bank account. Refilling may take 3-5 business days.');
  $minDescNegative = $mya->balance < 0 ? t(' <b>If you choose a negative amount</b>, it will be increased by %bump a week until it is greater than zero.', 'bump', u\fmtAmt(MIN_WEEKLY_BUMP)) : '';
  $minWarning = t('note|ach warning');
  $minDefault = u\fmtAmt($mya->minimum ?: 0, 's$');
  $target = textFld(t('Target Bal:'), [t('Your preferred "normal" balance'), "<p>$minDesc$minDescNegative</p><p>$minWarning</p>"], dft($minDefault));
  $achMin = textFld(t('Min Transfer:'), [t('Minimum bank transfer amount'), t('This much (or more, if necessary) will be transferred from your bank account in exchange for %RCREDITS when your balance goes below the Target Balance you chose, above. The suggested amount works well for most people.')], dft(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's$')));

//  $savingsAdd = textFld(t('Extra Savings:'), [t('Extra savings amount'), t('Your incentive rewards go automatically into a Savings Reserve, for later or as needed. Your Savings Reserve works like a traditional reserve savings account that kicks in whenever you would otherwise overdraw your primary account -- except there are no fees. This also helps the community by keeping more funds in the system. You get a monthly inflation adjustment on any funds you hold in the %PROJECT system (at @inflate% APR). If you want to hold even more in your Savings Reserve (a great idea!) enter the additional amount here.', ['@inflate' => R_INFLATION_RATE * 100])], required(u\fmtAmt($mya->savingsAdd)));
  $saveWeekly = textFld(t('Save Weekly:'), [t('Additional amount to add weekly to Target Balance'), t('Putting money away as savings can be a challenge. This may help: choose a small additional amount to be added automatically to your %what every week.', 'what', $mya->balance < 0 ? t('Target Balance') : t('Target Balance'))], dft(u\fmtAmt($mya->saveWeekly)));
  
  $targetFields = fieldSet('targetFields2', compact(ray('target achMin saveWeekly')), BARE);
  //$bankAccountType = radiosFld(t('Type:'), '', required($mya->can(B_SAVINGS)), $typeOptions);
    
  if ($hasBank = $mya->hasBank) {
//    if ($usa) list ($route, $bankAccount) = u\parseBankAccount(@$bankAccount);
    $remove = submi(t('Remove connection'), '', 'xs', ['parents' => ['remove']]);
    $bankAccount = item("xxxxxx$mya->last4bank &nbsp; &nbsp;", t('Account:'), '', w\suffix(\render($remove)));
//    $submit = submit();
    $fields = 'title bankAccount cashout autopay autoboxes refills targetFields';
  } else { // no connected account yet
//    $doConnect = (int) !@$mya->stepsDone['connect'];
    $securityLink = r\promoLink('security.html');
    $reassurance = $mya->minimum ? '' : t('<p><b>NOTE:</b> Funds will be taken from your bank account ONLY when you explicitly request a transfer or when you choose automatic refills.</p>');
    $subtext = t('<p>Choose how you want to get money in and out of your %PROJECT Account. Most people find it convenient to connect a checking account. Connecting lets you trade US Dollars for %RCREDITS easily. Otherwise you can trade cash for %RCREDITS at a participating business.</p><p>Complete this form to connect your checking account and authorize funds to be taken from your checking account in exchange for any %RCREDITS you request. Funds may be transferred either by Remotely-Created Check (RCC) or by Automated Clearing House (ACH) direct debit. See our <%a>Privacy and Security Policy</a> for details on how your information is protected.</p>', '_a', atag($securityLink)) . $reassurance . t('<p>Do you want to connect your checking account at this time?</p>');
    $connect = boolFld(t('Connect:'), '', 0);
    if ($usa) $routingNumber = textFld(t('Routing:'), t('Type carefully your bank\'s routing number.'));
    //$bankAccount = textFld(t('Account:'), t('Type carefully your checking account number.'));
    $bankAccount = passFld(t('Account:'), t('Type <b class="loud">carefully</b> your <b>checking</b> account number. The bank charges us $20 if you get this wrong. <span>Savings accounts will not work, at this time.</span>'));
    $bankAccount2 = passFld(t('Again:'), t('Type your account number again.<br>(we need to be REALLY SURE we don\'t connect to the wrong account)'));
  
    $connectFields = fieldSet('connectFields2', compact(ray('checkHint routingNumber bankAccount bankAccount2 cashout autopay autoboxes refills targetFields')), BARE);
//    $submit = submit(t('Connect'));
    $fields = 'title subtext connect connectFields';
  }

  jsx('bank-prefs', $vs = ray('connectLabel saveLabel mindft', t('Connect'), t('Save'), R_MIN_DEFAULT));
  return cgform(compact(ray($fields)) + w\setupFoot($hasBank ? $vs['saveLabel'] : $vs['connectLabel']));
}

function formConnect_validate($form, &$sta) {
  global $mya;

  extract(just('connect routingNumber bankAccount bankAccount2 bankAccountType refills target achMin saveWeekly', $sta['input']));

  foreach (ray('target:,achMin:,saveWeekly:>=0') as $k => $v) { // and savingsAdd someday 
    if (u\test() and !isset($sta['input'][$k])) $sta['input'][$k] = 0; // empty makes tests fail
    if ($err = amtErr($k, $sta, $v)) return sayFieldErr($err, $k);
    $$k = $sta['input'][$k];
  } // must be before return because params are always present and must have "$" removed by amtErr()

  if (op($sta) == 'remove') return;

  if (!$mya->hasBank) {
    if (is_null(@$connect)) return say('yes or no', 'connect');

    if (!@$connect) return; // nothing more to do

    foreach (ray($accts = 'routingNumber bankAccount bankAccount2') as $one) $$one = preg_replace('/[\.\- ]/', '', @$$one);
    u\preray(compact(ray($accts)), $sta['input']);
    if (!preg_match('/^(|[0-9]{9})$/', $routingNumber)) return say('bad routing number', 'routingNumber');
    if (!db\exists('r_banks', 'route=:routingNumber', compact('routingNumber'))) return say('bad routing number', 'routingNumber');
    if (!preg_match('/^[0-9]{3,17}$/', $bankAccount)) say('bad account number', 'bankAccount'); // {3,20} ? Dwolla says 3-17
    if ($bankAccount != $bankAccount2) say('mismatch', ['thing' => t('account number')], 'bankAccount2');
  }

  if (@$refills) {
    $floor = min($mya->balance, max(0, $mya->floor)); // can't go deeper in debt
    if ($target < $floor) return say('min sub floor', ['floor' => u\fmtAmt($floor)], 'target');
    if ($achMin < R_ACHMIN) return say('bad achmin', 'achMin');
  }
  
//  if ($savingsAdd < 0) return say('savings too low', 'savingsAdd');
//  $savingsMax = max($mya->savings, $mya->balance);
//  if ($savingsAdd > $savingsMax) return say('savings too high', ['max' => u\fmtAmt($savingsMax)], 'savingsAdd');
// NO! if ($minimum < 0 and $saveWeekly < MIN_WEEKLY_BUMP) return say('saveWeekly too low', ['min' => u\fmtAmt(MIN_WEEKLY_BUMP)], 'saveWeekly');
//  if ($saveWeekly < 0 and $mya->savingsAdd == 0) return say('negative saveWeekly', 'saveWeekly');
}

function formConnect_submit($form, &$sta) {
  global $mya;

  extract(just('connect routingNumber bankAccount cashout autopay autoboxes refills target achMin saveWeekly', $sta['input']));
  if (op($sta) == 'remove') $connect = FALSE;
  $minimum = $mya->minimum;

  if (@$connect or !isset($connect)) { // wants bank or has bank, not removing
    if (@$connect) { // wants bank
      if ($mya->country == US_COUNTRY_ID) $bankAccount = "USkk$routingNumber$bankAccount";
      $last4bank = substr($bankAccount, -4, 4);
    } else list ($connect, $bankAccount) = [TRUE, 'no change']; // has bank
    if ($refills) $minimum = $target; // change minimum only explicitly
  } else { // removing or choosing not to connect
    $bankAccount = $last4bank = '';
    $achMin = $mya->achMin ?: R_ACHMIN_DEFAULT;
  }

  list ($hasBank, $refill) = [$connect, $connect and $refills];
  list ($cashoutW, $cashoutM) = [$connect and @$cashout == 1, $connect and @$cashout == 2];
  $fields = ray('hasBank last4bank refill cashoutW cashoutM minimum achMin saveWeekly');
  if ($bankAccount != 'no change') $fields[] = 'bankAccount'; // avoid overwriting v-encrypted value
  $mya->update(compact($fields));
  
  if (@$autoboxes and $orig = unserialize($autoboxes)) foreach ($orig as $uid => $wasSet) {
    $setNow = isset($autopay[$uid]);
    if ($setNow xor $wasSet) r\acct($uid, $mya->id)->setRel('autopay', $setNow);
  }
  
//    if ($saveWeekly < 0) say(t('@amount will be moved every week from your Savings Reserve to your Primary Account, until your Savings Reserve reaches its minimum.', ['@amount' => u\fmtAmt($saveWeekly)]));
  return w\goNextStep('connect');
//  say('info saved' . ($mya->ok ? '' : '|return to membership'));
}

/**
 * Return a set of automatic payment checkboxes, if appropriate.
 * @param array $orig: (RETURNED) list of the autopayable account IDs (if any) and original setting
 * @return the checkbox fields or NULL if none.
 */
function autopays(&$orig = '') {
  global $mya;

  $rows = db\q('SELECT uid,fullName,r.:IS_AUTOPAY AS auto FROM r_relations r LEFT JOIN users u ON u.uid=r.main WHERE other=:myid AND r.:IS_CUSTOMER', ray('myid', $mya->id))->fetchAll();
  if (!$rows) return NULL;
  
  foreach ($rows as $i => $one) {
    extract((array) $one); // uid fullName auto
    $nms[$uid] = t('Autopay ') . $fullName;
    if ($orig[$uid] = $auto) $dfts[] = $uid;
  }
  if (!@$dfts) return NULL; // no autopays, so don't show choices
  $help = t('When you choose automatic invoice payment for a particular company, funds will be drawn from your bank account as needed. Leave the box(es) unchecked if you tend to bounce checks or live close to the wire.');
  return boxesFld(t('Invoices:'), $help, @$dfts ?: [], $nms);
}