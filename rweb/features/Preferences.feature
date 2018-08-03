Feature: Preferences
AS a member
I WANT to set certain preferences
SO I can automate and control the behavior of my rCredits account.

Setup:
  Given members:
  | id   | crumbs | minimum | savingsAdd | saveWeekly | achMin | floor | flags   |*
  | .ZZA |    .01 |     100 |          0 |          1 |     20 |    10 | ok,confirmed,nosearch,paper |
  | .ZZB |    .02 |     -10 |         10 |          0 |     50 |     0 | ok,confirmed,weekly,secret |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  
Scenario: A member visits the preferences page
  When member ".ZZA" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Crumbs | 1 |
  And radio "statements" is "printed statements"
  And radio "notices" is "daily"
  And we show checked:
  | No Search | by name or account ID only |
  And we show unchecked:
  | Secret Balance | Don't let merchants |

Scenario: Another member visits the preferences page
  When member ".ZZB" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Crumbs | 2 |
  And radio "statements" is "accept electronic"
  And radio "notices" is "weekly"
  And we show checked:
  | Secret Balance | Don't let merchants |
  And we show unchecked:
  | No Search | by name or account ID only |
  
Scenario: A member changes preferences
  Given transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  |   3 | %today-1m | grant  |    250 | ctty | .ZZA | grant   |
  And member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal |*
  |       1 |      3 | monthly | electronic |        0 |         1 |
  Then members:
  | id   | crumbs |  flags   |*
  | .ZZA |    .03 | ok,confirmed,monthly,secret,roundup |
