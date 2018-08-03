Feature: Statistics
AS a member
I WANT accurate, up-to-date system statistics
SO I can see how well the rCredits system is doing for myself, for my ctty, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | id   | fullName   | email | flags                     | minimum | floor | share |
  | .ZZA | Abe One    | a@    | dft,ok,person,bona        |       5 |     0 |    10 |
  | .ZZB | Bea Two    | b@    | dft,ok,person,bona        |    1000 |   -20 |    20 |
  | .ZZC | Corner Pub | b@    | dft,ok,company,bona,payex |    2000 |    10 |    30 |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And balances:
  | id   | usd   |
  | ctty | 10000 |
  | .ZZA |  1000 |
  | .ZZB |  2000 |
  | .ZZC |  3000 |
  And transactions: 
  | xid   | created   | type      | state   | amount | r   | from | to   | purpose | taking |
  | .AAAB | %today-7m | signup    | done    |    250 | 250 | ctty | .ZZA | signup  | 0      |
  | .AAAC | %today-6m | signup    | done    |    250 | 250 | ctty | .ZZB | signup  | 0      |
  | .AAAD | %today-6m | signup    | done    |    250 | 250 | ctty | .ZZC | signup  | 0      |
  | .AAAE | %today-5m | transfer  | done    |     10 |  10 | .ZZB | .ZZA | cash E  | 0      |
  | .AAAF | %today-4m | transfer  | done    |    100 |  20 | .ZZC | .ZZA | usd F   | 1      |
  | .AAAG | %today-3m | transfer  | done    |    240 |  40 | .ZZA | .ZZB | what G  | 0      |
  | .AAAH | %today-3m | rebate    | done    |      2 |   2 | ctty | .ZZA | rebate  | 0      |
  | .AAAI | %today-3m | bonus     | done    |      4 |   4 | ctty | .ZZB | bonus   | 0      |
  | .AAAJ | %today-3w | transfer  | pending |    100 | 100 | .ZZA | .ZZB | pie N   | 1      |
  | .AAAK | %today-3w | rebate    | pending |      5 |   5 | ctty | .ZZA | rebate  | 0      |
  | .AAAL | %today-3w | bonus     | pending |     10 |  10 | ctty | .ZZB | bonus   | 0      |
  | .AAAM | %today-2w | transfer  | pending |    100 | 100 | .ZZC | .ZZA | labor M | 0      |
  | .AAAN | %today-2w | rebate    | pending |      5 |   5 | ctty | .ZZC | rebate  | 0      |
  | .AAAO | %today-2w | bonus     | pending |     10 |  10 | ctty | .ZZA | bonus   | 0      |
  | .AAAP | %today-2w | transfer  | done    |     50 |   5 | .ZZB | .ZZC | cash P  | 0      |
  | .AAAQ | %today-1w | transfer  | done    |    120 |  80 | .ZZA | .ZZC | this Q  | 1      |
  | .AAAR | %today-1w | rebate    | done    |      4 |   4 | ctty | .ZZA | rebate  | 0      |
  | .AAAS | %today-1w | bonus     | done    |      8 |   8 | ctty | .ZZC | bonus   | 0      |
  | .AAAT | %today-6d | transfer  | pending |    100 | 100 | .ZZA | .ZZB | cash T  | 0      |
  | .AAAU | %today-6d | transfer  | pending |    100 | 100 | .ZZB | .ZZA | cash U  | 1      |
  | .AAAV | %today-6d | transfer  | done    |    100 |   0 | .ZZA | .ZZB | cash V  | 0      |
  | .AAAW | %today-2d | inflation | done    |      1 |   1 | ctty | .ZZA | inflate | 0      |
  | .AAAX | %today-2d | inflation | done    |      2 |   2 | ctty | .ZZB | inflate | 0      |
  | .AAAY | %today-2d | inflation | done    |      3 |   3 | ctty | .ZZC | inflate | 0      |
  | .AAAZ | %today-2d | grant     | done    |      4 |   2 | ctty | .ZZA | grant   | 0      |
  | .AABA | %today-2d | loan      | done    |      5 |   3 | ctty | .ZZB | loan    | 0      |
  | .AABB | %today-2d | fine      | done    |      6 |   4 | .ZZC | ctty | fine    | 1      |
  Then balances:
  | id   | r    | usd      | rewards | committed |
  | ctty | -775 | 10000.00 |       0 |         0 |
  | .ZZA |  169 |   742.00 |     257 |      0.60 |
  | .ZZB |  284 |  2257.00 |     256 |      0.80 |
  | .ZZC |  322 |  3003.00 |     261 |      2.40 |
  # total rewards < total r, because we made a grant, a loan, and a fine.
  When cron runs ""
  # causes coverFee() to run
  Then balances:
  | id   | r       | usd      | rewards    |
  | ctty | -776.25 | 10000.00 |       0.00 |
  | .ZZA |  169.25 |   741.75 |     257.25 |
  | .ZZB |  284.50 |  2256.50 |     256.50 |
  | .ZZC |  322.50 |  3002.50 |     261.50 |
  
Scenario: cron calculates the statistics
  Given cron runs "stats"
  When member ".ZZA" visits page "stats"
  Then we show "Statistics" with values: ""

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