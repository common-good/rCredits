/**
 * @file
 * javascript for the bottom of every page
 */

var vs = parseUrlQuery($('#script-misc').attr('src').replace(/^[^\?]+\??/,''));
//alert($('#script-misc').attr('src').replace(/^[^\?]+\??/,''));
var baseUrl = vs['baseUrl'];
var isSafari = vs['isSafari'];
var signoutUrl = baseUrl + '/signout/timedout';
var ajaxUrl = baseUrl + '/ajax';
var ajaxSid = vs['sid'];
var sessionLife = 1000 * vs['life']; // convert to seconds
var signoutWarningAdvance = Math.min(sessionLife / 2, 5 * 60 * 1000); // give the user a few minutes to refresh
if (ajaxSid) var sTimeout = vs['life'] != '0' ? sessionTimeout() : 0; // Warn the user before automatically signing out.

jQuery("#which, #help").addClass("popup");
jQuery('button[type="submit"]').click(function() {
  this.form.opid.value = this.id;
//  $('<input type="hidden" name="opid" />').appendTo(this.form).val(this.form.id);
});

$('[data-toggle="popover"][data-trigger="hover"]').click(function () {$(this).popover('toggle');});
$('.submenu .popmenu a').click(function () {$(this).find('.glyphicon').css('color', 'darkblue');});
$('.submenu a[data-trigger="manual"]').click(function () {
  if (isSafari) location.href = baseUrl + '/' + $(this).parents('.submenu').attr('id').replace('menu-', ''); // work around Safari bug (doesn't show menus on hover)
  $(this).popover('toggle');
  $('.submenu a').not($(this)).popover('hide');
});

var page=0;
var more=false;
var indexZ = 2;
jQuery("#index a").mouseover(function() {
  var detail = jQuery("#" + this.id + "-detail");
  indexZ++;
  detail.css("zIndex", indexZ); // hiding the others fails here (as does detail.zIndex(indexZ))
  detail.show();
});
jQuery(".index-detail").click(function() {jQuery("#edit-acct-index, .index-detail").hide();});
jQuery(".noEdit").prev().attr("disabled", 1);

jQuery('[data-toggle="popover"][data-trigger="hover"]').popover(); 
jQuery('[data-toggle="popover"][data-trigger="click"]').popover(); 

var mobile = jQuery('.navbar-toggle').is(':visible');
jQuery('.submenu [data-toggle="popover"]').each(function(index) {
  jQuery(this).popover({
    html: true,
    content: function() {return jQuery(this).prev().html();},
    placement: (mobile ? 'left' : 'bottom')
  });
});
jQuery('#main .list-group-item.ladda-button').attr('data-spinner-color', '#191970').click(function() {
  jQuery(this).find('.glyphicon').css('color', 'white');
});
if (Ladda != null) Ladda.bind('.ladda-button');
if (!mobile) jQuery('.navbar-nav > li > a').hover(function() {
  jQuery(this).popover('show');
  if (Ladda != null) Ladda.bind('.ladda-button'); // these buttons are not available to Ladda until now
  // ('#' + jQuery(this).parent().parent().attr('id') + ' > li > a') doesn't work
  jQuery('.submenu > a').not(jQuery(this)).popover('hide');
});
if (!mobile) jQuery('form div').hover(function() {jQuery('* [data-toggle="popover"]').popover('hide');});

$('.test-next').click(function () {
  $('#testError' + $(this).attr('index'))[0].scrollIntoView(true); window.scrollBy(0, -100);
});

function showMore(pgFactor) {
  page = Math.floor(page * pgFactor); 
  if (more) {
    $.alert('Click &#9654; (far right) to see the next page', 'Tip');
  } else {
    more = true;
    if ($('.PAGE-' + (page + 1)).length) {
      $('.showMore a').css('color','silver'); 
    } else $('.showMore').css('visibility','hidden'); 
  }
  showPage(0);
}

function showPage(add) {
  page += add;
  var pghd = more ? '.PAGE-' : '.page-'; 
  $('.prevPage').css('visibility', page < 1 ? 'hidden' : 'visible'); 
  $('.nextPage').css('visibility', $(pghd + (page + 1)).length ? 'visible' : 'hidden'); 
  $('.txRow').hide(); 
  $('.txRow.head, ' + pghd + page).show();
}

function deleteCookie(name) {
  document.cookie = name + '=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

function toggleFields(fields, show) {
  fields.split(' ').forEach(function(e) {$('.form-item-' + e).toggle(show); });
}

function toggle(field) {
  field = "#" + field;
  jQuery(field + "-YES, " + field + "-NO").toggle().toggleClass("visible invisible");
  jQuery(field).val(jQuery(field + "-YES").is(":visible"));
}

function commafy(n) {return isNaN(n) ? '0.00' : n.toLocaleString();}

/**
 * post or get data to/from the server
 * @param string op: what to get
 * @param object data: parameters for the get
 * @param function success(jsonObject): what to do upon success (do nothing on failure)
 */
function get(op, data, success) {
  data = {op:op, sid:ajaxSid, data:JSON.stringify(data)}; // sub-objects must be stringified
  jQuery.get(ajaxUrl, data, success);
}

function post(op, data, success) {
  data = {op:op, sid:ajaxSid, data:JSON.stringify(data)};
  jQuery.post(ajaxUrl, data, success); // jQuery not $, because drupal.js screws it up on formVerify
}

function yesno(question, yes, no) {
  if (typeof no === 'undefined') no = (function() {});
  $.confirm({title: 'Yes or No', text: question, confirm: yes, cancel: no, confirmButton: 'Yes', cancelButton: 'No'});
}

function which(question, choices, choose, cancel) {
  $("#which").modal("show");
}

var yesSubmit = false; // set true when user confirms submission (or makes a choice)
var jForm; // jquery form object

function noSubmit() {
  $('.ladda-button').removeAttr('disabled').removeAttr('data-loading');
  $('#messages').hide();
}
function yesSubmit() {}

function who(form, fid, question, amount, allowNonmember, coOnly) {
  jForm = $(form);
  var who = $(fid).val();
  if (yesSubmit) return true;
  get('who', {who:who, question:question, amount:amount, coOnly:coOnly}, function(j) {
    if (j.ok) {
      if (j.who) {
        $(fid).val(j.who);
        yesno(j.confirm, function() {
          yesSubmit = true; jForm.submit();
        }, noSubmit);
      } else which(jForm, fid, j.title, j.which);
    } else if (allowNonmember && who.includes('@')) {
      yesno('The email address (' + who + ') is for a non-member (or for a member with a non-public email address). Do you want to send them an invoice anyway, with an invitation to join?', function() {
        yesSubmit = true; jForm.submit();
      }, noSubmit);
    } else {
      noSubmit(); $.alert(j.message);
    }
  });
  return false;
}

function which(jForm, fid, title, body) {
  $('<div id="which">' + body + '</div>').dialog({
    title: title,
    modal: true,
    closeText: '&times;', // fails
    dialogClass: 'which'
  });
  $('.ui-dialog-titlebar-close').html('&times;');
  $('.ui-dialog-titlebar-close').click(function() {noSubmit();});
  $('#which option').click(function() {
    yesSubmit = true;
    $(fid).val($(this).val());
    jForm.submit();
  });
}

var signoutWarning = 'You still there? (otherwise we\'ll sign you out, to protect your account)';

function sessionTimeout() {
  return setTimeout(function() {
    $.confirm({
      title:'Long Time No Click',
      text:signoutWarning,
      confirmButton:'Yes',
      cancelButtonClass:'hidden',
      confirm:function() {
        clearTimeout(sTimeout); // don't sign out
        $.get(ajaxUrl, {op:'refresh'}); // reset PHP garbage collection timer
        sTimeout = sessionTimeout(); // restart warning timer
      }
    });
    sTimeout = setTimeout(function() {location.href = signoutUrl;}, Math.max(1, signoutWarningAdvance - 10));
  }, sessionLife - signoutWarningAdvance);
}

function SelectText(element) { // from http://stackoverflow.com/questions/985272
  var doc = document;
  var text = doc.getElementById(element);
  var range, selection;
  if (doc.body.createTextRange) {
    range = doc.body.createTextRange();
    range.moveToElementText(text);
    range.select();
  } else if (window.getSelection) {
    selection = window.getSelection();        
    range = doc.createRange();
    range.selectNodeContents(text);
    selection.removeAllRanges();
    selection.addRange(range);
  }
}
/*
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
})();
*/
