Feature: Statistics
AS a member
I WANT accurate, up-to-date system statistics
SO I can see how well the rCredits system is doing for myself, for my ctty, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | id   | fullName   | email | flags      | minimum | floor | share | created   |*
  | .ZZA | Abe One    | a@    | ok,bona    |       5 |     0 |    10 | %today-2m |
  | .ZZB | Bea Two    | b@    | ok,bona    |    1000 |   -20 |    20 | %today-3w |
  | .ZZC | Corner Pub | c@    | ok,co,bona |    2000 |    10 |    30 | %today-2w |
  And relations:
  | id   | main | agent | permission |*
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payer | payee | amount | completed |*
  |  100 | .ZZA  |     0 |  -1000 | %today-3d |
  |  101 | .ZZB  |     0 |  -2000 | %today-4d |
  |  102 | .ZZC  |     0 |  -3050 | %today-5d |
  |  103 | .ZZC  |     0 |     50 | %today-2d |
  Then balances:
  | id   | r     |*
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  Given transactions: 
  | xid | created   | type      | amount | from | to   | purpose | goods |*
  |   1 | %today-4m | signup    |    250 | ctty | .ZZA | signup  | 0     |
  |   2 | %today-4m | signup    |    250 | ctty | .ZZB | signup  | 0     |
  |   3 | %today-4m | signup    |    250 | ctty | .ZZC | signup  | 0     |
  |   4 | %today-3m | transfer  |     10 | .ZZB | .ZZA | cash E  | 0     |
  |   5 | %today-3m | transfer  |    100 | .ZZC | .ZZA | usd F   | 0     |
  |   6 | %today-3m | transfer  |    240 | .ZZA | .ZZB | what G  | 1     |
  And statistics get set "%tomorrow-1m"
  And transactions: 
  | xid | created   | type      | amount | from | to   | purpose | goods | channel  |*
  |  15 | %today-2w | transfer  |     50 | .ZZB | .ZZC | p2b     | 1     | %TX_WEB  |
  |  18 | %today-1w | transfer  |    120 | .ZZA | .ZZC | this Q  | 1     | %TX_WEB  |
  |  23 | %today-6d | transfer  |    100 | .ZZA | .ZZB | cash V  | 0     | %TX_WEB  |
  |  24 | %today-2d | inflation |      1 | ctty | .ZZA | inflate | 0     | %TX_WEB  |
  |  25 | %today-2d | inflation |      2 | ctty | .ZZB | inflate | 0     | %TX_WEB  |
  |  26 | %today-2d | inflation |      3 | ctty | .ZZC | inflate | 0     | %TX_WEB  |
  |  27 | %today-2d | grant     |      4 | ctty | .ZZA | grant   | 0     | %TX_WEB  |
  |  28 | %today-2d | loan      |      5 | ctty | .ZZB | loan    | 0     | %TX_WEB  |
  |  29 | %today-2d | fine      |     -6 | ctty | .ZZC | fine    | 0     | %TX_WEB  |
  |  30 | %today-1d | transfer  |    100 | .ZZC | .ZZA | payroll | 1     | %TX_WEB  |
  |  33 | %today-1d | transfer  |      1 | .ZZC | .AAB | sharing rewards with CGF | 1 | %TX_CRON |
  Then balances:
  | id   | r       | rewards | committed |*
  | ctty | -835.65 |    0.00 |         0 |
  | .ZZA | 1033.00 |  279.00 |      2.80 |
  | .ZZB | 2563.50 |  278.50 |      5.30 |
  | .ZZC | 3238.05 |  275.05 |      6.62 |
  | .AAB |    1.10 |    0.10 |         0 |
  # total rewards < total r, because we made a grant, a loan, and a fine.
  
Scenario: cron calculates the statistics
  Given cron runs "acctStats"
  And cron runs "cttyStats"
  When member ".ZZA" visits page "community"
  Then we show "Statistics" with:
  | | for %R_REGION_NAME |
  |_rCredits Accounts: | 2 members + 1 co = 3 |
  |_Funds in the rCredits System: | $12,835.65 |
  |_rCredits Circulation Velocity: | 4.0% per mo. |
  |_Monthly Bank Transfers | $6,000 (net) |
  |_Monthly Transactions | 4 @ $67.75 |
  |_rCredits Issued To-Date | $6,835.65 |
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
