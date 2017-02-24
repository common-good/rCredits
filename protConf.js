exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
//	seleniumSessionId: 'ebe2288e-ec14-4564-88c2-226c6a381327',
	baseUrl:"http://localhost:8100/#/app/home",
	specs: ['karma.conf.js',
		'r2.js',
		'test/*.js'],
	capabilities: {
		browserName: 'chrome'
	}
//	,
//	onPrepare: function() {
//		browser.driver.get(
//	}
};