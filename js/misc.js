function toggle(field) {
  field = "#" + field;
  jQuery(field + "-YES, " + field + "-NO").toggle().toggleClass("visible invisible");
  jQuery(field).val(jQuery(field + "-YES").is(":visible"));
}

function commafy(n) {
  if(isNaN(n)) return '0.00';
  n=parseFloat(n).toFixed(2).split(".");
  n[0] = n[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
  return n.join(".");
}

//$(function() { // the DOM is ready enough (this being in the footer)
jQuery("#which, #help").addClass("popup");

var yesSubmit = false; // set true when user confirms submission (or makes a choice)
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
Ladda.bind('.ladda-button');
if (!mobile) jQuery('.navbar-nav > li > a').hover(function() {
  jQuery(this).popover('show');
  Ladda.bind('.ladda-button'); // these buttons are not available to Ladda until now
  // ('#' + jQuery(this).parent().parent().attr('id') + ' > li > a') doesn't work
  jQuery('.submenu > a').not(jQuery(this)).popover('hide');
});
if (!mobile) jQuery('form div').hover(function() {jQuery('* [data-toggle="popover"]').popover('hide');});

/**
 * post or get data to/from the server
 * @param string op: what to get
 * @param object data: parameters for the get
 * @param function success(jsonObject): what to do upon success (do nothing on failure)
 */
function get(op, data, success) {
  data = {op:op, sid:ajaxSid, data:data};
  $.get(ajaxUrl, data, success);
}

function post(op, data, success) {
  data = {op:op, sid:ajaxSid, data:data};
  $.post(ajaxUrl, data, success);
}

function yesno(question, yes, no) {
  if (typeof no === 'undefined') no = (function() {});
  $.confirm({title: 'Yes or No', text: question, confirm: yes, cancel: no, confirmButton: 'Yes', cancelButton: 'No'});
}

function which(question, choices, choose, cancel) {
  $("#which").modal("show");
}

function noSubmit(button) {
  $('#edit-submit').removeAttr('disabled').removeAttr('data-loading');
  $('#messages').hide();
}

function who(form, id) {
  var jForm = $(form);
  if (yesSubmit) return true;
  get('who', {who:$(id).val()}, function(j) {
    if (j.ok) {
      if (j.who) {
        $(id).val(j.who);
        yesno(j.confirm, function() {yesSubmit = true; jForm.submit();}, noSubmit);
      } else which(jForm, id, j.title, j.which);
    } else $.alert(j.message);
  });
  return false;
}

function which(jForm, id, title, body) {
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
    $(id).val($(this).val());
    jForm.submit();
  });
}

var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
})();
