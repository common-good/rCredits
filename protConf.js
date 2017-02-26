exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
	seleniumSessionId: 'ef30ca56-ba1b-4e92-b57e-cca83b869154',
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