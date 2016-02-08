(function(app) {
  'use strict';

  app.service('ExchangeService', function(MoneyType, PaymentType) {

    var self;
    var ExchangeService = function() {
      self = this;
      this.moneyTypes = [];
      this.paymentTypes = [];

      this.init();
    };

    ExchangeService.prototype.init = function() {
      this.createMoneyTypes_();
      this.createPaymentTypes_();
    };

    ExchangeService.prototype.createMoneyTypes_ = function() {
      this.moneyTypes = _.map(window.moneyTypesDefinitions, MoneyType.parseMoneyType);
    };

    ExchangeService.prototype.createPaymentTypes_ = function() {
      this.paymentTypes = _.map(window.paymentTypesDefinitions, PaymentType.parsePaymentType);
    };

    ExchangeService.prototype.getMoneyTypes = function() {
      return _.map(this.moneyTypes, _.clone);
    };

    ExchangeService.prototype.getPaymentTypes = function() {
      return _.map(this.paymentTypes, _.clone);
    };


    ExchangeService.prototype.removeMoneySwitch = function() {
      this.moneySwitch = null;
    };

    ExchangeService.prototype.setMoneySwitch = function(moneySwitch) {
      this.moneySwitch = moneySwitch;
    };

    ExchangeService.prototype.getMoneySwitch = function() {
      return this.moneySwitch;
    };

    return new ExchangeService();
  });

})(app);
