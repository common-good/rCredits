(function() {

  'use strict';

  app.controller('ExchangeCtrl', function($scope, ExchangeService, $translate) {
    var self = this;

    $scope.amount = 0;

    this.init = function() {
      $translate('exchange_includes_fee').then(function(msg) {
        self.paymentFeeTitle = msg;
      });

      this.moneySwitch = ExchangeService.getMoneySwitch();
    };

    this.doExchange = function() {
    };

    this.init();
  });

})(app);

