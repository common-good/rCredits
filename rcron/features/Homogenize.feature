Feature: Homogenize
AS a community
WE WANT our US Dollars to be distributed somewhat evenly throughout the members
SO we can do most transactions in rCredits and thereby avoid US Dollar transfers and their fees

Setup:
  Given members:
  | id   | fullName   | email | flags              | floor |
  | .ZZA | Abe One    | a@    | dft,ok,person,bona |     0 |
  | .ZZB | Bea Two    | b@    | dft,ok,person,bona |   -20 |
  | .ZZC | Corner Pub | c@    | dft,ok,company     |     0 |
  | .ZZD | Dee Four   | d@    | dft,ok,person      |     1 |
  | .ZZE | Eve Five   | e@    | dft,ok,person      |   -50 |
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
  | .ZZD |  0 |   0 |
  | .ZZE |  0 |  40 |

Scenario: Normal leveling happens
  When cron runs "homogenize"
  Then balances:
  | id   | r   | usd |
  | .AAA |   0 |   0 |
  | .AAB |   0 |   0 |
  | .ZZA |   0 |  20 |
  | .ZZB | -18 |  20 |
  | .ZZC |  35 |  70 |
  | .ZZD |   0 |   0 |
  | .ZZE |   0 |  40 |
