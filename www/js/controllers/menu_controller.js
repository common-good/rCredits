app.controller('MenuCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService) {

  // Logout

  $scope.logout = function() {
    $ionicLoading.show();

    UserService.logout()
      .then(function() {
        $state.go("app.login");
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
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

  $scope.redirectPreferences = function() {
    $state.go("app.preferences");
  };
});
