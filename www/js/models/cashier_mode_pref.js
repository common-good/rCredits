(function(window, app) {

  var CashierMode = Class.create(window.Preference, {

    isOptionEnabled: function(optionId) {
      var chargeOption = _.find(this.options, function(o) {
        return o.id == optionId
      });
      return chargeOption && chargeOption.value;
    },

    isChargeEnabled: function() {
      return this.isOptionEnabled('charge');
    },

    isRefundEnabled: function() {
      return this.isOptionEnabled('refund');
    },

    isExchangeEnabled: function() {
      return this.isOptionEnabled('refund');
    },

    getPref_: function(id) {
      return _.find(this.options, function(o) {
        return o.id == id;
      });
    },

    setPropertyValue: function(id, boolCan) {
      this.getPref_(id).value = boolCan;
    },

    setCanCharge: function(boolCan) {
      this.setPropertyValue('charge', boolCan);
    },

    setCanRefund: function(boolCan) {
      this.setPropertyValue('refund', boolCan);
    },

    setCanTradeRcreditsForUSD: function(boolCan) {
      this.setPropertyValue('trade_rcredits_for_usd', boolCan);
    },

    setCanTradeUSDforRcredits: function(boolCan) {
      this.setPropertyValue('trade_usd_for_rcredtis', boolCan);
    },

    enableAll: function() {
      _.each(this.options, function(o) {
        o.value = true;
      });
    },

    disableAll: function() {
      _.each(this.options, function(o) {
        o.value = false;
      });
    }

  });


  window.CashierMode = CashierMode;

})(window, app);
