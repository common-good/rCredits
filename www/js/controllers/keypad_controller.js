app.controller('KeyPadCtrl', function($scope, $state) {
  $scope.amount = 0;

  $scope.addHundred = function(key) {
    $scope.amount = $scope.amount * 100;
  }

  $scope.addNumber = function(num) {
    $scope.amount = ($scope.amount * 10) + (num/100);
  }

  $scope.clearEntry = function(key) {
    $scope.amount = 0;
  }

  $scope.keyPress = function(key) {
    $scope.addNumber(key);
  }
});
