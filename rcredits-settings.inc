<?php
/**
 * @file
 * Specialized rCredits settings for this rCredits server
 */

define('R_REGION_NAME', 'Western Massachusetts Region');
define('R_SERVER_ID', 'NEW'); // qid header for this region's server (New England / Western MA)
define('CT_SERVER_ID', 'NEC'); // a temporary fake server
// other 3rd letters: M=ME H=NH V=VT C=CT R=RI P=PA Y=NY A=NYC (apple) B=Boston
define('R_ORG_PHONE', '413-628-1723');
define('R_ORG_EMAIL', 'info@rCredits.org');
define('R_SITE_URL', 'http://new.rCredits.org');

// (you don't get an ID card unless you are in this geographic area)
define('R_REGION_POSTAL_AREA', '^01.*'); // anyone with a postal code that matches this regular expression is in the region
date_default_timezone_set('America/New_York'); // server handles just one timezone
define('R_COUNTRY_ID', 1228); // United States
define('R_STATE_ID', 1020); // Massachusetts
define('R_INITIAL_DEMAND', 25000); // total escrowed rCredits during TRIAL RUN
define('R_REBATE', 0.05);
define('R_BONUS', 0.10);
define('R_GETR_ADVANCE', 0.10); // fraction of get rCredits request we take as advance
define('R_MAX_AMOUNT', 100000); // you can't ask for more rCredits than this
define('R_MIN_DEMAND', 10); // you can't buy less rCredits than this at a time
define('R_STARTING_BALANCE', 250); // how much people start with in the TRIAL RUN
define('R_SIGNUP_INCENTIVES', '20 100 150 200 250'); // possible total amounts of signup incentives in launch month
define('R_MIN_BALANCE', 0); // how low can user's balance go, by default
define('R_STATEMENT_COST', 10); // how much member must pay (in rCredits dollars) for monthly and annual statements by mail
