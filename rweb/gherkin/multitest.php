<?php
$SHOWERRORS = true;
error_reporting($SHOWERRORS ? E_ALL : 0);
ini_set('display_errors', $SHOWERRORS);
ini_set('display_startup_errors', $SHOWERRORS);

$statement = 'phone -2.8 has $-26,432,518.90, no "whatever" name "DATA'
    . PHP_EOL . '| field1 | field2   | field3 |'
    . PHP_EOL . '|  1     | 2        | 3      |'
    . PHP_EOL . '|  a     | b        | c      |'
    . '"';

  $arg_patterns = '"(.*?)"|(-?[0-9]+(?:[\.,-][0-9]+)*)';
  preg_match_all("/$arg_patterns/ms", $statement, $matches);
  $args = multiline_check($matches[0]); // php bug: $matches[1] has null for numeric args
 print_r($args);

function multiline_check($args) {
  for($i = 0; $i < count($args); $i++) $args[$i] = squeeze($args[$i], '"');
  if (substr($last = end($args), 0, 4) != 'DATA') return $args;

  $data = explode(PHP_EOL, preg_replace('/ *\| */m', '|', $last));
  array_shift($data);
  $keys = explode('|', squeeze(array_shift($data), '|'));
  $result = array();
  foreach ($data as $line) $result[] = array_combine($keys, explode('|', squeeze($line, '|')));
  $args[count($args) - 1] = $result;
  return $args;
}

// if first char of $string is $char1, shrink it by 1 char at both ends
function squeeze($string, $char) {
  $first = substr($string, 0, 1);
  $last = substr($string, -1);
  return ($first == $char and $last == $char)? substr($string, 1, strlen($string) - 2) : $string;
}