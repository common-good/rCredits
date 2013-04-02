jQuery(".form-type-checkbox > .description, .form-checkboxes .form-type-checkbox > label").each(function(index) {
  var input=jQuery(this).parent().children("input:first");
  if(input.attr("checked")) jQuery(this).addClass("opt-on");
  input.val(input.attr("checked") ? 1 : 0);
  jQuery(this).click(function() {
    jQuery(this).toggleClass("opt-on");
    input.attr("checked", jQuery(this).hasClass("opt-on"));
    input.val(input.attr("checked") ? 1 : 0);
  });
});

jQuery(".form-type-radio > label").each(function(index) {
  var input=jQuery(this).parent().children("input:first");
  if(input.attr("checked")) jQuery(this).addClass("opt-on");
  jQuery(this).click(function() {
    if(!input.attr("checked")) {
      jQuery(this).parent().parent().find("label").removeClass("opt-on");
      jQuery(this).addClass("opt-on");
      input.attr("checked", true);
    }
  });
});
