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

var indexZ = 2;
jQuery("#index a").mouseover(function() {
  var detail = jQuery("#" + this.id + "-detail");
  indexZ++;
  detail.css("zIndex", indexZ); // hiding the others fails here (as does detail.zIndex(indexZ))
  detail.show();
});
jQuery(".index-detail").click(function() {jQuery("#edit-acct-index, .index-detail").hide();});
jQuery(".noEdit").prev().attr("disabled", 1);

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

var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-30262912-1']);
_gaq.push(['_trackPageview']);

(function() {
  var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
  ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
})();
