(function(window, app) {

  app.service('SqlQuery', function() {

    var SqlQuery = function() {
      this.queryString = '';
      this.queryData = [];
    };

    SqlQuery.prototype.setQueryString = function(queryStr) {
      this.queryString = queryStr;
    };

    SqlQuery.prototype.setQueryData = function(data) {
      this.queryData = data;
    };

    SqlQuery.prototype.getQueryString = function() {
      return this.queryString;
    };

    SqlQuery.prototype.getQueryData = function() {
      return this.queryData;
    };

    return SqlQuery;
  });
})(window, app);
