(function(app) {
  'use strict';

  app.service('PreferenceService', function(localStorageService) {

    var self;
    var PreferenceService = function() {
      self = this;
      this.preferences = this.getAll();
    };

    PreferenceService.PREFERENCES_KEY = 'seller_preferences';

    PreferenceService.prototype.getAll = function() {
      if (this.preferences && this.preferences.length) {
        return this.preferences;
      }

      var savedPreferences = this.loadSavedPreferences_();
      var preferences = Preference.getDefinitions();

      _.each(savedPreferences, _.partial(this.updatePref_, preferences).bind(this));
      console.log("Prefs => ", preferences);
      return preferences;
    };

    /**
     * Finds a pref for "Preferences" and merge all properties into the found one.
     * @param preferences All prefereces to have updated
     * @param pref pref to get properties
     * @private
     */
    PreferenceService.prototype.updatePref_ = function(preferences, pref) {
      var prefToUpdate = _.find(preferences, function(p) {
        return p.id === pref.id
      });

      if (prefToUpdate) {
        _.extendOwn(prefToUpdate, pref);
      }
    };

    /**
     * Returns a list of preferences saved in localStorage
     * @private
     */
    PreferenceService.prototype.loadSavedPreferences_ = function() {
      var strPrefs = localStorageService.get(PreferenceService.PREFERENCES_KEY);
      if (strPrefs) {
        var jsonPrefs = _.map(strPrefs, JSON.parse);
        return _.map(jsonPrefs, Preference.parse);
      }
    };

    PreferenceService.prototype.savePreferences = function(preferences) {
      if (!_.isArray(preferences)) {
        console.log("Preferences must be an Array");
        return;
      }

      localStorageService.remove(PreferenceService.PREFERENCES_KEY);

      localStorageService.set(PreferenceService.PREFERENCES_KEY, _.map(preferences, window.angular.toJson));
    };

    PreferenceService.prototype.getPrefById = function(prefId) {
      return _.find(this.getAll(), function(p) {
        return p.id == prefId;
      });
    };

    PreferenceService.prototype.getCashierModePref = function() {
      return this.getPrefById('enable_cashier');
    };

    /**
     * Returns the Pref which says what can Cashier Mode do
     */
    PreferenceService.prototype.getCashierCanPref = function() {
      return this.getPrefById('cashier_can');
    };

    PreferenceService.prototype.isCashierModeEnabled = function() {
      return this.getCashierModePref().isEnabled();
    };

    PreferenceService.prototype.isSelfServiceEnabled = function() {
      return this.getPrefById('self_service_mode').isEnabled();
    };

    return new PreferenceService();
  });

})(app);
