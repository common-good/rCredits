///* global _$compile_, _$rootScope_, expect, browser */
////
//// Feature: Transfer funds to or from a customer.
////   AS a company or individual
////   I WANT to scan a customer card and transfer funds from their account to my account or vice versa
////   SO we can account fairly for our business dealings.
//var R2_steps =require('../r2.js');
//describe('r2% -- FEATURE_NAME', function () {
//	'use strict';
//	var steps = new R2_steps();
//	var eachStep;
//	beforeEach(function () { // Setup
//		eachStep = new Promise(function(resolve, reject) {steps.extraSetup()?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showPage('Home')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('Scan Customer rCard')).toBe(true)?resolve:reject;});
//	});
//	it('Scenario: We identify and charge a customer', function () {
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.buttonPressed('Scan Customer rCard')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showScanner()).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.scannerSeesQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showPhotoOfMember('NEWABB')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showText('Susan Shopper')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showText('Montague, MA')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('Charge')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('Refund')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('Trade USD')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('< Back')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.buttonPressed('Charge')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showNumberKeypad()).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showAmount(0.00)).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showDropdownWithSelected('groceries')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('Charge')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showButton('< Back')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.buttonPressed(3)).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showAmount(0.03)).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.buttonPressed('00')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showAmount(3.00)).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.buttonPressed('Charge')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showMessageTitled('ok', 'Susan Shopper paid you $3.00', 'Success!')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 0?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.messageButtonPressed('ok')).toBe(true)?resolve:reject;});
//		eachStep.then(function (resolve, reject) {steps.testOnly = 1?resolve:reject;});
//		eachStep.then(function (resolve, reject) {expect(steps.showPage('Home')).toBe(true)?resolve:reject;});
//	});
//});
//  