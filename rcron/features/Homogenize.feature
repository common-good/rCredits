Feature: Homogenize
AS a community
WE WANT our US Dollars to be distributed somewhat evenly throughout the members
SO we can do most transactions in rCredits and thereby avoid US Dollar transfers and their fees

Setup:
  Given members:
  | id   | fullName   | email | flags              | floor |
  | .ZZA | Abe One    | a@    | dft,ok,dw,person,bona |     0 |
  | .ZZB | Bea Two    | b@    | dft,ok,dw,person,bona |   -20 |
  | .ZZC | Corner Pub | c@    | dft,ok,dw,company     |     0 |
  | .ZZD | Dee Four   | d@    | dft,ok,dw,person      |     1 |
  | .ZZE | Eve Five   | e@    | dft,ok,dw,person      |   -50 |
  | .ZZF | Flo Six    | f@    | dft,ok,dw,person      |     0 |
  | .ZZG | Guy Seven  | g@    | dft,ok,dw,person      |     0 |
  | .ZZH | Hal Eight  | h@    | dft,ok,dw,person      |     0 |
  | .ZZI | Ida Nine   | i@    | dft,ok,dw,person      |     0 |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |
  |   1 | %today-6m | signup |     11 | ctty | .ZZA | signup  |
  |   2 | %today-6m | signup |      2 | ctty | .ZZB | signup  |
  |   3 | %today-6m | signup |      5 | ctty | .ZZC | signup  |
  |   4 | %today-6m | signup |     35 | ctty | .ZZF | signup  |
  |   5 | %today-6m | signup |     45 | ctty | .ZZG | signup  |
  |   6 | %today-6m | signup |     55 | ctty | .ZZH | signup  |
  |   7 | %today-6m | signup |     65 | ctty | .ZZI | signup  |
  And balances:
  | id   | r    | usd |
  | ctty | -218 |   0 |
  | .AAA |    0 |   0 |
  | .AAB |    0 |   0 |
  | .ZZA |   11 |  11 |
  | .ZZB |    2 |   0 |
  | .ZZC |    5 | 102 |
  | .ZZD |    0 |   0 |
  | .ZZE |    0 |  43 |
  | .ZZF |   35 |   4 |
  | .ZZG |   45 |   5 |
  | .ZZH |   55 |   6 |
  | .ZZI |   65 |   7 |

Scenario: Leveling happens
  When cron runs "homogenize"
  Then balances:
  | id   | r    | usd |
  | ctty | -218 |   0 |
  | .AAA |    0 |   0 |
  | .AAB |    0 |   0 |
  | .ZZA |    1 |  21 |
  | .ZZB |  -18 |  20 |
  | .ZZC |   65 |  42 |
  | .ZZD |    0 |   0 |
  | .ZZE |   20 |  23 |
  | .ZZF |   15 |  24 |
  | .ZZG |   35 |  15 |
  | .ZZH |   45 |  16 |
  | .ZZI |   55 |  17 |

  When cron runs "homogenize"
  Then balances:
  | id   | r   | usd |
  | ctty | -218 |   0 |
  | .AAA |    0 |   0 |
  | .AAB |    0 |   0 |
  | .ZZA |    1 |  21 |
  | .ZZB |  -18 |  20 |
  | .ZZC |   85 |  22 |
  | .ZZD |    0 |   0 |
  | .ZZE |   20 |  23 |
  | .ZZF |   15 |  24 |
  | .ZZG |   25 |  25 |
  | .ZZH |   35 |  26 |
  | .ZZI |   55 |  17 |

  When cron runs "homogenize"
  Then balances:
  | id   | r   | usd |
  | ctty | -218 |   0 |
  | .AAA |    0 |   0 |
  | .AAB |    0 |   0 |
  | .ZZA |    1 |  21 |
  | .ZZB |  -18 |  20 |
  | .ZZC |   85 |  22 |
  | .ZZD |    0 |   0 |
  | .ZZE |   20 |  23 |
  | .ZZF |   15 |  24 |
  | .ZZG |   25 |  25 |
  | .ZZH |   35 |  26 |
  | .ZZI |   55 |  17 |
# no change  