/* global rCreditsConfig, Transaction, _, app, Sha256, parseFloat, moment */
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
			console.log(params, account);
//			params.amount=parseFloat(params.amount.toFixed(2).toString());
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
			if (UserService.currentUser().accountInfo) {
				var sellerAccountInfo = UserService.currentUser().accountInfo,
					customerAccountInfo = UserService.currentCustomer();
				if (_.isUndefined(goods) || _.isNull(goods)) {
					goods = 1;
				}
				if (_.isUndefined(force) || _.isNull(force)) {
					force = 0;
				}
				try {
					var params = new RequestParameterBuilder()
						.setOperationId('charge')
						.setSecurityCode(customerAccountInfo.accountInfo.securityCode)
						.setAgent(sellerAccountInfo.accountId)
						.setMember(customerAccountInfo.accountInfo.accountId)
						.setField('amount', parseFloat(parseFloat(amount).toFixed(2)))
						.setField('description', description)
						.setField('created', moment().unix())
						.setField('force', force)
						.setField('goods', goods)
						.setField('photoid', 0)
						.getParams();
					var proof = Sha256.hash((params.agent + params.amount + params.member + customerAccountInfo.accountInfo.unencryptedCode + params.created).toString());
					params['proof'] = proof;
					params['seller'] = sellerAccountInfo;
				} catch (e) {
					console.log('catch');
					NotificationService.showAlert({title: 'error', template: e});
				}
				if (NetworkService.isOnline()) {
					console.log(params, customerAccountInfo, sellerAccountInfo);
					return this.makeRequest_(params, sellerAccountInfo).then(function (res) {
						console.log(res);
						return res;
					});
				} else {
					// Offline
					console.log(params.proof, customerAccountInfo, sellerAccountInfo);
					return this.doOfflineTransaction(params, customerAccountInfo).then(function (result) {
						console.log(result);
						self.warnOfflineTransactions();
						return result;
					});
				}
			} else {
				NotificationService.showAlert({title: 'error', template: 'We were unable to find your account'});
			}
		};
		TransactionService.prototype.charge = function (amount, description, goods, force) {
			return this.makeTransactionRequest(amount, description, goods, force)
				.then(function (transactionResult) {
					console.log(transactionResult.data.ok, transactionResult);
					if (transactionResult.data.ok === TRANSACTION_OK) {
						var transaction = self.parseTransaction_(transactionResult);
						transaction.configureType(amount);
						var customer = UserService.currentCustomer();
						customer.balance = transactionResult.data.balance;
						customer.rewards = transactionResult.data.rewards;
						transaction.amount = amount;
						transaction.description = description;
						transaction.goods = 1;
						transaction.data = transactionResult.data;
						customer.setLastTx(transaction);
						console.log(customer.balance);
						customer.saveInSQLite().then(function () {
							self.saveTransaction(transaction);
						});
						self.lastTransaction = transaction;
						console.log(transaction);
						return transaction;
					} else {
						for (var v in transactionResult) {
						}
						console.log(transactionResult.data.ok, transactionResult);
					}
					self.lastTransaction = transactionResult;
					return transactionResult;
				})
				.finally(function () {
					$rootScope.$emit("TransactionDone");
				});
		};
		TransactionService.prototype.refund = function (amount, description) {
			return this.charge(((parseFloat(amount * -1)).toFixed(2)), description);
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
			console.log(transaction);
			return this.charge(parseFloat(transaction.amount * -1), transaction.description, transaction.goods, 0);
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
			console.log(transaction);
			var seller = UserService.currentUser(),
				customer = UserService.currentCustomer();
			var sqlQuery = new SqlQuery();
			sqlQuery.setQueryString('INSERT INTO txs (me, txid, status, created, agent, member, amount, goods, data, account, description) VALUES (?,?,?,?,?,?,?,?,?,?,?)');
			sqlQuery.setQueryData([
				seller.getId(),
				transaction.getId(),
				transaction.status,
				transaction.created,
				seller.getId(),
				customer.getId(),
				transaction.amount,
				transaction.goods,
				JSON.stringify(transaction.data),
				JSON.stringify({
					account: customer.accountInfo,
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
		TransactionService.prototype.doOfflineTransaction = function (params, customer) {
			var q = $q.defer();
			var transactionResponseOk = {
				"message": "",
				"txid": customer.getId(),
				"created": moment().unix(),
				"balance": '',
				"rewards": '',
				"did": "",
				"undo": "",
				"transaction_status": Transaction.Status.OFFLINE,
				"data": params,
				"ok":"1"
			};
			transactionResponseOk.data.ok="1";
			var transactionResponseError = {
				"ok": "0",
				"message": "There has been an error"
			};
			console.log(customer);
			if (customer.isPersonal === false) {
				return q.reject();
			}
			if (params.amount > rCreditsConfig.transaction_max_amount_offline) {
				transactionResponseError.message = "Limit $" + rCreditsConfig.transaction_max_amount_offline + " exceeded";
				q.reject(transactionResponseError);
				return q.promise;
			}
			MemberSqlService.existMember(customer.getId())
				.then(function (customerDbInfo) {
					// do transaction
					transactionResponseOk.ok = '1';
					console.log(transactionResponseOk);
					return q.resolve(transactionResponseOk);
				})
				.catch(function (msg) {
					askConfirmation('cashier_permission', '', 'ok', 'cancel')
						.then(function () {
							// do transaction
							transactionResponseOk.ok = '1';
							console.log(transactionResponseOk);
							return q.resolve(transactionResponseOk);
						})
						.catch(function () {
							// reject transaction
							transactionResponseError.message = "Not Authorized";
							return q.reject(transactionResponseError);
						});
				});
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
				}
			});
		};
		return new TransactionService();
	});