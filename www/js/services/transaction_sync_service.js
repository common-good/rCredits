app.service('TransactionSyncService',
  function($q, TransactionService, RequestParameterBuilder, SQLiteService, SqlQuery, NetworkService, TransactionSql, $timeout) {

    'use strict';
    var self;
    var TransactionSyncService = function() {
      self = this;
    };

    var send = function(sqlTransaction) {
      var params = new RequestParameterBuilder()
        .setOperationId('charge')
        .setSecurityCode(sqlTransaction.proof.sc)
        .setAgent(sqlTransaction.agent)
        .setMember(sqlTransaction.member)
        .setField('amount', sqlTransaction.amount)
        .setField('description', sqlTransaction.description)
        .setField('created', sqlTransaction.created)
        .setField('force', 0)
        .setField('goods', sqlTransaction.goods)
        .setField('photoid', 0)
        .getParams();

      return TransactionService.makeRequest_(params, sqlTransaction.agent);
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
          return send(sqlTransac);
        })
        .then(function(res) {
          return res.data;
        })
        .then(function(response) {
          if (response.ok == 0) { // Error;
            console.log("Error syncing transaction: ", response);
            return;
          }
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
  }
);
