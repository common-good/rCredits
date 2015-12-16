app.controller('MenuCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory, NotificationService) {

  // Logout

  $scope.logout = function() {
    $ionicLoading.show();

    UserService.logout()
      .then(function() {
        $scope.redirectToLogin();
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert(errorMsg);
      })
      .finally(function() {
        $ionicLoading.hide();
      });
  };

  // Redirects

  $scope.redirectToLogin = function() {
    $state.go("app.login");
  };

  $scope.redirectHome = function() {
    $state.go("app.home");
  };

});
