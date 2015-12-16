describe ('User Service', function() {

  'use strict';

  beforeEach (module ('rcredits'));

  var userService, rootScope;
  var SCAN_RESULT = {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};

  beforeEach (inject (function(UserService, $rootScope) {
    userService = UserService;
    rootScope = $rootScope;
  }));


  it ('Should send correct member and code values based on input URL', function() {

  });


});

