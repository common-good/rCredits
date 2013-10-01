Feature: Homogenize
AS a community
WE WANT our US Dollars to be distributed somewhat evenly throughout the members
SO we can do most transactions in rCredits and thereby avoid US Dollar transfers and their fees

Setup:
  Given members:
  | id   | fullName   | email | flags              |
  | .ZZA | Abe One    | a@    | dft,ok,person,bona |
  | .ZZB | Bea Two    | b@    | dft,ok,person,bona |
  | .ZZC | Corner Pub | c@    | dft,ok,company     |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |
  |   1 | %today-6m | signup |     10 | ctty | .ZZA | signup  |
  |   2 | %today-6m | signup |      2 | ctty | .ZZB | signup  |
  |   3 | %today-6m | signup |      5 | ctty | .ZZC | signup  |
  And balances:
  | id   | r  | usd |
  | .AAA |  0 |   0 |
  | .AAB |  0 |   0 |
  | .ZZA | 10 |  10 |
  | .ZZB |  2 |   0 |
  | .ZZC |  5 | 100 |

Scenario: Normal leveling happens
  When cron runs "homogenize"
  Then balances:
  | id   | r   | usd |
  | .AAA | -10 |  10 |
  | .AAB | -10 |  10 |
  | .ZZA |   0 |  20 |
  | .ZZB |  -8 |  10 |
  | .ZZC |  45 |  60 |
