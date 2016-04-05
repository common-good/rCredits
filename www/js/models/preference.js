(function(window) {

  var Preference = Class.create({});

  Preference.getDefinitions = function() {
    var preferences = [
      {
        id: 'forget_company',
        type: 'toggle',
        name: 'forget_company'
      },
      {
        id: 'enable_cashier',
        type: 'toggle',
        name: 'enableCashierMode',
        value: true,
        display: false
      },
      {
        id: 'cashier_can',
        type: 'checkbox',
        name: 'cashierCan',
        subtitle: 'cashierCanSubtitle',
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
    var p;
    if (jsonPreference.id == 'cashier_can') {
      p = _.extendOwn(new CashierMode(), jsonPreference);
    } else {
      p = _.extendOwn(new Preference(), jsonPreference);
    }

    if (!p.hasOwnProperty('display')) {
      p.display = true;
    }
    return p;
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
