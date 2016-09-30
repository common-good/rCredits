/* global Sha256 */

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
		this.parts = this.accountInfo.url.split(/[/\\.-]/);
		this.count = this.parts.length;
		console.log(this.count,this.parts);
//		this.isPersonal = false;
//		this.isCompany = false;
//		this.memberId = '';
//		this.accountId = '';
//		this.securityCode = '';
//		this.url = '';
//		this.serverType = '';
//		this.signin = '';
//isCompany:true
//isPersonal:false
//memberId:"NEW"
//accountId:"NEW:AAD"
//securityCode:"51955e77234f285f58ebd3db39a6499b9eb5ad569e045dda909b4e897ffd1837"
//url:"HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw"
//serverType:"rc4"
//signin:1
//unencryptedCode:"utbYceW3KLLCcaw"
		if (this.count === 6) {
			var region = this.parts[2];
			var tail = this.parts[5];
			var fmt = tail.charAt(0);
			var fmts = ["", "", "012389ABGHIJ", "4567CDEFKLMN", "OPQRSTUV", "WXYZ"];
			var acctLen;
			var i = 0;
			for (acctLen = 2; acctLen < 6; acctLen++) {
				i = fmts[acctLen].indexOf(fmt);
				if (i !== -1) {
					break;
				}
			}
			var agentLen = i % 4;
			if (acctLen === 6 || tail.length < 1 + acctLen + agentLen) {
				console.log('That is not a valid rCard.');
				throw 'That is not a valid rCard.';
			}
			var account = tail.substring(1, 1 + acctLen);
			var seller = tail.substring(1 + acctLen, 1 + acctLen + agentLen);
			this.accountInfo.unencryptedCode = tail.substring(1 + acctLen + agentLen);
			this.accountInfo.securityCode = Sha256.hash(this.accountInfo.unencryptedCode);
			this.accountInfo.isCompany = (agentLen > 0);
			this.accountInfo.isPersonal = !this.accountInfo.isCompany;
			if (this.accountInfo.isCompany) {
				this.accountInfo.signin = 1;
			} else {
				this.accountInfo.signin = 0;
			}
			region = n2a(a2n(region));
			account = n2a(a2n(account));
			seller = this.accountInfo.isCompany ? ("-" + n2a(a2n(seller), -1, 26)) : "";
			this.accountInfo.memberId = region + account;
			this.accountInfo.accountId = this.accountInfo.memberId + seller;
			if (!this.accountInfo.accountId.includes(/^[A-Z]{3,4}[A-Z]{3,5}(-[A-Z]{1,5})?/)) {
				console.log('That is not a valid rCard.');
				throw 'That is not a valid rCard.';
			}
		} else {
			this.parseAccountType_();
			this.parseAccountCode_();
			this.parseSecurityCode_();
		}
		this.parseServerType_();
		return this.accountInfo;
	};
	QRCodeParser.prototype.getAccountInfo = function () {
		return this.accountInfo;
	};
	QRCodeParser.prototype.parseAccountType_ = function () {
		if (this.url.pathname.indexOf(COMPANY_INDICATOR) !== -1) {
			this.accountInfo.isCompany = true;
			this.accountInfo.signin = 1;
		} else if (this.url.pathname.indexOf(PERSONAL_INDICATOR) !== -1) {
			this.accountInfo.isPersonal = true;
			this.accountInfo.signin = 0;
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
		console.log(this.url.pathname.substr(5, this.url.pathname.length - 1));
		this.accountInfo.securityCode = Sha256.hash(this.url.pathname.substr(5, this.url.pathname.length - 1));//
		this.accountInfo.unencryptedCode = this.url.pathname.substr(5, this.url.pathname.length - 1);
		console.log(this.accountInfo.securityCode);
	};
	function n2a(n, len, base) {
		var A = toInteger('A');
		var result = "";
		var digit;
		for (var i = 0; (len > 0 ? (i < len) : (n > 0 || i < -len)); i++) {
			digit = n % base;
			result = ((char)(A + digit)) + result;
			n /= base;
		}
		return result;
	}
	function n2a(n) {
		return n2a(n, -3, 26);
	}
	function a2n(s, base) {
		var A = 'A';
		var zero = '0';
		var result = 0;
		var n;
		for (var i = 0; i < s.length; i++) {
			n = s.charAt(i);
			result = result * base + n - (n >= A ? A - 10 : zero);
		}
		return result;
	}
	function a2n(s) {
		return a2n(s, 36);
	}
	window.QRCodeParser = QRCodeParser;
})(window);