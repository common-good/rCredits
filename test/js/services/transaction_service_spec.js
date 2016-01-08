describe('Transaction Service', function() {

  'use strict';

  beforeEach(module('rcredits'));
  beforeEach(function() {
    module(function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var userService, rootScope, httpBackend, localStorageService, transactionService;
  var customer, seller, request;

  var LOGIN_WITH_RCARD_ERROR_RESPONSE = {"ok": "0", "message": "bad agent NEW:AABs"};

  var SELLER_SCAN_RESULT = {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};
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

  var TRANSACTION_RESPONSE_OK = {
    "ok": "1",
    "message": "You charged Susan Shopper $0.05 for goods and services. Your reward is $0.01.",
    "txid": "35026",
    "created": "1452101561",
    "balance": "1450.83",
    "rewards": 1020.81,
    "did": "\n\nWe just charged $0.05 .",
    "undo": "Undo transfer of $0.05 from Susan Shopper?"
  };

  var TRANSACTION_RESPONSE_ERROR = {
    "ok": "0",
    "message": "You already just charged that member that much. Wait a few minutes or type a different amount."
  };

  beforeEach(inject(function(UserService, $rootScope, $httpBackend, _TransactionService_) {
    userService = UserService;
    httpBackend = $httpBackend;
    rootScope = $rootScope;
    transactionService = _TransactionService_;

    $httpBackend.whenGET(/templates\/*/).respond(function(method, url, data, headers) {
      return [200, '<div></div>'];
    });

    $httpBackend.whenGET(/js\/languages\/definitions\//).respond(function(method, url, data, headers) {
      return [200, {}];
    });

  }));

  afterEach(function() {
    httpBackend.verifyNoOutstandingExpectation();
    httpBackend.verifyNoOutstandingRequest();
  });

  // Logs in the Seller and the Customer
  beforeEach(function() {
    request = httpBackend.whenPOST(rCreditsConfig.serverUrl).respond(SELLER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);

    userService.loginWithRCard(SELLER_SCAN_RESULT.text).then(function(sellerResponse) {
      seller = sellerResponse;

      request.respond(CUSTOMER_LOGIN_WITH_RCARD_SUCESS_RESPONSE);
      userService.identifyCustomer(CUSTOMER_SCAN_RESULT.text)
        .then(function(customerResponse) {
          customer = customerResponse;
        });
    });

    httpBackend.flush();
  });

  describe('Charge', function() {

    it('Should create a Transaction given a transaction response', function() {
      var transaction = transactionService.parseTransaction_(TRANSACTION_RESPONSE_OK);
      expect(transaction.getId()).toBe(TRANSACTION_RESPONSE_OK.txid);
      expect(transaction.created).toBe(TRANSACTION_RESPONSE_OK.created);
      expect(transaction.did).toBe(TRANSACTION_RESPONSE_OK.did);
      expect(transaction.undo).toBe(TRANSACTION_RESPONSE_OK.undo);
      expect(transaction.message).toBe(TRANSACTION_RESPONSE_OK.message);
    });

    it('Should charge and return a Transaction Object', function() {
      request.respond(TRANSACTION_RESPONSE_OK);
      transactionService.charge(0.12, 'description').then(function(transaction) {
        expect(transaction.getId()).toBe(TRANSACTION_RESPONSE_OK.txid);
        expect(transaction.created).toBe(TRANSACTION_RESPONSE_OK.created);
        expect(transaction.did).toBe(TRANSACTION_RESPONSE_OK.did);
        expect(transaction.undo).toBe(TRANSACTION_RESPONSE_OK.undo);

        expect(transaction.description).toBe('description');
        expect(transaction.amount).toBe(0.12);
        expect(transaction.goods).toBe(1);
      });

      httpBackend.flush();
    });

    it('Should charge and update customer reward and balance', function() {
      request.respond(TRANSACTION_RESPONSE_OK);
      transactionService.charge(0.12, 'description').then(function(transaction) {
        expect(customer.rewards).toBe(TRANSACTION_RESPONSE_OK.rewards);
        expect(customer.balance).toBe(TRANSACTION_RESPONSE_OK.balance);
      });

      httpBackend.flush();
    });

    it('Charge transaction should fail', function() {
      request.respond(TRANSACTION_RESPONSE_ERROR);
      transactionService.charge(0.12, 'description').catch(function(err) {
        expect(err.message).toBe(TRANSACTION_RESPONSE_ERROR.message);
      });

      httpBackend.flush();
    });

  });

  describe('Refund', function() {
    it('Should charge and return a Transaction Object', function() {
      request.respond(TRANSACTION_RESPONSE_OK);
      transactionService.refund(0.12, 'description').then(function(transaction) {
        expect(transaction.getId()).toBe(TRANSACTION_RESPONSE_OK.txid);
        expect(transaction.created).toBe(TRANSACTION_RESPONSE_OK.created);
        expect(transaction.did).toBe(TRANSACTION_RESPONSE_OK.did);
        expect(transaction.undo).toBe(TRANSACTION_RESPONSE_OK.undo);
      });

      httpBackend.flush();
    });

  });

});

