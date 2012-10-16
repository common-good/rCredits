<?php
use rCredits\Util as u;

define('TESTING', TRUE);
$module = 'rsms';
//$tests = 'OpenAnAccountForTheCaller AbbreviationsWork ExchangeForCash GetHelp GetInformation Transaction Undo';
$tests = 'OfferToExchangeUSDollarsForRCredits';

class DrupalWebTestCase {
  function setUp() {}
  function assertTrue($bool) {
    global $results, $summary;
    $trace = debug_backtrace();
    list ($zot, $step, $test) = $trace[0]['args'];
//    $step = preg_replace('/ *\| */', '|', $step);
    $step = str_replace('\\', "\n     ", $step);
    $step = str_replace("''", '"', $step);
    $results[] = $result = ($bool ? 'OK' : 'NO') . ": [$test] $step";
    if (!$bool) $results[0] = 'FAIL';
//    echo $result . "<br>\r\n";
  }
}

foreach (u\ray($tests) as $test) dotest($module, $test);

function dotest($module, $test) {
  global $results, $summary, $user;
  include ($test_filename = __DIR__ . "/$module/tests/$test.test");
  
  $temp_user = $user; $user = array();
  $classname = $module . $test;
//  $t = new rsmsOpenAnAccountForTheCaller();
  $t = new $classname();
  $s = file_get_contents($test_filename);
  preg_match_all('/function (test.*?)\(/sm', $s, $matches);
  foreach ($matches[1] as $one) {
    $results = array('PASS!');
//    echo "<br>\r\n$one<br>\r\n";
    $t->setUp();
    $t->$one(); // run one test
    debug($results);
  }
  $user = $temp_user;
}