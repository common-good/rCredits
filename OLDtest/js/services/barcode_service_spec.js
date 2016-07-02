/* global _ */
describe('Barcode Scanner Service', function () {
	'use strict';
	beforeEach(module('rcredits'));
	var barcodeScannerService, rootScope, $httpBackend;
	var SCAN_RESULT = {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};
	beforeEach(inject(function (BarcodeService, $rootScope, _$httpBackend_) {
		barcodeScannerService = BarcodeService;
		rootScope = $rootScope;
		$httpBackend = _$httpBackend_;
		$httpBackend.whenGET(/templates\/*/).respond(function (method, url, data, headers) {
			return [200, '<div></div>'];
		});
		$httpBackend.whenGET(/js\/languages\/definitions\//).respond(function (method, url, data, headers) {
			return [200, {}];
		});
	}));
	var createScanSpy = function (returnValue) {
		barcodeScannerService.scanner.scan = jasmine.createSpy('scan()')
			.and.callFake(function (sucessFn) {
				sucessFn(returnValue);
			});
	};
	describe('Scan a code', function () {
		it('Scan is a QR CODE', function (done) {
			createScanSpy(_.clone(SCAN_RESULT));
			barcodeScannerService.scan()
				.then(function (scanResult) {
					done();
				});
			rootScope.$apply();
		});
		it('Scan is NOT a QR CODE', function (done) {
			var scanResponse = _.clone(SCAN_RESULT);
			scanResponse.format = 'DATA_MATRIX';
			createScanSpy(scanResponse);
			barcodeScannerService.scan()
				.catch(function (scanResultError) {
					done();
				});
			rootScope.$apply();
		});
		it('Scan Was Cancelled', function (done) {
			var scanResponse = _.clone(SCAN_RESULT);
			scanResponse.cancelled = true;
			createScanSpy(scanResponse);
			barcodeScannerService.scan()
				.catch(function (scanResultError) {
					done();
				});
			rootScope.$apply();
		});
		it('Scan Failed', function (done) {
			barcodeScannerService.scanner.scan = jasmine.createSpy('scan()')
				.and.callFake(function (sucessFn, failedFn) {
					failedFn("Scan failed");
				});
			barcodeScannerService.scan()
				.catch(function (scanResultError) {
					done();
				});
			rootScope.$apply();
		});
	});
});