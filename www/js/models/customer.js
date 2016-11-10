/* global Class */
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
				console.log(balance);
				return this;
			},
			getPlace: function () {
				return this.place;
			},
			getBalance: function () {
				return this.balance;
			},
			getRewards: function () {
				console.log(this.balance);
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
			var proof = JSON.parse(customerJson.data);
			customer.setBalance(customerJson.balance);
			customer.setRewards((customerJson.rewards));
			customer.setLastTx(customerJson.lastTx);
			customer.place = customerJson.place;
			customer.company = customerJson.company;
			customer.accountInfo.accountId = customerJson.qid;
			customer.accountInfo.securityCode = proof.sc;
			if (customerJson.photo) {
//				console.log(customerJson.photo);
				customer.photo = customerJson.photo;
			} else {
				customer.photo = '/img/New-rCredits-Customer.png';
			}
			return customer;
		};
		window.Customer = Customer;
		return Customer;
	});
})(window, app);
