app.controller('SelfServiceModeController', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory, NotificationService, $rootScope, SelfServiceMode) {


  $scope.init = function() {
    BarcodeService.scan()
      .then(function(barcodeResult) {
        console.log("SelfService SCAN: ", barcodeResult)
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
      });
  };


  $scope.init();

});
