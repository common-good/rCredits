module.exports = function () {
	this.Given('the charge button has been pressed', function (callback) {
		this.fire()
	});
	this.When(/^I increment the variable by (\d+)$/, function (number) {
		this.openScanner();
	});
	this.Then(/^the variable should contain (\d+)$/, function (number) {
		if (this.variable !== parseInt(number))
			throw new Error('Variable should contain ' + number +
				' but it contains ' + this.variable + '.');
	});
};