(function(app) {
  'use strict';

  app.service('NetworkService', function($rootScope, $timeout) {

    var self;
    var NetworkService = function() {
      self = this;
      this.connectionOnline = false;
      //this.init_();
    };

    NetworkService.prototype.init_ = function() {
      document.addEventListener("online", this.onOnline.bind(this), false);
      document.addEventListener("offline", this.onOffline.bind(this), false);

      if (navigator && navigator.connection) {
        this.connectionOnline = navigator.connection.type !== 'none';
      }

      $timeout(function() {
        if (self.isOnline()) {
          self.onOnline();
        } else {
          self.onOffline();
        }
      }, 5000);
    };

    NetworkService.prototype.onOffline = function() {
      this.connectionOnline = false;
      $rootScope.$apply();
      $rootScope.$emit('onOffline');
    };

    NetworkService.prototype.onOnline = function() {
      this.connectionOnline = true;
      $rootScope.$apply();
      $rootScope.$emit('onOnline');
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
