<?php
/**
 * @file
 * Specialized settings
 * To be inserted in place of database settings in settings.php like this:
 * require_once __DIR__ . '/../all/modules/rcredits/boot.php';
 * Also used by /do.php, for no-sign-in database changes
 */
 
define('STAGE', 'ws.rcredits.org'); // staging site (for secrets, etc.)
define('PRODUCTION', 'new.rcredits.org'); // production site (used for setting $base_url)
define('isDEV', (bool) @$_SERVER['WINDIR']); // developing on Windows (unlike production or staging server)
define('isPRODUCTION', strtolower($_SERVER['HTTP_HOST']) == PRODUCTION);

$dbs = (array) json_decode(utf8_encode(file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . '/.databases')));
$db_name = isDEV ? 'new_rcredits' : key($dbs);
extract((array) $dbs[$db_name], EXTR_PREFIX_ALL, 'db');

define('R_WORD', hex2bin($db_word)); // hex2bin($db_word));
define('DW_API_KEY', isPRODUCTION ? $db_dwollaKey :$db_dwSandKey);
define('DW_API_SECRET', isPRODUCTION ? $db_dwollaSecret :$db_dwSandSecret);
define('R_SALTY_PASSWORD', $db_pass); // (Drupal's salt is too long for our encryption algorithm)
// $db_salt is used further below
// define('R_STAGE_WORD', @$db_stageWord); // password for staging (UNUSED)

$databases = array (
  'default' => 
  array (
    'default' => 
    array (
      'database' => $db_name,
      'username' => $db_user,
      'password' => $db_pass,
      'host' => $db_host,
      'port' => $db_port,
      'driver' => $db_driver,
      'prefix' => '',
    ),
  ),
);

ini_set('error_reporting', isDEV ? E_ALL : (E_ALL & ~E_NOTICE & ~E_DEPRECATED & ~E_STRICT));
ini_set('max_execution_time', isDEV ? 0 : 240); // don't ever timeout when developing
$conf['cron_safe_threshold'] = 0; // disable poorman's cron


$uri = 'I' . $_SERVER['REQUEST_URI']; // handle scanned rCard URIs
if (isDEV) $uri = str_replace('/devcore/', '/', $uri);
if (preg_match('~^I/[A-Z]+[\.-]~', $uri)) $_GET['q'] = $_SERVER['REQUEST_URI'] = $uri;

$drupal_hash_salt = $db_salt;
$protocol = isPRODUCTION ? 'https://' : 'http://';
$base_url = $protocol . $_SERVER['HTTP_HOST'] . (isDEV ? '/devcore' : ''); // NO trailing slash!
define('BASE_URL', $base_url);
define('R_PATH', '/sites/all/modules/rcredits');
global $rUrl; $rUrl = $base_url . R_PATH;
