// rCredits Register

var app = angular.module('rcredits', ['ionic', 'routes', 'pascalprecht.translate', 'LocalStorageModule'])

  .config(['$translateProvider', 'localStorageServiceProvider',
    function($translateProvider, localStorageServiceProvider) {

      $translateProvider
        .useMissingTranslationHandlerLog()
        .useStaticFilesLoader({
          prefix: 'js/languages/definitions/',
          suffix: '.json'
        })
        .preferredLanguage(Language.DEFAULT_LANGUAGE)
        .fallbackLanguage(Language.DEFAULT_LANGUAGE)
        .useSanitizeValueStrategy('sanitizeParameters');

      localStorageServiceProvider
        .setPrefix('rcredits');
    }])

  .run(function($ionicPlatform) {
    $ionicPlatform.ready(function() {

      // This only for web development to enable proxy
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
    });
  });
