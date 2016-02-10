(function(app) {
  'use strict';

  app.service('CashierModeService', function(PreferenceService) {

    var self;
    var CashierModeService = function() {
      self = this;
      this.isActivated = false;
    };

    /**
     * Checks if the Preference 'Cashier Mode' is on and
     * if the user is on this Mode
     * @returns boolean
     */
    CashierModeService.prototype.isEnabled = function() {
      return this.isActivated && PreferenceService.isCashierModeEnabled();
    };

    CashierModeService.prototype.disable = function() {
      this.isActivated = false;
    };

    CashierModeService.prototype.activateCashierMode = function() {
      if (!PreferenceService.isCashierModeEnabled()) {
        throw new Error("Unable to activate Cashier Mode because it's not enable on Preferences.")
      }
      this.isActivated = true;
    };

    CashierModeService.prototype.canCharge = function() {
      if (!this.isActivated) {
        return true;
      }
      return PreferenceService.getCashierCanPref().isChargeEnabled();
    };

    CashierModeService.prototype.canRefund = function() {
      if (!this.isActivated) {
        return true;
      }
      return PreferenceService.getCashierCanPref().isRefundEnabled();
    };

    CashierModeService.prototype.canExchange = function() {
      if (!this.isActivated) {
        return true;
      }
      return PreferenceService.getCashierCanPref().isRefundEnabled();
    };


    return new CashierModeService();
  });

})(app);
