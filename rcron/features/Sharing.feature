Feature: Sharing
AS a member
I WANT to donate part of my rebates and bonuses to CGF monthly
SO CGF can continue to promote and maintain the rCredits system for my benefit and everyone's

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags                  | share | rebate |*
  | .ZZA | Abe One    | -100  | personal    | ok,confirmed,bona,debt |    50 |      5 |
  | .ZZB | Bea Two    | -200  | personal    | ok,confirmed,co,bona   |    10 |     10 |
  | .ZZC | Corner Pub | -300  | corporation | ok,confirmed,co,bona   |     0 |     10 |
  When transactions: 
  | xid | created | type     | amount | from | to   | purpose |*
  |   1 | %today  | transfer |     40 | .ZZA | .ZZB | what G  |
  |   2 | %today  | rebate   |      2 | ctty | .ZZA | rebate on #1 |
  |   3 | %today  | bonus    |      4 | ctty | .ZZB | bonus on #1  |

Scenario: members share their transaction rewards
  When cron runs "lessOften"
  Then gifts:
  | id   | giftDate | amount | often | honor  | share |*
  | .ZZA | %today   |      1 |     1 | share  |    -1 |
  | .ZZB | %today   |   0.40 |     1 | share  |    -1 |
  And we notice "share gift" to member ".ZZA" with subs:
  | share |*
  | 50    |
  And we notice "share gift" to member ".ZZB" with subs:
  | share |*
  | 10    |
  And balances:
  | id   | committed |*
  | .ZZA |         0 |
  | .ZZB |         0 |
  When cron runs "gifts"
  Then transactions: 
  | xid | created| type     | amount | from | to   | purpose |*
  |   4 | %today | transfer |      1 | .ZZA | cgf | sharing rewards with CGF |
  |   5 | %today | transfer |   0.40 | .ZZB | cgf | sharing rewards with CGF |
  # plus reward transactions
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount |*
  | $1     |
  And we notice "gift sent" to member ".ZZB" with subs:
  | amount |*
  | $0.40  |

Scenario: periodic test timer works
  Then oftenness works
  