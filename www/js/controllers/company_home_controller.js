app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  var currentUser = UserService.currentUser();
  $scope.currentUser = currentUser;

  if (!window.localStorage.getItem('notfirstlogin')) {
    $ionicPopup.alert({
      title: "This device is now associated with " + currentUser.company + ".",
      template: "To set your preferences, please see the main menu."
    });
  }

  $scope.scanCustomer = function(){
    $ionicLoading.show();

    BarcodeService.scan()
    .then(function(id){
      UserService.identifyCustomer(id)
      .then(function(){
        customer = UserService.currentCustomer();
        $scope.customer = customer;

        if (customer.firstPurchase) {
          $ionicPopup.confirm({
            templateUrl: "templates/first-purchase.html",
            scope: $scope,
            okText: "Confirm"
          })
          .then(function(confirmed){
            if (confirmed) {
              $state.go("app.customer");
            }
          });
          $ionicLoading.hide();
        } else {
          $state.go("app.customer");
        };
      })
      .catch(function(errorMsg){
        $scope.showAlert(errorMsg);
        $ionicLoading.hide();
      });
    })
    .catch(function(errorMsg){
      $scope.showAlert(errorMsg);
      $ionicLoading.hide();
    });
  };
});
