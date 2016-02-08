(function() {

  'use strict';

  app.controller('ExchangeCtrl', function(ExchangeService) {
    var self = this, moneyTypes = ExchangeService.getMoneyTypes();

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
    };

    this.goNextPage = function() {

    };


    this.init();


  });

})(app);

