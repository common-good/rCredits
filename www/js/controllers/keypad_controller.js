app.controller('KeyPadCtrl', function($scope, $state, $stateParams, $filter) {

  $scope.addHundred = function(num) {
    setEntry($scope.$parent.$parent.amount * 100);
  };

  $scope.addNumber = function(num) {
    var x = (num / 100);
    var y = ($scope.$parent.$parent.amount * 10);
    setEntry(($scope.$parent.$parent.amount * 10) + (num / 100));
  };

  $scope.clearEntry = function() {
    $scope.$parent.$parent.amount = 0;
  };

  var setEntry = function(num) {
    if (num < 1000000) {
      $scope.$parent.$parent.amount = parseFloat($filter('currency')(num, '', 2));
      console.log("Amount = ", $scope.$parent.$parent.amount);
    }
  };

});
