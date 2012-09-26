<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use \rCredits\SMS as rSMS;
use \rCredits\API as api;
use \rCredits\Utilities as u;
use \rCredits as r; // get rid of this
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
    var yes=$("#" + field).value;
    $("#" + field).value = !yes;
    $("#" + field + "-YES").css("display", yes ? "none" : "inline-block");
    $("#" + field + "-NO").css("display", !yes ? "none" : "inline-block");
  }
');

$GLOBALS['Web messages'] = array(
//  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"

  // reports
  'report share change' => t('Sharing permission for account "@account_name" has been changed to "@permission".'),
  'report new share' => t('You are now sharing this account with @who'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account ID is @qid. Your@asif starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in.</p><p>Pretend to buy and sell just as you would in a typical month. We need that information, to make sure everyone has plenty of places to receive and spend their rCredits.</p><p>Experiment! Have fun! (and please let us know if you run into any glitches)'),
  'info saved' => t('Your information has been saved.'),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  
  // errors
  'missing what' => t('For buying or selling actual goods and services, you must include a description. Otherwise select "cash/loan/etc.". Rebates and bonuses are intended as rewards for productive economic activity in rCredits. Note: for everyone\'s protection, the rCredits software automatically detects and penalizes attempts to "game" the system.'),
  'missing field' => t('Missing a required field: @field'),
  'no txs' => t('There are no transactions in that period.'),
  'no shares' => t('This account is not yet shared with anyone.'),
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
  'request & demand' => t('Your standing request for rCredits is <b>@request</b><br>Total demand for rCredits overall is <b>@demand</b>'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm share permission' => t('Are you sure you want to change the selected shares to "@permission"?'),
  'confirm delete cell' => t('Are you sure you want to remove the selected cell phones from this account?'),
  'already shared' => t('You are already sharing with that account. If you want to change the permissions, check the box next to that account, choose the new permissions, and click "Go".'),
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
  if ($arg == 'reset') {
    r\reset();
    $uid = variable_get('rcredits_community_uid') + 1;
    $uid2 = $uid + 1; $uid3 = $uid2 + 1;
    $pass = '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae';
    $sql = <<<EOF
      INSERT INTO users (uid, name, pass, mail, short_name, full_name, status, postal_code) VALUES
      ($uid, 'uten', '$pass', 'uten@ex.com', 'uten', 'U Ten', 1, '01301'),
      ($uid2, 'btwo', '$pass', 'btwo@ex.com', 'btwo', 'B Two', 1, '01301'),
      ($uid3, 'cstore', '$pass', 'cstore@ex.com', 'cornerstore', 'Corner Store', 1, '01301');
      
      INSERT INTO sms_user (number, uid, status) VALUES
      (10, $uid, 2),
      (2, $uid2, 2);
EOF;
    r\db_q($sql);
    api\give_signup_incentive($uid, TX_WEB);
    api\give_signup_incentive($uid2, TX_WEB);
    r\db_q(file_get_contents(__DIR__ . '\..\sql\r_companies-insert.sql'));
  }
  
  if ($arg == 'clear_sessions') r\db_q('TRUNCATE table SESSIONS');
  
  drupal_set_message("Doing rCredits Util '$arg'...");
  drupal_goto('handy');
}

function handy_links() {
  global $base_path;

  $links = array(
    'rCredits/util/modal|Test modal form|',
    'admin/smsframework/devel|Simulate an SMS transaction|Simulate SMS',
    'admin/config/development/testing|Test|Test',
    'sites/all/modules/rcredits/rsms/gherkin/compile.php|Compile tests|Compile tests',
    'deletetests.php|Delete old tests|Delete old tests',
    'rCredits/util/reset|Reset rCredits|',
    'rCredits/util/clear_sessions|Clear Sessions|',
    
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
    $result .= <<<EOF
<div><a href="{$base_path}$url" title="$title">$label</a></div>
EOF;
  }
  return "<style>div.develw div {display:inline; margin-right:20px;}</style><div class='develw'>\n$result</div>";
}

function show_form($function, $arg1 = '', $arg2 = '', $arg3 = '', $arg4 = '') {
  global $cid;
  if (!$cid) u\cid('', TX_WEB); // in case of forms that require no permissions
  u\log('rWeb', __FUNCTION__, compact(u\ray('function arg1 arg2 arg3 arg4')));
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  $form = drupal_get_form("rCredits\\Web\\{$function}_form", $arg1, $arg2, $arg3, $arg4);
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
}

/**
 * Offer choices for user to choose from
 * This form is called only as a detour from another form. 
 * It could be presented as an overlay, using leanmodal.
 *
function which_form($form, &$form_state) {
  if (svar('detour_via') != 'which') return NULL;
  extract('detour_args', EXTR_PREFIX_ALL, 'my');
  $onchange = "this.form.elements[$my_which_field].value=this.options[this.selectedIndex].text; this.form.submit();";
  $title = form_field('item', t('Which one do you mean?'),
  $choice = form_field('select', '', '', attrib(compact('onchange')), $my_choices);
*/

/**
 * Offer choices for user to choose from
 * 1. If called with all arguments, the arguments are stored in svar('pop') and the form is reloaded
 * 2. If called as which('info'), this function returns $info from svar('pop')
 * 3. If called with NO arguments, this function returns a fieldset to insert as a popup layover div into
 * the form awaiting the choice. Clicking on a choice fills in the missing value and submits the form.
 * This function expects svar('pop') to contain:
 *   $form: the form name
 *   $choices: an array of choices
 *   $result_field: name of field that gets the choice
 *   $question: the question to ask
 */
function which($choices = '', $result_field = '', $info = '', $question = 'Which one?') {
  $form = basename(current_path());
  
  if (is_array($choices)) {
    svar('pop', compact(u\ray('form choices result_field info question')));
    drupal_goto($form); // reload page
  }

  if (!is_array($pop = svar('pop'))) return array();
  extract(svar('pop'), EXTR_PREFIX_ALL, 'pop');
  if ($form != $pop_form) return array();
  if ($choices == 'info') return $pop_info;
  svar('pop', NULL);
  
  $onchange = "this.form.elements['$pop_result_field'].value=this.options[this.selectedIndex].text; this.form.submit();";
  $subtext = form_field('item', $pop_question);
  $choice = form_field('select', '', '', attrib(compact('onchange') + array('size' => 12)), $pop_choices);
  $popup = form_field('fieldset', t('Which One?'), '', array('id' => 'popup')) + compact(u\ray('subtext choice'));
  return compact('popup'); // later return a fieldset
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
 * @param string $role: 'payer', 'payee', 'industry', or '' (other), depending on the role of the user
 * @todo: ditch this altogether or make it efficient with separate rcredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the autocomplete URL to the backend? privacy is not crucial here
 * @todo: fix Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($string = '', $role = '') {
//  return drupal_json_output(array('zot' => "cid=$cid saved=".svar('cid')));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\short_name($string) . '%';
//  $string = str_replace(' ', '%', \db_like(" $string "));
  
  $table = r\t_table();
  $cid = u\cid(); // global fails here
  
  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
    $selection = <<<EOF
      u.uid, u.full_name AS display, u.mail AS email 
      FROM users u LEFT JOIN $table t ON t.$other=u.uid 
      WHERE (u.short_name LIKE :string OR u.mail LIKE :string)
      AND (t.xid IS NULL OR t.$role=$cid) ORDER BY t.created DESC, u.full_name
EOF;
  } elseif ($role == 'industry') {
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\short_name(" $string0 ", '%');
    $selection = <<<EOF
      full_name AS display FROM users WHERE account_type!=:R_PERSONAL AND short_name LIKE :string
      UNION SELECT industry AS display FROM r_industries 
      WHERE industry LIKE :string $soundex
      ORDER BY display
EOF;
  } else $selection = 'full_name AS display FROM users WHERE short_name LIKE :string ORDER BY full_name';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection LIMIT 10";

  $result = r\db_q($sql, compact('string'));
  $matches = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    if ($display == R_REGION_NAME) continue; // never list the community itself
    $email = @$email ? " ($email)" : '';
    $matches[$display] = check_plain("$display$email");
  }
  drupal_json_output($matches);
}

function home_form($form, &$form_state) {
  global $cid;

  $account = \user_load($cid);
  $is_company = api\access('company');
  
  $stagelist = 'rTrader Steward Partner Member';
  if ($is_company) $stagelist = str_replace('Steward ', '', $stagelist);
  $stages = u\ray($stagelist);
  foreach ($stages as $stage) if (api\access($stage)) break;

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
  
  extract(api\credit_info()->fancy, EXTR_PREFIX_ALL, 'my');
  
  $form = array(
    'title' => form_field('item', t('Overview of Your rCredits Account')),
    'subtext' => basic_account_info(),
    'account_name' => form_field('item', t('Account Name:'), '', $account->full_name),
    'account_id' => form_field('item', t('Account ID:'), '', r\qid()),
    'account_type' => form_field('item', t('Account type:'), '', $GLOBALS['account types'][$account->account_type+0]), // +0 for admin
    'stage' => form_field('item', t('You are:'), '', $now[$stage]),
    'future' => $future[$stage] ? form_field('item', t('Future milestones:'), '', $future[$stage]) : NULL,
    'steps' => form_field('item', t('Next Steps:'), '', t('<div id="next-steps" class="labeled-block"><ol><li>Do at least three realistic "As-If" Payments to participating businesses, so the system knows where you will be able to spend your rCredits.</li><li>Do at least one As-If Charge, so the system knows where you will get your rCredits.</li><li>Try some of the other rCredits features.</li><li>Read or talk to other participants until you are confident that you understand how rCredits works.</li></ol></div>')),
    'balance' => form_field('item', t('Balance:'), '', $my_balance),
    'credit_limit' => form_field('item', t('Credit limit:'), '(this is how far in debt you can go &mdash; for up to 30 days)', u\format_amount(-$account->min_balance)),
    'rewards_to_date' => form_field('item', t('Rewards to date:'), '', $my_rewards),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function account_form($form, &$form_state) {
  global $cid;
  if (!($accounts = api\account_choices())) return FALSE;
  $choices = array();
  foreach ($accounts as $uid => $zot) {
    $balance = u\format_amount(api\credit_info(compact('uid'))->balance);
    $choices[$uid] = api\user_field('full_name', $uid) . ": $balance";
  }

  $qid = r\qid(); // $credit_id = api\user_field('credit_id', $cid);
  $full_name = api\user_field('full_name', $cid);
  $onchange = "this.form.submit();";
  $account = form_field('select', t('Account:'), t('(Select an account)'), 
    dft($cid) + attrib(compact('onchange')), $choices);
  $one_account = form_field('item', t('Account') . ": <span>$full_name ($qid)</span>");
  $one_balance = form_field('item', t('Current Balance') . ": <span>$balance</span>");
  $submit = form_field('submit');
  $form = count($choices) == 1 ? compact(u\ray('one_account one_balance')) : u\prefix_keys('account_', compact(u\ray('account submit')));

  //  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function account_form_submit($form, &$form_state) {
//  global $cid;
  extract($form_state['values']);
//  $old_uid = $cid;
  svar('cid', u\cid($account_account)); // this gets checked ONLY in rweb_access()
//  if (api\access('can create transactions', $old_uid) != api\access('can create transactions', $account_account)) menu_rebuild();
}

/**
 * Display the member company's profile (no editable fields here)
 */
function profile_form($form, &$form_state, $company) {
  $uid = api\user_field('uid', 'short_name=:company', compact('company'));
  if (!$uid or \user_load($uid)->account_type == R_PERSONAL) return say('no such company', 'zot');
  extract((array) \user_load($uid));
  $pic = profile_picture($uid, TRUE);
  $country = u\country_abbrev($country);
  if ($website) $website = "<a href='http://$website' target='_blank'>$website</a>";
  if ($phone) $phone = 'Phone: ' . u\format_phone($phone, '+');
  if ($fax) $fax = ($phone ? '&nbsp; * &nbsp;' : '') . 'Fax: ' . u\format_phone($fax, '+');
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = r\db_q($sql, compact('uid'))->fetchAllKeyed();
  $cats = empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats));

  $markup = <<<EOF
  <div id='member-picture'>$pic</div>
  <div id='member-details'>
    <div id='member-contact'>
    <h2>$full_name</h2>
    $address<br>
    $city, $state $postal_code $country<br><br>
    $phone$fax<br>
    $website<br><br>
    </div>
    <div id='member-description'>$description</div>
    <div id='member-categories'>$cats</div>
  </div>
EOF;

  $form = array(
    'all' => form_field('item', $markup),
  );
  return $form;
}

function profile_picture($uid, $with_markup = FALSE) {
  global $userid;
  $account = \user_load($uid);
  $data = $account->data;
  $markup = str_replace('styles/thumbnail/public/', '', \theme('user_picture', compact('account')));
  if ($markup) {
    $a = explode('"', $markup);
    $url = $a[api\access('administrator', $userid) ? 7 : 3]; // Drupal probably has an easier way to do this
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
  if ($info = which('info')) extract($info, EXTR_PREFIX_ALL, 'my');
  $type_low = strtolower($type);
  $role = $type == 'Pay' ? 'payer' : 'payee';
  //$onchange = "nixwhat();";
  
  $form = array(
    'title' => form_field('item', $type),
    'who' => form_field('textfield', $type . ' whom:', t('Type a name, id, email, or phone'), dft(@$my_who ?: $company) + autocomplete($role)),
    'amount' => form_field('textfield', 'Amount: $', t('Amount to ') . $type_low, dft(@$my_amount)),
    'goods' => form_field('radios', '', '', 
      dft(isset($my_goods) ? $my_goods : 1) + attrib(compact('onchange')), 
      array(t('cash/loan/etc.'), t('goods & services'))),
    'what' => form_field('textfield', 'Purpose:', t('Description of goods and services or purpose'), dft(@$my_what)),
//    'what' => form_field('textfield', 'Purpose:', t(''), dft(@$my_what)),
    $type_low => form_field('submit', NULL, t('Submit button'), $type),
//    'charge' => form_field('submit', NULL, t('Submit button'), 'Charge'),
  );
  $form['#attributes']['class'] = array('labeled');

  $js = <<<EOF
    function nixwhat() {
      var c0=document.getElementById('edit-goods-0');
      jQuery('.form-item-what').css('display', c0.checked ? 'none' : 'block');
    }
//    nixwhat();
    document.getElementById('edit-who').focus();
EOF;

  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  
  return $form;
}

function tx_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;
  if ($confirm = transfer($form_state['values'], FALSE)) confirm($confirm, $form_state);
}

function tx_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  transfer($form_state['values'], TRUE);
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
    if (!($uid = r\qid(@$my_company))) return say('bad account id', 'company');
//  print_r("confirmed=$confirmed uid=$uid company=$my_company logged=".user_is_logged_in());
    $name = api\user_field('full_name', $uid); 

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
    if (u\valid_url(@$my_return_to)) return array(form_field('item', "<a href='$my_return_to'>Click here</a> to return to the $name website."));
    drupal_goto('');
  }

  $form = array(
    'title' => form_field('item', t('Confirm Payment')),
    'xwho' => form_field('textfield', 'to:', t('Pay whom?'), disabled($name)),
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

/*
function example_store_form($form, &$form_state) {
  if (user_is_logged_in()) return array('item' => form_field('item', t('You must log out, to test the Example Store')));
  
  global $base_url;
  $form = array(
    'title' => form_field('item', '<i>Corner Store\'s</i><br><b><small>Awesome Apple Pie<br> &nbsp; &nbsp; &nbsp; &nbsp;only $23</small></b>'),
    'item' => form_field('hidden', 'Awesome Apple Pie'),
    'amount' => form_field('hidden', 23),
    'company' => form_field('hidden', 'Z9AAAAAC'), // (use Corner Store online)
    'verb' => form_field('hidden', t('Pay')), // the default
    'code' => form_field('hidden', '29c8ad9f8d-908'),
    'submit' => form_field('image_button', NULL, t('Submit button'), array('src' => $base_url . '/images/rbutton-buynow.png')),
  );
  $form['#method'] = 'POST';
  $form['#action'] = $base_url . '/pay-with-rcredits';
  return $form;
}
*/
/*
function charge_form($form, &$form_state) { // almost exactly like payment
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = u\prefix_keys('charge_', array(
    'title' => form_field('item', t('Charge / Invoice')),
    'amount' => form_field('textfield', '$', t('Amount to charge')),
    'who' => form_field('textfield', 'from:', t('Charge whom?')),
    'goods' => form_field('radios', '', '', 
      dft(1) + attrib(array('onchange' => "nixwhat('charge');")), 
      array(t('cash/credit/gift/etc.'), t('goods & services'))),
    'what' => form_field('textfield', 'for:', t('Description of goods and services')),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Charge'),
  ));
  return $form;
}

function charge_form_validate($form, &$form_state) {payment_form_validate($form, $form_state, 'charge');}
function charge_form_submit($form, &$form_state) {payment_form_submit($form, $form_state, 'charge');}
*/

/**
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 */
function exchange_form($form, &$form_state) {
  global $cid;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $request = u\format_amount(api\user_field('demand', $cid));
  $demand = api\credit_info()->fancy['total_demand'];

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
  global $cid;
  if (confirming_v($form_state)) return;

  extract($form_state['values']);
  if (blank_field(compact('amount'))) return;
  list ($message, $args, $confirm) = api\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cid, FALSE, TX_WEB);
  if ($confirm) return confirm(tt($message, $args), $form_state);
  if (!@$args['success']) say($message, $args, 'amount');
}

function exchange_form_submit($form, &$form_state) {
  global $cid;
  if (confirming_s($form_state)) return;

  extract($form_state['values']);
  list ($message, $args, $confirm) = api\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $cid, TRUE, TX_WEB);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($op == 'Get rCredits') {
    if ($amount < R_MIN_DEMAND) say('demand too small');
    if (!api\access('partner')) say('demand not yet');
  }
}

/**
 * @NOTE: The country and state drop down require a change in form.inc (see form.inc.patch)
 * @todo: substitute get_record for user_load (throughout this module?) and save data in backend (same for login and register)
 */
function contact_info_form($form, &$form_state) {
  global $cid;
  api\merge_check(compact(u\ray('short_name website phone fax'))); // check before and after
  extract((array) \user_load($cid), EXTR_PREFIX_ALL, 'my'); // can't easily check for plain because roles and sms_user are arrays

  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form = array(
    'title' => form_field('item', t('Contact Information')),
    'subtext' => basic_account_info(),
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), dft($my_full_name) + required()),
    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\format_phone($my_phone): '') + required()),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)),
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))), // can't use required() here
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), dft($my_postal_code)+ required()),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)), // can't use required() here
    'city' => form_field('textfield', t('City:'), t(''), dft($my_city)+ required()),
    'address' => form_field('textfield', t('Address:'), t('Mailing address'), dft($my_address)+ required()),
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
  if ($phone and !u\format_phone($phone, '+n')) say('bad phone', 'phone');
  if ($fax and !u\format_phone($fax, '+n')) say('bad phone', 'fax');
}

function contact_info_form_submit($form, &$form_state) {
  global $cid;
  $account = \user_load($cid);
  extract($info = $form_state['values'], EXTR_PREFIX_ALL, 'my');
  $info['website'] = str_replace('http://', '', strtolower(@$my_website));
  if ($my_phone) $info['phone'] = u\format_phone($my_phone, '+n');
  $info['short_name'] = u\short_name($my_full_name);
  user_save($account, $info);
  api\merge_check(compact(u\ray('short_name website phone fax'))); // check before and after
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
  global $cid;
  extract((array) \user_load($cid), EXTR_PREFIX_ALL, 'my'); // can't easily check for plain because roles and sms_user are arrays

  if (!$my_bank_account_name) $my_bank_account_name = $my_full_name;
  
  $form = array();
  if (api\user_field('country') == 'United States') {
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
  extract(u\trim_all($form_state['values']));
  if (!u\valid_name($bank_account_name, FALSE, FALSE)) say('bad name', 'bank_account_name');
  if (!preg_match('/^(|[0-9]{9})$/', $routing_number)) say('bad routing number', 'routing_number');
  if (!preg_match('/^[0-9]{3,20}$/', $bank_account_number)) say('bad account number', 'bank_account_number');
}

function bank_form_submit($form, &$form_state) {
  global $cid;
  extract($form_state['values']);
  $account = \user_load($cid);
  if ($account->country == 'United States') $bank_account_number = "USkk$routing_number$bank_account_number";
  user_save($account, compact(u\ray('bank_account_name bank_account_number')));
  say('info saved');
}

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function relation_form($form, &$form_state) {
  global $cid;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $account_identifiers = api\account_name($cid);
  $header = u\ray('Person,Permission,Employee?,Owner?');
//  $actions = $GLOBALS['share permissions']
  
  $sql = <<<EOF
    SELECT s.relid, u.name as username, u.full_name, s.permission, s.is_employee, s.is_owner
    FROM r_relations s RIGHT JOIN users u ON u.uid = s.individual_uid
    WHERE s.main_uid=:cid
    ORDER BY username
EOF;
  $result = r\db_q($sql, compact('cid'));
  $rows = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $permission = ''; // dropdown($permission, $GLOBALS['share permissions'], $relid);
//    $qid = r\qid($uid);
    $rows["x$relid"] = array($full_name, $permission, toggle($is_employee, $relid), toggle($is_owner, $relid));
  }

  $help = t("Use this form to mark someone as an employee or owner or to give someone access to this account.<br><br>For example, for a company account, you will want to give access to the employees who are authorized to buy or sell on behalf of the company. Then you will know who is responsible for any particular transaction.<br><br>Similarly, for your personal account, you may want to give \"read\" access to your accountant.<br><br>Individuals marked as employees are the ones who will get paid in rCredits when you use the Pay form to pay \"employees\".");
  $subtext = '<span id="account-label">' . t('Current Account') . "</span></b><span id='account-identifiers'>$account_identifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  $table = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => form_field('item', t('Relations')),
    'subtext' => form_field('item', $subtext),
//    'shares' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no shares')), $list),
    'change' => form_field('select', t('Change settings for the selected individuals:'), '', array(), $actions),
    'go' => form_field('submit', NULL, t('Submit button'), 'Go'),

    'new_share' => form_field('textfield', t('Share this account with another user:'), t("name, id, email, or phone"), autocomplete('')),
    'new_permission' => form_field('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'new_go' => form_field('submit', NULL, t('Submit button'), 'Add'),
  );
  drupal_add_js(R_TOGGLE_JS, array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function toggle($name, $yes, $id) {
  $path = current_path();
  $field_name = "$name-$id";
  $hidden = array($field_name => form_field('hidden', '', '', $yes, array('id' => $field_name)));
  $field = \render($hidden);
  $b1 = button(' X ', "javascript:toggle('$field_name');", t('Change to "NO"'), 'local');
  $b2 = button(' &#10004; ', "javascript:toggle('$field_name');", t('Change to "YES"'), 'local');
  if (!$yes) list ($b1, $b2) = array($b2, $b1);
  return <<<EOF
    <div id='$field_name-YES' style='display:inline-block;'>$b1</div>
    <div id='$field_name-NO' style='display:none;'>$b2</div>
EOF;
}

function relation_form_validate($form, &$form_state) {
  global $cid;
  extract($form_state['values']);
  if ($op == 'Go' and no_selection($shares)) {
    if ($new_share) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new share
  }
  if (confirming_v($form_state)) return;
  
  if ($op == 'Go') {
    $permission = $GLOBALS['share permissions'][$change_permission];
    confirm(tt('confirm share permission', compact('permission')), $form_state);
  } else { // $op == 'Add'
    if (blank_field(compact('new_share'))) return;
    $uid = api\identify($new_share, FALSE, 'no self-sharing');
    if (is_array($uid)) return say($uid, 'new_share'); // error returned by identify()
    $already = r\db_lookup('1', 'r_relations', 'main_uid=:cid AND individual_uid=:uid', compact('cid', 'uid'));
    if ($already) return say('already shared', 'new_share');
    $form_state['values']['individual_uid'] = $uid; // save for submission handling
  }
}

function relation_form_submit($form, &$form_state) {
  global $cid;
  if (confirming_s($form_state)) return;
  $main_uid = $cid;
  
  extract($form_state['values']);
  if ($op == 'Add') {
    $permission = $new_permission;
    $zot = \drupal_write_record('r_relations', compact(u\ray('main_uid individual_uid permission')));
    $who = api\user_field('full_name', $individual_uid);
    return say('report new share', compact('who'));
  }
  
  // $op == 'Go'
  $permission = $GLOBALS['share permissions'][$change_permission];
  foreach ($shares as $key => $uid) if ($uid) {
    $individual_uid = substr($uid, 1); // ignore the leading 'x'
    $where = "WHERE individual_uid=:individual_uid AND main_uid=:main_uid LIMIT 1";
    if ($change_permission) {
      r\db_q("UPDATE r_relations SET permission=:change_permission $where", compact(u\ray('individual_uid main_uid change_permission')));
    } else r\db_q("DELETE FROM r_relations $where", compact(u\ray('individual_uid main_uid')));
    $account_name = api\user_field('full_name', $individual_uid);
    say('report share change', compact(u\ray('account_name permission')));
  }
}

/**
 * @todo: focus on code input
 */
function cell_form($form, &$form_state) {
  global $cid;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only deletion
  if (form_step($form_state, $info) == 'prove') {
    $number = u\format_phone($form_state['storage']['values']['number']);
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

  $sql = "SELECT number FROM sms_user WHERE uid=:cid ORDER BY number";
  $result = r\db_q($sql, compact('cid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(u\format_phone($number));
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
  global $cid;
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
    rSMS\send($number, tt('verification code', compact('nonce')));
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
  $account_types = u\ray('Personal Commercial Non-profit/Government');
  $my_country = $my_state = '';
  extract($form_state['input'], EXTR_PREFIX_ALL, 'my'); // overwrites my_country and my_state with input values, on input error
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form += array(
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), required()),
/*    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\format_phone($my_phone): '')),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)), */
    'account_type' => form_field('select', t('Account type:'), '', dft('Personal'), $account_types),
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))),
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), required()),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)),
    'city' => form_field('textfield', t('City:'), t(''), required()),
/*    'address' => form_field('textfield', t('Address:'), t('Mailing address'), dft($my_address)),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Submit'), */
  );

  drupal_add_js('inc/countries.js', array('type'=>'file', 'scope'=>'header'));
  drupal_add_js("print_country(\"edit-country\", \"$my_country\", \"$my_state\");", array('type'=>'inline', 'scope'=>'footer'));

  $form['#attributes']['class'][] = 'labeled';
  $form['#attributes']['class'][] = 'rweb';
  $form['#validate'][] = 'rCredits\\Web\\register_form_validate';
  $form['#submit'][] = 'rCredits\\Web\\register_form_submit';
}

function register_form_validate($form, &$form_state) {
  extract($form_state['values']);
  $form_state['values']['short_name'] = u\short_name($full_name);
}

function register_form_submit($form, &$form_state) {
  extract($form_state['values']);
  $qid = r\qid($uid); // api\make_credit_id($uid);
// this fails because account is not saved yet  api\merge_check(compact(u\ray('email short_name'))); // email fails anyway because it's not unique
  $balance = u\format_amount(api\give_signup_incentive($uid), TX_WEB);
  say('your account is ready', compact(u\ray('balance qid')));
}

/**
 * Modify the login form.
 * @see also login_followup, below (called from rweb_user_login()).
 */
function login_alter(&$form, &$form_state) {
  $form['tip'] = form_field('item', t('<b>NOTE: </b>If you already created an account using cell texting and have never yet logged in here, start by requesting a "new password" for your email address.'));
  $form['external_input'] = form_field('hidden', serialize($form_state['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';
  $form['#attached']['css'][] = \drupal_get_path('module', 'rweb') . '/rweb.css';

}

function login_followup($edit) {
  global $userid, $cid, $user;
  extract($edit['values']);

  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($external_input));
  u\cid($user->uid, TX_WEB); // set up current account (always start with primary)
  assert ($cid);
  svar('cid', $cid); // this is also set in account_form, but checked ONLY in rweb_access()
  u\log('rWeb', 'login', compact(u\ray('name userid')));
}

function directory_form($form, &$form_state) {
  global $cid;
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: substr(api\user_field('postal_code', $cid), 0, 3);
  
  $list = ($region !== FALSE or $which) ? directory_list($which, $region) : '';
  
  $form = array(
    'title' => form_field('item', t('Find businesses in your region')), // should be 'Find member businesses in your region'
//    'subtext' => form_field('item', t('for a country name or postal code...')),
    'which' => form_field('textfield', t('Search for:'), t('Type part of business name or industry category'), dft($which) + autocomplete('industry')),
    'region' => form_field('textfield', t('Where:'), t('Type the start of your postal code or country name'), dft($region)),
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
  $which = str_replace(' ', '%', \db_like(" $which ")); // allow abbreviations of each word
  $region = \db_like($region) . '%';
  $rows = array();
  $sql = <<<EOF
    SELECT DISTINCT i.industry, u.short_name, u.full_name, u.website FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE account_type!=:R_PERSONAL AND u.$field LIKE :region AND (u.full_name LIKE :which OR i.industry LIKE :which)
    ORDER BY i.industry, u.full_name
EOF;
  $result = r\db_q($sql, compact(u\ray('region which')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]);
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $pay = button('Pay', "pay/$short_name");
    $chg = button('Chg', "charge/$short_name");
//    if ($website) $full_name = "<a href='http://$website' target='_blank'>$full_name</a>";
    $full_name = "<a href='$base_url/member/$short_name'>$full_name</a>";
    $item = "$pay$chg$full_name";
    if (!$headed) $item = "Other: $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return $list ?: t('There are no such businesses listed.');
}

function company_form($form, &$form_state) {
  global $cid;
//  extract((array) \user_load($cid), EXTR_PREFIX_ALL, 'my');
//  $my_description = \check_markup(\user_load($cid)->description);
  $account = \user_load($cid);
  $my_description = $account->description;
  
  $cats = r\db_q('SELECT iid, industry FROM r_industries ORDER BY industry')->fetchAllKeyed();
  $my_cats = r\db_q('SELECT iid FROM r_user_industries WHERE uid=:cid', compact('cid'))->fetchCol();
  $multiple = array('multiple' => TRUE);
  $filtered_html = array('format' => 'filtered_html');
  
  $form = array(
    'title' => form_field('item', t('Company Information')),
    'subtext' => basic_account_info(),
    'name' => form_field('item', t('Company name:'), '', $account->full_name),
    'categories' => form_field('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), $multiple + dft($my_cats), $cats),
    'description' => form_field('text_format', t('Description (What does this company do?)'), t(''), $filtered_html + dft($my_description)),
    'submit' => form_field('submit', NULL, t('Submit button'), t('Submit')),
  );

// (doesn't work well for textarea) $form['#attributes']['class'] = array('labeled');
  return $form;
}

function company_form_validate($form, &$form_state) {
//  extract($form_state['values']);
}

function company_form_submit($form, &$form_state) {
  global $cid;
  extract($form_state['values']);
//  $description = \check_markup($description['value']);
  $description = $description['value'];
  $account = \user_load($cid);
  \user_save($account, compact('description'));

  //*** Begin DBTX
  $dbtx = db_transaction();
  r\db_q('DELETE FROM r_user_industries WHERE uid=:cid', compact('cid')); // out with the old
  foreach ($categories as $iid) {
    r\db_q("INSERT INTO r_user_industries (uid, iid) VALUES (:cid, :iid)", compact('cid', 'iid')); // in with the new
  }
  unset($dbtx);
  //*** End DBTX
  say('info saved');
}

function basic_account_info() {
  global $cid;
  global $base_url;
  $link_text = t('Click here to edit your Basic Account Info');
  return form_field('item', "<a href='$base_url/user/$cid/edit'>$link_text</a>" . t(' (including email and profile picture)'));
}

function button($text, $href = '', $title = '', $type = 'local form-submit') {
  $title = htmlspecialchars($title);
  $markup = render($zot = array($text => form_field('button', '', '', array('value' => $text, 'button_type' => $type))));
  $markup = str_replace('"submit"', '"button"', $markup); // compensate for Drupal pigheadedness
  if ($href) $markup = "<a href='$href' title='$title'>$markup</a>";
  return $markup;
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
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and what)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($info, $confirmed) {
  global $cid;
  extract($info);
  $tx_type = $op == 'Pay' ? 'payment' : 'charge';
  if (blank_field(compact(u\ray('who amount')))) return NULL;
  if ($goods and trim($what) == '') return say('missing what', 'what');
  if (!$goods) $what = 'cash'; // tell api\transfer it's not for goods (should be the only place this string occurs)
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', 'amount');
  if (is_array($result = api\identify($who, TRUE))) {
    list ($message, $args, $choices) = $result;
    if (empty($choices)) return say($result, 'who'); else which($choices, 'who', $info, tt($message, $args));
  } else $other_uid = $result;

  list ($message, $args, $confirm) = api\transfer($tx_type, $other_uid, $amount, $what, $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (@$args['success']) return say($message, $args);
  return say($message, $args, 'amount');
}

function menu($title, $type, $function = NULL, $function_args = array(), $access = 'member', $other = array()) {
  global $menu_weight;
  $menu_weight += !@$menu_weight ?: 1;
  
  if (function_exists($local_func = "\\rCredits\\Web\\$function")) $function = $local_func;
  if ($function == 'drupal_get_form') {
    $form_func = $function_args[0];
    if (function_exists($form_func = "\\rCredits\\Web\\{$form_func}_form")) $function_args[0] = $form_func;
  }
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
//    'access callback' => TRUE,
    'access callback' => 'rCredits\\Web\\web_access', // don't use api\access directly (rweb needs to remember $cid)
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
  $field = u\prefix_keys('#', array_merge(compact(u\ray('type title description options')), $other));
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
  global $cid;
  if (!@$index) return;
  if (is_array($index)) list ($index, $args, $error_field) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $error_field) = array($error_field, $args); // allow either order, for easy 2-param calls
  $message = tt($index, $args);
  if ($error_field) \form_set_error($error_field, $message); else \drupal_set_message($message);
  u\log('rWeb', __FUNCTION__, compact(u\ray('type cid message')));
  return FALSE;
}

function web_access($access, $uid = '') {
  u\cid(svar('cid'), TX_WEB); // remember who we are (the only reason for this function)
  return api\access($access, $uid);
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs, 'Web');}
function block_def($info, $extra = array()) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(u\ray('subject content'));}
function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($value) {return array('default_value' => $value);}
function attrib($attribs) {return array('attributes' => $attribs);}
function autocomplete($path) {return array('autocomplete_path' => "autocomplete/$path");}
function required() {return array('required' => TRUE);}
// UNUSED function check($array, $type = 'plain') {return array_map("check_$type", $array);} // Check an array for plain or markup.
