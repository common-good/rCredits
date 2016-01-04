rCreditsConfig = {

  // @if BUILD_TARGET='development'
  serverproxyUrl:'http://localhost:8100/pos',
  serverUrl:'https://stage-xxx.rcredits.org/pos',
  version: '3.0',
  build: 300
  // @endif

  // @if BUILD_TARGET='staging'
  serverproxyUrl:'http://localhost:8100/pos',
  serverUrl:'https://stage-xxx.rcredits.org/pos',
  version: '3.0',
  build: 300
  // @endif

  // @if BUILD_TARGET='production'
  serverUrl:'https://xxx.rcredits.org/pos',
  version: '3.0',
  build: 300
  // @endif



};
