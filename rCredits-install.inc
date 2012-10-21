<?php

namespace rCredits;
use rCredits\Util as u;

/**
 * @file
 * Extra (non-hook) functions for rCredits.install
 */

/**
 * Make Fields
 *
 * Make (or unmake) database fields, according to the schema
 * @param boolean $unmake: whether to unmake, rather than make
 * @param array $extras: associative array of table_name (only for unmaking)
 */
function make_fields($unmake = FALSE, $extras = array()) {
  $schema = array();
  \rCredits_schema_alter($schema); // get the fields to add (or drop)
  foreach ($schema as $table_name => $one) {
    if ($unmake and isset($extras[$table_name])) $one += $extras[$table_name];
    foreach ($one['fields'] as $field_name => $spec) {
      if (\db_field_exists($table_name, $field_name)) {
        if ($unmake) \db_drop_field($table_name, $field_name);
        if (!$unmake) \db_change_field($table_name, $field_name, $field_name, $spec);
      } else {
        if (!$unmake) \db_add_field($table_name, $field_name, $spec);
      }
    }
  }
}

function setup_table($description, $fields, $primary_key, $foreign_keys = array(), $indexes = array()) {
  $result = compact('description', 'fields', 'indexes') + array('foreign keys' => $foreign_keys);
  return is_null($primary_key) ? $result : ($result + array('primary key' => (is_array($primary_key) ? $primary_key : array($primary_key))));
}

function foreign_key($local, $foreign = 'uid', $table = 'users') {
  $columns = array($local => $foreign);
  return array($local => compact('table', 'columns'));
}

function index($field) {return array($field => array($field));}

/**
 * Setup a schema field
 *
 * @param string $type: data_type and size, separated by a space (a numeric size is interpreted as a string's maximum length)
 * @param string of boolean $SUN: $serialize, $unsigned, $notnull (one binary digit for each)
 * @param string $precision: precision and scale, separated by a comma
 *
 * @return array: the field's schema (UNUSED?)
 */
function setup_field($description = '', $type = 'varchar 255', $default = '', $SUN = '000') {
  $size = '';
  $precision = ',';

  if (strpos($type, ' ')) list ($type, $size) = explode(' ', $type);
  if ($type == 'numeric') {
    $precision = $size;
    $size = '';
  } elseif (is_numeric($size)) {
    $length = $size;
    $size = '';
  }
  list ($serialize, $unsigned, $notnull) = str_split($SUN);
  list ($precision, $scale) = explode(',', $precision); // don't use u\ray here
  
  $scheme = compact(u\ray('description type size length precision default scale serialize unsigned'));
  foreach ($scheme as $key => $value) if ($value === '') unset($scheme[$key]);

  $scheme['not null'] = $notnull;
//  ['fields'][$field_name] = $scheme;
  return $scheme;
}
