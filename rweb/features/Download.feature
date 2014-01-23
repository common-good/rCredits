Feature: Download
AS a member
I WANT to download my transactions
SO I can see what happened and possbily integrate with an accounting program.

Setup:
  Given members:
  | id   | fullName | floor | acctType    | flags                         |
  | .ZZA | Abe One  | -100  | personal    | dft,ok,dw,person,bona         |
  | .ZZB | Bea Two  | -200  | personal    | dft,ok,dw,person,company,bona |
  | .ZZC | Our Pub  | -300  | corporation | dft,ok,dw,company,bona        |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | payer | payee | amount | created   | completed | tid |
  | .ZZA  |     0 |  -1000 | %today-4m | %today-4m |   1 |
  | .ZZB  |     0 |  -2000 | %today-5m | %today-5m |   1 |
  | .ZZC  |     0 |  -3000 | %today-6m | %today-6m |   1 |
  | .ZZA  |     0 |   -200 | %today-3d |         0 |   2 |
  And balances:
  | id   | usd   |
  | ctty | 10000 |
  And transactions: 
  | xid | created   | type     | state    | amount | from | to   | purpose  | taking |
  |   1 | %today-7m | signup   | done     |    250 | ctty | .ZZA | signup   | 0      |
  |   2 | %today-6m | signup   | done     |    250 | ctty | .ZZB | signup   | 0      |
  |   3 | %today-6m | signup   | done     |    250 | ctty | .ZZC | signup   | 0      |
  |   4 | %today-5m | transfer | done     |     10 | .ZZB | .ZZA | cash E   | 0      |
  |   5 | %today-4m | transfer | done     |    100 | .ZZC | .ZZA | usd F    | 1      |
  |   6 | %today-3m | transfer | done     |    240 | .ZZA | .ZZB | what G   | 0      |
  |   7 | %today-3m | rebate   | done     |     12 | ctty | .ZZA | rebate   | 0      |
  |   8 | %today-3m | bonus    | done     |     24 | ctty | .ZZB | bonus    | 0      |
  |   9 | %today-3w | transfer | pending  |    100 | .ZZA | .ZZB | pie N    | 1      |
  |  10 | %today-3w | rebate   | pending  |      5 | ctty | .ZZA | rebate   | 0      |
  |  11 | %today-3w | bonus    | pending  |     10 | ctty | .ZZB | bonus    | 0      |
  |  12 | %today-2w | transfer | pending  |    100 | .ZZC | .ZZA | labor M  | 0      |
  |  13 | %today-2w | rebate   | pending  |      5 | ctty | .ZZC | rebate   | 0      |
  |  14 | %today-2w | bonus    | pending  |     10 | ctty | .ZZA | bonus    | 0      |
  |  15 | %today-2w | transfer | done     |     50 | .ZZB | .ZZC | cash P   | 0      |
  |  16 | %today-1w | transfer | done     |    120 | .ZZA | .ZZC | this Q   | 1      |
  |  17 | %today-1w | rebate   | done     |      6 | ctty | .ZZA | rebate   | 0      |
  |  18 | %today-1w | bonus    | done     |     12 | ctty | .ZZC | bonus    | 0      |
  |  19 | %today-6d | transfer | pending  |    100 | .ZZA | .ZZB | cash T   | 0      |
  |  20 | %today-6d | transfer | pending  |    100 | .ZZB | .ZZA | cash U   | 1      |
  |  21 | %today-6d | transfer | done     |    100 | .ZZA | .ZZB | cash V   | 0      |
  |  22 | %today-5d | transfer | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
  |  23 | %today-5d | rebate   | denied   |      5 | ctty | .ZZC | rebate   | 0      |
  |  24 | %today-5d | bonus    | denied   |     10 | ctty | .ZZA | bonus    | 0      |
  |  25 | %today-5d | transfer | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
  |  26 | %today-5d | transfer | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
  |  27 | %today-5d | rebate   | disputed |      4 | ctty | .ZZA | rebate   | 0      |
  |  28 | %today-5d | bonus    | disputed |      8 | ctty | .ZZC | bonus    | 0      |
  |  29 | %today-5d | transfer | deleted  |    200 | .ZZA | .ZZC | USD nope | 1      |
  |  30 | %today-5d | transfer | disputed |    100 | .ZZC | .ZZA | cash CJ  | 1      |
  Then balances:
  | id   | r    | usd   | rewards |
  | ctty | -816 | 10000 |       0 |
  | .ZZA |  -58 |  1000 |     272 |
  | .ZZB |  554 |  2000 |     274 |
  | .ZZC |  320 |  3000 |     270 |

Scenario: A member downloads transactions for the past year
  When member ".ZZA" visits page "transactions/period=365&download=1&options=%RUSD_BOTH%STATES_BOTH%_N%_N%_N%_XCH%_VPAY"
  Then we download "rcredits%todayn-12m-%todayn.csv" with:
  # For example rcredits20120525-20130524.csv
  | t# | Created | Name    | From bank | From you | To you | Status   | Purpose    | Reward | Net  |
  | b2 | %ymd-3d |         |       200 |          |        | pending  | from bank  |        |  200 |
  | b1 | %ymd-4m |         |      1000 |          |        | done     | from bank  |        | 1000 |
  | 15 | %ymd-5d | Our Pub |           |          |    100 | disputed | cash CJ    |        |  100 |
  | 13 | %ymd-5d | Our Pub |           |       80 |        | disputed | this CF    |      4 |  -76 |
  | 11 | %ymd-5d | Our Pub |           |          |    100 | denied   | labor CA   |     10 |  110 |
  | 10 | %ymd-6d | Bea Two |           |      100 |        | done     | cash V     |        | -100 |
  | 9  | %ymd-6d | Bea Two |           |          |    100 | pending  | cash U     |        |  100 |
  | 8  | %ymd-6d | Bea Two |           |      100 |        | pending  | cash T     |        | -100 |
  | 7  | %ymd-1w | Our Pub |           |      120 |        | done     | this Q     |      6 | -114 |
  | 6  | %ymd-2w | Our Pub |           |          |    100 | pending  | labor M    |     10 |  110 |
  | 5  | %ymd-3w | Bea Two |           |      100 |        | pending  | pie N      |      5 |  -95 |
  | 4  | %ymd-3m | Bea Two |           |      240 |        | done     | what G     |     12 | -228 |
  | 3  | %ymd-4m | Our Pub |           |          |    100 | done     | usd F      |        |  100 |
  | 2  | %ymd-5m | Bea Two |           |          |     10 | done     | cash E     |        |   10 |
  | 1  | %ymd-7m | %ctty   |           |          |        | done     | signup     |    250 |  250 |
  |    |         | TOTALS  |      1200 |      740 |    510 |          |            |    297 | 1267 |
  And with download columns:
  | column |
  | Date   |

Scenario: A member downloads completed transactions for the past year
  When member ".ZZA" visits page "transactions/period=365&download=1&options=%RUSD_BOTH%STATES_DONE%_N%_N%_N%_XCH%_VPAY"
  Then we download "rcredits%todayn-12m-%todayn.csv" with:
  | t# | Created | Name    | From bank | From you | To you | Status   | Purpose    | Reward | Net  |
  | b1 | %ymd-4m |         |      1000 |          |        | done     | from bank  |        | 1000 |
  | 15 | %ymd-5d | Our Pub |           |          |    100 | disputed | cash CJ    |        |  100 |
  | 13 | %ymd-5d | Our Pub |           |       80 |        | disputed | this CF    |      4 |  -76 |
  | 10 | %ymd-6d | Bea Two |           |      100 |        | done     | cash V     |        | -100 |
  | 7  | %ymd-1w | Our Pub |           |      120 |        | done     | this Q     |      6 | -114 |
  | 4  | %ymd-3m | Bea Two |           |      240 |        | done     | what G     |     12 | -228 |
  | 3  | %ymd-4m | Our Pub |           |          |    100 | done     | usd F      |        |  100 |
  | 2  | %ymd-5m | Bea Two |           |          |     10 | done     | cash E     |        |   10 |
  | 1  | %ymd-7m | %ctty   |           |          |        | done     | signup     |    250 |  250 |
  |    |         | TOTALS  |      1000 |      540 |    210 |          |            |    272 |  942 |
