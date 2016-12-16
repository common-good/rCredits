Feature: Recache
AS a member
I WANT my cached rCredits balances to reflect reality
SO I don't lose money or get confused.
# Note that this uses the REAL Dwolla account

Setup:
  Given members:
  | id   | fullName   |email | flags    |*
  | .ZZA | Abe One    | a@    | ok,bona |
  | .ZZB | Bea Two    | b@    | ok,bona |
  | .ZZC | Corner Pub | c@    | ok,co   |
  And transactions: 
  | xid   | created   | type       | amount | from | to   | purpose | taking |*
  | .AAAB | %today-6m | %TX_SIGNUP |     10 | ctty | .ZZA | signup  | 0      |
  Then balances:
  | id   | r  | rewards |*
  | .ZZA | 10 |      10 |
  | .ZZB |  0 |       0 |

Scenario: Balances get out of whack
  Given balances:
  | id     | r  |rewards |*
  | .ZZA   |  0 |      0 |
  | .ZZB   | 20 |      0 |
  When cron runs "recache"
  Then we tell admin "cache mismatch" with subs:
  | id   | key     | is   | shouldBe |*
  | .ZZA | r       |    0 |       10 |
  | .ZZA | rewards |    0 |       10 |
  | .ZZB | r       |   20 |        0 |
  And balances:
  | id     | r  | rewards |*
  | .ZZA   | 10 |      10 |
  | .ZZB   |  0 |       0 |
# (we might never want this feature)
#  And we message member ".ZZA" with topic "account suspended" and subs:
#  | why                        |*
#  | to protect data integrity. |
#  And we message member ".ZZB" with topic "account suspended" and subs:
#  | why                        |*
#  | to protect data integrity. |
  
Scenario: Balances get a tiny bit out of whack
  Given balances:
  | id     | r       |*
  | .ZZA   | 10.0001 |
  | .ZZB   |       0 |
  When cron runs "recache"
  Then we tell admin "cache mismatch" with subs:
  | id   | key     | is      | shouldBe |*
  | .ZZA | r       | 10.0001 |       10 |
