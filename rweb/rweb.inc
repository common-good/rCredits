<?php
namespace rCredits\Web;
use rCredits\API as api;
use rCredits\Util as u;
use rCredits as r;
include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent
require_once __DIR__ . '/../rcredits-api.inc';

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

define('R_TOGGLE_JS', '
  function toggle(field) {
    field = "#" + field;
    jQuery(field + "-YES").toggle();
    jQuery(field + "-NO").toggle();
    jQuery(field).val(jQuery(field + "-YES").is(":visible"));
  }
');

$GLOBALS[TX_WEB . ' messages'] = array(
//  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"

  // reports
  'report new relation' => t('@who is now related to this account. You will need to adjust the permissions or other settings.'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account ID is @quid. Your@asif starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in. Pretend to buy and sell just as you would in a typical month. We need that information, to make sure everyone has plenty of places to receive and spend their rCredits.</p><p>Experiment! Have fun! (and please let us know if you run into any glitches)'),
  'info saved' => t('Your information has been saved.'),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  
  // errors
  'missing purpose' => t('For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc.". Rebates and bonuses are intended as rewards for productive economic activity in rCredits. Note: for everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.'),
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no relations' => t('There are not yet any relations for this account.'),
  'no cells' => t('This account has no related cell phones.'),
  'required field' => t('@field must not be blank.'),
  'bad name' => t('That is not a plausible name.'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad account id' => t('That is not a proper account ID.'),
  'bad amount' => t('Amount must be a number.'),
  'bad account number' => t('The account number must be three or more digits'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'no such company' => t('There is no such company.'),
  'op canceled' => t('Operation Canceled.'),
  'request & demand' => t('<div class="r-explain">There is a waiting list, to get more rCredits. You will receive an email when your turn comes around.<br><br>Meanwhile, you can get $20 worth of rCredits, as an advance.</div><br><div class="r-amounts">Your current order for rCredits is <b>@request</b><br>Total demand for rCredits overall is <b>@demand</b></div>'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm delete cell' => t('Are you sure you want to remove the selected cell phones from this account?'),
  'already related' => t('That person is already related to this account. If you want to change the settings, find this person in the list and change the settings on that row.'),
  'already cell' => t('That cell phone (@number) is already connected to this account.'),
  'cell taken' => t('A cell phone can be connected to only one account. You are already using that cell phone (@number) in connection with account "@account_name". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'bad routing number' => t('US bank routing numbers are 9 digits. Use the number on the lower left edge of your checks.'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'ambiguous other' => t('"@who" might mean any of these rCredits accounts. Click on the one you want.'),
);

function modal_form($form, &$form_state) {
  $form = array(
    'one' => form_field('textfield', 'prompt', 'help text', 'default'),
    'submit' => form_field('submit', '', '', 'Go!'),
  );
  
  return $form;
}

function util($arg) {
  if ($arg == 'modal') drupal_goto('modal');
  if ($arg == 'test') {
    include __DIR__ . '/../../gherkin/test.php';
  }
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
    api\give_signup_incentive($uid);
    api\give_signup_incentive($uid2);
    r\dbQ(file_get_contents(__DIR__ . '\..\sql\r_companies-insert.sql'));
*/
  }
  
  if ($arg == 'clear_sessions') r\dbQ('TRUNCATE table SESSIONS');
  
  drupal_set_message("Doing rCredits Util '$arg'...");
  drupal_goto('handy');
}

function handy_links() {
  global $base_path, $base_url, $base_root;

  $test_path = "$base_url/sites/all/modules";

  $links = array(
    'rcredits/util/test?rcredits/rsms|Test rSMS|',
    'rcredits/util/test?rcredits/rsmart|Test rSmart|',
    'rcredits/util/test?rcredits/rweb|Test rWeb|',
    "$test_path/gherkin/compile.php?module=rcredits/rsms&return=1|Compile rSMS|",
    "$test_path/gherkin/compile.php?module=rcredits/rsmart&return=1|Compile rSmart|",
    "$test_path/gherkin/compile.php?module=rcredits/rweb&return=1|Compile rWeb|",
    'sms|Simulate an SMS transaction|Simulate SMS',
    'admin/config/development/testing|Test|Test',
    'deletetests.php|Delete old tests|Delete old tests',
    'rcredits/util/reset|Reset rCredits|',
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
    'devel/run-cron|Run cron|Run cron',
    'devel/session|List the contents of SESSION.|Session viewer',
    'devel/theme/registry|View a list of available theme functions across the whole site.|Theme registry',
    'devel/variable?destination=node|Edit and delete site variables.|Variable editor',
  );

  $result = '';
  foreach ($links as $link) {
    list ($url, $title, $label) = explode('|', $link);
    if (!$label) $label = $title;
    if (!u\abbreviates('http://', $url)) $url = $base_path . $url;
    $result .= <<<EOF
<div><a href="$url" title="$title">$label</a></div>
EOF;
  }
  return "<style>div.develw div {display:inline; margin-right:20px;}</style><div class='develw'>\n$result</div>";
}

function show_form($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  try {
    setup_globals(); // in case it's an anonymous form, so web_access doesn't have a chance to set up
    if ($function != 'accounts' and $function != 'home') u\log("show $function", compact(u\ray('arg1 arg2 arg3 arg4')));
    drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME) + weight(-9));
    $form_name = substr($function, 0, 1) == '\\' ? $function : "rCredits\\Web\\{$function}_form";
    $form = drupal_get_form($form_name, $arg1, $arg2, $arg3, $arg4);
  //  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
    $form['#attributes']['class'][] = 'rweb';
    if (!@$form['confirm']) $form += which(); // add the popup form, if any
    if ($colgroup = @$form['#colgroup']) {
      unset($form['#colgroup']);
      $markup = '';
      foreach ($colgroup as $col) {
        $guts = '';
        foreach ($col as $attrib => $value) $guts .= " $attrib=\"$value\"";
        $markup .= "<col $guts>\n";
      }
      $markup = "<colgroup>\n$markup</colgroup>\n";
      return str_replace('<thead>', $markup . '<thead>', render($form)); // shore up Drupal lack of colgroups in tableselect
    } else return render($form);
  } catch(u\fit $e) {r\Web\exception_handler($e);}
}

/**
 * Offer choices for user to choose from
 * 1. If called with all arguments, the arguments are stored in svar('pop') and the form is reloaded
 * 2. If called as which('info'), this function returns $info from svar('pop')
 * 3. If called with NO arguments (from show_form()), this function returns a fieldset to insert as a popup layover div into
 * the form awaiting the choice. Clicking on a choice fills in the missing value and submits the form.
 * For #3, we expect svar('pop') to contain: $form_name, $choices, $result_field, and $question
 *   $form_name: the form name
 *   $choices: an array of choices
 * @param array $choices: associative array of things to choose from
 * @param string $result_field: name of field that gets the choice
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param string $question: the question to ask
 */
function which($choices = '', $result_field = '', $info = '', $question = 'Which one?') {
  $form_name = current_path();
  
  if (is_array($choices)) { // called with all args (#1)
    svar('pop', compact(u\ray('form_name choices result_field info question')));
    drupal_goto($form_name); // reload page
  }

  if (!is_array($pop = svar('pop'))) return array();
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($form_name != $pop_form_name) return array();
  if ($choices == 'info') return $pop_info; // called as which('info') (#2)
  if ($choices) return array();
  
  svar('pop', NULL); // called with no args (#3)
  $onchange = "this.form.elements['$pop_result_field'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $subtext = form_field('item', $pop_question);
  $choice = form_field('select', '', '', attrib(compact('onchange') + array('size' => 12)), $pop_choices);
  $popup = form_field('fieldset', t('Which One?'), '', array('id' => 'popup')) + compact(u\ray('subtext choice'));
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
 */
function close_form() {
  $js = 'window.opener = bottom; window.close();';
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  return '';
}

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
  $sub_vars = 'string';
  
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
      FROM users u WHERE (u.name LIKE :string OR u.mail LIKE :string)
EOF;
    $sub_vars .= ' cuid';
  } elseif ($role == 'industry') {
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT full_name AS display FROM users WHERE (account_type!=:R_PERSONAL AND name LIKE :string)
EOF;
  } elseif ($role == 'company') {
    $selection = 'full_name AS display FROM users WHERE (account_type!=:R_PERSONAL AND name LIKE :string)';
  } else $selection = 'full_name AS display FROM users WHERE name LIKE :string';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid >= 0 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => $sql));

  $result = r\dbQ($sql, compact(u\ray($sub_vars)));
  $matches = array();
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = substr(r\quid($uid), 3) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
  }
  drupal_json_output($matches);
}

function home_form($form, &$form_state) {
  $cacct = r\acct();
  $is_company = api\access('company');
  $name = $cacct->name;
  
  $stagelist = 'rTrader Steward Partner Member';
  if ($is_company) $stagelist = str_replace('Steward ', '', $stagelist);
  $stages = u\ray($stagelist);
  foreach ($stages as $stage) if (api\access(strtolower($stage))) break;

  $rTrader = 'authorized to use rCredits "for real"';
  if (!$is_company) $Steward = "a responsible voter, stewarding your community's finances";
  $Partner = 'a committed financial partner in the rCredits system';
  $Member = 'signed up to do rCredits "As If" transactions, for real credit';
  $details = compact($stages);
  foreach ($details as $key => $value) $details[$key] = "<em>$key</em> - $value<br>\n";

  extract($details);
  $Partner = $Member . $Partner;
  if (!$is_company) $Steward = $Partner . @$Steward;
  $rTrader = ($is_company ? $Partner : $Steward) . $rTrader;
  $now = compact($stages);
  foreach ($now as $key => $value) $now[$key] = "<div id='overview-stage' class='labeled-block'>$value</div>\n";

  extract($details);
  $Member = $Partner . @$Steward . $rTrader;
  $Partner = @$Steward . $rTrader;
  if (!$is_company) $Steward = $rTrader;
  $rTrader = '';
  $future = compact($stages);
  foreach ($future as $key => $value) $future[$key] = "<div id='overview-future' class='labeled-block'>$value</div>\n";
  
  extract(api\creditInfo()->fancy, EXTR_PREFIX_ALL, 'my');

  $form = array(
    'title' => form_field('item', t('Overview of Your rCredits Account')),
//    'subtext' => basic_account_info(),
    'account_name' => form_field('item', t('Account Name:'), '', $cacct->full_name . ' ' . u\hug($name)),
    'account_id' => form_field('item', t('Account ID:'), '', $cacct->qid . ' ' . u\hug(R_REGION_NAME)),
    'account_type' => form_field('item', t('Account type:'), '', $GLOBALS['account types'][$cacct->account_type+0]), // +0 for admin
    'stage' => form_field('item', t('You are:'), '', $now[$stage]),
    'future' => $future[$stage] ? form_field('item', t('Future milestones:'), '', $future[$stage]) : NULL,
    'steps' => form_field('item', t('Next Steps:'), '', t('<div id="next-steps" class="labeled-block"><ol><li>Do at least three realistic "As-If" Payments to participating businesses, so the system knows where you will be able to spend your rCredits.</li><li>Do at least one As-If Charge, so the system knows where you will get your rCredits.</li><li>Try some of the other rCredits features.</li><li>Read or talk to other participants until you are confident that you understand how rCredits works.</li></ol></div>')),
    'balance' => form_field('item', t('Balance:'), '', $my_balance),
    'credit_limit' => form_field('item', t('Credit limit:'), '(this is how far in debt you can go &mdash; for up to 30 days)', u\formatAmount(-$cacct->min_balance)),
    'rewards_to_date' => form_field('item', t('Rewards to date:'), '', $my_rewards),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function account_form($form, &$form_state) {
  $form['#user'] = r\acct()->account();
  user_account_form($form, $form_state);
  system_user_timezone($form, $form_state);

  unset($form['account']['name']);
  $form['account']['mail']['#description'] = t('All emails from the system will be sent to this address. The email address is not made public');
  $form['account']['pass']['#description'] = t('Enter your new password in both fields above.<br>To change your email address and/or password, enter your current password below.');
  $form['account']['pass']['#process'] = array('form_process_password_confirm', 'rCredits\\Web\\password_confirm');
  $form['account']['current_pass']['#description'] = '';
  $form['account']['current_pass']['#weight'] = 4;
  $form['account']['status']['#weight'] = 5;
  $form['account']['roles']['#weight'] = 6;
  $form['account']['title'] = form_field('item', t('Account Information'), '', array('weight' => -99));
  $form['submit'] = form_field('submit', '', '', array('value' => t('Save'), 'weight' => 99));
  $form['#user_category'] = 'account';
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'][0] = 'rCredits\\Web\\account_form_validate'; // use ours instead
  
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
 * Replacement for user_account_form_validate()
 * (because we want to allow the same email for several accounts)
 */
function account_form_validate($form, &$form_state) {
  $mail = trim($form_state['values']['mail']);
  form_set_value($form['account']['mail'], $mail, $form_state);
  if ($error = user_validate_mail($mail)) form_set_error('mail', $error);
}

function account_form_submit($form, &$form_state) {
  $account = r\acct()->account();
  \user_save($account, $form_state['values']);
  r\acct()->reread();
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function accounts_form($form, &$form_state) {
  $cuid = r\acct()->id;
  if (!($accounts = api\account_choices())) return FALSE;
  $choices = array();
  foreach ($accounts as $uid => $zot) {
    $balance = u\formatAmount(api\creditInfo(compact('uid'))->balance);
    $choices[$uid] = r\userField('full_name', $uid) . ": $balance";
  }

  $qid = r\quid(); // $credit_id = r\userField('credit_id', $cuid);
  $full_name = r\userField('full_name', $cuid);
  $onchange = "this.form.submit();";
  $account = form_field('select', t('Account:'), t('(Select an account)'), 
    dft($cuid) + attrib(compact('onchange')), $choices);
  $one_account = form_field('item', t('Account') . ": <span>$full_name ($qid)</span>");
  $one_balance = form_field('item', t('Current Balance') . ": <span>$balance</span>");
  $submit = form_field('submit');
  $form = count($choices) == 1 ? compact(u\ray('one_account one_balance')) : u\prefixKeys('account_', compact(u\ray('account submit')));

  //  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function accounts_form_submit($form, &$form_state) {
  extract($form_state['values']);
  svar('cuid', $account_account); // this gets checked ONLY in setup_globals()
}

/**
 * Display the member company's profile (no editable fields here)
 */
function profile_form($form, &$form_state, $company) {
  $uid = r\userField('uid', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or $acct->account_type == R_PERSONAL) return say('no such company', 'zot');
  extract((array) $acct->account(), EXTR_PREFIX_ALL, 'my');

  $pic = profile_picture($uid, TRUE);
  $country = u\countryAbbrev($my_country);
  $website = $my_website ? "<a href='http://$website' target='_blank'>$my_website</a>" : '';
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
  <div id='member-picture'>$pic</div>
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
    'all' => form_field('item', $markup),
  );
  return $form;
}

function profile_picture($uid, $with_markup = FALSE) {
  global $user; $user = r\acct()->account(); // keeps \theme from dying
  $acct = r\acct($uid);
  $data = $acct->data;
  $account = $acct->account();
  $markup = str_replace('styles/thumbnail/public/', '', \theme('user_picture', compact('account')));
  if ($markup) {
    $a = explode('"', $markup);
    if (strpos($url = $a[3], 'http://') === FALSE) $url = $a[7]; 
  } else $url = is_array($data) ? @$data['picture'] : '';
  
  if (!$with_markup or !$url) return $url;
  $info = @getimagesize($url);
  list ($width, $height) = is_array($info) ? $info : array(1, 0);
  $new_height = min($height * 2, 800 * $height / $width, 400);
  if (!$new_height) $new_height = 400; // in case getimagesize fails
  $new_width = $height ? round($width * $new_height / $height, 0) : '';
  $subs = compact(u\ray('new_width new_height url'));
  return strtr('<div class="user-picture"><img src="url" width="new_width" height="new_height" alt="profile picture" /></div>', $subs);
}

/**
 * Get a transaction request (pay or charge) and process it)
 */
function tx_form($form, &$form_state, $type, $company = '') {
  if ($confirm = sure_form($form_state)) return $confirm;
  $type_low = strtolower($type);
  $role = $type == 'Pay' ? 'payer' : 'payee';
  //$onchange = "nixwhat();";
  $onchange = "if(this.value.toUpperCase() == 'ALL') jQuery('#edit-purpose').val('payroll');";
  $radios = array(t('cash/loan/etc.'), t('goods & services'));
  
  $form = array(
    'title' => form_field('item', $type),
    'who' => form_field('textfield', $type . ' whom:', t('Type a name, id, email, or phone'), dft($company) + autocomplete($role) + attrib(compact('onchange'))),
    'amount' => form_field('textfield', 'Amount: $', t('Amount to ') . $type_low),
    'goods' => form_field('radios', '', '', dft(1), $radios), //  + attrib(compact('onchange'))
    'purpose' => form_field('textfield', 'Purpose:', t('Description of goods and services or purpose')),
    $type_low => form_field('submit', NULL, t('Submit button'), $type),
  );
  $form['#attributes']['class'] = array('labeled');
  
/*
  $js = <<<EOF
    function nixwhat() {
      var c0=document.getElementById('edit-goods-0');
      jQuery('.form-item-what').css('display', c0.checked ? 'none' : 'block');
    }
    nixwhat();
    document.getElementById('edit-who').focus();
EOF;
*/
  $js = "document.getElementById('edit-who').focus();";
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  
  return keep_values($form); // get defaults from which('info')
}

function tx_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;
  if ($confirm = transfer($form_state['values'], FALSE)) confirm($confirm, $form_state); // op who amount goods what
}

function tx_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  transfer($form_state['values'], TRUE); // op who amount goods what
  drupal_goto(strpos(@$form['#action'], 'pay') ? 'pay' : 'charge');
}

/**
 * Pay with rCredits, arriving from some other site.
 * @todo: handle <user logged in> better
 */
function buy_now_form($form, &$form_state) {
  $in = @$form_state['input'] ?: (svar('external_input') ?: $_GET); // $_GET is always an array here
  extract($in, EXTR_PREFIX_ALL, 'my'); // get input from outside site or (stored) from login
  if (@$my_external_input) extract(unserialize($my_external_input), EXTR_PREFIX_ALL, 'my');
  $confirmed = @$my_confirmed ?: (!@$form_state['input'] and user_is_logged_in());
  
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

    $button = user_is_logged_in() ? form_field('submit', NULL, t('Submit button'), $verb)
      : form_field('item', t('By typing your username and password, you agree to pay with rCredits, as detailed above.'));

    $my_amount = @number_format($my_amount, 2);
    $disabled = array('disabled' => 1);
    $verblower = strtolower($verb);
  }
  
  list ($op, $amount, $goods) = array('Pay', $my_amount, TRUE);
  list ($who, $what) = @$my_confirmed ? array($my_who, $my_what) : array($my_company, $my_item);

  if (@$confirmed) {
    transfer(compact(u\ray('op who amount goods what')), TRUE);
    if (u\valid_url(@$my_return_to)) return array(form_field('item', "<a href='$my_return_to'>Click here</a> to return to the $company_name website."));
    drupal_goto('');
  }

  $form = array(
    'title' => form_field('item', t('Confirm Payment')),
    'xwho' => form_field('textfield', 'to:', t('Pay whom?'), disabled($company_name)),
    'who' => form_field('hidden', $who),
    'xamount' => form_field('textfield', '$', t("Amount to $verblower"), disabled($amount)),
    'amount' => form_field('hidden', $amount),
    'xwhat' => form_field('textfield', 'for:', t('Description of goods and services'), disabled($what)),
    'what' => form_field('hidden', $what),
    'goods' => form_field('hidden', $goods),
    'return_to' => form_field('hidden', @$my_return_to),
    'confirmed' => form_field('hidden', TRUE),
    'submit' => $button,
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function buy_now_form_submit($form, &$form_state) {
//  tx_form_submit($form, $form_state);
//  drupal_goto('charge'); // member page when done
}

/**
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 */
function exchange_form($form, &$form_state) {
  $cuid = r\acct()->id;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $request = u\formatAmount(r\userField('demand', $cuid));
  $demand = api\creditInfo()->fancy['total_demand'];

  $form = array(
    'title' => form_field('item', t('Get rCredits / USD')),
    'subtext' => form_field('item', tt('request & demand', compact(u\ray('request demand')))),
    'amount' => form_field('textfield', t('Amount:'), t('Amount to exchange (r$ or us$)')),
//    'way' => form_field('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
//    'who' => form_field('textfield', 'to/from:', t('Exchange with whom?')),
    'getr' => form_field('submit', '', t('Submit button'), 'Get rCredits'),
    'getusd' => form_field('submit', '', t('Submit button'), 'Get USD'),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function exchange_form_validate($form, &$form_state) {
  $cuid = r\acct()->id;
  if (confirming_v($form_state)) return;

  extract($form_state['values']);
  if (blank_field(compact('amount'))) return;
  list ($message, $args, $confirm) = api\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cuid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $form_state);
  if (!@$args['success']) say($message, $args, 'amount');
}

function exchange_form_submit($form, &$form_state) {
  $cuid = r\acct()->id;
  if (confirming_s($form_state)) return;

  extract($form_state['values']);
  list ($message, $args, $confirm) = api\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cuid, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($op == 'Get rCredits') {
    if ($amount < R_MIN_DEMAND) say('demand too small');
    if (!api\access('partner')) say('demand not yet');
  }
}

/**
 * @NOTE: The country and state drop down require a change in form.inc (see form.inc.patch)
 */
function contact_info_form($form, &$form_state) {
  api\merge_check(compact(u\ray('name website phone fax'))); // check before and after
  extract((array) r\acct()->account(), EXTR_PREFIX_ALL, 'my'); // can't easily check for plain because roles is an array
  $full_name_desc = t('Your full legal name, properly capitalized');
  if (api\access('company')) $full_name_desc .= '<br>' . t('(If you change this, your short identifier and rCredits webpage URL will also change.');
  
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form = array(
    'title' => form_field('item', t('Contact Information')),
//    'subtext' => basic_account_info(),
    'full_name' => form_field('textfield', t('Full name:'), $full_name_desc, dft($my_full_name) + required()),
    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\formatPhone($my_phone): '') + required()),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)),
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))), // can't use required() here
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), dft($my_postal_code)+ required()),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)), // can't use required() here
    'city' => form_field('textfield', t('City:'), t(''), dft($my_city) + required()),
    'address' => form_field('textfield', t('Address:'), t('Mailing address'), dft($my_address) + required()),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Submit'),
  );

  $form['#attributes']['class'] = array('labeled');
  drupal_add_js('inc/countries.js', array('type'=>'file', 'scope'=>'header'));
  drupal_add_js("print_country('edit-country', \"$my_country\", \"$my_state\");", array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function contact_info_form_validate($form, &$form_state) {
  extract($form_state['values']);
  if (blank_field(compact(u\ray('phone city address')))) return;
  if ($phone and !u\formatPhone($phone, '+n')) say('bad phone', 'phone');
  if ($fax and !u\formatPhone($fax, '+n')) say('bad phone', 'fax');
}

function contact_info_form_submit($form, &$form_state) {
  extract($info = $form_state['values'], EXTR_PREFIX_ALL, 'my');
  $info['website'] = str_replace('http://', '', strtolower(@$my_website));
  if ($my_phone) $info['phone'] = u\formatPhone($my_phone, '+n');
  $info['name'] = u\shortName($my_full_name);
  r\userUpdate($info);
  api\merge_check(compact(u\ray('name website phone fax'))); // check before and after
  say('info saved');
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

/**
 * Get the bank account info
 * @todo: store two verification EFT deposit amounts in bank_account_status as 1000*one+two
 * (if status > 100, it's waiting for verification)
 */
function bank_form($form, &$form_state) {
  extract((array) r\acct()->account(), EXTR_PREFIX_ALL, 'my'); // can't easily check for plain because roles is an array

  if (!$my_bank_account_name) $my_bank_account_name = $my_full_name;
  
  $form = array();
  if ($my_country == 'United States') {
    $my_routing_number = substr($my_bank_account_number, 4, 9); // chop of USkk
    $my_bank_account_number = substr($my_bank_account_number, 4 + 9); // everything after the routing number
    $routing_number = form_field('textfield', t('Routing number:'), t('What is the 9-digit routing number (type carefully)'), dft($my_routing_number) + required());
  } else $routing_number = NULL;
  
  $form = array(
    'title' => form_field('item', t('Bank Information')),
//    'subtext' => ,
    'bank_account_name' => form_field('textfield', t('Account name:'), t('What is the name on this bank account?'), dft($my_bank_account_name) + required()),
    'routing_number' => $routing_number,
    'bank_account_number' => form_field('textfield', t('Account number:'), t('What is the account number? (be sure to get this right)'), dft($my_bank_account_number) + required()),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Submit'),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function bank_form_validate($form, &$form_state) {
  extract(u\trimAll($form_state['values']));
  if (!u\validName($bank_account_name, FALSE, FALSE)) say('bad name', 'bank_account_name');
  if (!preg_match('/^(|[0-9]{9})$/', $routing_number)) say('bad routing number', 'routing_number');
  if (!preg_match('/^[0-9]{3,20}$/', $bank_account_number)) say('bad account number', 'bank_account_number');
}

function bank_form_submit($form, &$form_state) {
  extract($form_state['values']);
  if (r\acct()->country == 'United States') $bank_account_number = "USkk$routing_number$bank_account_number";
  r\userUpdate(compact(u\ray('bank_account_name bank_account_number')));
  say('info saved');
}

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function relations_form($form, &$form_state) {
  $cuid = r\acct()->id;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $account_identifiers = api\account_name($cuid);
  $header = u\ray('Person,Employee?,Owner?,Permission');
  $classes = u\prefixValues('rel-', u\ray('person employee owner permission'));
  
  $sql = <<<EOF
    SELECT s.reid, u.full_name, s.permission, s.is_employee, s.is_owner
    FROM r_relations s RIGHT JOIN users u ON u.uid = s.agent_uid
    WHERE s.main_uid=:cuid
    ORDER BY u.full_name
EOF;
  $result = r\dbQ($sql, compact('cuid'));
  $rows = $original = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $original[$reid] = compact(u\ray('permission is_employee is_owner full_name'));
    $permission = dropdown('permission', $permission, $GLOBALS['share permissions'], $reid);
    $rows[] = array($full_name, toggle('is_employee', $is_employee, $reid), toggle('is_owner', $is_owner, $reid), $permission);
  }
  $form_state['original_settings'] = $original;
  
  $help = t("Use this form to mark someone as an employee or owner or to give someone access to this account. Individuals marked as employees are the ones who will get paid in rCredits when you use the Pay form to pay \"all\".<br><br>For example, you might authorize some employees to buy or sell for your company. The system will track who does what. Or you might give your accountant \"read\" permission.");
  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$account_identifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no relations');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => form_field('item', t('Relations')),
    'subtext' => form_field('item', $subtext),
    'new_person' => form_field('textfield', t('Add Someone:'), t("name, id, email, or phone"), autocomplete('')),
    'list' => form_field('item', '', '', array('markup' => $list)),
//    'new_permission' => form_field('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'go' => form_field('submit', NULL, t('Submit button'), 'Save Changes'),
  );

  drupal_add_js(R_TOGGLE_JS, array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function dropdown($name, $value, $options, $id) {
  $name = "$name-$id";
  return render($zot = array("$name-$id" => form_field('select', '', '', compact('value') + attrib(compact('name')), $options)));
}

function toggle($field, $value, $id) {
  $path = current_path();
  $name = $id = "$field-$id";
  $input = \render($zot = array($id => form_field('hidden', '', '', attrib(compact(u\ray('id name value'))))));
  $b1 = button(' - ', "toggle('$id');", t('Change to NO'), 'form-no');
  $b2 = button(' + ', "toggle('$id');", t('Change to YES'), 'form-yes');
  list ($yesvis, $novis) = $value ? array('visible', 'invisible') : array('invisible', 'visible');
  return <<<EOF
    <div class="$field-buttons">
    <div id="$id-YES" class="$yesvis form-yes">Yes $b1</div>
    <div id="$id-NO" class="$novis form-no">No &nbsp;$b2</div>
    $input
    </div>
EOF;
}

function relations_form_validate($form, &$form_state) {
  $cuid = r\acct()->id;
  extract($form_state['values']);
  if (confirming_v($form_state)) return;
  
  if (@$new_person) {
    $acct = whois($new_person, 'new_person', $form_state['values'], FALSE, FALSE, 'no self-relation');
    if (api\access('company', $acct)) return say('no company relations', 'new_person');
    $uid = $acct->id;
    $already = r\dbLookup('1', 'r_relations', 'main_uid=:cuid AND agent_uid=:uid', compact('cuid', 'uid'));
    if ($already) return say('already related', 'new_person');
    $form_state['values']['agent_uid'] = $uid; // save for submission handling
  }
//  }
}

function relations_form_submit($form, &$form_state) {
  $main_uid = r\acct()->id;
  if (confirming_s($form_state)) return;
  
  extract($form_state['values']);
  $original = $form_state['original_settings'];

  if (@$agent_uid) {
    $permission = 0; // default to no permissions
    $info = compact(u\ray('main_uid agent_uid permission'));
    $zot = \drupal_write_record('r_relations', $info);
    $who = r\userField('full_name', $agent_uid);
    say('report new relation', compact('who'));
  }

  $changes = array();
  foreach ($_POST as $key => $value) { // Drupal doesn't pass along our hidden inputs, so get them directly from $_POST
    if (!strpos($key, '-')) continue;
    list ($key, $reid) = explode('-', $key); // eg split permission-2 
    if (!in_array($key, u\ray('permission is_employee is_owner'))) return NULL; // hack attempt?
    if ($value != @$original[$reid][$key]) {
      $changes[$reid][$key] = strtr($value, array('true' => 1, 'false' => 0));
      $changes[$reid]['full_name'] = $original[$reid]['full_name']; // save for update reporting
    }
  }
  if (!empty($changes)) \drupal_set_message(api\update_relations($changes));
}

/**
 * @todo: focus on code input
 */
function cell_form($form, &$form_state) {
  $cuid = r\acct()->id;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only deletion
  if (form_step($form_state, $info) == 'prove') {
    $number = u\formatPhone($form_state['storage']['values']['number']);
    $form = array(
      'title' => form_field('item', t('Verify')),
      'subtext' => form_field('item', tt('verify cell', compact('number'))),
      'code' => form_field('textfield', t('Code:')),
      'number' => form_field('hidden', $number),
      'verify' => form_field('submit', NULL, t('Submit button'), 'Verify'),
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
    'title' => form_field('item', t('Cell Phones for this account')),
    'cells' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no cells')), $list),
    'delete' => form_field('submit', NULL, t('Submit button'), 'Remove Selected Numbers'),

    'number' => form_field('textfield', t('Add another cell phone:')),
    'add' => form_field('submit', NULL, t('Submit button'), 'Add'),
  );
  return $form;
}

function cell_form_validate($form, &$form_state) {
  $cuid = r\acct()->id;
  if (confirming_v($form_state)) return;
  extract($form_state['values']);

  if ($op == 'Verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($form_state, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op != 'Add') { // $op == remove
    if (no_selection($cells)) {
      if ($number) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $form_state);
  }

  if ($op == 'Add') {
    if (blank_field(compact('number'))) return;
    $form_state['values']['op'] = $op; // make it stick if user pressed Enter
    say(api\add_cell($number, 'VALIDATE'), 'number');
  }
}

function cell_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  
  extract($form_state['values']);
  if ($op == 'Add') {
    if ($info = previous_state($form_state)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($number, tt('verification code', compact('nonce')));
    return form_step($form_state, compact('number'), 'prove');
  }

  if ($op == 'Verify') {
    step_one($form_state); // no next step (back to original form)
    say(api\add_cell($number));
    drupal_goto('cell-access');
  }
    
  // $op == remove
  foreach ($cells as $key => $number) if ($number) say(api\delete_cell(substr($number, 1))); // ignore the leading 'x'
}

/**
 * Modify the user registration form.
 */
function register_form(&$form, &$form_state) {
  unset($form['account']['name']);
  $account_types = u\ray('Personal,Commercial,Public Non-profit/Government');
  $account_type_help = t('NOTE: If you choose "@commercial" or "@nonprofit", your contact information and photo will be public.', array('@commercial' => $account_types[1], '@nonprofit' => $account_types[2]));
  $my_country = $my_state = '';
  extract($form_state['input'], EXTR_PREFIX_ALL, 'my'); // overwrites my_country and my_state with input values, on input error
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $country_attrib = attrib(compact('onchange'));
  $company_options = array('is_employee' => t('employee'), 'is_owner' => t('owner'));
  $onchange = "var info = jQuery('#company_info'); if(jQuery('#edit-account-type').val() == 0) info.show(); else info.hide();";
  $account_type_attrib = attrib(compact('onchange'));
  
  $company_info = array(
    'company_text' => form_field('item', t('Are you an owner or employee of a local company (or of any participating rCredits company)?')),
    'company' => form_field('textfield', t('Company: '), t(''), autocomplete('company')),
    'company_options' => form_field('checkboxes', '', '', '', $company_options),
  );
  
  $form += array(
    'title' => form_field('item', t('Sign up for rCredits'),'', weight(-99)),
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), required() + weight(-98)),
/*    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\formatPhone($my_phone): '')),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)), */
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + $country_attrib),
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), required()),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)),
    'city' => form_field('textfield', t('City:'), t(''), required()),
    'account_type' => form_field('select', t('Account type:'), $account_type_help, dft('Personal') + $account_type_attrib, $account_types),  
    'company_info' => form_field('fieldset', '', '', array('id' => 'company_info')) + $company_info,

/*    'address' => form_field('textfield', t('Address:'), t('Mailing address'), dft($my_address)),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Submit'), */
  );

  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME) + weight(-9));
  drupal_add_js('inc/countries.js', array('type'=>'file', 'scope'=>'header'));
  drupal_add_js("print_country(\"edit-country\", \"$my_country\", \"$my_state\");", array('type'=>'inline', 'scope'=>'footer'));

  $form['#attributes']['class'][] = 'labeled';
  $form['#attributes']['class'][] = 'rweb';
  $form['#validate'][] = 'rCredits\\Web\\register_form_validate';
  $form['#submit'][] = 'rCredits\\Web\\register_form_submit';
  keep_values($form);
}

function register_form_validate($form, &$form_state) {
  extract($values = $form_state['values'], EXTR_PREFIX_ALL, 'my');

  if ($my_company) {
    extract($my_company_options); // just is_employee and is_owner
    if (!($is_employee or $is_owner)) return say('what relationship', 'company');
    $company = $my_company;
    $data = serialize(compact(u\ray('company is_employee is_owner')));
  }
  
  if ($error = user_validate_name($my_full_name)) return say(str_replace('The username', 'That name', $error));
  $name = $name0 = r\uniqueName($my_full_name);
//  $name = $name0 = u\shortName($my_full_name);
//  if (r\userField(1, 'name=:name', compact('name'))) return say('duplicate name'); // require unique full_name

  u\normalizeCase(u\ray('full_name city address'));
  $communityUid = r\communityUid($my_postal_code);
  $form_state['values'] = array_merge($values, compact(u\ray('name data communityUid')));
}

function register_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_PREFIX_ALL, 'my');
  
  if ($my_company) {
    extract($my_company_options, EXTR_PREFIX_ALL, 'my');
    $companyAcct = whois($my_company, 'company', $form_state['values']);
    if (!r\reid($companyAcct->id, $my_uid)) {
      list ($main_uid, $agent_uid, $is_owner) = array($companyAcct->id, $my_uid, $my_is_owner);
      $info = compact(u\ray('main_uid agent_uid is_owner'));
      \drupal_write_record('r_relations', $info); // leave it to the owner to mark is_employee
    }
  }
// merge fails because account is not saved yet:  api\merge_check(compact(u\ray('email short_name'))); // email fails anyway because it's not unique
  $quid = r\quid($my_uid, $my_uid);
  $balance = u\formatAmount(api\give_signup_incentive($my_uid));
  say('your account is ready', compact(u\ray('balance quid')));
}

/**
 * Modify the login form.
 * @see also login_followup, below (called from rweb_user_login()).
 */
function login_alter(&$form, &$form_state) {
  $form['tip'] = form_field('item', t('<b>NOTE: </b>If you already created an account using cell texting and have never yet logged in here, start by requesting a "new password" for your email address.'));
  $form['external_input'] = form_field('hidden', serialize($form_state['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $i = array_search('user_login_authenticate_validate', $form['#validate']);
  assert($i !== FALSE);
  $form['#validate'][$i] = 'rCredits\\Web\\login_extra_validate'; // our replacement function
  
  $form['name']['#title'] = t('Username/Email/ID');
  
  $form['#attributes']['class'][] = 'rweb';
  $form['#attached']['css'][] = \drupal_get_path('module', 'rweb') . '/rweb.css';
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name), email, or account id, with password.
 * Sets $form_state['uid'] to signify successful authentication.
 */
function login_extra_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_PREFIX_ALL, 'my');
  if (!$my_name or empty($my_pass)) return;
  $failed_login_ip_limit = variable_get('user_failed_login_ip_limit', 50);
  $failed_login_ip_window = variable_get('user_failed_login_ip_window', 3600);
  if (!flood_is_allowed('failed_login_attempt_ip', $failed_login_ip_limit, $failed_login_ip_window)) {
    $form_state['flood_control_triggered'] = 'ip';
    return;
  }
  $cacct = u\isQid(strtoupper($my_name)) ? r\acct(strtoupper($my_name)) : FALSE; // try to interpret it as a qid
  $crit =  $cacct ? ('uid=' . $cacct->id) : ':my_name IN (name,mail)';
  $subs = $cacct ? array() : compact('my_name');
  if (!($fields = r\userField('uid,name', $crit, $subs))) return; // not a user
  
  extract($fields, EXTR_PREFIX_ALL, 'my'); // uid and my_name (replacing earlier my_name)
  $failed_login_identifier_uid_only = variable_get('user_failed_login_identifier_uid_only', FALSE);
  $failed_login_user_limit = variable_get('user_failed_login_user_limit', 5);
  $failed_login_user_window = variable_get('user_failed_login_user_window', 21600);
  
  $identifier = $failed_login_identifier_uid_only ? $my_uid : $my_uid . '-' . ip_address();
  $form_state['flood_control_user_identifier'] = $identifier;

  if (!flood_is_allowed('failed_login_attempt_user', $failed_login_user_limit, $failed_login_user_window, $identifier)) {
    $form_state['flood_control_triggered'] = 'user';
    return;
  }

  $form_state['uid'] = user_authenticate($my_name, $my_pass);
}

function login_followup($edit) {
  global $user;

  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($my_external_input)); // save params from external online merchant

  $name = @$edit['values']['name'];
  svar('cuid', $uid = $user->uid);
  setup_globals();
  u\log('login', compact('name'), $uid);
}

function directory_form($form, &$form_state) {
  $cuid = r\acct()->id;
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: substr(r\userField('postal_code', $cuid), 0, 3);
  
  $list = ($region !== FALSE or $which) ? directory_list($which, $region) : '';
  
  $form = array(
    'title' => form_field('item', t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => form_field('item', t('for a country name or postal code...')),
    'which' => form_field('textfield', t('Search for:'), t('Type part of company name or industry category'), dft($which) + autocomplete('industry')),
    'region' => form_field('textfield', t('Where:'), t('Type the first few characters of your postal code<br>Try 013, to see the list for Greenfield, MA.'), dft($region)), // or country name
    'submit' => form_field('submit', NULL, t('Submit button'), t('Go')),
    'item' => form_field('item', '', '', $list),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function directory_form_validate($form, &$form_state) {
}

function directory_form_submit($form, &$form_state) {
  extract($form_state['values']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

/**
 * Return a formatted, categorized list of businesses
 */
function directory_list($which, $region) {
  global $base_url;
  $field = is_numeric($region) ? 'postal_code' : 'country';
  $which = u\ignoreSuffix($which, 'ants ant ian es ers ing er or ion s');
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $region = \db_like($region) . '%';
  $rows = array();
  $sql = <<<EOF
    SELECT DISTINCT IFNULL(i.industry, '~Other') AS industry, u.name AS short_name, u.full_name, u.website FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE account_type!=:R_PERSONAL AND u.$field LIKE :region AND (u.full_name LIKE :which OR i.industry LIKE :which)
    ORDER BY industry, u.full_name
EOF;
  $result = r\dbQ($sql, compact(u\ray('region which')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]); // industry, short_name, full_name, website
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $pay = api\access('buy and sell') ? button('pay', "pay/$short_name") : '';
    $pay = api\access('sell') ? $chg = button('chg', "charge/$short_name") : '';
//    if ($website) $full_name = "<a href='http://$website' target='_blank'>$full_name</a>";
    $full_name = "<a href='$base_url/member/$short_name'>$full_name</a>";
    $item = "$pay$chg $full_name";
    if (!$headed) $item = "- $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return $list ?: t('There are no such businesses listed.');
}

function company_form($form, &$form_state) {
  $cacct = r\acct();
  $cuid = $cacct->id;
  $my_description = $cacct->description;
  
  $cats = r\dbQ('SELECT iid, industry FROM r_industries ORDER BY industry')->fetchAllKeyed();
  $my_cats = r\dbQ('SELECT iid FROM r_user_industries WHERE uid=:cuid', compact('cuid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  $afterlinks = <<<EOF
  <a id='show-webpage' href="member/$cacct->name" target="_blank">Show my public rCredits web page</a> (in a new window)
EOF;
  
  $form = array(
    'title' => form_field('item', t('Company Information')),
//    'subtext' => basic_account_info(),
    'name' => form_field('item', t('Company name:'), '', $cacct->full_name),
    'categories' => form_field('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($my_cats), $cats),
    'description' => form_field('text_format', t('Description:'), t('What does this company do?'), $filtered_html + dft($my_description)),
    'submit' => form_field('submit', NULL, t('Submit button'), t('Submit')),
    'afterlinks' => form_field('item', $afterlinks),
  );

// (doesn't work well for textarea) $form['#attributes']['class'] = array('labeled');
  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function company_form_validate($form, &$form_state) {
//  extract($form_state['values']);
}

function company_form_submit($form, &$form_state) {
  $cuid = r\acct()->id;
  extract($form_state['values']);
//  $description = \check_markup($description['value']);
  $description = $description['value'];
  r\userUpdate(compact('description'));

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

function button($text, $href = '', $title = '', $type = '') {
  $title = htmlspecialchars($title);
  $type = 'local form-submit' . ($type ? " $type" : ''); // Drupal prefixes this with 'form-'
  $markup = render($zot = array($text => form_field('button', '', '', array('value' => $text, 'button_type' => $type))));
  $markup = str_replace('"submit"', '"button"', $markup); // compensate for Drupal pigheadedness
  if ($href) {
    if (strpos($href, 'http://') !== FALSE or substr($href, 0, 1) == '/') $href = "document.location.href='$href';";
    $name = 'name="op"';
    $markup = str_replace($name, ($type == 'local' ? '' : "$name ") . "onclick=\"$href\"", $markup);
//  if ($href) $markup = "<a href=\"$href\" title=\"$title\">$markup</a>";
  }
  return $markup;
}

function sms_form($form, &$form_state) {
  $title = form_field('item', t('Test SMS'));
  $my_number = variable_get('last SMS number used');
  $number = form_field('textfield', t('From (number): '), '', dft($my_number));
  $message = form_field('textfield', t('Message: '));
  $submit = form_field('submit', '', '', t('Send it!'));

  drupal_add_js("document.getElementById('edit-message').focus();", array('type'=>'inline', 'scope'=>'footer'));  
  $form = compact(u\ray('title number message submit'));
  $form['#attributes']['class'] = array('labeled');
  return $form;  
}

function sms_form_submit($form, $form_state) {
  global $sms_devel; $sms_devel = TRUE;
  extract($form_state['values'], EXTR_PREFIX_ALL, 'my');
  $my_number = u\formatPhone($my_number, '+n');
  variable_set('last SMS number used', $my_number);
  \rsms_sms_incoming('process', $my_number, $my_message);
}

/**
 * Set or report next step in form's workflow
 * Syntax:
 *   form_step($form_state, $info, 'id of next step') -- sets the next step
 *   form_step($form_state, $zot, NULL) -- sets the next step to none
 *   form_step($form_state, $info) -- gets the name of the next step and (in $info) any parameters
 * @param string $next_step: a step id ('' means ignore argument, NULL means no next step)
 * @param array $info: associative array of parameters to next step (passed or returned)
 * @return string: the next step ('' if none)
 * @see also step_one() and previous_state()
 */
function form_step(&$form_state, &$info = NULL, $next_step = '') {
  if (is_null($next_step)) { // set the next step (and args) to none
    $form_state['storage'] = NULL;
    $form_state['rebuild'] = TRUE;
  } elseif ($next_step) { // set the next step (and args)
    $form_state['storage']['previous'][$next_step] = $form_state;
    $form_state['storage']['step'] = $next_step;
    $form_state['storage']['values'] = $info;
    $form_state['rebuild'] = TRUE;
  } else $info = @$form_state['storage']['values']; // return args
  
  return @$form_state['storage']['step']; // return the next step
}

function step_one(&$form_state) {form_step($form_state, $zot, NULL);}

function previous_state(&$form_state, $message = '', $args = array()) {
  if ($message) {
    $form_state = $form_state['storage']['previous'][$form_state['storage']['step']];
    $form_state['storage']['say'] = array($message, $args);
  } else return @$form_state['storage']['say'] ?: '';
}

/**
 * Asks the user to confirm, calls the caller with the answer.

function ok_form($form, &$form_state) {
  //extract($form_state['input'], EXTR_PREFIX_ALL, 'in');
  extract(form_args('ok'), EXTR_PREFIX_ALL, 'in');
  $form = array(
    'title' => form_field('item', @$in_title),
    'question' => form_field('item', @$in_question),
    'confirm' => form_field('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => form_field('submit', NULL, t("Don't do it"), 'Cancel'),

    'detour_to' => form_field('hidden', @$in_detour_to),
    'return_with' => form_field('hidden', @$in_return_with),

    '#skip_duplicate_check' => TRUE, // Confirm form fails duplication check, as the form values rarely change -- so skip it.
    '#attributes' => array('class' => u\ray('rweb confirmation')),
  );

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

function ok_form_submit($form, &$form_state) {
  extract($form_state['values']);
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
function sure_form(&$form_state, $title = 'Please Confirm') {
  if (!isset($form_state['confirm'])) return FALSE;
  
  $form = array(
    'title' => form_field('item', $title),
    'question' => form_field('item', $form_state['confirm']),
    'confirm' => form_field('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => form_field('submit', NULL, t("Don't do it"), 'Cancel'),
  );
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => u\ray('rweb confirmation'));
//  $form['#validate'] = 'rCredits\\Web\\sure_form_validate';
//  $form['#submit'] = 'rCredits\\Web\\sure_form_submit';
//  $form_state['rebuild'] = TRUE;

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

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
function svar($name) {
  $name = 'rcredits_' . $name;
  $args = func_get_args();
  if (count($args) == 1) return @$_SESSION[$name]; else $_SESSION[$name] = $args[1];
}

/**
 * Figure out who the user means, offering choices to disambiguate
 * @param string $who: what the user typed
 * @param string $field_name: name of the field the user typed in
 * @param array $info: field values to be stored, then reinstated when the page is refreshed to show the choices
 * @param boolean $allow_ALL: all $who to be "all" (meaning all employees)
 * @param boolean $create: whether to create the person/company if it doesn't exist
 * @param string $self_message: index to error message to give if user self-refers
 * @return:
 *   NULL if there is an error
 *   acct of the identified person
 *   no return (refreshes page) if ambiguous
 */
function whois($who, $field_name, $info, $create = TRUE, $allow_ALL, $self_message = 'no self-trading') {
  $result = api\identify($who, $create, $allow_ALL, $self_message);
  if ($result == 'ALL' or u\isAcct($result)) return $result;

  list ($message, $args, $choices) = $result;
  foreach ($choices as $uid => $full_name) $choices[$uid] = substr(r\quid($uid), 3) . '   ' . $full_name;
  if (empty($choices)) return say($result, $field_name); else which($choices, $field_name, $info, tt($message, $args));
}
  
/**
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and purpose)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($info, $confirmed) {
  $cuid = r\acct()->id;
  extract($info);
  $tx_type = $op == 'Pay' ? 'payment' : 'charge';
  if (blank_field(compact(u\ray('who amount')))) return FALSE;
  if ($goods and trim($purpose) == '') return say('missing purpose', 'purpose');
  if (!$goods) $purpose = 'cash'; // tell api\transfer it's not for goods (should be the only place this string occurs)
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', 'amount');
  if (!($acct = whois($who, 'who', $info, TRUE, $tx_type == 'payment'))) return FALSE; // allow "ALL"
  list ($message, $args, $confirm) = api\transfer($tx_type, $acct, $amount, $purpose, !u\forCash($purpose), $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (@$args['success']) return say($message, $args);
  return say($message, $args, 'amount');
}

function menu($title, $type, $function = NULL, $function_args = array(), $access = 'member', $other = array()) {
  global $menu_weight;
  $menu_weight += !@$menu_weight ?: 1;
  
  if (function_exists($local_func = "rCredits\\Web\\$function")) $function = $local_func;
  if ($function == 'drupal_get_form') {
    $form_func = $function_args[0];
    if (function_exists($form_func = "rCredits\\Web\\{$form_func}_form")) $function_args[0] = $form_func;
  }
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
//    'access callback' => TRUE,
    'access callback' => 'rCredits\\Web\\web_access', // don't use api\access directly (rweb needs to remember $cuid)
    'access arguments' => array($access),
    'weight' => @$menu_weight - 1,
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  ), $other); // $other overwrites, if there is a conflict
}

function form_field($type, $title = '', $description = '', $other = array(), $options = NULL, $ajax = array()) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  if (empty($other) and ($type == 'item' or $type == 'hidden')) $other['value'] = $title;
  $field = u\prefixKeys('#', array_merge(compact(u\ray('type title description options')), $other));
  return $field;
}

/**
 * Say whether user has submitted the confirmation form (as opposed to the primary form)
 * The "v" verion is called from _validate, the "s" version from _submit.
 */
function confirming_s(&$form_state) {return ($form_state['rebuild'] = isset($form_state['confirm']));}
function confirming_v(&$form_state) {
  if(@$form_state['confirm']) {
    extract($form_state['values']);
    $form_state = $form_state['submitted_state'];
    if ($op == 'Cancel') {
      $form_state['rebuild'] = TRUE;
      say('op canceled'); // not an error message, else confirmation form persists
    }
    return TRUE;
  }
  $form_state['submitted_state'] = $form_state;
  return FALSE;
}

function confirm($message, &$form_state) {return ($form_state['confirm'] = $message);}

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
  u\log($error_field ? 'say' : 'say error', compact('message'));
  return FALSE;
}

function setup_globals() {
  global $user, $channel;
  if (@$channel) return; // already setup
  $agent = @$user->uid;
  $cuid = svar('cuid') ?: $agent;
  r\acct::setDefault(r\acct($cuid, $agent)); // set default for r\acct()
  $channel = TX_WEB;
}

function web_access($access) {
  setup_globals(); // remember who we are (the only reason for this function)
  return api\access($access);
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs);}
function block_def($info, $extra = array()) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(u\ray('subject content'));}

function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($default_value) {return compact('default_value');}
function weight($weight) {return compact('weight');}
function attrib($attribs) {return array('attributes' => $attribs);}
function autocomplete($path) {return array('autocomplete_path' => "autocomplete/$path/" . r\acct()->id);}
function required() {return array('required' => TRUE);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
