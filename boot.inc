<?php
/**
 * @file
 * Set general environment and operational settings
 * Also used by /do.php, for no-sign-in database changes
 */

strip($_POST); strip($_GET); // No HTML input EVER (unless encoded in a way we expect)
 
global $base_url, $base_path, $base_root, $styleNonce;
global $databases, $drupal_hash_salt, $cryHows;
global $cookie_domain, $conf, $is_https, $base_secure_url, $base_insecure_url;
 global $CGdomain;

require __DIR__ . '/defs.inc';

set_error_handler('_drupal_error_handler');
timer_start('page');
if (NOT_PRODUCTION) {
  error_reporting(E_ALL);
  ini_set('display_errors', 'On');
}

$uri = 'I' . $_SERVER['REQUEST_URI']; // handle scanned rCard URIs
if (isDEV) $uri = str_replace(DEV_ROOT . '/', '/', $uri);
$_SERVER['REQUEST_URI'] = $_GET['q'] = preg_match('~^I/[A-Z]+[\.-]~', $uri) ? $uri : \request_path();
  
$secrets = (array) json_decode(utf8_encode(file_get_contents(dirname($_SERVER['DOCUMENT_ROOT']) . '/cg.cnf')));
global $db_name;
$db_name = key($secrets); // members db must be the first db name in the file
extract((array) $secrets[$db_name], EXTR_PREFIX_ALL, 'db');

global $cryHows; $cryHows = (array) $db_encryption;
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
define('SESSION_LIFE', (@$_COOKIE['pkv'] or @$_POST['pkv']) ? 12 * HOUR_SECS : 24 * MIN_SECS); // standard 24 mins (more for admin)
ini_set('session.gc_maxlifetime', SESSION_LIFE); // session timeout (and garbage collection happens)
ini_set('session.cookie_lifetime', 0); // SESSION_LIFE); // (drupal doesn't refresh the cookie)

if (isDEV or NOT_PRODUCTION) {
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

$agt = $_SERVER['HTTP_USER_AGENT'];
define('isSAFARI', stripos($agt, 'Chrome') === FALSE and stripos($agt, 'Safari') !== FALSE);

/**
 * Remove any (potentially hazardous) HTML from input.
 * Specifically, remove <, followed by ! or any letter, through the closing > or end of string.
 */
function strip(&$v) {
  if (is_array($v)) {
    foreach ($v as $k => $v2) strip($v[$k]);
  } else $v = preg_replace('~<[!/a-z][^>]*(>|$)~i', '', $v); // not strip_tags, which zaps <3, etc.
}

define('FRAME_SOURCES', "https://cg4.us/chart.php $base_url/rcredits/misc/chart.php $base_url/sadmin/export-list/"); // base_url not self, because we use this to sanitize page too

/** Set Content-Security-Policy
 * We might want to use require-sri-for eventually for even our own scripts and style sheets
 * 'sha256-47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=' for inline style in spin.min.js
 * 'sha256-2EA12+9d+s6rrc0rkdIjfmjbh6p2o0ZSXs4wbZuk/tA=' for inline style in jquery-3.1.1.min.js
 * 'sha256-nX7L7GcvxmPKgeYAFQSzFcLplU7F/i/72ytdtLdJYoI=' for jquery also
 * 'sha256-+t47tgldASBy5zANml11C5vRg3a/7oPrKstKXlzZfkI=' for jquery also
 * 'sha256-+7g9GIVwIQyRW5AWmV3tOknRu/VejUoNtGLu4+COYXU=' for inline style in chartist.min.js
 * nonces are for spin.min.js on iPhone (but it doesn't work)
 */
function setCSP() {
  $styleNonce = base64_encode(substr(hash('sha256', PROJECT . time(), TRUE), 0, 32));
  $sid = session_id();

  \drupal_add_http_header('Content-Security-Policy', 
      "base-uri 'self';"
    . "block-all-mixed-content;"
    . "default-src 'self';"
          . "connect-src 'self';"
          . "font-src 'self';"
          . "frame-src " . FRAME_SOURCES . ';' // we use iframes for admin member download and for graphs
          . "manifest-src 'self';"
          . "media-src 'self';"
          . "object-src 'none';"
          . "script-src 'self' 'nonce-$styleNonce' 'report-sample';" // report-sample allows script fragment to be included in CSP reports
          . "img-src 'self' data:;" // data: is for photo upload
          . "style-src 'self' 'nonce-$styleNonce' 'sha256-47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=' 'sha256-+t47tgldASBy5zANml11C5vRg3a/7oPrKstKXlzZfkI=' 'sha256-nX7L7GcvxmPKgeYAFQSzFcLplU7F/i/72ytdtLdJYoI=' 'sha256-2EA12+9d+s6rrc0rkdIjfmjbh6p2o0ZSXs4wbZuk/tA=';"
          . "worker-src 'none';"
    . "form-action 'self';"
    . "frame-ancestors 'none';"
    . "report-uri /csp-report.php?sid=$sid;"
  );

  \drupal_add_http_header('X-Frame-Options', 'DENY');
  // Use <iframe sandbox="allow-scripts" ...> for cg4.us frame showing charts

}
