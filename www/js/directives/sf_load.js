app.directive('sfLoad', ['$parse', function ($parse) {
  return {
    restrict: 'A',
    link: function (scope, elem, attrs) {
      var fn = $parse(attrs.sfLoad);
      elem.on('load', function (event) {
        scope.$apply(function() {
          fn(scope, { $event: event });
        });
      });
    }
  };
}]);
