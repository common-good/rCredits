app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService) {

  $scope.currentUser = UserService.currentUser();

  if (!$scope.currentUser) {
    $state.go("app.login");
    return;
  }

  if ($scope.currentUser.firstLogin) {
    NotificationService.showAlert({
      scope: $scope,
      title: 'deviceAssociated',
      template: 'toSetPreferences'
    },
    {
      company: $scope.currentUser.company
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
                  title: 'firstPurchase',
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
              $ionicLoading.hide();
              $state.go("app.customer");
            };
          })
          .catch(function(errorMsg) {
            NotificationService.showAlert({title: "error", template: errorMsg});
            $ionicLoading.hide();
          });
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
        $ionicLoading.hide();
      });
  };
});
