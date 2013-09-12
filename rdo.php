<?php
/**
 * @file
 * Does specific one-time actions without requiring a user to log in.
 */

use rCredits as r;
use rCredits\Util as u;

define('DRUPAL_ROOT', __DIR__);
include_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

if (!@$code = $_SERVER['QUERY_STRING']) exit();
if (!$action = r\dbLookup('action', 'r_do', 'code=:code', compact('code'))) exit();

