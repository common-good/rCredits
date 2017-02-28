exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
	seleniumSessionId: 'dadd935c-b75c-4e97-b1e2-c65e5aec0121',
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