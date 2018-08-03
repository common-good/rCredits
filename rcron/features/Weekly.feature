Feature: Preferences
AS a member
I WANT certain adjustments to my account to be made automatically every week
SO my financial position will be progressively better.

Setup:
  Given members:
  | id   | fullName | minimum | savingsAdd | saveWeekly | achMin | floor | risks   | flags   |*
  | .ZZA | Abe One  |    -100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,refill  |
  | .ZZB | Bea Two  |     100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,cashoutW |
  
Scenario: A member crawls out of debt
  When cron runs "everyWeek"
  Then balances:
  | id   | minimum |*
  | .ZZA |     -80 |
  
Scenario: A member builds up savings
  Given members have:
  | id   | minimum |*
  | .ZZA |     100 |
  When cron runs "everyWeek"
  Then balances:
  | id   | minimum | savingsAdd |*
  | .ZZA |     120 |          0 |

#Scenario: A member draws down savings bit by bit
#  Given members have:
#  | id   | minimum | savingsAdd | saveWeekly | achMin |*
#  | .ZZA |     100 |         25 |        -20 |     20 |
#  When cron runs "everyWeek"
#  Then balances:
#  | id   | minimum | savingsAdd |*
#  | .ZZA |     100 |          5 |
#  When cron runs "everyWeek"
#  Then balances:
#  | id   | minimum | savingsAdd |*
#  | .ZZA |     100 |          0 |

Scenario: A member cashes out automatically
  Given transactions:
  | xid | created   | type     | amount | from | to   | purpose |*
  |   1 | %today-8w | signup   |    900 | ctty | .ZZA | signup  |
  |   2 | %today-7w | transfer |    200 | .ZZA | .ZZB | stuff   |
  |   3 | %today-6w | transfer |    500 | .ZZA | .ZZB | stuff   |
  And members have:
  | id   | activated | floor |*
  | .ZZB | %today-9w |  -100 |
  Then balances:
  | id   | balance |*
  | .ZZB |     700 |
  When cron runs "tickle"
  Then usd transfers:
  | txid | payee | amount |*
  |    1 | .ZZB  |   -680 |
#  And we notice "banked|bank tx number" to member ".ZZB" with subs:
#  | action     | amount | checkNum |*
#  | deposit to | $680   |        1 |
  And we notice "banked" to member ".ZZB" with subs:
  | action     | amount |*
  | deposit to | $680   |
