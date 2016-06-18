/* global rCreditsConfig, Transaction, _, app */

app.service('TransactionService',
	function ($q, UserService, RequestParameterBuilder, $http, $httpParamSerializer, SQLiteService,
		SqlQuery, NetworkService, MemberSqlService, NotificationService, $ionicLoading, TransactionSql, $rootScope) {
		var self;
		var TRANSACTION_OK = "1";
		var TransactionService = function () {
			self = this;
			this.lastTransaction = null;
		};
		TransactionService.prototype.makeRequest_ = function (params, account) {
			var urlConf = new UrlConfigurator();
			return $http({
				method: 'POST',
				url: urlConf.getServerUrl(account),
				headers: {
					'Content-Type': 'application/x-www-form-urlencoded'
				},
				data: $httpParamSerializer(params)
			});
		};
		TransactionService.prototype.parseTransaction_ = function (transactionInfo) {
			var transaction = new Transaction();
			_.keys(transaction).forEach(function (k) {
				if (transactionInfo.hasOwnProperty(k)) {
					transaction[k] = transactionInfo[k];
				}
			});
			transaction.status = transactionInfo.transaction_status || Transaction.Status.DONE;
			return transaction;
		};
		TransactionService.prototype.makeTransactionRequest = function (amount, description, goods, force) {
			var sellerAccountInfo = UserService.currentUser().accountInfo,
				customerAccountInfo = UserService.currentCustomer().accountInfo;
			if (_.isUndefined(goods) || _.isNull(goods)) {
				goods = 1;
			}
			if (_.isUndefined(force) || _.isNull(force)) {
				force = 0;
			}
			if (NetworkService.isOnline()) {
				var params = new RequestParameterBuilder()
					.setOperationId('charge')
					.setSecurityCode(customerAccountInfo.securityCode)
					.setAgent(sellerAccountInfo.accountId)
					.setMember(customerAccountInfo.accountId)
					.setField('amount', amount.toString())
					.setField('description', description)
					.setField('created', moment().unix())
					.setField('force', force)
					.setField('goods', goods)
					.setField('photoid', 0)
					.getParams();
				return this.makeRequest_(params, sellerAccountInfo).then(function (res) {
					return res.data;
				});
			} else {
				// Offline
				return this.doOfflineTransaction(amount, description, goods, force).then(function (result) {
					console.log(result.message);
					self.warnOfflineTransactions();
					return result;
				});
			}
		};
		TransactionService.prototype.charge = function (amount, description, goods, force) {
			return this.makeTransactionRequest(amount, description, goods, force)
				.then(function (transactionResult) {
					if (transactionResult.ok === TRANSACTION_OK) {
						var transaction = self.parseTransaction_(transactionResult);
						transaction.configureType(amount);
						var customer = UserService.currentCustomer();
						customer.balance = transactionResult.balance;
						customer.rewards = transactionResult.rewards;
						transaction.amount = amount;
						transaction.description = description;
						transaction.goods = 1;
						customer.setLastTx(transaction);
						customer.saveInSQLite().then(function () {
							self.saveTransaction(transaction);
						});
						self.lastTransaction = transaction;
						return transaction;
					}
					self.lastTransaction = transactionResult;
					throw transactionResult;
				})
				.finally(function () {
					$rootScope.$emit("TransactionDone");
				});
		};
		TransactionService.prototype.refund = function (amount, description) {
			return this.charge(((amount * -1).toFixed(2)).toString(), description);
		};
		TransactionService.prototype.exchange = function (amount, currency, paymentMethod) {
			var exchangeType = 'USD in';
			var amountToSend = amount;
			if (!currency.isUSD()) {
				exchangeType = 'USD out';
			} else {
				amountToSend = amount * (-1);
			}
			var description = exchangeType + '(' + paymentMethod.getId() + ')';
			return this.charge(amountToSend, description, 0);
		};
		TransactionService.prototype.undoTransaction = function (transaction) {
			return this.charge(transaction.amount, transaction.description, transaction.goods, -1);
		};
		TransactionService.prototype.saveTransaction = function (transaction) {
			//"me TEXT," + // company (or device-owner) account code (qid)
			//"txid INTEGER DEFAULT 0," + // transaction id (xid) on the server (for offline backup only -- not used by the app) / temporary storage of customer cardCode pending tx upload
			//"status INTEGER," + // see A.TX_... constants
			//"created INTEGER," + // transaction creation datetime (unixtime)
			//"agent TEXT," + // qid for company and agent (if any) using the device
			//"member TEXT," + // customer account code (qid)
			//"amount REAL," +
			//"goods INTEGER," + // <transaction is for real goods and services>
			//"proof TEXT," + // hash of cardCode, amount, created, and me (as proof of agreement)
			//"description TEXT);" // always "reverses..", if this tx undoes a previous one (previous by date)
			var seller = UserService.currentUser(),
				customer = UserService.currentCustomer();
			var sqlQuery = new SqlQuery();
			sqlQuery.setQueryString('INSERT INTO txs (me, txid, status, created, agent, member, amount, goods, proof, description) VALUES (?,?,?,?,?,?,?,?,?,?)');
			sqlQuery.setQueryData([
				seller.getId(),
				transaction.getId(),
				transaction.status,
				transaction.created,
				seller.getId(),
				customer.getId(),
				transaction.amount,
				transaction.goods,
				JSON.stringify({
					account: JSON.stringify(customer.accountInfo),
					sc: customer.accountInfo.securityCode,
					customerId: customer.getId(),
					amount: transaction.amount,
					created: transaction.created,
					sellerId: seller.getId()
				}),
				transaction.description
			]);
			return SQLiteService.executeQuery(sqlQuery);
		};
		TransactionService.prototype.doOfflineTransaction = function (amount, description, goods, force) {
			var customer = UserService.currentCustomer();
			var q = $q.defer();
			var message;
			if (force === -1) {
				message = "The transaction has been canceled";
				return q.reject();
			} else {
				message = 'You charged ' + customer.name + ' $' + amount.toFixed(2).toString() + ' for goods and services';
			}
			var transactionResponseOk = {
				"ok": "1",
				"message": message,
				"txid": customer.getId(),
				"created": moment().unix(),
				"balance": (customer.setBalance(customer.getBalance() - ((amount).toFixed(2))).getBalance()).toString(),
				"rewards": (customer.getBalance() * 0.9).toFixed(2).toString(),
				"did": "",
				"undo": "",
				"transaction_status": Transaction.Status.OFFLINE,
				"description": description,
				"goods": goods
			};
			var transactionResponseError = {
				"ok": "0",
				"message": ""
			};
			if (amount > rCreditsConfig.transaction_max_amount_offline) {
				transactionResponseError.message = "Limit $" + rCreditsConfig.transaction_max_amount_offline + " exceeded";
				q.reject(transactionResponseError);
				return q.promise;
			}
			MemberSqlService.existMember(customer.getId())
				.then(function (customerDbInfo) {
					// do transaction
					return q.resolve(transactionResponseOk);
				})
				.catch(function (msg) {
					askConfirmation('cashier_permission', '', 'ok', 'cancel')
						.then(function () {
							// do transaction
							return q.resolve(transactionResponseOk);
						})
						.catch(function () {
							// reject transaction
							transactionResponseError.message = "Not Authorized";
							return q.reject(transactionResponseError);
						});
				});
			console.log(transactionResponseOk.message);
			return q.promise;
		};
		var askConfirmation = function (title, subTitle, okText, cancelText) {
			$ionicLoading.hide();
			return NotificationService.showConfirm({
				title: title,
				subTitle: subTitle,
				okText: okText,
				cancelText: cancelText
			})
				.then(function (confirmed) {
					$ionicLoading.show();
					console.log(title, subTitle, okText, cancelText);
					if (confirmed) {
						return true;
					} else {
						throw false;
					}
				});
		};
		TransactionService.prototype.warnOfflineTransactions = function () {
			TransactionSql.exist24HsTransactions().then(function (exists) {
				if (exists) {
					NotificationService.showAlert('offline_old_transactions');
					//					askConfirmation('offline_old_transactions','','OK','Cancel');
				}
			});
		};
		return new TransactionService();
	});