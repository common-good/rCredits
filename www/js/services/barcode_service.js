(function(app) {
  'use strict';

  var WebScanner = function() {
    this.scan = function(success, fail) {
      success (WebScanner.DEFAULT_WEB_SCAN);
    }
  };

  // This the read from the 'Member' 'Curt Customer'
  WebScanner.DEFAULT_WEB_SCAN = {text: "HTTP://NEW.RC4.ME/AAK.NyCBBlUF1qWNZ2k", format: "QR_CODE", cancelled: false};

  app.service('BarcodeService', function($q, $ionicPlatform) {

    var self;
    var BarcodeService = function() {
      self = this;
      this.configure_();
    };

    BarcodeService.WebScanner = WebScanner;

    BarcodeService.prototype.configure_ = function() {
      if (ionic.Platform.isWebView()) {
        $ionicPlatform.ready(function() {
          self.scanner = cordova.plugins.barcodeScanner;
        });
      } else {
        this.scanner = new WebScanner ();
      }
    };

    // Fetches a barcode.
    // Returns a promise that resolves with the scanned data when scanning is complete.
    BarcodeService.prototype.scan = function() {
      return $q (function(resolve, reject) {
        self.scanner.scan(function(scanResult) {
            self.scanSuccess_(resolve, reject, new BarcodeResult (scanResult));
          },
          _.partial(self.scanFail_, reject).bind(self));
      });
    };

    BarcodeService.prototype.scanSuccess_ = function(sucessFn, rejectFn, barCodeResult) {
      console.log("Scan result: ", barCodeResult);

      if (barCodeResult.wasCancelled()) {
        rejectFn ('Scan was Cancelled');
      }

      if (!barCodeResult.isQRCode()) {
        rejectFn ('Scan must be a QR CODE');
      } else {
        sucessFn (barCodeResult.text);
      }
    };

    BarcodeService.prototype.scanFail_ = function(rejectFn, scanError) {
      console.error("Scan failed: ", scanError);
      rejectFn (scanError);
    };

    return new BarcodeService ();
  });

}) (app);
