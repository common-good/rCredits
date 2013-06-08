Feature: Incomplete USD Txs
AS a member
I WANT my transactions to go through regardless of whether Dwolla is up
SO I can depend on having access to the money I have in the rCredits system

Setup:
  Given members:
  | id   | fullName   | dwolla          | country | email         | flags                |
  | .ZZA | Abe One    | %DW_TESTER_ACCT | US      | a@example.com | dft,ok,personal,bona |
  | .ZZB | Bea Two    | %DW_TEST_ACCT   | US      | b@example.com | dft,ok,personal,bona |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose |
  | 1   | %today-7m | signup |      9 | ctty | .ZZA | signup  |
  | 2   | %today-7m | signup |      5 | ctty | .ZZB | signup  |
  And balances:
  | id   | r | usd   |
  | .ZZA | 9 | +AMT1 |
  | .ZZB | 5 |    10 |
  And transactions: 
  | xid | created   | type     | amount | r | from | to   | purpose | taking | usdXid | goods |
  | 3   | %today-6m | transfer |      5 | 3 | .ZZA | .ZZB | loan    | 0      |     -1 |     0 |
  Then incomplete transaction count 1
  And balances:
  | id   | r | usd    |
  | .ZZB | 8 |     12 |

Scenario: a Dwolla outage leaves a transaction incomplete
  Given usd:
  | id   | usd   |
  | .ZZA | +AMT2 |
  # reset cache
  When cron runs "usdTxsHere"
  Then incomplete transaction count 0
  And usd transfers:
  | payer | payee | amount |
  | .ZZA  |  .ZZB |      2 |
  And balances:
  | id   | r | usd    |
  | .ZZA | 6 | AMT1-2 |
  | .ZZB | 8 |     12 |
