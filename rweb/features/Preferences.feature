Feature: Preferences
AS a member
I WANT to set certain preferences
SO I can automate and control the behavior of my rCredits account.

Setup:
  Given members:
  | id   | share | minimum | savingsAdd | saveWeekly | achMin | floor | flags   |*
  | .ZZA |    30 |     100 |          0 |          1 |     20 |    10 | ok,confirmed,bona,nosearch,paper |
  | .ZZB |    20 |     -10 |         10 |          0 |     50 |     0 | ok,confirmed,bona,weekly,secret |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  
Scenario: A member visits the preferences page
  When member ".ZZA" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Minimum |  100 |
  | Savings |  250 |
  | Save Weekly | 1 |
  | Min Transfer | 20 |
  | Share | 30% |
  And we show checked:
  | Email Notices | daily |
  | No Search | by name or account number only |
  And we show unchecked:
  | Statements | I will accept electronic |
  | Secret Balance | Don't let merchants |

Scenario: Another member visits the preferences page
  When member ".ZZB" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Minimum | -10 |
  And we show checked:
  | Email Notices | weekly |
  | Secret Balance | Don't let merchants |
  | Statements | I will accept electronic |
  And we show unchecked:
  | No Search | by name or account number only |
  
Scenario: A member changes preferences
  Given transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  |   3 | %today-1m | grant  |    250 | ctty | .ZZA | grant   |
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |     200 |     300 |         20 |     10 |    25 | monthly | electronic |        0 |         1 |
  Then members:
  | id   | share | minimum | savingsAdd | saveWeekly | achMin | flags   |*
  | .ZZA |    25 |     200 |         50 |         20 |  10 | member,ok,confirmed,bona,monthly,secret |

Scenario: A member chooses too low a minimum, with a positive balance
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose |*
  |   3 | %today  | transfer |    400 | .ZZB | .ZZA | stuff   |
  Then balances:
  | id   | balance |*
  | .ZZA |     400 |
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |       5 |     300 |         20 |     10 |    25 | monthly | electronic |        0 |         1 |
  Then we say "error": "min sub floor" with subs:
  | floor |*
  | $10   |
  
Scenario: A member chooses too low a minimum, with a negative balance
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose |*
  |   3 | %today  | transfer |    400 | .ZZA | .ZZB | stuff   |
  Then balances:
  | id   | balance |*
  | .ZZA |    -400 |
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |    -401 |     250 |         20 |     10 |    25 | monthly | electronic |        0 |         1 |
  Then we say "error": "min sub floor" with subs:
  | floor |*
  | $-400 |
  
Scenario: A member chooses too low a savings reserve
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |     200 |     249 |         20 |     10 |    25 | monthly | electronic |        0 |         1 |
  Then we say "error": "savings too low" with subs:
  | rewards |*
  | $250    |
  
Scenario: A member chooses negative weekly savings without any savings
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |     200 |     250 |        -20 |     10 |    25 | monthly | electronic |        0 |         1 |
  Then we say "error": "negative saveWeekly"

Scenario: A member chooses too low an ACH minimum
  When member ".ZZA" completes form "settings/preferences" with values:
  | minimum | savings | saveWeekly | achMin | share | notices | statements | nosearch | secretBal |*
  |     200 |     250 |         20 |      1 |    25 | monthly | electronic |        0 |         1 |
  Then we say "error": "bad achmin"