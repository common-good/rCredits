<?php
/**
 * @file
 * Set general environment and operational settings
 * Also used by /do.php, for no-sign-in database changes
 */
 
global $base_url, $base_path, $base_root;
global $databases, $drupal_hash_salt;
global $cookie_domain, $conf, $is_https, $base_secure_url, $base_insecure_url;
 
global $CGdomain;

require __DIR__ . '/defs.inc';

set_error_handler('_drupal_error_handler');
set_exception_handler('_drupal_exception_handler');
timer_start('page');

global $R_POST; $R_POST = $_POST;
$uri = 'I' . $_SERVER['REQUEST_URI']; // handle scanned rCard URIs
if (isDEV) $uri = str_replace(DEV_ROOT . '/', '/', $uri);
$_SERVER['REQUEST_URI'] = $_GET['q'] = preg_match('~^I/[A-Z]+[\.-]~', $uri) ? $uri : \request_path();
  
$secrets = (array) json_decode(utf8_encode(file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . '/.databases')));
global $db_name;
$db_name = key($secrets); // members db must be the first db name in the file
extract((array) $secrets[$db_name], EXTR_PREFIX_ALL, 'db');

define('R_WORD', hex2bin($db_word));
define('DKIM_PRIVATE', $db_dkimPrivate); // DKIM private key for email signing
define('R_INVITE_KEY', $db_inviteKey); // this value is the same for all servers (except DEV)
define('R_SSN_USER', @$db_ssnUser);
define('R_SSN_PASS', @$db_ssnPass);
define('R_SALTY_PASSWORD', $db_pass); // (Drupal's salt is too long for our encryption algorithm)
//define('POSTMARK_TOKEN', $secrets['postmarkToken']);
// $db_salt is used below

$databases = array (
  'default' => 
  array (
    'default' => 
    array (
      'database' => $db_name,
      'username' => $db_user,
      'password' => $db_pass,
      'collation' => 'utf8_general_ci', // or utf8mb4_general_ci
      'host' => $db_host,
      'port' => $db_port,
      'driver' => $db_driver,
      'prefix' => '',
      'pdo' => [PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES utf8'], // or utf8mb4
    ),
  ),
);

ini_set('magic_quotes_runtime', '0'); // Don't escape quotes when reading files from the database, disk, etc.
ini_set('session.use_cookies', '1'); // Use session cookies, not transparent sessions with the session id in the query string.
ini_set('session.use_only_cookies', '1');
ini_set('session.use_trans_sid', '0');
ini_set('session.cache_limiter', ''); // Don't send HTTP headers using PHP's session handler.
ini_set('session.cookie_httponly', '1'); // Use httponly session cookies.

ini_set('error_reporting', isDEV ? E_ALL : (E_ALL & ~E_NOTICE & ~E_DEPRECATED & ~E_STRICT));
ini_set('max_execution_time', isDEV ? 0 : 240); // don't ever timeout when developing
ini_set('session.gc_probability', 1); // garbage collection this fraction of the time on startup (numerator)
ini_set('session.gc_divisor', 100); // (denominator)
define('SESSION_LIFE', @$_COOKIE['pw2'] ? 12 * HOUR_SECS : 24 * MIN_SECS); // standard 24 mins (more for admin)
ini_set('session.gc_maxlifetime', SESSION_LIFE); // session timeout (and garbage collection happens)
ini_set('session.cookie_lifetime', 0); // SESSION_LIFE); // (drupal doesn't refresh the cookie)

if (isDEV) {
  error_reporting(E_ALL);
  ini_set('display_errors', TRUE);
  ini_set('display_startup_errors', TRUE);
}

setlocale(LC_ALL, 'C'); // Set sane locale settings, to ensure consistent string, dates, times and numbers handling.

$drupal_hash_salt = $db_salt;

$conf = array();
$conf['cron_safe_threshold'] = 0; // disable poorman's cron

$is_https = isset($_SERVER['HTTPS']) && strtolower($_SERVER['HTTPS']) == 'on';

$base_secure_url = str_replace('http://', 'https://', $base_url);
$base_insecure_url = str_replace('https://', 'http://', $base_url);

// Strip leading periods, www., and port numbers from cookie domain.
$cookie_domain = ltrim($host, '.');
if (strpos($cookie_domain, 'www.') === 0) $cookie_domain = substr($cookie_domain, 4);
$cookie_domain = explode(':', $cookie_domain);
$cookie_domain = '.' . $cookie_domain[0];

// use $base_url, without the protocol, as session name, to use the same session identifiers across HTTP and HTTPS.
//list( , $session_name) = explode('://', $base_url, 2);
$session_name = $cookie_domain;
// To prevent session cookies from being hijacked, a user can configure the
// SSL version of their website to only transfer session cookies via SSL by
// using PHP's session.cookie_secure setting. The browser will then use two
// separate session cookies for the HTTPS and HTTP versions of the site. So we
// must use different session identifiers for HTTPS and HTTP to prevent a
// cookie collision.
if ($is_https) ini_set('session.cookie_secure', TRUE);
$prefix = $is_https ? 'SSESS' : 'SESS';

session_name($prefix . substr(hash('sha256', $session_name), 0, 32));

// Per RFC 2109, cookie domains must contain at least one dot other than the
// first. For hosts such as 'localhost' or IP Addresses we don't set a cookie domain.
if (count(explode('.', $cookie_domain)) > 2 && !is_numeric(str_replace('.', '', $cookie_domain))) {
  ini_set('session.cookie_domain', $cookie_domain);
}
