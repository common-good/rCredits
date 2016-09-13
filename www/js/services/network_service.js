/* global app */
(function (app) {
	'use strict';
	app.service('NetworkService', function ($rootScope, $timeout) {
		var self;
		var NetworkService = function () {
			self = this;
			this.connectionOnline = true;
			this.connectionStateChange=false;
			this.fakeIt=false;
			this.init_();
		};
		NetworkService.prototype.init_ = function () {
			document.addEventListener("online", this.onOnline.bind(this), false);
			document.addEventListener("offline", this.onOffline.bind(this), false);
			if (navigator && navigator.connection) {
				this.connectionOnline = navigator.connection.type !== 'none';
			}
			$timeout(function () {
				if (self.isOnline()&&!self.fakeIt) {
					self.onOnline();
				} else if(!self.fakeIt){
					self.onOffline();
				}
			}, 1000);
		};
		NetworkService.prototype.onOffline = function () {
			$rootScope.$apply();
			$rootScope.$emit('onOffline');
			this.connectionStateChange=false;
			this.connectionOnline = false;
		};
		NetworkService.prototype.onOnline = function () {
			this.connectionStateChange=true;
			$rootScope.$apply();
			$rootScope.$emit('onOnline');
			this.connectionOnline = true;
		};
		NetworkService.prototype.isOffline = function () {
			this.connectionStateChange=false;
			return !this.connectionOnline;
		};
		NetworkService.prototype.isOnline = function () {
			this.connectionStateChange=true;
			return this.connectionOnline;
		};
		NetworkService.prototype.fakingIt = function (areYou) {
			this.connectionStateChange=false;
			 this.fakeIt=areYou;
			 return this.connectionOnline=!areYou;
		};
		NetworkService.prototype.areYouFakingIt = function () {
			this.connectionStateChange=false;
			return this.fakeIt;
		};
		return new NetworkService();
	});
})(app);
