(function(app) {
  'use strict';

  app.service('ExchangeService', function(Currency, PaymentType) {

    var self;
    var ExchangeService = function() {
      self = this;
      this.Currencies = [];
      this.paymentTypes = [];

      this.init();
    };

    ExchangeService.prototype.init = function() {
      this.createCurrencies_();
      this.createPaymentTypes_();
    };

    ExchangeService.prototype.createCurrencies_ = function() {
      this.Currencies = _.map(window.CurrenciesDefinitions, Currency.parseCurrency);
    };

    ExchangeService.prototype.createPaymentTypes_ = function() {
      this.paymentTypes = _.map(window.paymentTypesDefinitions, PaymentType.parsePaymentType);
    };

    ExchangeService.prototype.getCurrencies = function() {
      return _.map(this.Currencies, _.clone);
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
