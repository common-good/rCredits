function toggle(field) {
  field = "#" + field;
  jQuery(field + "-YES, " + field + "-NO").toggle();
  jQuery(field).val(jQuery(field + "-YES").is(":visible"));
}
function toggleMain(me) {
  var was=jQuery(me).is(":visible");
  jQuery("#main, #help, #menu").hide();
  jQuery(was ? "#main" : me).show();
}
jQuery("#which, #help").addClass("popup");
jQuery("#help-close, #which-close").addClass("popup-close");
jQuery("#help-link").click(function() {jQuery("fieldset#help").show();});
jQuery("#edit-acct-showhelp").click(function() {toggleMain("#help");});
jQuery("#edit-acct-showmenu").click(function() {toggleMain("#menu");});
jQuery(".popup-close").click(function() {jQuery(this).parents("fieldset").hide();});
jQuery("#which-cancel").click(function() {jQuery("fieldset#which").hide();});
jQuery("#idcard").click(function() {window.open("Advance-ID-Card", "tempcard","height=250,width=600,left=100,location=0,menubar=0,resizable=0,scrollbars=0,status=0,toolbar=0");}); // unused