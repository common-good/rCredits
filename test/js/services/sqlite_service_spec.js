describe('Transaction Service', function() {

  'use strict';

  jasmine.DEFAULT_TIMEOUT_INTERVAL  = 5000;

  beforeEach(module('rcredits'));
  beforeEach(function() {
    module(function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var rootScope, httpBackend, SQLiteService, SqlQuery;
  var request;


  beforeEach(inject(function($rootScope, $httpBackend, _SQLiteService_, _SqlQuery_) {
    httpBackend = $httpBackend;
    rootScope = $rootScope;
    SQLiteService = _SQLiteService_;
    SqlQuery = _SqlQuery_;

    $httpBackend.whenGET(/templates\/*/).respond(function(method, url, data, headers) {
      return [200, '<div></div>'];
    });

    $httpBackend.whenGET(/js\/languages\/definitions\//).respond(function(method, url, data, headers) {
      return [200, {}];
    });

  }));

  // Logs in the Seller and the Customer
  beforeEach(function(done) {
    var deleteQuery = new SqlQuery();
    deleteQuery.setQueryString("Delete from members");
    var p = SQLiteService.executeQuery(deleteQuery);

    p.then(function() {
        done();
      })
      .catch(function() {
        done();
      });
    rootScope.$apply();
  });


  describe('Managing DB', function() {

    it('Should create Members table', function(done) {

      rootScope.$apply();
      done()
    });

  });

});



