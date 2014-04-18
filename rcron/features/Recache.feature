Feature: Recache
AS a member
I WANT my cached rCredits balances to reflect reality
SO I don't lose money or get confused.
# Note that this uses the REAL Dwolla account

Setup:
  Given members:
  | id   | fullName   | dw | email | flags      |
  | .ZZA | Abe One    |  1 | a@    | ok,dw,bona |
  | .ZZB | Bea Two    |  0 | b@    | ok,dw,bona |
  | .ZZC | Corner Pub |    | c@    | ok,dw,co   |
  And balances:
  | id   | dw/usd |
  | .ZZA |      5 |
  And transactions: 
  | xid   | created   | type       | amount | from | to   | purpose | taking |
  | .AAAB | %today-6m | %TX_SIGNUP |     10 | ctty | .ZZA | signup  | 0      |
  Then balances:
  | id   | r  | usd | rewards |
  | .ZZA | 10 |   5 |      10 |
  | .ZZB |  0 |   0 |       0 |

Scenario: Balances get out of whack
  Given balances:
  | id     | r  | usd  | rewards | minimum | floor |
  | .ZZA   |  0 | 8.52 |       0 |       0 |     2 |
  | .ZZB   | 20 |    0 |       0 |     -10 |   -50 |
  When cron runs "recache"
  Then we tell staff "cache mismatch" with subs:
  | id   | key     | is   | shouldBe |
  | .ZZA | r       |    0 |       10 |
  | .ZZA | usd     | 8.52 |        5 |
  | .ZZA | rewards |    0 |       10 |
  | .ZZA | minimum |    0 |        2 |
  | .ZZB | r       |   20 |        0 |
  | .ZZB | minimum |  -10 |        0 |
  And balances:
  | id     | r  | usd | rewards | minimum |
  | .ZZA   | 10 |   5 |      10 |       2 |
  | .ZZB   |  0 |   ? |       0 |       0 |
  Skip (we might never want this feature)
  And we message member ".ZZA" with topic "account suspended" and subs:
  | why                        |
  | to protect data integrity. |
  And we message member ".ZZB" with topic "account suspended" and subs:
  | why                        |
  | to protect data integrity. |
Resume
  
Scenario: Balances get a tiny bit out of whack
  Given balances:
  | id     | r       | usd |
  | .ZZA   | 10.0001 |   5 |
  | .ZZB   |       0 |   ? |
  When cron runs "recache"
  Then we tell staff "cache mismatch" with subs:
  | id   | key     | is      | shouldBe |
  | .ZZA | r       | 10.0001 |       10 |
