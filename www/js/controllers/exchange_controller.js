(function() {

  'use strict';

  app.controller('ExchangeCtrl', function($scope, ExchangeService, $translate) {
    var self = this;

    $scope.amount = 0;

    this.init = function() {
      this.exchange = ExchangeService.getExchange();
      this.paymentMethod = this.exchange.getPaymentMethod();

      $translate('exchange_includes_fee', {
        feeValue: this.paymentMethod.getFee().getTitle(),
        paymentName: this.paymentMethod.getName()
      }).then(function(msg) {
        self.paymentFeeTitle = msg;
      });
    };

    this.calculateOutAmount = function() {
      return this.paymentMethod.applyFeeTo($scope.amount);
    };

    this.doExchange = function() {
    };

    this.init();
  });

})(app);

