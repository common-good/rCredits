Feature: Exchange
AS a member
I WANT to trade rCredits for another member's USD or other money
SO I can buy stuff I can't buy with rCredits.

OR I WANT to trade other money for another member's rCredits
SO I can buy more with rCredits and get incentive rewards.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | postalAddr | rebate | flags      |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK |      5 | ok,bona    |
  | .ZZB | Bea Two    | 2 B St. | Btown | Utah   | 02000      | US      | 2 B, B, UT |     10 | ok,bona    |
  | .ZZC | Corner Pub | 3 C St. | Ctown | Cher   |            | France  | 3 C, C, FR |     10 | ok,co,bona |
  And relations:
  | id   | main | agent | permission |*
  | :ZZA | .ZZA | .ZZB  | buy        |
  | :ZZB | .ZZB | .ZZA  | read       |
  | :ZZC | .ZZC | .ZZB  | buy        |
  | :ZZD | .ZZC | .ZZA  | sell       |
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
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %R_FOR_USD | cash    |
  Then we show "confirm charge" with subs:
  | amount | otherName | why         |*
  | $100   | Bea Two   | other money |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %R_FOR_USD | paper   |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount | why         |*
  | charged | Bea Two   | $100   | other money |
  And we notice "new invoice" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | purpose |*
  | %today  | Bea Two  | Abe One   | $100   | paper   |
  And invoices:
  | nvid | created | status      | amount | goods      | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | %R_FOR_USD | .ZZB | .ZZA | paper |
  And balances:
  | id   | balance |*
  | ctty |    -950 |
  | .ZZA |     450 |
  | .ZZB |     250 |
  | .ZZC |     250 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 100    | %R_FOR_USD | paper   |
  Then we show "confirm payment" with subs:
  | amount | otherName | why         |*
  | $100   | Bea Two   | other money |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 100    | %R_FOR_USD | paper   |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount | why         |*
  | paid   | Bea Two   | $100   | other money |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One   | $100   | paper        |
  And transactions:
  | xid | created | type     | amount | from  | to   | goods      | purpose      | taking |*
  |   5 | %today  | transfer |    100 | .ZZA  | .ZZB | %R_FOR_USD | paper        | 0      |
  And balances:
  | id   | balance |*
  | ctty |    -950 |
  | .ZZA |     350 |
  | .ZZB |     350 |
  | .ZZC |     250 |

Scenario: A member asks to cash out too much
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 300    | %R_FOR_USD | paper   |
  Then we say "error": "short to|short cash help" with subs:
  | short |*
  | $100  |
