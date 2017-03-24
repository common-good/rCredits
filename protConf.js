exports.config = {
	framework: 'jasmine',
	jasmineNodeOpts: {defaultTimeoutInterval: 900000},
	seleniumAddress: 'http://localhost:4444/wd/hub',
//	seleniumSessionId: '7b635c7f-89e1-44f5-8f71-2c2de2deb9a5',
	restartBrowserBetweenTests: true,
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
	getPageTimeout: 8000
//	,onPrepare: function () {
//		browser.driver.manage().window().maximize();
//	}
};