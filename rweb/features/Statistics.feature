Feature: Statistics
AS a member
I WANT accurate, up-to-date system statistics
SO I can see how well the rCredits system is doing for myself, for my ctty, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | id   | fullName   | email | flags                     | minimum | floor | share | created   |
  | .ZZA | Abe One    | a@    | dft,ok,person,bona        |       5 |     0 |    10 | %today-2m |
  | .ZZB | Bea Two    | b@    | dft,ok,person,bona        |    1000 |   -20 |    20 | %today-3w |
  | .ZZC | Corner Pub | b@    | dft,ok,company,bona,payex |    2000 |    10 |    30 | %today-2w |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payer | payee | amount | completed |
  |  100 | .ZZA  |     0 |  -1000 | %today-3d |
  |  101 | .ZZB  |     0 |  -2000 | %today-4d |
  |  102 | .ZZC  |     0 |  -3050 | %today-5d |
  |  103 | .ZZC  |     0 |     50 | %today-2d |
  And balances:
  | id   | usd   |
  | ctty |     0 |
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  And transactions: 
  | xid   | created   | type      | state   | amount | r   | from | to   | purpose | goods |
  | .AAAB | %today-4m | signup    | done    |    250 | 250 | ctty | .ZZA | signup  | 0     |
  | .AAAC | %today-4m | signup    | done    |    250 | 250 | ctty | .ZZB | signup  | 0     |
  | .AAAD | %today-4m | signup    | done    |    250 | 250 | ctty | .ZZC | signup  | 0     |
  | .AAAE | %today-3m | transfer  | done    |     10 |  10 | .ZZB | .ZZA | cash E  | 0     |
  | .AAAF | %today-3m | transfer  | done    |    100 |  20 | .ZZC | .ZZA | usd F   | 0     |
  | .AAAG | %today-3m | transfer  | done    |    240 |  40 | .ZZA | .ZZB | what G  | 1     |
  | .AAAJ | %today-3m | transfer  | pending |    100 | 100 | .ZZA | .ZZB | pie N   | 1     |
  | .AAAM | %today-2m | transfer  | pending |    100 | 100 | .ZZC | .ZZA | labor M | 1     |
  And statistics get set "%tomorrow-1m"
  And transactions: 
  | xid   | created   | type      | state   | amount | r   | from | to   | purpose | goods | channel |
  | .AAAP | %today-2w | transfer  | done    |     50 |   5 | .ZZB | .ZZC | cash P  | 0     | %TX_WEB |
  | .AAAQ | %today-1w | transfer  | done    |    120 |  80 | .ZZA | .ZZC | this Q  | 1     | %TX_WEB |
  | .AAAT | %today-6d | transfer  | pending |    100 | 100 | .ZZA | .ZZB | cash T  | 0     | %TX_WEB |
  | .AAAU | %today-6d | transfer  | pending |    100 | 100 | .ZZB | .ZZA | cash U  | 0     | %TX_WEB |
  | .AAAV | %today-6d | transfer  | done    |    100 |   0 | .ZZA | .ZZB | cash V  | 0     | %TX_WEB |
  | .AAAW | %today-2d | inflation | done    |      1 |   1 | ctty | .ZZA | inflate | 0     | %TX_WEB |
  | .AAAX | %today-2d | inflation | done    |      2 |   2 | ctty | .ZZB | inflate | 0     | %TX_WEB |
  | .AAAY | %today-2d | inflation | done    |      3 |   3 | ctty | .ZZC | inflate | 0     | %TX_WEB |
  | .AAAZ | %today-2d | grant     | done    |      4 |   4 | ctty | .ZZA | grant   | 0     | %TX_WEB |
  | .AABA | %today-2d | loan      | done    |      5 |   5 | ctty | .ZZB | loan    | 0     | %TX_WEB |
  | .AABB | %today-2d | fine      | done    |      6 |   6 | .ZZC | ctty | fine    | 0     | %TX_WEB |
  | .AABC | %today-1d | transfer  | done    |      0 |  50 | .ZZC | .ZZA | payroll | 1     | %TX_WEB |
  | .AABF | %today-1d | transfer  | done    | 1 | 1 | .ZZC | .AAB | sharing rewards with CGF | 1 | %TX_CRON |
  Then balances:
  | id   | r       | usd      | rewards | committed |
  | ctty | -813.15 | 10000.00 |    0.00 |         0 |
  | .ZZA |  233.00 |   690.00 |  269.00 |      1.80 |
  | .ZZB |  306.00 |  2255.00 |  276.00 |      4.80 |
  | .ZZC |  273.05 |  3055.00 |  265.05 |      3.62 |
  | .AAB |    1.10 |     0.00 |    0.10 |         0 |
  # total rewards < total r, because we made a grant, a loan, and a fine.
  When cron runs ""
  # causes coverFee() to run
  Then balances:
  | id   | r       | usd      | rewards    |
  | ctty | -814.65 | 10000.00 |       0.00 |
  | .ZZA |  233.25 |   689.75 |     269.25 |
  | .ZZB |  306.50 |  2254.50 |     276.50 |
  | .ZZC |  273.80 |  3054.25 |     265.80 |
  | .AAB |    1.10 |     0.00 |       0.10 |
  
Scenario: cron calculates the statistics
  Given statistics get set "%REQUEST_TIME"
  When member ".ZZA" visits page "community"
  Then we show "Statistics" with:
  | | for %R_REGION_NAME |
  | Accounts        | 5 (3 personal, 2 companies) — up 5 from a month ago |
  | rCredits issued | $814.65r — up $814.65r from a month ago |
  | | signup: $750r, inflation adjustments: $6r, rebates/bonuses: $55.65r, grants: $4r, loans: $5r |
  | Demand          | $5,998.50 — up $5,998.50 from a month ago |
  | Total funds     | $814.65r + $5,998.50us = $6,813.15 |
  | | including about $6,642.15 in savings = 110.7% of demand (important) |
  | Banking / mo    | $6,050us (in) - $50us (out) - $1.50 (fees) = +$5,998.50us (net) |
  | Purchases / mo  | 3 ($171) / mo = $34.20 / acct |
  | p2p             | 0 ($0) / mo = $0 / acct |
  | p2b             | 1 ($120) / mo = $40 / acct |
  | b2b             | 1 ($1) / mo = $0.50 / acct |
  | b2p             | 1 ($50) / mo = $25 / acct |
  | Velocity        | 2.5% per month |

#  | Trades / mo     | 20 ($192.15) / mo = $38.43 / acct |
#  Then totals:
#  | r      | floor | rewards    | usd     | minimum | signup | rebate | bonus | committed | inflation | grant | loan | fine | maxRebate | balance    | demand   | capacity |
#  | 776.25 |   -10 |     775.25 | 6000.75 |    3005 |    750 |      6 |    12 |      3.80 |         6 |     4 |    5 |    6 |        4 |    -776.25 |   2228.75 |  6000.75 |

  # Here's why:
  #
  # demand is min(usd, minimum-r)
  #   = min(741.75, 5 - 169.25)
  #   + min(2256.50, 1000 - 284.50)
  #   + min(3002.50, 2000 - 322.50)
  #
  # capacity now is usd [was usd or (if virtual) minimum-(r+usd), whichever is less -- but never less than zero]
  #   = 741.75
  #   + 2256.50
  #   + min(3002.50, max(0, 2000 - (322.50 + 3002.50)))  (zero)