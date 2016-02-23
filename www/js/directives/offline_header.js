(function(app) {

  'use strict';
  app.directive('offlineHeader', function($timeout, NetworkService) {
    return {
      restrict: 'E',
      templateUrl: 'templates/offline_header.html',
      bindToController: true,
      controllerAs: 'offlCtrl',
      controller: function($scope) {
        var self = this, ionContent;

        this.init = function() {
          ionContent = document.getElementsByTagName("ion-content")[0];
        };

        $scope.$watch(function() {
          return NetworkService.isOffline()
        }, function(newValue, oldValue) {
          if (newValue) {
            $timeout(function() {
              ionContent.addClassName('has-subheader');
            });
          } else {
            $timeout(function() {
              ionContent.removeClassName('has-subheader');
            });
          }
        });

        $timeout(function() {
          self.init();
        });

      }

    };
  });

})(app);
