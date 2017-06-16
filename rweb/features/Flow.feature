Feature: Flow
AS a member
I WANT my account to draw automatically from another, when I overdraw
SO I can spend up to my total credit line.

Setup:
  Given members:
  | id   | fullName   | rebate | flags      |*
  | .ZZA | Abe One    |      5 | ok,confirmed,bona    |
  | .ZZB | Bea Two    |     10 | ok,confirmed,bona    |
  | .ZZC | Corner Pub |     10 | ok,confirmed,co,bona |
  And relations:
  | main | agent | permission | draw |*
  | .ZZC | .ZZA  | manage     |    1 |
  | .ZZC | .ZZB  | sell       |    0 |
  And balances:
  | id   | balance | rewards |*
  | .ZZA |       0 |      20 |
  | .ZZB |     100 |      20 |
  | .ZZC |     100 |      20 |

Scenario: A member draws
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |     30 | %FOR_GOODS | food    |
  Then transactions:
  | xid | type     | amount | rebate | bonus | from | to   | purpose      |*
  |   1 | transfer |     10 |      0 |     0 |  .ZZC | .ZZA | automatic transfer to NEWZZA,automatic transfer from NEWZZC |
  |   2 | transfer |     30 |   1.50 |  3.00 |  .ZZA | .ZZB | food         |
  
Scenario: A member draws again
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    130 | %FOR_GOODS | food    |
  Then transactions:
  | xid | type     | amount | rebate | bonus | from | to   | purpose      |*
  |   1 | transfer |    110 |      0 |     0 | .ZZC | .ZZA | automatic transfer to NEWZZA,automatic transfer from NEWZZC |
  |   2 | transfer |    130 |   6.50 | 13.00 | .ZZA | .ZZB | food         |

Scenario: A member overdraws with not enough to draw on
  When member ".ZZA" completes form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    200 | %FOR_GOODS | food    |
  Then we say "error": "short to|increase min" with subs:
  | short |*
  | $60   |
  
# add a scenario for drawing from two sources