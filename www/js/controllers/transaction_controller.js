app.controller('TransactionCtrl', function($scope, $state, $stateParams,
  $ionicLoading, $filter, NotificationService, UserService,
  TransactionService) {

  $scope.transactionType = $stateParams.transactionType;
  $scope.amount = 0;

  // $scope.emptyAmount = function() {
  //   if ($scope.amount === 0 || $this.amount === 0){
  //     return true;
  //   };
  // }

  $scope.charge = function() {
    TransactionService.charge($scope.amount)
    .then(function(result) {
      var transactionAmount = $scope.amount;
      $state.go('app.transaction_result',
        {'transactionStatus': 'success', 'transactionAmount': transactionAmount});

      // Transaction Service or Transaction Controller makes call to User Service
      // User Service has updated rewards and balance values

      // Show success screen
      // Call user service to show updated balance
    })
    .catch(function(result) {
      console.log('Transaction fail');
      // Show failure screen
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
