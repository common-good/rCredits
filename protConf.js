exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
	seleniumSessionId: '6c9617aa-8e0b-4a15-ba7f-d1361e5ad5ba',
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