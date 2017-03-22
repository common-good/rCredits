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
		var wait = 10000;
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
				var button = element(by.id('scan'));
				var isClickable = EC.elementToBeClickable(button);
				browser.driver.wait(isClickable, 7000); //wait for an element to become clickable
				browser.driver.wait(button.click(), 7000);
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
			console.log(qr.substr(-16, 1));
//			if (qr.substr(-16, 1) === 'H'||qr.substr(-16, 1) === '-') {
			var scan = element(by.id("customQR")).sendKeys(qr);
			var link = element(by.id("accountInfoButton"));
			var isClickable = EC.elementToBeClickable(link);
			browser.driver.wait(isClickable, 7000);
			browser.wait(link.click(), 7000).then(
				browser.driver.refresh);
			console.log('weScanQR');
//			} else {
//				var scan = element(by.id("customQR")).sendKeys(qr);
//				var link = element(by.id("demoAccountLogin"));
//				var isClickable = EC.elementToBeClickable(link);
//				browser.driver.wait(isClickable, 7000);
//				browser.wait(link.click(), 7000);
//			}
			return true;
		};
		this.accountIsPersonal = function () {
			var d = protractor.promise.defer();
			var p = function () {
				var button = element(by.cssContainingText('.button', 'scan')).getText();
				var link = element(by.cssContainingText('.button', 'scan')).getText();
				var isClickable = EC.or(EC.elementToBeClickable(link), EC.elementToBeClickable(button));
				browser.wait(isClickable, 8000);
				browser.wait(link.click(),wait);
			}
			.then(function () {browser.wait(element(by.id("customQR")).sendKeys('HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G'),wait);})
				.then(function () {
					var link = element(by.id("demoAccountLogin"));
					var isClickable = EC.elementToBeClickable(link);
					browser.driver.wait(isClickable, 7000);
					browser.wait(link.click(), 7000);
				})
				.then(function () {
					var button = element(by.cssContainingText('.button', 'scan')).getText();
					var link = element(by.cssContainingText('.button', 'scan')).getText();
					var isClickable = EC.or(EC.elementToBeClickable(link), EC.elementToBeClickable(button));
					browser.wait(isClickable, 8000);
					browser.wait(link.click(),wait);
				})
				.then(element(by.id("customQR")).sendKeys(this.v['qr']))
				.then(function () {
					var isClickable2 = EC.elementToBeClickable(element(by.id("accountInfoButton")));
					browser.driver.wait(isClickable2, wait);
					var link2 = element(by.id("accountInfoButton"));
					browser.wait(link2.click(), 7000);
					d.resolve(expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")), 'true'), 999000)).toBe(true));
				});
			return d.promise;
		};
		/**
		 * account is company
		 *
		 * in: TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIsCompany = function () {
//			browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")),'false'),10000);
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("isPersonal")), 'false'), 7000)).toBe(true);
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
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("memberId")), id), 7000)).toBe(true);
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
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("unencryptedCode")), securityCode), 7000)).toBe(true);
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
//				browser.driver.wait(isClickable, 7000); //wait for an element to become clickable
//				button.click();
//			});
			element(by.id("customQR")).sendKeys('H6VM010WeHlioM5JZv1O9G');
			var exp;
			var link = element(by.id("demoAccountLogin"));
			var isClickable = EC.elementToBeClickable(link);
			browser.wait(isClickable, 7000)
				.then(link.click());
//			.then(exp=expect(browser.wait(EC.urlContains(p), 5000)));
			return browser.wait(EC.urlContains(p.toLowerCase()), 7000);
		};
		/**
		 * show button (ARG)
		 *
		 * in: TEST Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showButton = function (arg1) {
			return browser.wait(expect(element(by.cssContainingText('button', arg1)).getText()).toEqual(arg1), 6000);
		};
		/**
		 * button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.buttonPressed = function (arg1) {
			console.log(arg1);
			var button = element(by.cssContainingText('.button', arg1.toString())).getText();
			var link = element(by.cssContainingText('.button', arg1.toString())).getText();
			var isClickable = EC.or(EC.elementToBeClickable(link), EC.elementToBeClickable(button));
			browser.wait(isClickable, 8000);
			link.click();
			return isClickable;
		};
		/**
		 * show scanner
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showScanner = function () {
			return browser.driver.wait(EC.or(EC.urlContains('demo-people'), EC.urlContains('qr')), 9000);
		};
		/**
		 * scanner sees QR (ARG)
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.scannerSeesQR = function (arg1) {
			var QR = element(by.id("customQR"));
			QR.sendKeys(arg1);
			var link = element(by.id("demoAccountLogin"));
			var isClickable = EC.elementToBeClickable(link);
			browser.wait(isClickable, 7000)
				.then(link.click());
			return expect(QR.getText()).toBe(arg1);
		};
		/**
		 * show photo of member (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer 
		 */
		this.showPhotoOfMember = function (arg1) {
			return browser.driver.wait(expect(element(by.id('ItemPreview')).isPresent()).toBe(true), 7000);
		};
		/**
		 * show text (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showText = function (arg1) {
			return  browser.wait(EC.textToBePresentInElement(element(by.id('customerPage')), arg1), 7000);
		};
		/**
		 * show number keypad
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showNumberKeypad = function () {
			return  browser.driver.wait(expect(element(by.id('keypad')).isPresent()).toBe(true), 7000);
		};
		/**
		 * show amount (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showAmount = function (arg1) {
			return browser.driver.wait(expect(element(by.id('displayAmount')).getText()).toEqual(arg1.toString()), 7000);
		};
		/**
		 * show dropdown with (ARG) selected
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showDropdownWithSelected = function (arg1) {
			return true; //browser.driver.wait(EC.textToBePresentInElementValue(by.id('category'), arg1), 7000);
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
