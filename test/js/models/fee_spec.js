describe('Fee', function () {
	'use strict';
	beforeEach(module('rcredits'));
	beforeEach(function () {
		module(function ($exceptionHandlerProvider) {
			$exceptionHandlerProvider.mode('log');
		});
	});
	var Fee, FeeDef = {
		cash: {
			title: 'zero',
			value: 5,
			unit: 'cash'
		},
		percent: {
			percent: '3%',
			value: 3,
			unit: 'percent'
		}
	};
	beforeEach(inject(function (_Fee_) {
		Fee = _Fee_;
	}));
	describe('Should apply fee', function () {
		var fee, amount;
		beforeEach(function () {
			amount = 100;
		});
		it('Apply fee of Cash', function () {
			fee = Fee.parseFee(FeeDef.cash);
			expect(fee.apply(amount)).toBe(95);
		});
		it('Apply fee of Percent', function () {
			fee = Fee.parseFee(FeeDef.percent);
			expect(fee.apply(amount)).toBe(97);
		});
		it('Apply fee for Zero amount', function () {
			fee = Fee.parseFee(FeeDef.cash);
			expect(fee.apply(0)).toBe(0);
		});
		it('Apply fee for Negative amount', function () {
			fee = Fee.parseFee(FeeDef.cash);
			expect(fee.apply(-1)).toBe(0);
		});
	});
});
