(function(app) {
  'use strict';

  app.service('RequestParameterBuilder', function(localStorageService) {

    var RequestParameterBuilder = function() {
      this.result = {
        op: '',
        device: this.getDeviceId() || '',
        agent: null,
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

    RequestParameterBuilder.prototype.setMember = function(member) {
      this.result.member = member;
      return this;
    };

    RequestParameterBuilder.prototype.setSecurityCode = function(securityCode) {
      this.result.code = securityCode;
      return this;
    };

    RequestParameterBuilder.prototype.setAgent = function(agent) {
      this.result.agent = agent;
      return this;
    };

    return RequestParameterBuilder;
  });

}) (app);