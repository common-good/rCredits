Feature: Lost USD Txs
AS a member
I WANT my inconsistent USD transaction data to be made consistent ASAP
SO I can depend on the accuracy of rCredits system records
# In particular, a USD transaction in an interrupted transaction atom must be rolled back by creating an opposite transfer.
# Dwolla's "reflector account" is: 812-713-9234
# $1000 is the maximum allowed per day for r transfers (our arbitrary rule)

Setup:
  Given members:
  | id   | fullName   | dw | country | email | flags      | rebate |*
  | %id1 | Abe One    |  1 | US      | a@    | ok,dw,bona |      5 |
  | %id2 | Bea Two    |  0 | US      | b@    | ok,dw,bona |      5 |
  And balances:
  | id   | dw/usd |*
  | %id1 |      2 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  | 1   | %today-7m | signup   |   1000 | ctty | %id1 | signup  |
  | 2   | %today-7m | signup   |   1000 | ctty | %id2 | signup  |
  Then usd transfer count is 0

Scenario: a system crash leaves a transaction incomplete
  When member %id1 trades $1 USD to member %id2 for rCredits
  Then usd transfer count is 1
  Given usd transfer count is 0
  When cron runs "usdTxsThere"
  Then usd transfers:
  | payer | payee | amount |*
  | %id1  |  %id2 |      1 |