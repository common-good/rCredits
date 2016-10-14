Feature: Bank Info
AS a member
I WANT to connect a bank account to my rCredits account and set related settings
SO I can trade USD for rCredits and perhaps automate such trades.

Setup:
  Given members:
  | id   | fullName | minimum | achMin | floor | bankAccount | risks   | flags   |*
  | .ZZA | Abe One  |       0 |     20 |     0 |             |         | member,ok,confirmed,bona,ided |
  | .ZZB | Bea Two  |     -10 |     50 |    10 |      901234 | hasBank | member,ok,confirmed,bona,ided,refill |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  
Scenario: A member visits the bank info page before connecting
  When member ".ZZA" visits page "settings/bank"
  Then we show "Bank Information"
  And radio "connect" is "No"

Scenario: A member visits the bank info page after connecting
  When member ".ZZB" visits page "settings/bank"
  Then we show "Bank Information" with:
  | Account | xxx1234 |
  | Target  |     -10 |
  | Min Transfer | 50 |
  And radio "refills" is "Yes"

Scenario: A member connects a bank account
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       1 |     053000196 |         123 |          123 |       0 |     $0 |    $20 |         $0 |
  Then members:
  | id   | bankAccount      | last4bank | minimum | achMin | risks   | flags |*
  | .ZZA | USkk053000196123 | 6123      |       0 |     20 | hasBank | member,ok,confirmed,bona,ided |
  And we show "Bank Information"
  
Scenario: A member chooses not to connect a bank account
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       0 |               |             |              |       0 |     $0 |    $20 |         $0 |
  Then members have:
  | id   | risks |*
  | .ZZA |       |
  And we show "Bank Information"
  
Scenario: A member chooses automatic refills
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       1 |     053000196 |        1-23 |         1-23 |       1 |    $20 | $40.00 |          5 |
  Then members have:
  | id   | bankAccount      | last4bank | minimum | achMin | saveWeekly |*
  | .ZZA | USkk053000196123 | 6123      |      20 | 40     |          5 |
  And we show "Bank Information"

Scenario: A member disconnects a bank account
  When member ".ZZB" completes form "settings/bank" with values:
  | op     |*
  | remove |
  Then members have:
  | id   | bankAccount | last4bank | minimum | achMin | risks |*
  | .ZZB |             |           |     -10 |     50 |       |
  When member ".ZZB" visits page "settings/bank"
  Then we show "Bank Information"
  And radio "connect" is "No"
  
Scenario: A member gives a bad routing number
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       1 |           zot |         123 |          123 |       0 |     $0 |    $20 |         $0 |
  Then we say "error": "bad routing number"
  
Scenario: A member gives a bad account number
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       1 |     053000196 |         zot |          zot |       0 |     $0 |    $20 |         $0 |
  Then we say "error": "bad account number"

Scenario: A member gives mismatched account numbers
  When member ".ZZA" completes form "settings/bank" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  | submit |       1 |     053000196 |         123 |          456 |       0 |     $0 |    $20 |         $0 |
  Then we say "error": "mismatch" with subs:
  | thing          |*
  | account number |

Scenario: A member gives a bad target amount
  When member ".ZZB" completes form "settings/bank" with values:
  | op     | refills | target | achMin | saveWeekly |*
  | submit |       1 |    zot |     10 |          0 |
  Then we say "error": "TARGET: The amount must be a number."
  
Scenario: A member gives a bad minimum transfer amount
  When member ".ZZB" completes form "settings/bank" with values:
  | op     | refills | target | achMin | saveWeekly |*
  | submit |       1 |      5 |    zot |          0 |
  Then we say "error": "ACHMIN: The amount must be a number."

Scenario: A member chooses too low a target, with a positive balance
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose |*
  |   3 | %today  | transfer |    400 | .ZZA | .ZZB | stuff   |
  Then balances:
  | id   | balance |*
  | .ZZB |     400 |
  When member ".ZZB" completes form "settings/bank" with values:
  | op     | refills | target | achMin | saveWeekly |*
  | submit |       1 |      5 |     10 |          0 |
  Then we say "error": "min sub floor" with subs:
  | floor |*
  | $10   |
  
Scenario: A member chooses too low a target, with a negative balance
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose |*
  |   3 | %today  | transfer |    400 | .ZZB | .ZZA | stuff   |
  Then balances:
  | id   | balance |*
  | .ZZB |    -400 |
  When member ".ZZB" completes form "settings/bank" with values:
  | op     | refills | target | achMin | saveWeekly |*
  | submit |       1 |   -401 |     10 |          0 |
  Then we say "error": "min sub floor" with subs:
  | floor |*
  | $-400 |
  
Scenario: A member chooses too low an ACH minimum
  When member ".ZZB" completes form "settings/bank" with values:
  | op     | refills | target | achMin | saveWeekly |*
  | submit |       1 |    200 |      0 |          0 |
  Then we say "error": "bad achmin"
  
#Scenario: A member chooses negative weekly savings without any savings
#  When member ".ZZA" completes form "settings/preferences" with values:
#  | minimum | savingsAdd | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
#  |     200 |          0 |        -20 |     10 |    25 | monthly | electronic |        0 |         1 |
#  Then we say "error": "negative saveWeekly"
