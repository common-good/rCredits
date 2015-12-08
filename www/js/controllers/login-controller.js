app.controller('UserCtrl', function($scope, $state, $ionicLoading) {
  // console.log("This is the login controller");

  var loggedIn = false;

  var messages = {
    "loginSuccess": "You successfully logged in.",
    "loginFailure": "You were not logged in.",
    "logoutSuccess": "You were successfully logged out.",
    "logoutFailure": "Logout was unsuccessful. Please try again."
  }

  $scope.openScanner = function(){
    $scope.login();
  };

  // Login

  $scope.login = function(){
    $ionicLoading.show();

    setTimeout(function(){

      loggedIn = true;

      if (loggedIn) {
        $scope.loginSuccess();
      } else {
        $scope.loginError();
      }

    }, 2000);
  };

  $scope.loginSuccess = function(){
    $scope.redirectHome();
    $scope.showMessage(messages.loginSuccess);
  };

  $scope.loginError = function(){
    $scope.showMessage(messages.loginFailure);
  }

  // Logout

  $scope.logout = function(){
    $ionicLoading.show();

    setTimeout(function(){

      loggedIn = false;

      if (loggedIn){
        $scope.logoutError();
      } else {
        $scope.logoutSuccess();
      }

    }, 1000);
  };

  $scope.logoutSuccess = function(){
    $scope.redirectToLogin();
    $scope.showMessage(messages.logoutSuccess);
  };

  $scope.logoutError = function(){
    $scope.showMessage(messages.logoutFailure);
  }

  // Redirects

  $scope.redirectHome = function(){
    $state.go("app.home");
  };

  $scope.redirectToLogin = function(){
    $state.go("app.rlogin");
  };

  // Show Messages

  $scope.showMessage = function(message){
    $ionicLoading.hide();
    console.log(message);
  };

  $scope.showUserModal = function(text){
    $ionicLoading.hide();
  };
});
