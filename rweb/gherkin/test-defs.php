<?php

define('GHERKIN_EOL', '\\'); // record delimiter in multiline arguments
global $sceneTest; // current scenario test object
global $testOnly; // step functions can create the asserted reality, except when called as "Then" (or "And")

function Gherkin($statement, $type) {global $sceneTest; return $sceneTest->gherkin($statement, $type);}
function Given($statement) {return Gherkin($statement, __FUNCTION__);}
function When_($statement) {return Gherkin($statement, __FUNCTION__);}
function Then_($statement) {return Gherkin($statement, __FUNCTION__);}
function And__($statement) {return Gherkin($statement, __FUNCTION__);}

// the body of the gherkin function in each test file
// (indirectly called by Given(), When(), etc.
function gherkinGuts($statement, $type) {
  global $sceneTest, $testOnly;
  if($type == 'Given' or $type == 'When_') $testOnly = FALSE;
  if($type == 'Then_') $testOnly = TRUE;

  $arg_patterns = '"(.*?)"|([\-\+]?[0-9]+(?:[\.\,\-][0-9]+)*)';
  $statement = getConstants(strtr($statement, $sceneTest->subs));
  preg_match_all("/$arg_patterns/ms", $statement, $matches);
  $args = otherFixes(multilineCheck($matches[0])); // phpbug: $matches[1] has null for numeric args (so the check removes quotes)
  $count = count($args);
  $function = lcfirst(preg_replace("/$arg_patterns|[^A-Z]/ims", '', ucwords($statement)));

  switch ($count) {
    case 0: return $function();
    case 1: return $function($args[0]);
    case 2: return $function($args[0], $args[1]);
    case 3: return $function($args[0], $args[1], $args[2]);
    case 4: return $function($args[0], $args[1], $args[2], $args[3]);
    default: die("Too many args ($count) in statement: $statement");
  }
}

function sceneSetup($scene, $test) {
  global $sceneTest;
  $sceneTest = $scene;
  $sceneTest->subs = usualSubs(); 
  $sceneTest->currentTest = $test;
}

/**
 * Random String Generator
 *
 * int $len: length of string to generate (0 = random 1->50)
 * string $type: ?=any 9=digits
 * return semi-random string with no single or double quotes in it (but maybe spaces)
 */
function randomString($len = 0, $type = '?'){
  if (!$len) $len = rand(1, 50);

 	$symbol = '-_^~@&|=+;!,(){}[].?%*# '; // no quotes
  $upper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $lower = 'abcdefghijklmnopqrstuvwxwz';
  $digits = '0123456789';
  
  $chars = $upper . $lower . $digits . $symbol;
  if ($type == '9') $chars = $digits;
  if ($type == 'A') $chars = $upper . $lower;
  
  for($s = ''; $len > 0; $len--) $s .= $chars{rand(0, strlen($chars)-1)};
  return($s); //  return str_shuffle($s); ?
}

function randomPhone() {return '+1' . rand(2, 9) . randomString(9, '9');}

/**
 * The Usual Subtitutions
 *
 * Set some common parameters that will remain constant throughout the Scenario
 * These may or may not get used in any particular Scenario, but it is convenient to have them always available.
 */
function usualSubs() {
  $subs_filename = __DIR__ . '/../usualSubs.inc';
  $date_format = '%d-%b-%Y';
  
  $result = array();
  $randoms = array('%whatever', '%random');
  for ($i = 1; $i <= 3; $i++) {
    $ramdoms[] = "%whatever$i";
    $ramdoms[] = "%random$i";
  }
  foreach ($randoms as $key) $result[$key] = '"' . randomString() . '"';
  
  for ($i = 1; $i <= 5; $i++) { // phone numbers
    while (in_array($number = randomPhone(), $result)) {} // US phone
    $result["%number$i"] = $number;
  }
  for ($i = 15; $i > 0; $i--) { // set up past dates highest first, eg to avoid missing the "5" in %today-15
    $result["%today-{$i}d"] = strftime($date_format, strtotime("-$i days"));
    $result["%today-{$i}w"] = strftime($date_format, strtotime("-$i weeks"));
    $result["%today-{$i}m"] = strftime($date_format, strtotime("-$i months"));
  }
  $result['%today'] = strftime($date_format, time()); // must be last
  if (file_exists($subs_filename)) include $subs_filename; // a chance to add or replace the usual subs
  return $result;
}

/**
 * Multi-line check
 *
 * If the final argument is a string representing a Gherkin multi-line definition of records,
 * then change it to the associative array represented by that string.
 *
 * @param $args: list of arguments to pass to the step function
 *
 * @return $args, possibly with the final argument replaced by an array
 */
function multilineCheck($args) {
  for($i = 0; $i < count($args); $i++) $args[$i] = squeeze($args[$i], '"');
  if (substr($last = end($args), 0, 4) != 'DATA') return $args;
  $data = explode(GHERKIN_EOL, preg_replace('/ *\| */m', '|', $last));
  array_shift($data);
  $keys = explode('|', squeeze(array_shift($data), '|'));
  $result = array();
  foreach ($data as $line) {
    if (function_exists('multiline_tweak')) multiline_tweak($line);
    $result[] = array_combine($keys, explode('|', squeeze($line, '|')));
  }
  $args[count($args) - 1] = $result;
  return $args;
}

/**
 * Squeeze a string
 *
 * If the first and last char of $string is $char, shrink the string by one char at both ends.
 */
function squeeze($string, $char) {
  $first = substr($string, 0, 1);
  $last = substr($string, -1);
  return ($first == $char and $last == $char)? substr($string, 1, strlen($string) - 2) : $string;
}

/**
 * Make a string's first character lowercase
 *
 * @param string $str
 * @return string the resulting string.
 */
if(!function_exists('lcfirst')) {
  function lcfirst($str) {
    $str[0] = strtolower($str[0]);
    return (string)$str;
  }
}

/**
 * Translate constant parameters in a string.
 * @param string $string: the string to fix
 * @return string: the string with constant names (preceded by %) replaced by their values
 * Constants must be uppercase and underscores (for example, if A_TIGER is defined as 1, %A_TIGER gets replaced with "1")
 */
function getConstants($string) {
  preg_match_all("/%([A-Z_]+)/ms", $string, $matches);
  $map = array();
  foreach ($matches[1] as $one) $map["%$one"] = constant($one);
  return strtr($string, $map);
}

/**
 * 
 */
function otherFixes($args) {
  foreach ($args as $key => $one) {
    if (!is_array($one)) {
      $one = str_replace("''", '"', $one); // lastly, interpret double apostrophes as double quotes
      if (is_numeric($without = str_replace(',', '', $one))) $one = $without; // remove commas from numbers
      $args[$key] = $one;
    } else $args[$key] = otherFixes($one);
  }
  return $args;
}