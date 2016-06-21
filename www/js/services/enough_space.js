/* global app */
(function (window) {
	var EnoughSpace = function () {
	};
	EnoughSpace.prototype.enoughSpace = function ($rootScope, $http) {
		cordova.exec(function (result) {
			console.log("Free Disk Space: " + result);
			if (result <= 7813952) {
				return false;
			} else {
				return true;
			}
		}, function (error) {
			console.log("Error: " + error);
			return error;
		}, "File", "getFreeDiskSpace", []);
		return true;
	};
})(window);