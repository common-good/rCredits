/* global app, $ionicHistory 
 {name: 'Cathy Cashier', url: 'HTTP://NEW.RC4.ME/ABJ-ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
 {name: 'Bob Bossman', url: 'HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G', signin: '1', img: 'img/BobBossman.jpg'},
 {name: 'Curt Customer', url: 'HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k', signin: '0', img: 'img/CurtCustomerMember.jpg'},
 {name: 'Susan Shopper', url: 'HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
 {name: "Curt-Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
 * */
app.controller('SelectDemoCust', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService, $location, $rootScope, NetworkService, $ionicHistory) {
	$scope.populateDemoCustomers = [
		[
			{name: 'Cathy Cashier', url: 'HTTP://NEW.RC4.ME/ABJ-ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
			{name: 'Bob Bossman', url: 'HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G', signin: '1', img: 'img/BobBossman.jpg'},
			{name: 'Curt Customer', url: 'HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k', signin: '0', img: 'img/CurtCustomerMember.jpg'},
			{name: 'Susan Shopper', url: 'HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
			{name: "Curt-Helga's Hardware", url: 'HTTP://NEW.RC4.ME/AAD-utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
		], [
			{name: 'Cathy', url: 'HTTP://6VM.RC4.ME/H021ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
			{name: 'Bob', url: 'HTTP://6VM.RC4.ME/H010WeHlioM5JZv1O9G:somethingForBob', signin: '1', img: 'img/BobBossman.jpg'},
			{name: 'Curt', url: 'HTTP://6VM.RC4.ME/G0ANyCBBlUF1qWNZ2k.something', signin: '0', img: 'img/CurtCustomerMember.jpg'},
			{name: 'Susan', url: 'HTTP://6VM.RC4.ME/G0RZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
			{name: "Curt's Hardware", url: 'HTTP://6VM.RC4.ME/H0G0utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
		], [
			{name: 'Cathy', url: 'H6VM021ME04nW44DHzxVDg', signin: '1', img: 'img/CathyCashier.jpg'},
			{name: 'Bob', url: 'H6VM010WeHlioM5JZv1O9G:somethingForBob', signin: '1', img: 'img/BobBossman.jpg'},
			{name: 'Curt', url: 'G6VM0ANyCBBlUF1qWNZ2k.something', signin: '0', img: 'img/CurtCustomerMember.jpg'},
			{name: 'Susan', url: 'G6VM0RZzhWMCq0zcBowqw', signin: '0', img: 'img/SusanShopper.jpg'},
			{name: "Curt's Hardware", url: 'H6VM0G0utbYceW3KLLCcaw', signin: '1', img: 'img/CurtCustomerAgent.jpg'}
		]
	];
	var formats = document.getElementsByName('formattype');
//	var typeOfQR=['old','new','short'];
	$scope.iswebview = ionic.Platform.platform();
	$scope.format = {
		type: 1
	};
	for (var i = 0; i < formats.length; i++) {
		formats[i].onclick = function () {
			$scope.format.type = this.value;
			console.log($scope.format.type, this.value);
			$scope.customer = $scope.populateDemoCustomers[$scope.format.type];
			$scope.manager = $scope.populateDemoCustomers[$scope.format.type];
		};
	}
	var type_Of_QR = $scope.format.type[0];
	console.log($scope.format.type);
	//[{value: 'old', text: 'Old'}, {value: 'new', text: 'New'}, {value: 'short', text: 'Short'}];
//	$scope.changeOfType={
//		selected:$scope.format.type
//	};
	$scope.data = {
		clientSide: 'new'
	};
	$scope.customer = $scope.populateDemoCustomers[$scope.format.type];
	$scope.selectedCustomer = {
		selected: $scope.customer
	};
	$scope.manager = $scope.populateDemoCustomers[$scope.format.type];
	$scope.selectedManager = {
		selected: $scope.manager
	};
	$scope.whereWasI = $rootScope.whereWasI;
	$scope.onSelectCustomer = function (person) {
		var selected = person;
		console.log(selected, $location.state(), $scope.whereWasI, $scope.format.type);
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