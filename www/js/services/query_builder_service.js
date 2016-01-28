(function(app) {

  app.service('QueryBuilderService', function($q, SqlQuery) {

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
    };

    QueryBuilderService.prototype.buildCustomerQuery = function(customer) {

      // TODO: Save last TxID time

      //"qid TEXT," + // customer or manager account code (like NEW.AAA or NEW:AAA)
      //"name TEXT," + // full name (of customer or manager)
      //"company TEXT," + // company name, if any (for customer or manager)
      //"place TEXT," + // customer location / manager's company account code  ??????
      //"balance REAL," + // current balance (as of lastTx) / manager's rCard security code
      //"rewards REAL," + // rewards to date (as of lastTx) / manager's permissions / photo ID (!rewards.matches(NUMERIC))
      //"lastTx INTEGER," + // unixtime of last reconciled transaction / -1 for managers
      //"photo BLOB);" // lo-res B&

      var sqlQuery = new SqlQuery();
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


    };

    return new QueryBuilderService();
  });

})(app);
