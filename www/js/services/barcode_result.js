/* global _ */
(function (window) {
	'use strict';
	var BarcodeResult = function (scanReult) {
		if (!_.isUndefined(scanReult)) {
			_.extendOwn(this, scanReult);
		}
	};
	BarcodeResult.prototype.wasCancelled = function () {
		return this.hasOwnProperty('cancelled') && this.cancelled;
	};
	BarcodeResult.prototype.isQRCode = function () {
		return this.hasOwnProperty('format') && this.format === 'QR_CODE';
	};
	window.BarcodeResult = BarcodeResult;
})(window);
