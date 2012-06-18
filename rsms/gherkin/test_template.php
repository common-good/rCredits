<?php
%FEATURE_HEADER
require_once dirname(__FILE__) . '/../gherkin/test_defs.php';
require_once dirname(__FILE__) . '/../%MODULE.steps';

class %MODULE%FEATURE_NAME extends DrupalWebTestCase {
  var $subs; // percent parameters (to Given(), etc.) and their replacements (eg: %number1 becomes some random number)
  var $current_test;
  const FEATURE_NAME = '%MODULE Test - %FEATURE_NAME';
  const DESCRIPTION = '%FEATURE_LONGNAME';
  const MODULE = '%MODULE';

  public function gherkin($statement, $type) {
    $this->assertTrue(gherkin_guts($statement, $type), $statement, $this->current_test);
  }
  
  public static function getInfo() {
    return array('name' => self::FEATURE_NAME, 'description' => self::DESCRIPTION, 'group' => ucwords(self::MODULE));
  }

  public function setUp() { // especially, enable any modules required for the tests
    parent::setUp(self::MODULE);
    $setup_filename = dirname(__FILE__) . '/../' . self::MODULE . '_setup.inc';
    if (file_exists($setup_filename)) include $setup_filename;
  }
%TESTS
}