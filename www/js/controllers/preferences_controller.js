app.controller('PreferencesCtrl', function($scope, $state, UserService, PreferenceService) {


  var preferences = PreferenceService.getAll();
  $scope.preferences = preferences;

  $scope.saveItems = function() {
    PreferenceService.savePreferences(preferences);
  }


});
