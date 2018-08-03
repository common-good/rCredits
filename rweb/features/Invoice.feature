Feature: Invoice
AS a member
I WANT to charge other members and pay invoices from other members
SO I can buy and sell stuff.

Setup:
  Given members:
  | id   | fullName | floor | flags             |*
  | .ZZA | Abe One  |     0 | ok,confirmed      |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co   |
  | .ZZD | Dee Four |     0 |                   |
  And relations:
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup |    250 | ctty | .ZZC | signup  | 0      |
  Then balances:
  | id   | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then invoices:
  | nvid | created | status      | amount | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB | .ZZA | labor |
  And we message "new invoice" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |

  When member ".ZZB" visits page "handle-invoice/nvid=1&toMe=1"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | ~ | Pay |
  | Reason ||
  | ~ | Dispute |
  
  When member ".ZZB" confirms form "handle-invoice/nvid=1&toMe=1" with values:
  | op   | ret | nvid | amount | payer | payee | purpose | created |*
  | pay  |     |    1 |    100 | .ZZB  | .ZZA  | labor   | %today  |
  Then transactions:
  | xid | created | amount | from | to   | purpose                | taking |*
  |   4 | %today  |    100 | .ZZB | .ZZA | labor (%PROJECT inv#1)  | 0      |
  And invoices:
  | nvid | created | status | amount | from | to   | for   |*
  |    1 | %today  | 4      |    100 | .ZZB | .ZZA | labor |
  And balances:
  | id   | balance |*
  | .ZZA |     100 |
  | .ZZB |    -100 |
  | .ZZC |       0 |

Scenario: A member confirms request to charge a not-yet member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who      | amount | goods          | purpose |*
  | charge | Dee Four | 100    | %FOR_GOODS     | labor   |
  Then invoices:
  | nvid | created | status      | amount | from | to   | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZD | .ZZA | labor |
  And we message "new invoice" to member ".ZZD" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |

  When member ".ZZD" visits page "handle-invoice/nvid=1&toMe=1"
  Then we show "Confirm Payment" with:
  | ~question | Pay $100 to Abe One for labor |
  | ~ | Pay |
  | Reason ||
  | ~ | Dispute |

  When member ".ZZD" confirms form "handle-invoice/nvid=1&toMe=1" with values:
  | op   | ret | nvid | amount | payer | payee | purpose | created |*
  | pay  |     |    1 |    100 | .ZZD  | .ZZA  | labor   | %today  |
  Then invoices:
  | nvid | created | status       | amount | from | to   | for   |*
  |    1 | %today  | %TX_APPROVED |    100 | .ZZD | .ZZA | labor |
  And we say "status": "finish signup|when funded"
  
Scenario: A member denies an invoice
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1&toMe=1" with values:
  | op   | ret | nvid | amount | payer | payee | purpose | created | whyNot |*
  | deny |     |    1 |    100 | .ZZB  | .ZZA  | labor   | %today  | broke  |
  Then invoices:
  | nvid | created | status     | amount | from | to   | for   |*
  |    1 | %today  | %TX_DENIED |    100 | .ZZB | .ZZA | labor |
  And we notice "invoice denied" to member ".ZZA" with subs:
  | payerName | created | amount | purpose | reason |*
  | Bea Two   | %dmy    |   $100 | labor   | broke  |
  And balances:
  | id   | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member approves an invoice with insufficient funds
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 300    | %FOR_GOODS     | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1&toMe=1" with values:
  | op   | ret | nvid | amount | payer | payee | purpose | created | whyNot |*
  | pay  |     |    1 |    300 | .ZZB  | .ZZA  | labor   | %today  |        |
  Then invoices:
  | nvid | created | status       | amount | from | to   | for   |*
  |    1 | %today  | %TX_APPROVED |    300 | .ZZB | .ZZA | labor |
  And we say "status": "short invoice|when funded" with subs:
  | short | payeeName |*
  | $50   | Abe One   |
  And balances:
  | id   | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member approves invoices forevermore
  Given members have:
  | id   | risks   |*
  | .ZZB | hasBank |
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 300    | %FOR_GOODS | labor   |
  And member ".ZZB" confirms form "handle-invoice/nvid=1&toMe=1" with values:
  | op   | ret | nvid | amount | payer | payee | purpose | created | whyNot | always |*
  | pay  |     |    1 |    300 | .ZZB  | .ZZA  | labor   | %today  |        |      1 |
  Then invoices:
  | nvid | created | status       | amount | from | to   | for   |*
  |    1 | %today  | %TX_APPROVED |    300 | .ZZB | .ZZA | labor |
  And relations:
  | main | agent | flags            |*
  | .ZZA | .ZZB  | customer,autopay |

Scenario: A member approves an invoice to a trusting customer
  Given relations:
  | main | agent | flags            |*
  | .ZZA | .ZZB  | customer,autopay |
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods      | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS | labor   |
  Then transactions:
  | xid | created | amount | from | to   | purpose                | taking |*
  |   4 | %today  |    100 | .ZZB | .ZZA | labor (%PROJECT inv#1)  | 0      |
  And invoices:
  | nvid | created | status | amount | from | to   | for   |*
  |    1 | %today  | 4      |    100 | .ZZB | .ZZA | labor |
  And balances:
  | id   | balance |*
  | .ZZA |     100 |
  | .ZZB |    -100 |
  | .ZZC |       0 |
  