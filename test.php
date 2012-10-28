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
  global $ok, $no, $okALL, $noALL, $oneScene, $oneVariant;
  $ok = $no = 0; // results counters

  $path = __DIR__ . "/$module"; // relative path from compiler to module directory
  // SMS: OpenAnAccountForTheCaller AbbreviationsWork ExchangeForCash GetHelp GetInformation Transact Undo OfferToExchangeUSDollarsForRCredits
  // Smart: Startup IdentifyQR Transact UndoCompleted UndoAttack Insufficient
  $features = str_replace("$path/features/", '', str_replace('.feature', '', findFiles("$path/features", '/.*\.feature/')));

  $features = array('Insufficient'); // uncomment to run just one feature (test set)
//  $oneScene = 'testAMemberConfirmsRequestToUndoACompletedCashCharge'; // uncomment to run just one test scenario
//  $oneVariant = 0; // uncomment to focus on a single variant (usually 0)

  foreach ($features as $feature) dotest($module, $feature);
  debug("MODULE $module: OK:$ok NO:$no");
}  

function dotest($module, $feature) {
  global $results, $summary, $user, $oneScene, $oneVariant;
  include ($feature_filename = __DIR__ . "/$module/tests/$feature.test");
  
  $temp_user = $user; $user = array();
  $classname = $module . $feature;
  $t = new $classname();
  $s = file_get_contents($feature_filename);
  preg_match_all('/function (test.*?)\(/sm', $s, $matches);

  foreach ($matches[1] as $one) {
    list ($scene, $variant) = explode('_', $one);
    if (@$oneScene) if ($scene != $oneScene) continue;
    if (isset($oneVariant)) if ($variant != $oneVariant) continue;

    $results = array('PASS!');
    $t->setUp();
    $t->$one(); // run one test
    
    // Display results intermixed with debugging output, if any (so don't collect results before displaying)
    $results[0] .= " [$feature] $one";
    $results[0] = color($results[0], 'darkgoldenrod');
    \drupal_set_message(join(PHP_EOL, $results));
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
    list ($zot, $step, $feature) = $trace[0]['args'];
    $step = str_replace('\\', "\n     ", $step);
    $step = str_replace("''", '"', $step);
    $where = $feature == 'Setup' ? "[$feature] " : '';
    list ($result, $color) = $bool ? array('OK', 'lightgreen') : array('NO', 'yellow');
    $results[] = $result = color("$result: $where$step", $color);
    if ($bool) {
      $ok++; $okALL++;
    } else {
      $no++; $noALL++;
      $results[0] = 'FAIL';
    }
  }
}

function color($msg, $color) {
  return "<pre style='background-color:$color;'>$msg</pre>";
}