<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Completely redo the login form (see user_login() in user.module).
 */
function formSignin($form, &$sta) {
  global $base_url;

  extract(just('name pw2 vKeyE', @$sta['input']));
  if (!isset($name)) $name = @$_GET['name']; // allow links to signin (for example from app)
  //$name = 'L5MA LLDC 3MBG';
  if (strlen(str_replace(' ', '', @$name)) >= ICARD_CODELEN_MIN and !strpos($name, '.') and !db\exists('users', 'name=:name', compact('name'))) {
    \drupal_get_messages(); // drop previous errors
    if (r\iCardAcct($name)) return w\go("signup/code=$name"); // invitation card manual signup!
    return w\go('accept', 'bad invite num', 'err');
  }
  unset($form['actions']);
  unset($form['pass']);
  focusOn((@$name == 'admin' or strlen(@$name) > 4) ? 'pass' : 'name');

  $title = t('Welcome to %PROJECT!');
  $signup = isDEV ? "<a href=\"$base_url/signup\">Signup</a>" : '';
  $subtext = t('<p>Not yet a member? <%a>Check it out</a>!</p>', '_a', atag(r\promoLink(''))) . $signup . t('<p><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right website (<b><em>%BASE_URL/</em>...</b>). And be sure to sign OUT when you\'re done!</p>');

  $_a = atag('/settings/password/' . @$name, ['tabindex' => 3]); // uses $name, so must precede $name = textField
  $passDesc = t('<%a>Sign-in problems?</a>', compact('_a'));
  list ($name, $pass) = w\signinFlds(@$name, t('Your account ID or email address'), $passDesc);
  $pw2 = hidFld(@$pw2 ?: ''); // extra password for admins
  $vKeyE = hidFld(@$vKeyE ?: ''); // extra password for admins
  
  $submit = t('Sign in');
  $form = compact(ray('title name pass pw2 vKeyE submit subtext'));

  $form['external_input'] = hidFld(serialize(@$sta['input'])); // pass this to rweb_user_login

  $form['#attributes']['class'][] = 'rweb labeled';
//  if (TRUE) js('e:\\rCredits-admin.js', 'external', 'footer'); // get extra admin password
//  js('radiocheck.js', 'file', 'footer'); // this normally gets called in showForm()
  return cgform($form);
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account ID, with password.
 * Sets $sta['uid'] to signify successful authentication.
 * @param string $type: 'acct', 'pw', or 'both' -- what parameters to check
 */
function formSignin_validate($form, &$sta, $type = 'both') {
  extract(just('name pass pw2 vKeyE', $sta['input']));
  if (isDEV and @$name == 'admin' and !a(1)) {
    require_once __DIR__ . '/../admin/admin.inc';
    a\setupBasicAccounts(); // happens sometimes during testing
  }
  if (!$uid = r\loginString(@$name)) return say('bad login', 'name');
  if (w\flooding($uid, $sta, $err)) return say($err, 'pass');
  $a = r\acct($uid);
  $sta['uid'] = ((isDEV and $uid == 1) or $a->passwordOkay($pass, 'pass', $err)) ? $uid : say($err, 'pass'); // ignore admin pass on dev machine
  w\updateFlood($sta, $success = (bool) $sta['uid']); // should be before pw2 and vKeyE checks
	if (!$success) return say('login failed', 'name');

  if ($uid == 1) { // later this should be if ($a->regAdmin) -- regional Admin permission
    $pw2 = @$pw2 ? u\b64decode($pw2) : (isDEV ? DEV_PW2 : '');
    if ($pw2 and !$a->pw2) a(1)->update('pw2', r\passHash($pw2)); // first ever admin signin sets pw2
    if (!$a->passwordOkay($pw2, 'pw2', $err)) return say(t('Admin requires second pass.'), 'pass');
    r\setCryptCook('pw2', $pw2); // store as cookie, so pw2 is never on server
		if (isDEV) u\setDft($vKeyE, \variable_get('vKeyE', '')); // make it easier to sign in
		if (@$vKeyE) { // set cookie for very-secure private key, encrypted (encrypt it further for cookie)
			if (!$vKeyE = u\b64decode($vKeyE)) return w\hack(t('bad vKeyE'));
		  r\setCryptCook('vKeyE', $vKeyE);
			if (isDEV) \variable_set('vKeyE', $vKeyE); // remember new vKeyE, make it easier to sign in (this is not redundant)
		}
  }

  return TRUE; // TRUE is used by formBuy()
}

/**
 * Finish signing in.
 * @param assoc $sta: intermal input or external input (from a member merchant website), including
 *   $uid: record ID of account signing in
 * @param string $goto: what page to start on for non-admin
 */
function formSignin_submit($form, &$sta, $goto = 'summary') {
  extract(just('uid input external_input vKeyE', $sta));
  if (!@$uid) return hack('sign-in is not authorized');
  $mya = r\acct($uid);
  global $user; $user = $mya->account('pass timezone'); // timezone keeps Drupal happy in bootstrap.inc
  $user->uid = $uid; // uid is not returned by ->account() (not sure why)
  \drupal_session_regenerate();
  
//  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
//  svar('external_input', @unserialize($sta['external_input'])); // save params from external online merchant

//  svar('myid', $uid);
//  svar('scanned_in', FALSE);
  setAcct($uid); // , TRUE);

//  r\setCook('loginto', '', r\rTime()-1); // no more default login
  u\loga('signin', ['name' => $mya->name]);
//  if ($mya->superAdmin) f('a.adminSignin');
  if (isSTAGE and !$mya->cAdmin) f('a.makeTestAdmins'); // make sure rCOs can test stuff
  return w\go($mya->admin2 ? 'sadmin/followup' : $goto);
}