Feature: A member has options
AS a member
I WANT to see and select from my options
SO I can easily participate and manage my participation

Scenario: User logs in
Given user @email1 has a confirmed account
When user @email1 submits username and password
Then we say ‘choose’ with options:
| option |
| pay |
| charge |
| cash exchange |
| manage account |

Scenario: User chooses to manage account
Given user @email1 is logged in
When user @email1 clicks ‘manage account’
Then we say ‘choose’ with options:
| option |
| get r |
| get usd |
| review transactions |
| manage nicknames |
| account settings |
| choose a different account to manage (if you have more than one) |

_____________________

Scenario: User chooses to pay someone
Given user @email1 is logged in
When user @email1 clicks ‘pay’
Then we ask for:
| field |
| amount |
| other |
| what |
# pay AMOUNT to OTHER for WHAT
Scenario: User chooses to charge someone
Given user @email1 is logged in
When user @email1 clicks ‘charge’
Then we ask for:
| field |
| amount |
| other |
| what |
# charge OTHER AMOUNT for WHAT

Scenario: User chooses to do a cash exchange
Given user @email1 is logged in
When user @email1 clicks ‘cash exchange’
Then we ask for:
| field |
| amount |
| other |
| way |
# exchange AMOUNT with OTHER WAY (cash for rCredits OR rCredits for cash)