app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  $scope.currentUser = UserService.currentUser();

  if ($scope.currentUser.firstLogin) {
    $ionicPopup.alert({
      title: "This device is now associated with " + $scope.currentUser.company + ".",
      template: "To set your preferences, please see the main menu."
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
          $ionicPopup.confirm({
            templateUrl: "templates/first-purchase.html",
            scope: $scope,
            okText: "Confirm"
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
        $scope.showAlert(errorMsg);
        $ionicLoading.hide();
      });
    })
    .catch(function(errorMsg) {
      $scope.showAlert(errorMsg);
      $ionicLoading.hide();
    });
  };
});
