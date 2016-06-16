/* global app */
(function (app) {
	app.service('SQLiteService', function ($q, $timeout) {
		var self;
		var SQLiteService = function () {
			self = this;
			//this.sqlPlugin = window.sqlitePlugin || window;
			this.sqlPlugin = window;
			this.db = null;
		};
		SQLiteService.prototype.isDbEnable = function () {
			console.log(!!this.sqlPlugin);
			return !!this.sqlPlugin;
		};
		SQLiteService.prototype.createDatabase = function () {
			var openPromise = $q.defer();
			this.db = this.sqlPlugin.openDatabase(
				window.rCreditsConfig.SQLiteDatabase.name,
				window.rCreditsConfig.SQLiteDatabase.version,
				window.rCreditsConfig.SQLiteDatabase.description, 100000);
			$timeout(function () {
				openPromise.resolve();
			}, 1000);
			return openPromise.promise;
		};
		SQLiteService.prototype.ex = function () {
			var txPromise = $q.defer();
			txPromise.resolve(true);
			return txPromise.promise;
		};
		SQLiteService.prototype.executeQuery_ = function (query, params) {
			var txPromise = $q.defer();
			this.db.transaction(function (tx) {
				tx.executeSql(query, params, function (tx, res) {
					txPromise.resolve(res);
				}, function (tx, e) {
					console.error("executeSql ERROR: " + e.message);
					txPromise.reject(e.message);
				});
			}, function (error) {
				console.error('transaction error: ' + error.message);
//				txPromise.reject(error.message);
			}, function () {
			});
			return txPromise.promise;
		};
		SQLiteService.prototype.executeQuery = function (sqlQuery) {
			return this.executeQuery_(sqlQuery.getQueryString(), sqlQuery.getQueryData());
		};
		SQLiteService.prototype.createSchema = function () {
			this.executeQuery_(
				"CREATE TABLE IF NOT EXISTS members (" + // record of customers (and managers)
				"qid TEXT," + // customer or manager account code (like NEW.AAA or NEW:AAA)
				"name TEXT," + // full name (of customer or manager)
				"company TEXT," + // company name, if any (for customer or manager)
				"place TEXT," + // customer location / manager's company account code
				"balance REAL," + // current balance (as of lastTx) / manager's rCard security code
				"rewards REAL," + // rewards to date (as of lastTx) / manager's permissions / photo ID (!rewards.matches(NUMERIC))
				"lastTx INTEGER," + // unixtime of last reconciled transaction / -1 for managers
				"proof TEXT," +
				"photo TEXT);" // lo-res B&W photo of customer (normally under 4k) / full res photo for manager
				).then(function () {
				return self.executeQuery_(
					"CREATE TABLE IF NOT EXISTS txs (" +
					"me TEXT," + // company (or device-owner) account code (qid)
					"txid INTEGER DEFAULT 0," + // transaction id (xid) on the server (for offline backup only -- not used by the app) / temporary storage of customer cardCode pending tx upload
					"status INTEGER," + // see A.TX_... constants
					"created INTEGER," + // transaction creation datetime (unixtime)
					"agent TEXT," + // qid for company and agent (if any) using the device
					"member TEXT," + // customer account code (qid)
					"amount REAL," +
					"goods INTEGER," + // <transaction is for real goods and services>
					"proof TEXT," + // hash of cardCode, amount, created, and me (as proof of agreement)
					"description TEXT);" // always "reverses..", if this tx undoes a previous one (previous by date)
					);
			}).then(function () {
				self.executeQuery_("CREATE INDEX IF NOT EXISTS custQid ON members(qid)");
			});
		};
		SQLiteService.prototype.init = function () {
			if (!this.isDbEnable()) {
				console.warn("SQLite is not enable");
			}
			this.createDatabase().then(this.createSchema.bind(this));
		};
		return new SQLiteService();
	});
})(app);
