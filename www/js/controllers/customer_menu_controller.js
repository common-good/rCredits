app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, UserService, $ionicHistory, NotificationService, CashierModeService, PermissionService) {

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
    var chargeFn = function() {
      $state.go('app.transaction', {'transactionType': 'charge'});
    };

    if (CashierModeService.canCharge()) {
      chargeFn();
    } else {
      executeAction(chargeFn);
    }
  };


  $scope.openRefund = function() {
    //$state.go('app.transaction', {'transactionType': 'refund'});
    var refundFn = function() {
      $state.go('app.transaction', {'transactionType': 'refund'});
    };

    if (CashierModeService.canRefund()) {
      refundFn();
    } else {
      executeAction(refundFn);
    }
  };


  var executeAction = function(fn) {
    NotificationService.showConfirm({
      title: 'cashier_permission',
      subTitle: "",
      okText: "confirm",
      cancelText: "cancel"
    }, {}).then(function(res) {
      if (res) {
        return PermissionService.authorizeSeller()
          .then(function(authResult) {
            if (!authResult) {
              NotificationService.showAlert({title: 'cashier_permission_rejected'});
              return;
            }
            //$state.go('app.transaction', {'transactionType': 'charge'});
            fn();
          });
      }
    });
  };


});
