(function(window, app) {

  var _ = window._;
  app.service('MoneyType', function() {

    var MoneyType = Class.create({

      name: '',

      initialize: function($super) {
      },

      getName: function() {
        return this.name;
      }

    });

    MoneyType.parseMoneyType = function(jsonMoney) {
      return _.extendOwn(new MoneyType, jsonMoney);
    };


    return MoneyType;
  });
})(window, app);
