Feature: Lost USD Txs
AS a member
I WANT my inconsistent USD transaction data to be made consistent ASAP
SO I can depend on the accuracy of rCredits system records
# In particular, a USD transaction in an interrupted transaction atom must be rolled back by creating an opposite transfer.
# Dwolla's "reflector account" is: 812-713-9234

Setup:
  Given members:
  | id   | fullName   | dwolla          | country | email         | flags                | usd |
  | %id1 | Abe One    | %DW_TESTER_ACCT | US      | a@ | dft,ok,person,bona |   1 |
  | %id2 | Bea Two    | %DW_TEST_ACCT   | US      | b@ | dft,ok,person,bona |   0 |
  And transactions: 
  | xid | created   | type     | amount | r  | from | to   | purpose |
  | 1   | %today-7m | signup   |     10 | 10 | ctty | %id1 | signup  |
  | 2   | %today-7m | signup   |     10 | 10 | ctty | %id2 | signup  |
#  And usd transfer count is 0

Scenario: a system crash leaves a transaction incomplete
  Given member "%id1" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 10.20  | 1     | labor   |
  # And USD payment by member %id1 is not recorded
  When cron runs "usdTxsThere"

