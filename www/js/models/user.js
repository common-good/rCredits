(function(window) {

  var User = Class.create({

    initialize: function(name) {
      this.name = name;
      this.can = 0;
      this.company = '';
    },

    isFromUrl: function(strUrl) {
      console.log("AccountUrl: ", this.accountInfo.url);
      console.log("ScannedUrl: ", strUrl);
      return this.accountInfo && this.accountInfo.url === strUrl;
    },

    default: '',
  });

  window.User = User;

})(window);
