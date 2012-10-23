<?php
/**
 * @file
 *
 * rSMS Setup
 *
 * Do any standard setup for rCredits SMS testing, beyond what is done in the .install scripts.
 *
 * @todo
 *   Maybe set up a few standard users and transactions here, for testing.
 */
//$caller = $this->drupalCreateUser(array('Access developer information', 'Transact', 'receive SMS', 'Transact via SMS'));
//$this->drupalLogin($caller);

use rCredits as r;
global $picturePath; 
list ($picturePath) = explode('sites', __DIR__);
$picturePath .= 'sites/default/files/pictures';

global $sms_devel; $sms_devel = TRUE;
r\db_q('DELETE FROM users WHERE uid>1000');
r\db_q('DELETE FROM r_sms');
r\db_q('DELETE FROM r_asif');
r\db_q('DELETE FROM r_txs');
//r\db_q('DELETE FROM r_smarts WHERE uid>1000');
r\db_q('DELETE FROM file_managed WHERE uid>1000');
r\db_q('DELETE FROM r_relations');

if (!function_exists('multiline_tweak')) {function multiline_tweak(&$line) {
  $line = str_replace('%last_quid', r\quid(r\acct::nextId() - 1), $line);
}}

r\acct::_clear(); // empty the acct cache
global $cuid; unset($cuid);