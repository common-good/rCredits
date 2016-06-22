app.module('enoughSpace', [])
	.controller('EnoughSpace', ['$scope', '$interval',
		function ($scope, $interval) {
			$scope.format = 'M/d/yy h:mm:ss a';
			var stop;
			$scope.querySpace = function () {
				if (angular.isDefined(stop)) {
					return;
				}
				stop = $interval(function () {
				}, 1000);
			};
			$scope.stopQuery = function () {
				if (angular.isDefined(stop)) {
					$interval.cancel(stop);
					stop = undefined;
				}
			};
			$scope.$on('$destroy', function () {
				$scope.stopQuery()();
			});
		}])
	.directive('nextCheck', ['$interval', 'dateFilter',
		function ($interval, dateFilter) {
			return function (scope, element, attrs) {
				var stopTime; 
				function updateStorage() {
					cordova.exec(function (result) {
						if (result <= 999999999999999) {
							console.log("Low Disk Space: " + result);
							return false;
						} else {
							console.log("Plenty of Disk Space: " + result);
							return true;
						}
					}, function (error) {
						console.log("Error!... The details follow: " + error);
						return error;
					}, "File", "getFreeDiskSpace", []);
				}
				scope.$watch(attrs.myCurrentTime, function (value) {
					updateStorage();
				});
				stopTime = $interval(updateStorage, 1000);
				element.on('$destroy', function () {
					$interval.cancel(stopTime);
				});
			};
		}]);