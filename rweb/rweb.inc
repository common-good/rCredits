<?php
namespace rCredits\Web;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits as r;
use rCredits\Testing as t;

include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent
require_once __DIR__ . '/../rcredits-backend.inc';
require_once __DIR__ . '/../rcredits-testing.inc'; // else fails in showForm()

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

define('R_NONE', '<span class="none">--</span>'); // let the tests see null values

$GLOBALS[TX_WEB . ' messages'] = array(
//  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"

  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account ID is @quid. Your@asif starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in. Pretend to buy and sell just as you would in a typical month. We need that information, to make sure everyone has plenty of places to receive and spend their rCredits.</p><p>Experiment! Have fun!'),
  'info saved' => t('Your information has been saved.'),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  'options saved' => t('Your options have been saved'),
  'ok to continue' => t('You may now continue what you were originally doing.'),
  'password reset' => t('Your new password has been emailed to you.'),
  'changed agent' => t('You successfully changed the agent to @agentName.'),
  'changed account' => t('You successfully changed the account to @accountName.'),
  'nothing undone' => t('Nothing undone.'),
  'got asif credits' => t('You have received a "grant" of $@R_STARTING_BALANCE additional @R_ASIF_TEXT rCredits.'),
  'signed agreement' => t('You signed the rCredits Agreement on @date.'),
  'downloading' => t('Your download will begin shortly...'),

  // confirm
  'yes or no' =>  '@yesNo', // to replace with yesNo(url)
  'confirm undo' => t('Undo @created@asif transfer of @amount @tofrom @otherName?'),

  // errors
  'missing purpose' => t('<p>For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc.". Rebates and bonuses are intended as rewards for productive economic activity in rCredits.</p>
    <p>For everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.</p>'),
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no cells' => t('This account has no related cell phones.'),
  'required field' => t('@field must not be blank.'),
  'duplicate email' => t('WARNING: There is already an account with that email address. <a href="@password">Have you forgotten your password?</a>'), // mirrors message in user.module
  'bad name' => t('That is not a plausible name.'),
  'bad login' => t('That account does not exist. Check your typing?'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad account id' => t('That is not a proper account ID.'),
  'bad account number' => t('The account number must be three or more digits'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'no such company' => t('There is no such company.'),
  'op canceled' => t('Operation Canceled.'),
  'request & demand' => t('<div class="r-explain">There is a waiting list, to get more rCredits. You will receive an email when your turn comes around.</div><br><div class="r-amounts">Your total request for rCredits is <b>@request</b><br>Total demand for rCredits overall is <b>@demand</b></div>'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm delete cell' => t('Are you sure you want to remove the selected cell phones from this account?'),
  'already related' => t('That person is already related to this account. If you want to change the settings, find this person in the list and change the settings on that row.'),
  'already cell' => t('That cell phone (@number) is already connected to this account.'),
  'cell taken' => t('A cell phone can be connected to only one account. You are already using that cell phone (@number) in connection with account "@accountName". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'bad routing number' => t('US bank routing numbers are 9 digits. Use the number on the lower left edge of your checks.'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'unknown member' => t('"@who" has not yet signed up for rCredits. <a href="@draftLink">Click here to add them</a> to the list or <a href="" id="which-cancel">click here to cancel</a>.'),
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click on the one you want or, if you don\'t see them listed, <a href="@draftLink">Click here to add them</a> to the list or <a id="which-cancel">here to cancel</a>.'),
  'similar found' => t('Do you mean @otherName (@phone)? If so, <a href="@draftLink">click here</a>. Otherwise, click on any tab to cancel.'),
  'get app' => t('To scan Account ID cards with a smartphone, you need the app! You can download the Android app <a href="http://rCredits.org/rc.apk">here</a>. The iphone version is in the works (check again in January).'),
  'no help' => t('There is no written help available for this page. Give it a try! If you are still puzzled, please feel free to give us a call at @R_ORG_PHONE. Or send your question by <a href="mailto:@R_ORG_EMAIL?subject=help with rCredits Members Site, @page page" target="_blank">email</a>.'),
  'no permission' => t('You do not have permission to use that account.'),
  'bad transient id' => t('That QR code is out of date. Try again?'),
  'bad email' => t('The e-mail address <em class="placeholder">@email</em> is not valid.'),
  'bad company phone' => t('You must give a valid phone number for your company.'),
  'what relation' => t('Are you an owner or an employee of the company? (please choose one or both)'),
  'broke to' => t('You do not have enough@asif rCredits, for that transaction. <a href="@how">Go here</a> to get more rCredits.'),
  'incomplete agreement' => t('To sign this agreement, you must put a check in each box, meaning that you have read that section and agree to it.'),
  'bad signature' => t('That signature does not match your full legal name ("@fullName").'),
);

/**
 * Return yes and no buttons.
 * Do NOT use button() here (it kills the site)
 */
function yesNo($yesURL, $noURL = '', $yesNext = 'empty', $noNext = 'empty') {
  svar('yesNext', $yesNext);
  svar('noNext', $noNext);
  return <<<EOF
<br>
<input type="button" name="op" onclick="document.location.href='$yesURL'" value="Yes" class="form-local form-submit">
&nbsp;
<input type="button" name="op" onclick="document.location.href='$noURL'" value="No" class="form-local form-submit">
EOF;
}

function button($value, $href = '', $title = '', $type = '') {
  $title = htmlspecialchars($title);
  $button_type = 'local form-submit' . ($type ? " $type" : ''); // Drupal prefixes this with 'form-'
  $markup = array($value => formField('button', '', '', compact(u\ray('value button_type title'))));
  $markup = render($markup);
  $markup = str_replace('"submit"', '"button"', $markup); // compensate for Drupal pigheadedness
  if ($href) {
    if (strpos($href, 'http://') !== FALSE or substr($href, 0, 1) == '/') $href = "document.location.href='$href';";
    $name = 'name="op"';
    $markup = str_replace($name, ($button_type == 'local' ? '' : "$name ") . "onclick=\"$href\"", $markup);
//  if ($href) $markup = "<a href=\"$href\" title=\"$title\">$markup</a>";
  }
  return $markup;
}

function formField($type, $title = '', $description = '', $other = array(), $options = NULL, $ajax = array()) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  if ($type == 'submit' and !$title) $title = t('Submit');
  if (empty($other) and in_array($type, u\ray('item hidden submit'))) $other['value'] = $title;
  if ($type == 'select' and !isset($other['default_value'])) $other['default_value'] = key($options); // Drupal's select fails without a default
  $field = u\prefixKeys('#', array_merge(compact(u\ray('type title description options')), $other));
  return $field;
}

function test($arg) {
  global $user;
//  cache_set('t_resume', FALSE); // cleanup after potential previous failures
//  cache_set('t_messages', FALSE);
  include __DIR__ . '/../../gherkin/test.php';
  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate();
  \drupal_goto('handy');
}

function util($arg) {
//  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'reset') {
    r\reset();
/*    $uid = variable_get('rcredits_communityUid') + 1;
    $uid2 = $uid + 1; $uid3 = $uid2 + 1;
    $pass = '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae';
    $sql = <<<EOF
      INSERT INTO users (uid, name, pass, mail, full_name, status, postal_code) VALUES
      ($uid, 'uten', '$pass', 'uten@ex.com', 'U Ten', 1, '01301'),
      ($uid2, 'btwo', '$pass', 'btwo@ex.com', 'B Two', 1, '01301'),
      ($uid3, 'cornerstore', '$pass', 'cstore@ex.com', 'Corner Store', 1, '01301');
      
      INSERT INTO r_sms (number, uid, status) VALUES
      (10, $uid, 2),
      (2, $uid2, 2);
EOF;
    r\dbQ($sql);
    be\fund($uid, TX_SIGNUP);
    be\fund($uid2, TX_SIGNUP);
    r\dbQ(file_get_contents(__DIR__ . '\..\sql\r_companies-insert.sql'));
*/
  }
  
  if ($arg == 'clear_sessions') r\dbQ('TRUNCATE table SESSIONS');
  
  \drupal_set_message("Doing rCredits Util '$arg'...");
  \drupal_goto('handy');
}

function handy_links() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
    'rcredits/test?menu=1|Test Menu|',
//    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
    'sms|Simulate an SMS transaction|Simulate SMS',
//    'admin/config/development/testing|Test|Test',
//    'deletetests.php|Delete old tests|Delete old tests',
//    'rcredits/util/reset|Reset rCredits|',
    'rcredits/util/clear_sessions|Clear Sessions|',
    
    'devel/settings|Helper functions, pages, and blocks to assist Drupal developers. The devel blocks can be managed via the block administration page.|Devel settings',
    'devel/cache/clear?destination=node|Clear the CSS cache and all database cache tables which store page, node, theme and variable caches.|Empty cache',
    'devel/entity/info|View entity information across the whole site.|Entity info',
    'devel/php|Execute some PHP code|Execute PHP Code',
    'devel/field/info|View fields information across the whole site.|Field info',
    'devel/reference|View a list of currently defined user functions with documentation links.|Function reference',
    'devel/elements|View the active form/render elements for this site.|Hook_elements()',
    'devel/menu/item?path=node|Details about a given menu item.|Menu item',
    'devel/phpinfo|View your server&#039;s PHP configuration|PHPinfo()',
    'devel/menu/reset?destination=node|Rebuild menu based on hook_menu() and revert any custom changes. All menu items return to their default settings.|Rebuild menus',
    'devel/reinstall?destination=node|Run hook_uninstall() and then hook_install() for a given module.|Reinstall modules',
//    'devel/run-cron|Run cron|Run cron',
    'devel/session|List the contents of SESSION.|Session viewer',
    'devel/theme/registry|View a list of available theme functions across the whole site.|Theme registry',
    'devel/variable?destination=node|Edit and delete site variables.|Variable editor',
  );

  $result = array();
  foreach ($links as $link) {
    list ($url, $title, $label) = explode('|', $link);
    if (!$label) $label = $title;
    if (!u\abbreviates('http://', $url)) $url = $base_path . $url;
    $result[] = <<<EOF
<div><a href="$url" title="$title">$label</a></div>
EOF;
  }
  $result = join(' | ', $result);
  return "<div class='develw'>\n$result</div>";
}

function showForm($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $formOutput; // for testing
  try {
    setupGlobals(); // in case it's an anonymous form, so webAccess doesn't have a chance to set up
//    $layout = @$_GET['layout'] ?: svar('layout');
//    if ($function == 'I') $layout = 'Mobile';
//    $newLayout = ($layout != svar('layout'));
    
    $args = 'arg1' . ($arg2 !== '' ? ' arg2' : '') . ($arg3 !== '' ? ' arg3' : '') . ($arg4 !== '' ? ' arg4' : '');
    if ($function != 'accounts' and $function != 'summary') u\log("show $function", compact(u\ray($args)));
//    if ($layout == 'Mobile') css('mobile.css', 100);
    $formName = substr($function, 0, 1) == '\\' ? substr($function, 1) : "rCredits\\Web\\form$function";
//    if ($newLayout) svar('layout', $layout); // do this before drupal_get_form, so forms can know the current layout
    $form = \drupal_get_form($formName, $arg1, $arg2, $arg3, $arg4);
    $sta = array(); \rweb_form_alter($form, $sta, $formName);
    $form['#attributes']['class'][] = 'rweb';
//    if ($newLayout) $form['#cache'] = FALSE;
    if (!@$form['confirm']) $form += which(); // add the choices popup form, if any


    if ($function != 'help') if (($i = array_search('title', array_keys($form))) !== FALSE or $function == 'Txs') {
      if ($function == 'Txs') $i = -1; // no title on Transactions page, so insert at beginning
      $i = -1; // always at beginning
      $extras = array('#id' => 'help-link', '#weight' => @$form['title']['#weight']);
      $help_link = item(t('<a><div></div>Help with this page</a>')) + $extras; // add a help link at top of page
      $form = array_slice($form, 0, $i+1) + compact('help_link') + array_slice($form, $i+1);
      js('inc/misc.js', 'file', 'footer');
//      debug($form);
    }

    $form2 = $form; $rent = render($form2);
    t\testOutput($formOutput = t\strip($rent), 'screen');
    if (strpos($rent, 'form-type-checkbox') or strpos($rent, 'form-type-radio')) js('inc/radiocheck.js', 'file', 'footer');
    
    if ($colgroup = @$form['#colgroup']) {
      unset($form['#colgroup']);
      $markup = '';
      foreach ($colgroup as $col) {
        $guts = '';
        foreach ($col as $attrib => $value) $guts .= " $attrib=\"$value\"";
        $markup .= "<col $guts>\n";
      }
      $markup = "<colgroup>\n$markup</colgroup>\n";
      return str_replace('<thead>', $markup . '<thead>', $rent); // shore up Drupal lack of colgroups in tableselect
    } else return $rent;
  } catch(u\fit $e) {r\Web\exception_handler($e);}
}

function setupGlobals($agent = '') {
  global $user, $channel;
  if (@$channel) return; // already setup
  if (!$agent) $agent = @$user->uid; // register_submit supplies agent (no one else does)
  $cuid = svar('cuid') ?: $agent;
  r\acct::setDefault(r\acct($cuid, $agent)); // set default for r\acct()
  $channel = TX_WEB;
}

/**
 * Set or retrieve an rCredits session variable.
 */
function svar($name) {
  $name = 'rcredits_' . $name;
  $args = func_get_args();
  if (count($args) == 1) return @$_SESSION[$name]; else $_SESSION[$name] = $args[1];
}

/**
 * Offer choices for user to choose from
 * 1. If called with all arguments, the arguments are stored in svar('pop') and the form is reloaded
 * 2. If called as which('info'), this function returns $info from svar('pop')
 * 3. If called with NO arguments (from showForm()), this function returns a fieldset to insert as a popup layover div into
 * the form awaiting the choice. Clicking on a choice fills in the missing value and submits the form.
 * For #3, we expect svar('pop') to contain: $formName, $choices, $resultField, and $question
 *   $formName: the form name
 *   $choices: an array of choices
 * @param array $choices: associative array of things to choose from
 * @param string $resultField: name of field that gets the choice
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $question: the question to ask
 */
function which($choices = '', $resultField = '', $info = '', $question = 'Which one?') {
  $formName = current_path();
  
  if (is_array($choices)) { // called with all args (#1)
    svar('pop', compact(u\ray('formName choices resultField info question')));
    r\go($formName); // reload page
  }

  if (!is_array($pop = svar('pop'))) return array();
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($formName != $pop_formName) return array();
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return array();
  
  svar('pop', NULL); // called with no args (#3)
  $onchange = "this.form.elements['$pop_resultField'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $subtext = item($pop_question);
  $choice = formField('select', '', '', attrib(compact('onchange') + array('size' => 12)), $pop_choices);
  $close = popupCloser('which'); // this is needed in mobile layout
  $popup = formField('fieldset', t('Which One?'), '', array('id' => 'which')) + compact(u\ray('subtext close choice'));
  return compact('popup');
}

function keep_values(&$form) {
  if (!($info = which('info'))) return $form;
  foreach ($form as $key => $value) {
    if (isset($info[$key])) {
      $form[$key]['#default_value'] = $info[$key];
    } elseif (substr($key, 0, 1) != '#') foreach ($form[$key] as $key2 => $value2) { // fieldset
      if (isset($info[$key2])) $form[$key][$key2]['#default_value'] = $info[$key2];
    }
  }
  return $form; // for convenience we both modify the form and return it
}

/**
 * Modal window closer
 *//*
function close_form() {
  $js = 'window.opener = bottom; window.close();';
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  return '';
}
*/

/**
 * Generate choices for a member autocomplete field
 * @param string $string: what the user has typed so far
 * @param string $role: 'payer', 'payee', 'industry', 'company', or '' (other), depending on the role of the user
 * @param int $cuid: the current account (globals and session variables are inappropriate here)
 * @todo: ditch this altogether or make it efficient with separate rCredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the autocomplete URL to the backend? privacy is not crucial here
 * @todo: fix Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($role = '', $cuid = 0, $string = '') {
//  return drupal_json_output(array('zot' => "role=$role cuid=$cuid string=$string"));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\shortName($string, '%') . '%';
//  $string = str_replace(' ', '%', \db_like(" $string "));
  $params = 'string';
  
  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
/*     AND (t.$role IS NULL OR t.$role=:cuid) ORDER BY t.created DESC, u.full_name
    $selection = <<<EOF
      u.uid, u.full_name AS display, u.mail AS email 
      FROM users u LEFT JOIN {zxs} t ON t.$other=u.uid 
      WHERE (u.name LIKE :string OR u.mail LIKE :string)
      AND (t.$role IS NULL OR :cuid IN (t.payer, t.payee))
      ORDER BY t.created DESC, u.full_name
*/
    $selection = <<<EOF
      u.uid, u.full_name AS display, u.mail AS email 
      FROM users u WHERE (u.uid>1 AND (u.name LIKE :string OR u.mail LIKE :string))
EOF;
    $params .= ' cuid';
  } elseif ($role == 'industry') { // the one place we don't need to include the qid (for definiteness)
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT full_name AS display FROM users WHERE (uid>1 AND account_type!=:R_PERSONAL AND name LIKE :string)
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, full_name AS display FROM users WHERE (uid>1 AND account_type!=:R_PERSONAL AND name LIKE :string)';
  } else $selection = 'uid, full_name AS display FROM users WHERE (uid>1 AND name LIKE :string)';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => $sql));

  $result = r\dbQ($sql, compact(u\ray($params)));
  $matches = array();
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = substr(r\quid($uid), 3) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
  }
  drupal_json_output($matches);
}

function formHelp($form, &$sta, $what = '') {
  require_once __DIR__ . '/rweb-help.inc';
  $title = item($what ? t('Help for ') . ucwords($what) : t('General Help'));
  $text = formField('item', '', '', helpText($what ?: 'general'));
  return compact(u\ray('title text'));
}

function popupCloser($idHead) {
  global $base_url;
  return item("<img src='$base_url/sites/all/modules/rcredits/images/icons/close.png' border='0'>") + array('#id' => "$idHead-close");
}

/**
 * Return a help overlay
 * @todo: make it move right when "labeled"
 */
function help($page, $class = '') {
  require_once __DIR__ . '/rweb-help.inc';
//  $subtext = item($question);
  $close = popupCloser('help');
  $text = formField('item', '', '', helpText($page));
  $help = formField('fieldset', t('Help with this page'), '', array('id' => 'help')) + compact(u\ray('close subtext text'));
  return compact('help');
}

/**
 * Show the user's current membership status and steps to get to the next milestone.
 * @todo: This function also currently gathers the data. Separate that out, so it can be summarized on Summary screen.
 */
function formMembership($form, &$sta) {
  if (!($acct = r\agent())) return NULL;
  $stages = u\ray(R_STAGES);
  $stage = r\stage();
//  $stage = 'rTrader'; // for testing

  if ($stage == 'Member') {
    $id = $acct->id;
    $where = 'state!=:TX_DELETED AND type=:TX_TRANSFER AND goods';
    $buys = $where . ' AND (NOT taking AND payer=:id)';
    $sells = $where . ' AND (taking AND payee=:id)';
    $buyCount = r\txField('COUNT(DISTINCT payee)', $buys, compact('id')); // (presumably selected from asif table)
    $sellCount = r\txField('COUNT(DISTINCT payer)', $sells, compact('id'));
//    debug(compact('buyCount','sellCount','buys','sells'));
    $asifed = ($buyCount >= R_ASIF_BUY_MIN and $sellCount >= R_ASIF_SELL_MIN);
    $usedId = $acct->hasBit(BIT_USED_ID);
    $sawVid1 = $acct->hasBit(BIT_SAW_VID1);
    $eligible = ($usedId and $asifed and $sawVid1);

    $message1 = u\getConstants(t(<<<EOF
<h2>Congratulations on becoming an rCredits Member!</h2>
<p>As a member, you are eligible to do :R_ASIF_TEXT transactions, to learn how to use rCredits. These transactions should be realistic amounts that you spend at actual businesses in a typical month. Or realistic amounts that you get paid by your employer, by your customers, or by anyone who pays you. The :R_ASIF_TEXT transactions don't count, but you get <b>real rewards</b> for doing them. You can spend your rewards once you are approved to use rCredits for real.</p>
<p><b>Your next milestone</b>, toward using rCredits for real, is to become a <em>Partner</em>. Here's what you need to do, to be eligible to be a Partner. Click on any step, to get started.</p>
EOF
    ));

    $step1 = t('Print out an <a>Advance ID card</a> for yourself AND get another member (or member business) to scan it with their smartphone using a free QR Code Reader app. This is how you normally pay for things with rCredits -- just carry your card around! Once you are authorized to use rCredits for real, a permanent Member ID card will arrive in the mail.');
    $step2 = u\getConstants(t('Think about where you spend money and where you get money. Complete at least :R_ASIF_BUY_MIN :R_ASIF_TEXT (realistic) <a href="pay">purchases</a> and at least :R_ASIF_SELL_MIN :R_ASIF_TEXT <a href="charge">charge</a>.'));
    $step3 = t('Watch our <a>5-minute video</a> about how rCredits works.');
    $message1 .= memberStep(1, $usedId, $step1, 'Advance-ID-Card', TRUE)
               . memberStep(2, $asifed, $step2, $buyCount < R_ASIF_BUY_MIN ? 'pay' : 'charge')
               . memberStep(3, $sawVid1, $step3, 'membership/video/1');

    if ($eligible) { // don't bother with all this unless eligible
      $donated = ($acct->tithe > 0 or r\tithe($acct)); // r\tithe() also updates the database
      $signed = $acct->signed_agreement;
      $message2 = t(<<<EOF
<h2>You are Eligible to Become a Partner in rCredits!</h2>
<p>Being a Partner means you accept a higher level of commitment to rCredits and the Common Good Economy. This is an easy, essential milestone on your way to using rCredits for real. To accept this invitation and become an rCredits Partner:</p>
EOF
      );
  //<p>As a Partner, you accept a joint responsibility with hundreds of other rCredits members, for the economic well being of your community. This is an 
      $step1 = t('Sign the <a>rCredits Agreement</a>.');
      $step2 = t('Make a <a>contribution</a> of any size, to support the rCredits system.');
      $message2 .= memberStep(1, $signed, $step1, 'membership/agreement')
                 . memberStep(2, $donated, $step2, 'http://rCredits.org/donate');
      if ($eligible and $signed and $donated) {
        $acct->setBit(BIT_PARTNER);
        $stage = 'Partner';
      }
      $message = $message2;
    } else $message = $message1;
  }
  
  if ($stage == 'Partner') {
    $gotContact = ($acct->phone and $acct->mail and $acct->address and $acct->city and $acct->country);
    $sawVid2 = $acct->hasBit(BIT_SAW_VID2);
    $passedTest = $acct->hasBit(BIT_PASSED_TEST);
    $met = $acct->hasBit(BIT_MET);
    $eligible = ($gotContact and $sawVid2 and $passedTest);

    $message1 = t(<<<EOF
<h2>Congratulations on becoming a Partner in rCredits!</h2>
<p>Your final milestone, on the way to using rCredits for real, is to become an rCredits Steward. Being a Steward means you have a good understanding of how to manage the rCredits system in your community, together with hundreds of other members.</p>
<p>To become an rCredits Steward:</p>
EOF
    );
    $step1 = t('Complete your <a>contact information</a>.');
    $step2 = t('Watch our <a>5-minute video</a> about how to manage rCredits as a community.');
    $step3 = t('Take a <a>short quiz</a> about the video.');
    $message1 .= memberStep(1, $gotContact, $step1, 'account/contact')
               . memberStep(2, $sawVid2, $step2, 'membership/video/2')
               . memberStep(3, $passedTest, $step3, 'membership/steward-test');
               
    $message2 = t(<<<EOF
<h2>Hurray! You are Eligible to Be a Steward of rCredits!</h2>
<p>To actually BE a Steward, you must participate in at least one rCredits community meeting -- either online or in person.</p>
EOF
    );
    $step = t('<a>Sign up</a> for an online meeting or <a>verify</a> that you have participated at a meeting in your community.');
    $message2 .= memberStep('', $met, $step, 'meetings');
    $message = $eligible ? $message2 : $message1;
    if ($eligible and $met) {
      $acct->setBit(BIT_STEWARD);
      $stage = 'Steward';
    }
  }

  if ($stage == 'Steward') {
    $gotPhoto = ($acct->picture > 0);
    $gotIdProof = (@$acct->id_proof > 0 and @$acct->dob and @$acct->federal_id); // include SS#
    $gotUsdInfo = (@$acct->usd_account and @$acct->usd_auth);
    $demanding = ($acct->demand > 0);
    $rqualified = $acct->hasBit(BIT_RQUALIFIED);
    $eligible = ($gotPhoto and $gotIdProof and $gotUsdInfo and $demanding);

    $message1 = t(<<<EOF
<h2>Congratulations on becoming a Steward of rCredits!</h2>
<p>There are a few final tasks we need you to complete, before you are eligible to become an rTrader and use rCredits for real:</p>
EOF
    );
    $step1 = t('<a>Upload a photo</a> for your Member ID Card.');
    $step2 = t('Open a <a>Dwolla</a> account and <a href="account/usd">connect it</a> to your rCredits account. In partnership with Veridian Federal Credit Union, Dwolla connects to your US Dollar bank account, so you can easily exchange US Dollars for rCredits and vice versa.');
    $step3 = t('<a>Upload a photo</a> of your driver\'s license, birth certificate, passport, or other official ID.');
    $step4 = t('Choose an amount of US Dollars that you would like to <a>trade for rCredits</a> (Remember that you will also get some free when you become an rTrader).');
    $message1 .= memberStep(1, $gotPhoto, $step1, 'account')
               . memberStep(2, $gotUsdInfo, $step2, 'http://dwolla.com', TRUE)
               . memberStep(3, $gotIdProof, $step3, 'account/usd')
               . memberStep(4, $demanding, $step4, 'exchange');

    $message2 = u\getConstants(t(<<<EOF
<h2>You are eligible to be an rTrader!</h2>
<p>You have completed all the requirements. At the end of each month, a limited number of members are promoted to rTrader status. Members are selected according to who makes the strongest economic circles -- that is, who provides goods and services that are most in demand among the current rTraders. Companies will typically be promoted at the same time as their eligible employees.</p>
<p>Expect a short wait. rCredits growth is accelerating. Once enough people in your community sign up, everyone who is eligible will be promoted to rTrader status immediately.</p>
<p>You can improve your position to be chosen, by encouraging individuals and companies in your own economic circle to sign up. If you are an employee, invite other employees and the company itself to sign up. If you are a business owner, invite your customers, employees, and suppliers to sign up. If you are self-employed or retired, think about where your money comes from and invite them to sign up. Doing :R_ASIF_TEXT transactions is one way to invite and encourage others to sign up.</p>
EOF
    ));
    $step = u\getConstants(t('Do some more <a>:R_ASIF_TEXT transactions</a>.'));
    $message2 .= memberStep('', FALSE, $step, 'charge'); // do nothing with $rqualified?
    $message = $eligible ? $message2 : $message1;
  }
  
  if ($stage == 'rTrader') {
    $message = t(<<<EOF
<h2>Congratulations! You are an rTrader.</h2>
<p>You are authorized to use rCredits for real transactions. You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning for a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>
EOF
    );
  }
  $message = "<table width=\"100%\">$message</table>";
  
  $rTrader = t('authorized to use rCredits "for real"');
  $Steward = t("a responsible voter, stewarding your community's finances");
  $Partner = t('a committed rCredits member');
  $Member = u\getConstants(t('signed up to do rCredits :R_ASIF_TEXT transactions, for real credit'));
  $details = compact($stages);
  foreach ($details as $key => $value) $details[$key] = "<em>$key</em> - $value<br>\n";

  extract($details);
  $Partner = $Member . $Partner;
  $Steward = $Partner . $Steward;
  $rTrader = $Steward . $rTrader;
  $now = compact($stages);
  foreach ($now as $key => $value) $now[$key] = "<div id='overview-stage' class='labeled-block'>$value</div>\n";

  extract($details);
  $Member = $Partner . $Steward . $rTrader;
  $Partner = $Steward . $rTrader;
  $Steward = $rTrader;
  $rTrader = '';
  $future = compact($stages);
  foreach ($future as $key => $value) $future[$key] = "<div id='overview-future' class='labeled-block'>$value</div>\n";

  $title = item(t('Membership Status'));
  $subtext = item(t('for ') . $acct->full_name);
  $stage_now = formField('item', t('You are a:'), '', $now[$stage]);
  $future = $future[$stage] ? formField('item', t('Not yet a:'), '', $future[$stage]) : NULL;
  $steps = formField('item', '', '', $message);
//  $steps = formField('item', t('Next Steps:'), '', t('<div id="next-steps" class="labeled-block"><ol><li>Do at least three realistic "As-If" Payments to participating businesses, so the system knows where you will be able to spend your rCredits.</li><li>Do at least one As-If Charge, so the system knows where you will get your rCredits.</li><li>Try some of the other rCredits features.</li><li>Read or talk to other participants until you are confident that you understand how rCredits works.</li></ol></div>'));
  $form = compact(u\ray('title subtext stage_now future steps'));
  return labeled($form);
}

function memberStep($step, $done, $text, $link = '', $newWindow = FALSE) {
  $target = $newWindow ? ' target="_blank"' : '';
  if ($link) {
    $link = "<a href=\"$link\"$target>";
    $linkx = '</a>';
    $text = str_replace('<a>', $link, $text);
  } else $link = $linkx = '';
  return <<<EOF
<tr>
  <td class="done state$done">$link<div>&nbsp;</div>$linkx</td>
  <td class="step step$step">$link<div>$step</div>$linkx</td>
  <td class="text"><div>$text</div></td>
</tr>
EOF;
}

/**
 * Give member a chance to sign the rCredits Agreement.
 */
function formAgreement($form, &$sta) {
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
  $id = "edit-$name";
  $check = formField('checkbox', '', '', attrib(compact('id', 'name')));
  $model = \render($check);
  if ($date = r\agent()->signed_agreement) {
    $instructions = '';
    $date = u\formatDate($date);
    say('signed agreement', compact('date'));
  } else $instructions = <<<EOF
<div id='instructions'><b>Instructions:</b> Put a check mark in each checkbox to indicate that you have read that section and agree to it. Really DO read the agreement and make sure you understand it! This agreement, that we make with each other, is the foundation for the rCredits system and the Common Good Economy.</div>
EOF;

  $agreement = <<<EOF
$instructions
<div id='agreement'>
<h2>A. I recognize that:</h2>
<ul>
  <li>[-2] rCredits will be the framework for the Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1] Together as a community we have the power to issue money for our own use. It is our promise to accept rCredits that gives them value.</li>
  <li>[0] Governance by the people is essential, for a just society.</li>
</ul>

<h2>B. As a responsible participant in the Common Good Economy:</h2>
<ol>
  <li>[1] I will accept rCredits as payment for my goods and services, without limit, as long as I can spend them easily or exchange them easily for US Dollars. I will exchange rCredits for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will price my goods and services the same in rCredits and in US Dollars. Whenever I exchange rCredits for US Dollars or vice versa, I will exchange them one for one.</li>
  <li>[3] I understand that I will receive some rCredits from the community as Incentive Rewards. If I stop participating (or if the system ends) within six months of receiving my first rCredits, I will immediately pay back whatever amount of Incentive Rewards I have received (while keeping any rCredits I received from other participants).</li>
  <li>[4] I promise to maintain a high level of honesty, integrity, and ethics in my dealings with other rCredits participants and with the community (including the rCredits system itself). When there is a dispute, I will follow the Dispute Resolution Process as written in the ever-evolving Common Good Economy governance documents, and will honor its outcome.</li>
  <li>[5] I agree to participate responsibly in managing the Common Good Economy to the best of my ability and as my life circumstances allow, including:
    <ul>
      <li>understanding the workings and governance of the Common Good Economy,</li>
      <li>participating in the decision process to set funding priorities for my community and for the common good of all, and</li>
      <li>participating in other decisions (including amending the Common Good Economy governance documents) from time to time, as needed.</li>
    </ul></li>
</ol>
</div>
EOF;
//  <li>[5] I will move my deposits and investments to the Common Good Economy when that is sensible, in my judgment.</li>

  for ($i = -2; $i <= 5; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", $box, $agreement);
  }
  $agreement = formField('item', '', '', $agreement);
  $signed = formField('textfield', t('Signed:'), t('Type your full name here'));
  $submit = submit();
  $form = compact(u\ray('title agreement signed submit'));
  return labeled($form);
}

function formAgreement_validate($form, &$sta) {
  for ($i = -2; $i <= 5; $i++) if (!@$_POST["check$i"]) return say('incomplete agreement', "check$i"); // $_POST not $sta
  $fullName = r\agent()->full_name;
  if (strcasecmp(trim($sta['values']['signed']), $fullName) != 0) return say('bad signature', compact('fullName'), 'signed');
}

function formAgreement_submit($form, &$sta) {
  $signed_agreement = time();
  r\agent()->update(compact('signed_agreement'));
}

function formVideos($form, &$sta) {
  $title = item(t('rCredits Member Videos'));
  $subtitle = formField('item', '', '', t('The videos may not be available until mid-January. Try again in a few days!'));
  $form = compact(u\ray('title subtitle'));
  return labeled($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from r\go() in formSummary

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 */
function formSummary($form, &$sta) {
  global $base_url;
  
  $cacct = r\acct();
//  debug($cacct->can(-1));
  if (!$cacct) r\go('user');
  
  extract(be\creditInfo()->fancy, EXTR_PREFIX_ALL, 'my');

  $name = $cacct->name;
  $address = u\location($cacct, TRUE);

  if (!$cacct->isCompany() or !$cacct->proSe()) {
// FAILS on mobile    $idLink = 'id="idcard"'; // use jQuery to create popup (see inc/misc.js)
    $idLink = 'href="Advance-ID-Card" target="_blank"';
    $card = $cacct->mainQid ? ("<a $idLink>" . t('Print Advance ID Card') . '</a>') : ''; // no card for community accounts
if ($cacct->agentId == 1) $card .= ' [<a href="http://localhost/devcore/rcredits/test?module=rcredits/rweb&feature=Signup&scene=&restart=1">run current test</a>]'; // DEBUG (keep)
  } else $card = t("To print employee ID cards, visit the <a href=\"$base_url/account/relations\">Account Relations</a> page.");

  $title = item(t('Account Summary'));
//  'subtext = basic_account_info();
  $accountName = formField('item', t('Name:'), $address, $cacct->full_name . ' ' . u\hug($name));
  $balance = formField('item', t('Balance:'), '', $my_balance . (r\isRTrader() ? '' : R_ASIF_TEXT));
  if ($cacct->id > 0) {
    $accountId = formField('item', t('Account ID:'), $card, ($cacct->mainQid ?: 'N/A') . ' ' . u\hug(R_REGION_NAME));
    $account_type = formField('item', t('Account type:'), '', $GLOBALS['account types'][$cacct->account_type+0]); // +0 for admin
    $min_balance = formField('item', t('Credit limit:'), '(You can go this far in debt for up to 30 days)', u\formatAmount(-$cacct->min_balance));
    $rewards = formField('item', t('Rewards:'), t('(to date)'), $my_rewards);
  }
  if (r\agent()->can(PERM_CTTY_ADMIN) and !$cacct->proSe() and $cacct->id > 0) { // ctty admin not for self or community
    $account_type = formField('select', t('Account type:'), '', $cacct->account_type+0, $GLOBALS['account types']); // +0 for admin
    $min_balance = formField('textfield', t('Credit limit:'), '(You can go this far in debt for up to 30 days)', dft(u\formatAmount(-$cacct->min_balance)));

    $stageChoices = u\ray(R_STAGES);
    if ($cacct->account_type != R_PERSONAL) array_splice($stageChoices, 1, 2); // just Member or rTrader for companies
    $stage = formField('select', 'Stage:', '', dft(r\stage($cacct, TRUE)), $stageChoices);
    $submit = submit();
  }

  $form = compact(u\ray('title accountId accountName account_type balance min_balance rewards stage submit'));
  return labeled($form);
}

function formSummary_validate($form, &$sta) {
  extract(u\just('account_type min_balance stage met', $sta['values']));
  if (u\badAmount($min_balance)) return say('bad amount', 'min_balance'); 
  $sta['values']['min_balance'] = -u\cleanAmount($min_balance);
}

function formSummary_submit($form, &$sta) {
  extract(u\just('account_type min_balance stage met', $sta['values']));
  $cacct = r\acct();
  $cacct->update(compact(u\ray('account_type min_balance')));
  for ($i = 1; $i <= BIT_RTRADER; $i++) $cacct->setBit(1 << ($i - 1), $i <= $stage);
  say('account updated');
}

function formAccount($form, &$sta) {
  $form['#user'] = r\acct()->account();
  user_account_form($form, $sta);
  system_user_timezone($form, $sta);

  $form['title'] = formField('item', t('Account Information'), '', weight(-99));
  unset($form['account']['name']);
  $form['account']['mail']['#description'] = t('All emails from the system will be sent to this address. The email address is not made public.');
  $form['account']['pass']['#description'] = t('Enter your new password in both fields above.<br>To change your email address and/or password, enter your current password below.');
  $form['account']['pass']['#process'] = array('form_process_password_confirm', 'rCredits\\Web\\password_confirm');
  $form['account']['current_pass']['#description'] = '';
  $form['account']['current_pass']['#weight'] = 4;
  $form['account']['status']['#weight'] = 5;
  $form['account']['roles']['#weight'] = 6;
  $form['submit'] = formField('submit', '', '', array('value' => t('Save')) + weight(99));
  $form['#user_category'] = 'account';
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'][0] = 'rCredits\\Web\\formAccount_validate'; // use ours instead
  
  return $form;
}

/**
 * Implements expand_password_confirm.
 */
function password_confirm($element) {
  $element['pass1']['#title'] = t("New Password (if any)");
  $element['pass2']['#title'] = t("Confirm New Password");
  return $element;
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formAccount_validate($form, &$sta) {
  $mail = trim($sta['values']['mail']);
  form_set_value($form['account']['mail'], $mail, $sta);
  if ($error = user_validate_mail($mail)) form_set_error('mail', $error);
}

function formAccount_submit($form, &$sta) {
  $account = r\acct()->account();
  \user_save($account, $sta['values']);
  r\acct()->reread();
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  if (!($cacct = r\acct())) return NULL; // no account selector before login
  $cuid = @$cacct->id;
  if (!($accounts = array_keys(be\accountChoices()))) return FALSE;
  if (!@$accounts[$cuid] and $cacct->can()) array_splice($accounts, 1, 0, $cuid); // admin or ctty_admin

  $choices = array();
  foreach ($accounts as $uid) {
    $balance = u\formatAmount(be\creditInfo(compact('uid'))->balance);
    $choices[$uid] = r\userField('full_name', $uid) . ": $balance";
  }

  $qid = r\quid(); // $credit_id = r\userField('credit_id', $cuid);
  $full_name = r\userField('full_name', $cuid);
  $onchange = "this.form.submit();";
//  $account = formField('select', t('Account:'), t('(Select an account)'), dft($cuid) + attrib(compact('onchange')), $choices);
  $oneText = "$full_name: $balance";
  
  $account = count($choices) == 1 ? 
      formField('textfield', '', '', dft($oneText) + attrib(array('disabled' => TRUE, 'size' => strlen($oneText)+2)))
    : formField('select', '', '', dft($cuid) + attrib(compact('onchange')), $choices);
//  $link = item('<a href="http://rCredits.org">Main Site</a>');
  $submit = submit(); // default submission
  $showMenu = formField('item', '', '', button('Menu', '', $title = 'Show the menu'));
  $showHelp = formField('item', '', '', button('Help', '', $title = 'Help for this page'));
//  $version = item(t('v 0.1d'));
  $form = u\prefixKeys('account_', compact(u\ray('account showMenu showHelp submit'))); // always give a dropdown

  $chunks = explode('/', $_SERVER['REQUEST_URI']);
  if (isDEV) array_shift($chunks); // site is in a subfolder on the dev server
  $chunkIndex = (($chunks[1] == 'account' and @$chunks[2]) or $chunks[0] == 'user') ? 2 : 1;
  $function = $chunks[$chunkIndex];
  $menu = formMenu($function);
  $help = help($function);
  $form += $help + $menu;

  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op account_account', $sta['values']));
  $cacct = r\acct();
  if (!$cacct->can(PERM_ADMIN) and !r\acct($account_account, $cacct->agentId)->can()) r\go('user/logout'); // hack attempt?
  if ($op == 'Menu') {
    r\go('menu'); 
  } else svar('cuid', $account_account); // this gets checked ONLY in setupGlobals()
}

function formMenu($page) {
  $menu = array()
        + goButtonIf(PERM_SELL, t('Charge'), 'charge', t('Charge someone'))
        + goButtonIf(PERM_BUY, t('Pay'), 'pay', t('Pay someone'))
  ;
  if ($lastZxid = svar('lastZxid')) $menu += goButton(t('Undo'), "undo", t('Undo the last transaction'));
  if (r\otherBalance(svar('lastOther'))) $menu += goButton(t('Customer Bal'), 'remainder', t("Display the customer's current balance."));
//  if (count($menu) == 1) $menu += goButton(t(''), '', ''); // filler
  $menu += array()
        + goButtonIf(PERM_MANAGE, t('Summary'), 'summary', t('Account summary'))
        + goButtonIf(PERM_MANAGE, t('Transactions'), 'transactions', t('Show transaction history'))
        + goButtonIf(PERM_MANAGE, t('Get r/USD'), 'exchange', t('Exchange rCredits for USD or vice-versa'))
        + goButton(t('Find Companies'), 'directory', t('Find an rCredits member company'))
        + goButton(t('Help'), "help/$page", t('Help with this page'))
        + goButton(t('Reports'), 'reports', t('rCredits syste-wide or community-wide activity reports'))
        + goButtonIf(PERM_MANAGE, t('Account'), 'account', t('Basic account information'))
        + goButtonIf(PERM_MANAGE, t('Mobile'), 'account/cells', t('Authorize mobile devices for this account'))
        + goButtonIf(PERM_MANAGE, t('Relations'), 'account/relations', t('Connect other members to this account'))
        + goButtonIf(PERM_MANAGE, t('Preferences'), 'account/preferences', t('Set your account preferences'))
        + goButtonIf(PERM_MANAGE, t('Contact Info'), 'account/contact', t('Tell us where to find you'))
        + goButtonIf(PERM_MANAGE, t('Membership'), 'membership', t('Your rCredits membership progress'))
        + goButtonIf(PERM_MANAGE | PERM_COMPANY, t('Company Info'), 'account/company', t('Describe this company'))
        + goButton(t('Logout'), 'user/logout', t('End your rCredits session'))
  ;
  $menu = formField('fieldset', '', '', array('id' => 'menu')) + compact('menu');
  return compact('menu');
}

function goButton($label, $goto, $title) {
  $gotoHead = isDEV ? '/devcore' : '';
  $name = str_replace('/', '_', $goto);
  return array($name => formField('item', '', '', button($label, "$gotoHead/$goto", $title)));
}

function goButtonIf($perm, $label, $goto, $title) {return r\acct()->can($perm) ? goButton($label, $goto, $title) : array();}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company) {
  $uid = r\userField('uid', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or !$acct->isCompany()) return say('no such company', 'zot');
  extract((array) $acct->account(), EXTR_PREFIX_ALL, 'my');

  $pic = profile_picture($uid, TRUE);
  $country = u\countryAbbrev($my_country);
  $website = $my_website ? "<a href='http://$my_website' target='_blank'>$my_website</a>" : '';
  $phone = $my_phone ? 'Phone: ' . u\formatPhone($my_phone, '+') : '';
  $fax = $my_fax ? (($phone ? '&nbsp; * &nbsp;' : '') . 'Fax: ' . u\formatPhone($my_fax, '+')) : '';
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = r\dbQ($sql, compact('uid'))->fetchAllKeyed();
  $cats = empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats));

  $markup = <<<EOF
  $pic
  <div id='member-details'>
    <div id='member-contact'>
    <h2>$my_full_name</h2>
    $my_address<br>
    $my_city, $my_state $my_postal_code $country<br><br>
    $phone$fax<br>
    $website<br><br>
    </div>
    <div id='member-description'>$my_description</div>
    <div id='member-categories'>$cats</div>
  </div>
EOF;

  $form = array(
    'all' => formField('item', $markup),
  );
  return $form;
}

/**
 * Return specified account's profile picture, with or without markup
 * @return FALSE if none
 */
function profile_picture($uid, $with_markup = FALSE) {
  global $base_url;
  $acct = r\acct($uid);
  $data = @unserialize($acct->data);
  if ($fid = r\acct($uid)->picture) { 
    if (!($url = r\dbLookup('filename', 'file_managed', 'fid=:fid AND uid=:uid', compact('fid', 'uid')))) return FALSE;
    $url = "$base_url/sites/default/files/pictures/$url";
  } else $url = @$data['picture']; // temporary fudge for Greenfield pictures
  if (!$with_markup or !$url) return $url;

  $info = @getimagesize($url);
  list ($width, $height) = is_array($info) ? $info : array(1, 0);
  $new_width = min(max($width, R_MIN_PHOTO_WIDTH), R_MAX_PHOTO_WIDTH);
  $new_height = round($new_width * $height / $width, 0);
  $subs = compact(u\ray('new_width new_height url'));
  return strtr('<div class="user-picture"><img src="url" width="new_width" height="new_height" alt="profile picture" /></div>', $subs);
}

function formSystem($form, &$sta, $args = '') {
//  $form = u\insertBefore(compact('communities'), formCommunity(array(), &$sta, $args), 'count');

/*  $rows = r\dbQ('SELECT uid, full_name FROM users WHERE uid<0 AND uid<=:R_FIRST_CTTY_UID ORDER BY full_name')->fetchAllKeyed();
  $cttyChoices = array_merge(array('ctty-0' => 'Select a community to manage'), u\prefixKeys('ctty', $rows));
  $community = formField('select', t('Manage:'), t('Select a community to manage'), '', $cttyChoices);
*/
  $title = item(t('System Administration'));
  $submit = submit();
  $form = compact(u\ray('title community submit'));
  return labeled($form);
}

function formSystem_submit($form, &$sta) {
  extract(u\just('', $sta['values']));
//  list ($community) = explode('-', $community, 2); // get the absolute value of community
//  if ($community) svar('cuid', -$community); // make it the new current account
}

function formCommunity($form, &$sta, $args = '') {
  $cacct = r\acct();
  $cuid = $cacct->id;
  $count = r\userfield('COUNT(uid)', 'community_uid=:cuid', compact('cuid'));

  $title = item(t('Community Overview'));
  $subtext = item($cacct->full_name);
  $count = formField('item', t('Members:'), '', dft(0));
  $who = formField('textfield', t('Who'), t('Type a name, id, email, or phone.'), autocomplete(''));
  $submit = submit();
  $form = compact(u\ray('title $subtext who submit'));
  return labeled($form);
}

function formCommunity_validate($form, &$sta) {
  extract(u\just('who', $sta['values']));
  if (!($acct = whois($who, 'who', $sta['values']))) return FALSE;
  $sta['values']['who'] = $acct;
}

function formCommunity_submit($form, &$sta) {
  extract(u\just('who', $sta['values']));
  if ($who and $who->community_uid == r\agent()->community_uid) svar('cuid', $who->id); // make it the new current account
  r\go('');
}

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 */
function formTx($form, &$sta, $args = '') {
  if ($confirm = sureForm($sta)) return $confirm;
  
  extract(u\just('who amount goods purpose scanned', $args));
  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path()));
  $role = in_array($type, u\ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, u\ray('fine grant loan'));
  focusOn(@$who ? 'amount' : 'who');

  //$onchange = "nixwhat();";
  $radios = array(t('cash/loan/etc.'), t('goods & services'));
  
  // set up form fields
  $title = item(ucwords($type));
  $whoLabel = ($type == 'loan' ? 'Lend to' : ucwords($type)) . ' whom:';

  $onchange = $incentive ? '' : "if(this.value.toUpperCase() == 'ALL') jQuery('#edit-purpose').val('payroll');";
  $who = formField('textfield', $whoLabel, t('Type a name, id, email, or phone'), dft(@$who) + autocomplete($role) + attrib(compact('onchange')));

  $amount = formField('textfield', 'Amount: $', t('Amount to ') . $type, dft(@$amount));
  $goods = $incentive ? formField('hidden', FALSE) : formField('radios', '&nbsp;', '', dft(u\retSet(@$goods) ?: 1), $radios); //  + attrib(compact('onchange'))
  $purpose = formField('textfield', 'Purpose:', t('Description of goods and services or purpose'), dft(@$purpose));
  $scanned = formField('hidden', @$scanned); // scanned means user already saw a name and photo
  $$type = formField('submit', ucwords($type));

  $form = compact(u\ray("title who amount goods purpose $type scanned"));
  $form['#attributes']['class'] = array('labeled');
  
  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  if (confirming_v($sta)) return;
  if ($confirm = transfer($sta['values'], FALSE)) confirm($confirm, $sta); // op who amount goods what
}

function formTx_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  transfer($sta['values'], TRUE); // op who amount goods what
  $sta['redirect'] = current_path(); // clean out the fields
//  if (!@TESTING == 1) r\go(current_path());
}

/**
 * Undo the last transaction.
 */
function undo($confirmed = FALSE) {
  if (!($zxid = svar('lastZxid'))) r\go('');
  if ($confirmed) {
    list ($message, $args) = be\undoTx($zxid);
    say($message, $args);
    if (@$args['success'] and svar('lastZxid') == $zxid) svar('lastZxid', FALSE);
  } elseif ($confirmed === '0') {
    say('nothing undone');
  } else {
    $args = be\lastTx('zxid=:zxid', compact('zxid'));
    $args['yesNo'] = yesNo('undo/1', 'undo/0');
    if ($args) say('confirm undo|yes or no', r\txReportArgs($args));
  }
  r\go('empty');
}

/**
 * Show remaining customer balance.
 */
function remainder() {
  $acct = svar('lastCustomer');
  if ($acct) {
    $otherName = $acct->full_name;
    $uid = $acct->id;
    $balance = be\creditInfo(compact('uid'))->fancy['balance'];
    say('customer balance', compact('otherName', 'balance'));
  } else say('no last customer', 'zot');
  r\go('empty');
}

function formEmpty($form, &$sta) {
  return array('title' => item('this'));
}

/**
 * Pay with rCredits, arriving from some other site.
 * @todo: handle <user logged in> better
 */
function formBuy($form, &$sta) {
  $in = @$sta['input'] ?: (svar('external_input') ?: $_GET); // $_GET is always an array here
  extract($in, EXTR_PREFIX_ALL, 'my'); // get input from outside site or (stored) from login
  if (@$my_external_input) extract(unserialize($my_external_input), EXTR_PREFIX_ALL, 'my');
  $confirmed = @$my_confirmed ?: (!@$sta['input'] and user_is_logged_in());
  
  if (!$confirmed) {
    foreach (u\ray('amount company item code') as $field) {
      $my_field = "my_$field";
      if (!@$$my_field) return say('missing field', compact('field'), $field); // handle hackers gracefully
    }
    if (!is_numeric($my_amount)) return say('bad amount', 'amount');
    if (!u\isQid(@$my_company) or !($acct = r\acct($my_company))) return say('bad account id', 'company'); // qid?
    
    $company_name = $acct->full_name; 

    $verbs = u\ray('Contribute, Donate, Pay');
    $verb = in_array(ucwords(@$my_verb), $verbs) ? ucwords($my_verb) : 'Pay';

    $button = user_is_logged_in() ? formField('submit', $verb)
      : item(t('By typing your username and password, you agree to pay with rCredits, as detailed above.'));

    $my_amount = @number_format($my_amount, 2);
    $disabled = array('disabled' => 1);
    $verblower = strtolower($verb);
  }
  
  list ($op, $amount, $goods) = array('Pay', $my_amount, TRUE);
  list ($who, $what) = @$my_confirmed ? array($my_who, $my_what) : array($my_company, $my_item);

  if (@$confirmed) {
    transfer(compact(u\ray('op who amount goods what')), TRUE);
    if (u\valid_url(@$my_return_to)) return array(item("<a href='$my_return_to'>Click here</a> to return to the $company_name website."));
    r\go('');
  }

  $form = array(
    'title' => item(t('Confirm Payment')),
    'xwho' => formField('textfield', 'to:', t('Pay whom?'), disabled($company_name)),
    'who' => formField('hidden', $who),
    'xamount' => formField('textfield', '$', t("Amount to $verblower"), disabled($amount)),
    'amount' => formField('hidden', $amount),
    'xwhat' => formField('textfield', 'for:', t('Description of goods and services'), disabled($what)),
    'what' => formField('hidden', $what),
    'goods' => formField('hidden', $goods),
    'return_to' => formField('hidden', @$my_return_to),
    'confirmed' => formField('hidden', TRUE),
    'submit' => $button,
  );

  return labeled($form);
}

function formBuy_submit($form, &$sta) {
//  formTx_submit($form, $sta);
//  drupal_goto('charge'); // member page when done
}

/**
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 */
function formExchange($form, &$sta) {
  $cuid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $request = u\formatAmount(r\userField('demand', $cuid));
  $demand = be\creditInfo()->fancy['totalDemand'];

  $form = array(
    'title' => item(t('Get rCredits / USD')),
    'subtext' => item(tt('request & demand', compact(u\ray('request demand')))),
    'amount' => formField('textfield', t('Amount:'), t('Amount to exchange (r$ or us$)'), required()),
//    'way' => formField('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
//    'who' => formField('textfield', 'to/from:', t('Exchange with whom?')),
    'getr' => formField('submit', t('Get rCredits')),
    'getusd' => formField('submit', t('Get USD')),
  );

  return labeled($form);
}

function formExchange_validate($form, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return;

  extract($sta['values']);
//  if (blank_field(compact('amount'))) return;
  list ($message, $args, $confirm) = be\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cuid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $sta);
  if (!@$args['success']) say($message, $args, 'amount');
}

function formExchange_submit($form, &$sta) {
  $cacct = r\acct();
  if (confirming_s($sta)) return;

  extract($sta['values']);
  list ($message, $args, $confirm) = be\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cacct->id, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($op == 'Get rCredits' and $amount != 0) {
    if ($amount < R_MIN_DEMAND) say('demand too small');
    if (!$cacct->can(PERM_PARTNER)) say('demand not yet');
  }
}

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  be\mergeCheck(compact(u\ray('name website phone fax'))); // check before and after
  $cacct = r\acct();
  extract((array) $cacct->account(), EXTR_PREFIX_ALL, 'my');
  $full_name_desc = t('Your full legal name, properly capitalized');
  if ($cacct->can(PERM_COMPANY)) $full_name_desc .= '<br>' . t('(If you change this, your short identifier and rCredits webpage URL will also change.');
  
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form = array(
    'title' => item(t('Contact Information')),
//    'subtext' => basic_account_info(),
    'full_name' => formField('textfield', t('Full name:'), $full_name_desc, dft($my_full_name) + required()),
    'phone' => formField('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\formatPhone($my_phone): '') + required()),
    'fax' => formField('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => formField('textfield', t('Website:'), t(''), dft($my_website)),
    'country' => formField('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))), // can't use required() here
    'postal_code' => formField('textfield', t('Postal code:'), t(''), dft($my_postal_code)+ required()),
    'state' => formField('select', t('State:'), t(''), dft($my_state)), // can't use required() here
    'city' => formField('textfield', t('City:'), t(''), dft($my_city) + required()),
    'address' => formField('textfield', t('Address:'), t('Mailing address'), dft($my_address) + required()),
    'submit' => submit(),
  );

  $form['#attributes']['class'] = array('labeled');
  js('inc/countries.js', 'file', 'header');
  js("print_country('edit-country', \"$my_country\", \"$my_state\");", 'inline', 'footer');
  focusOn('full_name');

  return $form;
}

function formContact_validate($form, &$sta) {
  $cuid = r\acct()->id;
  extract(u\just('phone fax city address full_name country state', u\trimAll($sta['values']))); // trim ALL values, for later
//  if (blank_field(compact(u\ray('phone city address')))) return;

  // add dropdowns, just in case there is an error
  js('inc/countries.js', 'file', 'header');
  js("print_country('edit-country', \"$country\", \"$state\");", 'inline', 'footer');

  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['values']['phone'] = $phone;
  if ($fax and !($fax = u\formatPhone($fax, '+n'))) say('bad phone', 'fax'); else $sta['values']['fax'] = $fax;
  if ($full_name != r\acct()->full_name) {
    $shortName = u\shortName($full_name);
    $otherName = r\userField('full_name', 'name=:shortName AND uid<>:cuid', compact('shortName', 'cuid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'full_name');
    $sta['values']['name'] = $shortName;
  }
}

function formContact_submit($form, &$sta) {
  extract(u\just('name website phone fax', $info = $sta['values']));
  $info['website'] = str_replace('http://', '', strtolower(@$website));
  r\acct()->update($info);
  be\mergeCheck(compact(u\ray('name website phone fax'))); // check before and after
  say('info saved');
}

/**
 * Draft an individual or company into the database.
 * Called when a member tries to pay or charge a non-member.
 */
function formDraft($form, &$sta, $args = '') {
  extract(u\just('who return', $args));
  $full_name_desc = t('Their full legal name, properly capitalized');
  unset($args['who']); unset($args['return']); // be neat
  if (!($phone = u\formatPhone($who))) $full_name = $who;
  
  $form = array(
    'title' => item(t('Draft a Potential New rCredits Member')),
    'subtext' => item(t('Complete both fields and click on "Submit" OR click on any tab to cancel.')),
//    'subtext' => basic_account_info(),
    'full_name' => formField('textfield', t('Full name:'), $full_name_desc, dft(@$full_name) + required()),
    'phone' => formField('textfield', t('Phone:'), t('Their primary phone (landline or cell)'), dft($phone) + required()),
    'argString' => formField('hidden', http_build_query($args)),
    'return' => formField('hidden', $return),
    'submit' => submit(),
    'cancel' => formField('submit', t('Cancel')),
  );

  return labeled($form);
}

function formDraft_validate($form, &$sta) {
  global $base_url;
  extract(u\just('op full_name phone argString return', u\trimAll($sta['values']))); // (trim ALL values for later)
  if (op($sta) == 'cancel') return r\go("$return/$argString");
  if ($phone and !($phone = u\formatPhone($phone, '+n'))) say('bad phone', 'phone'); else $sta['values']['phone'] = $phone;
  if (!u\validName($full_name, FALSE, FALSE)) say('bad name', 'name');
  $sta['values']['full_name'] = u\normalizeCase($full_name);
  $shortName = u\shortName($full_name);
  if (($otherName = r\userField('full_name', 'name=:shortName or phone=:phone', compact('shortName', 'phone')))) {
    $draftLink = "$base_url/$return/who=$otherName&$argString";
    say('similar found', compact(u\ray('otherName phone draftLink')), u\shortName($otherName) == $shortName ? 'full_name' : 'phone');
  } else $sta['values']['name'] = $shortName;
}

function formDraft_submit($form, &$sta) {
  extract(u\just('argString return name', $info = $sta['values']));
  $acct = new r\acct($info);
  say('info saved|ok to continue');
  $return = str_replace('%_%', '/', $return); // undo original Drupal-anomaly workaround
  $sta['redirect'] = "$return/who=$name&$argString";
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}


/**
 * Get the Dwolla account info
 * @todo: add subtle link to disconnect from Dwolla
 */
function formUsd($form, &$sta) {
  $cacct = r\acct();
  $statementRadios = array(
    t('I will accept electronic statements.'), 
    u\getConstants(t('I choose to receive printed statements by mail instead and I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.'))
  );
  $form = array(
    'title' => item(t('US Dollar Account (Dwolla) Info')),
//    'subtext' => ,
    'usd_account' => formField('textfield', t('Account:'), t('What is your 10-digit Dwolla account number?'), dft($cacct->usd_account) + required()),
    'statements' => formField('radios', t('Statements:'), '', dft($cacct->hasBit(BIT_ELECTRONIC)), $statementRadios),
    'submit' => submit(),
  );

  return labeled($form);
}

function formUsd_validate($form, &$sta) {
  extract(u\just('usd_account', $sta['values']));
  $usd_account = $sta['values']['usd_account'] = str_replace('-', '', trim($usd_account));
  if (!preg_match('/^[0-9]{10}$/', $usd_account)) say('bad account number', 'bank_account_number');
}

function formUsd_submit($form, &$sta) {
  extract(u\just('usd_account electronic', $sta['values']));
  //DWOLLA: get open auth code
  // $usd_auth = whatever;
  r\acct()->update(compact(u\ray('usd_account usd_auth')));
  r\acct()->setBit(BIT_ELECTRONIC, $electronic);
  say('info saved');
}

/**
 * Get the bank account info
 * @todo: store two verification EFT deposit amounts in bank_account_status as 1000*one+two
 * (if status > 100, it's waiting for verification)
 */
 /*
function formBank($form, &$sta) {
  extract((array) r\acct()->account(), EXTR_PREFIX_ALL, 'my');

  if (!$my_bank_account_name) $my_bank_account_name = $my_full_name;
  
  $form = array();
  if ($my_country == R_COUNTRY) {
    $my_routing_number = substr($my_bank_account_number, 4, 9); // chop of USkk
    $my_bank_account_number = substr($my_bank_account_number, 4 + 9); // everything after the routing number
    $routing_number = formField('textfield', t('Routing number:'), t('What is the 9-digit routing number (type carefully)'), dft($my_routing_number) + required());
  } else $routing_number = NULL;
  
  $form = array(
    'title' => item(t('Bank Information')),
//    'subtext' => ,
    'bank_account_name' => formField('textfield', t('Account name:'), t('What is the name on this bank account?'), dft($my_bank_account_name) + required()),
    'routing_number' => $routing_number,
    'bank_account_number' => formField('textfield', t('Account number:'), t('What is the account number? (be sure to get this right)'), dft($my_bank_account_number) + required()),
    'submit' => submit(),
  );

  return labeled($form);
}

function formBank_validate($form, &$sta) {
  extract(u\trimAll($sta['values']));
  if (!u\validName($bank_account_name, FALSE, FALSE)) say('bad name', 'bank_account_name');
  if (!preg_match('/^(|[0-9]{9})$/', $routing_number)) say('bad routing number', 'routing_number');
  if (!preg_match('/^[0-9]{3,20}$/', $bank_account_number)) say('bad account number', 'bank_account_number');
}

function formBank_submit($form, &$sta) {
  extract($sta['values']);
  if (r\acct()->country == R_COUNTRY) $bank_account_number = "USkk$routing_number$bank_account_number";
  r\acct()->update(compact(u\ray('bank_account_name bank_account_number')));
  say('info saved');
}
*/

/**
 * Give someone access to the current account or mark them as an employee or owner.
 * This function uses jQuery code defined in misc.js
 */
function formRelations($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $cuid = $cacct->id;
  $isPerson = !$cacct->isCompany();
  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $account_identifiers = be\accountName($cuid);
  $header = u\ray('Person,Is employer?,Is employee?,Is owner?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person employer employee owner permission link'));
  if (!$isPerson) {
    unset($header[1]);
    unset($classes[1]);
  }
  
  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main_uid, s.agent_uid, IF(:cuid=s.main_uid, v.full_name, u.full_name) AS full_name,
      s.permission, s.employer_ok, s.employee_ok, s.is_owner
    FROM (r_relations s RIGHT JOIN users u ON u.uid = s.main_uid)
      RIGHT JOIN users v ON v.uid = s.agent_uid
    WHERE s.main_uid=:cuid OR (s.agent_uid=:cuid AND (s.employee_ok OR s.employer_ok))
    ORDER BY full_name, IF(:cuid=s.main_uid, s.agent_uid, s.main_uid), (:cuid=s.agent_uid)
EOF;
  $result = r\dbQ($sql, compact('cuid'));
  $rows = $raws0 = $raws = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original data from table
    $isAgent = ($cuid != $main_uid);
    $combo = ($isAgent and @$previousAgent == $main_uid); // related both ways
    $otherIsPerson = !r\acct($isAgent ? $main_uid : $agent_uid)->isCompany();
    $idLink = (!$isAgent and $otherIsPerson and $employee_ok and $permission >= PERM_SELL) ? "<a target=\"_blank\" href=\"$base_url/Advance-ID-Card/$agent_uid\">print&nbsp;ID&nbsp;Card</a>" : R_NONE;

    $id = (count($rows) - ($combo ? 1 : 0)) . "-$reid"; // "row-reid"
    $discrepant = ($employer_ok xor $employee_ok); // exactly one has claimed the relation
    list ($employee_ok, $employer_ok) = array($employee_ok and $isAgent, $employer_ok and !$isAgent);
    $isEmployer = toggle('employee_ok', $employee_ok, $id, $isAgent and $discrepant);
    $isEmployee = $otherIsPerson ? toggle('employer_ok', $employer_ok, $id, !$isAgent and $discrepant) : R_NONE;
    $is_owner = ($is_owner and !$isAgent);
    $isOwner = toggle('is_owner', $is_owner, $id);
    $permission = $isAgent ? 0 : $permission;
    $perm = dropdown('permission', $permission, $GLOBALS['share permissions'], $id);

    if ($combo) { // employee record preceded by other record for same entity
      $thisRow = array_pop($rows); // throw out everything (everything but $isEmployer), use previous record data
      if ($isPerson) $thisRow[1] = $isEmployer;
      $thisRaw = array_pop($raws);
      $thisRaw['employee_ok'] = $employee_ok;
    } else {
      $thisRow = array($full_name, $isEmployer, $isEmployee, $isOwner, $perm, $idLink);
      $thisRaw = compact(u\ray('employee_ok employer_ok permission is_owner'));
      if (!$isPerson) array_splice($thisRow, 1, 1); // companies can't be employees
    }
    
    $rows[] = $thisRow;
    $raws[] = $thisRaw;
    $previousAgent = $agent_uid;
  }
  $sta['original_settings'] = compact('raws0', 'raws');
  
  $help = t("Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account.");
  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$account_identifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no relations');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => item(t('Relations')),
    'subtext' => item($subtext),
    'new_person' => formField('textfield', t('Add Someone:'), t("name, id, email, or phone"), autocomplete('')),
    'list' => formField('item', '', '', array('markup' => $list)),
//    'new_permission' => formField('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'go' => formField('submit', t('Save Changes')),
  );

  return $form;
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  $zot = array("$name-$id" => formField('select', '', '', compact('value') + attrib(compact('name')), $options));
  return render($zot);
}

function toggle($field, $value, $id, $highlight = 'never') {
  $path = current_path();
  $name = $id = "$field-$id";
  $zot = array($id => formField('hidden', '', '', attrib(compact(u\ray('id name value')))));
  $input = \render($zot);
  $b1 = button(' - ', "toggle('$id');", t('Change to NO'), 'form-no');
  $b2 = button(' + ', "toggle('$id');", t('Change to YES'), 'form-yes');

  if ('never' != (string) $highlight) {
    list ($visHigh, $invisHigh) = u\order($highlight, ' highlight', '');
  } else $highlight = $visHigh = $invisHigh = '';

  list ($yesvis, $novis) = u\order($value, "visible$visHigh", "invisible$invisHigh");
  return <<<EOF
    <div class="$field-buttons$highlight">
    <div id="$id-YES" class="$yesvis form-yes">Yes $b1</div>
    <div id="$id-NO" class="$novis form-no">No &nbsp;$b2</div>
    $input
    </div>
EOF;
}

function formRelations_validate($form, &$sta) {
  if (confirming_v($sta)) return;
  $cuid = r\acct()->id;
  extract(u\just('new_person', $sta['values']));
  
  if (@$new_person) {
    $acct = whois($new_person, 'new_person', $sta['values'], FALSE, 'no self-relation');
//    if ($acct->can(PERM_COMPANY)) return say('no company relations', 'new_person');
    if (r\relation(1, $cuid, $acct->id)) return say('already related', 'new_person');
    $sta['values']['agent_uid'] = $acct->id; // save for submission handling
  }
}

function formRelations_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  $cuid = r\acct()->id;

  if ($agent_uid = @$sta['values']['agent_uid']) { // adding a new relation
    $main_uid = $cuid;
    $permission = 0; // default to no permissions
    $info = compact(u\ray('main_uid agent_uid permission'));
    $zot = \drupal_write_record('r_relations', $info);
    $who = r\acct($agent_uid)->full_name;
    say('report new relation', compact('who'));
  }

  $changes = array();
  extract(u\just('raws0 raws', $sta['original_settings']));
//  debug(compact('raws0','raws'));
  
  // Drupal doesn't pass along our hidden inputs, so get them directly from $_POST ...
  foreach ($_POST as $key => $value) { // ... be\updateRelations will have to check permissions and data carefully
    if (!strpos($key, '-')) continue;
    list ($key, $rawnum, $reid) = explode('-', $key); // eg split permission-2-37
    if (!in_array($key, array_keys($raws))) return 'bad field in relations'; // hack attempt?

    $value = (int) strtr($value, array('true' => 1, 'false' => 0));
    if ($value != (int) $raws[$rawnum][$key]) { // changed?
//      debug("key=$key reid=$reid rawnum=$rawnum value=$value orig=".$raws[$rawnum][$key]);
      $changes[$reid][$key] = $value;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
//  if (!empty($changes)) \drupal_set_message(be\updateRelations($changes));
  if (!empty($changes)) say(be\updateRelations($changes));
}

/**
 * @todo: focus on code input
 */
function formCells($form, &$sta) {
  $cuid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only deletion
  if (form_step($sta, $info) == 'prove') {
    $number = u\formatPhone($sta['storage']['values']['number']);
    $form = array(
      'title' => item(t('Verify')),
      'subtext' => item(tt('verify cell', compact('number'))),
      'code' => formField('textfield', t('Code:')),
      'number' => formField('hidden', $number),
      'verify' => formField('submit', t('Verify')),
    );
    return $form;
  }

  $headers = array('Cell Number');

  $sql = "SELECT number FROM r_sms WHERE uid=:cuid ORDER BY number";
  $result = r\dbQ($sql, compact('cuid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(u\formatPhone($number));
  }

  $form = array(
    'title' => item(t('Cell Phones')),
    'subtitle' => item(t('Authorized devices for this account')),
    'cells' => formField('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no cells')), $list),
    'delete' => formField('submit', t('Remove Selected Numbers')),

    'number' => formField('textfield', t('Add another cell phone:')),
    'add' => formField('submit', t('Add')),
  );
  return $form;
}

function formCells_validate($form, &$sta) {
  $cuid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract($sta['values']);

  if ($op == 'Verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op != 'Add') { // $op == remove
    if (no_selection($cells)) {
      if ($number) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $sta);
  }

  if ($op == 'Add') {
    if (blank_field(compact('number'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($number, 'VALIDATE'), 'number');
  }
}

function formCells_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract($sta['values']);
  if ($op == 'Add') {
    if ($info = previous_state($sta)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($number, tt('verification code', compact('nonce')));
    if (isDEV) debug("nonce is $nonce"); // tell developer what code to type
    return form_step($sta, compact('number'), 'prove');
  }

  if ($op == 'Verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($number));
    $sta['redirect'] = 'account/cells';
  }
    
  if ($op == 'Remove') {
    foreach ($cells as $key => $number) if ($number) say(be\deleteCell(substr($number, 1))); // ignore the leading 'x'
  }
}

/**
 * Modify the user registration form.
 */
function formRegister(&$form, &$sta) {
  unset($form['account']['name']);
  $account_types = u\ray('Personal,Commercial,Public Non-profit/Government');
  $account_type_help = t('NOTE: If you choose "@commercial" or "@nonprofit", your contact information and photo will be public.', array('@commercial' => $account_types[1], '@nonprofit' => $account_types[2]));
  $my_country = $my_state = '';
  extract(u\just('country state', $sta['input']), EXTR_PREFIX_ALL, 'my'); // on input error, sets to intents
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $country_attrib = attrib(compact('onchange'));
  $company_options = array('employee_ok' => t('employee'), 'is_owner' => t('owner'));
  $onchange = "var info = jQuery('#company_info'); if(jQuery('#edit-account-type').val() == 0) info.show(); else info.hide();";
  $account_type_attrib = attrib(compact('onchange'));
  
  $company_info = array(
    'company_text' => item(t('Are you an owner and/or employee of a local company (or of any participating rCredits company)? Otherwise leave it blank.')),
    'company' => formField('textfield', t('Company: '), t(''), autocomplete('company')),
    'company_phone' => formField('textfield', t('<small>Company</small> Phone:'), t('What is the company\'s primary phone number?')),
    'company_options' => formField('checkboxes', '', '', '', $company_options),
  );
  
  $form += array(
    'title' => formField('item', t('Sign up for rCredits'),'', weight(-99)),
    'full_name' => formField('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), required() + weight(-98)),
/*    'phone' => formField('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\formatPhone($my_phone): '')),
    'fax' => formField('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => formField('textfield', t('Website:'), t(''), dft($my_website)), */
    'country' => formField('select', t('Country:'), t(''), dft($my_country) + $country_attrib),
    'postal_code' => formField('textfield', t('Postal code:'), t(''), required()),
    'state' => formField('select', t('State:'), t(''), dft($my_state)),
    'city' => formField('textfield', t('City:'), t(''), required()),
    'account_type' => formField('select', t('Account type:'), $account_type_help, dft('Personal') + $account_type_attrib, $account_types),  
    'company_info' => formField('fieldset', '', '', array('id' => 'company_info')) + $company_info,

/*    'address' => formField('textfield', t('Address:'), t('Mailing address'), dft($my_address)),
    'submit' => submit(), */
  );

  css('rweb.css', -9); // needed because this function doesn't go through show()
  js('inc/countries.js', 'file', 'header');
  js("print_country(\"edit-country\", \"$my_country\", \"$my_state\");", 'inline', 'footer');

  $form['#attributes']['class'][] = 'labeled';
  $form['#attributes']['class'][] = 'rweb';
  $form['#validate'][] = 'rCredits\\Web\\formRegister_validate';
  $form['#submit'][] = 'rCredits\\Web\\formRegister_submit';
  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost
  focusOn('full_name');
  keep_values($form);
  t\snapShot($form);
}

function formRegister_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = array(); // for testing (keep Drupal from griping)
  extract(u\just('company company_phone company_options full_name postal_code', $values = u\trimAll($sta['values'])));
  $sta['values'] = array(); // make sure Drupal doesn't save the record WE find an error
  
  if (@$company) {
    if (!($companyPhone = u\formatPhone($company_phone, '+n'))) return say('bad company phone', compact('companyPhone'), 'company_phone');
    $sta['company_phone'] = $companyPhone;
    if (@$company_options) extract(u\just('employee_ok is_owner', $company_options));
    if (!(@$employee_ok or @$is_owner)) return say('what relation', 'company');
    $company = $company;
    $data = compact(u\ray('company employee_ok is_owner'));
  }
  
  if ($error = user_validate_name($full_name)) return say(str_replace('The username', 'That name', $error), 'full_name');
  $name = $name0 = r\uniqueName($full_name);
//  $name = $name0 = u\shortName($full_name);
//  if (r\userField(1, 'name=:name', compact('name'))) return say('duplicate name'); // require unique full_name

  $params = u\ray('full_name city address'); // prepare for pass-by-reference
  list ($full_name, $city, $address) = u\normalizeCase($params);
  $community_uid = r\communityUid($postal_code);
  $flags = BIT_DEFAULTS;
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  $sta['values'] = array_merge($values, compact(u\ray('name data community_uid flags administer_users status')));
}

function formRegister_submit($form, &$sta) {
  extract(u\just('company company_options company_phone uid name', $sta['values']));
  if (FALSE) {
    $cacct = new r\acct($sta['values']); // create test record
    $uid = $cacct->id;
  } else u\EXPECT(@$uid > 0, 'registration failure');
  setupGlobals($uid); // some rcredits functions need to know who we are

  if (@$company) {
    extract(u\just('is_owner employee_ok', $company_options));
    // (can't use whois to create record because user is logged out) $companyAcct = whois($company, 'company', $sta['values']);
    $companyShortName = u\shortName($company);
    $companyUid = r\dbLookup('uid', 'users', 'phone=:company_phone', compact('company_phone'));
    if (!$companyUid) $companyUid = r\dbLookup('uid', 'users', 'name=:companyShortName', compact('companyShortName'));
    if (!$companyUid) { // company account doesn't exist, so create it
      $companyAcct = new r\acct(array('full_name' => $company, 'phone' => $company_phone, 'account_type' => R_COMMERCIAL));
      $companyUid = $companyAcct->id;
    }
    if (!r\relation(1, $companyUid, $uid)) {
      list ($main_uid, $agent_uid, $is_owner) = array($companyUid, $uid, $is_owner);
      $info = compact(u\ray('main_uid agent_uid employee_ok is_owner'));
      \drupal_write_record('r_relations', $info);
    }
  }

  // merge fails because account is not saved yet:  be\mergeCheck(compact(u\ray('email short_name')));
  $quid = r\quid($uid, $uid);
  $balance = u\formatAmount(be\fund($uid, TX_SIGNUP));
  if (@$_SESSION['messages']['status'][0] == t('A welcome message with further instructions has been sent to your e-mail address.')) unset($_SESSION['messages']['status']);
  say('your account is ready', compact(u\ray('balance quid')));
}

/**
 * Modify the password reset form.
 */
function resetAlter(&$form, &$sta) {
  $form['title'] = formField('item', t('First Time Login'), '', weight(-99));
  $form['message']['#markup'] = str_replace('</p><p>' . t('Click on this button to log in to the site and change your password.'), t(' Choose a new password, then click on the button to log in.'), $form['message']['#markup']);
  $form['account']['pass'] = formField('password_confirm', '', t('Enter your new password in both fields.'));
  $form['actions']['submit'] = formField('submit', t('Login')); // + weight(99));
  $form['#submit'][] = 'rCredits\\Web\\resetSubmit';
  $form['#attributes']['class'][] = 'rweb';
  focusOn('pass-pass1'); // note that password field gets expanded to two fields, of which pass1 is the first
  t\snapShot($form);
}

function resetSubmit(&$form, &$sta) {
  svar('resetPass', $sta['values']['pass']);
//  $uid = $sta['buid_info']['args'][0];
//  r\acct($uid)->update(compact('pass'));
//  debug($form);
//  debug($sta);
}

/**
 * Modify the login form.
 * @see also loginFollowup, below (called from rweb_user_login()).
 * @param bool $regular: is this the full-page login form (default TRUE) -- otherwise the narrower login block
 */
function loginAlter(&$form, &$sta, $regular = TRUE) {
  if ($regular) {
    $title = item(t('rCredits Login'));
    $subtext = item(t('Welcome! If you have not yet signed up for rCredits, click on "New account" above.'));
    array_unshift($form, compact(u\ray('title subtext')));
    $form['tip'] = item(t('<b>NOTE: </b>If you created an account using cell texting request a "New password".'));
    $form['name']['#description'] = t('Enter your username (the letters of your legal name, all lowercase) OR your rCredits account ID.');
    $form['pass']['#description'] = t('Enter your account password.');
  }
  
  $form['external_input'] = formField('hidden', serialize(@$sta['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $form['#validate'] = array('rCredits\\Web\\login_extra_validate', 'user_login_final_validate'); // add replacement function
  
  $form['name']['#title'] = t('Username/ID:');
  $form['pass']['#title'] = t('Password:');
  
  $form['#attributes']['class'][] = $regular ? 'rweb labeled' : 'rweb';
  focusOn('name');
  css('rweb.css', -9); // needed because this function doesn't go through show()
  t\snapShot($form);
  loginCheck(); // must follow snapShot()
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account id, with password.
 * Sets $sta['uid'] to signify successful authentication.
 */
function login_extra_validate($form, &$sta) {
  extract(u\just('name pass', $sta['values']));
  if (!$name or empty($pass)) return;

  $failed_login_ip_limit = variable_get('user_failed_login_ip_limit', 50);
  $failed_login_ip_window = variable_get('user_failed_login_ip_window', 3600);
  if (!flood_is_allowed('failed_login_attempt_ip', $failed_login_ip_limit, $failed_login_ip_window)) {
    $sta['flood_control_triggered'] = 'ip';
    return;
  }
  if (!($uid = r\loginString($name))) return; // not a user
  
  $failed_login_identifier_uid_only = variable_get('user_failed_login_identifier_uid_only', FALSE);
  $failed_login_user_limit = variable_get('user_failed_login_user_limit', 5);
  $failed_login_user_window = variable_get('user_failed_login_user_window', 21600);
  
  $identifier = $failed_login_identifier_uid_only ? $uid : $uid . '-' . ip_address();
  $sta['flood_control_user_identifier'] = $identifier;

  if (!flood_is_allowed('failed_login_attempt_user', $failed_login_user_limit, $failed_login_user_window, $identifier)) {
    $sta['flood_control_triggered'] = 'user';
    return;
  }

  $cacct = r\acct($uid);
  $sta['uid'] = $cacct->passwordOkay($pass) ? $uid : FALSE;
}

function loginFollowup($edit, $account) {
  $resetPass = svar('resetPass');

  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($my_external_input)); // save params from external online merchant

  $name = @$edit['values']['name'];
  svar('cuid', $uid = $account->uid);
  if ($uid and $resetPass) {
    require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
    $pass = \user_hash_password(trim($resetPass));
    r\acct($uid)->update(compact('pass'));
  }
  setupGlobals();
  u\log('login', compact('name'), $uid);
  /*
  if ($resetting) { // 
    $token = drupal_hash_base64(drupal_random_bytes(55));
    $_SESSION['pass_reset_' . $user->uid] = $token;
    drupal_goto('account', array('query' => array('pass-reset-token' => $token)));
  } else 
  */
  r\go(''); // go to default page unless resetting password (then let 
}

function passAlter(&$form, $sta) {
  $title = item(t('Request New Password'));
  array_unshift($form, compact('title'));
  $form['name']['#title'] = t('Account:');
  $form['name']['#description'] = t('Username or Account ID');
  $form['#validate'] = array('rCredits\\Web\\pass_validate');
  $form['#attributes']['class'] = array('rweb', 'labeled');
  focusOn('name');
  t\snapShot($form);
}

function pass_validate(&$form, &$sta) {
  if (!($uid = r\loginString($sta['values']['name']))) return say('bad login', 'name');
  form_set_value(array('#parents' => array('account')), r\acct($uid)->account(), $sta);
  say('password reset');
}

/**
 * Automatically log back in and continue testing, if testing (offline only)
 */
function loginCheck() {
  global $channel;
u\log('top of loginCheck');
  if (@TESTING == 1) return;
u\log('loginCheck not testing yet');
  if (!@cache_get('t_resume')->data) return;
u\log('loginCheck redirecting');
/*  global $user;
  $adminPass = '123';
  $account = user_authenticate('admin', $adminPass);
  $user = user_load($account, TRUE);
  drupal_session_regenerate();
  $user = (object) array(); // we need this empty for registration and login tests
  setupGlobals(1);
*/
  \drupal_goto('http://localhost/devcore/rcredits/test'); // must be drupal_goto not r\go
}

function formDirectory($form, &$sta) {
  $cuid = r\acct()->id;
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: substr(r\userField('postal_code', $cuid), 0, 3);
  
  $list = ($region !== FALSE or $which) ? directoryList($which, $region) : '';
  
  $form = array(
    'title' => item(t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => item(t('for a country name or postal code...')),
    'which' => formField('textfield', t('Search for:'), t('Type part of company name or industry category'), dft($which) + autocomplete('industry')),
    'region' => formField('textfield', t('Where:'), t('Type the first few characters of your postal code<br>Try 013, to see the list for Greenfield, MA.'), dft($region)), // or country name
    'submit' => formField('submit', t('Go')),
    'item' => formField('item', '', '', $list),
  );

  return labeled($form);
}

function formDirectory_validate($form, &$sta) {
}

function formDirectory_submit($form, &$sta) {
  extract($sta['values']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

/**
 * Return a formatted, categorized list of businesses
 */
function directoryList($which, $region) {
  global $base_url;
  list ($symbolMember, $symbolRTrader, $symbolNonMember) = array('m', 'R', '-');
  $field = is_numeric($region) ? 'postal_code' : 'country';
  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $region = \db_like($region) . '%';
  $rows = array();
  $sql = <<<EOF
    SELECT DISTINCT u.uid, IFNULL(i.industry, '~Other') AS industry, u.name AS short_name, u.full_name, u.website 
    FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE account_type!=:R_PERSONAL AND u.$field LIKE :region AND (u.full_name LIKE :which OR i.industry LIKE :which)
    ORDER BY industry, u.full_name
EOF;
  $result = r\dbQ($sql, compact(u\ray('region which')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]); // uid, industry, short_name, full_name, website
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $headed = TRUE; // always show the category (but keep the option to disable this choice)
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $cacct = r\acct();
    $acct = r\acct($uid);
    $type = $acct->pass ? (r\isRTrader($uid) ? $symbolRTrader : $symbolMember) : $symbolNonMember;
    $class = strtr($type, array($symbolRTrader => 'rtrader', $symbolMember => 'member', $symbolNonMember => 'nonmember'));
    $pay = $cacct->can(PERM_BUY) ? button('pay', "$base_url/pay/who=$short_name") : '';
    $chg = $cacct->can(PERM_SELL) ? button('chg', "$base_url/charge/who=$short_name") : '';
    $full_name = "<a href='$base_url/member/$short_name'>$full_name</a>";
    $item = "<b class=\"$class\">$type</b> $pay$chg $full_name";
    if (!$headed) $item = "- $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  $key = $list ? ('<b class="member">' . $symbolMember . '</b> = ' . t('member')
       . ' &nbsp; <b class="rtrader">' . $symbolRTrader . '</b> = ' . t('accepts real rCredits')
       . ' &nbsp; <b class="nonmember">' . $symbolNonMember . '</b> = ' . t('not yet signed up'))
       : t('There are no such businesses listed.');
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return "<ul id=\"key\">$key</ul>$list"; // ul so key gets indented like the rest
}

function formCompany($form, &$sta) {
  global $base_url;
  $cacct = r\acct();
  $cuid = $cacct->id;
  $my_description = $cacct->description;
  
  $cats = r\dbQ('SELECT iid, industry FROM r_industries ORDER BY industry')->fetchAllKeyed();
  $my_cats = r\dbQ('SELECT iid FROM r_user_industries WHERE uid=:cuid', compact('cuid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$cacct->name" target="_blank">Show my public rCredits web page</a> (in a new window)
EOF;
  
  $form = array(
    'title' => item(t('Company Information')),
    'links' => item($links),
//    'subtext' => basic_account_info(),
    'name' => formField('item', t('Company name:'), '', $cacct->full_name),
    'categories' => formField('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($my_cats), $cats),
    'description' => formField('text_format', t('Description:'), t('What does this company do?'), $filtered_html + dft($my_description)),
    'submit' => submit(),
  );

// (doesn't work well for textarea) $form['#attributes']['class'] = array('labeled');
//  if (svar('layout') != 'Mobile') $form['#attributes']['class'] = array('labeled');
  return labeled($form);
}

function formCompany_validate($form, &$sta) {
//  extract($sta['values']);
}

function formCompany_submit($form, &$sta) {
  $cuid = r\acct()->id;
  extract(u\trimAll($sta['values']));
//  $description = \check_markup($description['value']);
  $description = $description['value'];
  r\acct()->update(compact('description'));

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\dbQ('DELETE FROM r_user_industries WHERE uid=:cuid', compact('cuid')); // out with the old
  foreach ($categories as $iid) {
    r\dbQ("INSERT INTO r_user_industries (uid, iid) VALUES (:cuid, :iid)", compact('cuid', 'iid')); // in with the new
  }
  unset($dbtx);
  //*** End DBTX
  say('info saved');
}

/**
 * Show a page when someone scans an account ID QR, for example with QRDroid.
 * Cases (ideally):
 *   Using Account ID card as a business card -- show the company's rCredits webpage
 *   Curious or trying to use the app -- say a bit about rCredits and how to get the app and how to sign up / sign in.
 *   Charging / Paying with Webcam (signed in) -- show pic and name, and ask whether to charge or pay
 */
function formI($form, &$sta, $who) {
  if (!($acct = r\acct($who))) return r\go(''); // not a real code -- just go to home page

  list ($myId, $myAgent) = ($cacct = r\acct()) ? array($cacct->id, $cacct->agentId) : array(FALSE, FALSE);
  list ($otherId, $otherAgent) = array($acct->id, $acct->agentId);
  if ($otherId == $myId) {
    say('no self-trading', array(), 'error');
    return r\go('');
  }
  r\acct($acct->agentId)->setBit(BIT_USED_ID); // track membership progress
  
  $shortName = $acct->name;
  if ($acct->can(PERM_COMPANY)) $site = goButton(t('Show Webpage'), "member/$shortName", t("See this company's rCredits webpage"));

  if ($cacct) { // can charge, pay, or change account or agent only if signed in
    $theId = formField('hidden', $otherId);
    $theAgent = formField('hidden', $otherAgent);
    if ($cacct->can(PERM_SELL)) $charge = goButton(t('Charge'), "charge/who=$shortName&scanned=1", t('Charge someone'));
    if ($cacct->can(PERM_BUY)) $pay = goButton(t('Pay'), "pay/who=$shortName&scanned=1", t('Pay someone'));
    if (r\acct($myId, $otherAgent)->can()) $changeAgent = goButton(t('Change Agent'), "user/logout/who=$shortName", t('Change Shift?'));
    if (r\acct($otherId, $myAgent)->can()) $changeAccount = formField('submit', t('Change Account'));
    $zot = compact(u\ray('charge pay changeAgent changeAccount site'));
    $all = identifiers($acct) . render($zot);
    svar('lastCustomer', $acct);
  } else {
    $login = item(t('Please login, to Pay or Charge this account.'));
    $zot = compact('login', 'site');
    $all = render($zot);
  }

  $all = formField('item', '', '', $all);
  return compact('all');
}

function formI_submit($form, &$sta) {
  extract(u\just('theId theAgent', $sta['values']));
  $acct = r\acct($theId, $theAgent);
  $op = op($sta);
  if ($op == 'changeAccount') {
    $cacct = r\acct();
    if (!$cacct or !r\acct($theId, $cacct->id)->can()) return r\go('user/logout'); // hacker. so leave.
    svar('cuid', $theId);
    $accountName = $acct->full_name;
    say('changed account', compact('accountName'));
  }
}

function formSMS($form, &$sta) {
  $title = item(t('Test SMS'));
  $my_number = svar('lastSMS');
  $number = formField('textfield', t('From (number): '), '', dft($my_number));
  $message = formField('textfield', t('Message: '));
  $submit = formField('submit', t('Send it!'));

  focusOn('message');
  $form = compact(u\ray('title number message submit'));
  $form['#attributes']['class'] = array('labeled');
  return $form;  
}

function formSMS_submit($form, $sta) {
  global $sms_devel; $sms_devel = TRUE;
  extract(u\just('number message', $sta['values']));
  $number = u\formatPhone($number, '+n');
  svar('lastSMS', $number);
  \rsms_sms_incoming('process', $number, $message);
}

function formOptions($form, &$sta) {
  $cacct = r\acct();
  $title = item(t('Account Preferences'));
  $notify_bys = array(t('no notifications'), t('by email'), t('by SMS'), t('by both email and SMS'));
  $dft = $cacct->hasBit(BIT_NOTIFY_EMAIL) + 2 * $cacct->hasBit(BIT_NOTIFY_SMS);
  $notify_by = formField('radios', t('Notifications:'), t('How do you want to be notified about new charges to your account, short-term opportunities, and the like?'), dft($dft), $notify_bys);
  $explain_virtual = t('When you pay people, do you want to pay them directly in rCredits or do you want to pay them in US Dollars, then give them the automatic option to buy their share of rCredits from you with US Dollars? The second way lets you use your current payroll system without any change. You still get the 5% rebate, minus 25 cents for each payment. For example, on a $100 payment, you get a net rebate of $5.00 - 0.25 = $4.75.');
  $pay_hows = array(t('everyone directly'), t('employees virtually'), t('everyone virtually'));
  $dft = $cacct->hasBit(BIT_VIRTUAL_ALL) ? 2 : ($cacct->hasBit(BIT_VIRTUAL_EMPLOYEES) ? 1 : 0);
  $pay_how = formField('radios', t('Pay how:'), $explain_virtual, dft($dft), $pay_hows);
  $secret_bal = formField('checkbox', t('Secret Balance:'), t("Don't let qualified merchants tell me my balance, when I ask."), $cacct->hasBit(BIT_SECRET_BAL));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $submit = formField('submit', t('Update options'));
  
  $form = compact(u\ray('title pay_how notify_by secret_bal submit'));
  $form['#attributes']['class'] = array('labeled');
  return $form;  
}

function formOptions_submit($form, $sta) {
  $cacct = r\acct();
  extract(u\just('pay_how notify_by', $sta['values']));
  $secret_bal = @$sta['input']['secret_bal']; // damn Drupal sets this (=1) iff true. ['values']['secret_bal'] is always FALSE.
  $cacct->setBit(BIT_VIRTUAL_ALL, $pay_how == 2);
  $cacct->setBit(BIT_VIRTUAL_EMPLOYEES, $pay_how == 1);
  $cacct->setBit(BIT_NOTIFY_EMAIL, $notify_by & 1);
  $cacct->setBit(BIT_NOTIFY_SMS, $notify_by & 2);
  $cacct->setBit(BIT_SECRET_BAL, $secret_bal);
  say('options saved');
}

function formReports($form, &$sta) {
  $title = item(t('Reports'));
  $subtitle = item(t('After rCredits is launched, this page will report progress statistics.'));
  $form = compact(u\ray('title subtitle'));
  return $form;
}

/**
 * Print an Advance ID card
 * @todo: add "temp" in lower right corner
 */
function memberID($uid = '') {
  require_once(__DIR__ . "/../../tcpdf/config/lang/eng.php");
  require_once(__DIR__ . "/../../tcpdf/tcpdf.php");
  define('PHOTO_MAX', 40); // maximum height and width of photo

  $cacct = r\acct();
  $acct = $uid ? r\acct($cacct->id, $uid) : $cacct;
  if (!$acct) return 'No such account. Your hack attempt has been logged.';
  
  $full_name = $acct->full_name;
  if (!$acct->proSe()) {
    $agent_name = r\acct($acct->agentId)->full_name;
    $full_name = "$agent_name<br>$full_name";
    $nameSize = '36px';
  } else $nameSize = '60px';
  $qid = str_replace(':', '=', $acct->qid()); // "=" to avoid GoDaddy's 'Bad Request' death redirecting RC2.ME with a ":" qid
  if(!($photo = profile_picture($acct->agentId))) $photo = __DIR__ . '/../images/no-photo-available.gif';

  $info = @getimagesize($photo);
  list ($photoW, $photoH) = is_array($info) ? $info : array(0, 0);
  list ($photoW, $photoH) = $photoW > $photoH ? array(PHOTO_MAX, '') : array('', PHOTO_MAX);
  $pdf = new \TCPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
  $pdf->setPrintHeader(FALSE);
  $pdf->setPrintFooter(FALSE);
  $pdf->AddPage();

  $style = array( // set style for barcode
    'border' => 0,
    'vpadding' => '0',
    'hpadding' => '0',
    'fgcolor' => array(0,0,0),
    'bgcolor' => false, //array(255,255,255)
    'module_width' => 1, // width of a single module in points
    'module_height' => 1 // height of a single module in points
  );

  // Image params: $file, $x='', $y='', $w=0, $h=0, $type='', $link='', $align='', $resize=false, $dpi=300, $palign='', $ismask=false, $imgmask=false, $border=0, $fitbox=false, $hidden=false, $fitonpage=false, $alt=false, $altimgs=array())
  $pdf->Image(__DIR__ . '/../images/idcard/frame.jpg', 5, 5, 101, 64, '', '', '', true, 150, '', false, false, 0, false, false, false);
  $pdf->Image($photo, 10, 10, $photoW, $photoH, '', './account', '', true, 150, '', false, false, 0, false, false, false);
  $pdf->write2DBarcode("HTTP://RC2.ME/I/$qid", 'QRCODE,Q', 53, 25, 25, 25, $style, 'N'); // L,M,Q,H are low-high error-correction
  $pdf->Image(__DIR__ . '/../images/rlogo150.png', 85, 25, 14.93, 20, '', '', '', true, 150, '', false, false, 0, false, false, false);

  $html = strtr('<span style="font-size:22px;">Member<br><span style="font-size:38px; color:darkred;">@CODE</span><br>@REGION</span>', array('@CODE' => $qid, '@REGION' => R_REGION_NAME));
  $pdf->writeHTMLCell(50, 15, 52, 10, $html); // w, h, x, y
  $pdf->writeHTMLCell(20, 5, 85, 47, '<div style="font-size:22px;">rCredits.org</div>');
  $pdf->writeHTMLCell(91, 20, 10, 55, "<div style=\"font-size:$nameSize; text-align:center; color:midnightblue; font-weight:bold;\">$full_name</div>");
  
$instructions = <<<EOF
<style>
  big {color:darkgreen; font-size:2em;}
  p {margin-top:.5em;}
</style>
Here is your<br><b style="color:darkgreen; font-size:1.3em;">Advance ID Card</b>
<p>You may print this card and use it for "AS-IF" transactions, to complete your membership requirements. To print, right-click here, then click on "Print" OR use your browser's print icon.</p>
<p>To upload or change your photo, visit the "Settings" section.</p>
EOF;
 
  $pdf->writeHTMLCell(90, 64, 110, 5, $instructions);
  // Add temporary symbol over image (maybe remove lower right corner and add "temp"), to discourage unauthorized use
//  $pdf->Image(__DIR__ . '/../images/icons/print.png', 120, 30, 32, 32, '', '', '', true, 150, '', false, false, 0, false, false, false);
  
  $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

/**
 * Return markup to identify the agent of the given account.
 */
function identifiers($acct) {
  $cacct = r\acct();
  list ($agent, $company) = $acct->proSe() ? array($acct, '') : array(r\acct($acct->agentId), $acct->full_name);
  $location = u\location($acct);
  $pic = ($cacct and $cacct->can(PERM_CHARGE)) ? profile_picture($agent->id, TRUE) : ''; // let qualified companies see the picture
  return <<<EOF
    $pic
    <div id="id-other">
      <div id="id-name">$agent->full_name</div>
      <div id="id-company">$company</div>
      <div id="id-location">$location</div>
    </div id-other>
EOF;
}

/**
 * Set or report next step in form's workflow
 * Syntax:
 *   form_step($sta, $info, 'id of next step') -- sets the next step
 *   form_step($sta, $zot, NULL) -- sets the next step to none
 *   form_step($sta, $info) -- gets the name of the next step and (in $info) any parameters
 * @param string $next_step: a step id ('' means ignore argument, NULL means no next step)
 * @param array $info: associative array of parameters to next step (passed or returned)
 * @return string: the next step ('' if none)
 * @see also step_one() and previous_state()
 */
function form_step(&$sta, &$info = NULL, $next_step = '') {
  if (is_null($next_step)) { // set the next step (and args) to none
    $sta['storage'] = NULL;
    $sta['rebuild'] = TRUE;
  } elseif ($next_step) { // set the next step (and args)
    $sta['storage']['previous'][$next_step] = $sta;
    $sta['storage']['step'] = $next_step;
    $sta['storage']['values'] = $info;
    $sta['rebuild'] = TRUE;
  } else $info = @$sta['storage']['values']; // return args
  
  return @$sta['storage']['step']; // return the next step
}

function step_one(&$sta) {form_step($sta, $zot, NULL);}

function previous_state(&$sta, $message = '', $args = array()) {
  if ($message) {
    $sta = $sta['storage']['previous'][$sta['storage']['step']];
    $sta['storage']['say'] = array($message, $args);
  } else return @$sta['storage']['say'] ?: '';
}

/**
 * Asks the user to confirm, calls the caller with the answer.

function ok_form($form, &$sta) {
  //extract($sta['input'], EXTR_PREFIX_ALL, 'in');
  extract(form_args('ok'), EXTR_PREFIX_ALL, 'in');
  $form = array(
    'title' => item(@$in_title),
    'question' => item(@$in_question),
    'confirm' => formField('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => formField('submit', NULL, t("Don't do it"), 'Cancel'),

    'detour_to' => formField('hidden', @$in_detour_to),
    'return_with' => formField('hidden', @$in_return_with),

    '#skip_duplicate_check' => TRUE, // Confirm form fails duplication check, as the form values rarely change -- so skip it.
    '#attributes' => array('class' => u\ray('rweb confirmation')),
  );

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

function ok_form_submit($form, &$sta) {
  extract($sta['values']);
  $ok = ($op == 'Okay');
  return_from_detour($detour_to, compact('ok', 'return_with'));
}

function get_ok($question = '', $title = '', $args = FALSE) {detour('ok', compact(u\ray('question title args')));}

function got_ok() {}
*/
/*
function detour($detour_via, $args = array(), $detour_to = '') {
  if (!$detour_to) $detour_to = current_path();
  if ($detour_to != 'none') $args += compact('detour_to');
  form_args($detour_via, $args);
  drupal_goto($detour_via);
}

function return_from_detour($detour_to, $args = '') {if ($detour_to) detour($detour_to, $args, 'none');}

function form_args($function, $args = '') {
  if (!$args) return svar('form_args']['function'] == $function ? svar('form_args'] : array();
  svar('form_args'] = $args + compact('function');
}
*/

/**
 * Replacement for \confirm_form
 */
function sureForm(&$sta, $title = '') {
  if (!isset($sta['confirm'])) return FALSE;
  
  $form = array(
    'title' => $title ? item($title) : NULL,
    'question' => item($sta['confirm']),
    'confirm' => formField('submit', 'Okay'),
    'cancel' => formField('submit', 'Cancel'),
  );
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => u\ray('rweb confirmation'));
//  $form['#validate'] = 'rCredits\\Web\\sureForm_validate';
//  $form['#submit'] = 'rCredits\\Web\\sureForm_submit';
//  $sta['rebuild'] = TRUE;

  focusOn('confirm');

  return $form;
}

/*
function detour($detour_via, $detour_args, $whence = '', $modal = TRUE) {
  svar('detour_via', $detour_via);
  svar('detour_args', $args);
  svar('whence', $whence ?: current_path());
//  drupal_goto($modal ? $whence : $detour_via); // modal forms pop up over the reloaded current form
  drupal_goto($detour_via); // no modal forms yet
}
*/

/**
 * Figure out who the user means, offering choices to disambiguate
 * If the intended person or company does not exist, 
 * @param string $who: what the user typed
 * @param string $field_name: name of the field the user typed in
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param boolean $allow_ALL: all $who to be "all" (meaning all employees) (default FALSE)
 * @param string $selfMsg: index to error message to give if user self-refers
 * UNUSED @param boolean $create: whether to create the person/company if it doesn't exist
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $allow_ALL = FALSE, $selfMsg = 'no self-trading') {
  global $base_url;
  $result = be\identify($who, $allow_ALL, $selfMsg);
  if ($result == 'ALL' or u\isAcct($result)) return $result;

  list ($msg, $args, $choices) = $result;
  foreach (u\ray('form_build_id form_token form_id op') as $one) unset($info[$one]); // just for neatness
  if (u\abbreviates('who=', $last = basename($return = \current_path()))) $return = dirname($return);
  $info['return'] = str_replace('/', '%_%', $return); // even urlencoded slashes confuse Drupal
  $args['draftLink'] = "$base_url/draft/" . http_build_query($info);
  foreach ($choices as $uid => $full_name) $choices[$uid] = substr(r\quid($uid), 3) . '   ' . $full_name;
  if (empty($choices)) return say($msg, $args, $field_name); else which($choices, $field_name, $info, tt($msg, $args));
}
  
/**
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($info, $confirmed) {
  global $lastZxid, $scanned;
  $cuid = r\acct()->id;
  extract(u\just('op who amount goods purpose scanned', $info)); // $scanned is global, for be\transfer
  $txType = $op == 'Pay' ? 'payment' : strtolower($op);
  if (blank_field(compact(u\ray('who amount')))) return FALSE;
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if (!$goods) $purpose = 'cash'; // tell be\transfer it's not for goods (should be the only place this string occurs)
  if ($error = u\badAmount($amount, '>0')) return say($error, 'amount');
  if (!($acct = whois($who, 'who', $info, $txType == 'payment'))) return FALSE; // (allow "ALL" for payment)
  svar('lastCustomer', $acct);
  list ($msg, $args, $confirm) = in_array($txType, array('payment', 'charge')) ?
       be\transfer($txType, $acct, $amount, $purpose, !u\forCash($purpose), $confirmed)
     : communityTransfer($txType, $acct, $amount, $confirmed);
  if (!@$scanned and strtoupper($who) != 'ALL') { // not scanned -- We need to show identifying photo, name, etc. for the target account
    $identifiers = identifiers($acct);
    $identifiers = "<div id='identifiers' class='clearfix'>$identifiers</div identifiers>";
  }
  if ($confirm) return @$identifiers . tt('confirm ' . $msg, $args);
  if (!@$args['success']) return say($msg, $args, 'amount');

  svar('lastZxid', $lastZxid); // remember identity of most recent transaction
  return say($msg, $args);
}

/**
 * Record a transfer to or from the community.
 * @param int $type: what type of transfer
 * @param acct $acct: the account to credit or debit
 * @param int $amount: how much to transfer
 * @param bool $confirmed: has this transaction been confirmed by the user?
 * @return simple array: 
 *    index: index to result message string (if success, the index begins with "report ")
 *    parms: replacement parameters, specific to the message
 *    confirm: boolean "this needs to be confirmed"
 */
function communityTransfer($type, $acct, $amount, $confirmed) {
  $success = (bool) be\fund($acct->id, constant(strtoupper("TX_$type")), $amount, $confirmed);
  $msg = $success ? ($confirmed ? 'report funding' : 'funding') : 'funding error';
  $confirmAction = ucwords($type);
  $action = $confirmAction . 'ed';
  $otherName = r\userField('full_name', $acct->id);
  $amount = u\formatAmount($amount);
  return array($msg, compact(u\ray('success action confirmAction amount otherName')), $success and !$confirmed);
}  
    
function menu($title, $type, $function = NULL, $function_args = array(), $access = 'member', $other = array()) {
  global $menu_weight;
  $menu_weight += !@$menu_weight ?: 1;
  
  if (function_exists($local_func = "rCredits\\Web\\$function")) $function = $local_func;
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
//    'access callback' => TRUE,
    'access callback' => 'rCredits\\Web\\webAccess', // don't use be\access directly (rweb needs to remember $cuid)
    'access arguments' => array('a' . $access), // 'a' because Drupal doesn't like numbers
    'weight' => @$menu_weight - 1,
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  ), $other); // $other overwrites, if there is a conflict
}

/**
 * Say whether user has submitted the confirmation form (as opposed to the primary form)
 * The "v" verion is called from _validate, the "s" version from _submit.
 */
function confirming_s(&$sta) {return ($sta['rebuild'] = isset($sta['confirm']));}
function confirming_v(&$sta) {
  if(@$sta['confirm']) {
    extract($sta['values']);
    $sta = $sta['submitted_state'];
    if ($op == 'Cancel') {
      $sta['rebuild'] = TRUE;
      say('op canceled'); // not an error message, else confirmation form persists
    }
    return TRUE;
  }
  $sta['submitted_state'] = $sta;
  return FALSE;
}

function confirm($msg, &$sta) {
  t\testOutput(preg_replace('/\s*$\s*/sm', PHP_EOL, strip_tags($msg)), 'screen');
  return ($sta['confirm'] = $msg);
}

/**
 * Complain about an input error if a required field is blank
 * @param array $fields: associative array of field names, with or without a prefix
 * @param string $prefix: option field name prefix (add if missing, else remove from message)
 */
function blank_field($fields, $prefix = '') {
  foreach ($fields as $key => $value) {
    $prefixed = u\abbreviates($prefix, $key);
    $field = strtoupper($prefixed ? substr($key, strlen($prefix)) : $key);
    $actual_name = $prefixed ? $key : ($prefix . $key);
    if (trim($value) == '') {say('required field', compact('field'), $actual_name); return TRUE;}
  }
  return FALSE;
}

/**
 * Display a drupal message (error or not)
 * Possible syntaxes:
 *   say(NULL) [do nothing in this case]
 *   say('index', array(optional args), 'optional error field')
 *   say('index', 'error field')
 *   say(array(index, args), 'optional error field')
 * @return FALSE (transfer() depends on this)
 */
function say($index, $args = array(), $error_field = '') {
  if (!@$index) return;
  if (is_array($index)) list ($index, $args, $error_field) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $error_field) = array($error_field, $args); // allow either order, for easy 2-param calls
  $message = tt($index, $args);
  if ($error_field) \form_set_error($error_field, $message); else \drupal_set_message($message);
  u\log($error_field ? 'say error' : 'say', compact('message'));
  return FALSE;
}

function userMenuItem($router_path, $link_title) {
  $menu_name = 'user-menu';
  $link_path = $router_path;
  if (r\dbLookup(1, '{menu_links}', 'menu_name=:menu_name AND link_path=:link_path', compact('menu_name', 'link_path'))) return; // done
  $module = 'menu';
  $customized = TRUE;
  $depth = 1;
  $mlid = 0;
  $item = compact(u\ray('mlid menu_name router_path link_path link_title module customized depth'));
  menu_link_save($item);
}

function focusOn($focusFieldName) {
  u\EXPECT(!empty($focusFieldName), 'focus on null');
  $focusFieldName = str_replace('_', '-', $focusFieldName);
  js("document.getElementById('edit-$focusFieldName').focus();", 'inline', 'footer');
}

function formFooter() {
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $markup =<<<EOF
<hr>
<a href="http://rCredits.org">rCredits promo site</a> |
<a href="help">Help</a>
<br><br>
website copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance&trade;</a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:info@rCredits.org">info@rCredits.org</a><br>
EOF;
  $footerText = item($markup); // unique name for this page
$js =<<<EOF
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
EOF;
  js($js, 'inline', 'footer');
  return compact('footerText');
}

/**
 * Give the user some more ASIF rCredits
 */
function getAsif($return) {
  be\fund(r\acct()->id, TX_GRANT, R_STARTING_BALANCE, TRUE, FALSE);
  say('got asif credits');
  r\go($return);
}

function webAccess($access) {
  $access = substr($access, 1);
  setupGlobals(); // remember who we are (the only reason for this function)
  $cacct = $cacct = r\acct();
  return ($access and $cacct) ? $cacct->can($access) : FALSE;
}

function css($sheet, $weight = -99) {
  drupal_add_css(\drupal_get_path('module', 'rweb') . "/$sheet", array('group' => CSS_THEME) + weight($weight));
}

function js($what, $type, $scope) {
  global $base_url;
  if ($type == 'file') $what = "$base_url/sites/all/modules/rcredits/$what";
  return drupal_add_js($what, compact('type', 'scope'));
}

function labeled($form) {
  $form['#attributes']['class'] = array('labeled');
  return $form;
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs);}
//function block_def($info, $extra = array()) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function blockView($subject, $content) {return compact(u\ray('subject content'));}

function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($default_value) {return compact('default_value');}
function weight($weight) {return compact('weight');}
function attrib($attribs) {return array('attributes' => $attribs);}
function autocomplete($path) {return array('autocomplete_path' => "autocomplete/$path/" . @r\acct()->id);}
function required() {return array('required' => TRUE);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
function op($sta) {return $sta['clicked_button']['#parents'][0];}
function submit($label = '') {return formField('submit', $label ?: t('Submit'));}
function item($label) {return formField('item', $label);}
