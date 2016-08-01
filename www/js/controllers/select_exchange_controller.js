/* global app */
(function () {
	'use strict';
	app.controller('SelectExchangeCtrl', function ($scope, ExchangeService, $translate, $state, Exchange) {
		var self = this,
			currencies = ExchangeService.getCurrencies(),
			paymentTypes = ExchangeService.getPaymentTypes();
		$scope.hide_payment = true;
		console.log(currencies);
		this.switchTypes = function (type) {
//			var inMoney = this.exchange.getCurrencyFrom();
			this.exchange.setCurrencyFrom(currencies[!type]);
			this.exchange.setCurrencyTo(currencies[type]);
			console.log(this.exchange.setCurrencyFrom, this.exchange.getCurrencyTo());
			$scope.hide_payment = !$scope.hide_payment;
		};
		this.init = function () {
			this.exchange = new Exchange();
			this.exchange.setCurrencyFrom(currencies[0]);
			this.exchange.setCurrencyTo(currencies[1]);
			this.paymentTypes = paymentTypes;
			this.selectedPayment = this.paymentTypes[0];
			this.paymentAdvice = '';
			this.onPaymentChange();
		};
		this.onPaymentChange = function () {
			this.exchange.setPaymentMethod(this.selectedPayment);
			$translate('exchange_selected_payment_advice', {
				feeValue: this.selectedPayment.getFee().getTitle(),
				paymentName: this.selectedPayment.getName()
			}).then(function (msg) {
				self.paymentAdvice = msg;
			});
		};
		this.goNextPage = function () {
			ExchangeService.setExchange(this.exchange);
			$state.go('app.transaction_exchange');
		};
		this.init();
	});
})(app);

