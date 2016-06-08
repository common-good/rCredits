app.controller('TransactionResultCtrl', function ($scope, $state,
	$stateParams, $ionicLoading, $filter, NotificationService, UserService,
	TransactionService, BackButtonService, $timeout, SelfServiceMode) {

	$scope.transactionStatus = $stateParams.transactionStatus;
	$scope.transactionAmount = $stateParams.transactionAmount;
//	$scope.transactionMessage = $stateParams.transactionMessage;

	BackButtonService.disable();

	var statusKey;
	$scope.success = false;
	$scope.timeCan = true;

	// Enable UNDO btn for 1 min
	$timeout(function () {
		$scope.timeCan = false;
	}, 60 * 1000);

	$scope.setMessages = function (transactionResult) {
//		console.log('transaction_results' + transactionResult.ok);
		$scope.note = transactionResult.message;
		if (transactionResult.txid) {
			if (transactionResult.message.indexOf("ransaction has been canceled") > -1) {
				$scope.heading = 'Canceled';
				$scope.success = false;
			} else {
				$scope.heading = 'Successful';
				$scope.success = true;
			}
		} else {
			$scope.heading = 'Unsuccessful';
			$scope.success = false;
		}
	};

	$scope.customer = UserService.currentCustomer();
	$scope.user = UserService.currentUser();


	if ($scope.transactionStatus === 'failure') {
//		for (var t in TransactionService) {
//			console.log(t);
//			console.log(TransactionService[t]);
//		}
//		console.log($stateParams.transactionMessage);
//		console.log($stateParams.transactionMessage);
//		console.log($stateParams.transactionMessage);
//		$scope.setMessages($scope.transactionMessage);
		$scope.setMessages(TransactionService.lastTransaction);
	} else {
		$scope.setMessages(TransactionService.lastTransaction);
		$scope.transactionInfo = {
			amount: $filter('currency')($scope.transactionAmount),
			company: $scope.user.company,
			customerName: $scope.customer.name
		};
	}

	$scope.undoTransaction = function () {
		NotificationService.showConfirm({
			title: 'confirm_undo_transaction',
			subTitle: "",
			okText: "yes",
			cancelText: "no"
		}).then(function (res) {
			if (res === true) {
				$ionicLoading.show();
				TransactionService.undoTransaction(TransactionService.lastTransaction)
					.then(function (transactionResult) {
						$scope.setMessages(transactionResult);
						$scope.undo = true;
					})
					.finally(function () {
						$ionicLoading.hide();
					});
			}
		});
	};

	$scope.$on('$destroy', function () {
		BackButtonService.enable();
	});

	$scope.isSelfServiceEnabled = function () {
		return SelfServiceMode.isActive();
	};

});
