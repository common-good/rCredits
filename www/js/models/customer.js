(function(window, app) {

  app.service('Customer', function() {

    var Customer = Class.create(User, {

      balance: 0,
      reward: null

    });

    window.Customer = Customer;

    return Customer;
  });
}) (window, app);
