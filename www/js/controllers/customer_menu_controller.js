app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  UserService.identifyCustomer()
  .then(function(str){
    $scope.customer = str;
  })
  .catch(function(errorMsg){
    $state.go("app.home");
    $scope.showAlert(errorMsg);
  })
  .finally(function(){
    $ionicLoading.hide();
  });

});
