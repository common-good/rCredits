<?php
/**
 * @file
 * Specialized rCredits settings for this rCredits server
 */

define('R_REGION_NAME', 'Western Massachusetts Region');
define('R_SERVER_ID', 'NEW'); // qid header for this region's server (New England / Western MA)
// other 3rd letters: M=ME H=NH V=VT C=CT R=RI P=PA Y=NY A=NYC (apple) B=Boston
define('R_ORG_PHONE', '413-628-1723');
define('R_TESTER_EMAIL', 'wspademan@gmail.com'); // tester's personal email
define('R_ORG_EMAIL', strpos($_SERVER['HTTP_HOST'], '.') < 3 ? str_replace('@', '_cgf@', R_TESTER_EMAIL) : (strtolower(R_SERVER_ID) . '@rCredits.org'));
define('R_SITE_URL', 'https://new.rCredits.org');

// (you don't get an ID card unless you are in this geographic area)
define('R_REGION_POSTAL_AREA', '^01.*'); // anyone with a postal code that matches this regular expression is in the region
date_default_timezone_set('America/New_York'); // server handles just one timezone
define('R_COUNTRY_ID', 1228); // United States
define('R_STATE_ID', 1020); // Massachusetts
define('R_REBATE', 5);
define('R_MIN_REBATE', .05); // members get this even for a purchase with no rcredits (subject to dwindling)
define('R_BONUS_FACTOR', 2); // standard bonus is this times standard rebate
define('R_MAX_AMOUNT', 100000); // transaction request size limit (some smaller limits also apply)
define('R_SIGNUP_GIFT', 5); // how much to give people to play with, even before they are fully set up
define('R_SIGNUP_BONUS', 20); // how much people start with at this time
define('R_FLOOR', 0); // how low can user's balance go, by default
define('R_STATEMENT_COST', 10); // how much member must pay (in rCredits dollars) for monthly and annual statements by mail
define('R_DAILY_HOUR', '01'); // what time (2-digit 24-hour clock hour) to start intensive daily maintenance tasks
define('R_WEEKLY_DAY', 'Sunday'); // what day of the week to do weekly tasks
define('R_MONTHLY_DAY', '01'); // what day of the month to do monthly tasks (2-digits)
define('R_FEE_FOR_CASH', FALSE); // does customer pay the Dwolla fee when trading rCredits for cash?
define('R_DEBT_FEE', 25); // fee for overdue reserve credit
define('R_DEBT_PERCENT', 1.5); // percentage fee for overdue reserve credit
define('R_DEBT_DAYS', 30); // how long to pay back reserve credit
