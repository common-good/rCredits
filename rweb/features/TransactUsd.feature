Feature: Transact
AS a member
I WANT to transfer USD to or from another member (acting on their own behalf)
SO I can buy and sell stuff.

Setup:
  Given members:
  | id      | fullName   | country | email         | flags           |
  | NEW.ZZA | Abe One    | US      | a@example.com | dft,ok,personal |
  | NEW.ZZB | Bea Two    | US      | b@example.com | dft,ok,personal |
  | NEW.ZZC | Corner Pub | France  | c@example.com | dft,ok,company  |
  And relations:
  | id      | main    | agent   | permission |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy        |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA | read       |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy        |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell       |
  And usd:
  | id        | usd  |
  | community | 1000 |
  | NEW.ZZA   |  100 |
  | NEW.ZZB   |  200 |
  | NEW.ZZC   |  300 |

Scenario: A member asks to charge another member
  When member "NEW.ZZA" completes form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | Charge | Bea Two | 100    | 1     | labor   |
  Then we show "confirm charge" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  And balances:
  | id      | r | usd | rewards |
  | NEW.ZZA | 0 | 100 |       0 |
  | NEW.ZZB | 0 | 200 |       0 |
  | NEW.ZZC | 0 | 300 |       0 |
  
Scenario: A member confirms request to charge another member
  When member "NEW.ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | Charge | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report invoice" with subs:
  | action  | otherName | amount | tid |
  | charged | Bea Two   | $100   | 1   |
  And we email "new-invoice" to member "b@example.com" with subs:
  | created | fullName | otherName | amount | payerPurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And transactions:
  | tx_id    | created   | type      | state       | amount | from      | to      | purpose      | taking |
  | NEW.AAAB | %today | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZB   | NEW.ZZA | labor        | 1      |
  | NEW.AAAC | %today | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZB | rebate on #1 | 0      |
  | NEW.AAAD | %today | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus on #1  | 0      |
  And balances:
  | id      | r | usd | rewards |
  | NEW.ZZA | 0 | 100 |       0 |
  | NEW.ZZB | 0 | 200 |       0 |
  | NEW.ZZC | 0 | 300 |       0 |

Scenario: A member asks to pay another member
  When member "NEW.ZZA" completes form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | Pay | Bea Two | 100    | 1     | labor   |
  Then we show "confirm payment" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | Pay | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report transaction" with subs:
  | action | otherName | amount | tid | rewardType | rewardAmount |
  | paid   | Bea Two   | $100   | 1   | rebate     | $0.05        |
  And we email "new-payment" to member "b@example.com" with subs:
  | created | fullName | otherName | amount | payeePurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And transactions:
  | tx_id    | created   | type      | state    | amount | r    | from      | to      | purpose      | taking |
  | NEW.AAAB | %today | %TX_TRANSFER | %TX_DONE |    100 | 0    | NEW.ZZA   | NEW.ZZB | labor        | 0      |
  | NEW.AAAC | %today | %TX_REBATE   | %TX_DONE |   0.05 | 0.05 | community | NEW.ZZA | rebate on #1 | 0      |
  And balances:
  | id        | r     | usd    | rewards |
  | community | -0.05 |      - |       - |
  | NEW.ZZA   |  0.05 |   0.00 |    0.05 |
  | NEW.ZZB   |     0 | 299.75 |       0 |
  | NEW.ZZC   |     0 | 300.00 |       0 |
