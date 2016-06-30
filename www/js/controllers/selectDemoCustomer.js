/* global app */
app.controller('SelectDemoCust', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService) {
	var populateDemoCustomers = [
		{name: 'Susan Shopper', url: 'HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw'},
		{name: 'Curt Customer', url: 'HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k'},
		{name: "Curt from Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw'},
		{name: 'Bob Bossman', url: 'HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G'}
	];
	$scope.customer = populateDemoCustomers;
	$scope.selectedCustomer = {
		selected: $scope.customer[0]
	};
	$scope.onSelectCustomer = function () {
		var selected = $scope.selectedCustomer.selected;
		console.log(selected);
		UserService.identifyCustomer(selected.url)
			.then(function () {
				$scope.customer = UserService.currentCustomer();
				if ($scope.customer.firstPurchase) {
					NotificationService.showConfirm({
						title: 'firstPurchase',
						templateUrl: "templates/first-purchase.html",
						scope: $scope,
						okText: "confirm"
					})
						.then(function (confirmed) {
							if (confirmed) {
								$ionicLoading.show();
								$state.go("app.customer");
							}
						});
					$ionicLoading.hide();
				} else {
					$ionicLoading.hide();
					$state.go("app.customer");
				}
			})
			.catch(function (errorMsg) {
				console.log($scope.currentUser.name);
//						for (var prop in $scope.currentUser) {
//						}
				if (errorMsg === 'login_your_self') {
					NotificationService.showAlert({title: "Error", template: "You are already signed in as: " + $scope.currentUser.name});
				} else {
					NotificationService.showAlert({title: "Error", template: errorMsg});
				}
				$ionicLoading.hide();
			});
	};
});