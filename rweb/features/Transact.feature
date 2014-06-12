Feature: Transact
AS a member
I WANT to transfer rCredits to or from another member (acting on their own behalf)
SO I can buy and sell stuff.
 We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc.
 And foreigner (member on a different server) to member, etc.

Setup:
  Given members:
  | id   | fullName   | address | city  | state  | postalCode | country | postalAddr | email | flags      |*
  | .ZZA | Abe One    | 1 A St. | Atown | Alaska | 01000      | US      | 1 A, A, AK | a@    | ok,bona    |
  | .ZZB | Bea Two    | 2 B St. | Btown | Utah   | 02000      | US      | 2 B, B, UT | b@    | ok,bona    |
  | .ZZC | Corner Pub | 3 C St. | Ctown | Cher   |            | France  | 3 C, C, FR | c@    | ok,co,bona |
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
  Then balances:
  | id   | balance |*
  | ctty | -750 |
  | .ZZA |  250 |
  | .ZZB |  250 |
  | .ZZC |  250 |

# (rightly fails, so do this in a separate feature) Variants: with/without an agent
#  | ".ZZA" | # member to member (pro se) |
#  | ".ZZA" | # agent to member           |

Scenario: A member asks to charge another member
  When member ".ZZA" completes form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | 1     | labor   |
  Then we show "confirm charge" with subs:
  | amount | otherName |*
  | $100   | Bea Two   |

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report invoice" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And we notice "new invoice" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | purpose |*
  | %today  | Bea Two  | Abe One   | $100   | labor   |
  And invoices:
  | nvid | created | status      | amount | from | to  | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB  | .ZZA | labor |
  And balances:
  | id   | balance |*
  | ctty |    -750 |
  | .ZZA |     250 |
  | .ZZB |     250 |
  | .ZZC |     250 |

Scenario: A member asks to pay another member
  When member ".ZZA" completes form "pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | 1     | labor   |
  Then we show "confirm payment" with subs:
  | amount | otherName |*
  | $100   | Bea Two   |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | 1     | labor   |
  Then we say "status": "report transaction" with subs:
  | did    | otherName | amount | rewardType | rewardAmount |*
  | paid   | Bea Two   | $100   | reward     | $5           |
  And we notice "new payment|reward other" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |*
  | %today  | Bea Two  | Abe One   | $100   | labor        | reward          |               $10 |
  And transactions:
  | xid | created | type     | amount | from  | to   | purpose      | taking |*
  |   4 | %today  | transfer |    100 | .ZZA  | .ZZB | labor        | 0      |
  |   5 | %today  | rebate   |      5 | ctty  | .ZZA | rebate on #2 | 0      |
  |   6 | %today  | bonus    |     10 | ctty  | .ZZB | bonus on #2  | 0      |
  And balances:
  | id   | balance |*
  | ctty |    -765 |
  | .ZZA |     155 |
  | .ZZB |     360 |
  | .ZZC |     250 |

Scenario: A member confirms request to pay a member company
  Given next DO code is "whatever"
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | goods | purpose |*
  | pay | Corner Pub | 100    | 1     | stuff   |
  Then we say "status": "report transaction" with subs:
  | did    | otherName  | amount | rewardType | rewardAmount |*
  | paid   | Corner Pub | $100   | reward     | $5           |
  And we notice "new payment|reward other" to member ".ZZC" with subs:
  | created | fullName   | otherName | amount | payeePurpose | otherRewardType | otherRewardAmount |*
  | %today  | Corner Pub | Abe One   | $100 | stuff | reward | $10 |
Skip messages don't show up in get_file_contents simulation and I haven't figured out how to test this yet
  And that "notice" has link results:
  | _name | Abe One |
  | _postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
Resume
  And transactions:
  | xid | created | type     | amount | from  | to   | purpose      | taking |*
  |   4 | %today  | transfer |    100 | .ZZA  | .ZZC | stuff        | 0      |
  |   5 | %today  | rebate   |      5 | ctty  | .ZZA | rebate on #2 | 0      |
  |   6 | %today  | bonus    |     10 | ctty  | .ZZC | bonus on #2  | 0      |
  And balances:
  | id   | balance |*
  | ctty |    -765 |
  | .ZZA |     155 |
  | .ZZB |     250 |
  | .ZZC |     360 |

Scenario: A member confirms request to pay the same member the same amount
  Given member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | 1     | labor   |  
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | 1     | labor   |
  Then we say "error": "duplicate transaction" with subs:
  | op   |*
  | paid |
  
Scenario: A member confirms request to charge the same member the same amount
  Given member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | 1     | labor   |  
  When member ".ZZA" confirms form "charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | 1     | labor   |
  Then we say "error": "duplicate transaction" with subs:
  | op      |*
  | charged |
  