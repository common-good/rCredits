(function(app) {

  'use strict';
  app.directive('subheader', function($timeout, NetworkService, UserService) {
    return {
      restrict: 'E',
      templateUrl: 'templates/subheader.html',
      bindToController: true,
      controllerAs: 'offlCtrl',
      link: function(scope) {
        var self = this, ionContent;

        var init = function() {
          ionContent = document.getElementsByTagName("ion-content")[0];
          scope.ionContent = ionContent;
          scope.$watch(function() {
            return NetworkService.isOffline() || scope.offlCtrl.isDemoMode();
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
        }, 1000);
      },
      controller: function($scope) {

        this.isOffline = function() {
          return NetworkService.isOffline();
        };

        this.isDemoMode = function() {
          var user = UserService.currentUser();
          return user && user.isDemo();
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
