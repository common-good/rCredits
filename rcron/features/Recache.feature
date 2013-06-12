Feature: Recache
AS a member
I WANT my cached rCredits balances to reflect reality
SO I don't lose money or get confused.
# Note that this uses the REAL Dwolla account

Setup:
  Given members:
  | id   | fullName   | dwolla          | email         | flags                |
  | .ZZA | Abe One    | %DW_TESTER_ACCT | a@example.com | dft,ok,personal,bona |
  | .ZZB | Bea Two    | %DW_TEST_ACCT   | b@example.com | dft,ok,personal      |
  | .ZZC | Corner Pub |                 | c@example.com | dft,ok,company       |
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
  | id     | r  | usd     | rewards | minimum | floor |
  | .ZZA   |  0 | AMT1+99 |       0 |       0 |     2 |
  | .ZZB   | 20 |       0 |       0 |     -10 |   -50 |
  When cron runs "recache"
  Then we tell staff "cache mismatch" with subs:
  | id   | key     | is      | shouldBe |
  | .ZZA | r       |       0 |       10 |
# (too tricky for now)  | .ZZA | usd     | AMT1+99 |     AMT1 |
  | .ZZA | rewards |       0 |       10 |
  | .ZZA | minimum |       0 |        2 |
  | .ZZB | r       |      20 |        0 |
  | .ZZB | minimum |     -10 |        0 |
  And balances:
  | id     | r  | usd  | rewards | minimum |
  | .ZZA   | 10 | AMT1 |      10 |       2 |
  | .ZZB   |  0 |    0 |       0 |       0 |

Scenario: Balances get a tiny bit out of whack
  Given balances:
  | id     | r       |
  | .ZZA   | 10.0001 |
  When cron runs "recache"
  Then we do not tell staff "cache mismatch"