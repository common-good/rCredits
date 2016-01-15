(function(window) {

  var AccountInfo = function() {
    this.isPersonal = false;
    this.isCompany = false;
    this.memberId = '';
    this.accountId = '';
    this.securityCode = '';
  };

  AccountInfo.prototype.isPersonalAccount = function() {
    return this.isPersonal;
  };

  AccountInfo.prototype.isCompanyAccount = function() {
    return this.isCompany;
  };

  AccountInfo.prototype.getMemberId = function() {
    return this.memberId;
  };


  window.AccountInfo = AccountInfo;

})(window);
