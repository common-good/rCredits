(function(window, app) {
  'use strict';

  app.service('User', function(MemberSqlService) {

    var User = Class.create({

      initialize: function(name) {
        this.name = name;
        this.can = 0;
        this.company = '';
        this.accountInfo = new AccountInfo();
      },

      isFromUrl: function(strUrl) {
        console.log("AccountUrl: ", this.accountInfo.url);
        console.log("ScannedUrl: ", strUrl);
        return this.accountInfo && this.accountInfo.url === strUrl;
      },

      getId: function() {
        return this.accountInfo.accountId;
      },

      getName: function() {
        return this.name
      },

      getCompany: function() {
        return this.company;
      },

      getPlace: function() {
        return this.accountInfo.accountId;
      },

      getBalance: function() {
        return this.accountInfo.securityCode;
      },

      getRewards: function() {
        return this.can;
      },

      getLastTx: function() {
        return -1
      },

      getBlobImage: function() {
        return this.accountInfo.blobImage;
      },

      saveInSQLite: function() {
        return MemberSqlService.saveMember(this);
      },

      default: '',
    });

    window.User = User;

    return User;

  });

})(window, app);
