Feature: Real USD
AS a member
I WANT my rCredits transaction history to reflect accurately my Dwolla balance
SO I don't lose money or get confused.
# Dwolla's "reflector account" is:
#	Dwolla	812-713-9234
#	Email	reflector@dwolla.com
#	Phone	(406) 699-9999
#	Facebook	dwolla.reflector
#	Twitter	@DwollaReflector

# Assumes real TESTER balance is at least $0.20
# TESTER must have a Dwolla account set up and connected

Setup:
  Given members:
  | id   | fullName   | dwolla          | country | email         | flags                |
  | .ZZA | Abe One    | %DW_TESTER_ACCT | US      | a@example.com | dft,ok,personal,bona |
  | .ZZB | Bea Two    | %DW_TEST_ACCT   | US      | b@example.com | dft,ok,personal,bona |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |
  |   1 | %today-6m | signup |     10 | ctty | .ZZA | signup  | 0      |
  And usd:
  | id   | usd   |
  | .ZZA | +AMT1 |
  
Scenario: A mixed rCredits/USD transaction happens
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 10.20  | 1     | labor   |
  Then transactions: 
  | xid | type     | state | amount | r    | from | to   | purpose      | taking |
  |   2 | transfer | done  |  10.20 |   10 | .ZZA | .ZZB | labor        | 0      |
  |   3 | rebate   | done  |    .50 |  .50 | ctty | .ZZA | rebate on #2 | 0      |
  |   4 | bonus    | done  |   1.00 | 1.00 | ctty | .ZZB | bonus on #1  | 0      |
  And usd transfers:
  | payer | payee | amount |
  | .ZZA  |  .ZZB |   0.20 |
  And balances:
  | id   | r    | usd      | rewards |
  | .ZZA | 0.50 | AMT1-.20 |   10.50 |
  When member ".ZZA" visits page "transactions/period=365"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | From You | To You | Rewards | End Balance |
  | %dmy-12m   | %dmy     | $0.00         | 10.20    |   0.00 |   10.50 |       $0.30 |
  |            |          | PENDING       | 0.00     |   0.00 |    0.00 |     + $0.00 |
  And we show "Transaction History" with:
  | tid | Date | Name    | From you | To you | r%   | Status  | Buttons | Purpose | Rewards |
  | 2   | %dm  | Bea Two | 10.20    | --     | 98.0 | %chk    | X       | labor   |    0.50 |

Scenario: A member confirms payment with insufficient USD balance
  Given usd:
  | id   | usd       |
  | .ZZA | +AMT1+100 |
# meaning the cached usd amount is $100 higher than the actual USD balance
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount     | goods | purpose |
  | pay | Bea Two | AMT1+10.01 | 1     | labor   |
  Then we say "error": "short to" with subs:
  | short |
  | $0.01 |
  And balances:
  | id   | r  | usd  | rewards |
  | .ZZA | 10 | AMT1 |      10 |
  
Scenario: A member buys something when Dwolla is down
  Given Dwolla is down
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 10.20  | 1     | labor   |
  Then transactions: 
  | xid | type     | state | amount | r    | from | to   | purpose      | taking | usdXid |
  |   2 | transfer | done  |  10.20 |   10 | .ZZA | .ZZB | labor        | 0      |     -1 |
  |   3 | rebate   | done  |    .50 |  .50 | ctty | .ZZA | rebate on #2 | 0      |        |
  |   4 | bonus    | done  |   1.00 | 1.00 | ctty | .ZZB | bonus on #1  | 0      |        |
  And balances:
  | id   | r    | usd      | rewards |
  | .ZZA | 0.50 | AMT1-.20 |   10.50 |
  And usd transfer count is 0
  