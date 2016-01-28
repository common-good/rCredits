(function(app) {

  app.service('QueryBuilderService', function($q, SqlQuery, SQLiteService) {

    var self;

    var QueryBuilderService = function() {
      self = this;
    };

    /**
     *
     * @param seller
     * @returns {SqlQuery}
     */
    QueryBuilderService.prototype.buildSellerQuery = function(seller) {
      var sqlQuery = new SqlQuery();
      //"qid TEXT," + // customer or manager account code (like NEW.AAA or NEW:AAA)
      //"name TEXT," + // full name (of customer or manager)
      //"company TEXT," + // company name, if any (for customer or manager)
      //"place TEXT," + // customer location / manager's company account code  ??????
      //"balance REAL," + // current balance (as of lastTx) / manager's rCard security code
      //"rewards REAL," + // rewards to date (as of lastTx) / manager's permissions / photo ID (!rewards.matches(NUMERIC))
      //"lastTx INTEGER," + // unixtime of last reconciled transaction / -1 for managers
      //"photo BLOB);" // lo-res B&

      return this.existMember(seller.accountInfo.accountId)
        .then(function(sqlMemeber) {
          console.log("SQL seller: ", sqlMemeber);
          sqlQuery.setQueryString("UPDATE members SET " +
            "name = '" + seller.name + "'," +
            "company = '" + seller.company + "'," +
            "place = '" + seller.accountInfo.accountId + "'," +
            "balance = '" + seller.accountInfo.securityCode + "'," +
            "rewards = '" + seller.can + "'," +
            "lastTx = '" + "-1" + "'," +
            "photo = '" + " " + "' " +
            " WHERE qid = '" + seller.accountInfo.accountId + "'"
          );
          return sqlQuery;
        })
        .catch(function(errorMessage) {
          console.log(errorMessage);
          sqlQuery.setQueryString('INSERT INTO members (qid, name, company, place, balance, rewards, lastTx, photo) VALUES (?,?,?,?,?,?,?,?)');
          sqlQuery.setQueryData([
            seller.accountInfo.accountId,
            seller.name,
            seller.company,
            seller.accountInfo.accountId,
            seller.accountInfo.securityCode,
            seller.can,
            -1,
            null
          ]);
          return sqlQuery;
        });
    };

    QueryBuilderService.prototype.buildCustomerQuery = function(customer) {
      var sqlQuery = new SqlQuery();

      return this.existMember(customer.accountInfo.accountId)
        .then(function(sqlMemeber) {
          console.log("SQL Customer: ", sqlMemeber);
          sqlQuery.setQueryString("UPDATE members SET " +
            "name = '" + customer.name + "'," +
            "company = '" + customer.company + "'," +
            "place = '" + customer.place + "'," +
            "balance = '" + customer.balance + "'," +
            "rewards = '" + customer.rewards + "'," +
            "lastTx = '" + "-1" + "'," +
            "photo = '" + customer.accountInfo.blobImage + "' " +
            " WHERE qid = '" + customer.accountInfo.accountId + "'"
          );
          return sqlQuery;
        })
        .catch(function(errorMessage) {
          console.log(errorMessage);
          sqlQuery.setQueryString('INSERT INTO members (qid, name, company, place, balance, rewards, lastTx, photo) VALUES (?,?,?,?,?,?,?,?)');
          sqlQuery.setQueryData([
            customer.accountInfo.accountId,
            customer.name,
            customer.company,
            customer.place,
            customer.balance,
            customer.rewards,
            -1,
            customer.accountInfo.blobImage
          ]);
          return sqlQuery;
        });

      // TODO: Save last TxID time


    };

    QueryBuilderService.prototype.existMember = function(qId) {
      var sqlQuery = new SqlQuery();
      sqlQuery.setQueryString("SELECT * FROM members WHERE qid = ?");
      sqlQuery.setQueryData(qId);
      return SQLiteService.executeQuery(sqlQuery).then(function(SQLResultSet) {
        console.log("Exist Member Id: " + qId + ' :', SQLResultSet);
        if (SQLResultSet.rows.length > 0) {
          return SQLResultSet.rows[0];
        } else {
          throw "Member not Exists: " + qId;
        }
      });
    };

    return new QueryBuilderService();
  });

})(app);
