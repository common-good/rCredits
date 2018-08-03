Feature: Exchange
AS a member
I WANT to trade CG Credits for another member's USD or exchange of US Dollars or other currency
SO I can buy stuff I can't buy with CG Credits.

OR I WANT to trade exchange of US Dollars or other currency for another member's CG Credits
SO I can buy more with Common Good Credits and thereby support the community and the common good.

Setup:
  Given members:
  | id   | fullName   | floor | flags      |*
  | .ZZA | Abe One    |  -250 | ok,confirmed         |
  | .ZZB | Bea Two    |  -250 | ok,confirmed         |
  | .ZZC | Corner Pub |  -250 | ok,confirmed,co      |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  |   4 | %today-6m | grant  |    200 | ctty | .ZZA | heroism | 0      |
  Then balances:
  | id   | balance |*
  | ctty |    -200 |
  | .ZZA |     200 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to charge another member
  When member ".ZZA" completes form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_USD | cash    |
  Then we scrip "suggest-who" with subs:
  | question                 | allowNonmember |*
  | Charge %amount to %name? |              1 |
#  Then we show "confirm charge" with subs:
#  | amount | otherName |*
#  | $100   | Bea Two   |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_USD | paper   |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And we message "new invoice" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | paper   |
  And invoices:
  | nvid | created | status      | amount | goods      | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | %FOR_USD | .ZZB | .ZZA | paper |
  And balances:
  | id   | balance |*
  | ctty |    -200 |
  | .ZZA |     200 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 100    | %FOR_USD | paper   |
  Then we scrip "suggest-who" with subs:
  | question              | allowNonmember |*
  | Pay %amount to %name? |                |
#  Then we show "confirm payment" with subs:
#  | amount | otherName |*
#  | $100   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 100    | %FOR_USD | paper   |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Bea Two   | $100   |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One   | $100   | paper        |
  And transactions:
  | xid | created | type     | amount | rebate | bonus | from  | to   | goods    | purpose      | taking |*
  |   5 | %today  | transfer |    100 |      0 |     0 | .ZZA  | .ZZB | %FOR_USD | paper        | 0      |
  And balances:
  | id   | balance |*
  | ctty |    -200 |
  | .ZZA |     100 |
  | .ZZB |     100 |
  | .ZZC |       0 |
Scenario: A member asks to cash out too much
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %FOR_USD | paper   |
  Then we say "error": "short to" with subs:
  | short |*
  | $100  |
