(function(window, app) {

  app.service('Customer', function() {

    var Customer = Class.create(User, {

      balance: 0,
      rewards: null,
      lastTx: null,

      setRewards: function(rewards) {
        this.rewards = parseFloat(rewards);
      },

      getPlace: function() {
        return this.place;
      },

      getBalance: function() {
        return this.balance;
      },

      getRewards: function() {
        return this.rewards;
      },

      setLastTx: function(transaction) {
        this.lastTx = transaction;
      },

      getLastTx: function() {
        return this.lastTx.getId();
      }

    });


    window.Customer = Customer;

    return Customer;
  });
})(window, app);
