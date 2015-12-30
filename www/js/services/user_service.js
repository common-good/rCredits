app.service('UserService', function($q, $http, $httpParamSerializer, RequestParameterBuilder, Seller, Customer) {

  'use strict';

  var self;

  var LOGIN_FAILED = '0';
  var LOGIN_BY_AGENT = '1';
  var LOGIN_BY_CUSTOMER = '0';
  var FIRST_PURCHASE = '-1';

  var self;
  var UserService = function() {
    self = this;
    this.seller = null;
    this.LOGIN_SELLER_ERROR_MESSAGE = 'Not a valid rCard for seller login';
  };

  // Gets the current user. Returns the user object,
  // or null if there is no current user.
  UserService.prototype.currentUser = function() {
    return this.seller;
  };

  // Gets the current customer. Returns an object
  // or null if there is no current customer.
  UserService.prototype.currentCustomer = function() {
    return self.customer;
  };

  UserService.prototype.makeRequest_ = function(params) {
    return $http ({
      method: 'POST',
      url: rCreditsConfig.serverUrl,
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      data: $httpParamSerializer (params)
    });
  };

  UserService.prototype.loginWithRCard_ = function(params) {
    return this.makeRequest_(params).then(function(res) {
        var responseData = res.data;

        if (responseData.ok === LOGIN_FAILED) {
          throw responseData.message;
        }
        return responseData;
      })
      .catch(function(err) {
        if (_.isString(err)) {
          throw err
        }
        throw err.statusText;
      });
  };

  // Logs user in given the scanned info from an rCard.
  // Returns a promise that resolves when login is complete.
  // If this is the first login, the promise will resolve with {firstLogin: true}
  // The app should then give notice to the user that the device is associated with the
  // user.
  UserService.prototype.loginWithRCard = function(str) {
    var qrcodeParser = new QRCodeParser ();
    qrcodeParser.setUrl(str);
    var accountInfo = qrcodeParser.parse();
    var params = new RequestParameterBuilder ()
      .setOperationId('identify')
      .setSecurityCode(accountInfo.securityCode)
      .setMember(accountInfo.accountId)
      .getParams();

    return this.loginWithRCard_(params)
      .then(function(responseData) {
        if (responseData.logon === LOGIN_BY_AGENT) {
          self.seller = self.createSeller(responseData);
          return self.seller;
        }

        if (responseData.logon === LOGIN_BY_CUSTOMER) {
          throw self.LOGIN_SELLER_ERROR_MESSAGE;
        }
      });
  };

  UserService.prototype.createSeller = function(sellerInfo) {
    var props = ['can', 'descriptions', 'company', 'default', 'time'];
    var seller = new Seller (sellerInfo.name);

    _.each(props, function(p) {
      seller[p] = sellerInfo[p];
    });

    if (!seller.hasDevice()) {
      if (seller.isValidDeviceId(sellerInfo.device)) {
        seller.setDeviceId(sellerInfo.device);
      } else {
        seller.firstLogin = true;
      }
    }

    return seller;
  };

  // Gets customer info and photo given the scanned info from an rCard.
  // Returns a promise that resolves with the following arguments:
  // 1. user - The User object
  // 2. flags - A hash with the following elements:
  //      firstPurchase - Whether this is the user's first rCredits purchase. If so, the
  //        app should notify the seller to request photo ID.
  UserService.prototype.identifyCustomer = function(str) {
    var qrcodeParser = new QRCodeParser ();
    qrcodeParser.setUrl(str);
    var accountInfo = qrcodeParser.parse();
    var params = new RequestParameterBuilder ()
      .setOperationId('identify')
      .setAgent(this.seller.default)
      .setMember(accountInfo.accountId)
      .setSecurityCode(accountInfo.securityCode)
      .getParams();
    return this.loginWithRCard_(params)
      .then(function(responseData) {
        if (responseData.logon === LOGIN_BY_CUSTOMER || responseData.logon === FIRST_PURCHASE) {
          self.customer = self.createCustomer(responseData);

          if (responseData.logon === FIRST_PURCHASE) {
            self.customer.firstPurchase = true;
          }

          return self.customer;
        }

        if (responseData.logon === LOGIN_BY_AGENT) {
          throw self.LOGIN_SELLER_ERROR_MESSAGE;
        }
      })
      //.then(function(customer) {
      //  return self.getProfilePicture(accountInfo.accountId, accountInfo.securityCode);
      //})
      //.then(function(binaryImage){
      //  self.customer.photo = binaryImage;
      //})
  };

  UserService.prototype.createCustomer = function(customerInfo) {
    var props = ['balance', 'can', 'company', 'place'];
    var customer = new Customer (customerInfo.name);
    customer.setRewards(customerInfo.rewards);

    _.each(props, function(p) {
      customer[p] = customerInfo[p];
    });

    return customer;
  };

  UserService.prototype.getProfilePicture = function(member, securityCode) {
    var params = new RequestParameterBuilder ()
      .setOperationId('photo')
      .setAgent(this.seller.default)
      .setMember(member)
      .setSecurityCode(securityCode)
      .getParams();

    return this.makeRequest_(params)
      .then(function(res) {
        console.log(res);
        return res.data
      })
      .catch(function(err) {
        console.error(err);
        throw err;
      })
  };

  // Logs the user out on the remote server.
  // Returns a promise that resolves when logout is complete, or rejects with error of fail.
  UserService.prototype.logout = function() {
    // Simulates logout. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;
    return $q (function(resolve, reject) {
      if (SUCCEED) {
        self.customer = null;
        self.seller = null;
        resolve ();
      } else {
        reject ("logoutFailure");
      }
    });
  };

  return new UserService ();
});
