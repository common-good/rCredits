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
  global $base_url;
  $mya = r\acct();
  $company = !@$mya->can(B_PERSONAL);
  $acctName = @$mya->name; // @ is for SMS testing (when not signed in)

  $site = str_replace('https://', '', R_SITE_URL);
  $companyAccount = $company ? '' : t(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic rCredits webpage: <a target="_blank" href="$base_url/member/$mya->name">$site/member/$mya->name</a> &mdash; so choose wisely. See also the <a href="$base_url/account/company">Company Info</a> form.</p>
EOF
  );
  $paySpecial = t(' If you have employees you can say pay "ALL", to pay each employee the same amount.');

  // the <a href="$base_url/help/smartphone">Android Smartphone Interface</a> and 
  $helps = array(
    'general' => tt(<<<EOF
    <p>Help is also available for the <a href="$base_url/help/sms">Text Messaging Interface</a>.
    Use the tabs above and the links at the top right, to try all the features of the rCredits system. Here's an overview of what's where:</p>
    <h2>Tabs:</h2>
    <ul>
      <li><b>Summary</b> gives you an overview of your account.</li>
      <li><b>Transactions</b> shows your recent transactions so you can accept, edit, print, or undo them.</li>
      <li><b>Charge</b> lets you invoice another participant (to be paid in rCredits and/or US Dollars). Or, if you have a qualified business account, you can charge them. Normally, both buyer and seller get an incentive reward of additional rCredits.</li>
      <li><b>Pay</b> lets you transfer your rCredits to another participant. Normally, both buyer and seller get an incentive reward of additional rCredits.</li>
      <li><b>Find Businesses</b>. This is where you can find out who takes rCredits for what you want.</li>
    </ul>
    <h2>Upper Right Links</h2>
    <ul>
      <li><b>Settings</b> has several subtabs:
      <ul>
        <li><b>Contact Info</b> is where you add or change your contact phone numbers and address.</li>
        <li><b>Basic Account Settings</b> is where you change your email address, password, picture, or time zone.</li>
        <li><b>Cell Phones</b> lets you connect your account to one or more cell phones, so you can use Text Messaging to buy and sell things with rCredits.</li>
        <li><b>Relations</b> lets you connect individual rCredits participants to your account &mdash; as owners, employees, suppliers, or others, with limited or full access to the account. This is useful for authorizing sales clerks or setting up joint accounts.</li>
        <li><b>Preferences</b> gives you several choices about how your rCredits account works.</li>
      </ul>
      </li>
      <li><b>Membership</b> tells you where you are in the process of opening and activating your account. This section will give you links to your next steps.</li>
      <li><b>Help</b> is this page.</li>
      <li><b>Log out</b> protects your account from unauthorized passersby.</li>
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
    <p><b>Whom:</b> Type all or part of the full name of the account you want to pay. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID. Note that a period in the account ID (a regular account) is different from a colon (an agent acting on behalf of an account).$paySpecial</p>
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
    ) . ($company ? t('<p>If you give the person at least "Sell" permission (and save the record), you can request a Member ID Card for them to use on behalf of this account.</p><p>"$/mo" is the most important column. Type the amount you pay each employee or supplier in a month &mdash; more or less. Then, if you select the virtual payment option on the preferences page, your rCredits will get distributed every night to those employees and suppliers in exchange for US Dollars.</p>') : ''),
    
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
      <li><b>Minimum.</b> You will want to keep enough rCredits plus US Dollars in your account to cover the total amount you expect to spend at rCredits member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
      <li><b>To bank.</b> This setting is usually a good idea unless you use your Dwolla account outside the rCredits system.</li>
    </ul>
    <p>If there are settings you would like to see here &mdash; that is, things you want to work differently &mdash; please let us know. We expect to offer many additional options over the next few months.</p>
EOF
    ),
    'account/usd' => t(<<<EOF
    <p>Open a Dwolla account and type the number here, to connect it to your rCredits account. In partnership with Veridian Federal Credit Union, Dwolla communicates with your US Dollar bank account. Whenever you buy something, your rCredits (if any) are used first and the remainder is paid in US Dollars &mdash; just like a typical debit card. Tips:<ol><li>Connect your <i>checking</i> account to Dwolla (other account types are limited to 3 withdrawals/deposits per month).</li><li>Disable Dwolla <a href="http://dwolla.com/notifications">Money In</a> notifications.</li></ol></p>
    <p>Just as for opening any other financial account, you need to supply identifying information and proof of identity. Your information is then stored doubly-encrypted on a separate server.</p>
EOF
    ),
    'why-wait' => t(<<<EOF
    <p>Exchanges of rCredits for US Dollars reflect an incomplete economic circle. That is, the individual or company selling the rCredits does not yet have enough places to spend them.</p>
    <p>The 2-week delay in spending rCredits that you buy takes that amount out of circulation temporarily, giving the seller time to find more places to spend rCredits, without meanwhile having to sell more and more of them.</p>
EOF
    ),
    'to-bank' => t(<<<EOF
    <p>This "To bank" setting is probably a good idea unless you use your Dwolla account outside the rCredits system. It can take up to 3 days for the bank to receive the funds, once they are sent.</p>
EOF
    ),
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
    'membership/agreement' => t(<<<EOF
    <p>The rCredits Agreement is at the heart of the the rCredits system. It is our agreement to accept rCredits from each other that gives them value. Please read each section carefully and click the checkboxes to indicate your agreement. For more specific information, click on the hyperlinks in the text.</p>
EOF
    ),
    'membership/contribute' => t(<<<EOF
    <p>Your contribution helps fund administration and promotion of the rCredits system during the pilot phase. It is also a formal "sealing" of your signature on the rCredits agreement.</p>
EOF
    ),
    'membership/proxies' => t(<<<EOF
    <p>During the pilot phase, it may be hard to choose two proxies from among the other members, unless you invite them to join first. You can change your selections at any time, so no worries.</p>
EOF
    ),
    'membership/invite' => t(<<<EOF
    <p>The rCredits system is designed to be community-centered, so our relationships with friends and neighbors in our own geographical community is important. We can work better together for the well being of everyone in our community and our region, if we know we can trust one another.</p>
    <p>Invite as many people as you like, but only people you trust.</p>
EOF
    ),
    'virtual-average' => t(<<<EOF
    <p>Virtual payments <i>approximate</i> direct payments in rCredits. Someday, when rCredits become very popular, there may be accounting programs that make it easy to do payroll directly in rCredits. Meanwhile, virtual payments let you keep using your current payroll and accounting systems without change, and still have the effect of paying people in rCredits.</p><p>Your estimates of average monthly payments do NOT have to be exact. If an employee or supplier ends up buying more rCredits from you than their share, it's not a big problem for anyone.</p>
EOF
    ),
  );

  $helps['charge'] = strtr($helps['pay'], array('pay' => 'charge', $paySpecial => ''));  
  return u\SUBS(@$helps[strtolower($page)]) ?: tt('no help', compact('page'));
  //        <p>Download the app from <a href="http://rCredits.org/rc.apk">rCredits.org/rc.apk</a>, install, run. Try all the buttons.</p>

}
/*
      <p>If you are acting as an agent for someone else's account &mdash; for example if you are a sales clerk for a department store &mdash; the period will be a colon instead. Your company ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) identifies both you and the company.</p>
*/