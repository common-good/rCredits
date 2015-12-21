(function(window) {

  var AccountInfo = function() {
    this.isPersonal = false;
    this.isCompany = false;
    this.accountId = '';
    this.securityCode = '';
  };

  AccountInfo.prototype.isPersonalAccount = function() {
    return this.isPersonal;
  };

  AccountInfo.prototype.isCompanyAccount = function() {
    return this.isCompany;
  };


  window.AccountInfo = AccountInfo;

}) (window);
