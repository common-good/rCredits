<?php
/**
 * @file
 * Tell admin about any Security Policy violations
 * (save this file in the web root)
 */

define('HOST', $_SERVER['HTTP_HOST']);
define('isDEV', HOST == 'localhost');

$json = file_get_contents('php://input');
if ($json === false) {
    throw new Exception('Bad Request');
}
$message = 'The user agent "' . $_SERVER['HTTP_USER_AGENT'] . '" '
. 'from ' . $_SERVER['REMOTE_HOST'] . ' '
. '(IP ' . $_SERVER['REMOTE_ADDR'] . ') '
. 'reported the following content security policy (CSP) violation:' . "\n\n";
						
$message .= $json . "\n\n";
						
$csp = json_decode($json, true);
if (is_null($csp)) throw new Exception('Bad JSON Violation');

foreach ($csp['csp-report'] as $key => $value) $message .= '    ' . $key . ": " . $value ."\n";

#
# Send the report
//$reported = mail( $recipient, $subject, $message, $hdrs );

if (isDEV) {file_put_contents('_CSP.txt', print_r($message, 1)); exit();}

$sender  = "info@" . HOST;
$to = 'wspademan@gmail.com';
$subject = 'CSP Report for ' . HOST;
$hdrs = 'From: ' . $sender;
mail($to, $subject, print_r($message, 1), $hdrs);