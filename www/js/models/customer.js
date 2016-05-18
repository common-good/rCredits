(function (window, app) {
	app.service('Customer', function (User) {
		var Customer = Class.create(User, {
			balance: 0,
			rewards: null,
			lastTx: null,
			unregistered: false, // Customer logs in Offline mode for first time and not have any data
			setRewards: function (rewards) {
				this.rewards = parseFloat(rewards);
			},
			setBalance: function (balance) {
				this.balance = balance;
				return this;
			},
			getPlace: function () {
				return this.place;
			},
			getBalance: function () {
				return this.balance;
			},
			getRewards: function () {
				return this.rewards;
			},
			setLastTx: function (transaction) {
				this.lastTx = transaction;
			},
			getLastTx: function () {
				return this.lastTx.getId();
			}
		});
		Customer.parseFromDb = function (customerJson) {
			var customer = new Customer(customerJson.name);
			var proof = JSON.parse(customerJson.proof);
			customer.setBalance(customerJson.balance);
			customer.setRewards((customerJson.rewards));
			customer.setLastTx(customerJson.lastTx);
			customer.place = customerJson.place;
			customer.company = customerJson.company;
			customer.accountInfo.accountId = customerJson.qid;
			customer.accountInfo.securityCode = proof.sc;
			customer.photo = customerJson.photo;
			return customer;
		};
		window.Customer = Customer;
		return Customer;
	});
})(window, app);
