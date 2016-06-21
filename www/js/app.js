/* global Language, rCreditsConfig, StatusBar */
// rCredits Register
var app = angular.module('rcredits', ['ionic', 'routes', 'pascalprecht.translate', 'LocalStorageModule'])
	.config(['$translateProvider', 'localStorageServiceProvider', '$ionicConfigProvider',
		function ($translateProvider, localStorageServiceProvider, $ionicConfigProvider) {
			$ionicConfigProvider.views.maxCache(0);
			$translateProvider
				.useMissingTranslationHandlerLog()
				.useStaticFilesLoader({
					prefix: 'js/languages/definitions/',
					suffix: '.json'
				})
				.preferredLanguage(Language.DEFAULT_LANGUAGE)
				.fallbackLanguage(Language.DEFAULT_LANGUAGE)
				.useSanitizeValueStrategy('sanitizeParameters');
			localStorageServiceProvider.setPrefix('rcredits');
		}])
	.run(function ($ionicPlatform, SQLiteService, NetworkService, $rootScope, TransactionSyncService, BackButtonService, UserService, $timeout, EnoughSpace) {
		$ionicPlatform.ready(function () {
			$timeout(function () {
				if (!EnoughSpace.enoughSpace) {
//					UserService.logout();
					console.log("Oh no, there isn't enough space!");
				} else {
					console.log("Yay, there's enough space!");
				}
			}, 1000);
			// This only for web development to enable proxy
			if (!ionic.Platform.isWebView()) {
//				console.log('web view');
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
			SQLiteService.init();
			BackButtonService.init();
		});
	});
