Feature: Monthly
AS a member
I WANT various monthly automatic account calculations and transactions
SO I can support the Common Good System and be supported by it.

Setup:
  Given members:
  | id   | fullName   | floor | acctType    | flags       | rebate | crumbs | city |*
  | .ZZA | Abe One    | -500  | personal    | ok,roundup  |      5 |      0 | Avil |
  | .ZZB | Bea Two    | -500  | personal    | ok,co       |     10 |      0 | Bvil |
  | .ZZC | Corner Pub | -500  | corporation | ok,co,paper |     10 |   0.02 | Cvil |
  And transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   1 | %today-2m | signup   |    100 | ctty | .ZZA | signup  |
  |   2 | %today-2m | signup   |    100 | ctty | .ZZB | signup  |
  |   3 | %today-2m | signup   |    100 | ctty | .ZZC | signup  |
  And usd transfers:
  | payee | amount | completed |*
  | .ZZA  |    400 | %today-2m |  
  | .ZZB  |    100 | %today-2m |  
  | .ZZC  |    300 | %today-2m |  
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     400 |     100 |
  | .ZZB |     100 |     100 |
  | .ZZC |     300 |     100 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-9d | transfer |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     410 |     100 |
  | .ZZB |      90 |     100 |
  | .ZZC |     300 |     100 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   5 | %today-8d | transfer |    100 | .ZZC | .ZZA | usd F   |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     510 |     100 |
  | .ZZB |      90 |     100 |
  | .ZZC |     200 |     100 |
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |   6 | %today-7d | transfer | 240.01 |     12 |    24 | .ZZA | .ZZB | what G  |
  # pennies here and below, to trigger roundup contribution
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |  269.99 |    112 |
  | .ZZB |  330.01 |    124 |
  | .ZZC |  200.00 |    100 |
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |   7 | %today-6d | transfer |  99.99 |      5 |    10 | .ZZA | .ZZB | pie N   |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     170 |     117 |
  | .ZZB |     430 |     134 |
  | .ZZC |     200 |     100 |
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |  8 | %today-5d | transfer |    100 |     10 |     5 | .ZZC | .ZZA | labor M |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     270 |     122 |
  | .ZZB |     430 |     134 |
  | .ZZC |     100 |     110 |
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |  9 | %today-4d | transfer |     50 |      0 |     0 | .ZZB | .ZZC | cash P  |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     270 |     122 |
  | .ZZB |     380 |     134 |
  | .ZZC |     150 |     110 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |  10 | %today-3d | transfer |    120 |      6 |    12 | .ZZA | .ZZC | this Q  |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |     150 |     128 |
  | .ZZB |     380 |     134 |
  | .ZZC |     270 |     122 |
  When transactions: 
  | xid | created   | type     | amount | rebate | bonus | from | to   | purpose |*
  |  11 | %today-1d | transfer |    100 |      0 |     0 | .ZZA | .ZZB | cash V  |
  Then balances:
  | id   | balance | rewards |*
  | .ZZA |      50 |     128 |
  | .ZZB |     480 |     134 |
  | .ZZC |     270 |     122 |

Scenario: Inflation adjustments, round up donations, and crumb donations are made
  When cron runs "everyMonth"
# inflation  
  Then transactions: 
  | xid| created| type      | amount | bonus                               | from | to   | purpose |*
  | 12 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*29.2, 2)) | ctty | .ZZA | %IAOY average balance |
  # 29.6?
  | 13 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *8.8, 2)) | ctty | .ZZA | %IAOY credit reserve  |
  | 14 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*13.6, 2)) | ctty | .ZZB | %IAOY average balance |
  # 14.2?
  | 15 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *9.0, 2)) | ctty | .ZZB | %IAOY credit reserve  |
  | 16 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*22.8, 2)) | ctty | .ZZC | %IAOY average balance |
  # 23.0?
  | 17 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *8.6, 2)) | ctty | .ZZC | %IAOY credit reserve  |
  And member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  And member ".ZZC" cache is ok
  
#  | 20 | %today | inflation | %(round(%R_INFLATION_RATE*38.42, 2)) | ctty | .ZZA | inflation adjustment |
#  | 21 | %today | inflation | %(round(%R_INFLATION_RATE*23.11, 2)) | ctty | .ZZB | inflation adjustment |
#  | 22 | %today | inflation | %(round(%R_INFLATION_RATE*31.52, 2)) | ctty | .ZZC | inflation adjustment |
# Counting savings as part of balance:
# A: (21*500 + 510 + 610 + 382 + 287 + 2*(392) + 2*(278) + 173)/30 * R/12 = 38.3389*R = 1.92
# B: (21*200 + 2*190 + 454 + 2*564 + 3*514 + 614)/30 * R/12 = 23.10556*R = 1.16
# C: (22*400 + 3*300 + 210 + 260 + 3*392)/30 * R/12 = 31.5167*R = 1.58

# roundups (creation date is last second of previous month)
  And transactions:
  | xid | created | type     | amount | rebate | bonus | from | to  | purpose       | flags         |*
  | 18  |       ? | transfer |   1.00 |    .05 |   .10 | .ZZA | cgf | roundups desc | gift,roundups |
 
# crumbs (creation date is last second of previous month)
  | 19  |       ? | transfer |   3.40 |    .34 |   .34 | .ZZC | cgf | crumbs desc   | gift,crumbs   |

# alerting admin about paper statements
  And we tell admin "Send paper statements" with subs:
  | list |*
  | Corner Pub (Cvil) |

# distribution of shares to CGCs
  And transactions:
  | xid | created | type     | amount | from | to   | flags |*
  |  20 |       ? | transfer |   2.20 |  cgf | ctty | gift  |
