exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
	seleniumSessionId: '617d2eb0-1e4c-4504-9921-f2c2207a06e7',
	baseUrl: "http://localhost:8100/#/app/home",
	specs: ['karma.conf.js',
		'r2.js',
		'test/*.js'],
	capabilities: {
		browserName: 'chrome'
	},
//	onPrepare: function () {
//		browser.driver.manage().window().maximize();
//	}
};