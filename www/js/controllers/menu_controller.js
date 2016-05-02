app.controller('MenuCtrl', function($scope, $state, $ionicLoading, BarcodeService, UserService, $ionicHistory,
                                    NotificationService, CashierModeService, PreferenceService, NetworkService,
                                    SelfServiceMode, $ionicSideMenuDelegate, $timeout) {



  // Logout

  $scope.logout = function() {
    $ionicLoading.show();

    UserService.logout()
      .then(function() {
        $state.go("app.login");
      })
      .catch(function(errorMsg) {
        NotificationService.showAlert({title: "error", template: errorMsg});
      })
      .finally(function() {
        $ionicLoading.hide();
      });
  };

  $scope.softLogout = function() {
    return UserService.softLogout();
  };

  // Redirects

  $scope.redirectToLogin = function() {
    $state.go("app.login");
  };

  $scope.redirectHome = function() {
    $ionicHistory.clearHistory();
    $ionicHistory.nextViewOptions({
      disableBack: true,
      disableAnimate: true
    });
    $ionicHistory.clearCache().then(function() {
      $state.go("app.home");
    });
  };

  $scope.redirectPreferences = function() {
    $state.go("app.preferences");
  };

  $scope.isCashierModeEnabled = function() {
    return CashierModeService.isEnabled();
  };

  $scope.isCashierModeAvailable = function() {
    return PreferenceService.getCashierModePref().isEnabled();
  };

  $scope.changeCompany = function() {
    var seller = UserService.currentUser();

    NotificationService.showConfirm({
      title: 'disassociate_company',
      subTitle: "haveToSignInAgain",
      okText: "confirm",
      cancelText: "cancel"
    }, {
      company: seller.company
    }).then(function(res) {
      if (res) {
        seller.removeDevice();
        $scope.logout();
      }
    });
  };

  $scope.enterCashierMode = function() {
    return UserService.enterCashierMode();
  };

  $scope.isOnline = function() {
    return NetworkService.isOnline();
  };

  $scope.changeToSelfServiceMode = function() {
    NotificationService.showConfirm({
      subtitle: '',
      title: "confirm_self_service_mode",
      okText: "confirm",
      cancelText: "cancel"
    }).then(function(res) {
      if (res) {
        SelfServiceMode.active();
      }
    });
  };

  $scope.isSelfServiceEnabled = function() {
    return PreferenceService.isSelfServiceEnabled();
  };

  $scope.isSelfServiceActive = function() {
    return SelfServiceMode.isActive();
  };

  $scope.$watch(function() {
    return !!UserService.currentUser();
  }, function(newValue, oldValue) {

    if (!newValue) {
      $timeout(function() {
        $ionicSideMenuDelegate.canDragContent(false);
      });
    }
  });

});
