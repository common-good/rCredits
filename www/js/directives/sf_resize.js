app.directive('sfResize', function ($window) {
  return function (scope, element) {
    scope.getWinHeight = function() {
      return $window.innerHeight;
    }

    var setNavHeight = function(newHeight) {
      element.css('height', newHeight + 'px');
    }

    // Set on load
    scope.$watch(scope.getWinHeight, function (newValue, oldValue) {
      element.css('height', (scope.getWinHeight() - 44) + 'px');
    }, true);

    // Set on resize
    angular.element($window).bind('resize', function () {
      scope.$apply();
    });
  };
});
