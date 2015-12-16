rCreditsConfig = {

  // @if BUILD_TARGET='development'
  serverproxyUrl:'http://localhost:8100/pos',
  serverUrl:'https://ws.rcredits.org/pos?23',
  version: '3.0',
  build: 100
  // @endif

  // @if BUILD_TARGET='staging'
  serverproxyUrl:'http://localhost:8100/pos',
  serverUrl:'https://ws.rcredits.org/pos?23',
  version: '3.0',
  build: 100
  // @endif

  // @if BUILD_TARGET='production'
  serverUrl:'',
  version: '3.0',
  build: 100
  // @endif



};
