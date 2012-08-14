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
  "can't change what" => t('You can edit the description of the transaction only if you created it or the transaction is completed.'),
  "can't ok for other" => t('You can okay only transactions that are waiting for YOU to okay them.'),
  "can't ok if done" => t('There is no need to okay an already-completed transaction.'),
  'required field' => t('@field must not be blank.'),
  'bad phone' => t('That is not a proper phone number.'),
  'bad amount' => t('Amount must be a number.'),
  'op canceled' => t('Operation Canceled.'),
  'request & demand' => t('Your standing request for rCredits is <b>@request</b>.<br>Total demand for rCredits overall is <b>@demand</b>.'),
  'confirm share permission' => 'Are you sure you want to change the selected shares to "@permission"?',
  'already shared' => 'You are already sharing with that account. If you want to change the permissions, check the box next to that account, choose the new permissions, and click "Go".',
  'report new share' => 'You are now sharing this account with @who',
  'nothing selected' => 'You did not select anything',
);

function member_section() {
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  foreach (r\ray('account payment charge exchange') as $one) $$one = render(\rweb_block_view($one));
  return array('#markup' => "
  $account
  <table cellspacing='10'>
  <tr><td>$payment</td><td>$charge</td></tr>
  <tr><td>$exchange</td></tr>
  </table>
");
}

function show_form($function) {
  drupal_add_css(\drupal_get_path('module', 'rweb') . '/rweb.css', array('group' => CSS_THEME, 'weight' => -9));
  return render(drupal_get_form("rCredits\\Web\\{$function}_form"));
}

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
  $onchange = "this.form.submit();";
  $account = form_field('select', t('<b>Current Balance</b> for'), t('(Select an account)'), 
    dft($myid) + attrib(compact('onchange')), $accounts);
  $current_balance = form_field('item', t('<b>Current Balance</b> for account') . " $credit_id: $balance");
  $submit = form_field('submit');
  $form = r\prefix_keys('account_', compact(r\ray(count($accounts) == 1 ? 'current_balance' : 'account submit')));

//  $form['#attributes']['class'] = 'rweb rblock';
// use drupal_add_css() on the page instead. $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  $js = <<<EOF
    function nixwhat(op) {
      var c0=document.getElementById('edit-' + op + '-type-0');
      jQuery('.form-item-' + op + '-what').css('visibility', c0.checked ? 'hidden' : 'visible');
    }
    nixwhat('payment');
    nixwhat('charge');
EOF;
  drupal_add_js($js, array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function account_form_submit($form, &$form_state) {
  extract($form_state['values']);
  r\be_who($account_account);
}

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

/**
 * @todo: sanitize input!
 */
function buy_now_form($form, &$form_state, $action = 'Payment', $verb = 'Pay') {
  if ($confirm = sure_form($form_state)) return $confirm;
  extract(plain($_POST), EXTR_PREFIX_ALL, 'my');
  if (isset($my_payment_amount)) list ($my_amount, $my_company, $my_item) = array($my_payment_amount, $my_payment_who, $my_payment_what);
  if (!isset($my_amount)) $my_amount = $my_company = $my_item = ''; else $my_amount = @number_format($my_amount, 2);

  $disabled = array('disabled' => 1);
  if (!($name = r\db_lookup('full_name', 'users', "credit_id='$my_company'"))) $name = 'Unregistered Company'; 
  $verblower = strtolower($verb);
  $prefix = 'payment_';
  
  $form = r\prefix_keys($prefix, array(
    'description' => form_field('item', t("Confirm $action")),
    'xamount' => form_field('textfield', '$', t("Amount to $verblower"), disabled($my_amount)),
    'amount' => form_field('hidden', '', '', $my_amount),
    'xwho' => form_field('textfield', 'to:', t('Pay whom?'), disabled($name)),
    'who' => form_field('hidden', '', '', $my_company),
    'xwhat' => form_field('textfield', 'for:', t('Description of goods and services'), disabled($my_item)),
    'what' => form_field('hidden', '', '', $my_item),
    'goods' => form_field('hidden', '', '', 1),
    'submit' => form_field('submit', NULL, t('Submit button'), $verb),
  ));

  $form['#attributes']['class'] = 'rweb rblock';
  return $form;
}

function buy_now_form_submit($form, &$form_state) {
  payment_form_submit($form, $form_state);
  drupal_goto('members'); // member page when done
}

function example_store_form($form, &$form_state) {
  global $base_url;
  $form = array(
    'description' => form_field('item', '<i>Corner Store\'s</i><br><b>Awesome Apple Pie<br><big>$23</big></b>'),
    'item' => form_field('hidden', '', '', 'Awesome Apple Pie'),
    'amount' => form_field('hidden', '', '', 23),
    'company' => form_field('hidden', '', '', 'Z9AAAAAA'), // (use Corner Store online)
    'code' => form_field('hidden', '', '', '29c8ad9f8d-908'),
    'submit' => form_field('image_button', NULL, t('Submit button'), array('src' => $base_url . '/images/rbutton-buynow.png')),
  );
  $form['#attributes']['class'] = 'rweb rblock';
  $form['#method'] = 'POST';
  $form['#action'] = $base_url . '/node/4';
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
  $form['#attributes']['class'] = 'rweb rblock';
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

  $form = r\prefix_keys('xch_', array(
    'description' => form_field('item', t('Exchange')),
    'amount' => form_field('textfield', '$', t('Amount to exchange')),
//    'way' => form_field('radios', 'trade:', '', array(), array(t('rCredit for cash'), t('cash for rCredits'))),
//    'who' => form_field('textfield', 'to/from:', t('Exchange with whom?')),
    'getr' => form_field('submit', '', t('Submit button'), 'Get rCredits'),
    'getusd' => form_field('submit', '', t('Submit button'), 'Get USD'),
    'request' => form_field('item', tt('request & demand', compact(r\ray('request demand')))),
  ));
  $form['#attributes']['class'] = 'rweb rblock';
  return $form;
}

function exchange_form_validate($form, &$form_state) {
  if (confirming_v($form_state)) return;

  $myid = r\be_who();
  extract($form_state['values']);
  if (blank_field(compact('xch_amount'), 'xch_')) return;
  list ($message, $args, $confirm) = r\get($op == 'Get rCredits' ? 'r' : 'usd', $xch_amount, $myid, FALSE);
  if ($confirm) return confirm(tt($message, $args), $form_state);
  if (!@$args['success']) say($message, $args, 'xch_amount');
}

function exchange_form_submit($form, &$form_state) {
  if (confirming_s($form_state)) return;

  $myid = r\be_who();
  extract($form_state['values']);
  list ($message, $args, $confirm) = r\get($op == 'Get rCredits' ? 'r' : 'usd', $xch_amount, $myid, TRUE);
  say($message, $args, @$args['success'] ? '' : 'xch_amount'); // error or success
  if ($op == 'Get rCredits') {
    if ($xch_amount < RCREDITS_MIN_DEMAND) say('demand too small');
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
function txs_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD

  $myid = r\be_who();
  extract(tx_days(), EXTR_PREFIX_ALL, 'tx');
  $ending = time();
  $starting = strtotime($tx_period == '0' ? '1jan' : ((1 - $tx_days[$tx_period]) . ' days'), $ending);

  $headers = r\ray('#,Date,Name,From you,To you,Status,Purpose,Rewards');
  $states = array(TX_DONE => '&#10004;', TX_PENDING => 'ok?', TX_DENIED => 'disputed');

  $sql = <<<EOF
    SELECT t.*,
      (SELECT full_name FROM users WHERE uid = IF(t.payer = $myid, t.payee, t.payer)) AS name,
      IF(t.payer = $myid, t.payer_for, t.payee_for) AS purpose
    FROM mcapi_transactions t 
    WHERE t.type IN('payment', 'charge')
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
    $reward = $data[$one_type == 'charge' ? 'bonus' : 'rebate'];
    $reward = $reward ? number_format($reward * $one_amount, 2) : '';
    $amount = number_format($one_amount, 2);
    list ($from_you, $to_you) = $one_payer == $myid ? array($amount, '') : array('', $amount);
    $state = ($one_state == TX_PENDING and $one_payee == $myid) ? 'pending' : $states[$one_state];
    $list["x$one_xid"] = array($tid, date('d-M', $one_created), $one_name, $from_you, $to_you, $state, $one_purpose, $reward);
  }

  $tasks = array('ok' => 'Approve', 'undo' => 'Dispute/Cancel/Undo', 'edit' => 'Edit description'); // could have used drupal_map_assoc
  $form = r\prefix_keys('tx_', array(
    'period' => form_field('select', t('Show transactions for the past'), '', dft($tx_period), $tx_days),
    'show' => form_field('submit', NULL, t('Submit button'), 'Show'),

    'summary' => form_field('markup', '', '', array('markup' => tx_summary($starting, $ending))),
    'transactions' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no txs')), $list),
    'task' => form_field('select', t('With the selected transactions:'), '', array(), $tasks),
    'go' => form_field('submit', NULL, t('Submit button'), 'Go'),
  ));
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
    $result = db_query("SELECT * FROM mcapi_transactions WHERE xid = $xid LIMIT 1")->fetchAssoc();
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

function manage_account_form($form, &$form_state) {
  $form = array(
    'description' => form_field('item', t('Payment')),
    'submit' => form_field('submit', '', t('Submit button'), 'Pay'),
  );
  $form['#attributes']['class'] = 'rweb rblock';
  return $form;
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

  $form['#attached']['css'] = array(\drupal_get_path('module', 'rweb') . '/rweb.css');
  $form['#attributes']['class'] = 'rweb';
  drupal_add_js('inc/countries.js', array('type'=>'file', 'scope'=>'header'));
  drupal_add_js("print_country('edit-country', \"$my_country\", \"$my_state\");", array('type'=>'inline', 'scope'=>'footer'));
  return $form;
}

function contact_info_form_validate($form, &$form_state) {
  extract($info = plain($form_state['values']), EXTR_PREFIX_ALL, 'my');
  if ($my_phone and !r\format_phone($my_phone, '+n')) say('bad phone', compact('tid'), 'phone');
}

function contact_info_form_submit($form, &$form_state) {
  $account = \user_load(r\be_who());
  extract($info = plain($form_state['values']), EXTR_PREFIX_ALL, 'my');
  $info['website'] = str_replace('http://', '', strtolower(@$my_website));
  if ($my_phone) $info['phone'] = r\format_phone($my_phone, '+n');
  user_save($account, $info);
}

function no_selection($list) {
  foreach ($list as $one) if ($one) return FALSE;
  return TRUE;
}

function share_form($form, &$form_state) {
  if ($confirm = sure_form($form_state)) return $confirm; // confirm only getUSD

  extract((array) \user_load(r\be_who()), EXTR_PREFIX_ALL, 'my');
  $account_identifiers = @$my_credit_id ? ": $my_credit_id ($my_full_name $my_mail)" : ": $my_mail";

  $headers = r\ray('Credit ID,Full name,Email,Permission');

  $sql = <<<EOF
    SELECT u.name as username, credit_id, u.full_name, u.mail as email, s.shared_with, s.permission
    FROM r_shared_accounts s RIGHT JOIN users u ON u.uid = s.shared_uid 
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
    'description' => form_field('item', t('Manage Sharing')),
    'subtitle' => form_field('item', t('Sharing Account') . $account_identifiers),
    'shares' => form_field('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no shares')), $list),
    'change_permission' => form_field('select', t('Change permission for the selected shares:'), '', array(), $GLOBALS['share permissions']),
    'go' => form_field('submit', NULL, t('Submit button'), 'Go'),

    'new_share' => form_field('textfield', t('Share with another user:'), t("Type the user's credit_id, email, or phone.")),
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
    $sql = $permission ? "UPDATE r_shared_accounts SET permission=$change_permission $where" : "DELETE FROM r_shared_accounts $where";
    \db_query($sql);
//    $result = db_update('r_shared_accounts')->fields(compact('permission'))->condition('shared_with shared_by')))->fetchAssoc();
  }
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
  $form['#attributes'] = array('class' => 'rweb confirmation');

  drupal_add_js("document.getElementById('edit-confirm').focus();", array('type'=>'inline', 'scope'=>'footer'));  

  return $form;
}

/**
 * Transfer funds
 * @param string $op: 'payment' or 'charge'
 * @param array $info: associative array indexed by field names for who, amount, goods, and what (eg payment_who or charge_who)
 * @param boolean $confirmed: whether the transaction request has been confirmed
 * @return confirmation message (FALSE if confirmation not appropriate -- Note that say() returns FALSE)
 */
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

function tx_summary($starting, $ending) {
  $myid = r\be_who();
  $previous_ending = strtotime('-1 days', $starting);
  extract($now_info = (array) r\credit_info($myid, TX_DONE, $ending), EXTR_PREFIX_ALL, 'now');
  extract($pending_info = (array) r\credit_info($myid, TX_PENDING, $ending), EXTR_PREFIX_ALL, 'pending');
  extract($old_info = (array) r\credit_info($myid, TX_DONE, $previous_ending), EXTR_PREFIX_ALL, 'old');
  $dif = array();
  foreach (r\ray('gross_in_proper gross_out rewards') as $key) $dif[$key] = r\format_amount($now_info[$key] - $old_info[$key]);
  
  extract($dif, EXTR_PREFIX_ALL, 'dif');
  extract($pending_fancy, EXTR_PREFIX_ALL, 'pfancy');
  
  $starting = date('d-M-Y', $starting);
  $ending = date('d-M-Y', $ending);
  $pending_balance = r\format_amount($now_balance + $pending_gross_in_proper - $pending_gross_out + $pending_rewards);
  $now_balance = $now_fancy['balance'];
  $old_balance = $old_fancy['balance'];
  
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
      'data' => r\ray("PENDING: | + | $pfancy_gross_in_proper | - | $pfancy_gross_out | + | $pfancy_rewards | = | $pending_balance"),
    ),
  );
  $attributes = array('id' => 'txsum');
  $header = $cols = array();
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = $empty = '';
  return theme_table(compact(r\ray('header rows attributes caption colgroups sticky empty')));
}

function menu($title, $type, $function = NULL, $function_args = array()) {
  if (function_exists($local_func = "\\rCredits\\Web\\$function")) $function = $local_func;
  return array(
    'title' => $title,
    'type' => $type,
    'page callback' => $function,
    'page arguments' => $function_args,
    'access callback' => 'user_access',
    'access arguments' => array('member'),
    'menu_name' => 'main-menu', 
    'module' => 'rweb',
    'file' => 'rweb.inc',
  );
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
  if(isset($form_state['confirm'])) {
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

function confirm($message, &$form_state) {
  $form_state['confirm'] = $message;
}

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
  return FALSE;
}

// one-line functions that need no explanation
function tt($message, $subs = array()) {return r\tt($message, $subs, 'Web');}
function block_def($info, $cache = DRUPAL_CACHE_PER_ROLE) {return compact(r\ray('info cache'));} // also status, region, visibility, pages
function block_view($subject, $content) {return compact(r\ray('subject content'));}
function disabled($value) {return array('value' => $value, 'disabled' => TRUE);}
function dft($value) {return array('default_value' => $value);}
function attrib($attribs) {return array('attributes' => $attribs);}
