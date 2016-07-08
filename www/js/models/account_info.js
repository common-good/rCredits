(function (window) {
	var AccountInfo = function () {
		this.isPersonal = false;
		this.isCompany = false;
		this.memberId = '';
		this.accountId = '';
		this.securityCode = '';
		this.url = '';
		this.serverType = '';
		this.signin = '';
	};
	AccountInfo.IS_DEMO = 'rc4';
	AccountInfo.prototype.isPersonalAccount = function () {
		return this.isPersonal;
	};
	AccountInfo.prototype.isCompanyAccount = function () {
		return this.isCompany;
	};
	AccountInfo.prototype.getMemberId = function () {
		return this.memberId;
	};
	AccountInfo.prototype.isDemo = function () {
		return this.serverType === AccountInfo.IS_DEMO;
	};
	window.AccountInfo = AccountInfo;
})(window);
