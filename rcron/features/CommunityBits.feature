Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | zip | country | postalAddr | flags        |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK | ok,confirmed |
  And balances:
  | id   | balance | floor |*
  | cgf  |       0 |     0 |
  | .ZZA |     100 |   -20 |

Scenario: Community bans spending below zero
  Given members have:
  | id   | flags    |*
  | ctty | ok,up,co |
  And stats:
  | created    | ctty | usdIn | usdOut |*
  | %today-90d | ctty |   200 |     80 |
  | %today-60d | ctty |   201 |     90 |
  | %today-30d | ctty |   202 |    100 |
  | %today     | ctty |   203 |    110 |
  When cron runs "cttyStats"
  Then we tell "ctty" CO "credit ban on" with subs:
  | months |*
  |      3 |
