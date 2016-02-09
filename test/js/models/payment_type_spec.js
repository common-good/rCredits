describe('Payment Type', function() {

  'use strict';

  beforeEach(module('rcredits'));
  beforeEach(function() {
    module(function($exceptionHandlerProvider) {
      $exceptionHandlerProvider.mode('log');
    });
  });

  var PaymentType;

  beforeEach(inject(function(_PaymentType_) {
    PaymentType = _PaymentType_;
  }));


  describe('Should apply fee', function() {
    var paymentType;

    beforeEach(function() {
      paymentType = PaymentType.parsePaymentType(window.paymentTypesDefinitions[1]);
    });

    it('Apply fee', function() {
      var amount = 123;
      var fee = paymentType.getFee();
      spyOn(fee, 'apply');
      paymentType.applyFeeTo(amount);
      expect(fee.apply).toHaveBeenCalledWith(amount);
    });

  });

});
