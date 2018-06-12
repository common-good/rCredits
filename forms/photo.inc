<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;


/**
 * Upload a photo.
 * @later: maybe drag&drop (see http://hayageek.com/drag-and-drop-file-upload-jquery/)
 */
function formPhoto($form, &$sta, $arg = '') {
  global $base_url, $rUrl;
  global $mya;
  $chooserStyle = 'btn-md btn-primary';
	extract(just('ok err recrop', $arg));
//	$recrop = @$recrop ? plain($mya->tempPhotoFilename) : '';
	$recrop = @$recrop ? "$base_url/account-photo/orig=1/" . time() : '';

  $body = t('I am attaching a photo of me for my %PROJECT card. &nbsp; - %fullName', 'fullName', $mya->fullName);
  $emailIt = t('<%a>Email one to us</a>', '_a', atag('mailto:' . r\regionField('email') . '?subject=photo&body=' . $body));
	$recropIt = $mya->cAdmin ? w\btn('/settings/photo/recrop=1', t('Recrop')) : '';

  if (@$ok) {
    say('got photo');
    $chooserStyle = 'btn-xs btn-secondary';
// No! wait for user to be happy with the photo   $mya->stepDone('photo');
    extract(w\setupFoot()); // get nextStep and progress fields
    $buttonName = $mya->stepsRemaining() > 1 ? t('Next') : t('Finish');
    $allSet = $mya->member ? t('you\'re all set') : t('click "%buttonName" above', compact('buttonName'));
    $desc = t('If you like this photo, %allSet. Otherwise:', compact('allSet'));
  } else {
//    if ($err) say($err, 'err');
    if ($mya->hasPhoto) extract(w\setupFoot()); // be sure new member has a chance to continue
    $desc = $mya->co
    ? t('<p>1. Choose a picture to represent your company in %PROJECT promotions.</p>')
    : t('<p>1. Choose a recent color picture of yourself for your photo-ID %PROJECT card. Your photo will be encrypted and will not be visible to other members (unless you show it to them).<%b>Requirements</b>:
    <ul>
    <li>It should be a color photo of you and only you (you can crop other people out).</li>
    <li>It must show clearly your full face (no sunglasses, eyes open).</li>
    <li>Your head should fill most of the frame (no shoulders needed).</li></ul>', '_b', 'b class="loud"');    
    $desc .= t('<p>2. Play with the buttons to adjust the position, size, and rotation, then click the <%img>button to upload the image. Please feel free to %emailIt, if that is easier for you.', '_b _emailIt _img', 'b class="crop-button"', $emailIt, "img src=\"$rUrl/images/icons/crop.png\" height=\"20\" /");
  }
  
  $title = t('Photo ID Picture');

  $rnd = u\code();
  
  if ($mya->hasPhoto) {
/*    $oldPhotoFilename = BASE_URL . $mya->photoFilename();
    //$oldPhoto = 'data:image/jpeg;base64,' . base64_encode(file_get_contents($oldPhotoFilename));
    $oldPhoto = $mya->photo ? 'data:image/jpeg;base64,' . base64_encode($mya->photo) : "$oldPhotoFilename?$rnd";
    list ($background, $choose) = ["background-image:url($oldPhoto); background-size:cover;", t('Choose a New Photo')];
    */
    $choose = t('Choose a New Photo');
    if (!$mya->co and $mya->ok) $desc .= '<p>' . t('triggers new card', ['field' => t('photo')]) . '</p>';
    if ($mya->superAdmin) $del = submit(t('Delete'));
  } else $choose = t('Choose a Photo File');
//  } else list ($background, $choose) = ['', t('Choose a Photo File')];
// <style>#photoUp, #photoUp.co { $background }</style>

  $class = $mya->co ? ' class="co"' : '';
  w\svar('userAgent', @$_SERVER['HTTP_USER_AGENT']); // for debugging spurious photo rotations

  $desc = item($desc);
  $newPhoto = item(<<<EOF
<div id="choosePhotoWrap"><div class="btn $chooserStyle btn-success" id="choosePhoto">$choose</div> <div id="emailPhoto" class="btn btn-warning btn-xs">$emailIt</div> $recropIt</div>
<div class="cropHeaderWrapper">
  <div id="photoUp"$class></div>
</div>

EOF
  );
  
//  $sid = db\get('sid', 'sessions', 'uid=:uid', ['uid' => $mya->agentA->id]);
//  $sid = session_id();
//  $failure = urlencode(t('That is not a usable image file. Try again.'));
//  $maxSize = R_MAX_UPLOAD_SIZE * 1024 * 1024;
//  $maxSizeDpy = R_MAX_UPLOAD_SIZE . 'MB';

  js('croppic');
  js('crop-setup', 'recrop uid maxSize', $recrop, $mya->id, R_MAX_UPLOAD_SIZE);
  css('x/croppic.css');

  $form = compact(ray('title nextStep progress desc newPhoto del'));
  return cgform($form);
}

function formPhoto_submit($form, &$sta) {
  global $base_url;

  global $mya;
  
  if (op($sta) == 'del') {
    u\EXPECT($mya->cAdmin and $mya->hasPhoto, 'del ghost photo');
/*    $pictureFilename = DRUPAL_ROOT . $mya->photoFilename(TRUE);
    $newFilename = str_replace('.jpg', '.X', $pictureFilename);
    if (file_exists($pictureFilename)) {
      rename($pictureFilename, $newFilename);
    } else imagejpeg(imagecreatefromstring($mya->photo), $newFilename, 100);
    */
    $mya->stepDone('photo', FALSE);
    $mya->update('photo', '');
    $mya->setBit(B_MEMBER, FALSE);
    $mya->setBit(B_OK, FALSE);
    say(t('photo removed, step undone, person set to non-member'));
    return w\go('/settings/photo');
  } else return w\goNextStep('photo');
}