app.service('TransactionSyncService',
  function($q, SQLiteService, SqlQuery, NetworkService, TransactionSql, $timeout) {

    var self;

    var TransactionSyncService = function() {
      self = this;
    };

    var send = function() {
      var ok = true;

      var q = $q.defer();
      if (ok)
        q.resolve();
      else
        q.reject();

      return q.promise;
    };

    TransactionSyncService.prototype.syncOfflineTransactions = function() {

      if (NetworkService.isOffline()) {
        return;
      }

      var sqlTransaction;
      TransactionSql.getOfflineTransaction()
        .then(function(sqlTransac) {
          sqlTransaction = sqlTransac;
          console.log("sqlTransaction: ", sqlTransaction);
          // send to server
          return send();
        })
        .then(function() {
          return TransactionSql.setTransactionSynced(sqlTransaction);
        })
        //.then(self.syncOfflineTransactions.bind(self))
        .then(function() {
          $timeout(self.syncOfflineTransactions.bind(self), 1000);
        })
        .catch(function(err) {
          // err no transactions
          console.error(err);
        })

    };

    var t = new TransactionSyncService();
    window.ts = t;
    return t;
  });
