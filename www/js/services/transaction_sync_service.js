app.service('TransactionSyncService',
  function($q, TransactionService, RequestParameterBuilder, SQLiteService, SqlQuery, NetworkService, TransactionSql, $timeout, $rootScope) {

    'use strict';
    var self;
    var TransactionSyncService = function() {
      self = this;
      this.exludedTxs = [];

      $rootScope.$on('onOnline', function() {
        self.syncOfflineTransactions();
      });
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

      return TransactionService.makeRequest_(params, sqlTransaction.agent).then(function(res) {
        return res.data;
      });
    };

    TransactionSyncService.prototype.syncOfflineTransactions = function() {

      if (NetworkService.isOffline()) {
        return;
      }

      var sqlTransaction;

      TransactionSql.getOfflineTransaction(self.exludedTxs)
        .then(function(sqlTransac) {
          sqlTransaction = sqlTransac;
          // send to server
          return send(sqlTransac);
        })
        .then(function(response) {
          if (response.ok == 0) { // Error;
            console.error("Error syncing transaction: ", response);
            throw response;
          }
          return TransactionSql.setTransactionSynced(sqlTransaction);
        })
        //.then(self.syncOfflineTransactions.bind(self))
        .then(function() {
          $timeout(self.syncOfflineTransactions.bind(self), 1000);
        })
        .catch(function(err) {
          if (sqlTransaction) {
            self.exludedTxs.push(sqlTransaction.created);
            $timeout(self.syncOfflineTransactions.bind(self), 1000);
          } else {
            self.exludedTxs = [];
          }

          // err no transactions || error ocurred
          console.error(err);
        })

    };

    var t = new TransactionSyncService();
    window.ts = t;
    return t;
  }
);
