<?php
namespace rCredits\Web;
use \rCredits as r;
/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

$GLOBALS['Web messages'] = array(
  'tid' => t('Transaction #@tid: '),
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
  'your account is ready' => t('<p></p><p>Welcome to rCredits! Your account id is @credit_id. Your starting balance is @balance.</p><p>After you confirm your email address (by clicking the link in the email we sent you), come back here and log in.</p><p>Pretend to buy and sell just as you would in a typical month. We need that information, to make sure that rCredits don\'t get stuck anywhere.</p><p>Experiment! Have fun! (and please let us know if you run into any glitches)'),
  'nothing selected' => t('You did not select anything'),
);

/*
function transact_page() {
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  foreach (r\ray('payment charge exchange') as $one) $$one = render(\rweb_block_view($one));
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
      INSERT INTO users (uid, name, pass, mail, short_name, full_name, credit_id, status) VALUES
      ($uid, 'uten', '$pass', 'uten@ex.com', 'uten', 'U Ten', 'Z9AAAAAA', 1),
      ($uid2, 'btwo', '$pass', 'btwo@ex.com', 'btwo', 'B Two', 'Z9AAAAAB', 1),
      ($uid3, 'cstore', '$pass', 'cstore@ex.com', 'cornerstore', 'Corner Store', 'Z9AAAAAC', 1);
      
      INSERT INTO sms_user (number, uid, status) VALUES
      (10, $uid, 2),
      (2, $uid2, 2);
EOF;
    \db_query($sql);
    r\give_signup_incentive($uid);
    r\give_signup_incentive($uid2);
  }
  if ($arg == 'clear_sessions') \db_query('TRUNCATE table SESSIONS');
  
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

function show_form($function) {
  r\r_log('WEBout', "{$function}_form", r\be_who());
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  $form = drupal_get_form("rCredits\\Web\\{$function}_form");
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
 * @todo: move query and updates to rcredits.inc or maybe rcredits-interface.inc (i\), which
 *   would define the entire interface to the rcredits core (ultimately to be moved to a separate VPS)
 */
function account_form($form, &$form_state) {
  $myid = r\be_who();
  
  $result = db_query("SELECT shared_uid, permission FROM r_shared_accounts WHERE shared_with=" . $myid);
  while ($row = $result->fetchAssoc()) $accounts[$row['shared_uid']] = $row['permission'];
  if ($uids = @$_SESSION['account_choices']) {
    foreach (unserialize($uids) as $uid => $permission) $accounts[$uid] = max(@$accounts[$uid], $permission); // add or upgrade
  } else $accounts[$myid] = count($GLOBALS['share permissions']) - 1; // don't give current account all permission if it's a share
  $_SESSION['account_choices'] = serialize($accounts);
  if (!r\be_who()) r\be_who($myid);
  
  foreach ($accounts as $uid => $zot) {
    $balance = r\format_amount(r\credit_info($uid)->balance);
    $accounts[$uid] = r\user_field('full_name', $uid) . ": $balance";
  }

  $credit_id = r\user_field('credit_id', $myid);
  $full_name = r\user_field('full_name', $myid);
  $onchange = "this.form.submit();";
  $account = form_field('select', t('Account:'), t('(Select an account)'), 
    dft($myid) + attrib(compact('onchange')), $accounts);
  $one_account = form_field('item', t('Account') . ": <span>$full_name ($credit_id)</span>");
  $one_balance = form_field('item', t('Current Balance') . ": <span>$balance</span>");
  $submit = form_field('submit');
  $form = count($accounts) == 1 ? compact(r\ray('one_account one_balance')) : r\prefix_keys('account_', compact(r\ray('account submit')));
  
//  $form['#attributes']['class'] = array('labeled');
/*
  $js = <<<EOF
    function nixwhat(op) {
      var c0=document.getElementById('edit-' + op + '-type-0');
      jQuery('.form-item-' + op + '-what').css('visibility', c0.checked ? 'hidden' : 'visible');
    }
    nixwhat('payment');
    nixwhat('charge');
EOF;
*/
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
  r\be_who($account_account);
}

function tx_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = array(
    'description' => form_field('item', t('Pay / Charge')),
    'amount' => form_field('textfield', '$', t('Amount to pay or charge')),
    'who' => form_field('textfield', 'who:', t('Pay or charge whom?')),
    'goods' => form_field('radios', '', '', 
//      dft(1) + array('attributes' => array('onchange' => "nixwhat('payment');")), 
      dft(1) + attrib(array('onchange' => "nixwhat();")), 
      array(t('cash/credit/gift/etc.'), t('goods & services'))),
    'what' => form_field('textfield', 'for:', t('Description of goods and services')),
    'pay' => form_field('submit', NULL, t('Submit button'), 'Pay'),
    'charge' => form_field('submit', NULL, t('Submit button'), 'Charge'),
  );
  $form['#attributes']['class'] = array('labeled');
  return $form;
}

function tx_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;
  if ($confirm = transfer($form_state['values'], FALSE)) confirm($confirm, $form_state);
}

function tx_form_submit($form, &$form_state) {
  if (!confirming_s($form_state)) transfer($form_state['values'], TRUE);
}

/*
function payment_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = r\prefix_keys('payment_', array(
    'description' => form_field('item', t('Payment')),
    'amount' => form_field('textfield', '$', t('Amount to pay')),
    'who' => form_field('textfield', 'to:', t('Pay whom?')),
    'goods' => form_field('radios', '', '', 
//      dft(1) + array('attributes' => array('onchange' => "nixwhat('payment');")), 
      dft(1) + attrib(array('onchange' => "nixwhat('payment');")), 
      array(t('cash/credit/gift/etc.'), t('goods & services'))),
    'what' => form_field('textfield', 'for:', t('Description of goods and services')),
    'submit' => form_field('submit', NULL, t('Submit button'), 'Pay'),
  ));
  $form['#attributes']['class'] = 'rweb rblock';
  return $form;
}

function payment_form_validate($form, &$form_state, $op = 'payment') {
  if (confirming_v($form_state)) return;
  if ($confirm = transfer($op, $form_state['values'], FALSE)) confirm($confirm, $form_state);
}

function payment_form_submit($form, &$form_state, $op = 'payment') {
  if (!confirming_s($form_state)) transfer($op, $form_state['values'], TRUE);
}
*/
/**
 * Buy Now
foreach (array('', 'Contribution
$title = isset($_POST['button']) ? $_POST['button'] : '';
$title = (in_array(strtolower($title), rCredits\ray('Buy Now, Contribute, Donate, Buy, Pay')) ? ucwords($title) : 'Pay') . ' With rCredits';
echo "<h1>$title</h1>\n";
 */
function buy_now_form($form, &$form_state, $action = 'Payment', $verb = 'Pay') {
  if ($confirm = sure_form($form_state)) return $confirm;
  extract(plain($_POST), EXTR_PREFIX_ALL, 'my');
  if (isset($my_what)) list ($my_amount, $my_company, $my_item) = array($my_amount, $my_who, $my_what);
  if (!isset($my_amount)) $my_amount = $my_company = $my_item = ''; else $my_amount = @number_format($my_amount, 2);

  $disabled = array('disabled' => 1);
  if (!($name = r\db_lookup('full_name', 'users', "credit_id='$my_company'"))) $name = 'Unregistered Company'; 
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
    'description' => form_field('item', '<i>Corner Store\'s</i><br><b>Awesome Apple Pie<br><big>only $23</big></b>'),
    'item' => form_field('hidden', '', '', 'Awesome Apple Pie'),
    'amount' => form_field('hidden', '', '', 23),
    'company' => form_field('hidden', '', '', 'Z9AAAAAC'), // (use Corner Store online)
    'code' => form_field('hidden', '', '', '29c8ad9f8d-908'),
    'submit' => form_field('image_button', NULL, t('Submit button'), array('src' => $base_url . '/images/rbutton-buynow.png')),
  );
  $form['#method'] = 'POST';
  $form['#action'] = $base_url . '/members/buy-now';
  return $form;
}

function charge_form($form, &$form_state) { // almost exactly like payment
  if ($confirm = sure_form($form_state)) return $confirm;

  $form = r\prefix_keys('charge_', array(
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
 * @todo: handle submit
 */
function exchange_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm;

  $myid = r\be_who();
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $request = r\format_amount(r\user_field('demand', $myid));
  $demand = r\credit_info($myid)->fancy['total_demand'];

  $form = array(
    'description' => form_field('item', t('Get rCredits / USD')),
    'request' => form_field('item', tt('request & demand', compact(r\ray('request demand')))),
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

  $myid = r\be_who();
  extract($form_state['values']);
  if (blank_field(compact('amount'))) return;
  list ($message, $args, $confirm) = r\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $myid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $form_state);
  if (!@$args['success']) say($message, $args, 'amount');
}

function exchange_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;

  $myid = r\be_who();
  extract($form_state['values']);
  list ($message, $args, $confirm) = r\get($op == 'Get rCredits' ? 'r' : 'usd', $amount, $myid, TRUE);
  say($message, $args, @$args['success'] ? '' : 'amount'); // error or success
  if ($op == 'Get rCredits') {
    if ($amount < RCREDITS_MIN_DEMAND) say('demand too small');
    if (!user_access('Partner')) say('demand not yet');
  }
}
/*
function manage_form($form, &$form_state) {
  $form = r\prefix_keys('manage_', array(
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
function txs_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD

  $myid = r\be_who();
  extract(tx_days(), EXTR_PREFIX_ALL, 'tx');
  $ending = time();
  $starting = strtotime($tx_period == '0' ? '1jan' : ((1 - $tx_days[$tx_period]) . ' days'), $ending);

  $headers = r\ray('#,Date,Name,From you,To you,Status,Purpose,Rewards');
  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'disputed');
  $types = r\tx_types("'rebate', 'bonus'");
  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid = IF(t.payer = $myid, t.payee, t.payer)) AS name,
      IF(t.payer = $myid, t.payer_for, t.payee_for) AS purpose
    FROM r_txs t 
    WHERE t.type NOT IN($types)
    AND $myid IN (t.payer, t.payee) 
    AND (t.state IN(:tx_done, :tx_pending) OR (t.state = :tx_denied AND t.payee = $myid))
    AND t.created >= $starting AND t.created <= $ending
    ORDER BY t.xid DESC
EOF;
  $txs = db_query($sql, array(':tx_done' => TX_DONE, ':tx_pending' => TX_PENDING, ':tx_denied' => TX_DENIED));
  $list = array();
  while ($row = $txs->fetchAssoc()) {
    extract($row, EXTR_PREFIX_ALL, 'one');
    $tid = r\tid_from_xid($myid, $one_xid);
    $data = unserialize($one_data);
    $type = r\tx_type($one_type);
    $reward = @$data[$type == 'payment' ? 'rebate' : 'bonus'];
    $reward = $reward ? number_format($reward * $one_amount, 2) : '';
    $amount = number_format($one_amount, 2);
    if ($type != 'payment' and $type != 'charge') list($amount, $reward) = array('', $amount); // show interest and signup bonus as rewards
    list ($from_you, $to_you) = $one_payer == $myid ? array($amount, '') : array('', $amount);
    $state = ($one_state == TX_PENDING and $one_payee == $myid) ? 'pending' : $states[$one_state];
    $list["x$one_xid"] = array($tid, date('d-M', $one_created), $one_name, $from_you, $to_you, $state, $one_purpose, $reward);
  }

  $cols = array(); // note: this all fails because Drupal does not recognize colgroups in tableselect, so pass it to show_form
  foreach (r\array_prefix('tx-', array_merge(array('selector'), $headers)) as $class) { // Drupal will insert a selector column
    $class = str_replace(' ', '-', $class == 'tx-#' ? 'tx-tid' : strtolower($class));
    $cols[] = compact('class');
  }
  $tx_other = array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no txs')); // 'colgroups' => array($cols) fails
  $tx_select = form_field('tableselect', '', '', $tx_other, $list);
  
  $tasks = array('ok' => 'Approve', 'undo' => 'Dispute/Cancel/Undo', 'edit' => 'Edit description'); // could have used drupal_map_assoc
  $form = r\prefix_keys('tx_', array(
    'period' => form_field('select', t('Show transactions for the past'), '', dft($tx_period), $tx_days),
    'show' => form_field('submit', NULL, t('Submit button'), 'Show'),

    'summary' => form_field('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => $tx_select,
    'task' => form_field('select', t('With the selected transactions:'), '', array(), $tasks),
    'go' => form_field('submit', NULL, t('Submit button'), 'Go'),
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
    variable_set('rweb_tx_period', $tx_period);
    $form_state['redirect'] = \current_path();
    return;
  }
  
  // $op == 'Go' (fiddling with the old transactions)
  $myid = r\be_who();
  foreach ($tx_transactions as $key => $xid) if ($xid) {
    $xid = substr($xid, 1); // ignore the leading 'x'
    $result = db_query("SELECT * FROM r_txs WHERE xid = $xid LIMIT 1")->fetchAssoc();
    extract($result, EXTR_PREFIX_ALL, 'my');
    $tid = r\tid_from_xid($myid, $xid);
    $xname = "tx_transactions[x$xid]";
    if ($tx_task == 'ok') {
      if ($my_payer != $myid) {
        say("tid|can't ok for other", compact('tid'), $xname);
      } elseif ($my_state != TX_PENDING) {
        say("tid|can't ok if done", compact('tid'), $xname);
      } else {
        list ($message, $args) = r\pay_invoice($xid);
        say("tid|$message", $args);
      }
    }
    if ($tx_task == 'undo') {
      list ($message, $args) = r\undo_tx($xid);
      say("tid|$message", $args);
    }
    if ($tx_task == 'edit') {
      if ($my_state != TX_DONE and $my_payer != $myid) {
        say("tid|can't change what", compact('tid'), $xname);
      } else \drupal_set_message('Editing descriptions is not yet possible');
    }
  }
}

function tx_summary($starting, $ending) {
  $myid = r\be_who();
  $previous_ending = strtotime('-1 days', $starting);
  extract($now_info = (array) r\credit_info($myid, TX_DONE, $ending), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) r\credit_info($myid, TX_PENDING, $ending), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) r\credit_info($myid, TX_DONE, $previous_ending), EXTR_PREFIX_ALL, 'old');
  $dif = $pfancy = array();
  foreach (r\ray('gross_in_proper gross_out rewards') as $key) {
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
  
  $classes = r\array_prefix('txsum-', r\ray('one tosign toyou fromsign fromyou rewardssign rewards balancesign balance'));
  $rows = array(
    array('id' => 'txsum-dates', 'data' => r\ray("$starting,,,,,,,,$ending")),
    array('id' => 'txsum-headers', 'data' => r\ray('Previous balance,,To You,,From You,,Rewards,,New balance')),
    array(
      'id' => 'txsum-now', 
      'data' => r\ray("$old_balance | + | $dif_gross_in_proper | - | $dif_gross_out | + | $dif_rewards | = | $now_balance"),
    ),
    array(
      'id' => 'txsum-pending',
      'data' => r\ray("<b>PENDING:</b> | + | $pfancy_gross_in_proper | - | $pfancy_gross_out | + | $pfancy_rewards | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return theme_table(compact(r\ray('header rows attributes caption colgroups sticky empty')));
}

/**
 * @NOTE: The country and state drop down require a change in form.inc, line 2649:
 *   if (is_null($choices = $element['#options'])) return '';
 */
function contact_info_form($form, &$form_state) {
  extract(plain((array) \user_load(r\be_who())), EXTR_PREFIX_ALL, 'my');
  
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $form = array(
    'description' => form_field('item', t('Contact Information')),
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), dft($my_full_name)),
    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? r\format_phone($my_phone): '')),
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
  extract($info = plain($form_state['values']), EXTR_PREFIX_ALL, 'my');
  if ($my_phone and !r\format_phone($my_phone, '+n')) say('bad phone', 'phone');
}

function contact_info_form_submit($form, &$form_state) {
  $account = \user_load(r\be_who());
  extract($info = plain($form_state['values']), EXTR_PREFIX_ALL, 'my');
  $info['website'] = str_replace('http://', '', strtolower(@$my_website));
  if ($my_phone) $info['phone'] = r\format_phone($my_phone, '+n');
  $info['short_name'] = r\short_name($my_full_name);
  user_save($account, $info);
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function share_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD
  $my_uid = r\be_who();
  $account_identifiers = r\account_name($my_uid);
  $headers = r\ray('Account ID,Full name,Email,Permission');

  $sql = <<<EOF
    SELECT u.name as username, credit_id, u.full_name, u.mail as email, s.shared_with, s.permission
    FROM r_shared_accounts s RIGHT JOIN users u ON u.uid = s.shared_with
    WHERE s.shared_uid = $my_uid
    ORDER BY username
EOF;
  $result = db_query($sql);
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
  if ($op == 'Go' and no_selection($shares)) return say('nothing selected');
  if (confirming_v($form_state)) return;
  
  if ($op == 'Go') {
    $permission = $GLOBALS['share permissions'][$change_permission];
    confirm(tt('confirm share permission', compact('permission')), $form_state);
  } else { // $op == 'Add'
    $myid = r\be_who();
    if (blank_field(compact('new_share'))) return;
    $uid = r\identify($new_share, $myid, FALSE, 'no self-sharing');
    if (is_array($uid)) return say($uid, 'new_share');
    if (r\db_lookup('1', 'r_shared_accounts', "shared_uid=$myid AND shared_with=$uid")) return say('already shared', 'new_share');
    $form_state['values']['shared_with'] = $uid; // save for submission handling
  }
}

function share_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  $shared_uid = r\be_who();
  
  extract($form_state['values']);
  if ($op == 'Add') {
    $permission = $new_permission;
    $zot = \drupal_write_record('r_shared_accounts', compact(r\ray('shared_uid shared_with permission')));
    $who = r\user_field('full_name', $shared_with);
    return say('report new share', compact('who'));
  }
  
  // $op == 'Go'
  $permission = $GLOBALS['share permissions'][$change_permission];
  foreach ($shares as $key => $uid) if ($uid) {
    $shared_with = substr($uid, 1); // ignore the leading 'x'
    $where = "WHERE shared_with=$shared_with AND shared_uid=$shared_uid LIMIT 1";
    $sql = $change_permission ? "UPDATE r_shared_accounts SET permission=$change_permission $where" : "DELETE FROM r_shared_accounts $where";
    \db_query($sql);
    $account = r\user_field('full_name', $shared_with);
    say('report share change', compact(r\ray('account permission')));
//    $result = db_update('r_shared_accounts')->fields(compact('permission'))->condition('shared_with shared_by')))->fetchAssoc();
  }
}

function cell_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only deletion
  if (form_step($form_state, $info) == 'prove') {
    $number = r\format_phone($form_state['storage']['values']['number']);
    $form = array(
      'description' => form_field('item', t('Verify')),
      'subtext' => form_field('item', tt('verify cell', compact('number'))),
      'code' => form_field('textfield', t('Code:')),
      'number' => form_field('hidden', '', '', $number),
      'verify' => form_field('submit', NULL, t('Submit button'), 'Verify'),
    );
    return $form;
  }

  $my_uid = r\be_who();
  $headers = array('Cell Number');

  $sql = "SELECT number FROM sms_user WHERE uid = $my_uid ORDER BY number";
  $result = db_query($sql);
  $list = array();
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $list["x$number"] = array(r\format_phone($number));
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

  if ($op == 'Add') {
    $myid = r\be_who();
    if (blank_field(compact('number'))) return;
    $number_internal = r\format_phone($number, '+n');
    if (!$number_internal) return say('bad phone', 'number');
    if ($result = r\db_get_record('sms_user', "number=$number_internal")) {
      extract($result, EXTR_PREFIX_ALL, 'old');
      if ($old_uid == $myid) return say('already cell', 'number');
      extract(r\db_get_record('users', "uid=$old_uid", 'full_name, credit_id'));
      $account = "$full_name ($credit_id)"; // maybe make the previous line and this a function
      if (!r\is_temp_account($old_uid)) return say('cell taken', compact(r\ray('number account')), 'number');
    }
  } elseif ($op == 'Verify') {
    if (strtoupper($code) == $_SESSION['nonce']) return;
    previous_state($form_state, 'bad nonce'); // can't just give error message, because then verification form persists
  } else { // $op == remove
    if (no_selection($cells)) return say('nothing selected');
    confirm(tt('confirm delete cell'), $form_state);
  }
}

function cell_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;
  $uid = r\be_who();
  
  extract($form_state['values']);
  if ($op == 'Add') {
    if ($info = previous_state($form_state)) return say($info, 'number'); // just returning from failed validation of step 2 ('prove')
    $_SESSION['nonce'] = $nonce = r\nonce(); // don't use POST (user could peek)
    r\SMS\send($number, tt('verification code', compact('nonce')));
    return form_step($form_state, compact('number'), 'prove');
  }

  if ($op == 'Verify') {
    step_one($form_state); // no next step (back to original form)
    $status = SMS_USER_CONFIRMED;
    $number = r\format_phone($number, '+n');
    if ($old_uid = r\db_lookup('uid', 'sms_user', "number=$number")) {
      r\merge_accounts($old_uid, r\be_who());
    } else \drupal_write_record('sms_user', compact(r\ray('uid number status')));
    return say('report new cell', compact('number'));
  }
    
  // $op == remove
  foreach ($cells as $key => $number) if ($number) {
    $number = substr($number, 1); // ignore the leading 'x'
    $sql = "DELETE FROM sms_user WHERE number=$number LIMIT 1";
    \db_query($sql);
    say('report delete cell', compact('number'));
  }
}

/**
 * Modify the user registration form.
 */
/* Here's what it looks like before modifying:
array (
  'administer_users' => 
  array (
    '#type' => 'value',
    '#value' => false,
  ),
  '#user' => 
  stdClass::__set_state(array(
     'uid' => 0,
     'hostname' => '::1',
     'roles' => 
    array (
      1 => 'anonymous user',
    ),
     'cache' => 0,
  )),
  '#user_category' => 'register',
  '#attached' => 
  array (
    'library' => 
    array (
      0 => 
      array (
        0 => 'system',
        1 => 'jquery.cookie',
      ),
    ),
  ),
  '#attributes' => 
  array (
    'class' => 
    array (
      0 => 'user-info-from-cookie',
    ),
  ),
  '#validate' => 
  array (
    0 => 'user_account_form_validate',
    1 => 'user_validate_picture',
    2 => 'user_register_validate',
  ),
  'account' => 
  array (
    '#type' => 'container',
    '#weight' => -10,
    'name' => 
    array (
      '#type' => 'textfield',
      '#title' => 'Username',
      '#maxlength' => 60,
      '#description' => 'Spaces are allowed; punctuation is not allowed except for periods, hyphens, apostrophes, and underscores.',
      '#required' => true,
      '#attributes' => 
      array (
        'class' => 
        array (
          0 => 'username',
        ),
      ),
      '#default_value' => '',
      '#access' => true,
      '#weight' => -10,
    ),
    'mail' => 
    array (
      '#type' => 'textfield',
      '#title' => 'E-mail address',
      '#maxlength' => 254,
      '#description' => 'A valid e-mail address. All e-mails from the system will be sent to this address. The e-mail address is not made public and will only be used if you wish to receive a new password or wish to receive certain news or notifications by e-mail.',
      '#required' => true,
      '#default_value' => '',
    ),
    'status' => 
    array (
      '#type' => 'radios',
      '#title' => 'Status',
      '#default_value' => true,
      '#options' => 
      array (
        0 => 'Blocked',
        1 => 'Active',
      ),
      '#access' => false,
    ),
    'roles' => 
    array (
      '#type' => 'checkboxes',
      '#title' => 'Roles',
      '#default_value' => 
      array (
      ),
      '#options' => 
      array (
        3 => 'administrator',
      ),
      '#access' => false,
      2 => 
      array (
        '#type' => 'checkbox',
        '#title' => 'authenticated user',
        '#default_value' => true,
        '#disabled' => true,
      ),
    ),
    'notify' => 
    array (
      '#type' => 'checkbox',
      '#title' => 'Notify user of new account',
      '#access' => false,
    ),
  ),
  'signature_settings' => 
  array (
    '#type' => 'fieldset',
    '#title' => 'Signature settings',
    '#weight' => 1,
    '#access' => false,
    'signature' => 
    array (
      '#type' => 'text_format',
      '#title' => 'Signature',
      '#default_value' => '',
      '#description' => 'Your signature will be publicly displayed at the end of your comments.',
      '#format' => NULL,
    ),
  ),
  'picture' => 
  array (
    '#type' => 'fieldset',
    '#title' => 'Picture',
    '#weight' => 1,
    '#access' => false,
    'picture' => 
    array (
      '#type' => 'value',
      '#value' => NULL,
    ),
    'picture_current' => 
    array (
      '#markup' => '',
    ),
    'picture_delete' => 
    array (
      '#type' => 'checkbox',
      '#title' => 'Delete picture',
      '#access' => false,
      '#description' => 'Check this box to delete your current picture.',
    ),
    'picture_upload' => 
    array (
      '#type' => 'file',
      '#title' => 'Upload picture',
      '#size' => 48,
      '#description' => 'Your virtual face or picture. Pictures larger than 1024x1024 pixels will be scaled down. ',
    ),
  ),
  '#parents' => 
  array (
  ),
  '#pre_render' => 
  array (
    0 => '_field_extra_fields_pre_render',
  ),
  '#entity_type' => 'user',
  '#bundle' => 'user',
  'actions' => 
  array (
    '#type' => 'actions',
    'submit' => 
    array (
      '#type' => 'submit',
      '#value' => 'Create new account',
    ),
  ),
  '#submit' => 
  array (
    0 => 'user_register_submit',
  ),
  '#form_id' => 'user_register_form',
  '#type' => 'form',
  '#build_id' => 'form-4h0aXkaHG7TOiMiI73zlMrbiMBEVrbn0dbbaUhMAJWQ',
  'form_build_id' => 
  array (
    '#type' => 'hidden',
    '#value' => 'form-4h0aXkaHG7TOiMiI73zlMrbiMBEVrbn0dbbaUhMAJWQ',
    '#id' => 'form-4h0aXkaHG7TOiMiI73zlMrbiMBEVrbn0dbbaUhMAJWQ',
    '#name' => 'form_build_id',
  ),
  'form_id' => 
  array (
    '#type' => 'hidden',
    '#value' => 'user_register_form',
    '#id' => 'edit-user-register-form',
  ),
  '#id' => 'user-register-form',
  '#method' => 'post',
  '#action' => '/devcore/user/register',
  '#theme_wrappers' => 
  array (
    0 => 'form',
  ),
  '#tree' => false,
  '#theme' => 
  array (
    0 => 'user_register_form',
  ),
)
*/
function register_form(&$form, &$form_state) {
  $my_country = $my_state = '';
  extract($form_state['input'], EXTR_PREFIX_ALL, 'my'); // overwrites my_country and my_state with input values, on input error
  $onchange = "print_state('edit-state',this.selectedIndex,'');";
  $required = array('required' => TRUE);
  $form += array(
    'full_name' => form_field('textfield', t('Full name:'), t('Your full legal name, properly capitalized'), $required),
/*    'phone' => form_field('textfield', t('Phone:'), t('Your primary phone (landline or cell)'), dft($my_phone ? r\format_phone($my_phone): '')),
    'fax' => form_field('textfield', t('Fax:'), t(''), dft($my_fax)),
    'website' => form_field('textfield', t('Website:'), t(''), dft($my_website)), */
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
  $form_state['values']['short_name'] = r\short_name($full_name);
}

function register_form_submit($form, &$form_state) {
  extract($form_state['values']);
//  debug($form_state, '', 1);
  $credit_id = r\make_credit_id($uid);
  $balance = r\format_amount(r\give_signup_incentive($uid));
  say('your account is ready', compact(r\ray('balance credit_id')));
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
  $form['#attributes'] = array('class' => r\ray('rweb confirmation'));

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
  $myid = r\be_who();
  if (blank_field(compact(r\ray('amount who')))) return NULL;
  if ($goods and trim($what) == '') return say('missing what', 'what');
  if (!$goods) $what = 'cash';
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', 'amount');
  if (is_array($other_uid = r\identify($who, $myid))) return say($other_uid[0], compact('who'), 'who');

  list ($message, $args, $confirm) = r\transfer($tx_type, $myid, $other_uid, $amount, $what, $confirmed);
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
  $myid = r\be_who();
  foreach (r\ray('who amount goods what') as $one) $$one = $info["{$op}_$one"];
  if (blank_field(compact(r\ray('amount who')), $op . '_')) return NULL;
  if ($goods and trim($what) == '') return say('missing what', $op . '_what');
  if (!$goods) $what = 'cash';
  $amount = str_replace(',', '', $amount); // ignore commas
  if (!is_numeric($amount)) return say('bad amount', $op . '_amount');
  if (is_array($other_uid = r\identify($who, $myid))) return say($other_uid[0], compact('who'), $op . '_who');

  list ($message, $args, $confirm) = r\transfer($op, $myid, $other_uid, $amount, $what, $confirmed);
  if ($confirm) return tt('confirm ' . $message, $args);
  if (@$args['success']) return say($message, $args);
  return say($message, $args, $op . '_amount');
}
*/

function menu($title, $type, $function = NULL, $function_args = array(), $other = array()) {
  static $weight;
  if (function_exists($local_func = "\\rCredits\\Web\\$function")) $function = $local_func;

  return array_merge(array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
    'access callback' => 'user_access',
    'access arguments' => array('member'),
    'weight' => @$weight - 1,
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  ), $other); // $other overwrites, if there is a conflict
}

function form_field($type, $title = '', $description = '', $other = array(), $options = NULL, $ajax = array()) {
  if (!is_array($other)) $other = array(($type == 'item' ? 'markup' : 'value') => $other);
  $field = r\prefix_keys('#', array_merge(compact(r\ray('type title description options')), $other));
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
    $prefixed = r\abbreviates($prefix, $key);
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
  $days = array('4' => '4 days', '30' => '30 days', '90' => '90 days', '180' => '6 months', '365' => '12 months', '0' => 'Year to date');
  $period = variable_get('rweb_tx_period', 30);
  return compact(r\ray('days period'));
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
  r\r_log('WEBout', $message, r\be_who());
  return FALSE;
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return r\tt($message, $subs, 'Web');}
function block_def($info, $extra = array()) {return compact(r\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(r\ray('subject content'));}
function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($value) {return array('default_value' => $value);}
function attrib($attribs) {return array('attributes' => $attribs);}
