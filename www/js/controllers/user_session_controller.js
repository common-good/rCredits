app.controller('UserSessionCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {
  var loggedIn = false;

  var messages = {
    "loginSuccess": "You successfully logged in.",
    "loginFailure": "You were not logged in.",
    "logoutSuccess": "You were successfully logged out.",
    "logoutFailure": "Logout was unsuccessful. Please try again."
  }

  // Scanner Login

  $scope.openScanner = function(){
    $ionicLoading.show();

    BarcodeService.scan()
    .then(function(str){
      UserService.loginWithRCard(str)
      .then(function(){
        $ionicHistory.nextViewOptions({
          disableBack: true
        });

        $scope.redirectHome();
      })
      .catch(function(errorMsg){
        $scope.showAlert(errorMsg);
      })
      .finally(function(){
        $ionicLoading.hide();
      });
    })
    .catch(function(errorMsg){
      $scope.showAlert(errorMsg);
      $ionicLoading.hide();
    });
  };

  // Logout

  $scope.logout = function(){
    $ionicLoading.show();

    UserService.logout()
    .then(function(str){
      $scope.redirectToLogin();
    })
    .catch(function(errorMsg){
      $scope.showAlert(errorMsg);
    })
    .finally(function(){
      $ionicLoading.hide();
    });
  };

  // Redirects

  $scope.redirectHome = function(){
    $state.go("app.home");
  };

  $scope.redirectToLogin = function(){
    $state.go("app.login");
  };

  // Alert

  $scope.showAlert = function(message) {
    $ionicLoading.hide();
    $ionicPopup.alert({
      template: message
    });
  };
});
