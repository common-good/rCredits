%FEATURE_HEADER
describe('%MODULE% -- FEATURE_NAME', function () {
	'use strict';
  var steps = new %MMODULE_steps();

  beforeEach(function () { // Setup
    steps.extraSetup();
%SETUP_LINES  });
%TESTS
});
  