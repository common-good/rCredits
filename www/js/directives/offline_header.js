(function(app) {

  'use strict';
  app.directive('offlineHeader', function($timeout, NetworkService) {
    return {
      restrict: 'E',
      templateUrl: 'templates/offline_header.html',
      bindToController: true,
      controllerAs: 'offlCtrl',
      link: function(scope) {
        var self = this, ionContent;

        var init = function() {
          ionContent = document.getElementsByTagName("ion-content")[0];
          scope.ionContent = ionContent;
          scope.$watch(function() {
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
        };


        $timeout(function() {
          init();
        },1000);
      },
      controller: function($scope) {

        this.isOffline = function() {
          return NetworkService.isOffline();
        };

        $scope.$on('$destroy', function() {
          if ($scope.ionContent) {
            //$scope.ionContent.removeClassName('has-subheader');
          }
        });
      }

    };
  });

})(app);
