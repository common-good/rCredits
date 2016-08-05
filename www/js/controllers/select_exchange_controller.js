/* global app */
(function () {
	'use strict';
	app.controller('SelectExchangeCtrl', function ($scope, ExchangeService, $translate, $state, Exchange) {
		var self = this,
			currencies = ExchangeService.getCurrencies(),
			paymentTypes = ExchangeService.getPaymentTypes();
		$scope.hide_payment = true;
		$scope.currency = 'USD';
		console.log(true % 2);
		this.switchTypes = function (selected) {
			var inMoney = this.exchange.getCurrencyFrom();
			this.exchange.setCurrencyFrom(this.exchange.getCurrencyTo());
			this.exchange.setCurrencyTo(inMoney);
//			document.getElementsByClassName('currencySelect')[selected-1].style.color = 'black';
//			document.getElementsByClassName('currencySelect')[selected % 2].style.color = 'lightgray';
			console.log(document.getElementsByClassName('currencySelect')[selected-1].style.color );
			$scope.hide_payment = !$scope.hide_payment;
//			return type.valueOf();
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

