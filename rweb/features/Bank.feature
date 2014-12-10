Feature: Bank
AS a member
I WANT to transfer funds from my bank account to my rCredits account or vice versa
SO I can get money in and out of my rCredits account.

Setup:
  Given members:
  | id   | fullName | flags   | risks   |*
  | .ZZA | A Mem    | ok,bona | hasBank |
  | .ZZB | B Mem    | ok,bona |         | 
  | .ZZC | C Mem    | ok,bona |         |
  | .ZZD | D Mem    | ok,bona |         |

  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  | 1   | %today-7m | signup   |    100 | ctty | .ZZA | signup  |
  | 2   | %today-7m | signup   |    100 | ctty | .ZZB | signup  |
  | 3   | %today-7m | signup   |    100 | ctty | .ZZC | signup  |
  | 4   | %today-7m | signup   |    100 | ctty | .ZZD | signup  |
  | 8   | %today    | transfer |     80 | .ZZD | .ZZA | gift    |

Scenario: A member transfers funds from the bank
  When member ".ZZA" completes form "get" with values:
  | amount | op  |*
  |     40 | get |
  Then we say "status": "banked|bank tx number" with subs:
  | action     | amount | checkNum |*
  | draw from  | $40    |        1 |
  And usd transfers:
  | txid | payer | amount |*
  |    1 | .ZZA  |    -40 |
  
Scenario: A member transfers funds to the bank
  When member ".ZZA" completes form "get" with values:
  | amount | op  |*
  |     40 | put |
  Then we say "status": "banked" with subs:
  | action     | amount |*
  | deposit to | $40    |
  And usd transfers:
  | payer | amount |*
  | .ZZA  |     40 |
