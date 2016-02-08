(function() {

  'use strict';

  app.controller('SelectExchangeCtrl', function(ExchangeService, $translate, $state) {
    var self = this,
      moneyTypes = ExchangeService.getMoneyTypes(),
      paymentTypes = ExchangeService.getPaymentTypes();

    this.switchTypes = function() {
      var inMoney = this.moneySwitch.in;
      this.moneySwitch.in = this.moneySwitch.out;
      this.moneySwitch.out = inMoney;
    };

    this.init = function() {
      this.moneySwitch = {
        'in': moneyTypes[0],
        'out': moneyTypes[1]
      };

      this.paymentTypes = paymentTypes;
      this.selectedPayment = this.paymentTypes[0];
      this.paymentAdvice = '';
      this.onPaymentChange();
    };

    this.onPaymentChange = function() {
      $translate('exchange_selected_payment_advice', {
        feeValue: this.selectedPayment.getFee().getTitle(),
        paymentName: this.selectedPayment.getName()
      }).then(function(msg) {
        self.paymentAdvice = msg;
      })
    };

    this.goNextPage = function() {
      ExchangeService.setMoneySwitch(this.moneySwitch);
      $state.go('app.transaction_exchange');
    };


    this.init();


  });

})(app);

