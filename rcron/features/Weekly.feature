Feature: Preferences
AS a member
I WANT certain preferences to be adjusted automatically every week
SO my rCredits account will make my financial position progressively better.

Setup:
  Given members:
  | id   | fullName | minimum | savingsAdd | saveWeekly | achMin | floor | risks   | flags   |*
  | .ZZA | Abe One  |    -100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,bona,refill |
  
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
