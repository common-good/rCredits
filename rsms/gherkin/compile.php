<?php
$SHOWERRORS = TRUE;
error_reporting($SHOWERRORS ? E_ALL : 0); ini_set('display_errors', $SHOWERRORS); ini_set('display_startup_errors', $SHOWERRORS);

// Gherkin compiler
//
// Create a skeleton test for each feature in a module

$first_scenario_only = TRUE; // activate just the first scenario for each feature, to save time testing
// When ready, textedit replace "function notest" with "function test" in the .test file

$path = '..'; // relative path from compiler to module directory
$gEOL = '\\\\'; // end of line marker

$flnms = glob("$path/*.module");
if (empty($flnms)) error('No features found. The gherkin directory should have the same parent as the features directory. The parent should contain the module file. See the file "howto.txt".');

$MODULE = str_replace('.module', '', substr($flnms[0], 3));
$test_dir = "$path/tests";
if (!file_exists($test_dir)) mkdir($test_dir);

$info_filename = "$path/$MODULE.info";
$info = file_get_contents($info_filename);

$steps_filename = "$path/$MODULE.steps";
$steps_header = '<' . <<<EOF
?php
/**
 * @file
 *  Steps
 *
 * Provide step functions for functional testing.
 * This file is created automatically by the Gherkin compiler.
 *
 * Note, therefore, that most of this file might be changed automatically
 * when you run the compiler again. This @file header will not be affected,
 * but all of the function header comments are (re)generated automatically.
 *
 * Be assured that no functions will be deleted and the compiler will
 * never alter code within a function.
 *
 * You may also add statements just below this header (for example "use" and "require_once").
 */
EOF;

$steps_text = file_exists($steps_filename) ? file_get_contents($steps_filename) : $steps_header;

/** $steps array
 *
 * Associative array of step function information, indexed by step function name
 * Each step is in turn an associative array:
 *   'original' is "new", "changed", or the original function header from the steps file
 *   'english' is the english language description of the step
 *   'to_replace' is the text to replace in the steps file when the callers change: from the first caller through the function name
 *   'callers' array is the list of tests that use this specific step function
 *   'arg_count' is the number of arguments (for new steps only)
 */
$steps = get_steps($steps_text);

$features = findFiles("$path/features", '/\.feature$/', FALSE);

foreach ($features as $feature_filename) {
  $test_filename = str_replace('features/', 'tests/', str_replace('.feature', '.test', $feature_filename));
  $file_line = str_replace("$path/", '', "files[] = $test_filename\n");
  if (strpos($info, $file_line) === FALSE) $info .= $file_line;
  $test_data = do_feature($feature_filename, $steps);
  $test_data['MODULE'] = $MODULE;
  $test = file_get_contents('test_template.php');
  foreach($test_data as $from => $to) $test = str_replace("%$from", $to, $test);
  file_put_contents($test_filename, $test);
  echo "Created: $test_filename<br>";
}

foreach ($steps as $function_name => $step) {
  extract($step);
  $replacement = replacement($callers, $function_name); // (replacement includes function's opening parenthesis)
  if ($original == 'new') {
    for ($arg_list = '', $i = 0; $i < $arg_count; $i++) {
      $arg_list .= ($arg_list ? ', ' : '') . '$arg' . ($i + 1);
    }
    $steps_text .= "\n" // do not use <<<EOF here, because it results in extraneous EOLs
    . "/**\n"
    . " * $english\n"
    . " *\n"
    . " * in: {$replacement}$arg_list) {\n"
    . "  global \$test_only;\n"
    . "  todo;\n"
    . "}\n";
  } else {
//  echo "to=$to_replace rep=$replacement<br>\n"; $prev = $steps_text;
    $steps_text = str_replace($to_replace, $replacement, $steps_text);
//    die ('same:' . ($prev == $steps_text));
  }
}

file_put_contents($steps_filename, $steps_text);
file_put_contents($info_filename, $info);

echo "<br>Updated $steps_filename<br>Done. " . date('g:ia');

// END of program

/**
 * Do Feature
 *
 * Get the specific parameters for the feature's tests
 *
 * @param string $feature_filename
 *   feature path and filename relative to module
 *
 * @param array $steps (by reference)
 *   an associative array of empty step function objects, keyed by function name
 *   returned: the original array, with unique new steps added (old steps are not duplicated)
 *
 * @return associative array:
 *   GROUP: sub-project name (currently unused in template)
 *   FEATURE_NAME: titlecase feature name, with no spaces
 *   FEATURE_LONGNAME: feature name in normal english
 *   FEATURE_HEADER: standard Gherkin feature header, formatted as a comment
 *   TESTS: all the tests and steps
 */
function do_feature($feature_filename, &$steps) {
  global $first_scenario_only;
  $GROUP = basename(dirname(dirname($feature_filename)));
  $FEATURE_NAME = str_replace('.feature', '', basename($feature_filename));
  $FEATURE_LONGNAME = $FEATURE_NAME; // default English description of feature, in case it's missing from feature file
  $FEATURE_HEADER = '';
  $TESTS = '';
 
  $lines = explode("\n", file_get_contents($feature_filename));
  $state = '';
  $arg_patterns = '"(.*?)"|(-?[0-9]+(?:[\.,-][0-9]+)*)|(%[A-Z][A-Z0-9]+)';
  $lead = '  '; // line leader (indentation for everything in class definition
  $is_first_scenario = TRUE;

  while (!is_null($line = array_shift($lines))) {
    $line = trim($line);
    $any = preg_match('/^([A-Z]+)/i', $line, $matches);
    $word1 = $word1_original = $any ? $matches[1] : '';
    $tail = trim(substr($line, strlen($word1) + 1));
    if ($word1 == 'And') $word1 = 'And__';
    if ($word1 == 'When' or $word1 == 'Then') $word1 .= '_';

    if ($word1 == 'Feature') {
      $FEATURE_HEADER .= "//\n// $line\n";
      $FEATURE_LONGNAME = $tail;
      $is_first_scenario = TRUE;
    } elseif ($word1 == 'Scenario') {
      $test_function = 'test' . (preg_replace("/[^A-Z]/i", '', ucwords($tail)));
      if ($first_scenario_only and !$is_first_scenario) $test_function = 'no' . $test_function;
      $test_function_qualified = "$FEATURE_NAME - $test_function";
      $is_first_scenario = FALSE; // won't be first next time

      if ($state != 'Feature') {
        $TESTS .= "$lead}\n";
      }
      $TESTS .= "\n"
        . "$lead// $line\n"
        . "{$lead}public function $test_function() {\n"
        . "$lead  scene_setup(\$this, __FUNCTION__);\n";
    
    } elseif (in_array($word1, array('Given', 'When_', 'Then_', 'And__'))) {
      $multiline_arg = multiline_arg($lines);
      $tail_escaped = str_replace("'", "\\'", $tail) . $multiline_arg;
      $tail .= str_replace("\\'", "'", $multiline_arg);
      $TESTS .= "$lead  $word1('$tail_escaped');\n";

      $english = preg_replace("/$arg_patterns/msi", '(ARG)', $tail);
      $step_function = lcfirst(preg_replace("/$arg_patterns|[^A-Z]/msi", '', ucwords($tail)));

      if(isset($steps[$step_function])) {
        $old_english = $steps[$step_function]['english'];
        if($old_english != $english) {
          error("<br>WARNING: You tried to redefine step function $step_function. "
          . "Delete the old one first.<br>\nOld: $old_english<br>\nNew: $english<br>\n"
          . "in Feature: $FEATURE_LONGNAME<br>\n"
          . "in Scenario: $test_function<br>\n"
          . "in Step: $line<br>\n"
          );
        }
        if ($steps[$step_function]['original'] != 'new') $steps[$step_function]['original'] = 'changed';
        if (!in_array($test_function, $steps[$step_function]['callers'])) {
          $steps[$step_function]['callers'][] = $test_function_qualified;
        }
      } else {
        $original = 'new';
        $callers = array($test_function_qualified);
        preg_match_all("/$arg_patterns/msi", $tail, $matches);
        $args = $matches[1];
        $arg_count = count($args);
        $steps[$step_function] = compact(explode(',', 'original,english,callers,arg_count'));
      }
    } else {
      if ($state == 'Feature') {
        $FEATURE_HEADER .= "//   $line\n";
      } elseif ($state == 'Scenario') {
        $TESTS .= "$lead *   $line\n";
      }
      $word1 = ''; // don't use this to set state
    }
    if ($word1) $state = $word1;
  }

  if ($state != '' and $state != 'Feature') $TESTS .= "$lead}\n"; // close the final test function definition

  return compact(explode(',', 'INC_PATH,GROUP,FEATURE_NAME,FEATURE_LONGNAME,FEATURE_HEADER,TESTS'));
}

/**
 * Find Files
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
  $prefix = $path . '/';
  $dir = dir($path);
  
  while ($filename = $dir->read()) {
    if ($filename === '.' || $filename === '..') continue;
    $filename = $prefix . $filename;
    if (is_dir($filename) and $recurse) $result = findFiles($filename, $pattern, $result);
    if (preg_match($pattern, $filename)){
      $result[] = $filename;
    }
  }
  return $result;
}

function error($error_message) {
  die($error_message);
}

/**
 * Get steps
 *
 * Given the text of the steps file, return an array of steps (see $steps)
 *
 */
function get_steps($steps_text) {
  $step_keys = explode(',', 'original,english,to_replace,callers,function_name');
  $pattern = ''
  . '^/\*\*$\s'
  . '^ \* (.*?)$\s'
  . '^ \*$\s'
  . '^ \* in: ((.*?)$\s'
  . '^ \*/$\s'
  . '^function (.*?)\()';
  preg_match_all("~$pattern~ms", $steps_text, $matches, PREG_SET_ORDER);
  $steps = array();
  foreach ($matches as $step) {
    $step = array_combine($step_keys, $step);
//    $step['callers'] = explode("\n *     ", $step['callers']); // add to the list, but don't delete
    $step['callers'] = array(); // rebuild this list every time
    $steps[$step['function_name']] = $step; // use the function name as the index for the step
  }
//  print_r($matches); die();
  return $steps;
}

/**
 * Replacement text
 *
 * When updating an existing step function, replace the header with this.
 * (guaranteed to be unique for each step)
 */
function replacement($callers, $function_name) {
  $callers = join("\n *     ", $callers);
  return "$callers\n */\nfunction $function_name(";
}

/**
 * Multiline Argument
 *
 * See if the next few lines represent data records using the syntax:
 *   | field1 | field2 | field3 |
 *   | a1     | b1     | c1     |
 *   | a2     | b2     | c2     |
 * 
 * @param array $lines: the remaining lines of the feature file
 *
 * @return
 *   the additional lines, if any, to add to the first
 *   $lines (implicit) the remaing lines of the feature file
 */
function multiline_arg(&$lines) {
  global $gEOL;
  $result = '';
  while (substr(trim($lines[0]), 0, 1) == '|') {
    $line = str_replace("'", "\\'", trim(array_shift($lines)));
    $result .= "'\n    . '$gEOL$line";
  }
  return $result ? " \"DATA$result\"" : '';
}
