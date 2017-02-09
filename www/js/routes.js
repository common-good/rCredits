angular.module('routes', [])
	.config(function ($stateProvider, $urlRouterProvider) {
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
				},
				params: {
					disableLoadSeller: false,
					openScanner: false
				},
				resolve: {
					seller: function ($q, $timeout, UserService, $state, $stateParams) {
						var deferred = $q.defer();
						if ($stateParams.disableLoadSeller) {
							deferred.resolve();
						} else {
							$timeout(function () {
								var seller = UserService.loadSeller();
								if (seller) {
									$state.go("app.home");
									deferred.reject();
								} else {
									deferred.resolve();
								}
							});
						}
						return deferred.promise;
					}
				}
			})
			.state('app.preferences', {
				url: '/preferences',
				views: {
					'menuContent': {
						templateUrl: 'templates/preferences.html',
						controller: 'PreferencesCtrl'
					}
				}
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
			// Template file to show styles - remove in production
			.state('app.template', {
				url: '/template',
				views: {
					'menuContent': {
						templateUrl: 'templates/template.html'
					}
				}
			})
			.state('app.transaction_select_exchange', {
				url: '/transaction/select/exchange',
				views: {
					'menuContent': {
						templateUrl: 'templates/select_exchange.html',
						controller: 'SelectExchangeCtrl as selExCtrl'
					}
				}
			})
			.state('app.transaction_exchange', {
				url: '/transaction/exchange',
				views: {
					'menuContent': {
						templateUrl: 'templates/exchange.html',
						controller: 'ExchangeCtrl as exCtrl'
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
			})
			.state('app.self_service_mode', {
				url: '/self_service_mode',
				views: {
					'menuContent': {
						templateUrl: 'templates/self_service_mode_login.html',
						controller: 'SelfServiceModeController'
					}
				}
			})
			.state('app.demo', {
				url: '/demo-people',
				views: {
					'menuContent': {
						templateUrl: 'templates/demo-people.html',
						controller: 'SelectDemoCust'
					}
				}
			})
			.state('app.qr', {
				url: '/customer-qr',
				views: {
					'menuContent': {
						templateUrl: 'templates/customer-qr.html',
						controller: 'QRCtrl'
					}
				}
			});;
		// if none of the above states are matched, use this as the fallback
		$urlRouterProvider.otherwise('/app/login');
	});