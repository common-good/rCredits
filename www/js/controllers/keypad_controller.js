app.controller('KeyPadCtrl', function($scope, $state, $ionicLoading) {
  $scope.keyPress = function(key) {
    console.log(key);

    // Calls another function for calculate
  }

  $scope.addHundred = function(key) {
    console.log("00 was clicked");
  }

  $scope.clearEntry = function(key) {
    console.log("Clear was clicked");
  }
});
