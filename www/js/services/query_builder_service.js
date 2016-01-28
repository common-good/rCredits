(function(app) {

  app.service('QueryBuilderService', function($q, SqlQuery, SQLiteService) {

    var self;

    var QueryBuilderService = function() {
      self = this;
    };

    //"qid TEXT," + // customer or manager account code (like NEW.AAA or NEW:AAA)
    //"name TEXT," + // full name (of customer or manager)
    //"company TEXT," + // company name, if any (for customer or manager)
    //"place TEXT," + // customer location / manager's company account code  ??????
    //"balance REAL," + // current balance (as of lastTx) / manager's rCard security code
    //"rewards REAL," + // rewards to date (as of lastTx) / manager's permissions / photo ID (!rewards.matches(NUMERIC))
    //"lastTx INTEGER," + // unixtime of last reconciled transaction / -1 for managers
    //"photo BLOB);" // lo-res B&

    QueryBuilderService.prototype.saveMember = function(user) {
      var sqlQuery = new SqlQuery();

      return this.existMember(user.accountInfo.accountId)
        .then(function(sqlMemeber) {
          console.log("SQL Member: ", sqlMemeber);
          sqlQuery.setQueryString("UPDATE members SET " +
            "name = '" + user.getName() + "'," +
            "company = '" + user.getCompany() + "'," +
            "place = '" + user.getPlace() + "'," +
            "balance = '" + user.getBalance() + "'," +
            "rewards = '" + user.getRewards() + "'," +
            "lastTx = '" + user.getLastTx() + "'," +
            "photo = '" + user.getBlobImage() + "' " +
            " WHERE qid = '" + user.getId() + "'"
          );
          return sqlQuery;
        })
        .catch(function(errorMessage) {
          console.log(errorMessage);
          sqlQuery.setQueryString('INSERT INTO members (qid, name, company, place, balance, rewards, lastTx, photo) VALUES (?,?,?,?,?,?,?,?)');
          sqlQuery.setQueryData([
            user.getId(),
            user.getName(),
            user.getCompany(),
            user.getPlace(),
            user.getBalance(),
            user.getRewards(),
            user.getLastTx(),
            user.getBlobImage()
          ]);
          return sqlQuery;
        })
        .then(function(sqlQuery) {
          return SQLiteService.executeQuery(sqlQuery);
        });
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
