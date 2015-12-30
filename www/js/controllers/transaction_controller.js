app.controller('TransactionCtrl', function($scope, $state, $stateParams,
  $ionicLoading, $filter, NotificationService, UserService,
  TransactionService) {

  $scope.transactionType = $stateParams.transactionType;
  $scope.amount = 0;

  $scope.disableTransaction = function() {
    if ($scope.amount === 0){
      return true;
    }
  }

  $scope.charge = function() {
    $ionicLoading.show();

    var transactionAmount = $scope.amount;

    TransactionService.charge(transactionAmount)
    .then(function (result) {
      $state.go('app.transaction_result',
        {'transactionStatus': 'success', 'transactionAmount': transactionAmount});
      $ionicLoading.hide();
    }, function (errorMsg) {
      $state.go('app.transaction_result',
        {'transactionStatus': 'failure', 'transactionAmount': transactionAmount});
      $ionicLoading.hide();
    })
    .catch(function(errorMsg) {
      NotificationService.showAlert({title: "error", template: errorMsg});
    });
  };

  $scope.refund = function(amount) {
  };

  $scope.initiateTransaction = function() {
    if ($scope.transactionType == 'charge') {
      $scope.charge();
    } else {
      scope.refund();
    }
  }
});
