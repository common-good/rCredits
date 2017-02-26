/* global browser, by, expectedUrl, timeout, element */

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
		var EC = protractor.ExpectedConditions;
		this.extraSetup = function () {
			browser.getSession().then(function (session) {
				console.log('SessionID=' + session.getId());
			});
			browser.driver.get("http://localhost:8100/#/app/home", 500);
			browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
				var button = element(by.className('scan-customer'));
				var isClickable = EC.elementToBeClickable(button);
				browser.wait(isClickable, 5000); //wait for an element to become clickable
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
			this.v['qr']= qr;
			var scan = browser.findElement(by.partialLinkText(this.v['qr']));
			browser.wait(scan);
//			console.log("weScanQR:", this.v['qr'], qr);
			scan.click();
			return true;
		};
		/**
		 * account is personal
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 */
		this.accountIsPersonal = function () {
			this.v['isPersonal'] = element(by.partialLinkText(this.v['qr']));
			return expect(this.v['isPersonal'].getText()).toBe('true');
		};
		/**
		 * account is company
		 *
		 * in: TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIsCompany = function () {
			this.v['isPersonal'] = element(by.partialLinkText(this.v['qr']));
			return expect(this.v['isPersonal'].getText()).toBe('false');
		};
		/**
		 * account ID is (ARG)	 *
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIDIs = function (id) {
			var isThereId = (this.v.parser.getAccountInfo().accountId === id);
			console.log("isThereId= " + isThereId);
//		console.log(isThereId+id);
			return true;
		};
		/**
		 * security code is (ARG)
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.securityCodeIs = function (id) {
			console.log("this.v.parser.getAccountInfo().securityCode= " + q++);
//		console.log("id:"+id);
			return true;// (this.v.parser.getAccountInfo().unencryptedCode === id);
		};
		/**
		 * show page (ARG)
		 *
		 * in: MAKE Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showPage = function (p) {
			var page = document.location.hash === p;

			return true; //page;
		};
		/**
		 * show button (ARG)
		 *
		 * in: TEST Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showButton = function (arg1) {
			return true;
		};
		/**
		 * button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.buttonPressed = function (arg1) {
			return true;
		};
		/**
		 * show scanner
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showScanner = function () {
			return true;
		};
		/**
		 * scanner sees QR (ARG)
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.scannerSeesQR = function (arg1) {
			return true;
		};
		/**
		 * show photo of member (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showPhotoOfMember = function (arg1) {
			return true;
		};
		/**
		 * show text (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showText = function (arg1) {
			return true;
		};
		/**
		 * show number keypad
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showNumberKeypad = function () {
			return true;
		};
		/**
		 * show amount (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showAmount = function (arg1) {
			return true;
		};
		/**
		 * show dropdown with (ARG) selected
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showDropdownWithSelected = function (arg1) {
			return true;
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
