<?php
/**
 * Store or retrieve personal identifying information.
 * @param string $id: key of record to replace or retrieve ('' means create new)
 * @param string $data: the data to store
 * @return string: if retrieving, return the data. if storing return the id.
 */
ini_set('display_errors',1); error_reporting(E_ALL); 
define('isDEV', @$_SERVER['WINDIR']); // developing on Windows (unlike production server)
$root = dirname($_SERVER['DOCUMENT_ROOT']);

$caller = $_SERVER['REMOTE_ADDR'];

$dbs = (array) json_decode(utf8_encode(file_get_contents("$root/.databases")));
if (!$cryptKey = @$dbs['_offsiteKeys']->$caller) die('access denied, caller '.$caller); // unique password for each caller (unknown to them)

offlog(17, $args = $_POST);

$id = @$args['id'];
$data = @$args['data'];
if (empty($id) and is_null($data)) die('server error (no data)');
try {
  exit((string) secret($dbs, $cryptKey, $id, $data));
} catch(PDOException $ex) {die('server error (query)');}

function secret($dbs, $cryptKey, $id, $data = NULL) {
  global $db;
  $maxLen = strlen('9223372036854775807') - 1; // keep it shorter than biggest usable unsigned "big int" in MySQL
  $table = 'offsite';

  extract((array) $dbs[$db_name = key($dbs)], EXTR_PREFIX_ALL, 'db');
  $db = new PDO("$db_driver:host=$db_host;port=$db_port;dbname=$db_name", $db_user, $db_pass);

  if (!isset($data)) return (offlog(35, $result = lookup('data', $table, $id)) == '' ? '' : base64_encode(ezdecrypt($result, $cryptKey))); // retrieval is easy
  
  $data = ezencrypt(base64_decode($data), $cryptKey);
  if ($id) return offlog(38, query("UPDATE $table SET data=? WHERE id=?", array($data, $id)) ? $id : FALSE); // replacing old data with new value

  while (TRUE) { // new data, find an unused id
    $id = randomInt($maxLen);
    if (!lookup(1, $table, $id)) break;
  }
  return offlog(44, query("INSERT INTO $table (id, data) VALUES (?, ?)", array($id, $data)) ? $id : FALSE);
}

function lookup($field, $table, $id) {
  $result = query("SELECT $field FROM $table WHERE id=? LIMIT 1", array($id));
  return $result ? $result[0][$field] : FALSE;
}

function query($sql, $subs = array()) {
  global $db;
  $q = $db->prepare($sql);
  $result = $q->execute($subs);
  return substr($sql, 0, 7) == 'SELECT ' ? $q->fetchAll(PDO::FETCH_ASSOC) : $result;
}

/**
 * Quick encrypt/decrypt.
 * @param string $data: what to encrypt or decrypt
 * @param string $cryptKey: a base64-encoded encryption key (maximum 24 characters, so the resultant key is at most 16 chars)
 * @param bool $encrypt: TRUE to encrypt, FALSE to decrypt
 * @return: the encrypted or decrypted data
 */
function ezencrypt($data, $cryptKey = '', $encrypt = TRUE) {
  $function = $encrypt ? 'mcrypt_encrypt' : 'mcrypt_decrypt';
  $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB); 
  $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND); 
  $data = $function(MCRYPT_RIJNDAEL_256, base64_decode(substr($cryptKey, 0, 24)), $data, MCRYPT_MODE_ECB, $iv); 
  return $encrypt ? $data : rtrim($data, "\0");
} 

function ezdecrypt($data, $cryptKey = '') {return ezencrypt($data, $cryptKey, FALSE);}

function randomInt($len = NULL) {
  $result = '';
  while (strlen($result) < $len) $result .= mt_rand(100000000, 999999999);
  return substr($result, 0, $len);
}

function offlog($line, $s) {
  global $offlog;
  $offlog = @$offlog . "\r\n $line: " . print_r($s, 1);
  file_put_contents('offlog.txt', $offlog);
  return $s;
}