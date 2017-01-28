/* global app, User */
(function (app) {
	app.service('SQLiteService', function ($q, $timeout, NotificationService) {
		var self;
		var SQLiteService = function () {
			self = this;
			//this.sqlPlugin = window.sqlitePlugin || window;
			window.indexedDB = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
			window.IDBTransaction = window.IDBTransaction || window.webkitIDBTransaction || window.msIDBTransaction || {READ_WRITE: "readwrite"};
			window.IDBKeyRange = window.IDBKeyRange || window.webkitIDBKeyRange || window.msIDBKeyRange;
			this.sqlPlugin = window;
			this.db = null;
			this.request = null;
		};
		SQLiteService.prototype.createDatabase = function () {
			var openPromise = $q.defer();
			this.request = indexedDB.open(window.rCreditsConfig.SQLiteDatabase.name);
			this.request.onupgradeneeded = function () {
				// The database did not previously exist, so create object stores and indexes.
				this.db = this.request.result;
				this.members = this.db.createObjectStore("members", {keyPath: "qid"});
				this.txs = this.db.createObjectStore("members", {keyPath: "me"});
				this.titleIndex = this.members.createIndex("custQid", "members", {unique: true});
				this.errors = this.db.createObjectStore("errors", {keypath: "qid"});
				this.db.oncomplete = function () {
					this.db = this.request.result;
					console.log(db);
					return openPromise.promise;
				};
			};
			this.request.onerror = function (event) {
				console.log(event);
				return openPromise.promise;
			};
			this.request.onsuccess = function () {
				this.db = this.request.result;
				console.log(db);
				return openPromise.promise;
			};
			$timeout(function () {
				openPromise.resolve();
			}, 1000);
		};
		SQLiteService.prototype.ex = function () {
			var txPromise = $q.defer();
			txPromise.resolve(true);
			return txPromise.promise;
		};
		SQLiteService.prototype.executeQuery = function (table, params) {
			var txPromise = $q.defer();
			if (table === "member") {
				this.members.put({params});
			} else if (table === "txs") {
				this.txs.put({params});
			} else {
				console.log({qid: User.getId, table:table, details: params});
				this.errors.put({qid: User.getId, details: params});
			}
			return txPromise.promise;
		};
		SQLiteService.prototype.executeQueryOld = function (sqlQuery) {
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
					"data TEXT," + // hash of cardCode, amount, created, and me (as proof of agreement)
					"account TEXT," + //account particulars
					"description TEXT);" // always "reverses..", if this tx undoes a previous one (previous by date)
					);
			}).then(function () {
				self.executeQuery_("CREATE INDEX IF NOT EXISTS custQid ON members(qid)");
			});
		};
		SQLiteService.prototype.init = function () {
			this.createDatabase();
			console.log(this);
		};
		return new SQLiteService();
	});
})(app);
