var vs = parseUrlQuery($('#script-crop-setup').attr('src').replace(/^[^\?]+\??/,''));

var uniq = (new Date()).getTime(); // unique value (so default picture does not repeat a real one)
$('#photoUp').css('background-image', 'url(' + baseUrl + '/account-photo?t=' + uniq + ')'); 

var err = false;
var croppicOptions = {
//      uploadUrl:'$base_url/settings/photo',
  cropUrl:baseUrl + '/settings/photo/upload',
  loadPicture:vs['recrop'],
  customUploadButtonId:'choosePhoto',
  cropData:{
    'uid':vs['uid'],
    'sid':ajaxSid
  },
  modal:false,
  processInline:true,
  loaderHtml:'<div class="loader bubblingG"><span id="bubblingG_1"></span><span id="bubblingG_2"></span><span id="bubblingG_3"></span></div> ',
  onBeforeImgUpload: function() {
    if (vs['recrop']) { $('#choosePhotoWrap').hide(); return; } // else recrop fails (space after { is required)
    var photoUp = jQuery('#photoUp_imgUploadField').get(0);
    if (photoUp.files[0].size > vs['maxSize'] * 1024 * 1024) {
      croppic.err = 'That photo file is too big. The maximum file size is ' + vs['maxSize'] + 'MB. Reduce the file size and try again (or choose a smaller file).';
    }
  },
  onAfterImgUpload: function() { console.log('onAfterImgUpload') }, 
//    onImgDrag: function(){ console.log('onImgDrag') },
//    onImgZoom: function(){ console.log('onImgZoom') },
  onBeforeImgCrop: function() { console.log('onBeforeImgCrop'); },
  onAfterImgCrop:function(){
    jQuery('#photoUp').hide();
    window.location.replace(err ? baseUrl + '/settings/photo' : baseUrl + '/settings/photo/ok=1/' + Math.random());
  },
//    onReset:function(){ console.log('onReset') },
  onError:function(errormessage){
    jQuery.alert(errormessage, 'Error');
    err = true;
  }
}
var croppic = new Croppic('photoUp', croppicOptions);
jQuery('#choosePhoto').click(function () {jQuery('#emailPhoto').hide();});