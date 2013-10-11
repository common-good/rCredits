<?php

/*
 * @file
 * rCredits Help Messages
 * This file contains a single function that returns a help message.
 */
 
namespace rCredits\Web;
use rCredits as r;
use rCredits\Util as u;

/**
 * Return a help message for the page.
 */
function helpText($page) {
  global $rUrl, $base_url;
  $mya = r\acct();
  $company = !@$mya->person();
  $acctName = @$mya->name; // @ is for SMS testing (when not signed in)

  $site = str_replace('https://', '', R_SITE_URL);
  $companyAccount = $company ? '' : t(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic rCredits webpage: <a target="_blank" href="$base_url/member/$mya->name">$site/member/$mya->name</a> &mdash; so choose wisely. See also the <a href="$base_url/account/company">Company Info</a> form.</p>
EOF
  );

  // the <a href="$base_url/help/smartphone">Android Smartphone Interface</a> and 
  $helps = array(
    'general' => tt(<<<EOF
    <p>Help is also available for the <a href="$base_url/help/sms">Text Messaging Interface</a>.
    Use the tabs above and the icons at the top right, to try all the features of the rCredits system. Here's an overview of what's where:</p>
    <h2>Tabs:</h2>
    <ul>
      <li><b>Summary</b>: an overview of your account.</li>
      <li><b>Transactions</b>: shows your recent transactions so you can accept, edit, print, or undo them.</li>
      <li><b>Charge</b>: Invoice another participant (to be paid in rCredits and/or US Dollars). Or, if you have a qualified business account, you can charge them. Normally, both buyer and seller get an incentive reward of additional rCredits.</li>
      <li><b>Pay</b>: Transfer your rCredits to another participant. Normally, both buyer and seller get an incentive reward of additional rCredits.</li>
      <li><b>Get r/USD</b> Transfer money to or from your bank account.</li>
      <li><b>Find Companies</b>. Find out who takes rCredits for what you want.</li>
      <li><b>Reports</b>. See how the rCredits economy is doing in your community.</li>
    </ul>
    <h2>Icons, etc.</h2>
    <ul>
      <li><b>Sign out (Your icon)</b>: Click on the picture of yourself to sign out or change users.</li>
      <li><b>Settings (Gear icon)</b>: Change your account settings, customize your company webpage (or make a Buy-with-rCredits button), make another contribution, or connect a mobile device.</li>
    </ul>

    <p>If you need help and can't find the answer here, please feel free to <a href="mailto:@R_ORG_EMAIL?subject=help with rCredits" target="_blank">email us</a>.</p>
EOF
    ),
    'smartphone' => t(<<<EOF
    <p>To use the smartphone interface, you will need to install an app to read QR codes (for example QRDroid on Android phones or Qrafter for iPhones). You can pay participating businesses by scanning their QR code, then visiting the scanned URL.</p>
    <p>Your customers (or employer) can pay you by scanning the QR code on your member ID card. You can print an Advance ID card for your individual account, using the link on the Summary page.</p>
EOF
    ),
    'sms' => t("The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the <a href=\"$base_url/sms\">Test SMS form</a> to simulate sending a text."),

    'summary' => tt(<<<EOF
    <p>This page shows an overview of your rCredits account.</p>
    <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region (@R_REGION_NAME). The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
  <p><b>Balance</b> is the sum of your rCredits (green) and your US Dollars (blue).</p>
  <p><b>Rewards</b> totals your signup bonus, adjustments for inflation, and all your rebates and bonuses ever.</p>
  <p><b>Credit floor</b> goes negative (meaning you can go into debt) a little at a time, based on your rCredits history.</p>
EOF
    ),
    'txs' => t(<<<EOF
    <p>Select the period you want to see transactions for. Both the Summary and Transactions sections will reflect just the transactions in that period. However, the "Pending" line will reflect all your not-yet-accepted invoices and offers EVER. The Ending balance in the Summary is always your current balance.</p>
    <p>If the status is "pending", either you or the other party will need to click on "OK" to complete the deal. Click on the "X" to reject an invoice, dispute a charge, reject an offer, or cancel or undo something you did. Click on the pencil to edit the description. The description is just for you &mdash; the other party will not see it.</p>
    <p>The Rewards column shows how much rCredits were added to your account as a reward for joining or for buying or selling real goods and services. Loans, exchanges for US Dollars, and other purely financial deals do not earn rewards.</p>
EOF
    ),
    'pay' => t(<<<EOF
    <p><b>Whom:</b> Type all or part of the full name of the account you want to pay. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID. Note that a period in the account ID (a regular account) is different from a colon (an agent acting on behalf of an account).</p>
    <p><b>Amount:</b> The amount of the transaction, measured in dollars, to be paid in rCredits.</p>
    <p><b>Purpose:</b> If this is a purely financial transaction, click on "cash/loan/etc". Otherwise, a description is required (for purchases and sales of real goods and services). Contributions to nonprofit organizations count as purchases of real services &mdash; services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank &mdash; your statements will call the purpose "cash".</p>
EOF
    ),
    'directory' => t(<<<EOF
    <p>This is where you can find out who takes rCredits for what you want. Click on a business name to see its rCredits webpage.</p>
    <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you choose the United States "01" postal code region.</p>
EOF
    ),
    'account/basic' => t(<<<EOF
    <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
EOF
    ),
    'account/cells' => t(<<<EOF
    <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with rCredits. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
EOF
    ),
    'account/relations' => t(<<<EOF
    <p>Relations let you connect individual rCredits participants to your account &mdash; as owners, employees, suppliers, or others, with limited or full access to the account. This is useful for authorizing sales clerks or setting up joint accounts. Or you might give your accountant "read" permission. Company accounts are managed only by an individual agent.</p>
    <p>Yellow highlights show settings that need to be confirmed &mdash; either by you or by the other party.</p>
    <p>To change settings in a Yes/No column, click on + or -. To remove someone from the list, make them all "No"s, and no permissions (if they are already that way, change something and change it back). Be sure to "Save Changes".</p>
    <p>You can allow another account to "draw" from this one to avoid overdrafts (a 25 cent fee may apply).</p>
EOF
    ) . ($company ? t('<p>If you give the person at least "Sell" permission (and save the record), you can request a Member ID Card for them to use on behalf of this account.</p>') : ''),
    
    'account/contact' => t(<<<EOF
    <p><b>Full name</b> should be your full legal name. If you change the name on the account, you will need to buy a new rCard and your account will be disabled until you supply new proof of identity. For companies, your rCredits website address will also change.</p>
    <p><b>Postal code:</b> Your postal code determines what rCredits "Common Good Community" you are in.</p>
    <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
EOF
    ),
    'account/photo' => tt(<<<EOF
    <p>For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. If you have trouble coming up with a photo in a format that works, have a youngster take picture of you with their cell phone or tablet. Ask them to email you the picture, save that attachment to your desktop, then upload it here. If you still have trouble, email a description of your predicament to us at <a href="mailto:@R_ORG_EMAIL?subject=photo trouble" target="_blank">@R_ORG_EMAIL</a> and we will help.</p>
    $companyAccount
EOF
    ),
    'account/company' => t(<<<EOF
    <p>All of the information from this form goes automatically on your <a target="_blank" href="$base_url/member/$acctName">rCredits webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. Click on the link, to see the results (after saving your changes).</p>
EOF
    ),
    'account/preferences' => t(<<<EOF
    <p>These settings start out as what most people prefer. Feel free to adjust them as you like. You can always come back and change them again later.</p>
    <ul>
      <li><b>Minimum.</b> You will want to keep enough in your account to cover the total amount you expect to spend at rCredits member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
      <li><b>Email notices.</b> We suggest you begin with daily notices, until you feel comfortable with the system. You will receive a "daily" notice only when there is something new that you will want to know about (such as a credit to your account).</li>
    </ul>
EOF
    ),
    'account/usd' => t(<<<EOF
    <p>Open a Dwolla account and type your PIN number here, to connect it to your rCredits account. In partnership with Veridian Federal Credit Union, Dwolla communicates with your US Dollar bank account. Whenever you buy something, your rCredits (if any) are used first and the remainder is paid in US Dollars &mdash; just like a typical debit card.</p>
    <p>Just as for opening any other financial account, you need to supply identifying information and proof of identity. Your information is then stored doubly-encrypted on a separate server.</p>
EOF
    ),
/*    'dwolla-account-setup' => u\SUBS(t(<<<EOF
    <p>In partnership with Veridian Federal Credit Union, Dwolla communicates with your US Dollar bank account. Whenever you buy something, any rCredits you have are used first and the remainder is paid in US Dollars &mdash; just like a typical debit card, but with rebates and bonuses!</p>
    <p>This help window will stay open while you set up your Dwolla account. So you can come back here for tips and next steps.  <a href=":R_DW_LINK" target="_blank"></p>
    <p>Click here now to set up your Dwolla account</a> (following the tips below). When you're done, <a href="$base_url/account/usd">click here to continue </a> your rCredits account setup.</p>
    <h2>Tips:</h2>
    <ol>
      <li><b>Email and password.</b> These can be the same as or different from your rCredits email and password.</li>
      <li><b>Contact info and security info.</b> This information should generally be the same as what you tell the rCredits system.</li>
      <li><b>Connecting to your bank.</b> Click on "Funding sources" on the left side. Dwolla will ask for information about the bank account you want to connect. You will want to connect your <i>checking</i> account (other account types are limited to 3 withdrawals/deposits per month).
        <ul>
          <li><i>Account name</i> can be anything that reminds you what account you connected, such as "my checking account".</li>
          <li>The <i>routing number</i> appears at the bottom of your checks, on the left.</li>
          <li>The <i>account number</i> appears at the bottom of your checks, in the middle.</li>
          <li>Click "Add your account" and you're done. Dwolla will send two tiny deposits to your bank. You will have to wait a couple days, then find out from your bank the amounts of those two deposits. Dwolla will email you a link to click on, to return to that page and tell it those amounts, to verify that you really own that bank account.</li>
        </ul>
      <li><b>Nothing else.</b> You do NOT need to do anything else on the Dwolla site, despite its many choices. Come back and <a href="$base_url/account/usd">click here to continue</a> your rCredits account setup.</li>
    </ol>
EOF
    )),
    'to-bank' => t(<<<EOF
    <p>This "To bank" setting is probably a good idea unless you use your Dwolla account outside the rCredits system. It can take up to 5 days for the bank to receive the funds, once they are sent.</p>
EOF
    ),
    */
    'issue-credit' => t(<<<EOF
    <p>rCredits are a type of credit. Like any medium of exchange, they represent value. They make it possible for us to trade goods and services with each other in our community, without having to trade something directly with whoever has what we want, and without having to give them something of precisely the same value. The medium of exchange lets us give any amount of our goods or services to any participant in the economy, then receive the same value of goods or services from some other participant(s). If everyone both buys and sells, the credits move through the community in interconnected circles of exchange.</p>

    <p>Issuing rCredits is a community responsibility. This is the key to creating a Common Good Economy. We must issue as much credit as we need, to enable our exchanges of goods and services -- but no more than the value that we as a community are capable of producing.  Once rCredits become well-established and popular in a community, we can issue them together, with careful planning, as grants, loans, and equity investments. Together we can fund whatever enterprises and activities we decide would be best -- for our community, for communities elsewhere, and for the world.</p>
EOF
    ),
    'common-good-community' => t(<<<EOF
    <p>The purpose of the rCredits system is to create and fund community-centered participatory democracy. A community that has met the <a href='$base_url/help/common-good-community-requirements'>requirements</a> to become a Common Good Community may apply to Common Good Finance for certification. This certification permits the community to issue rCredits independently -- deciding for itself how many rCredits to issue (within safe limits) and what to fund with them.</p>

    <p>A Common Good Community is in "good standing" when it is issuing rCredits responsibly so that they accurately represent the reality and potential of its members to produce goods and services to back those rCredits.  A Common Good Community may lose its good standing status if it runs up a trade deficit with other Common Good Communities that it fails to redeem in timely manner. Common Good Finance and the other communities will provide ongoing feedback, to prevent this from happening accidentally.</p>
EOF
    ),
    'graceful-failure' => t(<<<EOF
     <p>Common Good Finance has put great care into designing the rCredits system so that it will NOT fail. Paradoxically, one requirement for its success is a detailed plan for a graceful shutdown. In the unlikely event that the whole rCredits system proves to be unworkable, the rCredits Agreement give us a way to end without leaving anyone "holding the bag".</p>
     
     <p>As part of the agreement, you promise to give back all the rCredits you received for free &mdash; as signup bonus, rebates, and sales bonuses. You can do that either by continuing to accept rCredits until your balance is equal to your total incentive rewards (then returning them to the community) OR you can substitute US Dollars for any missing rCredits. Complementarily, if you have more rCredits than you received as incentive rewards, you will be able to continue to make purchases with them for a short time, or trade them for other members' US Dollars that <i>they</i> returned to the community. Once a Common Good Community has ongoing revenue to cover its expenses and any incentive rewards, it is no longer in any danger of ending abruptly.</p>
     
     <p>We make this agreement &mdash; all of us &mdash; so that if the system fails, everyone is protected and no one is left holding the bag.</p>
EOF
    ),
    'ethics' => t(<<<EOF
    <p>A Common Good Community is relying on its members to exhibit a high level of ethical behaviour, as the basis for creating a culture of trust and possibility.  Of course it is always possible for someone to "game" the system and we will continue to develop software that brings such misbehavior to the attention of the community. Sometimes what looks like unethical behavior is in reality a desperate bid to escape from poverty. It is our responsibility as a community to think well about what our members need and want, and use the funding power of rCredits accordingly, so that such desperation is never necessary.</p>
EOF
    ),
    'dispute-resolution-process' => t(<<<EOF
    <p>Disputes between rCredits members will be resolved as follows:</p>
      <ol>
        <li>The members will work together cooperatively in good faith to attempt to resolve the matter themselves, amicably.</li>
        <li>If, after 30 days, the members have been unable to agree on a resolution, at the request of either member the members will attempt to resolve the matter creatively with the help of a mediator.</li>
        <li>Failing such resolution, at the initiative of either member, the members will submit the matter to binding arbitration, as follows:
          <ol style='list-style-type:lower-alpha;'>
            <li>Either member will initiate the dispute resolution process by sending a registered letter to the other at their address of record in which they appoint a representative and include the representative's full contact information. The representative should be familiar with Common Good Finance and agree to put in the time and good will necessary to resolve the dispute.</li>
            <li>Within 10 days of the postmark of the registered letter, the other Party will submit a registered letter naming their representative, again with full contact information, and including best times for a meeting.</li>
            <li>The representative of the initiating Party will contact the other representative and set up a meeting. If there are any costs associated with setting up the meeting between organizers, they will be born by the organizers with the expectation that they will be reimbursed by their representees.</li>
            <li>The organizers will agree on and appoint an arbitrator who agrees to spend the time and energy to resolve the dispute. Any costs incurred by the arbitrator will be borne equally by each Party.</li>
            <li>The arbitrator may request any documentation or witnesses deemed necessary to arrive at a resolution, and both parties agree to comply promptly.  The decision of the arbitrator is binding on both parties.</li>
            <li>The dispute resolution will be written up by the arbitrator and signed by both organizers.</li>
            <li>The following timetables will be adhered to as well as possible: The organizers have 10 days to agree on an arbitrator, and the arbitrator will have 10 days to come up with a resolution.  All parties agree to complete the dispute resolution process within 30 days.</li>
          </ol>
        </li>
      </ol>
EOF
    ),
    'paying-attention' => t(<<<EOF
    <p>Common Good Finance and the rCredits system will give you regular feedback on how your actions impact on your finances and on the well being of the community. In order for this feedback to be effective, you have to pay attention to it. Please feel free to make suggestions to improve the quality or timing of the feedback.</p>
EOF
    ),
    'decision-process' => t(<<<EOF
    <p>Each Common Good Community will decide for itself how to manage its decision making, so that active participation by all members is valued and encouraged. Common Good Finance is developing recommendations for best practices.</p>
EOF
    ),
    'other-decisions' => t(<<<EOF
    <p>From time to time, members will need to make a variety of non-financial decisions. Trustees need to be elected. Policies and procedures ocasionally need to be revised. Other issues may arise.</p>
EOF
    ),
    'common-good-community-requirements' => t(<<<EOF
    <p>Requirements for certification as a Common Good Community are not yet finalized. They may include such things as qualified leadership, minimum size, experience using the rCredits system, diversity of participants, and successful completion of a training program.</p>
EOF
    ),
    'agreement' => t(<<<EOF
    <p>The rCredits Agreement is at the heart of the the rCredits system. It is our agreement to accept rCredits from each other that gives them value. Please read each section carefully and click the checkboxes to indicate your agreement. For more specific information, click on the hyperlinks in the text.</p>
EOF
    ),
    'contribute' => t(<<<EOF
    <p>Your contribution helps fund administration and promotion of the rCredits system during the pilot phase. It is also a formal "sealing" of your signature on the rCredits agreement.</p>
EOF
    ),
    'account/proxies' => t(<<<EOF
    <p>During the pilot phase, it may be hard to choose two proxies from among the other members, unless you invite them to join first. You can change your selections at any time, so no worries.</p>
EOF
    ),
    'invite' => t(<<<EOF
    <p>The rCredits system is designed to be community-centered, so our relationships with friends and neighbors in our own geographical community is important. We can work better together for the well being of everyone in our community and our region, if we know we can trust one another.</p>
    <p>Invite as many people as you like, but only people you trust.</p>
EOF
    ),
    'payment-exchanges' => t(<<<EOF
    <p>Payment exchanges are designed to make it easy for business owners and managers to use rCredits. Pay your employees and suppliers the usual way, on the usual schedule, in US Dollars, with no change to your payroll or accounting systems. Then they trade some of those US Dollars back to you for rCredits.</p>
    <p>Payments are automated, based on what you choose for your account settings. Here's how it works:</p>
    <img src="$rUrl/images/payment-exchanges-companies.png" align="right"><p>Every day, customers pay you with rCredits and US Dollars. You get a 10% bonus on the rCredits part. Then at night, all the rCredits you took in get traded automatically to your employees and suppliers, in exchange for US Dollars. You get a 5% rebate on the exchange.</p>
    <p>How much does each employee and supplier get? They should get their usual share. On the "Relations" page, estimate how much you normally pay each employee or supplier in a typical month. Divide each one by 30 and that's their maximum daily share of the rCredits you pay out. At first you will have only small amounts of rCredits to pay out, so everyone gets proportionately less (automatically).</p>
    <p>Someday, when rCredits become very popular, there may be accounting programs that make it easy to do payroll directly in rCredits. Meanwhile, payment exchanges let you keep using your current payroll and accounting systems without change, and still have the effect of paying people in rCredits. The net effect for you is that you receive extra US Dollar income from using the rCredits system.</p>
    <p>Your estimates of average monthly payments do NOT have to be exact. If an employee or supplier ends up buying more rCredits from you than their exact share, it's no big problem for anyone.</p>
EOF
    ),
    'transactions' => t(<<<EOF
    Hover over any option or column header for an explanation of what it means. Select some options, and the period or specific date range that interests you, then click on either "Show" or "Download". The downloaded spreadsheet (in CSV format) has more complete information, including column totals.
EOF
    ),
    'demand' => t(<<<EOF
    "Demand" for additional rCredits is the total amount of official currency people have moved into the rCredits system, that can then be traded for rCredits. Generally, as long as people have plenty of places to spend within the rCredits system, they will keep moving money in. So the demand gives us a reliable measure of the system's soundness. Generally, we can keep issuing rCredits as long as there is a net inflow of official currency (a increasing demand). If members need to make more purchase outside the rCredits network and are taking more official currency out of the system than they are putting in (a decreasing demand), then we must temporarily stop issuing rCredits. We can issue more rCredits again once people have more places to spend within the system (typically because more people and businesses have signed up) and
EOF
    ),
    'to-bank' => t(<<<EOF
    <p>Transfers from an rCredits account to a bank account often reflect an incomplete economic circle. That is, the individual or company moving rCredits to a bank account does not yet have enough places to spend them. Generally, this means they are spending them outside the community (and outside the rCredits system). This is an opportunity for import-replacement &mdash; that is, an opportunity to start new local businesses to supply goods or services that are not currently available in our region.</p>
    <p>The total monthly volume of transfers to a bank account is a measure of how well we are doing as a community, in our progress toward a sustainable local economy.</p>
    <p>As an individual, you can help by finding ways to spend within the rCredits system, instead of transferring your funds to a bank account. For all the money that circulates within the system, we can use rCredits, freeing up that much official currency to use for something else. So, in effect, when you can keep money in the system, we are all that much richer.</p>
EOF
    ),
    'savings' => t(<<<EOF
    <p>The value of rCredits, like any currency, is what you can get for them. When rCredits members save funds for spending a month or more in the future, it means they are confident (1) that they will be able to spend them within the rCredits system OR (2) that they will be able to trade them for US Dollars because other members can spend them within the system. So savings are a <em>speculative</em> demand for rCredits, rather than a demand based in current reality. The speculation may indeed be accurate, but the circulation margin (monthly purchase volume beyond the amount of rCredits already issued) is a stronger measure of the system's capacity for additional rCredits.</p>
    <p>The amount of savings among members is calculated (roughly) as the total amount of funds in the system minus the purchase volume over the past month.</p>
EOF
    ),
    'your-return' => tt(<<<EOF
    <p>Your effective return is calculated as your monthly inflation adjustment (currently based on @R_INFLATION_RATE% APR) plus any incentive rewards you got in the period, all divided by your average balance for the period. The result is then annualized to give you an APR (annual percentage rate). Regular financial institutions give you interest on your <em>minimum</em> balance for the month, rather than your <em>average</em> balance, but that seems a bit petty.</p>
EOF
    ),
    'social-return' => tt(<<<EOF
    <p>The community's benefit from your participation is calculated as the sum of:</p>
    <ul>
      <li>The amount of money you moved into the system, minus the amount you moved out. By using rCredits in place of those funds, as they circulate among rCredits members, we can free up that much US Dollars for the community to grant, lend, or invest.</li>
      <li>Your incentive rewards for buying and selling. Every time you get such a reward, someone somewhere gets the same amount.</li>
      <li>A percentage (@R_CC_FEE%) of your purchase and sale amounts, equal to half the typical processing fee that businesses pay for credit card purchases. Together with the other party to the transaction, you kept that much money from leaking out of the community to the big banks.</li>
    </ul>
    </p>
EOF
    ),
  );

  $helps['charge'] = strtr($helps['pay'], u\ray('pay', 'charge'));  
  return u\SUBS(@$helps[strtolower($page)]) ?: tt('no help', compact('page'));
  //        <p>Download the app from <a href="http://rCredits.org/rc.apk">rCredits.org/rc.apk</a>, install, run. Try all the buttons.</p>

}

/*
<p>"$/mo" is the most important column. Type the amount you pay each employee or supplier in a month &mdash; more or less. Then, if you select the payment exchange option on the preferences page, your rCredits will get distributed every night to those employees and suppliers in exchange for US Dollars.</p>

    'why-wait' => t(<<<EOF
    <p>Exchanges of rCredits for US Dollars reflect an incomplete economic circle. That is, the individual or company selling the rCredits does not yet have enough places to spend them.</p>
    <p>The 2-week delay in spending rCredits that you buy takes that amount out of circulation temporarily, giving the seller time to find more places to spend rCredits, without meanwhile having to sell more and more of them.</p>
EOF
    ),
    
      <p>If you are acting as an agent for someone else's account &mdash; for example if you are a sales clerk for a department store &mdash; the period will be a colon instead. Your company ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) identifies both you and the company.</p>
*/