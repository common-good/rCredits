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
function R2_steps() {
	this.v = []; // miscellaneous data
	/**
	 * Add additional setup for any or all features or tests
	 */
	this.extraSetup = function () {};
	/**
	 * we scan QR (ARG)
	 * in: MAKE ParseQRCode WeScanAValidOldPersonalCard
	 *     MAKE ParseQRCode WeScanAValidOldCompanyCard
	 *     MAKE ParseQRCode WeScanAValidPersonalCard
	 *     MAKE ParseQRCode WeScanAValidCompanyCard
	 */
	this.weScanQR = function (qr) {
		this.v.parser = new QRCodeParser();
		this.v.parser.setUrl(qr);
		this.v.parser.parse();
		console.log(this.v.parser);
		return true;
	};
	/**
	 * account is personal
	 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
	 *     TEST ParseQRCode WeScanAValidPersonalCard
	 */
	this.accountIsPersonal = function () {
		var isItP=(this.v.parser.getAccountInfo().isPersonalAccount()===this.v.parser.accountInfo.isPersonal);
		console.log("isItP");
		console.log(isItP);
		return isItP;
	};
	/**
	 * account is company
	 *
	 * in: TEST ParseQRCode WeScanAValidOldCompanyCard
	 *     TEST ParseQRCode WeScanAValidCompanyCard
	 */
	this.accountIsCompany = function () {
		var isItC=(this.v.parser.getAccountInfo().isCompanyAccount()===this.v.parser.accountInfo.isCompany);
		console.log("isItC");
		console.log(isItC);
		return isItC;
	};
	/**
	 * account ID is (ARG)	 *
	 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
	 *     TEST ParseQRCode WeScanAValidOldCompanyCard
	 *     TEST ParseQRCode WeScanAValidPersonalCard
	 *     TEST ParseQRCode WeScanAValidCompanyCard
	 */
	this.accountIDIs = function (id) {
		var isThereId=(this.v.parser.getAccountInfo().accountId === id);
		console.log("isThereId:"+this.v.parser.getAccountInfo().accountId);
		console.log(isThereId+id);
		return isThereId;
	};
	/**
	 * security code is (ARG)
	 * in: TEST ParseQRCode WeScanAValidOldPersonalCard
	 *     TEST ParseQRCode WeScanAValidOldCompanyCard
	 *     TEST ParseQRCode WeScanAValidPersonalCard
	 *     TEST ParseQRCode WeScanAValidCompanyCard
	 */
	this.securityCodeIs = function (id) {
		console.log("this.v.parser.getAccountInfo().securityCode:"+this.v.parser.getAccountInfo().unencryptedCode);
		console.log("id:"+id);
		return (this.v.parser.getAccountInfo().unencryptedCode === id);
	};
	/**
	 * show page (ARG)
	 *
	 * in: MAKE Transact Setup
	 *     TEST Transact WeIdentifyAndChargeACustomer
	 */
	this.showPage = function (p) {
		var page=document.location.hash===p;
		
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
}