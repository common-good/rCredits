app.controller('KeyPadCtrl', function($scope, $state, $stateParams, $filter) {

  var convertToNumber = function(amount) {
    return parseFloat(Number(amount).toFixed(2));
  };

  $scope.addHundred = function(num) {
    setEntry(convertToNumber($scope.$parent.$parent.amount) * 100);
  };

  $scope.addNumber = function(num) {
    var x = convertToNumber($scope.$parent.$parent.amount) * 10;
    var y = convertToNumber(num) / 100;
    setEntry(x + y);
  };

  $scope.clearEntry = function() {
    setEntry(0);
  };

  var setEntry = function(num) {
    $scope.$parent.$parent.amount = convertToNumber(num);
  };

});
