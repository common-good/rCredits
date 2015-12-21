describe ('User Service', function() {

  'use strict';

  beforeEach (module ('rcredits'));
  beforeEach (function() {
    module (function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var userService, rootScope, httpBackend;
  var SCAN_RESULT = {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};

  var LOGIN_WITH_RCARD_ERROR_RESPONSE = {"ok": "0", "message": "bad agent NEW:AABs"};
  var LOGIN_WITH_RCARD_SUCESS_RESPONSE = {
    "ok": "1",
    "logon": "1",
    "name": "Bob Bossman",
    "company": "Corner Store",
    "descriptions": ["groceries", "gifts", "sundries", "deli", "baked goods"],
    "can": 6019,
    "default": "NEW.AAB",
    "time": 1450364516
  };

  beforeEach (inject (function(UserService, $rootScope, $httpBackend) {
    userService = UserService;
    httpBackend = $httpBackend;
    rootScope = $rootScope;

    $httpBackend.whenGET(/templates\/*/).respond(function(method, url, data, headers) {
      return [200, '<div></div>'];
    });

    $httpBackend.whenGET(/js\/languages\/definitions\//).respond(function(method, url, data, headers) {
      return [200, {}];
    });

  }));

  afterEach (function() {
    httpBackend.verifyNoOutstandingExpectation();
    httpBackend.verifyNoOutstandingRequest();
  });

  it ('Should send correct member and code values based on input URL', function() {
    httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(LOGIN_WITH_RCARD_SUCESS_RESPONSE);

    userService.loginWithRCard(SCAN_RESULT.text).then(function(seller) {
      expect (seller.name).toBe(LOGIN_WITH_RCARD_SUCESS_RESPONSE.name);
      expect (seller.company).toBe(LOGIN_WITH_RCARD_SUCESS_RESPONSE.company);
      expect (seller.can).toBe(LOGIN_WITH_RCARD_SUCESS_RESPONSE.can);
      expect (seller.default).toBe(LOGIN_WITH_RCARD_SUCESS_RESPONSE.default);
    });

    httpBackend.flush();
  });

  it ('Should handle error (i.e. ok = 0)', function() {
    httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(LOGIN_WITH_RCARD_ERROR_RESPONSE);
    userService.loginWithRCard(SCAN_RESULT.text)
      .catch(function(err) {
        expect (err).toBe(LOGIN_WITH_RCARD_ERROR_RESPONSE.message);
      });

    httpBackend.flush();
  });

  it ("Should also set firstLogin: true on seller object if deviceid was not sent", function() {
    httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(LOGIN_WITH_RCARD_SUCESS_RESPONSE);

    userService.loginWithRCard(SCAN_RESULT.text).then(function(seller) {
      expect (seller.firstLogin).toBe(true);
    });

    httpBackend.flush();
  });

  it ("Seller login error", function() {
    SCAN_RESULT.logon = 0;
    httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(LOGIN_WITH_RCARD_SUCESS_RESPONSE);

    userService.loginWithRCard(SCAN_RESULT.text)
      .catch(function(err) {
        expect (err).toBe(userService.LOGIN_SELLER_ERROR_MESSAGE);
      });

    httpBackend.flush();
  });

});

