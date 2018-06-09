<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Show general or specific help. For members seeking general help, provide a secure communications channel.
 */
function formHelp($form, &$sta, $what = '', $args = '') {
  include_once R_ROOT . '/rweb/rweb-help.inc';

  $signedIn = ($mya = r\acct()) ? TRUE : FALSE; // no need to use eLinkAcct here, just pass args on to help()
  if ($signedIn and $mya->cAdmin and $what and strpos($what2 = @u\deurlify($what), '&')) {
    extract(just('filename type', $what2));
    $s = file_get_contents($filename);
    list ($zot, $type) = explode('/', "$type/");
    $ext = $type ?: 'bin';
    u\beginDownload("from $mya->id.$ext");
/**/ echo $s;
    exit();
  }
  
  list ($what, $tag) = (strpos($what, '@')) ? explode('@', $what) : array($what == 'other' ? '' : $what, '');
  $title = $what ? ucwords(str_replace('-', ' ', $what)) : t('General Help');
  $stranger = t('Interested in %PROJECT? Visit <%a>%CG_DOMAIN</a> for more information or to request an invitation to participate.', '_a', atag(PROMO_URL));
  $text = item(($signedIn or $what)? helpText($what ?: 'general', $tag, $args) : $stranger);
  if ($signedIn and !$what) {
    
    $message = areaFld(t('Send a message to a %PROJECT Administrator:'), '', required());
    $file = fileFld(t('Secure Attachment:'), t('(optionally) attach a file to your message. The file will be encrypted.'));
    $submit = t('Send');
  }

  if (is_array($args)) $data = $args; // include returned data, if any
  return cgform(compact(ray('title text message file data submit')));
}

function formHelp_validate($form, &$sta) {
  if ($err = $_FILES['files']['error']['file'] and $err != 4) return say('file save error', 'file');
}

function formHelp_submit($form, &$sta) {
  global $mya; 
  $uid = $mya->id;
  $who = "$mya->fullName ($mya->mainQid)";
  extract(just('message file', $sta['input']));
  $time = r\rTime();
  $msg = 'msg to admin';

  foreach ($_FILES['files'] as $k => $v) $file[$k] = $v['file'];
  if ($tmp = $file['tmp_name']) { // got an attachment
    $msg .= '|secure attach';
    $filename = UPLOAD_DIR . "/$mya->id-$time";
    extract(just('name type size', $file));
    $link = '/help/' . u\urlify("type=$type&filename=$filename");
    $_aAttach = atag($link);
    if (!rename($tmp, $filename)) return say('file save error', 'file');
  }  
//  $message = u\crypt($message);
//  db\insert('r_tous', compact('uid', 'time', 'message'));
  $subs = compact(ray('who aAttach name type size'));
  r\tellAdmin("$msg|$message", $subs, @$aAttach ? $uid : NULL); // tell CO only if there is no attachment
  say(t('Your message has been sent. An administrator will respond as soon as possible.'));
}