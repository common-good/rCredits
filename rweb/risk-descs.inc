<?php
use rCredits\Util as u;
/**
 * @file
 * Set up the risk descriptions for HTML hover titles
 */
global $riskHelp; $riskHelp = array(
  'adminOk' => t('An rCredits community administrator says this account is low risk'),
  'trusted' => t('Has access to, was invited by, or is proxy to, other account(s) in good standing'),
  'socialConx' => t('Has 1st or 2nd-degree social media connections to an rCredits community administrator'),
  'hasBank' => t('Has a connected bank account'),
  'geography' => t('Resides in an HIDTA'),
  'cashCo' => t('Is a type of business that has been identified by regulators as high risk'),
  'new' => t('Recently moved to current address (before opening an rCredits account)'),
  'moves' => t('Has changed counties several times in the past decade'),
  'rents' => t('Rents (rather than owns) a residence'),
  'badConx' => t('Has access to, or was invited by, other suspicious account(s)'),
  'homeCo' => t('An individual account has the same postal address'),
  'shady' => t('A web search for this individual or company produces sketchy results'),
  'miser' => t('Contributed an unusually small amount to support administration and promotion of rCredits'),
  'photoOff' => t('rCard photo differs photo ID or birthdate'),
  'addrOff' => t('Address does not match address on photo ID'),
  'ssnOff' => t('Something is akimbo in social security number history records'),
  'poBox' => t('Postal address is a post box'),
  'fishy' => t('Something just seems a little off'),
  'moreIn' => u\tt('Recipient has received @K_UNUSUAL_BUMP% more than usual over the past seven weeks'),
  'moreOut' => u\tt('Payer has spent @K_UNUSUAL_BUMP% more than usual over the past seven weeks'),
  'bigDay' => u\tt('Has an unusually large transaction volume (over $@K_DAY_THRESHOLD per employee) over the past day'),
  'bigWeek' => u\tt('Has an unusually large transaction volume (over $@K_WEEK_THRESHOLD per employee) over the past week'),
  'big7Week' => u\tt('Has an unusually large transaction volume (over $@K_7WEEK_THRESHOLD per employee) over the past 52 days (about 7 weeks)'),
  'bigYear' => u\tt('Has an unusually large transaction volume (over $@K_YEAR_THRESHOLD per employee) over the past year'),
  'txAdminOk' => t('An rCredits community administrator says this transaction is low risk'),
  'redo' => t('The transaction reverses another transaction that itself reversed a transaction.'),
  'exchange' => t('A member exchanges cash for another member\'s rCredits'),
  'cashIn' => t('A member trades cash for a company\'s rCredits'),
  'cashOut' => t('A member trades rCredits for a company\'s cash'),
  'fromBank' => t('Transfers funds from bank account to rCredits account'),
  'toBank' => t('Transfers funds from rCredits account to bank account'),
  'b2p' => t('A company pays an individual who is not an employee'),
  'p2p' => t('An individual member pays another individual member'),
  'inhouse' => t('An employee, owner, or consultant pays the company'),
  'fromSuspect' => t('A suspicious account pays out'),
  'toSuspect' => t('A suspicious account receives funds'),
  'absent' => t('Transaction by web browser or text message (not in-person)'),
  'invoiceless' => t('Company pays a supplier or consultant without an invoice'),
  'bigFrom' => t('An unusually large expenditure by this payer (in the top 10% over the past seven weeks)'),
  'biggestFrom' => t('The largest single expenditure by this payer over the past seven weeks'),
  'oftenFrom' => t('Recipient has received unusually many payments from this payer over the past seven weeks'),
  'bigTo' => t('An unusually large payment to this recipient (in the top 10% over the past seven weeks)'),
  'biggestTo' => t('The largest single payment to this recipient over the past seven weeks'),
  'oftenTo' => t('Payer has made unusually many payments to this recipient over the past seven weeks'),
  'offline' => t('Payment was taken offline (without an internet connection)'),
  'firstOffline' => t('Payment was taken offline, with no rCard photo verification'),
  'origins' => t('Receipts by this payer over the past year are suspicious in aggregate'),
  'suspectOut' => t('A suspicious account exchanges rCredits for US Dollars'),
);
$helpDescs = join('|', array_values($riskHelp));
$js = <<<EOF
var a = "$helpDescs".split('|');
for (i = 0; i < a.length; i++) jQuery('.riskHelp' + i).prop('title', a[i]);
EOF;
\rCredits\Web\js($js, 'inline', 'footer');