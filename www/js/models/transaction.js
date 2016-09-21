(function (window) {
	var Transaction = function () {
		this.txid = 0;
		this.created = null;
		this.did = null;
		this.undo = null;
		this.message = null;
		this.amount = null;
		this.description = null;
		this.goods = null;
		this.status = null;
	};
	Transaction.Types = {
		CHARGE: 1,
		REFUND: -1
	};
	Transaction.Status = {
		CANCEL: -1, // transaction has been canceled offline, but may exist on server
		PENDING: 0, // transaction is being sent to server
		OFFLINE: 1, // connection failed, offline transaction is waiting to be uploaded
		DONE: 2     // completed transaction is on server
	};
	Transaction.prototype.getId = function () {
		return this.txid;
	};
	Transaction.prototype.configureType = function (amout) {
		if (parseFloat(amout) < 0) {
			this.type = Transaction.Types.REFUND;
		} else if (parseFloat(amout) >= 0) {
			this.type = Transaction.Types.CHARGE;
		}
	};
	Transaction.prototype.isRefund = function () {
		return this.type === Transaction.Types.REFUND;
	};
	window.Transaction = Transaction;
})(window);
