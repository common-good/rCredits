(function(window) {

  var UrlConfigurator = function() {
    this.baseUrl = rCreditsConfig.serverUrl;
  };

  UrlConfigurator.prototype.getServerUrl = function(memberId) {
    return this.baseUrl.replace ('xxx', memberId);
  };


  window.UrlConfigurator = UrlConfigurator;

}) (window);
