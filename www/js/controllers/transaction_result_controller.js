app.controller('TransactionResultCtrl', function($scope, $state,
                                                 $stateParams, $ionicLoading, $filter, NotificationService, UserService,
                                                 TransactionService, BackButtonService, $timeout, SelfServiceMode) {

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

  $scope.setMessages = function(transactionResult) {
    $scope.note = transactionResult.message;
    if (transactionResult.txid) {
      $scope.heading = 'Successful';
    } else {
      $scope.heading = 'Unsuccessful';
    }
  };

  $scope.customer = UserService.currentCustomer();
  $scope.user = UserService.currentUser();

  // Keys for Translation
  $scope.heading = 'transaction' + statusKey + 'Heading';

  debugger
  $scope.setMessages(TransactionService.lastTransaction);

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
            debugger
            $scope.setMessages(transactionResult);
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

  $scope.isSelfServiceEnabled = function() {
    return SelfServiceMode.isActive();
  };

});
