<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

include_once R_ROOT . '/forms/signin.inc';

/**
 * Modify the user registration form.
 * @param string $args: if not empty, 
 *   a urlencoded assoc (from an email invitation or the "Create a Company Account" button) OR
 *   an assoc gleaned from POSTed parameters (for combined signup with Coop Power's heating fuel program, for example)
 *
 *   code: the invitation code (or a friend's name with zip in parens)
 *   personal: opening a personal account (default TRUE)
 *   fullName: the likely name of the account to create
 *   phone: the account's likely phone number
 *   owner: is the inviter an owner (these last four params are processed by suCreatorRelation())
 *   employee: is the inviter an employee
 *   flow: which way credit can flow (be drawn)
 *   helper: uid of account creating this one -- non null (same as $inviter) means created with formSignupCo
 */
function formSignup($form, &$sta, $args = '') {
  if (!$args) $args = @$_POST ?: basename($_SERVER['REQUEST_URI']); // because menu_execute_active_handler fails

  $contactDefaults = 'phone address city zip address2 city2 zip2 state2 postalAddr partner partnerCode customer source';
  extract(just('joint reid inviter code personal fullName state email helper ' . $contactDefaults, $args));
  extract(just('hidCountry hidState', $sta['input']));

  if (@$reid) { // continuing signup of a partner's customer
    if (u\test()) $reid = f('t.uid', $reid);
    if ($args2 = w\continuePartnerSignup($reid, @$customer)) extract($args = $args2);
  } elseif (@$customer and @$partner and @$partnerCode) {
    $myid = w\partnerSignup($args); // info passed from partner signup
  } elseif (@$joint and $a = r\acct() and $a->canManage) {
    if (!$a->co) $myid = w\jointSignup(); else return w\go('', t('A company cannot have a joint account.'), 'err');
  }
  if (u\test() and is_array(@$myid)) return; // array means returned r\go
    
  $myid = hidFld($myid0 = @$myid);
  
  if (@$personal and r\signedIn()) return formEmpty('must be signed out');
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost
  u\setDft($personal, TRUE);

  $title = $personal 
  ? (@$joint ? t('Create a Partner Account Joined to Yours') : t('Open a Personal %PROJECT Account'))
  : t('Open a Company Account');
  
  if (!@$helper) { // no need to clarify if user clicked the "Open a Company Account" button
    $helper = @$partner ?: 0;
    $clarify = $personal ? t('To open a company account, use the "Open a Company Account" button after your personal account is approved.') : t('To open another personal account, send an Invitation (click the Invite tab).');
//    $onclick = $onmouseover = 
    $forother = $personal ? t('open a company account instead') : t('open a personal account instead');
    $forother = item("[<a class=\"nospin\">$forother</a>]");
  }
    
  if (!@$code) $code = 'self ()'; // can't actually tell whether this is allowable, because we don't yet know which community

  if ($i = strpos(@$code, '(')) { // self-invitation with friend's name and zipcode
    $inviterName = substr($code, 0, $i);
    if (!@$partner) $notes = hidFld("knows: $code");
    $code = r\acct(1)->iCardCode(IBY_SELF); // inviter is admin for now; invitation() will set $iCode=IBY_SELF
  }
    
  $inviteOk = w\invitation(@$code, '', $inviter, $iCode, $err);
  if (@$inviter and $iCode != IBY_SELF) {
    $ia = r\acct($inviter);
    $inviterEmail = plain($ia->email);
    $inviterName = plain($ia->fullName);
  }

  $count = $personal ? PERSONAL_STEP_COUNT : COMPANY_STEP_COUNT;
  $proxyInfo = w\lnk('help/proxies', t('more about proxies'), w\away());
  $proxyInfo = "<small>[$proxyInfo]</small>";
  $proxyInfo = w\btn('/help/proxies', t('more about proxies'), 'default', 'xs', w\away());
  
  $subtext = @$joint 
  ? t('From here on, "you" means the person for whom you are creating a new account, which will be joined to an existing account (creating a joint account).') 
  : t(<<<EOF
<p>Opening a %PROJECT Account takes 10-15 minutes and is very much like opening an online bank account. For example, you will be asked for your social security number or tax ID.</p>

<p>After this page we will guide you through <%b>several short steps</b>. Here are some highlights:</p>
<ul>
  <li>You will be asked to <b>sign</b> the short but ESSENTIAL %PROJECT Agreement.</li>
  <li>You will need to upload a <b>photo</b> for your %PROJECT card. Your photo will be encrypted and will not be visible to other members (unless you show it to them).</li>
  <li>You will be invited (but not required) to make a <b>donation</b> <i>of any size</i> to support the project. You decide how much (%PROJECT is a nonprofit).</li>
  <li>Your name and city will be visible to other members who may wish to select you as their <b>proxy</b> in the community's democratic process. %proxyInfo</li>
  <li>You will need your <b>bank</b> account information if you choose to connect it. Connecting a bank account makes it easier to transfer funds into and out of your %PROJECT account.</li>
</ul>
<p>If you need help, please <%a>ask</a> (our contact information is at the bottom of every page).</p>
EOF
  , '_b _a count _proxyInfo', 'b class="loud"', w\emailtag(CGF_EMAIL, t('signup help')), $count, $proxyInfo);

//Opening a %PROJECT Account is very much like opening an online bank account. After this long signup page there are <%b>some short steps</b> we will guide you through. If you need help, please don't hesitate to ask (<%a>click here to send us an email</a>).
  
  if ($personal) {
    $acctType = hidFld(CO_PERSONAL);
//    $pin = passFld(t('PIN:'), [t('Choose a PIN (any 4 digits)'), t('Your security code for online purchases.')]); // first time logging in, get PIN
    $calling = textFld(t('Life Calling:'), [t('Your calling in life (optional)'), t('One of the main purposes of %PROJECT is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do, to earn %RCREDITS (maybe something you\'re not doing yet)? What might you have to offer?')]);
    $companyOptions = array(
      'owner' => t('owner'),
      'employee' => t('employee'),
      'contractor' => t('other worker (contractor, supplier, etc.)'),
    );
    $copts = boxOpt('companyOptions', $companyOptions, $sta);

    $companyInfo = array(
      'company_text' => item(t('Are you an owner and/or employee of a local company, or of any %PROJECT member company? Otherwise leave this part blank.')),
//      'company' => textFld(t('Company: '), [t('Your company')], autocomplete('company')),
      'company' => textFld(t('Company: '), [t('Your company')]),
      'companyPhon' => textFld(t('Company Ph<span>&nbsp;</span>one:'), [t('Company phone number')]), // Google Chrome bug require obfuscation of "phone"
      'companyOptions' => boxesFld('', '', @$copts, $companyOptions), 
    );
    $companyInfo = fieldSet('companyInfo', $companyInfo, BARE);
  } else { // company
    $coTypes = ray(CO_TYPES);
    unset($coTypes[CO_PERSONAL]);
    $acctType = radiosFld(t('Account type:'), '', required(), $coTypes);
    // if custodian, subtype field should appear with all these same choices: $custodianFor = ...
  }

//  $buysAts = db\q('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
//  $ats = boxOpt('ats', $buysAts, $sta);
//  $buysAt = boxesFld(t('<br>Over 60 Greenfield businesses have already signed up. Which of these <b>Not-Yet-Member Businesses</b> do you frequent?'), '', @$ats, $buysAts);

  $submit = t('Next');
  $args = hidFld(@$args);
  $confirmed = hidFld(@$iCode == IBY_FORM); // iff inviter used form (or opening a company account), invitation is already confirmed
  $iCode = hidFld(@$iCode + 0);
  $helper = hidFld(@$helper ? r\acct($helper)->id : @$inviter);
  
  $contactDefaults = compact(ray($contactDefaults));
  $contactFlds = w\contactFields($contactDefaults + ray('country state state2', @$country ?: @$hidCountry, @$state ?: @$hidState, @$state2 ?: @$hidState2), TRUE, @$myid0);
  $nameAndEmail = w\nameAndEmail($personal ? t('Full name') : t('Company'), @$fullName, @$email, isset($contactFlds['partner']), @$myid0);

  $form = compact(ray('title forother subtext acctType'))
    + $nameAndEmail + $contactFlds + w\ssnFields($personal)
    + compact(ray('calling companyInfo buysAt submit myid args confirmed helper iCode notes'));
//  keep_values($form);
//  $sponsorConfirmation = w\whoFldSubmit('sponsor', t('Your sponsoring organization is %name, yes?'));
  jsx('signup', compact('clarify'));
  return cgform($form);
}

function formSignup_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = []; // for testing (keep Drupal from griping)
  extract(just('myid confirmed acctType email phone country tenure pin company companyPhon companyOptions fullName legalName zip address city state postalAddr', $sta['input']));
//  if (!$inviteOk) return w\go('signup'); // in case user resubmits
  //and @$code != bin2hex(R_WORD)

  \variable_set('signupData', ray('time phone state companyOptions', time(), @$phone, @$state, @$companyOptions));
  if (!emailOkay($email, $acctType != CO_PERSONAL, @$myid ? r\acct($myid) : '')) return;
  if (!phoneOk($phone, $country)) return;
  if ($err = u\badZip($zip)) return say($err, 'zip');
  if (!@$state) return say('bad state', 'state');
  if (!formBasic_validate($form, $sta, $acctType != CO_PERSONAL)) return;
  if ($err = u\badAmount($tenure, '>=0')) return say(t('How-long: ') . t($err), 'tenure');
//  if (@$pin === '') return say('pin required', 'pin');
//  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  
  if (@$companyOptions) extract(just('owner employee contractor', $companyOptions));
  $gotrelation = (@$employee or @$owner or @$contractor);
  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (ray('company companyPhon') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $signupCo = compact(ray('company companyPhone employee owner contractor'));
  }
  
  if (!@$legalName) $legalName = $fullName;
  foreach (ray('fullName legalName address city postalAddr') as $field) {
    $$field = trim(str_replace('  ', ' ', u\normalizeCase(@$$field))); // @ for testing
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }
  if (u\poAddress($address)) return say('po in location', 'address');

  $flags = $acctType == CO_PERSONAL ? 0 : u\bit(B_CO);
  if (@$confirmed) $flags |= u\bit(B_CONFIRMED); // companies are always pre-confirmed, because the company account is opened by a member
//  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  u\preray(compact(ray('fullName legalName phone address city zip postalAddr email flags signupCo status')), $sta['input']);
}

function formSignup_submit($form, &$sta) {
  global $mya;
  $dbFields = 'fullName legalName email flags phone federalId dob pin country zip address city state postalAddr helper iCode notes wants calling signupCo tenure source';
  extract(just('myid confirmed acctType args ats owns partner customer source2 ' . $dbFields, $sta['input']));

  if (u\test() and !@$args) $args = basename($_SERVER['REQUEST_URI']);

  u\EXPECT((bool) $args, 'no args');
  extract(just('code', $args));
  if (isDEV) $helper = $helper + 0; // don't let lack of helper stop us

  $wants = wants(@$ats);
  if (@$notes) list ($source, $notes) = [trim("$notes $source"), ''];
  if (@$source2) $source = trim(strtoupper($source2) . "-$source");

  $info = compact(ray($dbFields));
  if ($acctType != CO_PERSONAL) $info += ray('coType', $acctType);
  
//  $DBTX = \db_transaction();

  if (@$myid) {
    $a = r\acct($myid);
    if ($a->jid) $info['name'] = r\uniqueName($fullName, $info); // make sure name reflects partner name
    $a->update($info);
    $name = $a->name;
  } else {
    if (!$a = new r\Acct($info)) {unset($DBTX); return say('cannot create account', 'zot');}
    list ($myid, $name) = [$a->id, $a->name];
  }
  
  if (!@$mya or !$mya->cAdmin) w\setAcct(NULL); // sign agent out if creating joint Account (so it looks joint)
  $mya = w\setAcct($myid); // this works for both an individual account (proSe) and a company account (with agent)
  $qid = $mya->mainQid;
  
  if (@$iCode < 0) w\invitation($code, $myid); // mark the invitation used
  $email = u\cry('P', $email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!@$owns) $mya->setRisk('rents');
  
  if (@$signupCo) {
    suCompanyRelation($myid, $signupCo);
    $mya->update(compact('signupCo'));
  }
  if ($h = $mya->helperA and @$args and !$h->cAdmin) suCreatorRelation($helper, $args, $myid);
//  unset($DBTX); // commit    
  
  // report to staff and user
  tellStaffNewMember($info + compact(ray('acctType qid name')));

  $pw1 = $mya->oneTimePass($name, TRUE);
  
  if ($mya->co) {
    say('company is ready', compact('qid'));
    //if (@$helper) $qid = r\qid($helper); // "if" for dev faked code HUH?
    if (!@$h) hack('company no helper');
    if ($mya->email == $h->email) {
      $mya->stepDone('verify');
    } else r\notify($myid, 'verifyCo', ['code' => $pw1], TRUE); 
    return w\goNextStep('signup');  
  } else { // non-personal company (no independent login)

    r\notify($myid, 'verify', ray('name qid code', $name, $qid, $pw1), TRUE); 

    if (!@$confirmed) {
      u\EXPECT((bool) $h, 'unconfirmed with no helper');
      if ($helper != 1) { // sysadmin helper means the potential inviter is named in the notes field
        $_a1 = $h->makeDo('confirmInvite', $myid); // link to confirm invitedness
        r\message($helper, 'confirm invite', compact('_a1', 'fullName')); // ask inviter to confirm
      }
    }
    
    $sta2['uid'] = $myid; // tell Drupal we're signed in
    $nextStepUrl = $mya->nextStepUrl('signup', $msg);
    say($msg);
    formSignin_submit($form, $sta2, $nextStepUrl); // lead user to next step 
  } 
}

/**
 * Handle simplified signup for second person in a joint account.
 * @param acct $a: the account to join, if it exists (otherwise create it) -- UNUSED
 */
function jointSignup($a = NULL) {
  $mya = r\acct();
  if ($mya->jid) return $mya->jid; // in case form gets run twice
  if (!$a) {
    $fields = 'fullName phone address city state zip country postalAddr source minimum achMin';
    $args = (array) $mya->account($fields);
    $args['fullName'] .= t(' PARTNER');
    $a = new r\Acct($args + ['email' => t('name@') . EMAILX]);
    u\EXPECT($a, 'cannot create account');
    $a->update('tenure', round($mya->tenure + ($a->created - $mya->created) / (30 * DAY_SECS)));
    if (@$mya->risk('rents')) $a->setRisk('rents');
    $a->newRelation(ray('other permission', $mya->id, r\perm(B_JOINT)));
  }
  $mya->newRelation(ray('other permission', $a->id, r\perm(B_JOINT)));
  $a->join($mya);
  return $a->id;
}

/**
 * Handle info passed from partner signup, for use in CG signup
 * @param assoc $args: what the partner passed (includes non-empty customer, partner and partnerCode)
 * @return string $myid: account record ID for the newly-created account
 *   but if person already has an account, we redirect to continue signup
 *   we also redirect if all that needs to be done is to display a message (including if the person is already a member)
 */
function partnerSignup($args) {
  foreach ($args as $k => $v) u\badName($args[$k]); // fix double spaces and leading or trailing
  extract(just('partner partnerCode customer qid phone fullName email', $args));
  if (!$partnerA = r\acct($partner) or $partnerA->emailCode != $partnerCode) return NULL;
  $partnerName = $partnerA->fullName;
  
  $reid = r\relation('reid', 'main=:main AND code=:customer', ray('main customer', $partnerA->id, $customer));
  
  if (!@$qid) { // person is not a member yet
    if ($reid and $uid = r\relation('other', $reid)) return $uid;
    return newCustomer($reid, $args); // mark new or old member as a customer of the partner
  }
    
  // partner sent an already-member
  $valid = ($a = r\acct($qid) and ($a->phone == u\fmtPhone(@$phone, '+n') or (u\similar($a->fullName, @$fullName, 5, .5))));
  r\tellAdmin(t('new customer of %partner: %fullName', compact('partner', 'fullName')) . (@$valid ? '' : t('(invalid)')), $args);

  if (!$valid) return w\softErr(t('That does not appear to be your correct %PROJECT member ID. But no worries. We will try to figure out who you are and match up your %PROJECT account with your account at %partnerName. If you have not yet finished opening your %PROJECT account, <%a>sign in</a> (if you have not yet begun, go to %CG_DOMAIN and click "Sign up").', 'partnerName _a', $partnerName, atag('/signin'))); // member mistyped their qid (a royal pain in the butt)

  r\acct($partnerA->id, $a->id)->setRel('customer code', TRUE, $customer);
  tellPartner($partnerA, $a, 'complete', $customer);
  return w\go($a->member ? 'empty' : "signin?name=$qid"); // finish account setup or report success
}

/**
 * Tell the partner about this signup.
 */
function tellPartner($pA, $a, $op, $customer = NULL) {
  if (!$pA->website) return w\softErr(t('Partner has no website. Fix it on the Company Info settings page.')); // partner should always have a website

  if ($a->member) w\say('new customer done', ray('partnerName', $pA->fullName));
  $fields = ray('op partnerCode qid name email customer', $op, $pA->emailCode, $a->mainQid, $a->fullName, $a->email, $customer);
  u\post($pA->api(), $fields); // tell partner we're starting or done
}

/**
 * Create a new account and a customer relation with a company, if it doesn't already exist.
 */
function newCustomer($reid, $args) {
  foreach (ray('address2 city2 state2 zip2') as $k) $$k = @$args[$k] ?: '?';
  $args += ['country' => US_COUNTRY_ID, 'postalAddr' => "$address2, $city2, $state2 $zip2"];
  $a = new r\Acct(just('fullName phone email address city state zip country postalAddr source', $args));
  u\EXPECT($a, 'cannot create account');
  if (!$reid) {
    extract(just('partner customer email autopay', $args));
    $partnerA = r\acct($partner);
    $reid = $partnerA->newRelation(ray('other customer code autopay data', $a->id, TRUE, $customer, (bool) @$autopay, serialize($args))); // (bool) is required to avoid NULL
    w\say('new customer welcome', ['partnerName' => $partnerA->fullName]);
    try { // give them a way to continue, even if they haven't completed the first page
      $noFrame = TRUE;
      r\rMail('partner-signup', $email, compact(ray('fullName reid customer noFrame'))); // so customer can continue
    } catch (Exception $e) {r\tellAdmin('bad email addr from partner', $args);} // bad email gets caught in signup submit also
    tellPartner($partnerA, $a, 'start', $customer);
  }

  return $a->id;
}

/**
 * Customer clicked on link to begin CG signup after completing partner signup
 * @param int $reid: r_relations record ID linking customer to partner
 * @param int $reid: (RETURNED) set to NULL if not a valid r_relations ID
 * @return assoc of signup args from r_relations record (NULL if none)
 *   redirects to signup step, if already in process
 */
function continuePartnerSignup(&$reid, $customer) {
  if (!$info = r\relation('other,data', 'reid=:reid AND code=:customer', compact('reid', 'customer'))) return $reid = NULL;
  extract($info);
  $a = r\acct($other);
  if ($a->federalId) return w\go("signin?name=$a->mainQid", t('Sign in to continue your account set-up.'));

  return unserialize($data) + ['myid' => @$a ? $a->id : 0];
}

/**
 * Gather the just-submitted checkbox values as which-boxes-to-check for boxesFld.
 * @param string $field: name of boxesFld field
 * @param assoc $opts: display names of the boxes, keyed by their index
 * @param assoc $sta: form status
 * @return flat array of values (not display names) of checked boxes
 */
function boxOpt($field, $opts, $sta) {
///   debug(compact('field','opts','sta'));
  foreach ((@$sta['input'][$field] ?: []) as $k => $v) $res[] = $k; // $opts[$k]; // note that k==v
  return @$res ?: [];
}
