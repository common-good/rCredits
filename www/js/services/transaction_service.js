app.service('TransactionService', function($q, UserService, RequestParameterBuilder, $http, $httpParamSerializer, SQLiteService, SqlQuery) {

  var self;

  var TRANSACTION_OK = "1";

  var TransactionService = function() {
    self = this;
    this.lastTransaction = null;
  };

  TransactionService.prototype.makeRequest_ = function(params, accountInfo) {
    var urlConf = new UrlConfigurator();
    return $http({
      method: 'POST',
      url: urlConf.getServerUrl(accountInfo.getMemberId()),
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
    return transaction;
  };

  TransactionService.prototype.charge_ = function(amount, description) {
    var sellerAccountInfo = UserService.currentUser().accountInfo,
      customerAccountInfo = UserService.currentCustomer().accountInfo,
      params = new RequestParameterBuilder()
        .setOperationId('charge')
        .setSecurityCode(customerAccountInfo.securityCode)
        .setAgent(sellerAccountInfo.accountId)
        .setMember(customerAccountInfo.accountId)
        .setField('amount', amount)
        .setField('description', description)
        .setField('created', moment().unix())
        .setField('force', 0)
        .setField('goods', 1)
        .setField('photoid', 0)
        .getParams();

    return this.makeRequest_(params, sellerAccountInfo).then(function(res) {
      return res.data;
    });
  };

  TransactionService.prototype.charge = function(amount, description) {
    return this.charge_(amount, description)
      .then(function(transactionResult) {
        console.log("Transcation Result: ", transactionResult);

        if (transactionResult.ok === TRANSACTION_OK) {
          var transaction = self.parseTransaction_(transactionResult);
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

          return transaction;
        }

        throw transactionResult;
      });
  };

  TransactionService.prototype.refund = function(amount, description) {
    return this.charge(amount * -1, description);
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

    return this.makeRequest_(params, sellerAccountInfo)
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
      Transaction.Status.DONE,
      transaction.created,
      seller.getId(),
      customer.getId(),
      transaction.amount,
      transaction.goods,
      JSON.stringify({
        customerId: customer.getId(),
        amount: transaction.amount,
        created: transaction.created,
        sellerId: seller.getId()
      }),
      transaction.description
    ]);

    return SQLiteService.executeQuery(sqlQuery);
  };

  return new TransactionService();
});
