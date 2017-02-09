/* global Language, rCreditsConfig, StatusBar */
// rCredits Register
var app = angular.module('rcredits', ['ionic', 'routes', 'pascalprecht.translate', 'LocalStorageModule', 'ui.router'])
	.config(['$translateProvider', 'localStorageServiceProvider', '$ionicConfigProvider',
		function ($translateProvider, localStorageServiceProvider, $ionicConfigProvider) {
			$ionicConfigProvider.views.maxCache(0);
			$translateProvider
				.useStaticFilesLoader({
					prefix: 'js/languages/definitions/',
					suffix: '.json'
				})
				.preferredLanguage(Language.DEFAULT_LANGUAGE)
				.fallbackLanguage(Language.DEFAULT_LANGUAGE)
				.useSanitizeValueStrategy('sanitizeParameters');
			localStorageServiceProvider.setPrefix('rcredits');
			var storageQuota = false;
		}])
	.run(function ($ionicPlatform,$state, SQLiteService, NetworkService, $rootScope, TransactionSyncService, BackButtonService, UserService, NotificationService, $rootScope) {
		$ionicPlatform.ready(function () {
			// This only for web development to enable proxy
			$rootScope.whereWasI=location.hash;
			$rootScope.amIOnline=NetworkService.isOffline();
			if (!ionic.Platform.isWebView()) {
				rCreditsConfig.serverUrl = rCreditsConfig.serverproxyUrl;
			}
			// Hide the accessory bar by default (remove this to show the accessory bar above the keyboard
			// for form inputs)
			if (window.cordova && window.cordova.plugins.Keyboard) {
				cordova.plugins.Keyboard.hideKeyboardAccessoryBar(true);
				cordova.plugins.Keyboard.disableScroll(true);

			}
			if (window.StatusBar) {
				// org.apache.cordova.statusbar required
				StatusBar.styleDefault();
			}
			//Check to make sure there is still space on the device
			if (window.cordova) {
				var spaceCheck = window.setInterval(checkSpace, 1000);
				function checkSpace() {
					cordova.exec(function (result) {
						if (result < 512) {
							UserService.storageOverQuota();
							console.log("Low Disk Space: " + result);
							clearInterval(spaceCheck);
							var alertPopup = NotificationService.showAlert({
								title: "error",
								template: "Low Disk Space: " + result + ", Please Free Up Some More Space and Try Again",
								buttons: [
									{
										text: 'Exit',
										type: 'button-positive',
										onTap: function (e) {
											ionic.Platform.exitApp();
										}
									}
								]
							});
						} else {
						}
					}, function (error) {
						console.log("Error!... The details follow: " + error);
						return error;
					}, "File", "getFreeDiskSpace", []);
				}
			}
			SQLiteService.init();
			BackButtonService.init();
		});
	});