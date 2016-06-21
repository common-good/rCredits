/* global app */
(function (app) {
	app.service('EnoughSpace', function () {
		var EnoughSpace = function () {
		};
		EnoughSpace.prototype.enoughSpace = function () {
			cordova.exec(function (result) {
				if (result <= 99999999) {
					console.log("Low Disk Space: " + result);
					throw false;
				} else {
					console.log("Plenty of Disk Space: " + result);
					return true;
				}
			}, function (error) {
				console.log("Error!... The details follow: " + error);
				return error;
			}, "File", "getFreeDiskSpace", []);
			return true;
		};
		return new EnoughSpace;
	});
})(app);