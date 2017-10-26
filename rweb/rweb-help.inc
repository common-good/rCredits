<?php

/*
 * @file
 * rCredits Help Messages
 * This file contains a single function that returns a help message.
 */
 
namespace rCredits\Web;
use rCredits as r;
use rCredits\Util as u;
use rCredits\Web as w;

/**
 * Return the appropriate raw help text.
 * @param string $i: identifier for the appropriate help text.
 * @param string $tag: (optional) item to highlight)
 * @param string $args: other URL arguments
 */
function rawHelp($i, $tag = '', $args = '') {
  global $rUrl;
  
  extract(just('gifts years', $args)); // must precede eLinkAcct
  $mya = w\eLinkAcct($zot, $args, 'any');
  
  $ctty = @$mya->community ?: 0;
  $company = @$mya->co;
  $username = @$mya->name; // @ is for SMS testing (when not signed in)
  $cttyName = @$mya->cttyA->fullName;
  
  $site = str_replace('https://', '', BASE_URL);
  $_aPage = w\atag("/member/$username", w\away());
  $_aInfo = w\atag('account/company');
  $regEmail = r\regionField('email');
 
  $companyAccount = $company ? '' : t(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic %PROJECT webpage: <%aPage>%site/member/%username</a> &mdash; so choose wisely. See also the <%aInfo>Company Info</a> form.</p>
EOF
  , compact('_aPage', '_aInfo', 'username', 'site'));
  
  //Help is also available for the <a href="%BASE_URL/help/sms">Text Messaging Interface</a>.
  $back = '<p>' . u\backButton() . '</p>';
  if ($i == 'general') return t(<<<EOF
    <p>Here's what to click, to find what you want:</p>
    <ul>
      <li><b>Summary</b>: See an overview of your account or open a %PROJECT Account for your company.</li>
      <li><b>Charge</b>: Invoice another participant.</li>
      <li><b>Pay</b>: Transfer %RCREDITS to another participant.</li>
      <li><b>Bank</b> Transfer money to or from your bank account.</li>
      <li><b>History</b>: See a list of your recent transactions, invoices, notices, monthly statements, or annual tax forms (Form 1099-B).</li>
      <li><b>Community</b>. See how the %PROJECT economy is doing in your community. Review the %PROJECT Agreement, signed by all %PROJECT members. Find a participating company. Help the %PROJECT community grow by making a financial contribution or by inviting someone.</li>
      <li><b>Settings</b>. Change your account settings, including contact information, general preferences, security settings, bank account connection, relations with other accounts, and proxy choices.</li>
      <li><b>Photo (Choose an Account to Manage)</b>: Click your photo in the upper right corner, to select an account to manage, if you have more than one.</li>
    </ul>
    <p>If you don't see the help you need here, please <%aEmail>email us</a> or use this form to send a secure encrypted message (if your message is secret).</p>
EOF
  , '_aEmail', u\emailTag($regEmail, t('help with %PROJECT')));

  if ($i == 'other') return t(<<<EOF
    <h2>Need Help Signing In?</h2>
    <p>If your username and password aren't working, no worries. Just click "<%aForgot>Forgot it</a>" here or on the sign-in page, to get an email for choosing a new password.</p>
    <h2>About %RCREDITS&reg;</h2>
    <p>%RCREDITS is a credits/debit card and payment system to support the common good. It is also a %PROJECT Economics Toolkit, to help you and people everywhere create a democratic, community-centered economic system that puts people and planet first -- one community at a time.</p>
    <p>We take transparency seriously &mdash; for true democratic control of our local economy we have to be able to see what's going on. Even if you are not a member, you can see what's happening in the %PROJECT economy graphically on the <%aCtty>Community page</a>.</p>
    <p>The <%aAgree>%PROJECT Agreement</a> is at the heart of the %PROJECT system. It is the promise we make together to accept %RCREDITS that gives them value.</p>
    <p>Participation in %PROJECT is by invitation. Ask a friend to invite you or <%aPromo>request an invitation here</a> and we will let you know when the %PROJECT Economy comes to your community.</p>
    <p>%PROJECT is a 501(c)(3) Massachusetts-based nonprofit.</p>
EOF
  , 'username _aForgot _aCtty _aAgree _aPromo', $username, w\atag("/account/password/%username"), w\atag('/community'), w\atag('/agreement'), w\atag(PROMO_URL));

  if ($i == 'smartphone') return t(<<<EOF
    <p>To use the smartphone interface, you will need to install an app to read QR codes (for example QRDroid on Android phones or Qrafter for iPhones). You can pay participating businesses by scanning their QR code, then visiting the scanned URL.</p>
    <p>Your customers (or employer) can pay you by scanning the QR code on your member ID card. You can print an Advance ID card for your individual account, using the link on the Summary page.</p>
EOF
  );

  if ($i == 'sms') return t('The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the <%aSMS>Test SMS form</a> to simulate sending a text.', '_aSMS', w\atag('/sms'));

  if ($i == 'summary') return t(<<<EOF
    <p>This page shows an overview of your %PROJECT Account.</p>
    <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region (%regName). The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
  <p><b>Balance</b> is the amount in your %PROJECT Account.</p>
  <p><b>Credit Reserve</b> is how far negative your account balance is allowed to go. Your credit limit increases a little at a time, based on your how much you receive and spend each month and how many friends and neighbors you invite (who then sign up).</p>
EOF
  , 'regName', $cttyName);

  if ($i == 'txs') return t(<<<EOF
    <p>Select the period you want to see transactions for. Both the Summary and Transactions sections will reflect just the transactions in that period. However, the "Pending" line will reflect all your not-yet-accepted invoices and offers EVER. The Ending balance in the Summary is always your current balance.</p>
    <p>If the status is "pending", either you or the other party will need to click "OK" to complete the deal. click the "X" to reject an invoice, dispute a charge, reject an offer, or cancel or undo something you did. click the pencil to edit the description. The description is just for you &mdash; the other party will not see it.</p>
EOF
  );

  if ($i == 'pay' and $whys = ray(R_WHYS)) return t(<<<EOF
    <p><b>Whom:</b> Type all or part of the full name of the account you want to pay. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID.</p>
    <p><b>Amount:</b> The amount of the transaction, measured in dollars, to be paid in %RCREDITS.</p>
    <p><b>Purpose:</b> If this is a purely financial transaction, click "%forMoney" or "%forNongoods". Otherwise, a description is required (for purchases and sales of real goods and services). Donations to nonprofit organizations count as purchases of real services &mdash; services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank &mdash; your statements will call the purpose "cash".</p>
EOF
  , 'forMoney forNongoods', $whys[FOR_USD], $whys[FOR_NONGOODS]);

  if ($i == 'directory') return t(<<<EOF
    <p>This is where you can find out who takes %RCREDITS for what you want. click a business name to see its %PROJECT webpage.</p>
    <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you choose the United States "01" postal code region.</p>
EOF
  );

  if ($i == 'account/basic') return t(<<<EOF
    <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
EOF
  );

  if ($i == 'account/cells') return t(<<<EOF
    <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with %RCREDITS. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
EOF
  );

  if ($i == 'account/relations') return t(<<<EOF
    <p>Relations let you connect individual %PROJECT participants to your account &mdash; as owners, employees, suppliers, or others, with limited or full access to the account. This is useful for authorizing sales clerks or setting up joint accounts. Or you might give your accountant "read" permission. Company accounts are managed only by an individual agent.</p>
    <p>Yellow highlights show settings that need to be confirmed &mdash; either by you or by the other party.</p>
    <p>To change settings in a Yes/No column, click + or -. To remove someone from the list, make them all "No"s, and no permissions (if they are already that way, change something and change it back). Be sure to "Save Changes".</p>
    <p>You can allow another account to "draw" from this one to avoid overdrafts (a 25 cent fee may apply).</p>
EOF
    ) . ($company ? t('<p>If you give the person at least "Sell" permission (and save the record), you can request a Member ID Card for them to use on behalf of this account.</p>') : '');
    
  if ($i == 'account/contact') return t(<<<EOF
    <p><b>Full name</b> should be your full legal name. If you change the name on the account, you will need to buy a new %PROJECT Card and your account will be disabled until you supply new proof of identity. For companies, your %PROJECT website address will also change.</p>
    <p><b>Postal code:</b> Your postal code determines what %PROJECT Community you are in.</p>
    <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
EOF
  );

  if ($i == 'account/photo') return t(<<<EOF
    <p>For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. If you have trouble coming up with a photo in a format that works, have a youngster take picture of you with their cell phone or tablet. Ask them to email you the picture, save that attachment to your desktop, then upload it here. If you still have trouble, email a description of your predicament to us at <%a>%regEmail</a> and we will help.</p>
EOF
  , '_a regEmail', u\emailTag($regEmail, t('photo trouble'))) . $companyAccount;

  if ($i == 'account/company') return t(<<<EOF
    <p>All of the information from this form goes automatically on your <%aPage>%PROJECT webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. click the link, to see the results (after saving your changes).</p>
EOF
  , 'username _aPage', $username, w\atag("/member/$username"));

  if ($i == 'account/preferences') return t(<<<EOF
    <p>These settings start out as what most people prefer. Feel free to adjust them as you like. You can always come back and change them again later.</p>
    <ul>
      <li><b>Target.</b> You will want to keep enough in your account to cover the total amount you expect to spend at %PROJECT member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
      <li><b>Email notices.</b> We suggest you begin with daily notices, until you feel comfortable with the system. You will receive a "daily" notice only when there is something new that you will want to know about (such as a credit to your account).</li>
    </ul>
EOF
    );

  if ($i == 'issuing-credits') return showCharts('issued', $ctty) . t(<<<EOF
    <p>%RCREDITS are a type of credit. Like any medium of exchange, they represent value. They make it possible for us to trade goods and services with each other in our community, without having to trade something directly with whoever has what we want, and without having to give them something of precisely the same value ("barter"). The medium of exchange lets us give any amount of our goods or services to any participant in the economy, then receive the same value of goods or services from some other participant(s). If everyone both buys and sells, the credits move through the community in interconnected circles of exchange.</p>
    <p>Issuing %RCREDITS is a community responsibility. This is the key to creating a %PROJECT Economy. We must issue as much credit as we need, to enable our exchanges of goods and services -- but no more than the value we as a community are capable of producing.</p>
    <p>Once %RCREDITS become well-established and popular in a community, we can issue them together, with careful planning, as grants, zero-interest loans, equity investments, and incentives for doing the right thing. Together we can fund whatever enterprises and activities we decide would be best -- for our community, for communities elsewhere, and for the world.</p>
    <p>In the meantime, until then, %RCREDITS are issued automatically:</p>
    <ol>
      <li>as a stand-in for US Dollars, which are held unused in a separate account, and</li>
      <li>as an increase in your credit line (or in some %PROJECT communities &mdash; as tentative incentive rewards*) to encourage doing what makes the system stronger.</li>
    </ol>
    <p>* Any incentive rewards you receive (signup bonuses, purchase rewards, inviter/helper rewards, and inflation offset) are yours to keep once your %PROJECT community says so. Meanwhile you can borrow them interest-free.</p>
EOF
  ) . $back;
  
  if ($i == 'common-good-community') return t(<<<EOF
    <p>The purpose of the %PROJECT system is to create and fund community-centered participatory democracy. In a %PROJECT Community* you decide together how many %RCREDITS to issue (within safe limits) and what to fund with them.</p>
    
    <p>A %PROJECT Community is in "Good Standing" when it is issuing %RCREDITS responsibly, so they accurately represent the potential of its members to produce corresponding goods and services in the short term. A %PROJECT Community may lose its good standing if it runs a trade deficit with other %PROJECT Communities and fails to redeem it in a timely manner. %PROJECT staff and the other %PROJECT Communities will provide ongoing feedback, to prevent this from happening accidentally.</p>
    <hr class="narrow">
    <p><small>* There are no specific requirements for a group of members to become a sovereign %PROJECT Community. But <%aClearness>these Clearness Questions</a> will help you get ready for that responsibility.</small></p>
EOF
  , '_aClearness', w\atag('https://docs.google.com/document/d/1-lBTNHTxIPrTSrIK18ZdbcMeEPS5g5jFhST34VGkR8E')) . $back;
  
  if ($i == 'cashflow') return t(<<<EOF
    <p>As a %PROJECT Community you and other members decide together when to spend, grant, lend, and/or invest some of the US Dollars members have put into their %PROJECT accounts, which are held in a Community Escrow Fund in a bank.</p>
    
    <p>Your %PROJECT Community must take care to keep enough money in the Community Escrow Fund to cover requests by members to <%aCashout>move funds to their bank account</a>, trading some of their %RCREDITS for US Dollars.</p>
    
    <p>If you misjudge and use too much of the Community Escrow Fund, you will have a short-term cash flow challenge until more US Dollars come in. The "<%aBack>backing</a>" clause in the %PROJECT Agreement provides stability, so your %PROJECT Community can handle such challenges smoothly.</p>
EOF
  , '_aCashout _aBack', w\atag('cashing-out'), w\atag('backing-common-good-credits')) . $back;

  if ($i == 'backing-common-good-credits') return t(<<<EOF
    <p>To ensure that %RCREDITS maintain their value, you (and every other member) agree to accept however many you typically spend and/or receive in a month, even if your %PROJECT Community is unable to redeem them immediately for US Dollars.</p>

    <p>Like the Federal Reserve's backing for US Dollars held as bank deposits, backing for %RCREDITS helps prevent unfounded fears from causing a "<%aCrunch>run on the bank</a>". All members agree to back %RCREDITS so your %PROJECT Community can continue to run smoothly.</p>
EOF
  , '_aCrunch', w\atag('cashflow')) . $back;

  if ($i == 'cashing-out') return t(<<<EOF
    <p>%RCREDITS have value and act like money because we agree to accept them as payment for our good and services, so they move from member to member to member as a token of exchange. When someone pays you in %RCREDITS, you can spend those funds &mdash; OR you can "cash them out" by transferring them to your bank account or exchanging them for US Dollars.</p>
    <p>You can cash out %RCREDITS that someone <i>pays</i> you, but any %RCREDITS you borrow from your Credit Reserve must continue to circulate within the %PROJECT system. You cannot cash them out &mdash; you can only spend them.</p>
    <p>You can help keep the %PROJECT system healthy by maintaining a positive balance most of the time.</p>  
EOF
, '_aReserve', w\atag('credit-reserve')) . $back;

/*  if ($i == 'credit-reserve') return t(<<<EOF
    <p><b>Background.</b> It is crucial to the success of the %PROJECT system that most members maintain a positive account balance, so there are some %RCREDITS to circulate in the community. While the system is small, it is a bit frail in that regard, so we need to be extra careful to make sure there is no "<%aCrunch>cash flow crunch</a>" at least until we are ready to handle it as a strong, well-functioning democratic %PROJECT community.</p>

    <p><b>Your Credit Reserve.</b> So we divide your account into two pieces: your Balance and your Credit Reserve. Your incentive rewards go into the Credit Reserve, to encourage you to keep them in the system until you need them. This is like a traditional reserve credit line that kicks in whenever you would otherwise overdraw your checking account &mdash; except there are no fees here. You can spend those reserved funds any time, simply by letting your Balance go negative.</p>

    <p>For example, if you have $300 in your account and $100 of that is incentive rewards that you received (free), then your balance will show as $200, with a Credit Reserve of $100.</p>
    
    <p>Remember, we issue %RCREDITS together as a commmunity, so it is up to all of us to protect their value. Most people choose to keep a positive Balance, by setting a Target Balance on the Preferences settings page. Keeping most of the %RCREDITS we issue in the system helps the system stay healthy while it grows more and more robust.</p>
EOF
  , '_aCrunch', w\atag('cashflow')) . $back; */
  
  if ($i == 'closing-down') return t(<<<EOF
     <p>%PROJECT staff and advisers have put great care into designing the %PROJECT system so that it will continue to flourish. In the unlikely event the system needs to close down, the %PROJECT Agreement gives us a way to end gracefully.</p>
     <p>As part of the %PROJECT Agreement, you promise to bring your balance up to zero if it is negative. You can either:</p>
     <ul><li>continue to accept %RCREDITS until you are square OR</li>
         <li>trade US Dollars to another member in exchange for %RCREDITS OR</li>
         <li>transfer US Dollars from your bank account.</li>
     </ul>
     <p>Complementarily, if the system ends and you have a positive balance, you can:</p>
     <ul><li>continue to make purchases with your credits for a limited time OR</li>
         <li>trade them to another member in exchange for US Dollars OR</li>
         <li>transfer them to your bank account.</li>
     </ul>
     <p>We all make this agreement so if the system closes down, everyone is protected.</p>
EOF
  ) . $back;

/*  if ($i == 'ready') return t(<<<EOF
     <p>There is no magic formula (yet) for deciding when a Common Good Community is ready to allow cashing out incentive rewards. The important question is: who gets to decide such things?</p>
     <p>The %PROJECT system was invented for a single, critically important purpose: to create an opportunity for the community to take responsibility for its own economy. In a <%aCGC>Common Good Community</a> you decide, along with your friends and neighbors, how many %RCREDITS to issue as incentive rewards, and how many as grants, loans, and investments. You decide what to incentivize:</p>
     <ul>
       <li>buying local?</li>
       <li>eating organic food?</li>
       <li>using renewable energy sources?</li>
       <li>creating works of art?</li>
     </ul>
     <p>Being able to discuss and decide together what to incentivize and what to fund means we decide together what to make happen in our world. The future is ours. Together we can provide a decent life for everyone in our community, offer a hand to people elsewhere, and do our part to rescue ourselves from the brink of climate-change disaster.</p>
     <p>Having enough throughput to be able to issue incentive rewards is a small step along that path. Nobody knows yet when we will have a fully-functioning Common Good Community, capable of making that judgment. One of our community organizers predicts it will be when there are about 3,000 %PROJECT members. Time will tell.</p>
     <p>In the meantime, you are encouraged to maintain a positive account balance, treating your incentive rewards (if any) as a <%aReserve>Credit Reserve</a>, so those %RCREDITS can keep circulating among the participants as much as possible, while we grow.</p>
EOF
  , '_aCGC _aReserve', w\atag('common-good-community'), w\atag('credit-reserve')) . $back; */
  
  if ($i == 'ethics') return t(<<<EOF
    <p>A %PROJECT Community relies on its members to show a high level of ethical behavior, as the basis for creating a culture of trust and possibility. Of course it is always possible for someone to "game" the system and we will continue to develop software that brings such misbehavior to the attention of the community.</p>
    <p>Sometimes what looks like unethical behavior is in reality a desperate bid to escape from poverty. It is our responsibility as a community to think well about what our members need and want, and use the funding power of %PROJECT accordingly, so that such desperation is never necessary.</p>
EOF
  ) . $back;

  if ($i == 'dispute-resolution-process') return t(<<<EOF
    <p>Disputes between %PROJECT members will be resolved as follows:</p>
      <ol>
        <li>The members work together cooperatively in good faith to attempt to resolve the matter themselves, amicably.</li>
        <li>If, after 30 days, the members have been unable to agree on a resolution, at the request of either member the members attempt to resolve the matter creatively with the help of a mediator.</li>
        <li>Failing such resolution, at the initiative of either member, the members submit the matter to binding arbitration, as follows:
          <ol style='list-style-type:lower-alpha;'>
            <li>Either member will initiate the dispute resolution process by sending a registered letter to the other at their address of record in which they appoint a representative and include the representative's full contact information. The representative should be familiar with Common Good and agree to put in the time and good will necessary to resolve the dispute.</li>
            <li>Within 10 days of the postmark of the registered letter, the other Party will submit a registered letter naming their representative, again with full contact information, and including best times for a meeting.</li>
            <li>The representative of the initiating Party will contact the other representative and set up a meeting. If there are any costs associated with setting up the meeting between representatives, they will be born by the representatives with the expectation that they will be reimbursed by their representees.</li>
            <li>The representatives will agree on and appoint an arbitrator who agrees to spend the time and energy to resolve the dispute. Any costs incurred by the arbitrator will be borne equally by each Party.</li>
            <li>The arbitrator may request any documentation or witnesses deemed necessary to arrive at a resolution, and both parties agree to comply promptly.  The decision of the arbitrator is binding on both parties.</li>
            <li>The dispute resolution will be written up by the arbitrator and signed by both representatives.</li>
            <li>The following timetables will be adhered to as well as possible: The representatives have 10 days to agree on an arbitrator, and the arbitrator will have 10 days to come up with a resolution.  All parties agree to complete the dispute resolution process within 30 days.</li>
          </ol>
        </li>
      </ol>
EOF
  ) . $back;

  if ($i == 'participate') return t(<<<EOF
  <p>%PROJECT staff and the %PROJECT system will give you regular feedback on how your actions impact your finances and the well being of the community. For this feedback to be effective, you have to pay attention to it.</p>

  <p>The %PROJECT system needs feedback too! Please make suggestions when you see something that could be improved.</p>
  
  <p>Similarly, participatory economic democracy works only if you participate in your community's <%aProcess>decision process</a> to set funding priorities and make <%aOther>other decisions</a> from time to time, as needed.</p>
EOF
  , '_aProcess _aOther', w\atag('decision-principles'), w\atag('other-decisions')) . $back;

  if ($i == 'decision-principles') return t(<<<EOF
    <p>Each %PROJECT Community must decide for itself how to manage its decision making. %PROJECT staff are developing recommendations for best practices to value and encourage active participation by all members.</p>
EOF
  ) . $back;

  if ($i == 'other-decisions') return t(<<<EOF
    <p>From time to time, members will need to make a variety of non-financial decisions. A governing board may need to be elected. Policies and procedures occasionally need to be created or revised. Other issues may arise.</p>
EOF
  ) . $back;

  if ($i == 'common-good-community-requirements') return t(<<<EOF
    <p>Requirements for certification as a %PROJECT Community are not yet finalized. They may include such things as qualified leadership, minimum size, experience using the %PROJECT system, diversity of participants, and successful completion of a training program.</p>
EOF
  ) . $back;

  if ($i == 'agreement') return t(<<<EOF
    <p>The %PROJECT Agreement is at the heart of the the %PROJECT system. It is our agreement to accept %RCREDITS from each other that gives them value. Please read each section carefully and click the checkboxes to indicate your agreement. For more specific information, click the hyperlinks in the text.</p>
EOF
  );

  if ($i == 'donate') return t(<<<EOF
    <p>Your donation helps fund administration and promotion of the %PROJECT system during the pilot phase. It is also a formal "sealing" of your signature on the %PROJECT Agreement.</p>
EOF
  );

  if ($i == 'account/proxies') return t(<<<EOF
    <p>During the pilot phase, it may be hard to choose two proxies from among the other members, unless you invite them to join first. You can change your selections at any time, so no worries.</p>
EOF
  );

  if ($i == 'invite') return t(<<<EOF
    <p>The %PROJECT system is designed to be community-centered, so our relationships with friends and neighbors in our own geographical community is important. We can work better together for the well being of everyone in our community and our region, if we know we can trust one another.</p>
    <p>Invite as many people as you like, but only people you trust.</p>
EOF
  );

  if ($i == 'transactions') return t(<<<EOF
    Hover over any option or column header for an explanation of what it means. Select some options, and the period or specific date range that interests you, then click either "Show" or "Download". The downloaded spreadsheet (in CSV format) has more complete information, including column totals.
EOF
  );
  if ($i == 'proxies') return t('proxy help');

  if ($i == 'demand') return showCharts('banking', $ctty) . t(<<<EOF
    <p>The "demand" for %RCREDITS is the net amount of US Dollars people want to trade for %RCREDITS. As long as people have plenty of places to spend within the %PROJECT system, they will generally keep moving money in. So the demand gives us a pretty reliable measure of the system's soundness.</p>
    <p>We can keep issuing %RCREDITS as long as there is a net inflow of US Dollars (more <b class="in">green</b> than <b class="out">orange</b> on the chart &mdash; an increasing demand). If members need to make more purchases outside the %PROJECT network and are taking more US Dollars <b class="out">out</b> of the system than they are putting <b class="in">in</b> (a decreasing demand), then we must temporarily stop <i>issuing</i> %RCREDITS. Of course, we can keep <i>using</i> the %RCREDITS already in circulation.</p>
    <p>Once people have more places to spend within the system (typically because more people and businesses have signed up) and they start moving money <b class="in">in</b> again, then we can begin again to issue additional %RCREDITS.</p>
    <p>Transfers from a %PROJECT Account <b class="out">to</b> a bank account often reflect an incomplete <%aCircles>economic circle</a>. That is, the individual or company moving %RCREDITS to a bank account does not yet have enough places to spend them. Generally, this means they are spending them outside the community (and outside the %PROJECT system). This is an opportunity for import-replacement &mdash; starting new local businesses (or expanding existing businesses) to supply goods or services not currently produced in our region.</p>
    <p>How <b style="color:red;">low</b> our total monthly volume of <b class="out">outward</b> transfers is, is a measure of how well we are doing as a community in our progress toward a sustainable local economy.</p>
    <p>As an individual, you can help by finding ways to spend within the %PROJECT system, instead of transferring your funds to a bank account. For all the money that circulates within the system, we can use %RCREDITS, freeing up that many US Dollars to use for something else. So, in effect, when you can keep money in the system, we are all that much richer.</p>
EOF
  , '_aCircles', w\atag('/help/economic-circles')) . $back;

  if ($i == 'growth') {
    $_helpus = @$mya ? t('You can help! <%aInvite>Click here to invite</a> your friends and local businesses where you want to spend your %RCREDITS. Sit with your friends while they sign up &mdash; it takes only half an hour, but without a nudge most people spend several weeks putting it off.', '_aInvite', w\atag('/community/invite')) : t('<_aSignup>Join us</a>!', '%_aSignup', w\atag('/signup'));
    
    return showCharts('accts', $ctty) . t(<<<EOF
      <p>The real power and purpose of the %PROJECT system is in our gathering as a community to decide what our funding priorities should be &mdash;</p>
      <ul>
        <li>for sustainable agriculture and energy systems,</li>
        <li>for supporting local businesses, innovation, and local self-reliance,</li>
        <li>for ensuring everyone in our community has access to healthy food, a home, health care, cultural enrichment, and satisfying work</li>
      </ul>
      <p>&mdash; then issuing %RCREDITS to fund our decisions.</p>
      <p>That community-level sovereignty requires <i>broad participation</i>. The green line shows how many we are today.</p>
      <p>%helpus</p>
EOF
    , compact('_helpus')) . $back;
  }

  if ($i == 'funds') return showCharts('funds', $ctty) . t(<<<EOF
    <p>There are two kinds of funds in the %PROJECT system:</p>
    <ul>
      <li><b style='color:white; background-color:darkgreen;'>%RCREDITS</b>. These are the funds that we issue together as a community, by agreeing to accept them as payment for our goods and services (or labor).</li>
      <li><p><b style='color:white; background-color:darkblue;'>US Dollars</b>. When you transfer funds from bank account to your %PROJECT Account, that money goes into the community's US Dollar bank account, the "%PROJECT Escrow". The system issues you an equal amount of %RCREDITS to spend or hold in your account. Then when a business gets paid in %RCREDITS, if they don't have enough places to spend them yet, they trade some of their %RCREDITS for US Dollars from the %PROJECT Escrow.</p>
      
      <p>The %PROJECT Escrow accumulates more and more US Dollars, as the system grows, and those Dollars just sit there, unused. Once we have enough active participation, we can decide together how best to use that money &mdash; for strategic incentives, grants, zero-interest loans, and equity investments for sustainability and the common good.</p></li>
    </ul>
    <p>The total funds in the %PROJECT system (the total under our control as a community) is the Sum of the %RCREDITS (circulating or sitting) and the US Dollars.</p>
    
    <p>The chart also shows three measures of the community's risk of having a <%aCrunch>cash flow crunch</a>:</p>
    <ul>
      <li><b style='color:white; background-color:orange;'>In Use</b>. The orange line shows how much money is being used each month. Money in consistent use tends to stay in the system.</li>
      <li><p><b style='color:black; background-color:yellow;'>Runny Funds</b>. Incentive rewards simply increase your Credit Reserve. If you keep a positive balance while the system grows to become stable, then no one can cash out those rewards, shrinking the %PROJECT Escrow. Maybe you plan to spend them next month. Or maybe you are holding them in your %PROJECT Account as a hedge against inflation. But the system can't guess that. Unless you use the auto-refill feature, the system assumes you might spend your cash reserve at any moment (resulting in a negative Balance). We call those funds that might be spent at any moment "runny".</p>
      <p>On the other hand, if you DO use the auto-refill feature, you have chosen to keep a certain Target Balance in your %PROJECT Account. That target partially offsets the runny funds in other accounts, making a cash flow crunch even less likely. The yellow line shows the total of runny funds minus the estimated total balances in accounts that use auto-refill (conservatively assuming each person's average balance is half their chosen target). The formula looks something like this:</p>
      
      <blockquote>Total Runniness = SUM(runny funds) - SUM(auto-refill levels / 2)</blockquote>
      </li>
              
      <li><b style='color:white; background-color:darkred;'>Top 5</b>. For accounts with the most runny funds (the top 5 or top 5%, whichever is higher), reward amounts are totaled and shown as a red line. It is unlikely that more than 5% of the accounts would decide to spend all their cash reserve at the same time.</li>

      <li><b style='color:white; background-color:magenta;'>Runny Balance</b>. The violet line shows the actual total combined balance in runny accounts (Balance plus Credit Reserve). They <i>could</i> spend all their cash reserve, but this is what they actually have done so far.</li>
    </ul>
EOF
  , '_aCrunch', w\atag('cashflow')) . $back;
  
/*
      <li><b style='color:white; background-color:magenta;'>Runny Debt</b>. The violet line shows how much of their incentive rewards the runny accounts have already spent. This total has already reduced the %PROJECT Escrow, so you can think of it as mitigating the yellow line (subtract it from the yellow line).</li>
      */
/*
    <p>* The distinction between "savings" and %RCREDITS that are actually being used is important. The value of %RCREDITS, like any currency, is what you can get for them. When you save your %PROJECT funds for spending a month or more in the future, it probably means you are confident (1) that you will be able to spend them within the %PROJECT system OR (2) that you will be able to trade them for US Dollars because other members can spend them within the system. So savings are a <em>speculative</em> <a href="%BASE_URL/help/demand">demand</a> for %RCREDITS, rather than a demand based in current reality. The speculation may indeed be accurate, but <a href='velocity'>circulation velocity</a> and <a href='demand'>net monthly influx of US Dollars</a> are stronger measures of the system's capacity for additional %RCREDITS.</p>
    <p>** <b style='color:white; background-color:darkred;'>Top 5</b>. Balanced participation among many members maximizes the system's predictability and protects against a cash flow crunch. Expect the "Top 5" line to level off as the system grows.</p>
*/    

  if ($i == 'velocity') return showCharts('velocity', $ctty) . t(<<<EOF
    <p>In our society at large, how fast the money moves is an indicator of the health of our economy. Faster velocity implies more people spending more money per month.</p>
    <p>More spending is not necessarily a good thing, if we all already have what we need. But when there are unmet needs, increased economic activity can be a sign of things getting better. In the %PROJECT system, economic activity means people are finding the %PROJECT system useful, which is always a good thing.</p>
    <p>Notice that the faster money circulates, the less money we need, to buy the same stuff. Velocity is a money-stretcher! For example, suppose there are $1 million worth of %RCREDITS in circulation. Let's say the velocity is 1 in April (the whole million gets spent) and 2 in May (the whole million gets spent twice). In May we used the same amount of money to buy twice as much.</p>
    <p>For comparison, the velocity of M2 money stock in the United States over the past half century has fluctuated between .25 and .45 per month (as you can see on the chart, %RCREDITS circulation velocity is in that same range).
EOF
  ) . $back;

  if ($i == 'economic-circles') return showCharts('tx', $ctty) . t(<<<EOF
    <p>Economic circles are crucial to a healthy regional economy. The more we can keep money circulating locally, the richer we are. Why? Because we can use that money over and over &mdash; in effect turning each dollar into millions.</p>
    <p>In a typical economic circle, you buy something from a local store or pay for local services. That business then pays its local suppliers and its local employees. Those employees in turn become consumers like you &mdash; they buy something from a local store, etc. When at any point in this circle we lose the "local" and pay someone or some company outside our region, that money is gone. It has leaked out of our community and we are that much poorer.</p>
    <p>The %PROJECT system gives us an easy way to track our economic circles and opportunities for improvement.</p>
    <p>This chart shows four types of transactions. For strong economic circles, we should see the volume of person-to-business (<b class='p2b'>p2b</b>) transactions matching the volume of business-to-person (<b class='b2p'>b2p</b>) transactions. More <b class='p2b'>p2b</b> than <b class='b2p'>b2p</b> means business are not paying enough local employees (or other local businesses). More <b class='b2p'>b2p</b> than <b class='p2b'>p2b</b> means people are shopping elsewhere. As you see (in red), several member businesses have begun paying their employees in %RCREDITS (see our <%aFB>FaceBook post</a> on how we solved the payroll challenge).</p>
    <p>The <%aDemand>Monthly Bank Transfers</a> chart also measures how well we are doing as a community, at keeping money in the system.</p>
EOF
  , '_aFB _aDemand', w\atag('http://rc4.me/fb'), w\atag('demand')) . $back;

  if ($i == 'your-return') return t(<<<EOF
    <p>Your effective return is calculated as your monthly inflation adjustment (currently based on %R_INFLATION_RATE% APR) plus any incentive rewards you got in the period, all divided by your average balance for the period. The result is then annualized to give you an APR (annual percentage rate). Regular financial institutions give you interest on your <em>minimum</em> balance for the month, which is nearly always lower than your <em>average</em> balance.</p>
EOF
  ) . $back;

  if ($i == 'social-return') return t(<<<EOF
    <p>The community's financial benefit from your participation is very loosely calculated as the sum of:</p>
    <ul>
      <li><b>Donations.</b> Your gifts to the %PROJECT project.</li>
      <li><b>Money in.</b> The amount of money you moved into the system, minus the amount you moved out (but not less than zero). By using %RCREDITS in place of those funds, as they circulate among %PROJECT members, we can free up that much US Dollars for the community to grant, lend, or invest.</li>
      <li><b>Rewards.</b> The amount of your incentive rewards for buying and selling (if your %PROJECT community offers incentive rewards) &mdash; reflecting the explicit value the community places on your using %RCREDITS for purchases and sales.</li>
      <li><b>Community funding.</b> Your per-capita share of the funds the community has already granted, loaned, or invested. The continued viability of that funding is supported by your participation. (counted as part of social benefits ever, but not in the past month)</li>
      <li><b>Card fees saved.</b> A percentage (%R_CC_RATE%) of your purchase and sale amounts &mdash; the typical processing fee that businesses pay for credit card purchases. Together with the other party to the transaction, you kept that much money from leaking out of the community to the big banks.</li>
    </ul>
    </p>
    <p><b>Control.</b> Of course your participation benefits the community far beyond mere money. The commmunity benefits by all of us together having greater control over our local economy, so we can plan and decide together how to use the power of money to make a better life for everyone.</p>
EOF
  ) . $back;

  if ($i == 'taxes') {
    $taxhead = ray('Tax Form:;1120;990;990EZ;990-A;1040;1040A;1040 Sched C');
    $taxline[] = ray('Sales / Other Income,1a,varies,varies,varies,7 or 21 ("barter"),7,1');
    $taxline[] = ray('Inflation Adjustment,5 or 10,VIII(3)D,I(4),II(B)8,8a or 21,7 or 8a,6');
    $taxline[] = ray('Incentive Rewards,10,VIII(3)D,I(4),II(B)8,21,7,6');
    $taxline[] = ray('"Sharing" Donations,19,IX(1),I(10),N/A,,Subtract from Rewards,4; 8; 10; 17; or 27a');
    $taxes = '<tr><th class="cat">' . join('</th><th>', $taxhead) . "</th></tr>\n";
    foreach ($taxline as $line) $taxes .= '<tr><td class="cat">' . join('</td><td>', $line) . "</td></tr>\n";
    $taxes = strtr($taxes, ['<td></td><td>' => '<td colspan="2">', ';' => ',']);
    
    return t(<<<EOF
  <p>Pay taxes on %PROJECT income just as you would for US Dollar income, including income, excise, and sales taxes.</p>

  <p>We recommend you NOT report %PROJECT rewards or inflation adjustments as income, because your %PROJECT community has not yet decided you can keep them &mdash; they are still just a credit reserve.</p>
  
  <p>However, if you prefer to count rewards and inflation adjustments as income NOW, rather than when the community makes that decision, you have that option. The chart below suggests some tax form lines you might choose, for reporting your %PROJECT activity. It also suggests how you might report your monthly tax-deductible "sharing" contributions to the %PROJECT project.</p>

  <p>If you follow the <%a1>accounting practices recommended</a> by %PROJECT staff, your software already includes your %PROJECT income, properly categorized, in your accounting records and reports.</p>

      <table id="taxes">
$taxes
      </table>
EOF
    , '_a1', w\atag('http://rc4.me/doc/accounting.pdf'));
  }
  
  if ($i == 'bits') {
    include_once __DIR__ . '/bit-descs.inc';
    foreach ($flagHelp as $k => $v) $res[] = "<tr><td>$k</td><td>$v</td></tr>";
    $res = join("\n", $res);
    return t(<<<EOF
      <div id="bit-help">
      <p>Community Administrators can see and set/unset most of these bits on each account's Summary page.</p>
      <table>$res</table>
      </div bit-help>
EOF
    );
  }
  
  
  if ($i == 'risks') {
    include_once __DIR__ . '/risk-descs.inc';
    extract($riskHelp, EXTR_PREFIX_ALL, 'h');
    $acctRisks = $txRisks = $_special = '';
    foreach (ray(K_ACCT_RISKS) as $k => $weight) {
      $extra = '';
      if ($k == 'trusted') $extra = t('This risk factor is multiplied by the number of other accounts that trust this member. Being chosen as an alternate proxy counts as half. Likewise being chosen as someone\'s proxy\'s proxy counts as half.');
      if ($k == 'socialConx' or $k == 'badConx') $extra = t('This risk factor is multiplied by the number of such connections.');
      if ($k == 'geography') $extra = t('Federal and state law enforcement agencies have identified 28 High Intensity Drug Trafficking Areas (HIDTAs), which include 16 percent of all counties in the United States and 60 percent of the U.S. population. For example, Washtenaw County, Michigan has been designated as part of the Michigan HIDTA. Since so much of the population qualifies, the risk weight is low.');
      if ($k == 'new') $extra = t('This risk factor is divided by the number of years the member (individual or company) has been at its current address.');
      if ($k == 'moves') $extra = t('This risk factor is multiplied by the number of times the member has moved, over the past decade (if known)');
      if ($k == 'bigDay') $extra = t('For individual accounts, the number of "employees" is 1.');
      if (in_array($k, array('bigWeek', 'big7Week', 'bigYear'))) $extra = t('(see bigDay)');
      $acctRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $_special = $line;
    }
    foreach (ray(K_TX_RISKS) as $k => $weight) {
      $extra = '';
      if ($k == 'origins') $extra = t('This risk factor is multiplied by the average risk of the payer\'s receiving transactions over the past year.');
      if (in_array($k, ray('fromSuspect toSuspect suspectOut suspectIn'))) $extra = t('This risk factor is multiplied by the suspicious account risk, divided by the "unusually large" threshold ($%K_THRESHOLD).');
      if ($k == 'invoiceless') $extra = t('And no mention of an invoice in the transaction description.');
      $txRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $_special = $line;
    }

    $acctRisks = t('<h2>Account Flags</h2>') . "\n<table>$acctRisks</table>\n";
    $txRisks = t('<h2>Transaction and ACH Flags</h2>') . "\n<table>$txRisks</table>\n";
    $_score = number_format(K_RED*300*8/15/K_THRESHOLD, 0);
    $_score = "<span class=\"badRisk\">$_score</span>";
//    if ($_special) $_special = preg_replace('/^(.*)\[(.*)\](.*)/', '<div id="special-flag"><big style="color:red;">$1</big> (weight:$2) $3</div>', $_special);
    return t(<<<EOF
      <div id="risk-help">
      <table id="special-flag">%special</table>
      <p>Suspicious Activity Risk Scores (flags) are calculated based on a weight divisor assigned to each type of risk. That divisor (shown in square brackets for each item below) is an assessment of how many same-sized risks would together make an account or transaction suspicious -- a higher divisor means less risk. <span class="goodRisk">Risk factors with a negative divisor</span> (shown in green) are mitigating.</p>
      <p>Risk scores are then proportional to the sum of the inverses of those weight divisors - a higher score means more risk. For transactions, the score is further multiplied by the transaction amount and divided by the "unusually large" threshold ($%K_THRESHOLD). All risk scores (whether for accounts, transactions, or ACHs) are finally multiplied by %K_RED and rounded to the nearest integer. So %K_RED is the minimum "suspicious" score.</p>
      <p>For example, a $300 transaction with two risk factors having weight divisors of 3 and 5 respectively, would be scored 100 * 300 * (&#8531; + &#x2155;) / %K_THRESHOLD = %score (asterisk here means multiply, slash means divide).</p>
EOF
    , compact('_score', '_special')) . $acctRisks . $txRisks . '</div risk-help>';
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