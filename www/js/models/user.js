(function(window) {

  var User = Class.create({

    initialize: function(name) {
      this.name = name;
    },

    default: '',
  });

  window.User = User;

}) (window);
