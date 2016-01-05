(function(window) {

  var Transaction = function() {
    this.txid = null;
    this.created = null;
    this.did = null;
    this.undo = null;
  };


  window.Transaction = Transaction;

})(window);
