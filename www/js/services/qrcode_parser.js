(function(window) {

  var COMPANY_INDICATOR = '-';
  var PERSONAL_INDICATOR = '.';

  var QRCodeParser = function(url) {
  };

  QRCodeParser.prototype.setUrl = function(url) {
    this.url = new URL (url);
  };

  QRCodeParser.prototype.parse = function() {
    this.parsedInfo = new ParsedInfo ();
    this.parseAccountType_();
    this.parseAccountCode_();
    this.parseSecurityCode_();
  };

  QRCodeParser.prototype.getParsedInfo = function() {
    return this.parsedInfo;
  };

  QRCodeParser.prototype.parseAccountType_ = function() {
    if (this.url.pathname.indexOf(COMPANY_INDICATOR) !== -1) {
      this.parsedInfo.isCompany = true;
    } else if (this.url.pathname.indexOf(PERSONAL_INDICATOR) !== -1) {
      this.parsedInfo.isPersonal = true;
    } else {
      throw 'Unable to detect Account type: ' + this.url.href;
    }
  };

  QRCodeParser.prototype.parseAccountCode_ = function() {
    var xxx = this.url.hostname.toUpperCase().split('.')[0];
    var yyy = this.url.pathname.substring(1, 4);
    var separator;
    if (this.parsedInfo.isPersonalAccount()) {
      separator = PERSONAL_INDICATOR;
    } else {
      separator = COMPANY_INDICATOR;
    }
    this.parsedInfo.accountType = xxx + separator + yyy;
  };

  QRCodeParser.prototype.parseSecurityCode_ = function() {
    this.parsedInfo.securityCode = this.url.pathname.substr(5, this.url.pathname.length - 1);
  };


  window.QRCodeParser = QRCodeParser;

}) (window);
