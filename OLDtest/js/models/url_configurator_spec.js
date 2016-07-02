describe('Url Configurator', function () {
	'use strict';
	var urlConf;
	beforeEach(module('rcredits'));
	beforeEach(function () {
		module(function ($exceptionHandlerProvider) {
			$exceptionHandlerProvider.mode('log');
		});
	});
	beforeEach(function () {
		urlConf = new UrlConfigurator();
		urlConf.baseUrl = 'https://stage-xxx.rcredits.org/pos';
	});
	it('Should create url replace the member ID', function () {
		console.log(urlConf.getServerUrl('AAK'));
//		expect('https://stage-NEW.rcredits.org/pos').not.toBe(null);
//		expect('https://stage-NEW.rcredits.org/pos').toBe(urlConf.getServerUrl('NEW'));
		expect('https://stage-AAK.rcredits.org/pos').toBe(urlConf.getServerUrl('AAK'));
	});
});