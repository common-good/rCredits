app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, UserService, $ionicHistory, NotificationService) {

  $scope.customer = UserService.currentCustomer();

  $scope.showBalance = function() {
    if ($scope.customer.balanceSecret) {
      NotificationService.showAlert('balanceIsSecret');
    } else {
      NotificationService.showAlert({
        scope: $scope,
        title: 'customerBalance',
        templateUrl: 'templates/customer-balance.html'
      });
    }
  };

  $scope.hideLoading = function() {
    $ionicLoading.hide();
  };

  $scope.$on('$destroy', function() {
    $scope.customer = null;
  });

  $scope.openCharge = function() {
    $state.go('app.transaction', {'transactionType': 'charge'});
  };

  $scope.openRefund = function() {
    $state.go('app.transaction', {'transactionType': 'refund'});
  };
});
