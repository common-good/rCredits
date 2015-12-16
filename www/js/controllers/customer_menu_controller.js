app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, UserService, $ionicHistory, NotificationService) {

  $scope.customer = UserService.currentCustomer();

  $scope.showBalance = function() {
    if ($scope.customer.balanceSecret) {
      NotificationService.showAlert("This customer's balance is secret.");
    } else {
      NotificationService.alert({
        title: "Balance for",
        subTitle: $scope.customer.name,
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
