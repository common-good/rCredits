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
    if (!$scope.pin.value) {
      NotificationService.showAlert({title: "error"});
    }

    $ionicLoading.show();

    UserService.identifyCustomer($scope.scanUrl, $scope.pin.value)
      .then(function() {
        $scope.customer = UserService.currentCustomer();
        $ionicLoading.hide();
        $ionicHistory.nextViewOptions({
          disableBack: true
        });
        $state.go("app.customer");
      })
      .catch(function(err) {
        console.error(err);
        NotificationService.showAlert({title: "error", template: err});
      })
      .finally(function() {
        $ionicLoading.hide();
      });
  };

  $scope.init();

  $scope.hasValidPin = function() {
    return $scope.pin.value > 0;
  };

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
