/* global app */
app.controller('SelectDemoCust', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService, $location) {
	var populateDemoCustomers = [
		{name: 'Cathy Cashier', url: 'HTTP://NEW.RC4.ME/ABJ-ME04nW44DHzxVDg', signin: '1', img: '/img/CathyCashier.jpg'},
		{name: 'Bob Bossman', url: 'HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G', signin: '1', img: '/img/BobBossman.jpg'},
		{name: 'Curt Customer', url: 'HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k', signin: '0', img: '/img/CurtCustomerMember.jpg'},
		{name: 'Susan Shopper', url: 'HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw', signin: '0', img: '/img/SusanShopper.jpg'},
		{name: "Curt-Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw', signin: '1', img: '/img/CurtCustomerAgent.jpg'}
	];
	$scope.iswebview = ionic.Platform.platform();
	$scope.customer = populateDemoCustomers;
	console.log($scope.customer[1].img);
	$scope.selectedCustomer = {
		selected: $scope.customer[0]
	};
	$scope.manager = populateDemoCustomers;
	$scope.selectedManager = {
		selected: $scope.manager[0]
	};
	$scope.location = $location.url();
	$scope.whereAmI = function () {
		return $location.url();
	};
	$scope.onSelectCustomer = function () {
		console.log($scope.location, $scope.whereAmI());
		var selected = $scope.selectedCustomer.selected;
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
	$scope.onSelectManager = function () {
		console.log($scope.location, $scope.whereAmI());
		var selected = $scope.selectedManager.selected;
		UserService.loginWithRCard(selected.url)
			.then(function () {
				$ionicHistory.nextViewOptions({
					disableBack: true
				});
				$state.go("app.home");
			})
			.catch(function (errorMsg) {
				NotificationService.showAlert({title: "error", template: errorMsg});
			})
			.finally(function () {
				$ionicLoading.hide();
			})
			.catch(function (errorMsg) {
				console.log($scope.currentUser.name);
				if (errorMsg === 'login_your_self') {
					NotificationService.showAlert({title: "Error", template: "You are already signed in as: " + $scope.currentUser.name});
				} else {
					NotificationService.showAlert({title: "Error", template: errorMsg});
				}
				$ionicLoading.hide();
			});
	};
});