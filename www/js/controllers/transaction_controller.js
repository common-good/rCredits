app.controller('TransactionCtrl', function($scope, $state, $stateParams,
  $ionicLoading, $filter, NotificationService, UserService) {

  $scope.transactionType = $stateParams.transactionType;

  $scope.charge = function(amount) {
  };

  $scope.refund = function(amount) {
  };


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
