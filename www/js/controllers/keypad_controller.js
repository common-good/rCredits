app.controller('KeyPadCtrl', function($scope, $state) {
  $scope.amount = 0;

  $scope.addHundred = function(num) {
    value = $scope.amount * 100;
    checkValue(value);
  }

  $scope.addNumber = function(num) {
    value = ($scope.amount * 10) + (num / 100);
    checkValue(value);
  }

  $scope.clearEntry = function(key) {
    $scope.amount = 0;
  }

  var checkValue = function(num) {
    if (num < 1000000) {
      $scope.amount = num;
    }
  }
});
