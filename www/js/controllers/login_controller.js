app.controller('LoginCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService, CashierModeService, $stateParams) {

  $scope.$on('$ionicView.loaded', function() {
    ionic.Platform.ready(function() {
      if (navigator && navigator.splashscreen) navigator.splashscreen.hide();
    });
  });

  // Scanner Login
  $ionicHistory.clearHistory();
  $scope.openScanner = function() {
    $ionicLoading.show();

    BarcodeService.scan()
      .then(function(str) {
        UserService.loginWithRCard(str)
          .then(function() {
            $ionicHistory.nextViewOptions({
              disableBack: true
            });
            $state.go("app.home");
          })
          .catch(function(errorMsg) {
            NotificationService.showAlert({title: "error", template: errorMsg});
          })
          .finally(function() {
            $ionicLoading.hide();
          });
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
        $ionicLoading.hide();
      });
  };

  if (CashierModeService.isEnabled() || $stateParams.openScanner) {
    $scope.openScanner();
  }

});
