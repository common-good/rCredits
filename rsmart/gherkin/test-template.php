<?php
%FEATURE_HEADER
require_once __DIR__ . '/../gherkin/test-defs.php';
require_once __DIR__ . '/../%MODULE.steps';

class %MODULE%FEATURE_NAME extends DrupalWebTestCase {
  var $subs; // percent parameters (to Given(), etc.) and their replacements (eg: %number1 becomes some random number)
  var $currentTest;
  const FEATURE_NAME = '%MODULE Test - %FEATURE_NAME';
  const DESCRIPTION = '%FEATURE_LONGNAME';
  const MODULE = '%MODULE';

  public function gherkin($statement, $type) {
    $this->assertTrue(gherkinGuts($statement, $type), $statement, $this->currentTest);
  }
  
  public static function getInfo() {
    return array('name' => self::FEATURE_NAME, 'description' => self::DESCRIPTION, 'group' => ucwords(self::MODULE));
  }

  public function setUp() { // especially, enable any modules required for the tests
    parent::setUp(self::MODULE);
    $setup_filename = __DIR__ . '/../' . self::MODULE . '-setup.inc';
    if (file_exists($setup_filename)) include $setup_filename;
%SETUP_LINES
  }
%TESTS
}