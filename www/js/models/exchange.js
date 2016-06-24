/* global app */
(function (window, app) {
	var _ = window._;
	app.service('Exchange', function () {
		var Exchange = Class.create({
			currencyFrom: null,
			currencyTo: null,
			paymentMethod: null,
			initialize: function ($super) {
			},
			setCurrencyFrom: function (currency) {
				this.currencyFrom = currency;
			},
			setCurrencyTo: function (currency) {
				this.currencyTo = currency;
			},
			setPaymentMethod: function (paymentMethod) {
				this.paymentMethod = paymentMethod;
			},
			getCurrencyFrom: function () {
				return this.currencyFrom;
			},
			getCurrencyTo: function () {
				return this.currencyTo;
			},
			getPaymentMethod: function () {
				return this.paymentMethod;
			}
		});
		return Exchange;
	});
})(window, app);