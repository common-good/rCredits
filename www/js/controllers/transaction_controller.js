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
    var transactionAmount = $scope.amount;

    TransactionService.charge(transactionAmount)
    .then(function(result) {
      $state.go('app.transaction_result',
        {'transactionStatus': 'success', 'transactionAmount': transactionAmount});

      // Transaction Service or Transaction Controller makes call to User Service
      // User Service has updated rewards and balance values

      // Show success screen
      // Call user service to show updated balance
    })
    .catch(function(result) {
      $state.go('app.transaction_result',
        {'transactionStatus': 'failure', 'transactionAmount': transactionAmount});
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
