<?php
namespace CG\Web;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use CG\Txs as x;
use CG\Admin as a;
use CG\Risk as k;
use CG\Web as w;

/**
 * @file
 * rWeb include file
 *
 * Utilities and definitions for the rWeb module
 * (anything that doesn't have to be in rweb.module)
 */

require_once __DIR__ . '/rweb-subs.inc';
require_once __DIR__ . '/../cg-backend.inc';
//include_once __DIR__ . '/rweb-txs.inc';

define('UPLOAD_DIR', dirname($_SERVER['DOCUMENT_ROOT']) . '/public_ftp/incoming');

/**
 * Show a form called from a form linked to in an email or during signup, without signing in.
 */
function formProx($form, &$sta, $args) {
  extract(just('page', $args0 = $args));
/*  foreach (ray('Event Democracy Proposal') as $k) {
    if (strpos($page, $k) !== FALSE) include_once DRUPAL_ROOT . R_PATH . '/rvote/rvote-web.inc';
  } */
  $file = strtolower(R_ROOT . "/forms/$page.inc");
  if (file_exists($file)) include_once $file;
  
  $func = 'CG\\Web\\form' . ucwords($page);
  $form = $func($form, $sta, $args0);
  foreach (['validate', 'submit'] as $k) {
    if (!@$form["#$k"] and function_exists($func . "_$k")) $form["#$k"] = [$func . "_$k"];
  }
  return $form;
}

/**
 * Print a JSON array of communities
 */
function cttyList() {
  $cttys = db\q('SELECT uid,fullName FROM users WHERE uid<0 ORDER BY fullName')->fetchAllKeyed();
  $cttys[0] = t('ALL communities');
  foreach ($cttys as $k => $v) $cttys[$k] = trim(str_replace(PROJECT, '', $v));
  asort($cttys);
  unset($cttys[$seed = r\serverUid()]);
  $cttys[$seed] = t('Seedpack (no community yet)'); // last
/**/ echo json_encode($cttys);
  exit();
}

/**
 * Sections with submenus.
 */
function formCommunity($form, &$sta) {return w\subMenu(t('Community'), 'community');}
function formHistory($form, &$sta) {return w\subMenu(t('Account History'), 'history');}
function formSettings($form, &$sta) {return w\subMenu(t('Settings'), 'settings');}

/**
 * Show an example invitation email.
 */
function inviteExample() {
  global $rUrl;
  global $mya;
  $s = file_get_contents("$rUrl/templates/invite.html");
  $note = str_replace("\n", '', t('invite default', 'name', ''));
/**/  echo tr($s, '_personalNote signed fullName phone CODE', $note, u\fmtDate($mya->signed), $mya->fullName, u\fmtPhone($mya->phone), '');
  exit();
}

function formVideos($form, &$sta) {
  $title = t('%PROJECT Member Videos');
  $subtext = t('The videos may not be available until mid-January. Try again in a few days!');
  $form = compact(ray('title subtext'));
  return cgform($form);
}

function formUser($form, &$sta) {return \user_login($form, $sta);} // called from w\go() in Summary

/**
 * Recalculate risk.
 * @param string $type: type of entity: acct or tx
 * @param int $id: record ID for the entity
 * @param bool $show: <show the risk calculation>
 */
function formRerisk($form, &$sta, $type = '', $id = '', $show = FALSE) {
  include_once __DIR__ . '/../cg-risk.inc';
  global $showRiskCalc; 
  
  $showRiskCalc = '';

  if ($type == 'acct') {
    k\acctRisk1($id, $risk, $calc);
    $w = ''; $m = K_RED;
  } elseif ($type == 'tx') {
    $x = r\x($id);
    k\txRisk1($x->ray, $risk, $calc);
    $w = u\fmtAmt($x->amount);
    $m = t('@K_RED / @K_THRESHOLD');
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
  w\go("sadmin/$type-info/$id", t('info saved'));
}

/**
 * List the people honored or remembered with contributions to Common Good.
 */
function formHonors($form, &$sta) {
  $title = t('Honored and Remembered');
  $subtext = t('Listed here are the people and principles honored and remembered with a <%a>contribution</a> to %PROJECT.', '_a', atag('/community/donate'));
  $oftens = ray(OFTENS);
  $q = db\q("SELECT DISTINCT honor,honored FROM r_gifts WHERE honored<>'' ORDER BY honor,honored");
  $list = '';
  $type = 'none';
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($honor != $type) {
      $list .= '<div class="honor-type">' . (trim(u\honorDesc($honor)) ?: t('other')) . "</div>\n";
      $type = $honor;
    }
    $list .= "<div class=\"well\">$honored</div>\n";
  }
  return cgform(compact(ray('title subtext list')));
}

/**
 * Output an account photo if the caller is authorized.
 * @param string $qid: account letter-code
 * @param bool $small: output a small version instead
 * @param bool $orig: show the original photo before it was cropped
 * @return the photo if the account is a company or the cardCode is valid. Otherwise a generic picture.
 */
function formAccountPhoto($form, &$sta, $args = '') {
  global $mya;
  extract(just('qid small orig', $args));

  header("Content-type: image/jpeg");

  if ($a = @$qid ? r\acct($qid, $mya->agentId) : $mya) {
    if ($a->co or $a->can(B_READ)) $photo = @$orig ? @file_get_contents($a->tempPhotoFilename) : $a->photo;
  }
//      if ($a->co ?: ($a->cardCode == $cardCode)) $photo = $a->photo; // was photo()
/**/  echo @$photo ?: \file_get_contents(DRUPAL_ROOT . R_PATH . '/images/icons/account.png');
  return exit();
}

//function formIcon($orig = FALSE) {return memberPhoto(@r\acct()->mainQid, TRUE, $orig);} // Output a small version of the current account's photo.

/**
 * Handle the uploaded photo: crop, resize, rotate, and save.
 * @param int $uid: current account record ID
 * @param string $sid: session ID
 * POSTed: parameters from croppic
 * @return json success or error message            
 */
function photoUpload($uid = '', $sid = '') {
  global $channel; $channel = TX_AJAX;
  global $base_url;
  $f0 = ray('imgUrl imgInitW imgInitH imgW imgH imgY1 imgX1 cropH cropW rotation uid sid');
  $f = ray('url w0 h0 w h y1 x1 cropH cropW angle uid sid');
  $params = $_POST; // don't sanitize this, it only comes from us!
  foreach ($params2 = array_combine($f0, $f) as $k0 => $k) $$k = $params[$k0];
  
  if (!@$uid or !@$sid or !$sessionVars = w\sessionVars($sid)) return w\photoRet(t('bad sid'));
  $mya = r\acct($uid);
  $tempFilename = $mya->tempPhotoFilename;
  $quality = $mya->co ? 100 : 50; // jpeg compression/quality (0=worst quality, 100=best quality)
  if ($mya->co) foreach ([$cropH, $cropW] as $k) $$k *= CO_PHOTO_FACTOR; // higher quality for businesses

  if ($recrop = u\starts($url, 'http')) $url = $tempFilename; // picture is already saved and is just being recropped
        
  if (!$what = @getimagesize($url)) return w\photoRet(t('That is not an image file.'));
  list ($actualW, $actualH, $type, $zotAttr) = $what;
  if ($actualW != $w0 or $actualH != $h0) r\tellAdmin('photo problem?', $_POST + compact('actualW', 'actualH', 'uid'));
  if (!(imagetypes() & $type)) return w\photoRet(t('That image format is not recognized.'));
  $ext = image_type_to_extension($type, FALSE);
  $func = 'imagecreatefrom' . ($ext == 'bmp' ? 'wbmp' : $ext);
///   flog(compact(ray('uid sid f0 f params params2 sessionVars quality cropH cropW what actualW actualH type zotAttr ext func')));
  if (!$img0 = $func($url)) return w\photoRet(t('image creation error'));
//  if (!$mya->co)
  try {
//    $filename = str_replace(' ', '_', DRUPAL_ROOT . R_PICTURE_DIR . "$mya->fullName-" . u\randomString(10, 'word') . '.jpg');
    if (!$recrop) imagejpeg($img0, $tempFilename, 100); // save for a few days in case we have to recrop
  } catch (Exception $e) {
/**/    lug('failed to save photo file ' . $tempFilename);
  }

  $img = u\alterImg($img0, $w, $h, $x1, $y1, $cropW, $cropH, $angle);

  try {
/*    if ($mya->co) {
      imagejpeg($img, DRUPAL_ROOT . $mya->photoFilename(TRUE), $quality);
    } else { */
      ob_start();
      imagejpeg($img, NULL, $quality);
      $photo = ob_get_clean();
      $mya->update(compact('photo')); // do not combine this with line above
//    }
  } catch (Exception $e) {
    return w\photoRet(t('Your photo could not be uploaded (%err). Try a smaller version or <%a>email it</a> to us?', ray('_err _a', $e->getMessage(), w\emailtag(NULL, t('Here is a photo for my %PROJECT card')))));
  }
  
//  return w\photoRet(str_replace(DRUPAL_ROOT, BASE_URL, $tempFilename), 'success');
  return w\photoRet("$base_url/account-photo", 'success');
}

/**
 * Account Selector form
 * appears above the whole Members Section
 */
function formAccounts($form, &$sta) {
  global $base_url, $rUrl;
  if (!$mya = r\acct()) return []; //['signin' => item('<a href="' . "$base_url/signin" . '">Sign in</a>')];

  $myid = @$mya->id; // may be a non-existent account if tests are run while looking at .ZZA, for example
  if (!$accounts = be\accountChoices()) return [];
//  if (!in_array($myid, @$accounts) and $mya->can()) array_splice($accounts, 1, 0, $myid); // admin or cadmin
  $choices = $ltrs = [];

  foreach ($accounts as $uid => $choice) {
    $a = r\acct($uid);
    if (!$a and isDEV) continue; // ignore this if testing -- "current" account has been undermined is all
    if ($mya->cAdmin or $mya->isAdmin2) {
      $qid = $a->mainQid;
      if ($mya->cAdmin and !$mya->admin) $qid = substr($qid, -2, 2); // show just last two letters for cAdmins
      if ($a->isCtty) { // show communities in a separate category
//        $class = $a->id < 0 ? 'cgc' : '';
        $choice = r\cttyName($uid);
        $ltrs['_'][$uid] = "<small>$qid</small> $choice";
      } else {
        $pos = substr($choice, 1, 2) == '. ' ? 3 
        : (substr($choice, 1, 1) == ' ' ? 2
        : (strtoupper(substr($choice, 0, 4)) == 'THE ' ? 4 : 0));
        $ltr = strtoupper(substr($choice, $pos, 1));
        $ltrs[$ltr][$uid] = "<small>$qid</small> $choice";
        $choice =  "$qid $choice";
      }
    }
    $choices[$uid] = $choice;
  }

  if ($mya->cAdmin or $mya->isAdmin2) {
    ksort($ltrs);
    $index = $details = '';
    foreach ($ltrs as $ltr => $accts) {
      $index .= "<a id=\"index-$ltr\">$ltr</a>";
      $detail = $detailDivs = '';
      $i = 0;
      foreach ($accts as $uid => $v) {
        $i++;
        $a = r\acct($uid);
        $class = '';
        foreach (ray('closed ok co') as $k) if ($a->$k) $class .= "$k ";
        $detail .= r\changeAcctLink($uid, $v, trim($class));

        if ($i == count($accts) or $i % 12 == 0) {
          $detailDivs .= "<div>$detail</div>\n";
          $detail = '';
        }
      }
      $details .= "<div id=\"index-$ltr-detail\" class=\"index-detail\">$detailDivs</div>\n";
    }
    $account = item("<div id=\"index\">$index</div>$details");
  } else {
    $links = '';
    foreach ($choices as $k => $v) {
      $one = $k == $mya->id ? $v : r\changeAcctLink($k, $v);
      $links .= "<li>$one</li>\n";
    }
    $account = item("<ul>$links</ul>");
  }
  
  $form = u\prefixKeys('acct_', compact(ray('account')));
  return $form;
}

function formAccounts_submit($form, &$sta) {
  extract(just('op acct_account', $sta['input']));
  $op = op($sta);
  if ($op == 'Menu') return w\go('menu'); 
}

/**
 * Change the current account.
 */
function changeWho($args = '') {
  global $mya;
  extract(just('acct page', $args));

  if (@$acct == 'signout') return signout();
  if (!@$mya and !isDEV) return say(t('No previous account. Try again.')); // get old current account
  $mya = r\acct($acct, @$mya->agentId); // tentative new account
  
  if (!@$mya or (!@$mya->admin and !@$mya->isAdmin2 and !$mya->can())) return hack('accts');
  
  w\setAcct($mya->id); // get new current account

//  if (isDEV) $page = str_replace(DEV_ROOT, '', @$page);
  $page = str_replace(',', '/', @$page);
  
  w\go($mya->cAdmin ? $page : 'summary'); // go to summary, unless admin
}

/**
 * Display the member company's profile (no editable fields here)
 */
function formProfile($form, &$sta, $company = '') {
  $uid = db\get('uid', 'users', 'name=:company', compact('company'));
  $a = r\acct($uid);
  if (!$uid or !$a->co) return w\softErr(t('That is not a %PROJECT company profile page.'));

  $pic = item($a->photoHtml());
  $state = r\realState($a->state);
  $country = r\realCountry($a->country);
  $siteList = '';
  if ($website = $a->website) foreach (ray($website) as $one) {
    $atag = atag("http://$one");
    $siteList .= "<$atag>$one</a><br>";
  }
    
  $title = $a->fullName;
  $phone = 'Phone: ' . u\fmtPhone($a->phone, '+');
  $physical = "$a->address<br>$a->city, $state $a->zip";
  $mailing = str_replace('<br>', ', ', $physical) == $a->postalAddr ? '' : "Mailing address: $a->postalAddr<br>";

  $contact = item("$physical $country<br>$mailing<br>
    $phone<br>
    $siteList");
	$desc = item($a->coDesc());	

  return cgform(compact(ray('pic title contact desc')));
}

/**
 * Say the site is down
 */
function formSystemDown($form, &$sta) {return array('zot' => item(t('system down')));}

/**
 * Pay or charge a nonmember (continued from Charge page)
 */
function formTxNonmember($form, &$sta, $args = '') {
  if (!$args or !$info = unserialize(u\deurlify($args))) return w\go('', t('Url Error'), 'err');
  extract($info = just('who amount goods purpose', $info));
  if (count($info) != 4) return w\go('', t('Url Error'), 'err');
  
  $title = t('Invoice a Non-member');
  $subtext = t('Sending an invoice to %email (%amount for %purpose)', 'email amount purpose', $who, u\fmtAmt($amount), $purpose);
  $name = textFld(t('Recipient Name:'), [t('Recipient\'s full name')], required());
  $submit = t('Send Invoice');
  $args = hidFld($args);
  return cgform(compact(ray('title subtext name submit args')));
}

function formTxNonmember_submit($form, &$sta) {
  extract(just('name args', $sta['input']));
  $info = unserialize(u\deurlify($args));
  be\invoiceNonmember($name, $info);
}

function formMenu($form, &$sta) {return array('title' => item('Menu'));}

/**
 * Present an empty slate for status and error messages.
 * @param mixed $form: usually unused (but required by Drupal); if string, output it.
 * @param string $args: say=1 (if called from doSay) or field=<field>&msg=<msg> (or empty)
 */
function formEmpty($form, &$sta = '', $args = '') {
  if ($form and !is_array($form)) say(plain($form));
  extract(just('say field msg', $args));
  
  $title = t('Miscellaneous'); // this is suppressed with CSS
  if (@$msg) say(plain($msg), @$field);
  if (@$say) $subtext = t('<p>To protect your account, <%span>no sign-in was required</span> for this action. To sign in safely, always go to %CG_DOMAIN.</p><p>You may now close this browser page.</p>', '_span', 'span class="loud"');
///   FAILS :(  if ($doSay == 'test') $testMessages = item(print_r(\drupal_get_messages(), 1));
  return cgform(compact(ray('title testMessages subtext')));
}

/**
 * Return a one-use auth code
 */
function authcode() {
  extract($args = just('company cocode', $_GET));
  if (!@$cocode or !u\isQid(@$company) or !($co = r\acct($company)) or !$co->co or $co->emailCode != $cocode) exit(t('bad company account ID or cocode'));
  $now = time();
  exit(substr(u\cry('P', u\randomString(1) . serialize(compact('cocode', 'now'))), 2));
}

/**
 * Certify that a business is indeed a Common Good participant.
 */
function formCgBiz($form, &$sta, $id) {
  if (!@$id or !$a = r\acct(strtoupper($id)) or !$a->ok) return w\go('', t('That is not a %PROJECT Business.'));
  w\go('member/' . $a->name, t('<big><b>%fullName</b> is a %PROJECT Business since %activated.</big> For more information about Common Good, <%a>click here</a>.', 'fullName activated _a', $a->fullName, u\fmtDate($a->activated), atag(r\promoLink(''))));
}

//function toggler($sel) {return w\attrib(['onclick' => "$('$sel').toggle(this.checked)"]);}

/**
 * Display a list of people nearby, waiting for an invitation.
 */
function formWaiting($form, &$sta) {
  global $mya;
  
  $title = t('People You Might Know, Waiting to Be Invited');
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
      if ($city = db\get('city', 'users', 'MID(zip, 1, 5)=:zip', compact('zip'))) $zip = $city;
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
  global $mya;
  $uid = $mya->cAdmin ? $mya->id : $mya->agentId;
  
  $title = t('Your Invitees');
  $subtext = t('<p>If the status of someone you invited is "<span class="invite-incomplete">Not Complete</span>", they have not finished setting up an account and may need your help.</p><p>If the status is "<span class="invite-expired">expired</span>", you may want to call them and send another invitation.</p>');
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
    if (!$started = strpos($who, ' ') and u\starts($who, CRYPT_FLAG . 'P')) $who = u\decry('P', $who); // decrypt email
    $status = ($member or $ok) ? t('is a Member!') : ($started ? t('account Not Complete') : ($expired ? t('expired') : '(no response yet)'));
    $color = $member ? 'member' : ($started ? 'incomplete' : ($expired ? 'expired' : 'other'));
    $nameClass = $co ? 'name co' : 'name';
    $list .= "<tr><td>$invited</td><td>$who</td><td class=\"invite-$color\">$status</td></tr>\n";
  }
  $list .= "</table>\n";
  
  $list = item($list);
  return compact(ray('title subtext list'));
}

define('R_BASIC_FIELDS', 'fullName federalId dob');

/**
 * Validate the data that affects social security number verification.
 * called from signup validation and ssn validation, as well as from formBasic()
 * @param string/bool $co: is this a company account ('mya' means use the current account's type)
 *   'mya' also means we are retrying basic information (the AccountInfo step) during signup
 */
function formBasic_validate($form, &$sta, $co = 'mya', $fields = R_BASIC_FIELDS) {
  extract(just($fields, $sta['input']));
  if ((string) $co == 'mya') {
    $mya = w\eLinkAcct($sta);
    $co = $mya->co;
  }
  if (isset($fullName) and $err = u\badName($fullName)) return say($err, ['field' => 'fullName'], 'fullName');

  $what = $co ? t('federal ID') : t('social security number');
  if (!$co and isset($dob) and $err = u\badDob($dob)) return say($err, 'dob');
  if (isset($federalId) and $err = u\badSsn($federalId, @$dob)) return say($err, compact('what'), 'federalId');

  u\preray(compact(ray($fields)), $sta['input']);
  return TRUE;
}

/**
 * Show Common Good's accountant (or any employee) our Deposits page, without the buttons
 */
function formShowDeposits($form, &$sta) {
  global $mya;
  if (!$mya or $mya->id != r\cgfId()) return w\softErr(t('You are not authorized to view that page.'));
  include_once __DIR__ . '/../admin/admin-forms.inc';
  return w\formDeposits($form, $sta);
}

/**
 * Show a page when someone scans an Common Good not with the CGPay app, for example with QRDroid.
 * RC4.ME and RC2.ME must do some interpretation and redirection (NYI on those sites)
 */
function formI($form, &$sta, $who = '') {
  $a = r\acct($who);
  return w\go(($a and $a->co) ? "member/$a->name" : r\promoLink(''));
}

/**
 * Show raw data for charting.
 * This data is typical requested by cg4.us/charts.php for display in an iframe on the community/graphs page (etc)
 * @param string $args: URL query string with
 *   chart   name of chart to provide data for
 *   ctty    community to provide data for
 *   dt0     first date to include
 *   dt9     last date to include
 * @return: does not return. Prints the data (json-encoded) and exits.
 */
function chartData($args = '') {
  extract(just('chart ctty dt0 dt9', $args));
  showCharts(@$chart, @$ctty, $data);
/**/  if (!@$data) die(t('No such chart data.'));
/**/  echo $data;
  exit();
}

function formGraphs($form, &$sta, $args) {
  $a = w\eLinkAcct($sta, $args, 'any'); // may be called with qid=<communityUid>
  extract(just('qid', $args));

  $title = t('Community Statistics');
  $dft = ($a and @$qid !== '0') ? $a->community : 0; $dft += 0; // + 0 for admin
  $subtext = t('This page helps us keep an eye on our economy so we can manage it effectively together. Choose a chart. <%b>Hover</b> over a graph for the numbers.', '_b', 'class="loud"');
  
  $charts = item(w\chartFrame('growth', TRUE, @$a->community ?: 0)); // src must be first
  if (u\test()) $charts = item(w\showCharts('all', $dft, $chartData));
  
  return cgform(compact(ray('title subtext charts')));
}

function formGraphs_validate($form, &$sta) {
  formTxs_validate($form, $sta);
}

function formGraphs_submit($form, &$sta) {
  formTxs_submit($form, $sta, 'community');
}

function formFooter() {
  global $base_url, $rUrl;
//  $layout = (svar('layout') == 'Mobile') ? 'Desktop' : 'Mobile';
  $year = date('Y');

  //<a href="?layout=$layout">$layout layout</a> |
  global $mya;
  $extraLinks = ($mya and $mya->can(B_MANAGE)) 
  ? t('|<%aDonate>Donate</a> | <%aInvite>Invite Someone</a> | ',
    '_aDonate _aInvite', atag('/community/donate'), atag('/community/invite'))
  : ($mya ? '' : t('|<%aSignin>Sign in</a> |', '_aSignin', atag('/signin')));

  $helpType = $mya ? '' : 'other';
  
  $markup = "<hr><div id=\"footer-menu\">$extraLinks" . t(<<<EOF
|   <%aSite>About %PROJECT</a> |
    <%aAgree>The Agreement</a> |
    <%aSecurity>Privacy & Security</a> |
    <%aHelp>Help</a>
<br>&nbsp;
</div>
copyright &copy; %year <%aSite>%PROJECT<sup>&trade;</sup></a><span id="footer-details">, a nonprofit organization<br/>
%CGF_POSTALADDR<br/>
%CGF_PHONE &nbsp; <%aEmail>%CGF_EMAIL</a><br>
<br>
IP: %ip</span>
<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>&nbsp;
EOF
  , '_aSite _aAgree _aSecurity _aHelp year _aEmail ip', atag(r\promoLink('about.html')), atag('/community/agreement'), atag(r\promoLink('security.html')), atag("/help/$helpType"), $year, u\emailTag(CGF_EMAIL), $_SERVER['REMOTE_ADDR']);
//    <%aCttys>Communities</a> |

  $footerText = item($markup); // unique name for this page
//  $sid = db\get('sid', 'sessions', 'uid=:uid', ['uid' => $mya ? $mya->agentA->id : 0]);
  
  js('parse-query');
  js('misc', 'baseUrl sid life isSafari', BASE_URL, session_id(), ($mya and !isDEV) ? SESSION_LIFE : 0, isSAFARI);
  return compact('footerText');
}

/**
 * Return a display of participating companies (for promo site)
 */
function whosin($ctty = '') {
//  if (!$uid = db\get('uid', 'users', 'name=:ctty', ['ctty' => "$ctty."])) return '';
  $ctty = '!' . $ctty; // promo site omits the leading '!' from the community's QID
  if (!$uid = r\qo($ctty)->id or !$list = r\companies($uid, FALSE)) return '';
  $map = [
    '<tr><td width="200">' => '<div class="row"><div class="cmpy-name">',
    '</td><td style="margin-left:10px;">' => '</div><div class="cmpy-does">',
    '</td></tr>' => '</div></div>',
  ];
  $list = strtr($list, $map); // temporary fudge
/**/  echo $list;
  exit();
}

