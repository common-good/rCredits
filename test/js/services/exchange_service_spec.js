describe('Exchange Service', function () {
	'use strict';
	beforeEach(module('rcredits'));
	beforeEach(function () {
		module(function ($exceptionHandlerProvider) {
			$exceptionHandlerProvider.mode('log');
		});
	});
	var rootScope, httpBackend, exchangeService;
	beforeEach(inject(function ($rootScope, $httpBackend, _ExchangeService_) {
		httpBackend = $httpBackend;
		rootScope = $rootScope;
		exchangeService = _ExchangeService_;
		$httpBackend.whenGET(/templates\/*/).respond(function (method, url, data, headers) {
			return [200, '<div></div>'];
		});
		$httpBackend.whenGET(/js\/languages\/definitions\//).respond(function (method, url, data, headers) {
			return [200, {}];
		});
	}));
	describe('Create Money Types', function () {
		it('Should create Money Types when instantiated', function () {
			expect(exchangeService.getCurrencies()).not.toBe(undefined);
		});
		it('Should return cloned Money Instances', function () {
			expect(exchangeService.getCurrencies()).not.toBe(exchangeService.currencies);
		});
	});
	describe('Create Payment Types', function () {
		it('Should create Payment Types when instantiated', function () {
			expect(exchangeService.getPaymentTypes()).not.toBe(undefined);
		});
		it('Should return cloned Payment Instances', function () {
			expect(exchangeService.getPaymentTypes()).not.toBe(exchangeService.paymentTypes);
		});
	});
});



