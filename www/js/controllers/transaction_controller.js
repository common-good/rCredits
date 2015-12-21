app.controller('TransactionCtrl', function($scope, $state, $stateParams,
  $ionicLoading, $filter, NotificationService, UserService) {

  $scope.transactionType = $stateParams.transactionType;

  $scope.charge = function() {
    // TransactionService.charge($scope.amount)
    // .then(function(result) {
    //   console.log('Transaction success');
    //   console.log(result);

    //   $state.go('app.transaction_result',
    //     {'transactionStatus': 'success', 'amount': result.amount});

    //   console.log(result.amount);
    //   // Transaction Service or Transaction Controller makes call to User Service
    //   // User Service has updated rewards and balance values

    //   // Show success screen
    //   // Call user service to show updated balance
    // })
    // .catch(function(result) {
    //   console.log('Transaction fail');
    //   // Show failure screen
    // });
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

  $scope.transactionStatus = $stateParams.transactionStatus;

  var statusKey;
  $scope.success = false;

  // New key gets used to in transactionInfo for translation
  if ($scope.transactionStatus == 'success') {
    statusKey = 'Success';
    $scope.success = true;
  } else {
  }

  $scope.customer = UserService.currentCustomer();
  $scope.user = UserService.currentUser();

  // Keys for Translation
  $scope.heading = 'transaction' + statusKey + 'Heading';
  $scope.note = 'transaction' + statusKey + 'Note';

  $scope.transactionInfo = {
    amount: $filter('currency')(0),
    company: $scope.user.company,
    customerName: $scope.customer.name,
  }
});
