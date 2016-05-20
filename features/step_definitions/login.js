///// Your step definitions /////

// use this.Given(), this.When() and this.Then() to declare step definitions
module.exports = function () {
	this.Given('The app has focus', function () {
		
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