// Karma configuration
// Generated on Tue Dec 15 2015 11:52:17 GMT-0300 (ART)

module.exports = function(config) {
  config.set({

    // base path that will be used to resolve all patterns (eg. files, exclude)
    basePath: '',


    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],


    // list of files / patterns to load in the browser
    files: [
      'www/lib/ionic/js/ionic.bundle.js',
      'www/lib/angular-translate/angular-translate.min.js',
      'www/lib/angular-translate-loader-static-files/angular-translate-loader-static-files.min.js',
      'www/lib/angular-translate-handler-log/angular-translate-handler-log.min.js',
      'www/lib/angular-local-storage/dist/angular-local-storage.min.js',
      'www/lib/prototypejs/dist/prototype.min.js',
      'www/lib/underscore/underscore-min.js',
      'www/lib/moment/moment.js',
      'node_modules/angular-mocks/angular-mocks.js',
      'www/js/models/user.js',
      'www/js/**/*.js',
      'test/**/*_spec.js',
      'www/templates/**/*.html'
    ],


    // list of files to exclude
    exclude: [],


    // preprocess matching files before serving them to the browser
    // available preprocessors: https://npmjs.org/browse/keyword/karma-preprocessor
    preprocessors: {},

    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress', 'html', 'junit', 'mocha'],

    // web server port
    port: 9876,


    // enable / disable colors in the output (reporters and logs)
    colors: true,


    // level of logging
    // possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
    logLevel: config.LOG_INFO,


    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: true,


    // start these browsers
    // available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
    browsers: ['Chrome'],


    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    singleRun: false,

    // Concurrency level
    // how many browser should be started simultanous
    concurrency: Infinity
  })
}
