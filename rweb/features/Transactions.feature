Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags      | created    | rebate |*
  | .ZZA | Abe One    | -100  | personal    | ok,bona    | %today-15m |      5 |
  | .ZZB | Bea Two    | -200  | personal    | ok,co,bona | %today-15m |     10 |
  | .ZZC | Corner Pub | -300  | corporation | ok,co,bona | %today-15m |     10 |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | payer | amount | tid | created    | completed  |*
  |  .ZZA |  -1000 |   1 | %today-13m | %today-13m |
  |  .ZZB |  -2000 |   1 | %today-13m | %today-13m |
  |  .ZZC |  -3000 |   1 | %today-13m | %today-13m |
  |  .ZZA |    -11 |   2 | %today-1w  |         0  |
  |  .ZZA |     22 |   4 | %today-5d  |         0  |
  |  .ZZA |     33 |   3 | %today-5d  |         0  |
  Then balances:
  | id   | r     |*
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  Given transactions: 
  | xid   | created   | type     | amount | from | to   | purpose | taking |*
  | .AAAB | %today-7m | signup   |    250 | ctty | .ZZA | signup  | 0      |
  | .AAAC | %today-6m | signup   |    250 | ctty | .ZZB | signup  | 0      |
  | .AAAD | %today-6m | signup   |    250 | ctty | .ZZC | signup  | 0      |
  | .AAAE | %today-5m | transfer |     10 | .ZZB | .ZZA | cash E  | 0      |
  | .AAAF | %today-4m | transfer |   1100 | .ZZC | .ZZA | usd F   | 1      |
  | .AAAG | %today-3m | transfer |    240 | .ZZA | .ZZB | what G  | 0      |
  | .AAAH | %today-3m | rebate   |     12 | ctty | .ZZA | rebate  | 0      |
  | .AAAI | %today-3m | bonus    |     24 | ctty | .ZZB | bonus   | 0      |
  | .AAAP | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P  | 0      |
  | .AAAQ | %today-1w | transfer |    120 | .ZZA | .ZZC | this Q  | 1      |
  | .AAAR | %today-1w | rebate   |      6 | ctty | .ZZA | rebate  | 0      |
  | .AAAS | %today-1w | bonus    |     12 | ctty | .ZZC | bonus   | 0      |
  | .AAAV | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V  | 0      |
  Then balances:
  | id   | r       | rewards |*
  | .ZZA |    1918 |     268 |
  | .ZZB |    2554 |     274 |
  | .ZZC |    2332 |     262 |

Scenario: A member looks at transactions for the past year
  When member ".ZZA" visits page "history/period=365"
  Then we show "Transaction History" with:
  |_Start Date |_End Date |
  | %dmy-12m   | %dmy     |
  And with:
  | Start     | From Bank | From You | To You   | Rewards | End       |
  | $1,000.00 |      0.00 |   460.00 | 1,110.00 |  268.00 | $1,918.00 |
  | PENDING   |    -44.00 |     0.00 |     0.00 |    0.00 |  - $44.00 |
  And with:
  |_tid | Date   | Name       | From you | To you | Status  |_do   | Purpose    | Reward/Fee |
# | b4  | %dm-5d |            |  22.00   | --     | pending |      | to bank    | --     |
# | b3  | %dm-5d |            |  33.00   | --     | pending |      | to bank    | --     |
  | 6   | %dm-6d | Bea Two    | 100.00   | --     | %chk    | X    | cash V     | --     |
  | 5   | %dm-1w | Corner Pub | 120.00   | --     | %chk    | X    | this Q     | 6.00   |
  | 4   | %dm-3m | Bea Two    | 240.00   | --     | %chk    | X    | what G     | 12.00  |
  | 3   | %dm-4m | Corner Pub | --       | 100.00 | %chk    | X    | usd F      | --     |
  | 2   | %dm-5m | Bea Two    | --       | 10.00  | %chk    | X    | cash E     | --     |
  | 1   | %dm-7m | %ctty      | --       | --     | %chk    |      | signup     | 250.00 |
  And without:
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "history/period=15"
  Then we show "Transaction History" with:
  |_Start Date |_End Date |
  | %dmy-15d   | %dmy     |
  And with:
  | Start     | From Bank | From You | To You | Rewards | End       |
  | $2,132.00 |      0.00 |   220.00 |   0.00 |    6.00 | $1,918.00 |
  | PENDING   |    -44.00 |     0.00 |   0.00 |    0.00 |  - $44.00 |
  And with:
  |_tid | Date   | Name       | From you | To you | Status  | _    | Purpose    | Reward/Fee |
  | 6   | %dm-6d | Bea Two    | 100.00   | --     | %chk    | X    | cash V     | --     |
  | 5   | %dm-1w | Corner Pub | 120.00   | --     | %chk    | X    | this Q     | 6.00   |
  And without:
  | pie N    |
  | whatever |
  | usd F    |
  | cash E   |
  | signup   |
  | rebate   |
  | bonus    |
Skip
Scenario: Transactions with other states show up properly
  Given transactions:
  | xid   | created   | type     | state    | amount | from | to   | purpose  | taking |*
  | .AACA | %today-5d | transfer | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
  | .AACB | %today-5d | rebate   | denied   |      5 | ctty | .ZZC | rebate   | 0      |
  | .AACC | %today-5d | bonus    | denied   |     10 | ctty | .ZZA | bonus    | 0      |
  | .AACD | %today-5d | transfer | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
  | .AACE | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  | .AACF | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  | .AACG | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  | .AACH | %today-5d | transfer | deleted  |    200 | .ZZA | .ZZC | never    | 1      |
  | .AACK | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
  Then balances:
  | id   | balance |*
  | .ZZA |    1942 |
  | .ZZB |    2554 |
  | .ZZC |    2320 |
  When member ".ZZA" visits page "history/period=5"
  Then we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status   | _  | Purpose    | Reward/Fee |
  | 15  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X  | cash CL    | --     |
  | 13  | %dm-5d | Corner Pub | 80.00    | --     | disputed | OK | this CF    | 4.00   |
  | 11  | %dm-5d | Corner Pub | --       | 100.00 | denied   | X  | labor CA   | 10.00  |
  | b4  | %dm-5d |            |  22.00   | --     | pending  |    | to bank    | --     |
  | b3  | %dm-5d |            |  33.00   | --     | pending  |    | to bank    | --     |
  # 12 is missing because ZZA denied it
  And without:
  | cash CE |
  | never   |
  | rebate  |
  | bonus   |
  When member ".ZZC" visits page "history/period=5"
  Then we show "Transaction History" with:
  |_tid | Date   | Name       | From you | To you | Status   | _  | Purpose    | Reward/Fee |
  | 10  | %dm-5d | Abe One    | 100.00   | --     | disputed | OK | cash CL    | --     |
  | 8   | %dm-5d | Abe One    | --       | 80.00  | disputed | X  | this CF    | 8.00   |
  | 7   | %dm-5d | Abe One    | --       | 5.00   | denied   | X  | cash CE    | --     |
  And without:
  | labor CA|
  | never   |
  | rebate  |
  | bonus   |
Resume
