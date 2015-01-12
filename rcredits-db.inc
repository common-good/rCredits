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
 * @param bool $log: whether to log the operation
 * @return: if inserting, then the id of the inserted record, else the query object (FALSE if failure)
 */
function q($sql, $subs = [], $log = FALSE) {
  global $channel;
  u\EXPECT(compact(u\ray('sql subs')), 'notempty assoc');
  if (strpos($sql, 'INSERT INTO') !== FALSE) $return = \Database::RETURN_INSERT_ID;
  if ($log and !strpos($sql, 'r_log') and strpos($sql, 'SELECT ') === FALSE) u\loga('db', $subs, $sql);
  try {
    return \db_query(u\SUBS($sql), u\prefixKeys(':', $subs), compact('return'));
  } catch (Exception $e) {
/**/ if (@$channel == TX_WEB) r\Web\exception_handler($e); else die('in q');
  }
}

/**
 * Look the field up in the table, returning the first record matching the criteria.
 * @param string $fieldName: field name, function, or list of field names (* for all)
 *   A list of field names cannot contain a function or a left paren
 * @param string $table: table to look in
 * @param string $where: criteria (defaults to ALL)
 * @param assoc $subs: optional substitutions -- keys will be prefixed by ':'
 * @return: the value of what was looked up -- an assoc of values if a list of fields was requested
 *   FALSE if none
 */
function lookup($fieldName, $table, $where = '1', $subs = []) {
  u\EXPECT(compact(u\ray('fieldName table where subs')), 'notempty notempty notempty assoc');
  if (!strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $sql = "SELECT $fieldName FROM $table WHERE $where";
/// debug(compact('sql','subs'));
  $result = db\q($sql, $subs);
  $fieldCount = strpos($fieldName, '(') ? 1 : substr_count($fieldName, ',') + 1; // function or array, not both
  $several = ($fieldName == '*' or $fieldCount > 1);
  $result = $several ? $result->fetchAssoc() : $result->fetchField();
  if ($several and $fieldName != '*' and $result) u\EXPECT(\count($result) == $fieldCount, 'field count is off'); // eg, if $fieldName == 'a,a' (\count because this class has a count function!)
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
  //if ($table != 'r_log') u\loga('db', $info, "$table:$key" . ($either ? ' EITHER' : ''));
  if ($either and !db\exists($table, "$key=:$key", array($key => $info[$key]))) $key = '';
  $command = $key ? 'UPDATE' : 'INSERT INTO';
  $sql = '';
  foreach ($info as $field => $value) if ($field != $key) $sql .= ",`$field`=:$field";
  $sql = "$command $table SET " . substr($sql, 1) . ($key ? " WHERE `$key`=:$key" : '');
///  if ($table=='r_do') {print_r(compact(u\ray('table info key either command sql'))); die('dbU');}
  try {
    $result = db\q($sql, $info);
    if ($command != 'INSERT INTO') return TRUE;
    if ($result) return $result; // if there is no serial field, result will be false, so check

    if ($table == 'users') {
      $id = 'uid';
    } else {
      require_once __DIR__ . '/rcredits-install.inc';
      $schema = r\tableDefs();
      $id = $schema[$table]['primary key'][0];
      u\EXPECT($id and isset($info[$id]), 'no primary key ' . $id);
      $field1 = reset($schema[$table]['fields']);
      if ($hasSerial = ($field1['type'] == 'serial')) return FALSE; // record ID would have been returned above
    }
    
    return db\lookup($id, $table, "`$id`=:id", u\ray('id', $info[$id])); // return the supplied id if success
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
function exists($table, $where = '1', $subs = []) {
  if (is_array($where)) {
    $subs = $where;
    foreach ($where as $key => $value) $where[$key] = "$key=:$key";
    $where = join(' AND ', $where);
  }
  return db\lookup(1, $table, $where, $subs);
}

/**
 * Return an array of record objects.
 */
function records($table, $where = '1', $subs = [], $fields = '*') {
  return db\q("SELECT $fields FROM $table WHERE $where", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

/**
 * Return a list of matching data.
 */
function lst($field, $table, $where = '1', $subs = []) { // lst because "list" is a reserved word
  $ray = db\q("SELECT $field FROM $table WHERE $where", $subs)->fetchCol();
  return join(', ', $ray);
}

/**
 * Set new values in the data field of a record.
 * @param string $table: what record the table is in
 * @param string $id: the record ID
 * @param assoc $values: values to add to the data field in that record
 */
function setData($table, $id, $values) {
  $keys = u\ray('users r_txs r_invoices', 'uid', 'xid', 'nvid');
  $key = $keys[$table];
  u\EXPECT($key, 'bad table');
  $data = $values + (db\lookup('data', $table, "$key=:id", compact('id')) ?: []);
  return $table == 'users' ? r\acct($id)->update(compact('data')) : db\update($table, compact('data'));
}

function count($table, $where = '1', $subs = []) {return db\lookup('COUNT(*)', $table, $where, $subs);}

