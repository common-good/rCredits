app.service('PermissionService', function ($q, UserService, BarcodeService, UserService) {
	var self;
	var PermissionService = function () {
		self = this;
	};
	PermissionService.prototype.authorizeSeller = function () {
		if (!ionic.Platform.isWebView()) {
			BarcodeService.setScanForCustomer();
		}
		return BarcodeService.scan().then(function (scannedUrl) {
			return UserService.currentUser().isFromUrl(scannedUrl);
		})
			.catch(function () {
				return false;
			});
	};
	return new PermissionService();
});
