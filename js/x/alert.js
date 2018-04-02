/**
 * jQuery alert extension (thanks to Anders Abel coding.abel.nu)
 * Usage: $.alert("message", "title");
 */
$.extend({ alert: function (message, title) {
  $("<div></div>").dialog( {
    buttons: { "Ok": function () { $(this).dialog("close"); } },
    close: function (event, ui) { $(this).remove(); },
    resizable: false,
    title: title,
    modal: true
  }).html(message);
}
});