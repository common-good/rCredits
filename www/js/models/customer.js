(function(window, app) {

  app.service('Customer', function() {

    var Customer = Class.create(User, {

      balance: 0,
      rewards: null,

      setRewards: function(rewards) {
        this.rewards = parseFloat (rewards);
      }
    });


    window.Customer = Customer;

    return Customer;
  });
}) (window, app);
