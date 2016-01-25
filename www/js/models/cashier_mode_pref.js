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
    }


  });


  window.CashierMode = CashierMode;

})(window, app);
