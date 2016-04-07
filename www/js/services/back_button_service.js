(function(app) {

  'use strict';
  app.service('BackButtonService', function($ionicPlatform) {

    var self = this;
    var desregisterBackButton;
    var BackButtonService = function() {
      self = this;
      this.enableBackButton = true;
    };

    BackButtonService.prototype.init = function() {
    };

    BackButtonService.prototype.disable = function() {
      this.enableBackButton = true;
      desregisterBackButton = $ionicPlatform.registerBackButtonAction(function(event) {
        if (!self.isEnable()) {
          event.preventDefault();
          event.stopPropagation();
        }
      }, 100);
    };

    BackButtonService.prototype.enable = function() {
      this.enableBackButton = false;
      if (desregisterBackButton) {
        desregisterBackButton();
        desregisterBackButton = null;
      }
    };

    BackButtonService.prototype.isEnable = function() {
      return this.enableBackButton;
    };

    return new BackButtonService();
  });


})(app);
