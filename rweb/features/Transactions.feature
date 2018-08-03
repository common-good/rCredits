Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags      | created    |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup | %today-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co      | %today-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co      | %today-15m |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | payee | amount | tid | created    | completed  |*
  |  .ZZA |   1000 |   1 | %today-13m | %today-13m |
  |  .ZZB |   2000 |   1 | %today-13m | %today-13m |
  |  .ZZC |   3000 |   1 | %today-13m | %today-13m |
  |  .ZZA |     11 |   2 | %today-1w  |         0  |
  |  .ZZA |    -22 |   4 | %today-5d  |         0  |
  |  .ZZA |    -33 |   3 | %today-5d  |         0  |
  Then balances:
  | id   | balance |*
  | .ZZA |    1000 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given transactions: 
  | xid | created   | type     | amount | from | to   | purpose | taking |*
  |   1 | %today-7m | signup   |      0 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup   |      0 | ctty | .ZZB | signup  | 0      |
  |   3 | %today-6m | signup   |      0 | ctty | .ZZC | signup  | 0      |
  |   4 | %today-5m | transfer |     10 | .ZZB | .ZZA | cash E  | 0      |
  |   5 | %today-4m | transfer |   1100 | .ZZC | .ZZA | usd F   | 1      |
  |   6 | %today-3m | transfer |    240 | .ZZA | .ZZB | what G  | 0      |
  |   7 | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P  | 0      |
  |   8 | %today-1w | transfer |    120 | .ZZA | .ZZC | this Q  | 1      |
  |   9 | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V  | 0      |
  Then balances:
  | id   | balance |*
  | .ZZA |    1650 |
  | .ZZB |    2280 |
  | .ZZC |    2070 |

Scenario: A member looks at transactions for the past year
  Given members have:
  | id   | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365"
  Then we show "Transaction History" with:
  | Start        |   | 1,000.00 | %dmy-12m |
  | Received     | + | 1,110.00 |          |
  | Out          | - |   460.00 |          |
#  | Credit Line+ |   |          |          |
  | End          |   | 1,650.00 | %dmy     |
  And with:
  |~tid | Date   | Name       | Purpose  | Amount   |  Balance |~do |
  | 6   | %mdy-6d | Bea Two    | cash V  |  -100.00 | 1,650.00 | X  |
  | 5   | %mdy-1w | Corner Pub | this Q  |  -120.00 | 1,750.00 | X  |
  | 4   | %mdy-3m | Bea Two    | what G  |  -240.00 | 1,870.00 | X  |
  | 3   | %mdy-4m | Corner Pub | usd F   | 1,100.00 | 2,110.00 | X  |
  | 2   | %mdy-5m | Bea Two    | cash E  |    10.00 | 1,010.00 | X  |
#  | 1   | %mdy-7m | ZZrCred    | signup  |     0.00 |   .00 |    |
  And without:
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | - 44.00 Pending |
  | Received     | + |     0.00 |          |
  | Out          | - |   220.00 |          |
#  | Credit Line+ | + |          |          |
  | End          |   | 1,650.00 | %dmy     |
  And with:
  |~tid | Date   | Name       | Purpose    | Amount  |  Balance |~do |
  | 6   | %mdy-6d | Bea Two    | cash V    | -100.00 | 1,650.00 | X  |
  | 5   | %mdy-1w | Corner Pub | this Q    | -120.00 | 1,750.00 | X  |
  And without:
  | pie N    |
  | whatever |
  | usd F    |
  | cash E   |
  | signup   |
  | rebate   |
  | bonus    |

Scenario: A member looks at transactions with roundups
  Given transactions:
  | xid | created | type     | amount | from | to   | purpose  |*
  |  10 | %today  | transfer |  49.95 | .ZZA | .ZZC | sundries |
  Then balances:
  | id   | balance |*
  | .ZZA | 1600.05 |
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | - 44.00 Pending |
  | Received     | + |     0.00 |          |
  | Out          | - |   270.00 |          |
#  | Credit Line+ | + |          |          |
  | End          |   | 1,600.00 | %dmy     |
  And with:
  |~tid | Date    | Name       | Purpose   | Amount  |  Balance |~do |
  | 7   | %mdy    | Corner Pub | sundries  |  -50.00 | 1,600.00 | X  |
  | 6   | %mdy-6d | Bea Two    | cash V    | -100.00 | 1,650.00 | X  |
  | 5   | %mdy-1w | Corner Pub | this Q    | -120.00 | 1,750.00 | X  |
  
#Scenario: Transactions with other states show up properly
#  Given transactions:
#  | xid   | created   | type     | state    | amount | from | to   | purpose  | taking |*
#  | .AACA | %today-5d | transfer | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
#  | .AACB | %today-5d | rebate   | denied   |      5 | ctty | .ZZC | rebate   | 0      |
#  | .AACC | %today-5d | bonus    | denied   |     10 | ctty | .ZZA | bonus    | 0      |
#  | .AACD | %today-5d | transfer | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
#  | .AACE | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
#  | .AACF | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
#  | .AACG | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
#  | .AACH | %today-5d | transfer | deleted  |    200 | .ZZA | .ZZC | never    | 1      |
#  | .AACK | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
#  Then balances:
#  | id   |    r |*
#  | .ZZA | 1942 |
#  | .ZZB | 2554 |
#  | .ZZC | 2320 |
#  When member ".ZZA" visits page "history/transactions/period=5"
#  Then we show "Transaction History" with:
#  |~tid | Date   | Name       | From you | To you | Status   | ~  | Purpose    | Reward/Fee |
#  | 15  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X  | cash CL    | --     |
#  | 13  | %dm-5d | Corner Pub | 80.00    | --     | disputed | OK | this CF    | 4.00   |
#  | 11  | %dm-5d | Corner Pub | --       | 100.00 | denied   | X  | labor CA   | 10.00  |
#  | b4  | %dm-5d |            |  22.00   | --     | pending  |    | to bank    | --     |
#  | b3  | %dm-5d |            |  33.00   | --     | pending  |    | to bank    | --     |
#  # 12 is missing because ZZA denied it
#  And without:
#  | cash CE |
#  | never   |
#  | rebate  |
#  | bonus   |
#  When member ".ZZC" visits page "history/transactions/period=5"
#  Then we show "Transaction History" with:
#  |~tid | Date   | Name       | From you | To you | Status   | ~  | Purpose    | Reward/Fee |
#  | 10  | %dm-5d | Abe One    | 100.00   | --     | disputed | OK | cash CL    | --     |
#  | 8   | %dm-5d | Abe One    | --       | 80.00  | disputed | X  | this CF    | 8.00   |
#  | 7   | %dm-5d | Abe One    | --       | 5.00   | denied   | X  | cash CE    | --     |
#  And without:
#  | labor CA|
#  | never   |
#  | rebate  |
#  | bonus   |
