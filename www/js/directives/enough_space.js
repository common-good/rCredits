/* global app */
(function (app) {
	'use strict';
	app.directive('enoughSpace', function ($window) {
		return function (scope, element) {
			scope.enoughSpace = cordova.exec(function (result) {
				if (result <= 99999999) {
					console.log("Low Disk Space: " + result);
					throw false;
				} else {
					console.log("Plenty of Disk Space: " + result);
				}
			}, function (error) {
				console.log("Error!... The details follow: " + error);
				throw error;
			}, "File", "getFreeDiskSpace", []);
		};
	});
});