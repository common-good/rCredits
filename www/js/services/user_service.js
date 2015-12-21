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
    return self.user;
  };

  // Gets the current customer. Returns an object
  // or null if there is no current customer.
  UserService.prototype.currentCustomer = function() {
    return {
      name: "Phillip Blivers", place: "Ann Arbor, MI", balance: 110.23,
      balanceSecret: false, rewards: 8.72, photo: "img/sample-customer.png", firstPurchase: true
    };
  };

  // Logs user in given the scanned info from an rCard.
  // Returns a promise that resolves when login is complete.
  // Current user should be retrieved using currentUser function above.
  UserService.prototype.loginWithRCard = function(str) {
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
      var responseData = res.data;

      if (responseData.ok === LOGIN_FAILED) {
        throw responseData.message;
      }

      if (responseData.logon === LOGIN_BY_AGENT) {
        self.user = self.createSeller(responseData);
        return self.user;
      }

      if (responseData.logon === LOGIN_BY_CUSTOMER) {
        throw self.LOGIN_SELLER_ERROR_MESSAGE;
      }

    }, function(res) {
      throw res.statusText;
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
    // Simulates a login. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;

    return $q (function(resolve, reject) {
      setTimeout (function() {
        if (SUCCEED) {
          resolve ();
        } else {
          reject("userLookupFailure");
        }
      }, 1000);
    });
  };

  // Logs the user out on the remote server.
  // Returns a promise that resolves when logout is complete, or rejects with error of fail.
  UserService.prototype.logout = function() {
    // Simulates logout. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;
    return $q(function(resolve, reject) {
        if (SUCCEED) {
          resolve();
        } else {
          reject("logoutFailure");
        }
    });
  };

  return new UserService ();
});
