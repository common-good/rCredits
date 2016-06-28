function World() {
	this.fire = function (method, callback) {
		this.fire(method);
	};
};
module.exports = function() {
  this.World = World;
};