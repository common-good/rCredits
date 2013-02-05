<?php
/**
 * Store or retrieve personal identifying information.
 * @param string $id: key of record to replace or retrieve ('' means create new)
 * @param string $data: the data to store
 * @return string: if retrieving, return the data. if storing return the id.
 */

define('isDEV', @$_SERVER['WINDIR']); // developing on Windows (unlike production server)
$caller = $_SERVER['REMOTE_ADDR'];
if ($caller != (isDEV ? '::1' : '168.144.87.128')) die('access denied');

$args = $_GET;
$id = @$args['id'];
$data = @$args['data'];
if (empty($id) and is_null($data)) die('server error (no data)');
try {
  exit((string) secret($id, $data));
} catch(PDOException $ex) {die('server error (query)');}

function secret($id, $data = NULL) {
  global $db, $caller;
  $offsite = file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . "/.offsite");
  extract((array) unserialize($offsite)); // list ($word, $dsn, $dbuser, $dbpass)
  $maxLen = strlen('9223372036854775807') - 1; // keep it shorter than biggest usable unsigned "big int" in MySQL
  $table = 'offsite';
  $key = $word . $caller;

  $db = new PDO($dsn, $dbuser, $dbpass);
  if (!isset($data)) return ezdecrypt(lookup('data', $table, $id), $key); // retrieval is easy
  
  $data = ezencrypt($data, $key);
  if ($id) return query("UPDATE $table SET data=? WHERE id=?", array($data, $id)) ? $id : FALSE; // replacing old data with new value

  while (TRUE) { // new data, find an unused id
    $id = randomInt($maxLen);
    if (!lookup(1, $table, $id)) break;
  }
  return query("INSERT INTO $table (id, data) VALUES (?, ?)", array($id, $data)) ? $id : FALSE;
}

function lookup($field, $table, $id) {
  $result = query("SELECT $field FROM $table WHERE id=? LIMIT 1", array($id));
  return $result ? $result[0][$field] : FALSE;
}

function query($sql, $subs = array()) {
  global $db;
  $q = $db->prepare($sql);
  $q->execute($subs);
  return substr($sql, 0, 7) == 'SELECT ' ? $q->fetchAll(PDO::FETCH_ASSOC) : $q->rowCount();
}

/**
 * Quick encrypt/decrypt.
 */
function ezencrypt($data, $key = '', $encrypt = TRUE) {
  $function = $encrypt ? 'mcrypt_encrypt' : 'mcrypt_decrypt';
  $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB); 
  $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND); 
  $data = $function(MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv); 
  return $encrypt ? $data : rtrim($data, "\0");
} 

function ezdecrypt($data, $key = '') {return ezencrypt($data, $key, FALSE);}

function randomInt($len = NULL) {
  $result = '';
  while (strlen($result) < $len) $result .= mt_rand(100000000, 999999999);
  return substr($result, 0, $len);
}
