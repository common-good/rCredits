Feature: Transact
AS a member
I WANT to transfer USD to or from another member (acting on their own behalf)
SO I can buy and sell stuff.

Setup:
  Given members:
  | id   | fullName   | country | email | flags                |
  | .ZZA | Abe One    | US      | a@    | dft,ok,person,bona |
  | .ZZB | Bea Two    | US      | b@    | dft,ok,person,bona |
  | .ZZC | Corner Pub | France  | c@    | dft,ok,company,bona  |
  And relations:
  | id      | main | agent | permission |
  | NEW.ZZA | .ZZA | .ZZB  | buy        |
  | NEW.ZZB | .ZZB | .ZZA  | read       |
  | NEW.ZZC | .ZZC | .ZZB  | buy        |
  | NEW.ZZD | .ZZC | .ZZA  | sell       |
  And balances:
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
  | did     | otherName | amount |
  | charged | Bea Two   | $100   |
  And we notice "new invoice" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payerPurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And transactions:
  | xid   | created | type     | state   | amount | from | to   | purpose      | taking |
  | .AAAB | %today  | transfer | pending |    100 | .ZZB | .ZZA | labor        | 1      |
  | .AAAC | %today  | rebate   | pending |      5 | ctty | .ZZB | rebate on #1 | 0      |
  | .AAAD | %today  | bonus    | pending |     10 | ctty | .ZZA | bonus on #1  | 0      |
  And balances:
  | id   | r | usd | rewards |
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
  | did    | otherName | amount | rewardType | rewardAmount |
  | paid   | Bea Two   | $100   | reward     | $5           |
  And we notice "new payment|reward" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose | rewardType |rewardAmount |
  | %today  | Bea Two  | Abe One   | $100   | labor        | reward     | $10         |
  And transactions:
  | xid   | created | type     | state | amount | from | to   | purpose      | taking |
  | .AAAB | %today  | transfer | done  |    100 | .ZZA | .ZZB | labor        | 0      |
  | .AAAC | %today  | rebate   | done  |      5 | ctty | .ZZA | rebate on #1 | 0      |
  | .AAAD | %today  | bonus    | done  |     10 | ctty | .ZZB | bonus on #1  | 0      |
  And balances:
  | id   | r      | usd     | rewards |
  | ctty | -15.00 | 1000.00 |       - |
  | .ZZA | -95.00 |  100.00 |    5.00 |
  | .ZZB | 110.00 |  200.00 |   10.00 |
  | .ZZC |      0 |  300.00 |       0 |
#  When cron runs ""
#  Then balances:
#  | id   | r      | usd    | rewards |
#  | ctty | -15.25 |      - |       - |
#  | .ZZB |  10.25 | 299.75 |   10.25 |
  