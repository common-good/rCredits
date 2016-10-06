/* global app, $ionicHistory 
 * 		{name: 'Cathy Cashier', url: 'HTTP://NEW.RC4.ME/ABJ-ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
		{name: 'Bob Bossman', url: 'HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G', signin: '1', img: 'img/BobBossman.jpg'},
		{name: 'Curt Customer', url: 'HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k', signin: '0', img: 'img/CurtCustomerMember.jpg'},
		{name: 'Susan Shopper', url: 'HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
		{name: "Curt-Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
 * */
app.controller('SelectDemoCust', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService, $location, $rootScope, NetworkService,$ionicHistory) {
	var populateDemoCustomers = [
		{name: 'Cathy Cashier', url: 'HTTP://NEW.RC4.ME/ABJ-ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
		{name: 'Bob Bossman', url: 'HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G', signin: '1', img: 'img/BobBossman.jpg'},
		{name: 'Curt Customer', url: 'HTTP://6VM.RC4.ME/G0ANyCBBlUF1qWNZ2k', signin: '0', img: 'img/CurtCustomerMember.jpg'},
		{name: 'Susan Shopper', url: 'HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
		{name: "Curt-Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
	];
	$scope.iswebview = ionic.Platform.platform();
	$scope.customer = populateDemoCustomers;
	$scope.selectedCustomer = {
		selected: $scope.customer
	};
	$scope.manager = populateDemoCustomers;
	$scope.selectedManager = {
		selected: $scope.manager
	};
	$scope.whereWasI = $rootScope.whereWasI;
//	console.log($scope.whereWasI);
	$scope.onSelectCustomer = function (person) {
		var selected = person;
		console.log(selected, $location.state(), $scope.whereWasI);
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
				for (var prop in $scope.currentUser) {
				}
				if (errorMsg === 'login_your_self') {
					NotificationService.showAlert({title: "Error", template: "You cannot use yourself as a customer while you are an agent"});
				} else if (errorMsg === 'login_your_self') {
					NotificationService.showAlert({title: "Error", template: "You are already signed in as: " + $scope.currentUser.name});
				} else {
					NotificationService.showAlert({title: "Error", template: errorMsg});
				}
				$ionicLoading.hide();
			});
	};
	$scope.onSelectManager = function (person) {
		var selected = person;
		console.log(selected, $scope.whereWasI);
		UserService.loginWithRCard(selected.url)
			.then(function () {
				$ionicHistory.nextViewOptions({
					disableBack: true
				});
				$state.go("app.home");
			})
			.catch(function (errorMsg) {
				if (errorMsg === 'login_your_self') {
					NotificationService.showAlert({title: "Error", template: "You are already signed in as: " + selected.name});
				} else if (errorMsg === 'TypeError: this.db is null') {
				} else {
					NotificationService.showAlert({title: "Error", template: errorMsg});
				}
				$ionicLoading.hide();
				$state.go("app.home");
			})
			.finally(function () {
				$ionicLoading.hide();
			});
	};
	$scope.wifi = {checked: !NetworkService.isOnline()};
	$scope.toggleWiFi = function () {
		console.log($scope.wifi.checked);
		if (!$scope.wifi.checked) {
			NetworkService.fakingIt(false);
			console.log(NetworkService.areYouFakingIt());
		} else {
			NetworkService.fakingIt(true);
			console.log(NetworkService.areYouFakingIt());
		}
	};
});