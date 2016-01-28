(function(window, app) {

  app.service('Customer', function(QueryBuilderService, SQLiteService) {

    var Customer = Class.create(User, {

      balance: 0,
      rewards: null,

      setRewards: function(rewards) {
        this.rewards = parseFloat(rewards);
      },

      saveInSQLite: function() {
        return QueryBuilderService.buildCustomerQuery(this).then(SQLiteService.executeQuery.bind(SQLiteService));
      }
    });


    window.Customer = Customer;

    return Customer;
  });
})(window, app);
