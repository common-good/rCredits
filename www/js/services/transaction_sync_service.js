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
			var sellerAccountInfo = sqlTransaction.accountInfo,
				customerAccountInfo = sqlTransaction.accountInfo;
			if (_.isUndefined(sqlTransaction.goods) || _.isNull(sqlTransaction.goods)) {
				sqlTransaction.goods = 1;
			}
			if (_.isUndefined(sqlTransaction.force) || _.isNull(sqlTransaction.force)) {
				sqlTransaction.force = 0;
			}
			try {
				var params = new RequestParameterBuilder()
					.setOperationId('charge')
					.setSecurityCode(customerAccountInfo.securityCode)
					.setAgent(sellerAccountInfo.accountId)
					.setMember(customerAccountInfo.accountId)
					.setField('amount', sqlTransaction.amount.toString())
					.setField('description', sqlTransaction.description)
					.setField('created', sqlTransaction.moment().unix())
					.setField('force', sqlTransaction.force)
					.setField('goods', sqlTransaction.goods)
					.setField('photoid', 0)
					.getParams();
				var proof = Sha256.hash((params.agent + params.amount + params.member + customerAccountInfo.unencryptedCode + params.created).toString());
				params['proof'] = proof;
				console.log(proof, customerAccountInfo.unencryptedCode);
				var account = _.extendOwn(new AccountInfo(), JSON.parse(sqlTransaction.proof.account));
				return TransactionService.makeRequest_(params, account).then(function (res) {
					console.log(res);
					return res.data;
				});
			} catch (err) {
				console.log(err);
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
				//.then(self.syncOfflineTransactions.bind(self))
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
					// err no transactions || error ocurred
//					NotificationService.showAlert({
//						title: "error",
//						template: "There has been an Error: " + err.message
//					});
				});
		};
		var t = new TransactionSyncService();
		window.ts = t;
		return t;
	}
);
