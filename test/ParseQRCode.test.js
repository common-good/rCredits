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
		eachStep = new Promise(function (resolve, reject) {
			resolve(1);
		});
		eachStep.then(function (resolve, reject) {return browser.get('http://localhost:8100/#/app/home');});
		eachStep.then(function (resolve, reject) {return steps.extraSetup();});
//		eachStep.then(function (resolve, reject) {steps.extraSetup());
	});
	it('Scenario: We scan a valid old personal card.', function () {
		eachStep.then(function (resolve, reject) {
				return steps.testOnly = 0;
		});
		eachStep.then(function (resolve, reject) {return expect(steps.weScanQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.weScanQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIsPersonal()).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIsPersonal());resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIDIs('NEWABB')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIDIs('NEWABB'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.securityCodeIs('ZzhWMCq0zcBowqw')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.securityCodeIs('ZzhWMCq0zcBowqw'));resolve(true);});
	});

	it('Scenario: We scan a valid old company card.', function () {
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.weScanQR('HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIsCompany()).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIDIs('NEWAAB')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.securityCodeIs('WeHlioM5JZv1O9G')).toBe(true);resolve(true);});
	});

	it('Scenario: We scan a valid personal card.', function () {
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.weScanQR('HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.weScanQR('HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIsPersonal()).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIsPersonal());resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIDIs('NEWABB')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIDIs('NEWABB'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.securityCodeIs('ZzhWMCq0zcBowqw')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.securityCodeIs('ZzhWMCq0zcBowqw'));resolve(true);});
	});

	it('Scenario: We scan a valid company card.', function () {
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.weScanQR('HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.weScanQR('HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIsCompany()).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIsPersonal());resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.accountIDIs('NEWAAB-A')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.accountIDIs('NEWAAB-A'));resolve(true);});
		eachStep.then(function (resolve, reject) {return steps.testOnly = 1;resolve(true);});
		eachStep.then(function (resolve, reject) {return expect(steps.securityCodeIs('WeHlioM5JZv1O9G')).toBe(true);resolve(true);});
		eachStep.then(function (resolve, reject) {return console.log(steps.securityCodeIs('WeHlioM5JZv1O9G'));resolve(true);});
	});
});
  