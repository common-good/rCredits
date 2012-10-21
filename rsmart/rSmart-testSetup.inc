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

global $sms_devel; $sms_devel = TRUE;
r\db_q('DELETE FROM users WHERE uid>1000');
r\db_q('DELETE FROM r_sms WHERE uid>1000');
r\db_q('DELETE FROM r_asif WHERE payer>1000 or payee>1000');
r\db_q('DELETE FROM r_txs WHERE payer>1000 or payee>1000');
r\db_q('DELETE FROM r_relations WHERE main_uid>1000');

if (!function_exists('multiline_tweak')) {function multiline_tweak(&$line) {
  $line = str_replace('%last_quid', r\quid(r\acct::nextId() - 1), $line);
}}

r\acct::_clear(); // empty the acct cache
global $cuid; unset($cuid);