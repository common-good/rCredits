Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | id      | full_name  | min_balance | account_type  | flags         |
  | NEW.ZZA | Abe One    | -100        | %R_PERSONAL   | %BIT_DEFAULTS |
  | NEW.ZZB | Bea Two    | -200        | %R_PERSONAL   | %BIT_PARTNER  |
  | NEW.ZZC | Corner Pub | -300        | %R_COMMERCIAL | %BIT_RTRADER  |
  And relations:
  | id      | main    | agent   | permission        |
  | NEW:ZZA | NEW.ZZA | NEW.ZZB | buy and sell      |
  | NEW:ZZB | NEW.ZZB | NEW.ZZA | read transactions |
  | NEW:ZZC | NEW.ZZC | NEW.ZZB | buy and sell      |
  | NEW:ZZD | NEW.ZZC | NEW.ZZA | sell              |
  And transactions: 
  | tx_id    | created   | type         | state       | amount | from      | to      | purpose      | taking |
  | NEW.AAAB | %today-7m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZA | signup       | 0      |
  | NEW.AAAC | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZB | signup       | 0      |
  | NEW.AAAD | %today-6m | %TX_SIGNUP   | %TX_DONE    |    250 | community | NEW.ZZC | signup       | 0      |
  | NEW.AAAE | %today-5m | %TX_TRANSFER | %TX_DONE    |     10 | NEW.ZZB   | NEW.ZZA | cash E       | 0      |
  | NEW.AAAF | %today-4m | %TX_TRANSFER | %TX_DONE    |     20 | NEW.ZZC   | NEW.ZZA | usd F        | 1      |
  | NEW.AAAG | %today-3m | %TX_TRANSFER | %TX_DONE    |     40 | NEW.ZZA   | NEW.ZZB | whatever     | 0      |
  | NEW.AAAH | %today-3m | %TX_REBATE   | %TX_DONE    |      2 | community | NEW.ZZA | rebate       | 0      |
  | NEW.AAAI | %today-3m | %TX_BONUS    | %TX_DONE    |      4 | community | NEW.ZZB | bonus        | 0      |
  | NEW.AAAJ | %today-3w | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZA   | NEW.ZZB | pie N        | 1      |
  | NEW.AAAK | %today-3w | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZC | rebate       | 0      |
  | NEW.AAAL | %today-3w | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus        | 0      |
  | NEW.AAAM | %today-2w | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZC   | NEW.ZZA | labor M      | 0      |
  | NEW.AAAN | %today-2w | %TX_REBATE   | %TX_PENDING |      5 | community | NEW.ZZC | rebate       | 0      |
  | NEW.AAAO | %today-2w | %TX_BONUS    | %TX_PENDING |     10 | community | NEW.ZZA | bonus        | 0      |
  | NEW.AAAP | %today-2w | %TX_TRANSFER | %TX_DONE    |      5 | NEW.ZZB   | NEW.ZZC | cash P       | 0      |
  | NEW.AAAQ | %today-1w | %TX_TRANSFER | %TX_DONE    |     80 | NEW.ZZA   | NEW.ZZC | this & that  | 1      |
  | NEW.AAAR | %today-1w | %TX_REBATE   | %TX_DONE    |      4 | community | NEW.ZZA | rebate       | 0      |
  | NEW.AAAS | %today-1w | %TX_BONUS    | %TX_DONE    |      8 | community | NEW.ZZC | bonus        | 0      |
  | NEW.AAAT | %today-3d | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZA   | NEW.ZZB | cash AB      | 0      |
  | NEW.AAAU | %today-2d | %TX_TRANSFER | %TX_PENDING |    100 | NEW.ZZB   | NEW.ZZA | cash BA      | 1      |
  Then balances:
  | id        | balance |
  | community |    -768 |
  | NEW.ZZA   |     166 |
  | NEW.ZZB   |     279 |
  | NEW.ZZC   |     323 |


Scenario: A member looks at transactions for the past year
  When member "NEW.ZZA" visits page "txs" with options "period=365"
  Then we show page "txs" with:
  | Start Date | End Date | Start Balance | To You | From You | Rewards | End Balance |
  | %dmy-12m   | %dmy     | $0.00         | 30.00  | 120.00   | 256.00  | $166.00     |
  |            |          | PENDING       | 200.00 | 200.00   | 20.00   | + $20.00    |
  And we show page "txs" with:
  | tid | Date   | Name       | From you | To you | Status  | Buttons | Purpose     | Rewards |
  | 9   | %dm-2d | Bea Two    |          | 100.00 | pending | X       | cash        |         |
  | 8   | %dm-3d | Bea Two    | 100.00   |        | pending | X       | cash        |         |
  | 7   | %dm-1w | Corner Pub | 80.00    |        | %chk    | X       | this & that | 4.00    |
  | 6   | %dm-2w | Corner Pub |          | 100.00 | ok?     | OK X    | labor M     | 10.00   |
  | 5   | %dm-3w | Bea Two    |          | 100.00 | ok?     | OK X    | pie N       | 5.00    |
  | 4   | %dm-3m | Bea Two    | 40.00    |        | %chk    | X       | whatever    | 2.00    |
  | 3   | %dm-4m | Corner Pub |          | 20.00  | %chk    | X       | usd F       |         |
  | 2   | %dm-5m | Bea Two    |          | 10.00  | %chk    | X       | cash E      |         |
  | 1   | %dm-7m | %ctty      |          |        | %chk    |         | signup      | 250.00  |

Scenario: A member looks at transactions for the past few days
  When member "NEW.ZZA" visits page "txs" with options "period=15"
  Then we show page "txs" with:
  | Start Date | End Date | Start Balance | To You | From You | Rewards | End Balance |
  | %dmy-15d   | %dmy     | $0.00         | 0.00   | 8.00     | 4.00    | $166.00     |
  |            |          | PENDING       | 200.00 | 200.00   | 20.00   | + $20.00    |
  And we show page "txs" with:
  | tid | Date   | Name       | From you | To you | Status  | Buttons | Purpose     | Rewards |
  | 9   | %dm-2d | Bea Two    |          | 100.00 | pending | X       | cash        |         |
  | 8   | %dm-3d | Bea Two    | 100.00   |        | pending | X       | cash        |         |
  | 7   | %dm-1w | Corner Pub | 80.00    |        | %chk    | X       | this & that | 4.00    |
  | 6   | %dm-2w | Corner Pub |          | 100.00 | ok?     | OK X    | labor M     | 10.00   |
