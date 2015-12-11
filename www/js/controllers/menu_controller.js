app.controller('MenuCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

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

  $scope.redirectToLogin = function(){
    $state.go("app.login");
  };

  $scope.redirectHome = function(){
    $state.go("app.home");
  };

  // Alert

  $scope.showAlert = function(message) {
    $ionicLoading.hide();
    $ionicPopup.alert({
      template: message
    });
  };
});
