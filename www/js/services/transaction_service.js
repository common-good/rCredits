/* global rCreditsConfig, Transaction, _, app, Sha256 */
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
			console.log(params, account);
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
				if (NetworkService.isOnline()) {
					var sellerAccountInfo = UserService.currentUser().accountInfo,
						customerAccountInfo = UserService.currentCustomer().accountInfo;
					if (_.isUndefined(goods) || _.isNull(goods)) {
						goods = 1;
					}
					if (_.isUndefined(force) || _.isNull(force)) {
						force = 0;
					}
					try {
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
						var proof = Sha256.hash((params.agent + params.amount + params.member + customerAccountInfo.unencryptedCode + params.created).toString());
						params['proof'] = proof;
//						console.log(proof, customerAccountInfo.unencryptedCode);
						return this.makeRequest_(params, sellerAccountInfo).then(function (res) {
							console.log(res);
							return res.data;
						});
					} catch (e) {
						console.log('catch');
						console.log(e);
					}
				} else {
					// Offline
					console.log(amount, description, goods, force, this);
					return this.doOfflineTransaction(amount, description, goods, force).then(function (result) {
						console.log(result.message);
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
					if (transactionResult.ok === TRANSACTION_OK) {
						var transaction = self.parseTransaction_(transactionResult);
						console.log(transaction);
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
						console.log(transaction);
						return transaction;
					} else {
						for (var v in transactionResult) {
							console.log(transactionResult.ok, transactionResult[v]);
						}
						NotificationService.showAlert({title: 'error', template: transactionResult.message});
					}
					self.lastTransaction = transactionResult;
					throw transactionResult;
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
			var seller = UserService.currentUser(),
				customer = UserService.currentCustomer();
			var q = $q.defer();
			var message;
			console.log(amount);
			amount = parseFloat(parseFloat(amount).toFixed(2));
			console.log(customer, amount, description, goods, force, customer.unencryptedCode);
			if (force === 0) {
				message = "The transaction has been canceled";
				console.log(message);
			} else {
				message = 'You charged ' + customer.name + ' $' + amount + ' for goods and services';
			}
			var transactionResponseOk = {
				"ok": "1",
				"agent":seller,
				"member":customer,
				"message": message,
				"txid": customer.getId(),
				"created": moment().unix(),
				"balance": ((customer.setBalance(customer.getBalance() - amount)).getBalance()).toFixed(2),
				"rewards": (customer.getBalance() * 0.9).toFixed(2),
				"did": "",
				"undo": "",
				"unencryptedCode":customer.unencryptedCode,
				"transaction_status": Transaction.Status.OFFLINE,
				"description": description,
				"goods": goods
			};
			console.log(customer, amount, description, goods, force);
			var transactionResponseError = {
				"ok": "0",
				"message": "There has been an error"
			};
			if (customer.isPersonal === false) {
				console.log(customer);
				return q.reject();
			}
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