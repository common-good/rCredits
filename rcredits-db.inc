<?php
/**
 * @file
 * Database functions.
 */

namespace rCredits\DB; // typically abbreviated as "db"
use rCredits as r;
use rCredits\DB as db;
use rCredits\Util as u;

/**
 * Do a database query with easy substitutions.
 * @param string $sql: the raw query with substitution markers preceded by ':'
 *   These can be keys supplied in $subs OR constants (all uppercase and underscores)
 * @param assoc $subs: array of substitutions to make in $sql
 *   Each key, with a colon tacked on the front, is replaced (in $sql) with its value
 * @return: if inserting, then the id of the inserted record, else the query object (FALSE if failure)
 */
function q($sql, $subs = array(), $log = TRUE) {
  u\EXPECT(compact(u\ray('sql subs')), 'notempty assoc');
  if (strpos($sql, 'INSERT INTO') !== FALSE) $return = \Database::RETURN_INSERT_ID;
  if ($log and !strpos($sql, 'r_log') and strpos($sql, 'SELECT ') === FALSE) u\loga('db', $subs, $sql);
  try {
    return \db_query(u\SUBS($sql), u\prefixKeys(':', $subs), compact('return'));
/**/ } catch (Exception $e) {die('here');}
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @param string $fieldName: field name, function, or list of field names
 *   A list of field names cannot contain a function or a left paren
 * @param string $table: table to look in
 * @param string $where: criteria (defaults to ALL)
 * @param assoc $subs: optional substitutions -- keys will be prefixed by ':'
 * @return: the value of what was looked up -- an assoc of values if a list of fields was requested
 *   FALSE if none
 */
function lookup($fieldName, $table, $where = '1', $subs = array()) {
  u\EXPECT(compact(u\ray('fieldName table where subs')), 'notempty notempty notempty assoc');
  if (!strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $sql = "SELECT $fieldName FROM $table WHERE $where";
///  if (strpos($table, 'nonce')) debug(compact('sql','subs'));
  $result = db\q($sql, $subs);
  $fieldCount = strpos($fieldName, '(') ? 1 : substr_count($fieldName, ',') + 1; // function or array, not both
  $result = $fieldCount > 1 ? $result->fetchAssoc() : $result->fetchField();
  u\EXPECT($fieldCount == 1 or !$result or count($result) == $fieldCount, 'field count is off'); // eg, if $fieldName == 'a,a'
  return $result;
}

/**
 * Update a database record 
 * (replaces the functionality of drupal_write_record, which does not update bigint fields properly)
 * @param string $table: table name
 * @param assoc $info: data to update (must include record key, unless inserting)
 * @param string $key: record key name (if empty then insert record, otherwise update)
 * @param bool either: if TRUE, insert the record if it does not exist (otherwise update)
 * @return: if inserting, then the id of the inserted record, else TRUE (FALSE on failure)
 */
function update($table, $info, $key, $either = FALSE) {
  if ($table != 'r_log') u\loga('db', $info, "$table:$key" . ($either ? ' EITHER' : ''));
  if ($either and !db\exists($table, "$key=:$key", array($key => $info[$key]))) $key = '';
  $command = $key ? 'UPDATE' : 'INSERT INTO';
  $sql = '';
  foreach ($info as $field => $value) if ($field != $key) $sql .= ",`$field`=:$field";
  $sql = "$command $table SET " . substr($sql, 1) . ($key ? " WHERE `$key`=:$key" : '');
///  if ($table=='r_do') {print_r(compact(u\ray('table info key either command sql'))); die('dbU');}
  try {
    $result = db\q($sql, $info);
    if ($command == 'INSERT INTO') { // if there is no serial field, result will be false, so check
      if ($result) return $result; // must have had a serial field -- return its value
      require_once __DIR__ . '/rcredits-install.inc';
      $schema = r\tableDefs();
      $id = $table == 'users' ? 'uid' : $schema[$table]['primary key'][0];
      u\EXPECT($id and isset($info[$id]), 'no primary key ' . $id);
      $field1 = reset($schema[$table]['fields']);
      if ($hasSerial = ($field1['type'] == 'serial')) return FALSE;
      return db\lookup($id, $table, "`$id`=:$id", array($id => $info[$id])); // return the supplied id if success
    } else return TRUE;
  } catch (PDOException $e) {u\EXPECT(FALSE, "PDOException: $e->message");} 
}

/**
 * Insert a database record (just one)
 * (replaces half the functionality of drupal_write_record)
 * @param string $table: table name
 * @param assoc $info: data to insert
 * @return: record key of the inserted record (FALSE on failure)
 */
function insert($table, $info) {return db\update($table, $info, '');}

/**
 * Say whether the described record exists.
 * call by dbExists($table, $where, $subs)
 *      or dbExists($table, $subs) where $subs contains the required field names and their values
 */
function exists($table, $where = '1', $subs = array()) {
  if (is_array($where)) {
    $subs = $where;
    foreach ($where as $key => $value) $where[$key] = "$key=:$key";
    $where = join(' AND ', $where);
  }
  return db\lookup(1, $table, $where, $subs);
}
