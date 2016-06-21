/* global app */
(function (app) {
	'use strict';
	app.module('enoughSpace',[]), function ($rootScope, $injector, $window) {
		var scope = $rootScope;
		scope.enough = true;
		scope.enoughSpace = cordova.exec(
			function (result) {
				console.log()
				return result;
			},
			function (error) {
				console.log("Error!... The details follow: " + error);
				throw error;
			}, "File", "getFreeDiskSpace", []
			);
		scope.$watch('enoughSpace',
			function (newAmount, oldAmount) {
				if (newAmount !== oldAmount) {
					if (newAmount <= 99999999) {
						console.log("Low Disk Space: " + newAmount);
						scope.enough = false;
//						return scope.enough;
					} else {
						console.log("Plenty of Disk Space: " + newAmount);
						scope.enough = true;
//						return scope.enough;
					}
				}
			}
		);
		$timeout(function () {
			scope.$digest();
		}, 1000);
	});
});