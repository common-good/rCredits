app.service('TransactionService', function($q) {

  var TransactionService = function () {
    self = this;
  };

  TransactionService.prototype.charge = function(amount) {
    // Simulates a charge. Resolves the promise if SUCCEED is true, rejects if false.
    var SUCCEED = true;

    return $q(function(resolve, reject) {
      setTimeout(function() {
        if (SUCCEED) {
          resolve({
            ok: 1,
            message: "",
            balance: Math.round((110.23 - amount) * 100) / 100,
            rewards: Math.round((8.72 + amount / 10) * 100) / 100,
            txid: 123,
            created: 1450485351,
            did: amount + " transferred from Phillip Blivers to Tasty Soaps, Inc.",
            undo: "(Undo message)"
          });
        } else {
          reject({
            ok: 0,
            message: "(Failure message)",
            balance: 110.23,
            rewards: 8.72
          });
        }
      }, 1000);
    });
  };

  TransactionService.prototype.refund = function(amount) {
    return self.charge(amount * -1);
  }

  return new TransactionService();
});
