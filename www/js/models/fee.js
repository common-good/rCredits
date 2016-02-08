(function(window, app) {

  var _ = window._;
  app.service('Fee', function() {

    var Fee = Class.create({

      title: '',
      value: 0,
      unit: '',

      initialize: function($super) {
      },

      getName: function() {
        return this.title;
      }

    });

    Fee.parseFee = function(jsonFee) {
      return _.extendOwn(new Fee(), jsonFee);
    };


    return Fee;
  });
})(window, app);
