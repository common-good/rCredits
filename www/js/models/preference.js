(function(window) {

  var Preference = Class.create({

  });

  Preference.getDefinitions = function() {
    var preferences = [
      {
        id: 'self_service_mode',
        type: 'toggle',
        name: 'selfServiceMode'
      },
      {
        id: 'enable_cashier',
        type: 'toggle',
        name: 'enableCashierMode'
      },
      {
        id: 'cashier_can',
        type: 'checkbox',
        name: 'cashierCan',
        options: [
          {
            id: 'charge',
            name: 'charge'
          },
          {
            id: 'refund',
            name: 'refund'
          },
          {
            id: 'trade_rcredits_for_usd',
            name: 'tradeRCreditsUSD'
          },
          {
            id: 'trade_usd_for_rcredtis',
            name: 'tradeUSDRCredits'
          }
        ]
      }
    ];

    return _.map(preferences, Preference.parse);
  };

  Preference.parse = function(jsonPreference) {
    if (jsonPreference.id == 'cashier_can') {
      return _.extendOwn(new CashierMode(), jsonPreference);
    }

    return _.extendOwn(new Preference(), jsonPreference);
  };

  Preference.prototype.isToggle = function() {
    return this.type === 'toggle';
  };

  Preference.prototype.isCheckbox = function() {
    return this.type === 'checkbox';
  };

  Preference.prototype.hasOptions = function() {
    return this.options && this.options.length;
  };

  Preference.prototype.isEnabled = function() {
    return this.value === true;
  };


  window.Preference = Preference;

})(window);
