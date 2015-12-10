app.service('UserService', function ($q) {

  var UserService = function () {
    self = this;
    this.user = null;
  };

  // Gets the current user. Returns the user object,
  // or null if there is no current user.
  UserService.prototype.currentUser = function() {
    return {name: "Andrea Green", company: "Tasty Soaps, Inc."}
  };

  // Logs user in given the scanned info from an rCard.
  // Returns a promise that resolves when login is complete.
  // If this is the first login, the promise will resolve with {firstLogin: true}
  // The app should then give notice to the user that the device is associated with the
  // user.
  UserService.prototype.loginWithRCard = function(str) {
    // Simulates a login. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;

    return $q(function(resolve, reject) {
      setTimeout(function() {
        if (SUCCEED) {
          if (window.localStorage.getItem('notfirstlogin')) {
            resolve();
          } else {
            resolve({firstLogin: true});
          }
          window.localStorage.setItem('notfirstlogin', true);
        } else {
          reject('Login failed.');
        }
      }, 1000);
    });
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

    return $q(function(resolve, reject) {
      setTimeout(function() {
        if (SUCCEED) {
          resolve(
            {name: "Phillip Blivers", place: "Ann Arbor, MI", balance: 110.23,
              balanceSecret: false, rewards: 8.72, photo: "img/sample-customer.png"},
            {firstPurchase: true}
          );
        } else {
          reject('User lookup failed.');
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
      setTimeout(function() {
        if (SUCCEED) {
          resolve();
        } else {
          reject('Logout failed.');
        }
      }, 1000);
    });
  }

  return new UserService();
});
