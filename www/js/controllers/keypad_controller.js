app.controller('KeyPadCtrl', function($scope, $state) {
  $scope.$parent.amount = 0;

  $scope.addHundred = function(num) {
    setEntry($scope.$parent.amount * 100);
  }

  $scope.addNumber = function(num) {
    setEntry(($scope.$parent.amount * 10) + (num / 100));
  }

  $scope.clearEntry = function() {
    $scope.$parent.amount = 0;
  }

  var setEntry = function(num) {
    if (num < 1000000) {
      $scope.$parent.amount = num;
    }
  }
});
