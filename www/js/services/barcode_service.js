(function(app) {
  'use strict';

  var WebScanner = function() {
    this.id = 0;
    this.scan = function(success, fail) {
      success(WebScanner.SCANS[this.id++]);
    }
  };

  // This the read from Bob Bossman: NEW:AAB, WeHlioM5JZv1O9G
  WebScanner.SCANS = [
    {text: "HTTP://NEW.RC4.ME/AAB-WeHlioM5JZv1O9G", format: "QR_CODE", cancelled: false}, // Seller Bob Bossman
    {text: "HTTP://NEW.RC4.ME/ABB.ZzhWMCq0zcBowqw", format: "QR_CODE", cancelled: false}  // Customer Susan Shopper
  ];

  app.service('BarcodeService', function($q, $ionicPlatform, $rootScope) {

    var self;
    var BarcodeService = function() {
      self = this;
      this.configure_();
    };

    BarcodeService.WebScanner = WebScanner;

    /**
     * Only used on web app. not on mobile
     */
    BarcodeService.prototype.setScanForCustomer = function() {
      self.scanner.id = 0;
    };

    BarcodeService.prototype.configure_ = function() {
      if (ionic.Platform.isWebView()) {
        $ionicPlatform.ready(function() {
          self.scanner = cordova.plugins.barcodeScanner;
        });
      } else {
        this.scanner = new WebScanner();

        $rootScope.$on('sellerLogin', function() {
          self.scanner.id = 1;
        });

        $rootScope.$on('sellerLogout', function() {
          self.setScanForCustomer();
        });
      }
    };

    // Fetches a barcode.
    // Returns a promise that resolves with the scanned data when scanning is complete.
    BarcodeService.prototype.scan = function() {
      return $q(function(resolve, reject) {
        self.scanner.scan(function(scanResult) {
            self.scanSuccess_(resolve, reject, new BarcodeResult(scanResult));
          },
          _.partial(self.scanFail_, reject).bind(self));
      });
    };

    BarcodeService.prototype.scanSuccess_ = function(sucessFn, rejectFn, barCodeResult) {
      console.log("Scan result: ", barCodeResult);

      if (barCodeResult.wasCancelled()) {
        rejectFn('scanCancelled');
      }

      if (!barCodeResult.isQRCode()) {
        rejectFn('scanQRCode');
      } else {
        sucessFn(barCodeResult.text);
      }
    };

    BarcodeService.prototype.scanFail_ = function(rejectFn, scanError) {
      console.error("Scan failed: ", scanError);
      rejectFn(scanError);
    };

    return new BarcodeService();
  });

})(app);
