Feature: Inflation
AS a member
I WANT to receive an inflation adjustment on my average rCredits account balance over the past month
SO I maintain the value of my earnings and savings.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags      | rebate |*
  | .ZZA | Abe One    | -500  | personal    | ok,bona    |      5 |
  | .ZZB | Bea Two    | -500  | personal    | ok,co,bona |     10 |
  | .ZZC | Corner Pub | -500  | corporation | ok,co,bona |     10 |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   1 | %today-2m | signup   |    100 | ctty | .ZZA | signup  |
  |   2 | %today-2m | signup   |    100 | ctty | .ZZB | signup  |
  |   3 | %today-2m | signup   |    100 | ctty | .ZZC | signup  |
  And usd transfers:
  | payer | amount | completed |*
  | .ZZA  |   -400 | %today-2m |  
  | .ZZB  |   -100 | %today-2m |  
  | .ZZC  |   -300 | %today-2m |  
  Then balances:
  | id   | r   |*
  | .ZZA | 500 |
  | .ZZB | 200 |
  | .ZZC | 400 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-9d | transfer |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | id   | r   |*
  | .ZZA | 510 |
  | .ZZB | 190 |
  | .ZZC | 400 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   5 | %today-8d | transfer |    100 | .ZZC | .ZZA | usd F   |
  Then balances:
  | id   | r   |*
  | .ZZA | 610 |
  | .ZZB | 190 |
  | .ZZC | 300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   6 | %today-7d | transfer |    240 | .ZZA | .ZZB | what G  |
  |   7 | %today-7d | rebate   |     12 | ctty | .ZZA | rebate on #4  |
  |   8 | %today-7d | bonus    |     24 | ctty | .ZZB | bonus on #3  |
  Then balances:
  | id   | r   |*
  | .ZZA | 382 |
  | .ZZB | 454 |
  | .ZZC | 300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   9 | %today-6d | transfer |    100 | .ZZA | .ZZB | pie N   |
  |  10 | %today-6d | rebate   |      5 | ctty | .ZZA | rebate on #5 |
  |  11 | %today-6d | bonus    |     10 | ctty | .ZZB | bonus on #4  |
  Then balances:
  | id   | r   |*
  | .ZZA | 287 |
  | .ZZB | 564 |
  | .ZZC | 300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  12 | %today-5d | transfer |    100 | .ZZC | .ZZA | labor M |
  |  13 | %today-5d | rebate   |     10 | ctty | .ZZC | rebate on #3 |
  |  14 | %today-5d | bonus    |      5 | ctty | .ZZA | bonus on #6  |
  Then balances:
  | id   | r   |*
  | .ZZA | 392 |
  | .ZZB | 564 |
  | .ZZC | 210 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  15 | %today-4d | transfer |     50 | .ZZB | .ZZC | cash P  |
  Then balances:
  | id   | r   |*
  | .ZZA | 392 |
  | .ZZB | 514 |
  | .ZZC | 260 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  16 | %today-3d | transfer |    120 | .ZZA | .ZZC | this Q  |
  |  17 | %today-3d | rebate   |      6 | ctty | .ZZA | rebate on #7 |
  |  18 | %today-3d | bonus    |     12 | ctty | .ZZC | bonus on #5  |
  Then balances:
  | id   | r   |*
  | .ZZA | 278 |
  | .ZZB | 514 |
  | .ZZC | 392 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  19 | %today-1d | transfer |    100 | .ZZA | .ZZB | cash V  |
  Then balances:
  | id   | r   |*
  | .ZZA | 178 |
  | .ZZB | 614 |
  | .ZZC | 392 |

Scenario: Inflation adjustments are distributed
  When cron runs "lessOften"
  Then transactions: 
  | xid| created| type      | amount                               | from | to   | purpose |*
  | 20 | %today | inflation | %(round(%R_INFLATION_RATE*38.42, 2)) | ctty | .ZZA | inflation adjustment |
  | 21 | %today | inflation | %(round(%R_INFLATION_RATE*23.11, 2)) | ctty | .ZZB | inflation adjustment |
  | 22 | %today | inflation | %(round(%R_INFLATION_RATE*31.52, 2)) | ctty | .ZZC | inflation adjustment |
# A: (21*500 + 510 + 610 + 382 + 287 + 2*(392) + 2*(278) + 173)/30 * R/12 = 38.3389*R = 1.92
# B: (21*200 + 2*190 + 454 + 2*564 + 3*514 + 614)/30 * R/12 = 23.10556*R = 1.16
# C: (22*400 + 3*300 + 210 + 260 + 3*392)/30 * R/12 = 31.5167*R = 1.58