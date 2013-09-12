Feature: Incomplete USD Txs
AS a member
I WANT my transactions to go through regardless of whether Dwolla is up
SO I can depend on having access to the money I have in the rCredits system

Setup:
  Given members:
  | id   | fullName   | dw | country | email | flags              |
  | .ZZA | Abe One    |  1 | US      | a@    | dft,ok,person,bona |
  | .ZZB | Bea Two    |  0 | US      | b@    | dft,ok,person,bona |
  And balances:
  | id   | dw/usd |
  | .ZZA |      5 |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |
  | 1   | %today-7m | signup |      9 | ctty | .ZZA | signup  |
  | 2   | %today-7m | signup |      5 | ctty | .ZZB | signup  |
  And transactions: 
  | xid | created   | type     | amount | r | from | to   | purpose | taking | usdXid | goods |
  | 3   | %today-6m | transfer |      5 | 3 | .ZZA | .ZZB | loan    | 0      |     -1 |     0 |
  Then incomplete transaction count 1
  And balances:
  | id   | usd | dwolla |
  | .ZZA |   3 |      5 |

Scenario: a Dwolla outage leaves a transaction incomplete
  Given cron runs "recache"
  # reset cache
  Then balances:
  | id   | usd | dwolla |
  | .ZZA |   5 |      5 |
  When cron runs "usdTxsHere"
  Then incomplete transaction count 0
  And usd transfers:
  | payer | payee | amount |
  | .ZZA  |  .ZZB |      2 |
  Then balances:
  | id   | usd | dwolla |
  | .ZZA |   5 |      3 |
