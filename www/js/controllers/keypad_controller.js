app.controller('KeyPadCtrl', function($scope, $state, $stateParams, $filter) {

  $scope.addHundred = function(num) {
    setEntry($scope.$parent.$parent.amount * 100);
  }

  $scope.addNumber = function(num) {
    setEntry(($scope.$parent.$parent.amount * 10) + (num / 100));
  }

  $scope.clearEntry = function() {
    $scope.$parent.$parent.amount = 0;
  }

  var setEntry = function(num) {
    if (num < 1000000) {
      $scope.$parent.$parent.amount = num;
    }
  }
});
