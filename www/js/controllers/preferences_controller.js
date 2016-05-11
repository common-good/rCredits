app.controller('PreferencesCtrl', function ($scope, $state, UserService, PreferenceService, $ionicLoading, NotificationService) {
	var preferences = PreferenceService.getAll();
	$scope.preferences = preferences;
	$scope.saveItems = function () {
		PreferenceService.savePreferences(preferences);
	};
	$scope.cashierModePref = PreferenceService.getCashierModePref();
	$scope.cashierCanModePref = PreferenceService.getCashierCanPref();
	$scope.doAction = function (pref) {
		if (pref.id === 'forget_company') {
			$ionicLoading.show();
			UserService.logout()
				.then(function () {
					$state.go("app.login");
				})
				.catch(function (errorMsg) {
					NotificationService.showAlert({title: "error", template: errorMsg});
				})
				.finally(function () {
					$ionicLoading.hide();
				});
		}
	};
	$scope.openLink = function (url) {
		window.open(encodeURI(url), '_system');
	};
});
