describe ('Seller Model', function() {

  'use strict';

  beforeEach (module ('rcredits'));
  beforeEach (function() {
    module (function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var Seller, localStorageService, seller;
  var SELLER_TEST_NAME = 'Jhon';

  beforeEach (inject (function(_Seller_, _localStorageService_) {
    Seller = _Seller_;
    localStorageService = _localStorageService_;
  }));

  beforeEach (function() {
    localStorageService.set('deviceID', 'device123');
    seller = new Seller (SELLER_TEST_NAME);
  });

  it ('Should set Name', function() {
    expect (seller.name).toBe(SELLER_TEST_NAME);
  });

  it ('Should have localStorage device', function() {
    expect (seller.device).toBe('device123');
  });

  it ('Should not have Device ID', function() {
    localStorageService.remove('deviceID');
    seller = new Seller (SELLER_TEST_NAME);
    expect (seller.device).toBe('');
  });

  it ('Should have custom Device ID', function() {
    localStorageService.remove('deviceID');
    seller = new Seller (SELLER_TEST_NAME);
    seller.setDeviceId('device_xyz');
    expect (seller.device).toBe('device_xyz');
  });

  it ('Not valid device ID', function() {
    expect (seller.isValidDeviceId(undefined)).toBe(false);
    expect (seller.isValidDeviceId(null)).toBe(false);
    expect (seller.isValidDeviceId('')).toBe(false);
  });

  it ('Throw exception when setting invalid Device ID', function() {
    expect (function(){
      seller.setDeviceId(null)
    }).toThrow();
  });

});

