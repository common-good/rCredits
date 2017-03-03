exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
//	seleniumSessionId: '4f43aa12-925e-4efb-8717-9a1afb0d22f0',
	restartBrowserBetweenTests: true,
	baseUrl: "http://localhost:8100/#/app/home",
	specs: ['karma.conf.js',
		'r2.js',
		'test/*.js'],
	capabilities: {
		browserName: 'chrome'
	}
//	,onPrepare: function () {
//		browser.driver.manage().window().maximize();
//	}
};