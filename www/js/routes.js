angular.module('routes', [])

.config(function($stateProvider, $urlRouterProvider) {
  $stateProvider

  .state('app', {
    url: '/app',
    abstract: true,
    templateUrl: 'templates/menu.html',
    controller: 'MenuCtrl'
  })

  .state('app.customer', {
    url: '/customer',
    views: {
      'menuContent': {
        templateUrl: 'templates/customer-menu.html',
        controller: 'CustomerMenuCtrl'
      }
    }
  })

  .state('app.home', {
    url: '/home',
    views: {
      'menuContent': {
        templateUrl: 'templates/home.html',
        controller: 'CompanyHomeCtrl'
      }
    }
  })

  .state('app.login', {
    url: '/login',
    views: {
      'menuContent': {
        templateUrl: 'templates/login.html',
        controller: 'LoginCtrl'
      }
    }
  })

  .state('app.preferences', {
    url: '/preferences',
    views: {
      'menuContent': {
        templateUrl: 'templates/preferences.html'
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

  .state('app.transaction', {
    url: '/transaction/{transactionType}',
    views: {
      'menuContent': {
        templateUrl: 'templates/transaction.html',
        controller: 'TransactionCtrl'
      }
    }
  })

  .state('app.transaction_result', {
    url: '/transaction-result/{transactionStatus}/{transactionAmount}',
    views: {
      'menuContent': {
        templateUrl: 'templates/transaction-result.html',
        controller: 'TransactionResultCtrl'
      }
    }
  });

  // if none of the above states are matched, use this as the fallback
  $urlRouterProvider.otherwise('/app/login');
});
