Feature: Flow
AS a member
I WANT my account to draw automatically from another, when I overdraw
SO I can spend up to my total credit line.

Setup:
  Given members:
  | id   | fullName   | email | flags      |
  | .ZZA | Abe One    | a@    | ok,bona    |
  | .ZZB | Bea Two    | b@    | ok,bona    |
  | .ZZC | Corner Pub | c@    | ok,co,bona |
  And relations:
  | id      | main | agent | permission | draw |
  | NEW.ZZA | .ZZC | .ZZA  | manage     |    1 |
  | NEW.ZZB | .ZZC | .ZZB  | sell       |    0 |
  And balances:
  | id   | usd  | r   | rewards |
  | .ZZA |    0 |  20 |      20 |
  | .ZZB |  100 |  20 |      20 |
  | .ZZC |  100 |  20 |      20 |

Scenario: A member overdraws
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods | purpose |
  | pay | .ZZB |     30 |     1 | food    |
  Then transactions:
  | xid | type     | amount | from | to   | purpose      |
  |   1 | transfer |     10 | .ZZC | .ZZA | automatic transfer to NEW.ZZA,automatic transfer from NEW.ZZC |
  |   2 | transfer |     30 | .ZZA | .ZZB | food         |
  |   3 | rebate   |   1.50 | ctty | .ZZA | rebate on #2 |
  |   4 | bonus    |   3.00 | ctty | .ZZB | bonus on #1  |
  
Scenario: A member overdraws and draws from both r and USD
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods | purpose |
  | pay | .ZZB |    130 |     1 | food    |
  Then transactions:
  | xid | type     | amount | from | to   | purpose      |
  |   1 | transfer |    110 | .ZZC | .ZZA | automatic transfer to NEW.ZZA,automatic transfer from NEW.ZZC |
  |   2 | transfer |    130 | .ZZA | .ZZB | food         |
  |   3 | rebate   |   6.50 | ctty | .ZZA | rebate on #2 |
  |   4 | bonus    |  13.00 | ctty | .ZZB | bonus on #1  |

Scenario: A member overdraws with not enough to draw on
  When member ".ZZA" completes form "pay" with values:
  | op  | who  | amount | goods | purpose |
  | pay | .ZZB |    200 |     1 | food    |
  Then we say "error": "short to" with subs:
  | short |
  | $60   |
  
# add a scenario for drawing from two sources