# rCredits App

## Initial Setup

1.Use a version of node.js that works, including a compatible version of npm (node.js 5.11 worked but the latest beta6 did not)
1.If developing on Windows, use cygwin (because you need it for the Linux-style set-variable console command (the BUILD_TARGET line below)
1.add this line to the test server's .htaccess file: Header set Access-Control-Allow-Origin *

In your project folder:
1.In the root 'index.html' file, in ' <meta http-equiv="Content-Security-Policy" ' change 'http://192.168.2.2:*' to your own local url
1.$ npm install -g bower
1.$ npm install
1.$ npm install karma --save-dev
1.In karma.conf.js, add this line as the first element in the list of files:
      'node_modules/jquery.min.js',
1.Download and copy jquery.min.js to node_modules
1.$ ionic state restore
1.$ bower install
1.$ ionic setup sass
1.$ BUILD_TARGET=development gulp config
  "development" might instead be "staging", or "production". This command copies config from the `/config.js` and `/local_config.js` files. Only config matching the given build target (or lying outside an @if block) is copied.

## To run the app in your default browser:

$ ionic serve

## To Run on Device

```
$ ionic state restore # Only if plugins or platforms may have changed
$ ionic run <android|ios>
```

## Adding a Plugin

1.Always use `ionic plugin add`, not `cordova plugin add`
1.Ensure the version is specified in the package.json.
1.For regular plugins, add @version to the end of the command, e.g1.`ionic plugin add cordova-plugin-audio-recorder-api@0.0.6`.
1.For git repos, use `#` instead.
1.If you don't know the version, you can add without version, check it with `ionic plugin list`, remove, and then add versioned.


### If Packages Get Screwed Up

$ npm rebuild

### If SCSS/CSS Gets Screwed Up

$ gulp sass

### To Run Unit Tests

$ . test.sh
