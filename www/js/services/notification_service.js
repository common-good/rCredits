/* global app, _ */
(function (app) {
	'use strict';
	app.factory('NotificationService', ['$translate', '$ionicPopup', function ($translate, $ionicPopup) {
			var self;
			var NotificationService = function () {
				self = this;
			};
			NotificationService.prototype.showAlert = function (options, params) {
				if (!_.isObject(options)) {
					options = {
						template: options
					};
				}
				return $translate([options.title, options.subTitle, options.template], params).then(function (translations) {
					if (options.title === 'error' || options.title === 'Error') {
						options.title = '<span class="errorPopupTitle">'+translations[options.title]+'</span>';
					} else {
						options.title = translations[options.title];
					}
					options.subTitle = translations[options.subTitle];
					options.template = translations[options.template];
					return $ionicPopup.alert(options);
				});
			};
			NotificationService.prototype.showConfirm = function (options, params) {
				return $translate([options.title, options.subTitle, options.template, options.okText, options.cancelText], params).
					then(function (translations) {
						options.title = translations[options.title];
						options.subTitle = translations[options.subTitle];
						options.template = translations[options.template];
						options.okText = translations[options.okText];
						options.cancelText = translations[options.cancelText];
						return $ionicPopup.confirm(options);
					});
			};
			return new NotificationService();
		}]);
})(app);
