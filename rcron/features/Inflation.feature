Feature: Inflation
AS a member
I WANT to receive an inflation adjustment on my average rCredits account balance over the past month
SO I maintain the value of my earnings and savings.

Setup:
  Given members:
  | id   | fullName   | floor | acctType      | flags                        |
  | .ZZA | Abe One    | -100  | personal    | dft,ok,person,bona         |
  | .ZZB | Bea Two    | -200  | personal    | dft,ok,person,company,bona |
  | .ZZC | Corner Pub | -300  | corporation | dft,ok,company,bona          |
  When transactions: 
  | xid | created   | type     | state    | amount | r  | from | to   | purpose |
  |   1 | %today-2m | signup   | done     |    500 | 100 | ctty | .ZZA | signup  |
  |   2 | %today-2m | signup   | done     |    200 | 100 | ctty | .ZZB | signup  |
  |   3 | %today-2m | signup   | done     |    400 | 100 | ctty | .ZZC | signup  |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 100 | 400 |
  | .ZZB | 100 | 100 |
  | .ZZC | 100 | 300 |
  When transactions: 
  | xid | created   | type     | state    | amount | r | from | to   | purpose |
  |   4 | %today-9d | transfer | done     |     10 |10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 110 | 400 |
  | .ZZB |  90 | 100 |
  | .ZZC | 100 | 300 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |   5 | %today-8d | transfer | done     |    100 |  20 | .ZZC | .ZZA | usd F   |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 130 | 480 |
  | .ZZB |  90 | 100 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |   6 | %today-7d | transfer | done     |    240 |  40 | .ZZA | .ZZB | what G  |
  |   7 | %today-7d | rebate   | done     |     12 |  12 | ctty | .ZZA | rebate  |
  |   8 | %today-7d | bonus    | done     |     24 |  24 | ctty | .ZZB | bonus   |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 102 | 280 |
  | .ZZB | 154 | 300 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |   9 | %today-6d | transfer | done     |    100 | 100 | .ZZA | .ZZB | pie N   |
  |  10 | %today-6d | rebate   | done     |      5 |   5 | ctty | .ZZA | rebate  |
  |  11 | %today-6d | bonus    | done     |     10 |  10 | ctty | .ZZB | bonus   |
  Then balances:
  | id   | r   | usd |
  | .ZZA |   7 | 280 |
  | .ZZB | 264 | 300 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |  12 | %today-5d | transfer | done     |    100 | 100 | .ZZC | .ZZA | labor M |
  |  13 | %today-5d | rebate   | done     |      5 |   5 | ctty | .ZZC | rebate  |
  |  14 | %today-5d | bonus    | done     |     10 |  10 | ctty | .ZZA | bonus   |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 117 | 280 |
  | .ZZB | 264 | 300 |
  | .ZZC | -15 | 220 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |  15 | %today-4d | transfer | done     |     50 |   5 | .ZZB | .ZZC | cash P  |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 117 | 280 |
  | .ZZB | 259 | 255 |
  | .ZZC | -10 | 265 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |  16 | %today-3d | transfer | done     |    120 |  80 | .ZZA | .ZZC | this Q  |
  |  17 | %today-3d | rebate   | done     |      6 |   6 | ctty | .ZZA | rebate  |
  |  18 | %today-3d | bonus    | done     |     12 |  12 | ctty | .ZZC | bonus   |
  Then balances:
  | id   | r   | usd |
  | .ZZA |  43 | 240 |
  | .ZZB | 259 | 255 |
  | .ZZC |  82 | 305 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |  19 | %today-2d | transfer | pending  |    123 |  70 | .ZZA | .ZZB | cash T  |
  |  20 | %today-2d | transfer | denied   |     86 |  50 | .ZZB | .ZZA | cash U  |
  Then balances:
  | id   | r   | usd |
  | .ZZA |  43 | 240 |
  | .ZZB | 259 | 255 |
  | .ZZC |  82 | 305 |
  When transactions: 
  | xid | created   | type     | state    | amount | r   | from | to   | purpose |
  |  21 | %today-1d | transfer | done     |    100 |   0 | .ZZA | .ZZB | cash V  |
  Then balances:
  | id   | r   | usd |
  | .ZZA |  43 | 140 |
  | .ZZB | 259 | 355 |
  | .ZZC |  82 | 305 |

Scenario: Inflation adjustments are distributed
  When cron runs "lessOften"
  Then transactions: 
  | xid| created| type      | state | amount                               | from | to   | purpose |
  | 22 | %today | inflation | done  | %(round(%R_INFLATION_RATE*38.42, 2)) | ctty | .ZZA | inflation adjustment |
  | 23 | %today | inflation | done  | %(round(%R_INFLATION_RATE*23.11, 2)) | ctty | .ZZB | inflation adjustment |
  | 24 | %today | inflation | done  | %(round(%R_INFLATION_RATE*31.45, 2)) | ctty | .ZZC | inflation adjustment |
# A: (21*(100+400) + 110+400 + 130+480 + 102+280 + 7+280 + 2*(117+280) + 2*(43+240) + 43+140)/30 * R/12 = 38.4222*R = 1.92
# B: (21*200 + 2*(90+100) + 154+300 + 2*(264+300) + 3*(259+255) + 259+355)/30 * R/12 = 23.10556*R = 1.16
# C: (22*400 + 3*(80+220) + -15+220 + -10+265 + 3*(82+305))/30 * R/12 = 31.4472*R = 1.57