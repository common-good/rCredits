/* global app */
app.controller('TransactionCtrl', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService) {
	$scope.transactionType = $stateParams.transactionType;
	$scope.amount = 0;
	var seller = UserService.currentUser();
	var customer = UserService.currentCustomer();
	var isTransactionTypeCharge = function () {
		return $scope.transactionType === 'charge';
	};
	var fillCategories = function () {
		if (isTransactionTypeCharge()) {
			// You can put in other and none for the future here
			//return _.union(seller.descriptions, ['other', 'none']);
			console.log(seller.descriptions);
			return seller.descriptions;
		}
		console.log(seller.descriptions);
		return seller.descriptions;
	};
	$scope.moreThan1Category = function () {
		if (seller.descriptions.length > 1) {
			return true;
		} else {
			return false;
		}
	};
	$scope.categories = fillCategories();
	$scope.selectedCategory = {
		selected: $scope.categories[0],
		custom: null
	};
	$scope.disableTransaction = function () {
		if ($scope.amount === 0 || !$scope.selectedCategory.selected) {
			return true;
		}
	};
	$scope.charge = function () {
		return TransactionService.charge($scope.amount, $scope.selectedCategory.selected);
	};
	$scope.refund = function () {
		return TransactionService.refund($scope.amount, $scope.selectedCategory.selected);
	};
	$scope.initiateTransaction = function () {
		$ionicLoading.show();
		var transactionAmount = $scope.amount;
		var transactionPromise;
		if (isTransactionTypeCharge()) {
			transactionPromise = $scope.charge();
		} else {
			transactionPromise = $scope.refund();
		}
		transactionPromise.then(function (transaction) {
			$state.go('app.transaction_result', {'transactionStatus': 'success', 'transactionAmount': transactionAmount});
			$ionicLoading.hide();
		}, function (errorMsg) {
			TransactionService.lastTransaction = errorMsg;
			$state.go('app.transaction_result', {'transactionStatus': 'failure', 'transactionAmount': transactionAmount, 'transactionMessage': errorMsg.message});
			$ionicLoading.hide();
		});
	};
	$scope.onSelectCategory = function () {
		console.log(seller.descriptions);
		if (!isTransactionTypeCharge() || $scope.selectedCategory.selected !== 'other') {
			return;
		}
		$scope.selectedCategory.custom = null;
		var myPopup = NotificationService.showConfirm({
			template: '<input type="text" ng-model="selectedCategory.custom">',
			title: 'enterNewCategory',
			subTitle: '',
			scope: $scope,
			buttons: [
				{text: 'Cancel'},
				{
					text: '<b>Save</b>',
					type: 'button-positive',
					onTap: function (e) {
						if (!$scope.selectedCategory.custom) {
							//don't allow a user to close unless they enter a wifi password
							e.preventDefault();
						} else {
							return $scope.selectedCategory.custom;
						}
					}
				}
			]
		});
		myPopup.then(function (res) {
			if (res) {
				$scope.categories.push(res);
				$scope.selectedCategory.selected = res;
			}
		});
	};
});
