<?php
/**
 * @file
 * Database functions.
 */

namespace CG\DB; // typically abbreviated as "db"
use CG as r;
use CG\DB as db;
use CG\Util as u;

require_once __DIR__ . '/cg-util.inc';

define('NO_LOGSQL_TABLES', 'r_do queue sessions r_invites r_near r_nonmembers r_request cache_form r_stats'); // don't log deletions from these files

/**
 * Do a database query with easy substitutions.
 * @param string $sql: the raw query with substitution markers preceded by ':'
 *   These can be keys supplied in $subs OR constants (all uppercase and underscores)
 * @param assoc $subs: array of substitutions to make in $sql
 *   Each key, with a colon tacked on the front, is replaced (in $sql) with its value
 * @param bool $retId: <return an inserted record ID rather than a recordset>
 * @param bool $log: <log the operation>
 * @return: if inserting, then the id of the inserted record, else the query object (FALSE if failure)
 */
function q($sql, $subs = [], $retId = FALSE, $log = FALSE) {
  global $channel;
//  \db_query("SET NAMES 'utf8'"); // only needed once per connection, but hey
  u\EXPECT(compact(ray('sql subs')), 'notempty assoc');
//  if (strpos($sql, 'INSERT INTO') !== FALSE and strpos($sql, ' VALUES (')) $return = \Database::RETURN_INSERT_ID;
  if ($retId) $return = \Database::RETURN_INSERT_ID;
  if (preg_match('/^DELETE FROM ([^ ]+)( |$)/', $sql, $match) and !in(NO_LOGSQL_TABLES, @$match)) logSql(compact(ray('sql subs retId')));
  if ($log and strpos($sql, 'SELECT ') === FALSE) u\loga('db', $subs + compact('sql'));
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
 * @param mixed $where: criteria (defaults to ALL) -- can be an assoc of required field values
 * @param assoc $subs: optional substitutions -- keys will be prefixed by ':'
 * @return: the value of what was looked up -- an assoc of values if a list of fields was requested
 *   FALSE if none
 * @see also: db\records()
 */
function get($fieldName, $table, $where = '1', $subs = []) {
  u\EXPECT(compact(ray('fieldName table where subs')), 'notempty notempty notempty assoc');
  
  if (is_array($where)) list ($where, $subs) = [u\where($where), $where];
  if (!strpos($where, ' LIMIT ')) $where .= ' LIMIT 1';
  $sql = "SELECT $fieldName FROM $table WHERE $where";
/// debug(compact('sql','subs'));
  $result = db\q($sql, $subs);
  $fieldCount = strpos($fieldName, '(') ? 1 : substr_count($fieldName, ',') + 1; // function or array, not both
  if (substr($fieldName, -1, 1) == '*') $many = TRUE; elseif ($fieldCount > 1) $several = TRUE;
  $result = (@$several or @$many) ? $result->fetchAssoc() : $result->fetchField();
  if (@$several and $result) u\EXPECT(\count($result) == $fieldCount, 'field count is off'); // eg, if $fieldName == 'a,a' (\count because this class has a count function!)
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
  logSql(compact(ray('table info key either')));
  //if ($table != 'r_log') u\loga('db', $info, "$table:$key" . ($either ? ' EITHER' : ''));
  if ($either) if (!isset($info[$key]) or !db\exists($table, "$key=:$key", [$key => $info[$key]])) $key = '';
  $command = $key ? 'UPDATE' : 'INSERT INTO';

  foreach (ray('created modified') as $k) if (!isset($info[$k]) and hasField($table, $k)) {
		if ($k == 'modified' or !$key) $info[$k] = NOW; // set these automatically
	}
	
  $sql = '';
  foreach ($info as $field => $v) {
    if ($field != $key) $sql .= ",`$field`=:$field";
    if (is_array($v)) u\EXPECT(FALSE, 'array in update param'); // exception fails if called in $DBTX
  }
  $sql = "$command $table SET " . substr($sql, 1) . ($key ? " WHERE `$key`=:$key" : '');
///  if ($table=='r_do') {print_r(compact(ray('table info key either command sql'))); die('dbU');}
  try {
    $result = db\q($sql, $info, !$key);
    if ($command != 'INSERT INTO') return TRUE;
    if ($result) return $result; // if there is no serial field, result will be false, so check

    if ($table == 'users') {
      $id = 'uid';
    } else {
      $id = db\idnm($table, $field1);
      u\EXPECT($id and isset($info[$id]), 'no primary key ' . $id);
      if ($hasSerial = ($field1['type'] == 'serial')) return FALSE; // record ID would have been returned above
    }
    
    return db\get($id, $table, "`$id`=:id", ray('id', $info[$id])); // return the supplied id if success
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
  return db\get(1, $table, $where, $subs);
}

/**
 * Return an array of record objects.
 * @param string $fields: comma-separated list of fields (actual or calculated) -- use * for all
 */
function records($fields, $table, $where = '1', $subs = []) {
  return db\q("SELECT $fields FROM $table WHERE $where", $subs)->fetchAll(); // fetchAll means fetchAllObject
}

/**
 * Return an array of matching data.
 */
function col($field, $table, $where = '1', $subs = []) {
  return db\q("SELECT $field FROM $table WHERE $where", $subs)->fetchCol();
}
function lst($field, $table, $where = '1', $subs = []) { // lst because "list" is a reserved word
  return join(', ', db\col($field, $table, $where, $subs));
}

/**
 * Log the given update, insert, or delete query
 */
function logSql($args) {
  global $mya;
  $args = date('d') . ' ' . @$mya->mainQid . ': ' . u\jsonEncode($args);
  $args = strtr($args, ['\\"' => '', '"' => '', '\\n' => '|', ';}' => '}', ';' => '; ', ',' => ', ']);
  $fileName = DRUPAL_ROOT . LOG_DIR . date('Ym') . '.logsql'; // new log file every month
  file_put_contents($fileName, "$args\n", FILE_APPEND);
  return $fileName;
}  

/**
 * Set new values in the data field of a record.
 * @param string $table: what record the table is in
 * @param string $id: the record ID
 * @param assoc $values: values to add to the data field in that record
 */
function setData($table, $id, $values) {
  $keys = ray('users r_txs r_invoices', 'uid', 'xid', 'nvid');
  $key = $keys[$table];
  u\EXPECT($key, 'bad table');
  $data = db\get('data', $table, "$key=:id", compact('id'));
  $data = $values + ($data ? unserialize($data) : []);
  if ($table == 'users') {
    return r\acct($id)->update(compact('data'));
  } else {
    $data = serialize($data);
    return db\update($table, ray("data $key", $data, $id), $key);
  }
}

/**
 * Set (or unset) a bit in the flags field of a record.
 * @param string $table: what record the table is in
 * @param string $id: the record ID
 * @param int $bit: which bit to change
 * @param bool $bool: TRUE or FALSE (how to set the bit -- defaults to TRUE)
 */
function setFlag($table, $id, $bit, $bool = TRUE) {
  if ($table == 'users') return r\acct($id)->setBit($bit, $bool);

  $keys = ray('users r_txs r_invoices', 'uid', 'xid', 'nvid');
  $key = $keys[$table];
  u\EXPECT($key, 'bad table');
  $flags = db\get('flags', $table, "$key=:id", compact('id'));
  u\setBit($flags, $bit, $bool);
  return db\update($table, ray("flags $key", $flags, $id), $key);
}

/**
 * Make a backup copy of the record before deleting it if appropriate (then delete it).
 * @param string $table: delete a record from this table
 * @param mixed $id: integer id of record to delete OR criteria
 * @param assoc $subs: parameters for selection criteria
 */
function del($table, $id, $subs = []) {
  $xtable = $table == 'users' ? 'x_users' : str_replace('r_', 'x_', $table);
  if (is_numeric($id)) {
    $idnm = db\idnm($table, $field1); // $field1 not used
    $where = "$idnm=$id";
  } else $where = $id;
  $now = time();
  if (in($table, TRACK_DELETED)) {
    $q = db\q("INSERT INTO $xtable SELECT :now AS deleted,a.* FROM $table a WHERE $where", compact('now') + $subs);
    $ok = ($q->rowCount() > 0);
  } else $ok = TRUE;
  if ($ok) db\q("DELETE FROM $table WHERE $where", $subs);
}

//function restore($table, $id) {

/**
 * Return the table's record id field name.
 * @param string $table: table name
 * @param string $field1: (RETURNED) descriptive array of first field in the table
 */
function idnm($table, &$field1 = '') {
  $scheme = scheme($table);
  $id = $table == 'users' ? 'uid' : $scheme['primary key'][0];
  $field1 = reset($scheme['fields']);
  return $id;
}
  
function scheme($table) {return f('i.scheme', $table);}
function hasField($table, $field) {return ($scheme = scheme($table) and @$scheme['fields'][$field]);}
function count($table, $where = '1', $subs = []) {return db\get('COUNT(1)', $table, $where, $subs);}
function max($field, $table, $where = '1', $subs = []) {return db\get("MAX($field)", $table, $where, $subs);}
function min($field, $table, $where = '1', $subs = []) {return db\get("MIN($field)", $table, $where, $subs);}
function sum($field, $table, $where = '1', $subs = []) {return db\get("SUM($field)", $table, $where, $subs);}