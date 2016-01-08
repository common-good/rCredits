(function(window, app) {

  app.service('Seller', function(localStorageService) {

    var DEVICE_ID_KEY = 'deviceID';

    var Seller = Class.create(User, {

      descriptions: [],
      device: '',
      firstLogin: false,

      initialize: function($super, name) {
        $super(name);
        this.configureDeviceId_();
      },

      isValidDeviceId: function(device) {
        return !_.isEmpty(device);
      },

      configureDeviceId_: function() {
        var localDeviceId = localStorageService.get(DEVICE_ID_KEY);
        if (this.isValidDeviceId(localDeviceId)) {
          this.device = localDeviceId;
        }
      },

      setDeviceId: function(device) {
        if (!this.isValidDeviceId(device)) {
          throw new Error('Invalid deviceID: ' + device);
        }
        this.device = device;
        localStorageService.set(DEVICE_ID_KEY, device);
      },

      hasDevice: function() {
        return !_.isEmpty(this.device);
      },

      isFirstLogin: function() {
        return this.firstLogin;
      }

    });

    window.Seller = Seller;

    return Seller;
  });
})(window, app);
