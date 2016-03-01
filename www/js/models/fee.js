(function(window, app) {

  var _ = window._;
  app.service('Fee', function($filter) {

    var Fee = Class.create({

      title: '',
      value: 0,
      unit: '',

      initialize: function($super) {
      },

      getTitle: function() {
        return this.title;
      },

      apply: function(amount) {
        var amountWithFee = parseFloat(amount);

        if (this.unit === 'cash') {
          amountWithFee = amount - this.value;
        } else if (this.unit === 'percent') {
          amountWithFee = amount - (amount * (this.value / 100));
        }

        if (amountWithFee <= 0) {
          return 0
        }
        return amountWithFee;
      }
    });

    Fee.parseFee = function(jsonFee) {
      return _.extendOwn(new Fee(), jsonFee);
    };


    return Fee;
  });
})(window, app);
