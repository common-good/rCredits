describe ('User Service', function() {

  'use strict';

  beforeEach (module ('rcredits'));
  beforeEach (function() {
    module (function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var userService, rootScope, httpBackend, localStorageService;
  var LOGIN_WITH_RCARD_ERROR_RESPONSE = {"ok": "0", "message": "bad agent NEW:AABs"};

  var SELLER_SCAN_RESULT =   {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};
  var CUSTOMER_SCAN_RESULT = {text: "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw", format: "QR_CODE", cancelled: false};

  var SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE = {
    "ok": "1",
    "logon": "1",
    "name": "Bob Bossman",
    "company": "Corner Store",
    "descriptions": ["groceries", "gifts", "sundries", "deli", "baked goods"],
    "can": 6019,
    "default": "NEW.AAB",
    "time": 1450364516
  };

  var CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE = {
    "ok": "1",
    "logon": "0",
    "name": "Susan Shopper",
    "place": "Montague, MA",
    "company": "",
    "balance": "1451.15",
    "rewards": "1020.77",
    "can": 131
  };

  beforeEach (inject (function(UserService, $rootScope, $httpBackend, _localStorageService_) {
    userService = UserService;
    httpBackend = $httpBackend;
    rootScope = $rootScope;
    localStorageService = _localStorageService_;

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

  describe ('Seller login withRcard', function() {


    it ('Should send correct member and code values based on input URL', function() {
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);

      userService.loginWithRCard(SELLER_SCAN_RESULT.text).then(function(seller) {
        expect (seller.name).toBe(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.name);
        expect (seller.company).toBe(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.company);
        expect (seller.can).toBe(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.can);
        expect (seller.default).toBe(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.default);
      });

      httpBackend.flush();
    });

    it ('Should handle error (i.e. ok = 0)', function() {
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(LOGIN_WITH_RCARD_ERROR_RESPONSE);
      userService.loginWithRCard(SELLER_SCAN_RESULT.text)
        .catch(function(err) {
          expect (err).toBe(LOGIN_WITH_RCARD_ERROR_RESPONSE.message);
        });

      httpBackend.flush();
    });

    it ("Should also set firstLogin: true on seller object if deviceId was not sent", function() {
      localStorageService.remove('deviceID');
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);

      userService.loginWithRCard(SELLER_SCAN_RESULT.text).then(function(seller) {
        expect (seller.firstLogin).toBe(true);
      });

      httpBackend.flush();
    });

    it ("Seller login error", function() {
      SELLER_SCAN_RESULT.logon = 0;
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);

      userService.loginWithRCard(SELLER_SCAN_RESULT.text)
        .catch(function(err) {
          expect (err).toBe(userService.LOGIN_SELLER_ERROR_MESSAGE);
        });

      httpBackend.flush();
    });

  });

  describe ('Identify Customer', function() {

    it ('Customer login', function() {
      userService.seller = userService.createSeller(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);
      userService.identifyCustomer(CUSTOMER_SCAN_RESULT.text).then(function(customer) {
        expect (customer.name).toBe(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.name);
        expect (customer.company).toBe(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.company);
        expect (customer.can).toBe(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.can);
        expect (customer.rewards).toBe(parseFloat (CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.rewards));
      });
      httpBackend.flush();
    });

    it ('Customer login with FIRST PURCHASE', function() {
      userService.seller = userService.createSeller(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);
      CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE.logon = '-1';
      httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);
      userService.identifyCustomer(CUSTOMER_SCAN_RESULT.text).then(function(customer) {
        expect (customer.firstPurchase).toBe(true);
      });
      httpBackend.flush();
    });

  });

});

