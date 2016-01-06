app.service('TransactionService', function($q, UserService, RequestParameterBuilder, $http, $httpParamSerializer) {

  var self;

  var TRANSACTION_OK = "1";

  var TransactionService = function() {
    self = this;
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

  TransactionService.prototype.charge_ = function(amount, description, customer) {
    var accountInfo = UserService.currentUser().accountInfo,
      params = new RequestParameterBuilder()
        .setOperationId('charge')
        .setSecurityCode(customer.accountInfo.securityCode)
        .setAgent(accountInfo.accountId)
        .setMember(customer.accountInfo.accountId)
        .setField('amount', amount)
        .setField('description', description)
        .setField('created', moment().unix())
        .setField('force', 0)
        .setField('goods', 1)
        .setField('photoid', 0)
        .getParams();

    return this.makeRequest_(params, accountInfo).then(function(res) {
      return res.data;
    });
  };

  TransactionService.prototype.charge = function(amount, description, customer) {
    return this.charge_(amount, description, customer)
      .then(function(transactionResult) {
        console.log("Transcation Result: ", transactionResult);

        if (transactionResult.ok === TRANSACTION_OK) {
          var transaction = self.parseTransaction_(transactionResult);

          return transaction;
        }

        throw transactionResult;
      });


    // Simulates a charge. Resolves the promise if SUCCEED is true, rejects if false.
    //var SUCCEED = true;
    //return $q(function(resolve, reject) {
    //  setTimeout(function() {
    //    if (SUCCEED) {
    //      resolve({
    //        ok: 1,
    //        message: "",
    //        balance: Math.round((110.23 - amount) * 100) / 100,
    //        rewards: Math.round((8.72 + amount / 10) * 100) / 100,
    //        txid: 123,
    //        created: 1450485351,
    //        did: amount + " transferred from Phillip Blivers to Tasty Soaps, Inc.",
    //        undo: "(Undo message)"
    //      });
    //    } else {
    //      reject({
    //        ok: 0,
    //        message: "(Failure message)",
    //        balance: 110.23,
    //        rewards: 8.72
    //      });
    //    }
    //  }, 1000);
    //});
  };

  TransactionService.prototype.refund = function(amount) {
    return self.charge(amount * -1);
  }

  return new TransactionService();
});
