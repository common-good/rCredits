Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | id   | fullName   | floor | acctType      | flags                        |
  | .ZZA | Abe One    | -100  | %R_PERSONAL   | dft,ok,personal,bona         |
  | .ZZB | Bea Two    | -200  | %R_PERSONAL   | dft,ok,personal,company,bona |
  | .ZZC | Corner Pub | -300  | %R_COMMERCIAL | dft,ok,company,bona          |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd:
  | id   | usd   |
  | ctty | 10000 |
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  And transactions: 
  | xid   | created   | type     | state   | amount | r   | from | to   | purpose | taking |
  | .AAAB | %today-7m | signup   | done    |    250 | 250 | ctty | .ZZA | signup  | 0      |
  | .AAAC | %today-6m | signup   | done    |    250 | 250 | ctty | .ZZB | signup  | 0      |
  | .AAAD | %today-6m | signup   | done    |    250 | 250 | ctty | .ZZC | signup  | 0      |
  | .AAAE | %today-5m | transfer | done    |     10 |  10 | .ZZB | .ZZA | cash E  | 0      |
  | .AAAF | %today-4m | transfer | done    |    100 |  20 | .ZZC | .ZZA | usd F   | 1      |
  | .AAAG | %today-3m | transfer | done    |    240 |  40 | .ZZA | .ZZB | what G  | 0      |
  | .AAAH | %today-3m | rebate   | done    |      2 |   2 | ctty | .ZZA | rebate  | 0      |
  | .AAAI | %today-3m | bonus    | done    |      4 |   4 | ctty | .ZZB | bonus   | 0      |
  | .AAAJ | %today-3w | transfer | pending |    100 | 100 | .ZZA | .ZZB | pie N   | 1      |
  | .AAAK | %today-3w | rebate   | pending |      5 |   5 | ctty | .ZZA | rebate  | 0      |
  | .AAAL | %today-3w | bonus    | pending |     10 |  10 | ctty | .ZZB | bonus   | 0      |
  | .AAAM | %today-2w | transfer | pending |    100 | 100 | .ZZC | .ZZA | labor M | 0      |
  | .AAAN | %today-2w | rebate   | pending |      5 |   5 | ctty | .ZZC | rebate  | 0      |
  | .AAAO | %today-2w | bonus    | pending |     10 |  10 | ctty | .ZZA | bonus   | 0      |
  | .AAAP | %today-2w | transfer | done    |     50 |   5 | .ZZB | .ZZC | cash P  | 0      |
  | .AAAQ | %today-1w | transfer | done    |    120 |  80 | .ZZA | .ZZC | this Q  | 1      |
  | .AAAR | %today-1w | rebate   | done    |      4 |   4 | ctty | .ZZA | rebate  | 0      |
  | .AAAS | %today-1w | bonus    | done    |      8 |   8 | ctty | .ZZC | bonus   | 0      |
  | .AAAT | %today-6d | transfer | pending |    100 | 100 | .ZZA | .ZZB | cash T  | 0      |
  | .AAAU | %today-6d | transfer | pending |    100 | 100 | .ZZB | .ZZA | cash U  | 1      |
  | .AAAV | %today-6d | transfer | done    |    100 |   0 | .ZZA | .ZZB | cash V  | 0      |
  Then balances:
  | id   | r    | usd      | rewards |
  | ctty | -768 | 10000.00 |       0 |
  | .ZZA |  166 |   739.75 |     256 |
  | .ZZB |  279 |  2254.50 |     254 |
  | .ZZC |  323 |  3004.50 |     258 |

Scenario: A member looks at transactions for the past year
  When member ".ZZA" visits page "transactions/period=365&currency=0"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | To You | From You | Rewards | End Balance |
  | %dmy-12m   | %dmy     | $0.00         | 30.00  | 120.00   | 256.00  | $166.00     |
  |            |          | PENDING       | 200.00 | 200.00   | 15.00   | + $15.00    |
  And we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | r%   | Status  | Buttons | Purpose | Rewards |
  | 10  | %dm-6d | Bea Two    | 0.00     | --     | 0.0  | %chk    | X       | cash V  | --      |
  | 9   | %dm-6d | Bea Two    | --       | 100.00 | 100  | pending | X       | cash U  | --      |
  | 8   | %dm-6d | Bea Two    | 100.00   | --     | 100  | pending | X       | cash T  | --      |
  | 7   | %dm-1w | Corner Pub | 80.00    | --     | 66.7 | %chk    | X       | this Q  | 4.00    |
  | 6   | %dm-2w | Corner Pub | --       | 100.00 | 100  | ok?     | OK X    | labor M | 10.00   |
  | 5   | %dm-3w | Bea Two    | 100.00   | --     | 100  | ok?     | OK X    | pie N   | 5.00    |
  | 4   | %dm-3m | Bea Two    | 40.00    | --     | 16.7 | %chk    | X       | what G  | 2.00    |
  | 3   | %dm-4m | Corner Pub | --       | 20.00  | 20.0 | %chk    | X       | usd F   | --      |
  | 2   | %dm-5m | Bea Two    | --       | 10.00  | 100  | %chk    | X       | cash E  | --      |
  | 1   | %dm-7m | %ctty      | --       | --     | 100  | %chk    |         | signup  | 250.00  |
  And we show "Transaction History" without:
  | Purpose |
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "transactions/period=15&currency=0"
  Then we show "Transaction History" with:
  | Start Date | End Date | Start Balance | To You | From You | Rewards | End Balance |
  | %dmy-15d   | %dmy     | $242.00       | 0.00   | 80.00    | 4.00    | $166.00     |
  |            |          | PENDING       | 200.00 | 200.00   | 15.00   | + $15.00    |
  And we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status  | Buttons | Purpose | Rewards |
  | 10  | %dm-6d | Bea Two    | 0.00     | --     | %chk    | X       | cash V  | --      |
  | 9   | %dm-6d | Bea Two    | --       | 100.00 | pending | X       | cash U  | --      |
  | 8   | %dm-6d | Bea Two    | 100.00   | --     | pending | X       | cash T  | --      |
  | 7   | %dm-1w | Corner Pub | 80.00    | --     | %chk    | X       | this Q  | 4.00    |
  | 6   | %dm-2w | Corner Pub | --       | 100.00 | ok?     | OK X    | labor M | 10.00   |
  And we show "Transaction History" without:
  | Purpose  |
  | pie N    |
  | whatever |
  | usd F    |
  | cash E   |
  | signup   |
  | rebate   |
  | bonus    |

Scenario: Transactions with other states show up properly
  Given transactions:
  | xid   | created   | type     | state    | amount | from | to   | purpose  | taking |
  | .AACA | %today-5d | transfer | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
  | .AACB | %today-5d | rebate   | denied   |      5 | ctty | .ZZC | rebate   | 0      |
  | .AACD | %today-5d | bonus    | denied   |     10 | ctty | .ZZA | bonus    | 0      |
  | .AACE | %today-5d | transfer | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
  | .AACF | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  | .AACG | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  | .AACH | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  | .AACI | %today-5d | transfer | deleted  |    200 | .ZZA | .ZZC | never    | 1      |
  | .AACL | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
  Then balances:
  | id   | balance |
  | ctty |    -780 |
  | .ZZA |     190 |
  | .ZZB |     279 |
  | .ZZC |     311 |
  When member ".ZZA" visits page "transactions/period=5&currency=0"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose | Rewards |
  | 15  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X       | cash CL | --      |
  | 13  | %dm-5d | Corner Pub | 80.00    | --     | disputed | OK      | this CF | 4.00    |
  | 11  | %dm-5d | Corner Pub | --       | 100.00 | denied   | X       | labor CA| 10.00   |
  And we show "Transaction History" without:
  | Purpose |
  | cash CE |
  | never   |
  | rebate  |
  | bonus   |
  When member ".ZZC" visits page "transactions/period=5&currency=0"
  Then we show "Transaction History" with:
  | tid | Date   | Name       | From you | To you | Status   | Buttons | Purpose | Rewards |
  | 10  | %dm-5d | Abe One    | 100.00   | --     | disputed | OK      | cash CL | --      |
  | 8   | %dm-5d | Abe One    | --       | 80.00  | disputed | X       | this CF | 8.00    |
  | 7   | %dm-5d | Abe One    | --       | 5.00   | denied   | X       | cash CE | --      |
  And we show "Transaction History" without:
  | Purpose |
  | labor CA|
  | never   |
  | rebate  |
  | bonus   |