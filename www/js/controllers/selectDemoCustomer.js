app.controller('SelectDemoCust', function ($scope, $state, $stateParams, $ionicLoading, $filter, NotificationService, UserService, TransactionService) {
	$scope.selectedCategory = {
		selected: $scope.categories[0]
	};
	$scope.onSelectCustomer = function () {
		var selected = $scope.selectedCategory.selected;
		console.log(selected);
		if (selected === 'Susan Shopper') {
			UserService.loginWithRCard({op: "identify", device: "F83swEagSJA9jnDGD5dh", agent: "NEW.AAB", version: 300, member: "NEW.ABB", code: "ZzhWMCq0zcBowqw"}, {isPersonal: true, isCompany: false, memberId: "NEW", accountId: "NEW.ABB", securityCode: "ZzhWMCq0zcBowqw", url: "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBow…", serverType: "rc4"});
		}
	};
});