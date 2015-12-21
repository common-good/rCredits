(function(app) {
  'use strict';

  app.service('RequestParameterBuilder', function(localStorageService) {

    var RequestParameterBuilder = function(qrCodeParsedInfo) {
      this.result = {
        op: '',
        device: this.getDeviceId() || '',
        code: qrCodeParsedInfo.securityCode,
        member: qrCodeParsedInfo.accountId,
        version: rCreditsConfig.build
      };
    };

    RequestParameterBuilder.prototype.setOperationId = function(op) {
      this.result.op = op;
      return this;
    };

    RequestParameterBuilder.prototype.getDeviceId = function() {
      return localStorageService.get('deviceID');
    };

    RequestParameterBuilder.prototype.getParams = function() {
      return this.result;
    };

    return RequestParameterBuilder;
  });

}) (app);
