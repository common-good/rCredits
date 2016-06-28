# rCredits App

## Initial Setup

. Use a version of node.js that works, including a compatible version of npm (node.js 5.11 worked but the latest beta6 did not)
. If developing on Windows, use cygwin (because you need it for the Linux-style set-variable console command (the BUILD_TARGET line below)
. add this line to the test server's .htaccess file: Header set Access-Control-Allow-Origin *

In your project folder:
. In the root 'index.html' file, in ' <meta http-equiv="Content-Security-Policy" ' change 'http://192.168.2.2:*' to your own local url
. $ npm install -g bower
. $ npm install
. $ npm install karma --save-dev
. In karma.conf.js, add this line as the first element in the list of files:
      'node_modules/jquery.min.js',
. Download and copy jquery.min.js to node_modules
. $ ionic state restore
. $ bower install
. $ ionic setup sass
. $ BUILD_TARGET=development gulp config
  "development" might instead be "staging", or "production". This command copies config from the `/config.js` and `/local_config.js` files. Only config matching the given build target (or lying outside an @if block) is copied.

## To run the app in your default browser:

$ ionic serve

## To Run on Device

```
$ ionic state restore # Only if plugins or platforms may have changed
$ ionic run <android|ios>
```

## Adding a Plugin

. Always use `ionic plugin add`, not `cordova plugin add`
. Ensure the version is specified in the package.json.
. For regular plugins, add @version to the end of the command, e.g. `ionic plugin add cordova-plugin-audio-recorder-api@0.0.6`.
. For git repos, use `#` instead.
. If you don't know the version, you can add without version, check it with `ionic plugin list`, remove, and then add versioned.


### If Packages Get Screwed Up

$ npm rebuild

### If SCSS/CSS Gets Screwed Up

$ gulp sass

### To Run Unit Tests

$ . test.sh
