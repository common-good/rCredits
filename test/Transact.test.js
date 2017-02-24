/* global _$compile_, _$rootScope_, expect, browser */
//
// Feature: Transfer funds to or from a customer.
//   AS a company or individual
//   I WANT to scan a customer card and transfer funds from their account to my account or vice versa
//   SO we can account fairly for our business dealings.
var R2_steps =require('../r2.js');
describe('r2% -- FEATURE_NAME', function () {
	'use strict';
	var steps = new R2_steps();
	var eachStep;
	beforeEach(function () { // Setup
		eachStep = new Promise(browser.get('http://localhost:8100/#/app/home'));//
		eachStep.then(steps.extraSetup());
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.showPage('Home')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('Scan Customer rCard')).toBe(true));
	});
	it('Scenario: We identify and charge a customer', function () {
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.buttonPressed('Scan Customer rCard')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showScanner()).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.scannerSeesQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showPhotoOfMember('NEWABB')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showText('Susan Shopper')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showText('Montague, MA')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('Charge')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('Refund')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('Trade USD')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('< Back')).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.buttonPressed('Charge')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showNumberKeypad()).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showAmount(0.00)).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showDropdownWithSelected('groceries')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('Charge')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showButton('< Back')).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.buttonPressed(3)).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showAmount(0.03)).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.buttonPressed('00')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showAmount(3.00)).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.buttonPressed('Charge')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showMessageTitled('ok', 'Susan Shopper paid you $3.00', 'Success!')).toBe(true));
		eachStep.then(steps.testOnly = 0);
		eachStep.then(expect(steps.messageButtonPressed('ok')).toBe(true));
		eachStep.then(steps.testOnly = 1);
		eachStep.then(expect(steps.showPage('Home')).toBe(true));
	});
});
  