app.controller('TransactionResultCtrl', function($scope, $state,
  $stateParams, $ionicLoading, $filter, NotificationService, UserService,
  TransactionService) {

  $scope.transactionStatus = $stateParams.transactionStatus;
  $scope.transactionAmount = $stateParams.transactionAmount;

  var statusKey;
  $scope.success = false;

  // New key gets used in transactionInfo for translation
  if ($scope.transactionStatus === 'success') {
    statusKey = 'Success';
    $scope.success = true;
  } else if ($scope.transactionStatus == 'failure') {
    statusKey = 'LowFunds';
  }

  $scope.customer = UserService.currentCustomer();
  $scope.user = UserService.currentUser();

  // Keys for Translation
  $scope.heading = 'transaction' + statusKey + 'Heading';
  $scope.note = 'transaction' + statusKey + 'Note';

  $scope.transactionInfo = {
    amount: $filter('currency')($scope.transactionAmount),
    company: $scope.user.company,
    customerName: $scope.customer.name,
  }
});
//
