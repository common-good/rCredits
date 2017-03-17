exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
//	seleniumSessionId: '7b635c7f-89e1-44f5-8f71-2c2de2deb9a5',
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