(function(window) {

  var Transaction = function() {
    this.txid = 0;
    this.created = null;
    this.did = null;
    this.undo = null;
    this.message = null;
    this.amount = null;
    this.description = null;
    this.goods = null;
  };

  Transaction.Status = {
    CANCEL: -1, // transaction has been canceled offline, but may exist on server
    PENDING: 0, // transaction is being sent to server
    OFFLINE: 1, // connection failed, offline transaction is waiting to be uploaded
    DONE: 2     // completed transaction is on server
  };

  Transaction.prototype.getId = function() {
    return this.txid;
  };

  window.Transaction = Transaction;

})(window);
