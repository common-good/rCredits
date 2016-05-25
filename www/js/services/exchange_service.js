/* global _, app */
(function (app) {
	'use strict';
	app.service('ExchangeService', function (Currency, PaymentType) {
		var self;
		var ExchangeService = function () {
			self = this;
			this.Currencies = [];
			this.paymentTypes = [];
			this.exchange = null;
			this.init();
		};
		ExchangeService.prototype.init = function () {
			this.createCurrencies_();
			this.createPaymentTypes_();
		};
		ExchangeService.prototype.createCurrencies_ = function () {
			this.Currencies = _.map(window.CurrenciesDefinitions, Currency.parseCurrency);
		};
		ExchangeService.prototype.createPaymentTypes_ = function () {
			this.paymentTypes = _.map(window.paymentTypesDefinitions, PaymentType.parsePaymentType);
		};
		ExchangeService.prototype.getCurrencies = function () {
			return _.map(this.Currencies, _.clone);
		};
		ExchangeService.prototype.getPaymentTypes = function () {
			return _.map(this.paymentTypes, _.clone);
		};
		ExchangeService.prototype.removeExchange = function () {
			this.exchange = null;
		};
		ExchangeService.prototype.setExchange = function (exchange) {
			this.exchange = exchange;
		};
		ExchangeService.prototype.getExchange = function () {
			return this.exchange;
		};
		return new ExchangeService();
	});
})(app);
