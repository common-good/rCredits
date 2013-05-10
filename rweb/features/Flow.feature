Feature: Flow
AS a member
I WANT my account to draw automatically from another, when I overdraw
SO I can spend up to my total credit line.

Setup:
  Given members:
  | id   | fullName   | email         | flags           |
  | .ZZA | Abe One    | a@example.com | dft,ok,personal |
  | .ZZB | Bea Two    | b@example.com | dft,ok,personal |
  | .ZZC | Corner Pub | c@example.com | dft,ok,company  |
  And relations:
  | id      | main | agent | permission | draw |
  | NEW:ZZA | .ZZC | .ZZA  | manage     |    1 |
  | NEW:ZZB | .ZZC | .ZZB  | sell       |    0 |
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
  | xid   | type     | amount | from      | to   | purpose      | r    |
  | .AAAB | transfer |     10 | .ZZC      | .ZZA | automatic transfer to NEW.ZZA,automatic transfer from NEW.ZZC | 10 |
  | .AAAC | transfer |     30 | .ZZA      | .ZZB | food         |   30 |
  | .AAAD | rebate   |   1.50 | community | .ZZA | rebate on #2 | 1.50 |
  | .AAAE | bonus    |   3.00 | community | .ZZB | bonus on #1  | 3.00 |
  
Scenario: A member overdraws and draws from both r and USD
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods | purpose |
  | pay | .ZZB |    130 |     1 | food    |
  Then transactions:
  | xid   | type     | amount | from      | to   | purpose      | r    |
  | .AAAB | transfer |     20 | .ZZC      | .ZZA | automatic transfer to NEW.ZZA,automatic transfer from NEW.ZZC | 20 |
  | .AAAC | transfer |     90 | .ZZC      | .ZZA | automatic transfer to NEW.ZZA,automatic transfer from NEW.ZZC |  0 |
  | .AAAD | transfer |    130 | .ZZA      | .ZZB | food         |   40 |
  | .AAAE | rebate   |   2.00 | community | .ZZA | rebate on #3 | 2.00 |
  | .AAAF | bonus    |   4.00 | community | .ZZB | bonus on #1  | 4.00 |

Scenario: A member overdraws with not enough to draw on
  When member ".ZZA" completes form "pay" with values:
  | op  | who  | amount | goods | purpose |
  | pay | .ZZB |    200 |     1 | food    |
  Then we say "error": "short to" with subs:
  | short  |
  | $60.25 |
  
# add a scenario for drawing from two sources