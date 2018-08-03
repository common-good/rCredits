Feature: Download
AS a member
I WANT to download my transactions
SO I can see what happened and possbily integrate with an accounting program.

Setup:
  Given members:
  | id   | fullName | floor | acctType    | flags      |*
  | .ZZA | Abe One  | -100  | personal    | ok         |
  | .ZZB | Bea Two  | -200  | personal    | ok,co      |
  | .ZZC | Our Pub  | -300  | corporation | ok,co      |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount | created   | completed | tid |*
  |  501 | .ZZA  |   1000 | %today-4m | %today-4m |   1 |
  |  502 | .ZZB  |   2000 | %today-5m | %today-5m |   1 |
  |  503 | .ZZC  |   3000 | %today-6m | %today-6m |   1 |
  |  504 | .ZZA  |    200 | %today-3d |         0 |   2 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose  | taking |*
  |   1 | %today-7m | signup   |    250 | ctty | .ZZA | signup   | 0      |
  |   2 | %today-6m | signup   |    250 | ctty | .ZZB | signup   | 0      |
  |   3 | %today-6m | signup   |    250 | ctty | .ZZC | signup   | 0      |
  |   4 | %today-5m | transfer |     10 | .ZZB | .ZZA | cash E   | 0      |
  |   5 | %today-4m | transfer |    100 | .ZZC | .ZZA | usd F    | 1      |
  |   6 | %today-3m | transfer |    240 | .ZZA | .ZZB | what G   | 0      |
  |   7 | %today-2w | transfer |     50 | .ZZB | .ZZC | cash P   | 0      |
  |   8 | %today-1w | transfer |    120 | .ZZA | .ZZC | this Q   | 1      |
  |   9 | %today-6d | transfer |    100 | .ZZA | .ZZB | cash V   | 0      |
  |  10 | %today-5d | transfer |     80 | .ZZA | .ZZC | this CF  | 1      |
  |  11 | %today-5d | transfer |    100 | .ZZC | .ZZA | cash CJ  | 1      |
  Then balances:
  | id   | balance |*
  | .ZZA |     670 |
  | .ZZB |    2280 |
  | .ZZC |    3050 |

Scenario: A member downloads transactions for the past year
  Given members have:
  | id   | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365&download=1"
  Then we download "%PROJECT_ID%todayn-12m-%todayn.csv" with:
  # For example rcredits20120525-20130524.csv
  | Tx# | Date    | Name    | Purpose   | From Bank | From You | To You | Balance | Net  |*
  | 8   | %ymd-5d | Our Pub | cash CJ   |           |          |    100 |    670 |  100 |
  | 7   | %ymd-5d | Our Pub | this CF   |           |       80 |        |    570 |  -80 |
  | 6   | %ymd-6d | Bea Two | cash V    |           |      100 |        |    650 | -100 |
  | 5   | %ymd-1w | Our Pub | this Q    |           |      120 |        |    750 | -120 |
  | 4   | %ymd-3m | Bea Two | what G    |           |      240 |        |    870 | -240 |
  | 501 | %ymd-4m |         | from bank |      1000 |          |        |   1110 | 1000 |
  | 3   | %ymd-4m | Our Pub | usd F     |           |          |    100 |    110 |  100 |
  | 2   | %ymd-5m | Bea Two | cash E    |           |          |     10 |     10 |   10 |
  |     |         | TOTALS  |           |      1000 |      540 |    210 |        |  670 |
#  | 1   | %ymd-7m | ZZrCred | signup    |           |          |        |    250 |  250 |
  And with download columns:
  | column |*
  | Date   |

