(function (app) {
	app.service('EnoughSpace', function () {
		var EnoughSpaceService=function (){
			var self=this;
		};
		EnoughSpaceService.prototype.enoughSpace = function () {
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
	});
})(app);