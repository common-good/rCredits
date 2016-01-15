app.service('TransactionService', function($q, UserService, RequestParameterBuilder, $http, $httpParamSerializer) {

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

  return new TransactionService();
});
