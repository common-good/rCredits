(function(app) {
  'use strict';


  app.service('PreferenceService', function(localStorageService) {

    var self;
    var PreferenceService = function() {
      self = this;
    };

    PreferenceService.PREFERENCES_KEY = 'seller_preferences';

    PreferenceService.prototype.getAll = function() {
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


    return new PreferenceService();
  });

})(app);
