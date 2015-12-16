describe ('QRCode Parser', function() {

  'use strict';

  var INDIVIDUAL_ACCOUNT = "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw";
  var COMPANY_ACCOUNT = "HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G";

  var qrcodeParser;
  beforeEach (function() {
    qrcodeParser = new QRCodeParser ();
  });


  it ('Account type must be Company', function() {
    qrcodeParser.setUrl(COMPANY_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().isCompanyAccount()).toBe(true);
  });

  it ('Account type must be Personal', function() {
    qrcodeParser.setUrl(INDIVIDUAL_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().isPersonalAccount()).toBe(true);
  });

  it ('Parse Company Account Type', function() {
    qrcodeParser.setUrl(COMPANY_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().accountType).toEqual('NEW-AAB');
  });

  it ('Parse Individual Account Type', function() {
    qrcodeParser.setUrl(INDIVIDUAL_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().accountType).toEqual('NEW.ABB');
  });

  it ('Parse Company Security Code', function() {
    qrcodeParser.setUrl(COMPANY_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().securityCode).toEqual('WeHlioM5JZv1O9G');
  });

  it ('Parse Individual Security Code', function() {
    qrcodeParser.setUrl(INDIVIDUAL_ACCOUNT);
    qrcodeParser.parse();
    expect (qrcodeParser.getParsedInfo().securityCode).toEqual('ZzhWMCq0zcBowqw');
  });


});

