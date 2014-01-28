Feature: Sharing
AS a member
I WANT to contribute part of my rebates and bonuses to CGF monthly
SO CGF can continue to promote and maintain the rCredits system for my benefit and everyone's

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags                      | share |
  | .ZZA | Abe One    | -100  | personal    | dft,ok,person,bona         |    50 |
  | .ZZB | Bea Two    | -200  | personal    | dft,ok,person,company,bona |    10 |
  | .ZZC | Corner Pub | -300  | corporation | dft,ok,company,bona        |     0 |
  When transactions: 
  | xid | created | type     | state    | amount | from | to   | purpose |
  |   1 | %today  | transfer | done     |     40 | .ZZA | .ZZB | what G  |
  |   2 | %today  | rebate   | done     |      2 | ctty | .ZZA | rebate  |
  |   3 | %today  | bonus    | done     |      4 | ctty | .ZZB | bonus   |

Scenario: Inflation adjustments are distributed
  When cron runs "lessOften"
  Then gifts:
  | id   | giftDate | amount | often | honor  | share |
  | .ZZA | %today   |      1 |     1 | share  |    -1 |
  | .ZZB | %today   |   0.40 |     1 | share  |    -1 |
  And we notice "share gift" to member ".ZZA" with subs:
  | share |
  | 50.0  |
  And we notice "share gift" to member ".ZZB" with subs:
  | share |
  | 10.0  |
  And balances:
  | id   | committed |
  | .ZZA |         0 |
  | .ZZB |         0 |
  When cron runs "gifts"
  Then transactions: 
  | xid | created| type     | state | amount | from | to   | purpose |
  |   4 | %today | transfer | done  |      1 | .ZZA | cgf | sharing rewards with CGF |
  |   7 | %today | transfer | done  |   0.40 | .ZZB | cgf | sharing rewards with CGF |
  # plus reward transactions