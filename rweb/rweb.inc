<?php
namespace rCredits\Web; // typically abbreviated as "rWeb"
use \rCredits\SMS as rSMS;
use \rCredits\API as i;
use \rCredits\Utilities as u;
use \rCredits as r; // get rid of this

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

$GLOBALS['Web messages'] = array(
  'atid' => t('#@atid: '), // avoid duplicate verbage, don't say "transaction #"
  'missing what' => t('For buying or selling actual goods and services, you must include a description. Otherwise select "cash/credit/gift/etc.". Rebates and bonuses are intended as rewards for productive economic activity in rCredits. Heads up: the rCredits software automatically detects and penalizes repeated attempts to "game" the system.'),
  'no txs' => t('There are no transactions in that period.'),
  'no shares' => t('This account is not yet shared with anyone.'),
  'no cells' => t('This account has no related cell phones.'),
  "can't change what" => t('You can edit the description of the transaction only if you created it or the transaction is completed.'),
  "can't ok for other" => t('You can okay only transactions that are waiting for YOU to okay them.'),
  "can't ok if done" => t('There is no need to okay an already-completed transaction.'),
  'required field' => t('@field must not be blank.'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad amount' => t('Amount must be a number.'),
  'bad nonce' => t('That is not the right verification code. Try again (start over).'),
  'op canceled' => t('Operation Canceled.'),
  'request & demand' => t('Your standing request for rCredits is <b>@request</b><br>Total demand for rCredits overall is <b>@demand</b>'),
  'demand not yet' => t('You will be notified when there are some rCredits for you to buy with US Dollars.'),
  'confirm share permission' => t('Are you sure you want to change the selected shares to "@permission"?'),
  'confirm delete cell' => t('Are you sure you want to remove the selected cell phones from this account?'),
  'already shared' => t('You are already sharing with that account. If you want to change the permissions, check the box next to that account, choose the new permissions, and click "Go".'),
  'already cell' => t('That cell phone (@number) is already connected to this account.'),
  'cell taken' => t('A cell phone can be connected to only one account. You are already using that cell phone (@number) in connection with account "@account". If you really want to switch that phone to this account, you must first sign in to the other account and release it.'),
  'verify cell' => t('We sent a verification code to your cell phone (@number).<br>Please type that code in the box below.'),
  'report share change' => t('Sharing permission for account @account has been changed to "@permission".'),
  'report new share' => t('You are now sharing this account with @who'),
  'report new cell' => t('Cell number @number is now connected to this account.'),
  'report delete cell' => t('Cell number @number has been removed from this account.'),
  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account id is @credit_id. Your starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in.</p><p>Pretend to buy and sell just as you would in a typical month. We need that information, to make sure everyone has plenty of places to receive and spend their rCredits.</p><p>Experiment! Have fun! (and please let us know if you run into any glitches)'),
  'nothing selected' => t('You did not select anything'),
  'undo incomplete' => t('The undo operation is not yet complete.'),
  'ambiguous other' => t('"@who" is associated with more than one rCredits account:<br>@list'),
  'info saved' => t('Your information has been saved.'),
);

/*
function transact_page() {
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  foreach (u\ray('payment charge exchange') as $one) $$one = render(\rweb_block_view($one));
  return array('#markup' => ('<div id="transact_page">' . $payment . $charge . $exchange . '</div>'));
}
*/

function util($arg) {
  if ($arg == 'reset') {
    r\reset();
    $uid = variable_get('rcredits_community_uid') + 1;
    $uid2 = $uid + 1; $uid3 = $uid2 + 1;
    $pass = '$S$D2YT5TTwLHHbbdO3Zpzi9EPcMt5WSuCTWAO274vjzYXyOxZ9kFae';
    $sql = <<<EOF
      INSERT INTO users (uid, name, pass, mail, short_name, full_name, credit_id, status, postal_code) VALUES
      ($uid, 'uten', '$pass', 'uten@ex.com', 'uten', 'U Ten', 'Z9AAAAAA', 1, '01301'),
      ($uid2, 'btwo', '$pass', 'btwo@ex.com', 'btwo', 'B Two', 'Z9AAAAAB', 1, '01301'),
      ($uid3, 'cstore', '$pass', 'cstore@ex.com', 'cornerstore', 'Corner Store', 'Z9AAAAAC', 1, '01301');
      
      INSERT INTO sms_user (number, uid, status) VALUES
      (10, $uid, 2),
      (2, $uid2, 2);
EOF;
    r\db_q($sql);
    i\give_signup_incentive($uid);
    i\give_signup_incentive($uid2);
  }
  if ($arg == 'clear_sessions') r\db_q('TRUNCATE table SESSIONS');
  
  drupal_set_message("Doing rCredits Util '$arg'...");
  drupal_goto('handy');
}

function handy_links() {
  global $base_path;

  $links = array(
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

function show_form($function, $arg1 = '') {
  i\r_log('WEBout', "{$function}_form", i\be_who());
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  $form = drupal_get_form("rCredits\\Web\\{$function}_form", $arg1);
//  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  $form['#attributes']['class'][] = 'rweb';

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
 * Account Selector form
 * appears above the whole Members Section
 */
function account_form($form, &$form_state) {
  $myid = i\be_who();
  
  if (!($accounts = i\account_choices())) return FALSE;
  foreach ($accounts as $uid => $zot) {
    $balance = u\format_amount(i\credit_info($uid)->balance);
    $accounts[$uid] = i\user_field('full_name', $uid) . ": $balance";
  }

  $credit_id = i\user_field('credit_id', $myid);
  $full_name = i\user_field('full_name', $myid);
  $onchange = "this.form.submit();";
  $account = form_field('select', t('Account:'), t('(Select an account)'), 
    dft($myid) + attrib(compact('onchange')), $accounts);
  $one_account = form_field('item', t('Account') . ": <span>$full_name ($credit_id)</span>");
  $one_balance = form_field('item', t('Current Balance') . ": <span>$balance</span>");
  $submit = form_field('submit');
  $form = count($accounts) == 1 ? compact(u\ray('one_account one_balance')) : u\prefix_keys('account_', compact(u\ray('account submit')));
  
//  $form['#attributes']['class'] = array('labeled');
  $js = <<<EOF
    function nixwhat() {
      var c0=document.getElementById('edit-goods-0');
      jQuery('.form-item-what').css('visibility', c0.checked ? 'hidden' : 'visible');
    }
    nixwhat();
EOF;
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function account_form_submit($form, &$form_state) {
  extract($form_state['values']);
  $old_uid = i\be_who();
  i\be_who($account_account);
  if (i\access('can create transactions', $old_uid) != i\access('can create transactions', $account_account)) menu_rebuild();
}

function tx_form($form, &$form_state, $company = '') {
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = array(
    'description' => form_field('item', t('Pay / Charge')),
    'amount' => form_field('textfield', '$', t('Amount to pay or charge')),
    'who' => form_field('textfield', 'who:', t('Pay or charge whom?'), dft($company)),
    'goods' => form_field('radios', '', '', 
//      dft(1) + array('attributes' => array('onchange' => "nixwhat('payment');")), 
      dft(1) + attrib(array('onchange' => "nixwhat();")), 
      array(t('cash/credit/gift/etc.'), t('goods & services'))),
    'what' => form_field('textfield', 'for:', t('Description of goods and services')),
    'pay' => form_field('submit', NULL, t('Submit button'), 'Pay'),
    'charge' => form_field('submit', NULL, t('Submit button'), 'Charge'),
  );
  $form['#attributes']['class'] = array('labeled');
  drupal_add_js("document.getElementById('edit-amount').focus();", array('type'=>'inline', 'scope'=>'footer'));  
  return $form;
}

function tx_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;
  if ($confirm = transfer($form_state['values'], FALSE)) confirm($confirm, $form_state);
}

function tx_form_submit($form, &$form_state) {
  if (!confirming_s($form_state)) transfer($form_state['values'], TRUE);
}

/**
 * Pay with rCredits
 */
function buy_now_form($form, &$form_state, $action = 'Payment', $verb = 'Pay') {
  global $user;
  if (!user_is_logged_in()) return array(form_field('item', t('Please log in, to pay with rCredits.')));

  if (!@$_SESSION['buy_now']) $_SESSION['buy_now'] = serialize($form_state['input']); // user is buying from his/her own store
  
  extract(unserialize($_SESSION['buy_now']), EXTR_PREFIX_ALL, 'my'); // login_form() stores this
  // NO! Drupal calls form several times. Null it out in login instead. $_SESSION['buy_now'] = NULL;
  $verbs = u\ray('Buy, Contribute, Donate, Pay');
  if (in_array(ucwords(@$my_verb), $verbs)) $verb = ucwords($my_verb);

 
  if ($confirm = sure_form($form_state)) return $confirm;
  if (isset($my_what)) list ($my_company, $my_item) = array($my_who, $my_what);
  if (!isset($my_amount)) $my_amount = $my_company = $my_item = ''; else $my_amount = @number_format($my_amount, 2);

  $disabled = array('disabled' => 1);
  if (!($name = i\user_field('full_name', "credit_id='$my_company'"))) $name = 'Unregistered Company'; 
  $verblower = strtolower($verb);
  
  $form = array(
    'description' => form_field('item', t("Confirm $action")),
    'xamount' => form_field('textfield', '$', t("Amount to $verblower"), disabled($my_amount)),
    'amount' => form_field('hidden', '', '', $my_amount),
    'xwho' => form_field('textfield', 'to:', t('Pay whom?'), disabled($name)),
    'who' => form_field('hidden', '', '', $my_company),
    'xwhat' => form_field('textfield', 'for:', t('Description of goods and services'), disabled($my_item)),
    'what' => form_field('hidden', '', '', $my_item),
    'goods' => form_field('hidden', '', '', 1),
    'submit' => form_field('submit', NULL, t('Submit button'), $verb),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function buy_now_form_submit($form, &$form_state) {
  tx_form_submit($form, $form_state);
  drupal_goto('members'); // member page when done
}

function example_store_form($form, &$form_state) {
  global $base_url;
  $form = array(
    'description' => form_field('item', '<i>Corner Store\'s</i><br><b><small>Awesome Apple Pie<br> &nbsp; &nbsp; &nbsp; &nbsp;only $23</small></b>'),
    'item' => form_field('hidden', '', '', 'Awesome Apple Pie'),
    'amount' => form_field('hidden', '', '', 23),
    'company' => form_field('hidden', '', '', 'Z9AAAAAC'), // (use Corner Store online)
    'verb' => form_field('hidden', '', '', t('Pay')), // the default
    'code' => form_field('hidden', '', '', '29c8ad9f8d-908'),
    'submit' => form_field('image_button', NULL, t('Submit button'), array('src' => $base_url . '/images/rbutton-buynow.png')),
  );
  $form['#method'] = 'POST';
  $form['#action'] = $base_url . '/members/pay-with-rcredits';
  return $form;
}

function charge_form($form, &$form_state) { // almost exactly like payment
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = u\prefix_keys('charge_', array(
    'description' => form_field('item', t('Charge / Invoice')),
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

/**
 * Exchange USD for rCredits or vice-versa
 * @todo: validate amount
 */
function exchange_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm;

  $myid = i\be_who();
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $request = u\format_amount(i\user_field('demand', $myid));
  $demand = i\credit_info($myid)->fancy['total_demand'];

  $form = array(
    'description' => form_field('item', t('Get rCredits / USD')),
    'request' => form_field('item', tt('request & demand', compact(u\ray('request demand')))),
    'amount' => form_field('textfield', '$', t('Amount to exchange')),
//    'way' => form_field('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
//    'who' => form_field('textfield', 'to/from:', t('Exchange with whom?')),
    'getr' => form_field('submit', '', t('Submit button'), 'Get rCredits'),
    'getusd' => form_field('submit', '', t('Submit button'), 'Get USD'),
  );

  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function exchange_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;

  $myid = i\be_who();
  extract($form_state['values']);
  if (blank_field(compact('amount'))) return;
  list ($message, $args, $confirm) = i\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $myid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $form_state);
  if (!@$args['success']) say($message, $args, 'amount');
}

function exchange_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;

  $myid = i\be_who();
  extract($form_state['values']);
  list ($message, $args, $confirm) = i\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $myid, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($op == 'Get rCredits') {
    if ($amount < i\min_demand()) say('demand too small');
    if (!i\access('partner')) say('demand not yet');
  }
}
/*
function manage_form($form, &$form_state) {
  $form = u\prefix_keys('manage_', array(
    'description' => form_field('item', t('Manage')),
    'transactions' => form_field('submit', '', t('Submit button'), 'Manage Transactions'),
    'account' => form_field('submit', '', t('Submit button'), 'Manage Account'),
    'sharing' => form_field('submit', '', t('Submit button'), 'Share Account'),
  ));
  $form['#attributes']['class'] = 'rweb rblock';
  return $form;
}

function manage_form_submit($form, &$form_state) {
  $op = str_replace(' ', '-', strtolower($form_state['values']['op']));
  drupal_goto("members/$op");
}
*/

/**
 * Manage Transactions
 * @todo put transaction query in rcredits.inc instead (no reference to specific tables here)
 */
function txs_form($form, &$form_state, $edit_xid = '') {
  global $base_path;
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  if ($edit_xid) return tx_edit($edit_xid);

  $myid = i\be_who();
  extract(tx_days(), EXTR_PREFIX_ALL, 'tx');
  $ending = time();
  $starting = strtotime($tx_period == '-1' ? '1jan' : ((1 - $tx_days[$tx_period]) . ' days'), $ending);
  $can_create = i\access('can create transactions');

  $headers = u\ray('#,Date,Name,From you,To you,Status,Purpose,Rewards');
  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'disputed', TX_CONTESTED => 'contested');
  $colors = array('&#10004;' => 'green', 'ok?' => 'orange', 'pending' => 'darkgreen', 'disputed' => 'red', 'contested' => 'red');
  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid = IF(t.payer = $myid, t.payee, t.payer)) AS name,
      IF(t.payer = $myid, t.payer_for, t.payee_for) AS purpose
    FROM r_txs t 
    WHERE t.type NOT IN (:rebate, :bonus)
    AND $myid IN (t.payer, t.payee) 
    AND (t.state IN(:done, :pending, :contested) OR (t.state = :denied AND t.payee = $myid))
    AND t.created >= $starting AND t.created <= $ending
    ORDER BY t.xid DESC
EOF;
  $tx_map = array(
    'done' => TX_DONE, 'pending' => TX_PENDING, 'denied' => TX_DENIED, 'rebate' => TX_REBATE, 'bonus' => TX_BONUS, 'contested' => TX_CONTESTED,
  );
  $txs = r\db_q($sql, $tx_map);
  $list = array();
  while ($row = $txs->fetchAssoc()) {
    extract($row, EXTR_PREFIX_ALL, 'one');
    $tid = i\tid_from_xid($myid, $one_xid);
    $data = unserialize($one_data);
    $is_reward = ($one_type != TX_TRANSFER);
    $reward = @$data[$one_payer == $myid ? 'rebate' : 'bonus'];
    $reward = $reward ? number_format($reward, 2) : '';
    $amount = number_format($one_amount, 2);
    if ($is_reward) list($amount, $reward) = array('', $amount); // show interest and signup bonus as rewards
    list ($from_you, $to_you) = $one_payer == $myid ? array($amount, '') : array('', $amount);
    $state = ($one_state == TX_PENDING and ($one_amount < 0 xor $one_payee == $myid)) ? 'pending' : $states[$one_state];
    $color = $colors[$state];
    $state = "<span style='color:$color;'>$state</span>";
    if ($can_create and !$is_reward) $one_purpose = "<a href='transactions/$one_xid'><img src='{$base_path}images/edit-icon.png' width=20 height=20 border=0 /></a> &nbsp; $one_purpose";
    $list["x$one_xid"] = array($tid, date('d-M', $one_created), $one_name, $from_you, $to_you, $state, $one_purpose, $reward);
  }

  $cols = array(); // note: this all fails because Drupal does not recognize colgroups in tableselect, so pass it to show_form
  foreach (u\array_prefix('tx-', array_merge(array('selector'), $headers)) as $class) { // Drupal will insert a selector column
    $class = str_replace(' ', '-', $class == 'tx-#' ? 'tx-tid' : strtolower($class));
    $cols[] = compact('class');
  }
  $tx_other = array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no txs')); // 'colgroups' => array($cols) fails
  
//  $tasks = array('ok' => 'Approve', 'undo' => 'Dispute/Cancel/Undo', 'edit' => 'Edit description'); // could have used drupal_map_assoc
  $tasks = array('ok' => 'Approve', 'undo' => 'Dispute/Cancel/Undo'); // could have used drupal_map_assoc
  $form = u\prefix_keys('tx_', array(
    'period' => form_field('select', t('Show transactions for the past'), '', dft($tx_period), $tx_days),
    'show' => form_field('submit', NULL, t('Submit button'), 'Show'),

    'summary' => form_field('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => form_field('tableselect', '', '', $tx_other, $list),
    'task' => $can_create ? form_field('select', t('With the selected transactions:'), '', array(), $tasks) : NULL,
    'go' => $can_create ? form_field('submit', NULL, t('Submit button'), 'Go') : NULL,
  ));
  $form['#colgroup'] = $cols; // pass this to show_form()
  return $form;
}

function txs_form_validate($form, &$form_state) {
  extract($form_state['values']);
  if ($op == 'Go' and no_selection($tx_transactions)) return say('nothing selected');
  if (confirming_v($form_state)) return;
  
  if ($op == 'Go' and $tx_task != 'edit') confirm("Are you sure you want to $tx_task the selected transactions?", $form_state);
}

function txs_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  extract($form_state['values']);

  if ($op == 'Show') { // changing the period to show
    $_SESSION['rweb_tx_period'] = $tx_period;
    $form_state['redirect'] = \current_path();
    return;
  }
  
  // $op == 'Go' (fiddling with the old transactions)
  $myid = i\be_who();
  foreach ($tx_transactions as $key => $xid) if ($xid) {
    $xid = substr($xid, 1); // ignore the leading 'x'
    $result = i\last_tx('xid=:xid', compact('xid'));
    extract($result, EXTR_PREFIX_ALL, 'my');
    $atid = i\tid_from_xid($myid, $xid);
    $xname = "tx_transactions[x$xid]";
    if ($tx_task == 'ok') {
      if ($my_payer != $myid) {
        say("atid|can't ok for other", compact('atid'), $xname);
      } elseif ($my_state != TX_PENDING) {
        say("atid|can't ok if done", compact('atid'), $xname);
      } else {
        list ($message, $args) = i\pay_invoice($xid);
        say("atid|$message", $args);
      }
    }
    if ($tx_task == 'undo') {
      list ($message, $args) = i\undo_tx($xid, $myid);
      say("atid|$message", array_merge($args, compact('atid')));
    }
    if ($tx_task == 'edit') {
      if ($my_state != TX_DONE and $my_payer != $myid) {
        say("atid|can't change what", compact('atid'), $xname);
      } else \drupal_set_message('Editing descriptions is not yet possible');
    }
  }
}

function tx_edit($xid) {
  if (!i\access('can create transactions')) return FALSE;

  $my_uid = i\be_who();
  extract(r\db_get_record('r_txs', 'xid=:xid', '*', compact('xid')));
  $amount = u\format_amount($amount);
  $tofrom = $payer == $my_uid ? 'to' : 'from';
  $who = i\user_field('full_name', $payer == $my_uid ? $payee : $payer);
  $role = $payer == $my_uid ? 'payer' : 'payee';
  $purpose = $payer == $my_uid ? $payer_for : $payee_for;
  
  $form = array(
    'description' => form_field('item', t('Edit Description')),
    'review' => form_field('item', "$amount $tofrom $who"),
    'purpose' => form_field('textfield', t('Purpose'), '', dft($purpose)),
    'go' => form_field('submit', NULL, t('Submit button'), 'Update'),
    'xid' => form_field('hidden', '', '', $xid),
    'field' => form_field('hidden', '', '', $role . '_for'),
    
    '#validate' => array('rCredits\\Web\\tx_edit_validate'),
    '#submit' => array('rCredits\\Web\\tx_edit_submit'),
  );
  return $form;
}

function tx_edit_validate($form, &$form_state) {
}

function tx_edit_submit($form, &$form_state) {
  extract($form_state['values']);
  r\set_tx_field($field, $purpose, $xid);
  //r\db_q("UPDATE r_txs SET $field=:purpose WHERE xid=:xid", compact('purpose', 'xid'));
  drupal_goto('members/transactions');
}

function tx_summary($starting, $ending) {
  $myid = i\be_who();
  $previous_ending = strtotime('-1 days', $starting);
  extract($now_info = (array) i\credit_info($myid, TX_DONE, $ending), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) i\credit_info($myid, TX_PENDING, $ending), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) i\credit_info($myid, TX_DONE, $previous_ending), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (u\ray('gross_in_proper gross_out rewards') as $key) {
    $dif[$key] = number_format($now_info[$key] - $old_info[$key], 2);
    $pfancy[$key] = number_format($pending_info[$key], 2);
  }
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pfancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = date('d-M-Y', $starting);
  $ending = date('d-M-Y', $ending);
  $pending_balance = '$' . number_format($now_balance + $pending_gross_in_proper - $pending_gross_out + $pending_rewards, 2);
  $now_balance = '$' . number_format($now_balance, 2);
  $old_balance = '$' . number_format($old_balance, 2);
  
  $classes = u\array_prefix('txsum-', u\ray('one tosign toyou fromsign fromyou rewardssign rewards balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => u\ray("$starting,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => u\ray('Previous balance,,To You,,From You,,Rewards,,New balance')),
    array(
      'id' => 'txsum-now', 
      'data' => u\ray("$old_balance | + | $dif_gross_in_proper | - | $dif_gross_out | + | $dif_rewards | = | $now_balance"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => u\ray("<b>PENDING:</b> | + | $pfancy_gross_in_proper | - | $pfancy_gross_out | + | $pfancy_rewards | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
}

/**
 * @NOTE: The country and state drop down require a change in form.inc, line 2649:
 *   if (is_null($choices = $element['#options'])) return '';
 */
function contact_info_form($form, &$form_state) {
  extract(plain((array) \user_load(i\be_who())), EXTR_PREFIX_ALL, 'my');
  
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form = array(
    'description' => form_field('item', t('Contact Information')),
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), dft($my_full_name)),
    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\format_phone($my_phone): '')),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)),
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))),
//      array('#SKIP_CHECK' => TRUE)),
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), dft($my_postal_code)),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)),
//      array('#SKIP_CHECK' => TRUE)),
    'city' => form_field('textfield', t('City:'), t(''), dft($my_city)),
    'address' => form_field('textfield', t('Address:'), t('Mailing address'), dft($my_address)),
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
  $account = \user_load(i\be_who());
  extract($info = plain($form_state['values']), EXTR_PREFIX_ALL, 'my');
  $info['website'] = str_replace('http://', '', strtolower(@$my_website));
  if ($my_phone) $info['phone'] = u\format_phone($my_phone, '+n');
  $info['short_name'] = u\short_name($my_full_name);
  user_save($account, $info);
  say('info saved');
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

/**
 * Give someone access to the current account.
 */
function share_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $my_uid = i\be_who();
  $account_identifiers = i\account_name($my_uid);
  $headers = u\ray('Account ID,Full name,Email,Permission');

  $sql = <<<EOF
    SELECT u.name as username, credit_id, u.full_name, u.mail as email, s.shared_with, s.permission
    FROM r_shared_accounts s RIGHT JOIN users u ON u.uid = s.shared_with
    WHERE s.shared_uid=:my_uid
    ORDER BY username
EOF;
  $result = r\db_q($sql, compact('my_uid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $permission = $GLOBALS['share permissions'][$permission]; // explicit
    $list["x$shared_with"] = array($credit_id, $full_name, $email, $permission);
  }

  $form = array(
    'description' => form_field('item', t('Sharing')),
    'subtitle' => form_field('item', t('Current Account') . "<span>$account_identifiers</span>"),
    'shares' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no shares')), $list),
    'change_permission' => form_field('select', t('Change permission for the selected shares:'), '', array(), $GLOBALS['share permissions']),
    'go' => form_field('submit', NULL, t('Submit button'), 'Go'),

    'new_share' => form_field('textfield', t('Share this account with another user:'), t("account id, email, or phone.")),
    'new_permission' => form_field('select', '', '', dft(count($GLOBALS['share permissions']) - 1), $GLOBALS['share permissions']),
    'new_go' => form_field('submit', NULL, t('Submit button'), 'Add'),
  );
  return $form;
}

function share_form_validate($form, &$form_state) {
  extract($form_state['values']);
  if ($op == 'Go' and no_selection($shares)) {
    if ($new_share) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new share
  }
  if (confirming_v($form_state)) return;
  
  if ($op == 'Go') {
    $permission = $GLOBALS['share permissions'][$change_permission];
    confirm(tt('confirm share permission', compact('permission')), $form_state);
  } else { // $op == 'Add'
    $myid = i\be_who();
    if (blank_field(compact('new_share'))) return;
    $uid = i\identify($new_share, $myid, FALSE, 'no self-sharing');
    if (is_array($uid)) return say($uid, 'new_share'); // error returned by identify()
    $already = r\db_lookup('1', 'r_shared_accounts', 'shared_uid=:myid AND shared_with=:uid', compact('myid', 'uid'));
    if ($already) return say('already shared', 'new_share');
    $form_state['values']['shared_with'] = $uid; // save for submission handling
  }
}

function share_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  $shared_uid = i\be_who();
  
  extract($form_state['values']);
  if ($op == 'Add') {
    $permission = $new_permission;
    $zot = \drupal_write_record('r_shared_accounts', compact(u\ray('shared_uid shared_with permission')));
    $who = i\user_field('full_name', $shared_with);
    return say('report new share', compact('who'));
  }
  
  // $op == 'Go'
  $permission = $GLOBALS['share permissions'][$change_permission];
  foreach ($shares as $key => $uid) if ($uid) {
    $shared_with = substr($uid, 1); // ignore the leading 'x'
    $where = "WHERE shared_with=:shared_with AND shared_uid=:shared_uid LIMIT 1";
    if ($change_permission) {
      r\db_q("UPDATE r_shared_accounts SET permission=:change_permission $where", compact(u\ray('shared_with shared_uid change_permission')));
    } else r\db_q("DELETE FROM r_shared_accounts $where", compact(u\ray('shared_with shared_uid')));
    $account = i\user_field('full_name', $shared_with);
    say('report share change', compact(u\ray('account permission')));
  }
}

function cell_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only deletion
  if (form_step($form_state, $info) == 'prove') {
    $number = u\format_phone($form_state['storage']['values']['number']);
    $form = array(
      'description' => form_field('item', t('Verify')),
      'subtext' => form_field('item', tt('verify cell', compact('number'))),
      'code' => form_field('textfield', t('Code:')),
      'number' => form_field('hidden', '', '', $number),
      'verify' => form_field('submit', NULL, t('Submit button'), 'Verify'),
    );
    return $form;
  }

  $my_uid = i\be_who();
  $headers = array('Cell Number');

  $sql = "SELECT number FROM sms_user WHERE uid=:my_uid ORDER BY number";
  $result = r\db_q($sql, compact('my_uid'));
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(u\format_phone($number));
  }

  $form = array(
    'description' => form_field('item', t('Cell Phones for this account')),
    'cells' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no cells')), $list),
    'delete' => form_field('submit', NULL, t('Submit button'), 'Remove Selected Numbers'),

    'number' => form_field('textfield', t('Add another cell phone:')),
    'add' => form_field('submit', NULL, t('Submit button'), 'Add'),
  );
  return $form;
}

function cell_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;
  extract($form_state['values']);

  if ($op == 'Verify') {
    if (strtoupper($code) == $_SESSION['nonce']) return;
    previous_state($form_state, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op != 'Add') { // $op == remove
    if (no_selection($cells)) {
      if ($number) $op = 'Add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    }
    confirm(tt('confirm delete cell'), $form_state);
  }

  if ($op == 'Add') {
    $myid = i\be_who();
    if (blank_field(compact('number'))) return;
    $number_internal = u\format_phone($number, '+n');
    if (!$number_internal) return say('bad phone', 'number');
    if ($result = r\db_get_record('sms_user', 'number=:number_internal', '*', compact('number_internal'))) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return say('already cell', 'number');
      extract(r\db_get_record('users', 'uid=:old_uid', 'full_name, credit_id', compact('old_uid')));
      $account = "$full_name ($credit_id)"; // maybe make the previous line and this a function
      if (!i\is_temp_account($old_uid)) return say('cell taken', compact(u\ray('number account')), 'number');
    }
  }
}

function cell_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  $uid = i\be_who();
  
  extract($form_state['values']);
  if ($op == 'Add') {
    if ($info = previous_state($form_state)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    $_SESSION['nonce'] = $nonce = u\nonce(); // don't use POST (user could peek)
    rSMS\send($number, tt('verification code', compact('nonce')));
    return form_step($form_state, compact('number'), 'prove');
  }

  if ($op == 'Verify') {
    step_one($form_state); // no next step (back to original form)
    $status = SMS_USER_CONFIRMED;
    $number = u\format_phone($number, '+n');
    if ($old_uid = r\db_lookup('uid', 'sms_user', 'number=:number', compact('number'))) {
      i\merge_accounts($old_uid, i\be_who());
    } else \drupal_write_record('sms_user', compact(u\ray('uid number status')));
    return say('report new cell', compact('number'));
  }
    
  // $op == remove
  foreach ($cells as $key => $number) if ($number) {
    $number = substr($number, 1); // ignore the leading 'x'
    r\db_q('DELETE FROM sms_user WHERE number=:number LIMIT 1', compact('number'));
    say('report delete cell', compact('number'));
  }
}

/**
 * Modify the user registration form.
 */
function register_form(&$form, &$form_state) {
  $account_types = u\ray('Personal Commercial Non-profit/Government');
  $my_country = $my_state = '';
  extract($form_state['input'], EXTR_PREFIX_ALL, 'my'); // overwrites my_country and my_state with input values, on input error
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $required = array('required' => TRUE);
  $form += array(
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), $required),
/*    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? u\format_phone($my_phone): '')),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)), */
    'account_type' => form_field('select', t('Account type:'), '', dft('Personal'), $account_types),
    'country' => form_field('select', t('Country:'), t(''), dft($my_country) + attrib(compact('onchange'))),
    'postal_code' => form_field('textfield', t('Postal code:'), t(''), $required),
    'state' => form_field('select', t('State:'), t(''), dft($my_state)),
    'city' => form_field('textfield', t('City:'), t(''), $required),
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
//  debug($form_state, '', 1);
  $credit_id = i\make_credit_id($uid);
  $balance = u\format_amount(i\give_signup_incentive($uid));
  say('your account is ready', compact(u\ray('balance credit_id')));
}

function login_form(&$form, &$form_state) {
  $_SESSION['account_choices'] = $_SESSION['being'] = NULL;
//  $uid = $user->uid;
//  r\db_q('DELETE FROM sessions WHERE uid=:uid'
  if (@$form_state['input']['amount']) $_SESSION['buy_now'] = serialize($form_state['input']);
}

function directory_form($form, &$form_state) {
  $my_uid = i\be_who();
  $which = @$_SESSION['directory_which'] ?: '';
  $area = @$_SESSION['directory_area'] ?: substr(i\user_field('postal_code', $my_uid), 0, 3);
  
  $list = directory_list($which, $area);
  
  $form = array(
    'description' => form_field('item', t('Find businesses in your area')), // should be 'Find member businesses in your area'
//    'subtitle' => form_field('item', t('for a country name or postal code...')),
    'which' => form_field('textfield', t('Which:'), t('part of business name or industry type'), dft($which)),
    'area' => form_field('textfield', t('Area:'), t('start of postal code or country name'), dft($area)),
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
  $_SESSION['directory_which'] = $which;
  $_SESSION['directory_area'] = $area;
}

/**
 * Return a formatted, categorized list of businesses
 */
function directory_list($which, $area) {
  $field = is_numeric($area) ? 'postal_code' : 'country';
  $which = "%$which%"; // allow abbreviations
  $area .= '%';
  $account_type = RCREDITS_PERSONAL;
  $rows = array();
  $sql = <<<EOF
    SELECT DISTINCT i.industry, u.short_name, u.full_name FROM users u 
    LEFT JOIN r_user_industries ui ON ui.uid=u.uid
    LEFT JOIN r_industries i ON i.iid=ui.iid
    WHERE account_type!=:account_type AND u.$field LIKE :area AND (u.full_name LIKE :which OR i.industry LIKE :which)
    ORDER BY i.industry, u.full_name
EOF;
  $result = r\db_q($sql, compact(u\ray('area which account_type')));
  while ($row = $result->fetchAssoc()) $rows[] = $row;
  $cat = $headed = $list = '';
  for ($i = 0; $i < count($rows); $i++) {
    extract($rows[$i]);
    if ($industry != $cat) { // new category
      $cat = $industry;
      $headed = (@$rows[$i+1]['industry'] == $industry); // header for 2 or more businesses in the same industry
      $list .= $headed ? ($list ? '</ul>' : '') . "<ul><h2>$cat</h2>\n" : '<li>&nbsp;</li>';
    }
    $item = "<a href='transact/$short_name'>$full_name</a>";
    if (!$headed) $item = "Other: $item <span>($industry)</span>";
    $list .= "<li>$item</li>\n";
  }
  
  if (strpos($list, '<ul>') !== FALSE) $list .= '</ul>';
  return $list ?: t('There are no such businesses listed.');
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
 * Replacement for \confirm_form
 */
function sure_form(&$form_state, $title = 'Please Confirm') {
  if (!isset($form_state['confirm'])) return FALSE;
  
  $form = array(
    'description' => form_field('item', $title),
    'question' => form_field('item', NULL, $form_state['confirm']),
    'confirm' => form_field('submit', NULL, t('Go ahead'), 'Okay'),
    'cancel' => form_field('submit', NULL, t("Don't do it"), 'Cancel'),
  );
  $form['#skip_duplicate_check'] = TRUE; // Confirm form fails duplication check, as the form values rarely change -- so skip it.
  $form['#attributes'] = array('class' => u\ray('rweb confirmation'));

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

/**
 * Transfer funds
 * @param array $info: associative array indexed by field names (op, who, amount, goods, and what)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
function transfer($info, $confirmed) {
  extract($info);
  $tx_type = $op == 'Pay' ? 'payment' : 'charge';
  $myid = i\be_who();
  if (blank_field(compact(u\ray('amount who')))) return NULL;
  if ($goods and trim($what) == '') return say('missing what', 'what');
  if (!$goods) $what = 'cash';
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', 'amount');
  if (is_array($other_uid = i\identify($who, $myid))) return say($other_uid[0], compact('who'), 'who');

  list ($message, $args, $confirm) = i\transfer($tx_type, $myid, $other_uid, $amount, $what, $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (@$args['success']) return say($message, $args);
  return say($message, $args, 'amount');
}

/**
 * Transfer funds
 * @param string $op: 'payment' or 'charge'
 * @param array $info: associative array indexed by field names for who, amount, goods, and what (eg payment_who or charge_who)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)

function transfer($op, $info, $confirmed) {
  $myid = i\be_who();
  foreach (u\ray('who amount goods what') as $one) $$one = $info["{$op}_$one"];
  if (blank_field(compact(u\ray('amount who')), $op . '_')) return NULL;
  if ($goods and trim($what) == '') return say('missing what', $op . '_what');
  if (!$goods) $what = 'cash';
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', $op . '_amount');
  if (is_array($other_uid = i\identify($who, $myid))) return say($other_uid[0], compact('who'), $op . '_who');

  list ($message, $args, $confirm) = i\transfer($op, $myid, $other_uid, $amount, $what, $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (@$args['success']) return say($message, $args);
  return say($message, $args, $op . '_amount');
}
*/

function menu($title, $type, $function = NULL, $function_args = array(), $access = 'member', $other = array()) {
  static $weight;
  if (function_exists($local_func = "\\rCredits\\Web\\$function")) $function = $local_func;
  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
    'access callback' => 'rCredits\\API\\access',
    'access arguments' => array($access),
    'weight' => @$weight - 1,
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  ), $other); // $other overwrites, if there is a conflict
}

function form_field($type, $title = '', $description = '', $other = array(), $options = NULL, $ajax = array()) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  $field = u\prefix_keys('#', array_merge(compact(u\ray('type title description options')), $other));
  return $field;
}

/**
 * Say whether user has submitted the confirmation form (as opposed to the primary form)
 * The "v" verion is called from _validate, the "s" version from _submit.
 */
function confirming_s(&$form_state) {return ($form_state['rebuild'] = isset($form_state['confirm']));}
function confirming_v(&$form_state) {
  extract($form_state['values']);
  if(@$form_state['confirm']) {
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

function confirm($message, &$form_state) {$form_state['confirm'] = $message;}

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

function plain($array) {
  foreach ($array as $key => $value) $array[$key] = @check_plain($value);
  return $array;
}

function tx_days() {
  $days = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '-1' => 'Year to date');
  $period = @$_SESSION['rweb_tx_period'] ?: 30;
  return compact(u\ray('days period'));
}

/**
 * Display a drupal message (error or not)
 * Possible syntaxes:
 *   say('index', array(optional args), 'optional error field')
 *   say('index', 'error field')
 *   say(array(index, args), 'optional error field')
 * @return FALSE (transfer() depends on this)
 */
function say($index, $args = array(), $error_field = '') {
  if (is_array($index)) list ($index, $args, $error_field) = array($index[0], $index[1], $args); // error returned from a function
  if (!is_array($args)) list ($args, $error_field) = array($error_field, $args); // allow either order, for easy 2-param calls
  $message = tt($index, $args);
  if ($error_field) \form_set_error($error_field, $message); else \drupal_set_message($message);
  i\r_log('WEBout', $message, i\be_who());
  return FALSE;
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return u\tt($message, $subs, 'Web');}
function block_def($info, $extra = array()) {return compact(u\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(u\ray('subject content'));}
function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($value) {return array('default_value' => $value);}
function attrib($attribs) {return array('attributes' => $attribs);}
