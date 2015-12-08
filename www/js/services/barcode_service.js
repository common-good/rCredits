app.service('BarcodeService', function ($q) {

  var BarcodeService = function () {
    self = this;
    this.user = null;
  };

  // Fetches a barcode.
  // Returns a promise that resolves with the scanned data when scanning is complete.
  BarcodeService.prototype.scan = function() {
    // Simulates scanning. Resolves the promise if SUCCEED is true, rejects with an error if false.
    var SUCCEED = false;

    return $q(function(resolve, reject) {
      setTimeout(function() {
        if (SUCCEED) {
          resolve('89317593q4oaosjo182yo4wi');
        } else {
          reject('Scanning failed for some reason.');
        }
      }, 1000);
    });
  };

  return new BarcodeService();
});
