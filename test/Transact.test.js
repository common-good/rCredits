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
	var createdItemUrl;
	beforeAll(function () {
		browser.manage().timeouts().pageLoadTimeout(40000);
		browser.manage().timeouts().implicitlyWait(25000);
		browser.driver.get("http://localhost:8100", 500);
		browser.waitForAngular();
		browser.getCurrentUrl().then(function (url) {
			createdItemUrl = url;
		});
	});
	beforeEach(function () { // Setup
		steps.extraSetup();
		steps.testOnly = 0;
		expect(steps.showPage('Home'));
		steps.testOnly = 1;
		expect(steps.showButton('Scan Customer rCard'));
	});
	it('Scenario: We identify and charge a customer', function () {
		steps.testOnly = 0;
		expect(steps.buttonPressed('Scan Customer rCard'));
		steps.testOnly = 1;
		expect(steps.showScanner());
		steps.testOnly = 0;
		expect(steps.scannerSeesQR('HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw'));
		steps.testOnly = 1;
		expect(steps.showPhotoOfMember('NEWABB'));
		steps.testOnly = 1;
		expect(steps.showText('Susan Shopper'));
		steps.testOnly = 1;
		expect(steps.showText('Montague, MA'));
		steps.testOnly = 1;
		expect(steps.showButton('Charge'));
		steps.testOnly = 1;
		expect(steps.showButton('Refund'));
		steps.testOnly = 1;
		expect(steps.showButton('Trade USD'));
		steps.testOnly = 1;
		expect(steps.showBackButton('Back'));
		steps.testOnly = 0;
		expect(steps.buttonPressed('Charge'));
		steps.testOnly = 1;
		expect(steps.showNumberKeypad());
		steps.testOnly = 1;
		expect(steps.showAmount("0.00"));
		steps.testOnly = 1;
		expect(steps.showDropdownWithSelected('groceries'));
		steps.testOnly = 1;
		expect(steps.showButton('Charge'));
		steps.testOnly = 1;
		expect(steps.showBackButton('Back'));
		steps.testOnly = 0;
		expect(steps.buttonPressed("1"));
		steps.testOnly = 1;
		expect(steps.showAmount("0.01"));
		steps.testOnly = "0";
		expect(steps.buttonPressed('00'));
		steps.testOnly = 1;
		expect(steps.showAmount("1.00"));
		steps.testOnly = 0;
		expect(steps.buttonPressed('Charge'));
		steps.testOnly = 1;
		expect(steps.showMessageTitled('Home', 'You charged Susan Shopper $1', 'Successful'));
		steps.testOnly = 0;
		expect(steps.messageButtonPressed('Home'));
		steps.testOnly = 1;
		expect(steps.homepage('Home'));
	});
});
  