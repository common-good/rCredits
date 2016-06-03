app.service('UserService', function ($q, $http, $httpParamSerializer, RequestParameterBuilder, User, Seller, Customer, $rootScope, $timeout,
	PreferenceService, CashierModeService, $state, NetworkService, MemberSqlService, NotificationService, SelfServiceMode) {
	'use strict';
	var LOGIN_FAILED = '0';
	var LOGIN_BY_AGENT = '1';
	var LOGIN_BY_CUSTOMER = '0';
	var FIRST_PURCHASE = '-1';
	var self;
	var UserService = function () {
		self = this;
		this.seller = null;
		this.LOGIN_SELLER_ERROR_MESSAGE = 'login_your_self';
	};
	// Gets the current user. Returns the user object,
	// or null if there is no current user.
	UserService.prototype.currentUser = function () {
		return this.seller;
	};
	// Gets the current customer. Returns an object
	// or null if there is no current customer.
	UserService.prototype.currentCustomer = function () {
//		console.log(this.customer.accountInfo);
		return this.customer;
	};
	UserService.prototype.loadSeller = function (sellerId) {
		try {
			var seller = new Seller();
			seller.fillFromStorage();
			if (sellerId && sellerId !== seller.getId()) {
				throw "Seller not found";
			}
			if (!seller.hasDevice()) {
				throw "Seller does not have deviceID";
			}
			this.seller = seller;
			CashierModeService.init();
			$timeout(function () {
				$rootScope.$emit('sellerLogin');
			});
			return seller;
		} catch (e) {
			console.error(e.message);
			return null;
		}
	};
	UserService.prototype.makeRequest_ = function (params, accountInfo) {
		var urlConf = new UrlConfigurator();
		return $http({
			method: 'POST',
			url: urlConf.getServerUrl(accountInfo),
			headers: {
				'Content-Type': 'application/x-www-form-urlencoded'
			},
			data: $httpParamSerializer(params)
		});
	};
	UserService.prototype.validateDemoMode = function (accountInfo) {
		if (!this.currentUser()) {
			return;
		}
		if (this.currentUser().isDemo() && !accountInfo.isDemo()) {
			throw "can_not_use_real_card";
		} else if (!this.currentUser().isDemo() && accountInfo.isDemo()) {
			throw "can_not_use_demo_card";
		}
	};
	UserService.prototype.loginWithRCard_ = function (params, accountInfo) {
		return this.makeRequest_(params, accountInfo).then(function (res) {
			var responseData = res.data;
			if (responseData.ok === LOGIN_FAILED) {
				throw responseData.message;
			}
			return responseData;
		}).catch(function (err) {
			console.error(err);
			if (_.isString(err)) {
				throw err;
			}
			throw err.statusText;
		});
	};
	UserService.prototype.loginWithRCardOffline = function (accountInfo) {
		var loadSellerPromise = $q.defer();
		var seller = this.loadSeller(accountInfo.accountId);
		if (seller) {
			loadSellerPromise.resolve(seller);
		} else {
			loadSellerPromise.reject("Seller not found");
		}
		return loadSellerPromise.promise;
	};
	// Logs user in given the scanned info from an rCard.
	// Returns a promise that resolves when login is complete.
	// If this is the first login, the promise will resolve with {firstLogin: true}
	// The app should then give notice to the user that the device is associated with the
	// user.
	UserService.prototype.loginWithRCard = function (str) {
		var qrcodeParser = new QRCodeParser();
		qrcodeParser.setUrl(str);
		var accountInfo = qrcodeParser.parse();

		this.validateDemoMode(accountInfo);

		var params = new RequestParameterBuilder()
			.setOperationId('identify')
			.setSecurityCode(accountInfo.securityCode)
			.setMember(accountInfo.accountId)
			.getParams();

		if (NetworkService.isOffline()) {
			return this.loginWithRCardOffline(accountInfo).then(function () {
				PreferenceService.parsePreferencesNumber(self.currentUser().getCan());
			});
		}
		return this.loginWithRCard_(params, accountInfo)
			.then(function (responseData) {
				if (responseData.logon === LOGIN_BY_AGENT) {
					self.seller = self.createSeller(responseData);
					self.seller.accountInfo = accountInfo;
					self.seller.save();
					return self.seller;
				}

				if (responseData.logon === LOGIN_BY_CUSTOMER) {
					throw self.LOGIN_SELLER_ERROR_MESSAGE;
				}
			})
			.then(function () {
				PreferenceService.parsePreferencesNumber(self.currentUser().getCan());
			});
	};
	UserService.prototype.createSeller = function (sellerInfo) {
		var props = ['can', 'descriptions', 'company', 'default', 'time'];
		var seller = new Seller(sellerInfo.name);
		_.each(props, function (p) {
			seller[p] = sellerInfo[p];
		});
		if (!seller.hasDevice()) {
			if (seller.isValidDeviceId(sellerInfo.device)) {
				seller.setDeviceId(sellerInfo.device);
				seller.firstLogin = true;
			}
		}
		return seller;
	};
	// Gets customer info and photo given the scanned info from an rCard.
	// Returns a promise that resolves with the following arguments:
	// 1. user - The User object
	// 2. flags - A hash with the following elements:
	//      firstPurchase - Whether this is the user's first rCredits purchase. If so, the
	//        app should notify the seller to request photo ID.
	UserService.prototype.identifyCustomer = function (str, pin) {
		var qrcodeParser = new QRCodeParser();
		qrcodeParser.setUrl(str);
		var accountInfo = qrcodeParser.parse();
		this.validateDemoMode(accountInfo);
		var params = new RequestParameterBuilder()
			.setOperationId('identify')
			.setAgent(this.seller.default)
			.setMember(accountInfo.accountId)
			.setSecurityCode(accountInfo.securityCode);
		if (pin) {
			params.setPIN(pin);
		}
		params = params.getParams();
		if (NetworkService.isOffline()) {
			return MemberSqlService.existMember(accountInfo.accountId)
				.then(function (member) {
					self.customer = Customer.parseFromDb(member);
					return self.customer;
				})
				.catch(function (err) {
					console.log("Err", err);
					console.log("Customer accountInfo => ", accountInfo);
					return self.identifyOfflineCustomer().then(function (customerResponse) {
						self.customer = self.createCustomer(customerResponse);
						self.customer.unregistered = true;
						self.customer.accountInfo = accountInfo;
						return self.customer;
					});
				});
		}
		//is Online
		return this.loginWithRCard_(params, accountInfo)
			.then(function (responseData) {
				if (responseData.logon === LOGIN_BY_CUSTOMER || responseData.logon === FIRST_PURCHASE) {
					self.customer = self.createCustomer(responseData);
					if (responseData.logon === FIRST_PURCHASE) {
						self.customer.firstPurchase = true;
					}
					self.customer.accountInfo = accountInfo;
					return self.customer;
				}
				if (responseData.logon === LOGIN_BY_AGENT) {
					throw self.LOGIN_SELLER_ERROR_MESSAGE;
				}
			})
			.then(function (customer) {
				return self.getProfilePicture(accountInfo, accountInfo);
			})
			.then(function (blobPhotoUrl) {
				//self.customer.photo = blobPhotoUrl;
				return self.customer;
			});
	};
	UserService.prototype.identifyOfflineCustomer = function () {
		var customerLoginResponse = {
			"ok": "1",
			"logon": "0",
			"name": "",
			"place": "",
			"company": "",
			"balance": "0",
			"rewards": "0",
			"can": 131
		};
		var identifyPromise = $q.defer();
		identifyPromise.resolve(customerLoginResponse);
		return identifyPromise.promise;
	};
	UserService.prototype.createCustomer = function (customerInfo, customerAccountInfo) {
		var props = ['balance', 'can', 'company', 'place'];
		var customer = new Customer(customerInfo.name);
		customer.setRewards(customerInfo.rewards);

		_.each(props, function (p) {
			customer[p] = customerInfo[p];
		});

		return customer;
	};
	function convertImgToDataURLviaCanvas(url, callback, outputFormat) {
		var img = new Image();
		img.crossOrigin = 'Anonymous';
		img.onload = function () {
			var canvas = document.createElement('CANVAS');
			var ctx = canvas.getContext('2d');
			var dataURL;
			canvas.height = 208;
			canvas.width = 156;
			ctx.drawImage(this, 0, 0, 156, 208);
			dataURL = canvas.toDataURL("image/jpeg", .1);
			callback(dataURL);
			canvas = null;
		};
		img.src = url;
	}
	UserService.prototype.getProfilePicture = function (accountInfo) {
		var params = new RequestParameterBuilder()
			.setOperationId('photo')
			.setAgent(this.seller.default)
			.setMember(accountInfo.accountId)
			.setSecurityCode(accountInfo.securityCode)
			.getParams();
		var urlConf = new UrlConfigurator();
		return $http({
			method: 'POST',
			url: urlConf.getServerUrl(accountInfo),
			headers: {
				'Content-Type': 'application/x-www-form-urlencoded'
			},
			data: $httpParamSerializer(params),
			responseType: "arraybuffer"
		}).then(function (res) {
			var arrayBufferView = new Uint8Array(res.data);
			var blob = new Blob([arrayBufferView], {type: "image/jpeg"});
			var urlCreator = window.URL || window.webkitURL;
			var imgUrl = urlCreator.createObjectURL(blob);
			var imageConvert = $q.defer();
			convertImgToDataURLviaCanvas(imgUrl, function (base64Img) {
				self.customer.photo = base64Img;
				imageConvert.resolve(self.customer.photo);
			});
			return imageConvert.promise;
		})
			.catch(function (err) {
				console.error(err);
				throw err;
			});
	};
	UserService.prototype.authorize = function () {
		var SUCCEED = true;
		return $q(function (resolve, reject) {
			resolve(SUCCEED);
		});
	};
	UserService.prototype.enterCashierMode = function () {
		CashierModeService.activateCashierMode();
		$state.go('app.home');
	};
	// Logs the user out on the remote server.
	// Returns a promise that resolves when logout is complete, or rejects with error of fail.
	UserService.prototype.logout = function () {
		return $q(function (resolve, reject) {
			SelfServiceMode.disable();
			$rootScope.$emit('sellerLogout');
			self.customer = null;
			self.seller.removeFromStorage();
			self.seller = null;
			CashierModeService.disable();
			resolve();
		});
	};
	UserService.prototype.softLogout = function () {
		return $q(function (resolve, reject) {
			SelfServiceMode.disable();
			CashierModeService.disable();
			self.customer = null;
			self.seller = null;
			$state.go('app.login', {disableLoadSeller: true, openScanner: true});
			$rootScope.$emit('sellerLogout');
			resolve();
		});
	};
	return new UserService();
});