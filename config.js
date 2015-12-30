// Most app config should go here. Use the BUILD_TARGET directives below to set config that depends on
// whether the app is in dev mode, staging mode, etc.

rCreditsConfig = {};

// @if BUILD_TARGET='development'
rCreditsConfig.serverproxyUrl = 'http://localhost:8100/pos';
rCreditsConfig.serverUrl = 'https://ws.rcredits.org/pos';
// @endif

// @if BUILD_TARGET='staging'
rCreditsConfig.serverproxyUrl = 'http://localhost:8100/pos';
rCreditsConfig.serverUrl = 'https://ws.rcredits.org/pos';
// @endif

// @if BUILD_TARGET='production'
rCreditsConfig.serverUrl = 'https://xxx.rcredits.org/pos';
// @endif

rCreditsConfig.version = '3.0';
rCreditsConfig.build = 300;
