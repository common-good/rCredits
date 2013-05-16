Feature: Download
AS a member
I WANT to download my transactions
SO I can see what happened and possbily integrate with an accounting program.

Setup:
  Given members:
  | id   | fullName | floor | acctType      | flags                        |
  | .ZZA | Abe One  | -100  | %R_PERSONAL   | dft,ok,personal,bona         |
  | .ZZB | Bea Two  | -200  | %R_PERSONAL   | dft,ok,personal,company,bona |
  | .ZZC | Our Pub  | -300  | %R_COMMERCIAL | dft,ok,company,bona          |
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
  | xid   | created   | type     | state    | amount | r   | from | to   | purpose  | taking |
  | .AAAB | %today-7m | signup   | done     |    250 | 250 | ctty | .ZZA | signup   | 0      |
  | .AAAC | %today-6m | signup   | done     |    250 | 250 | ctty | .ZZB | signup   | 0      |
  | .AAAD | %today-6m | signup   | done     |    250 | 250 | ctty | .ZZC | signup   | 0      |
  | .AAAE | %today-5m | transfer | done     |     10 |  10 | .ZZB | .ZZA | cash E   | 0      |
  | .AAAF | %today-4m | transfer | done     |    100 |  20 | .ZZC | .ZZA | usd F    | 1      |
  | .AAAG | %today-3m | transfer | done     |    240 |  40 | .ZZA | .ZZB | what G   | 0      |
  | .AAAH | %today-3m | rebate   | done     |      2 |   2 | ctty | .ZZA | rebate   | 0      |
  | .AAAI | %today-3m | bonus    | done     |      4 |   4 | ctty | .ZZB | bonus    | 0      |
  | .AAAJ | %today-3w | transfer | pending  |    100 | 100 | .ZZA | .ZZB | pie N    | 1      |
  | .AAAK | %today-3w | rebate   | pending  |      5 |   5 | ctty | .ZZA | rebate   | 0      |
  | .AAAL | %today-3w | bonus    | pending  |     10 |  10 | ctty | .ZZB | bonus    | 0      |
  | .AAAM | %today-2w | transfer | pending  |    100 | 100 | .ZZC | .ZZA | labor M  | 0      |
  | .AAAN | %today-2w | rebate   | pending  |      5 |   5 | ctty | .ZZC | rebate   | 0      |
  | .AAAO | %today-2w | bonus    | pending  |     10 |  10 | ctty | .ZZA | bonus    | 0      |
  | .AAAP | %today-2w | transfer | done     |     50 |   5 | .ZZB | .ZZC | cash P   | 0      |
  | .AAAQ | %today-1w | transfer | done     |    120 |  80 | .ZZA | .ZZC | this Q   | 1      |
  | .AAAR | %today-1w | rebate   | done     |      4 |   4 | ctty | .ZZA | rebate   | 0      |
  | .AAAS | %today-1w | bonus    | done     |      8 |   8 | ctty | .ZZC | bonus    | 0      |
  | .AAAT | %today-6d | transfer | pending  |    100 | 100 | .ZZA | .ZZB | cash T   | 0      |
  | .AAAU | %today-6d | transfer | pending  |    100 | 100 | .ZZB | .ZZA | cash U   | 1      |
  | .AAAV | %today-6d | transfer | done     |    100 |   0 | .ZZA | .ZZB | cash V   | 0      |
  | .AACA | %today-5d | transfer | denied   |    100 |  40 | .ZZC | .ZZA | labor CA | 0      |
  | .AACB | %today-5d | rebate   | denied   |      2 |   2 | ctty | .ZZC | rebate   | 0      |
  | .AACD | %today-5d | bonus    | denied   |      4 |   4 | ctty | .ZZA | bonus    | 0      |
  | .AACE | %today-5d | transfer | denied   |      5 |   2 | .ZZA | .ZZC | cash CE  | 1      |
  | .AACF | %today-5d | transfer | disputed |     80 |  20 | .ZZA | .ZZC | this CF  | 1      |
  | .AACG | %today-5d | rebate   | disputed |      1 |   1 | ctty | .ZZA | rebate   | 0      |
  | .AACH | %today-5d | bonus    | disputed |      2 |   2 | ctty | .ZZC | bonus    | 0      |
  | .AACI | %today-5d | transfer | deleted  |    200 |  50 | .ZZA | .ZZC | USD nope | 1      |
  | .AACJ | %today-5d | transfer | disputed |    100 |  80 | .ZZC | .ZZA | cash CL  | 1      |
  Then balances:
  | id   | r    | usd      | rewards |
  | ctty | -771 | 10000.00 |       0 |
  | .ZZA |  227 |   699.50 |     257 |
  | .ZZB |  279 |  2254.50 |     254 |
  | .ZZC |  265 |  3044.25 |     260 |

Scenario: A member downloads transactions for the past year
  When member ".ZZA" visits page "transactions/period=365&download=1&pendings=1"
  Then we download "rcredits%todayn-12m-%todayn.csv" with:
  | t# | Created | Name    | From you | To you | rCredits | USD  | Status   | Purpose | Reward | Net  |
  | 15 | %ymd-5d | Our Pub |          |    100 |       80 |   20 | disputed | cash CL |        |  100 |
  | 13 | %ymd-5d | Our Pub |       80 |        |      -20 |  -60 | disputed | this CF |      1 |  -79 |
  | 11 | %ymd-5d | Our Pub |          |    100 |       40 |   60 | denied   | labor CA|      4 |  104 |
  | 10 | %ymd-6d | Bea Two |      100 |        |        0 | -100 | done     | cash V  |        | -100 |
  | 9  | %ymd-6d | Bea Two |          |    100 |      100 |    0 | pending  | cash U  |        |  100 |
  | 8  | %ymd-6d | Bea Two |      100 |        |     -100 |    0 | pending  | cash T  |        | -100 |
  | 7  | %ymd-1w | Our Pub |      120 |        |      -80 |  -40 | done     | this Q  |      4 | -116 |
  | 6  | %ymd-2w | Our Pub |          |    100 |      100 |    0 | pending  | labor M |     10 |  110 |
  | 5  | %ymd-3w | Bea Two |      100 |        |     -100 |    0 | pending  | pie N   |      5 |  -95 |
  | 4  | %ymd-3m | Bea Two |      240 |        |      -40 | -200 | done     | what G  |      2 | -238 |
  | 3  | %ymd-4m | Our Pub |          |    100 |       20 |   80 | done     | usd F   |        |  100 |
  | 2  | %ymd-5m | Bea Two |          |     10 |       10 |    0 | done     | cash E  |        |   10 |
  | 1  | %ymd-7m | %ctty   |          |        |        0 |    0 | done     | signup  |    250 |  250 |
  And with download columns:
  | column |
  | Date   |
  | USD#   |

Scenario: A member downloads completed transactions for the past year
  When member ".ZZA" visits page "transactions/period=365&download=1&pendings=0"
  Then we download "rcredits%todayn-12m-%todayn.csv" with:
  | t# | Created | Name    | From you | To you | rCredits | USD  | Status   | Purpose | Reward | Net  |
  | 15 | %ymd-5d | Our Pub |          |    100 |       80 |   20 | disputed | cash CL |        |  100 |
  | 13 | %ymd-5d | Our Pub |       80 |        |      -20 |  -60 | disputed | this CF |      1 |  -79 |
  | 10 | %ymd-6d | Bea Two |      100 |        |        0 | -100 | done     | cash V  |        | -100 |
  | 7  | %ymd-1w | Our Pub |      120 |        |      -80 |  -40 | done     | this Q  |      4 | -116 |
  | 4  | %ymd-3m | Bea Two |      240 |        |      -40 | -200 | done     | what G  |      2 | -238 |
  | 3  | %ymd-4m | Our Pub |          |    100 |       20 |   80 | done     | usd F   |        |  100 |
  | 2  | %ymd-5m | Bea Two |          |     10 |       10 |    0 | done     | cash E  |        |   10 |
  | 1  | %ymd-7m | %ctty   |          |        |        0 |    0 | done     | signup  |    250 |  250 |
