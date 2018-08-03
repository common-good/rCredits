Feature: Recache
AS a member
I WANT my cached rCredits balances to reflect reality
SO I don't lose money or get confused.
# Note that this uses the REAL Dwolla account

Setup:
  Given members:
  | id   | fullName   | dwolla          | email         | flags                | minimum | maximum |
  | .ZZA | Abe One    | %DW_TESTER_ACCT | a@example.com | dft,ok,personal,bona |       5 |       1 |
  | .ZZB | Bea Two    | %DW_TEST_ACCT   | b@example.com | dft,ok,personal      |     100 |      50 |
  | .ZZC | Corner Pub |                 | c@example.com | dft,ok,company       |     100 |      -1 |
  And transactions: 
  | xid   | created   | type       | amount | from      | to   | purpose | taking |
  | .AAAB | %today-6m | %TX_SIGNUP |     10 | community | .ZZA | signup  | 0      |
  And usd:
  | id   | usd   |
  | .ZZA | +AMT1 |
  Then balances:
  | id   | r  | usd  | rewards |
  | .ZZA | 10 | AMT1 |      10 |
  | .ZZB |  0 |    0 |       0 |

Scenario: Balances get out of whack
  Given balances:
  | id     | r | usd     | rewards |
  | .ZZA   | 0 | AMT1+99 |       0 |
  When cron runs "recache"
  Then balances:
  | id     | r  | usd  | rewards | minimum | maximum |
  | .ZZA   | 10 | AMT1 |      10 |       5 |      10 |
  | .ZZB   |  0 |    0 |       0 |     100 |     100 |
