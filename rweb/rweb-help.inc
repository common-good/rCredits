<?php

/*
 * @file
 * rCredits Help Messages
 * This file contains a single function that returns a help message.
 */
 
namespace CG\Web;
use CG as r;
use CG\Util as u;
use CG\Web as w;

/**
 * Return the appropriate raw help text.
 * @param string $i: identifier for the appropriate help text.
 * @param string $tag: (optional) item to highlight)
 * @param string $args: other URL arguments
 * @param assoc $args:  (RETURNED) arbitrary data
 */
function rawHelp($i, $tag = '', &$args = '') {
  global $rUrl;
  
  extract(just('gifts years', $args)); // must precede eLinkAcct
  $mya = w\eLinkAcct($zot, $args, 'any');
  
  $ctty = @$mya->community ?: 0;
  $company = @$mya->co;
  $username = @$mya->name; // @ is for SMS testing (when not signed in)
  $cttyName = @$mya->cttyA->fullName;
  
  $site = str_replace('https://', '', BASE_URL);
  $_aPage = atag("/member/$username", w\away());
  $_aInfo = atag('/account/company');
  $regEmail = r\regionField('email');
 
  $companyAccount = $company ? '' : t(<<< X
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic %PROJECT webpage: <%aPage>%site/member/%username</a> &mdash; so choose wisely. See also the <%aInfo>Company Info</a> form.</p>
X
  , compact('_aPage', '_aInfo', 'username', 'site'));
  
  //Help is also available for the <a href="%BASE_URL/help/sms">Text Messaging Interface</a>.
  $back = '<p>' . 
  w\backButton() . '</p>';
  
  if ($i == 'general') return t(<<< X
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
X
  , '_aEmail', u\emailTag($regEmail, t('help with %PROJECT')));

  if ($i == 'other') return t(<<< X
    <h2>Need Help Signing In?</h2>
    <p>If your username and password aren't working, no worries. Just click "<%aForgot>Forgot it</a>" here or on the sign-in page, to get an email for choosing a new password.</p>
    <h2>About %RCREDITS&reg;</h2>
    <p>%RCREDITS is a credits/debit card and payment system to support the common good. It is also a %PROJECT Economics Toolkit, to help you and people everywhere create a democratic, community-centered economic system that puts people and planet first -- one community at a time.</p>
    <p>We take transparency seriously &mdash; for true democratic control of our local economy we have to be able to see what's going on. Even if you are not a member, you can see what's happening in the %PROJECT economy graphically on the <%aCtty>Community page</a>.</p>
    <p>The <%aAgree>%PROJECT Agreement</a> is at the heart of the %PROJECT system. It is the promise we make together to accept %RCREDITS that gives them value.</p>
    <p>Participation in %PROJECT is by invitation. Ask a friend to invite you or <%aPromo>request an invitation here</a> and we will let you know when the %PROJECT Economy comes to your community.</p>
    <p>%PROJECT is a 501(c)(3) Massachusetts-based nonprofit.</p>
X
  , 'username _aForgot _aCtty _aAgree _aPromo', $username, atag("/account/password/%username"), atag('/community'), atag('/agreement'), atag(PROMO_URL));

  if ($i == 'smartphone') return t(<<< X
    <p>To use the smartphone interface, you will need to install an app to read QR codes (for example QRDroid on Android phones or Qrafter for iPhones). You can pay participating businesses by scanning their QR code, then visiting the scanned URL.</p>
    <p>Your customers (or employer) can pay you by scanning the QR code on your member ID card. You can print an Advance ID card for your individual account, using the link on the Summary page.</p>
X
  );

  if ($i == 'sms') return t('The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the <%aSMS>Test SMS form</a> to simulate sending a text.', '_aSMS', atag('/sms'));

  if ($i == 'summary') return t(<<< X
    <p>This page shows an overview of your %PROJECT Account.</p>
    <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region (%regName). The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
  <p><b>Balance</b> is the amount in your %PROJECT Account.</p>
  <p><b>Credit line</b> is how far negative your account balance is allowed to go. Your credit limit increases a little at a time, based on your how much you receive and spend each month.</p>
X
  , 'regName', $cttyName);

  if ($i == 'txs') return t(<<< X
    <p>Select the period you want to see transactions for. Both the Summary and Transactions sections will reflect just the transactions in that period. However, the "Pending" line will reflect all your not-yet-accepted invoices and offers EVER. The Ending balance in the Summary is always your current balance.</p>
    <p>If the status is "pending", either you or the other party will need to click "OK" to complete the deal. click the "X" to reject an invoice, dispute a charge, reject an offer, or cancel or undo something you did. click the pencil to edit the description. The description is just for you &mdash; the other party will not see it.</p>
X
  );

  if ($i == 'pay' and $whys = ray(R_WHYS)) return t(<<< X
    <p><b>Whom:</b> Type all or part of the full name of the account you want to pay. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID.</p>
    <p><b>Amount:</b> The amount of the transaction, measured in dollars, to be paid in %RCREDITS.</p>
    <p><b>Purpose:</b> If this is a purely financial transaction, click "%forMoney" or "%forNongoods". Otherwise, a description is required (for purchases and sales of real goods and services). Donations to nonprofit organizations count as purchases of real services &mdash; services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank &mdash; your statements will call the purpose "cash".</p>
X
  , 'forMoney forNongoods', $whys[FOR_USD], $whys[FOR_NONGOODS]);

  if ($i == 'directory') return t(<<< X
    <p>This is where you can find out who takes %RCREDITS for what you want. click a business name to see its %PROJECT webpage.</p>
    <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you choose the United States "01" postal code region.</p>
X
  );

  if ($i == 'account/basic') return t(<<< X
    <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
X
  );

  if ($i == 'account/cells') return t(<<< X
    <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with %RCREDITS. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
X
  );

  if ($i == 'account/relations') return t(<<< X
    <p>Relations let you connect individual %PROJECT participants to your account &mdash; as owners, employees, suppliers, or others, with limited or full access to the account. This is useful for authorizing sales clerks or setting up joint accounts. Or you might give your accountant "read" permission. Company accounts are managed only by an individual agent.</p>
    <p>Yellow highlights show settings that need to be confirmed &mdash; either by you or by the other party.</p>
    <p>To change settings in a Yes/No column, click + or -. To remove someone from the list, make them all "No"s, and no permissions (if they are already that way, change something and change it back). Be sure to "Save Changes".</p>
    <p>You can allow another account to "draw" from this one to avoid overdrafts (a 25 cent fee may apply).</p>
X
    ) . ($company ? t('<p>If you give the person at least "Sell" permission (and save the record), you can request a Member ID Card for them to use on behalf of this account.</p>') : '');
    
  if ($i == 'account/contact') return t(<<< X
    <p><b>Full name</b> should be your full legal name. If you change the name on the account, you will need to buy a new %PROJECT card and your account will be disabled until you supply new proof of identity. For companies, your %PROJECT website address will also change.</p>
    <p><b>Postal code:</b> Your postal code determines what %PROJECT Community you are in.</p>
    <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
X
  );

  if ($i == 'account/photo') return t(<<< X
    <p>For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. If you have trouble coming up with a photo in a format that works, have a youngster take picture of you with their cell phone or tablet. Ask them to email you the picture, save that attachment to your desktop, then upload it here. If you still have trouble, email a description of your predicament to us at <%a>%regEmail</a> and we will help.</p>
X
  , '_a regEmail', u\emailTag($regEmail, t('photo trouble'))) . $companyAccount;

  if ($i == 'account/company') return t(<<< X
    <p>All of the information from this form goes automatically on your <%aPage>%PROJECT webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. click the link, to see the results (after saving your changes).</p>
X
  , 'username _aPage', $username, atag("/member/$username"));

  if ($i == 'account/preferences') return t(<<< X
    <p>These settings start out as what most people prefer. Feel free to adjust them as you like. You can always come back and change them again later.</p>
    <ul>
      <li><b>Target.</b> You will want to keep enough in your account to cover the total amount you expect to spend at %PROJECT member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
      <li><b>Email notices.</b> We suggest you begin with daily notices, until you feel comfortable with the system. You will receive a "daily" notice only when there is something new that you will want to know about (such as a credit to your account).</li>
    </ul>
X
    );

  if ($i == 'issuing-credits') return t(<<< X
    <p>%RCREDITS are a type of credit. Like any medium of exchange, they represent value. They make it possible for us to trade goods and services with each other in our community, without having to trade something directly with whoever has what we want, and without having to give them something of precisely the same value ("barter"). The medium of exchange lets us give any amount of our goods or services to any participant in the economy, then receive the same value of goods or services from some other participant(s). If everyone both buys and sells, the credits move through the community in interconnected circles of exchange.</p>
    <p>From the start, %RCREDITS are issued as a one-to-one stand-in for US Dollars.</p>
    <p>Once %RCREDITS become well-established and popular in a community, we can also issue them together, with careful planning, as grants, zero-interest loans, equity investments, and incentives for doing the right thing. Together we can fund whatever enterprises and activities we decide would be best -- for our community, for communities elsewhere, and for the world.</p>
    <p>Issuing %RCREDITS is a community responsibility. This is the key to creating a %PROJECT Economy. We must issue as much credit as we need, to fund our priorities -- but no more than the value we as a community are capable of producing.</p>
X
  ) . $back;
  
  if ($i == 'common-good-community') return t(<<< X
    <p>The purpose of the %PROJECT system is to create and fund community-centered participatory democracy. In a %PROJECT Community* you decide together how many %RCREDITS to issue (within safe limits) and what to fund with them.</p>
    
    <p>A %PROJECT Community is in "Good Standing" when it is issuing %RCREDITS responsibly, so they accurately represent the potential of its members to produce corresponding goods and services in the short term. A %PROJECT Community may lose its good standing if it runs a trade deficit with other %PROJECT Communities and fails to redeem it in a timely manner. %PROJECT staff and the other %PROJECT Communities will provide ongoing feedback, to prevent this from happening accidentally.</p>
    <hr class="narrow">
    <p><small>* There are no specific requirements for a group of members to become a sovereign %PROJECT Community. But <%aClearness>these Clearness Questions</a> will help you get ready for that responsibility.</small></p>
X
  , '_aClearness', atag('https://docs.google.com/document/d/1-lBTNHTxIPrTSrIK18ZdbcMeEPS5g5jFhST34VGkR8E')) . $back;
  
  if ($i == 'cashflow') return t(<<< X
    <p>As a %PROJECT Community you and other members decide together when to spend, grant, lend, and/or invest some of the US Dollars members have put into their %PROJECT accounts, which are held in a Community Fund Dollar Pool in a bank.</p>
    
    <p>Your %PROJECT Community must take care to keep enough money in the Community Dollar Pool to cover requests by members to <%aCashout>move funds to their bank account</a>, trading some of their %RCREDITS for US Dollars.</p>
    
    <p>If you misjudge and use too much of the Community's Dollar Pool, you will have a short-term cash flow challenge until more US Dollars come in. The "<%aBack>backing</a>" clause in the %PROJECT Agreement provides stability, so your %PROJECT Community can handle such challenges smoothly.</p>
X
  , '_aCashout _aBack', atag('/cashing-out'), atag('/backing-common-good-credits')) . $back;

  if ($i == 'backing-common-good-credits') return t(<<< X
    <p>To ensure that %RCREDITS maintain their value, you (and every other member) agree to accept however many you typically spend and/or receive in a month, even if your %PROJECT Community is unable to redeem them immediately for US Dollars.</p>

    <p>Like the Federal Reserve's backing for US Dollars held as bank deposits, backing for %RCREDITS helps prevent unfounded fears from causing a "<%aCrunch>run on the bank</a>". All members agree to back %RCREDITS so your %PROJECT Community can continue to run smoothly.</p>
X
  , '_aCrunch', atag('/cashflow')) . $back;

  if ($i == 'cashing-out') return t(<<< X
    <p>%RCREDITS have value and act like money because we agree to accept them as payment for our good and services, so they move from member to member to member as a token of exchange. When someone pays you in %RCREDITS, you can spend those funds &mdash; OR you can "cash them out" by transferring them to your bank account or exchanging them for US Dollars.</p>
    <p>You can cash out %RCREDITS that someone <i>pays</i> you, but any %RCREDITS you borrow, using your credit line, must continue to circulate within the %PROJECT system. You cannot cash them out &mdash; you can only spend them.</p>
    <p>You can help keep the %PROJECT system healthy by maintaining a positive balance most of the time.</p>  
X
, '_aReserve', atag('/credit-reserve')) . $back;

/*  if ($i == 'credit-reserve') return t(<<< X
    <p><b>Background.</b> It is crucial to the success of the %PROJECT system that most members maintain a positive account balance, so there are some %RCREDITS to circulate in the community. While the system is small, it is a bit frail in that regard, so we need to be extra careful to make sure there is no "<%aCrunch>cash flow crunch</a>" at least until we are ready to handle it as a strong, well-functioning democratic %PROJECT community.</p>

    <p><b>Your Credit Reserve.</b> So we divide your account into two pieces: your Balance and your Credit Reserve. Your incentive rewards go into the Credit Reserve, to encourage you to keep them in the system until you need them. This is like a traditional reserve credit line that kicks in whenever you would otherwise overdraw your checking account &mdash; except there are no fees here. You can spend those reserved funds any time, simply by letting your Balance go negative.</p>

    <p>For example, if you have $300 in your account and $100 of that is incentive rewards that you received (free), then your balance will show as $200, with a Credit Reserve of $100.</p>
    
    <p>Remember, we issue %RCREDITS together as a commmunity, so it is up to all of us to protect their value. Most people choose to keep a positive Balance, by setting a Target Balance on the Preferences settings page. Keeping most of the %RCREDITS we issue in the system helps the system stay healthy while it grows more and more robust.</p>
X
  , '_aCrunch', atag('/cashflow')) . $back; */
  
  if ($i == 'closing-down') return t(<<< X
     <p>%PROJECT staff and advisers have put great care into designing the %PROJECT System so that it will continue to flourish. In the unlikely event your %PROJECT Community or the %PROJECT System needs to close down, the %PROJECT Agreement gives us a way to end gracefully.</p>
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
		 <p>However, if you and your %PROJECT Community have made mistakes in handling the Community's Dollar Pool and you need to close down when there are no longer enough Dollars in the Pool to redeem the %RCREDITS in circulation, the value of everyone's %RCREDITS will be proportionately diminished, as you are settlings the accounts and closing down.</p>
X
  ) . $back;

/*  if ($i == 'ready') return t(<<< X
     <p>There is no magic formula (yet) for deciding when a %PROJECT Community is ready to allow cashing out incentive rewards. The important question is: who gets to decide such things?</p>
     <p>The %PROJECT system was invented for a single, critically important purpose: to create an opportunity for the community to take responsibility for its own economy. In a <%aCGC>%PROJECT Community</a> you decide, along with your friends and neighbors, how many %RCREDITS to issue as incentive rewards, and how many as grants, loans, and investments. You decide what to incentivize:</p>
     <ul>
       <li>buying local?</li>
       <li>eating organic food?</li>
       <li>using renewable energy sources?</li>
       <li>creating works of art?</li>
     </ul>
     <p>Being able to discuss and decide together what to incentivize and what to fund means we decide together what to make happen in our world. The future is ours. Together we can provide a decent life for everyone in our community, offer a hand to people elsewhere, and do our part to rescue ourselves from the brink of climate-change disaster.</p>
     <p>Having enough throughput to be able to issue incentive rewards is a small step along that path. Nobody knows yet when we will have a fully-functioning %PROJECT Community, capable of making that judgment. One of our community organizers predicts it will be when there are about 3,000 %PROJECT members. Time will tell.</p>
     <p>In the meantime, you are encouraged to maintain a positive account balance, treating your incentive rewards (if any) as a <%aReserve>Credit Reserve</a>, so those %RCREDITS can keep circulating among the participants as much as possible, while we grow.</p>
X
  , '_aCGC _aReserve', atag('/common-good-community'), atag('/credit-reserve')) . $back; */
  
  if ($i == 'ethics') return t(<<< X
    <p>A %PROJECT Community relies on its members to show a high level of ethical behavior, as the basis for creating a culture of trust and possibility. Of course it is always possible for someone to "game" the system and we will continue to develop software that brings such misbehavior to the attention of the community.</p>
    <p>Sometimes what looks like unethical behavior is in reality a desperate bid to escape from poverty. It is our responsibility as a community to think well about what our members need and want, and use the funding power of %PROJECT accordingly, so that such desperation is never necessary.</p>
X
  ) . $back;

  if ($i == 'dispute-resolution-process') return t(<<< X
    <p>Disputes between %PROJECT members will be resolved as follows:</p>
      <ol>
        <li>The members work together cooperatively in good faith to attempt to resolve the matter themselves, amicably.</li>
        <li>If, after 30 days, the members have been unable to agree on a resolution, at the request of either member the members attempt to resolve the matter creatively with the help of a mediator.</li>
        <li>Failing such resolution, at the initiative of either member, the members submit the matter to binding arbitration, as follows:
          <ol class="abc">
            <li>Either member will initiate the dispute resolution process by sending a registered letter to the other at their address of record in which they appoint a representative and include the representative's full contact information. The representative should be familiar with %PROJECT and agree to put in the time and good will necessary to resolve the dispute.</li>
            <li>Within 10 days of the postmark of the registered letter, the other Party will submit a registered letter naming their representative, again with full contact information, and including best times for a meeting.</li>
            <li>The representative of the initiating Party will contact the other representative and set up a meeting. If there are any costs associated with setting up the meeting between representatives, they will be born by the representatives with the expectation that they will be reimbursed by their representees.</li>
            <li>The representatives will agree on and appoint an arbitrator who agrees to spend the time and energy to resolve the dispute. Any costs incurred by the arbitrator will be borne equally by each Party.</li>
            <li>The arbitrator may request any documentation or witnesses deemed necessary to arrive at a resolution, and both parties agree to comply promptly.  The decision of the arbitrator is binding on both parties.</li>
            <li>The dispute resolution will be written up by the arbitrator and signed by both representatives.</li>
            <li>The following timetables will be adhered to as well as possible: The representatives have 10 days to agree on an arbitrator, and the arbitrator will have 10 days to come up with a resolution.  All parties agree to complete the dispute resolution process within 30 days.</li>
          </ol>
        </li>
      </ol>
X
  ) . $back;

  if ($i == 'participate') return t(<<< X
  <p>%PROJECT staff and the %PROJECT system will give you regular feedback on how your actions impact your finances and the well being of the community. For this feedback to be effective, you have to pay attention to it.</p>

  <p>The %PROJECT system needs feedback too! Please make suggestions when you see something that could be improved.</p>
  
  <p>Similarly, participatory economic democracy works only if you participate in your community's <%aProcess>decision process</a> to set funding priorities and make <%aOther>other decisions</a> from time to time, as needed.</p>
X
  , '_aProcess _aOther', atag('/decision-principles'), atag('/other-decisions')) . $back;

  if ($i == 'decision-principles') return t(<<< X
    <p>Each %PROJECT Community must decide for itself how to manage its decision making. %PROJECT staff are developing recommendations for best practices to value and encourage active participation by all members.</p>
X
  ) . $back;

  if ($i == 'other-decisions') return t(<<< X
    <p>From time to time, members will need to make a variety of non-financial decisions. A governing board may need to be elected. Policies and procedures occasionally need to be created or revised. Other issues may arise.</p>
X
  ) . $back;

  if ($i == 'common-good-community-requirements') return t(<<< X
    <p>Requirements for certification as a %PROJECT Community are not yet finalized. They may include such things as qualified leadership, minimum size, experience using the %PROJECT system, diversity of participants, and successful completion of a training program.</p>
X
  ) . $back;

  if ($i == 'agreement') return t(<<< X
    <p>The %PROJECT Agreement is at the heart of the the %PROJECT system. It is our agreement to accept %RCREDITS from each other that gives them value. Please read each section carefully and click the checkboxes to indicate your agreement. For more specific information, click the hyperlinks in the text.</p>
X
  );

  if ($i == 'donate') return t(<<< X
    <p>Your donation helps fund administration and promotion of the %PROJECT system during the pilot phase. It is also a formal "sealing" of your signature on the %PROJECT Agreement.</p>
X
  );

  if ($i == 'account/proxies') return t(<<< X
    <p>During the pilot phase, it may be hard to choose two proxies from among the other members, unless you invite them to join first. You can change your selections at any time, so no worries.</p>
X
  );

  if ($i == 'invite') return t(<<< X
    <p>The %PROJECT system is designed to be community-centered, so our relationships with friends and neighbors in our own geographical community is important. We can work better together for the well being of everyone in our community and our region, if we know we can trust one another.</p>
    <p>Invite as many people as you like, but only people you trust.</p>
X
  );

  if ($i == 'transactions') return t(<<< X
    Hover over any option or column header for an explanation of what it means. Select some options, and the period or specific date range that interests you, then click either "Show" or "Download". The downloaded spreadsheet (in CSV format) has more complete information, including column totals.
X
  );
  if ($i == 'proxies') return t('proxy help');

  if ($i == 'demand' or $i == 'exchanges-for-dollars') return w\chartFrame('banking') . t(<<< X
    <p>This chart show the amount of Dollars flowing into and out of the Dollar Pool &mdash; either through transfers <b class="out">to</b> or <b class="in">from</b> a participant's bank account OR through trade with another community. <b class="export">Exports</b> bring Dollars in from another %PROJECT Community. <b class="import">Imports</b> send Dollars out to another %PROJECT community.</p>
    <p>The "demand" for %RCREDITS is the net amount of US Dollars people want to trade for %RCREDITS. As long as people have plenty of places to spend within the %PROJECT system, they will generally keep moving money in. So the demand gives us a pretty reliable measure of the system's soundness.</p>
    <p>We can keep issuing %RCREDITS (in exchange for Dollars or as negative account balances) as long as there is a net inflow of US Dollars (more <b class="in">green</b> than <b class="out">orange</b> on the chart &mdash; an increasing demand). If members need to make more purchases outside the %PROJECT network and are taking more US Dollars <b class="out">out</b> of the system than they are putting <b class="in">in</b> (a decreasing demand), then we must temporarily slow down <i>issuing</i> %RCREDITS. So when the community's demand is negative three months in a row, the system automatically alerts the %PROJECT Community's administrators, who can put a temporary ban on spending below a zero balance.</p>
    <p>Transfers from a %PROJECT Account to a bank account often reflect an incomplete <%aCircles>economic circle</a>. That is, the individual or company moving %RCREDITS to a bank account does not yet have enough places to spend them. Generally, this means they are spending them outside the community (and outside the %PROJECT system). This is an opportunity for import-replacement &mdash; starting new local businesses (or expanding existing businesses) to supply goods or services not currently produced in our region.</p>
    <p>How low our total monthly volume of outward transfers is, is a measure of how well we are doing as a community in our progress toward a sustainable local economy.</p>
    <p>As an individual, you can help by finding ways to spend within the %PROJECT system, instead of transferring your funds to a bank account. For all the money that circulates within the system, we can use %RCREDITS, freeing up that many US Dollars to use for something else. So, in effect, when you can keep money in the system, we are all that much richer.</p>
X
  , '_aCircles', atag('/help/economic-circles')) . $back;

  if ($i == 'growth') {
    $_helpus = @$mya ? t('You can help! Invite your friends and all the local businesses where you want to spend your %RCREDITS. Sit with your friends while they sign up &mdash; it takes only half an hour, but without a nudge most people spend several weeks putting it off.', '_aInvite', atag('/community/invite')) : t('<%aSignup>Join us</a>!', '_aSignup', atag('/signup'));
    
    return w\chartFrame('growth') . t(<<< X
      <p>The real power and purpose of the %PROJECT system is in our gathering as a community to decide what our funding priorities should be &mdash;</p>
      <ul>
        <li>for sustainable agriculture and energy systems,</li>
        <li>for supporting local businesses, innovation, and local self-reliance,</li>
        <li>for ensuring everyone in our community has access to healthy food, a home, health care, cultural enrichment, and satisfying work</li>
      </ul>
      <p>&mdash; then issuing %RCREDITS to fund our decisions.</p>
      <p><b class="cos">Companies</b> and <b class="members">Members</b>. That community-level sovereignty requires <i>broad participation</i>. This growth chart shows how many we are today.</p>
      <p>%helpus</p>
      <p><b class="active">Active members</b> (red line) are the number who made a purchase in the past 30 days.</p>
X
    , compact('_helpus')) . $back;
  }
//      <p><b class="conx">Connections</b> ("conx") are the number of other members and participating businesses each active member has transacted with in the %PROJECT System.</p>

  if ($i == 'dollar-pool') return w\chartFrame('funds') . t(<<< X
    <p>There are two kinds of funds in the %PROJECT system:</p>
    <ul>
      <li><b class="cgcredits">%RCREDITS</b>. These are the funds we issue together as a community, by agreeing to accept them as payment for our goods and services (or labor). We issue %RCREDITS in two ways: (1) in exchange for US Dollars AND (2) by allowing member accounts to have a negative balance. The orange area shows the total negative balances. The amount of %RCREDITS is exactly equal to the Dollar Pool amount (blue line) plus the amount of negative balances.</li>
      <li><p><b class="usd">US Dollars</b>. When you transfer funds from bank account to your %PROJECT Account, that money goes into the %PROJECT Community Fund Dollar Pool &mdash; a bank account owned by the community. You get an equal amount of %RCREDITS to use instead, to buy something from a participating business.</p>
      
      <p>The business can then use those %RCREDITS to pay its employees and suppliers, those suppliers pay their employees and suppliers, then all those employees become customers again and use those same %RCREDITS to buy things again from participating businesses. The credits go around and around.</p>

      <p>Meanwhile, the Dollars you put in have <i>not</i> been used for purchases, so as participation grows the Dollar Pool accumulates more and more US Dollars, waiting to be used. From time to time when someone can't easily spend all their credits within the %PROJECT System, they trade some back in for US Dollars from the Dollar Pool. So the amount in the Dollar Pool goes up and down a little as it grows.</p>
      
      <p>Once we have enough active participation, we can decide together, as a community, how best to use the Dollar Pool funds &mdash; for strategic incentives, grants, zero-interest loans, and equity investments for sustainability and the common good.</p></li>
    </ul>

    <p>The chart also shows several measures to help you manage your community's %PROJECT economy without putting it in danger of a <%aCrunch>cash flow crunch</a>:</p>
    <ul>
      <li><b class="savings">Savings</b>. This line shows how much of the money in circulation is not actually being used each month for purchases. Money in use tends to stay in the system, whereas participants are more likely to cash out any extra money in their accounts beyond what they use regularly for purchases.</li>
      <li><b class="top3">Top 3(%)</b>. For accounts with the highest balance (the top 3 or top 3%, whichever is higher), those balances are totaled and shown as a red line. It is unlikely that 3% of the account-holders would decide all at once to spend (or cash out) their entire balance.</li>
      <li><b class="top3">Bottom 3(%)</b>. The accounts with the lowest balances are also shown with a red line. Encourage members to bring these balances up to zero or more, to increase the size of the community's Dollar Pool (blue line).</li>
      <li><b class="neg">Negative Balances</b>. The orange area shows the total of all negative balances. This is money borrowed by members and participating businesses, that would otherwise be in the Dollar Pool. Every time you spend money you don't have (resulting in a negative balance), you are <i>creating money</i>. As a community, we have a limit to how much money we can create. When your balance goes negative, we have that much less ability as a community to fund the common good. So the system encourages you to use your credit line only as a temporary convenience, bringing it back above zero as soon as possible.</li>
      <li><b class="limit">Credit Limits</b>. Each account has a credit line, equal to its average monthly activity over the past 6 months. For example, if your credit limit is $100, your account balance is allowed to go as low as negative $100 and no lower*. The pink line totals those credit limits for all accounts.</li>
    </ul>
    <p>* Your balance is not allowed to go below zero unless you choose that setting in your Preferences (on the Settings menu).</p>
X
  , '_aCrunch', atag('/help/cashflow')) . $back;
  
  if ($i == 'velocity' or $i == 'circulation-velocity') return w\chartFrame('velocity') . t(<<< X
    <p>In our society at large, how fast the money moves is an indicator of the health of our economy. Faster velocity implies more people spending more money per month.</p>
    <p>More spending is not necessarily a good thing, if we all already have what we need. But when there are unmet needs, increased economic activity can be a sign of things getting better. In the %PROJECT system, economic activity means people are finding the %PROJECT system useful, which is always a good thing.</p>
    <p>Notice that the faster money circulates, the less money we need, to buy the same stuff. Velocity is a money-stretcher! For example, suppose there are $1 million worth of %RCREDITS in circulation. Let's say the velocity is 1 in April (the whole million gets spent) and 2 in May (the whole million gets spent twice). In May we used the same amount of money to buy twice as much.</p>
    <p>For comparison, the velocity of M2 money stock in the United States over the past half century has fluctuated between .25 and .45 per month (as you can see on the chart, %RCREDITS circulation velocity is quite a bit higher).
    <p>The <b class="local">local</b> and <b class="inter">inter-community</b> rates are shown separately. When either rate is high, it reflects healthy trade (within the community or between %PROJECT Communities).</p>
    <p>The Dollar <b class="usd">exchange</b> velocity is also shown (blue line), to reflect opportunities for import-replacement &mdash; buying things within the %PROJECT System that currently are being bought elsewhere.</p>
X
  ) . $back;

  if ($i == 'economic-circles' or $i == 'transaction-volume') return w\chartFrame('volume') . t(<<< X
    <p>Economic circles are crucial to a healthy regional economy. The more we can keep money circulating locally, the richer we are. Why? Because we can use that money over and over &mdash; in effect turning each dollar into millions.</p>
    <p>In a typical economic circle, you buy something from a local store or pay for local services. That business then pays its local suppliers and its local employees. Those employees in turn become consumers like you &mdash; they buy something from a local store, etc. When at any point in this circle we lose the "local" and pay someone or some company outside our region, that money is gone. It has leaked out of our community and we are that much poorer.</p>
    <p>The %PROJECT system gives us an easy way to track our economic circles and opportunities for improvement.</p>
    <p>This chart shows four types of transactions. For strong economic circles, we should see the volume of person-to-business (<b class='p2b'>p2b</b>) transactions matching the volume of business-to-person (<b class='b2p'>b2p</b>) transactions. More <b class='p2b'>p2b</b> than <b class='b2p'>b2p</b> means business are not paying enough local employees (or other local businesses). More <b class='b2p'>b2p</b> than <b class='p2b'>p2b</b> means people are shopping elsewhere. As you see (in red), several member businesses have begun paying their employees in %RCREDITS (see our <%aFB>FaceBook post</a> on how we solved the payroll challenge).</p>
    <p>The <%aDemand>Monthly Bank Transfers</a> chart also measures how well we are doing as a community, at keeping money in the system.</p>
X
  , '_aFB _aDemand', atag('http://rc4.me/fb'), atag('/help/demand')) . $back;

  if ($i == 'your-return') return t(<<< X
    <p>Your effective return is calculated as your monthly inflation adjustment (currently based on %R_INFLATION_RATE% APR) plus any incentive rewards you got in the period, all divided by your average balance for the period. The result is then annualized to give you an APR (annual percentage rate). Regular financial institutions give you interest on your <em>minimum</em> balance for the month, which is nearly always lower than your <em>average</em> balance.</p>
X
  ) . $back;

  if ($i == 'social-return') return t(<<< X
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
X
  ) . $back;
	
	if ($i == '2018-question-1') return u\parseUp(t(<<< X
**Investments, grants, and incentives.** Investment terms can be whatever we negotiate with the recipient -- ranging from a zero-interest loan to a full-ownership stake with regular dividends indefinitely. 

Incentives are a sort of diffuse grant. **Made-up Examples:** Get a 10% rebate on any purchase with your Common Good card during the month of December, while funds last. Or donate to a member nonprofit this month and Common Good Greenfield increases your donation by 50%.

**Arguments for $20,000.** Growth of the Dollar Pool since last year has been very steady, so repeating what worked last year, but in proportion to our current size, follows from common sense. Funding community projects contributes to growth in participation by new members and businesses, while also continuing to inspire hope, which many new members are seeking.

**Arguments for MORE than $20,000.** Our Dollar Pool has grown steadily since the end of 2015, with no dips of more than $8,000, before resuming the steady climb. If that trend continues, we could use as much as $93,000 this year ($101,000 minus $8,000).
![CGG July 2018 Dollar Pool](%BASE_URL/rcredits/images/rvote/CGG-2018-07-dollar-pool.png)
**Arguments for LESS than $20,000.** The red line on the chart shows the total in the top 10 Common Good accounts: $94,000. If all 10 at once try to transfer their Common Good Credits to their bank accounts it would be like a “run on the bank” and some people would have to wait for more money to come in, before they could take money out.

Similarly, the yellow line on the chart shows the total amount members are saving to spend later**: $84,000. If everyone at once tried to transfer these savings to a bank account, it would be like a “run on the bank” and some people would have to wait before they could take money out.

* ** The amount members are saving is the amount NOT used last month for transactions.

Our Coordinating Committee thinks the risk of either of these events is very, very small and believes we are capable, as a community, of handling any such crunch in a way that minimizes the financial inconvenience to any participant; for example, by borrowing funds until the system has time to grow again. But it is important that you judge this risk for yourself and vote accordingly.

There is also one argument for using nothing at all: Some people may choose not to participate in Common Good unless every Common Good Credit is backed by a US Dollar, so once we use ANY of the Dollar Pool, we would miss out on their participation. Our Coordinating Committee believes the number of participants attracted by our funding of worthwhile projects (a key part of our mission) will outweigh the number who shy away.
X
  )) . $back;
	
	if ($i == '2018-question-2') return u\parseUp(t(<<< X
**About Grants.** Grant funding can make a huge difference to community-spirited projects. Grants are harder to get than loans, so they make a bigger publicity splash, which could more quickly propel the Common Good movement forward in the public eye, bringing in many new members and businesses. When we make a grant, we are permanently trading our financial assets for short-term publicity and one-shot support of something worthwhile.

Once we grant funds from the Dollar Pool, the system can no longer end without someone losing out. When we make grants (or incentives) we are saying we are very confident the Common Good System will continue indefinitely -- and in the very unlikely event it fails, we would each pay our fair share of the grants we made together.

**About Incentives.** Offering incentives to people to make different everyday choices can also make a big difference—reshaping society locally according to our local values. This might also make a splash (we haven’t tried it yet, so we don’t know). 

**About Investments.** Fewer applicants wanted investment funds last year than wanted grants, so we might have to work harder to find such applicants this year. When we invest, we are temporarily trading our financial assets for incoming-producing assets, plus some publicity (but probably less than with grants). Investments help the Common Good system become financially sustainable by providing ongoing interest or dividends to cover administrative costs, and eventually a possible [Social Dividend](https://en.wikipedia.org/wiki/Social_dividend).
X
  )) . $back;

/*
  <p>We recommend you NOT report %PROJECT rewards or inflation adjustments as income, because your %PROJECT community has not yet decided you can keep them &mdash; they are still just a credit reserve.</p>
  
  <p>However, if you prefer to count rewards and inflation adjustments as income NOW, rather than when the community makes that decision, you have that option. The chart below suggests some tax form lines you might choose, for reporting your %PROJECT activity. It also suggests how you might report your monthly tax-deductible "sharing" contributions to the %PROJECT project.</p>
*/  
  if ($i == 'taxes') {
    $taxhead = ray('Tax Form:;1120;990;990EZ;990-A;1040;1040A;1040 Sched C');
    $taxline[] = ray('Sales / Other Income,1a,varies,varies,varies,7 or 21 ("barter"),7,1');
    $taxline[] = ray('Inflation Adjustment,5 or 10,VIII(3)D,I(4),II(B)8,8a or 21,7 or 8a,6');
    $taxline[] = ray('Incentive Rewards,10,VIII(3)D,I(4),II(B)8,21,7,6');
    $taxline[] = ray('"Sharing" Donations,19,IX(1),I(10),N/A,,Subtract from Rewards,4; 8; 10; 17; or 27a');
    $taxes = '<tr><th class="cat">' . join('</th><th>', $taxhead) . "</th></tr>\n";
    foreach ($taxline as $line) $taxes .= '<tr><td class="cat">' . join('</td><td>', $line) . "</td></tr>\n";
    $taxes = strtr($taxes, ['<td></td><td>' => '<td colspan="2">', ';' => ',']);
    
    return t(<<< X
  <p>Pay taxes on %PROJECT income just as you would for US Dollar income, including income, excise, and sales taxes.</p>

  <p>If you follow the <%a1>accounting practices recommended</a> by %PROJECT staff, your software already includes your %PROJECT income, properly categorized, in your accounting records and reports.</p>

      <table id="taxes">
$taxes
      </table>
X
    , '_a1', atag('http://rc4.me/doc/accounting.pdf'));
  }
  
  if ($i == 'bits') {
    include_once __DIR__ . '/bit-descs.inc';
    foreach ($flagHelp as $k => $v) $res[] = "<tr><td>$k</td><td>$v</td></tr>";
    $res = join("\n", $res);
    return t(<<< X
      <div id="bit-help">
      <p>Community Administrators can see and set/unset most of these bits on each account's Summary page.</p>
      <table>$res</table>
      </div bit-help>
X
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
      if (in($k, 'bigWeek big7Week bigYear')) $extra = t('(see bigDay)');
      $acctRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $_special = $line;
    }
    foreach (ray(K_TX_RISKS) as $k => $weight) {
      $extra = '';
      if ($k == 'origins') $extra = t('This risk factor is multiplied by the average risk of the payer\'s receiving transactions over the past year.');
      if (in($k, 'fromSuspect toSuspect suspectOut suspectIn')) $extra = t('This risk factor is multiplied by the suspicious account risk, divided by the "unusually large" threshold ($%K_THRESHOLD).');
      if ($k == 'invoiceless') $extra = t('And no mention of an invoice in the transaction description.');
      $txRisks .= ($line = riskHelpLine($k, $weight, $extra, $tag));
      if ($tag == $k) $_special = $line;
    }

    $acctRisks = t('<h2>Account Flags</h2>') . "\n<table>$acctRisks</table>\n";
    $txRisks = t('<h2>Transaction and ACH Flags</h2>') . "\n<table>$txRisks</table>\n";
    $_score = number_format(K_RED*300*8/15/K_THRESHOLD, 0);
    $_score = "<span class=\"badRisk\">$_score</span>";
//    if ($_special) $_special = preg_replace('/^(.*)\[(.*)\](.*)/', '<div id="special-flag"><big style="color:red;">$1</big> (weight:$2) $3</div>', $_special);
    return t(<<< X
      <div id="risk-help">
      <table id="special-flag">%special</table>
      <p>Suspicious Activity Risk Scores (flags) are calculated based on a weight divisor assigned to each type of risk. That divisor (shown in square brackets for each item below) is an assessment of how many same-sized risks would together make an account or transaction suspicious -- a higher divisor means less risk. <span class="goodRisk">Risk factors with a negative divisor</span> (shown in green) are mitigating.</p>
      <p>Risk scores are then proportional to the sum of the inverses of those weight divisors - a higher score means more risk. For transactions, the score is further multiplied by the transaction amount and divided by the "unusually large" threshold ($%K_THRESHOLD). All risk scores (whether for accounts, transactions, or ACHs) are finally multiplied by %K_RED and rounded to the nearest integer. So %K_RED is the minimum "suspicious" score.</p>
      <p>For example, a $300 transaction with two risk factors having weight divisors of 3 and 5 respectively, would be scored 100 * 300 * (&#8531; + &#x2155;) / %K_THRESHOLD = %score (asterisk here means multiply, slash means divide).</p>
X
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

    'why-wait') return t(<<< X
    <p>Exchanges of rCredits for US Dollars reflect an incomplete economic circle. That is, the individual or company selling the rCredits does not yet have enough places to spend them.</p>
    <p>The 2-week delay in spending rCredits that you buy takes that amount out of circulation temporarily, giving the seller time to find more places to spend rCredits, without meanwhile having to sell more and more of them.</p>
X
    ),
    
      <p>If you are acting as an agent for someone else's account &mdash; for example if you are a sales clerk for a department store &mdash; the period will be a colon instead. Your company ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) identifies both you and the company.</p>
*/