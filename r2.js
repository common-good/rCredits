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
		var wait = 20000;
		var count = 0;
		var EC = protractor.ExpectedConditions;
		/**
		 * Add additional setup for any or all features or tests
		 */
		this.extraSetup = function () {
			browser.getSession().then(function (session) {
				console.log('SessionID=' + session.getId());
			});
			browser.driver.get("http://localhost:8100/#/app/login", 500);
		};
		var waitForElement = function (selector, waitFor) {
			waitFor = waitFor || 50000;
			browser.driver.manage().timeouts().implicitlyWait(waitFor);
			browser.driver.findElement(by.css(selector));
			browser.driver.manage().timeouts().implicitlyWait(0);
		};
		/**
		 * we scan QR (ARG)
		 * in: MAKE ParseQRCode WeScanAValidOldPersonalCard
		 *     MAKE ParseQRCode WeScanAValidOldCompanyCard
		 *     MAKE ParseQRCode WeScanAValidPersonalCard
		 *     MAKE ParseQRCode WeScanAValidCompanyCard
		 */
		var del = .5;
		this.weScanPersonalQR = function (qr) {
			this.v['qr'] = qr;
			var parts = qr.split(/[/\\.-]/);
			console.log(parts[5].length, qr.indexOf('HTTP://'));
			browser.wait(browser.executeScript("document.getElementById('scan').click();"), wait)
				.then(function () {
					if (parts[5].length <= 4) {
						browser.wait(element(by.id('oldURL')).click(), wait);
					} else if (qr.indexOf('HTTP://') !== -1) {
						browser.wait(element(by.id('newURL')).click(), wait);
					}
				})
				.then(function () {
					console.log("document.getElementById('" + qr + "').click();");
					element(by.id("customQR")).sendKeys(qr);
					var link = element(by.id("accountInfoButton"));
					var isClickable = EC.elementToBeClickable(link);
					browser.wait(isClickable, wait)
						.then(link.click());
					console.log('weScanQR');
				})
				.then(browser.executeScript('return window.find("isCompany:true");'))
				.then(function () {
					browser.wait(browser.driver.get("https://otherrealm.org"), wait)
						.then(browser.wait(browser.driver.get("http://localhost:8100/#/app/home"), wait))
						.then(browser.wait(browser.executeScript("document.getElementById('scan').click();"), wait))
						.then(function () {
							element(by.id("customQR")).sendKeys('H6VM010WeHlioM5JZv1O9G');
							var link = element(by.id("demoAccountLogin"));
							var isClickable = EC.elementToBeClickable(link);
							browser.wait(isClickable, wait)
								.then(link.click());
						}).then(browser.wait(browser.executeScript('window.scrollTo(0,document.body.scrollHeight)')
						.then(function () {
							var button = element(by.id('scan'));
							var isClickable = EC.elementToBeClickable(button);
							browser.driver.wait(isClickable, wait); //wait for an element to become clickable
							browser.driver.wait(button.click(), wait);
						}), wait))
						.then(function () {
							element(by.id("customQR")).sendKeys(qr);
							var link = element(by.id("accountInfoButton"));
							var isClickable = EC.elementToBeClickable(link);
							browser.wait(isClickable, wait)
								.then(link.click());
						});
					return expect(browser.wait(EC.textToBePresentInElement(element(by.id("url")), qr), wait)).toBe(true);
				});
		};
		this.weScanCompanyQR = function (qr) {
			this.v['qr'] = qr;
			var parts = qr.split(/[/\\.-]/);
			console.log(parts[5].length, qr.indexOf('HTTP://'));
			browser.wait(browser.executeScript("document.getElementById('scan').click();"), wait)
				.then(function () {
					if (parts[5].length <= 4) {
						browser.wait(element(by.id('oldURL')).click(), wait);
					} else if (qr.indexOf('HTTP://') !== -1) {
						browser.wait(element(by.id('newURL')).click(), wait);
					}
				})
				.then(function () {
					console.log("document.getElementById('" + qr + "').click();");
					element(by.id("customQR")).sendKeys(qr);
					var link = element(by.id("accountInfoButton"));
					var isClickable = EC.elementToBeClickable(link);
					browser.wait(isClickable, wait)
						.then(link.click());
					console.log('weScanQR');
				})
				.then(browser.executeScript('return window.find("isCompany:true");'))
				.then(function () {
					return expect(browser.wait(EC.textToBePresentInElement(element(by.id("url")), qr), wait)).toBe(true);
				});
		};
		this.accountIsPersonal = function () {
			return expect(browser.wait(EC.textToBePresentInElement(element(by.id("isPersonal")), 'true'), wait)).toBe(true);
		};
		/**
		 * account is company
		 *
		 * in: TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIsCompany = function () {
			return expect(browser.wait(EC.textToBePresentInElement(element(by.id("isCompany")), 'true'), wait)).toBe(true);
		};
		/**
		 * account ID is (ARG)	 *
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.accountIDIs = function (id) {
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("memberId")), id), wait)).toBe(true);
		};
		/**
		 * security code is (ARG)
		 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
		 *     TEST ParseQRCode WeScanAValidOldCompanyCard
		 *     TEST ParseQRCode WeScanAValidPersonalCard
		 *     TEST ParseQRCode WeScanAValidCompanyCard
		 */
		this.securityCodeIs = function (securityCode) {
			return expect(browser.driver.wait(EC.textToBePresentInElement(element(by.id("unencryptedCode")), securityCode), wait)).toBe(true);
		};
		/**
		 * show page (ARG)
		 *
		 * in: MAKE Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showPage = function (p) {
			if (count++ === 0) {
				browser.executeScript('window.scrollTo(0,document.body.scrollHeight)').then(function () {
					var button = element(by.id('scan'));
					var isClickable = EC.elementToBeClickable(button);
					browser.driver.wait(isClickable, wait); //wait for an element to become clickable
					button.click();
				});
				element(by.id("customQR")).sendKeys('H6VM010WeHlioM5JZv1O9G');
				var exp;
				var link = element(by.id("demoAccountLogin"));
				var isClickable = EC.elementToBeClickable(link);
				browser.wait(isClickable, wait)
					.then(link.click());
			}
			return expect(browser.wait(EC.urlContains(p.toLowerCase()), wait)).toBe(true);
		};
		/**
		 * show button (ARG)
		 *
		 * in: TEST Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showButton = function (arg1) {
			var button = expect(element(by.cssContainingText('.button', arg1)).getText()).toEqual(arg1);
			return browser.wait(button, wait);
		};
		/**
		 * show back button (ARG)
		 *
		 * in: TEST Transact Setup
		 *     TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showBackButton = function (arg1) {
			return browser.wait(expect(browser.executeScript("return document.getElementById('testing').innerHTML")).toBe(arg1));
		};
		/**
		 * button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.buttonPressed = function (arg1) {
			var button = element(by.cssContainingText('.button', arg1.toString())).getText();
			var link = element(by.cssContainingText('.button', arg1.toString())).getText();
			var isClickable = EC.or(EC.elementToBeClickable(link), EC.elementToBeClickable(button));
			browser.wait(isClickable, wait);
			link.click();
			return isClickable;
		};
		/**
		 * show scanner
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showScanner = function () {
			return expect(browser.driver.wait(EC.or(EC.urlContains('demo-people'), EC.urlContains('qr')), wait)).toBe(true);
		};
		/**
		 * scanner sees QR (ARG)
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.scannerSeesQR = function (arg1) {
			var QR = element(by.id("customQR"));
			var infoB = element(by.id('accountInfoButton'));
			var link = element(by.id("demoAccountLogin"));
			var isClickable1 = EC.elementToBeClickable(infoB);
			var isClickable2 = EC.elementToBeClickable(link);
			console.log(arg1);
			var url;
			browser.wait(QR.sendKeys(arg1), wait)
				.then(browser.wait(isClickable1, wait))
				.then(infoB.click())
				.then(function () {
					return browser.wait(url = element(by.binding("customPerson.accountInfo.url")), wait);
				}).then(QR.sendKeys(arg1))
				.then(browser.wait(isClickable2, wait))
				.then(link.click())
				.then(function () {
					return browser.wait(expect(url.getText()).toEqual('url:' + arg1), wait);
				});
		};
		/**
		 * show photo of member (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer 
		 */
		this.showPhotoOfMember = function (arg1) {
			return expect(browser.driver.wait(element(by.id('ItemPreview')).isPresent(), wait)).toBe(true);
		};
		/**
		 * show text (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showText = function (arg1) {
			return  expect(browser.wait(EC.textToBePresentInElement(element(by.id('customerPage')), arg1), wait)).toBe(true);
		};
		/**
		 * show number keypad
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showNumberKeypad = function () {
			return  expect(browser.driver.wait(element(by.id('keypad')).isPresent(), wait)).toBe(true);
		};
		/**
		 * show amount (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showAmount = function (arg1) {
			return expect(browser.driver.wait(element(by.id('displayAmount')).getText(), wait)).toEqual(arg1.toString());
		};
		/**
		 * show dropdown with (ARG) selected
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showDropdownWithSelected = function (arg1) {
			return expect(browser.driver.wait(element(by.options('c |  translate for c in categories')).getText(), wait)).toEqual(arg1.toString());
		};
		/**
		 * show (ARG) message (ARG) titled (ARG)
		 *
		 * in: TEST Transact WeIdentifyAndChargeACustomer
		 */
		this.showMessageTitled = function (arg1, arg2, arg3) {
			var note;
			var headding;
			var backHome;
			browser.ignoreSynchronization = true;
			browser.wait(EC.urlContains('http://localhost:8100/#/app/transaction-result/success/'), wait)
				.then(function () {
					browser.wait((note = element(by.id('note'))), 50000)
						.then(function () {
							browser.wait((headding = element(by.id('headingSuccess'))), 50000)
								.then(function () {
									browser.wait((backHome = element(by.id('backHome'))), 50000)
										.then(function () {
											return browser.wait(EC.and(EC.textToBePresentInElement(note, arg2), EC.textToBePresentInElement(headding, arg3), EC.textToBePresentInElement(backHome, arg1)), 50000);
										});
								});
						});
				});
			return true;
		};
		/**
		 * message button (ARG) pressed
		 *
		 * in: MAKE Transact WeIdentifyAndChargeACustomer
		 */
		this.messageButtonPressed = function (arg1) {
			browser.ignoreSynchronization = true;
			var button= element(by.id('backHome'));
			var isClickable = EC.elementToBeClickable(button);
			browser.wait(isClickable, wait)
				.then(browser.wait(browser.executeScript("arguments[0].click();", button.getWebElement()), wait))
				.then(function () {
					return isClickable;
				});
		};
		this.homepage = function (p) {
			return expect(browser.wait(EC.urlContains(p.toLowerCase()), wait)).toBe(true);
		};
	};
	module.exports = function () {
		return new R2_steps();
	};
}());
