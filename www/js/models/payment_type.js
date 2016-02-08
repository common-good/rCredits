(function(window, app) {

  var _ = window._;
  app.service('PaymentType', function(Fee) {

    var PaymentType = Class.create({

      name: '',
      fee: null,

      initialize: function($super) {
      },

      getName: function() {
        return this.name;
      },

      getFee: function() {
        return this.fee;
      }

    });

    PaymentType.parsePaymentType = function(jsonPayment) {
      var paymentType = _.extendOwn(new PaymentType(), jsonPayment);
      paymentType.fee = Fee.parseFee(jsonPayment.fee);
      return paymentType;
    };

    return PaymentType;
  });
})(window, app);
