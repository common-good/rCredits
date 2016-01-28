(function(window, app) {

  app.service('Customer', function(QueryBuilderService, SQLiteService) {

    var Customer = Class.create(User, {

      balance: 0,
      rewards: null,

      setRewards: function(rewards) {
        this.rewards = parseFloat(rewards);
      },

      saveInSQLite: function() {
        var sqlQuery = QueryBuilderService.buildCustomerQuery(this);
        return SQLiteService.executeQuery(sqlQuery);
      }
    });


    window.Customer = Customer;

    return Customer;
  });
})(window, app);
