<?php
use rCredits\Util as u;

define('TESTING', TRUE); // use this to activate extra debugging statements (if (defined('TESTING')))
$module = $_SERVER['QUERY_STRING'];

$modules = $module ? array($module) : array('rsms', 'rsmart', 'rweb');

global $okALL, $noALL;
$okALL = $noALL = 0; // overall results counters
foreach($modules as $module) doModule($module);
debug("OVERALL: OK:$okALL NO:$noALL");

function doModule($module) {
  global $ok, $no, $okALL, $noALL, $oneScene;
  $ok = $no = 0; // results counters

  $path = __DIR__ . "/$module"; // relative path from compiler to module directory
  // SMS: OpenAnAccountForTheCaller AbbreviationsWork ExchangeForCash GetHelp GetInformation Transact Undo OfferToExchangeUSDollarsForRCredits
  // Smart: Startup IdentifyQR TransactMemberToMember TransactMemberToAgent TransactAgentToMember TransactAgentToAgent Undo
  $tests = str_replace("$path/features/", '', str_replace('.feature', '', findFiles("$path/features", '/.*\.feature/')));
//  $tests = array('TransactMemberToMember'); // uncomment to run just one feature (test set)
//  $oneScene = 'testTheCallerConfirmsAPayment'; // uncomment to run just one test scenario

  foreach ($tests as $test) dotest($module, $test);
  debug("MODULE $module: OK:$ok NO:$no");
}  

function dotest($module, $test) {
  global $results, $summary, $user, $oneScene;
  include ($test_filename = __DIR__ . "/$module/tests/$test.test");
  
  $temp_user = $user; $user = array();
  $classname = $module . $test;
//  $t = new rsmsOpenAnAccountForTheCaller();
  $t = new $classname();
  $s = file_get_contents($test_filename);
  preg_match_all('/function (test.*?)\(/sm', $s, $matches);
  $scenes = @$oneScene ? array($oneScene) : $matches[1];
  foreach ($scenes as $one) {
    $results = array('PASS!');
//    echo "<br>\r\n$one<br>\r\n";
    $t->setUp();
    $t->$one(); // run one test
    
    // Display results intermixed with debugging output, if any (so don't collect results before displaying)
    $results[0] .= " [$test] $one";
    debug(join(PHP_EOL, $results));
  }
  $user = $temp_user;
}

/**
 * Find Files (copied from gherkin)
 *
 * Return an array of files matching the given pattern.
 *
 * @param string $path (optional, defaults to current directory)
 *   the directory to search
 *
 * @param string $pattern (optional, defaults to all files)
 *   return filenames matching this pattern
 *
 * @param array $result (optional)
 *   partial results. if this array is supplied, then recurse subdirectories
 *
 * @return
 *   an array of filenames, qualified by path (including the initial directory $path)
 */
function findFiles($path = '.', $pattern = '/./', $result = '') {
  if (!($recurse = is_array($result))) $result = array();
  if (!is_dir($path)) die('No features folder found for that module.');
  $dir = dir($path);
  
  while ($filename = $dir->read()) {
    if ($filename == '.' or $filename == '..') continue;
    $filename = "$path/$filename";
    if (is_dir($filename) and $recurse) $result = findFiles($filename, $pattern, $result);
    if (preg_match($pattern, $filename)){
      $result[] = $filename;
    }
  }
  return $result;
}

class DrupalWebTestCase {
  function setUp() {}
  function assertTrue($bool) {
    global $results, $summary;
    global $ok, $no, $okALL, $noALL;
    $trace = debug_backtrace();
    list ($zot, $step, $test) = $trace[0]['args'];
//    $step = preg_replace('/ *\| */', '|', $step);
    $step = str_replace('\\', "\n     ", $step);
    $step = str_replace("''", '"', $step);
    $where = $test == 'Setup' ? "[$test] " : '';
    $results[] = $result = ($bool ? 'OK' : 'NO') . ": $where$step";
    if ($bool) {
      $ok++; $okALL++;
    } else {
      $no++; $noALL++;
      $results[0] = 'FAIL';
    }
//    echo $result . "<br>\r\n";
  }
}