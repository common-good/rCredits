Feature: Totals
AS a member
I WANT accurate system totals
SO I can see how well the rCredits system is doing for myself, for my community, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | id   | fullName   | email         | flags           | minimum | maximum | floor |
  | .ZZA | Abe One    | a@example.com | dft,ok,personal |       5 |       1 |     0 |
  | .ZZB | Bea Two    | b@example.com | dft,ok,personal |    1000 |      50 |   -20 |
  | .ZZC | Corner Pub | b@example.com | dft,ok,company  |    2000 |       0 |    10 |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd:
  | id        | usd   |
  | community | 10000 |
  | .ZZA      |  1000 |
  | .ZZB      |  2000 |
  | .ZZC      |  3000 |
  And transactions: 
  | xid   | created   | type     | state   | amount | r   | from      | to   | purpose | taking |
  | .AAAB | %today-7m | signup   | done    |    250 | 250 | community | .ZZA | signup  | 0      |
  | .AAAC | %today-6m | signup   | done    |    250 | 250 | community | .ZZB | signup  | 0      |
  | .AAAD | %today-6m | signup   | done    |    250 | 250 | community | .ZZC | signup  | 0      |
  | .AAAE | %today-5m | transfer | done    |     10 |  10 | .ZZB      | .ZZA | cash E  | 0      |
  | .AAAF | %today-4m | transfer | done    |    100 |  20 | .ZZC      | .ZZA | usd F   | 1      |
  | .AAAG | %today-3m | transfer | done    |    240 |  40 | .ZZA      | .ZZB | what G  | 0      |
  | .AAAH | %today-3m | rebate   | done    |      2 |   2 | community | .ZZA | rebate  | 0      |
  | .AAAI | %today-3m | bonus    | done    |      4 |   4 | community | .ZZB | bonus   | 0      |
  | .AAAJ | %today-3w | transfer | pending |    100 | 100 | .ZZA      | .ZZB | pie N   | 1      |
  | .AAAK | %today-3w | rebate   | pending |      5 |   5 | community | .ZZA | rebate  | 0      |
  | .AAAL | %today-3w | bonus    | pending |     10 |  10 | community | .ZZB | bonus   | 0      |
  | .AAAM | %today-2w | transfer | pending |    100 | 100 | .ZZC      | .ZZA | labor M | 0      |
  | .AAAN | %today-2w | rebate   | pending |      5 |   5 | community | .ZZC | rebate  | 0      |
  | .AAAO | %today-2w | bonus    | pending |     10 |  10 | community | .ZZA | bonus   | 0      |
  | .AAAP | %today-2w | transfer | done    |     50 |   5 | .ZZB      | .ZZC | cash P  | 0      |
  | .AAAQ | %today-1w | transfer | done    |    120 |  80 | .ZZA      | .ZZC | this Q  | 1      |
  | .AAAR | %today-1w | rebate   | done    |      4 |   4 | community | .ZZA | rebate  | 0      |
  | .AAAS | %today-1w | bonus    | done    |      8 |   8 | community | .ZZC | bonus   | 0      |
  | .AAAT | %today-6d | transfer | pending |    100 | 100 | .ZZA      | .ZZB | cash T  | 0      |
  | .AAAU | %today-6d | transfer | pending |    100 | 100 | .ZZB      | .ZZA | cash U  | 1      |
  | .AAAV | %today-6d | transfer | done    |    100 |   0 | .ZZA      | .ZZB | cash V  | 0      |
  | .AAAW | %today-2d | inflation| done    |      1 |   1 | community | .ZZB |inflation| 0      |
  | .AAAX | %today-2d | inflation| done    |      2 |   2 | community | .ZZB |inflation| 0      |
  | .AAAY | %today-2d | inflation| done    |      3 |   3 | community | .ZZB |inflation| 0      |
  | .AAAZ | %today-2d | grant    | done    |      4 |   2 | community | .ZZB | grant   | 0      |
  | .AABA | %today-2d | loan     | done    |      5 |   3 | community | .ZZB | loan    | 0      |
  | .AABB | %today-2d | fine     | done    |      6 |   4 | community | .ZZB | fine    | 1      |
  Then balances:
  | id        | r    | usd      | rewards |
  | community | -768 | 10000.00 |       0 |
  | .ZZA      |  166 |   739.75 |     256 |
  | .ZZB      |  279 |  2254.50 |     254 |
  | .ZZC      |  323 |  3004.50 |     258 |
  
Scenario: cron calculates the totals
  When cron runs "totals"
  Then totals:
  | r   | floor | rewards | usd   | minimum | maximum | signup | rebate | bonus | inflation | grant | loan | fine | maxRebate | balance | demand |
  | 768 |   -10 |     768 | 16000 |    3005 |      51 |    750 |     16 |    32 |         6 |     4 |    5 |   6 |         5 |    -768 |   2237 |

