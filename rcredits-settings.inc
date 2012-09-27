<?php
/**
 * @file
 * Secret rCredits Settings for this rCredits server
 */

define('R_REGION_NAME', 'Western Massachusetts rCredits Region');
define('R_SERVER_ID', 'NEW'); // qid header for this region's server (New England / Western MA)
// other 3rd letters: M=ME H=NH V=VT C=CT R=RI P=PA Y=NY A=NYC (apple) B=Boston

// (you don't get an ID card unless you are in this geographic area)
define('R_REGION_POSTAL_AREA', '01%'); // anyone with a postal code that matches this is in
define('R_INITIAL_DEMAND', 25000); // total escrowed rCredits during TRIAL RUN
define('R_REBATE', 0.05);
define('R_BONUS', 0.10);
define('R_MAX_DEMAND', 1000000); // you can't ask for more rCredits than this
define('R_MIN_DEMAND', 10); // you can't buy less rCredits than this at a time
define('R_STARTING_BALANCE', 250); // how much people start with in the TRIAL RUN
define('R_MIN_BALANCE', -1000); // how low can user's balance go, by default (be generous in TRIAL RUN)
define('R_FIREWALL', 'lkj234lisufsd89072osi%!@J@$K'); // nobody can use the backend without knowing this code

