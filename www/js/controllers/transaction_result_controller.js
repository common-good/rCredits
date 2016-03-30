app.controller('TransactionResultCtrl', function($scope, $state,
                                                 $stateParams, $ionicLoading, $filter, NotificationService, UserService,
                                                 TransactionService, BackButtonService, $timeout) {

  $scope.transactionStatus = $stateParams.transactionStatus;
  $scope.transactionAmount = $stateParams.transactionAmount;

  BackButtonService.disable();

  var statusKey;
  $scope.success = false;
  $scope.timeCan = true;

  // Enable UNDO btn for 1 min
  $timeout(function() {
    $scope.timeCan = false;
  }, 60 * 1000);

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

  if (TransactionService.lastTransaction) {
    if (TransactionService.lastTransaction.isRefund()) {
      $scope.note = 'transactionRefund' + statusKey + 'Note';
    } else {
      $scope.note = 'transaction' + statusKey + 'Note';
    }
  }

  $scope.transactionInfo = {
    amount: $filter('currency')($scope.transactionAmount),
    company: $scope.user.company,
    customerName: $scope.customer.name
  };

  $scope.undoTransaction = function() {
    NotificationService.showConfirm({
      title: 'confirm_undo_transaction',
      subTitle: "",
      okText: "yes",
      cancelText: "no"
    }).then(function(res) {
      if (res == true) {
        $ionicLoading.show();
        TransactionService.undoTransaction(TransactionService.lastTransaction)
          .then(function(transactionResult) {
            $scope.note = 'transactionUndoSuccessNote';
            $scope.undo = true;
          })
          .finally(function() {
            $ionicLoading.hide();
          });
      }
    });
  };

  $scope.$on('$destroy', function() {
    BackButtonService.enable();
  });

});
