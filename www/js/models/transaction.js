(function(window) {

  var Transaction = function() {
    this.txid = null;
    this.created = null;
    this.did = null;
    this.undo = null;
    this.message = null;
  };

  Transaction.prototype.getId = function() {
    return this.txid;
  };


  window.Transaction = Transaction;

})(window);
