<?php
use rCredits\Util as u;

define('TESTING', TRUE);
$module = 'rsms';
$tests = 'Transact';
//$tests = 'OpenAnAccountForTheCaller AbbreviationsWork ExchangeForCash GetHelp GetInformation Transact Undo OfferToExchangeUSDollarsForRCredits'; // uncomment to run all tests

class DrupalWebTestCase {
  function setUp() {}
  function assertTrue($bool) {
    global $results, $summary, $ok, $no;
    $trace = debug_backtrace();
    list ($zot, $step, $test) = $trace[0]['args'];
//    $step = preg_replace('/ *\| */', '|', $step);
    $step = str_replace('\\', "\n     ", $step);
    $step = str_replace("''", '"', $step);
    $results[] = $result = ($bool ? 'OK' : 'NO') . ": [$test] $step";
    if ($bool) $ok++; else $no++;
    if (!$bool) $results[0] = 'FAIL';
//    echo $result . "<br>\r\n";
  }
}

global $ok, $no;
$ok = $no = 0; // results counters
foreach (u\ray($tests) as $test) dotest($module, $test);
debug("OVERALL: OK:$ok NO:$no");

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