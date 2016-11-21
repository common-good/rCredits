/* global QRCode, app, PermissionService */
app.controller('QRCtrl', function ($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory,
	NotificationService, CashierModeService, PreferenceService, NetworkService,
	SelfServiceMode, $ionicSideMenuDelegate, $timeout) {
	// Create a QR code
	$scope.customer = UserService.currentCustomer();
//	var qrcode = new QRCode(document.getElementById("qrcode"), {
//		text:$scope.customer.accountInfo.url,
//		width: 200,
//		height: 200
//	});
//	var makeCode = 
	$scope.qrcode=new QRCode(document.getElementById("qrcode"), {
		text:$scope.customer.accountInfo.url,
		width: 200,
		height: 200
	});
	$scope.showBalance = function () {
		if ($scope.customer.balanceSecret) {
			NotificationService.showAlert('balanceIsSecret');
		} else {
			console.log($scope.customer);
			NotificationService.showAlert({
				scope: $scope,
				title: 'customerBalance',
				templateUrl: 'templates/customer-balance.html'
			});
		}
	};
	$scope.hideLoading = function () {
		$ionicLoading.hide();
	};
	$scope.showQR = function () {
		$state.go('app.qr');
	};
	$scope.$on('$destroy', function () {
		$scope.customer = null;
	});
	$scope.openCharge = function () {
		var chargeFn = function () {
			$state.go('app.transaction', {'transactionType': 'charge'});
		};
		if (CashierModeService.canCharge()) {
			chargeFn();
		} else {
			executeAction(chargeFn);
		}
	};
	$scope.openRefund = function () {
		var refundFn = function () {
			$state.go('app.transaction', {'transactionType': 'refund'});
		};
		if (CashierModeService.canRefund()) {
			refundFn();
		} else {
			NotificationService.showAlert({title: 'action_not_enabled'});
		}
	};
	$scope.openExchange = function () {
		var exchangeFn = function () {
			$state.go('app.transaction_select_exchange');
		};
		if (CashierModeService.canExchange()) {
			exchangeFn();
		} else {
			NotificationService.showAlert({title: 'action_not_enabled'});
		}
	};
	var executeAction = function (fn) {
		NotificationService.showConfirm({
			title: 'cashier_permission',
			subTitle: "",
			okText: "scanIn",
			cancelText: "cancel"
		}, {}).then(function (res) {
			if (res) {
				return PermissionService.authorizeSeller()
					.then(function (authResult) {
						if (!authResult) {
							NotificationService.showAlert({title: 'cashier_permission_rejected'});
							return;
						}
						fn();
					});
			}
		});
	};
	$scope.isSelfServiceEnabled = function () {
		return SelfServiceMode.isActive();
	};

});