Feature: Gift
AS a participating business
I WANT to issue gift coupons and discount coupons
SO I can reward my employees and attract customers
AS a member
I WANT to redeem a coupon or issue gift coupons
SO I can pay less for stuff or treat a friend.

Setup:
  Given members:
  | id   | fullName   | email | cc  | cc2  | floor | flags             |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 | ok,co,confirmed   |
  And devices:
  | id   | code |*
  | .ZZC | devC |
  And selling:
  | id   | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | id   | flags        |*
  | .ZZC | refund,r4usd |
  And relations:
  | reid | main | agent | num | permission |*
  | .ZZA | .ZZC | .ZZA  |   1 | scan       |
  
Scenario: A member redeems a gift coupon
  Given  coupons:
  | coupid | fromId | amount | minimum | ulimit | flags | start  | end       |*
  |      1 |   .ZZC |     10 |       0 |      1 |     0 | %today | %today+7d |
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at %today
  Then transactions: 
  | xid | created | amount | from | to   | purpose |*
  | 1   | %today  |    100 | .ZZB | :ZZA | food    |
  | 2   | %today  |    -10 | .ZZB | .ZZC | discount rebate (on #1) |
  
  When agent "C:A" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description | created |*
  | .ZZB   | ccB  | 100.00 |     1 | food        | %today  |
  Then transactions:
  | xid | created | amount | from | to   | purpose |*
  | 3   | %today  |   -100 | .ZZB | :ZZA | food (reverses #1)   |
  | 4   | %today  |     10 | .ZZB | .ZZC | discount rebate (on #2) |

  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $50 for "goods": "sundries" at %today
  Then transactions: 
  | xid | created | amount | from | to   | purpose |*
  | 5   | %today  |     50 | .ZZB | :ZZA | sundries |
  | 6   | %today  |    -10 | .ZZB | .ZZC | discount rebate (on #3) |
  
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $60 for "goods": "stuff" at %today
  Then transactions: 
  | xid | created | amount | from | to   | purpose |*
  | 7   | %today  |     60 | .ZZB | :ZZA | stuff   |
  And transaction count is 7
# ulimit has been reached, so no rebate