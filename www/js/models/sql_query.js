/* global app, _ */
(function (window, app) {
	app.service('SqlQuery', function () {
		var SqlQuery = function () {
			this.queryString = '';
			this.queryData = [];
		};
		SqlQuery.prototype.setQueryString = function (tbl,queryStr) {
			this.queryString = (queryStr ? queryStr : this.queryString);
			this.tbl = tbl;
		};
		SqlQuery.prototype.setQueryData = function (data) {
			var sqlData = data;
			if (!_.isArray(data)) {
				sqlData = [data];
			}
			this.queryData = sqlData;
		};
		SqlQuery.prototype.getQueryString = function () {
			return this.queryString;
		};
		SqlQuery.prototype.getQueryData = function () {
			return this.queryData;
		};
		return SqlQuery;
	});
})(window, app);
