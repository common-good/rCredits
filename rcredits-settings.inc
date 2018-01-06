<?php
/**
 * @file
 * Specialized settings for this rCredits server
 */

define('R_SERVER_ID', 'NEW'); // qid header for this region's server (New England / Western MA)
// other 3rd letters: M=ME H=NH V=VT C=CT R=RI P=PA B=Boston
define('ADMIN_NAME', 'William Spademan');
define('SIGNATORY', 'William Spademan, President');
define('R_ADMIN_EMAIL', 'wspademan@gmail.com'); // regional admin's personal email
//define('R_ADMIN_ADDRESS', 'PO Box 305, Ashfield, MA 01330');
define('R_ADMIN_PHONE', '+14136281021');
define('R_REGION_EIN', '46-1821792');

date_default_timezone_set('America/New_York'); // server handles just one timezone
define('R_COUNTRY_ID', US_COUNTRY_ID); // this region's country: United States
define('R_STATE_ID', 1020); // Massachusetts
define('R_REBATE', 10); // standard initial rebate percentage
define('R_BONUS_FACTOR', 1); // standard bonus is this times standard rebate
define('R_MAX_REBATE', 500); // cap on rebate for a single transaction (max bonus is in proportion)
define('R_MIN_REWARD_PCT', 1); // minimum reward percentage
define('R_REWARD_STEP', 100); // how much rewards can the member get before their reward percentage goes down by 1%
define('R_XFEE_CARD', .03); // percentage fee for exchanges of USD for Common Good Credits using a credit/debit card
define('R_XFEE_CHECK', 0); // flat fee for exchanges of USD for Common Good Credits with a check (was 3.00)
define('R_MAX_AMOUNT', 100000); // transaction request size limit (some smaller limits also apply)
define('R_SIGNUP_BONUS', 20); // how much people start with at this time
define('R_HELPER_BONUS', 20); // how much you get for inviting and assisting someone with account opening
define('R_COUNTED_BONUS', 10); // UNUSED how much you get for each member employee of a company you invite/assist
define('R_FLOOR', 0); // how low can user's combined balance go, by default
define('MIN_WEEKLY_BUMP', 1); // automatic weekly increase in negative minimum balance
define('R_DAILY_HOUR', isDEV ? '00' : '01'); // what time (2-digit 24-hour clock hour) to start intensive daily maintenance tasks
define('R_WEEKLY_DAY', 'Sunday'); // what day of the week to do weekly tasks
define('R_MONTHLY_DAY', '01'); // what day of the month to do monthly tasks (2-digits)
define('R_DEBT_FEE', 25); // fee for overdue reserve credit
define('R_DEBT_PERCENT', 1.5); // percentage fee for overdue reserve credit
define('R_DEBT_DAYS', 30); // how long to pay back reserve credit
define('R_STATEMENT_COST', 10); // how much member must pay (in rCredits dollars) for monthly and annual statements by mail
define('R_INFLATION_RATE', .05); // annual USD inflation rate (but never less than zero) (extra big is okay)
//define('R_ADMIN_QID', 'NEW.AAA'); // William's account code
define('R_USDTX_DAYS', 5); // how many days it takes for a transfer to clear from bank to Common Good account
define('R_USDTX_QUICK', 200); // maximum amount a member can transfer overnight (instead of waiting USDTX days)

define('BANK_NAME', 'Citizens Bank');
define('BANK_ADDR', 'Greenfield Branch<br>89 French King Highway<br>Greenfield, MA 01301<br>+1 413 774 2135');
define('ESCROW_ROUTING', '211070175'); // escrow account routing number
define('ESCROW_IN', '1326532860'); // escrow account number for checks from bank to escrow
define('ESCROW_OUT', '1326532739'); // escrow account number for outgoing checks (no longer used, since we do ACHs)
