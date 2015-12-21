app.controller('MenuCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService) {

  // Logout

  $scope.logout = function() {
    $ionicLoading.show();

    UserService.logout()
      .then(function() {
        $state.go("app.login");
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert(errorMsg);
      })
      .finally(function() {
        $ionicLoading.hide();
      });
  };
});
