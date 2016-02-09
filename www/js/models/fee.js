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
          return Fee.format(0);
        }
        return Fee.format(amountWithFee);
      }
    });

    Fee.format = function(amount) {
      return parseFloat($filter('currency')(amount, '', 2));
    };

    Fee.parseFee = function(jsonFee) {
      return _.extendOwn(new Fee(), jsonFee);
    };


    return Fee;
  });
})(window, app);
