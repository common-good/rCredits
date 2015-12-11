app.controller('LoginCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  // Scanner Login

  $scope.openScanner = function(){
    $ionicLoading.show();

    BarcodeService.scan()
    .then(function(str){
      UserService.loginWithRCard(str)
      .then(function(){
        $ionicHistory.nextViewOptions({
          disableBack: true
        });

        $scope.redirectHome();
      })
      .catch(function(errorMsg){
        $scope.showAlert(errorMsg);
      })
      .finally(function(){
        $ionicLoading.hide();
      });
    })
    .catch(function(errorMsg){
      $scope.showAlert(errorMsg);
      $ionicLoading.hide();
    });
  };

  // Redirects

  $scope.redirectHome = function(){
    $state.go("app.home");
  };

  // Alert

  $scope.showAlert = function(message) {
    $ionicLoading.hide();
    $ionicPopup.alert({
      template: message
    });
  };
});
