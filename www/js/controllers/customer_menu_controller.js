app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, UserService, $ionicHistory, NotificationService) {

  $scope.customer = UserService.currentCustomer();

  $scope.showBalance = function() {
    if ($scope.customer.balanceSecret) {
      NotificationService.showAlert('balanceIsSecret');
    } else {
      NotificationService.alert({
        title: "Customer Balance",
        templateUrl: "templates/customer-balance.html",
        scope: $scope
      });
    }
  };

  $scope.hideLoading = function() {
    $ionicLoading.hide();
  };

  $scope.$on("$destroy", function() {
    $scope.customer = null;
  });
});
