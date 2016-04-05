(function(app) {

  'use strict';
  app.service('BackButtonService', function($ionicPlatform) {

    var self = this;
    var BackButtonService = function() {
      self = this;
      this.enableBackButton = true;
    };

    BackButtonService.prototype.init = function() {
      $ionicPlatform.registerBackButtonAction(function(event) {
        if (self.isEnable()) {
          navigator.app.exitApp();
        }
      }, 100);
    };

    BackButtonService.prototype.enable = function() {
      this.enableBackButton = true;
    };

    BackButtonService.prototype.disable = function() {
      this.enableBackButton = false;
    };

    BackButtonService.prototype.isEnable = function() {
      return this.enableBackButton;
    };

    return new BackButtonService();
  });


})(app);
