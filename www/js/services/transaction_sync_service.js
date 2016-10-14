/* global _, app */
app.service('TransactionSyncService',
	function ($q, TransactionService, RequestParameterBuilder, SQLiteService, SqlQuery, NetworkService, TransactionSql, $timeout, $rootScope, NotificationService, UserService) {
		'use strict';
		var self;
		var TransactionSyncService = function () {
			self = this;
			this.exludedTxs = [];
			$rootScope.$on('onOnline', function () {
				self.syncOfflineTransactions();
			});
		};
		var send = function (sqlTransaction) {
			console.log("TRANSACTION TO SEND: ", sqlTransaction);
			console.log(sqlTransaction.data);
			try {
				var account = _.extendOwn(new AccountInfo(), JSON.parse(sqlTransaction.account).account);
				return TransactionService.makeRequest_(JSON.parse(sqlTransaction.data), account).then(function (res) {
					console.log(res);
					return res.data;
				});
			} catch (err) {
				console.log(err);
				return err;
			}
		};
		TransactionSyncService.prototype.syncOfflineTransactions = function () {
			if (NetworkService.isOffline()) {
				console.log('Thinks it\'s offline');
				return;
			}
			var sqlTransaction;
			TransactionSql.getOfflineTransaction(self.exludedTxs)
				.then(function (sqlTransac) {
					console.log(sqlTransac, self.exludedTxs);
					sqlTransaction = sqlTransac;
					// send to server
					return send(sqlTransac);
				})
				.then(function (response) {
					if (response.ok === 0) { // Error;
						console.error("Error syncing transaction: ", response);
						throw response;
					}
					return TransactionSql.setTransactionSynced(sqlTransaction);
				})
				.then(function () {
					console.log(self);
					$timeout(self.syncOfflineTransactions.bind(self), 500);
				})
				.catch(function (err) {
					if (sqlTransaction) {
						self.exludedTxs.push(sqlTransaction.created);
						$timeout(self.syncOfflineTransactions.bind(self), 500);
						console.log(self, err.message);
					} else {
						self.exludedTxs = [];
					}
				});
		};
		var t = new TransactionSyncService();
		window.ts = t;
		return t;
	}
);
