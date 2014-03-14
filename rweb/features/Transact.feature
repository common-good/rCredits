Feature: Transact
AS a member
I WANT to transfer rCredits to or from another member (acting on their own behalf)
SO I can buy and sell stuff.
 We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc.
 And foreigner (member on a different server) to member, etc.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | postalAddr | email | flags      |
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK | a@    | ok,bona    |
  | .ZZB | Bea Two    | 2 B St. | Btown | Utah   | 02000      | US      | 2 B, B, UT | b@    | ok,bona    |
  | .ZZC | Corner Pub | 3 C St. | Ctown | Cher   |            | France  | 3 C, C, FR | c@    | ok,co,bona |
  And relations:
  | id      | main    | agent   | permission |
  | NEW.ZZA | NEW.ZZA | NEW.ZZB | buy        |
  | NEW.ZZB | NEW.ZZB | NEW.ZZA | read       |
  | NEW.ZZC | NEW.ZZC | NEW.ZZB | buy        |
  | NEW.ZZD | NEW.ZZC | NEW.ZZA | sell       |
  And transactions: 
  | xid      | created   | type       | amount | from      | to      | purpose | taking |
  | NEW.AAAB | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZA | signup  | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZB | signup  | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP |    250 | community | NEW.ZZC | signup  | 0      |
  Then balances:
  | id        | balance |
  | community | -750 |
  | NEW.ZZA   |  250 |
  | NEW.ZZB   |  250 |
  | NEW.ZZC   |  250 |

# (rightly fails, so do this in a separate feature) Variants: with/without an agent
#  | "NEW.ZZA" | # member to member (pro se) |
#  | "NEW.ZZA" | # agent to member           |
Skip
Scenario: A member asks to charge another member
  When member "NEW.ZZA" completes form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |
  Then we show "confirm charge" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
Resume
Scenario: A member confirms request to charge another member
  When member "NEW.ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report invoice" with subs:
  | did     | otherName | amount |
  | charged | Bea Two   | $100   |
  And we notice "new invoice" to member "NEW.ZZB" with subs:
  | created | fullName | otherName | amount | payerPurpose |
  | %today  | Bea Two  | Abe One   | $100   | labor        |
  And invoices:
  | id | created | state   | amount | from      | to      | for   |
  | 1  | %today  | pending |    100 | NEW.ZZB   | NEW.ZZA | labor |
  And balances:
  | id        | balance |
  | community |    -750 |
  | NEW.ZZA   |     250 |
  | NEW.ZZB   |     250 |
  | NEW.ZZC   |     250 |
Skip
Scenario: A member asks to pay another member
  When member "NEW.ZZA" completes form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |
  Then we show "confirm payment" with subs:
  | amount | otherName |
  | $100   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report transaction" with subs:
  | did    | otherName | amount | rewardType | rewardAmount |
  | paid   | Bea Two   | $100   | reward     | $5           |
  And we notice "new payment|reward other" to member "NEW.ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |
  | %today  | Bea Two  | Abe One   | $100   | labor        | reward          |               $10 |
  And transactions:
  | xid      | created   | type      | state    | amount | from      | to      | purpose      | taking |
  | NEW.AAAE | %today | %TX_TRANSFER | %TX_DONE |    100 | NEW.ZZA   | NEW.ZZB | labor        | 0      |
  | NEW.AAAF | %today | %TX_REBATE   | %TX_DONE |      5 | community | NEW.ZZA | rebate on #2 | 0      |
  | NEW.AAAG | %today | %TX_BONUS    | %TX_DONE |     10 | community | NEW.ZZB | bonus on #2  | 0      |
  And balances:
  | id        | balance |
  | community |    -765 |
  | NEW.ZZA   |     155 |
  | NEW.ZZB   |     360 |
  | NEW.ZZC   |     250 |

Scenario: A member confirms request to pay a member company
  Given next DO code is "whatever"
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who        | amount | goods | purpose |
  | pay | Corner Pub | 100    | 1     | stuff   |
  Then we say "status": "report transaction" with subs:
  | did    | otherName  | amount | rewardType | rewardAmount |
  | paid   | Corner Pub | $100   | reward     | $5           |
  And we notice "new payment|reward other" to member "NEW.ZZC" with subs:
  | created | fullName   | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |
  | %today  | Corner Pub | Abe One   | $100 | stuff | reward | $10 |
  And that "notice" has link results:
  | _name | Abe One |
  | _postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
  And transactions:
  | xid      | created   | type      | state    | amount | from      | to      | purpose      | taking |
  | NEW.AAAE | %today | %TX_TRANSFER | %TX_DONE |    100 | NEW.ZZA   | NEW.ZZC | stuff        | 0      |
  | NEW.AAAF | %today | %TX_REBATE   | %TX_DONE |      5 | community | NEW.ZZA | rebate on #2 | 0      |
  | NEW.AAAG | %today | %TX_BONUS    | %TX_DONE |     10 | community | NEW.ZZC | bonus on #2  | 0      |
  And balances:
  | id        | balance |
  | community |    -765 |
  | NEW.ZZA   |     155 |
  | NEW.ZZB   |     250 |
  | NEW.ZZC   |     360 |

Scenario: A member confirms request to pay the same member the same amount
  Given member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |  
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 100    | 1     | labor   |
  Then we say "error": "duplicate transaction" with subs:
  | op   |
  | paid |
  
Scenario: A member confirms request to charge the same member the same amount
  Given member "NEW.ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |  
  When member "NEW.ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |
  | charge | Bea Two | 100    | 1     | labor   |
  Then we say "error": "duplicate transaction" with subs:
  | op      |
  | charged |
  