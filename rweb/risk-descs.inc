<?php
use CG\Util as u;
/**
 * @file
 * Set up the risk descriptions for HTML hover titles
 */
global $riskHelp; $riskHelp = array(
  'adminOk' => t('A %PROJECT community administrator says this account is low risk (flag set by administrator)'),
  'trusted' => t('Has access to, was invited by, or is proxy to, other account(s) in good standing'),
  'socialConx' => t('Has 1st or 2nd-degree social media connections to a %PROJECT community administrator'),
  'hasBank' => t('Has a connected bank account'),
  'geography' => t('Resides in a High Intensity Drug Trafficking Area'),
  'cashCo' => t('Is a type of business that has been identified by regulators as high risk (flag set by administrator)'),
  'new' => t('Recently moved to current address (before opening a %PROJECT account)'),
  'moves' => t('Has changed counties several times in the past decade'),
  'rents' => t('Rents (rather than owns) a residence'),
  'badConx' => t('Has access to, or was invited by, other suspicious account(s)'),
  'homeCo' => t('An individual account has the same postal address as this company account'),
  'shady' => t('A web search for this individual or company produces sketchy results (flag set by administrator)'),
  'miser' => t('Contributed an unusually small amount to support administration and promotion of %PROJECT'),
  'photoOff' => t('rCard photo differs photo ID or birthdate (flag set by administrator)'),
  'addrOff' => t('Address does not match address on photo ID (flag set by administrator)'),
  'ssnOff' => t('Something is awry in the account\'s social security number lookup (flag set by administrator)'),
  'dobOff' => t('Member\'s birth date could not be confirmed or is at odds with social security number lookup data'),
  'poBox' => t('Postal address is a post box'),
  'fishy' => t('Something just seems a little off (flag set by administrator on intuition)'),
  'moreIn' => t('Recipient has received %K_UNUSUAL_BUMP% more than usual over the past seven weeks'),
  'moreOut' => t('Payer has spent %K_UNUSUAL_BUMP% more than usual over the past seven weeks'),
  'bigDay' => t('Has an unusually large transaction volume (over $%K_DAY_THRESHOLD per employee) over the past day'),
  'bigWeek' => t('Has an unusually large transaction volume (over $%K_WEEK_THRESHOLD per employee) over the past week'),
  'big7Week' => t('Has an unusually large transaction volume (over $%K_7WEEK_THRESHOLD per employee) over the past 52 days (about 7 weeks)'),
  'bigYear' => t('Has an unusually large transaction volume (over $%K_YEAR_THRESHOLD per employee) over the past year'),
  'txAdminOk' => t('A %PROJECT community administrator says this transaction is low risk'),
  'redo' => t('The transaction reverses another transaction that itself reversed a transaction.'),
  'exchange' => t('A member trades cash for another member\'s %RCREDITS'),
  'cashIn' => t('A member trades cash for a company\'s %RCREDITS'),
  'cashOut' => t('A member trades %RCREDITS for a company\'s cash'),
  'fromBank' => t('Transfers funds from bank account to %PROJECT account'),
  'toBank' => t('Transfers funds from %PROJECT account to bank account'),
  'b2p' => t('A company pays an individual who is not an employee'),
  'p2p' => t('An individual member pays another individual member'),
  'inhouse' => t('An employee, owner, or consultant pays the company'),
  'fromSuspect' => t('A suspicious account pays out'),
  'toSuspect' => t('A suspicious account receives funds'),
  'absent' => t('Transaction by web browser or text message (not in-person)'),
  'invoiceless' => t('Company pays a supplier or consultant without mention of an invoice'),
  'bigFrom' => t('An unusually large expenditure by this payer (in the top 10% over the past seven weeks)'),
  'biggestFrom' => t('The largest single expenditure by this payer over the past seven weeks'),
  'oftenFrom' => t('Recipient has received unusually many payments from this payer over the past seven weeks'),
  'bigTo' => t('An unusually large payment to this recipient (in the top 10% over the past seven weeks)'),
  'biggestTo' => t('The largest single payment to this recipient over the past seven weeks'),
  'oftenTo' => t('Payer has made unusually many payments to this recipient over the past seven weeks'),
  'offline' => t('Payment was taken offline (without an internet connection)'),
  'firstOffline' => t('Payment was taken offline, with no rCard photo verification'),
  'origins' => t('Receipts by this payer over the past year are suspicious in aggregate'),
  'suspectOut' => t('A suspicious account trades %RCREDITS for US Dollars'),
);
/*
$helpDescs = join('|', array_values($riskHelp));
$js = <<<EOF
var a = "$helpDescs".split('|');
for (i = 0; i < a.length; i++) jQuery('.riskHelp' + i).prop('title', a[i]);
EOF;
\rCredits\Web\js($js, 'inline', 'footer');
*/