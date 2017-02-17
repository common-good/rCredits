app.controller('LoginCtrl', function ($scope, $ionicLoading, $state, $ionicPlatform, BarcodeService, BackButtonService, UserService, $ionicHistory, NotificationService, CashierModeService, $stateParams, $rootScope) {
	$scope.$on('$ionicView.loaded', function () {
		ionic.Platform.ready(function () {
			if (navigator && navigator.splashscreen)
				navigator.splashscreen.hide();
		});
	});
	// Scanner Login
	$ionicHistory.clearHistory();
	$scope.openScanner = function () {
		if (ionic.Platform.platform() === 'win64' || ionic.Platform.platform() === 'win32') {
			$rootScope.whereWasI = location.hash;
			$state.go("app.demo");
			$ionicLoading.hide();
		} else {
			$ionicLoading.show();
			$ionicPlatform.ready(function () {
				BarcodeService.scan('app.login')
					.then(function (str) {
						console.log(str);
						UserService.loginWithRCard(str)
							.then(function () {
								$ionicHistory.nextViewOptions({
									disableBack: true
								});
								$state.go("app.home");
							})
							.catch(function (errorMsg) {
								NotificationService.showAlert({title: "error", template: errorMsg});
							})
							.finally(function () {
								$ionicLoading.hide();
							});
					})
					.catch(function (errorMsg) {
						NotificationService.showAlert({title: "error", template: errorMsg});
						$ionicLoading.hide();
					});
			});
		}
	};
	if (CashierModeService.isEnabled() || $stateParams.openScanner) {
		$scope.openScanner();
	}
});
