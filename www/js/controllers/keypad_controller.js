app.controller('KeyPadCtrl', function($scope, $state, $ionicLoading) {
  $scope.amount = 0;

  $scope.keyPress = function(key) {
    console.log(key);

    // Calls another function for calculate
    $scope.addNumber(key);
  }

  $scope.addHundred = function(key) {
    console.log("00 was clicked");
  }

  $scope.clearEntry = function(key) {
    console.log("Clear was clicked");
  }

  $scope.addNumber = function(num) {
    // console.log("Starting Amount:" + $scope.amount);
    $scope.amount = ($scope.amount * 10) + (num/100);
    // console.log("New amount: " + $scope.amount);
  }
});
