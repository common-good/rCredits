jQuery(".checkbox > .description, .form-checkboxes .checkbox > label").each(function(index) {
  var input=jQuery(this).parent().children("input:first");
  if(input.attr("checked")) jQuery(this).addClass("opt-on");
  jQuery(this).click(function() {
    jQuery(this).toggleClass("opt-on");
    input.click();
    input.attr("checked", jQuery(this).hasClass("opt-on"));
  });
});

jQuery(".radio > label").each(function(index) {
  var input=jQuery(this).parent().children("input:first");
  if(input.attr("checked")) jQuery(this).addClass("opt-on");
  jQuery(this).click(function() {
  if(!jQuery(this).hasClass("opt-on")) { // more reliable than !input.attr("checked")
	  input.click();
      jQuery(this).parent().parent().find("label").removeClass("opt-on");
      jQuery(this).addClass("opt-on");
      input.attr("checked", true);
    } else input.click();
  });
});
