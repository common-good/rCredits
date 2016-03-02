(function(app) {

  app.service('TransactionSql', function($q, SqlQuery, SQLiteService) {

    var self;

    var TransactionSql = function() {
      self = this;
    };

    TransactionSql.prototype.getOfflineTransaction = function() {
      var sqlQuery = new SqlQuery();
      sqlQuery.setQueryString("SELECT * FROM txs where STATUS = " + Transaction.Status.OFFLINE + " order by rowid asc limit 1");
      return SQLiteService.executeQuery(sqlQuery).then(function(SQLResultSet) {
        console.log("Offline transaction: ", SQLResultSet);
        if (SQLResultSet.rows.length > 0) {
          return SQLResultSet.rows[0];
        } else {
          throw "No offline transactions";
        }
      });
    };

    TransactionSql.prototype.setTransactionSynced = function(sqlTransaction) {
      var sqlQuery = new SqlQuery();
      sqlQuery.setQueryString("UPDATE txs SET status = '" + Transaction.Status.DONE + "' where created = " + sqlTransaction.created);
      console.log(sqlQuery.getQueryString());
      return SQLiteService.executeQuery(sqlQuery);
    };

    return new TransactionSql();

  });


})(app);
