<?php

/**
 * @file
 * Strings used throughout the system.
 */

require_once __DIR__ . '/defs.inc';

$GLOBALS['tStrings'] = array(
  // reports
  'system down' => t('The %PROJECT system is under maintenance. Please try again in a few minutes?'),
  'your community down' => t('Your %PROJECT community is temporarily on hold. Contact your %PROJECT community Admin for details: %contact'),
  'other community down' => t('That %PROJECT community is temporarily on hold.'),
  'account info' => t('Your balance for account %qid is %balance, including %rewards of incentive rewards. Total demand for %RCREDITS is %totalDemand.'),
  'invite reward' => t('inviting and/or assisting new member %fullName'),
//  'employee invite reward' => t('inviting and/or assisting a member\'s manager (at %fullName -- $%R_COUNTED_BONUSr per employee)'),
//  'account info2' => t("More account info"), // not used yet
//  'your account is ready' => t('Your starting balance is %balance. Pretend to buy and sell as you would in a typical month (we need that info). Have fun! Type "helpme" to get started.'), // separate out the last sentence
//  'the reward' => t('(reward: %rewardAmountr)'), // %rewardType
//  'report offer' => t('You offered %otherName %amount (%rewardType: %rewardAmount). Your balance is unchanged, pending acceptance.'), //  Transaction #%tid
//  'report invoice' => t('You %did %otherName %amount.'), // Invoice #%tid
   'report tx' => t('You %did %otherName %amount.'), // for %why.'),
   'selfhelp tx' => t('You paid %myName %amount.'), // for %why.'),
   'for why' => t('(for %why)'),
//  'report exchange request' => t("You requested %amount from %otherName for %why. Your balance is unchanged, pending approval."), //  Invoice #%tid
  'report funding' => t("You successfully %did %amount to %otherName."),
  'balance unchanged' => t('Your balance is unchanged, pending payment.'),
//  'report broke invoice' => t('%otherName does not have enough %RCREDITS at this time, so an invoice has been sent, for %short.'), // UNUSED probably // Invoice #%tid
//  'report short payment' => t("SPLIT! You paid %otherName %amount (%short remains to be paid). Transaction #%tid"),
//  'report short invoice' => t("SPLIT! You charged %otherName %amount (%short has been invoiced). Transaction #%tid, Invoice #%t2id"),
//  'report short invoice' => t("SPLIT! You charged %otherName %amount (%short remains to be paid). Transaction #%tid"),
  'denial done' => t('The invoice has been marked "denied".'),
  'invoice delete done' => t('The invoice has been deleted.'),

  'paid employees' => t('You distributed %total to %successes of %employeeCount employees. See your transactions page for details.'),
//  'new-invoice' => t('%otherName requests %amount for "%payerPurpose". Type "pay" or "undo".'),
//  'new-offer' => t('%otherName is offering you a payment exchange of %amount for "%payeePurpose". Type "accept" or "undo".'),
  'report undo' => t('Transaction has been %solution.'),
//  'report get usd' => t('%Exchange approved. %request will be transferred directly to your bank account within 24 hours.'),
//  'get usd inactive' => t('If you were an Active Participant in the %PROJECT system, your request would transfer %amount directly to your bank account. Alas, you are not yet an Active Participant.'),
  'your demand' => t('Your total request for %RCREDITS is now %request.'),
  'updated relation' => t('Updated settings for relation to %otherName'),
  'deleted relation' => t('Deleted relation with %otherName'),
  'customer balance' => t('The current balance for %otherName is %balance.'),
  'info saved' => t('Your information has been saved.'),
  'not saved' => t('No change has been made to your information.'),
//  'payment accepted' => t('%RCREDITS payment accepted. In exchange, us%amount will be refunded to the payer, from your account.'), // UNUSED
  'charge accepted' => t('Your charge to %who\'s account has been approved (it is no longer in dispute). No action is necessary.'),
  'gift successful' => t('Your donation of %amount is successful. Thank you!'),
  'nothing done' => t('Nothing was done.'),
  'nothing undone' => t('Nothing undone.'),
  'event' => t('Account %fullName (%qid) is now %status.'),
  'approved' => t('Your account has been approved! You may now buy and sell through the %PROJECT system, including using your %PROJECT card for purchases at member businesses.'),
  'got funding' => t('You have received a %thing of %amountr from your %PROJECT community for %purpose.'),
  'got fine' => t('Your %PROJECT community charged you a fee of %amountr for %purpose.'),
  'inflation details' => t('Inflation adjustment is set at 5% APR pending a reliable measure of the true inflation rate.'),
  'gift sent' => t('Your chosen gift of %amount to %PROJECT has gone through.'),
  'tx desc passive' => t('%otherName %otherDid you %amount'),
  'tx desc active' => t('You %did %otherName %amount'),
  'when' => t('on %created'),
  'purpose' => t('for %purpose'),
  'confirm tx action' => t('Okay to %txAction?'),
  'share gift' => t('Your chosen ongoing gift of %share% of your rewards has been initiated for this month.'),
  'account suspended' => t('Your account has been temporarily suspended %why'),
  'to protect data integrity.' => t('to protect data integrity.'),
  'account funded' => t('Funds have arrived in your %PROJECT Account! You may now use your %PROJECT card for purchases.'),
  'transfer complete' => t('Your requested %transfer of %amount (into your %PROJECT Account) has completed.'),
  'triggers new card' => t('<b>NOTE:</b> <%span>If you change your %field</span>, a new %PROJECT card will be mailed to you and your account will be charged $%R_CARD_COST. ', '_span', 'span class="loud"'),
  'call bank' => t('Remember to check your bank account online or call your bank %when and ask them for the two tiny deposit amounts to your account (both under 20 cents). Then sign in to your account and follow the links to enter those amounts, so we can send you your %PROJECT card!'),
  'do step one' => t('Remember to sign back in to %PROJECT, to complete your account setup (you have several steps left to go).'),
  'take another step' => t('Your %PROJECT Account setup is not yet complete!'),
  'daily messages' => t('You will continue to receive reminders until your account is complete. (If you changed your mind, please click the "unsubscribe" link below.)'),
  'sign in' => t('Go to %CG_DOMAIN and click "Sign in" in the upper right corner.'),
  'invitation required' => t('You must be invited, to participate in %PROJECT. Ask someone to invite you or <%a1>click here</a> to request an invitation.'),
  'confirm invite' => t('Your friend %fullName has accepted your invitation to %PROJECT or named you as a reference. <%a1>Click here to confirm</a> you trust this person (for example, if you lent them $250 you would trust them to pay it back).</p><p>The %PROJECT community is a network built on mutual trust, so it is important that you invite (and confirm) only people you know and trust.</p>'),
  'the reward' => t('Your reward is %rewardAmount.'),
  'reward other' => t('Your reward is %otherRewardAmount.'),
  'reward customer' => t('The customer\'s reward is %otherRewardAmount.'),
  'new charge' => t('<b>Charge:</b> %otherName charged you %amount for "%payerPurpose".'),
  'new refund' => t('<b>Refund:</b> %otherName refunded you %amount for "%payerPurpose".'),
  'new payment' => t('<b>Payment:</b> %otherName paid you %amount for "%payeePurpose".'),
  'new payment linked' => t('<b>Payment:</b> <%aPayLink>%otherName</a> paid you %amount for "%payeePurpose".'),
  'new invoice' => t('<b>Invoice:</b> %myName invoiced you %amount for "%purpose". <%a1>Click here to pay or deny this invoice</a>. Or sign in at %CG_DOMAIN and click "Invoices TO You" on the History menu. If you have any questions or concerns about this invoice, please contact %myName at %email or %phone.'), // can't separate out 'invoice concerns'
  'stale invoice' => t('<b>Invoice Reminder:</b> %daysAgo days ago we emailed you an invoice from %payeeName for %amount. <%a1>Click here to pay or deny this invoice</a>. Or sign in at %CG_DOMAIN and click "Invoices TO You" on the History menu. If you have any questions or concerns about this invoice, please contact %payeeName at %email or %phone.'),
  'stale invoice report' => t('<b>Invoice Update:</b> %daysAgo days ago you invoiced %payerName %amount for %purpose (invoice #%nvid, %created). That invoice is still marked "pending", so we sent %payerName a reminder. If this invoice is no longer valid, sign in at %CG_DOMAIN and choose "Invoices FROM You" on the History menu to cancel it.'),
  //  'new invoice' => t('<b>Invoice:</b> %otherName invoiced you %amount for "%purpose". To pay (or deny) this invoice, <%a>click here</a>.'),
  'new customer done' => t('<h2>Congratulations!</h2>Your sign-up with %partnerName is complete. Expect a confirmation note from them by email.'),

  'short invoice' => t('Your balance is %short short, to pay your invoice from %payeeName.'),
  'expect a transfer' => t('Funds will be transferred from your connected bank account to cover this payment.'),
  'finish signup' => t('<b class="loud">IMPORTANT</b>: You must sign in at %CG_DOMAIN and finish setting up your %PROJECT account before this payment can be made.'),
  'when funded' => t('This payment will be made as soon as you put enough money in your account. To put money in your account, trade Dollars for %PROJECT Credits at a participating business or with another member OR sign in at %CG_DOMAIN and click "Bank" on the menu, to connect a bank account and transfer funds.'),
  'auto pay on' => t('From now on your invoices from %payeeName will be approved and paid automatically, drawing funds from your connected bank account as needed. (We do not expect there to be overdraft problems, of course, but if there are, you will be responsible for them.)'),
  'invoice denied' => t('<p><b>Invoice DENIED:</b> Your customer %payerName has declined to pay the following invoice:</p>
  <blockquote>%created %amount for "%purpose"</blockquote>
  <p>Reason given: "%reason". You may want to contact the customer directly, to resolve any dispute.</p>'),
  
  'invoice withdrawn' => t('<p><b>Invoice WITHDRAWN:</b> %payeeName has withdrawn the following invoice:</p>
  <blockquote>%created %amount for "%purpose"</blockquote>'),
  
  'charge disputed' => t('<p><b>Charge DISPUTED:</b> Your customer %otherName disputes the following charge:</p>
  <blockquote>%created %amount for "%payeePurpose"</blockquote>
  <p>You may want to contact the customer directly, to resolve this dispute.</p>'),
  'invoice canceled' => t('<p><b>Invoice CANCELED:</b> Your invoice from %otherName has been canceled:</p>
  <blockquote>%created %amount for "%payerPurpose"</blockquote>
  <p>This invoice will no longer appear in your list of invoices to you.'),
  'offer refused' => t('%otherName has declined to accept your %created offer of %amount for "%payerPurpose".'),
  'offer canceled' => t('NYI'),
  'over max' => t('Your balance is over the maximum you set.'),
  'under min' => t('Your balance has fallen below the target you set.'),
  'banked' => t('A request has been made to %action your bank a total of %amount. This request may take up to 5 days to go through.'),
  'bank tx number' => t('The transaction will appear on your bank statement as check #%checkNum.'),
  'drew' => t('A total of %amount has been drawn from your connected account(s).'),
  'payment exchange received' => t('You received from %fullName a payment exchange of %amount -- that is, you traded that much US Dollars for %RCREDITS, receiving a bonus of %bonus on the exchange.'),
  'payment exchanges offered' => t('You made payment exchanges to %offers %whom, totaling %total.'),
  'card ordered' => t('A card for %otherName will be US mailed to %company.'),
  'balances' => t('%balance with a Credit Line of %credit (you can <%a>cash out</a> up to %cashoutable today).', '_a', 'a href="' . BASE_URL . '/help/cashing-out"'),
//  'you sold out' => t('This week you traded %total worth of %RCREDITS for US Dollars, missing an opportunity for %rebates worth of rebates. To keep this from happening so much, <%a1>click here</a> to change your settings to the recommended minimum balance of %suggested.'),
  'new floor' => t('Your credit limit has been changed to %limit, based on your average use of %RCREDITS over the past 6 months.'),
  'no floor effect' => t('This currently has no effect, because you chose not to let your balance go negative. To change this setting, sign in at %CG_DOMAIN, click the Settings gear, then the Preferences tab. Put a check mark next to "Debt Okay" and save your preferences.'),
  'available now' => t('The funds are available immediately in your %PROJECT Account.'),
  'transfer canceled' => t('Your exchange request #%txid has been canceled.'),
  'invite languishing' => t('After %elapsed days, your friend at %email has not accepted your invitation to %PROJECT. Call maybe? (the invitation may be in their spam folder or lost).'),
  'invitee slow' => t('Your friend %fullName has not finished opening a %PROJECT Account (%elapsed days after accepting your invitation). The next step is: "%step". Call maybe, to offer help?'),
  'reminder coming' => t('We will send you just one reminder'),
  'reminder one' => t('This is your one reminder'),
  'invite default' => t("%PROJECT is great. I signed up. Check it out!\n\n%name"),
  'roundups desc' => t('roundup donations'),
  'crumbs desc' => t('crumbs donations: percentage of past month receipts'),
  'sent email' => t('<p>We emailed a Verification Code to you at <b>%email</b> (if you don\'t see it, check your Spam folder). Type the code here, choose a password, and press Next.</p><p>Mixed-case, digits, punctuation, and plain old length make your password strong.</p>'),
  'proxy help' => t(<<<EOF
    <p><b>Direct democracy</b> is at the heart of the %PROJECT design. In each community, members meet to discuss and decide on funding priorities for incentives, grants, loans, and investments of %RCREDITS for the greater good.</p>
    <p>You assign a person in your community as your ongoing Representative (Proxy). This should be a member you know and trust and generally agree with. Whenever you don't vote, your Representative votes for you. The Representative's vote simply counts double (one vote for them and one vote for you). If your Representative also fails to vote, then your Representative's Proxy votes for all three of you.<br><br>Everyone's voice is counted, whether or not they vote directly &mdash; one person one vote.</p>
    <p>Our community's most trusted Representatives (chosen by many people to be their proxy) will meet in person to research and debate specific issues. These trusted Representatives ("Trustees") do not decide broad policy issues &mdash; all members decide the policy issues. Instead, Trustees oversee public discussions, choose wording for questions to be voted on, take action as directed by the members, and assure that the system is running smoothly.</p>
    <p>There is no way to tell who has chosen you as their proxy. <i>Your proxy does NOT have access to your %PROJECT account, bank account, or personal information.</i></p>
EOF
  ),
  'credit ban on' => t('Maybe spending below a zero balance should be temporarily banned because demand for %RCREDITS has been negative %months months in a row.'),
  'credit ban off' => t('Temporary ban on spending below a zero balance should be lifted because demand for %RCREDITS has been positive %months months in a row.'),
  'coupon redeemed' => t('Success! %amount has been added to your balance, as a gift from %giver.'),
  'bad coupon' => t('That is not a valid coupon type.'),
  'invalid gift card' => t('That is not a valid gift card code. Try again.'),
  'already redeemed' => t('That coupon has already been redeemed.'),

  // questions
  "what's your name?" => t("To set up your %PROJECT Account, we need your full name and email address. First, what's your name?"),
  "what's your name really?" => t("Hmm. I don't know if I believe that. What's your name really?"),
  "what's your email?" => t("Welcome to %PROJECT, %fullName (account ID %qid). Last question: What's your email address?"),
  "what's your email really?" => t("Please type carefully. What is your email address?"),
  
  // confirmations
//  'confirm get r' => t('Trade us%request for r%request?'),
//  'confirm get usd' => t('Trade r%request for us%request?'),
  'confirm payment' => t('Pay %otherName %amount for %why?'),
  'confirm pay invoice' => t('Pay %otherName %amount (invoice %created)?'), // used in rSMS
  'confirm charge' => t('Charge %otherName %amount for %why?'),
//  'confirm exchange' => t('Okay to %confirmAction %otherName %amount in rCredits (for CASH/LOAN/ETC.)?'),
  'confirm pay employees' => t('Pay %employeeCount employees %amount each (a total of %total)?'),
  'confirm undo' => t('Undo %created transfer of %amount %tofrom %otherName?'),
  'confirm funding' => t('%confirmAction %amount to %otherName?'),
  'please confirm' => t('To confirm, type: %nonce'),
  'verification code' => t('Your verification code for using %PROJECT online is: %nonce'), // used in rWeb
  'whats up' => t('You have not had any %PROJECT transactions in the past month. Most members have at least one transaction per month, so this is a little unusual and we hope all is well with you. Please email or call us if you have any problems or questions.'),
  'co promo' => t('And if you need more promotional cards to hand out to customers, just let us know!'),
  
  // error messages
  'syntax' => t('Wrong syntax!'),
  'nyi' => t('That feature is not available through the SMS interface.'),
  'already started' => t('You are already a %PROJECT member. For help, type "helpme".'),
  'duplicate name' => t('That name is already taken, in your region. Try again. Use a middle initial?'),
  'name too long' => t('That name is too long. Try again (keep it under 60 characters).'),
  'no SMS permission' => t('Alas, you do not have permission to use the %PROJECT SMS interface at this time.'), // UNUSED
  "can't cash incentives" => t("You can SPEND your incentive rewards (%rewards to date), but you can't cash them out. Your %RCREDITS balance available to exchange for US Dollars is %cashable."),
//  "can't cash incentives" => t("Once you are an active participant, you can spend your incentive rewards (%balance to date), but you can't cash them out. Your rCredits balance available to exchange for US Dollars is $0."),
//  'get r/usd' => t('You can get only "r" or "usd" (rCredits or US Dollars).'), // not yet used (maybe used when there is a third option)
  'not a feature' => t('"%feature" is not a feature.'),
  'wrong nonce' => t('That is not the expected confirmation response. Try again (start over).'),
  'no self-trading' => t("You can't exchange credit with yourself."),
  'no self-relation' => t("You can't relate yourself to your own account."),
  'funding error' => t("You do not have permission to fund that person."),
//  'no company relations' => t("Only individuals can be related to this account."),
// not used  'no self-undo' => t("You can't undo transactions with yourself."),
  'no matching invoice' => t('That is not an invoice to be paid.'), // Type "helpme pay".'),
  'already paid' => t('That invoice is already paid.'), // message to either payer or payee
  'undo no match' => t("You have no matching transactions to undo."),
  'undo moving target' => t("Can't undo: the last matching transaction has changed."),
  'unknown member' => t('Unknown member: "%who" (they need to sign up before we can recognize them).'), // Type "helpme pay".'),
  'not an account' => t('That account does not exist.'), // this is usually a system error (or REST error)
  'non member' => t('%otherName does not have an active %PROJECT Account.'),
  'not yet active' => t('%name\'s account is not yet activated.'),
//  'ambiguous other' => t('"%who" is associated with more than one %PROJECT Account. You will need to identify them differently. Type "more" for options.'),
  'ambiguous other' => t('"%who" refers to more than one %PROJECT Account.'), // must be here for ajax
  'ambiguous other2' => t('You can identify a member by phone, email, fullname (no punctuation or spaces), or account ID.'),
  'what relationship' => t('You forgot to say whether you are an employee, owner, or both.'),
  'short put' => t('The most you can transfer to your bank at this time is %max.'),
//  'short cash to' => t('You are %short short for that transaction.'),
//  'short cash from' => t('%otherName is %short short for that transaction.'),
//  'short cash from vague' => t('%otherName does not have enough rCredits for that transaction.'),
  'short to' => t('You are %short short for that transaction. Maybe increase your target balance?'),
  'short from' => t('%otherName is %short short for this request (do NOT try again).'),
  'short from vague' => t('%otherName does not have enough funds for this request (do NOT try again).'),
  'min too low' => t('Your target balance is set too low, causing conflicting deposits and withdrawals from your bank account. To increase your target balance to %suggestion, visit the %page page (under Settings).'),
//  'you were short' => t('You tried to make a purchase but did not have enough funds in your account. A higher "minimum" setting might keep this from happening again. To increase your minimum to the recommended setting of %suggested, <%a1>click here</a>.'), // Arghh! some email programs apparently click this link automatically
  'you were short' => t('You tried to make a purchase but did not have enough funds in your account. Automatic refills or a higher "target balance" might keep this from happening again. To set automatic refills or to increase your target balance, sign in at %CG_DOMAIN and click "Bank Info" on the Settings menu.'),
  'be not secretive' => t('Also, you have chosen to keep your balance secret. <%a2>Click here to change that</a> and let the cashier tell you how much you are short next it happens.'),
  'try debt' => t('You might also want to <%aDebt>select "Debt Okay" in your Preferences</a> (there is no charge for using your credit line as long as you bring your balance back up within 30 days).'),
  'join accounts' => t('You are invited to create a joint account with %name. To accept that invitation, <%atag>click here</a>. Once your accounts are joined, you sign in to either account to view your combined transactions. Both %PROJECT cards will remain valid.'),
  'click option' => t('OR <a href="%url">click here</a>'),
//  'shortr to' => t('You do not have enough rCredits, for that transaction. Type "get r <amount>" to get more rCredits.'),
//  'shortr from' => t('%otherName does not have enough rCredits for that transaction at this time.'),
//  'demand too small' => t('NOTE: The minimum amount of rCredits you can actually buy at a time is $%min.', array('%min' => R_MIN_DEMAND)),
  'no access get' => t('You do not have permission to exchange %RCREDITS for US Dollars or vice-versa.'),
//  'no access create' => t('You do not have permission to create transactions.'),
  'no perm' => t('You do not have permission to make %what for this account.'),
  //'no sell' => t('You do not have permission to make sales for this account.'),
  //'no buy' => t('You do not have permission to make purchases for this account.'),
  'other no perm' => t('%otherName does not have permission to make %what for that account.'),
  //'other no sell' => t('%otherName does not have permission to make sales for that account.'),
  //'other no buy' => t('%otherName does not have permission to make purchases for that account.'),
  'bad amount' => t('The amount must be a number.'),
  'bad date' => t('That is not a valid date.'),
  'nonpositive amount' => t('The amount must be a positive number.'),
  'negative amount' => t('The amount cannot be negative.'),
  'zero amount' => t('The amount cannot be zero.'),
//  'fractional cents' => t('The amount must be a whole number of cents.'),
  'too fractional' => t('You have too many digits after the decimal point.'),
  'amount too big' => t('Amounts larger than %max are not allowed.'),
  'already undone' => t('Transaction is already undone.'),
  'no such tx' => t('That transaction does not exist.'),
  'bank unverified' => t('You cannot exchange %RCREDITS for US Dollars until your bank account has been verified.'),
  'no ALL' => t('"ALL" (meaning all employees) is not allowed here.'),
  'no employees' => t('There are no employees related to this account.'),
  'shortname taken' => t('WARNING: There is already an account with a nearly identical name ("%otherName").'),
  'cannot contact' => t('We are unable to contact %fullName by email or text message. You may want follow up directly.'),
  'no last customer' => t('The system does not know who your last customer was.'),
  'max exchange' => t('The amount you can transfer or exchange in %period is limited to %amount.'),
  'beyond today' => t('The date cannot be any later than today.'),
  'must trust' => t('Do not invite someone unless you trust their integrity.'),
  'payment exchange error' => t('An unknown technical problem prevented your payment exchange %way %other.'),
  'no buyers' => t('Uh oh. Nobody wants to buy %RCREDITS. Needed: $%amount.'),
  'bad phone code' => t('That is not the correct verification code.'),
  'cannot offload' => t('A temporary technical problem prevented some of your excess %RCREDITS from being exchanged for US Dollars. The system will try again tomorrow.'),
  'usd failed' => t('A temporary technical problem prevented the transfer of US Dollars. No transaction took place.'),
  'cannot bank' => t('The %PROJECT system was unable to %action your bank: %error'),
  'cannot draw' => t('But the %PROJECT system was unable to draw from your connected account(s).'),
  'no source' => t('there is no valid funding source'),
//  'not verified' => t('you have not yet verified the connection to your bank account. Check your bank statement online or call your bank (2-5 days after giving your routing and account numbers). There will be two small deposits made to your account, each less than %DW_VERIFY_ROOF cents. Sign in to rCredits.org, then follow the instructions there to supply those two deposit amounts.'),
  'year is 4 digits' => t('Be sure to type all four digits of the year.'),
  'date too early' => t('That date is too early.'),
  'date too late' => t('That date is too late.'),
  'tx save err' => t('The transaction could not be saved. Try again!'),
  'share too big' => t('You are not allowed to donate more than half your transaction rewards. Here\'s why: The %PROJECT system aligns your personal financial benefit with benefit to the community, so you can do what is best for the community just by doing what is best for you. If your personal benefit percentage is too small, you lose that convenient compass.'),
  'no photoid' => t('You must make a purchase in person using your %PROJECT card and show your driver\'s license or state ID, before any other transaction.'),
  'other no photoid' => t('%who must make a purchase in person to verify their identity, before you can pay or charge them.'),
  'duplicate transaction' => t('You already just %op that member that much. Wait a few minutes or type a different amount.'),
  'no search' => t('OR that member has chosen not to be identified by email or phone number.'),
  'must confirm' => t('%youName: membership must be approved by %inviterName OR you must do a transaction with %inviterName FIRST.'),
  'first at home' => t('%whose first transaction must be in person in your home community.'),
  'zero transaction' => t('The transaction amount cannot be zero.'),
  'bad birthdate' => t('Birth date does not match.'),
  
//  'savings too low' => t('Until your %PROJECT community declares itself a stable Common Good Community, your savings amount must be at least as much as the incentive rewards you have received (%rewards to date)'),
  'savings too low' => t('Your additional savings amount cannot be less than zero.'),
  'savings too high' => t('You can increase your savings amount only by what you have in your primary account. Your maximum additional savings amount at this time is %max.'),
  'organizer grant' => t('You have recruited %ORGANIZER_GRANT_MIN new %PROJECT members. You are probably eligible for a %PROJECT organizer grant of up to %ORGANIZER_GRANT_MAX! <%a>Click here</a> to apply.'),
  'bad forced tx' => t('You accepted a transaction (offline) that cannot be completed: customer %account, for %amount, on %created%by.'),  

  // notices to staff
  'cache mismatch' => 'Cron cache mismatch: %k=%is (cache) <> %shouldBe (calc) OR joint %k=%jIs (cache) <> %jShouldBe (calc)',
  'not enough usd' => 'Not enough usd (%usd) to send %need.',
);
