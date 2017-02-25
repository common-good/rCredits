/* global _$compile_, _$rootScope_, expect, browser */
//
// Feature: Parse QR Code
//   AS a cashier
//   I WANT the customer's QR code to be interpreted correctly
//   SO we know who we're dealing with.
var R2_steps = require('../r2.js');
describe('r2% -- FEATURE_NAME', function () {
	'use strict';
	var steps = new R2_steps();
	var eachStep;
	beforeEach(function () { // Setup
		steps.extraSetup();
//		steps.extraSetup();
//		steps.extraSetup());
	});
	it('Scenario: We scan a valid old personal card.', function () {
		steps.testOnly = 0;
		if(expect(steps.weScanQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw')).toBe(true));
		console.log(steps.weScanQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw'));
		steps.testOnly = 1;
		if(expect(steps.accountIsPersonal()).toBe(true));
		console.log(steps.accountIsPersonal());
		steps.testOnly = 1;
		if(expect(steps.accountIDIs('NEWABB')).toBe(true));
		console.log(steps.accountIDIs('NEWABB'));
		steps.testOnly = 1;
		if(expect(steps.securityCodeIs('ZzhWMCq0zcBowqw')).toBe(true));
//		console.log(steps.securityCodeIs('ZzhWMCq0zcBowqw'));
	});

	it('Scenario: We scan a valid old company card.', function () {
		steps.testOnly = 1;
		if(expect(steps.weScanQR('HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G')).toBe(true));
		steps.testOnly = 1;
		if(expect(steps.accountIsCompany()).toBe(true));
		steps.testOnly = 1;
		if(expect(steps.accountIDIs('NEWAAB')).toBe(true));
		steps.testOnly = 1;
		if(expect(steps.securityCodeIs('WeHlioM5JZv1O9G')).toBe(true));
	});

	it('Scenario: We scan a valid personal card.', function () {
		steps.testOnly = 1;
		if(expect(steps.weScanQR('HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw')).toBe(true));
		console.log(steps.weScanQR('HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw'));
		steps.testOnly = 1;
		if(expect(steps.accountIsPersonal()).toBe(true));
		console.log(steps.accountIsPersonal());
		steps.testOnly = 1;
		if(expect(steps.accountIDIs('NEWABB')).toBe(true));
		console.log(steps.accountIDIs('NEWABB'));
		steps.testOnly = 1;
		if(expect(steps.securityCodeIs('ZzhWMCq0zcBowqw')).toBe(true));
		console.log(steps.securityCodeIs('ZzhWMCq0zcBowqw'));
	});

	it('Scenario: We scan a valid company card.', function () {
		steps.testOnly = 1;
		if(expect(steps.weScanQR('HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G')).toBe(true));
		console.log(steps.weScanQR('HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G'));
		steps.testOnly = 1;
		if(expect(steps.accountIsCompany()).toBe(true));
		console.log(steps.accountIsPersonal());
		steps.testOnly = 1;
		if(expect(steps.accountIDIs('NEWAAB-A')).toBe(true));
		console.log(steps.accountIDIs('NEWAAB-A'));
		steps.testOnly = 1;
		if(expect(steps.securityCodeIs('WeHlioM5JZv1O9G')).toBe(true));
		console.log(steps.securityCodeIs('WeHlioM5JZv1O9G'));
	});
});
  