Feature: Gift
AS a participating business
I WANT to issue gift coupons and discount coupons
SO I can reward my employees and attract customers
AS a member
I WANT to redeem a coupon or issue gift coupons
SO I can pay less for stuff or treat a friend.

Setup:
  Given members:
  | id   | fullName   | floor | flags             |*
  | .ZZA | Abe One    |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub |  -250 | ok,confirmed,co   |  

Scenario: A member redeems a gift coupon
  Given members have:
  | id   | giftCoupons | created |*
  | .ZZC |           8 | 0039200 |
# created determines 3-letter lowSecurity code (7AA), which is used in coupon code
  When member ".ZZC" completes form "community/coupons/type=gift" with values:
  | type | amount | count |*
  | gift |     10 |    20 |
  Then coupons:
  | coupid | fromId | amount | ulimit | flags | start | end |*
  |      1 |   .ZZC |     10 |      1 |     1 |     8 |  28 |
#  And member ".ZZC" visits page "community/coupons/print/type=gift&amount=10&ulimit=1&count=20", which results in:
#  When member ".ZZC" visits page "community/coupons/print/type=gift&amount=10&count=20"
  And members have:
  | id   | giftCoupons |*
  | .ZZC |          28 |
  When member ".ZZA" completes form "community/coupons/type=gift" with values:
  | type   | code          |*
  | redeem | DD7K CLJW EAI |
  Then balances:
  | id   | balance |*
  | .ZZA |      10 |
  | .ZZC |     -10 |
  And members have:
  | id   | giftPot |*
  | .ZZA |      10 |
  When member ".ZZB" completes form "community/coupons/type=gift" with values:
  | type   | code          |*
  | redeem | DD7K CLJW EAI |
  Then we say "error": "already redeemed"

Scenario: A member redeems a discount coupon for a dollar amount
  When member ".ZZC" completes form "community/coupons/type=discount" with values:
  | type     | amount | minimum | start | end     | ulimit | automatic |*
  | discount |     12 |      20 | %mdY  | %mdY+9d |      1 |         1 |
  Then coupons:
  | coupid | amount | minimum | ulimit | flags | start  | end       |*
  |      1 |     12 |      20 |      1 |     0 | %today | %(%today+10d-1) |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 100    | fun     |
  Then we say "status": "report tx" with subs:
  | did    | otherName  | amount |*
  | paid   | Corner Pub | $100   |
  And transactions:
  | xid | created | type     | amount | from  | to   | purpose |*
  |   1 | %today  | transfer |    100 | .ZZA  | .ZZC | fun     |
  |   2 | %today  | transfer |    -12 | .ZZA  | .ZZC | rebate (discount coupon #1) |
  And balances:
  | id   | balance |*
  | .ZZA |     -88 |
  | .ZZB |       0 |
  | .ZZC |      88 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 40     | fun     |
  Then balances:
  | id   | balance |*
  | .ZZA |    -128 |
  | .ZZC |     128 |

Scenario: A member redeems a discount coupon for a dollar amount
  When member ".ZZC" completes form "community/coupons/type=discount" with values:
  | type     | amount | minimum | start | end     | ulimit | automatic |*
  | discount |    12% |      20 | %mdY  | %mdY+9d |      2 |         1 |
  Then coupons:
  | coupid | amount | minimum | ulimit | flags | start  | end       |*
  |      1 |    -12 |      20 |      2 |     0 | %today | %(%today+10d-1) |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | id   | balance |*
  | .ZZA |     -44 |
  | .ZZB |       0 |
  | .ZZC |      44 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | id   | balance |*
  | .ZZA |     -88 |
  | .ZZB |       0 |
  | .ZZC |      88 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | id   | balance |*
  | .ZZA |    -138 |
  | .ZZB |       0 |
  | .ZZC |     138 |
  When member ".ZZB" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | id   | balance |*
  | .ZZA |    -138 |
  | .ZZB |     -44 |
  | .ZZC |     182 |
