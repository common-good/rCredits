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

# Next: pay AMT1+10 USD and make sure the transaction totally fails

Setup:
  Given members:
  | id      | fullName   | dwolla             | country | email         | flags           |
  | NEW.ZZA | Abe One    | %DW_TESTER_ACCOUNT | US      | a@example.com | dft,ok,personal |
  | NEW.ZZB | Bea Two    | %DW_TEST_ACCOUNT   | US      | b@example.com | dft,ok,personal |
  And transactions: 
  | xid      | created   | type       | amount | from      | to      | purpose | taking |
  | NEW.AAAB | %today-6m | %TX_SIGNUP |     10 | community | NEW.ZZA | signup  | 0      |
  And usd:
  | id        | usd   |
  | NEW.ZZA   | +AMT1 |
  
Scenario: A mixed rCredits/USD transaction happens
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount | goods | purpose |
  | pay | Bea Two | 10.20  | 1     | labor   |
  Then transactions: 
  | xid      | type         | state    | amount | r    | from      | to      | purpose      | taking |
  | NEW:AAAC | %TX_TRANSFER | %TX_DONE |  10.20 |   10 | NEW.ZZA   | NEW.ZZB | labor        | 0      |
  | NEW:AAAD | %TX_REBATE   | %TX_DONE |    .50 |  .50 | community | NEW.ZZA | rebate on #2 | 0      |
  | NEW:AAAE | %TX_BONUS    | %TX_DONE |   1.00 | 1.00 | community | NEW.ZZB | bonus on #1  | 0      |
  And balances:
  | id        | r    | usd      | rewards |
  | NEW.ZZA   | 0.50 | AMT1-.20 |   10.50 |
  When member "NEW.ZZA" visits page "transactions/period=365"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | To You | From You | Rewards | End Balance |
  | %dmy-12m   | %dmy     | $0.00         | 0.00   |    10.20 |   10.50 |       $0.30 |
  |            |          | PENDING       | 0.00   |     0.00 |    0.00 |     + $0.00 |
  And we show "Transaction History" with:
  | tid | Date | Name    | From you | To you | r%   | Status  | Buttons | Purpose | Rewards |
  | 2   | %dm  | Bea Two | 10.20    | --     | 98.0 | %chk    | X       | labor   |    0.50 |

Scenario: A member confirms payment with insufficient USD balance
  Given usd:
  | id        | usd       |
  | NEW.ZZA   | +AMT1+100 |
# meaning the cached usd amount is $100 higher than the actual USD balance
  When member "NEW.ZZA" confirms form "pay" with values:
  | op  | who     | amount     | goods | purpose |
  | pay | Bea Two | AMT1+10.01 | 1     | labor   |
  Then we say "error": "short to" with subs:
  | short |
  | $0.01 |
  And balances:
  | id        | r  | usd  | rewards |
  | NEW.ZZA   | 10 | AMT1 |      10 |
