Feature: Reimburse
AS a member
I WANT to repay another member with rCredits
SO I can be cool and save time by not having to find cash or write a check.

Setup:
  Given members:
  | id   | fullName | address | city  | state  | postalCode | country | postalAddr | rebate | flags      |*
  | .ZZA | Abe One  | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK |      5 | ok,bona    |
  | .ZZB | Bea Two  | 2 B St. | Btown | Utah   | 02000      | US      | 2 B, B, UT |     10 | ok,bona    |
  | .ZZC | Our Pub  | 3 C St. | Ctown | Cher   |            | France  | 3 C, C, FR |     10 | ok,co,bona |
  And relations:
  | id   | main | agent | permission | draw |*
  | :ZZA | .ZZA | .ZZB  | buy        |    1 |
  | :ZZB | .ZZB | .ZZA  | read       |    0 |
  | :ZZC | .ZZC | .ZZB  | buy        |    0 |
  | :ZZD | .ZZC | .ZZA  | sell       |    0 |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  |   4 | %today-6m | grant  |    200 | ctty | .ZZA | heroism | 0      |
  Then balances:
  | id   | balance |*
  | ctty | -950 |
  | .ZZA |  450 |
  | .ZZB |  250 |
  | .ZZC |  250 |

Scenario: A member asks to charge another member
  When member ".ZZA" completes form "charge" with values:
  | op     | who     | amount | goods           | purpose |*
  | charge | Bea Two | 100    | %R_FOR_NONGOODS | payback |
  Then we show "confirm charge" with subs:
  | amount | otherName | why         |*
  | $100   | Bea Two   | loan/reimbursement/etc. |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %R_FOR_NONGOODS | payback |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount | why         |*
  | charged | Bea Two   | $100   | loan/reimbursement/etc. |
  And we notice "new invoice" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | purpose |*
  | %today  | Bea Two  | Abe One   | $100   | payback |
  And invoices:
  | nvid | created | status      | amount | goods      | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | %R_FOR_NONGOODS | .ZZB | .ZZA | payback |
  And balances:
  | id   | balance |*
  | ctty |    -950 |
  | .ZZA |     450 |
  | .ZZB |     250 |
  | .ZZC |     250 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %R_FOR_NONGOODS | payback |
  Then we show "confirm payment" with subs:
  | amount | otherName | why         |*
  | $300   | Bea Two   | loan/reimbursement/etc. |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %R_FOR_NONGOODS | payback |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount | why         |*
  | paid   | Bea Two   | $300   | loan/reimbursement/etc. |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One   | $300   | payback      |
  And transactions:
  | xid | created | type     | amount | from  | to   | goods           | purpose      | taking |*
  |   5 | %today  | transfer |    300 | .ZZA  | .ZZB | %R_FOR_NONGOODS | payback      | 0      |
  And balances:
  | id   | balance |*
  | ctty |    -950 |
  | .ZZA |     150 |
  | .ZZB |     550 |
  | .ZZC |     250 |

Scenario: A member repays someone, drawing from another account
  When member ".ZZB" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Our Pub | 300    | %R_FOR_NONGOODS | payback |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount | why         |*
  | paid   | Our Pub   | $300   | loan/reimbursement/etc. |
  And we notice "new payment" to member ".ZZC" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Our Pub  | Bea Two   | $300   | payback      |
  And transactions:
  | xid | created | type     | amount | from  | to   | goods           | purpose |*
  |   5 | %today  | transfer |     50 | .ZZA  | .ZZB | %R_FOR_NONGOODS | automatic transfer to NEW.ZZB,automatic transfer from NEW.ZZA |
  |   6 | %today  | transfer |    300 | .ZZB  | .ZZC | %R_FOR_NONGOODS | payback |
  And balances:
  | id   | balance |*
  | ctty |    -950 |
  | .ZZA |     400 |
  | .ZZB |       0 |
  | .ZZC |     550 |
