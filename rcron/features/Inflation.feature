Feature: Inflation
AS a member
I WANT to receive an inflation adjustment on my average rCredits account balance over the past month
SO I maintain the value of my earnings and savings.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags      |
  | .ZZA | Abe One    | -100  | personal    | ok,bona    |
  | .ZZB | Bea Two    | -200  | personal    | ok,co,bona |
  | .ZZC | Corner Pub | -300  | corporation | ok,co,bona |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   1 | %today-2m | signup   |    100 | ctty | .ZZA | signup  |
  |   2 | %today-2m | signup   |    100 | ctty | .ZZB | signup  |
  |   3 | %today-2m | signup   |    100 | ctty | .ZZC | signup  |
  And usd transfers:
  | payer | payee | amount | completed |
  | .ZZA  |     0 |   -400 | %today-2m |  
  | .ZZB  |     0 |   -100 | %today-2m |  
  | .ZZC  |     0 |   -300 | %today-2m |  
  Then balances:
  | id   | r   | usd |
  | .ZZA | 100 | 400 |
  | .ZZB | 100 | 100 |
  | .ZZC | 100 | 300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   4 | %today-9d | transfer |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 110 | 400 |
  | .ZZB |  90 | 100 |
  | .ZZC | 100 | 300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   5 | %today-8d | transfer |    100 | .ZZC | .ZZA | usd F   |
  And usd transfers:
  | payer | payee | amount | completed |
  | .ZZC  | .ZZA  |     80 | %today-8d |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 130 | 480 |
  | .ZZB |  90 | 100 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   6 | %today-7d | transfer |    240 | .ZZA | .ZZB | what G  |
  |   7 | %today-7d | rebate   |     12 | ctty | .ZZA | rebate on #4  |
  |   8 | %today-7d | bonus    |     24 | ctty | .ZZB | bonus on #3  |
    And usd transfers:
  | payer | payee | amount | completed |
  | .ZZA  | .ZZB  |    200 | %today-7d |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 102 | 280 |
  | .ZZB | 154 | 300 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |   9 | %today-6d | transfer |    100 | .ZZA | .ZZB | pie N   |
  |  10 | %today-6d | rebate   |      5 | ctty | .ZZA | rebate on #5 |
  |  11 | %today-6d | bonus    |     10 | ctty | .ZZB | bonus on #4  |
  Then balances:
  | id   | r   | usd |
  | .ZZA |   7 | 280 |
  | .ZZB | 264 | 300 |
  | .ZZC |  80 | 220 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |  12 | %today-5d | transfer |    100 | .ZZC | .ZZA | labor M |
  |  13 | %today-5d | rebate   |      5 | ctty | .ZZC | rebate on #3 |
  |  14 | %today-5d | bonus    |     10 | ctty | .ZZA | bonus on #6  |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 117 | 280 |
  | .ZZB | 264 | 300 |
  | .ZZC | -15 | 220 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |  15 | %today-4d | transfer |     50 | .ZZB | .ZZC | cash P  |
  And usd transfers:
  | payer | payee | amount | completed |
  | .ZZB  | .ZZC  |     45 | %today-4d |
  Then balances:
  | id   | r   | usd |
  | .ZZA | 117 | 280 |
  | .ZZB | 259 | 255 |
  | .ZZC | -10 | 265 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |  16 | %today-3d | transfer |    120 | .ZZA | .ZZC | this Q  |
  |  17 | %today-3d | rebate   |      6 | ctty | .ZZA | rebate on #7 |
  |  18 | %today-3d | bonus    |     12 | ctty | .ZZC | bonus on #5  |
  And usd transfers:
  | payer | payee | amount | completed |
  | .ZZA  | .ZZC  |     40 | %today-3d |
  Then balances:
  | id   | r   | usd |
  | .ZZA |  43 | 240 |
  | .ZZB | 259 | 255 |
  | .ZZC |  82 | 305 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |
  |  19 | %today-1d | transfer |    100 | .ZZA | .ZZB | cash V  |
  And usd transfers:
  | payer | payee | amount | completed |
  | .ZZA  | .ZZB  |    100 | %today-1d |
  Then balances:
  | id   | r   | usd |
  | .ZZA |  43 | 140 |
  | .ZZB | 259 | 355 |
  | .ZZC |  82 | 305 |

Scenario: Inflation adjustments are distributed
  When cron runs "lessOften"
  Then transactions: 
  | xid| created| type      | amount                               | from | to   | purpose |
  | 20 | %today | inflation | %(round(%R_INFLATION_RATE*38.42, 2)) | ctty | .ZZA | inflation adjustment |
  | 21 | %today | inflation | %(round(%R_INFLATION_RATE*23.11, 2)) | ctty | .ZZB | inflation adjustment |
  | 22 | %today | inflation | %(round(%R_INFLATION_RATE*31.45, 2)) | ctty | .ZZC | inflation adjustment |
# A: (21*(100+400) + 110+400 + 130+480 + 102+280 + 7+280 + 2*(117+280) + 2*(43+240) + 43+140)/30 * R/12 = 38.4222*R = 1.92
# B: (21*200 + 2*(90+100) + 154+300 + 2*(264+300) + 3*(259+255) + 259+355)/30 * R/12 = 23.10556*R = 1.16
# C: (22*400 + 3*(80+220) + -15+220 + -10+265 + 3*(82+305))/30 * R/12 = 31.4472*R = 1.57