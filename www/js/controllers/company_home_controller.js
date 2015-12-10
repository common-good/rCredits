app.controller('CompanyHomeCtrl', function($scope, $state, $ionicLoading, $ionicPopup, BarcodeService, UserService, $ionicHistory) {

  var userObject = UserService.currentUser();
  var current_user = {
    'name': userObject.name,
    'company': userObject.company
  };

  $scope.current_user = current_user;

  if (!window.localStorage.getItem('notfirstlogin')) {
    $ionicPopup.alert({
      // templateUrl: 'templates/store-first-login.html'
      title: "This device is now associated with " + current_user.company + ".",
      template: "To set your preferences, please see the main menu."
    });
  }
});
