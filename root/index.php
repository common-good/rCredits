<?php

/**
 * @file
 * The page that serves all page requests (slightly modified from Drupal).
 * All Drupal code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt.
 */
define('DRUPAL_ROOT', getcwd());
require_once __DIR__ . '/rcredits/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
menu_execute_active_handler();
