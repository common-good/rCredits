(function(window, app) {

  var _ = window._;
  app.service('MoneyType', function() {

    var MoneyType = Class.create({

      name: '',
      sign: '',

      initialize: function($super) {
      },

      getName: function() {
        return this.name;
      },

      getSign: function() {
        return this.sign;
      },

    });

    MoneyType.parseMoneyType = function(jsonMoney) {
      return _.extendOwn(new MoneyType(), jsonMoney);
    };


    return MoneyType;
  });
})(window, app);
