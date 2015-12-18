app.controller('KeyPadCtrl', function($scope, $state) {
  $scope.amount = 0;

  $scope.addHundred = function(num) {
    setEntry($scope.amount * 100);
  }

  $scope.addNumber = function(num) {
    setEntry(($scope.amount * 10) + (num / 100));
  }

  $scope.clearEntry = function() {
    $scope.amount = 0;
  }

  var setEntry = function(num) {
    if (num < 1000000) {
      $scope.amount = num;
    }
  }
});
