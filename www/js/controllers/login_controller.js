app.controller('LoginCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService) {

  // Scanner Login

  $scope.openScanner = function() {
    $ionicLoading.show();

    BarcodeService.scan()
      .then(function(str) {
        UserService.loginWithRCard(str)
          .then(function() {
            $ionicHistory.nextViewOptions({
              disableBack: true
            });

            $scope.redirectHome();
          })
          .catch(function(errorMsg) {
            NotificationService.showAlert(errorMsg);
          })
          .finally(function() {
            $ionicLoading.hide();
          });
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert(errorMsg);
        $ionicLoading.hide();
      });
  };
});
