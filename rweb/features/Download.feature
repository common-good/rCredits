Feature: Download
AS a member
I WANT to download my transactions
SO I can see what happened and possbily integrate with an accounting program.

Setup:
  Given members:
  | id   | fullName | floor | acctType    | flags      |*
  | .ZZA | Abe One  | -100  | personal    | ok,bona    |
  | .ZZB | Bea Two  | -200  | personal    | ok,co,bona |
  | .ZZC | Our Pub  | -300  | corporation | ok,co,bona |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payer | payee | amount | created   | completed | tid |*
  |  501 | .ZZA  |     0 |  -1000 | %today-4m | %today-4m |   1 |
  |  502 | .ZZB  |     0 |  -2000 | %today-5m | %today-5m |   1 |
  |  503 | .ZZC  |     0 |  -3000 | %today-6m | %today-6m |   1 |
  |  504 | .ZZA  |     0 |   -200 | %today-3d |         0 |   2 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose  | taking |*
  |   1 | %today-7m | signup   |    250 | ctty | .ZZA | signup   | 0      |
  |   2 | %today-6m | signup   |    250 | ctty | .ZZB | signup   | 0      |
  |   3 | %today-6m | signup   |    250 | ctty | .ZZC | signup   | 0      |
  |   4 | %today-5m | transfer |     10 | .ZZB | .ZZA | cash E   | 0      |
  |   5 | %today-4m | transfer |    100 | .ZZC | .ZZA | usd F    | 1      |
  |   6 | %today-3m | transfer |    240 | .ZZA | .ZZB | what G   | 0      |
  |   7 | %today-3m | rebate   |     12 | ctty | .ZZA | rebate   | 0      |
  |   8 | %today-3m | bonus    |     24 | ctty | .ZZB | bonus    | 0      |
  |   9 | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P   | 0      |
  |  10 | %today-1w | transfer |    120 | .ZZA | .ZZC | this Q   | 1      |
  |  11 | %today-1w | rebate   |      6 | ctty | .ZZA | rebate   | 0      |
  |  12 | %today-1w | bonus    |     12 | ctty | .ZZC | bonus    | 0      |
  |  13 | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V   | 0      |
  |  14 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | 1      |
  |  15 | %today-5d | rebate   |      4 | ctty | .ZZA | rebate   | 0      |
  |  16 | %today-5d | bonus    |      8 | ctty | .ZZC | bonus    | 0      |
  |  17 | %today-5d | transfer |    100 | .ZZC | .ZZA | cash CJ  | 1      |
  Then balances:
  | id   | r    | rewards |*
  | .ZZA |  942 |     272 |
  | .ZZB | 2554 |     274 |
  | .ZZC | 3320 |     270 |

Scenario: A member downloads transactions for the past year
  When member ".ZZA" visits page "history/period=365&download=1"
  Then we download "rcredits%todayn-12m-%todayn.csv" with:
  # For example rcredits20120525-20130524.csv
  | Tx# | Date    | Name    | From bank | From you | To you | Purpose | Reward/Fee | Net  |*
  | 501 | %ymd-4m |         |      1000 |          |        | from bank  |        | 1000 |
  | 8   | %ymd-5d | Our Pub |           |          |    100 | cash CJ    |        |  100 |
  | 7   | %ymd-5d | Our Pub |           |       80 |        | this CF    |      4 |  -76 |
  | 6   | %ymd-6d | Bea Two |           |      100 |        | cash V     |        | -100 |
  | 5   | %ymd-1w | Our Pub |           |      120 |        | this Q     |      6 | -114 |
  | 4   | %ymd-3m | Bea Two |           |      240 |        | what G     |     12 | -228 |
  | 3   | %ymd-4m | Our Pub |           |          |    100 | usd F      |        |  100 |
  | 2   | %ymd-5m | Bea Two |           |          |     10 | cash E     |        |   10 |
  | 1   | %ymd-7m | %ctty   |           |          |        | signup     |    250 |  250 |
  |     |         | TOTALS  |      1000 |      540 |    210 |            |    272 |  942 |
  And with download columns:
  | column |*
  | Date   |

