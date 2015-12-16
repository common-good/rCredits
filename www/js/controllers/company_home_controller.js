app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService) {

  $scope.currentUser = UserService.currentUser();

  if ($scope.currentUser.firstLogin) {
    NotificationService.showAlert({
      title: "deviceAssociated", // Needs work
      template: "toSetPreferences"
    });
  }

  $scope.scanCustomer = function() {
    $ionicLoading.show();

    BarcodeService.scan()
      .then(function(id) {
        UserService.identifyCustomer(id)
          .then(function() {
            $scope.customer = UserService.currentCustomer();

            if ($scope.customer.firstPurchase) {
              NotificationService.showConfirm({
                  templateUrl: "templates/first-purchase.html",
                  scope: $scope,
                  okText: "confirm"
                })
                .then(function(confirmed) {
                  if (confirmed) {
                    $ionicLoading.show();
                    $state.go("app.customer");
                  }
                });
              $ionicLoading.hide();
            } else {
              $state.go("app.customer");
            };
          })
          .catch(function(errorMsg) {
            NotificationService.showAlert(errorMsg);
            $ionicLoading.hide();
          });
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert(errorMsg);
        $ionicLoading.hide();
      });
  };
});
