angular.module('controllers', [])

.controller('StarterAppCtrl', function($scope, $ionicModal, $timeout, $ionicLoading) {

  // With the new view caching in Ionic, Controllers are only called
  // when they are recreated or on app start, instead of every page change.
  // To listen for when this page is active (for example, to refresh data),
  // listen for the $ionicView.enter event:
  //$scope.$on('$ionicView.enter', function(e) {
  //});

  // Form data for the login modal
  $scope.loginData = {};

  // Create the login modal that we will use later
  $ionicModal.fromTemplateUrl('templates/login.html', {
    scope: $scope
  }).then(function(modal) {
    $scope.modal = modal;
  });

  // Triggered in the login modal to close it
  $scope.closeLogin = function() {
    $scope.modal.hide();
  };

  // Open the login modal
  $scope.login = function() {
    $scope.modal.show();
  };

  // Perform the login action when the user submits the login form
  $scope.doLogin = function() {
    console.log('Doing login', $scope.loginData);

    // Simulate a login delay. Remove this and replace with your login
    // code if using a login system
    $timeout(function() {
      $scope.closeLogin();
    }, 1000);
  };
})

.controller('LoginCtrl', function($scope, $state, $ionicLoading) {
  // console.log("This is the login controller");

  $scope.openScanner = function(){
    // console.log("You clicked on the scan button");
    $ionicLoading.show();

    // TODO: replace below with call to scan service
    // If login is successful, should redirect home
    setTimeout(function(){$scope.redirectHome(); }, 1000);
  };

  $scope.redirectHome = function(){
    // console.log("Redirect home");
    $ionicLoading.hide();
    $state.go("app.home");
  };
});
