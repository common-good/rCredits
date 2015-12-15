angular.module('routes', [])

.config(function($stateProvider, $urlRouterProvider) {
  $stateProvider

  .state('app', {
    url: '/app',
    abstract: true,
    templateUrl: 'templates/menu.html',
    controller: 'MenuCtrl'
  })

  .state('app.home', {
    url: '/home',
    views: {
      'menuContent': {
        templateUrl: 'templates/home.html'
      }
    }
  })

  .state('app.login', {
    url: '/login',
    views: {
      'menuContent': {
        templateUrl: 'templates/login.html'
      }
    }
  })

  .state('app.customer', {
    url: '/customer',
    views: {
      'menuContent': {
        templateUrl: 'templates/customer-menu.html'
      }
    }
  })

  // Template file to show styles - remove in production
  .state('app.template', {
    url: '/template',
    views: {
      'menuContent': {
        templateUrl: 'templates/template.html'
      }
    }
  })

  .state('app.keypad', {
    url: '/keypad',
    views: {
      'menuContent': {
        templateUrl: 'templates/keypad.html'
      }
    },
    controller: "KeyPadCtrl"
  });

  // if none of the above states are matched, use this as the fallback
  $urlRouterProvider.otherwise('/app/login');
});
