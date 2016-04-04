app.controller('SelfServiceModeController', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService, $rootScope, SelfServiceMode) {

  $scope.pin = {
    value: null
  };

  $scope.init = function() {

    BarcodeService.scan()
      .then(function(scanUrl) {
        $scope.scanUrl = scanUrl;
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
      });
  };

  $scope.identify = function() {
    console.log("SelfService SCAN: ", $scope.scanUrl);
    console.log("PIN: ", $scope.pin.value);
    UserService.identifyCustomer($scope.scanUrl, $scope.pin.value)
      .then(function() {
        $scope.customer = UserService.currentCustomer();
        console.log("Identified Customer: ", $scope.customer);
      })
      .catch(function(err) {
        console.error(err);
      });
  };

  $scope.init();

});


app.directive('numbersOnly', function() {
  return {
    require: 'ngModel',
    link: function(scope, element, attr, ngModelCtrl) {
      function fromUser(text) {
        if (text) {
          var transformedInput = ("" + text).substring(0, 4);

          if (transformedInput !== text) {
            ngModelCtrl.$setViewValue(transformedInput);
            ngModelCtrl.$render();
          }
          return transformedInput;
        }
        return undefined;
      }

      ngModelCtrl.$parsers.push(fromUser);
    }
  };
});
