app.service('TransactionService',
  function($q, UserService, RequestParameterBuilder, $http, $httpParamSerializer, SQLiteService,
           SqlQuery, NetworkService, MemberSqlService, NotificationService, $ionicLoading, TransactionSql, $rootScope) {

    var self;

    var TRANSACTION_OK = "1";

    var TransactionService = function() {
      self = this;
      this.lastTransaction = null;
    };

    TransactionService.prototype.makeRequest_ = function(params, memberId) {
      var urlConf = new UrlConfigurator();
      return $http({
        method: 'POST',
        url: urlConf.getServerUrl(memberId),
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded'
        },
        data: $httpParamSerializer(params)
      });
    };

    TransactionService.prototype.parseTransaction_ = function(transactionInfo) {
      var transaction = new Transaction();
      _.keys(transaction).forEach(function(k) {
        if (transactionInfo.hasOwnProperty(k)) {
          transaction[k] = transactionInfo[k];
        }
      });

      transaction.status = transactionInfo.transaction_status || Transaction.Status.DONE;
      return transaction;
    };

    TransactionService.prototype.makeTransactionRequest = function(amount, description, goods) {
      var sellerAccountInfo = UserService.currentUser().accountInfo,
        customerAccountInfo = UserService.currentCustomer().accountInfo;

      if (_.isUndefined(goods) || _.isNull(goods)) {
        goods = 1;
      }

      if (NetworkService.isOnline()) {
        var params = new RequestParameterBuilder()
          .setOperationId('charge')
          .setSecurityCode(customerAccountInfo.securityCode)
          .setAgent(sellerAccountInfo.accountId)
          .setMember(customerAccountInfo.accountId)
          .setField('amount', amount)
          .setField('description', description)
          .setField('created', moment().unix())
          .setField('force', 0)
          .setField('goods', goods)
          .setField('photoid', 0)
          .getParams();

        return this.makeRequest_(params, sellerAccountInfo.getMemberId()).then(function(res) {
          return res.data;
        });
      } else {
        // Offline
        return this.doOfflineTransaction(amount, description, goods).then(function(result) {
          self.warnOfflineTransactions();
          return result;
        });
      }
    };

    TransactionService.prototype.charge = function(amount, description, goods) {
      return this.makeTransactionRequest(amount, description, goods)
        .then(function(transactionResult) {
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
            customer.saveInSQLite().then(function() {
              self.saveTransaction(transaction);
            });

            self.lastTransaction = transaction;
            return transaction;
          }

          throw transactionResult;
        })
        .finally(function() {
          $rootScope.$emit("TransactionDone");
        });
    };

    TransactionService.prototype.refund = function(amount, description) {
      return this.charge(amount * -1, description);
    };

    TransactionService.prototype.exchange = function(amount, currency, paymentMethod) {
      var exchangeType = 'USD in';
      if (!currency.isUSD()) {
        exchangeType = 'USD out';
      }
      var description = exchangeType + '(' + paymentMethod.getId() + ')';
      return this.charge(amount, description, 0);
    };

    TransactionService.prototype.undoTransaction = function(transaction) {
      var sellerAccountInfo = UserService.currentUser().accountInfo,
        customerAccountInfo = UserService.currentCustomer().accountInfo,
        params = new RequestParameterBuilder()
          .setOperationId('charge')
          .setAgent(sellerAccountInfo.accountId)
          .setMember(customerAccountInfo.accountId)
          .setField('amount', transaction.amount)
          .setField('description', transaction.description)
          .setField('created', transaction.created)
          .setField('force', -1)
          .setField('goods', transaction.goods)
          .getParams();

      return this.makeRequest_(params, sellerAccountInfo.getMemberId())
        .then(function(res) {
          return res.data;
        })
        .then(function(transactionResult) {
          if (transactionResult.ok === TRANSACTION_OK) {
            var customer = UserService.currentCustomer();
            customer.rewards = transactionResult.rewards;
            customer.balance = transactionResult.balance;
            return transactionResult;
          }
          throw transactionResult;
        });
    };

    TransactionService.prototype.saveTransaction = function(transaction) {
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

    TransactionService.prototype.doOfflineTransaction = function(amount, description, goods) {
      var customer = UserService.currentCustomer();
      var q = $q.defer();

      var transactionResponseOk = {
        "ok": "1",
        "message": "",
        "txid": customer.getId(),
        "created": moment().unix(),
        "balance": customer.setBalance(customer.getBalance() - amount).getBalance(),
        "rewards": customer.getBalance() * 0.9,
        "did": "",
        "undo": "",
        "transaction_status": Transaction.Status.OFFLINE
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
        .then(function(customerDbInfo) {
          // do transaction
          return q.resolve(transactionResponseOk);
        })
        .catch(function(msg) {

          askConfirmation()
            .then(function() {
              // do transaction
              return q.resolve(transactionResponseOk);
            })
            .catch(function() {
              // reject transaction
              transactionResponseError.message = "Not Authorized";
              return q.reject(transactionResponseError);
            });

        });

      return q.promise;

    };

    var askConfirmation = function() {
      $ionicLoading.hide();
      return NotificationService.showConfirm({
          title: 'cashier_permission',
          subTitle: "",
          okText: "ok",
          cancelText: "cancel"
        })
        .then(function(confirmed) {
          $ionicLoading.show();
          if (confirmed) {
            return true;
          } else {
            throw false;
          }
        });
    };

    TransactionService.prototype.warnOfflineTransactions = function() {
      TransactionSql.exist24HsTransactions().then(function(exists) {
        if (exists) {
          NotificationService.showAlert('offline_old_transactions');
        }
      })
    };

    return new TransactionService();
  });
