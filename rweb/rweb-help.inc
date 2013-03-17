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
  $cacct = r\acct();
  $acctName = @$cacct->name; // @ is for SMS testing (when not signed in)

  $companySummary = (@$cacct->account_type == R_PERSONAL or !@$cacct->proSe())? '' : t('<p>You cannot print a Member ID Card for the company, because ID cards are for individuals, not companies. However, in the <a href="$base_url/account/relations">Account Relations</a> section you can print an ID card for each person authorized to buy and/or sell on behalf of the company.</p>');
  $site = str_replace('http://', '', R_SITE_URL);
  $companyAccount = (@$cacct->account_type == R_PERSONAL) ? '' : t(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic rCredits webpage: <a target="_blank" href="$base_url/member/$cacct->name">$site/member/$cacct->name</a> &mdash; so choose wisely. See also the <a href="$base_url/account/company">Company Info</a> form.</p>
EOF
  );
  $paySpecial = t(' If you have employees you can say pay "ALL", to pay each employee the same amount.');
  
  // the <a href="$base_url/help/smartphone">Android Smartphone Interface</a> and 
  $helps = array(
    'general' => t(<<<EOF
      <p>Help is also available for the <a href="$base_url/help/sms">Text Messaging Interface</a>.
      Use the tabs above and the links at the top right, to try all the features of the rCredits system. Here's an overview of what's where:</p>
      <h2>Tabs:</h2>
      <ul>
        <li><b>Summary</b> gives you an overview of your account.</li>
        <li><b>Transactions</b> shows your recent transactions so you can accept, edit, print, or undo them.</li>
        <li><b>Charge</b> lets you invoice another participant (to be paid in rCredits). Or, if you have a qualified business account, you can charge them. If the transaction is for real goods or services, both buyer and seller get an incentive reward of additional rCredits.</li>
        <li><b>Pay</b> lets you transfer your rCredits to another participant. If the transaction is for real goods or services, both buyer and seller get an incentive reward of additional rCredits.</li>
        <li><b>Get rCredits / USD</b> is where you trade US Dollars for rCredits or vice versa. Since you get more for rCredits, you will probably want as many as you can easily spend. We are issuing rCredits slowly, to avoid any possibility of economic disruption, so there is a waiting line. Meanwhile, some participants will occasionally have more rCredits than they can spend. If you are in that position, use this tab to exchange rCredits for US Dollars.</li>
        <li><b>Find Businesses</b>. This is where you can find out who takes rCredits for what you want.</li>
        <li><b>Test SMS</b> is a temporary tab where you can test the Text Messaging even if you don't have a cell phone or cell service.</li>
      </ul>
      <h2>Upper Right Links</h2>
      <ul>
        <li><b>Account Settings</b> has several subtabs:
        <ul>
          <li><b>Basic Account Settings</b> is where you change your email address, password, picture, or time zone.</li>
          <li><b>Cell Phone Access</b> lets you connect your account to one or more cell phones, so you can use Text Messaging to buy and sell things with rCredits.</li>
          <li><b>Relations</b> lets you connect individual rCredits participants to your account &mdash; either as owners, employees, or others, with limited or full access to the account. This is especially useful for authorizing sales clerks or setting up joint accounts.</li>
          <li><b>Contact Information</b> is where you add or change your phone numbers and address.</li>
          <li><b>Account Preferences</b> gives you several choices about how your rCredits account works. This section will be growing. Feel free to suggest additional options that you would like to see here.</li>
        </ul>
        </li>
      <li><b>Membership Status</b> tells you where you are in the process of becoming an "rTrader" &mdash; an active participant in the rCredits system. This section will give you links to your next steps.</li>
      <li><b>Account</b> shows the balance in your current account. If you have access to more than one account (that is, if some other account names you in the Relations section), then use this dropdown list to select an account to manage.</li>
      <li><b>Help</b> is this page.</li>
      <li><b>Log out</b> protects your account from unauthorized passersby.</li>
    </ul>
    <p>If you need help and can't find the answer here, please feel free to <a href="mailto:info@rCredits.org?subject=help with rCredits">email us</a>.</p>
EOF
    ),
    'smartphone' => t(<<<EOF
        <p>To use the smartphone interface, you will need to install an app to read QR codes (for example QRDroid on Android phones or Qrafter for iPhones). You can pay participating businesses by scanning their QR code, then visiting the scanned URL.</p>
        <p>Your customers (or employer) can pay you by scanning the QR code on your member ID card. You can print an Advance ID card for your individual account, using the link on the Summary page.</p>
EOF
    ),
    'sms' => t("The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the <a href=\"$base_url/sms\">Test SMS form</a> to simulate sending a text."),

    'summary' => t(<<<EOF
      <p>This page shows an overview of your rCredits account.</p>
      <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region. The region name is shown in parentheses after your account ID. The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
      <p>If you are acting as an agent for someone else's account &mdash; for example if you are a sales clerk for a department store &mdash; the period will be a colon instead. Your ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) will identify both you and the company.</p>
      $companySummary
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
      <p><b>Purpose:</b> If this is a purely financial transaction, click on "cash/loan/etc". Otherwise, a description is required (for purchases and sales of real goods and services). Donations to nonprofit organizations count as purchases of real services &mdash; services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank &mdash; your statements will call the purpose "cash".</p>
EOF
    ),
    'directory' => t(<<<EOF
      <p>This is where you can find out who takes rCredits for what you want. Click on a business name to see its rCredits webpage.</p>
      <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you choose the United States "01" postal code region.</p>
EOF
    ),
    'account' => t(<<<EOF
      <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
      <p><b>Picture:</b> For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. Ideally, the picture should be no wider than it is tall. (otherwise it looks peculiar on their ID card). If you choose too skinny a picture, though, the bottom will get cropped off.</p>
      $companyAccount
EOF
    ),
    'cells' => t(<<<EOF
      <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with rCredits. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
EOF
    ),
    'relations' => t(<<<EOF
      <p>This is especially useful for authorizing sales clerks or setting up joint accounts. Or you might give your accountant "read" permission. For companies, the best practice is to use the account ONLY through an individual agent account, so that you always know who did what.</p>
      <p>Individuals marked as employees are the ones who will get paid in rCredits when you use the Pay form to pay "all". Yellow highlights show settings that need to be confirmed &mdash; either by you or by the other party.</p>
      <p>To change settings in the Employee? or Owner? column, click on + or -. To remove someone from the list, make them not an employee, not an owner, and no permissions (if they are already that way, you will have to change something and change it back). Be sure to "Save Changes".</p>
      <p>The permissions you select will limit what the person can do. If you give the person at least "Sell" permission (and save the record), you can print a Member ID Card for them to use on behalf of this account.</p>
EOF
    ),
    'contact' => t(<<<EOF
      <p><b>Full name:</b> If you change the name on the account, you will need to buy a new member ID card (or cards) and your account will be disabled until you supply new proof of identity. For companies, your automatic rCredits website address will also change.</p>
      <p><b>Postal code:</b> Your postal code determines what rCredits "Common Good Community" you are in.</p>
      <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
EOF
    ),
    'company' => t(<<<EOF
      <p>All of the information from this form goes on your automatic <a target="_blank" href="$base_url/member/$acctName">rCredits webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. Click on the link, to see the results.</p>
EOF
    ),
    'preferences' => t(<<<EOF
      <p>These settings are, by default, what most people prefer. Feel free to adjust them to your taste. You can always come back and change them again later.</p>
      <ul>
        <li><b>Minimum.</b> You will want to keep enough rCredits plus US Dollars in your account to cover the total amount you expect to spend at rCredits member businesses in a typical 3-day period. Add to this however much you want to hold as inflation-protected savings (the inflation adjustment is a monthly credit to your account).</li>
        <li><b>To bank.</b> This setting is usually a good idea unless you use your Dwolla account outside the rCredits system.</li>
      </ul>
      <p>If there are settings you would like to see here &mdash; that is, things you want to work differently &mdash; please let us know. We expect to offer many additional options over the next few months.</p>
EOF
    ),
    'why-wait' => t(<<<EOF
      <p>Exchanges of rCredits for US Dollars reflect an incomplete economic circle. That is, the individual or company selling the rCredits does not yet have enough places to spend them.</p>
      <p>The 2-week delay in spending rCredits that you buy takes that amount out of circulation temporarily, giving the seller time to find more places to spend rCredits, without meanwhile having to sell more and more of them.</p>
EOF
    ),
    'to-bank' => t(<<<EOF
      <p>This "To bank" setting is usually a good idea unless you use your Dwolla account outside the rCredits system. It can take up to 3 days for the bank to receive the funds, once they are sent.</p>
EOF
    ),
  );

  $helps['charge'] = strtr($helps['pay'], array('pay' => 'charge', $paySpecial => ''));  
  return u\SUBS(@$helps[strtolower($page)]) ?: tt('no help', compact('page'));
  //        <p>Download the app from <a href="http://rCredits.org/rc.apk">rCredits.org/rc.apk</a>, install, run. Try all the buttons.</p>

}
