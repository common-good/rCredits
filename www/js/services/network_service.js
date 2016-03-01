(function(app) {
  'use strict';

  app.service('NetworkService', function($rootScope) {

    var self;
    var NetworkService = function() {
      self = this;
      this.connectionOnline = true;
      this.init_();
    };

    NetworkService.prototype.init_ = function() {
      document.addEventListener("online", this.onOnline.bind(this), false);
      document.addEventListener("offline", this.onOffline.bind(this), false);

      if (navigator && navigator.connection) {
        this.connectionOnline = navigator.connection.type !== 'none';
      }
    };

    NetworkService.prototype.onOffline = function() {
      this.connectionOnline = false;
      $rootScope.$apply();
    };

    NetworkService.prototype.onOnline = function() {
      this.connectionOnline = true;
      $rootScope.$apply();
    };

    NetworkService.prototype.isOffline = function() {
      return !this.connectionOnline;
    };

    NetworkService.prototype.isOnline = function() {
      return this.connectionOnline;
    };


    return new NetworkService();
  });

})(app);
