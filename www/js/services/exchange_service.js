(function(app) {
  'use strict';

  app.service('ExchangeService', function(MoneyType) {

    var self;
    var ExchangeService = function() {
      self = this;
      this.moneyTypes = [];

      this.init();
    };

    ExchangeService.prototype.init = function() {
      this.createMoneyTypes_();
    };

    ExchangeService.prototype.createMoneyTypes_ = function() {
      this.moneyTypes = _.map(window.moneyTypesDefinition, MoneyType.parseMoneyType);
    };

    ExchangeService.prototype.getMoneyTypes = function() {
      return _.map(this.moneyTypes, _.clone);
    };


    return new ExchangeService();
  });

})(app);
