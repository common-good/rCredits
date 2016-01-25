describe('User Service', function() {

  'use strict';

  beforeEach(module('rcredits'));
  beforeEach(function() {
    module(function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var userService, rootScope, httpBackend, localStorageService, preferenceService;


  beforeEach(inject(function(UserService, $rootScope, _localStorageService_, _PreferenceService_, $httpBackend) {
    userService = UserService;
    rootScope = $rootScope;
    localStorageService = _localStorageService_;
    preferenceService = _PreferenceService_;
    httpBackend = $httpBackend;

    $httpBackend.whenGET(/templates\/*/).respond(function(method, url, data, headers) {
      return [200, '<div></div>'];
    });

    $httpBackend.whenGET(/js\/languages\/definitions\//).respond(function(method, url, data, headers) {
      return [200, {}];
    });

  }));


  describe('Find preferences', function() {


    it('Should find a Preference by Id', function() {

    });


  });


});

