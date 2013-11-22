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
  | id   | fullName   | dw | country | email | flags              |
  | .ZZA | Abe One    |  1 | US      | a@    | dft,ok,person,bona |
  | .ZZB | Bea Two    |  0 | US      | b@    | dft,ok,person,bona |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |
  |   1 | %today-6m | signup |   1000 | ctty | .ZZA | signup  | 0      |
  And balances:
  | id   | dw/usd |
  | .ZZA |      5 |
  
Scenario: A mixed rCredits/USD transaction happens
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount  | goods | purpose |
  | pay | Bea Two | 1000.20 | 1     | labor   |
  Then transactions: 
  | xid | type     | state | amount  | r      | from | to   | purpose      | taking |
  |   2 | transfer | done  | 1000.20 |   1000 | .ZZA | .ZZB | labor        | 0      |
  |   3 | rebate   | done  |   50.01 |  50.01 | ctty | .ZZA | rebate on #2 | 0      |
  |   4 | bonus    | done  |  100.02 | 100.02 | ctty | .ZZB | bonus on #1  | 0      |
  And usd transfers:
  | payer | payee | amount |
  | .ZZA  |  .ZZB |   0.20 |
  And balances:
  | id   | r     | dw/usd | rewards |
  | .ZZA | 50.01 |   4.80 | 1050.01 |
  When member ".ZZA" visits page "transactions/period=365"
  Then we show "Transaction History" with:
  |_Start Date |_End Date |
  | %dmy-12m   | %dmy     |
  And with:
  | Starting | From You | To You | Rewards  | Ending  |
  | $0.00    | 1,000.20 |   0.00 | 1,050.01 |  $49.81 |
  | PENDING  | 0.00     |   0.00 |     0.00 | + $0.00 |
  And with:
  |_tid | Date | Name    | From you | To you | Status  |_buttons | Purpose | Reward |
  | 2   | %dm  | Bea Two | 1,000.20 | --     | %chk    | X       | labor   |  50.01 |

Scenario: A member confirms payment with insufficient USD balance
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount  | goods | purpose |
  | pay | Bea Two | 1005.01 | 1     | labor   |
  Then we say "error": "short to" with subs:
  | short |
  | $0.01 |
  And balances:
  | id   | r    | dw/usd | rewards |
  | .ZZA | 1000 |      5 |    1000 |
  
Scenario: A member buys something when Dwolla is down
  Given Dwolla is down
  When member ".ZZA" confirms form "pay" with values:
  | op  | who     | amount  | goods | purpose |
  | pay | Bea Two | 1000.20 | 1     | labor   |
  Then transactions: 
  | xid | type     | state | amount  | r      | from | to   | purpose      | taking | usdXid |
  |   2 | transfer | done  | 1000.20 |   1000 | .ZZA | .ZZB | labor        | 0      |     -1 |
  |   3 | rebate   | done  |   50.01 |  50.01 | ctty | .ZZA | rebate on #2 | 0      |        |
  |   4 | bonus    | done  |  100.02 | 100.02 | ctty | .ZZB | bonus on #1  | 0      |        |
  And balances:
  | id   | r     | dw/usd | rewards |
  | .ZZA | 50.01 |   4.80 | 1050.01 |
  And usd transfer count is 0
  