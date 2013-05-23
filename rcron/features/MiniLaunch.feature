Feature: MiniLaunch
AS a group of pioneering members
I WANT the first few transactions of the mini-launch to go as expected
SO I will have confidence to continue using the rCredits system.

Setup:
  Given members:
  | id   | fullName   | email         | flags           | minimum | maximum | floor |
  | .A | Abe One    | a@example.com | dft,ok,personal |       5 |       1 |     0 |
  | .ZZB | Bea Two    | b@example.com | dft,ok,personal |    1000 |      50 |   -20 |
  | .ZZC | Corner Pub | b@example.com | dft,ok,company  |    2000 |       0 |    10 |
  And relations:
  | id   | main | agent | permission |
  | .ZZA | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZC | .ZZB  | buy        |
  | .ZZD | .ZZC | .ZZA  | sell       |
  And usd:
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
  | id   | r    | usd      | rewards |
  | ctty | -775 | 10000.00 |       0 |
  | .ZZA |  169 |   741.75 |     257 |
  | .ZZB |  284 |  2256.50 |     256 |
  | .ZZC |  322 |  3002.50 |     261 |
  
Scenario: cron calculates the totals
  When cron runs "totals"
  Then totals:
  | r   | floor | rewards | usd     | minimum | maximum | excess | signup | rebate | bonus | inflation | grant | loan | fine | maxRebate | balance | demand |
  | 775 |   -10 |     774 | 6000.75 |    3005 |     513 |    350 | 750    |      6 |    12 |         6 |     4 |    5 |   6 |          4 |    -775 |   2394 |

