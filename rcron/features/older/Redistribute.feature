Feature: Redistribute
AS a member
I WANT to trade rCredits automatically for US Dollars and vice-versa, according to my chosen minimum and maximum
SO I can buy with rCredits what I can and have plenty of US Dollars for everything else
# Assumes 1 < R_BUY_CHUNK <= 50

Setup:
  Given members:
  | id   | fullName | floor | minimum | maximum | flags                   |
  | .ZZA | Abe One  |     0 |     100 |     200 | dft,personal,ok         |
  | .ZZB | Bea Two  |     0 |     100 |     200 | dft,personal,ok         |
  | .ZZC | Our Pub  |     0 |     100 |     200 | dft,personal,company,ok |
  | .ZZD | Dee Four |     0 |     100 |      -1 | dft,personal,ok         |

Scenario: a member has too much r
  Given balances:
  | id   | r                | usd | rewards |
  | .ZZA | %(201 + %chunk3) |   0 |      20 |
  | .ZZB |                5 | 120 |      20 |
  | .ZZC |                5 |  60 |      20 |
  | .ZZD |                0 |   0 |       0 |
  When cron runs "redistribute"
  Then transactions:
  | xid   | type     | state | amount | r       | from | to   | purpose               |
  | .AAAB | transfer | done  |      0 | %chunk  | ctty | .ZZB | rCredits/USD exchange |
  | .AAAC | transfer | done  |      0 | %chunk  | ctty | .ZZC | rCredits/USD exchange |
  | .AAAD | transfer | done  |      0 | %chunk  | ctty | .ZZB | rCredits/USD exchange |
  | .AAAE | transfer | done  |      0 | %chunk3 | .ZZA | ctty | rCredits/USD exchange |
  And bank transfers:
  | payer | payee | amount         |
  | .ZZC  |  .ZZA |         %chunk |
  | .ZZB  |  .ZZA |         %chunk |
  | .ZZC  |  .ZZA |         %chunk |
  And notice count is 0
  
Scenario: a member has too much r but too few buyers
  Given balances:
  | id   | r                | usd     | rewards |
  | .ZZA | %(201 + %chunk3) |       0 |      20 |
  | .ZZB | %(100 - %chunk2) | %chunk2 |      20 |
  | .ZZC |              100 |     160 |      20 |
  | .ZZD |                0 |       0 |      20 |
  When cron runs "redistribute"
  Then transactions:
  | xid   | type     | state | amount | r       | from | to   | purpose               |
  | .AAAB | transfer | done  |      0 | %chunk  | ctty | .ZZB | rCredits/USD exchange |
  | .AAAC | transfer | done  |      0 | %chunk  | ctty | .ZZB | rCredits/USD exchange |
  | .AAAD | transfer | done  |      0 | %chunk2 | .ZZA | ctty | rCredits/USD exchange |
  And we tell staff "no buyers" with subs:
  | amount |
  | %chunk |
  And we notice "cannot offload" to member ".ZZA" with subs: ""
  