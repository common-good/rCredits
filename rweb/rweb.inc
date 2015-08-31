<?php
namespace rCredits\Web;
use rCredits as r;
use rCredits\Backend as be;
use rCredits\Util as u;
use rCredits\Testing as t;
use rCredits\db as db;
use rCredits\Txs as x;
use rCredits\Admin as a;
use rCredits\Risk as k;
use rCredits\Web as w;

require_once __DIR__ . '/rweb-subs.inc';
require_once __DIR__ . '/../rcredits-backend.inc';
include_once __DIR__ . '/rweb-txs.inc'; // treat this as an extension of this file, just to make menu ['file'] consistent
// FAILS (WHY?) include_once __DIR__ . (u\test() ? '/../rcredits-testing.inc' : '/../testing-stub.inc'); // else fail in showForm()
include_once __DIR__ . '/../rcredits-testing.inc'; // else fail in showForm()

define('UPLOAD_DIR', dirname($_SERVER['DOCUMENT_ROOT']) . '/public_ftp/incoming');

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

/**
 * Output a small version of the current account's photo.
 */
function formIcon() {
/**/  echo ($a = r\acct() and $photo = $a->photo) ? $photo : file_get_contents(DRUPAL_ROOT . R_PATH . '/images/icons/account.png');
  exit();
}

/**
 * Complete a "no signin required" request (made by the user clicking a link we sent).
 * @param string code: an encoded serialized array of parameters, possibly including:
 *   op: what to do (REQUIRED)
 *   v: an amount or identifier
 *   mo: month link was created, relative to first rdo (see rdoMonth())
 *   sql: a database request, if any
 *   subs: substitutions for the database request
 *   
 * Possible values for op:
 *   (numeric): record id in r_do -- other parameters for one-time op are serialized in that record's data field
 *      including the true op. The record also includes uid and expiration time.
 *   min: change minimum to $v
 *   sql: do the database operation in $sql
 *   addr: show an address for account QID $v (for shipping or thank-you card)
 *   pay: pay invoice number $v
 * Alternatively, code can be an error message (recognized by having a space in it).
 * @param string $args: urlencoded array of arguments passed from an email (from whence $_POST is not reliably received by Drupal)
 * @see also r\makeDo()
 */
function rdo($code, $args = '') {
  global $channel;
  if (strpos($code, ' ')) return doSay($code);
  if (!$info = r\doCode($code)) return doSay(t('That link is archaic.'));
///  print_r($info);
  if ($once = is_numeric($doid = @$info['op'])) {
    if (!$row = db\lookup('uid,data,expires', 'r_do', 'doid=:doid', compact('doid'))) return doSay(t('That link is no longer valid.')); // already used OR deleted because it expired
    extract($row);
    $info = unserialize($data) + $row; // get new op, etc.
    $a = r\acct($uid);
  }
  extract(u\just('op id v mo expires sql subs', $info));

  if (@$mo) $expires = (r\rdoMonth() > $mo + 3) ? 1 : 0; // finagle expiration if link is older than 3 months
  if (@$expires and r\rTime() >= $expires) return doSay('link expired');

  $channel = TX_LINK;
  $DBTX = \db_transaction();
  
  if ($op == 'min') {
    if ($jid = $a->jid and $a->slave) $a = r\acct($jid); // for joint accounts, update on the master
    $res = $a->update('minimum', $v);
  }
  if ($op == 'nosecret') $res = $a->setBit(B_SECRET, FALSE);
  if ($op == 'debtok') $res = $a->setBit(B_DEBT, TRUE);
  if ($op == 'sql') $res = db\q($sql, $subs);

  if ($op == 'addr') {
    $a = r\acct($v);
    $phone = $a->co ? t('Phone: ') . u\fmtPhone($a->phone) . "<br>" : '';
    $addr = t('Physical address: ') . r\location($a, TRUE);
    return doSay("$a->fullName<br>$a->postalAddr<br>$addr<br>$phone", 'status');
  } 
  
  if ($op == 'inv') {
    if ($err = r\badUnpaidInv($v)) return doSay($err);
    return r\go("handle-invoice/nvid=$v&toMe=1&code=$code");
  } 
  
  if ($op == 'vote' or $op == 'yesno') { // NOTE: about a third of the time, $_POST comes in empty (even from the same computer)
    $issue = $v;
    if ($issue != '20141220') return doSay('link expired');
//    $vote = 0;
//    foreach (u\ray('wl sl wd ed no') as $k => $vv) $vote |= u\setBit($vote, $k, isset($_POST[$vv]));
    extract(u\just('yes no', $args));
    $vote = @$yes ? 1 : 0;
    $info = compact(u\ray('uid issue vote'));
    $vid = db\lookup('vid', 'r_votes', 'uid=:uid AND issue=:issue', compact('uid', 'issue'));
//    $vid = 0;
///debug(compact(u\ray('op args issue yes no vote info vid')));
    if ($vid) db\update('r_votes', $info + compact('vid'), 'vid'); else db\insert('r_votes', $info);
    $say = t('Thank you for your RSVP! ');
    if (@$no) $say .= t('We will miss you. ');
    r\tellStaff('RSVP!', ['coming'=>@$yes ? 'yes' : 'no'], $uid);
    $res = TRUE;
  }
  
  if ($op == 'join') {
    $otherId = $v;
    $joint = r\perm(B_JOINT);
    if  (r\relation(1, $otherId, $a->id, 'permission=:joint', compact('joint'))) { // inviter still wants it?
      $a->unjoin(); // zap any other joins or join requests
      if ($reid = r\relation('reid', $a->id, $otherId)) {
        $info = ['permission'=>$joint, 'draw'=>0] + compact('reid');
      } else $info = u\ray('main other permission draw', $a->id, $otherId, $joint, 0);
      db\update('r_relations', $info, 'reid', TRUE); // create relation or just set permission
      $a->join(r\acct($otherId)); // remember who is joined
      $res = TRUE;
    }
  }

  if (!@$res) return doSay(t('Request failed.'));
  if ($once) db\q('DELETE FROM r_do WHERE doid=:doid', compact('doid'));

  unset($DBTX);

  return doSay(@$say . t('Your request was successfully completed.'), 'status');
}

/**
 * Automatically deactivate an account (called when the user changes the Dwolla email or password).
 */ /*
function coup($usdEmail) {
  list ($email) = explode('@', $usdEmail);
  list ($code, $region, $id) = explode('.', $email);
///  if (!$a = r\acct(strtoupper("$region.$id"))) die('no1');
///  if (@$a->secure['usdEmail'] != $usdEmail) die('no2');
  if ($a->usd > 0) { // store its USD in ctty fund (single transfer for speed? many for hard-to-contest?)
    if (!r\tradeR(min(R_MAX_DAILY_EXCHANGE, $a->usd), $a->community, $a->id, $err) or $a->usd > R_MAX_DAILY_EXCHANGE) {
      r\tellStaff("Immediate coup threat: $a-fullName $$a->usd ($err)");
    }
  }
  $a->suspend(t('your Dwolla PIN, email, and/or password was changed.'));
  exit('ok');
}
*/

/**
 * Generate choices for a member autocomplete field
 * @param string $string: what the user has typed so far
 * @param string $role: 'payer', 'payee', 'industry', 'company', or '' (other), depending on the role of the user
 * @param int $myid: the current account (globals and session variables are inappropriate here)
 * @todo: ditch this altogether or make it efficient with separate rCredits backend (put it in the api)
 *    maybe even use .htaccess to redirect the autocomplete URL to the backend? privacy is not crucial here
 * @patched: Drupal bug which gets Ajax error on any input that matches (.*)/?.([^\.]|\..)
 */
function auto($role = '', $myid = 0, $string = '') {
//  return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string"));
  if (!$string) return;
  $string0 = $string;
  $string = '%' . u\shortName($string, '%') . '%'; // this also prevents malicious database injection
//  $string = str_replace(' ', '%', \db_like(" $string "));
  if (!is_numeric($myid)) return;
  $params = 'string'; // list of substitution parameters for query (each section below can add additional params)
  
  if (u\abbreviates('paye', $role)) {
    $other = $role == 'payer' ? 'payee' : 'payer';
/*  attempt to show recently used members first (add myid to dbQ query subs)
   AND (t.$role IS NULL OR t.$role=:myid) ORDER BY t.created DESC, u.fullName
    $selection = <<<EOF
      u.uid, u.fullName AS display, u.email 
      FROM users u LEFT JOIN r_txs t ON t.$other=u.uid 
      WHERE (u.name LIKE :string OR u.email LIKE :string)
      AND (t.$role IS NULL OR :myid IN (t.payer, t.payee))
      ORDER BY t.created DESC, u.fullName
*/
    $selection = <<<EOF
      u.uid, u.fullName AS display 
      FROM users u WHERE (u.uid>1 AND u.name LIKE :string)
EOF;
//      FROM users u WHERE (u.uid>1 AND (u.name LIKE :string OR u.email LIKE :string))
  } elseif ($role == 'industry') { // the one place we don't need to include the qid (for definiteness)
    if (strlen($string) > 6) {
      $soundex = soundex($string0) . '%';
      $soundex = " OR SOUNDEX(industry) LIKE '$soundex'";
    } else $soundex = '';
    $string = u\shortName(" $string0 ", '%');
    $selection = <<<EOF
      industry COLLATE utf8_general_ci AS display FROM r_industries WHERE (industry LIKE :string $soundex)
      UNION SELECT fullName COLLATE utf8_general_ci AS display FROM users WHERE :IS_CO AND name LIKE :string
EOF;
  } elseif ($role == 'company') {
    $selection = 'uid, fullName AS display FROM users WHERE :IS_CO AND name LIKE :string';
  } elseif (u\abbreviates('ctty', $role)) { // ctty or cttyPerson
    $mya = r\acct($myid);
    $cttyCrit = ($cttyId = $mya->community) ? "AND community=$cttyId" : '';
    $personal = $role == 'ctty' ? '' : ' AND NOT :IS_CO';
    $selection = "uid, fullName AS display FROM users WHERE name LIKE :string $cttyCrit $personal";
  } else $selection = 'uid, fullName AS display FROM users WHERE name LIKE :string';
  $sql = "SELECT DISTINCT HIGH_PRIORITY $selection AND uid>1 ORDER BY display LIMIT 10"; // omit communities and regions
//return drupal_json_output(array('zot' => "role=$role myid=$myid string=$string sql=$selection"));
///return drupal_json_output(array('zot' => "subs=" . print_r(compact(u\ray('string myid')), 1)));

  $result = db\q($sql, compact(u\ray($params)));
  $matches = [];
  while ($row = $result->fetchAssoc()) {
    extract($row); // ($uid), $display
    if (@$uid) $display = r\quid($uid) . '   ' . $display;
// (never show email) $display .= @$email ? " ($email)" : '';
    $matches[$display] = check_plain($display);
//    return drupal_json_output(array('zot' => "display=$display"));
  }
  drupal_json_output($matches);
}

/**
 * List the months for which notices are available for the current account, with links.
 */
function formNotices($form, $sta, $month = '') {
  return $month ? notice($month) : monthChoices(t('Notices'), 'notices', '', 0, '');
}

/**
 * List notices for a given month.
 * @param string $month: mmmyyyy
 */
function notice($month = '') {
  if (!$start = strtotime("1$month")) return r\go('empty', t('That is not a recognizable month.' . $month), 'ERR');
  $start = u\monthDay1($start);
  $end = strtotime('+1 month', $start) - 1;

  $mya = r\acct();
  if ($noticeArgs = r\noticeArgs($mya->id, "created BETWEEN $start AND $end", FALSE, TRUE)) {
    extract(u\just('messages', $noticeArgs));
  } else $messages = '';
  $title = item(t('Notices for ') . strftime('%B %Y', $start));
  $list = item("<table>$messages</table>");
  return compact('title', 'list');
}

/**
 * Generate an account statement for the given month.
 * @param string $month: mmmyyyy
 */
function statement($month) {
  include_once __DIR__ . '/../rcredits-txs.inc';
  $purposeLenMax = 60; // maximum number of characters in purpose field, not including added "..."

  if (!$start = strtotime("1$month")) return r\go('empty', t('That is not a recognizable month.'), 'ERR');
  $start = u\monthDay1($start);
  $end = u\plusMonths(1, $start) - 1;
  if (!$mya = r\acct()) return r\go('', t('You must first sign in.'), 'ERR');

  include_once __DIR__ . '/../statement.class';
  
  $pdf = new r\Statement($mya, $start);

  $size = $pdf->getFontSizePt();
  $m = 5; // left/right margin size
  $pdf->SetMargins($m, PDF_MARGIN_TOP, $m);
  $pdf->SetAutoPageBreak(TRUE, 15); // leave space at bottom
  $pdf->setLanguageArray($l); // (eg "page" in footer)
  $pdf->AddPage();
  $pageWidth = $pdf->getPageWidth();

/*  $pdf->SetLineStyle(array('width' => 0.5, 'cap' => 'butt', 'join' => 'miter', 'dash' => 0, 'color' => array(0, 0, 0)));
  $pdf->RoundedRect(91, 30, 60, 60, 3.50, '1111', 'DF', NULL, array(255, 255, 255)); // x, y, w, h, ?, fill?
  */
  //$pdf->Cell(0, 0, 'Page ', 0, false, 'L', 0, '', 0, false, 'T', 'M');
  //$pdf->writeHTMLCell(40, 10, 160, 84.5, '<div style="font-size:44px;">rCredits.org</div>');
  list ($address, $city, $state, $zip) = u\parseAddr($mya->postalAddr);
  $pdf->setX($m + 10);
  $pdf->Cell(0, 0, $mya->j_fullName, '', 2);
  $pdf->Cell(0, 0, $address, '', 2);
  $pdf->Cell(0, 0, "$city, $state $zip", '', 2);
  
  //$pdf->Line($m, $y = $pdf->getY(), $pageWidth - $m, $y);
  
  $pdf->setX($m);
  $pdf->setY($pdf->getY() + 10);

  list ($header, $classes, $rows, $tot) = x\getTxRows($start, $end); // get the transaction data
  extract(u\just('frombank fromyou toyou reward', $tot));
  $bal0 = be\creditInfo(u\ray('asof', $start - 1))->balance; // balance BEFORE start of this period
  $info = be\creditInfo(u\ray('asof', $end));

  list ($bal9, $rewardsEver) = array($info->balance, $info->rewards);
//  u\EXPECT($bal9 == round($bal0 + @$frombank - @$fromyou + @$toyou + @$reward, 2), 'balance mismatch');
/**/  if (round($bal9, 2) != round($bal0 + @$frombank - @$fromyou + @$toyou + @$reward, 2)) return r\go('empty', 'balance mismatch: ' . print_r(compact('tot', 'bal0', 'bal9'), 1));
  
  $pdf->barHead('SUMMARY');
  $fields = explode('; ', 'Starting Balance/bal0/32/; /plus/7/R; From Bank/frombank/25/C; /minus/7/C; FROM You/fromyou/25/C; /plus/7/C; TO You/toyou/25/C; /plus/7/C/; Rewards/reward/25/C; /equals/7/R; Ending Balance/bal9/33/R');
  $pdf->colHeads($fields);

  list ($plus, $minus, $equals, $space) = array('+', '-', '=', ' ');
  $committed = $mya->j_committed;
  $avg = r\averageBalance($mya->id, $mya->created, $end, TRUE);
  if ($jid = $mya->jid and $j = r\acct($jid)) $avg += r\averageBalance($j->id, $j->created, $end, TRUE);
  
  $yield = number_format($avg ? 100 * $rewardsEver / $avg : 0, 1) . '%';
  foreach (u\ray('bal0 frombank fromyou toyou reward bal9 rewardsEver committed') as $one) $$one = number_format(@$$one, 2);

  $pdf->setFontSize(.9 * $size);
  foreach ($fields as $one) {
    list ($head, $fieldName, $width, $align) = explode('/', $one);
    $pdf->Cell($width, 0, $$fieldName, '', $fieldName == 'bal9' ? 1 : 0, $align);
  }

  list ($labelWidth, $numWidth) = array(45, 25);
  $pdf->newLine();
  $pdf->Cell($labelWidth, 0, 'Incentive Rewards Ever:');  
  $pdf->Cell($numWidth, 0, $rewardsEver, '', 0, 'R');
  $pdf->Cell(0, 0, " (Effective yield $yield APR)", '', 1, 'L');
  if ($mya->id == CGF_ID) {
    $shared = db\lookup('SUM(amount)', 'r_txs', 'payee=:CGF_ID AND payeeFor=:R_SHARING_MSG AND created<=:end');
    $pdf->Cell($labelWidth, 0, 'Sharing Donations Ever:');  
    $pdf->Cell($numWidth, 0, $shared, '', 1, 'R');
  } elseif ($start == u\monthDay1()) { // committed amount is not available retroactively
    $pdf->Cell($labelWidth, 0, 'Reserved to donate to CGF:');  
    $pdf->Cell($numWidth, 0, $committed, '', 1, 'R');
  }
  $pdf->newLine();

  $pdf->setFontSize($size);
  $pdf->barHead('DETAILS');
  $fields = u\ray('Tx#/tid/15/C /space/1/ Date//17/ Name//55/ /space/1/ Purpose//70/ /space/1/ Amount//20/R *Extra/extra/20/R');
  $pdf->colHeads($fields);

  $pdf->setFontSize(.9 * $size);
  foreach ($rows as $row) {
    foreach ($row as $k => $v) $row[$k] = str_replace(',', '', strip_tags($v));
    extract(u\just('tid date name fromyou toyou purpose reward', array_combine($classes, $row)));
    $amount = number_format($toyou - $fromyou, 2);
    if (strlen($purpose) > $purposeLenMax + 3) $purpose = substr($purpose, 0, $purposeLenMax) . '...';
    if ($reward == '--') $reward = 0;
    $extra = number_format($reward, 2);
    foreach ($fields as $one) {
      list ($head, $fieldName, $width, $align) = explode('/', $one);
      if (!$fieldName) $fieldName = strtolower($head);
      $debit = ($align == 'R' and is_numeric($n = str_replace(',', '', $$fieldName)) and $n < 0);
      if ($debit) $pdf->SetTextColor(128, 0, 0); else $pdf->SetTextColor();
      $pdf->Cell($width, 0, $$fieldName, '', $head == '*Extra' ? 1 : 0, $align, '', '', 1);
    }
    if ($toyou != $fromyou) x\addCat($cats, $purpose, $toyou - $fromyou, $dups);
    $catDesc = r\usdin($purpose) ? t('exchange fees') 
    : ($purpose == INFLATION_DESC ? $purpose : t('rewards, not including inflation adjustment'));
    if ($reward) x\addCat($cats, $catDesc, $reward, $zot);
  }
  
  $pdf->newLine();
  $pdf->Cell($pageWidth, 0, t('* The Extras column includes incentive rewards and any fees paid or received.'), '', 1 , 'L');
  
  if (@$dups) { // don't show categorization unless it's worthwhile
    $pdf->setFontSize($size);
    $pdf->newLine();
    $pdf->barHead('CATEGORY TOTALS');
    $fields = u\ray('Category/cat/75/ Total/total/25/R Count/count/17/R');
    $pdf->colHeads($fields);
    $pdf->setFontSize(.9 * $size);

    x\finishCats($cats);
    
    foreach ($cats as $cat => $ray) {
      list ($count, $amount) = $ray;
      $total = number_format($amount, 2);
      foreach ($fields as $one) {
        list ($head, $fieldName, $width, $align) = explode('/', $one);
        $pdf->Cell($width, 0, $$fieldName, '', $head == 'Count' ? 1 : 0, $align, '', '', 1);
      }
    }
  }
  
  if (u\test()) $pdf->Close(); else $pdf->Output('rCredits-ID-Card.pdf', 'I'); //Close and output PDF document
}

define('R_GIFT_SHOWHIDE', "var other = jQuery('.form-item-amount'); if(jQuery('#edit-gift').val() == 0) {other.show();} else other.hide();");

function formDonate($form, &$sta) {
  $mya = r\acct();
  $donated = $mya->share ? (t('<p>You have already made a donation. Thank you! Please feel free to donate again. Whatever "share" percentage you choose this time will replace your previous choice') . " ($mya->share%).</p>") : '';
  $giftLevels = array(
  // 10000 => t('Marble .. $10,000'),
  // 5000 => t('Granite ... 5,000'),
    2500 => t('Slate .... $2,500'),
    1000 => t('Glass ..... 1,000'),
     500 => t('Iron ........ 500'),
     250 => t('Oak ......... 250'),
     100 => t('Two Bricks .. 100'),
      50 => t('One Brick .... 50'),
      25 => t('Half Brick ... 25'),
       0 => t('Water ... (other)'),
  );
  $oftens = array(0 => 'when?', 'M' => 'Monthly', 'Q' => 'Quarterly', 1 => 'Just Now (maybe more later)');
  $honors = array('-' => '(optional)', 'honor' => 'in honor of', 'memory' => 'in memory of', 'other' => 'other');
  $onchange = str_replace('show()', "show(); jQuery('#edit-amount').focus()", R_GIFT_SHOWHIDE);
  $giftAttrib = attrib(compact('onchange'));
   
  $delay = $mya->ok ? '' : t(' The amount you choose will come from your account once it is activated and has enough funds.</p>');
  $form = array(
    'title' => item(t('Donate to Common Good Finance')),
    'subtext' => item($donated . t('<p>Make a gift of any size, to help support the rCredits system. Buy a BRICK or some other piece of the Common Good Economy, as a tax-deductible donation (one-time, monthly, or quarterly). <b class="loud">NOTE: This is a donation, not a deposit.</b>') . "$delay</p><p>" . t('Also choose a percentage of your ongoing incentive rewards to share with Common Good Finance.') . t('</p><p>Thank you for your support!</p>')),
    'gift' => formField('select', t('Donation:'), '', required($mya->co ? R_COMPANY_GIFT : R_INDIVIDUAL_GIFT) + $giftAttrib, $giftLevels),
    'amount' => textField(t('Other amount $'), ''),
    'often' => formField('select', t('When:'), '', required(), $oftens),
    'honor' => formField('select', t('Honoring:'), '', '', $honors),
    'honored' => formField('textarea'),
    'share' => w\shareField(),
    'submit' => submit('Donate'),
  );

  js(R_GIFT_SHOWHIDE, 'inline', 'footer');
  return labeled($form);
}

function formDonate_validate($form, &$sta) {
  js(R_GIFT_SHOWHIDE, 'inline', 'footer');
  extract(u\just('gift amount share often', $sta['input']));
  if (!$amount = $gift ?: $amount) return say('missing field', array('field' => 'amount'), 'amount');
  if (!$often) return say('missing field', array('field' => 'when'), 'often');
  if ($err = u\badAmount($amount, '>0')) return say($err, 'amount');
  if ($err = w\badShare($share)) return say($err, 'share');
//  $share = min($share, 999.999); // any bigger kills PDO
  $sta['input'] = compact(u\ray('amount share')) + $sta['input'];
}

function formDonate_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('amount often honor honored share', $sta['input']);
  if ($info['honor'] == '-') $info['honor'] = 'honor';
  if (!$info['honored']) $info['honor'] = '';
  $uid = $mya->id;
  $giftDate = r\rTime();
  $info += compact('uid', 'giftDate');
  $donid = db\insert('r_gifts', $info);
  $mya->update(u\just('share', $info));
  $msg = 'gift successful';
  if (!r\acceptGift($info + compact('donid'))) $msg .= '|gift transfer later';

  $amount = u\fmtAmt($info['amount']);
  if (!$mya->ok) {
    $mya->stepDone('donate', $info);
    say("$msg|step completed", compact('amount'));
    r\go('status');
  } else {
    r\tellStaff('gift', $info);
    say($msg, compact('amount'));
  }
}

/**
 * Show general or specific help. For members seeking general help, provide a secure communications channel.
 */
function formHelp($form, &$sta, $what = '') {
  include_once __DIR__ . '/rweb-help.inc';

  $signedIn = ($mya = r\acct()) ? TRUE : FALSE;
  if ($signedIn and $mya->admin and $what and strpos($what2 = u\deurlify($what), '&')) {
    extract(u\just('filename type', $what2));
    $s = file_get_contents($filename);
    list ($zot, $type) = explode('/', "$type/");
    $ext = $type ?: 'bin';
    u\beginDownload("from $mya->id.$ext");
/**/ echo $s;
    exit();
  }
  
  list ($what, $tag) = (strpos($what, '@')) ? explode('@', $what) : array($what == 'other' ? '' : $what, '');
  $title = item($what ? str_replace('Rcredits', 'rCredits', ucwords(str_replace('-', ' ', $what))) : t('General Help'));
  $stranger = t('Interested in rCredits? Visit <@a>rCredits.org</a> for more information or to request an invitation to participate.', ['@a' => 'a href=http://rCredits.org']);
  $text = formField('item', '', '', ($signedIn or $what)? helpText($what ?: 'general', $tag) : $stranger);
  if ($signedIn and !$what) {
    if ($mya->admin) {
      $q = db\q('SELECT id, message AS msg FROM r_tous WHERE uid=:uid', ['uid' => $mya->id]);
      while ($row = $q->fetchAssoc()) {
        extract($row);
        $message[] = "#$id: " . u\decrypt($msg);
      }
      $message = $message ? join("<br><br>", $message) : '';
      $message = item($message, t('All messages:'), t('Be sure to delete resolved messages.'));
    } else $message = formField('textarea', t('Send a secure message to an rCredits Administrator:'), '', required());
    $file = formField('file', t('Attachment:'), t('(optionally) attach a file to your message. The file will also be encrypted.'));
    $submit = submit(t('Send'));
  }

  return compact(u\ray('title text message file submit'));
}

function formHelp_validate($form, &$sta) {
  if ($err = $_FILES['files']['error']['file'] and $err != 4) return say('file save error', 'file');
}

function formHelp_submit($form, &$sta) {
  $mya = r\acct(); $uid = $mya->id;
  extract(u\just('message file', $sta['input']));
  $time = r\rTime();

  foreach ($_FILES['files'] as $k => $v) $file[$k] = $v['file'];
  if ($tmp = $file['tmp_name']) { // got an attachment
    $filename = UPLOAD_DIR . "/$uid-$time";
    extract(u\just('name type size', $file));
    $link = BASE_URL . '/help/' . u\urlify("type=$type&filename=$filename");
    $extra = " [<a href=\"$link\">attachment</a>: $name type=$type size=$size bytes]";
    $message .= $extra;
    if (!rename($tmp, $filename)) return say('file save error', 'file');
  }  
  $message = u\crypt($message);
  db\insert('r_tous', compact('uid', 'time', 'message'));
  r\tellStaff(t('secure message ') . @$extra);
  say(t('Your secure message has been sent. An administrator will respond as soon as possible.'));
}

/**
 * Show the account's current membership status and steps to get to the next milestone.
 * @param acct $a: the account to test if just updating membership status bits -- otherwise empty
 * @param string $code: numeric means show specific screen even if not appropriate (for in-person demos)
 * @return TRUE if the member has done everything possible toward opening the account.
 */
function formMembership($a, &$sta = '', $code = '') {
  global $base_url;

  if (!($updating = (bool) $a) and !$a = r\acct()) return NULL; // no account available
  if ($updating and $a->ok) return TRUE;

  $myid = $a->id;  
  $co = $a->co;
  $ok = $a->ok;
  $mempage = is_numeric($code) ? $code : FALSE;
  extract($a->stepsDone ?: r\stepsDone0(), EXTR_PREFIX_ALL, 'ok');

  if (!$ok or !$a->can(B_MEMBER)) {
    $ready = (@$ok_sign and $ok_contact and $ok_donate and $ok_photo and $ok_prefs and $ok_connect); // @ for tests
    $ready = ($ready and ($co? ($ok_company and $ok_relations) : $ok_proxies));
  } else $ready = TRUE;

  if ($updating) return $ready;
  
  $message1 = $message2 = '';

  if ($ready and !$ok) {
    $nextMsg = $co ? t('you can begin accepting rCredits payments') : t('you can expect to receive your rCard in the mail within a day or two');
    $message1 = t("<h2>Your Account Setup Is Complete</h2><p>You have done everything you need to do. Once a staff member has approved your account, $nextMsg.</p><p>Thank you for joining us!</p>");
  } elseif (!$ok) { // not a member yet (no rCard, no vote)
    $message1 = ($ok_photo or $ok_sign or $ok_donate or ($ok_proxies and !$co) or $ok_contact or ($ok_connect and !$co) or $ok_prefs) ? 
          t("<h2>You're getting there!</h2><p>")
        : t('<h2>Welcome to rCredits</h2><p>Thanks for joining us. ');
    $message1 .= $co ? t('Before you can begin accepting rCredits payments, you must complete the following steps (in any order).') 
      : t('Before we can send your rCard, and before you can vote on community funding, you must complete the following steps (in any order). This usually takes about 20 minutes.');
    $message1 .= tt(' If you need help, please <@a1>send us an email</a>.</p>', ['a1' => 'a href="mailto:' . r\regionField('email') . '" target="_blank"']);

    $stepContact = t('Complete your <a>Contact Information</a>.');
    $stepAgree = t('Sign the <a>rCredits Agreement</a>.');
    $stepGift = t('Make a <a>Donation</a> of <i>any size</i>, to support the rCredits system.');
    $stepProxies = t('<a>Choose two people</a> to represent your views whenever you are not there, to discuss and vote on community funding issues. (A list of members in your area will be provided, to choose from.)');
    $stepPrefs = t('Set your <a>Account Preferences</a>.');
    $stepPhoto = t('Upload a <a>Photo</a> for your ') . ($co ? t('company profile.') : t('Member ID Card.'));
/*    $stepSecurity = t('<a>Provide your security information.</a> Upload ')
      . ($co ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotSecurity ? t(' <span class="pending">(DONE, pending approval)</span>') : ''); */
    $stepBank = t('Connect your <a>Bank Account</a> (your doorway to the mainstream economy) or choose not to for now.');
/*
    $stepProof = t('<a>Prove your identity.</a> Upload ')
      . ($co ? t('your Charter, Articles of Organization, or similar document.')
        : t('a photo of your driver\'s license or other official ID.'))
      . ($gotIdProof ? t(' <span class="pending">(DONE, pending approval)</span>') : '');*/
    $stepCompany = t('Tweak your <a>Company Information</a>.');
    $stepRelations = t('Set up <a>Relations</a> to individuals who will use or manage this account.');

    $stepNum = 1;
    $message2 = $ok_contact ? '' : memberStep(0, $ok_contact, $stepContact, 'account/contact');
    $message2 .= memberStep($stepNum++, $ok_sign, $stepAgree, 'agreement')
              . memberStep($stepNum++, $ok_donate, $stepGift, 'community/donate');
              
    if (!$co) 
    $message2 .= memberStep($stepNum++, $ok_proxies, $stepProxies, 'account/proxies');

//    $message2 .= memberStep($stepNum++, $gotSecurity, $stepSecurity, 'account/security');
    $message2 .= memberStep($stepNum++, $ok_prefs, $stepPrefs, 'account/preferences')
               . memberStep($stepNum++, $ok_photo, $stepPhoto, 'account/photo');

    $message2 .= memberStep($stepNum++, $ok_connect, $stepBank, 'account/bank');
//    if (!$co) $message2 .= memberStep($stepNum++, $gotIdProof, $stepProof, 'account/id-proof');
    if ($co)
    $message2 .= memberStep($stepNum++, $ok_company, $stepCompany, 'account/company')
               . memberStep($stepNum++, $ok_relations, $stepRelations, 'account/relations');

// (Suggest it after 1mo) if (!$co) $message2 .= memberStep(4, $invited, $step4, 'invite');
  } elseif (FALSE) {
    $message1 = t('<h2>You have completed all membership steps.</h2><p>Within 24 hours, you should receive an email saying your account is approved and you can expect your rCard in the mail within a day or two. Hurray!</p>'); // UNUSED
  } else {
    $bonusMsg = ($a->can(B_BONA) or ($a->bank and $a->j_minimum > 0)) ? '' : t('<p>To receive your $@BONUS signup bonus, you must have a way to get money into your account. Ask someone to pay you with rCredits, or <@a>transfer funds from your bank account</a>, or trade US Dollars for rCredits at a participating business. As soon as you get some money in, your signup bonus will arrive.</p>', ['@BONUS'=>R_SIGNUP_BONUS, '@a'=>"a href=$base_url/get"]);
    $personMsg = !$a->co ? t(' You are also responsible for guiding your local economy, together with hundreds of other rCredits members -- setting investment and grant-making priorities, electing local directors for your Common Good Community, planning a sustainable local prosperity for all residents, and offering help to communities elsewhere.</p>') . $bonusMsg : '</p>';
    $message1 = t('<h2>Congratulations! Your account is Activated.</h2><p>You are authorized to use rCredits to buy and sell with other members.') . $personMsg;
  }
  $message = $message1 . "<table width=\"100%\">$message2</table>";
  
  $title = item(t('Membership Steps'));
//  $subtext = item(t('for ') . $a->fullName);

  $steps = formField('item', '', '', $message);
  $form = compact(u\ray('title steps addendum'));
  return labeled($form);
}

/**
 * Give member a chance to sign the rCredits Agreement. (agreement revised 4/17/2014)
 */
function formAgreement($form, &$sta) {
  $mya = r\acct();
  $title = item(t('rCredits Agreement'));
  $name = 'checkNUMBER';
  $id = "edit-$name";
  $check = boxField('', '', attrib(compact('id', 'name')));
  $model = \render($check);
  $instructions = '';

  if (!$mya) {
    $instructions = '';
  } elseif ($date = $mya->signed) {
    $instructions = '';
    $date = u\fmtDate($date);
    say('signed agreement', compact('date'));
    $signedBy = item("Signed by <b>$mya->signedBy</b>, $date");
  } elseif (!$mya->co and !$mya->proSe and !$mya->admin) { // can't sign for someone else!
    say('self must sign', 'signedBy');
  } else {
    $aacct = $mya->cAdmin ? ($mya->co ? r\acct($mya->helper) : $mya) : $mya->agent;
    $signedBy = textField(t('Signed:'), t('Type your full name here ') . "($aacct->legalName)", required($mya->cAdmin ? $aacct->legalName : ''));
    $submit = submit(t('Sign'));
    $addendum = t('This means, for example, paying attention to your experience and making comments to improve the system.');
    if ($mya->cAdmin) $addendum .= t('</p><p>ADMINISTRATORS need not put checkmarks in the boxes.');
    
    $instructions = tt(<<<EOF
      <div id='instructions'>
      <p><b>Instructions:</b> Put a check mark in each checkbox to indicate you have read that section and agree to it. Really DO read the agreement and make sure you understand it. This agreement, that we make with each other, is the foundation for the rCredits system and the Common Good Economy.</p>

      <p><b>Summary.</b> In the first part of the agreement (A), you acknowledge the intent and purpose of the rCredits system. In the second part (B), you agree to use the rCredits system the way it is designed to work. Notice you can stop participating at any time, simply by giving back the rCredits you got for free (B4).</p>
      
      <p><b>Responsibility.</b> Since the purpose of the rCredits system is to give us control of our local economy, as a community, the agreement requires that each member be responsible. You agree to take on that responsibility "as [your] life circumstances allow".@addendum</p>
      </div>
EOF
    , compact('addendum'));
  }

  $personOnly = ($mya and $mya->co) ? '' : tt(<<<EOF
  <li>[6] I agree to participate responsibly in managing the Common Good Economy to the best of my ability and as my life circumstances allow, including:
    <ol style="list-style-type:lower-alpha;">
      <li><a href="@BASE_URL/help/paying-attention">paying attention</a> and understanding how the Common Good Economy works</li>
      <li>participating in the <a href="@BASE_URL/help/decision-principles">decision process</a> to set my community's funding priorities:
        <ol style="list-style-type:lower-roman;">
          <li>to foster healthy, fulfilling lives for all community members and</li>
          <li>to assist communities elsewhere</li>
        </ol></li>
      <li>participating in <a href="@BASE_URL/help/other-decisions">other decisions</a> from time to time, as needed.</li>
    </ol>
  </li>
EOF
  );

  $agreement = tt(<<<EOF
@instructions
<div id="agreement">
<h2>A. I recognize that:</h2>
<ul>
  <li>[-2] rCredits are intended as the basis for the Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</li>
  <li>[-1] Together as a community we have the power to <a href="@BASE_URL/help/issuing-rcredits">issue credit</a> for our own use as a medium of exchange. It is our promise to accept rCredits that gives them value.</li>
  <li>[0] Governance by the people is essential for a just society.</li>
</ul>

<h2>B. As a responsible participant in the Common Good Economy:</h2>
<ol>
  <li>[1] As long as I can spend my rCredits easily or exchange them easily for US Dollars, I will accept them without limit as payment for my goods and services. I will exchange rCredits for US Dollars only if I cannot spend them easily.</li>
  <li>[2] I will accept rCredits from members of any <a href="@BASE_URL/help/common-good-community">Common Good Community</a> in good standing. Once my geographic area becomes a Common Good Community, in the event of a <a href="@BASE_URL/help/cash-crunch">cash crunch</a> I will <a href="@BASE_URL/help/backing-rcredits">back</a> rCredits up to my average balance over the past six months.</li>
  <li>[3] Whenever I exchange rCredits for US Dollars or vice versa, I will exchange them one for one, and I will price my goods and services the same in rCredits and in US Dollars. </li>
  <li>[4] I understand I will receive some rCredits from the community as Incentive Rewards that I can spend but cannot <a href="@BASE_URL/help/cashing-out">cash out</a>. If I stop participating (or <a href="@BASE_URL/help/graceful-failure">if the system ends</a>) within 6 months of receiving my first rCredits or before the community has enough revenue to cover the Incentive Rewards, I will <a href="@BASE_URL/help/graceful-failure">immediately give back</a> whatever amount of Incentive Rewards I have received (while keeping any rCredits I received from other participants).</li>
  <li>[5] I promise to maintain a high level of honesty, integrity, and <a href="@BASE_URL/help/ethics">ethics</a> in my dealings with other rCredits participants and with the community (including the rCredits system itself). When there is a dispute, I will follow the <a href="@BASE_URL/help/dispute-resolution-process">Dispute Resolution Process</a> and will honor its outcome.</li>
  @personOnly
</ol>
</div>
EOF
  , compact('instructions', 'personOnly'));
//  <li>[5] I will move my deposits and investments to the Common Good Economy when that is sensible, in my judgment.</li>

  for ($i = R_AGREE_0; $i <= R_AGREE_9; $i++) {
    $box = str_replace('checkNUMBER', "check$i", $model);
    $agreement = str_replace("[$i]", (!$mya or @$date) ? '' : $box, $agreement);
  }
  $agreement = formField('item', '', '', $agreement);
  $form = compact(u\ray('title agreement signedBy submit'));
  return $form;
}

function formAgreement_validate($form, &$sta) {
  $mya = r\acct();
  $i9 = !$mya->co ? R_AGREE_9 : R_AGREE_9 - 1;
  if (!$mya->cAdmin) for ($i = R_AGREE_0; $i <= $i9; $i++) if (!@$sta['input']["check$i"]) return say('incomplete agreement', "check$i");
  $legalName = r\agent()->legalName;
  if (!$mya->cAdmin and strcasecmp(trim($sta['input']['signedBy']), $legalName) != 0) return say('bad signature', compact('legalName'), 'signedBy');
}

function formAgreement_submit($form, &$sta) {
  $mya = r\acct();
  $signed = r\rTime();
  extract(u\just('signedBy', $sta['input']));
  r\acct()->update($info = compact('signed', 'signedBy'));
  $mya->stepDone('sign', $info);
  say('info saved|step completed');
  r\go('status');
}

/**
 * Choose one proxy.
 * @param int $priority: 1 for proxy, 2 for alternate
 * uid is not passed, for security.
 */
function formProxy($form, &$sta, $priority = '') {
  if ($priority != 1 and $priority != 2) w\hack('bad proxy priority');
  $dft = r\acct()->proxy($priority);
  if (!$choices = w\proxyChoices($dft)) r\go('account/proxies', 'no local proxy');

  $title = $priority == 1 ? t('Your #1 Proxy') : t('Your Alternate Proxy');
  $title = item($title . t(' (choose one)'));
  $subtext = item(t('Choose someone you know and trust. Top suggestions for you are highlighted.'));
  $list = item($choices);
  $submit = submit();
  $priority = hidField($priority);
  $js = <<<EOF
jQuery("input[name='proxyChoice']").change(function() {
  jQuery("#saywait").show();
  jQuery("#edit-submit").click();
});
EOF;
  js($js, 'inline', 'footer');
  return compact(u\ray('title subtext list submit priority'));
}

function formProxy_validate($form, &$sta) {
  extract(u\just('priority proxyChoice', $sta['input']));
  if ($proxyChoice == r\acct()->proxy(1+2 - $priority)) r\go('account/proxies', 'doubled proxy', 'err'); // can't just return say() because js gets deactivated (drupal bug?)
}

function formProxy_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('priority proxyChoice', $sta['input']));

  $mya->proxy($priority, $proxyChoice);
  if ($mya->proxy(1+2 - $priority)) { // do we already have the other proxy?
    $mya->stepDone('proxies');
    r\go($mya->ok ? 'account/proxies' : 'status', 'info saved');
  } else say('proxy to go');

  r\go('account/proxies', 'info saved');
}

/**
 * Choose default proxies.
 */
function formProxies($form, &$sta) {
  $mya = r\acct();
  $title = item(t('Proxies'));
  $afterOk = $mya->ok ? '' : ', once your account is approved,';
  $subtext = item(tt(<<<EOF
    <p><b>Direct democracy</b> is at the heart of the rCredits design. In each community, members meet to discuss and decide on funding priorities for incentives, grants, loans, and investments of rCredits for the greater good.</p>
    <p>You assign a person in your community as your ongoing Representative (Proxy). This should be a member you know and trust and generally agree with. Whenever you don't vote, your Representative votes for you. The Representative's vote simply counts double (one vote for them and one vote for you). If your Representative also fails to vote, then your Representative's Proxy votes for all three of you.<br><br>Everyone's voice is counted, whether or not they vote directly &mdash; one person one vote.</p>
    <p>Our community's most trusted Representatives (chosen by many people to be their proxy) will meet in person to research and debate specific issues. These trusted Representatives ("Trustees") do not decide broad policy issues &mdash; all members decide the policy issues. Instead, Trustees oversee public discussions, choose wording for questions to be voted on, take action as directed by the members, and assure that the system is running smoothly.</p>
    <p>If you don't see listed here the people you want, choose someone else temporarily, then$afterOk use the <a href="@BASE_URL/community/invite">Invite</a> tab to invite who you most want to represent you.</p>
EOF
  ));

  $mya = r\acct();
  foreach ([1, 2] as $priority) {
    if ($p = $mya->proxy($priority)) $p = r\acct($p)->fullName; else $p = t('(none chosen)');
    $proxy[$priority] = $p . t(' [<@a>show choices</a>]', ['@a'=>"a href=proxy/$priority"]);
  }
  $proxy1 = item($proxy[1], t('Proxy:'), t('Who will vote on your behalf, when you don\'t vote directly? (You can change your choices at any time.)'));
  $proxy2 = item($proxy[2], t('Alternate:'), t('Who will vote on your behalf, if your Proxy doesn\'t vote either? (even before resorting to your Proxy\'s Proxy)'));
  $form = compact(u\ray('title subtext proxy1 proxy2'));
  return labeled($form);
}

/**
 * Invite someone to sign up for rCredits (on behalf of the account, not the agent).
 * NOTE: We allow agents to send invitations on behalf of a company, but be aware that companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever it invites -- something usually reserved for humans.
 */
function formInvite($form, &$sta) {
  global $base_url;
//  if ($agent = r\agent()) {
  if ($mya = r\acct()) {
    $cAdmin = $mya->cAdmin;
    if ($admin = $mya->admin) $mya = r\acct(R_ADMIN_QID); // use admin's personal info
    $stepsDone = (object) $mya->stepsDone;
    $nearly = ($admin or $mya->can(B_MEMBER)) ?: (@$stepsDone->sign and @$stepsDone->donate);
  } else $cAdmin = FALSE;
  
  $title = item(t('Invite Someone to rCredits'));
  $blurb = blurbLink();
  $a1 = "a href=$base_url/waiting";
  $a2 = "a href=$base_url/invited-whom";
  $where = r\location($mya->cttyA);
  $warning = $mya->co ? t('<p>Note that you are sending an invitation on behalf of a company. Companies are not people and there is a danger in giving companies permission to act like people. On this invitation form, the company is "trusting" whoever you invite -- something usually reserved for humans.</p>') : '';
  //Plus $@R_COUNTED_BONUSr for each employee, if they open a company account. 
  $subtext = item(!@$nearly ? tt("We're sorry, you cannot invite someone to sign up until you have signed the <a href=\"@BASE_URL/agreement\">Agreement</a> and made a <a href=\"@BASE_URL/community/donate\">donation</a>.<br><br>Return to the <a href=\"@BASE_URL/status\">Membership Steps</a> page to see where you stand.") : (tt('<p>Get a $@R_HELPER_BONUSr reward for each friend you invite, who signs up! You may need to help or remind the people you invite. For now, invite only people who live, shop, or work near @where. <@a1>Here are some people waiting to be invited</a>, near you.</p><p>The invitation will include your contact info and a short @blurb (click the link to see it). Adjust the subject and message, then click "Send". <small>[<@a2>friends you have invited</a>]</small></p>', compact('blurb', 'a1', 'a2', 'where')) . $warning));
//  $contact = textField(t('Email or Cell:'), t('An email address or cell number for the person you want to invite'), required());
  $email = formField($cAdmin ? 'textarea' : 'textfield', t('Email:'), $cAdmin ? t('A comma-separated list of email addresses to invite') : t('An email address for the person you want to invite'), required());
  if ($cAdmin) $dupsOk = boolField(t('Dups Okay?'), '', FALSE);
  $trusted = boolField(t('Trusted?'), t('If you lent this person $250, would you trust them to pay it back?'), dft(@$cAdmin ?: NULL));
  $subject = textField(t('Subject:'), t('The subject of your message'), required(($admin ? ADMIN_NAME : $mya->fullName) . t(' invites you to rCredits')));

  $default = @$admin ? 
      tt("The rCredits network is growing. Check it out and sign up!\n\nWilliam")
    : (@$mya->data['inviteBody'] ?: tt("rCredits are amazing. I signed up. Check it out!\n\n@name", u\ray('name', $mya->fullName)));
  $messageHelp = tt('Invite them to sign up, and say why. A short @blurb will follow your message, with a signup link.', compact('blurb'));
  $message = formField('textarea', t('Message:'), $messageHelp, required($default));
  $submit = submit(t('Send'));
  $form = $nearly ? 
      labeled(compact(u\ray('title subtext email dupsOk trusted subject message submit')))
    : compact(u\ray('title subtext'));
  return $form;
}

function formInvite_validate($form, &$sta) {
  extract(u\just('email trusted', $sta['input']));
  if (r\acct()->cAdmin) {
    foreach (u\ray($email) as $one) if (!\valid_email_address(trim($one))) return say("bad email: $one", 'email');
  } elseif (!\valid_email_address($email)) return say('bad email', compact('email'), 'email');
  if (!@$trusted) return say('must trust', 'trusted');
}

function formInvite_submit($form, &$sta) {
  extract(u\just('email subject message dupsOk', $sta['input']));
  $email = strtolower($email); // otherwise u\SUBS breaks on cap after @
  $mya = r\acct();
  if ($mya->cAdmin) {
    foreach (u\ray($email) as $one) sendInvite($one, $subject, $message, $dupsOk, 0);
  } else sendInvite($email, $subject, $message);
  
  $data = u\ray('inviteBody', substr($message, 0, 10000)) + $mya->data;
  $mya->update(compact('data'));
}

function formVideos($form, &$sta) {
  $title = item(t('rCredits Member Videos'));
  $subtitle = formField('item', '', '', t('The videos may not be available until mid-January. Try again in a few days!'));
  $form = compact(u\ray('title subtitle'));
  return labeled($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from r\go() in Summary

/**
 * Show a summary of the current account.
 * This function uses jQuery code defined in misc.js
 * @param string $qid: (optional) qid of the account to summarize and manage (for cAdmin)
 */
function formSummary($form, &$sta, $qid = '') {
  global $base_url;
  if (!$mya = r\acct()) return r\go(isGAME ? 'community/game' : 'signin'); // this cannot be '' (infinite redirect)
  
  if ($mya->can(B_REGULATOR)) { // some kind of admin (regulator, cAdmin2, cAdmin, or admin
    include_once __DIR__ . '/../admin/admin-web.inc';  
    if (!$a = r\acct($qid)) return r\go('/sadmin/member-list', t('Unrecognized Account Code: ') . $qid, 'ERR');
    $form = adminSummary($a, $mya->cAdmin and $a->id != $mya->agentId); // ctty admin not administering own acct

    $extras = '<br>' . u\fmtPhone($a->phone) . " ($a->faxetc)<br>$a->email";
    $idLink = 'href="' . $base_url . '/print-rcard/' . $a->mainQid . '" target="_blank"';
    if (!$a->co and !$a->ctty and $mya->cAdmin) $extras .= " <a $idLink>" . t('Print ID Card') . '</a>';
  } else { // normal
    $a = $mya; // non-cAdmin (or cAdmin managing own account)
//    $a->refreshCan(); // update any permissions that may have been set elsewhere (superfluous?)

    if ($mem = formMembership($a)) { // all steps complete
      if (u\test() and $mem !== TRUE) return $mem; // redirected to another page
      if (!$a->ok) say('setup complete');
//      say('adjust settings');
//      if ($a->can(B_DW) and !$a->can(B_BANK)) say('verify bank');
    } else say('take a step');
    
    if ($a->legalName != $a->fullName) $legalName = item($a->legalName, t('Legal Name:'));    
    //extract(u\just('r usd pAccts', r\stats($a->community)));
    extract(u\just('returnMo return cttyBeneMo cttyBene', $a->j_stats()));
    $returnMo = item("$returnMo% APR " . t('over the past month'), t('Your return:'));
    $return = item("$return% APR " . t('overall (ever)'), ' ', t('Your effective rate of return on your average balance [<@a>details</a>]', u\ray('@a', 'a href=help/your-return')));
    $cttyBeneMo = item("$cttyBeneMo " . t('over the past month'), t('Social return:'));
    $cttyBene = item("$cttyBene " . t('overall (ever)'), ' ', t('How much your participation has benefited the community financially [<@a>details</a>]', u\ray('@a', 'a href=help/social-return')));    
    $floorDesc = t('If you choose "Debt Okay", your balance can go this far below zero')
    . ($a->j_floor < 0 ? t(' for up to 30 days with no charge') : '')
    . '. ' . t('See "Debt Okay" in') . " <a href=\"$base_url/account/preferences\">Preferences</a>.";
    $form = compact(u\ray('legalName returnMo return cttyBeneMo cttyBene'));
  }
  
  if ($a->agent->can(B_MEMBER)) $another = submit(t('Open Another Account'));
  $title = item(t('Account Summary'));
  if (isDEV and !$a->proSe) $agentName = ' &nbsp; &nbsp; ' . t('<b>Agent: </b>') . $a->agent->fullName;
  $accountName = item("$a->j_fullName ($a->j_name)" . @$agentName, t('Name:'), $a->postalAddr . @$extras);
  if (!$a->ctty) { // unless this is a community account
    $acctType = $a->co ? t('company account') : ($a->jid ? t('joint account') : t('personal account'));
    $balance = u\fmtAmt($a->j_r);
    $balance = item($balance . ($a->j_rewards ? t(' (see <@a>History</a>)', ['@a'=>"a href=$base_url/history"]) : ''), t('Balance:'));

    $rewards = formField('item', t('Rewards:'), t('to date'), u\fmtAmt($a->j_rewards));
    $committed = formField('item', t('Committed:'), t('rewards reserved for donation to Common Good Finance'), u\fmtAmt(max(0, $a->j_committed)));
    $creditLimit = formField('item', t('Credit limit:'), @$floorDesc, u\fmtAmt(-$a->j_floor));
  } else $acctType = t('community account');

  if ($mya->cAdmin) {
    $extra = " id#$a->id";
    if ($bankInfo = $a->bankInfo) {
      extract($bankInfo, EXTR_PREFIX_ALL, 'b');
      $bankInfo = "a title=\"$b_name (routing #$b_route)\n$b_address, $b_city, $b_state $b_zip\"";
      $extra .= strtr(t(' <@a>bank info</a>'), ['@a'=>$bankInfo]); // strtr circumvents Drupal crap
    }
  } else $extra = '';
  $accountId = formField('item', t('ID:'), '', ($a->mainQid ?: 'N/A') . ' (' . $acctType . ')' . $extra);

  $fields = 'another photo title accountName accountId balance rewards creditLimit committed';
  if (@$form['photo']) $photo = $form['photo']; // move this up
  return labeled(compact(u\ray($fields)) + $form);
}

function formSummary_validate($form, &$sta) {
//  if (!r\acct()->cAdmin2) return hack('non-admin submitted summary form');
  if (op($sta) == 'another') return;

  extract(u\just('uid who legalName creditLimit rTrader helper mediaConx moves adminable', $sta['input']));
  $a = r\acct(@$uid);
  
  if ($rTrader and !$a->ok) {
    if (empty($helper)) return say(t('You must specify who gets the helper reward.'), 'helper');
    if (is_null(@$adminable[B_MEMBER])) return say(t('That account is not READY (not a member).'), 'adminable'); // test "is_null" because 0 is the value when set
  }

  if (@$helper) {
    if (!$h = whois($helper, 'helper', $sta['input'])) return say(t('I don\'t know what helper account you mean.'), 'helper');
    $helper = $h->id;
  } else {unset($helper); unset($sta['input']['helper']);}
  
  foreach (u\ray('creditLimit') as $k) {
    if (isset($$k)) {
      if ($err = u\badAmount($$k)) return sayFieldErr($err, $k); 
      $$k = u\cleanAmount($$k);
    }
  }
  if ($rTrader and isset($mediaConx) and $err = u\badAmount($mediaConx, '>=0', 0)) return sayFieldErr($err, 'mediaConx');
  if (isset($moves) and $err = u\badAmount($moves, '>=0', 0)) return sayFieldErr($err, 'moves');
  if (@$who and !$who = whois($who, 'who', $sta['input'])) return say(t('I don\'t know what account you mean to go to.'), 'who');
  u\preray(compact(u\ray('helper who creditLimit')), $sta['input']);
}

function formSummary_submit($form, &$sta) {
  include_once __DIR__ . '/../admin/admin.inc';
  
  if (op($sta) == 'another') return r\go('another');
  if (!r\acct()->cAdmin) return; // hack attempt

  extract(u\just('uid who legalName creditLimit notes helper mediaConx moves rTrader risks adminable', $sta['input']));
  $a = r\acct(@$uid);
  $aa = $a->agent;

  $a->stampNotes($notes);
  if (@$who and ($who->community == $aa->community or $aa->id == 1)) {
    svar('myid', $who->id); // make it the new current account
    list ($newAcct, $oldAcct) = array($who->fullName, $a->fullName);
    return say('changed account|lost old changes', compact('newAcct', 'oldAcct'));
  } 
  
  $DBTX = \db_transaction();

  if (!$a->ctty) {
    if ($rTrader and !$a->ok and !$a->co and !a\shrinkPhoto($a, $err)) return say($err, 'ERR');
    if (r\membershipEvent($a, 'ok', $rTrader)) a\secureVery($a); // activate the account
  }

  $a->setBit(B_OK, $rTrader, TRUE);
  $risks0 = $a->risks;
  $a->setRisks(@$risks);
  if ($a->risks != $risks0) {
    include_once __DIR__ . '/../rcredits-risk.inc';
    k\acctRisk1($uid);
  }

  $a->setBits(@$adminable);
  if ($a->helper and ($a->ok or !@$helper)) unset($helper); // don't overwrite if too late or empty
//  $floor = -$creditLimit;
  $a->update(compact(u\ray('legalName floor notes helper mediaConx moves')));
  
  unset($DBTX);
  say('info saved');
}

/**
 * Recalculate risk.
 * @param string $type: type of entity: acct or tx
 * @param int $id: record ID for the entity
 * @param bool $show: <show the risk calculation>
 */
function formRerisk($form, &$sta, $type = '', $id = '', $show = FALSE) {
  include_once __DIR__ . '/../rcredits-risk.inc';
  global $showRiskCalc; 
  
  $showRiskCalc = '';

  if ($type == 'acct') {
    k\acctRisk1($id, $risk, $calc);
    $w = ''; $m = K_RED;
  } elseif ($type == 'tx') {
    $x = r\x($id);
    k\txRisk1($x->ray, $risk, $calc);
    $w = u\fmtAmt($x->amount);
    $m = tt('@K_RED / @K_THRESHOLD');
  }

  $risk = round($risk);
  
  $showRiskCalc = <<< EOF
  <table id="riskCalc">
  <tr><th>Risk Name</th><th>Weight</th><th>Multiplier</th><th>Risk</th><th>Set Flag?</th></tr>
  $showRiskCalc
  <tr><td><b>TOTAL</b></td><td>$w</td><td>$m</td><td>$calc</td><td>= $risk</td></tr>
  </table>
EOF;
  say($showRiskCalc);
  r\go("sadmin/$type-info/$id", t('info saved'));
}

/**
 * Show information about a specific transaction and its risks.
 */
function formTxInfo($form, &$sta, $xid = '') {
  include_once __DIR__ . '/../admin/admin-web.inc';  
  include __DIR__ . '/risk-descs.inc';
  return txInfo($xid);
}

/**
 * Upload a photo.
 */
function formPhoto($form, &$sta) {
  $mya = r\acct();

  $title = item(t('Photo ID Picture'));
  $has = $mya->hasAux('picture');
  $currentPhoto = item($mya->photoHtml(TRUE));

  list ($width, $height) = explode(':', !$mya->co ? R_PERSONAL_ASPECT : R_COMMERCIAL_ASPECT);
  $width *= R_PIXEL_FACTOR;
  $height *= R_PIXEL_FACTOR;
  $desc = $mya->co ? 
    tt('<p>Upload a picture to represent your company in rCredits promotions. The picture must be wider than tall (@R_COMMERCIAL_ASPECT ratio is ideal).</p>')
    : tt('<p>Upload a recent color picture of yourself for your photo-ID rCard. It must be taller than wide (@R_PERSONAL_ASPECT ratio is ideal) and must show your full face clearly (no sunglasses), filling most of the frame.</p>');
  $desc .= tt('<p>File size must be under 3MB. The picture will be cropped and resized to @width x @height pixels.</p><p>Please feel free to <a href="mailto:new@rCredits.org?subject=photo&body=Attached is a photo of me for my rCard. &nbsp; - @fullName">email it to us</a>, if that is easier for you.', u\ray('width height fullName', $width, $height, $mya->fullName));
  $uploadPhoto = formField('file', '', $desc);
  
  if ($has and !$mya->co and $mya->ok) {
    $uploadPhoto = item(R_ON_FILE);
    //$uploadPhoto['#description'] .= '<p>' . tt('triggers new card', u\ray('field', t('photo'))) . '</p>';
  } else $submit = submit(t('Upload and Save'));
  if ($mya->admin and $has) $del = submit(t('Delete'));

  $form = compact(u\ray('title currentPhoto uploadPhoto submit del'));
  return $form;
}

function formPhoto_validate(&$form, &$sta) {
  $validators = array(
    'file_validate_is_image' => [],
    'file_validate_size' => array(R_MAX_UPLOAD_SIZE * 1024000),
  );

  $file = \file_save_upload('uploadPhoto', $validators); // Save the file as a temporary file.
  if ($file === FALSE) {
    say('photo upload failed', 'uploadPhoto');
  } elseif ($file !== NULL) $sta['values']['uploadPhoto'] = $file;
}

function formPhoto_submit($form, &$sta) {
  global $base_url;

  $mya = r\acct();
  $pictureFilename = DRUPAL_ROOT . $mya->auxFilename('picture', TRUE);
  
  if ($mya->admin and op($sta) == 'del' and $mya->hasAux('picture')) {
    rename($pictureFilename, str_replace('.jpg', '.X', $pictureFilename));
    $stepsDone = $mya->stepsDone;
    $stepsDone['photo'] = FALSE;
    $mya->update('photo stepsDone', '', $stepsDone);
    $mya->setBit(B_MEMBER, FALSE);
    $mya->setBit(B_OK, FALSE);
    return say(t('photo removed, step undone, person set to non-member (photo renamed to .X)'));
  }
    
  if (!$picture = @$sta['values']['uploadPhoto']) {
    say('nothing done');
    r\go('status');
  }  

  db\q('DELETE FROM file_managed');
  $tempName = str_replace('temporary://', \file_directory_temp() . '/', $picture->uri);
  $aspect = $mya->co ? R_COMMERCIAL_ASPECT : R_PERSONAL_ASPECT;
  if (!u\fixPicture($tempName, $aspect, R_PIXEL_FACTOR, TRUE, $err)) {
    if ($err == 'aspect') $err = !$mya->co ? 'bad personal aspect' : 'bad commercial aspect';
    return say($err, 'picture');
  }
  if (!rename($tempName, $pictureFilename)) return say('file save error', 'picture'); // (rename fails)

  if (!$co) {
    if ($mya->photo) r\tellStaff('changed photo');
    include_once __DIR__ . '/../admin/admin.inc';
    if (!a\shrinkPhoto($mya, $err)) return say($err, 'picture');
  }
  
  $mya->stepDone('photo');
  $msg = 'photo saved' . ($mya->ok ? '' : '|step completed|return to membership');
  $link = "$base_url/status";
  say($msg, compact('link'));
  if ($mya->ok) {
    $return = !$mya->co ? 'account/security' : 'account/company';
    return r\go($return);
  }
}

function formChangePassword($form, &$sta) {
  $mya = r\acct();
  $form['#user'] = $mya->account('uid pass');
  \user_account_form($form, $sta);
// Each region has its own timezone.  system_user_timezone($form, $sta);

  $form['title'] = formField('item', t('Change Password'), '', weight(-99));
  $form['account']['pass']['#description'] = NULL;
  //$form['account']['pass']['#process'] = array('form_process_password_confirm', 'user_form_process_password_confirm'); //'rCredits\\Web\\password_confirm');
  if (!$mya->admin) {
    $form['account']['current_pass'] = formField('password', t('Current Password:'));
  } else unset($form['account']['current_pass']);
  $form['strong'] = hidField(0, attrib(array('id' => 'edit-strong')));

  foreach (u\ray('mail name timezone status roles') as $one) unset($form['account'][$one]);
  unset($form['picture']);
  $form['submit'] = formField('submit', '', '', array('value' => t('Save')) + weight(99));
  $form['#user_category'] = 'account'; // ?
  u\EXPECT($form['#validate'][0] == 'user_account_form_validate', 'wrong account form validator');
  $form['#validate'] = array('rCredits\\Web\\formChangePassword_validate'); // use ours instead
  return $form;
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formChangePassword_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass current_pass', $sta['values'])); // values NOT input
  extract(u\just('strong', $sta['input'])); // input NOT values
  if (!$mya->admin) {
    if (empty($current_pass)) return say('password required', 'current_pass');
    if (!$strong) return say('weak pass', 'pass');
    if (!passwordOkay($mya, $current_pass)) return FALSE;
  }
}

function formChangePassword_submit($form, &$sta) {
  extract($info = u\just('pass', $sta['values'])); // values NOT input

  if (empty($pass) or !r\acct()->update(compact('pass'))) return r\go('security', t('Your password remains unchanged.'));
//  if (r\acct()->update(u\ray('pass', r\passHash($pass)))) {
  say(t('Your new password has been saved.'));
  return r\go('account/security');
}

function formChangePin($form, &$sta) {
  $mya = r\acct();

  $title = item(t('Change PIN'));
  if (!$mya->admin) {
    $pass = formField('password', t('Current Password:'));
  }
  $pin = formField('password', t('New PIN:'));
  $submit = submit();
  
  return labeled(compact(u\ray('title pin pass submit')));
}

/**
 * Replacement for user_formAccount_validate()
 * (because we want to allow the same email for several accounts)
 */
function formChangePin_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pass pin', $sta['input']));
  if (!$mya->admin) {
    if (empty($pass)) return say('password required', 'pass');
    if (!passwordOkay($mya, $pass)) return FALSE;
  }
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
}

function formChangePin_submit($form, &$sta) {
  extract($info = u\just('pin', $sta['values'])); // values NOT input
  $mya = r\acct();
  
  if (empty($pin) or !$mya->update(compact('pin'))) return r\go('security', t('Your PIN remains unchanged.'));
//  if (r\acct()->update(u\ray('pass', r\passHash($pass)))) {
  say(t('Your new pin has been saved.'));
  return r\go('account/security');
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  global $base_url, $rUrl;
  if (!$mya = r\acct()) return ['signin' => item('<a href="' . "$base_url/signin" . '">Sign in</a>')];

  $myid = @$mya->id; // may be a non-existent account if tests are run while looking at .ZZA, for example
  if (!$accounts = be\accountChoices()) return FALSE;
  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or cadmin
  $choices = $ltrs = [];
  foreach ($accounts as $uid) {
    $a = r\acct($uid);
    if (!$a and isDEV) continue; // ignore this if testing -- "current" account has been undermined is all
    $choice = substr($nm = $a->fullName, 0, 23); // keep it short
    if ($mya->cAdmin) {
      if (u\starts($nm, 'rCredits ')) { // show CGCs in a separate category
        $class = $a->cgc ? 'cgc' : '';
        $ltrs['_'][$uid] = str_replace('rCredits ', '', "<small class=\"class\">$a->name</small> $a->fullName");
        continue; // omit regions from dropdown
      }
      $ltr = strtoupper(substr($nm, 0, 1));
      $ltrs[$ltr][$uid] = "<small>$a->mainQid</small> $choice";
      $choice = ($mya->admin ? $a->mainQid : substr($a->mainQid, -3, 2)) . ' ' . $choice;
    }
    $choices[$uid] = $choice;
  }

  if ($mya->cAdmin) {
    ksort($ltrs);
    $index = $details = '';
    foreach ($ltrs as $ltr => $accts) {
      $index .= "<a id=\"index-$ltr\">$ltr</a>";
      $detail = $detailDivs = '';
      $i = 0;
      foreach ($accts as $uid => $v) {
        $i++;
        $page = urlencode($_SERVER['REQUEST_URI']);
        $detail .= "<a href=\"$base_url/change-account?acct=$uid&page=$page\">$v</a>";
        if ($i == count($accts) or $i % 12 == 0) {
          $detailDivs .= "<div>$detail</div>\n";
          $detail = '';
        }
      }
      $details .= "<div id=\"index-$ltr-detail\" class=\"index-detail\">$detailDivs</div>\n";
    }
    $account = item("<div id=\"index\">$index</div>$details");
  } else {
    
    if ($mya->can(B_MANAGE)) $settings = item(divButton('settings', 'Settings'));

    $page = $_SERVER['REQUEST_URI'];
  //  $onchange = "location.href='$base_url/change-account?acct=' + this.options[this.selectedIndex].value + '&page=$page'; jQuery('#saywait').show();";
  //  $account = formField('select', '', '', dft($myid) + attrib(compact('onchange')), $choices);
    $links = '';
    foreach ($choices as $k => $v) {
      $one = $k == $mya->id ? $v : "<a href=\"change-account?acct=$k&page=$page\">$v</a>";
      $links .= "<li>$one</li>\n";
    }
    $account = item("<ul>$links</ul>");
    /*
    $chunks = explode('/', $page);
    if (isDEV) array_shift($chunks); // site is in a subfolder on the dev server
    $function = @$chunks[1]; // normally just one part of URI is the page (but sometimes two)
    foreach (u\ray('account user') as $one) {
      if (in_array($one, $chunks) and @$chunks[2]) $function .= '/' . $chunks[2];
    }
*/
    //$functionName = ucwords($function);
//    $help = imageButton("help/$function", "Help for $functionName", 'qmark-white.png');
//    $menu = imageButton('menu', 'Show Menu', 'menu.png');
    
  //  $showMenu = formField('item', '', '', button('Menu', '', 'Show the menu'));
  //  $showHelp = formField('item', '', '', button('Help', '', 'Help for ' . ucwords($function)));
    //$showMenu = item($menu);
    //$showHelp = item($help);
  //  $version = item(t('v 0.1d'));
  //  $form = u\prefixKeys('acct_', compact(u\ray('settings account icon showMenu showHelp submit index')));
  }
//  $menu = formMenu($function);

  //$help = help($function);
  //$form += $help; // + $menu;
  $form = u\prefixKeys('acct_', compact(u\ray('account')));
  
  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(u\just('op acct_account', $sta['input']));
  $op = op($sta);
  if ($op == 'Menu') return r\go('menu'); 
}

/**
 * Change the current account.
 */
function changeWho() {
  extract(u\just('acct page', $_GET));

  if (@$acct == 'signout') return signout();
  setGlobals();
  $mya = r\acct(); // get old current account
  if (!@$mya->admin and !r\acct($acct, $mya->agentId)->can()) hack('accts');
  $mya = r\acct($acct); // get new current account

  svar('myid', $mya->id); // this gets checked ONLY in setGlobals()
  if (isDEV) $page = str_replace(DEV_ROOT, '', @$page);
  r\go(@$page);
}

/**
 * Settings form for mobile.
 */
function formSettings2($form, &$sta) {
  throw new Exception('mobile settings'); // UNUSED
  /*
  $menu = []
//    + goButtonIf(B_MANAGE, t('Basic'), 'account/basic', t('Email and password'))
    + goButtonIf(B_MANAGE, t('Contact Info'), 'account/contact', t('Tell us where to find you'))
    + goButtonIf(B_MANAGE, t('Preferences'), 'account/preferences', t('Set your account preferences'))
    + goButtonIf(array(B_PERSONISH, B_MANAGE), t('Security'), 'account/security', t('Adjust your security settings or passwords'))
//    + goButtonIf(B_MANAGE, t('Photo'), 'account/photo', t('Upload a photo for your rCard'))
//    + goButtonIf(B_MANAGE, t('Bank'), 'account/bank', t('Connect to your US Dollar bank account'))
    + goButtonIf(array(B_CO, B_MANAGE), t('Company Info'), 'account/company', t('Describe this company'))
    + goButtonIf(B_MANAGE, t('Relations'), 'account/relations', t('Connect other members to this account'))
    + goButtonIf(B_MANAGE, t('Devices'), 'account/boxes', t('Authorize mobile devices for this account'))
    + goButtonIf(array(B_PERSON, B_MANAGE), t('Proxies'), 'account/proxies', t('Choose two people as your representatives'))
//    + goButtonIf(array(B_PERSON, B_MANAGE), t('ID Proof'), 'account/id-proof', t('Provide proof of your identity'))
  ;
  $title = item(t('Settings'));
  $menu = formField('fieldset', '', '', array('id' => 'settings')) + compact('menu');
  return compact('title', 'menu');
  */
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company) {
  $uid = db\lookup('uid', 'users', 'name=:company', compact('company'));
  $acct = r\acct($uid);
  if (!$uid or !$acct->co) return say('no such company', 'ERR');

  $pic = $acct->photoHtml();
  $state = r\realState($acct->state);
  $country = r\realCountry($acct->country);
  $siteList = '';
  if ($website = $acct->website) foreach (u\ray($website) as $one) $siteList .= "<a href='http://$one' target='_blank'>$one</a><br>";
    
  $phone = 'Phone: ' . u\fmtPhone($acct->phone, '+');
  $sql = <<<EOF
    SELECT i.iid, i.industry FROM r_industries i 
    INNER JOIN r_user_industries ui ON ui.iid=i.iid 
    WHERE ui.uid=:uid ORDER BY i.industry
EOF;
  $cats = db\q($sql, compact('uid'))->fetchAllKeyed();
  $cats = empty($cats) ? '' : ('<h3>Categories:</h3>' . join('<br>', $cats));

  $markup = <<<EOF
  $pic
  <div id='member-details'>
    <div id='member-contact'>
    <h2>$acct->fullName</h2>
    $acct->address<br>
    $acct->city, $state $acct->postalCode $country<br><br>
    $phone<br>
    $siteList<br>
    </div>
    <div id='member-description'>$acct->description</div>
    <div id='member-categories'>$cats</div>
  </div>
EOF;

  $form = array(
    'all' => formField('item', $markup),
  );
  return $form;
}

/**
 * Say the site is down
 */
function formSystemDown($form, &$sta) {return array('zot' => item(tt('system down')));}

/**
 * Get a transaction request (usually pay or charge) and process it
 * @param string $args: all or some parameters for the form (when called from directory or draft form)
 */
function formTx($form, &$sta, $args = '') {
  if ($confirm = sureForm($sta)) return $confirm;
  extract(u\just('who amount goods purpose', $args));
  $mya = r\acct();

  if (!$mya->can(B_SELL)) return r\go('summary');

  $type = $args === '' ? basename(current_path()) : basename(dirname(current_path())); // pay, charge, etc.
  $role = in_array($type, u\ray('charge fine')) ? 'payee' : 'payer';
  $incentive = in_array($type, u\ray('fine grant loan invest'));
  $selling = explode("\n", $mya->selling);
  $purpose = @$purpose ?: ($type == 'charge' ? @$selling[0] : ($type == 'pay' ? '' : $type));
  
  focusOn(@$who ? 'amount' : 'who');

  // set up form fields
  $title = item(ucwords($type));
  $whoLabel = ($type == 'loan' ? 'Lend to' : ucwords($type)) . ' whom:';

  $who = textField($whoLabel, t('Type a name, id, email, or phone'), dft(@$who) + autocomplete($role));

  $amount = textField('Amount: $', t('Amount to ') . $type, dft(@$amount));
  $dollarSign = item('$'); // for mobile
//  u\setDft($goods, R_FOR_GOODS);
  $goods = $incentive ? hidField(FALSE) : formField('radios', t('For:'), '', dft(@$goods), u\ray(R_WHYS)); // required() fails here
  $purpose = textField(' ', t('Description of goods and services or purpose'), required(@$purpose));
  $calc = item(u\calculator('amount'));
  $scanClass = 'not-scanned';
  $$type = submit(ucwords($type));

  $form = labeled(compact(u\ray("title who amount dollarSign goods purpose calc $type")));
  $form['#attributes']['class'][] = $scanClass;

  return keep_values($form); // get defaults from which('info')
}

function formTx_validate($form, &$sta) {
  if (confirming_v($sta)) return;
//  if ($err = amtErr('amount', $sta)) return say($err, 'amount'); // not needed
  if ($confirm = transfer(op($sta), $sta['input'], FALSE)) confirm($confirm, $sta); // op who amount goods what
}

function formTx_submit($form, &$sta) {
  global $fieldErr;
  if (confirming_s($sta)) return;
  w\transfer($op = op($sta), $sta['input'], TRUE); // op who amount goods what
  if (!$fieldErr) return r\go($op); // this is especially important when skipping the confirmation (scanning)
}

/**
 * Upload payroll spreadsheet to create partial payments in rCredits.
 */
function formPayroll($form, &$sta) {
  $title = item(t('Payroll Upload'));
  $subtext = item(t('<p>In QuickBooks, set the amount of rCredits to pay each participating employee as an after-tax payroll deduction of type "rCredits".</p><p>Run the payroll (for the US Dollars part of what people get paid). Then export the payroll report as a CSV file and upload it here (to pay the rCredits part of what people get paid).</p>'));
  $payroll = formField('file', '', '', attrib(u\ray('enctype', "multipart/form-data")));
  $submit = submit('Upload and Pay');

  return compact(u\ray('title subtext payroll submit'));
}

function formPayroll_validate($form, &$sta) {
  if (!@$_FILES['files']['name']['payroll']) return say('upload required', 'payroll');
  $validators = array(
    'file_validate_extensions' => array('csv'),
    'file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000),
  );
  if (!$file = \file_save_upload('payroll', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'payroll');
  if ($file->filemime != 'text/csv') return say('bad file type', 'payroll');
  $sta['input']['filename'] = $filePath = file_directory_temp() . '/' . $file->filename;
}

function formPayroll_submit($form, &$sta) {
  $mya = r\acct();
  $filename = $sta['input']['filename'];
  file_put_contents($filename, str_replace("\r", "\n", file_get_contents($filename)));
  $fp = fopen($filename, "r"); 
  while ($row = fgetcsv($fp)) $rows[] = $row; // get all rows, so we can look ahead
  fclose($fp);
  unlink($filename);
  
  $coName = u\roughName($rows[0][0]);
  if ($coName != u\roughName($mya->legalName) and $coName != u\roughName($mya->fullName)) return say('wrong payroll co', u\ray('coName', $mya->fullName == $mya->legalName ? $mya->fullName : "$mya->legalName or $mya->fullName"), 'payroll');
  if (@$rows[1][0] != 'Payroll Details') return say('not payroll csv', 'payroll');
  $total = 0;

  foreach ($rows as $i => $row) {
    $v0 = $row[0];
    if ($v0 == 'Total') break;
    $line = $i + 1;
    $subs = compact('line');
    
    if (!@$deductionsCol) {
      if ($v0 == 'Employee' and !$deductionsCol = array_search('Deductions', $row)) return say('bad payroll csv', $subs, 'payroll'); else continue;
    }
    if ($v0 and substr($v0, 0, 1) != ' ') { // all first cells except for employee name begin with space
      if (!strpos($period = trim(@$rows[$i + 4][0]), '-')) return say('bad payroll csv', u\ray('line', $line + 4), 'payroll');
      $employee = $v0;
    }
    if ($row[$deductionsCol] == 'rCredits') {
      $amt = $row[$deductionsCol + 1];
      if ($err = u\badAmount($amt, '>0')) return say('bad payroll csv|' . $err, $subs, 'payroll');
      if (!@$employee) return say('bad payroll csv|' . t('Missing employee name.'), $subs, 'payroll');
      if (!$uid = r\worksForUs($employee)) return say('bad payroll employee', $subs + u\ray('employee', $employee), 'payroll');
      $toPay[] = array($uid, $amt, $period);
      $total += $amt;
      $employee = $amt = $period = '';
    }
  }
  if (!@$deductionsCol) return say('bad payroll csv', u\ray('line', t('0 -- missing headers')), 'payroll');
  if (!@$toPay) return say('empty payroll', 'payroll');
  if ($shortfall = $mya->shortfall(R_FOR_GOODS, $total)) return say('short to', u\ray('short', u\fmtAmt($shortfall)), 'payroll');
  
  foreach ($toPay as $one) {
    list ($uid, $amt, $period) = $one;
    list ($msg, $subs) = be\transfer('payment', $mya, r\acct($uid), $amt, R_FOR_GOODS, t('labor: ') . $period);
    say($msg, $subs);
    if (!@$subs['success']) $err = TRUE;
  }
  
  if (!@$err) return r\go('history', tt('payroll done', u\ray('total count', u\fmtAmt($total), count($toPay))));
}

function formMenu($form, &$sta) {return array('title' => item('Menu'));}

/**
 * Present an empty slate for status and error messages.
 * @param bool $doSay: TRUE if called from the doSay function.
 */
function formEmpty($form, &$sta, $doSay = FALSE) {
  $title = item(t('Miscellaneous')); // this is suppressed with CSS
  if ($doSay) $subtext = item(t('To protect your account, no sign-in was required for this action. To sign in safely, always go to rCredits.org.'));
/// FAILS :(  if ($doSay == 'test') $testMessages = item(print_r(\drupal_get_messages(), 1));
  return compact(u\ray('title testMessages subtext'));
}

function formSettings($form, &$sta) {
  $mya = r\acct();
  $tabs = u\ray('contact preferences security bank company relations boxes proxies');
  $captions = u\ray(t('Contact,Preferences,Security,Bank Info,Company,Relations,Devices,Proxies'));
  $title = item('Settings');
  foreach(array_combine($tabs, $captions) as $tab => $caption) {
    $img = imageButton("account/$tab", $caption, "$tab.png");
    $icons[$tab] = item("<div class=\"img\">$img</div><div class=\"caption\">$caption</div>");
  }
//  $icons['photo'] = str_replace('.png', rand(0, 1) ? '-f.png' : '-m.png', $icons['photo']);
  if ($mya->co) {
    unset($icons['security']);
    unset($icons['proxies']);
  }
  if (!$mya->co) unset($icons['company']);
  $icons = item(\drupal_render($icons));
  return compact(u\ray('title icons'));
}

/**
 * Pay with rCredits, arriving from some other site or from rDo().
 * @param string $doCode: the encrypted code from rDo, if any
 */
function formBuy($form, &$sta, $doCode = '') {
  $fields = 'item amount company code verb return_to cust_name ship_zip cust_email';
  extract(u\just($fields, $doCode ? r\doCode($doCode) : (@$sta['input'] ?: $_GET)));
  
  foreach (u\ray($fields) as $field) {
    if (!@$$field) return r\go('', 'missing field', compact('field'), 'zot'); // handle hackers gracefully
  }
  if ($err = u\badAmount($amount, '>0')) return r\go('', $err, 'zot');
  if (!u\isQid(@$company) or !($co = r\acct($company)) or !$co->co) return r\go('', 'bad account id', 'zot');
//  if ($code != @$co->data['cardCode']) return r\go('', 'bad account code', 'zot');
  $cust_email = strtolower($cust_email);
  if (($a = r\acct()) ? !u\similarEmail($a->email, $cust_email) : (!$a = r\acct($cust_email))) return r\go('', u\tt('bad buy info', u\ray('thing', t('email'))), 'zot');
//  if (!in_array(strtolower($cust_name), array(strtolower($a->fullName), strtolower($a->legalName)))) return r\go('', u\tt('bad buy info', u\ray('thing', t('name'))), 'zot');

  if (!\valid_url($return_to, TRUE)) return r\go('', 'bad url', 'zot');

  $verbs = u\ray(t('Donate, Donate, Pay'));
  $verb = in_array(ucwords(@$verb), $verbs) ? ucwords($verb) : t('Pay');
  $disabled = array('disabled' => 1);
  
  $title = item(t('Confirm Payment'));
  $subtext = item(t('To confirm payment, enter your account ID and PIN below.'));
  $amountx = textField("$verb $", '', disabled(number_format($amount, 2)));
  $companyx = textField(t('To:'), '', disabled($co->fullName));
  $forx = textField(t('For:'), '', disabled($item));
  $account = textField(t('Account ID:'), '', required());
  $pin = textField(t('PIN:'), '', required());
  if (substr($ship_zip, 0, 5) != substr($a->postalCode, 0, 3)) $answer = textField(t('Answer:'), $a->question, required());

  $for = $item; // note that disabled fields do not get passed
  $hid = hidField(serialize(compact(u\ray('amount for return_to co a'))));
  $submit = submit($verb);

  return labeled(compact(u\ray('title subtext amountx companyx forx account pin answer hid submit')));
}

function formBuy_validate($form, &$sta) {
  extract(u\just('account pin answer hid', $sta['input']));
  extract(u\just('a', unserialize($hid)));

  if (strtoupper($account) != $a->mainQid) return r\go('', 'not your account', 'zot');
  //if ($pin != $a->pin) return r\go('', 'wrong pin', 'zot');
  //if ($answer != $a->answer) return r\go('', 'wrong answer', 'zot');
}

function formBuy_submit($form, &$sta) {
  extract(u\just('hid', $sta['input']));
  extract(u\just('amount for co a return_to', unserialize($hid)));
  
  list ($index, $subs) = be\transfer('payment', $a, $co, $amount, R_FOR_GOODS, $for);
  if (!@$subs['success']) return r\go('', tt($index, $subs), 'zot');

  return r\go('', "Success! <a href='$my_return_to'>Click here</a> to return to the $company_name website.");
}

/**
 * Pay or deny an invoice, possibly by clicking a link without signing in (from rDo).
 * If no user is signed in, $args must be include the do code (to prevent mischief)
 * @param mixed $args: array of paramaters (from rDo) or invoice number (if signed in):
 *   int nvid: invoice number
 *   bool toMe: is the invoice for me to pay (or deny)? (FALSE if it is FROM me)
 *   string ret: URL to return to
 */
function formHandleInvoice($form, &$sta, $args = '') {
  $mya = r\acct();
  extract(u\just('nvid ret code toMe', $args));
  $ret = @$ret ? @u\deurlify($ret) : 'empty/1';
  
  if ($err = r\badUnpaidInv(@$nvid, $inv)) return r\go($ret, $err, 'ERR');
  extract(u\just('nvid amount payer payee purpose created data', $inv));
  if ($mya) {
    if ($payer != $mya->id and $payee != $mya->id) return hack(t('bad person in invoice payment'));
  } elseif (!$info = r\doCode($code) or $info['id'] != $nvid) return hack(t('fake doCode in handleInvoice'));

  $subs = u\ray('amount what', u\fmtAmt($amount), $purpose);
  
  if (@$toMe) {
    $title = item(t('Confirm Payment'));
    $subs += u\ray('who', r\acct($payee)->fullName);
    $subtext = item(tt('Pay @amount to @who for @what?', $subs));
    $pay = submit(t('Pay'));

    $whyNot = textField(t('Why not:'));
    $deny = submit(t('Deny'));
    $denySet = fieldSet('denySet', compact('whyNot', 'deny'));
    $toMe = hidField(TRUE);
  } else { // confirming delete by originator
    $title = item(t('Confirm Deletion'));
    $subs += u\ray('who', r\acct($payer)->fullName);
    $subtext = item(tt('Delete invoice charging @who @amount for @what?', $subs));
    $delete = submit(t('Delete'));
    $cancel = submit(t('Cancel'));
    $delSet = fieldSet('delSet', compact('delete', 'cancel')); // separate it from subtext
  }
  
  $hid = hidFields(compact('ret') + $inv);
  $form['#validate'] = array('rCredits\\Web\\formHandleInvoice_validate');
  $form['#submit'] = array('rCredits\\Web\\formHandleInvoice_submit');
  return labeled(compact(u\ray('title subtext pay denySet delSet toMe hid')) + $form);
}

function formHandleInvoice_validate($form, &$sta) {
  extract(u\just('whyNot toMe', $sta['input']));
  if (@$toMe and op($sta) == 'deny' and !$whyNot) return say('say why not', 'whyNot');
}

function formHandleInvoice_submit($form, &$sta) {
  extract(u\just('hid whyNot toMe', $sta['input']));
  extract(u\just('ret nvid amount payer payee purpose created data', hidFields($hid)));

  $subs = u\ray('amount payerName payeeName created purpose reason', u\fmtAmt($amount), r\acct($payer)->fullName, r\acct($payee)->fullName, u\fmtDate($created), $purpose, @$whyNot);

  $op = op($sta);
  $ok = TRUE;
  if ($op == 'pay' and !$whyNot) {
    $ok = be\payInvoice($nvid, $msg);
  } elseif ($op == 'delete') {
    db\q('DELETE FROM r_invoices WHERE nvid=:nvid', compact('nvid'));
    r\notify($payer, 'invoice withdrawn', $subs);
    $msg = 'invoice delete done';
  } elseif ($op == 'deny' or @$whyNot) {
    $data = serialize(compact('whyNot') + (@$data ? unserialize($data) : []));
    db\q('UPDATE r_invoices SET status=:TX_DENIED, data=:data WHERE nvid=:nvid', compact('nvid', 'data'));
    r\notify($payee, 'invoice denied', $subs);
    $msg = 'denial done';
  } else $msg = ''; // default: canceled

//  if ($toMe and !r\acct()) return r\go('do/code=' . ($msg ?: 'nothing done', $ok ? 'status' : 'error');
  return r\go($ret, $msg ?: 'nothing done', $ok ? '' : 'ERR');
}

/**
 * Exchange USD for rCredits or vice-versa (the Bank tab)
 */
function formGet($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
  $jid = $mya->jid ?: 0;
  extract(u\just('cancel', $args));
  if ($txid = @$cancel) {
    db\q('DELETE FROM r_usd WHERE txid=:txid', compact('txid'));
    say('transfer canceled', compact('txid'));
    r\tellStaff("canceled transfer #$txid");
  }
  $title = item(t('Bank Transfer'));
  $subtext = item(t('<p>Transfer funds to or from your connected bank account. Transfers may take 3-5 business days.</p><p>To refill your rCredits account automatically (or not), as needed, adjust your "minimum" setting on the <@a>Preferences</a> page.</p>', ['@a' => "a href=$base_url/account/preferences"]) . tt('note|ach warning'));

//  $minimum = textField(t('Minimum:'), tt('prompt min'), dft(u\fmtAmt($isSetUp ? $mya->minimum : R_MINR_DEFAULT, 's')));
  $toSpend = u\fmtAmt($mya->avail(R_FOR_GOODS));
  $waiting0 = $mya->waitingToBank();

  if (!db\exists('r_usd', 'payer IN (:myid,:jid) AND amount>0 AND created>:dayAgo', u\ray('myid jid dayAgo', $myid, $jid, strtotime('-1 day')))) { // limit to one withdrawal request per 24 hours
    $avail = min(R_MAX_DAILY_EXCHANGE, $mya->avail(R_FOR_USD)); // drawing from connected accounts not allowed
//    $avail = db\lookup('SUM(LEAST(usd, :DW_FEE_THRESHOLD))', 'users', 'community=:ctty and uid<>:myid', u\ray('ctty myid', $mya->community, $myid)); // remove this one-day limit if people complain?
//    $avail = min($myAvail, $mya->usd + $avail); // what i have or what USD is available, whichever is less
  } else $avail = 0;

  $toCash = u\fmtAmt(max(0, $avail));
  if ($waiting = u\fmtAmt(abs($waiting0))) {
    $where = 'payer IN (:myid, :jid) AND completed=0 AND deposit=0 ORDER BY txid DESC';
    if ($rec = db\lookup('amount, txid', 'r_usd', $where, compact('myid', 'jid'))) {
      extract($rec);
      $text = t('Cancel') . (($amount == $waiting0) ? '' : (' ' . u\fmtAmt(abs($amount)) . t(' request')));
      $link = " <a class=\"buttino\" href=\"get/cancel=$txid\">$text</a>";
    }
  }
  $way = $waiting0 < 0 ? 'from' : 'to';
  $waiting = item($waiting == '$0' ? t('You have no pending transfer requests.') : (tt('You have total pending transfer requests of @waiting @way your bank account.', compact('waiting', 'way')) . @$link), t('Pending:')); // amount already requested (if any)
  $avail = hidField($avail);
  $balance = item(tt('balances', compact('toSpend', 'toCash')), t('Available:'));
  $amount = textField(t('Amount $'), t('Transfer how much?') . ($mya->admin ? t(' (no limit for admin)') : ''));
  $get = submit('From Bank');
//  $fromBank = formField('checkbox', t('Automate:'), t('(recommended) Refill my rCredits account from my bank account automatically, as needed.'), dft($isSetUp ? $mya->can(B_FROM_BANK) : TRUE));
  
//  $maximum = textField(t('Maximum:'), tt('prompt max'), dft(u\fmtAmt($isSetUp ? $mya->maximum : R_MAXR_DEFAULT, 's')));
//  $excess = textField(t('Excess'), '', dft(u\fmtAmt($mya->overMax(), 's')));
//  $toBank = formField('checkbox', t('Automate:'), t('Transfer any excess to my bank account automatically every few days.'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  $put = submit('To Bank');

  return labeled(compact(u\ray('title subtext waiting balance amount get put avail')));
}
  
function formGet_validate($form, &$sta) {
  $mya = r\acct();
  if ($err = amtErr('amount', $sta)) return say($err, 'amount');
  extract(u\just('amount avail', $sta['input']));
  if ($amount < R_ACHMIN) return say('bank too little', 'amount');
// (this is unnecessary)  if ($amount > R_MAX_DAILY_EXCHANGE) return say('max daily usd', 'amount');
  if (op($sta) == 'get') return;
  
  if ($amount > ($mya->cAdmin ? ($mya->r - $mya->rewards) : $avail)) {
    $max = u\fmtAmt($avail);
    return say('short put|short cash help', compact('max'), 'amount');
  }
  if ($mya->j_r - $amount < $mya->j_minimum) return say('change min first', 'amount');
}
  
function formGet_submit($form, &$sta) {
  $mya = r\acct();
//  $us = new r\usd($mya = r\acct());
  extract(u\just('amount', $sta['input']));

  if (op($sta) == 'put') {
    $action = 'deposit to';
//    if (!$mya->cacheOk()) return r\go('', 'cannot bank', u\ray('action error', $action, t('data integrity issue')));
  } else list ($action, $amount) = array('draw from', -$amount);
  
//  if ($us->bank($amount, $error) or isDEV) { // ignore lack of verified bank connection, if testing
  $checkNum = $mya->bank($amount, $completed);
  if ($amount < 0) {
    $extra = '|bank tx number' . ($completed ? '|available now' : '');
  } else $extra = '';
  $amount = u\fmtAmt(abs(@$amount));
  say('banked' . $extra, $subs = compact(u\ray('action amount checkNum')));
  r\tellStaff('banked' . $extra, $subs);
//  } else return say('cannot bank', compact('action', 'error'), 'amount'); // delete this when we ditch Dwolla
}

/**
 * Create a pseudo-account (with no personal data and no money)
 */ /*
function formAgent($form, &$sta) {
  $mya = r\acct();
  if (!@$mya or !$mya->co) r\go(''); // no error message, in case user changed accounts
  $title = item(t('Open an rPOS Sign-in Account'));
  $subtext = item(t('Use this form only for cashiers who do not wish to participate in the rCredits system as individuals.'));
  $optedOut = boolField(t('Opted out:'), t('Was this agent invited to open a personal rCredits account? And did this agent decide not to?'));
  $fullName = textField(t('Name:'), t('The person\'s full name'), required());
  //$permissions = formField('radios', t('Permissions:'), '', required(), u\ray('read only,charge customers'));
//  $employee = boolField(t('Employee?'), t('Is this person an employee of the company?'));
//  $pay = textField(t('Pay:'), t('How much is this agent paid by the company each month (on average)?'), required());
  $submit = submit(t('Create Account'));
  return labeled(compact(u\ray('title subtext optedOut fullName submit')));
}

function formAgent_validate($form, &$sta) {
  extract(u\just('optedOut fullName', $sta['input']));
  if (!$optedOut) return say('must opt', 'optedOut');
  if (u\badName($fullName)) return say('bad name', 'fullName');
//  if (u\badAmount($pay)) return say('bad amount', 'pay');
}

function formAgent_submit($form, &$sta) {
  extract(u\just('fullName permissions', $sta['input']));
  $a = new r\acct(compact('fullName'));
//  $perms = ($permissions ? B_SCAN : B_READ) - B_RELATED;
  $info = u\ray('main other permission', r\acct()->id, $a->id, B_SCAN - B_RELATED);
  if (db\insert('r_relations', $info)) return r\go('', 'agent created');
}
*/

/**
 * Prepare to open another account.
 * This form creates an invite and redirects to signup, using that code.
 * @see also: formAgent
 */
function formAnother($form, &$sta) {
  $a = r\agent();
  if (!$a->cAdmin and !$a->can(B_MEMBER)) return r\go('', 'members only', 'ERR');
  
  $title = item('Open Another Account');
  $relations = array(t('a company and I own at least 10% of it'), t('a company or organization whose accounts I manage, but I am not a 10%-or-more owner'), t('an additional personal account that I will manage'), t('none of the above') . ($a->cAdmin ? '' : tt(' (send an <a href="@BASE_URL/community/invite">invitation</a> instead!)')));
  $relationPrompt = item(t('What is the new account for?'));
  $relation = formField('radios', t(''), '', required(3), $relations);
  $flows = array(t('neither way (recommended in most cases)'), t('Your personal account can cover the new account\'s debts and overdrafts.'), t
('The new account can cover your personal account\'s debts and overdrafts.'), t('both ways'));
  $flowPrompt = item(t('Which way can the credit flow, when you need it to?'));
  $flow = formField('radios', t(''), '', required(0), $flows);
  $submit = submit(t('Open Account'));
  return labeled(compact(u\ray('title relationPrompt relation flowPrompt flow submit')));
}

function formAnother_validate($form, &$sta) { 
  foreach (u\ray('relation flow') as $field) {
    $$field = $sta['input'][$field]; // values fails here (?!!)
    if (!is_numeric($$field)) say('required field', compact('field'), $field);
  }
  if (!r\agent()->cAdmin and $relation == 3) say('insufficient relation', 'relation');
}

function formAnother_submit($form, &$sta) {
  global $signupArgs; // for testing
  $mya = r\acct();
  $a = r\agent();
  if ($a->id == 1) $a = r\acct(R_ADMIN_QID); // use admin's personal account

  extract(u\just('relation flow', $sta['input']));

  $employeeOk = $relation < 2; // let signupCo info override this
  $personal = ($relation > 1);
  if (!$personal) {
    extract(u\just('company companyPhone isOwner employeeOk', $a->signupCo)); // predict co name
    if (!findCompany(@$company, @$companyPhone)) list ($fullName, $phone) = [@$company, @$companyPhone];
  }
  $isOwner = $relation == 0; // don't let signupCo info influence this

//  $helper = $a->id;
  $helper = $a->mainQid;
  $code = r\invite($a->email, $a->id);
  $flow += 0;

  $signupArgs = compact(u\ray('code personal isOwner employeeOk flow helper fullName phone'));
  $args = http_build_query($signupArgs);
  if ($personal) \session_destroy();
  r\go('signup/' . $args);
}

/**
 * Modify the user registration form.
 * @param string $args: if not empty, a urlencoded assoc (from an email invitation or the "Create Another Account" button):
 *   code: the invitation code 
 *   personal: opening a personal account (default TRUE)
 *   fullName: the likely name of the account to create
 *   phone: the account's likely phone number
 *   isOwner: is the inviter an owner (these last four params are processed by suCreatorRelation())
 *   employeeOk: is the inviter an employee
 *   flow: which way credit can flow (be drawn)
 *   helper: uid of account creating this one -- non null (same as $inviter) means created with formAnother
 */
function formSignup($form, &$sta, $args = '') {
  if (!$args) $args = basename(t\SERVER('REQUEST_URI')); // because menu_execute_active_handler fails
  extract(u\just('code personal fullName phone helper', $args));
  if (@$personal and r\acct()) return r\go('', 'must be signed out', 'err');

  $sta['no_cache'] = TRUE; // otherwise the javascript-populated dropdowns get lost
  u\setDft($personal, TRUE);

  $title = item($personal ? t('Open a Personal rCredits Account') : t('Open a Company Account'));
  $clarify = $personal ? t('To open a company account, use the "Open Another Account" button after your personal account is approved.') : t('To open another personal account, send an Invitation (click the Invite tab).');
  $onclick = $onmouseover = 
  $forother = $personal ? t('company account') : t('personal account');
  $forother = item("[<a class=\"nospin\">$forother</a>]");
  $js = <<<EOF
var clarify = function() {alert('$clarify');};
jQuery('#edit-forother a').click(clarify);
//jQuery('#edit-forother a').mouseover(clarify);
EOF;
  js($js, 'inline', 'footer');
  
  if (@$code != 'w' and !$email = invitation(@$code, '', $inviter, $err)) {
    if (@$inviter) {
      $ia = r\acct($inviter);
      $inviterEmail = htmlspecialchars($ia->email);
      $inviterName = htmlspecialchars($ia->fullName);
      $a = "a target=\"_blank\" href=\"mailto:$inviterEmail?subject=invite me again?&body=Hi $inviterName, the rCredits invitation you sent me expired. :( Please send me another one. :)\"";
    }
    $subtext = item(tt($err, compact('a', 'inviterName')));
    return compact(u\ray('title subtext'));
  } 
  /* elseif (!@$dwok) {
    r\setCook('dupPhone', FALSE);
    return dwollaTerms(); // Dwolla's terms not yet approved
  } */

//  If you want to use this address <b>for more than one account</b>, include a tag on the local part starting with "+" (for example, myemail<b>+whatever</b>@example.com).');

  $subtext = item(t(<<<EOF
Opening an rCredits account is very much like opening an online bank account. It takes about that much time. After this signup page there are <span class="loud">@count more short steps</span> we will guide you through. Opening your account is the hard part &mdash; after that it gets much easier. If you need help, please don't hesitate to ask (<a href="mailto:info@rCredits.org?subject=signup help">click here to send us an email</a>).
EOF
  , ['@count' => $personal ? t('six') : t('seven')]));

  if ($personal) {
    $acctType = hidField(R_PERSONAL);
    $pin = formField('password', t('PIN:'), t('Choose a 4-character security code.')); // first time logging in, get PIN
    $calling = textField(t('Life Calling:'), t('One of the main purposes of rCredits is to provide you the opportunity to do what you feel called to do in life. What would bring you joy to do, to earn rCredits (that you\'re not doing yet)? What might you have to offer?'));
    $companyOptions = array(
      'isOwner' => t('owner'),
      'employeeOk' => t('employee'),
      'contractor' => t('other worker (contractor, supplier, etc.)'),
    );
    $copts = boxOpt('copts', $companyOptions, $sta);

    $companyInfo = array(
      'company_text' => item(t('Are you an owner and/or employee of a local company, or of any rCredits member company? Otherwise leave this part blank.')),
      'company' => textField(t('Company: '), t(''), autocomplete('company')),
      'companyPhon' => textField(t('<small>Company</small> Ph<span>&nbsp;</span>one:'), t('What is the company\'s phone number?')), // Google Chrome bug require obfuscation of "phone"
      'companyOptions' => boxesField('copts', '', '', @$copts, $companyOptions), 
    );
    $companyInfo = fieldSet('companyInfo', $companyInfo);
  } else { // company
    $acctType = acctTypeField();
  }

  $buysAts = db\q('SELECT id, company FROM r_nonmembers')->fetchAllKeyed();
  $ats = boxOpt('ats', $buysAts, $sta);
  $buysAt = boxesField('ats', t('<br>Over 60 Greenfield businesses have already signed up. Which of these <b>Not-Yet-Member Businesses</b> do you frequent?'), '', @$ats, $buysAts);

  $submit = submit(t('Sign up'));
  $args = hidField(@$args);
  $helper = hidField(@$helper ? r\acct($helper)->id : @$inviter);
  
  $form = compact(u\ray('title forother subtext acctType'))
        + nameAndEmail($personal ? t('Full name') : t('Company'), @$fullName)
        + contactFields(compact('phone'), FALSE) + ssnFields($personal)
        + compact(u\ray('pin calling companyInfo buysAt submit args helper'));
//  keep_values($form);
  return labeled($form);
}

function formSignup_validate($form, &$sta) {
  if (!isset($sta['buttons'])) $sta['buttons'] = []; // for testing (keep Drupal from griping)
  extract(u\just('agreed args acctType email phone country tenure pin company companyPhon copts fullName legalName postalCode address city state postalAddr', $sta['input']));

  if (op($sta) == 'continue') { // don't require Dwolla checkbox, so we can give a good message
    return @$agreed ? '' : say('dwolla terms required', 'agreed');
  }
 
  if (@$args) extract(u\just('code', $args));
  if (!invitation(@$code) and @$code != 'w') return r\go('signup'); // in case user resubmits
  //and @$code != bin2hex(R_WORD)

  if (!emailOkay($email)) return;
  if (!phoneOk($phone, $country, 'signup')) return;
  if ($country == US_COUNTRY_ID) {
    if (!preg_match('/^\d{5}(\-\d{4})?$/', $postalCode)) return say('bad zip', 'postalCode');
  }
  if (!$state) return say('bad state', 'state');
  if (!formBasic_validate($form, $sta, $acctType == R_PERSONAL)) return;
  if ($err = u\badAmount($tenure, '>=0')) return say(t('How-long: ') . tt($err), 'tenure');
  if (@$pin === '') return say('pin required', 'pin');
  if (@$pin and strlen($pin) != 4) return say('wrong pin len', 'pin');
  
  if (@$copts) extract(u\just('isOwner employeeOk contractor', $copts));
  $gotrelation = (@$employeeOk or @$isOwner or @$contractor);
  if (@$company or @$companyPhon) { // NOT or $gotrelation (let user get by without)
    foreach (u\ray('company companyPhon') as $one) {
      if (u\nonish(@$$one)) $$one = '';
      // (no) return say('missing field', array('field' => $one), $one);
    }
    if (@$company and $err = u\badName($company)) return say($err, array('field' => 'Company'), 'company');
    if (@$companyPhon and !$companyPhone = u\fmtPhone($companyPhon, '+n')) return say('bad company phone', compact('companyPhone'), 'companyPhone');
    if (!$gotrelation) return say('what relation', 'company');
    $signupCo = compact(u\ray('company companyPhone employeeOk isOwner contractor'));
  }
  
  if (!@$legalName) $legalName = $fullName;
  foreach (u\ray('fullName legalName address city postalAddr') as $field) {
    if ($err = u\badName(@$$field)) return say($err, compact('field'), $field); // @ for testing
    $$field = u\normalizeCase($$field);
  }
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) return say('po in location', 'address');

  $community = r\communityUid($postalCode);
  $flags = $acctType == R_PERSONAL ? 0 : u\bit(B_CO);
  $administer_users = FALSE; // for testing (keep Drupal from griping)
  $status = 1;
  u\preray(compact(u\ray('fullName legalName address city postalAddr email community flags signupCo administer_users status')), $sta['input']);
}

function formSignup_submit($form, &$sta) {
  $dbFields = 'fullName legalName email community flags phone federalId dob pin country postalCode address city state postalAddr oneTimePass helper wants calling signupCo tenure'; // had verifyBy
  extract(u\just('acctType args company ats owns ' . $dbFields, $sta['input']));
//  if (op($sta) == 'continue') return r\go('signup/' . urldecode(u\urlArgs()) . '&dwok=1'); // got agreement, so continue
  if (!emailOkay($email)) return; // I don't know how people create duplicate accounts, but maybe this will prevent it. Nope, apparently not. (ws 1/27/2014)

  if (u\test() and !@$args) $args = basename(t\SERVER('REQUEST_URI'));

  u\EXPECT($args, 'no args');
  extract(u\just('code', $args)); // look for helper on URL in case of Create Another Account
  if (isDEV) $helper = $helper + 0; // don't let lack of helper stop us

  $DBTX = \db_transaction();
  $wants = wants(@$ats);
//  $verifyBy = verifyBy(@$verifyBy);
/*  if (u\test()) {
    $where = u\parseZip($postalCode) ?: u\ray('state', 'MA'); // get $state/$city (dft to MA)
    $postalAddr = "$address, $city, $state $postalCode";
  } else */
  $where = [];

  // @todo: try this 2 or 3 times (in case someone else was trying to register the same name
  $name = r\uniqueName($fullName);
  if ($acctType == R_PERSONAL) {
    $oneTimePass = oneTimePass($name);
    $pw1 = $oneTimePass['pass'];
  }
  
  $info = compact(u\ray('name ' . $dbFields)) + $where + acctType($acctType);
  if (!$mya = new r\acct($info)) return say('cannot create account', 'zot');
  
  $myid = $mya->id;
  u\EXPECT($myid > 0, 'registration failure');
  setGlobals($myid); // some rcredits functions need to know who we are
  
  w\invitation($code, $myid); // mark the invitation used
  $email = u\cryptN($email);
  db\q('UPDATE r_invites SET invitee=:myid WHERE email=:email', compact('myid', 'email')); // and all such
  if (!$owns) $mya->setRisk('rents');
  
  if (@$signupCo) {
    suCompanyRelation($myid, $signupCo);
    $mya->update(compact('signupCo'));
  }
  if (@$helper and @$args and !r\acct($helper)->cAdmin) suCreatorRelation($helper, $args, $myid);
  
  // report to staff and user
  $quid = $mya->mainQid;
  tellStaffNewMember($info + compact('acctType', 'quid'));

  if ($acctType == R_PERSONAL) {
    say('your account is ready');
    $region = strtolower(R_SERVER_ID);
    r\notify($myid, 'welcome', u\ray('name quid code', $name, $quid, $pw1), TRUE); 
    $goto = 'empty';
  } else { // non-personal company (no independent login)
    say('company is ready', compact('quid'));
    //if (@$helper) $quid = r\quid($helper); // "if" for dev faked code HUH?
    if (!@$helper) hack('company no helper');
    svar('myid', $mya->id); // this is necessary for company creation, to keep the company selected
    $goto = 'status';
  }
  $mya->stepDone('contact');
  
  unset($DBTX); // commit
  return r\go($goto);
}

/**
 * Display, for editing, contact info for the current account.
 * This function uses jQuery code defined in misc.js, for the country and state dropdowns
 * @NOTE: The country and state drop down require a change in form.inc (see patch/form.inc.patch)
 */
function formContact($form, &$sta) {
  $mya = r\acct();
  focusOn('fullname');
  $nameDesc = tt('usename desc');
  if ($mya->ok) $nameDesc .= '<br>' . ($mya->co ? t('Your username and rCredits webpage address') : tt('triggers new card', u\ray('field', t('name'))) . t('Your username')) . t(' will also change.');
  $title = item(t('Contact Information'));
  $fullName = textField($mya->co ? t('Company Name:') : t('Your Name:'), $nameDesc, required(@$mya->fullName));
  $onchange = "jQuery('.form-item-pin').show();";
  $email = textField(t('Email:'), '', attrib(compact('onchange')) + required(@$mya->email));
  if (!$mya->admin and $mya->pin !== '') $pin = formField('password', t('PIN:'), t('Required for changing your email address'));

  $submit = submit();

  $form = compact(u\ray('title fullName email pin')) 
        + contactFields(NULL, TRUE)
        + compact('submit');
  
  return labeled($form);
}

function formContact_validate($form, &$sta) {
  $mya = r\acct();
  $myid = $mya->id;
  extract(u\normalizeCase(u\just($normals = 'fullName city address postalAddr', $sta['input'])));
  extract(u\just('email pin phone country state', $sta['input']));

  // what's this doing in validate?!
//  js('js/countries.js', 'file', 'header'); // country and state dropdowns
//  js("print_country(\"$country\", \"$state\");", 'inline', 'footer'); // initialize dropdowns

  if ($fullName != $mya->fullName) {
    if ($err = u\badName($fullName)) return say($err, array('field' => 'fullName'), 'fullName');
    $name = u\shortName($fullName);
    $otherName = db\lookup('fullName', 'users', 'name=:name AND uid<>:myid', compact('name', 'myid'));
    if ($otherName) return say('shortname taken', compact('otherName'), 'fullName');
  }

  if (!emailOkay($email, $mya->id)) return; // say('bad email', u\ray('email', $email), 'email');
  if (!$mya->admin and $email != $mya->email and $pin != $mya->agent->pin) return say('bad pin', 'pin');
  if (!phoneOk($phone, $country, TRUE)) return;
  if (stripos(@$address, ' box ') or stripos(@$address, 'pob ') !== FALSE) return say('po in location', 'address');
  
  u\preray(compact(u\ray($normals . ' email phone name')), $sta['input']); // fix formatting and add name
}

function formContact_submit($form, &$sta) {
  extract($info = u\just('fullName name email phone faxetc country postalCode state city address postalAddr owns', $sta['input'])); // had verifyBy
  unset($info['owns']);
  
  $mya = r\acct();
  if ($mya->id == 1) unset($info['name']); // don't change 'admin'
  
//  if (isset($verifyBy)) $info['verifyBy'] =  verifyBy(@$verifyBy); // @ for testing
  $info = u\normalizeCase(u\just('fullName city address postalAddr', $info)) + $info;

  $mya->setRisk('rents', !@$owns); // @ for tests
  if ($mya->update($info)) {
// (problematic if addPhone fails)  if (!$mya->ok) say('step completed');
    $mya->stepDone('contact', $info);
    return r\go($mya->ok ? 'settings' : 'status', 'info saved');
  } else say('not saved');
}

/**
 * Display a list of people nearby, waiting for an invitation.
 */
function formWaiting($form, &$sta) {
  $mya = r\acct();
  
  $title = item(t('People You Might Know, Waiting to Be Invited'));
//  $subtext = item(t('<p>If the status of someone you invited is "<span style="color:orange;">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span style="color:red;">expired</span>", you may want to call them and send another invitation.</p>'));
  $table = "(SELECT CONCAT_WS(' ', first, last) AS name, MID(zip,1,5) AS zip FROM r_request WHERE NOT done AND ctty=:ctty ORDER BY created DESC,listid DESC LIMIT 50) r";
  $q = db\q("SELECT name, zip FROM $table ORDER BY zip,name", ['ctty' => $mya->community]);
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $lines[$zip] = (@$lines[$zip] ? $lines[$zip] . ', ' : '') . $name;
  }
  if ($mya->community == r\serverUid()) $lines['01301'] = (@$lines['01301'] ? $lines['01301'] . ', ' : '') . 'Caitlin Sullivan';
  if (@$lines) {
    foreach ($lines as $zip => $zipList) {
      if ($city = db\lookup('city', 'users', 'MID(postalCode, 1, 5)=:zip', compact('zip'))) $zip = $city;
      $list[] = "<tr><th>$zip</th><td>$zipList</td></tr>\n";
    }
  } else $list = [t('No one in your area is currently waiting to be invited.')];
  $list = item('<table>' . join('', $list) . '</table>');
  return compact('title', 'list');
}

/**
 * Display a list of the member's invitees.
 */
function formInvitedWhom($form, &$sta) {
  $mya = r\acct();
  $uid = $mya->cAdmin ? $mya->id : $mya->agentId;
  
  $title = item(t('Your Invitees'));
  $subtext = item(t('<p>If the status of someone you invited is "<span style="color:orange;">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span style="color:red;">expired</span>", you may want to call them and send another invitation.</p>'));
  $old = strtotime((R_INVITE_DAYS + 2) . ' days ago');
  $sql = <<<EOF
    SELECT DISTINCT IFNULL(u1.fullName, i.email) AS who, invited, invited<:old AS expired,
      (u1.flags&(1<<:B_MEMBER)) AS member
    FROM r_invites i LEFT JOIN users u1 ON u1.uid=i.invitee
    WHERE i.inviter=:uid ORDER BY invited DESC
EOF;
  $q = db\q($sql, compact('uid', 'old')); // IFNULL fails for u1.flags (not sure why -- it comes out 0)

  $list = "<table id='invitelist'>\n<tr><th>Invited</th><th>Whom</th><th>Status</th></tr>\n";
  while ($row = $q->fetchAssoc()) {
    extract($row);
    $invited = u\fmtDate($invited);
    if (!$started = strpos($who, ' ')) $who = u\cryptN($who0 = $who, FALSE) ?: $who0; // decrypt email
    $status = ($member or $ok) ? t('is a Member!') : ($started ? t('account Not Complete') : ($expired ? t('expired') : '(no response yet)'));
    $color = $member ? 'green' : ($started ? 'orange' : ($expired ? 'red' : 'gray'));
    $nameClass = $co ? 'name co' : 'name';
    $list .= "<tr><td>$invited</td><td>$who</td><td style='color:$color;'>$status</td></tr>\n";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  return compact(u\ray('title subtext list'));
}

/**
 * Ask for the 5-digit phone verification code that Dwolla sent.
 *//*
function formVerifyPhone($form, &$sta) {
  $mya = r\acct();
  $title = item(t('Verify Phone'));
  $subtext = item(t('<p>A verification code has been sent to your phone (@phone). Please enter it here.</p><p>Or, if you need to use a different telephone, <@a>click here to return to the Contact Information page</a>.</p>', u\ray('@phone @a', u\fmtPhone($mya->phone), 'a href=contact')));
  $code = textfield(t('Code:'), t('<@a>Send it again?</a>', u\ray('@a', 'a href=readd-phone')));
  $submit = submit(t('Continue'));
  return labeled(compact(u\ray('title subtext code submit')));
}

function formVerifyPhone_validate($form, &$sta) {
  if (strlen($code = u\digits($sta['input']['code'])) != 5) return say('bad phone code', 'code');
  $us = new r\usd($mya = r\acct());
  if (!$us->verifyPhone(svar('phoneId'), $code, $err) and $us->step() == 'Phone') return say($err, 'code');
}

function formVerifyPhone_submit($form, &$sta) {
  return r\go('status', 'phone verified');
}
*/
/**
 * Offer to revise data that affects social security number verification.
 *//*
function formBasic($form, &$sta) {
  $mya = r\acct();
  $nameDesc = t(', properly capitalized, as on your most recent government-issued ID');
  $title = item(t('Retry Verification'));
  if ($personal = !$mya->co) {
    list ($first, $last) = u\parseName($mya->legalName);
    $first = textField(t('First name:'), t('Your first name') . $nameDesc, required(@$first));
    $last = textField(t('Last name:'), t('Your last name') . $nameDesc, required(@$last));
  } else {
    $org = textField(t('Legal name:'), t('The company\'s') . tt('legalname desc'), required($mya->legalName));
    $acctType = acctTypeField();
  }
  $submit = submit();

  return labeled(compact(u\ray('title first last org acctType')) + ssnFields($personal) + compact('submit'));
}
*/
define('R_BASIC_FIELDS', 'first last org federalId dob');

/**
 * Validate the data that affects social security number verification.
 * called from signup validation, as well as from formBasic()
 * @param string/bool $personal: is this a personal account ('mya' means use the current account's type)
 *   'mya' also means we are retrying basic information (the AccountInfo step) during signup
 */
function formBasic_validate($form, &$sta, $personal = 'mya') {
  extract(u\just(R_BASIC_FIELDS, $sta['input']));
  if ((string) $personal == 'mya') {
//    $us = new r\usd($mya = r\acct());
    $personal = !$mya->co;
//    $dw = $mya->can(B_DW);
  }
  $what = $personal ? 'social security number' : 'federal ID';
  if ($personal) {
    list ($min, $max) = array(r\rTime() - 150 * 365.25 * DAY_SECS, r\rTime() + 9 * 31 * DAY_SECS);
    if ($err = u\badDate($dob, $min, $max)) return say($err, 'org'); // badDate makes dob internal
  }
  if ($err = u\badSsn($federalId, @$dob)) return say($err, compact('what'), 'org');

  if (@$mya) { // retrying account info
    if ($personal) { // personal
      if ($err = u\badName($first)) return say($err, 'first');
      if ($err = u\badName($last)) return say($err, 'last');
//      $emptyErr = t('There was a problem submitting your data. Try again?'); // (this happened early on)
//      if ($dw and !$us->ssn($federalId, $err)) return say(@$err ?: $emptyErr, 'org');
/*      if ($dw and $us->step() == 'AccountInfo' and !$us->accountInfo($dob, '', $first, $last, '', '', $err)) {
        return say(@$err ?: $emptyErr, 'first');
      } */
    } else { // org
      if ($err = u\badName($org)) return say($err, 'org');
/*      if ($dw and !$us->accountInfo('', $federalId, '', '', $org, $mya->businessStructure(), $err)) {
        if ($err) return say($err ?: $emptyErr, 'first');
      }
      */
    }
  }
  u\preray(compact(u\ray(R_BASIC_FIELDS)), $sta['input']);
  return TRUE;
}
/*
function formBasic_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just(R_BASIC_FIELDS, $sta['input']));
  $legalName = !$mya->co ? trim("$first $last") : $org;
  if ($mya->update(compact(u\ray('legalName federalId dob')))) say('info saved');
  return r\go('status'); // on to next step (or total failure)
}
*/
function getqs() {
  return json_decode ('[
  {
    "Id": "36848018",
    "Text": "According to your DRIVER\'S LICENSE, what COLOR is your HAIR?",
    "Choices": {
      "181649905": "AUBURN",
      "181649906": "BLOND",
      "181649907": "GREY",
      "181649908": "BROWN",
      "181649909": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848019",
    "Text": "Which of the following colleges have you PREVIOUSLY OR CURRENTLY attended?",
    "Choices": {
      "181649910": "TOCCOA FALLS COLLEGE",
      "181649911": "ARKANSAS TECH UNIVERSITY",
      "181649912": "CITY COLLEGES OF CHICAGO",
      "181649913": "HUSSIAN SCHOOL OF ART",
      "181649914": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848020",
    "Text": "According to your DRIVER\'S LICENSE, what COLOR are your EYES?",
    "Choices": {
      "181649915": "BROWN",
      "181649916": "GREEN",
      "181649917": "BLACK",
      "181649918": "BLUE",
      "181649919": "NONE OF THE ABOVE"
    }
  },
  {
    "Id": "36848021",
    "Text": "In which of the following CITIES have you NEVER lived or used in your address?",
    "Choices": {
      "181649920": "MOUNT VERNON",
      "181649921": "WOODSIDE",
      "181649922": "FLUSHING",
      "181649923": "EAST ELMHURST",
      "181649924": "ALL OF THE ABOVE"
    }
  }
  ]');
}

/**
 * Ask a set of personal questions to confirm the person's identity.
 * If the answers fail 3 times (or even if the user refreshes the screen 3 times), the PhotoId step is next.
 *//*
function formKba($form, &$sta) {
  $us = new r\usd($mya = r\acct());
  $form['title'] = item(t('Confirm Your Identity'));
//  if ($questions = getqs() and $transactionId = 123) { // debug
  if ($transactionId = $us->kba($questions, $err)) { // get the set of questions
    foreach ($questions as $qi => $question) {
      extract(u\just('Id Text Choices', $question));
      // Mysteriously, $Choices[$key] (or previously $Choices->$key) cannot be referenced
/// For example, if $key is '181649905', "echo $Choices[$key];" gives "Undefined offset: 181649905..."
      foreach ($Choices as &$value) $value = strlen($value) == 2 ? $value : u\titleCase($value);
      unset($value);
      $form["q-$Id"] = formField('radios', $Text, '', required(), (array) $Choices);
    }
    $form['transactionId'] = hidField($transactionId);
    $form['submit'] = submit(t('Submit answers'));
  } elseif ($us->step() != 'Kba') {
    return r\go('status'); // let it fall through, for appropriate message on status page
  } else say(t('Confirmation questions are not available. Please try again later.') . " ($err)");

  return $form;
}

function formKba_submit($form, &$sta) {
  $us = new r\usd($mya = r\acct());
  foreach ($sta['input'] as $key => $value) {
    if (substr($key, 0, 2) == 'q-' and is_numeric($qid = substr($key, 2))) $answers[] = u\ray('answerid, questionid', $value, $qid);
  }
///debug($answers); return;
  
  if (!$us->verifyKba($sta['input']['transactionId'], $answers, $err)) say($err, 'ERR');
  return r\go('status');
}
*/

/**
 * Connect a bank account.
 */
function formBank($form, &$sta, $args = '') {
  global $rUrl;
  $mya = r\acct();
  $usa = ($mya->country == US_COUNTRY_ID);

  $title = item(t('Bank Information'));
  
  if ($hasBank = $mya->risk('hasBank')) {
    $bankAccount = $mya->bankAccount;
    if ($usa) list ($route, $bankAccount) = u\parseBankAccount(@$bankAccount);
    $bankAccount = str_repeat('x', strlen($bankAccount) - 4) . substr($bankAccount, -4, 4);
    $bankAccount = item($bankAccount, t('Account:'));
    $remove = submit(t('Remove connection'));
    return labeled(compact(u\ray('title bankAccount remove')));
  } else { // no account yet or EFT not yet authorized
    $doConnect = (int) !@$mya->stepsDone['connect'];
    $subtext = item(t('<p>Complete this form to authorize funds to be taken from your checking account in exchange for any rCredits you request, in amounts not to exceed $1,000 per day. Funds may be transferred either by Remotely-Created Check (RCC) or by Automated Clearing House (ACH) direct debit.</p><p>Do you want to connect your checking account at this time?</p>'));
    $connect = boolField(t('Connect:'), t('Connecting lets you use your rCard like a debit card. Otherwise you can trade US Dollars for rCredits at a participating business.'), $doConnect);

    if ($usa) {
      $checkHint = item("<img src=\"$rUrl/images/check-hint.png\">");
      $routingNumber = textField(t('Routing:'), t('Type carefully your bank\'s routing number.'));
    }
    //$bankAccount = textField(t('Account:'), t('Type carefully your checking account number.'));
    $bankAccount = formField('password', t('Account:'), t('Type carefully your <b>checking</b> account number. <span>Savings accounts will not work, at this time.</span>'));
    
    $bankAccount2 = formField('password', t('Again:'), t('Type your account number again (we need to be REALLY SURE we don\'t connect to the wrong account).'));
    //$bankAccountType = formField('radios', t('Type:'), '', required($mya->can(B_SAVINGS)), $typeOptions);
    $connectFields = fieldSet('connectFields2', compact(u\ray('checkHint routingNumber bankAccount bankAccount2')));
    $submit = submit($doConnect ? t('Connect') : '');    
    
    $js = <<<EOF
function showBankFields(show) {var f=jQuery("fieldset#connectFields2"); if (show) f.show(); else f.hide();}
showBankFields(jQuery("#edit-connect-1").attr("checked"));
jQuery("#edit-connect").click(function() {
  var yes=jQuery("#edit-connect-1").parent().find("label").hasClass("opt-on");
  showBankFields(yes); jQuery("#edit-submit").val(yes ? "Connect" : "Save");
});
EOF;
    js($js, 'inline', 'footer');
    return labeled(compact(u\ray('title subtext connect connectFields submit')));
  }

}

function formBank_validate($form, &$sta) {
  if (op($sta) == 'remove') return;
  extract(u\just('connect routingNumber bankAccount bankAccount2 bankAccountType', $sta['input']));
  if (is_null($connect)) return say('yes or no', 'connect');
  if (!$connect) return; // nothing more to do

  foreach (u\ray($accts = 'routingNumber bankAccount bankAccount2') as $one) $$one = preg_replace('/[\.\- ]/', '', @$$one);
  u\preray(compact(u\ray($accts)), $sta['input']);
  if (!preg_match('/^(|[0-9]{9})$/', $routingNumber)) say('bad routing number', 'routingNumber');
  if (!db\exists('r_banks', 'route=:routingNumber', compact('routingNumber'))) return say('bad routing number', 'routingNumber');
  if (!preg_match('/^[0-9]{3,17}$/', $bankAccount)) say('bad account number', 'bankAccount'); // {3,20} ? Dwolla says 3-17
  if ($bankAccount != $bankAccount2) say('account number mismatch', 'bankAccount2');
}

function formBank_submit($form, &$sta) {
  $mya = r\acct();
  extract(u\just('connect routingNumber bankAccount', $sta['input']));
  $op = op($sta);
  if ($op == 'submit') {
    if ($connect) {
      if ($mya->country == US_COUNTRY_ID) $bankAccount = "USkk$routingNumber$bankAccount";
      $mya->update(compact('bankAccount'));
    }
    $mya->setRisk('hasBank', $connect);
    $mya->stepDone('connect');
    say('info saved|return to membership');
  } elseif ($op == 'remove') $mya->setRisk('hasBank', $connect = FALSE);
  
  if ($mya->minimum and !$connect and !r\relation('draw', 'other=:id', ['id' => $mya->id])) {
    $mya->update('minimum', 0);
    say('min zeroed');
  }
}

/**
 * Get the member's photo ID, as part of Dwolla's registration process.
 *//*
function formPhotoId($form, &$sta) {
  $mya = r\acct();
  $person = !$mya->co;

  say('after emailing|return to membership');
  $mya->setBit(B_PHOTOID, TRUE); // allow the member to continue completing steps
  
  $proofType = $person ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t('EIN documentation from the IRS or proof of registration with state or local government');

  //  -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  
  $title = item(t('Photo ID Verification'));
  $subtext = item(tt('<p>We are having trouble confirming your information. We want to be absolutely sure that YOU are creating this account, not someone else using your name.</p><p>Please upload either a PDF or an image file of your @proofType.', compact('proofType')));
  $name = urlencode($mya->fullName);
  $subtext = item(tt('<p>We are having trouble confirming your information. We want to be absolutely sure that YOU are creating this account, not someone else using your name.</p><p>Please <a href="mailto:new@rCredits.org?subject=Photo ID from @name&body=Dear rCredits Team, My photo ID file is attached. - @name">email us (click here)</a> a PDF or image file of your @proofType.', compact('proofType', 'name')));
//  $idProof = formField('file', t('Photo ID:'), t('maximum') . tt(' @R_MAX_PHOTOID_SIZE MB'), attrib(u\ray('enctype', "multipart/form-data")));
//  $submit = submit('Upload');

  $form = compact(u\ray('title subtext idProof submit'));
  return $form;
  return labeled($form);
}

function formPhotoId_validate($form, &$sta) {
  if (@$_FILES['files']['name']['idProof']) {
    $validators = array('file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000));
    if (!$file = \file_save_upload('idProof', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime) and @$file->filemime != 'application/pdf') return say('bad file type', 'idProof');
    $sta['input']['filename'] = $filePath = file_directory_temp() . '/' . $file->filename;
    $mya = r\acct();
    if (FALSE and $mya->can(B_DW)) { // Dwolla API bug (skip it)
      $us = new r\usd($mya);
      if (!$us->sendPhotoId($filePath, $err)) return say($err, 'idProof');
    } else {
      $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof', TRUE);
      if (!$key = $sta['input']['idProof'] = bin2hex(u\encryptFile($filePath, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    }
  } return say('upload required', 'idProof');
}

function formPhotoId_submit($form, &$sta) {
  unlink($sta['input']['filename']);
  say('id verification pending');
  return r\go('status');
}
*/

/**
 * Get the member's security info.
 * @param string $args: UNUSED
 */
function formSecurity($form, &$sta, $args = '') {
  global $base_url;
  $mya = r\acct();

  extract(u\just('show', $args));
  $person = !$mya->co;
    
/*
Dwolla says: 
We're having trouble confirming your information. We want to be absolutely sure that you are creating this account, not someone else creating an account under your name. Protecting your identity is our #1 concern.

Personal Accounts or Sole Proprietors:
Please upload a government-issued photo ID such as a U.S. driver's license or passport. 

Businesses or Non-Profits:
Please upload a copy of your EIN documentation or Non-Profit Status documentation. 

2.5MB max
*/
//  $has = $mya->hasAux('proof');
//  $proofType = $person ? t("driver's license or other official ID such as birth certificate or passport, showing your social security number (or the equivalent)") : t("EIN documentation from the IRS (or the equivalent) -- This could also be a letter from an officer or owner of the organization (who is an rCredits member), on the organization's letterhead, authorizing opening this rCredits account.");
  
//    'subtext = item(t('Just like for a bank account, we need to verify your identity. The photo ID file you upload can be either an image or a PDF. Please be patient while the file is encrypted &mdash; it may take several seconds.'));

  $title = item(t('Security Information'));
  $pw = item(t('<@a>Change your password</a>', u\ray('@a', "a href=$base_url/account/change-password")), ' ');
  $pin = item(t('<@a>Change your PIN</a>', u\ray('@a', "a href=$base_url/account/change-pin")), ' ');
  $photo = item(t('<@a>Change your photo</a>', u\ray('@a', "a href=$base_url/account/photo")), ' ', $mya->member ? tt('triggers new card', u\ray('field', 'photo')) : ' ');
  $question = $person ? textField(t('Question:'), t('Type a question no one but you could guess the answer to.'), required($mya->question)) : NULL;
  $answer = $person ? textField(t('Answer:'), t('The answer to that question'), required(@$answer)) : NULL;
/* KEEP   'idProof = $has ? 
        formField('item', t('Photo ID:'), '', R_ON_FILE . ($mya->ok ? '' : t(' (pending approval)')))
      : formField('file', t('Photo ID:'), tt('Upload an image of your @proofType. (maximum @R_MAX_PHOTOID_SIZE MB)', compact('proofType')), attrib(array('enctype = "multipart/form-data")));
      */
//    $submit = submit($has ? t('Save') : t('Upload and Save'));  );
  $submit = submit();
  $form = compact(u\ray('title pw pin photo question answer submit'));

  return labeled($form);
}

function formSecurity_validate($form, &$sta) {
  $mya = r\acct();
  extract(u\just('pin federalId dob', $sta['input']));
/*  if (!onFile('usdAccount', $sta)) {
    $usdAccount = $sta['input']['usdAccount'] = u\digits($usdAccount);
    if (!preg_match('/^[0-9]{10}$/', $usdAccount)) return say('bad account number', 'usdAccount');
  } */
/*  if (!onFile('pin', $sta)) {
    $pin = $sta['input']['pin'] = u\digits($pin);
    if (!preg_match('/^[0-9]{4}$/', $pin)) return say('bad pin', 'pin');
  } */
/*  if (!onFile('dob', $sta)) { // 12/14/1901 is the earliest
    if (!$dob = $sta['input']['dob'] = u\s2t($dob) or $dob > r\rTime()) return say('bad date', 'dob');
  }
  if (!onFile('federalId', $sta)) {
    $federalId = $sta['input']['federalId'] = u\digits($federalId);
    $what = !r\acct()->co ? 'social security number' : 'federal ID';
    if (!preg_match('/^[0-9]{9}$/', $federalId)) return say('bad federal id', compact('what'), 'federalId');
  } */
  
/*  $proofFilename = DRUPAL_ROOT . $mya->auxFilename('proof', TRUE);

  if (@$_FILES['files']['name']['idProof']) {
    $validators = array('file_validate_size' => array(R_MAX_PHOTOID_SIZE * 1024000));
    if (!$file = \file_save_upload('idProof', $validators, '', FILE_EXISTS_REPLACE)) return say('Try again.', 'idProof');
    if (!u\abbreviates('image/', @$file->filemime) and @$file->filemime != 'application/pdf') return say('bad file type', 'idProof');
    $tempfile = file_directory_temp() . '/' . $file->filename;
    if (!$key = $sta['input']['idProof'] = bin2hex(u\encryptFile($tempfile, $proofFilename, r\passSalt()))) return say('encryption failure', 'idProof');
    \file_delete($file);
  } elseif ($mya->hasAux('proof')) {
    $sta['input']['idProof'] = R_ON_FILE; // flag for no change
  } // not required! elseif (!$mya->co) return say('upload required', 'idProof'); */
}

function formSecurity_submit($form, &$sta) {
  $mya = r\acct();
  $info = u\just('federalId dob pin question answer idProof', $sta['input']);

  foreach ($info as $k => $v) if ($v == R_ON_FILE or $v == @$form[$k]['#default_value']) unset($info[$k]); // don't change if on file or unchanged

  if (!empty($info)) {
    if (!$mya->update($info)) return say('info not saved', 'ERR');
  }

  say('info saved');
}

//define('R_REL_EMPLOYER_COL', 2); // number of amount/employer column
define('R_REL_OWNER_COL', 3); // number of family/owner column

/**
 * Give someone access to the current account or mark them as an employee or owner.
 */
function formRelations($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;
//  if ($confirm = sureForm($sta)) return $confirm; // confirm only getUSD
  $acctIdentifiers = be\accountName($myid);
  $header = u\ray('Other account,Draw,My employee?,Family?,Permission,');
  $classes = u\prefixValues('rel-', u\ray('person draw employee owner permission link'));
  if ($co = $mya->co) {
    $header[R_REL_OWNER_COL] = 'Owns 10%+';
    if (!$mya->ok) $mya->stepDone('relations');
  }

  $sql = <<<EOF
    SELECT DISTINCT s.reid, s.main, s.other, IF(:myid=s.main, v.fullName, u.fullName) AS fullName,
      s.permission, s.employee, s.isOwner, s.draw
    FROM ((r_relations s RIGHT JOIN users u ON u.uid = s.main)
      RIGHT JOIN users v ON v.uid = s.other)
    WHERE s.main=:myid
    ORDER BY fullName, IF(:myid=s.main, s.other, s.main), (:myid=s.other)
EOF;
  $result = db\q($sql, compact('myid'));
  $rows = $raws0 = $raws = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $raws0[$reid] = $row; // original ("0") raw values from table
    $a = r\acct($other);
    $otherCo = $a->co;
    $otherPerson = !$otherCo;

    $idLink = R_NONE; // default to no link
    if (!$otherCo) {
      if ($employee and $permission >= r\perm(B_REFUND)) {
        $idLink = "<a href=\"$base_url/request-employee-rcard/$other\">request&nbsp;rCard</a>";
      }
    }

    $ii = count($rows) . "-$reid"; // "row-reid"
    $drawn = toggle('draw', $draw, $ii);
//    $employer = toggle('employeeOk', @$employeeOk2, $ii, @$employeeOk2 xor @$employee2);
    $isEmployee = !$otherCo ? toggle('employee', $employee, $ii) : R_NONE;
    $Owner = toggle('isOwner', $isOwner, $ii);
    $permissions = $GLOBALS['share permissions'];
// NO    if (!$mya->can(B_JOINED)) $permissions[$permission = B_JOINT] .= '?'; // pending agreement
    if ($co or $otherCo or !$mya->ok or !$a->ok) unset($permissions[r\perm(B_JOINT)]); // no joint accounts with a company or nonmember
    $perm = $otherPerson ? dropdown('permission', $permission, $permissions, $ii) : R_NONE;
    $amountValue = ''; // was amountField('amount', number_format($amount,0), $ii);

    $thisRow = array($fullName, $drawn, $isEmployee, $Owner, $perm, $idLink);
    $thisRaw = compact(u\ray('draw employee isOwner permission'));
//      if ($co) array_splice($thisRow, 1, 1, $amount); // companies can't be employees
    
    $rows[] = $thisRow; // what to show on screen
    $raws[] = $thisRaw; // original conceptual ("raw") values for what's show on screen
  }
  $sta['rawValues'] = compact('raws0', 'raws');
  
  $aExact = "a href=\"$base_url/help/payment-exchanges\"";
  $aInvite = "a href=\"$base_url/community/invite\"";
  $aAgent = "a href=\"$base_url/agent\"";
  $help = tt('Connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. If they don\'t have an account yet, <@aInvite>invite them</a>! Each permission includes all the permission above it.', compact(u\ray('aExact aInvite aAgent')));
  //If they wish NOT to open a real account, create an <@aAgent>rPOS Sign-in Account</a> for them.

  $subtext = '<span id="account-label">' . t('Current Account') . ":</span> <span id='account-identifiers'>$acctIdentifiers</span><br><br>$help";

  $attributes = array('id' => 'relations');
  $cols = [];
  foreach ($classes as $class) $cols[] = compact('class');
  $colgroups = array($cols);
  $caption = $sticky = '';
  $empty = tt('no relations');
  $list = theme_table(compact(u\ray('header rows attributes caption colgroups sticky empty')));
  
  $form = array(
    'title' => item(t('Relations')),
    'subtext' => item($subtext),
    'newPerson' => textField(t('Add Someone:'), t('name, id, email, or phone')), // autocomplete causes problems
//    'newPerson' => textField(t('Add Someone:'), t('name, id, email, or phone'), autocomplete('relation')),
    'list' => formField('item', '', '', array('markup' => $list)),
    'go' => submit(t('Save Changes')),
  );

  return $form;
}

function formRelations_validate($form, &$sta) {
//  if (confirming_v($sta)) return;
  $mya = r\acct();
  
  if ($newPerson = @$sta['input']['newPerson']) {
    $a = whois($newPerson, 'newPerson', $sta['input'], 'no self-relation');
    $id = $a ? $a->id : '';
    if ($id and r\relation(1, $mya->id, $id)) return say('already related', 'newPerson');
    $sta['input']['newPerson'] = $id; // save for submission handling
  }
  $managers = 0; // for company accounts there needs to be at least one manager
  foreach ($sta['input'] as $key => $value) {
    if (!strpos($key, '-')) continue;
    list ($keya, $rawnum, $reid) = explode('-', $key);
/*    if ($keya == 'amount') {
      $amount = $value;
      if ($err = u\badAmount($amount)) return say($err, compact('amount'), 'amount');
      if (($sta['input'][$key] = round($amount, 0)) != $amount) say('amount rounded', compact('amount'));
    } else
    */
    if ($keya == 'permission') {
      if ($value == r\perm(B_MANAGE)) $managers++;
    }
  }
  if ($mya->co and !$managers and !$mya->admin) return say('need a manager', 'permission');
}

function formRelations_submit($form, &$sta) {
//  if (confirming_s($sta)) return;
  $mya = r\acct();

  if ($other = @$sta['input']['newPerson']) { // adding a new relation
    $mya->newRelation($other);
    $who = r\acct($other)->fullName;
    say('report new relation', compact('who'));
  }

  $changes = [];
  extract(u\just('raws0 raws', $sta['rawValues']));
///  debug($sta['rawValues']);
  $joinPerm = r\perm(B_JOINT);
  
  $DBTX = \db_transaction();
  foreach ($sta['input'] as $k => $v) { // ... be\updateRelations must check permissions and data carefully
    if (!strpos($k, '-')) continue;
    list ($k, $rawnum, $reid) = explode('-', $k); // eg split permission-2-37
/**/ if (!in_array($k, $ks = array_keys($raws))) return say("bad field in relations: $k " . print_r($ks, 1), 'err'); // hack attempt?

    $v = 0 + strtr($v, array('true' => 1, 'false' => 0));
    $oldv = 0 + $raws[$rawnum][$k];
/// debug(compact('v','rawnum','k'));
/// debug(0 + $raws[$rawnum][$k]);
    if ($v != $oldv) { // changed
      if ($k == 'permission' and ($v == $joinPerm or $oldv == $joinPerm)) {
        if ($v == $joinPerm) {
          if (@$gotJoin) {say('too many joins', 'err'); continue;} else $gotJoin = TRUE;
        }
        r\setJoin($reid, @$gotJoin); // handle joining or unjoining accounts
      }
      $changes[$reid][$k] = $v;
      $changes[$reid]['original'] = @$raws0[$reid]; // save for update reporting and permission-checking
    }
  }
///  debug($changes);
  if (!empty($changes)) say(be\updateRelations($changes));
  unset($DBTX);
  
  if (!$mya->ok) return r\go('status', 'step completed');
  if (u\test()) return r\go('account/relations');
}

/**
 * @todo: focus on code input
 */
function formBoxes($form, &$sta) {
  $myid = r\acct()->id;
  if ($confirm = sureForm($sta)) return $confirm; // confirm only deletion
  if (form_step($sta, $info) == 'prove') {
    $new = u\fmtPhone($sta['storage']['values']['new']);
    $form = array(
      'title' => item(t('Verify')),
      'subtext' => item(tt('verify cell', u\ray('number', $new))),
      'code' => textField(t('Code:')),
      'new' => hidField($new),
      'verify' => submit(t('Verify')),
    );
    return $form;
  }

  $headers = array('#', 'Type', 'Device name', 'Last used');
  $types = u\ray(TX_CHANNELS);

  $sql = "SELECT channel, boxnum, code, boxName, access FROM r_boxes WHERE uid=:myid ORDER BY boxnum";
  $result = db\q($sql, compact('myid'));
  $list = [];
  while ($row = $result->fetchAssoc()) {
    extract($row);
    $type = $types[$channel += 0]; // make sure channel is int, not string
    $boxName = 
        @$boxName ? htmlspecialchars($boxName)
      : ($channel == TX_POS ? (t('POS Device #') . $boxnum)
      : ($channel == TX_WEB ? (t('Computer/Browser #') . $boxnum)
      : ($channel == TX_SMS ? u\fmtPhone($code)
      : 'ERROR')));
//    $list["x$boxid"] = array($boxid, $boxName, $restricted ? 'Yes' : 'No', u\fmtDate($access));
    $list["x$boxnum"] = array($boxnum, $type, $boxName, u\fmtDate($access));
  }

  unset($sta['input']['new']); // don't show number just added
  $form = array(
    'title' => item(t('Devices (computers, phones, tablets, etc.)')),
    'subtitle' => item(t('These are the devices used with this account.')),
    'boxes' => formField('tableselect', '', '', array('header' => $headers, 'multiple' => TRUE, 'empty' => tt('no devices')), $list),
    'new' => textField(t('Add a cell phone*: ')),
    'save' => submit(t('Save changes')),
//    'delete' => submit(t('Remove Selected')),
//    'postscript' => item(t('* You may have a cell phone listed on the <a href="account/contact">Contact Information</a> page, or even as a POS device on this page, but to use it for text message transactions you must add its phone number here.')),
    'postscript' => item(t('* A device may appear on this list more than once if you use it in different ways (for texting, as a Point of Sale device, and/or as a web browser).')),
  );
  return $form;
}

function formBoxes_validate($form, &$sta) {
  $myid = r\acct()->id;
  if (confirming_v($sta)) return;
  extract(u\just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'verify') {
    if (strtoupper($code) == svar('nonce')) return;
    previous_state($sta, 'bad nonce'); // can't just give error message, because then verification form persists
  } elseif ($op == 'delete') {
    if (no_selection($boxes)) {
      if ($new) $op = 'add'; else return say('nothing selected'); // user probably pressed Enter after specifying a new cell
    } else confirm(tt('confirm delete cell'), $sta);
  }

  if ($op == 'save') {
    if (blank_field(compact('new'))) return;
    $sta['values']['op'] = $op; // make it stick if user pressed Enter
    say(be\addCell($new, 'VALIDATE'), 'new');
  }
}

function formBoxes_submit($form, &$sta) {
  if (confirming_s($sta)) return;
  
  extract(u\just('op code boxes new', $sta['values'])); // values not input
  $op = op($sta);

  if ($op == 'save') {
    if ($info = previous_state($sta)) return say($info, 'new'); // just returning from failed validation of step 2 ('prove')
    svar('nonce', $nonce = u\nonce()); // don't use POST (user could peek)
    r\SMS\send($new, tt('verification code', compact('nonce')));
/**/ if (isDEV) debug("nonce is $nonce"); // (keep this) tell developer what code to type
    $info = compact('new');
    return form_step($sta, $info, 'prove');
  }

  if ($op == 'verify') {
    step_one($sta); // no next step (back to original form)
    say(be\addCell($new));
    $sta['redirect'] = 'account/boxes';
  }
    
  if ($op == 'delete') {
// needs work    foreach ($boxes as $key => $one) if ($one[1]) say(be\deleteCell(substr($one[1], 1))); // ignore the leading 'x'
  }
}

/**
 * Reset password by choosing a new one from an official email link.
 */
function formResetPassword($form, &$sta, $args = '') {
  extract(u\just('id code', $args)); // linked from an email

  if (flooding($uid = r\loginString(@$id), $sta)) return r\go('', 'flooded', 'ERR'); // getting hammered

  $mya = r\acct($uid);
  
  if (!passwordOkay($mya, @$code)) return r\go('', 'bad login', 'ERR');
  noFlood($sta);
  
  $title = item(t('Choose a New Password'));
  $nameDpy = item("$mya->fullName ($mya->mainQid)");
  $name = hidField($id); // used in loginFollowup
  $uid = hidField($uid);
  $newPass = formField('password_confirm', '', '', compact('process')); // not required()!!

  $strong = hidField(0, attrib(array('id' => 'edit-strong')));
  $submit = submit(t('Save'));
  focusOn('newpass-pass1');
  return compact(u\ray('title nameDpy name uid newPass strong submit'));
}

function formResetPassword_validate($form, &$sta) {
  extract(u\just('newPass strong', $sta['input']));
  if (!$strong and !(isDEV and @$newPass['pass1'] == '123')) return say('weak pass', 'newPass[pass1]');
}

function formResetPassword_submit($form, &$sta) {
  extract(u\just('newPass uid', $sta['values'])); // values not input (?)
  $mya = r\acct($uid);
  $mya->update('pass oneTimePass', $newPass, '');
  say('info saved');
  loginFollowup('', $sta + compact('uid')); // lead user to next step
}

/**
 * Login from elsewhere.
 *//*
function formSinx($form, &$sta) {
  extract(u\just('id pw pw2', t\POST())); // posted from rCredits.org or admin computer

  if (flooding($uid = r\loginString(@$id), $sta)) return r\go('', 'flooded', 'ERR'); // getting hammered
///  if (!\user_authenticate($name = r\acct($uid)->name, @$pw)) return r\go('', 'bad login', 'ERR');
  $a = r\acct($uid);
  // if admin, check pw2 against $a->pw2 (a datafield hash)
  $sta['uid'] = ((isDEV and $uid == 1) or passwordOkay($a, @$pw)) ? $uid : FALSE;
  if (!$sta['uid']) return r\go('', 'bad login', 'ERR');
  noFlood($sta);
  $input = ['name' => $a->name];
  loginFollowup('', compact('uid', 'input'));
}*/

/**
 * Completely redo the login form (see user_login() in user.module).
 * @see also loginFollowup, below (called from rweb_user_login()).
 */
function formSignin($form, &$sta) {
  global $base_url;

  extract(u\just('name pw2', @$sta['input']));
  unset($form['actions']);
  unset($form['pass']);
  focusOn((@$name == 'admin' or strlen(@$name) > 4) ? 'pass' : 'name');

  $title = item(t('Welcome to rCredits!'));
  $signup = isDEV ? "<a href=\"$base_url/signup/code=w\">Signup</a>" : '';
  $subtext = item(t('<p>Not yet a member? <@a>Check it out</a>!</p>', u\ray('@a', 'a href=' . r\promoLink(''))) . $signup . tt('<p><b>TIP</b>: Try not to sign in to your financial accounts on a public computer. Or if you do, be careful! Check the address bar at the top of the screen to make sure you have the right website (<b><em>@R_SITE_URL/</em>...</b>). And be sure to sign OUT when you\'re done!</p>'));

  $a = "a tabindex=3 href=$base_url/account/password/" . @$name; // uses $name, so must precede $name = textField
  $passDesc = tt('<@a>Password problems?</a>', compact('a'));
  $name = textField(t('Username/ID:'), t('Your account ID, email, or username (the letters of your full name, all lowercase)'), required(@$name));
  $pass = formField('password', t('Password:'), $passDesc, required());
  $pw2 = hidField(@$pw2 ?: ''); // extra password for admins
  
  $submit = submit(t('Sign in'));
  $form = compact(u\ray('title name pass pw2 submit subtext')) + $form;

  $form['#validate'] = array('rCredits\\Web\\login_extra_validate', 'user_login_final_validate'); // add replacement function
  $form['#submit'] = ['rCredits\\Web\\loginFollowup'];
  
  $form['external_input'] = hidField(serialize(@$sta['input'])); // pass this to rweb_user_login
//  $form['#submit'][] = 'rCredits\\Web\\login_form_submit';

  $form['#attributes']['class'][] = 'rweb labeled';
//  if (TRUE) js('e:\\rCredits-admin.js', 'external', 'footer'); // get extra admin password
  js('js/radiocheck.js', 'file', 'footer'); // this normally gets called in showForm()
  return t\snapShot($form);
}

/**
 * Replacement for user_login_authenticate_validate()
 * See the original function for explanation of flood control.
 * Accept name (short name) or account id, with password.
 * Sets $sta['uid'] to signify successful authentication.
 */
function login_extra_validate($form, &$sta) {
  extract(u\just('name pass pw2', $sta['input']));
  if (flooding($uid = r\loginString($name), $sta)) return FALSE; // getting hammered
  $a = r\acct($uid);
  $pw2 = isDEV ? DEV_PW2 : (@$pw2 ? hex2bin($pw2) : '');
  if (!isGAME and $uid == 1 and !w\passwordOkay($a, $pw2, 'pw2')) return say(t('Admin requires second pass.'), 'pass');
  $sta['uid'] = ((isDEV and $uid == 1) or passwordOkay($a, $pass)) ? $uid : FALSE;
  if ($sta['uid']) noFlood($sta); else return say('login failed', 'name');
  if ($uid == 1) r\setCook('pw2', $pw2, 0); // store as cookie, so pw2 is never on server
}

/**
 * Finish signing in.
 * @param assoc $sta: intermal input or external input (from a member merchant website), including
 *   $uid: record ID of account signing in
 * @param string $goto: what page to start on for non-admin
 */
function loginFollowup($form, $sta, $goto = 'summary') {
  extract(u\just('uid input external_input', $sta));
  if (!@$uid) hack('sign-in is not authorized');
  $mya = r\acct($uid);
  global $user; $user = $mya->account('uid pass timezone'); // timezone keeps Drupal happy in bootstrap.inc
  drupal_session_regenerate(); // this may need to be disabled for testing?

  extract(u\just('name', $sta['input']));
  
  foreach ($_SESSION as $key => $value) if (u\abbreviates('rcredits_', $key)) unset($_SESSION[$key]);
  svar('external_input', @unserialize($sta['external_input'])); // save params from external online merchant

  svar('myid', $uid);
  svar('scanned_in', FALSE);
  setGlobals($uid, TRUE);

//  r\setCook('loginto', '', r\rTime()-1); // no more default login
  u\loga('login', compact('name'), $uid);
  if ($mya->admin) {
    include_once __DIR__ . '/../admin/admin.inc';
    a\adminSignin();
  }
  return r\go($uid == 1 ? 'sadmin' : $goto);
}

function formPass($form, $sta, $id = '') {
  $id = urldecode(u\urlArgs());
  $title = item(t('Request New Password'));
  $name = textField(t('Account:'), t('Username, Email, or Account ID'), required(u\neq($id, 'password', '')));
  $submit = submit(t('Request'));
  /*$form['name']['#title'] = t('Account:');
  $form['name']['#description'] = t('Username, Email, or Account ID');
  $form['name']['#default_value'] = u\neq($id, 'password', '');*/
//  $form['#validate'] = array('rCredits\\Web\\formPass_validate');
//  $form['#submit'] = array('rCredits\\Web\\formPass_submit');
//  $form['#attributes']['class'] = array('rweb', 'labeled');
//  t\snapShot($form);
  $form = compact(u\ray('title name submit'));
  focusOn('name');
  return labeled($form);
}

function formPass_validate($form, &$sta) {
  extract(u\just('name', $sta['input']));
  if (!@$name) return say('missing field', array('field' => 'account'), 'name');
  if (!@$uid = r\loginString($name)) return say('bad account id', 'name');
  $a = r\acct($uid);
  if ($a->co) return say('no co pass', u\ray('company', $a->fullName), 'name');
  $sta['input']['uid'] = $uid;
}

function formPass_submit($form, &$sta) {
  extract(u\just('uid', $sta['input']));
  u\EXPECT($uid, 'missing uid in pass_submit');
  $a = r\acct($uid);
  $name = $a->name;
  $oneTimePass = oneTimePass($name);
  list ($region, $code) = array(strtolower(R_SERVER_ID), $oneTimePass['pass']);
  $a->update(compact('oneTimePass'));
  r\notify($uid, 'password-reset', compact(u\ray('name code')), TRUE);
  say('password reset');
  r\tellStaff('password reset', compact(u\ray('name uid')), $uid);
  return r\go('empty');
  //$sta['redirect'] = 'user';
}

function formFindCo($form, &$sta) {
  $mya = r\acct();
  $which = svar('which_companies') ?: '';
  $region = svar('region') ?: '';

  $list = directoryList($which, $region ?: @$mya->community);
  
  $form = array(
    'title' => item(t('Find companies in your region')), // should be 'Find member businesses in your region'
//    'subtext' => item(t('for a country name or postal code...')),
    'which' => textField(t('Search for:'), t('Type part of company name or industry category'), dft($which) + autocomplete('industry')),
    'region' => textField(t('Where:'), t('Type the first few characters of your postal code<br>For example, use 013 for Franklin County, Massachusetts; 05 for Vermont, etc.'), dft($region)), // or country name
    'submit' => submit(t('Find')),
    'item' => item($list),
  );

  return labeled($form);
}

function formFindCo_validate($form, &$sta) {
}

function formFindCo_submit($form, &$sta) {
  extract($sta['input']);
  svar('which_companies', trim($which));
  svar('region', trim($region));
}

function formCompany($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $myid = $mya->id;

  $sql = <<<EOF
    SELECT iid, IF(parent=iid, UCASE(industry), CONCAT('--', industry)) AS cat, parent 
    FROM r_industries ORDER BY parent, IF(parent=iid,0,iid)
EOF;
  $cats = db\q($sql)->fetchAllKeyed();
  $myCats = db\q('SELECT iid FROM r_user_industries WHERE uid=:myid', compact('myid'))->fetchCol();
  $links = <<<EOF
  <a id='show-webpage' href="$base_url/member/$mya->name" target="_blank">See your public rCredits web page</a> (in a new window)
EOF;

  $canDescs = u\ray(CO_CAN_BIT_DESCS);
  $permissions = '';
  
  foreach ($canDescs as $i => $desc) {
    $outbox = <<<EOF
<div class="form-item form-type-checkbox">
  <input type="checkbox" name="can[$i]" CHECKED class="form-checkbox" />
  <div class="description"><div class="box"></div></div>
</div>
EOF;
    $j = $i + CO_CANS_BITS; // convert signed out to signed in index
    $inbox = str_replace("can[$i]", "can[$j]", $outbox);
    if (!$mya->coCan(CO_CANS + $i)) $outbox = str_replace('CHECKED ', '', $outbox); // uncheck if not set
    if (!$mya->coCan(CO_CANS + $i + CO_CANS_BITS)) $inbox = str_replace('CHECKED ', '', $inbox);
    $permissions .= "<tr><th>$desc</th><td>$inbox</td><td>$outbox</td></tr>\n";
  }

  $permissions = <<<EOF
<div id="permissions"><table>
<tr id="perm-head"><td></td><th>Signed IN</th><th>Signed OUT</th></tr>
$permissions
</table></div>
EOF;

  $title = item(t('Company Information'));
  $links = item($links, ' ');
  $name = formField('item', t('Company name:'), '', $mya->fullName);
  $photo = item(t('<@a>Change your company photo</a>', u\ray('@a', 'a href=photo')), t('Photo:'));
  $categories = formField('select', t('Categories:'), t('Hold down the Ctrl key to select more than one'), ['multiple' => TRUE] + dft($myCats), $cats);
  $selling = formField('textarea', t('Selling:'), t('Enter one or more transaction descriptions for when you make a sale &mdash; one line for each. Put the most common one first.'), required($mya->selling));
  $can2 = item($permissions, t('Permissions:'), t('Download the rPOS app from Google Play Store. Limit app permissions here, or for each agent on the Relations page.'));
//    $shareRpos = boolField(t('Share rPOS:'), t('Allow other companies to use our POS device(s).'), dft($mya->coCan(CO_SHARE_RPOS) ?: 0));
//    $requireCashier = boolField(t('Must scan in:'), t('Require a cashier to sign in before using rPOS (this also allows you to share the device with another company).'), dft($mya->coCan(CO_REQUIRE_CASHIER) ?: 0));
  $website = textField(t('Website(s):'), t('Especially if you want to accept payments on your website, be sure to list it here.'), dft($mya->website));
  $descPreface = item(t('<br>What does this company do? The description you enter below will appear on your rCredits web page.'));
  $description = formField('text_format', t('Description:'), '', ['format' => 'filtered_html'] + dft(strtr($mya->description, ["\r" => '', "\n" => '', '<br>' => PHP_EOL, "\0" => ''])));
  $shortDesc = textField(t('Short Desc:'), t('Short description of your company\'s products and services. Maximum 35 characters.'), attrib(['maxlength'=>35]) + required($mya->shortDesc));
  $employees = textField(t('Employees:'), t('How many employees do you have?'), required($mya->employees));
  $gross = textField(t('Annual Gross:'), '', required($mya->gross));
  $tips = boolField(t('Show tip %s:'), t('After each rCard sale, ask customer to choose a tip percentage (or no tip)'), dft($mya->coCan(CO_TIP) ?: 0));
  if ($mya->cAdmin) $fast = boolField(t('No ID Chk:'), t('Never ask customer for photo ID (set by admin)'), dft($mya->coCan(CO_FAST) ?: 0));
  $submit = submit();
  return labeled(compact(u\ray('title links name photo categories selling shortDesc employees gross website descPreface description can2 tips fast submit')));
}

function formCompany_validate($form, &$sta) {
  extract(u\just($fields = 'selling employees gross', $sta['input']));
  $selling = explode("\n", $selling);
  foreach ($selling as $one) {
    $one = trim($one); // don't allow empty or reserved values
    if (!u\nonish($one) and !in_array($one, array('', S_REFUND, S_USD_OUT)) and !r\usdin($one)) $new[] = strtolower($one);
  }
  if (!@$new) return say('missing field', u\ray('field', 'selling'), 'selling');
  $selling = join("\n", $new);
  if ($err = u\badAmount($employees, '>=0')) return sayFieldErr($err, 'employees');
  if ($err = u\badAmount($gross, '', 2, '')) return sayFieldErr($err, 'gross');
  u\preray(compact(u\ray($fields)), $sta['input']);
}

function formCompany_submit($form, &$sta) {
  $mya = r\acct();
  $id = $mya->id;
  extract(u\just('description shortDesc website selling categories can requireCashier employees gross tips fast', $sta['input']));
//  $description = \check_markup($description['value']);
  if (is_array($description)) $description = @$description['value']; // when u\test(), description is scalar
  $description = str_ireplace([PHP_EOL, "\r", "\n", "\0", '</li><br>', '<ol><br>', '<ul><br>'], ['<br>', '', '', '', '</li>', '<ol>', '<ul>'], $description);
  $website = str_ireplace('http://', '', @$website);

  $mya->setCoCans(@$can);
  //$mya->setBits(CO_CAN_BITS, $can, 'co');
//  $mya->setBitx(CO_REQUIRE_CASHIER, @$requireCashier, 'coFlags');
  $mya->setCoBit(CO_TIP, $tips);
  if ($mya->cAdmin) $mya->setCoBit(CO_FAST, $fast);
  $mya->update($info = compact(u\ray('description shortDesc website selling employees gross')));
  
  $DBTX = db_transaction();
  db\q('DELETE FROM r_user_industries WHERE uid=:id', compact('id')); // out with the old
  if (@$categories) foreach ($categories as $iid) {
    db\q("INSERT INTO r_user_industries (uid, iid) VALUES (:id, :iid)", compact('id', 'iid')); // in with the new
  }
  unset($DBTX); // commit

  $mya->stepDone('company', $info);
  return $mya->ok ? say('options saved') : r\go('status', 'options saved|step completed');
}

/**
 * Show a page when someone scans an account ID QR, for example with QRDroid.
 * Cases:
 *   Curious about someone's card (not signed in, scanned person) also give a link to the promo site
 *   Agent looking at a business (signed in, scanned company) - show Site button (to company's rCredits page)
 *   Person looking at a business (signed in, scanned company) - show Site button on the Pay page (NYI)
 
 UNUSED:
 *   Scanning in (not signed in, scanned agent)
 *   Signing in (not signed in, scanned person) - show login page with account id filled in
 *   Agent charging a customer (signed in, scanned any) - show photo and Charge button
 *   Agent paying someone (signed in, scanned any) - show Pay button
 *   Person paying someone (signed in, scanned any) - go to Pay page

 * @param string $who: the part of a scanned rCard's QR code after the domain and '/I/':
 *   the account's qid tail, then "." (pro se) or "-" (agent), then an rCard security code:
 *   must match the cardCode (for pro se) or cardCode2 (for agent) stored in the account's secure field.
 */
function formI($form, &$sta, $who = '') {
  $parts = preg_split('/(' . R_MARKS . ')/', $who, 0, PREG_SPLIT_DELIM_CAPTURE);
  $count = count($parts);

  if ($count == 5) { // old fashioned format: ABC.DEF-whatever or ABC-DEF-whatever (agent)
    list ($region, $mark, $tail, $zot, $code) = $parts;
  } elseif ($count == 3) { // new format: DEF.whatever or DEF-whatever (agent)
    list ($tail, $mark, $code) = $parts;
    $region = R_SERVER_ID;
  } else return say('bad member id', 'ERR'); // not an ID QR
  
  if ($mark == R_AGENT_URL_MARK) {
    list ($mark, $field) = array(R_AGENT_MARK, 'cardCode2');
  } else $field = 'cardCode';
  
  $qid = "$region$mark$tail";
  if (!$a = r\acct($qid)) return say('bad card', 'ERR'); // not a real code -- just go to home page
  
  $mya = r\acct();
  if (!$gotCode = @$a->agent->$field) return say('bad card code', 'ERR');
  if ($gotCode != $code) return say('bad card code', 'ERR');
  list ($otherId, $otherAgent) = array($a->id, $a->agentId);

  //  if (!$mya) return $a->proSe ? r\go('user/login?name=' . $a->name) : scanIn($a);
  return r\go($a->co ? "member/$a->name" : r\promoLink(''));
  
  /*
  $shortName = $a->name;
  $txArgs = "/who=$shortName&scanned=1";
  if ($mya->proSe and !$co) return r\go("pay$txArgs");
  
  list ($myid, $myAgent) = array($mya->id, $mya->agentId);
  if ($otherId == @$myid) return r\go('', 'no self-trading');

  if ($co) $site = goButton(t('Show Webpage'), $goSite = "member/$shortName", t("See this company's rCredits webpage"));
  if ($mya->can(B_SCAN)) $charge = goButton(t('Charge'), "charge$txArgs", t('Charge someone'));
  if ($mya->can(B_BUY)) $pay = goButton(t('Pay'), "pay$txArgs", t('Pay someone'));
//    if (r\acct($myid, $otherAgent)->can()) $changeAgent = goButton(t('Change Agent'), "user/logout/who=$shortName", t('Use this button when your shift begins'));
//  if (r\acct($otherId, $myAgent)->can()) $changeAccount = goButton(t('Change Acct'), "change-account/who=" . $a->mainQid, t('Manage a different account'));
  $theId = hidField($otherId);
  $theAgent = hidField($otherAgent);

  if (!@$charge) return $co ? r\go($goSite) : r\go('empty', 'no can scan');

  $zot = compact(u\ray('charge pay site'));
  $all = identifiers($a) . render($zot);
  svar('lastCustomer', $a->id);
  $all = formField('item', '', '', $all);
  return compact('all'); */
}

function formOptions($form, &$sta) {
  global $base_url;
  $mya = r\acct();
  $isSetUp = !is_null($mya->minimum); // user has already saved some options
  $title = item(t('Account Preferences'));
//  $virtualMaxMessage = $mya->co ? t(' If you choose to enable payment exchanges, this setting is moot (irrelevant and unused).') : '';
  $subtext = item(t('(The defaults here are appropriate for most people.)'));
  $minimum = textField(t('Minimum:'), '<p>' . t('How much to keep in your rCredits account, for purchases or inflation-proof savings. When your rCredits balance goes below this amount, the system automatically refills it for you from your bank account. Refilling may take 3-5 business days. Choose zero if you tend to bounce checks, live close to the wire, or prefer not to use automatic refills.') . '</p><p>' . tt('note|ach warning') . '</p>', required(u\fmtAmt($isSetUp ? $mya->j_minimum : R_MINR_DEFAULT, 's$')));
  $achMin = textField(t('Min Transfer:'), t('Choose a multiple of $@dft. This much (or more, if necessary) will be transferred from your bank account to your rCredits account when your balance goes below the minumum you chose, above.', ['@dft'=>R_ACHMIN]), required(u\fmtAmt($mya->achMin ?: R_ACHMIN_DEFAULT, 's$')));
  //$maxDft = $isSetUp ? $mya->maximum : R_MAXR_DEFAULT;
  //$maxDft = $maxDft < 0 ? '' : u\fmtAmt($maxDft, 's');
//  $maximum = textField(t('Maximum:'), tt('prompt max') . $virtualMaxMessage, dft($maxDft));
//  $toBank = formField('checkbox', t('To bank:'), t('(recommended) Automatically transfer the excess to your bank account. [<a href="../help/to-bank">why ever not?</a>]'), dft($isSetUp ? $mya->can(B_TO_BANK) : TRUE));
  if ($mya->ok) $share = w\sharefield($mya); // don't show this during signup (it's on the donation page)

  $noticeRadios = array(t('daily'), t('weekly'), t('monthly'));
  $dft = $mya->can(B_WEEKLY) ? 1 : ($mya->can(B_MONTHLY) ? 2 : 0);
  $notices = formField('radios', t('Email notices:'), t('How often, at most, do you want an email digest of your rCredits messages? (No email if no messages.)'), required($dft), $noticeRadios);
  $smsNotices = boxField(t('SMS notices:'), tt('Send me a text message when I receive a payment, charge, or invoice (requires a <a href="' . $base_url . '/account/boxes">connected cell phone</a>).'), $mya->can(B_BYSMS)); // (currently disabled in CSS)
  $mediumRadios = array(
    t('I will accept electronic statements**.'), 
    t('I choose to receive printed statements by mail.') . u\SUBS(t('<div>I agree to pay $:R_STATEMENT_COST per month to cover the postage and handling costs for this service.</div>'))
  );
  $statements = formField('radios', t('Statements:'), '', required((int) $mya->can(B_PAPER)), $mediumRadios);

  $secretBal = boxField(t('Secret Balance:'), t("Don't let merchants see my balance, even if I ask."), $mya->can(B_SECRET));
  // Double confirmation: by SMS, by email, either. require extra confirmation of crucial account changes
  $limit = u\fmtAmt(-$mya->j_floor);
  $nosearch = boxField(t('No Search:'), tt("Require other members to identify my account by name or account number only &mdash; not by phone or email."), $mya->can(B_NOSEARCH));
  $debtOk = boxField(t('Debt Okay:'), tt('Let me use my rCard as a <i>credit</i> card, allowing my account balance to go negative, limited by my current credit limit (currently @limit &mdash; based on my account activity). I understand there will be a fee (currently $@R_DEBT_FEE plus @R_DEBT_PERCENT% per month) if I fail to bring my balance back up to zero within @R_DEBT_DAYS days.', compact('limit')), $mya->can(B_DEBT));
  $submit = submit(t('Update Preferences'));
  $posttext = item(tt(<<<EOF
<h4>** Disclaimers required by the IRS:</h4>
<p>Your choice of how to receive statements applies to all statements you receive from now on. You can change your preference at any time by returning to this Preferences page (using the Settings gear in the upper right corner) or by notifying the regional administrator by mail: @regName, @regAddr. Any change will be confirmed in writing (electronically if you accept electronic statements, otherwise by US Mail).</p>
<p>To view your electronic statements, you need an typical computer or mobile device connected to the internet, including an email program and a web browser. To print the statements, you need a printer.</p>
<p>Your annual tax statements will be accessible online from January through December.</p>
EOF
  , u\ray('regName regAddr', r\regionfield('legalName'), r\regionfield('postalAddr'))));
  
  $form = compact(u\ray('title subtext minimum achMin share smsNotices notices statements secretBal nosearch debtOk submit posttext'));
//  js('showMax();', 'inline', 'footer');
  return labeled($form);
}

function formOptions_validate($form, &$sta) {
  $mya = r\acct();
  $floor = u\fmtAmt($mya->j_floor);
  if ($err = amtErr('minimum', $sta)) return say($err, 'minimum');
  if ($err = amtErr('achMin', $sta)) return say($err, 'achMin');
  if ($mya->ok and $err = w\badShare($sta['input']['share'], TRUE)) return say($err, 'share');

  extract(u\just('minimum achMin', $sta['input'])); // after amtErr checks
  if ($minimum < $mya->j_floor) return say('min sub floor', compact('floor'), 'minimum');
  if ($achMin < R_ACHMIN) return say('bad achmin', 'achMin');
}

function formOptions_submit($form, $sta) {
  $mya = r\acct();
  extract($info = u\just('minimum achMin share smsNotices notices statements nosearch debtOk secretBal', $sta['input']));

//  $mya->setBit(B_NOTIFY_EMAIL, $notifyBy & 1, FALSE);
  $flags = $mya->flags;
  u\setBit($flags, B_BYSMS, @$smsNotices);
  u\setBit($flags, B_WEEKLY, @$notices == 1);
  u\setBit($flags, B_MONTHLY, @$notices == 2);
  u\setBit($flags, B_PAPER, @$statements);
  u\setBit($flags, B_DEBT, @$debtOk);
  u\setBit($flags, B_SECRET, @$secretBal);
  u\setBit($flags, B_NOSEARCH, @$nosearch);
  
  if (@$statements) r\tellStaff($mya->mainQid . ' wants paper statements');
  
  if ($jid = $mya->jid and $mya->slave) { // slave setting minimum and/or achMin
    r\acct($jid)->update(compact('minimum', 'achMin')); // set it only on master
    $minimum = 0;
  }
  $mya->update(compact(u\ray('minimum achMin share flags')));
  $mya->stepDone('prefs', $info);
  return $mya->ok ? say('options saved') : r\go('status', 'options saved|step completed');
}

function formReports($form, &$sta) {
  if ($mya = r\acct()) {
    $admin = $mya->admin;
// (use Admin tab instead)    $cadmin2 = $mya->can(B_CADMIN2);
    if ($myCtty = $mya->community and $ctty = r\acct($myCtty)) $cttyName = $ctty->fullName;
  }

  $title = item(t('Community Statistics'));
  $subheader = item(@$cttyName ? t('for ') . $cttyName : ' ');
  $subtext = item(t('This page helps us keep an eye on our economy so we can manage it effectively together. <b class="loud">Click</b> any chart for an explanation, or <b class="loud">hover</b> for the numbers.'));
  $charts = item(w\showCharts('all'));
//  $shared = @$admin ? item(fundRpt($shared, @$lm_shared, '$'), t('Shared w CGF:')) : NULL;

  $form = compact(u\ray('title subheader subtext charts'));

  return $form;
}

function formReports_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formReports_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'community');
}

/**
 * Show accounts and recent transactions and ACHs that carry risk of fraud.
 */
function formFlags($form, &$sta, $args = '') {
  $mya = r\acct();
  $MAXLINES = 20; // max number of lines to show in each section
  include __DIR__ . '/risk-descs.inc';

// fails  u\setDft($args['period'], 1);
  extract(u\just('period starting ending download downloaded', $args)); 
  if (!@$period) $args['period'] = $period = 7;
  extract(u\just('dateRange url starting ending', dateRange($args, 'community/flags', [], '', FALSE)));
  $url = hidField($url);
  
  $title = item(t('Suspicious Activity Report')); //  . strftime('%A, %m/%d/%y')
  $subtext = item(tt(t('<p>Any potentially suspicious accounts, transactions, and bank transfers (ACHs) are listed here, so all members can keep an eye on things. Only the top @MAXLINES are listed in each category. System administrators will investigate any account or transaction with a suspiciousness score of @K_RED or more (shown in red).</p><p>Green risks are "good" (they mitigate other risk factors). Hover over any risk for a brief description or click it for details.</p>'), compact('MAXLINES')));

  $achHeader = $txHeader = $acctHeader = '';
  $accts = $txs = $achs = '';
  $headers = u\ray(t('Score Account Type Created Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $acctHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $acctHeader = "<tr>$acctHeader</tr>";
  list ($cttyCrit, $cttysCrit) = (!$mya or $mya->admin)? [1, 1] 
    : ["community=$mya->community", "$mya->community IN (u1.community, u2.community)"];
  $sql = <<<EOF
    SELECT risk AS score,uid,IF(:IS_CO,'Co','Indiv') as type,created,risks FROM users 
    WHERE uid>1 AND :IS_OK AND $cttyCrit
    ORDER BY risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql);
  for ($i = 0; $line = riskLine($q, K_ACCT_RISKS, $red) and ($i < K_LINES or $red); $i++) $accts .= $line;
  
  $headers = u\ray(t('Score Tx# Date Amount From To Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $txHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $txHeader = "<tr>$txHeader</tr>";
  $subs = compact('starting', 'ending');
  $sql = <<<EOF
    SELECT t.risk AS score,xid,t.created,amount,payer,payee,payeeFor AS description,t.risks 
    FROM r_txs t LEFT JOIN users u1 ON u1.uid=payer LEFT JOIN users u2 ON u2.uid=payee 
    WHERE (payer>0 OR ABS(amount)>:K_CTTY_THRESHOLD) 
    AND $cttysCrit AND t.created BETWEEN :starting AND :ending 
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $txs .= $line;

  $headers = u\ray(t('Score Tx# Date Amount Account Description Risks'));
  foreach ($headers as $v) {$vLow = strtolower($v); $achHeader .= "<th class=\"k-$vLow\">$v</th>";}
  $achHeader = "<tr>$achHeader</tr>";
  $sql = <<<EOF
    SELECT t.risk AS score,txid,t.created,ABS(t.amount) AS amount,payer,IF(t.amount<0,'in','out') AS way,t.risks 
    FROM r_usd t LEFT JOIN users u ON u.uid=payer
    WHERE t.created BETWEEN :starting AND :ending AND $cttyCrit
    ORDER BY t.risk DESC LIMIT :K_LINES
EOF;
  $q = db\q($sql, $subs);
  for ($i = 0; $line = riskLine($q, K_TX_RISKS, $red) and ($i < K_LINES or $red); $i++) $achs .= $line;
  
  $table = item(<<<EOF
  <center>
  <h3>ACCOUNTS</h3>
  <table id="accts">
  $acctHeader
  $accts
  </table>
  <h3>TRANSACTIONS</h3>
  <table id="txs">
  $txHeader
  $txs
  </table>
  <h3>ACHs</h3>
  <table id="achs">
  $achHeader
  $achs
  </table>
  </center>
EOF
  );

  if (@$downloaded) js("window.open(\"$url&download=1\", \"download\");", 'inline', 'footer');
  return labeled(compact('title') + $dateRange + compact('subtext', 'table'));
}

function formFlags_validate($form, &$sta) {w\dateRangeValidate($sta);}
function formFlags_submit($form, &$sta) {dateRangeRedirect('community/flags', $sta);}

function formCttyFunds($form, &$sta) {
  $title = item(t('Funds in Each rCredits Community'));
  $subtext = item(t('How rCredits were issued in each community. Hover over headings for more information.'));
  $headers = u\ray(t('Community r USD Trade Signup+ Purchase+ Invite+ Inflate+ Spent Grant Loan Invest Fees'));
  $headHelps = u\ray(t('Community name (abbreviated)|Total rCredits issued to date|In exchange for US Dollars|Net trade balance with other communities|Signup bonuses (incentive rewards for opening an rCredits account)|Purchase rewards (incentive rewards for buying or selling using rCredits)|Incentive rewards for inviting (and helping) a new member to open an rCredits account|Monthly inflation adjustments|rCredits community spending (net)|rCredits community grants|Zero-interest loans from the rCredits community|Investments by the rCredits community|Fees collected by the rCredits community'));
  foreach ($headers as $k => $v) $headers[$k] = w\popHelp($v, $headHelps[$k]);
  $headers = '<tr><th>' . join('</th><th>', $headers) . '</th></tr>';
  
  $q = db\q('SELECT DISTINCT ctty FROM r_stats s LEFT JOIN users u ON u.uid=ctty WHERE ctty=0 OR :IS_CGC ORDER BY ctty=0, fullName');
  while ($row = $q->fetchAssoc()) { // not fetchField because one value is 0
    extract($row);
    $stats = w\fundStats(r\stats($ctty), FALSE, '1');
    $list[] = '<tr><td>' . join('</td><td>', $stats) . '</td></tr>';
  }
  $list = join("\n", @$list ?: [t('No statistics are available')]); // @ is for tests
  
  $list = item(<<<EOF
<table id="cttyFunds">
$headers
$list
</table>
EOF
  );
  return compact(u\ray('title subtext list'));
}

/**
 * Return a formatted line for the next record, describing its risks.
 * @param SQLresult $q: the query result
 * @param string $descs: a space-delimited string array of risk descriptions and weights
 * @param bool $red: (RETURNED) the
 * @return an HTML table row to show the record
 */
function riskLine($q, $descs, &$red = '') {
  global $base_url; 
  if (!$row = $q->fetchAssoc()) return FALSE;
  
  if (@$row['amount'] + 0 < 0) list ($row['payer'], $row['payee'], $row['amount']) = array($row['payee'], $row['payer'], -$row['amount']); // don't show user negative amounts
  extract(u\just('uid xid created description score amount risks', $row));

  $regulator = (($mya = r\acct()) and $mya->can(B_REGULATOR));
  global $riskQ, $riskCount; 
  if ($q == @$riskQ) {
    $riskCount++;
    if ($riskCount > K_LINES) return FALSE;
  } else {$riskQ = $q; $riskCount = 1;}
  
  u\setDft($score, '');
  $score = round($score) + 0; // +0 to avoid "-0"
  $red = ($score >= K_RED);
  if ($red) $score = "<span class=\"risky\">$score</span>";
  if ($score <= K_GREEN) $score = "<span class=\"safe\">$score</span>";
  $row['score'] = $score;
  
  foreach (u\just('uid payer payee', $row) as $k => $v) {
    list ($region, $local) = explode('.', $qid = r\quid($v));
//    $row[$k] = "<span>$region.</span>$local";
    $a = r\acct($v);
    if ($v < 0) {
      $v = t('ctty');
    } elseif ($regulator) {
      $v = "<a href=\"$base_url/sadmin/acct-info/$qid\" title=\"$a->fullName\">$local</a>";
    } elseif ($a->co) {
      $v = "<a title=\"$a->fullName\">$local</a>";
    } else $v = t('...');
    $row[$k] = $v;
  }
  
  if (isset($amount)) $row['amount'] = number_format($amount, 2);
  if ($regulator and isset($xid)) $row['xid'] = "<a href=\"$base_url/sadmin/tx-info/$xid\">$xid</a>";
  if (isset($created)) $row['created'] = strftime(@$uid ? '%d-%b-%Y' : '%d-%b', $created);
  if (isset($description)) foreach ([t('labor'), t('automatic transfer'), t('donation')] as $k) {
    if (u\abbreviates($k, $description)) $row['description'] = $k;
  }
  $row['risks'] = r\riskList($risks, $descs);
  
  $line = '';
  foreach ($row as $k => $v) $line .= "<td class=\"k-$k\">$v</td>";
  return "<tr>$line</tr>\n";
}

/**
 * Accept an invitation by supplying the invitee email address.
 */
function formAccept($form, &$sta) {
  $title = item(t('Accept Your Invitation'));
  $subtext = item(strtr(t('A short @BLURB was sent with the invitation (click the link to see it).'), ['@BLURB' => blurbLink()]));
  $email = textField(t('Your email:'), t('Type the email address to which the invitation was sent.'));
  $submit = submit(t('Continue'));
  return labeled(compact(u\ray('title subtext email submit')));
}

function formAccept_validate($form, &$sta) {
  extract(u\just('email', $sta['input']));
  if ($error = \user_validate_mail($email)) return say('bad email', compact('email'), 'email');
}

function formAccept_submit($form, &$sta) {
  extract(u\just('email', $sta['input']));
  $email = u\cryptN($email);
  if (!$code = db\lookup('code', 'r_invites', 'email=:email ORDER BY (invitee IS NULL) DESC, invited DESC', compact('email'))) return say('bad invite', 'email');
  return r\go("signup/code=$code");
}

/**
 * Show forms 1099-B for the previous calendar year.
 */
function form1099b($form, &$sta) {
  $mya = r\acct();
  $title = item(t('Your rCredits "Barter" Income for ') . (date('Y') - 1) . t(' (Forms 1099-B)'));

  $list = "<table id='f1099b'>\n";
  $list .= "<tr><th>Date</th><th class=\"num\">Income ($)</th><th>From</th><th>Description</th></tr>\n";
  $total = 0;
  $negatives = 0; // total negative transactions (refunds that do not specifically reverse another tx)
  $cats = [];
  $map = [t('rebate on') => t('rebates'), t('bonus on') => t('bonuses')];
  
  while ($row = be\get1099b($mya->id)) {
    extract(u\just('created amount uid for', $row));
    if ($amount <= 0) { // do not issue 1099-B for negative amounts (per IRC 1.6041-...(c)?)
      $negatives += $amount;
      continue;
    }
    x\addCat($cats, strtr($for, $map), $amount, $dups);
    $date = strftime('%d-%b', $created);
    $total += $amount;
    $amount = str_replace('-', '- ', number_format($amount, 2));
    $a = r\acct($uid);
    $addr = str_replace(', Massachusetts', ', MA', $a->postalAddr);
    $phone = str_replace('+1 ', '', u\fmtPhone($a->phone));
    $text = htmlspecialchars("$a->fullName, $addr ($phone)");
    $who = $a->fullName;
    $list .= <<<EOF
<tr>
<td>$date</td>
<td class="num">$amount</td>
<td>$who</td>
<td>$for</td>
</tr>
EOF;
  }

  x\finishCats($cats);
  $total = str_replace('-', '- ', number_format($total, 2));
  $list .= "<tr><td><b>TOTAL</b></td><td class=\"num\"><b>$ $total</b></td><td></td><td></td></tr>\n</table>\n";
  $list = item($list, t('Detail:'));

  $summary = '';
  foreach ($cats as $cat => $ray) { // for each category
    list ($count, $amount) = $ray;
    $amount = number_format($amount, 2);
    $summary .= <<<EOF
    <tr><td class="category">$cat</td><td class"count">$count</td><td class="amount">$amount</td></tr>\n
EOF;
  }
  $summary = <<<EOF
<table id="summary">
<tr><th></th><th>Count</th><th>Total</th></tr>
$summary
<tr><td><b>TOTAL</b></td><td class"count"></td><td class=\"num\"><b>$ $total</b></td></tr>
</table>
EOF;
  
  $summary = item($summary, t('Summary:'));
  $regName = r\regionfield('legalName');
  $regPhone = str_replace('+1 ', '', u\fmtPhone(r\regionfield('phone')));
  $negatives = $negatives ? t('<p>You also had negative (refund) transactions totaling @NEG. These are not reported to the IRS, so you might include this negative transaction total on line 2 of 1040 Form C.</p>', ['@NEG' => u\fmtAmt($negatives)]) : '';
 
  $subtext = item(t(<<<EOF
<p>Pay taxes on rCredits income just as you would for US Dollar income, including income, excise, and sales taxes.</p>

<p>This page reports all rCredits payments made to you in the previous calendar year. Each individual income transaction is reported to the IRS (on Forms 1099-B) and to state government, as required by law &mdash; including your incentive rewards and inflation adjustments.</p>

<p><i>You</i> are NOT required to file Form 1099-B itself. Don't worry if some transactions listed are for payments that aren't income (such as reimbursement) and consequently you need to report different totals than appear on this page &mdash; the purpose of 1099-B is to report transactions, not total income.</p>

<p>The chart below suggests some tax form lines you might choose, for reporting your rCredits activity. It also suggests how you might report your monthly tax-deductible "sharing" contributions to Common Good Finance.</p>

<p>If you follow the <@a1>accounting practices recommended</a> by Common Good Finance, your software already includes your rCredits income, properly categorized, in your accounting records and reports, so you can disregard the totals on this page. Tax form lines that should include both rCredits and US Dollar income are shown in <font color="red">red</font> below.</p>
EOF
  , ['@a1' => 'a href=http://commongoodfinance.org/files/AccountingTips.pdf']) . $negatives);

  $table = item(<<<EOF
<table id="taxes">
<tr><th class="cat">Tax Form:</th><th>1120</th><th>1040</th><th>1040A</th><th>1040 Sched C</th></tr>
<tr>
  <td class="cat">Sales</td>
  <td><b>1a</b></td>
  <td><b>7</b> or 21 ("barter income")</td>
  <td><b>7</b></td>
  <td><b>1</b></td>
</tr>
<tr>
  <td class="cat">Inflation Adjustment</td>
  <td><b>5</b> or 10</td>
  <td><b>8a</b> or 21 ("barter income")</td>
  <td><b>7</b> or <b>8a</b></td>
  <td><b>6</b></td>
</tr>
<tr>
  <td class="cat">rCredits Incentive Rewards</td>
  <td>10</td>
  <td>21</td>
  <td>7</td>
  <td>6</td>
</tr>
<tr>
  <td class="cat">"Sharing" Donations</td>
  <td>19</td>
  <td>Subtract from Rewards</td>
  <td>Subtract from Rewards</td>
  <td>4, 8, 10, 17, or 27a</td>
</tr>
</table>
EOF
  );
  $posttext = item(tt('<p>This report is provided by @regName (EIN #@R_REGION_EIN). Address: @regAddr (@regPhone, @regEmail).</p><p>You can see this report here until January 1 of next year (when the report for calendar year @thisYear replaces it).</p><p>If you would rather receive reports by US Mail, you can change that setting in your <a href="account/preferences">Preferences</a>.', u\ray('regName regAddr regPhone regEmail thisYear', $regName, r\regionfield('postalAddr'), $regPhone, r\regionfield('email'), date('Y'))));
  return compact(u\ray('title subtext table summary list posttext'));
}

/**
 * Request an employee ID card (for buying on behalf of the company).
 */
function formRequestRCard($form, &$sta, $other = '') {
  $mya = r\acct();
  if (!$acct = r\acct($mya->id, @$other)) return hack('request rCard');
//  if (!$mya->co or !$mya->can(B_REFUND)) return say('no account permission');
  if (!$mya->can(B_REFUND)) return say('no account permission');
  $fullName = $acct->agent->fullName;
  
  $form = array(
    'title' => item(t('Request Employee rCard')),
    'subtext' => item(tt('A Company Agent rCard will be US Mailed to @fullName, to be used for purchases on behalf of the company. Your company account will be charged $@R_CARD_COST.', compact('fullName'))),
    'other' => hidField($other),
    'request' => submit(t('Request Card')),
    'cancel' => submit(t('Cancel')),
  );
  return labeled($form);
}

function formRequestRCard_submit($form, &$sta) {
  global $base_url;
  extract(u\just('other', $sta['input']));
  if (op($sta) == 'request') {
    $mya = r\acct();
    $reid = r\relation('reid', $main = $mya->id, @$other);
    $quid = r\quid($main, $other);
    $manager = $mya->agent->fullName;
    $otherName = r\acct($other)->fullName;
    $address = $mya->postalAddr; // was r\location($mya, TRUE);
    $href = "$base_url/print-rcard/$quid";
    r\tellStaff('company rcard', compact(u\ray('main other reid quid href manager otherName address')));
    say('card ordered', compact('otherName', 'manager'));
  } else say('no card ordered');
  return r\go('account/relations');
}

/**
 * Resubmit the phone number to Dwolla, to get a verification code.
 * Called directly from /account/readd-phone, as well as from setupUsd()
 *//*
function addPhone() {
  $us = new r\usd($mya = r\acct());
  if (!$phoneId = $us->addPhone(substr($mya->phone, 2), @$mya->verifyBy, $err)) {
    return r\go('account/contact', $err ?: t('Unknown error'), 'phone');
  }
  $mya->update('usdPhone', $mya->phone);

  svar('phoneId', $phoneId);
  return r\go('account/verify-phone');
}*/

function formFooter() {
  global $base_url, $rUrl;
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  $mya = r\acct();
  $extraLinks = ($mya and ($mya->can(B_MANAGE) or $mya->admin)) ? <<<EOF
    <a href="$base_url/community/donate">Donate</a> |
    <a href="$base_url/community/invite">Invite Someone</a> |
EOF
  : ($mya ? '' : '<a href="' . $base_url . '/signin">Sign in</a> |');

  $helpType = $mya ? '' : 'other';
  $siteLink = r\promoLink('');
  $securityLink = r\promoLink('security.html');
  $sysEmail = SYS_EMAIL;
  
  $markup =<<<EOF
<hr>
$extraLinks
    <a href="$siteLink">About rCredits</a> |
    <a href="$base_url/agreement">The Agreement</a> |
    <a href="$base_url/community/graphs">Communities</a> |
    <a href="$securityLink">Privacy & Security</a> |
    <a href="$base_url/help/$helpType">Help</a>
<br><br>
copyright &copy; $year <a href="http://CommonGoodFinance.org">Common Good Finance&reg;</a>, a 501(c)(3) nonprofit<br/>
PO Box 21, Ashfield, MA 01330 USA<br/>
+1 413-628-1723 &nbsp; <a href="mailto:$sysEmail" target="_blank">$sysEmail</a><br>
<div id="saywait"><br><img src="$rUrl/images/saywait.gif" /><br>processing...</div>
EOF;

  $footerText = item($markup); // unique name for this page

  $js = in_array(basename($_SERVER['REQUEST_URI']), u\ray('sadmin handy 1099-B')) ? '' : <<<EOF
var aut=false;
jQuery('.form-autocomplete').keydown(function(e) {aut=(e.which == 13);});
jQuery('a, :submit').not('.nospin, #help-link a, #help-close a, a[target="_blank"]').click(function() {
  if (aut) aut=false; else jQuery('#saywait').show();
});
EOF;
  
  $js .= <<<EOF
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
})();
EOF;

  js('js/bootstrap.min.js', 'file', 'footer');
  js('js/ie10-viewport-bug-workaround.js', 'file', 'footer'); // IE10 viewport hack for Surface/desktop 
  js($js, 'inline', 'footer');
  return compact('footerText');
}

/**
 * Return a display of participating companies (for promo site)
 */
function whosin($ctty = '') {
  if (!$uid = db\lookup('uid', 'users', 'name=:ctty', ['ctty' => "$ctty."])) return '';
  $list = r\companies($uid, TRUE);
  $map = [
    '<tr><td width="200">' => '<div class="row"><div class="cmpy-name">',
    '</td><td style="margin-left:10px;">' => '</div><div class="cmpy-does">',
    '</td></tr>' => '</div></div>',
  ];
  $list = strtr($list, $map); // temporary fudge
/**/  echo $list;
  exit();
}