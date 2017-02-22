exports.config = {
	framework: 'jasmine',
	seleniumAddress: 'http://localhost:4444/wd/hub',
	seleniumSessionId: '9156724e-3b34-49b6-8cf1-0ef44c599dbe',
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