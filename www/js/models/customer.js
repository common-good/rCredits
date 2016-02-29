(function(window, app) {

  app.service('Customer', function(User) {

    var Customer = Class.create(User, {

      balance: 0,
      rewards: null,
      lastTx: null,

      setRewards: function(rewards) {
        this.rewards = parseFloat(rewards);
      },

      setBalance: function(balance) {
        this.balance = balance;
        return this;
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
