Feature: Transact
AS a member
I WANT to transfer USD to or from another member (acting on their own behalf)
SO I can buy and sell stuff.

Setup:
  Given members:
  | id   | fullName   | country | email         | flags                |
  | .ZZA | Abe One    | US      | a@example.com | dft,ok,personal,bona |
  | .ZZB | Bea Two    | US      | b@example.com | dft,ok,personal,bona |
  | .ZZC | Corner Pub | France  | c@example.com | dft,ok,company,bona  |
  And relations:
  | id      | main | agent | permission |
  | NEW.ZZA | .ZZA | .ZZB  | buy        |
  | NEW.ZZB | .ZZB | .ZZA  | read       |
  | NEW.ZZC | .ZZC | .ZZB  | buy        |
  | NEW.ZZD | .ZZC | .ZZA  | sell       |
  And usd:
  | id   | usd  |
  | ctty | 1000 |
  | .ZZA |  100 |
  | .ZZB |  200 |
  | .ZZC |  300 |

Scenario: A member asks to charge another member
  When member ".ZZA" completes form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |
  Then we show "confirm charge" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  And balances:
  | id   | r | usd | rewards |
  | .ZZA | 0 | 100 |       0 |
  | .ZZB | 0 | 200 |       0 |
  | .ZZC | 0 | 300 |       0 |
  
Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report invoice" with subs:
  | did     | otherName | amount | tid |
  | charged | Bea Two   | $100   | 1   |
  And we notice "new invoice" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payerPurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And transactions:
  | xid   | created | type     | state   | amount | from | to   | purpose      | taking |
  | .AAAB | %today  | transfer | pending |    100 | .ZZB | .ZZA | labor        | 1      |
  | .AAAC | %today  | rebate   | pending |      5 | ctty | .ZZB | rebate on #1 | 0      |
  | .AAAD | %today  | bonus    | pending |     10 | ctty | .ZZA | bonus on #1  | 0      |
  And balances:
  | id      | r | usd | rewards |
  | .ZZA | 0 | 100 |       0 |
  | .ZZB | 0 | 200 |       0 |
  | .ZZC | 0 | 300 |       0 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |
  Then we show "confirm payment" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report transaction" with subs:
  | did    | otherName | amount | tid | rewardType | rewardAmount |
  | paid   | Bea Two   | $100   | 1   | rebate     | $0.05        |
  And we notice "new payment" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And transactions:
  | xid   | created | type     | state | amount | r    | from | to   | purpose      | taking |
  | .AAAB | %today  | transfer | done  |    100 | 0    | .ZZA | .ZZB | labor        | 0      |
  | .AAAC | %today  | rebate   | done  |   0.05 | 0.05 | ctty | .ZZA | rebate on #1 | 0      |
  And balances:
  | id   | r     | usd    | rewards |
  | ctty | -0.05 |      - |       - |
  | .ZZA |  0.05 |   0.00 |    0.05 |
  | .ZZB |     0 | 299.75 |       0 |
  | .ZZC |     0 | 300.00 |       0 |
