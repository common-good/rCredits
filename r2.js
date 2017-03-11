/* global browser, by, expectedUrl, timeout, element, expect, protractor */

/**
 * @file
 *  Steps
 *
 * Provide step functions for functional testing.
 * This file is created and modified automatically by the Gherkin compiler.
 *
 * Note, therefore, that most of this file might be changed automatically
 * when you run the compiler again. This @file header will not be affected,
 * but all of the function header comments are (re)generated automatically.
 * Methods within the R2_steps object function are not indented -- do not change that!
 * The last character in this file must be the object function's closing brace (}).
 *
 * Be assured that no functions will be deleted and the compiler will
 * not alter code within a function unless you make it look like a function header.
 *
 * You may also add statements just below this header.
 */

(function () {
	var R2_steps = function () {
		this.v = []; // miscellaneous data
		this.v.parse = '';
		var EC = protractor.ExpectedConditions;
		/**
		 * Add additional setup for any or all features or tests
		 */
		this.extraSetup = function () {
			browser.getSession().then(function (session) {
				console.log('SessionID=' + session.getId());
			});
			browser.driver.get("http://localhost:8100/#/app/login", 500);
			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
				var button = element(by.id('scan-to-login'));
				var isClickable = EC.elementToBeClickable(button);
				browser.driver.wait(isClickable, 3000); //wait for an element to become clickable
				button.click();
			});
		};
		/**
		 * we scan QR (ARG)
		 * in: MAKE ParseQRCode WeScanAValidOldPersonalCard
		 *     MAKE ParseQRCode WeScanAValidOldCompanyCard
		 *     MAKE ParseQRCode WeScanAValidPersonalCard
		 *     MAKE ParseQRCode WeScanAValidCompanyCard
		 */
		var q = 0;
		var del = .5;
		this.weScanQR = function (qr) {
			this.v['qr'] = qr;
			var scan = element(by.id("customQR")).sendKeys(qr);
			var link=element(by.id("accountInfoButton"));
			var isClickable = EC.elementToBeClickable(link);
			browser.driver.wait(isClickable,3000);
			link.click();
			return true;
		};
		/**
		 * account is personal
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 */
		this.accountIsPersonal = function () {
			browser.driver.get("http://localhost:8100/#/app/home", 500);
			browser.driver.get("http://localhost:8100/#/app/login", 500);
			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
				var button = element(by.id('scan-to-login'));
				var isClickable = EC.elementToBeClickable(button);
				browser.driver.wait(isClickable, 3000); //wait for an element to become clickable
				button.click();
			});
			element(by.id("customQR")).sendKeys('H6VM010WeHlioM5JZv1O9G');
			var link=element(by.id("accountLogin"));
			var isClickable = EC.elementToBeClickable(link);
			browser.driver.wait(isClickable,3000);
			link.click();
			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
				var button = element(by.id('scan-customer'));
				var isClickable = EC.elementToBeClickable(button);
				browser.driver.wait(isClickable, 3000); //wait for an element to become clickable
				button.click();
			});
			element(by.id("customQR")).sendKeys(this.v['qr']);
			var link2=element(by.id("accountInfoButton"));
			var isClickable2 = EC.elementToBeClickable(link2);
			browser.driver.wait(isClickable2,3000);
			link2.click();
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")),'true'),3000)).toBe(true);
		};
		/**
		 * account is company
		 *
		 * in: TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIsCompany = function () {
//			browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")),'false'),10000);
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")),'false'),3000)).toBe(true);
		};
		/**
		 * account ID is (ARG)	 *
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIDIs = function (id) {
//			browser.driver.wait(EC.textToBePresentInElement(element(by.id("memberId")),'false'),10000);
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("memberId")),id),3000)).toBe(true);
		};
		/**
		 * security code is (ARG)
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.securityCodeIs = function (securityCode) {
//			browser.driver.wait(EC.textToBePresentInElement(element(by.id("unencryptedCode")),'false'),10000);
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("unencryptedCode")),securityCode),3000)).toBe(true);
		};
		/**
		 * show page (ARG)
		 *
		 * in: MAKE Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showPage = function (p) {
//			browser.driver.get("http://localhost:8100/#/app/home", 500);
//			browser.driver.get("http://localhost:8100/#/app/login", 500);
//			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
//				var button = element(by.id('scan-to-login'));
//				var isClickable = EC.elementToBeClickable(button);
//				browser.driver.wait(isClickable, 3000); //wait for an element to become clickable
//				button.click();
//			});
			element(by.id("customQR")).sendKeys('H6VM010WeHlioM5JZv1O9G');
			var link=element(by.id("accountLogin"));
			var isClickable = EC.elementToBeClickable(link);
			browser.driver.wait(isClickable,3000);
			link.click();
			return expect(browser.driver.wait(EC.urlContains(p), 5000));
		};
		/**
		 * show button (ARG)
		 *
		 * in: TEST Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showButton = function (arg1) {
			return expect(element(by.buttonText(arg1)).getText()).toEqual(arg1);
		};
		/**
		 * button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.buttonPressed = function (arg1) {
			return element(by.partialLinkText(arg1).click());
		};
		/**
		 * show scanner
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showScanner = function () {
//			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
//				var button = element(by.id('scan-customer'));
//				var isClickable = EC.elementToBeClickable(button);
//				browser.driver.wait(isClickable, 3000); //wait for an element to become clickable
//				button.click();
//			});
//			element(by.id("customQR")).sendKeys(p);
//			var link2=element(by.id("accountInfoButton"));
//			var isClickable2 = EC.elementToBeClickable(link2);
//			browser.driver.wait(isClickable2,3000);
//			link2.click();
			return expect(browser.driver.wait(EC.or(EC.urlContains('demo-people'),EC.urlContains('qr')), 5000));
		};
		/**
		 * scanner sees QR (ARG)
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.scannerSeesQR = function (arg1) {
			var code=this.securityCodeIs(arg1);
			var link=element(by.id("accountLogin"));
			var isClickable = EC.elementToBeClickable(link);
			browser.driver.wait(isClickable,3000);
			link.click();
			return code;
		};
		/**
		 * show photo of member (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer 
		 */
		this.showPhotoOfMember = function (arg1) {
			return browser.driver.wait(element(by.id('ItemPreview').isPresent()).toBe(true), 3000);
		};
		/**
		 * show text (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showText = function (arg1) {
			return browser.driver.wait(EC.textToBePresentInElement(by.id('customerPage'),arg1), 3000);
		};
		/**
		 * show number keypad
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showNumberKeypad = function () {
			return browser.driver.wait(element(by.id('keypad').isPresent()).toBe(true), 3000);
		};
		/**
		 * show amount (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showAmount = function (arg1) {
			return browser.driver.wait(element(by.id('keypad').getText()).toEqual(arg1), 3000);
		};
		/**
		 * show dropdown with (ARG) selected
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showDropdownWithSelected = function (arg1) {
			return browser.driver.wait(EC.textToBePresentInElementValue(by.id('category'),arg1), 3000);
		};
		/**
		 * show (ARG) message (ARG) titled (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showMessageTitled = function (arg1, arg2, arg3) {
			return true;
		};
		/**
		 * message button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.messageButtonPressed = function (arg1) {
			return true;
		};
	};
	module.exports = function () {
		return new R2_steps();
	};
}());
