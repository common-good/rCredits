app.controller('CustomerMenuCtrl', function ($scope, $state, $ionicLoading, UserService, $ionicHistory, NotificationService, CashierModeService, PermissionService, SelfServiceMode) {
	$scope.customer = UserService.currentCustomer();
	$scope.showBalance = function () {
		if ($scope.customer.balanceSecret) {
			NotificationService.showAlert('balanceIsSecret');
		} else {
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