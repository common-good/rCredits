app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  var currentUser = UserService.currentUser();
  $scope.currentUser = currentUser;

  if (!window.localStorage.getItem('notfirstlogin')) {
    $ionicPopup.alert({
      title: "This device is now associated with " + currentUser.company + ".",
      template: "To set your preferences, please see the main menu."
    });
  }
});
