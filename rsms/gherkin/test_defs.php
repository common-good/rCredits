<?php

define('GHERKIN_EOL', '\\\\'); // record delimiter in multiline arguments
global $scene_test; // current scenario test object
global $test_only; // step functions can create the asserted reality, except when called as "Then" (or "And")

function Gherkin($statement, $type) {global $scene_test; return $scene_test->gherkin($statement, $type);}
function Given($statement) {return Gherkin($statement, __FUNCTION__);}
function When_($statement) {return Gherkin($statement, __FUNCTION__);}
function Then_($statement) {return Gherkin($statement, __FUNCTION__);}
function And__($statement) {return Gherkin($statement, __FUNCTION__);}

// the body of the gherkin function in each test file
// (indirectly called by Given(), When(), etc.
function gherkin_guts($statement, $type) {
  global $scene_test, $test_only;
  if($type == 'Given' or $type == 'When_') $test_only = FALSE;
  if($type == 'Then_') $test_only = TRUE;

  $arg_patterns = '"(.*?)"|(-?[0-9]+(?:[\.,-][0-9]+)*)';
  foreach ($scene_test->subs as $from => $to) if($from) $statement = str_replace($from, '"'.$to.'"', $statement);
  preg_match_all("/$arg_patterns/ms", $statement, $matches);
  $args = multiline_check($matches[0]); // phpbug: $matches[1] has null for numeric args (so the check removes quotes)
  $count = count($args);
  $function = lcfirst(preg_replace("/$arg_patterns|[^A-Z]/ims", '', ucwords($statement)));

  switch ($count) {
    case 0: return $function();
    case 1: return $function($args[0]);
    case 2: return $function($args[0], $args[1]);
    case 3: return $function($args[0], $args[1], $args[2]);
    case 4: return $function($args[0], $args[1], $args[2], $args[3]);
  }
}

function scene_setup($scene, $test) {
  global $scene_test;

  $scene_test = $scene;
  $scene_test->subs = usual_subs(); 
  $scene_test->current_test = $test;
}

/*
 * Random String Generator
 *
 * int $len: length of string to generate (0 = random 1->50)
 * string $type: ?=any 9=digits
 * return semi-random string with no single or double quotes in it (but maybe spaces)
 */
function random_string($len = 0, $type = '?'){
  if (!$len) $len = rand(1, 50);

 	$symbol = '-_^~@&|=+;!,(){}[].?%*# '; // no quotes
  $upper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $lower = 'abcdefghijklmnopqrstuvwxwz';
  $digits = '0123456789';
  
  $chars = $upper . $lower . $digits . $symbol;
  if ($type == '9') $chars = $digits;
  
  for($s = ''; $len > 0; $len--) $s .= $chars{rand(0, strlen($chars)-1)};
  return($s); //  return str_shuffle($s); ?
}

/*
 * The Usual Subtitutions
 *
 * Set some common parameters that will remain constant throughout the Scenario
 * These may or may not get used in any particular Scenario, but it is convenient to have them always available.
 */
function usual_subs() {
  return array(
    '%number1' => random_string(0, '9'),
    '%number2' => random_string(0, '9'),
    '%whatever' => random_string(),
    '%random' => random_string(),
  );
}

/*
 * Multi-line check
 *
 * If the final argument is a string representing a Gherkin multi-line definition of records,
 * then change it to the associative array represented by that string.
 *
 * @param $args: list of arguments to pass to the step function
 *
 * @return $args, possibly with the final argument replaced by an array
 */
function multiline_check($args) {
  for($i = 0; $i < count($args); $i++) $args[$i] = squeeze($args[$i], '"');
  if (substr($last = end($args), 0, 4) != 'DATA') return $args;

  $data = explode(GHERKIN_EOL, preg_replace('/ *\| */m', '|', $last));
  array_shift($data);
  $keys = explode('|', squeeze(array_shift($data), '|'));
  $result = array();
  foreach ($data as $line) $result[] = array_combine($keys, explode('|', squeeze($line, '|')));
  $args[count($args) - 1] = $result;
  return $args;
}

/*
 * Squeeze a string
 *
 * If the first and last char of $string is $char, shrink the string by one char at both ends.
 */
function squeeze($string, $char) {
  $first = substr($string, 0, 1);
  $last = substr($string, -1);
  return ($first == $char and $last == $char)? substr($string, 1, strlen($string) - 2) : $string;
}

/*
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