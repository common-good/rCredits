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
  | id   | balance |*
  | .ZZA |     400 |
  | .ZZB |     100 |
  | .ZZC |     300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   4 | %today-9d | transfer |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | id   | balance |*
  | .ZZA |     410 |
  | .ZZB |      90 |
  | .ZZC |     300 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   5 | %today-8d | transfer |    100 | .ZZC | .ZZA | usd F   |
  Then balances:
  | id   | balance |*
  | .ZZA |     510 |
  | .ZZB |      90 |
  | .ZZC |     200 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   6 | %today-7d | transfer | 240.01 | .ZZA | .ZZB | what G  |
  # pennies here and below, to trigger roundup contribution
  Then balances:
  | id   | balance |*
  | .ZZA |  269.99 |
  | .ZZB |  330.01 |
  | .ZZC |  200.00 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   7 | %today-6d | transfer |  99.99 | .ZZA | .ZZB | pie N   |
  Then balances:
  | id   | balance |*
  | .ZZA |     170 |
  | .ZZB |     430 |
  | .ZZC |     200 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   8 | %today-5d | transfer |    100 | .ZZC | .ZZA | labor M |
  Then balances:
  | id   | balance |*
  | .ZZA |     270 |
  | .ZZB |     430 |
  | .ZZC |     100 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |   9 | %today-4d | transfer |     50 | .ZZB | .ZZC | cash P  |
  Then balances:
  | id   | balance |*
  | .ZZA |     270 |
  | .ZZB |     380 |
  | .ZZC |     150 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  10 | %today-3d | transfer |    120 | .ZZA | .ZZC | this Q  |
  Then balances:
  | id   | balance |*
  | .ZZA |     150 |
  | .ZZB |     380 |
  | .ZZC |     270 |
  When transactions: 
  | xid | created   | type     | amount | from | to   | purpose |*
  |  11 | %today-1d | transfer |    100 |  .ZZA | .ZZB | cash V  |
  Then balances:
  | id   | balance |*
  | .ZZA |      50 |
  | .ZZB |     480 |
  | .ZZC |     270 |

Scenario: Inflation adjustments, round up donations, and crumb donations are made
  When cron runs "everyMonth"
  Skip no inflation at present
# inflation  
  Then transactions: 
  | xid| created| type      | amount | bonus                               | from | to   | purpose |*
  | 12 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*29.2, 2)) | ctty | .ZZA | %IAOY average balance |
  # 29.6?
#  | 13 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *8.8, 2)) | ctty | .ZZA | %IAOY credit reserve  |
  | 13 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*13.6, 2)) | ctty | .ZZB | %IAOY average balance |
  # 14.2?
#  | 15 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *9.0, 2)) | ctty | .ZZB | %IAOY credit reserve  |
  | 14 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*22.8, 2)) | ctty | .ZZC | %IAOY average balance |
  # 23.0?
#  | 17 | %today | inflation |      0 | %(round(%R_INFLATION_RATE *8.6, 2)) | ctty | .ZZC | %IAOY credit reserve  |
Resume
  Then member ".ZZA" cache is ok
  And member ".ZZB" cache is ok
  And member ".ZZC" cache is ok
  
# roundups (creation date is last second of previous month)
  And transactions:
  | xid | created | type     | amount | from | to  | purpose       | flags         |*
  | 12  |       ? | transfer |   1.00 | .ZZA | cgf | roundups desc | gift,roundups |
 
# crumbs (creation date is last second of previous month)
  | 13  |       ? | transfer |   3.40 | .ZZC | cgf | crumbs desc   | gift,crumbs   |

# alerting admin about paper statements
  And we tell admin "Send paper statements" with subs:
  | list |*
  | Corner Pub (Cvil) |

# NO (Seedpack gets no distribution) distribution of shares to CGCs
#  And transactions:
#  | xid | created | type     | amount | from | to   | flags |*
#  |  20 |       ? | transfer |   2.20 |  cgf | ctty | gift  |
