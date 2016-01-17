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
 * Return the appropriate raw help text.
 * @param string $i: identifier for the appropriate help text.
 * @param string $tag: (optional) item to highlight)
 */
function rawHelp($i, $tag = '') {
  global $rUrl;
  
  $site = str_replace('https://', '', BASE_URL);
  $mya = r\acct();
  $company = @$mya->co;
  $username = @$mya->name; // @ is for SMS testing (when not signed in)
  $regEmail = r\regionField('email');
  $companyAccount = $company ? '' : tt(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic rCredits webpage: <a target="_blank" href="@BASE_URL/member/@username">@site/member/@username</a> &mdash; so choose wisely. See also the <a href="@BASE_URL/account/company">Company Info</a> form.</p>
EOF
  , compact('username', 'site'));
  
  //Help is also available for the <a href="@BASE_URL/help/sms">Text Messaging Interface</a>.
  $back = '<p>' . u\backButton() . '</p>';
  if ($i == 'general') return tt(<<<EOF
    <p>Here's what to click, to find what you want:</p>
    <ul>
      <li><b>Summary</b>: See an overview of your account or open an rCredits account for your company.</li>
      <li><b>Charge</b>: Invoice another participant (to be paid in rCredits and/or US Dollars). Both buyer and seller get an incentive reward for purchases of goods and services.</li>
      <li><b>Pay</b>: Transfer rCredits to another participant. Both buyer and seller get an incentive reward for purchases of goods and services.</li>
      <li><b>Bank</b> Transfer money to or from your bank account.</li>
      <li><b>History</b>: See a list of your recent transactions, invoices, notices, monthly statements, and annual tax forms (Form 1099-B).</li>
      <li><b>Community</b>. See how the rCredits economy is doing in your community. Review the rCredits Agreement, signed by all rCredits members. Find a participating company. Help the rCredits community grow by making a financial contribution or by inviting someone.</li>
      <li><b>Settings</b>. Change your account settings, including contact information, general preferences, security settings, bank account connection, relations with other accounts, and proxy choices.</li>
      <li><b>Photo (Choose an Account to Manage)</b>: Click your photo in the upper right corner, to select an account to manage, if you have more than one.</li>
    </ul>
    <p>If you don't see the help you need here, please <a href="mailto:@regEmail?subject=help with rCredits" target="_blank">email us</a> or use this form to send a secure encrypted message (if your message is secret).</p>
EOF
  , compact('regEmail'));

  if ($i == 'other') return tt(<<<EOF
    <h2>Need Help Signing In?</h2>
    <p>If your username and password aren't working, no worries. Just click "<a href="@BASE_URL/account/password/@username">Forgot it</a>" here or on the sign-in page, to get an email for choosing a new password.</p>
    <h2>About rCredits&reg;</h2>
    <p>rCredits are community-created credits issued as incentive rewards in a new community-centered payment system. Get $20 for signing up and 10% back on anything you pay for with your rCard&reg;. Merchants get rewards too &mdash; and save a bundle on credit card fees. rCredits are intended as the basis for a Common Good Economy -- a democratic, community-centered economic system that puts people and planet first.</p>
    <p>We take transparency seriously &mdash; for true democratic control of our local economy we have to be able to see what's going on. Even if you are not a member, you can see what's happening in the rCredits economy graphically on the <a href="@BASE_URL/community">Community page</a>.</p>
    <p>The <a href="@BASE_URL/agreement">rCredits Agreement</a> is at the heart of the rCredits system. It is the promise we make together to accept rCredits that gives them value.</p>
    <p>Participation in rCredits is by invitation. Ask a friend to invite you or <a href="http://rcredits.org/">request an invitation here</a> and we will let you know when the Common Good Economy comes to your community.</p>
    <p>rCredits are a project of Common Good Finance&reg;, a 501(c)(3) Massachusetts-based nonprofit.</p>
EOF
  , compact('username'));

  if ($i == 'smartphone') return t(<<<EOF
    <p>To use the smartphone interface, you will need to install an app to read QR codes (for example QRDroid on Android phones or Qrafter for iPhones). You can pay participating businesses by scanning their QR code, then visiting the scanned URL.</p>
    <p>Your customers (or employer) can pay you by scanning the QR code on your member ID card. You can print an Advance ID card for your individual account, using the link on the Summary page.</p>
EOF
  );

  if ($i == 'sms') return tt("The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the <a href=\"@BASE_URL/sms\">Test SMS form</a> to simulate sending a text.");

  if ($i == 'summary') return tt(<<<EOF
    <p>This page shows an overview of your rCredits account.</p>
    <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region (@regName). The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
  <p><b>Balance</b> The total spendable amount in your rCredits account. You may also cash out all but the total amount of your rewards.</p>
  <p><b>Rewards</b> are the total of your signup bonus, adjustments for inflation, purchase rewards, and any other bonuses ever.</p>
  <p><b>Credit limit</b> increases a little at a time, based on your rCredits history.</p>
EOF
  , ['@regName' => $mya->cttyA->fullName]);

  if ($i == 'txs') return t(<<<EOF
    <p>Select the period you want to see transactions for. Both the Summary and Transactions sections will reflect just the transactions in that period. However, the "Pending" line will reflect all your not-yet-accepted invoices and offers EVER. The Ending balance in the Summary is always your current balance.</p>
    <p>If the status is "pending", either you or the other party will need to click "OK" to complete the deal. click the "X" to reject an invoice, dispute a charge, reject an offer, or cancel or undo something you did. click the pencil to edit the description. The description is just for you &mdash; the other party will not see it.</p>
    <p>The Rewards column shows how much rCredits were added to your account as a reward for joining or for buying or selling real goods and services. Loans, exchanges for US Dollars, and other purely financial deals do not earn rewards.</p>
EOF
  );

  if ($i == 'pay' and $whys = u\ray(R_WHYS)) return t(<<<EOF
    <p><b>Whom:</b> Type all or part of the full name of the account you want to pay. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID. Note that a period in the account ID (a regular account) is different from a colon (an agent acting on behalf of an account).</p>
    <p><b>Amount:</b> The amount of the transaction, measured in dollars, to be paid in rCredits.</p>
    <p><b>Purpose:</b> If this is a purely financial transaction, click "@forMoney" or "@forNongoods". Otherwise, a description is required (for purchases and sales of real goods and services). Donations to nonprofit organizations count as purchases of real services &mdash; services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank &mdash; your statements will call the purpose "cash".</p>
EOF
  , ['@forMoney'=>$whys[R_FOR_USD], '@forNongoods'=>$whys[R_FOR_NONGOODS]]);

  if ($i == 'directory') return t(<<<EOF
    <p>This is where you can find out who takes rCredits for what you want. click a business name to see its rCredits webpage.</p>
    <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you choose the United States "01" postal code region.</p>
EOF
  );

  if ($i == 'account/basic') return t(<<<EOF
    <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
EOF
  );

  if ($i == 'account/cells') return t(<<<EOF
    <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with rCredits. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
EOF
  );

  if ($i == 'account/relations') return t(<<<EOF
    <p>Relations let you connect individual rCredits participants to your account &mdash; as owners, employees, suppliers, or others, with limited or full access to the account. This is useful for authorizing sales clerks or setting up joint accounts. Or you might give your accountant "read" permission. Company accounts are managed only by an individual agent.</p>
    <p>Yellow highlights show settings that need to be confirmed &mdash; either by you or by the other party.</p>
    <p>To change settings in a Yes/No column, click + or -. To remove someone from the list, make them all "No"s, and no permissions (if they are already that way, change something and change it back). Be sure to "Save Changes".</p>
    <p>You can allow another account to "draw" from this one to avoid overdrafts (a 25 cent fee may apply).</p>
EOF
    ) . ($company ? t('<p>If you give the person at least "Sell" permission (and save the record), you can request a Member ID Card for them to use on behalf of this account.</p>') : '');
    
  if ($i == 'account/contact') return t(<<<EOF
    <p><b>Full name</b> should be your full legal name. If you change the name on the account, you will need to buy a new rCard and your account will be disabled until you supply new proof of identity. For companies, your rCredits website address will also change.</p>
    <p><b>Postal code:</b> Your postal code determines what rCredits "Common Good Community" you are in.</p>
    <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
EOF
  );

  if ($i == 'account/photo') return tt(<<<EOF
    <p>For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. If you have trouble coming up with a photo in a format that works, have a youngster take picture of you with their cell phone or tablet. Ask them to email you the picture, save that attachment to your desktop, then upload it here. If you still have trouble, email a description of your predicament to us at <a href="mailto:@regEmail?subject=photo trouble" target="_blank">@regEmail</a> and we will help.</p>
EOF
  , compact('regEmail')) . $companyAccount;

  if ($i == 'account/company') return tt(<<<EOF
    <p>All of the information from this form goes automatically on your <a target="_blank" href="@BASE_URL/member/@username">rCredits webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. click the link, to see the results (after saving your changes).</p>
EOF
  , compact('username'));

  if ($i == 'account/preferences') return t(<<<EOF
    <p>These settings start out as what most people prefer. Feel free to adjust them as you like. You can always come back and change them again later.</p>
    <ul>
      <li><b>Minimum.</b> You will want to keep enough in your account to cover the total amount you expect to spend at rCredits member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
      <li><b>Email notices.</b> We suggest you begin with daily notices, until you feel comfortable with the system. You will receive a "daily" notice only when there is something new that you will want to know about (such as a credit to your account).</li>
    </ul>
EOF
    );

  if ($i == 'issuing-rcredits') return showCharts('issued') . t(<<<EOF
    <p>rCredits are a type of credit. Like any medium of exchange, they represent value. They make it possible for us to trade goods and services with each other in our community, without having to trade something directly with whoever has what we want, and without having to give them something of precisely the same value ("barter"). The medium of exchange lets us give any amount of our goods or services to any participant in the economy, then receive the same value of goods or services from some other participant(s). If everyone both buys and sells, the credits move through the community in interconnected circles of exchange.</p>
    <p>Issuing rCredits is a community responsibility. This is the key to creating a Common Good Economy. We must issue as much credit as we need, to enable our exchanges of goods and services -- but no more than the value we as a community are capable of producing.</p>
    <p>Once rCredits become well-established and popular in a community, we can issue them together, with careful planning, as grants, zero-interest loans, and equity investments. Together we can fund whatever enterprises and activities we decide would be best -- for our community, for communities elsewhere, and for the world.</p>
    <p>In the meantime, until then, rCredits are issued automatically:</p>
    <ol>
      <li>as a stand-in for US Dollars, which are held unused in a separate account, and</li>
      <li>as incentives for doing what will strengthen and grow the system (signup bonuses, purchase rewards, inviter/helper rewards, and inflation offset).</li>
    </ol>
EOF
  ) . $back;
  
  if ($i == 'common-good-community') return tt(<<<EOF
    <p>The purpose of the rCredits system is to create and fund community-centered participatory democracy. A community that has met the <aRequirements>requirements</a> to become a Common Good Community may apply to Common Good Finance for certification. This certification permits the community to issue rCredits independently -- deciding for itself how many rCredits to issue (within safe limits) and what to fund with them.</p>

    <p>A Common Good Community is in "good standing" when it is issuing rCredits responsibly, so they accurately represent the potential of its members to produce corresponding goods and services in the short term.  A Common Good Community may lose its good standing if it runs a trade deficit with other Common Good Communities and fails to redeem it in a timely manner. Common Good Finance and the other communities will provide ongoing feedback, to prevent this from happening accidentally.</p>
EOF
  , ['!aRequirements' => 'a href="common-good-community-requirements"']) . $back;
  
  if ($i == 'cashflow-crunch') return t(<<<EOF
    <p>Once your geographic area becomes a Common Good Community, you can together decide to spend, grant, lend, and/or invest some of the US Dollars that members have put into their rCredits accounts, that are sitting unused. Of course you want to be careful not to use ALL of the US Dollars. From time to time, members will want to exchange some of their rCredits for US Dollars, to pay companies and individuals not yet participating in the rCredits system. There need to be enough US Dollars in reserve to cover the likely amount of those withdrawals.</p>

    <p>If you and your community misjudge and use too much of your US Dollar reserve, you will have a cashflow crunch until more US Dollars come in. There are many ways to handle this situation. For example, you might ask members who can, to add funds to their rCredits account. Or you might get a short-term loan from a traditional bank or credit union. It will be up to you and your community to choose what to do.</p>
EOF
  ) . $back;

  if ($i == 'backing-rcredits') return t(<<<EOF
    <p>rCredits are backed by all rCredits members together. Backing rCredits means you will accept at least <i>a limited amount</i> of them as payment for your goods and services (or trade US Dollars for them) even if, at that time, you cannot easily spend them or easily exchange them for US Dollars. For example, if your average balance over the past six months is $200, you agree to back rCredits up to $200. Then if your community has a <!aCrunch>cashflow crunch</a>, you have agreed to keep accepting rCredits as payment for your goods and services at least until your balance goes over $200.</p>

    <p>Like the Federal Reserve's backing for US Dollars held as bank deposits, backing for rCredits helps prevent unfounded fears from causing a "run on the bank". Since we all make this agreement together, we can rest assured that even in a crisis rCredits will maintain their value.</p>
EOF
  , ['!aCrunch' => 'a href="cashflow-crunch"']) . $back;

  if ($i == 'cashing-out') return t(<<<EOF
    <p>rCredits have value and act like money because we agree to accept them as payment for our good and services, so they move from member to member to member as a token of exchange. When someone pays you in rCredits, you can spend those funds &mdash; OR you can "cash them out" by transferring them to your bank account or exchanging them for cash.</p>
    <p>You can cash out rCredits that someone <i>pays</i> you, but any rCredits you <i>receive free</i>, as Incentive Rewards, must continue to circulate within the rCredits system. You cannot cash them out &mdash; you can only spend them.</p>
    <p>You can help keep the rCredits system healthy by maintaining a positive Primary Balance (most of the time) and treating your incentive rewards as a <!aReserve>Savings Reserve</a>.</p>
EOF
, ['!aReserve' => 'a href="savings-reserve"']) . $back;

  if ($i == 'savings-reserve') return t(<<<EOF
    <p><b>Background.</b> It is crucial to the success of the rCredits system that most of the rCredits issued as incentive rewards stay in the system and keep circulating in the community. While the system is small, it is a bit frail in that regard, so we need to be extra careful to make sure there is no "<!aCrunch>cashflow crunch</a>" at least until we are ready to handle it as a strong, well-functioning democratic rCredits community.</p>

    <p><b>Your Savings Reserve.</b> So we divide your account into two pieces: your Primary Balance and your Savings Reserve. Your incentive rewards go into the Savings Reserve automatically, to encourage you to keep them in the system until you need them. This is like a traditional reserve credit line that kicks in whenever you would otherwise overdraw your checking account &mdash; except there are no fees here. You can spend those reserved funds any time, simply by letting your Primary Balance go negative.</p>

    <p>For example, if you have $300 in your account and $100 of that is incentive rewards that you received (free), then your balance will show as $200, with a savings reserve of $100.</p>
    
    <p>Remember, we issue rCredits together as a commmunity, so it is up to all of us to protect their value. Most people choose to keep a positive Primary Balance, by setting a Minimum on the Preferences settings page. Keeping most of the rCredits we issue in the system, this helps keep the system healthy while it grows to become stable and permanent.</p>
EOF
  , ['!aCrunch' => 'a href="cashflow-crunch"']) . $back;
  
  if ($i == 'graceful-failure') return t(<<<EOF
     <p>Common Good Finance has put great care into designing the rCredits system so that it will NOT fail. Paradoxically, one requirement for its success is a detailed plan for a graceful shutdown. In the unlikely event that the whole rCredits system proves unworkable, the rCredits Agreement gives us a way to end without leaving anyone "holding the bag".</p>
     <p>As part of the rCredits Agreement, you promise to give back all the rCredits you received for free &mdash; as signup bonus, purchase rewards, etc. You can do that either</p>
     <ul><li>by continuing to accept rCredits until your balance is equal to your total incentive rewards (then returning them to the community) OR</li>
         <li>by substituting US Dollars for any missing rCredits.</li>
     </ul>
     <p>Complementarily, if you have more rCredits than you received as incentive rewards, you will be able to</p>
     <ul><li>continue to make purchases with them for a limited time OR</li>
         <li>trade them for other members' US Dollars that <i>they</i> returned to the community.</li>
     </ul>
     <p>We ALL make this agreement so if the system fails, everyone is protected and no one is loses what they put in.</p>
      <p>Once a Common Good Community has ongoing revenue to cover its expenses and any incentive rewards, it is no longer in any danger of ending abruptly. The system has been running successfully for many months now, to everyone's increasing benefit. So the long-term success of our pioneering Common Good Community in Greenfield, Massachusetts is looking more and more certain.</p>
EOF
  ) . $back;

  if ($i == 'enough-revenue') return t(<<<EOF
     <p>There is no magic formula (yet) for deciding how much revenue is enough to cover the incentive rewards. The important question is: who gets to decide such things?</p>
     <p>The rCredits system was invented for a single, critically important purpose: to create an opportunity for the community to take responsibility for its own economy. In a <!aCGC>Common Good Community</a> you decide, along with your friends and neighbors, how many rCredits to issue as incentive rewards, and how many as grants, loans, and investments. You decide what to incentivize:</p>
     <ul>
       <li>buying local?</li>
       <li>eating organic food?</li>
       <li>using renewable energy sources?</li>
       <li>creating works of art?</li>
     </ul>
     <p>Being able to discuss and decide together what to incentivize and what to fund means we decide together what to make happen in our world. The future is ours. Together we can provide a decent life for everyone in our community, offer a hand to people elsewhere, and do our part to rescue ourselves from the brink of climate-change disaster.</p>
     <p>Having enough revenue to cover the incentive rewards is a small step along that path. Nobody knows yet when we will have a fully-functioning Common Good Community, capable of making that judgment. One of our community organizers predicts it will be when there are about 3,000 rCredits members. Time will tell.</p>
     <p>In the meantime, you are encouraged to maintain a positive Primary Balance, treating your incentive rewards as a <!aReserve>Savings Reserve</a>, so those rCredits can keep circulating among the participants as much as possible, while we grow.</p>
EOF
  , ['!aCGC' => 'a href="common-good-community"',
     '!aReserve' => 'a href="savings-reserve"']) . $back;
  
  if ($i == 'ethics') return t(<<<EOF
    <p>A Common Good Community relies on its members to show a high level of ethical behavior, as the basis for creating a culture of trust and possibility. Of course it is always possible for someone to "game" the system and we will continue to develop software that brings such misbehavior to the attention of the community.</p>
    <p>Sometimes what looks like unethical behavior is in reality a desperate bid to escape from poverty. It is our responsibility as a community to think well about what our members need and want, and use the funding power of rCredits accordingly, so that such desperation is never necessary.</p>
EOF
  ) . $back;

  if ($i == 'dispute-resolution-process') return t(<<<EOF
    <p>Disputes between rCredits members will be resolved as follows:</p>
      <ol>
        <li>The members work together cooperatively in good faith to attempt to resolve the matter themselves, amicably.</li>
        <li>If, after 30 days, the members have been unable to agree on a resolution, at the request of either member the members attempt to resolve the matter creatively with the help of a mediator.</li>
        <li>Failing such resolution, at the initiative of either member, the members submit the matter to binding arbitration, as follows:
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
  ) . $back;

  if ($i == 'paying-attention') return t(<<<EOF
    <p>Common Good Finance and the rCredits system will give you regular feedback on how your actions impact your finances and the well being of the community. For this feedback to be effective, you have to pay attention to it. Please feel free to make suggestions to improve the quality or timing of the feedback.</p>
EOF
  ) . $back;

  if ($i == 'decision-principles') return t(<<<EOF
    <p>Each Common Good Community must decide for itself how to manage its decision making. Common Good Finance is developing recommendations for best practices, so that active participation by all members is valued and encouraged.</p>
EOF
  ) . $back;

  if ($i == 'other-decisions') return t(<<<EOF
    <p>From time to time, members will need to make a variety of non-financial decisions. Trustees need to be elected. Policies and procedures occasionally need to be revised. Other issues may arise.</p>
EOF
  ) . $back;

  if ($i == 'common-good-community-requirements') return t(<<<EOF
    <p>Requirements for certification as a Common Good Community are not yet finalized. They may include such things as qualified leadership, minimum size, experience using the rCredits system, diversity of participants, and successful completion of a training program.</p>
EOF
  ) . $back;

  if ($i == 'agreement') return t(<<<EOF
    <p>The rCredits Agreement is at the heart of the the rCredits system. It is our agreement to accept rCredits from each other that gives them value. Please read each section carefully and click the checkboxes to indicate your agreement. For more specific information, click the hyperlinks in the text.</p>
EOF
  );

  if ($i == 'donate') return t(<<<EOF
    <p>Your donation helps fund administration and promotion of the rCredits system during the pilot phase. It is also a formal "sealing" of your signature on the rCredits agreement.</p>
EOF
  );

  if ($i == 'account/proxies') return t(<<<EOF
    <p>During the pilot phase, it may be hard to choose two proxies from among the other members, unless you invite them to join first. You can change your selections at any time, so no worries.</p>
EOF
  );

  if ($i == 'invite') return t(<<<EOF
    <p>The rCredits system is designed to be community-centered, so our relationships with friends and neighbors in our own geographical community is important. We can work better together for the well being of everyone in our community and our region, if we know we can trust one another.</p>
    <p>Invite as many people as you like, but only people you trust.</p>
EOF
  );

  if ($i == 'transactions') return t(<<<EOF
    Hover over any option or column header for an explanation of what it means. Select some options, and the period or specific date range that interests you, then click either "Show" or "Download". The downloaded spreadsheet (in CSV format) has more complete information, including column totals.
EOF
  );

  if ($i == 'demand') return showCharts('banking') . tt(<<<EOF
    <p>The "demand" for rCredits is the net amount of US Dollars people want to trade for rCredits. As long as people have plenty of places to spend within the rCredits system, they will generally keep moving money in. So the demand gives us a pretty reliable measure of the system's soundness.</p>
    <p>We can keep issuing rCredits as long as there is a net inflow of US Dollars (more <b class="in">green</b> than <b class="out">orange</b> on the chart &mdash; an increasing demand). If members need to make more purchases outside the rCredits network and are taking more US Dollars <b class="out">out</b> of the system than they are putting <b class="in">in</b> (a decreasing demand), then we must temporarily stop <i>issuing</i> rCredits. Of course, we can keep <i>using</i> the rCredits already in circulation.</p>
    <p>Once people have more places to spend within the system (typically because more people and businesses have signed up) and they start moving money <b class="in">in</b> again, then we can begin again to issue additional rCredits.</p>
    <p>Transfers from an rCredits account <b class="out">to</b> a bank account often reflect an incomplete <a href="@BASE_URL/help/economic-circles">economic circle</a>. That is, the individual or company moving rCredits to a bank account does not yet have enough places to spend them. Generally, this means they are spending them outside the community (and outside the rCredits system). This is an opportunity for import-replacement &mdash; starting new local businesses (or expanding existing businesses) to supply goods or services not currently produced in our region.</p>
    <p>How <b style="color:red;">low</b> our total monthly volume of <b class="out">outward</b> transfers is, is a measure of how well we are doing as a community in our progress toward a sustainable local economy.</p>
    <p>As an individual, you can help by finding ways to spend within the rCredits system, instead of transferring your funds to a bank account. For all the money that circulates within the system, we can use rCredits, freeing up that many US Dollars to use for something else. So, in effect, when you can keep money in the system, we are all that much richer.</p>
EOF
  ) . $back;

  if ($i == 'growth') return showCharts('accts') . tt(<<<EOF
    <p>The real power and purpose of the rCredits system is in our gathering as a community to decide what our funding priorities should be &mdash;</p>
    <ul>
      <li>for sustainable agriculture and energy systems,</li>
      <li>for supporting local businesses, local entrepreneurs, and local self-reliance,</li>
      <li>for ensuring that everyone has healthy food, a home, health care, cultural enrichment, and satisfying work</li>
    </ul>
    <p>&mdash; then issuing rCredits to fund our decisions.</p>
    <p>That community-level sovereignty requires <i>broad participation</i>. The green line shows how many we are today. Membership in rCredits is by invitation, so we can grow quickly toward full community-centered economic democracy, while building a network of trust.</p>
    <p>Almost everything is automated, once your account is set up. The one labor-intensive piece is opening the account.</p>
    <p>You can help! <a href="@BASE_URL/community/invite">Click here to invite</a> your friends and local businesses where you want to spend your rCredits. Sit with your friends while they sign up &mdash; it takes only half an hour, but without a nudge most people spend several weeks putting it off.</p>
EOF
  ) . $back;

  if ($i == 'funds') return showCharts('funds') . tt(<<<EOF
    <p>There are two kinds of funds in the rCredits system:</p>
    <ol>
      <li><b style='color:white; background-color:darkgreen;'>rCredits</b>. These are the funds that we issue together as a community, by agreeing to accept them as payment for our goods and services. The chart distinguishes between two categories of rCredits*:
      <ul>
        <li><b style='color:black; background-color:yellow;'>rCredits that sit</b>. You keep some rCredits in your account, unspent for now. Maybe you plan to spend them next month. Or maybe you are holding them in your rCredits account as a hedge against inflation. Either way, we call funds that are unused for a month or more "savings". The accounts with the 5 highest savings amounts are totaled and shown separately as well**.</li>
        <li><b>rCredits that circulate</b> (green above the yellow line). Members use the remaining rCredits every month, to make purchases from other members (check the <a href="@BASE_URL/help/velocity">Velocity graph</a>, to see what fraction of the rCredits are used each month).</li>
      </ul></li>
      <li><b style='color:white; background-color:darkblue;'>US Dollars</b> (which currently just sit). When you transfer funds from bank account to your rCredits account or when you trade US Dollars for rCredits at a member business, that money goes into the community's US Dollar bank account. The system issues you an equivalent amount of rCredits to spend or hold in your account. For now, the US Dollars just accumulate, unused. Once we have enough active participation to establish a Common Good Community, we can decide together how best to use that money &mdash; for grants, zero-interest loans, and equity investments for sustainability and the common good.</li>
    </ol>
    <p>The total funds in the rCredits system (the total under our control as a community) is the Sum of the rCredits (circulating or sitting) and the US Dollars.</p>
    <p>* The distinction between "savings" and rCredits that are actually being used is important. The value of rCredits, like any currency, is what you can get for them. When you save your rCredits funds for spending a month or more in the future, it probably means you are confident (1) that you will be able to spend them within the rCredits system OR (2) that you will be able to trade them for US Dollars because other members can spend them within the system. So savings are a <em>speculative</em> <a href="@BASE_URL/help/demand">demand</a> for rCredits, rather than a demand based in current reality. The speculation may indeed be accurate, but <a href='velocity'>circulation velocity</a> and <a href='demand'>net monthly influx of US Dollars</a> are stronger measures of the system's capacity for additional rCredits.</p>
    <p>** <b style='color:white; background-color:darkred;'>Top 5</b>. Balanced participation among many members maximizes the system's predictability and protects against a cashflow crunch. Expect the "Top 5" line to level off as the system grows.</p>
EOF
  ) . $back;

  if ($i == 'velocity') return showCharts('velocity') . t(<<<EOF
    <p>In our society at large, how fast the money moves is an indicator of the health of our economy. Faster velocity implies more people spending more money per month.</p>
    <p>More spending is not necessarily a good thing, if we all already have what we need. But when there are unmet needs, increased economic activity can be a sign of things getting better. In the rCredits system, economic activity means people are finding the rCredits system useful, which is always a good thing.</p>
    <p>Notice that the faster money circulates, the less money we need, to buy the same stuff. Velocity is a money-stretcher! For example, suppose there are $1 million worth of rCredits in circulation. Let's say the velocity is 1 in April (the whole million gets spent) and 2 in May (the whole million gets spent twice). In May we used the same amount of money to buy twice as much.</p>
    <p>For comparison, the velocity of M2 money stock in the United States over the past half century has fluctuated between .25 and .45 per month (as you can see on the chart, rCredits circulation velocity is in that same range).
EOF
  ) . $back;

  if ($i == 'economic-circles') return showCharts('tx') . t(<<<EOF
    <p>Economic circles are crucial to a healthy regional economy. The more we can keep money circulating locally, the richer we are. Why? Because we can use that money over and over &mdash; in effect turning each dollar into millions.</p>
    <p>In a typical economic circle, you buy something from a local store or pay for local services. That business then pays its local suppliers and its local employees. Those employees in turn become consumers like you &mdash; they buy something from a local store, etc. When at any point in this circle we lose the "local" and pay someone or some company outside our region, that money is gone. It has leaked out of our community and we are that much poorer.</p>
    <p>The rCredits system gives us an easy way to track our economic circles and opportunities for improvement. For now, "local" means within the rCredits system, but we expect the definition to expand as rCredits membership continues to grow.</p>
    <p>This chart shows four types of transactions. For strong economic circles, we should see the volume of person-to-business (<b class='p2b'>p2b</b>) transactions matching the volume of business-to-person (<b class='b2p'>b2p</b>) transactions. More <b class='p2b'>p2b</b> than <b class='b2p'>b2p</b> means business are not paying enough local employees (or other local businesses). More <b class='b2p'>b2p</b> than <b class='p2b'>p2b</b> means people are shopping elsewhere. As you can see, member businesses are just beginning pay their employees in rCredits (see our post on <a href="http://rc4.me/fb">FaceBook</a> about solving the payroll problem).</p>
    <p>The <a href="demand">Monthly Bank Transfers</a> chart also measures how well we are doing as a community, at keeping money in the system.</p>
EOF
  ) . $back;

  if ($i == 'your-return') return tt(<<<EOF
    <p>Your effective return is calculated as your monthly inflation adjustment (currently based on @R_INFLATION_RATE% APR) plus any incentive rewards you got in the period, all divided by your average balance for the period. The result is then annualized to give you an APR (annual percentage rate). Regular financial institutions give you interest on your <em>minimum</em> balance for the month, which is nearly always lower than your <em>average</em> balance.</p>
EOF
  ) . $back;

  if ($i == 'social-return') return tt(<<<EOF
    <p>The community's financial benefit from your participation is very loosely calculated as the sum of:</p>
    <ul>
      <li><b>Donations.</b> Your gifts to Common Good Finance (including shared rewards).</li>
      <li><b>Money in.</b> The amount of money you moved into the system, minus the amount you moved out (but not less than zero). By using rCredits in place of those funds, as they circulate among rCredits members, we can free up that much US Dollars for the community to grant, lend, or invest.</li>
      <li><b>Rewards.</b> The amount of your incentive rewards for buying and selling &mdash; reflecting the value the community places on your using rCredits for purchases and sales.</li>
      <li><b>Community funding.</b> Your per-capita share of the funds the community has already granted, loaned, or invested. The continued viability of that funding is supported by your participation. (counted as part of social benefits ever, but not in the past month)</li>
      <li><b>Card fees saved.</b> A percentage (@R_CC_RATE%) of your purchase and sale amounts &mdash; the typical processing fee that businesses pay for credit card purchases. Together with the other party to the transaction, you kept that much money from leaking out of the community to the big banks.</li>
    </ul>
    </p>
    <p><b>Control.</b> Of course your participation benefits the community far beyond mere money. The commmunity benefits by all of us together having greater control over our local economy, so we can plan and decide together how to use the power of money to make a better life for everyone.</p>
EOF
  ) . $back;

  if ($i == 'taxes') {
    $taxhead = u\ray('Tax Form:;1120;990;990EZ;990-A;1040;1040A;1040 Sched C');
    $taxline[] = u\ray('Sales / Other Income,1a,varies,varies,varies,7 or 21 ("barter"),7,1');
    $taxline[] = u\ray('Inflation Adjustment,5 or 10,VIII(3)D,I(4),II(B)8,8a or 21,7 or 8a,6');
    $taxline[] = u\ray('rCredits Incentive Rewards,10,VIII(3)D,I(4),II(B)8,21,7,6');
    $taxline[] = u\ray('"Sharing" Donations,19,IX(1),I(10),N/A,,Subtract from Rewards,4; 8; 10; 17; or 27a');
    $taxes = '<tr><th class="cat">' . join('</th><th>', $taxhead) . "</th></tr>\n";
    foreach ($taxline as $line) $taxes .= '<tr><td class="cat">' . join('</td><td>', $line) . "</td></tr>\n";
    $taxes = strtr($taxes, ['<td></td><td>' => '<td colspan="2">', ';' => ',']);
    
    return t(<<<EOF
  <p>Pay taxes on rCredits income just as you would for US Dollar income, including income, excise, and sales taxes.</p>

  <p>The chart below suggests some tax form lines you might choose, for reporting your rCredits activity. It also suggests how you might report your monthly tax-deductible "sharing" contributions to Common Good Finance.</p>

  <p>If you follow the <@a1>accounting practices recommended</a> by Common Good Finance, your software already includes your rCredits income, properly categorized, in your accounting records and reports.</p>

      <table id="taxes">
$taxes
      </table>
EOF
    , ['@a1' => 'a href=http://commongoodfinance.org/files/rCreditsAccountingTips.pdf target=_blank']);
  }
  
  if ($i == 'flags') {
    include_once __DIR__ . '/risk-descs.inc';
    extract($riskHelp, EXTR_PREFIX_ALL, 'h');
    $acctRisks = $txRisks = $special = '';
    foreach (u\ray(K_ACCT_RISKS) as $k => $weight) {
      $extra = '';
      if ($k == 'trusted') $extra = t('This risk factor is multiplied by the number of other accounts that trust this member. Being chosen as an alternate proxy counts as half. Likewise being chosen as someone\'s proxy\'s proxy counts as half.');
      if ($k == 'socialConx' or $k == 'badConx') $extra = t('This risk factor is multiplied by the number of such connections.');
      if ($k == 'geography') $extra = t('Federal and state law enforcement agencies have identified 28 High Intensity Drug Trafficking Areas (HIDTAs), which include 16 percent of all counties in the United States and 60 percent of the U.S. population. For example, Washtenaw County, Michigan has been designated as part of the Michigan HIDTA. Since so much of the population qualifies, the risk weight is low.');
      if ($k == 'new') $extra = t('This risk factor is divided by the number of years the member (individual or company) has been at its current address.');
      if ($k == 'moves') $extra = t('This risk factor is multiplied by the number of times the member has moved, over the past decade (if known)');
      if ($k == 'bigDay') $extra = t('For individual accounts, the number of "employees" is 1.');
      if (in_array($k, array('bigWeek', 'big7Week', 'bigYear'))) $extra = t('(see bigDay)');
      $acctRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $special = $line;
    }
    foreach (u\ray(K_TX_RISKS) as $k => $weight) {
      $extra = '';
      if ($k == 'origins') $extra = t('This risk factor is multiplied by the average risk of the payer\'s receiving transactions over the past year.');
      if (in_array($k, u\ray('fromSuspect toSuspect suspectOut suspectIn'))) $extra = tt('This risk factor is multiplied by the suspicious account risk, divided by the "unusually large" threshold ($@K_THRESHOLD).');
      if ($k == 'invoiceless') $extra = t('And no mention of an invoice in the transaction description.');
      $txRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $special = $line;
    }

    $acctRisks = t('<h2>Account Flags</h2>') . "\n<table>$acctRisks</table>\n";
    $txRisks = t('<h2>Transaction and ACH Flags</h2>') . "\n<table>$txRisks</table>\n";
    $score = number_format(K_RED*300*8/15/K_THRESHOLD, 0);
    $score = "<span class=\"badRisk\">$score</span>";
//    if ($special) $special = preg_replace('/^(.*)\[(.*)\](.*)/', '<div id="special-flag"><big style="color:red;">$1</big> (weight:$2) $3</div>', $special);
    return tt(<<<EOF
      <div id="risk-help">
      <table id="special-flag">@special</table>
      <p>Suspicious Activity Risk Scores (flags) are calculated based on a weight divisor assigned to each type of risk. That divisor (shown in square brackets for each item below) is an assessment of how many same-sized risks would together make an account or transaction suspicious -- a higher divisor means less risk. <span class="goodRisk">Risk factors with a negative divisor</span> (shown in green) are mitigating.</p>
      <p>Risk scores are then proportional to the sum of the inverses of those weight divisors - a higher score means more risk. For transactions, the score is further multiplied by the transaction amount and divided by the "unusually large" threshold ($@K_THRESHOLD). All risk scores (whether for accounts, transactions, or ACHs) are finally multiplied by @K_RED and rounded to the nearest integer. So @K_RED is the minimum "suspicious" score.</p>
      <p>For example, a $300 transaction with two risk factors having weight divisors of 3 and 5 respectively, would be scored 100 * 300 * (&#8531; + &#x2155;) / @K_THRESHOLD = @score (asterisk here means multiply, slash means divide).</p>
EOF
    , compact('score', 'special')) . $acctRisks . $txRisks . '</div risk-help>';
  }
}

function riskHelpLine($k, $weight, $extra = '', $tag) {
  global $riskHelp;
  $basic = u\uc1($riskHelp[$k]);
//  $class = $tag == $k ? 'tagged' : ($weight < 0 ? 'goodRisk' : 'badRisk');
  $class = $weight < 0 ? 'goodRisk' : 'badRisk';
  return "<tr><td class=\"$class\">$k</td><td class=\"weight\">[$weight]</td><td>$basic. $extra</td></tr>\n";
}

/*
<p>"$/mo" is the most important column. Type the amount you pay each employee or supplier in a month &mdash; more or less. Then, if you select the payment exchange option on the preferences page, your rCredits will get distributed every night to those employees and suppliers in exchange for US Dollars.</p>

    'why-wait') return t(<<<EOF
    <p>Exchanges of rCredits for US Dollars reflect an incomplete economic circle. That is, the individual or company selling the rCredits does not yet have enough places to spend them.</p>
    <p>The 2-week delay in spending rCredits that you buy takes that amount out of circulation temporarily, giving the seller time to find more places to spend rCredits, without meanwhile having to sell more and more of them.</p>
EOF
    ),
    
      <p>If you are acting as an agent for someone else's account &mdash; for example if you are a sales clerk for a department store &mdash; the period will be a colon instead. Your company ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) identifies both you and the company.</p>
*/