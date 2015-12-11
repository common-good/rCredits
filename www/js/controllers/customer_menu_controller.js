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

  $scope.showBalance = function() {
    if ($scope.customer) {
      if ($scope.customer.balanceSecret) {
        $scope.showAlert("This customer's balance is secret");
      } else {
        $ionicPopup.alert({
          title:  "Balance for",
          subTitle: $scope.customer.name,
          templateUrl: "templates/customer-balance.html",
          scope: $scope
        });
      }
    }
  };
});
