<?php
/**
 * @file
 * Set help for flag bits.
 */

$flagHelp = array(
  'member' => t('All account opening steps have been completed.'),
  'confirmed' => t('Whoever invited the member has confirmed the invitation OR member bought something locally in-person.'),
  'ok' => t('Account is active -- approved for transactions.'),
//  'bona' => t('Member has received a signup bonus.'),
  'carded' => t('A %PROJECT card and welcome letter has been mailed to the member.'),
  'ided' => t('A cashier has verified the member\'s government-issued photo ID.'),
  'refill' => t('The member refills the account automatically from their bank account.'),
  'co' => t('The account is a company, organization, club, or fund -- not a person.'),
  'roundup' => t('Round payments up to nearest dollar, donate the cents to the community fund and the common good.'),
  'nonudge' => t('Don\'t send reminder emails.'),
  'nosearch' => t('User wants to be not findable by other members, by phone or email.'),
  'payroll' => t('NYI Automatically pay employees their total daily salary - employer chooses pay period.'),
  'weekly' => t('Send notices only weekly.'),
  'monthly' => t('Send notices only monthly.'),
  'bysms' => t('NYI Send notifications by SMS.'),
  'paper' => t('Will not accept electronic statements.'),
  'secret' => t('Don\'t let merchants tell the member their balance on request.'),
  'debt' => t('Member chose to permit sub-zero balances.'),
  'draws' => t('This account automatically draws funds from one or more others.'),
  'savings' => t('NYI The connected bank account is a savings or money market account.'),
  'cashoutW' => t('Cash this account out weekly, leaving only its average weekly gross in the account.'),
  'cashoutM' => t('Cash this account out monthly, leaving only its average weekly gross in the account.'),
  'joined' => t('The account is a joint account.'),
//  'closed' => t('The account is closed (the newAccount field has the new account ID, if any).'),
  'beta' => t('The account is a beta tester.'),
  'icAdmin' => t('The account is an Investment Club administrator.'),
  'admin2' => t('The account is an overall assistant administrator or bank partner or government regulator (the Admin2 has read-only permission on all accounts, except with permission to add notes).'),
  'cAdmin2' => t('The account is a community sub-administrator (the cAdmin2 has read-only permission on all accounts, except with permission to add notes).'),
  'cAdmin' => t('The account is a community administrator.'),
  'admin' => t('The account is a system administrator -- can manage the server and autorize community administrators, but cannot manage communities or individual accounts (except temporarily while we\'re getting started).'),
  
  'cgc' => t('Account is an effectively autonomous Common Good Community.'),
  'up' => t('Community is operational (up).'),
  'invites' => t('Community requires invitations.'),
  'rewardy' => t('Community issues rewards explicitly.'),
  'noneg' => t('Community does not reduce a member\'s credit line on overdrafts.'),
);
