app.service('UserService', function($q, $http, $httpParamSerializer, RequestParameterBuilder, Seller) {

  var LOGIN_FAILED = '0';
  var LOGIN_BY_AGENT = '1';
  var LOGIN_BY_CUSTOMER = '0';
  var FIRST_PURCHASE = '-1';

  var UserService = function() {
    self = this;
    this.user = null;
    this.LOGIN_SELLER_ERROR_MESSAGE = 'Not a valid rCard for seller login';
  };


  // Gets the current user. Returns the user object,
  // or null if there is no current user.
  UserService.prototype.currentUser = function() {
    return {name: "Andrea Green", company: "Tasty Soaps, Inc.", firstLogin: true};
  };

  // Gets the current customer. Returns an object
  // or null if there is no current customer.
  UserService.prototype.currentCustomer = function() {
    return {
      name: "Phillip Blivers", place: "Ann Arbor, MI", balance: 110.23,
      balanceSecret: false, rewards: 8.72, photo: "img/sample-customer.png", firstPurchase: true
    };
  };

  UserService.prototype.loginWithRCard_ = function(str) {
    var qrcodeParser = new QRCodeParser ();
    qrcodeParser.setUrl(str);
    this.qrcodeInfo = qrcodeParser.parse();
    var params = new RequestParameterBuilder (this.qrcodeInfo).setOperationId('identify').getParams();

    return $http ({
      method: 'POST',
      url: rCreditsConfig.serverUrl,
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      data: $httpParamSerializer (params)
    }).then(function(res) {
      console.log("RESPONSE ", res);
      var responseData = res.data;

      if (responseData.ok === LOGIN_FAILED) {
        throw responseData.message;
      }
      return responseData;
    });
  };

  // Logs user in given the scanned info from an rCard.
  // Returns a promise that resolves when login is complete.
  // If this is the first login, the promise will resolve with {firstLogin: true}
  // The app should then give notice to the user that the device is associated with the
  // user.
  UserService.prototype.loginWithRCard = function(str) {
    return this.loginWithRCard_(str)
      .then(function(responseData) {
        if (responseData.logon === LOGIN_BY_AGENT) {
          return self.createSeller(responseData);
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

  UserService.prototype.createCustomer = function(customerInfo) {
    var customer = new Customer ();
  };

  // Gets customer info and photo given the scanned info from an rCard.
  // Returns a promise that resolves with the following arguments:
  // 1. user - The User object
  // 2. flags - A hash with the following elements:
  //      firstPurchase - Whether this is the user's first rCredits purchase. If so, the
  //        app should notify the seller to request photo ID.
  UserService.prototype.identifyCustomer = function(str) {
    return this.loginWithRCard_(str)
      .then(function(responseData) {
        if (responseData.logon === LOGIN_BY_CUSTOMER) {
          return self.createCustomer(responseData);
        }

        if (responseData.logon === LOGIN_BY_AGENT) {
          throw self.LOGIN_SELLER_ERROR_MESSAGE;
        }
      });
  };

  // Logs the user out on the remote server.
  // Returns a promise that resolves when logout is complete, or rejects with error of fail.
  UserService.prototype.logout = function() {
    // Simulates logout. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;
    return $q (function(resolve, reject) {
      if (SUCCEED) {
        resolve ();
      } else {
        reject ("logoutFailure");
      }
    });
  };

  return new UserService ();
});
