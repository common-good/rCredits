Feature: Bank
AS a member
I WANT to transfer funds from my bank account to my rCredits account or vice versa
SO I can get money in and out of my rCredits account.

Setup:
  Given members:
  | id   | fullName | flags           |*
  | .ZZA | A Mem    | ok,dw,bona,bank |
  | .ZZB | B Mem    | ok,dw,bona      |
  | .ZZC | C Mem    | ok,dw,bona      |
  | .ZZD | D Mem    | ok,dw,bona      |

  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  | 1   | %today-7m | signup   |    100 | ctty | .ZZA | signup  |
  | 2   | %today-7m | signup   |    100 | ctty | .ZZB | signup  |
  | 3   | %today-7m | signup   |    100 | ctty | .ZZC | signup  |
  | 4   | %today-7m | signup   |    100 | ctty | .ZZD | signup  |
  | 8   | %today    | transfer |     80 | .ZZD | .ZZA | gift    |
  And balances:
  | id   | usd |*
  | .ZZA |  10 |
  | .ZZB |  10 |
  | .ZZC |  10 |
  | .ZZD |  10 |

Scenario: A member transfers funds to the bank
  When member ".ZZA" completes form "get" with values:
  | amount | op  |*
  |     40 | put |
  Then we say "status": "banked" with subs:
  | action     | amount |*
  | deposit to | $40    |
  And usd transfers:
  | payer | payee | amount |*
  | .ZZB  | .ZZA  |     10 |
  | .ZZC  | .ZZA  |     10 |
  | .ZZD  | .ZZA  |     10 |
  | .ZZA  |     0 |     40 |
