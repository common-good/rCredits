<?php
namespace rCredits\Web;
use rCredits as r;

function helpText($page) {
  global $base_url;
  $cacct = r\acct();

  $companySummary = ($cacct->account_type == R_PERSONAL or !$cacct->proSe())? '' : t('<p>You cannot print a Member ID Card for the company, because ID cards are for individuals, not companies. However, in the <a href="$base_url/account/relations">Account Relations</a> section you can print an ID card for each person authorized to buy and/or sell on behalf of the company.</p>');
  $companyAccount = ($cacct->account_type == R_PERSONAL) ? '' : t(<<<EOF
  <p>For companies, upload a picture to represent your business. This picture will appear on your automatic rCredits webpage: <a target="_blank" href="$base_url/member/$cacct->name">devcore.rCredits.org/member/$cacct->name</a> -- so choose wisely. See also the <a href="$base_url/account/company-info">Company Info</a> form.</p>
EOF
  );

  $helps = array(
    'general' => t(<<<EOF
      <p>Thank you for helping test the rCredits Web Browser Interface. (Help is also available for the <a href="$base_url/help/smartphone">Android Smartphone Interface</a> and the <a href="$base_url/help/sms">Text Messaging Interface</a>.)
      Use the tabs above and the links at the top right, to try all the features of the rCredits system. Here's an overview of what's where:</p>
      <h2>Tabs:</h2>
      <ul>
        <li><b>Summary</b> gives you an overview of your account.</li>
        <li><b>Transactions</b> shows your recent transactions so you can accept, edit, print, or undo them.</li>
        <li><b>Charge</b> lets you invoice another participant (to be paid in rCredits). Or, if you have a qualified business account, you can charge them. If the transaction is for real goods or services, both buyer and seller get an incentive reward of additional rCredits.</li>
        <li><b>Pay</b> lets you transfer your rCredits to another participant. If the transaction is for real goods or services, both buyer and seller get an incentive reward of additional rCredits.</li>
        <li><b>Get rCredits / USD</b> is where you trade US Dollars for rCredits or vice versa. Since you get more for rCredits, you will probably want as many as you can easily spend. We are issuing rCredits slowly, to avoid any possibility of economic disruption, so there is a waiting line. Meanwhile, some participants will occasionally have more rCredits than they can spend. If you are in that position, use this tab to exchange rCredits for US Dollars.</li>
        <li><b>Find Businesses</b>. Once we begin for real, this is where you can find out who takes rCredits for what you want. Meanwhile, you can "AS-IF" buy things from anyone -- whether they have signed up or not. Once they are an active participant, you will get real rCredits rewards for the realistic "AS-IF" purchases you made. Since we are launching in Greenfield, Massachusetts, most businesses there are already listed, for your convenience.</li>
        <li><b>Test SMS</b> is a temporary tab where you can test the Text Messaging even if you don't have a cell phone or cell service.</li>
      </ul>
      <h2>Upper Right Links</h2>
      <ul>
        <li><b>Account Settings</b> has several subtabs:
        <ul>
          <li><b>Basic Account Settings</b> is where you change your email address, password, picture, or time zone.</li>
          <li><b>Cell Phone Access</b> lets you connect your account to one or more cell phones, so you can use Text Messaging to buy and sell things with rCredits.</li>
          <li><b>Relations</b> lets you connect individual rCredits participants to your account -- either as owners, employees, or others, with limited or full access to the account. This is especially useful for authorizing sales clerks or setting up joint accounts.</li>
          <li><b>Contact Information</b> is where you add or change your phone numbers and address.</li>
          <li><b>Account Options</b> gives you several choices about how your rCredits account works. This section will be growing. Feel free to suggest additional options that you would like to see here.</li>
        </ul>
        </li>
      <li><b>Membership Status</b> tells you where you are in the process of becoming an "rTrader" -- an active participant in the rCredits system. This section will give you links to your next steps.</li>
      <li><b>Account</b> shows the balance in your current account. If you have access to more than one account (that is, if some other account names you in the Relations section), then use this dropdown list to select an account to manage.</li>
      <li><b>Help</b> is this page.</li>
      <li><b>Log out</b> protects your account from unauthorized passersby.</li>
    </ul>
EOF
    ),
    'smartphone' => t(<<<EOF
        <p>Before you try the smartphone app (Android only, until January), you may want to create a business account and another individual account -- or test with a friend. From the Summary tab, print a Temporary Member ID Card for each account.</p>
        <p>Download the app from <a href="http://rCredits.org/rc.apk">rCredits.org/rc.apk</a>, install, run. Try all the buttons.</p>
EOF
    ),
    'sms' => t('The Text Message Interface is intended to be easy enough that you can just muddle your way through the first time. Give it a shot. You can even <i>sign up</i> with Text Messaging. Send your texts to +1 413.285.2867 OR use the Test SMS form to simulate sending a text.'),

    'summary' => t(<<<EOF
      <p>This page shows an overview of your rCredits account.</p>
      <p><b>Account ID</b> has two parts. The part before the period is a 3-character identifier for your region. The region name is shown in parentheses after your account ID. The part after the period uniquely identifies your account, within your region. If you have several accounts, this part will be different for each one.</p>
      <p>If you are acting as an agent for someone else's account -- for example if you are a sales clerk for a department store -- the period will be a colon instead. Your ID Card will show your picture, with the company name under yours. The second part of the account ID (and the scannable code) will identify both you and the company.</p>
      $companySummary
EOF
    ),
    'txs' => t(<<<EOF
      <p>Select the period you want to see transactions for. Both the Summary and Transactions sections will reflect just the transactions in that period. However, the "Pending" line will reflect all your not-yet-accepted invoices and offers EVER. The Ending balance in the Summary is always your current balance.</p>
      <p>Transactions numbers begin with "a" because they are "AS-IF" transactions.</p>
      <p>If the status is "pending", either you or the other party will need to click on "OK" to complete the deal. Click on the "X" to reject an invoice, dispute a charge, reject an offer, or cancel or undo something you did. Click on the pencil to edit the description. The description is just for you -- the other party will not see it.</p>
      <p>The Rewards column shows how much rCredits were added to your account as a reward for joining or for buying or selling real goods and services. Loans, exchanges for US Dollars, and other purely financial deals do not earn rewards.</p>
EOF
    ),
    'tx' => t(<<<EOF
      <p><b>Whom:</b> Type all or part of the full name of the account you want to pay or charge. You can leave out the punctuation, spaces, and capitals. Or you can type their phone number or email address or their account ID. Note that a period in the account ID (a regular account) is different from a colon (an agent acting on behalf of an account). If you have employees you can say pay "ALL", to pay each employee the same amount.</p>
      <p><b>Amount:</b> The amount of the transaction, measured in dollars, to be paid in rCredits.</p>
      <p><b>Purpose:</b> If this is a purely financial transaction, click on "cash/loan/etc". Otherwise, a description is required (for purchases and sales of real goods and services). Donations to nonprofit organizations count as purchases of real services -- services to society. For loans, cash exchanges, and other money deals, you can leave the purpose blank -- your statements will call the purpose "cash".</p>
EOF
    ),
    'directory' => t(<<<EOF
      <p>Once we begin for real, this is where you can find out who takes rCredits for what you want. Meanwhile, most businesses in Greenfield, Massachusetts (postal code 01301 -- our launch site) are already listed, for your convenience. Feel free to do "AS-IF" transactions with the ones you plan to buy from or sell to. Click on a business name to see its automatic rCredits webpage. (The photos were captured robotically from the internet, so some are less than perfect.)</p>
      <p>You can limit the results to a specific industry or a specific business by typing all or part of the industry or business name. Currently, if you limit the results geographically, there won't be any results unless you happen to live in the United States "01" postal code region.</p>
EOF
    ),
    'account' => t(<<<EOF
      <p><b>Current password:</b> To change your email address or password, you must supply your current password, as a security precaution.</p>
      <p><b>Picture:</b> For an individual account you will need to upload a clear photo of your face, for identification. This is the photo that will appear on your Member ID card. Ideally, the picture should be no wider than it is tall. (otherwise it looks peculiar on their ID card). If you choose too skinny a picture, though, the bottom will get cropped off.</p>
      $companyAccount
EOF
    ),
    'cell' => t(<<<EOF
      <p>Add one or more cell phones to your account so you can use Text Messaging to buy and sell things with rCredits. The form will send a secret word to your phone, which you can then enter here to confirm the connection.</p>
EOF
    ),
    'relations' => t(<<<EOF
      <p>Connect individual rCredits participants to your account -- either as owners, employees, or others, with limited or full access to the account. This is especially useful for authorizing sales clerks or setting up joint accounts. For companies, the best practice is to use the account ONLY through an individual agent account, so that you always know who did what.</p>
      <p>To change the setting in the Employee? or Owner? column, click on + or -. The permissions you select in the Permission column will limit what the person can do with this account.</p>
      <p>If you give the person at least "Sell" permission (and save the record), you can click on a link in the final column, to print a Temporary Member ID Card for them to use on behalf of this account.</p>
      <p>Remember to click on "Save Changes".</p>
      <p>To remove someone from the list, make them not an employee, not an owner, and no permissions (if they are already that way, you will have to change something and change it back).</p>
EOF
    ),
    'contact' => t(<<<EOF
      <p><b>Full name:</b> If you change the name on the account, you will need to buy a new member ID card (or cards) and your account will be disabled until you supply new proof of identity. For companies, your automatic rCredits website address will also change.</p>
      <p><b>Postal code:</b> Your postal code determines what rCredits "Common Good Community" you are in.</p>
      <p><b>All other settings on this page</b> are private for individual accounts, public for company accounts.</p>
EOF
    ),
    'company' => t(<<<EOF
      <p>All of the information from this form goes on your automatic <a target="_blank" href="$base_url/member/$cacct->name">rCredits webpage</a>, along with your company name and profile picture. Your email address will not be included unless you put it in the description explicitly. Click on the link, to see the results.</p>
EOF
    ),
    'options' => t(<<<EOF
      <p>These settings are, by default, what most people prefer. Feel free to adjust them to your taste. You can always come back and change them again later.</p>
      <p>If there are settings you would like to see here -- that is, things you want to work differently -- please let us know. We expect to offer many additional options over the next few months.</p>
EOF
    ),
  );
  return @$helps[$page] ?: t("This feature is pretty intuitive. Give it a try, then call us if you can't figure it out.");
}
