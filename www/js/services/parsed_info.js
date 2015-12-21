(function(window) {

  var ParsedInfo = function() {
    this.isPersonal = false;
    this.isCompany = false;
    this.accountId = '';
    this.securityCode = '';
  };

  ParsedInfo.prototype.isPersonalAccount = function() {
    return this.isPersonal;
  };

  ParsedInfo.prototype.isCompanyAccount = function() {
    return this.isCompany;
  };


  window.ParsedInfo = ParsedInfo;

}) (window);
