(function(app) {
  'use strict';

  app.service('SelfServiceMode', function() {

    var self;
    var isSelfServiceModeActive = false;

    var SelfServiceMode = function() {
      self = this;
    };

    SelfServiceMode.prototype.active = function() {
      isSelfServiceModeActive = true;
    };

    SelfServiceMode.prototype.disable = function() {
      isSelfServiceModeActive = false;
    };

    SelfServiceMode.prototype.isActive = function() {
      return isSelfServiceModeActive;
    };

    return new SelfServiceMode();
  });

})(app);
