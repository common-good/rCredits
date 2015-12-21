(function(window) {

  var User = Class.create({

    initialize: function(name) {
      this.name = name;
      this.can = 0;
      this.company = '';
    },

    default: '',
  });

  window.User = User;

}) (window);
