app.controller('CustomerMenuCtrl', function($scope, $state, $ionicLoading, $ionicPopup, UserService, $ionicHistory) {

  $scope.customer = UserService.currentCustomer();

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

  $scope.hideLoading = function() {
    $ionicLoading.hide();
    // console.log("Loading icon hidden");
  };

  $scope.$on("$destroy", function(){
    $scope.customer = null;
    // console.log("Customer Menu Controller scope destroyed");
  });
});
