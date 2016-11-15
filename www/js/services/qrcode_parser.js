/* global Sha256, parseInt, rCreditsConfig */
(function (window) {
	var COMPANY_INDICATOR = '-';
	var COMPANY_INDICATOR_URL = ':';
	var PERSONAL_INDICATOR = '.';
	var alphaB = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
	var regionLens = '112233344';
	var acctLens = '232323445';
	var oldCode = false;
	var QRCodeParser = function () {
	};
	QRCodeParser.prototype.setUrl = function (url) {
		console.log(url);
		if (url.indexOf('HTTP://') === -1) {
			var fmt = url.substring(0, 1);
			var i = parseInt(fmt, 36) / 4;
			var regionLen = parseInt(regionLens.charAt(i));
			var region = url.substring(1, regionLen + 1);
			var transformedURL = url.replace(region, '');
			if (url.indexOf(':') !== -1) {
				var realOrFake = 'RCREDITS.ORG';
			} else {
				var realOrFake = 'RC4.ME';
			}
			url = 'HTTP://' + region + '.' + realOrFake + '/' + transformedURL;
			console.log(region, url, (regionLens.indexOf(alphaB.indexOf(fmt)) + 1), regionLen);
		}
		this.plainUrl = url;
		this.url = new URL(url);
	};
	QRCodeParser.prototype.parse = function () {
		console.log(this.plainUrl);
		this.accountInfo = new AccountInfo();
		this.accountInfo.url = this.plainUrl;
		this.parts = this.accountInfo.url.split(/[/\\.-]/);
		if (this.parts[5].length <= 4) {
			oldCode = true;
		} else {
			oldCode = false;
		}
		this.count = this.parts.length;
		console.log(this.count, this.parts, this.parts[5].length);
//isCompany:true
//isPersonal:false
//memberId:"NEW"
//accountId:"NEW:AAD"
//securityCode:"51955e77234f285f58ebd3db39a6499b9eb5ad569e045dda909b4e897ffd1837"
//url:"HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw"
//serverType:"rc4"
//signin:1
//unencryptedCode:"utbYceW3KLLCcaw"
		if ((this.count === 6 || this.count === 7) && !oldCode) {
			var region = this.parts[2];
			if (this.parts[6]) {
				this.accountInfo.counter = this.parts[6];
				var tail = this.parts[5];
				console.log(this.accountInfo.counter, tail);
			} else if (this.parts[5].indexOf(':') > -1) {
				var tail = this.parts[5].split(':');
				this.accountInfo.counter = tail[1];
				tail = tail[0];
				console.log(this.accountInfo.counter, tail);
			} else {
				var tail = this.parts[5];
				console.log(this.accountInfo.counter, tail);
			}
			var fmt = tail.substring(0, 1);
			var acctLen = '';
			var i = parseInt(fmt, 36);
			var agentLen = i % 4;
			i = Math.floor(i / 4);
			console.log(i);
			var regionLen = parseInt(regionLens.charAt(i));
			acctLen = parseInt(acctLens.charAt(i));
			var account = r36ToR26(tail.substring(1, 1 + acctLen), acctLen);
			var memberId = '';
			console.log(tail, i, fmt, acctLen, agentLen, account, acctLens.charAt(i), regionLen);
			if (acctLen >= 6 || tail.length < 1 + acctLen + agentLen) {
				console.log('That is not a valid rCard.');
				throw 'That is not a valid rCard.';
			}
			this.accountInfo.unencryptedCode = tail.substring(1 + acctLen + agentLen);
			this.accountInfo.securityCode = Sha256.hash(this.accountInfo.unencryptedCode);
			this.accountInfo.isCompany = (agentLen > 0);
			this.accountInfo.isPersonal = !this.accountInfo.isCompany;
			region = r36ToR26(region, regionLen);
			if (this.accountInfo.isCompany) {
				this.accountInfo.signin = 1;
				this.accountInfo.accountId = region + account + '-' + r36ToR26(tail.substring(1 + acctLen, 1 + acctLen + agentLen), agentLen, true);
			} else {
				this.accountInfo.accountId = region + account;
				this.accountInfo.signin = 0;
			}
			memberId = region;
			this.accountInfo.memberId = memberId;
			console.log(this.accountInfo, agentLen, region, account, fmt);
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
		this.accountInfo.accountId = memberId +yyy;
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
	function r36ToR26(part, s2Len, isAgent) {
		console.log(part);
		var std = '0123456789abcdefghijklmnop';
		var ours = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
		var s = parseInt(part, 36).toString(26); // d4m
		var s2 = '';
		for (var i = 0; i < s.length; i++) {
			s2 += ours.charAt(std.indexOf(s.charAt(i)));
			console.log(s, s.charAt(i), std.indexOf(s.charAt(i)));
		}
		console.log(s2.length, s2Len, s.charAt(i), s2, s, part, s2Len);
		if (!isAgent) {
			while ((s2.length < s2Len) || (s2.length < 3)) {
				s2 = 'A' + s2;
			}
		}
		console.log(s2);
		return s2;
	}
	window.QRCodeParser = QRCodeParser;
})(window);