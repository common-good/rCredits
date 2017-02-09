// Karma configuration
// Generated on Tue Dec 15 2015 11:52:17 GMT-0300 (ART)
module.exports = function (config) {
	config.set({
		// base path that will be used to resolve all patterns (eg. files, exclude)
		basePath: '',
		// frameworks to use
		// available frameworks: https://npmjs.org/browse/keyword/karma-adapter
		frameworks: ['jasmine'],
		//don't run the test in an iFrame
		client: {
			useIframe: false
		},
		// list of files / patterns to load in the browser
		files: [
			'www/lib/ionic/js/ionic.bundle.js',
			'www/lib/angular-translate/angular-translate.min.js',
			'www/lib/angular-translate-loader-static-files/angular-translate-loader-static-files.min.js',
			'www/lib/angular-translate-handler-log/angular-translate-handler-log.min.js',
			'www/lib/angular-local-storage/dist/angular-local-storage.min.js',
			'www/lib/prototypejs/dist/prototype.js',
			'www/lib/underscore/underscore-min.js',
			'www/lib/moment/moment.js',
			'node_modules/angular-mocks/angular-mocks.js',
			'www/lib/ionic/js/ionic.bundle.js',
			'www/lib/underscore/underscore-min.js',
			'www/lib/angular-translate/angular-translate.min.js',
			'www/lib/angular-translate-loader-static-files/angular-translate-loader-static-files.min.js',
			'www/lib/angular-translate-handler-log/angular-translate-handler-log.min.js',
			'www/lib/angular-local-storage/dist/angular-local-storage.min.js',
			'www/lib/prototypejs/dist/prototype.min.js',
			'www/lib/moment/moment.js',
			'www/js/services/qrcode.min.js',
			'www/lib/jquery/dist/jquery.min.js',
			'www/js/config.js',
			'www/js/app.js',
			'www/js/routes.js',
			'www/js/languages/language.js',
			'www/js/controllers/login_controller.js',
			'www/js/controllers/menu_controller.js',
			'www/js/controllers/company_home_controller.js',
			'www/js/controllers/customer_menu_controller.js',
			'www/js/controllers/keypad_controller.js',
			'www/js/controllers/transaction_controller.js',
			'www/js/controllers/transaction_result_controller.js',
			'www/js/controllers/preferences_controller.js',
			'www/js/controllers/select_exchange_controller.js',
			'www/js/controllers/exchange_controller.js',
			'www/js/controllers/selfservice_mode_controller.js',
			'www/js/controllers/selectDemoCustomer.js',
			'www/js/controllers/qr_controller.js',
			'www/js/directives/sf_load.js',
			'www/js/directives/sf_resize.js',
			'www/js/directives/offline_header.js',
			'www/js/services/barcode_service.js',
			'www/js/services/barcode_result.js',
			'www/js/services/user_service.js',
			'www/js/services/notification_service.js',
			'www/js/services/request_parameter_builder.js',
			'www/js/services/qrcode_parser.js',
			'www/js/services/transaction_service.js',
			'www/js/services/transaction_sync_service.js',
			'www/js/services/transaction_sql.js',
			'www/js/services/preference_service.js',
			'www/js/services/cashier_mode_service.js',
			'www/js/services/permission_service.js',
			'www/js/services/sqlite_service.js',
			'www/js/services/member_sql_service.js',
			'www/js/services/exchange_service.js',
			'www/js/services/network_service.js',
			'www/js/services/back_button_service.js',
			'www/js/services/selfservice_mode_service.js',
			'www/js/services/hash_service.js',
			'www/js/models/user.js',
			'www/js/models/seller.js',
			'www/js/models/customer.js',
			'www/js/models/account_info.js',
			'www/js/models/preference.js',
			'www/js/models/cashier_mode_pref.js',
			'www/js/models/url_configurator.js',
			'www/js/models/transaction.js',
			'www/js/models/sql_query.js',
			'www/js/models/currency.js',
			'www/js/models/payment_type.js',
			'www/js/models/fee.js',
			'www/js/models/currencies_definitions.js',
			'www/js/models/payment_types_definitions.js',
			'www/js/models/exchange.js',
//			'**/*.steps', // ws
			'r2.js',
			'test/*.test', // ws: was test/**/*_spec.js
			'www/templates/**/*.html'
		],
		// list of files to exclude
		exclude: [],
		// preprocess matching files before serving them to the browser
		// available preprocessors: https://npmjs.org/browse/keyword/karma-preprocessor
		preprocessors: {},
		// test results reporter to use
		// possible values: 'dots', 'progress'
		// available reporters: https://npmjs.org/browse/keyword/karma-reporter
		reporters: ['progress', 'html', 'junit', 'mocha'],
		// web server port
		port: 9876,
		// enable / disable colors in the output (reporters and logs)
		colors: true,
		// level of logging
		// possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
		logLevel: config.LOG_INFO,
		// enable / disable watching file and executing tests whenever any file changes
		autoWatch: true,
		// start these browsers
		// available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
		browsers: ['Chrome'],
		// Continuous Integration mode
		// if true, Karma captures browsers, runs the tests and exits
		singleRun: false,
		// Concurrency level
		// how many browser should be started simultanous
		concurrency: Infinity
	});
};
