(function(app) {
  'use strict';

  app.service('PreferenceService', function(localStorageService) {

    var self;
    var PreferenceService = function() {
      self = this;
      this.preferences = [];
    };

    PreferenceService.PREFERENCES_KEY = 'seller_preferences';

    PreferenceService.prototype.getAll = function() {
      if (this.preferences.length) {
        return this.preferences;
      }

      var savedPreferences = this.loadSavedPreferences_();
      var prefsDefinition = Preference.getDefinitions();

      _.each(savedPreferences, _.partial(this.updatePref_, prefsDefinition).bind(this));
      return prefsDefinition;
    };

    PreferenceService.prototype.updatePref_ = function(preferences, pref) {
      var prefToUpdate = _.find(preferences, function(p) {
        return p.id === pref.id
      });

      if (prefToUpdate) {
        _.extendOwn(prefToUpdate, pref);
      }
    };

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

    PreferenceService.prototype.getCashierModePref = function() {
      return _.find(this.getAll(), function(p) {
        return p.id == 'enable_cashier';
      });
    };

    PreferenceService.prototype.isCashierModeEnabled = function() {
      return this.getCashierModePref().isEnabled();
    };

    return new PreferenceService();
  });

})(app);
