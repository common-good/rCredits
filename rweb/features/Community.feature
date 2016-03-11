Feature: Statistics
AS a member
I WANT accurate, up-to-date system statistics
SO I can see how well the rCredits system is doing for myself, for my ctty, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | id   | fullName   | rebate | flags      | minimum | floor | share | created   | activated |*
  | .ZZA | Abe One    |      5 | ok,bona    |       5 |     0 |    10 | %today-6m | %today-5m |
  | .ZZB | Bea Two    |      5 | ok,bona    |    1000 |   -20 |    20 | %today-5w | %today-4w |
  | .ZZC | Corner Pub |     10 | ok,co,bona |    2000 |    10 |    30 | %today-4w | %today-3w |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payer | amount | completed |*
  |  100 | .ZZA  |  -1000 | %today-3d |
  |  101 | .ZZB  |  -2000 | %today-4d |
  |  102 | .ZZC  |  -3050 | %today-5d |
  |  103 | .ZZC  |     50 | %today-2d |
  Then balances:
  | id   | r     |*
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  Given transactions: 
  | xid | created   | type      | amount | from | to   | purpose | goods      |*
  |   1 | %today-4m | signup    |    250 | ctty | .ZZA | signup  | %FOR_USD |
  |   2 | %today-4m | signup    |    250 | ctty | .ZZB | signup  | %FOR_USD |
  |   3 | %today-4m | signup    |    250 | ctty | .ZZC | signup  | %FOR_USD |
  |   4 | %today-3m | transfer  |     10 | .ZZB | .ZZA | cash E  | %FOR_USD |
  |   5 | %today-3m | transfer  |    100 | .ZZC | .ZZA | usd F   | %FOR_USD |
  |   6 | %today-3m | transfer  |    240 | .ZZA | .ZZB | what G  | %FOR_GOODS |
  And statistics get set "%tomorrow-1m"
  And transactions: 
  | xid | created   | type      | amount | from | to   | purpose | goods      | channel  |*
  |  15 | %today-2w | transfer  |     50 | .ZZB | .ZZC | p2b     | %FOR_GOODS | %TX_WEB  |
  |  18 | %today-1w | transfer  |    120 | .ZZA | .ZZC | this Q  | %FOR_GOODS | %TX_WEB  |
  |  23 | %today-6d | transfer  |    100 | .ZZA | .ZZB | cash V  | %FOR_USD | %TX_WEB  |
  |  24 | %today-2d | inflation |      1 | ctty | .ZZA | inflate | %FOR_USD | %TX_WEB  |
  |  25 | %today-2d | inflation |      2 | ctty | .ZZB | inflate | %FOR_USD | %TX_WEB  |
  |  26 | %today-2d | inflation |      3 | ctty | .ZZC | inflate | %FOR_USD | %TX_WEB  |
  |  27 | %today-2d | grant     |      4 | ctty | .ZZA | grant   | %FOR_USD | %TX_WEB  |
  |  28 | %today-2d | loan      |      5 | ctty | .ZZB | loan    | %FOR_USD | %TX_WEB  |
  |  29 | %today-2d | fine      |     -6 | ctty | .ZZC | fine    | %FOR_USD | %TX_WEB  |
  |  30 | %today-1d | transfer  |    100 | .ZZC | .ZZA | payroll | %FOR_GOODS | %TX_WEB  |
  |  33 | %today-1d | transfer  |      1 | .ZZC | .AAB | sharing rewards with CGF | %FOR_GOODS | %TX_CRON |
  Then balances:
  | id   | r       | rewards | committed |*
  | .ZZA | 1028.00 |  274.00 |      2.30 |
  | .ZZB | 2551.50 |  266.50 |      2.90 |
  | .ZZC | 3243.10 |  274.10 |      8.13 |
  | .AAB |    1.10 |    0.10 |         0 |
  # total rewards < total r, because we made a grant, a loan, and a fine.
  
Scenario: cron calculates the statistics
  Given cron runs "acctStats"
  And cron runs "cttyStats"
  When member ".ZZA" visits page "community/graphs"
  Then we show "Statistics" with:
  | | for %regionName |
  |_rCredits Accounts: | 2 members + 1 co |
  |_Funds in the rCredits System: | $6,823.70 |
  |_rCredits Circulation Velocity: | 4.0% per mo. |
  |_Monthly Bank Transfers | $6,000 (net) |
  |_rCredits Issued To-Date | $6,823.70 |
  |_fees | $-6 |
  |_Monthly Transactions | 4 @ $67.75 |
# only 2 members and 1 company -- CGF and WWS are not counted because they have no transactions before today
  
#  | Accounts        | 5 (3 personal, 2 companies) — up 5 from a month ago |
#  | rCredits issued | $835.90r — up $835.90r from a month ago |
#  | | signup: $750r, inflation adjustments: $6r, rebates/bonuses: $70.90r, grants: $4r, loans: $5r, fees: $-6r |
#  | Demand          | $5,999.75 — up $5,999.75 from a month ago |
#  | Total funds     | $835.90r + $5,999.75us = $6,835.65 |
#  | | including about $6,564.65 in savings = 109.4% of demand (important why?) |
#  | Banking / mo    | $6,050us (in) - $50us (out) - $0.25 (fees) = +$5,999.75us (net) |
#  | Purchases / mo  | 4 ($271) / mo = $54.20 / acct |
#  | p2p             | 0 ($0) / mo = $0 / acct |
#  | p2b             | 2 ($170) / mo = $56.67 / acct |
#  | b2b             | 1 ($1) / mo = $0.50 / acct |
#  | b2p             | 1 ($100) / mo = $50 / acct |
#  | Velocity        | 4.0% per month |
