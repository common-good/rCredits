Feature: Transact
AS a member
I WANT to transfer rCredits to or from another member (acting on their own behalf)
SO I can buy and sell stuff.
# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc.
# And foreigner (member on a different server) to member, etc.

Setup:
  Given members:
  | id      | fullName  | address | city  | state  | postalCode | country | email         | accountType  |
  | NEW.ZZA | Abe One    | POB 1   | Atown | Alaska | 01000       | US      | a@example.com | %R_PERSONAL   |
  | NEW.ZZB | Bea Two    | POB 2   | Btown | Utah   | 02000       | US      | b@example.com | %R_PERSONAL   |
  | NEW.ZZC | Corner Pub | POB 3   | Ctown | Cher   |             | France  | c@example.com | %R_COMMERCIAL |
  And relations:
  | id      | main    | agent   | permission        |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell      |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA | read transactions |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy and sell      |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell              |
  And transactions: 
  | tx_id    | created   | type       | amount | from      | to      | purpose | taking |
  | NEW.AAAB | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZA | signup  | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZB | signup  | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZC | signup  | 0      |
  Then balances:
  | id        | balance |
  | community |    -750 |
  | NEW.ZZA   |     250 |
  | NEW.ZZB   |     250 |
  | NEW.ZZC   |     250 |

# (rightly fails, so do this in a separate feature) Variants: with/without an agent
#  | "NEW.ZZA" | # member to member (pro se) |
#  | "NEW:ZZA" | # agent to member           |

Scenario: A member asks to charge another member
  When member "NEW.ZZA" completes form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | Charge | Bea Two | 100    | 1     | labor   |
  Then we show "confirm charge" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  
Scenario: A member confirms request to charge another member
  When member "NEW.ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | Charge | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report invoice" with subs:
  | action  | otherName | amount | tid |
  | charged | Bea Two   | $100   | 2   |
  And we email "new-invoice" to member "b@example.com" with subs:
  | created | fullName | otherName | amount | payerPurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And we show "Tx" with subs:
  | arg1   |
  | charge |
  And transactions:
  | tx_id    | created   | type      | state       | amount | from      | to      | purpose      | taking |
  | NEW.AAAE | %today | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZB   | NEW.ZZA | labor        | 1      |
  | NEW.AAAF | %today | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZB | rebate on #2 | 0      |
  | NEW.AAAG | %today | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus on #2  | 0      |
  And balances:
  | id        | balance |
  | community |    -750 |
  | NEW.ZZA   |     250 |
  | NEW.ZZB   |     250 |
  | NEW.ZZC   |     250 |

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
  | action | otherName | amount | tid | rewardType | rewardAmount | balance |
  | paid   | Bea Two   | $100   | 2   | rebate     | $5           | $155    |
  And we email "new-payment" to member "b@example.com" with subs:
  | created | fullName | otherName | amount | payeePurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And we show "Tx" with subs:
  | arg1 |
  | pay  |
  And transactions:
  | tx_id    | created   | type      | state    | amount | from      | to      | purpose      | taking |
  | NEW.AAAE | %today | %TX_TRANSFER | %TX_DONE |    100 | NEW.ZZA   | NEW.ZZB | labor        | 0      |
  | NEW.AAAF | %today | %TX_REBATE   | %TX_DONE |      5 | community | NEW.ZZA | rebate on #2 | 0      |
  | NEW.AAAG | %today | %TX_BONUS    | %TX_DONE |     10 | community | NEW.ZZB | bonus on #2  | 0      |
  And balances:
  | id        | balance |
  | community |    -765 |
  | NEW.ZZA   |     155 |
  | NEW.ZZB   |     360 |
  | NEW.ZZC   |     250 |
