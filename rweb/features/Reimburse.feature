Feature: Reimburse
AS a member
I WANT to repay another member with rCredits
SO I can be cool and save time by not having to find cash or write a check.

Setup:
  Given members:
  | id   | fullName | floor | flags             |*
  | .ZZA | Abe One  |  -100 | ok,confirmed,debt |
  | .ZZB | Bea Two  |     0 | ok,confirmed      |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co   |
  And relations:
  | main | agent | permission | draw |*
  | .ZZA | .ZZB  | buy        |    1 |
  | .ZZB | .ZZA  | read       |    0 |
  | .ZZC | .ZZB  | buy        |    0 |
  | .ZZC | .ZZA  | sell       |    0 |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  |   4 | %today-6m | grant  |    200 | ctty | .ZZA | heroism | 0      |
  Then balances:
  | id   | balance |*
  | .ZZA |     200 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to charge another member
  When member ".ZZA" completes form "charge" with values:
  | op     | who     | amount | goods           | purpose |*
  | charge | Bea Two | 100    | %FOR_NONGOODS | payback |
  Then we show "confirm charge" with subs:
  | amount | otherName |*
  | $100   | Bea Two   |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_NONGOODS | payback |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And we message "new invoice" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | payback |
  And invoices:
  | nvid | created | status      | amount | goods      | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | %FOR_NONGOODS | .ZZB | .ZZA | payback |
  And balances:
  | id   | balance |*
  | .ZZA |     200 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %FOR_NONGOODS | payback |
  Then we show "confirm payment" with subs:
  | amount | otherName |*
  | $300   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %FOR_NONGOODS | payback |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Bea Two   | $300   |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One   | $300   | payback      |
  And transactions:
  | xid | created | type     | amount | from  | to   | goods           | purpose      | taking |*
  |   5 | %today  | transfer |    300 | .ZZA  | .ZZB | %FOR_NONGOODS | payback      | 0      |
  And balances:
  | id   | balance |*
  | .ZZA |    -100 |
  | .ZZB |     300 |
  | .ZZC |       0 |

Scenario: A member repays someone, drawing from another account
  When member ".ZZB" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Our Pub | 300    | %FOR_NONGOODS | payback |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Our Pub   | $300   |
  And we notice "new payment linked" to member ".ZZC" with subs:
  | created | fullName | otherName | amount | payeePurpose | aPayLink |*
  | %today  | Our Pub  | Bea Two   | $300   | payback      | ?        |
  And transactions:
  | xid | created | type     | amount | from  | to   | goods           | purpose |*
  |   5 | %today  | transfer |    300 | .ZZA  | .ZZB | %FOR_NONGOODS | automatic transfer to NEWZZB,automatic transfer from NEWZZA |
  |   6 | %today  | transfer |    300 | .ZZB  | .ZZC | %FOR_NONGOODS | payback |
  And balances:
  | id   | balance |*
  | .ZZA |    -100 |
  | .ZZB |       0 |
  | .ZZC |     300 |
