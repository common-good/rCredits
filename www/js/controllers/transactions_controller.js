app.controller('TransactionsCtrl', function($scope, $state, $stateParams, $ionicLoading, NotificationService) {
  $scope.transactionType = $stateParams.transactionType;

  $scope.charge = function(amount) {
  };

  $scope.refund = function(amount) {
  };
});
