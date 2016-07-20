(function (window) {
	var COMPANY_INDICATOR = '-';
	var COMPANY_INDICATOR_URL = ':';
	var PERSONAL_INDICATOR = '.';
	var QRCodeParser = function () {
	};
	QRCodeParser.prototype.setUrl = function (url) {
		console.log(url);
		this.plainUrl = url;
		this.url = new URL(url);
	};
	QRCodeParser.prototype.parse = function () {
		this.accountInfo = new AccountInfo();
		this.accountInfo.url = this.plainUrl;
		this.parseAccountType_();
		this.parseAccountCode_();
		this.parseSecurityCode_();
		this.parseServerType_();
		return this.accountInfo;
	};
	QRCodeParser.prototype.getAccountInfo = function () {
		return this.accountInfo;
	};
	QRCodeParser.prototype.parseAccountType_ = function () {
		if (this.url.pathname.indexOf(COMPANY_INDICATOR) !== -1) {
			this.accountInfo.isCompany = true;
			this.accountInfo.signin=1;
		} else if (this.url.pathname.indexOf(PERSONAL_INDICATOR) !== -1) {
			this.accountInfo.isPersonal = true;
			this.accountInfo.signin=0;
		} else {
			console.log('That is not a valid rCard.');
			throw 'That is not a valid rCard.';
		}
	};
	QRCodeParser.prototype.parseAccountCode_ = function () {
		var memberId = this.url.hostname.toUpperCase().split('.')[0];
		var yyy = this.url.pathname.substring(1, 4);
		var separator;
		if (this.accountInfo.isPersonalAccount()) {
			separator = PERSONAL_INDICATOR;
		} else {
			separator = COMPANY_INDICATOR_URL;
		}
		this.accountInfo.memberId = memberId;
		this.accountInfo.accountId = memberId + separator + yyy;
	};
	QRCodeParser.prototype.parseServerType_ = function () {
		var lastPoint = this.url.host.lastIndexOf('.');
		this.accountInfo.serverType = this.url.host.substring(4, lastPoint).toLowerCase();
	};
	QRCodeParser.prototype.parseSecurityCode_ = function () {
		this.accountInfo.securityCode = this.url.pathname.substr(5, this.url.pathname.length - 1);
	};
	window.QRCodeParser = QRCodeParser;
})(window);