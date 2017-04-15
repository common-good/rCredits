exports.config = {
	framework: 'jasmine',
	jasmineNodeOpts: {defaultTimeoutInterval: 900000},
	seleniumAddress: 'http://localhost:4444/wd/hub',
//	seleniumSessionId: '2d8c59a8-3cd0-4b1c-a3bc-c119985a69ff',
	restartBrowserBetweenTests: true,
	allScriptsTimeout: 25000,
	baseUrl: "http://localhost:8100/#/app/home",
	seleniumServerJar: "C:\Users\Someone\AppData\Roaming\npm\node_modules\protractor\node_modules\webdriver-manager\selenium\selenium-server-standalone-3.1.0.jar",
	specs: ['karma.conf.js',
		'r2.js',
		'test/*.js'],
	capabilities: {
		browserName: 'chrome'
	},
	beforeLaunce: {
		q: require('q')
	},
	onPrepare: function () {
		browser.manage().window().setSize(1280, 1100);
	},
	getPageTimeout: 20000
//	,onPrepare: function () {
//		browser.driver.manage().window().maximize();
//	}
};